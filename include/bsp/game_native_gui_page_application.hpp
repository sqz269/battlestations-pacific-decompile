#pragma once
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp { struct ObjectHandleResolverSlots; }
namespace bsp::game {
class GameHostLog;
class GameNativeGuiManagerApplication;
class GameNativeReadOnlyData;
class GameNativeRendererApplication;
class GameNativeModelDomain;
class GameVfsHost;

// Existing AA5840 Page chain, with the actual raw default-Type2 GuiGroup
// reader/Model/lifetime service. Keep OFF until joint full-frontend authority,
// current95/Frame/Game and all required integration/runtime gates close.
// This component uses existing bodies and supplies no new Native-body credit.
inline constexpr bool kNativeGuiPageChainCc15Bound = false;

class GameNativeGuiPageApplication final {
public:
    // Starts F8BF50 (CD73D0), initializes the canonical Layer lineage
    // (CD73F0) and prepares private context views over the renderer domain.
    // No page, Group or Lua state is created. Logs/metadata precede the
    // once-only borrowed manager lifetime token, the last throwing step.
    // Requires the renderer's A1 startup scene domain to be open. Page roots
    // use raw Group lifetime providers, never legacy SceneResource* bindings.
    // Borrow the startup host's CURRENT installed Source resolver cells and
    // its stable GUI conversion capability bool through shared native drain.
    // Neither argument is an Original-VA publication or the live0109EEA4 word.
    GameNativeGuiPageApplication(GameHostLog&, GameNativeGuiManagerApplication&,
        GameNativeRendererApplication&, GameNativeModelDomain&, GameVfsHost&, GameNativeReadOnlyData&,
        const ObjectHandleResolverSlots&, const bool& crt_sse2_conversion);
    ~GameNativeGuiPageApplication() noexcept;
    GameNativeGuiPageApplication(const GameNativeGuiPageApplication&) = delete;
    GameNativeGuiPageApplication& operator=(const GameNativeGuiPageApplication&) = delete;

    // 00AA5840, __thiscall(ECX = manager; const NativeString* name, int
    // screen_flag, char add_reference) RET 0Ch -> page. actual_manager must be
    // the published 00F8BC5C manager. A hit takes CE221C on page+4 when
    // add_reference is nonzero. Serialized with every GUI entry; no reentry.
    // Misses compose actual raw Page/default-Type2 GuiGroups recursively,
    // retaining each <=250-supported-key invocation separately. Other widget
    // factories and the .mmod Model-page route remain outside this component.
    void* load_page_00aa5840(void* actual_manager, const void* actual_name_header,
        std::int32_t screen_flag, std::uint8_t add_reference);

    // An entered failure retains its frames, Group, slot and Lua state; the
    // host must _Exit before the singleton drain.
    bool requires_process_retention() const noexcept;
    void after_native_renderer_drain();
    std::uint32_t failure_site() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
