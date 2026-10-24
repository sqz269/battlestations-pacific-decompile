#pragma once
#include "bsp/gui_text_glyph_child_runtime.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include <exception>
#include <memory>
#include <string_view>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameNativeGuiEffectApplication;

// Required live application owners and native globals. There are no semantic
// texture/FontResources substitutes or inferred constant/default preimages.
// The font publication selects stable descriptors associated with ACTUAL image
// references in fonts. Renderer+48 must already return registered actual effects;
// the numeric D5F0A8 profile is never installed as a callable host vtable.
// Every referent, including the renderer, must outlive this application and all
// factory Texts, retained materials, pending mappings and failed acquisitions.
struct GameNativeTextApplicationBindings {
    GuiWidgetOwnerRuntime& widgets;
    NativeNodeParentingRuntime& parenting;
    NativeGuiTextPool& pool;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialParameterAccess& parameters;
    const GuiMaterialBindingServices& style;
    NativeFontResourceOwners& fonts;
    GuiTextFontRegistryAccess font_registry;
    const LocaleTables& locale;
    LocaleTextRuntimeHost& locale_runtime;
    GuiTextContentComparison content_comparison;
    // Required for borrowed-cell mode. Source-current mode obtains its
    // required/preflighted comparator from content_comparison and may pass null.
    int (*compare_current_locale_00c0392a)(const char16_t*, const char16_t*);
    GuiNativeNameCompare compare_names_00bf7fbf;
    const bool& crt_sse2_conversion;
    void* const volatile& configuration_0109cf04;
    const volatile std::uint32_t* material_vtable_00d5e520;
    const volatile std::uint32_t* mesh_vtable_00d62d60;
    ActualNativeStringPoolStorage& glyph_strings;
    const char* glyph_shader_name_00cefd78;
    const volatile float& vertical_scale_00e12fd4;
    const float& blend_factor_00f8be54;
    const volatile float& default_width_00d7a260;
    GuiTextConstructorConstants constructor;
    GuiTextPropertyConstants properties;
    GuiTextBoundsConstants bounds;
    GuiTextGlyphPositionConstants glyph_position;
    const volatile std::uint32_t (&lineage_00f8be28)[3];
};

enum class GameNativeTextStatus { complete, pending };

// Concrete native Text application graph. It borrows the renderer's SAME
// declaration cache, canonical geometry/stream owners, real buffer mapping,
// layout factory and half-float import. It owns one canonical Text factory,
// its concrete child deletion transport, and the composed property/content/
// material services. make_type is the Text arm of the real widget factory;
// the existing loader must still create/attach the primary Model and call74,
// base properties, derived properties18, then78 in their recovered order.
// Existing native bodies: AB9650 default, ABB630 properties, ABAED0/AB6AB0
// submission, ABA8D0 content, AB8530 sections, AB8400 buffers, AB8CE0 shader,
// AB9FD0/ABA270 mapped builders and AB98F0's actual child tail. Evidence:
// docs/GUI_TEXT_RUNTIME_FACTORY.md, docs/GUI_TEXT_BUFFERS.md,
// docs/GUI_TEXT_NATIVE_RENDERER.md and docs/GUI_TEXT_CHILD_DRIVER.md.
// This composition adds no newly recovered native body or Ghidra annotation.
//
// Source/UTF16 updates use the factory's SAME runtime implementation. Its
// retained continuations guard ordinary owner retirement and preserve mapped
// streams while the actual optional-child tail is pending. No scratch glyph
// quads, semantic images, draw substitute or second Text lifetime is created.
//
// This component is NOT activated by the current sprite frontend. That caller
// lacks the canonical GUI/material/effect and native-font production above.
// Serialize calls with renderer startup, reset/recreation, frames and drain.
// An entered C++ exception latches the application as nonreplayable; it does
// not recover native SEH or unexposed interrupted callee-local bookkeeping.
// New MSVC Win32 C++ ABI; native pooled string/vtable/SEH ABI is not reproduced.
class GameNativeTextApplication final {
public:
    GameNativeTextApplication(GameHostLog&, GameNativeRendererApplication&,
        GameNativeTextApplicationBindings);
    // Compose the SAME persistent effect application with this application's
    // private buffer services before any Text allocation. Pure domain admission
    // precedes binding; this does not acquire an effect or open/reset a cache.
    // configuration must borrow the renderer's exact publication cell and its
    // receiver must independently support the native +28 height read.
    //
    // Retire all Texts and fonts, perform shared native drain, then close and
    // destroy effect metadata WHILE THIS APPLICATION STILL EXISTS. Its
    // destructor clears our consumer slot; only then may this metadata die.
    // Further Text entries reject the closed binding. Retain both on failure.
    GameNativeTextApplication(GameHostLog&, GameNativeRendererApplication&,
        GameNativeTextApplicationBindings, GameNativeGuiEffectApplication&);
    ~GameNativeTextApplication() noexcept;
    GameNativeTextApplication(const GameNativeTextApplication&) = delete;
    GameNativeTextApplication& operator=(const GameNativeTextApplication&) = delete;

    std::unique_ptr<GuiWidgetTypeImplementation> make_type(GuiWidgetOwner&);
    GameNativeTextStatus submit_source(GuiWidgetOwner&, const std::string&, bool localize);
    GameNativeTextStatus submit_utf16(GuiWidgetOwner&, std::u16string_view);
    // Transfer the SAME standalone layout allocation to this factory's child
    // deletion transport, then execute native Text scalar4(flags1). Call only
    // after queued draws and material parameter borrowers have retired. The
    // transport retains the allocation on an entered failure; no retry or
    // implicit page-manager retirement is supplied.
    void retire_owned_text(std::unique_ptr<GuiLayoutWidget>);
    // Only after this SAME saved AB98F0 child tail is actually complete.
    // Existing runtime validation rejects a failed/unsupported child boundary.
    GameNativeTextStatus resume_after_completed_glyph_child(GuiWidgetOwner&);
    GuiTextRuntimeContentContinuation* pending_content(GuiWidgetOwner&);

    // Required by genuine cursor/glyph cloning and scalar flags0 retirement.
    // The returned factory is stable; do not replace/retarget its providers.
    // Borrow before native operations; never cache it to bypass application
    // failure/reentry checks or discard its retained construction/copy frames.
    GuiTextRuntimeFactory& factory();
    // Also true while the composed effect graph still borrows our services,
    // including an otherwise idle graph with no Text allocations.
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_entry() const noexcept;
    const std::exception_ptr& failure() const noexcept;

    struct Impl;
private:
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
