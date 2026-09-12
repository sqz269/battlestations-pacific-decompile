#pragma once
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/gui_texture.hpp"

namespace bsp {
struct GuiTimedEntryStorage;
struct GuiStartupHost;
// ABF420 producer: the single Section-derived EC..12B tail. Base values live
// exclusively in GuiWidgetOwner; this is not a second widget/mesh projection.
struct GuiSectionFields {
    NativeString texture_name_ec;
    float value_f4, start_angle_f8, texture_angle_fc, u0_100, u1_104;
    float rim_width_percent_108;
    std::uint8_t clockwise_10c;
    std::byte untouched_10d[3];
    std::int32_t texture_mode_110;
    void* texture_114;
    std::array<float, 4> atlas_118; // SAME native118..124, resolver writes in place
    std::int32_t segment_count_128; // constructor-unwritten; emitter establishes it
};
static_assert(sizeof(GuiSectionFields) == 0x40);
static_assert(offsetof(GuiSectionFields, value_f4) == 8);
static_assert(offsetof(GuiSectionFields, atlas_118) == 0x2c);
static_assert(offsetof(GuiSectionFields, segment_count_128) == 0x3c);

struct GuiSectionConstants {
    const volatile float& one_00d7a24c;
    const volatile float& rim_default_00ce3800;
    const volatile double& two_pi_00ce3828;
    const volatile double& negative_pi_00ce3d18;
    const volatile double& positive_pi_00ce3d28;
    const volatile double& half_00d7a280;
    const volatile double& sectors_00cf0058;
    const volatile double& remainder_epsilon_00ce3c70;
    const volatile double& angular_step_00d5c970;
    const volatile std::uint32_t& sse2_conversion_0109eea4;
};
struct GuiSectionRuntimeServices {
    GuiTextBufferServices& buffers; // existing canonical native owner domains
    NativeLogicalBufferMappingContext& mapping;
    NativeMeshSectionLayoutServices& layouts;
    NativeMaterialParameterAccess& parameters;
    const GuiMaterialBindingServices& colors;
    GuiSectionConstants constants;
    GuiStartupHost& startup; // existing004C12B0 singleton service, required
    // Existing atlas lookup, with actual retained texture identities. Atlas
    // records/owners outlive resolver callbacks; no LogicalTexture surrogate.
    const std::function<const TextureAtlasItem*(std::string_view)>& find_atlas_item_00aefb20;
    // ABF6F0 leaves its two local size DWORDs unwritten. Required caller-state
    // input, not a guessed zero/default: both zero can trigger current3C/40.
    const std::array<std::uint32_t, 2>& texture_size_scratch;
};

// New C++ companion ABI. ABF770 all three geometry modes and +74 operate on
// the SAME owner's actual model/mesh/streams/material. Actual renderer factory
// slots must be callable native-ABI bindings returning registered resources.
// AC0280 consumes the evaluated table AFTER the loader's one AAA710 base pass.
// ABF6F0 publishes one actual resolver reference; replacement deliberately
// does not release its old+114 (native behavior). ABF4F0 derived cleanup releases
// current+114 before its string; the canonical owner supplies base destruction.
// ABF5B0 copy/native pool deletion remain external. New C++ ABI throughout.
// Every callback must keep this companion, its widget/model/mesh and captured
// streams alive until emission returns; no active-deletion continuation exists.
// A later callback/profile error can leave an earlier mapping outstanding.
// There is no implicit unlock/rollback on this exceptional boundary.
class GuiSectionRuntimeImplementation final : public GuiWidgetTypeImplementation {
public:
    GuiSectionRuntimeImplementation(GuiWidgetOwner&, GuiSectionRuntimeServices);
    ~GuiSectionRuntimeImplementation() noexcept override;
    bool has_active_operation() const noexcept override { return active_calls_ != 0 || failed_; }
    GuiSectionFields& fields() noexcept { return fields_; }
    GuiWidgetOwner& owner() noexcept { return owner_; }
    void set_values_00abe6e0(float value, float start_angle, float u0, float u1);
    void set_texture_00abf6f0(const NativeString&);
    void emit7c_00abf770();
    void constructed74(GuiWidgetOwner&) override;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    void set_active60(GuiWidgetOwner&, bool) override;
    bool is_visible38(GuiWidgetOwner&) override;
    void visibility_changed3c(GuiWidgetOwner&, bool) override;
    std::int32_t type5c(GuiWidgetOwner&) override;
    void before_scene_release(GuiWidgetOwner&) override;
private:
    void require_owner(GuiWidgetOwner&) const;
    NativeModelOwner& model() const;
    void set_texture_impl(const NativeString&);
    void emit7c_impl();
    GuiWidgetOwner& owner_;
    GuiSectionRuntimeServices services_;
    GuiSectionFields fields_;
    std::uint32_t active_calls_{};
    bool emitting_{};
    bool failed_{}; // incomplete callback/mapping effects; no automatic replay
    bool retired_{}; // derived string header is stale after native destruction
};

// ABE7C9..ABE877 owner/emission companion, called only after the existing
// ABE7B0 current5C gate. Snapshots F8/100 before numeric work, uses the existing
// Section x87 approach, then performs FLD/FSTP argument copies and literal FLD1.
bool update_gui_section_timed_owner_00abe7b0(GuiSectionRuntimeImplementation&,
    GuiTimedEntryStorage&, float delta);
} // namespace bsp
