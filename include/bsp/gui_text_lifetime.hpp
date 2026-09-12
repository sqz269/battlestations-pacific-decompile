#pragma once
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_style.hpp"

namespace bsp {
// C++ diagnostic continuation marker, not an additional native Text field.
// A non-complete scalar deletion is retained and is NOT automatically resumable:
// a nested AB80C0 suspension also needs its parent's native clear-loop frame.
enum class GuiTextScalarDeletionPhase {
    not_started, derived, base_scene_nodes, base_children, base_detach,
    base_main_node, base_entries, base_containers, complete
};
// Required live native inputs, read during00AB9650. No copied/default global
// table: the caller binds these to its reconstructed constant/global storage.
struct GuiTextConstructorConstants {
    const volatile float& shadow_offset_00d5c5c0;
    const volatile float (&shadow_color_00e12fd8)[4];
    const volatile float& normal_rgb_00ce3e18;
    const volatile float& one_00d7a24c;
    const volatile float& disabled_alpha_00ce3800;
};

// Selects derived admission after AA9520 on this SAME destination owner.
// Admission checks canonical base-copy completion, including the actual
// primary-node virtual10 return. A default-constructed owner is insufficient.
struct GuiTextAfterBaseCopy00aa9520 final {};

// Fields absent from the existing Text projection; unwritten words are marked.
// Address names intentionally avoid assigning an unverified ownership role.
// The +180/+184 and +1AC/+1B0 pointers are borrowed:00AB8250 does not release
// them. The shader+1EC is separately retained below and IS an owned reference.
struct GuiTextLifetimeFields {
    float field_178{};
    std::int32_t field_17c{2};
    void* pointer_180{};
    // Borrowed cursor association. AB8910 parents this canonical Model under
    // the primary node; AB8250 does not independently release this slot.
    NativeNodeBinding* pointer_184{};
    float field_18c{};
    float field_190{};
    std::u16string string_1a4;
    void* pointer_1ac{};
    // New C++ pointer convention: borrowed canonical reference Text OWNER,
    // not a native Text address cast to a companion. Resolve through the same
    // GuiWidgetOwnerRuntime and its existing text_lifetime() association.
    GuiWidgetOwner* pointer_1b0{};
    std::uint8_t byte_1b4{};
    // Native AB9650 and ABB2C0 leave these floats unwritten. 00531380 writes
    // them after byte1B4; do not infer zero offsets from C++ construction.
    float field_1b8;
    float field_1bc;
    // C++ validity metadata only, not a native field or constructor store.
    // Both floats must be written before marking this true or reading them.
    bool glyph_offsets_written{};
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
    // ABB2FD..ABB5F8 only, AFTER the required AA9520 base copy. Copies the
    // source's constructor-written derived values in native order; resets
    // cursor/shadow/glyph/cache fields. Does not call AB8910/AB8530/ABB1D0.
    // Distinct owners in the same domain, completed distinct primary Model
    // copy, stable null-free strings and a live source lifetime are required.
    // GuiTextCopyContinuation supplies the following three native calls.
    GuiTextLifetime(GuiTextAfterBaseCopy00aa9520, GuiWidgetOwner&,
        GuiTextBufferServices&, GuiTextGlyphChildCalls&, const GuiTextLifetime& source);
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
    GuiTextScalarDeletionPhase scalar_deletion_phase() const noexcept { return scalar_phase_; }
    std::uint32_t scalar_deletion_flags() const noexcept { return scalar_flags_; }
    // Host preflight for the SAME owner/implementation: must reject ordinary
    // retirement, scalar deletion and copy reuse BEFORE native phase stores
    // while this is true, including failed cursor calls and pending content.
    bool has_incomplete_copy() const noexcept {
        return after_base_copy_ && !copy_continuation_complete_;
    }

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
    // Tail00AB83AF..83CF invokes base00AA9730/SEH; GuiTextChildDeletion composes
    // the ordinary zero-entry-header Text-tree base path in its new C++ ABI.
    // Native pooled string headers and AB8EE0/AB75A0 Text slot return are not
    // represented by C++ allocations. This is NOT a scalar deleting stub.
    void destroy_derived_00ab8250_fragment();
private:
    friend class GuiTextChildDeletion;
    friend class GuiTextCopyContinuation;
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
    GuiTextScalarDeletionPhase scalar_phase_{GuiTextScalarDeletionPhase::not_started};
    std::uint32_t scalar_flags_{};
    bool after_base_copy_{}; // C++ admission/one-shot guards, not native fields.
    bool copy_continuation_claimed_{};
    bool copy_continuation_complete_{};
};
} // namespace bsp
