// AppImgui.cpp - LoLScan Dear ImGui Application
// Backend: DirectX 11 + Win32 with Acrylic Blur

#include "Colors.h"
#include "Localization.h"
#include "Payloads.h"
#include "Driver.h"
#include "Theme.h"
#include "Widgets.h"
#include "Renderer.h"
#include "Blur.h"
#include "Toast.h"
#include "resource.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <tchar.h>
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>

#include <Shlobj.h>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdio>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "dcomp.lib")

// ============================================================================
// DirectX 11 Globals
// ============================================================================
ID3D11Device*            g_pd3dDevice = nullptr;
ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Shared globals (accessed by Renderer.cpp via extern)
HWND                     g_hwnd = nullptr;
const float              g_titleBarH = 36.0f;
bool                     g_isMaximized = false;

// System tray
#define WM_TRAYICON (WM_USER + 1)
#define WM_SHOW_TOAST (WM_USER + 100)
#define ID_TRAY_SHOW 4001
#define ID_TRAY_EXIT 4002
static NOTIFYICONDATAW          g_nid = {};
static bool                     g_reallyQuit = false;

// Font pointers (accessed by Renderer.cpp via extern)
ImFont* g_fontRegular  = nullptr;
ImFont* g_fontMedium   = nullptr;
ImFont* g_fontHeading  = nullptr;
ImFont* g_fontSmall    = nullptr;
ImFont* g_fontMono     = nullptr;
ImFont* g_fontIcons    = nullptr;

// Animation state (accessed by Renderer.cpp via extern)
float g_navIndicatorY   = 0.0f;
float g_navIndicatorAlpha = 1.0f;
float g_tabFadeAlpha    = 1.0f;
int   g_prevTab         = 0;
float g_titleBtnHover[3] = {};
float g_navBtnHover[4]  = {};

// Loading screen state (accessed by Renderer.cpp via extern)
bool  g_loadingDone      = false;
float g_loadingTimer     = 0.0f;
float g_loadingFadeOut   = 1.0f;
const float g_loadingMinTime = 1.2f;

// App state (accessed by Renderer.cpp via extern)
DriverComm g_driver;
bool g_rulesLoaded = false;
int  g_activeTab = 0;
bool g_autoScroll = true;
char g_clipboardBuf[4096] = {};

static bool g_windowDragging = false;
static POINT g_dragStart = {};

// Dynamic tray icon state
enum TrayState { TRAY_OK, TRAY_DETECTION, TRAY_DISABLED };
static TrayState g_trayState = TRAY_DISABLED;
static HICON g_trayIcons[3] = {}; // OK, Detection, Disabled

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
static ULONG_PTR g_gdiplusToken = 0;

static HICON LoadIconFromResource(int resourceId) {
    HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!hRes) return nullptr;
    HGLOBAL hData = LoadResource(nullptr, hRes);
    if (!hData) return nullptr;
    void* pData = LockResource(hData);
    DWORD dataSize = SizeofResource(nullptr, hRes);
    if (!pData || dataSize == 0) return nullptr;

    IStream* pStream = nullptr;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, dataSize);
    if (!hMem) return nullptr;
    void* pMem = GlobalLock(hMem);
    memcpy(pMem, pData, dataSize);
    GlobalUnlock(hMem);
    CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (!pStream) { GlobalFree(hMem); return nullptr; }

    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(pStream);
    pStream->Release();
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
        delete bmp;
        return nullptr;
    }

    int sz = GetSystemMetrics(SM_CXSMICON);
    Gdiplus::Bitmap resized(sz, sz, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&resized);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.DrawImage(bmp, 0, 0, sz, sz);
    delete bmp;

    HICON hIcon = nullptr;
    resized.GetHICON(&hIcon);
    return hIcon;
}

static void UpdateTrayIcon() {
    TrayState newState;
    if (!g_driver.protectionActive) {
        newState = TRAY_DISABLED;
    } else if (g_detectionCount > 0) {
        newState = TRAY_DETECTION;
    } else {
        newState = TRAY_OK;
    }
    if (newState == g_trayState) return;
    g_trayState = newState;
    g_nid.hIcon = g_trayIcons[newState];

    const wchar_t* tips[] = { L"LoLScan - Protected", L"LoLScan - Detection!", L"LoLScan - Disabled" };
    wcscpy_s(g_nid.szTip, tips[newState]);
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Toast HWND (owned here for window creation/destruction lifecycle)
static HWND g_toastHwnd = nullptr;
static IDXGISwapChain1* g_toastSwapChain = nullptr;
static ID3D11RenderTargetView* g_toastRTV = nullptr;

// ============================================================================
// Settings persistence
// ============================================================================
struct AppSettings {
    int  lang;
    bool protectionEnabled;
};

static wchar_t g_settingsPath[MAX_PATH] = {};

static void InitSettingsPath() {
    wchar_t appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
        _snwprintf_s(g_settingsPath, MAX_PATH, L"%s\\LoLScan", appData);
        CreateDirectoryW(g_settingsPath, nullptr);
        _snwprintf_s(g_settingsPath, MAX_PATH, L"%s\\LoLScan\\config.bin", appData);
    }
}

static void LoadSettings(AppSettings& s) {
    s.lang = LANG_EN;
    s.protectionEnabled = false;
    if (g_settingsPath[0] == 0) return;
    FILE* f = nullptr;
    _wfopen_s(&f, g_settingsPath, L"rb");
    if (!f) return;
    fread(&s, sizeof(s), 1, f);
    fclose(f);
    if (s.lang < 0 || s.lang > 1) s.lang = LANG_EN;
}

void SaveSettings() {
    if (g_settingsPath[0] == 0) return;
    AppSettings s;
    s.lang = g_lang;
    s.protectionEnabled = g_driver.protectionActive;
    FILE* f = nullptr;
    _wfopen_s(&f, g_settingsPath, L"wb");
    if (!f) return;
    fwrite(&s, sizeof(s), 1, f);
    fclose(f);
}

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ============================================================================
// DirectX 11 Helpers
// ============================================================================
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (hr == DXGI_ERROR_UNSUPPORTED)
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
            &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain)       { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice)       { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}



// ============================================================================
// Per-frame render helper (called from main loop AND from WM_TIMER during drag)
// ============================================================================
static void RenderFrame() {
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UpdateMainBlurBg();

    float dt = ImGui::GetIO().DeltaTime;
    if (dt > 0.1f) dt = 0.016f;
    g_loadingTimer += dt;
    bool readyToShow = g_loadingTimer >= g_loadingMinTime;

    if (!g_loadingDone) {
        if (readyToShow && g_rulesLoaded) {
            g_loadingDone = true;
        }
        RenderLoadingScreen();
    } else if (g_loadingFadeOut > 0.01f) {
        g_loadingFadeOut = AnimLerp(g_loadingFadeOut, 0.0f, 4.0f);
        RenderUI();
        RenderLoadingScreen();
    } else {
        g_loadingFadeOut = 0.0f;
        RenderUI();
    }

    ImGui::Render();
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);

    UpdateTrayIcon();
}

// ============================================================================
// Window Procedure
// ============================================================================
constexpr UINT_PTR TIMER_MOVESIZE = 1;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_ENTERSIZEMOVE:
        SetTimer(hWnd, TIMER_MOVESIZE, 1, nullptr);
        return 0;
    case WM_EXITSIZEMOVE:
        PrimeMainBlurBgSync(); // force fresh capture once move/size loop ends
        KillTimer(hWnd, TIMER_MOVESIZE);
        return 0;
    case WM_TIMER:
        if (wParam == TIMER_MOVESIZE) {
            RenderFrame();
            return 0;
        }
        break;
    case WM_NCCALCSIZE:
        if (wParam == TRUE) return 0;
        break;
    case WM_NCACTIVATE:
        return TRUE;
    case WM_NCPAINT:
        return 0;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        g_isMaximized = (wParam == SIZE_MAXIMIZED);
        return 0;
    case WM_GETMINMAXINFO: {
        HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hMon, &mi)) {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
            mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
            mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
            mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
            mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
        }
        return 0;
    }
    case WM_NCHITTEST: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        const int border = 6;
        if (pt.y < border) {
            if (pt.x < border) return HTTOPLEFT;
            if (pt.x > rc.right - border) return HTTOPRIGHT;
            return HTTOP;
        }
        if (pt.y > rc.bottom - border) {
            if (pt.x < border) return HTBOTTOMLEFT;
            if (pt.x > rc.right - border) return HTBOTTOMRIGHT;
            return HTBOTTOM;
        }
        if (pt.x < border) return HTLEFT;
        if (pt.x > rc.right - border) return HTRIGHT;
        if (pt.y < (int)g_titleBarH && pt.x < rc.right - 120)
            return HTCAPTION;
        return HTCLIENT;
    }
    case WM_NCLBUTTONDBLCLK:
        if (wParam == HTCAPTION) {
            ShowWindow(hWnd, g_isMaximized ? SW_RESTORE : SW_MAXIMIZE);
            return 0;
        }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_CLOSE:
        if (!g_reallyQuit) {
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        for (int i = 0; i < 3; i++) { if (g_trayIcons[i]) DestroyIcon(g_trayIcons[i]); g_trayIcons[i] = nullptr; }
        PostQuitMessage(0);
        return 0;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        } else if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            wchar_t wShow[64], wExit[64];
            MultiByteToWideChar(CP_UTF8, 0, L(S_TrayShow), -1, wShow, 64);
            MultiByteToWideChar(CP_UTF8, 0, L(S_TrayExit), -1, wExit, 64);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW, wShow);
            AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, wExit);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
            DestroyMenu(hMenu);
        }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_SHOW) {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        } else if (LOWORD(wParam) == ID_TRAY_EXIT) {
            g_reallyQuit = true;
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        return 0;
    case WM_SHOW_TOAST:
        ShowToastNotif();
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Entry Point
// ============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Single instance check
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"Global\\LoLScanSingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(L"LoLScanImGuiClass", nullptr);
        if (existing) {
            ShowWindow(existing, SW_SHOW);
            SetForegroundWindow(existing);
        }
        CloseHandle(hMutex);
        return 0;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LoLScanImGuiClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
    RegisterClassExW(&wc);

    int winW = 1000, winH = 550;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        wc.lpszClassName, L"LoLScan",
        WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
        (screenW - winW) / 2, (screenH - winH) / 2, winW, winH,
        nullptr, nullptr, hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, hInstance);
        return 1;
    }

    EnableAcrylicBlur(g_hwnd);

    // Set up system tray icon (use app icon)
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = g_hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    // Init GDI+ and load tray icons from embedded PNG resources
    Gdiplus::GdiplusStartupInput gdipIn;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdipIn, nullptr);
    InitBlurBg();
    PrimeMainBlurBgSync();
    g_trayIcons[TRAY_OK]        = LoadIconFromResource(IDR_ICON_CHECK);
    g_trayIcons[TRAY_DETECTION] = LoadIconFromResource(IDR_ICON_CROSS);
    g_trayIcons[TRAY_DISABLED]  = LoadIconFromResource(IDR_ICON_WARNING);
    g_trayState = TRAY_DISABLED;
    g_nid.hIcon = g_trayIcons[TRAY_DISABLED];
    wcscpy_s(g_nid.szTip, L"LoLScan - Disabled");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    // Create toast notification window (separate HWND with DWM blur)
    {
        WNDCLASSEXW tc = {};
        tc.cbSize = sizeof(tc);
        tc.lpfnWndProc = ToastWndProc;
        tc.hInstance = hInstance;
        tc.lpszClassName = L"LoLScanToast";
        RegisterClassExW(&tc);

        g_toastHwnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            L"LoLScanToast", nullptr, WS_POPUP,
            0, 0, TOAST_W, TOAST_H,
            nullptr, nullptr, hInstance, nullptr);
    }
    if (g_toastHwnd) {
        // Round the window corners via DWM (Windows 11+)
        const DWORD DWMWA_WINDOW_CORNER_PREFERENCE_ = 33;
        const DWORD DWMWCP_ROUND_ = 2;
        DwmSetWindowAttribute(g_toastHwnd, DWMWA_WINDOW_CORNER_PREFERENCE_, &DWMWCP_ROUND_, sizeof(DWMWCP_ROUND_));

        EnableAcrylicBlur(g_toastHwnd);

        // Create swap chain for toast HWND
        IDXGIDevice* dxgiDev = nullptr;
        IDXGIAdapter* dxgiAdp = nullptr;
        IDXGIFactory2* dxgiFact2 = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDev));
        if (dxgiDev) {
            dxgiDev->GetAdapter(&dxgiAdp);
            if (dxgiAdp) {
                dxgiAdp->GetParent(IID_PPV_ARGS(&dxgiFact2));
                dxgiAdp->Release();
            }
            dxgiDev->Release();
        }
        if (dxgiFact2) {
            DXGI_SWAP_CHAIN_DESC1 scd = {};
            scd.Width = TOAST_W;
            scd.Height = TOAST_H;
            scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            scd.SampleDesc.Count = 1;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.BufferCount = 1;
            scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            dxgiFact2->CreateSwapChainForHwnd(g_pd3dDevice, g_toastHwnd, &scd, nullptr, nullptr, &g_toastSwapChain);
            dxgiFact2->Release();
        }
        if (g_toastSwapChain) {
            ID3D11Texture2D* backBuf = nullptr;
            g_toastSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuf));
            if (backBuf) {
                g_pd3dDevice->CreateRenderTargetView(backBuf, nullptr, &g_toastRTV);
                backBuf->Release();
            }
        }
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    // Load JetBrains Mono fonts
    {
        ImFontConfig cfg;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        cfg.PixelSnapH = true;

        const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesCyrillic();

        g_fontRegular = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf", 15.0f, &cfg, glyphRanges);
        g_fontMedium  = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Medium.ttf",  15.0f, &cfg, glyphRanges);
        g_fontHeading = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Bold.ttf",    22.0f, &cfg, glyphRanges);
        g_fontSmall   = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf", 12.0f, &cfg, glyphRanges);
        g_fontMono    = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf", 13.0f, &cfg, glyphRanges);

        // Load Windows icon font for title bar buttons
        {
            ImFontConfig iconCfg;
            iconCfg.OversampleH = 2;
            iconCfg.OversampleV = 2;
            iconCfg.PixelSnapH = true;
            static const ImWchar iconRanges[] = { 0xE001, 0xF200, 0 };
            g_fontIcons = io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\segmdl2.ttf", 12.0f, &iconCfg, iconRanges);
        }

        if (!g_fontRegular) g_fontRegular = io.Fonts->AddFontDefault();
        if (!g_fontMedium)  g_fontMedium  = g_fontRegular;
        if (!g_fontHeading) g_fontHeading = g_fontRegular;
        if (!g_fontSmall)   g_fontSmall   = g_fontRegular;
        if (!g_fontMono)    g_fontMono    = g_fontRegular;
    }

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    SetupTheme();

    // Initialize toast ImGui context (separate context with own fonts & backends)
    ImGuiContext* mainImGuiCtx = ImGui::GetCurrentContext();
    if (g_toastHwnd && g_toastSwapChain) {
        InitToastContext(g_toastHwnd, g_pd3dDevice, g_pd3dDeviceContext,
                         g_toastSwapChain, g_toastRTV, mainImGuiCtx);
    }

    // Load saved settings
    InitSettingsPath();
    AppSettings savedSettings;
    LoadSettings(savedSettings);
    g_lang = savedSettings.lang;
    bool savedProtection = savedSettings.protectionEnabled;

    // Connect to driver
    AddLog(L(S_AppLoaded), LOG_INFO);
    if (g_driver.Connect()) {
        AddLog(L(S_ConnectedToDriver), LOG_SUCCESS);

        { char _b[128]; snprintf(_b, sizeof(_b), L(S_LoadingRules), g_payloadCount); AddLog(_b, LOG_INFO); }
        std::thread ruleLoader([savedProt = savedProtection]() {
            for (int i = 0; i < g_payloadCount; i++) {
                g_driver.AddRule(g_payloads[i]);
            }
            AddLog(L(S_AllRulesLoaded), LOG_SUCCESS);
            if (savedProt) {
                g_driver.ToggleProtection(true);
                AddLog(L(S_ProtectionStarted), LOG_SUCCESS);
            } else {
                g_driver.StartSubscriber();
            }
            g_rulesLoaded = true;
        });
        ruleLoader.detach();
    } else {
        AddLog(L(S_ErrorConnecting), LOG_ERROR);
        g_rulesLoaded = true;
    }

    // Main loop
    bool running = true;
    while (running) {
        // Determine idle wait based on app state
        int toastState = GetToastState();
        bool needsMainAnim = !g_loadingDone || (g_loadingFadeOut > 0.01f);
        bool toastSliding = (toastState == 1 || toastState == 3);
        bool toastHolding = (toastState == 2);

        DWORD waitMs;
        if (needsMainAnim) {
            waitMs = 0; // main window animating — render ASAP (vsync throttles)
        } else if (toastSliding) {
            waitMs = 16; // ~60fps for smooth slide animation
        } else if (toastHolding) {
            waitMs = 100; // toast is static, just check for timeout / messages
        } else {
            waitMs = 100; // fully idle — tray icon polling
        }

        if (waitMs > 0) {
            MsgWaitForMultipleObjects(0, nullptr, FALSE, waitMs, QS_ALLINPUT);
        }

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }
        if (!running) break;

        RenderFrame();
        RenderToastFrame();
    }

    SaveSettings();

    g_driver.Shutdown();

    // Shutdown toast ImGui context
    ShutdownToastContext();

    // Shutdown main ImGui context
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupBlurBg();

    // Release toast resources
    if (g_toastRTV) { g_toastRTV->Release(); g_toastRTV = nullptr; }
    if (g_toastSwapChain) { g_toastSwapChain->Release(); g_toastSwapChain = nullptr; }

    CleanupDeviceD3D();

    if (g_toastHwnd) DestroyWindow(g_toastHwnd);
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, hInstance);
    UnregisterClassW(L"LoLScanToast", hInstance);

    Gdiplus::GdiplusShutdown(g_gdiplusToken);

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return 0;
}
