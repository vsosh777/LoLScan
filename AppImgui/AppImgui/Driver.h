#pragma once
#include "imgui/imgui.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>

// Log system
enum LogLevel { LOG_INFO, LOG_SUCCESS, LOG_WARNING, LOG_ERROR };

struct LogEntry {
    std::string timestamp;
    std::string message;
    LogLevel level;
    ImVec4 color;
};

struct Detection {
    std::string timestamp;
    std::string command;
};

extern std::mutex              g_logMutex;
extern std::vector<LogEntry>   g_logs;
extern std::mutex              g_detMutex;
extern std::vector<Detection>  g_detections;
extern int                     g_detectionCount;

std::string GetTimestamp();
std::string GetTimestampShort();
void AddLog(const std::string& msg, LogLevel level);

// Driver communication
class DriverComm {
public:
    HANDLE hdl = INVALID_HANDLE_VALUE;
    bool connected = false;
    bool protectionActive = false;
    volatile bool isDisposing = false;
    std::thread subscriberThread;

    bool Connect();
    bool AddRule(const char* cmd);
    bool ToggleProtection(bool active);
    void StartSubscriber();
    void Shutdown();
};
