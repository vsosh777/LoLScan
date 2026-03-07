#pragma once
#include "imgui/imgui.h"

namespace Colors {
    // Backgrounds — translucent layers for depth
    static const ImVec4 Clear            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    static const ImVec4 WindowBg         = ImVec4(0.04f, 0.04f, 0.05f, 0.42f);
    static const ImVec4 SidebarBg        = ImVec4(0.035f, 0.035f, 0.045f, 0.88f);
    static const ImVec4 PanelBg          = ImVec4(0.06f, 0.065f, 0.08f, 0.55f);
    static const ImVec4 PanelBgHover     = ImVec4(0.08f, 0.085f, 0.10f, 0.62f);
    static const ImVec4 CardBg           = ImVec4(0.07f, 0.075f, 0.09f, 0.50f);

    // Accent — electric cyan-blue
    static const ImVec4 Accent           = ImVec4(0.22f, 0.62f, 1.00f, 1.00f);
    static const ImVec4 AccentDim        = ImVec4(0.22f, 0.62f, 1.00f, 0.45f);
    static const ImVec4 AccentHover      = ImVec4(0.35f, 0.72f, 1.00f, 1.00f);
    static const ImVec4 AccentBright     = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);
    static const ImVec4 AccentGlow       = ImVec4(0.22f, 0.62f, 1.00f, 0.12f);

    // Text — high contrast hierarchy
    static const ImVec4 Text             = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
    static const ImVec4 TextDim          = ImVec4(0.50f, 0.52f, 0.56f, 1.00f);
    static const ImVec4 TextMuted        = ImVec4(0.36f, 0.38f, 0.42f, 1.00f);

    // UI surfaces
    static const ImVec4 Border           = ImVec4(0.20f, 0.22f, 0.26f, 0.22f);
    static const ImVec4 Separator        = ImVec4(0.22f, 0.24f, 0.28f, 0.18f);
    static const ImVec4 FrameBg          = ImVec4(0.08f, 0.085f, 0.10f, 0.55f);
    static const ImVec4 FrameBgHov       = ImVec4(0.11f, 0.12f, 0.14f, 0.65f);
    static const ImVec4 SliderGrab       = ImVec4(0.22f, 0.62f, 1.00f, 0.80f);
    static const ImVec4 ScrollBar        = ImVec4(0.20f, 0.22f, 0.26f, 0.30f);
    static const ImVec4 ScrollBarHov     = ImVec4(0.28f, 0.30f, 0.35f, 0.50f);

    // Status
    static const ImVec4 Green            = ImVec4(0.30f, 0.85f, 0.50f, 1.00f);
    static const ImVec4 Red              = ImVec4(0.95f, 0.30f, 0.25f, 1.00f);
    static const ImVec4 Yellow           = ImVec4(1.00f, 0.80f, 0.15f, 1.00f);
    static const ImVec4 Blue             = ImVec4(0.35f, 0.65f, 0.95f, 1.00f);

    // Toggle
    static const ImVec4 ToggleOff        = ImVec4(0.20f, 0.22f, 0.26f, 1.00f);
    static const ImVec4 ToggleOn         = ImVec4(0.22f, 0.62f, 1.00f, 1.00f);
    static const ImVec4 ToggleKnob       = ImVec4(0.96f, 0.97f, 0.98f, 1.00f);

    // Badge backgrounds
    static const ImVec4 BadgeGreen       = ImVec4(0.15f, 0.40f, 0.22f, 0.70f);
    static const ImVec4 BadgeRed         = ImVec4(0.40f, 0.12f, 0.10f, 0.70f);
    static const ImVec4 BadgeBlue        = ImVec4(0.10f, 0.25f, 0.45f, 0.70f);
    static const ImVec4 BadgeYellow      = ImVec4(0.40f, 0.32f, 0.08f, 0.70f);
}
