// Ombro — Winamp/Cabrio visualization plugin
// A WhiteCap-inspired wireframe math-soundscape visualizer
// AGPL-3.0 License

#include "plugin.h"
#include "renderer.h"
#include "visualization.h"
#include <cstdio>

// --- Globals ---
static winampVisModule g_module = {};
static winampVisHeader g_header = {};
static HWND g_visWindow = nullptr;
static bool g_fullscreen = false;
static RECT g_windowedRect = {100, 100, 900, 700};
static LARGE_INTEGER g_perfFreq = {};
static LARGE_INTEGER g_lastTime = {};

static ombro::Renderer*      g_renderer = nullptr;
static ombro::Visualization* g_vis = nullptr;

static const wchar_t* VIS_CLASS_NAME = L"OmbroVisClass";
static const wchar_t* VIS_WINDOW_TITLE = L"Ombro";

// --- Forward declarations ---
static LRESULT CALLBACK VisWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void ToggleFullscreen(HWND hwnd);

// --- Window procedure ---
static LRESULT CALLBACK VisWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE:
            if (g_fullscreen) {
                ToggleFullscreen(hwnd);
            } else {
                DestroyWindow(hwnd);
            }
            return 0;
        case VK_F11:
            ToggleFullscreen(hwnd);
            return 0;
        case VK_RIGHT:
            if (g_vis) g_vis->NextPreset();
            return 0;
        case VK_LEFT:
            if (g_vis) g_vis->PrevPreset();
            return 0;
        case VK_SPACE:
            if (g_vis) g_vis->RandomPreset();
            return 0;
        default:
            // Forward unhandled keys to host for media controls
            if (g_module.hwndParent)
                PostMessageW(g_module.hwndParent, msg, wParam, lParam);
            return 0;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_VIS_NEXT:   if (g_vis) g_vis->NextPreset();   return 0;
        case ID_VIS_PREV:   if (g_vis) g_vis->PrevPreset();   return 0;
        case ID_VIS_RANDOM: if (g_vis) g_vis->RandomPreset(); return 0;
        case ID_VIS_FS:     ToggleFullscreen(hwnd);            return 0;
        }
        break;

    case WM_LBUTTONDBLCLK:
        ToggleFullscreen(hwnd);
        return 0;

    case WM_SIZE:
        if (g_renderer && wParam != SIZE_MINIMIZED) {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (w > 0 && h > 0)
                g_renderer->Resize(w, h);
        }
        return 0;

    case WM_DESTROY:
        g_visWindow = nullptr;
        return 0;

    case WM_ERASEBKGND:
        return 1; // Prevent flicker
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void ToggleFullscreen(HWND hwnd) {
    if (!g_fullscreen) {
        // Save windowed position
        GetWindowRect(hwnd, &g_windowedRect);

        // Get monitor dimensions
        HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {};
        mi.cbSize = sizeof(mi);
        if (!GetMonitorInfoW(mon, &mi)) return;

        SetWindowLongW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwnd, HWND_TOP,
                     mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
        g_fullscreen = true;
    } else {
        SetWindowLongW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowPos(hwnd, HWND_NOTOPMOST,
                     g_windowedRect.left, g_windowedRect.top,
                     g_windowedRect.right - g_windowedRect.left,
                     g_windowedRect.bottom - g_windowedRect.top,
                     SWP_FRAMECHANGED);
        g_fullscreen = false;
    }
}

// --- Plugin callbacks ---
static void __cdecl VisConfig(winampVisModule* mod) {
    MessageBoxW(mod->hwndParent,
        L"Ombro Visualization\n\n"
        L"Controls:\n"
        L"  Left/Right Arrow \u2014 Switch presets\n"
        L"  Space \u2014 Random preset\n"
        L"  F11 / Double-click \u2014 Toggle fullscreen\n"
        L"  Escape \u2014 Exit fullscreen or close\n",
        L"Ombro Configuration",
        MB_OK | MB_ICONINFORMATION);
}

static int __cdecl VisInit(winampVisModule* mod) {
    mod->spectrumNch = 2;
    mod->waveformNch = 2;
    mod->delayMs = 16;   // ~60 fps
    mod->latencyMs = 0;

    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = VisWndProc;
    wc.hInstance = mod->hDllInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = VIS_CLASS_NAME;
    RegisterClassExW(&wc);

    // Create window
    int startW = g_windowedRect.right - g_windowedRect.left;
    int startH = g_windowedRect.bottom - g_windowedRect.top;
    g_visWindow = CreateWindowExW(
        0, VIS_CLASS_NAME, VIS_WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        g_windowedRect.left, g_windowedRect.top,
        startW, startH,
        nullptr, nullptr, mod->hDllInstance, nullptr);

    if (!g_visWindow) return 1;

    // Get client rect for renderer
    RECT rc;
    GetClientRect(g_visWindow, &rc);
    int clientW = rc.right - rc.left;
    int clientH = rc.bottom - rc.top;
    if (clientW <= 0) clientW = 1;
    if (clientH <= 0) clientH = 1;

    // Initialize renderer
    g_renderer = new ombro::Renderer();
    if (!g_renderer->Init(g_visWindow, clientW, clientH)) {
        delete g_renderer;
        g_renderer = nullptr;
        DestroyWindow(g_visWindow);
        g_visWindow = nullptr;
        return 1;
    }

    // Initialize visualization
    g_vis = new ombro::Visualization();
    g_vis->Init();

    // Register external vis window with host
    if (mod->hwndParent)
        SendMessageW(mod->hwndParent, WM_WA_IPC, (WPARAM)g_visWindow, IPC_SETVISWND);

    // Initialize timing
    QueryPerformanceFrequency(&g_perfFreq);
    if (g_perfFreq.QuadPart == 0) g_perfFreq.QuadPart = 1; // Guard against zero
    QueryPerformanceCounter(&g_lastTime);

    g_fullscreen = false;
    return 0;
}

static int __cdecl VisRender(winampVisModule* mod) {
    if (!g_visWindow || !IsWindow(g_visWindow)) return 1;
    if (!g_renderer || !g_vis) return 1;

    // Handle device lost — attempt recovery
    if (g_renderer->IsDeviceLost()) {
        if (!g_renderer->HandleDeviceLost()) return 1;
    }

    // Compute delta time
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float dt = (g_perfFreq.QuadPart > 0)
        ? (float)(now.QuadPart - g_lastTime.QuadPart) / (float)g_perfFreq.QuadPart
        : 0.016f;
    g_lastTime = now;
    // Clamp dt to avoid large jumps
    if (dt > 0.1f) dt = 0.1f;
    if (dt < 0.0f) dt = 0.0f;

    // Update visualization
    g_vis->Update(dt, mod->spectrumData, mod->waveformData);

    // Compute view-projection matrix
    int w = g_renderer->GetWidth();
    int h = g_renderer->GetHeight();
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    float viewProj[16];
    g_vis->GetViewProjectionMatrix(viewProj, aspect);

    // Render
    g_renderer->BeginFrame(g_vis->GetFadeAmount());
    g_renderer->DrawLines(
        g_vis->GetVertices(), g_vis->GetVertexCount(),
        g_vis->GetIndices(), g_vis->GetIndexCount(),
        viewProj);
    g_renderer->EndFrame();

    // Process window messages
    MSG msg;
    while (PeekMessageW(&msg, g_visWindow, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}

static void __cdecl VisQuit(winampVisModule* mod) {
    // Unregister external vis window
    if (mod->hwndParent)
        SendMessageW(mod->hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);

    if (g_renderer) {
        g_renderer->Shutdown();
        delete g_renderer;
        g_renderer = nullptr;
    }

    if (g_vis) {
        delete g_vis;
        g_vis = nullptr;
    }

    if (g_visWindow) {
        DestroyWindow(g_visWindow);
        g_visWindow = nullptr;
    }

    UnregisterClassW(VIS_CLASS_NAME, mod->hDllInstance);
    g_fullscreen = false;
}

// --- Module enumeration ---
static winampVisModule* __cdecl GetModule(int index) {
    if (index != 0) return nullptr;

    g_module.description = (char*)"Ombro";
    g_module.Config = VisConfig;
    g_module.Init = VisInit;
    g_module.Render = VisRender;
    g_module.Quit = VisQuit;
    return &g_module;
}

// --- DLL exports ---
extern "C" {

__declspec(dllexport) winampVisHeader* __cdecl winampVisGetHeader(HWND hwndParent) {
    g_header.version = VIS_HDRVER;
    g_header.description = (char*)"Ombro v0.1.0 — Wireframe Math-Soundscapes";
    g_header.getModule = GetModule;
    return &g_header;
}

__declspec(dllexport) winampVisHeader* __cdecl cabrioVisGetHeader(HWND hwndParent) {
    return winampVisGetHeader(hwndParent);
}

__declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
    return 0; // Can uninstall immediately
}

} // extern "C"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    (void)hModule; (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}
