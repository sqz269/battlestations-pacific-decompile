#pragma once
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp {
struct ObjectHandleResolverSlots;
class NativeGuiIconReference;
struct NativeGuiIconStorageContext;
struct NativeGuiIconPropertyStateServices;
class NativeGuiIconConstructedStorageFrame;
struct NativeGuiIconConstructedStorageContext;
struct NativeGuiIconConstructedStorageAcquired;
class NativeGuiIconLoadedStorageFrame;
class NativeGuiIconLoadedStorageContext;
struct NativeGuiWidgetLifetimeContext;
struct NativeGuiWidgetTransformBindings;
class NativeGuiWidgetLoadedDispatch;
class NativeRenderActualOwnerRegistry;
}
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

// Retained typed calls, not new Native owners or callbacks replacing a body.
// A concrete provider owns the distinct nonmoving frame/context/backing behind
// each view. Preparation is cold address metadata only: no receiver/payload
// reads, Native calls, home seeds or resource acquisition. The Page retains the
// returned object before calling the existing whole raw routine. All backing
// and transitive borrowers survive normal return and any entered failure.
struct GameNativeGuiPageIconConstructedInvocation {
    GameNativeGuiPageIconConstructedInvocation(NativeGuiIconConstructedStorageFrame& f,
        NativeGuiIconConstructedStorageContext& c, NativeGuiIconConstructedStorageAcquired& a) noexcept
        : frame(f), context(c), acquired(a) {}
    virtual ~GameNativeGuiPageIconConstructedInvocation() = default;
    GameNativeGuiPageIconConstructedInvocation(const GameNativeGuiPageIconConstructedInvocation&) = delete;
    GameNativeGuiPageIconConstructedInvocation& operator=(const GameNativeGuiPageIconConstructedInvocation&) = delete;
    NativeGuiIconConstructedStorageFrame& frame;
    NativeGuiIconConstructedStorageContext& context;
    NativeGuiIconConstructedStorageAcquired& acquired;
};
struct GameNativeGuiPageIconLoadedInvocation {
    GameNativeGuiPageIconLoadedInvocation(NativeGuiIconLoadedStorageFrame& f,
        NativeGuiIconLoadedStorageContext& c) noexcept : frame(f), context(c) {}
    virtual ~GameNativeGuiPageIconLoadedInvocation() = default;
    GameNativeGuiPageIconLoadedInvocation(const GameNativeGuiPageIconLoadedInvocation&) = delete;
    GameNativeGuiPageIconLoadedInvocation& operator=(const GameNativeGuiPageIconLoadedInvocation&) = delete;
    NativeGuiIconLoadedStorageFrame& frame;
    NativeGuiIconLoadedStorageContext& context;
};

// Explicit missing production borrower. This interface manufactures no pool,
// actual registry, renderer/model/material/atlas/font bank or readiness proof.
// Bind once before the first Page to the SAME lifetime/registry/transform and
// loaded dispatch supplied below. Storage must borrow the real process strings,
// delayed service/singleton deletion arm and mapped default Icon table. State
// services borrow the same GUI/Icon/atlas/cache/RO/live0109EEA4 graph; the stable
// CRT bool is a different role. All provider objects survive shared native drain
// and sticky failed-invocation disposition. A present provider is not authority
// to activate the Page route or to close any existing external gate.
class GameNativeGuiPageIconServices {
public:
    using StatePrivatePreimages = std::uint32_t[4];
    virtual ~GameNativeGuiPageIconServices() = default;
    virtual NativeGuiIconStorageContext& bind_page_storage(NativeGuiWidgetLifetimeContext&,
        NativeRenderActualOwnerRegistry&, NativeGuiWidgetTransformBindings&,
        NativeGuiWidgetLoadedDispatch&) = 0;
    virtual const NativeGuiIconPropertyStateServices& property_state_services() = 0;
    // Required initialized PRIVATE Source ABI input. All DWORD patterns are
    // admitted; no zero default or Original stack identity is supplied here.
    // AB3310 reads CURRENT words only after each actual state prepare returns.
    virtual const volatile StatePrivatePreimages& property_private_preimages() = 0;
    virtual std::unique_ptr<GameNativeGuiPageIconConstructedInvocation>
        prepare_constructed_74(NativeGuiIconReference&) = 0;
    // Must use the supplied SAME loaded/transform dispatch, fresh Raw80/frame,
    // mutable raw key/backing and actual renderer layout factory observation.
    // No typed key copy, fixed four-entry capacity, no-op80 or replay is valid.
    virtual std::unique_ptr<GameNativeGuiPageIconLoadedInvocation>
        prepare_loaded_78(NativeGuiIconReference&) = 0;
    // Genuine AA1CC0 over CURRENT initialized F8BDD8 lineage. Preserve ordered
    // comparisons/AL; no fixed type6/false answer or copied-ID shortcut.
    virtual bool class_query_00aa1cc0(NativeGuiIconReference&,
        std::uint32_t descriptor) = 0;
    // Real default Icon+3C AB6120. Its full reached request/apply/placeholder/
    // current80 graph and fresh invocation backing are mandatory. The old
    // numeric-c_virtual tail body and semantic flag-rejecting adapter are not
    // valid providers for mapped Original DWORD tables in this Source host.
    virtual void visibility_changed_00ab6120(NativeGuiIconReference&,
        std::uint8_t visible) = 0;
};

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

    // Source-only cold binding before the first Page. No Icon/Native count or
    // resource is created. Unbound Type6 rejects BEFORE AA1410. A failed bind
    // is sticky and retains the borrowed graph; no retry/legacy fallback.
    void bind_default_icon_services(GameNativeGuiPageIconServices&);

    // 00AA5840, __thiscall(ECX = manager; const NativeString* name, int
    // screen_flag, char add_reference) RET 0Ch -> page. actual_manager must be
    // the published 00F8BC5C manager. A hit takes CE221C on page+4 when
    // add_reference is nonzero. Serialized with every GUI entry; no reentry.
    // Misses compose actual raw Page/default-Type2 GuiGroups recursively and,
    // only with the explicit service borrower, genuine default-Type6 Icons.
    // Each <=250-supported-key invocation is retained separately. Other widget
    // factories and the .mmod Model-page route remain outside this component.
    void* load_page_00aa5840(void* actual_manager, const void* actual_name_header,
        std::int32_t screen_flag, std::uint8_t add_reference);

    // Whole raw AA7E00 on a SAME live Page/widget companion. The caller keeps
    // its actual name header, data and both ordinary argument homes alive
    // through normal return or entered-failure disposition. Home1 is unread.
    // Returns an unretained raw child borrow; no canonical registration or
    // Native credit is added. Serialized top-level entry, never a callback.
    void* find_child_00aa7e00(void* captured_actual_parent,
        volatile std::uint32_t (&actual_homes)[2]);

    // Capture the SAME live widget's CURRENT profile/+34, require AA8530 and
    // invoke its whole raw body with this Page's existing visibility graph.
    // Per-call backing survives failure. This is a top-level current-slot
    // operation, not an adapter for an earlier captured profile/target pair.
    void set_widget_visibility_00aa8530(void* actual_widget, std::uint8_t requested);

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
