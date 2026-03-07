#include "Driver.h"
#include "Colors.h"
#include "Localization.h"
#include <winioctl.h>
#include <chrono>
#include <ctime>
#include <cstdio>

// IOCTL definitions
#define SIOCTL_TYPE 40000

static DWORD MakeCTL(DWORD func) {
    return (SIOCTL_TYPE << 16) | (0 << 14) | (func << 2) | 0;
}

static const DWORD IOCTL_HI            = MakeCTL(0x900);
static const DWORD IOCTL_SUBSCRIBE_LOG  = MakeCTL(0x901);
static const DWORD IOCTL_ADD_RULE       = MakeCTL(0x902);
static const DWORD IOCTL_TOGGLE_PROT    = MakeCTL(0x903);

// Globals
std::mutex              g_logMutex;
std::vector<LogEntry>   g_logs;
std::mutex              g_detMutex;
std::vector<Detection>  g_detections;
int                     g_detectionCount = 0;

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    struct tm t;
    localtime_s(&t, &time_t_now);
    char buf[32];
    snprintf(buf, sizeof(buf), "[%02d:%02d:%02d.%01d]",
        t.tm_hour, t.tm_min, t.tm_sec, (int)(ms.count() / 100));
    return buf;
}

std::string GetTimestampShort() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm t;
    localtime_s(&t, &time_t_now);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    return buf;
}

void AddLog(const std::string& msg, LogLevel level) {
    ImVec4 color;
    switch (level) {
        case LOG_INFO:    color = Colors::Blue; break;
        case LOG_SUCCESS: color = Colors::Green; break;
        case LOG_WARNING: color = Colors::Yellow; break;
        case LOG_ERROR:   color = Colors::Red; break;
    }
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logs.push_back({ GetTimestamp(), msg, level, color });
}

// DriverComm implementation
bool DriverComm::Connect() {
    hdl = CreateFileA("\\\\.\\LoLScanDRV", GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hdl == INVALID_HANDLE_VALUE) {
        connected = false;
        return false;
    }

    char input[] = "Hi";
    char output[64] = {};
    DWORD bytesReturned = 0;
    BOOL ok = DeviceIoControl(hdl, IOCTL_HI, input, (DWORD)strlen(input),
        output, sizeof(output), &bytesReturned, nullptr);
    connected = (ok != FALSE);
    return connected;
}

bool DriverComm::AddRule(const char* cmd) {
    if (hdl == INVALID_HANDLE_VALUE) return false;
    char output[64] = {};
    DWORD bytesReturned = 0;
    return DeviceIoControl(hdl, IOCTL_ADD_RULE,
        (LPVOID)cmd, (DWORD)strlen(cmd),
        output, sizeof(output), &bytesReturned, nullptr) != FALSE;
}

bool DriverComm::ToggleProtection(bool active) {
    isDisposing = true;
    if (subscriberThread.joinable()) {
        CancelIoEx(hdl, nullptr);
        subscriberThread.join();
    }
    if (hdl != INVALID_HANDLE_VALUE) {
        CloseHandle(hdl);
        hdl = INVALID_HANDLE_VALUE;
    }
    Sleep(100);
    isDisposing = false;

    hdl = CreateFileA("\\\\.\\LoLScanDRV", GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hdl == INVALID_HANDLE_VALUE) return false;

    const char* cmd = active ? "START" : "STOP";
    char output[64] = {};
    DWORD bytesReturned = 0;
    BOOL ok = DeviceIoControl(hdl, IOCTL_TOGGLE_PROT,
        (LPVOID)cmd, (DWORD)strlen(cmd),
        output, sizeof(output), &bytesReturned, nullptr);

    if (ok) {
        protectionActive = active;
        StartSubscriber();
    }
    return ok != FALSE;
}

void DriverComm::StartSubscriber() {
    if (subscriberThread.joinable()) {
        CancelIoEx(hdl, nullptr);
        subscriberThread.join();
    }
    subscriberThread = std::thread([this]() {
        while (!isDisposing && hdl != INVALID_HANDLE_VALUE) {
            BYTE buffer[1024] = {};
            DWORD bytesReturned = 0;
            BOOL ok = DeviceIoControl(hdl, IOCTL_SUBSCRIBE_LOG,
                nullptr, 0, buffer, sizeof(buffer),
                &bytesReturned, nullptr);

            if (isDisposing) break;

            if (ok && bytesReturned > 0) {
                std::wstring wmsg((wchar_t*)buffer, bytesReturned / sizeof(wchar_t));
                int sz = WideCharToMultiByte(CP_UTF8, 0, wmsg.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string msg(sz > 0 ? sz - 1 : 0, '\0');
                if (sz > 0) WideCharToMultiByte(CP_UTF8, 0, wmsg.c_str(), -1, &msg[0], sz, nullptr, nullptr);

                AddLog(msg, LOG_ERROR);

                if (msg.rfind("BLOCKED ", 0) == 0) {
                    {
                        std::lock_guard<std::mutex> lock(g_detMutex);
                        g_detections.push_back({ GetTimestampShort(), msg.substr(8) });
                        g_detectionCount++;
                    }
                    extern HWND g_hwnd;
                    if (g_hwnd) PostMessage(g_hwnd, WM_USER + 100, 0, 0);
                }
            } else {
                DWORD err = GetLastError();
                if (err == ERROR_OPERATION_ABORTED) {
                    Sleep(50);
                    continue;
                }
                Sleep(100);
            }
        }
    });
}

void DriverComm::Shutdown() {
    isDisposing = true;
    if (hdl != INVALID_HANDLE_VALUE) {
        CancelIoEx(hdl, nullptr);
    }
    if (subscriberThread.joinable())
        subscriberThread.join();
    if (hdl != INVALID_HANDLE_VALUE) {
        CloseHandle(hdl);
        hdl = INVALID_HANDLE_VALUE;
    }
}
