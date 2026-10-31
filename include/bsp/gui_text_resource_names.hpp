#pragma once
#include <cstdint>
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/native_font_resources.hpp"

namespace bsp {
struct GuiTextResourceNameServices {
    GuiWidgetOwnerRuntime& widgets;
    NativeRenderActualOwners& actual_owners;
};

// Explicit Source registry access; neither profile owns the registry.
// Legacy mode borrows the actual publication cell and real singleton manager,
// preserving the existing getter's allocation/registration/callback behavior.
// Borrowed-existing mode requires a live already-owned Source registry and
// returns that SAME owner, without publication, allocation or registration.
// It does not model Original singleton storage, ownership or ABI.
class GuiTextFontRegistryAccess final {
public:
    static GuiTextFontRegistryAccess published_singleton(
        FontRegistryStartup*& actual_publication, SingletonLifetimeManager& real_lifetime) noexcept;
    static GuiTextFontRegistryAccess borrowed_existing(FontRegistryStartup&) noexcept;
    // ON profile: selects the actual10h registry/list and Font+0C native names.
    // The owner collection is custody/identity resolution, not a name map.
    static GuiTextFontRegistryAccess borrowed_native_images(NativeFontResourceOwners&) noexcept;
    FontRegistryStartup* get() const;
    // Metadata only: return the borrowed owner or READ the current legacy cell.
    // Never invoke the lazy getter, allocate, publish, register or lock. A null
    // legacy cell remains null. Referents/cell must stay live and access must
    // be externally serialized; this does not freeze a mutable publication.
    FontRegistryStartup* existing_registry() const noexcept;
    NativeFontResourceOwners* native_image_owners() const noexcept { return native_images_; }
    void* existing_native_registry_image() const noexcept;

private:
    GuiTextFontRegistryAccess(FontRegistryStartup*, FontRegistryStartup**,
        SingletonLifetimeManager*) noexcept;
    FontRegistryStartup* existing_;
    FontRegistryStartup** publication_;
    SingletonLifetimeManager* lifetime_;
    NativeFontResourceOwners* native_images_{};
};

struct GuiTextFontNameServices {
    GuiTextResourceNameServices& names;
    GuiTextFontRegistryAccess registry;
    NativeFontResourceOwners& fonts;
    const volatile float& one_00d7a24c;

    GuiTextFontNameServices(GuiTextResourceNameServices&, GuiTextFontRegistryAccess,
        NativeFontResourceOwners&, const volatile float&) noexcept;
    // Preserve the previous five-argument brace/constructor call shape.
    GuiTextFontNameServices(GuiTextResourceNameServices&, FontRegistryStartup*&,
        SingletonLifetimeManager&, NativeFontResourceOwners&, const volatile float&) noexcept;
};

//00AB8E70..8ED6, ECX Text, name-header pointer stack, RET4 at8ED4/length3.
// Canonical typed-string domain: assign unless source aliases SAME member,
// then always capture actual shader+1EC. If nonnull, release its actual+04
// reference/current terminal callback and clear the LIVE slot AFTER callback,
// even when callback rebinds it. Equal spelling does not skip invalidation.
// Updates the existing has_cached_shader projection too; no new shader state.
void set_gui_text_shader_name_00ab8e70(GuiTextLifetime&, const std::string&,
    GuiTextResourceNameServices&);

//00AB8C30..8CD4, ECX Text, name-header pointer stack, RET4 at8CD2/length3.
// Native length/current CRT equality early-out; changed path stores font_name,
// invokes the selected registry access and stable owned-descriptor lookup,
// validates the SAME actual font-resource association, publishes borrowed font,
// then stores the selected Font owner's live alpha scale(+1C), or LIVE D7A24C for
// a genuine lookup miss. Only then captures/releases/clears actual shader.
// Does not reload geometry or copy scale_ratio(+18). Returned bool is a C++
// convenience (changed name), not a recovered native return-value contract.
bool set_gui_text_font_name_00ab8c30(GuiTextLifetime&, const std::string&,
    GuiTextFontNameServices&);

// Both use the SAME GuiTextWidget std::string members already owned by the
// canonical lifetime. No new native-header cache or second name/font table.
// Stable null-free DWORD-length names, successful typed-string allocation,
// current host CRT comparison, live owners and registered actual shader profiles
// are required. Native pooled-string layout/allocation callbacks and SEH are
// not reproduced. Registry/descriptors/actual fonts must outlive every Text
// and retained callback/continuation borrowing them. The existing Source owner
// remains responsible for borrowed-mode destruction; never register its member
// registry with a replacement singleton manager. Legacy publication storage
// and the real manager retain their existing getter lifetime requirements.
// Profile access, publication changes and owner retirement are externally
// serialized; the legacy cell is live state, not an immutable registry promise.
// FontRegistryStartup's compatibility vector/resources are
// never substituted for its stable descriptor or the actual typed/image Font.
// Retained Source invocation for the whole raw AB8E70 body. Preparation is
// address-only and may precede raw122 birth; actual entry alone binds the live
// canonical reference. All borrows, this record and its failed graph stay live.
// Entered records remain retained, including completed historical consumers.
// The ordinary DWORD preserve home is deliberately uninitialized until8E85.
// This is a new Source ABI, not Original stack/privacy/alias/FH3/SEH proof.
namespace game { class GameNativeCanonicalDataOwner; }
class NativeGuiTextRawModelReference;
struct NativeGuiTextRawModelReferenceContext;
struct NativeGuiTextReferenceFrame;
struct NativeGuiTextShaderStorageFrame;
class NativeGuiTextRawShaderNameStorageInvocation final {
public:
    enum class Phase { fresh, entered, complete, failed };
    NativeGuiTextRawShaderNameStorageInvocation(
        const NativeGuiTextRawModelReferenceContext&,
        const NativeGuiTextReferenceFrame&, NativeGuiTextShaderStorageFrame&,
        const void* actual_input_header8, const game::GameNativeCanonicalDataOwner&);
    NativeGuiTextRawShaderNameStorageInvocation(const NativeGuiTextRawShaderNameStorageInvocation&) = delete;
    NativeGuiTextRawShaderNameStorageInvocation& operator=(const NativeGuiTextRawShaderNameStorageInvocation&) = delete;
    NativeGuiTextRawShaderNameStorageInvocation(NativeGuiTextRawShaderNameStorageInvocation&&) = delete;
    NativeGuiTextRawShaderNameStorageInvocation& operator=(NativeGuiTextRawShaderNameStorageInvocation&&) = delete;
    Phase phase() const;
    bool requires_process_retention() const;
private:
    friend void set_native_gui_text_shader_name_storage_00ab8e70(
        NativeGuiTextRawModelReference&, const void*, NativeGuiTextRawShaderNameStorageInvocation&);
    const NativeGuiTextRawModelReferenceContext* const context_;
    const NativeGuiTextReferenceFrame* const reference_frame_;
    NativeGuiTextShaderStorageFrame& shader_;
    const void* const input_;
    const NativeGuiTextRawModelReference* reference_{};
    void* actual_text_{};
    Phase phase_{Phase::fresh};
    bool failed_{};
    const game::GameNativeCanonicalDataOwner* const canonical_data_;
    const volatile std::uint32_t* const feature_word_0109eea4_;
    std::uint32_t preserve_home_;
};
// SAME genuine completed raw93/canonical122, reserved credited ShaderFrame,
// actual input header, raw pool and canonical mutable feature-cell backing.
// Header self-alias skips only copy. CURRENT1EC capture requires its genuine
// private outstanding credit; arbitrary resize-installed replacement credit
// has no transfer API and is retained/rejected before decrement. The terminal
// callback's later replacement is still cleared by the LIVE post-call store.
// Failed invocation/provider debt stays retained: no disposal/retry/rollback,
// public credit/Native issuer, new NoThrow or Original/runtime permission.
void set_native_gui_text_shader_name_storage_00ab8e70(
    NativeGuiTextRawModelReference&, const void* actual_name_header8,
    NativeGuiTextRawShaderNameStorageInvocation&);
} // namespace bsp
