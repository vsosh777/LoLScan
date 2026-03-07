#include "Blur.h"

#include <dwmapi.h>
#include <vector>
#include <cstring>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

// ============================================================================
// Windows Acrylic/Blur API (undocumented)
// ============================================================================
typedef enum _ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor; // ABGR
    DWORD AnimationId;
} ACCENT_POLICY;

typedef enum _WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
static pSetWindowCompositionAttribute SetWindowCompositionAttribute_ = nullptr;

void EnableAcrylicBlur(HWND hwnd) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (!hUser32) return;
    SetWindowCompositionAttribute_ =
        (pSetWindowCompositionAttribute)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
    if (!SetWindowCompositionAttribute_) return;

    ACCENT_POLICY accent = {};
    accent.AccentState = ACCENT_ENABLE_BLURBEHIND;
    accent.GradientColor = 0x4C201810;
    accent.AccentFlags = 0;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    SetWindowCompositionAttribute_(hwnd, &data);
}

// ============================================================================
// Bitmap helpers
// ============================================================================
static void DestroyBitmap(Gdiplus::Bitmap*& bmp) {
    if (bmp) {
        delete bmp;
        bmp = nullptr;
    }
}

static void ForceBitmapOpaqueAlpha(Gdiplus::Bitmap* bmp) {
    if (!bmp) return;

    const int w = (int)bmp->GetWidth();
    const int h = (int)bmp->GetHeight();
    if (w <= 0 || h <= 0) return;

    Gdiplus::BitmapData bd = {};
    Gdiplus::Rect rect(0, 0, w, h);
    if (bmp->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
        PixelFormat32bppARGB, &bd) != Gdiplus::Ok) {
        return;
    }

    BYTE* row = (BYTE*)bd.Scan0;
    int stride = bd.Stride;
    if (stride < 0) {
        row += (h - 1) * (-stride);
    }

    for (int y = 0; y < h; y++) {
        BYTE* px = (stride >= 0) ? (row + y * stride) : (row - y * (-stride));
        for (int x = 0; x < w; x++) {
            px[x * 4 + 3] = 255;
        }
    }

    bmp->UnlockBits(&bd);
}

// ============================================================================
// Kawase Blur Core
// ============================================================================
Gdiplus::Bitmap* CaptureAndBlurRect(const RECT& rc, int w, int h, const KawaseBlurParams& params) {
    if (w <= 0 || h <= 0) return nullptr;

    HDC screenDC = GetDC(nullptr);
    if (!screenDC) return nullptr;

    HDC capDC = CreateCompatibleDC(screenDC);
    if (!capDC) {
        ReleaseDC(nullptr, screenDC);
        return nullptr;
    }

    HBITMAP capBmp = CreateCompatibleBitmap(screenDC, w, h);
    if (!capBmp) {
        DeleteDC(capDC);
        ReleaseDC(nullptr, screenDC);
        return nullptr;
    }

    HGDIOBJ oldBmp = SelectObject(capDC, capBmp);
    BitBlt(capDC, 0, 0, w, h, screenDC, rc.left, rc.top, SRCCOPY | CAPTUREBLT);
    if (oldBmp) SelectObject(capDC, oldBmp);

    Gdiplus::Bitmap* src = Gdiplus::Bitmap::FromHBITMAP(capBmp, nullptr);

    DeleteObject(capBmp);
    DeleteDC(capDC);
    ReleaseDC(nullptr, screenDC);

    if (!src) return nullptr;

    const int passes = (params.passes > 0) ? params.passes : 1;
    const int div = (params.downsampleDiv > 1) ? params.downsampleDiv : 2;

    int dw = w / div;
    int dh = h / div;
    if (dw < 1) dw = 1;
    if (dh < 1) dh = 1;

    Gdiplus::Bitmap* current = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
    {
        Gdiplus::Graphics gc(current);
        gc.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
        gc.DrawImage(src, 0, 0, w, h);
    }
    delete src;

    Gdiplus::Bitmap down(dw, dh, PixelFormat32bppARGB);
    for (int p = 0; p < passes; p++) {
        {
            Gdiplus::Graphics gd(&down);
            gd.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
            gd.DrawImage(current, 0, 0, dw, dh);
        }
        {
            Gdiplus::Graphics gu(current);
            gu.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            gu.DrawImage(&down, 0, 0, w, h);
        }
    }

    if (params.applyTint) {
        Gdiplus::Graphics gTint(current);
        Gdiplus::SolidBrush overlay(Gdiplus::Color(params.tintA, params.tintR, params.tintG, params.tintB));
        gTint.FillRectangle(&overlay, 0, 0, (INT)current->GetWidth(), (INT)current->GetHeight());
    }

    ForceBitmapOpaqueAlpha(current);

    return current;
}

// ============================================================================
// Async Blur Workers
// ============================================================================
void StartBlurWorker(AsyncBlurWorker& worker) {
    worker.stop = false;
    worker.thread = std::thread([&worker]() {
        while (true) {
            RECT reqRect = {};
            int reqW = 0;
            int reqH = 0;
            KawaseBlurParams reqParams = {};
            unsigned long long reqId = 0;

            {
                std::unique_lock<std::mutex> lock(worker.mtx);
                worker.cv.wait(lock, [&worker]() { return worker.stop || worker.hasRequest; });
                if (worker.stop) break;

                reqRect = worker.requestRect;
                reqW = worker.requestW;
                reqH = worker.requestH;
                reqParams = worker.requestParams;
                reqId = worker.activeRequestId;
                worker.hasRequest = false;
            }

            Gdiplus::Bitmap* blurred = CaptureAndBlurRect(reqRect, reqW, reqH, reqParams);
            if (!blurred) continue;

            std::lock_guard<std::mutex> lock(worker.mtx);
            if (worker.stop) {
                delete blurred;
                break;
            }

            DestroyBitmap(worker.readyBitmap);
            worker.readyBitmap = blurred;
            worker.readyW = reqW;
            worker.readyH = reqH;
            worker.readyRequestId = reqId;
            worker.hasReady = true;
        }
    });
}

void StopBlurWorker(AsyncBlurWorker& worker) {
    {
        std::lock_guard<std::mutex> lock(worker.mtx);
        worker.stop = true;
        worker.hasRequest = false;
    }
    worker.cv.notify_one();

    if (worker.thread.joinable()) {
        worker.thread.join();
    }

    std::lock_guard<std::mutex> lock(worker.mtx);
    DestroyBitmap(worker.readyBitmap);
    worker.hasReady = false;
    worker.readyW = 0;
    worker.readyH = 0;
    worker.readyRequestId = 0;
}

unsigned long long RequestBlurCapture(
    AsyncBlurWorker& worker, const RECT& rc, int w, int h, const KawaseBlurParams& params) {
    if (w <= 0 || h <= 0) return 0;

    std::lock_guard<std::mutex> lock(worker.mtx);
    worker.requestRect = rc;
    worker.requestW = w;
    worker.requestH = h;
    worker.requestParams = params;
    worker.requestId++;
    worker.activeRequestId = worker.requestId;
    worker.hasRequest = true;
    worker.cv.notify_one();
    return worker.requestId;
}

Gdiplus::Bitmap* ConsumeBlurResult(
    AsyncBlurWorker& worker, int& outW, int& outH, unsigned long long* outRequestId) {
    std::lock_guard<std::mutex> lock(worker.mtx);
    if (!worker.hasReady || !worker.readyBitmap) return nullptr;

    Gdiplus::Bitmap* ready = worker.readyBitmap;
    outW = worker.readyW;
    outH = worker.readyH;
    if (outRequestId) *outRequestId = worker.readyRequestId;

    worker.readyBitmap = nullptr;
    worker.readyW = 0;
    worker.readyH = 0;
    worker.readyRequestId = 0;
    worker.hasReady = false;
    return ready;
}

// ============================================================================
// Blur Profiles
// ============================================================================
BlurProfile GetMainBlurProfile(int w, int h) {
    BlurProfile profile;
    profile.passes = 5;
    profile.downsampleDiv = 6;
    profile.intervalMs = 100;
    return profile;
}

BlurProfile GetToastBlurProfile(int toastState) {
    BlurProfile profile;
    profile.passes = 6;
    profile.downsampleDiv = 3;
    if (toastState == 1 || toastState == 3) {
        profile.intervalMs = 50;
    } else {
        profile.intervalMs = 200;
    }
    return profile;
}

// ============================================================================
// Main Window Blur Background (D3D11 texture management)
// ============================================================================
ID3D11ShaderResourceView* g_blurSRV = nullptr;
AsyncBlurWorker g_mainBlurWorker;
AsyncBlurWorker g_toastBlurWorker;

static ID3D11Texture2D* g_blurTex = nullptr;
static int g_blurW = 0, g_blurH = 0;
static bool g_mainBlurHasRect = false;

// Provided by AppImgui.cpp
extern ID3D11Device*        g_pd3dDevice;
extern ID3D11DeviceContext*  g_pd3dDeviceContext;

void InitBlurBg() {
    StartBlurWorker(g_toastBlurWorker);
}

void PrimeMainBlurBgSync() {
    g_mainBlurHasRect = false;
}

void UploadMainBlurBitmap(Gdiplus::Bitmap* bitmap, int w, int h) {
    if (!bitmap || w <= 0 || h <= 0 || !g_pd3dDevice || !g_pd3dDeviceContext) {
        delete bitmap;
        return;
    }

    Gdiplus::BitmapData bd = {};
    Gdiplus::Rect lockRect(0, 0, w, h);
    if (bitmap->LockBits(&lockRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bd) != Gdiplus::Ok) {
        delete bitmap;
        return;
    }

    if (g_blurSRV && (g_blurW != w || g_blurH != h)) {
        g_blurSRV->Release();
        g_blurSRV = nullptr;
    }
    if (g_blurTex && (g_blurW != w || g_blurH != h)) {
        g_blurTex->Release();
        g_blurTex = nullptr;
    }

    bool createdTextureNow = false;
    if (!g_blurTex) {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        if (FAILED(g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_blurTex)) || !g_blurTex) {
            bitmap->UnlockBits(&bd);
            delete bitmap;
            return;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(g_pd3dDevice->CreateShaderResourceView(g_blurTex, &srvDesc, &g_blurSRV)) || !g_blurSRV) {
            g_blurTex->Release();
            g_blurTex = nullptr;
            bitmap->UnlockBits(&bd);
            delete bitmap;
            return;
        }

        g_blurW = w;
        g_blurH = h;
        createdTextureNow = true;
    }

    const int dstPitch = w * 4;
    std::vector<BYTE> upload((size_t)dstPitch * (size_t)h);

    const int srcStrideAbs = (bd.Stride >= 0) ? bd.Stride : -bd.Stride;
    const BYTE* srcBase = (const BYTE*)bd.Scan0;
    if (bd.Stride < 0) {
        srcBase += (h - 1) * srcStrideAbs;
    }

    for (int row = 0; row < h; row++) {
        const BYTE* src = (bd.Stride >= 0)
            ? (srcBase + row * srcStrideAbs)
            : (srcBase - row * srcStrideAbs);
        BYTE* dst = upload.data() + (size_t)row * (size_t)dstPitch;
        memcpy(dst, src, (size_t)dstPitch);

        for (int x = 0; x < w; x++) {
            dst[x * 4 + 3] = 255;
        }
    }

    // Guard against black/zero captures
    {
        const int stepY = (h > 48) ? (h / 48) : 1;
        const int stepX = (w > 64) ? (w / 64) : 1;
        unsigned long long sum = 0;
        unsigned long long sumSq = 0;
        int samples = 0;

        for (int y = 0; y < h; y += stepY) {
            const BYTE* row = upload.data() + (size_t)y * (size_t)dstPitch;
            for (int x = 0; x < w; x += stepX) {
                const BYTE* p = row + x * 4;
                int lum = (p[2] * 3 + p[1] * 6 + p[0]) / 10;
                sum += (unsigned long long)lum;
                sumSq += (unsigned long long)lum * (unsigned long long)lum;
                samples++;
            }
        }

        if (samples > 0) {
            const double mean = (double)sum / (double)samples;
            const double var = ((double)sumSq / (double)samples) - (mean * mean);
            const bool almostUniformBlack = (mean < 3.0 && var < 3.0);
            if (almostUniformBlack) {
                if (createdTextureNow) {
                    if (g_blurSRV) { g_blurSRV->Release(); g_blurSRV = nullptr; }
                    if (g_blurTex) { g_blurTex->Release(); g_blurTex = nullptr; }
                    g_blurW = 0;
                    g_blurH = 0;
                }
                bitmap->UnlockBits(&bd);
                delete bitmap;
                return;
            }
        }
    }

    g_pd3dDeviceContext->UpdateSubresource(g_blurTex, 0, nullptr, upload.data(), (UINT)dstPitch, 0);

    bitmap->UnlockBits(&bd);
    delete bitmap;
}

void UpdateMainBlurBg() {
    // Main window blur is handled by DWM blur-behind (GPU-accelerated).
}

void CleanupBlurBg() {
    StopBlurWorker(g_mainBlurWorker);
    StopBlurWorker(g_toastBlurWorker);

    if (g_blurSRV) { g_blurSRV->Release(); g_blurSRV = nullptr; }
    if (g_blurTex) { g_blurTex->Release(); g_blurTex = nullptr; }
    g_blurW = 0;
    g_blurH = 0;
    g_mainBlurHasRect = false;
}
