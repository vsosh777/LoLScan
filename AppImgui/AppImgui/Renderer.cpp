#include "Renderer.h"
#include "Colors.h"
#include "Localization.h"
#include "Widgets.h"
#include "Driver.h"
#include "Payloads.h"
#include "Blur.h"

#include "imgui/imgui.h"
#include <Windows.h>
#include <d3d11.h>
#include <cmath>
#include <cstdio>

// Shared globals defined in AppImgui.cpp
extern HWND  g_hwnd;
static const float g_titleBarH = 36.0f;
extern bool  g_isMaximized;

void SaveSettings(); // from AppImgui.cpp

extern ImFont* g_fontRegular;
extern ImFont* g_fontMedium;
extern ImFont* g_fontHeading;
extern ImFont* g_fontSmall;
extern ImFont* g_fontMono;
extern ImFont* g_fontIcons;

extern float g_navIndicatorY;
extern float g_navIndicatorAlpha;
extern float g_tabFadeAlpha;
extern int   g_prevTab;
extern float g_titleBtnHover[3];
extern float g_navBtnHover[4];

extern bool  g_loadingDone;
extern float g_loadingTimer;
extern float g_loadingFadeOut;
static const float g_loadingMinTime = 1.2f;

extern DriverComm g_driver;
extern bool g_rulesLoaded;
extern int  g_activeTab;
extern bool g_autoScroll;
extern char g_clipboardBuf[4096];

void RenderLoadingScreen() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.05f, 0.95f));

    ImGui::Begin("##LoadingScreen", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    float winW = vp->WorkSize.x;
    float winH = vp->WorkSize.y;
    float cx = vp->WorkPos.x + winW * 0.5f;
    float cy = vp->WorkPos.y + winH * 0.5f - 20.0f;

    float alpha = g_loadingFadeOut;
    float time = g_loadingTimer;

    // --- "L" letter (no background) ---
    if (g_fontHeading) {
        const char* letter = "L";
        ImVec2 textSize = g_fontHeading->CalcTextSizeA(g_fontHeading->FontSize, FLT_MAX, 0, letter);
        float tx = cx - textSize.x * 0.5f;
        float ty = cy - textSize.y * 0.5f;
        ImVec4 letterCol = ImVec4(Colors::Text.x, Colors::Text.y, Colors::Text.z, alpha);
        dl->AddText(g_fontHeading, g_fontHeading->FontSize, ImVec2(tx, ty),
            ImGui::ColorConvertFloat4ToU32(letterCol), letter);
    }

    // --- Material UI style spinner ---
    float radius = 28.0f;
    float thickness = 3.0f;
    int segments = 48;

    float rotSpeed = 4.0f;
    float baseAngle = time * rotSpeed;
    const float PI2 = 3.14159265f * 2.0f;

    float cycle = fmodf(time * 1.4f, 2.0f);
    float sweep;
    if (cycle < 1.0f) {
        float t = cycle;
        t = t * t * (3.0f - 2.0f * t);
        sweep = 0.15f + t * 0.70f;
    } else {
        float t = cycle - 1.0f;
        t = t * t * (3.0f - 2.0f * t);
        sweep = 0.85f - t * 0.70f;
    }

    float startOffset = 0.0f;
    if (cycle >= 1.0f) {
        float t = cycle - 1.0f;
        t = t * t * (3.0f - 2.0f * t);
        startOffset = t * 0.70f * PI2;
    }

    float arcStart = baseAngle + startOffset;
    float arcEnd = arcStart + sweep * PI2;

    ImVec4 arcCol = ImVec4(Colors::Accent.x, Colors::Accent.y, Colors::Accent.z, alpha);
    ImU32 arcColor = ImGui::ColorConvertFloat4ToU32(arcCol);
    float step = (arcEnd - arcStart) / (float)segments;
    for (int i = 0; i < segments; i++) {
        float a1 = arcStart + step * i;
        float a2 = arcStart + step * (i + 1);
        ImVec2 p1(cx + cosf(a1) * radius, cy + sinf(a1) * radius);
        ImVec2 p2(cx + cosf(a2) * radius, cy + sinf(a2) * radius);
        dl->AddLine(p1, p2, arcColor, thickness);
    }

    // --- "Loading" text ---
    if (g_fontSmall) {
        const char* loadText = L(S_Loading);
        ImVec2 textSize = g_fontSmall->CalcTextSizeA(g_fontSmall->FontSize, FLT_MAX, 0, loadText);
        float ltx = cx - textSize.x * 0.5f;
        float lty = cy + radius + 20.0f;
        ImVec4 loadCol = ImVec4(Colors::TextDim.x, Colors::TextDim.y, Colors::TextDim.z, alpha * 0.7f);
        dl->AddText(g_fontSmall, g_fontSmall->FontSize, ImVec2(ltx, lty),
            ImGui::ColorConvertFloat4ToU32(loadCol), loadText);
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void RenderUI() {
    ResetCardCounter();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::Clear);

    ImGui::Begin("##MainWindow", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    float windowW = ImGui::GetWindowWidth();
    float windowH = ImGui::GetWindowHeight();

    // Main window blur is provided by DWM blur-behind (GPU-accelerated).
    drawList->AddRectFilled(
        winPos,
        ImVec2(winPos.x + windowW, winPos.y + windowH),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.031f, 0.024f, 0.039f, 0.38f)));

    float sidebarW = 200.0f;

    // ====================================================================
    // CUSTOM TITLE BAR
    // ====================================================================
    {
        float barH = g_titleBarH;

        drawList->AddRectFilled(
            winPos,
            ImVec2(winPos.x + windowW, winPos.y + barH),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.045f, 0.05f, 0.06f, 0.92f)));

        drawList->AddLine(
            ImVec2(winPos.x, winPos.y + barH),
            ImVec2(winPos.x + windowW, winPos.y + barH),
            ImGui::ColorConvertFloat4ToU32(Colors::Border));

        // App name on left
        {
            float textY = winPos.y + (barH - (g_fontSmall ? g_fontSmall->FontSize : 12.0f)) * 0.5f;
            if (g_fontSmall) {
                drawList->AddText(g_fontSmall, g_fontSmall->FontSize,
                    ImVec2(winPos.x + 14, textY),
                    ImGui::ColorConvertFloat4ToU32(Colors::TextMuted), "LoLScan");
            } else {
                drawList->AddText(ImVec2(winPos.x + 14, textY),
                    ImGui::ColorConvertFloat4ToU32(Colors::TextMuted), "LoLScan");
            }
        }

        // Window control buttons
        float btnW = 38.0f;
        ImVec4 hoverTargets[] = {
            ImVec4(0.3f, 0.3f, 0.35f, 0.4f),
            ImVec4(0.3f, 0.3f, 0.35f, 0.4f),
            ImVec4(0.85f, 0.18f, 0.18f, 0.7f),
        };

        ImGui::SetCursorPos(ImVec2(windowW - btnW * 3, 0));
        for (int i = 0; i < 3; i++) {
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Clear);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::Clear);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::Clear);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::Clear);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);

            char id[16]; snprintf(id, sizeof(id), "##wb%d", i);
            if (ImGui::Button(id, ImVec2(btnW, barH))) {
                if (i == 0) ShowWindow(g_hwnd, SW_MINIMIZE);
                else if (i == 1) ShowWindow(g_hwnd, g_isMaximized ? SW_RESTORE : SW_MAXIMIZE);
                else ShowWindow(g_hwnd, SW_HIDE);
            }

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            float target = active ? 1.2f : (hovered ? 1.0f : 0.0f);
            g_titleBtnHover[i] = AnimLerp(g_titleBtnHover[i], target, 12.0f);

            ImVec2 bMin = ImGui::GetItemRectMin();
            ImVec2 bMax = ImGui::GetItemRectMax();

            if (g_titleBtnHover[i] > 0.01f) {
                float a = g_titleBtnHover[i];
                ImVec4 bg = hoverTargets[i];
                bg.w *= (a > 1.0f) ? 1.0f : a;
                drawList->AddRectFilled(bMin, bMax,
                    ImGui::ColorConvertFloat4ToU32(bg));
            }

            ImVec4 iconNormal = Colors::TextDim;
            ImVec4 iconHover = (i == 2) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : Colors::Text;
            float hoverT = (g_titleBtnHover[i] > 1.0f) ? 1.0f : g_titleBtnHover[i];
            ImVec4 iconColor = ImVec4(
                iconNormal.x + (iconHover.x - iconNormal.x) * hoverT,
                iconNormal.y + (iconHover.y - iconNormal.y) * hoverT,
                iconNormal.z + (iconHover.z - iconNormal.z) * hoverT,
                iconNormal.w + (iconHover.w - iconNormal.w) * hoverT);
            ImU32 iconCol = ImGui::ColorConvertFloat4ToU32(iconColor);

            if (g_fontIcons) {
                const char* glyphs[] = {
                    "\xEE\xA4\xA1",
                    g_isMaximized ? "\xEE\xA4\xA3" : "\xEE\xA4\xA2",
                    "\xEE\xA2\xBB",
                };
                float fontSize = g_fontIcons->FontSize;
                ImVec2 textSize = g_fontIcons->CalcTextSizeA(fontSize, FLT_MAX, 0, glyphs[i]);
                float tx = (bMin.x + bMax.x - textSize.x) * 0.5f;
                float ty = (bMin.y + bMax.y - textSize.y) * 0.5f;
                drawList->AddText(g_fontIcons, fontSize, ImVec2(tx, ty), iconCol, glyphs[i]);
            } else {
                ImFont* f = g_fontMedium ? g_fontMedium : ImGui::GetFont();
                const char* glyph = (i == 0) ? "-" : (i == 1) ? "=" : "x";
                ImVec2 textSize = f->CalcTextSizeA(f->FontSize, FLT_MAX, 0, glyph);
                float tx = (bMin.x + bMax.x - textSize.x) * 0.5f;
                float ty = (bMin.y + bMax.y - textSize.y) * 0.5f;
                drawList->AddText(f, f->FontSize, ImVec2(tx, ty), iconCol, glyph);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            if (i < 2) ImGui::SameLine(0, 0);
        }
    }

    // ====================================================================
    // SIDEBAR
    // ====================================================================
    {
        drawList->AddRectFilled(
            ImVec2(winPos.x, winPos.y + g_titleBarH),
            ImVec2(winPos.x + sidebarW, winPos.y + windowH),
            ImGui::ColorConvertFloat4ToU32(Colors::SidebarBg));

        drawList->AddLine(
            ImVec2(winPos.x + sidebarW, winPos.y + g_titleBarH),
            ImVec2(winPos.x + sidebarW, winPos.y + windowH),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.22f, 0.24f, 0.28f, 0.12f)));

        ImGui::SetCursorPos(ImVec2(0, g_titleBarH));
        ImGui::BeginChild("##Sidebar", ImVec2(sidebarW, windowH - g_titleBarH), false, ImGuiWindowFlags_NoScrollbar);

        // Logo
        ImGui::Dummy(ImVec2(0, 22));
        ImGui::SetCursorPosX(22);
        if (g_fontHeading) ImGui::PushFont(g_fontHeading);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Text);
        ImGui::Text("LoLScan");
        ImGui::PopStyleColor();
        if (g_fontHeading) ImGui::PopFont();

        ImGui::SetCursorPosX(22);
        if (g_fontSmall) ImGui::PushFont(g_fontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
        ImGui::Text("%s", L(S_LOLBinProtection));
        ImGui::PopStyleColor();
        if (g_fontSmall) ImGui::PopFont();

        ImGui::Dummy(ImVec2(0, 28));

        // Navigation
        ImGui::SetCursorPosX(16);
        if (g_fontSmall) ImGui::PushFont(g_fontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
        ImGui::Text("%s", L(S_Menu));
        ImGui::PopStyleColor();
        if (g_fontSmall) ImGui::PopFont();
        ImGui::Dummy(ImVec2(0, 6));

        struct NavItem { Str sid; int tab; };
        NavItem navItems[] = {
            { S_Protection,  0 },
            { S_Detections,  1 },
            { S_Logs,        2 },
        };

        float navYPositions[3] = {};

        for (int i = 0; i < 3; i++) {
            bool isActive = (g_activeTab == navItems[i].tab);

            ImGui::SetCursorPosX(10);
            navYPositions[i] = ImGui::GetCursorScreenPos().y;

            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Clear);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::Clear);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::Clear);

            ImVec4 textColor = ImVec4(
                Colors::TextDim.x + (Colors::Accent.x - Colors::TextDim.x) * g_navBtnHover[i],
                Colors::TextDim.y + (Colors::Accent.y - Colors::TextDim.y) * g_navBtnHover[i],
                Colors::TextDim.z + (Colors::Accent.z - Colors::TextDim.z) * g_navBtnHover[i],
                Colors::TextDim.w + (Colors::Accent.w - Colors::TextDim.w) * g_navBtnHover[i]);
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);

            char btnId[64];
            snprintf(btnId, sizeof(btnId), "%s##nav%d", L(navItems[i].sid), i);
            if (ImGui::Button(btnId, ImVec2(sidebarW - 20, 32))) {
                g_activeTab = navItems[i].tab;
            }

            bool hovered = ImGui::IsItemHovered();
            float target = (isActive || hovered) ? 1.0f : 0.0f;
            g_navBtnHover[i] = AnimLerp(g_navBtnHover[i], target, 10.0f);

            if (g_navBtnHover[i] > 0.01f && !isActive) {
                ImVec2 bMin = ImGui::GetItemRectMin();
                ImVec2 bMax = ImGui::GetItemRectMax();
                ImVec4 hoverBg = ImVec4(0.22f, 0.62f, 1.00f, 0.06f * g_navBtnHover[i]);
                drawList->AddRectFilled(bMin, bMax,
                    ImGui::ColorConvertFloat4ToU32(hoverBg), 6.0f);
            }

            ImGui::PopStyleColor(4);
            ImGui::Dummy(ImVec2(0, 2));
        }

        // Animated active indicator
        {
            float targetY = navYPositions[g_activeTab];
            g_navIndicatorY = AnimLerp(g_navIndicatorY, targetY, 14.0f);

            static bool navInit = false;
            if (!navInit) { g_navIndicatorY = targetY; navInit = true; }

            float iy = g_navIndicatorY;

            drawList->AddRectFilled(
                ImVec2(winPos.x + 10, iy + 2),
                ImVec2(winPos.x + 13, iy + 30),
                ImGui::ColorConvertFloat4ToU32(Colors::Accent), 1.5f);

            drawList->AddRectFilled(
                ImVec2(winPos.x + 13, iy),
                ImVec2(winPos.x + sidebarW - 10, iy + 32),
                ImGui::ColorConvertFloat4ToU32(Colors::AccentGlow), 8.0f);
        }

        // Bottom area
        float bottomY = (windowH - g_titleBarH) - 110;
        ImGui::SetCursorPos(ImVec2(16, bottomY));

        // Separator
        {
            ImVec2 p = ImGui::GetCursorScreenPos();
            drawList->AddRectFilled(
                ImVec2(p.x + 6, p.y), ImVec2(p.x + sidebarW - 38, p.y + 1),
                ImGui::ColorConvertFloat4ToU32(Colors::Separator));
        }
        ImGui::Dummy(ImVec2(0, 10));

        // Language switcher
        {
            ImGui::SetCursorPosX(22);
            ImVec2 flagSize(26, 18);
            float gap = 6.0f;

            for (int li = 0; li < 2; li++) {
                if (li > 0) ImGui::SameLine(0, gap);
                ImVec2 cp = ImGui::GetCursorScreenPos();

                bool selected = (g_lang == li);
                float borderAlpha = selected ? 0.9f : 0.25f;
                ImU32 borderCol = ImGui::ColorConvertFloat4ToU32(
                    ImVec4(Colors::Accent.x, Colors::Accent.y, Colors::Accent.z, borderAlpha));

                if (li == 0) {
                    float sw = flagSize.x, sh = flagSize.y;
                    float stripeH = sh / 5.0f;
                    for (int s = 0; s < 5; s++) {
                        ImU32 c = (s % 2 == 0)
                            ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.80f, 0.15f, 0.15f, 1.0f))
                            : ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        drawList->AddRectFilled(
                            ImVec2(cp.x, cp.y + s * stripeH),
                            ImVec2(cp.x + sw, cp.y + (s + 1) * stripeH), c);
                    }
                    drawList->AddRectFilled(cp, ImVec2(cp.x + sw * 0.4f, cp.y + sh * 0.6f),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.20f, 0.55f, 1.0f)));
                } else {
                    float th = flagSize.y / 3.0f;
                    drawList->AddRectFilled(cp, ImVec2(cp.x + flagSize.x, cp.y + th),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
                    drawList->AddRectFilled(ImVec2(cp.x, cp.y + th), ImVec2(cp.x + flagSize.x, cp.y + 2 * th),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.10f, 0.30f, 0.70f, 1.0f)));
                    drawList->AddRectFilled(ImVec2(cp.x, cp.y + 2 * th), ImVec2(cp.x + flagSize.x, cp.y + flagSize.y),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.80f, 0.10f, 0.10f, 1.0f)));
                }

                drawList->AddRect(cp, ImVec2(cp.x + flagSize.x, cp.y + flagSize.y), borderCol, 2.0f, 0, selected ? 1.5f : 1.0f);

                char fid[16]; snprintf(fid, sizeof(fid), "##lang%d", li);
                if (ImGui::InvisibleButton(fid, flagSize)) {
                    g_lang = li;
                    SaveSettings();
                }
            }
        }

        ImGui::Dummy(ImVec2(0, 6));

        // Status pill
        ImGui::SetCursorPosX(22);
        if (g_driver.connected) {
            StatusBadge(L(S_Online), Colors::BadgeGreen, Colors::Green);
        } else {
            StatusBadge(L(S_Offline), Colors::BadgeRed, Colors::Red);
        }

        ImGui::SetCursorPosX(22);
        if (g_fontSmall) ImGui::PushFont(g_fontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
        ImGui::Text(L(S_VersionRules), g_payloadCount);
        ImGui::PopStyleColor();
        if (g_fontSmall) ImGui::PopFont();

        ImGui::EndChild();
    }

    // ====================================================================
    // MAIN CONTENT
    // ====================================================================
    ImGui::SetCursorPos(ImVec2(sidebarW, g_titleBarH));
    float contentW = windowW - sidebarW;
    float contentAreaH = windowH - g_titleBarH;

    ImGui::BeginChild("##ContentArea", ImVec2(contentW, contentAreaH), false, ImGuiWindowFlags_NoScrollbar);

    if (g_activeTab != g_prevTab) {
        g_tabFadeAlpha = 0.0f;
        g_prevTab = g_activeTab;
    }
    g_tabFadeAlpha = AnimLerp(g_tabFadeAlpha, 1.0f, 8.0f);

    // Content header
    {
        ImGui::Dummy(ImVec2(0, 12));
        ImGui::SetCursorPosX(24);

        const char* tabTitles[] = { L(S_Protection)+2, L(S_Detections)+2, L(S_Logs)+2 };
        if (g_fontHeading) ImGui::PushFont(g_fontHeading);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::Text);
        ImGui::Text("%s", tabTitles[g_activeTab]);
        ImGui::PopStyleColor();
        if (g_fontHeading) ImGui::PopFont();

        ImGui::Dummy(ImVec2(0, 4));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 18));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_tabFadeAlpha);

    float contentH = ImGui::GetContentRegionAvail().y - 12.0f;

    // ====== PROTECTION TAB ======
    if (g_activeTab == 0) {
        ImGui::SetCursorPosX(24);
        ImGui::BeginChild("##ProtContent", ImVec2(contentW - 48, contentH), false);

        float cardW = (ImGui::GetContentRegionAvail().x - 12) * 0.5f;

        // Controls card
        BeginGlassCard("##ControlsCard", ImVec2(cardW, 180));
        {
            SectionHeader(L(S_Controls));
            ImGui::Dummy(ImVec2(0, 2));

            static bool protToggle = false;
            protToggle = g_driver.protectionActive;
            if (ToggleSwitch("##protToggle", &protToggle)) {
                if (protToggle != g_driver.protectionActive) {
                    g_driver.ToggleProtection(protToggle);
                    AddLog(protToggle ? L(S_ProtectionStarted) : L(S_ProtectionStopped),
                           protToggle ? LOG_SUCCESS : LOG_ERROR);
                    SaveSettings();
                }
            }
            ImGui::SameLine(0, 10);
            {
                float toggleH = ImGui::GetFrameHeight() * 0.80f;
                float textH = ImGui::CalcTextSize(L(S_EnableProtection)).y;
                float offset = (toggleH - textH) * 0.5f;
                ImVec2 cp = ImGui::GetCursorPos();
                ImGui::SetCursorPosY(cp.y + offset);
            }
            ImGui::Text("%s", L(S_EnableProtection));

            ImGui::Dummy(ImVec2(0, 4));

            ToggleSwitch("##autoScroll", &g_autoScroll);
            ImGui::SameLine(0, 10);
            {
                float toggleH = ImGui::GetFrameHeight() * 0.80f;
                float textH = ImGui::CalcTextSize(L(S_AutoScrollLogs)).y;
                float offset = (toggleH - textH) * 0.5f;
                ImVec2 cp = ImGui::GetCursorPos();
                ImGui::SetCursorPosY(cp.y + offset);
            }
            ImGui::Text("%s", L(S_AutoScrollLogs));
        }
        EndGlassCard();

        ImGui::SameLine(0, 12);

        // Status card
        BeginGlassCard("##StatusCard", ImVec2(cardW, 180));
        {
            SectionHeader(L(S_Status));
            ImGui::Dummy(ImVec2(0, 2));

            ImGui::Text("%s", L(S_Driver));
            ImGui::SameLine(cardW - 120);
            if (g_driver.connected) {
                StatusBadge(L(S_Connected), Colors::BadgeGreen, Colors::Green);
            } else {
                StatusBadge(L(S_Error), Colors::BadgeRed, Colors::Red);
            }

            ImGui::Text("%s", L(S_Protection)+2);
            ImGui::SameLine(cardW - 120);
            if (g_driver.protectionActive) {
                StatusBadge(L(S_Active), Colors::BadgeGreen, Colors::Green);
            } else {
                StatusBadge(L(S_Inactive), Colors::BadgeYellow, Colors::Yellow);
            }

            ImGui::Text("%s", L(S_Detections)+2);
            ImGui::SameLine(cardW - 120);
            if (g_detectionCount > 0) {
                char buf[32]; snprintf(buf, sizeof(buf), L(S_Found), g_detectionCount);
                StatusBadge(buf, Colors::BadgeRed, Colors::Red);
            } else {
                StatusBadge("0", Colors::BadgeGreen, Colors::Green);
            }
        }
        EndGlassCard();

        ImGui::Dummy(ImVec2(0, 12));

        // Info card
        BeginGlassCard("##InfoCard", ImVec2(ImGui::GetContentRegionAvail().x, 0));
        {
            SectionHeader(L(S_Rules));
            ImGui::Dummy(ImVec2(0, 2));

            ImGui::Text("%s", L(S_Loaded));
            ImGui::SameLine(140);
            if (g_fontMedium) ImGui::PushFont(g_fontMedium);
            ImGui::PushStyleColor(ImGuiCol_Text, g_rulesLoaded ? Colors::Accent : Colors::Yellow);
            ImGui::Text("%d", g_payloadCount);
            ImGui::PopStyleColor();
            if (g_fontMedium) ImGui::PopFont();

            ImGui::Dummy(ImVec2(0, 2));

            ImGui::Text("%s", L(S_Status));
            ImGui::SameLine(140);
            if (g_rulesLoaded) {
                StatusBadge(L(S_Ready), Colors::BadgeGreen, Colors::Green);
            } else {
                StatusBadge(L(S_LoadingDots), Colors::BadgeYellow, Colors::Yellow);
            }

            ImGui::Dummy(ImVec2(0, 10));

            if (g_fontSmall) ImGui::PushFont(g_fontSmall);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
            ImGui::TextWrapped("%s", L(S_Description));
            ImGui::PopStyleColor();
            if (g_fontSmall) ImGui::PopFont();
        }
        EndGlassCard();

        ImGui::EndChild();
    }

    // ====== DETECTIONS TAB ======
    else if (g_activeTab == 1) {
        ImGui::SetCursorPosX(24);
        ImGui::BeginChild("##DetContent", ImVec2(contentW - 48, contentH), false);

        {
            char buf[32]; snprintf(buf, sizeof(buf), L(S_DetectionsCount), g_detectionCount);
            if (g_fontSmall) ImGui::PushFont(g_fontSmall);
            StatusBadge(buf, g_detectionCount > 0 ? Colors::BadgeRed : Colors::BadgeGreen,
                        g_detectionCount > 0 ? Colors::Red : Colors::Green);

            if (g_detectionCount > 0) {
                ImGui::SameLine();
                float textH = ImGui::CalcTextSize(L(S_ClearDetections)).y;
                float rounding = (textH + 6.0f) * 0.5f;
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 3));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.1f, 0.1f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.15f, 0.15f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
                if (ImGui::Button(L(S_ClearDetections))) {
                    std::lock_guard<std::mutex> lock(g_detMutex);
                    g_detections.clear();
                    g_detectionCount = 0;
                }
                ImGui::PopStyleColor(4);
                ImGui::PopStyleVar(2);
            }

            if (g_fontSmall) ImGui::PopFont();
        }
        ImGui::Dummy(ImVec2(0, 6));

        if (g_fontMono) ImGui::PushFont(g_fontMono);

        ImGuiTableFlags tableFlags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
            ImGuiTableFlags_SizingStretchProp;

        float tableH = ImGui::GetContentRegionAvail().y;

        if (ImGui::BeginTable("##DetectionsTable", 2, tableFlags, ImVec2(0, tableH))) {
            ImGui::TableSetupColumn(L(S_Time), ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn(L(S_Command), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            std::lock_guard<std::mutex> lock(g_detMutex);
            ImGuiListClipper clipper;
            clipper.Begin((int)g_detections.size());
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
                    ImGui::Text("%s", g_detections[i].timestamp.c_str());
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
                    ImGui::TextWrapped("%s", g_detections[i].command.c_str());
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("DetCtx");
                        strncpy_s(g_clipboardBuf, g_detections[i].command.c_str(), sizeof(g_clipboardBuf) - 1);
                    }
                }
            }

            if (ImGui::BeginPopup("DetCtx")) {
                if (ImGui::MenuItem(L(S_Copy))) {
                    ImGui::SetClipboardText(g_clipboardBuf);
                }
                ImGui::EndPopup();
            }

            if (g_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndTable();
        }

        if (g_fontMono) ImGui::PopFont();
        ImGui::EndChild();
    }

    // ====== LOGS TAB ======
    else if (g_activeTab == 2) {
        ImGui::SetCursorPosX(24);
        ImGui::BeginChild("##LogContent", ImVec2(contentW - 48, contentH), false);

        {
            std::lock_guard<std::mutex> lock(g_logMutex);
            char buf[32]; snprintf(buf, sizeof(buf), L(S_EntriesCount), (int)g_logs.size());
            if (g_fontSmall) ImGui::PushFont(g_fontSmall);
            StatusBadge(buf, Colors::BadgeBlue, Colors::Accent);
            if (g_fontSmall) ImGui::PopFont();
        }
        ImGui::Dummy(ImVec2(0, 6));

        float logH = ImGui::GetContentRegionAvail().y;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.035f, 0.04f, 0.05f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Border, Colors::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));
        ImGui::BeginChild("##LogScroll", ImVec2(0, logH), true);

        if (g_fontMono) ImGui::PushFont(g_fontMono);

        std::lock_guard<std::mutex> lock(g_logMutex);
        ImGuiListClipper clipper;
        clipper.Begin((int)g_logs.size());
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
                ImGui::Text("%s", g_logs[i].timestamp.c_str());
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, g_logs[i].color);
                ImGui::TextWrapped("%s", g_logs[i].message.c_str());
                ImGui::PopStyleColor();
            }
        }

        if (g_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        if (g_fontMono) ImGui::PopFont();

        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);

        ImGui::EndChild();
    }

    ImGui::PopStyleVar(2); // WindowPadding + Alpha

    ImGui::EndChild(); // ContentArea

    ImGui::End();
}
