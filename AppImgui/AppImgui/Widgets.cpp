#include "Widgets.h"
#include "Colors.h"
#include <cmath>

// Animation state
static float g_toggleAnim[8] = {};
static float g_cardHover[8] = {};
static int g_cardCount = 0;

float AnimLerp(float current, float target, float speed) {
    float dt = ImGui::GetIO().DeltaTime;
    float t = 1.0f - expf(-speed * dt);
    return current + (target - current) * t;
}

ImVec4 AnimLerpColor(ImVec4 current, ImVec4 target, float speed) {
    float dt = ImGui::GetIO().DeltaTime;
    float t = 1.0f - expf(-speed * dt);
    return ImVec4(
        current.x + (target.x - current.x) * t,
        current.y + (target.y - current.y) * t,
        current.z + (target.z - current.z) * t,
        current.w + (target.w - current.w) * t);
}

bool ToggleSwitch(const char* label, bool* v) {
    const float height = ImGui::GetFrameHeight() * 0.80f;
    const float width = height * 1.85f;
    const float radius = height * 0.44f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool pressed = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();
    if (pressed) *v = !*v;

    ImGuiID id = ImGui::GetID(label);
    int slot = (unsigned int)id % 8;
    float target = *v ? 1.0f : 0.0f;
    g_toggleAnim[slot] = AnimLerp(g_toggleAnim[slot], target, 12.0f);
    float t = g_toggleAnim[slot];

    ImVec4 offColor = hovered ? ImVec4(0.26f, 0.28f, 0.32f, 1.0f) : (ImVec4)Colors::ToggleOff;
    ImVec4 trackColor = ImVec4(
        offColor.x + (Colors::ToggleOn.x - offColor.x) * t,
        offColor.y + (Colors::ToggleOn.y - offColor.y) * t,
        offColor.z + (Colors::ToggleOn.z - offColor.z) * t,
        offColor.w + (Colors::ToggleOn.w - offColor.w) * t);
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
        ImGui::ColorConvertFloat4ToU32(trackColor), height * 0.5f);

    if (t > 0.01f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.22f, 0.62f, 1.00f, 0.15f * t)), height * 0.5f);
    }

    float knobX = pos.x + radius + t * (width - radius * 2.0f);
    float knobY = pos.y + height * 0.5f;
    dl->AddCircleFilled(ImVec2(knobX, knobY), radius * 0.76f,
        ImGui::ColorConvertFloat4ToU32(Colors::ToggleKnob));

    return pressed;
}

void StatusBadge(const char* text, ImVec4 bgColor, ImVec4 textColor) {
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float padX = 10.0f, padY = 3.0f;
    float h = textSize.y + padY * 2;
    float w = textSize.x + padX * 2;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
        ImGui::ColorConvertFloat4ToU32(bgColor), h * 0.5f);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY),
        ImGui::ColorConvertFloat4ToU32(textColor), text);

    ImGui::Dummy(ImVec2(w, h));
}

void SectionHeader(const char* text) {
    extern ImFont* g_fontSmall;
    if (g_fontSmall) ImGui::PushFont(g_fontSmall);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMuted);
    ImGui::Text("%s", text);
    ImGui::PopStyleColor();
    if (g_fontSmall) ImGui::PopFont();

    ImVec2 p = ImGui::GetCursorScreenPos();
    float lineW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 accentCol = ImGui::ColorConvertFloat4ToU32(Colors::Accent);
    ImU32 fadeCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.22f, 0.62f, 1.00f, 0.0f));
    dl->AddRectFilledMultiColor(
        ImVec2(p.x, p.y - 1), ImVec2(p.x + lineW * 0.45f, p.y),
        accentCol, fadeCol, fadeCol, accentCol);
    ImGui::Dummy(ImVec2(0, 6));
}

void ResetCardCounter() {
    g_cardCount = 0;
}

void BeginGlassCard(const char* id, ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::Border);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 12));
    ImGui::BeginChild(id, size, true);
}

void EndGlassCard() {
    ImVec2 cMin = ImGui::GetWindowPos();
    ImVec2 cMax = ImVec2(cMin.x + ImGui::GetWindowSize().x, cMin.y + ImGui::GetWindowSize().y);
    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    int slot = g_cardCount % 8;
    g_cardCount++;
    float target = hovered ? 1.0f : 0.0f;
    g_cardHover[slot] = AnimLerp(g_cardHover[slot], target, 8.0f);

    if (g_cardHover[slot] > 0.01f) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float a = g_cardHover[slot];
        ImVec4 glowColor = ImVec4(Colors::Accent.x, Colors::Accent.y, Colors::Accent.z, 0.25f * a);
        dl->AddRect(cMin, cMax, ImGui::ColorConvertFloat4ToU32(glowColor), 10.0f, 0, 1.5f);
    }
}
