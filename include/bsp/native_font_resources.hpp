#pragma once
#include "bsp/font_data.hpp"
#include "bsp/font_registry.hpp"
#include "bsp/native_render_context.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace bsp {
namespace game { class GameNativeStringProcess; }
// Owns either the SAME decoded FontData allocation or a native AC-byte Font
// with a genuine glyph tree, plus the two transferred actual image references.
// No second live metric/glyph map, retained texture wrapper or
// count. Descriptor identity is borrowed and must remain stable until this
// owner and every Text borrowing it have been retired. Prefer the stable
// FontRegistryOwnedFont::descriptor; compatibility-vector mutation invalidates
// its borrowed pointers. The actual resource domain must outlive all textures,
// including material references that outlive the font.
class NativeFontResources final {
public:
    // Partial AD4C30 resource publication: already decoded complete initial
    // FontData (including0091 fallback), already returned native image refs.
    // On success transfers the unique allocation WITHOUT relocating it and
    // consumes/clears each caller image slot WITHOUT another retain. Both
    // slots may hold the same raw image if caller owns two references; the
    // caller slots themselves must be distinct. Null results are preserved.
    // Validation/allocation failure leaves all three inputs owned by caller.
    // Images must be actual registered owners with a live atomic raw+04;
    // admitted original-token profiles are D61948/D61870/D618B0 (the existing
    // 2D/cube/volume lifetime domain), never callable addresses.
    // semantic D3D9RetainedTexture2D objects cannot be substituted.
    // Native names/renderer64/VFS/DAT calls and reload AD51D0 remain external.
    static std::unique_ptr<NativeFontResources> adopt_00ad4c30_fragment(
        const FontDescriptor&, std::unique_ptr<FontData>& decoded,
        void*& gfx_reference, void*& alpha_reference, NativeRenderActualOwners&);
    // Optional real AC-byte Font arm. Construct with the established raw
    // AD55C0/string process, then populate DAT directly into genuine native
    // 18h tree nodes and separately allocated 20h glyph payloads. No FontData
    // or metric mirror survives in this arm. Keep an incomplete owner retained
    // after a throwing population; it is not a reload/rollback interface.
    // Publishes the Source retained-owner slot BEFORE native allocation/copy
    // effects, so a throwing constructor remains observable in its operation.
    static void construct_image_00ad55c0(const FontDescriptor&, NativeRenderActualOwners&,
        game::GameNativeStringProcess&, std::unique_ptr<NativeFontResources>& retained_owner);
    // Original saves these after GFX and before alpha production.
    void save_image_paths_00ad4c30_fragment(const std::string& root,
        const std::string& language_path);
    void populate_image_00ad4c30_fragment(MemoryStream&,
        void*& gfx_reference, void*& alpha_reference);
    // Call only after retiring the DAT stream, matching AD5079 before AD508F.
    void finish_image_00ad4c30_fragment(void*& gfx_reference, void*& alpha_reference);
    ~NativeFontResources() noexcept;
    NativeFontResources(const NativeFontResources&) = delete;
    NativeFontResources& operator=(const NativeFontResources&) = delete;

    const FontDescriptor& descriptor() const noexcept { return descriptor_; }
    bool live() const noexcept { return phase_ == Phase::live; }
    bool retired() const noexcept { return phase_ == Phase::destroyed; }
    // Compatibility only; an image Font has no FontData and rejects this call.
    FontData& data();
    const FontData& data() const;
    void* native_image() const noexcept;
    std::uint16_t scaled_height_14() const noexcept;
    const float& scale_ratio_18() const noexcept;
    const float& alpha_texture_scale_1c() const noexcept;
    bool uppercase_only_48() const noexcept;
    std::size_t glyph_count() const noexcept;
    bool has_glyph(std::uint16_t key) const noexcept;
    bool accepts_text_byte(std::uint8_t value) const noexcept;
    // Sign-extend the CURRENT sole lowword, not a descriptor/default/cached
    // height. Mutation of data().scaled_height is visible on the next read.
    std::int32_t signed_height_14() const noexcept;
    // Existing selector's special-key/fallback rules return the SAME record;
    // resource slots remain live borrowed identities with no added retain.
    const FontGlyphData& glyph(std::uint16_t key) const noexcept;
    NativeRenderActualOwners& actual_owners() const noexcept { return actual_owners_; }

    // Typed arm: partial AD53A0, payload destruction then GFX/alpha release.
    // Image arm: native payload -> captured GFX -> current alpha -> six
    // pooled strings -> native nodes/sentinel, then Source Font storage free.
    // Private Original heap/FH3/SEH/register ABI remains outside this owner.
    // After this call no glyph/data consumer may run; repeated cleanup is
    // only the C++ destructor-after-explicit-release bookkeeping case.
    void destroy_00ad53a0_fragment();
private:
    friend class NativeFontResourceOwners;
    NativeFontResources(const FontDescriptor&, NativeRenderActualOwners&);
    const FontDescriptor& descriptor_;
    NativeRenderActualOwners& actual_owners_;
    std::unique_ptr<FontData> data_;
    struct Image;
    Image* image_{};
    game::GameNativeStringProcess* image_strings_{};
    void* pending_image_glyph_{}; // Retained only if native insertion throws.
    bool image_paths_saved_{}, image_population_started_{}, image_dat_complete_{};
    enum class Phase { preparing, live, destroying, destroyed };
    Phase phase_{Phase::preparing};
};

// C++ lifetime association, not the native font registry/tree ABI. Each
// stable descriptor pointer selects exactly one typed OR image Font owner.
// No name-based substitute, metric copy or silent missing-font fallback.
// Publish transfers ownership; this collection and referenced descriptors
// must outlive Text font pointers. No mutation/removal during active calls.
class NativeFontResourceOwners final {
public:
    NativeFontResourceOwners() = default;
    ~NativeFontResourceOwners() noexcept;
    NativeFontResourceOwners(const NativeFontResourceOwners&) = delete;
    NativeFontResourceOwners& operator=(const NativeFontResourceOwners&) = delete;
    NativeFontResources& publish(std::unique_ptr<NativeFontResources>&);
    NativeFontResources& resolve(const FontDescriptor* current_font) const;
    NativeFontResources& resolve_image(const void* current_font_image) const;
    // Optional ON authority: actual10h registry header,0Ch circular list and
    // the SAME existing AC Font allocations. Custody metadata owns no name
    // copy and supplies no name lookup. Construct once inside retained startup;
    // append each constructed Font BEFORE AD4C30, then seal only after every
    // load completes. Failure retains the partial graph; no destructor replay.
    void construct_image_registry_00ac3690();
    NativeFontResources& adopt_image_before_load_00ac3910(
        std::unique_ptr<NativeFontResources>&);
    void complete_image_registry();
    void* native_registry_image() const noexcept;
    // AC3570 normal path: traverse native list order and compare current
    // Font+0C/+10 string header against the supplied live8B name header.
    // Source current CRT _stricmp boundary, not Original CRT/private-frame ABI.
    NativeFontResources* find_native_font_00ac3570(const void* name_header8) const;
    // AC36F0 terminal order: each actual Font once, then nodes/sentinel, base
    // identity and enclosing Source storage. No F8BF44 publication is invented.
    void retire_image_registry_00ac36f0();
private:
    std::vector<std::unique_ptr<NativeFontResources>> fonts_;
    struct ImageRegistry;
    ImageRegistry* image_registry_{};
};
} // namespace bsp
