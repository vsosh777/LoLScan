#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <d3d11.h>
#include <mutex>
#include <condition_variable>
#include <thread>

// ============================================================================
// Windows Acrylic/Blur API (undocumented)
// ============================================================================
void EnableAcrylicBlur(HWND hwnd);

// ============================================================================
// Kawase Blur Pipeline
// ============================================================================
struct KawaseBlurParams {
    int passes = 3;
    int downsampleDiv = 4;
    bool applyTint = false;
    BYTE tintA = 0;
    BYTE tintR = 0;
    BYTE tintG = 0;
    BYTE tintB = 0;
};

struct BlurProfile {
    int passes = 3;
    int downsampleDiv = 4;
    DWORD intervalMs = 60;
};

struct AsyncBlurWorker {
    std::mutex mtx;
    std::condition_variable cv;
    std::thread thread;
    bool stop = false;

    bool hasRequest = false;
    RECT requestRect = {};
    int requestW = 0;
    int requestH = 0;
    KawaseBlurParams requestParams = {};
    unsigned long long requestId = 0;
    unsigned long long activeRequestId = 0;

    bool hasReady = false;
    Gdiplus::Bitmap* readyBitmap = nullptr;
    int readyW = 0;
    int readyH = 0;
    unsigned long long readyRequestId = 0;
};

Gdiplus::Bitmap* CaptureAndBlurRect(const RECT& rc, int w, int h, const KawaseBlurParams& params);
void StartBlurWorker(AsyncBlurWorker& worker);
void StopBlurWorker(AsyncBlurWorker& worker);
unsigned long long RequestBlurCapture(AsyncBlurWorker& worker, const RECT& rc, int w, int h, const KawaseBlurParams& params);
Gdiplus::Bitmap* ConsumeBlurResult(AsyncBlurWorker& worker, int& outW, int& outH, unsigned long long* outRequestId = nullptr);

BlurProfile GetMainBlurProfile(int w, int h);
BlurProfile GetToastBlurProfile(int toastState);

// ============================================================================
// Main Window Blur Background (D3D11 texture from async capture)
// ============================================================================
extern ID3D11ShaderResourceView* g_blurSRV;
extern AsyncBlurWorker g_mainBlurWorker;
extern AsyncBlurWorker g_toastBlurWorker;

void InitBlurBg();
void PrimeMainBlurBgSync();
void UpdateMainBlurBg();
void UploadMainBlurBitmap(Gdiplus::Bitmap* bitmap, int w, int h);
void CleanupBlurBg();
