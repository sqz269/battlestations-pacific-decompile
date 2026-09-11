#pragma once
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_style.hpp"

namespace bsp {
// Required live native inputs, read during00AB9650. No copied/default global
// table: the caller binds these to its reconstructed constant/global storage.
struct GuiTextConstructorConstants {
    const volatile float& shadow_offset_00d5c5c0;
    const volatile float (&shadow_color_00e12fd8)[4];
    const volatile float& normal_rgb_00ce3e18;
    const volatile float& one_00d7a24c;
    const volatile float& disabled_alpha_00ce3800;
};

// Only constructor-written fields absent from the existing Text projection.
// Address names intentionally avoid assigning an unverified ownership role.
// The +180/+184 and +1AC/+1B0 pointers are borrowed:00AB8250 does not release
// them. The shader+1EC is separately retained below and IS an owned reference.
struct GuiTextLifetimeFields {
    float field_178{};
    std::int32_t field_17c{2};
    void* pointer_180{};
    void* pointer_184{};
    float field_18c{};
    float field_190{};
    std::u16string string_1a4;
    void* pointer_1ac{};
    void* pointer_1b0{};
    std::uint8_t byte_1b4{};
    // ABAA03/ABAA15 copy current base+94 and font+1D4 after new shader
    // selection. Native constructor leaves this live material payload unwritten.
    float overbright_alphatex_1dc[2];
    std::uint8_t byte_1f0{1};
};

// Raw00AB83D0..83F7, ECX ignored, one descriptor DWORD stack, RET4.
// Compares against the three CURRENT words at F8BE28/2C/30, in order.
bool gui_text_matches_type0c_00ab83d0(std::uint32_t descriptor,
    const volatile std::uint32_t (&lineage_00f8be28)[3]) noexcept;

// Canonical derived companion, not another widget/node/material hierarchy.
// Owns ONE GuiTextWidget, live shadow slot, glyph vector and cached actual
// shader reference. All style/content bindings borrow these exact members.
// Required owner, buffer services, child deletion implementation and their
// underlying pools/profiles must outlive this object and retained resources.
// No factory registration: missing Text virtual behavior remains an error.
class GuiTextLifetime final {
public:
    // Partial00AB9650: base00AA9390(3) must already have constructed owner;
    // initializes the derived fields then executes actual00AB8530. Native
    // vtable/SEH and byte-for-byte native string/vector allocation are outside
    // this C++ owner. +1D4 is NOT initialized by native; its existing semantic
    // default must not be treated as a native value before font resolution.
    GuiTextLifetime(GuiWidgetOwner&, GuiTextBufferServices&,
        GuiTextGlyphChildCalls&, const GuiTextConstructorConstants&);
    ~GuiTextLifetime() noexcept;
    GuiTextLifetime(const GuiTextLifetime&) = delete;
    GuiTextLifetime& operator=(const GuiTextLifetime&) = delete;

    GuiTextWidget& text() noexcept { return text_; }
    GuiTextLifetimeFields& fields() noexcept { return fields_; }
    NativeNodeBinding*& shadow_slot_188() noexcept { return shadow_188_; }
    std::vector<GuiLayoutWidget*>& glyph_children_198() noexcept { return glyph_children_198_; }
    // Actual raw shader identity, owning the renderer's returned reference.
    // A producer must use this slot, not the legacy has_cached_shader flag;
    // it must maintain that projection too until old setters are composed.
    void*& cached_shader_slot_1ec() noexcept { return cached_shader_1ec_; }
    GuiTextContentBinding content_binding() noexcept;
    GuiTextStyleBinding style_binding(const GuiMaterialBindingServices&);

    // Complete00AB73B0: capture current shadow; actual unlink/release; clear
    // slot AFTER callback, even if callback rebinds it. Null is a no-op.
    void release_shadow_00ab73b0();
    // AA8320 fragment8362..837F: current type0C test after child20, followed
    // by shadow release on match. This is the owner's secondary-scene hook;
    // primary-node release follows it in the SAME GuiWidgetOwner.
    // After derived teardown the embedding owner is in base destruction:
    // this hook no longer dispatches Text0C (AA9730 installs the base vtable).
    void release_secondary_scene_nodes_00aa8320_fragment(
        const volatile std::uint32_t (&lineage_00f8be28)[3]);

    // Partial00AB8250: actual shader release, shadow unlink, actual child
    // clear, then typed destruction of font/shader/optional/source/text
    // strings and glyph-vector allocation in native order. Must run from
    // derived before_scene_release BEFORE owner/base teardown. Destructor
    // also performs it if still live; reentry while destroying is invalid.
    // Remaining tail00AB83AF..83CF invokes base00AA9730/SEH; caller owns it.
    // Native pooled string headers and AB8EE0/AB75A0 Text slot return are not
    // represented by C++ allocations. This is NOT a scalar deleting stub.
    void destroy_derived_00ab8250_fragment();
private:
    void require_owner() const;
    GuiWidgetOwner& widget_;
    GuiTextBufferServices& buffers_;
    GuiTextGlyphChildCalls& child_calls_;
    GuiTextWidget text_;
    GuiTextLifetimeFields fields_;
    NativeNodeBinding* shadow_188_{};
    std::vector<GuiLayoutWidget*> glyph_children_198_;
    void* cached_shader_1ec_{};
    enum class Phase { constructing, live, destroying, destroyed };
    Phase phase_{Phase::constructing};
};
} // namespace bsp
