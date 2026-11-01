#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
namespace bsp {
class FontRegistryStartup;
class FingerprintPayload;
class NativeFontResourceOwners;
class GuiTextRuntimeFactory;
namespace game {
class GameHostLog;
class GameVfsHost;
class GameScriptHost;
class GameNativeRendererApplication;
class GameNativeRawDefaultTextOwner;
class GameNativeGuiPageApplication;
class GameNativeFontApplication;
class GameNativeGuiPageFontRetirementCut;

// Application ownership for the font/payload portion of0073bae0. Uses the
// existing mounted VFS, Lua environment and canonical native renderer/cache.
// Registry rows borrow the sole native font data/image owners. Retire all Text
// and sprite borrowers, then explicitly retire fonts before device release or
// shared native drain. Keep this host/registry metadata through that drain.
// GUI scene/widget ownership remains a separate startup dependency.
class GameFontHost {
public:
    GameFontHost(GameHostLog&, GameVfsHost&, GameScriptHost&,
        GameNativeRendererApplication&);
    ~GameFontHost();
    GameFontHost(const GameFontHost&) = delete;
    GameFontHost& operator=(const GameFontHost&) = delete;
    // The two halves of0073bae0's font work, so bsp::run_gui_startup can drive them in
    // the recovered order instead of this class choosing one. initialize keeps the
    // milestone-2a entry point and is the two calls in that order.
    void initialize(const std::string& language_font_path);
    // Select the image-shaped Font producer before the sole descriptor load.
    // The default typed producer remains unchanged unless explicitly selected.
    void enable_native_font_images();
    //00ac3910 with (root, descriptor, language path), the argument order the callee sees.
    void load_descriptors_00ac3910(const std::string& root, const std::string& descriptor,
        const std::string& language_font_path);
    //0073bc0d..bc17, the forced payload preload whose data pointer is discarded.
    void preload_fingerprint_payload_00be9760();
    // One export after complete startup, then one binding attempt to the SAME
    // empty Text factory using those exact resources. Export immediately opens
    // a persistent interruption window, including failure while constructing
    // that factory; only successful binding closes it. An admitted first bind
    // failure (including foreign resources/nonempty storage) cannot be retried.
    // Before-export, repeated and after-retirement requests reject before any
    // producer/factory effects. No cancel, reset or replacement is provided.
    // Bind before creating any Text borrower; no second factory may borrow
    // these resources. The producer also checks the factory's current font
    // lookup registry is this SAME host registry, without lazy publication.
    // The caller keeps that owner/cell and lookup association valid afterward.
    // Retain the factory, registry, VFS and renderer through Text/font retirement
    // and shared drain. This adds no native Text activation or registry/global
    // publication; the existing registry() is only its borrowed host owner.
    NativeFontResourceOwners& native_fonts();
    void bind_text_factory(GuiTextRuntimeFactory&);
    // Separate combined, exclusive Host/App/raw-owner metadata handshake.
    // Install the fresh stable owner FIRST, call once before its preparation
    // or native pool effects. Never bind an unrelated empty legacy factory.
    // Failed export/admission retains BOTH layers and the exact owner.
    void bind_raw_text_owner(GameNativeRawDefaultTextOwner&);
    // Explicit BOTH-layer unlink after owner.after_native_renderer_drain and
    // BEFORE owner.reset. Keep this host/resources/descriptors/VFS alive until
    // successful completion. No image/name/header payload is revisited here.
    void detach_raw_text_owner_after_parent_drain(GameNativeRawDefaultTextOwner&);
    // Distinct exclusive Page borrower. Bind after actual image production and
    // before any Page/Text entry. Preparation runs while Renderer is ready.
    // No real Page Font-use closure producer is supplied by this dormant lane.
    void bind_page_text_owner(GameNativeGuiPageApplication&);
    void prepare_page_fonts_for_drain(GameNativeGuiPageApplication&);
    void detach_page_text_owner_after_parent_drain(GameNativeGuiPageApplication&);
    GameNativeGuiPageApplication* page_text_owner() const noexcept; // Identity only.
    // One successful native retirement; repeated host closure is metadata-only.
    // Never retries a failed load/retirement or retires surviving borrowers.
    void retire_native_fonts();
    // Includes the unresolved export/bind window, not ordinary ready fonts or
    // a successful exact-factory binding awaiting its normal retirement.
    bool native_operation_interrupted() const noexcept;
    FontRegistryStartup& registry() noexcept;
    const FingerprintPayload& fingerprint() const noexcept;
    // Successful DAT reads (one per completed native font) plus fingerprint
    // payload reads. Image cache/provider opens belong to the native renderer.
    std::size_t resource_opens() const noexcept;
private:
    friend class GameNativeGuiPageApplication;
    friend class GameNativeFontApplication;
    bool owns_page_font_producer(const GameNativeFontApplication&) const noexcept;
    void latch_page_text_owner_failure(GameNativeGuiPageApplication&) noexcept;
    void fail_page_text_owner_link(GameNativeGuiPageApplication&) noexcept;
    void retire_prepared_page_fonts(GameNativeGuiPageApplication&,
        const GameNativeGuiPageFontRetirementCut&);
    bool page_font_retired_awaiting_parent(const GameNativeGuiPageApplication&) const noexcept;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
}
