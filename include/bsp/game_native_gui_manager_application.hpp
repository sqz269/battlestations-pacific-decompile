#pragma once
#include <memory>

namespace bsp {
class NativeRenderActualOwners;
struct NativeStringRawPoolContext;
struct NativePlatformFocusOwnersContext;
}

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;
class GameNativeReadOnlyData;

// cc15-gui4 P1, GUI Step 4: the warm actual 88h GUI manager. Default OFF.
// ON builds the image's manager through the complete raw 004C12B0 getter
// (constructor 00AA5D70, vtable 00D5BFCC) at 0073BC19, its first call inside
// 0073BAE0, publishes it in this application's 00F8BC5C cell, registers it with
// the shared 01090AA0 manager through 00BD0C30, and binds the native_gui
// deletion arm so the shared raw drain runs 00AA6540 -> 00AA6340.
//
// Owner split while the sprite bridge still draws: the bridge's
// GameFrontendHost ManagerState stays the 004C12B0 stand-in for every hosted
// caller (resource slots, ready flag, pointer slot). The native block owns its
// constructor state (tree +0C, page vector +14, string list +34/+38, cursor
// +5C/+60, pointer byte +48, +6C); nothing reads it until the Page chain (P3)
// borrows context(). Its resource slots and page vector stay empty, so the
// drain's +28/+2C releases are null-slot no-ops and no page virtual runs.
//
// 00F8BC5C reader census (2026-10-28, for P3 to re-check before it goes ON):
// image absolute-dword hits are only in 004C12B0 (5), 00736250 (5), 00AA0FF0,
// 00AA1F70 and 00AA6340. Every other reader goes through 004C12B0 (307 call
// sites in 192 functions). In bsp_game every hosted 004C12B0 call resolves to
// gui_manager_get_or_create_004c12b0 (game_hosts_frontend.cpp), which returns
// the bridge ManagerState, never this cell; the reconstructed callers' host
// hooks have no production implementation; Lua IsGUIActive (008CA010) uses
// the bridge's widget lookup. No cc9 file reads the cell or the stand-in.
inline constexpr bool kGuiManagerCc15Bound = false;

class GameNativeGuiManagerApplication {
public:
    GameNativeGuiManagerApplication(GameHostLog&, GameSingletonHost&, GameNativeReadOnlyData&,
        NativeRenderActualOwners&, NativeStringRawPoolContext&);
    ~GameNativeGuiManagerApplication();
    GameNativeGuiManagerApplication(const GameNativeGuiManagerApplication&) = delete;
    GameNativeGuiManagerApplication& operator=(const GameNativeGuiManagerApplication&) = delete;

    // 0073BC19: the raw getter. The first call constructs, publishes and
    // registers; later calls take its fast arm.
    void* get_004c12b0();
    // The same actual cells for later borrowers; valid until after the drain.
    NativePlatformFocusOwnersContext& context() noexcept;
    bool requires_process_retention() const noexcept;
    // After the shared raw drain: the drain's 00AA0FF0 must have cleared the cell.
    void after_singleton_drain();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
