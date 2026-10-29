#pragma once
#include "bsp/native_font_resources.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include <exception>
#include <memory>
#include <string>

namespace bsp {
class FontRegistryStartup;
class GuiTextRuntimeFactory;
class VfsLuaScriptFiles;
struct LuaRuntimeGlobals;
}
namespace bsp::game {
class GameHostLog;
class GameVfsHost;
class GameNativeRendererApplication;
class GameNativeRawDefaultTextOwner;

enum class GameNativeFontLoadPhase {
    fresh, gfx, alpha, data, decode, adoption, publication, complete, failed
};
// One initial font invocation. SAME stable descriptor and caller-owned cache
// frames; no destructor rolls back partial native references/registration.
// native_site names the corresponding observed body region/entry, not the
// exact failing instruction inside a nested provider or this C++ composition.
// Names with *_name_live=false are consumed headers, never replayable inputs.
// gfx/alpha hold returned references until adoption clears them. gfx_result/
// alpha_result and published are borrowed observations, never extra references.
struct GameNativeFontLoadOperation final {
    const FontDescriptor* descriptor{};
    GameNativeFontLoadPhase phase{GameNativeFontLoadPhase::fresh};
    std::uint32_t native_site{};
    std::string gfx_path, alpha_path, data_path;
    NativeString gfx_name, alpha_name;
    bool gfx_name_live{}, alpha_name_live{};
    NativeTextureCacheAcquired gfx_cache, alpha_cache;
    void* gfx{};
    void* alpha{};
    void* gfx_result{};
    void* alpha_result{};
    std::unique_ptr<FontData> decoded;
    std::unique_ptr<NativeFontResources> unpublished;
    NativeFontResources* published{};
    std::exception_ptr failure;
};

// Genuine normal initial-load producer over the SAME renderer/VFS/string/
// canonical-resource domain. Native64's observed B319B0 path loads GFX then
// alpha (flags0), then read-existing DAT goes through the real VFS and the
// proven scalar decoder, or selected direct native-image DAT population. Both
// arms transfer the two returned raw image references into the same canonical
// NativeFontResources without AddRef or a second live metric/glyph map.
// Registry descriptors remain stable across appends as name-lookup metadata.
//
// AD4C30 native ABI: ECX font, five string arguments, RET14h. This source
// composition implements successful final population/ownership; its DAT
// whole-file transport and failure timing are not the original per-field
// stream/SEH/private-heap ABI. The optional image arm uses genuine raw Font
// constructor/tree providers and native pooled strings; its complete normal
// retirement includes strings/nodes/sentinel. Typed mode retains the existing
// partial retirement. AD51D0 destructive reload remains unsupported.
// Evidence: FONT_RESOURCE_OWNERSHIP.md, NATIVE_FONT_RESOURCES.md,
// GUI_TEXT_NATIVE_RENDERER.md and GAME_NATIVE_FONT_APPLICATION.md.
//
// Keep renderer, VFS and registry descriptors alive through this component,
// all Texts/materials, failed frames and shared renderer drain. Serialize with
// native callbacks, frames, reset/recreation and singleton drain. Retire Texts,
// then retire_fonts(), then destroy this producer before registry destruction.
// A bound Text factory must remain alive through retire_fonts().
class GameNativeFontApplication final {
public:
    GameNativeFontApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, FontRegistryStartup&);
    ~GameNativeFontApplication() noexcept;
    GameNativeFontApplication(const GameNativeFontApplication&) = delete;
    GameNativeFontApplication& operator=(const GameNativeFontApplication&) = delete;

    // Select only while fresh. Default remains the established typed arm.
    // ON additionally owns the native10h registry/list and native-name lookup;
    // both arms share the actual renderer/VFS owners and descriptor custody.
    void enable_native_font_images();

    void load_descriptors_00ac3910(VfsLuaScriptFiles&, const LuaRuntimeGlobals&,
        const std::string& root, const std::string& descriptor,
        const std::string& language_path);
    // Export only after complete initial production. Before any Text factory
    // creates a borrower, bind that SAME factory below; no unrelated empty
    // factory can authorize retirement of this resource collection. Binding
    // also requires the factory's current font lookup authority to be this SAME
    // typed registry (OFF) or native registry image (ON), without a lazy getter. Keep
    // its owner/cell alive and the lookup association valid through retirement.
    NativeFontResourceOwners& native_fonts();
    void bind_text_factory(GuiTextRuntimeFactory&);
    // Exclusive alternative to the legacy factory export. The fresh owner is
    // already held by its caller. Entered state precedes any export; complete
    // binds this SAME ready actual image registry/collection and renderer/VFS
    // domain without Font lookup, native acquisition, retain or field writes.
    // Every admitted failure is sticky in producer AND owner. No replay/rebind.
    NativeFontResourceOwners& bind_raw_text_owner(GameNativeRawDefaultTextOwner&);
    // Called through the matching FontHost AFTER owner.after_native_renderer_drain
    // and BEFORE owner reset. Only saved metadata identities are used after
    // actual Font retirement; no retired image/name/texture payload is read.
    void detach_raw_text_owner_after_parent_drain(GameNativeRawDefaultTextOwner&);
    // Requires the matching legacy factory's zero retained allocations OR the
    // same raw owner's explicit terminal/full-custody guard. Actual font cleanup
    // runs forward in load order, then clears registry bindings. No retry/reload.
    void retire_fonts();
    const GameNativeFontLoadOperation& operation(std::size_t index) const;
    std::size_t produced_fonts() const noexcept;
    bool ready() const noexcept;
    bool requires_process_retention() const noexcept;
    const std::exception_ptr& failure() const noexcept;

private:
    friend class GameFontHost;
    // Shared Host/App interruption latch; callable only by that typed host.
    void fail_raw_text_owner_link(GameNativeRawDefaultTextOwner&) noexcept;
    bool raw_text_owner_awaits_detach(const GameNativeRawDefaultTextOwner*) const noexcept;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
