#pragma once
#include "imgui/imgui.h"

// Animation helpers
float AnimLerp(float current, float target, float speed);
ImVec4 AnimLerpColor(ImVec4 current, ImVec4 target, float speed);

// Toggle switch with animated knob
bool ToggleSwitch(const char* label, bool* v);

// Pill-shaped status badge
void StatusBadge(const char* text, ImVec4 bgColor, ImVec4 textColor);

// Section header with accent underline
void SectionHeader(const char* text);

// Glass card container with animated hover glow
void BeginGlassCard(const char* id, ImVec2 size = ImVec2(0, 0));
void EndGlassCard();
void ResetCardCounter();
