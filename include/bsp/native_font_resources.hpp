#pragma once
#include "bsp/font_data.hpp"
#include "bsp/font_registry.hpp"
#include "bsp/native_render_context.hpp"
#include <memory>
#include <vector>

namespace bsp {
// Owns the SAME decoded FontData allocation and the two transferred actual
// image references. No second metric/glyph map, retained texture wrapper or
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
    ~NativeFontResources() noexcept;
    NativeFontResources(const NativeFontResources&) = delete;
    NativeFontResources& operator=(const NativeFontResources&) = delete;

    const FontDescriptor& descriptor() const noexcept { return descriptor_; }
    FontData& data() noexcept { return *data_; }
    const FontData& data() const noexcept { return *data_; }
    // Sign-extend the CURRENT sole lowword, not a descriptor/default/cached
    // height. Mutation of data().scaled_height is visible on the next read.
    std::int32_t signed_height_14() const noexcept;
    // Existing selector's special-key/fallback rules return the SAME record;
    // resource slots remain live borrowed identities with no added retain.
    const FontGlyphData& glyph(std::uint16_t key) const noexcept;
    NativeRenderActualOwners& actual_owners() const noexcept { return actual_owners_; }

    // Partial AD53A0: typed glyph payload destruction, then captured GFX
    // release/clear, reload alpha release/clear. Native string/tree/sentinel
    // tail5477..5586 and payload allocator ABI are not this C++ owner.
    // After this call no glyph/data consumer may run; repeated cleanup is
    // only the C++ destructor-after-explicit-release bookkeeping case.
    void destroy_00ad53a0_fragment();
private:
    friend class NativeFontResourceOwners;
    NativeFontResources(const FontDescriptor&, NativeRenderActualOwners&);
    const FontDescriptor& descriptor_;
    NativeRenderActualOwners& actual_owners_;
    std::unique_ptr<FontData> data_;
    enum class Phase { preparing, live, destroying, destroyed };
    Phase phase_{Phase::preparing};
};

// C++ lifetime association, not the native font registry/tree ABI. Each
// stable descriptor pointer selects exactly one owned FontData allocation.
// No name-based substitute, metric copy or silent missing-font fallback.
// Publish transfers ownership; this collection and referenced descriptors
// must outlive Text font pointers. No mutation/removal during active calls.
class NativeFontResourceOwners final {
public:
    NativeFontResources& publish(std::unique_ptr<NativeFontResources>&);
    NativeFontResources& resolve(const FontDescriptor* current_font) const;
private:
    std::vector<std::unique_ptr<NativeFontResources>> fonts_;
};
} // namespace bsp
