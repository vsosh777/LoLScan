#include "Toast.h"
#include "Blur.h"
#include "Theme.h"
#include "Driver.h"
#include "Localization.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdio>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Toast window & rendering resources
static HWND g_toastHwnd = nullptr;
static IDXGISwapChain1* g_toastSwapChain = nullptr;
static ID3D11RenderTargetView* g_toastRTV = nullptr;
static ImGuiContext* g_mainImGuiCtx = nullptr;
static ImGuiContext* g_toastImGuiCtx = nullptr;
static ImFont* g_toastFontBold = nullptr;
static ImFont* g_toastFontSmall = nullptr;
static ImFont* g_toastFontIcons = nullptr;

// D3D context (set during init)
static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_deviceCtx = nullptr;

// Toast state
static char g_toastTitle[256] = {};
static char g_toastMsg[1024] = {};
static int g_toastState = 0; // 0=hidden, 1=slide-in, 2=hold, 3=slide-out
static DWORD g_toastStart = 0;
static bool g_toastHoverX = false;

int GetToastState() {
    return g_toastState;
}

void InitToastContext(HWND toastHwnd, ID3D11Device* device, ID3D11DeviceContext* ctx,
                      IDXGISwapChain1* swapChain, ID3D11RenderTargetView* rtv,
                      ImGuiContext* mainCtx) {
    g_toastHwnd = toastHwnd;
    g_device = device;
    g_deviceCtx = ctx;
    g_toastSwapChain = swapChain;
    g_toastRTV = rtv;
    g_mainImGuiCtx = mainCtx;

    g_toastImGuiCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(g_toastImGuiCtx);
    ImGuiIO& toastIO = ImGui::GetIO();
    toastIO.IniFilename = nullptr;
    {
        ImFontConfig cfg;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        cfg.PixelSnapH = true;
        const ImWchar* glyphRanges = toastIO.Fonts->GetGlyphRangesCyrillic();
        g_toastFontBold = toastIO.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Bold.ttf", 15.0f, &cfg, glyphRanges);
        g_toastFontSmall = toastIO.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf", 12.0f, &cfg, glyphRanges);
        if (!g_toastFontBold) g_toastFontBold = toastIO.Fonts->AddFontDefault();
        if (!g_toastFontSmall) g_toastFontSmall = g_toastFontBold;

        // Icon font for close button
        {
            ImFontConfig iconCfg;
            iconCfg.OversampleH = 2;
            iconCfg.OversampleV = 2;
            iconCfg.PixelSnapH = true;
            static const ImWchar iconRanges[] = { 0xE001, 0xF200, 0 };
            g_toastFontIcons = toastIO.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\segmdl2.ttf", 10.0f, &iconCfg, iconRanges);
        }
    }
    ImGui_ImplWin32_Init(g_toastHwnd);
    ImGui_ImplDX11_Init(g_device, g_deviceCtx);
    SetupTheme();
    ImGui::SetCurrentContext(g_mainImGuiCtx);
}

void ShutdownToastContext() {
    if (g_toastImGuiCtx) {
        ImGui::SetCurrentContext(g_toastImGuiCtx);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(g_toastImGuiCtx);
        g_toastImGuiCtx = nullptr;
        // Restore main context so subsequent shutdown calls work
        if (g_mainImGuiCtx) ImGui::SetCurrentContext(g_mainImGuiCtx);
    }
}

void DismissToast() {
    if (g_toastState == 1 || g_toastState == 2) {
        g_toastState = 3;
        g_toastStart = GetTickCount();
    }
}

void RenderToastFrame() {
    if (g_toastState == 0 || !g_toastImGuiCtx || !g_toastSwapChain || !g_toastRTV) return;

    // Advance state machine
    DWORD elapsed = GetTickCount() - g_toastStart;
    float progress = 0.0f;

    switch (g_toastState) {
    case 1:
        progress = (float)elapsed / TOAST_SLIDE_MS;
        if (progress > 1.0f) progress = 1.0f;
        if (elapsed >= (DWORD)TOAST_SLIDE_MS) {
            g_toastState = 2;
            g_toastStart = GetTickCount();
        }
        break;
    case 2:
        progress = 1.0f;
        if (elapsed >= (DWORD)TOAST_HOLD_MS) {
            g_toastState = 3;
            g_toastStart = GetTickCount();
        }
        break;
    case 3:
        progress = 1.0f - (float)elapsed / TOAST_SLIDE_MS;
        if (progress < 0.0f) progress = 0.0f;
        if (elapsed >= (DWORD)TOAST_SLIDE_MS) {
            g_toastState = 0;
            ShowWindow(g_toastHwnd, SW_HIDE);
            return;
        }
        break;
    default:
        return;
    }

    // Calculate position with smoothstep easing
    float eased = progress * progress * (3.0f - 2.0f * progress);
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.right - TOAST_W - 16;
    int yVisible = workArea.bottom - TOAST_H - 16;
    int yHidden = workArea.bottom + 10;
    int y = yHidden + (int)((float)(yVisible - yHidden) * eased);

    SetWindowPos(g_toastHwnd, HWND_TOPMOST, x, y, TOAST_W, TOAST_H,
        SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);

    // Render toast in its own ImGui context
    ImGui::SetCurrentContext(g_toastImGuiCtx);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)TOAST_W, (float)TOAST_H));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

    ImGui::Begin("##Toast", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p(0, 0);
    ImVec2 pMax((float)TOAST_W, (float)TOAST_H);

    // Dark tint overlay (DWM provides the blur behind)
    dl->AddRectFilled(p, pMax, IM_COL32(14, 11, 18, 110), 12.0f);

    // Left accent bar
    dl->AddRectFilled(ImVec2(0, 6), ImVec2(3, (float)TOAST_H - 6),
        IM_COL32(235, 65, 45, 220));

    // Warning dot
    dl->AddCircleFilled(ImVec2(18.5f, 17.5f), 4.5f, IM_COL32(235, 70, 45, 255));

    // Title text
    if (g_toastFontBold) ImGui::PushFont(g_toastFontBold);
    dl->AddText(ImVec2(30, 9), IM_COL32(240, 90, 50, 255), g_toastTitle);
    if (g_toastFontBold) ImGui::PopFont();

    // Close X button
    float xOff = (float)TOAST_W - 22.0f, yOff = 10.0f;
    ImVec2 mousePos = ImGui::GetMousePos();
    bool hoverX = (mousePos.x >= (float)TOAST_W - 30.0f && mousePos.x <= (float)TOAST_W &&
                   mousePos.y >= 0 && mousePos.y <= 28.0f);
    g_toastHoverX = hoverX;
    ImU32 xCol = hoverX ? IM_COL32(200, 200, 210, 220) : IM_COL32(200, 200, 210, 110);
    if (g_toastFontIcons) {
        const char* closeGlyph = "\xEE\xA2\xBB"; // U+E8BB ChromeClose
        float fontSize = g_toastFontIcons->FontSize;
        ImVec2 textSize = g_toastFontIcons->CalcTextSizeA(fontSize, FLT_MAX, 0, closeGlyph);
        float tx = xOff + 5.0f - textSize.x * 0.5f;
        float ty = yOff + 5.0f - textSize.y * 0.5f;
        dl->AddText(g_toastFontIcons, fontSize, ImVec2(tx, ty), xCol, closeGlyph);
    } else {
        dl->AddLine(ImVec2(xOff, yOff), ImVec2(xOff + 10, yOff + 10), xCol, 1.6f);
        dl->AddLine(ImVec2(xOff + 10, yOff), ImVec2(xOff, yOff + 10), xCol, 1.6f);
    }

    // Handle click on X
    if (hoverX && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        DismissToast();

    // Message text
    if (g_toastFontSmall) ImGui::PushFont(g_toastFontSmall);
    dl->PushClipRect(ImVec2(14, 33), ImVec2((float)TOAST_W - 14, (float)TOAST_H - 4), true);
    dl->AddText(ImVec2(14, 33), IM_COL32(210, 215, 225, 215), g_toastMsg);
    dl->PopClipRect();
    if (g_toastFontSmall) ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    ImGui::Render();
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_deviceCtx->OMSetRenderTargets(1, &g_toastRTV, nullptr);
    g_deviceCtx->ClearRenderTargetView(g_toastRTV, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_toastSwapChain->Present(0, 0);

    ImGui::SetCurrentContext(g_mainImGuiCtx);
}

void ShowToastNotif() {
    if (!g_toastHwnd || !g_toastSwapChain) return;

    {
        std::lock_guard<std::mutex> lock(g_detMutex);
        if (g_detections.empty()) return;
        auto& det = g_detections.back();
        snprintf(g_toastTitle, sizeof(g_toastTitle), "%s", L(S_ThreatBlocked));
        snprintf(g_toastMsg, sizeof(g_toastMsg), "%s", det.command.c_str());
    }

    g_toastHoverX = false;
    g_toastState = 1;
    g_toastStart = GetTickCount();
}

LRESULT CALLBACK ToastWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_toastImGuiCtx) {
        ImGuiContext* prev = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(g_toastImGuiCtx);
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            ImGui::SetCurrentContext(prev);
            return true;
        }
        ImGui::SetCurrentContext(prev);
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
