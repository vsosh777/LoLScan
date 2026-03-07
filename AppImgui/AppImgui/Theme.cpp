#include "Theme.h"
#include "Colors.h"
#include "imgui/imgui.h"

void SetupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 14.0f;
    style.ChildRounding     = 10.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;

    style.WindowPadding     = ImVec2(0, 0);
    style.FramePadding      = ImVec2(10, 6);
    style.ItemSpacing       = ImVec2(10, 6);
    style.ItemInnerSpacing  = ImVec2(8, 4);
    style.IndentSpacing     = 18.0f;
    style.ScrollbarSize     = 6.0f;
    style.GrabMinSize       = 8.0f;

    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 0.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;

    style.AntiAliasedLines  = true;
    style.AntiAliasedFill   = true;

    ImVec4* c = style.Colors;

    c[ImGuiCol_Text]                  = Colors::Text;
    c[ImGuiCol_TextDisabled]          = Colors::TextDim;
    c[ImGuiCol_WindowBg]              = Colors::Clear;
    c[ImGuiCol_ChildBg]               = Colors::Clear;
    c[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.09f, 0.95f);
    c[ImGuiCol_Border]                = Colors::Border;
    c[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_FrameBg]               = Colors::FrameBg;
    c[ImGuiCol_FrameBgHovered]        = Colors::FrameBgHov;
    c[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.62f, 1.00f, 0.15f);
    c[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.05f, 0.90f);
    c[ImGuiCol_TitleBgActive]         = ImVec4(0.04f, 0.04f, 0.05f, 0.95f);
    c[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.03f, 0.03f, 0.04f, 0.60f);
    c[ImGuiCol_MenuBarBg]             = ImVec4(0.04f, 0.04f, 0.05f, 0.90f);
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.08f);
    c[ImGuiCol_ScrollbarGrab]         = Colors::ScrollBar;
    c[ImGuiCol_ScrollbarGrabHovered]  = Colors::ScrollBarHov;
    c[ImGuiCol_ScrollbarGrabActive]   = Colors::AccentDim;
    c[ImGuiCol_CheckMark]             = Colors::Accent;
    c[ImGuiCol_SliderGrab]            = Colors::SliderGrab;
    c[ImGuiCol_SliderGrabActive]      = Colors::Accent;
    c[ImGuiCol_Button]                = ImVec4(0.10f, 0.12f, 0.16f, 0.45f);
    c[ImGuiCol_ButtonHovered]         = ImVec4(0.14f, 0.18f, 0.24f, 0.55f);
    c[ImGuiCol_ButtonActive]          = ImVec4(0.22f, 0.62f, 1.00f, 0.35f);
    c[ImGuiCol_Header]                = ImVec4(0.22f, 0.62f, 1.00f, 0.08f);
    c[ImGuiCol_HeaderHovered]         = ImVec4(0.22f, 0.62f, 1.00f, 0.15f);
    c[ImGuiCol_HeaderActive]          = ImVec4(0.22f, 0.62f, 1.00f, 0.22f);
    c[ImGuiCol_Separator]             = Colors::Separator;
    c[ImGuiCol_SeparatorHovered]      = Colors::AccentDim;
    c[ImGuiCol_SeparatorActive]       = Colors::Accent;
    c[ImGuiCol_ResizeGrip]            = ImVec4(0.22f, 0.62f, 1.00f, 0.06f);
    c[ImGuiCol_ResizeGripHovered]     = ImVec4(0.22f, 0.62f, 1.00f, 0.20f);
    c[ImGuiCol_ResizeGripActive]      = ImVec4(0.22f, 0.62f, 1.00f, 0.40f);
    c[ImGuiCol_Tab]                   = ImVec4(0.06f, 0.065f, 0.08f, 0.55f);
    c[ImGuiCol_TabHovered]            = ImVec4(0.22f, 0.62f, 1.00f, 0.18f);
    c[ImGuiCol_TabSelected]           = ImVec4(0.22f, 0.62f, 1.00f, 0.12f);
    c[ImGuiCol_TabDimmed]             = ImVec4(0.04f, 0.04f, 0.05f, 0.55f);
    c[ImGuiCol_TabDimmedSelected]     = ImVec4(0.08f, 0.085f, 0.10f, 0.55f);
    c[ImGuiCol_PlotLines]             = Colors::Accent;
    c[ImGuiCol_PlotLinesHovered]      = Colors::AccentHover;
    c[ImGuiCol_PlotHistogram]         = Colors::Accent;
    c[ImGuiCol_PlotHistogramHovered]  = Colors::AccentHover;
    c[ImGuiCol_TableHeaderBg]         = ImVec4(0.06f, 0.065f, 0.08f, 0.75f);
    c[ImGuiCol_TableBorderStrong]     = Colors::Border;
    c[ImGuiCol_TableBorderLight]      = ImVec4(0.16f, 0.18f, 0.22f, 0.15f);
    c[ImGuiCol_TableRowBg]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(0.05f, 0.055f, 0.065f, 0.25f);
    c[ImGuiCol_TextSelectedBg]        = ImVec4(0.22f, 0.62f, 1.00f, 0.18f);
    c[ImGuiCol_DragDropTarget]        = Colors::Accent;
    c[ImGuiCol_NavCursor]             = Colors::Accent;
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.22f, 0.62f, 1.00f, 0.45f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.40f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
}
