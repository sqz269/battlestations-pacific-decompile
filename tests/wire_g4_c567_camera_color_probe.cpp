// Standalone synthetic Win32 regression probe. The report records its cl/link
// command; the callback checks the real fixture device identity, not just color.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "bsp/native_renderer_viewport_clear.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
using Word = std::uint32_t;
alignas(4) unsigned char renderer[0x1bd8]{};
alignas(4) unsigned char camera[0x198]{};
struct Lock { CRITICAL_SECTION section; std::int32_t depth; } lock;
bsp::NativeRendererSynchronizationGlobals globals{};
volatile Word one = 0x3f800000;
bsp::NativeRendererViewportClearContext context(globals, one);
Word received_color, received_depth, received_stencil, calls;
void* expected_device;
void* received_device;
bool guarded;
void put(void* base, std::size_t offset, Word value) {
    std::memcpy(static_cast<unsigned char*>(base) + offset, &value, 4);
}
long __stdcall receive(void* actual_device, Word count, const void* rectangles, Word flags,
    Word color, float depth, Word stencil) {
    if (count != 0 || rectangles || flags != 1) return -1;
    received_device = actual_device;
    received_color = color;
    std::memcpy(&received_depth, &depth, 4);
    received_stencil = stencil;
    guarded = globals.nesting_04 == 1 && lock.depth == 1;
    ++calls;
    return 0;
}
DWORD WINAPI worker(void*) {
    bsp::clear_native_renderer_camera_color(renderer, &context, 0, nullptr,
        1, camera, 0.375f, 7);
    return 0;
}
}
int main() {
    static_assert(sizeof(Lock) == 0x1c);
    void* profile[44]{};
    profile[43] = reinterpret_cast<void*>(&receive); // COM Clear +ACh
    void** device = profile;
    expected_device = &device;
    put(renderer, 0x1a10, reinterpret_cast<Word>(&device));
    put(renderer, 4, reinterpret_cast<Word>(&lock));
    put(camera, 0x190, 0x11223344);
    InitializeCriticalSection(&lock.section);
    EnterCriticalSection(&lock.section);
    globals.mode_00 = globals.observed_mode_01 = 1;
    HANDLE thread = CreateThread(nullptr, 0, &worker, nullptr, 0, nullptr);
    if (!thread) return 1;
    const auto deadline = GetTickCount64() + 5000;
    while (globals.nesting_04 != 1 && GetTickCount64() < deadline) Sleep(0);
    const bool entered = globals.nesting_04 == 1;
    put(camera, 0x190, 0xa1b2c3d4); // live change while the clear waits
    LeaveCriticalSection(&lock.section);
    if (WaitForSingleObject(thread, 5000) != WAIT_OBJECT_0) return 2;
    CloseHandle(thread);
    const bool copied_after_entry = entered && guarded && calls == 1 &&
        received_color == 0xa1b2c3d4 && received_depth == 0x3ec00000 &&
        received_stencil == 7 && lock.depth == 0 && globals.nesting_04 == 0;
    const bool device_identity = received_device == expected_device;
    bsp::clear_native_renderer_camera_color(nullptr, nullptr, 0, nullptr,
        0, nullptr, 0.0f, 0); // zero flags must not read context/camera/device
    DeleteCriticalSection(&lock.section);
    std::printf("camera_color_after_guard=%s device_identity=%s calls=%u color=%08x depth=%08x stencil=%u zero_flags=pass\n",
        copied_after_entry ? "pass" : "FAIL", device_identity ? "pass" : "FAIL",
        calls, received_color, received_depth, received_stencil);
    return copied_after_entry && device_identity && calls == 1 ? 0 : 3;
}
