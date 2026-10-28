#pragma once
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeGuiManagerApplication;
class GameNativeReadOnlyData;
class GameNativeRendererApplication;
class GameVfsHost;

// cc15-gui4 P3a, GUI Step 4: the native page chain of 00AA5840 on the warm
// 88h manager. Default OFF. Nothing calls load_page_00aa5840 until the joint
// page-chain/P4 flip on FE_initial, which also retires the bridge for that page;
// there is never a second native owner of a page.
//
// P3a1 (this packet) composes the whole AA5840 schedule over existing native
// bodies: AA3140 lookup, ".mmod" (00D0E67C) concatenation and BDF4C0 probe, the
// raw Group root (B8F450 + B8F5E0 with the caller's name), the AC51A0 slot from
// F8BF50 (CD73D0 runs in the constructor), AC6600 as its prefix, AC6714 script,
// AC67CC reader and AC6806 tail stages, then AA52A0 and the path release.
// AC6806's current Page+18 is AC4C50, which needs the warm AC59A0 scene
// acquisition; that composition is packet P3a2. Until it exists, load rejects
// a missed lookup before any native effect. The .mmod model route (004C1400 /
// B80D70 / factory+8) is not composed: a probe hit rejects after releasing
// both name headers. Page widgets (AAACD4 -> AA6560) are P3b/P3c.
// Evidence: Ghidra 00AA5840 (SEH CB71F0) and src/cc13_gui1.cpp's whole-body model.
inline constexpr bool kNativeGuiPageChainCc15Bound = false;

class GameNativeGuiPageApplication final {
public:
    // Starts the F8BF50 layer pool (CD73D0) and binds the Group environment.
    // No page, Group or Lua state is created here.
    GameNativeGuiPageApplication(GameHostLog&, GameNativeGuiManagerApplication&,
        GameNativeRendererApplication&, GameVfsHost&, GameNativeReadOnlyData&);
    ~GameNativeGuiPageApplication() noexcept;
    GameNativeGuiPageApplication(const GameNativeGuiPageApplication&) = delete;
    GameNativeGuiPageApplication& operator=(const GameNativeGuiPageApplication&) = delete;

    // 00AA5840, __thiscall(ECX = manager; const NativeString* name, int
    // screen_flag, char add_reference) RET 0Ch -> page. actual_manager must be
    // the published 00F8BC5C manager. A hit takes CE221C on page+4 when
    // add_reference is nonzero. Serialized with every GUI entry; no reentry.
    void* load_page_00aa5840(void* actual_manager, const void* actual_name_header,
        std::int32_t screen_flag, std::uint8_t add_reference);

    // An entered failure retains its frames, Group, slot and Lua state; the
    // host must _Exit before the singleton drain.
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_site() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
