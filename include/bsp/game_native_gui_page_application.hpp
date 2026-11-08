#pragma once
#include "bsp/native_gui_media_focus_lifetime.hpp"
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>

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
class NativeFontResourceOwners;
class NativeGuiTextActiveStorageFrame;
class NativeGuiTextActiveStorageAcquired;
class NativeGuiTextActiveStorageContext;
namespace game { class GameNativeGuiPageFontRetirementCut; }
// One concrete Source event contract, not a callback registry or Native body.
class NativeGuiPageFontRetirementHost {
public:
    virtual ~NativeGuiPageFontRetirementHost() = default;
    virtual void after_gui_scalar_return(const game::GameNativeGuiPageFontRetirementCut&) = 0;
    virtual void before_registered_renderer_delete(
        const NativeGuiSelectedManagerDeleteAttempt&, void* actual_receiver) = 0;
};
}
namespace bsp::game {
class GameHostLog;
class GameNativeGuiManagerApplication;
class GameNativeReadOnlyData;
class GameNativeRendererApplication;
class GameNativeModelDomain;
class GameVfsHost;
class GameFontHost;
class GameNativeFontApplication;
class GameNativeGuiPageApplication;
class GameNativeCanonicalDataOwner;
class GameNativeGuiPageTextActiveChild34;

// Birth ONLY in the existing selected AA6540 normal-return site. The GUI may
// already be freed: every receiver below is an opaque identity. No public
// scalar flag, count or caller receipt can construct/copy/retain this scope.
class GameNativeGuiPageFontRetirementCut final {
public:
    GameNativeGuiPageFontRetirementCut(const GameNativeGuiPageFontRetirementCut&) = delete;
    GameNativeGuiPageFontRetirementCut& operator=(const GameNativeGuiPageFontRetirementCut&) = delete;
    GameNativeGuiPageFontRetirementCut(GameNativeGuiPageFontRetirementCut&&) = delete;
    GameNativeGuiPageFontRetirementCut& operator=(GameNativeGuiPageFontRetirementCut&&) = delete;
    void require_same(const NativeGuiSelectedManagerDeleteAttempt& attempt,
        const NativeGuiPageFontRetirementHost& provider, const GameSingletonHost& host,
        const NativeSingletonDeletionBindings& bindings, const NativeGuiMediaFocusLifetimeContext& context,
        void* actual_gui, void* registered_renderer) const {
        check(active_ && &attempt == &attempt_ && &provider == provider_ &&
            &host == host_ && &bindings == bindings_ && &context == context_ &&
            actual_gui == gui_ && registered_renderer == renderer_ &&
            attempt.page_font_scope_ == this, "Page Font cut scope/identity mismatch");
        require_active_attempt();
    }
    // Friendship is deliberately confined to this binding-only nested access.
    // Page has NO friendship with Cut itself and cannot call its ctor/dtor.
    class PageBindingAccess final {
    private:
        friend class GameNativeGuiPageApplication;
        static void bind(NativeGuiSelectedManagerDeleteAttempt& attempt,
            NativeGuiPageFontRetirementHost& provider, NativePlatformFocusOwnersContext& owners,
            NativeStringRawPoolContext& strings, NativeGuiPageLifetimeHost& actual_dispatcher,
            void* actual_gui, void* renderer, GameSingletonHost*& host,
            NativeGuiMediaFocusLifetimeContext*& context,
            const NativeSingletonDeletionBindings*& bindings, void*& gui) {
            GameNativeGuiPageFontRetirementCut::bind_binding(attempt, provider, owners,
                strings, actual_dispatcher, actual_gui, renderer, host, context, bindings, gui);
        }
        static void require_renderer(const NativeGuiSelectedManagerDeleteAttempt& attempt,
            const NativeGuiPageFontRetirementHost& provider, const GameSingletonHost* host,
            const NativeSingletonDeletionBindings* bindings, const NativeGuiMediaFocusLifetimeContext* context,
            void* gui, void* renderer) {
            GameNativeGuiPageFontRetirementCut::require_renderer_fence(attempt, provider,
                host, bindings, context, gui, renderer);
        }
    };
private:
    friend void* bsp::scalar_delete_native_gui_manager_00aa6540_fragment(void*,
        std::uint32_t, NativeGuiMediaFocusLifetimeContext&, NativeGuiSelectedManagerDeleteAttempt&);
    static void check(bool condition, const char* message) {
        if (!condition) throw std::logic_error(message);
    }
    static void bind_binding(NativeGuiSelectedManagerDeleteAttempt& attempt,
        NativeGuiPageFontRetirementHost& provider, NativePlatformFocusOwnersContext& owners,
        NativeStringRawPoolContext& strings, NativeGuiPageLifetimeHost& actual_dispatcher,
        void* actual_gui, void* renderer, GameSingletonHost*& host,
        NativeGuiMediaFocusLifetimeContext*& context,
        const NativeSingletonDeletionBindings*& bindings, void*& gui) {
        check(attempt.page_font_bind_entered_ && !attempt.page_fonts_ &&
            !attempt.page_font_gui_receiver_ && !attempt.page_font_renderer_receiver_ &&
            !attempt.page_font_scope_ && actual_gui && renderer,
            "Page Font binding requires the fresh entered selected metadata");
        attempt.page_fonts_ = &provider;
        attempt.page_font_gui_receiver_ = actual_gui;
        attempt.page_font_renderer_receiver_ = renderer;
        capture_binding(attempt, provider, owners, strings, actual_dispatcher, renderer,
            host, context, bindings, gui);
    }
    static void capture_binding(const NativeGuiSelectedManagerDeleteAttempt& attempt,
        NativeGuiPageFontRetirementHost& provider, NativePlatformFocusOwnersContext& owners,
        NativeStringRawPoolContext& strings, NativeGuiPageLifetimeHost& actual_dispatcher, void* renderer,
        GameSingletonHost*& host, NativeGuiMediaFocusLifetimeContext*& context,
        const NativeSingletonDeletionBindings*& bindings, void*& gui) {
        check(attempt.cold_bound_ && !attempt.host_entered_ && !attempt.core_entered_ &&
            !attempt.scalar_entered_ && !attempt.scalar_returned_ && !attempt.core_returned_ &&
            !attempt.host_returned_ && !attempt.source_fence_returned_ && !attempt.unknown_ &&
            attempt.host_ && attempt.context_ && attempt.bindings_ &&
            attempt.page_font_bind_entered_ && attempt.page_fonts_ == &provider &&
            attempt.page_font_gui_receiver_ && attempt.page_font_renderer_receiver_ == renderer &&
            renderer && !attempt.page_font_scope_ &&
            &attempt.context_->owners == &owners && &attempt.context_->strings == &strings &&
            &attempt.context_->pages == &actual_dispatcher,
            "Page Font binding requires the SAME cold selected attempt/provider domain");
        host = attempt.host_; context = attempt.context_; bindings = attempt.bindings_;
        gui = attempt.page_font_gui_receiver_;
    }
    static void require_bound_attempt(const NativeGuiSelectedManagerDeleteAttempt& attempt,
        const NativeGuiPageFontRetirementHost& provider, const GameSingletonHost* host,
        const NativeSingletonDeletionBindings* bindings, const NativeGuiMediaFocusLifetimeContext* context,
        void* gui, void* renderer) {
        check(attempt.host_ == host && attempt.bindings_ == bindings && attempt.context_ == context &&
            attempt.page_fonts_ == &provider && attempt.page_font_bind_entered_ &&
            attempt.page_font_gui_receiver_ == gui && attempt.page_font_renderer_receiver_ == renderer &&
            attempt.host_entered_ && !attempt.host_returned_ && attempt.core_entered_ &&
            !attempt.core_returned_ && !attempt.source_fence_returned_ && !attempt.unknown_,
            "Page Font selected attempt changed or escaped");
    }
    static void require_renderer_fence(const NativeGuiSelectedManagerDeleteAttempt& attempt,
        const NativeGuiPageFontRetirementHost& provider, const GameSingletonHost* host,
        const NativeSingletonDeletionBindings* bindings, const NativeGuiMediaFocusLifetimeContext* context,
        void* gui, void* renderer) {
        require_bound_attempt(attempt, provider, host, bindings, context, gui, renderer);
        check(attempt.scalar_entered_ && attempt.scalar_returned_ && !attempt.page_font_scope_,
            "Page Font renderer fence requires the completed exact scalar scope");
    }
    void require_active_attempt() const {
        require_bound_attempt(attempt_, *provider_, host_, bindings_, context_, gui_, renderer_);
        check(attempt_.scalar_entered_ && attempt_.scalar_returned_ &&
            attempt_.page_font_scope_ == this, "Page Font cut is not the actual scalar-return scope");
    }
    GameNativeGuiPageFontRetirementCut(NativeGuiSelectedManagerDeleteAttempt& attempt,
        NativeGuiPageFontRetirementHost& provider, NativeGuiMediaFocusLifetimeContext& context,
        void* actual_gui, std::uint32_t full_flags)
        : attempt_(attempt), provider_(&provider), host_(attempt.host_), bindings_(attempt.bindings_),
          context_(&context), gui_(actual_gui), renderer_(attempt.page_font_renderer_receiver_),
          flags_(full_flags) {
        check(!attempt.page_font_scope_ && attempt.context_ == &context &&
            attempt.page_fonts_ == &provider && attempt.page_font_gui_receiver_ == actual_gui &&
            actual_gui && renderer_, "Page Font cut requires its exact prepared scalar capture");
        require_bound_attempt(attempt, provider, host_, bindings_, context_, gui_, renderer_);
        check(attempt.scalar_entered_ && attempt.scalar_returned_,
            "Page Font cut birth requires actual selected scalar normal work");
        attempt.page_font_scope_ = this;
        active_ = true;
    }
    ~GameNativeGuiPageFontRetirementCut() noexcept {
        if (attempt_.page_font_scope_ == this) attempt_.page_font_scope_ = nullptr;
        active_ = false; // Source scope metadata only, no Native cleanup.
    }
    NativeGuiSelectedManagerDeleteAttempt& attempt_;
    NativeGuiPageFontRetirementHost* const provider_;
    GameSingletonHost* const host_;
    const NativeSingletonDeletionBindings* const bindings_;
    NativeGuiMediaFocusLifetimeContext* const context_;
    void* const gui_;
    void* const renderer_;
    const std::uint32_t flags_; // Actual full DWORD capture; never a caller permission.
    bool active_{};
};

// Existing AA5840 Page chain, with the actual raw default-Type2 GuiGroup
// reader/Model/lifetime service. Keep OFF until joint full-frontend authority,
// current95/Frame/Game and all required integration/runtime gates close.
// This component uses existing bodies and supplies no new Native-body credit.
inline constexpr bool kNativeGuiPageChainCc15Bound = true;

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

class GameNativeGuiPageApplication final : private NativeGuiPageFontRetirementHost {
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
    // AA8710 through its shared borrowed-view body on this Page's SAME raw
    // fields/list/node services. Current hosted profiles have null +DC and
    // the genuine A9E0A0 current+24; other callbacks reject explicitly.
    // The live platform+0D byte and Page graph outlive the retained scratch.
    void apply_page_widescreen_layout(void* actual_widget, const bool& widescreen);

    // An entered failure retains its frames, Group, slot and Lua state; the
    // host must _Exit before the singleton drain.
    bool requires_process_retention() const noexcept;
    void after_native_renderer_drain();
    std::uint32_t failure_site() const noexcept;
private:
    friend class GameFontHost;
    friend class GameNativeFontApplication;
    friend class GameNativeGuiManagerApplication;
    friend class GameNativeGuiPageTextActiveChild34;
    // Dormant private typed borrow. No current whole129 Text caller or guessed
    // producer friend is added; actual Type3/18 birth remains unbound.
    std::unique_ptr<GameNativeGuiPageTextActiveChild34> prepare_text_active_child34_borrow(
        const GameNativeCanonicalDataOwner&, NativeGuiTextActiveStorageFrame&,
        NativeGuiTextActiveStorageAcquired&);
    NativeGuiTextActiveStorageContext& text_active_context_for(GameNativeGuiPageTextActiveChild34&);
    void call_captured_text_child34(GameNativeGuiPageTextActiveChild34&, void*,
        std::uint32_t captured_profile, std::uint32_t captured_target, std::uint32_t captured_flags);
    void after_text_active_normal_return(GameNativeGuiPageTextActiveChild34&);
    void text_active_entered_failure(GameNativeGuiPageTextActiveChild34&) noexcept;
    void begin_page_font_link(GameFontHost&, GameNativeFontApplication&,
        GameNativeRendererApplication&, GameVfsHost&);
    void complete_page_font_link(GameFontHost&, GameNativeFontApplication&,
        NativeFontResourceOwners&, const void* actual_image);
    void fail_page_font_link(GameFontHost&, GameNativeFontApplication&) noexcept;
    void prepare_page_font_retirement(GameFontHost&, GameNativeFontApplication&);
    void enter_page_font_delete_attempt(NativeGuiSelectedManagerDeleteAttempt&);
    void bind_page_font_delete_attempt(GameNativeGuiManagerApplication&,
        NativeGuiPageLifetimeHost& actual_dispatcher, NativeGuiPageLifetimeHost& actual_delegate,
        void* captured_actual_gui,
        NativeGuiSelectedManagerDeleteAttempt&);
    void require_page_font_retirement_cut(GameFontHost&, GameNativeFontApplication&,
        const GameNativeGuiPageFontRetirementCut&) const;
    void require_actual_page_font_last_use_closed(const GameNativeGuiPageFontRetirementCut&) const;
    void detach_page_font_link_after_parent_drain(GameFontHost&, GameNativeFontApplication&,
        NativeFontResourceOwners&, const void* saved_image);
    void after_gui_scalar_return(const GameNativeGuiPageFontRetirementCut&) override;
    void before_registered_renderer_delete(const NativeGuiSelectedManagerDeleteAttempt&, void*) override;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
