#pragma once

enum Lang { LANG_EN = 0, LANG_RU = 1 };
extern int g_lang;

enum Str {
    S_Loading, S_Menu, S_Protection, S_Detections, S_Logs,
    S_LOLBinProtection, S_Online, S_Offline,
    S_Controls, S_EnableProtection, S_AutoScrollLogs,
    S_ProtectionStarted, S_ProtectionStopped,
    S_Status, S_Driver, S_Connected, S_Error,
    S_Active, S_Inactive, S_Found, S_Rules, S_Loaded,
    S_Ready, S_LoadingDots, S_Description,
    S_DetectionsCount, S_Time, S_Command, S_Copy,
    S_EntriesCount, S_VersionRules,
    S_AppLoaded, S_ConnectedToDriver, S_LoadingRules,
    S_AllRulesLoaded, S_ErrorConnecting,
    S_TrayShow, S_TrayExit,
    S_ClearDetections,
    S_ThreatBlocked,
    S_Settings, S_Startup,
    S_AutostartElevated, S_AutostartElevatedHint,
    S_TaskStatus, S_Enabled, S_Disabled,
    S_COUNT
};

extern const char* g_strings[2][S_COUNT];

inline const char* L(Str s) {
    const char* localized = g_strings[g_lang][s];
    return localized ? localized : g_strings[LANG_EN][s];
}
