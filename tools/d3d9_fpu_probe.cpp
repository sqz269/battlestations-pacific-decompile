// cc13-fpu1 probe: what Direct3D 9 does to the x87 control word of the calling thread, of a
// thread that already exists, and of threads created afterwards. Flags as the image passes them
// at 00B2AFF9 (0x44, fallback 0x24): no D3DCREATE_FPU_PRESERVE.
#include <windows.h>
#include <d3d9.h>
#include <process.h>
#include <stdio.h>

static unsigned short cw() {
    unsigned short v;
    __asm { fnstcw v }
    return v;
}
static void setcw(unsigned short v) {
    __asm { fldcw v }
}

static HANDLE g_go, g_done;
static unsigned short g_seen;
static DWORD WINAPI existing_thread(LPVOID) {
    printf("existing thread at start            : %04X\n", cw());
    WaitForSingleObject(g_go, INFINITE);
    g_seen = cw();
    SetEvent(g_done);
    return 0;
}
static DWORD WINAPI fresh_thread(LPVOID p) { *(unsigned short*)p = cw(); return 0; }
static unsigned __stdcall fresh_crt_thread(void* p) { *(unsigned short*)p = cw(); return 0; }

int main() {
    printf("main before D3D                     : %04X\n", cw());
    g_go = CreateEventA(0, TRUE, FALSE, 0); g_done = CreateEventA(0, TRUE, FALSE, 0);
    HANDLE ex = CreateThread(0, 0, existing_thread, 0, 0, 0);
    Sleep(100);
    HWND w = CreateWindowA("STATIC", "fp1", WS_OVERLAPPEDWINDOW, 0, 0, 320, 240, 0, 0, 0, 0);
    IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
    printf("main after Direct3DCreate9          : %04X\n", cw());
    D3DPRESENT_PARAMETERS pp = {};
    pp.Windowed = TRUE; pp.SwapEffect = D3DSWAPEFFECT_DISCARD; pp.BackBufferFormat = D3DFMT_UNKNOWN;
    pp.hDeviceWindow = w;
    IDirect3DDevice9* dev = 0;
    DWORD flags = 0x44;
    HRESULT hr = d3d->CreateDevice(0, D3DDEVTYPE_HAL, w, flags, &pp, &dev);
    if (FAILED(hr)) { flags = 0x24; hr = d3d->CreateDevice(0, D3DDEVTYPE_HAL, w, flags, &pp, &dev); }
    printf("CreateDevice flags %02X hr %08X\n", flags, hr);
    if (FAILED(hr)) return 1;
    printf("main after CreateDevice             : %04X\n", cw());
    SetEvent(g_go); WaitForSingleObject(g_done, INFINITE);
    printf("existing thread after CreateDevice  : %04X\n", g_seen);
    unsigned short a = 0, b = 0;
    HANDLE t = CreateThread(0, 0, fresh_thread, &a, 0, 0); WaitForSingleObject(t, INFINITE);
    printf("new CreateThread after CreateDevice : %04X\n", a);
    t = (HANDLE)_beginthreadex(0, 0, fresh_crt_thread, &b, 0, 0); WaitForSingleObject(t, INFINITE);
    printf("new _beginthreadex after CreateDev. : %04X\n", b);
    setcw(0x027F);
    dev->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0); dev->BeginScene(); dev->EndScene();
    dev->Present(0, 0, 0, 0);
    printf("main 027F then Clear/Begin/End/Present: %04X\n", cw());
    setcw(0x027F);
    hr = dev->Reset(&pp);
    printf("main 027F then Reset hr %08X        : %04X\n", hr, cw());
    setcw(0x027F);
    hr = dev->TestCooperativeLevel();
    printf("main 027F then TestCooperativeLevel : %04X\n", cw());
    setcw(0x037F);
    IDirect3DDevice9* dev2 = 0;
    hr = d3d->CreateDevice(0, D3DDEVTYPE_HAL, w, flags | D3DCREATE_FPU_PRESERVE, &pp, &dev2);
    printf("main 037F then CreateDevice+FPU_PRESERVE hr %08X: %04X\n", hr, cw());
    if (dev2) dev2->Release();
    dev->Release(); d3d->Release();
    return 0;
}
