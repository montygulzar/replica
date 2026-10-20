#include "ui.h"
#include "roblox.h"
#include "presets.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <dwmapi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

static ID3D11Device*           g_dev = nullptr;
static ID3D11DeviceContext*    g_ctx = nullptr;
static IDXGISwapChain*         g_sc  = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static HWND g_hwnd = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return 1;
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(h, m, w, l);
}

static bool make_overlay(int cx, int cy) {
    WNDCLASSEXW wc{ sizeof(wc), CS_CLASSDC, WndProc, 0, 0, GetModuleHandleW(nullptr),
                   nullptr, nullptr, nullptr, nullptr, L"LarpOverlay", nullptr };
    RegisterClassExW(&wc);
    g_hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
                             wc.lpszClassName, L"Larp", WS_POPUP, 0, 0, cx, cy,
                             nullptr, nullptr, wc.hInstance, nullptr);
    SetLayeredWindowAttributes(g_hwnd, 0, 255, LWA_ALPHA);
    MARGINS m{ -1,-1,-1,-1 };
    DwmExtendFrameIntoClientArea(g_hwnd, &m);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL fl;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_sc, &g_dev, &fl, &g_ctx)))
        return false;

    ID3D11Texture2D* bb = nullptr;
    g_sc->GetBuffer(0, IID_PPV_ARGS(&bb));
    g_dev->CreateRenderTargetView(bb, nullptr, &g_rtv);
    bb->Release();

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    return true;
}

static void toggle_input(bool interactive) {
    LONG ex = GetWindowLongW(g_hwnd, GWL_EXSTYLE);
    if (interactive) ex &= ~WS_EX_TRANSPARENT;
    else             ex |=  WS_EX_TRANSPARENT;
    SetWindowLongW(g_hwnd, GWL_EXSTYLE, ex);
}

void run_ui(Driver& drv, Roblox& rbx, Presets& ps) {
    HWND rblx = FindWindowW(nullptr, L"Roblox");
    RECT r{}; if (rblx) GetClientRect(rblx, &r);
    if (!make_overlay(r.right ? r.right : 1280, r.bottom ? r.bottom : 720)) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);

    bool interactive = true;
    int sel_theme = -1, sel_item = -1;
    MSG msg{};
    bool run = true;
    while (run) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessage(&msg);
            if (msg.message == WM_QUIT) run = false;
        }
        if (GetAsyncKeyState(VK_INSERT) & 1) { interactive = !interactive; toggle_input(interactive); }

        if (rblx) {
            RECT rc; GetWindowRect(rblx, &rc);
            SetWindowPos(g_hwnd, HWND_TOPMOST, rc.left, rc.top,
                         rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({20,20}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({340, 480}, ImGuiCond_Once);
        ImGui::Begin("Larp", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("pid %u  base %p", drv.pid(), (void*)drv.base());
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Themes", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < (int)ps.themes().size(); ++i) {
                if (ImGui::Selectable(ps.themes()[i].name.c_str(), sel_theme == i)) {
                    sel_theme = i;
                    ps.apply_theme(rbx, ps.themes()[i]);
                }
            }
        }
        if (ImGui::CollapsingHeader("Items", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < (int)ps.items().size(); ++i) {
                if (ImGui::Selectable(ps.items()[i].name.c_str(), sel_item == i)) {
                    sel_item = i;
                    ps.apply_item(rbx, ps.items()[i]);
                }
            }
        }

        ImGui::Separator();
        ImGui::TextDisabled("INSERT = toggle mouse");
        ImGui::End();

        ImGui::Render();
        float clr[4]{ 0,0,0,0 };
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_ctx->ClearRenderTargetView(g_rtv, clr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_sc->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_rtv) g_rtv->Release();
    if (g_sc)  g_sc->Release();
    if (g_ctx) g_ctx->Release();
    if (g_dev) g_dev->Release();
}
