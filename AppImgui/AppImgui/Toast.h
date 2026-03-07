#pragma once

#include <Windows.h>

// Toast dimensions and timing
static const int TOAST_W = 340;
static const int TOAST_H = 64;
static const int TOAST_SLIDE_MS = 250;
static const int TOAST_HOLD_MS = 4500;

// Current toast animation state (0=hidden, 1=slide-in, 2=hold, 3=slide-out)
int  GetToastState();

void InitToastContext(HWND toastHwnd, struct ID3D11Device* device, struct ID3D11DeviceContext* ctx,
                      struct IDXGISwapChain1* swapChain, struct ID3D11RenderTargetView* rtv,
                      struct ImGuiContext* mainCtx);
void ShutdownToastContext();

void RenderToastFrame();
void ShowToastNotif();
void DismissToast();

LRESULT CALLBACK ToastWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
