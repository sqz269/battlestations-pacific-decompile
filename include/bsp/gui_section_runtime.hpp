#pragma once
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"

namespace bsp {
struct GuiTimedEntryStorage;
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
    float atlas_u0_118, atlas_v0_11c, atlas_u1_120, atlas_v1_124;
    std::int32_t segment_count_128; // constructor-unwritten; emitter establishes it
};
static_assert(sizeof(GuiSectionFields) == 0x40);
static_assert(offsetof(GuiSectionFields, value_f4) == 8);
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
};

// New C++ companion ABI. ABF770 all three geometry modes and +74 operate on
// the SAME owner's actual model/mesh/streams/material. Actual renderer factory
// slots must be callable native-ABI bindings returning registered resources.
// AC0280 property/texture loading, ABF5B0 copy, and native Section pool deletion
// remain unsupported. No constructor/profile token is a callable native table.
class GuiSectionRuntimeImplementation final : public GuiWidgetTypeImplementation {
public:
    GuiSectionRuntimeImplementation(GuiWidgetOwner&, GuiSectionRuntimeServices);
    GuiSectionFields& fields() noexcept { return fields_; }
    GuiWidgetOwner& owner() noexcept { return owner_; }
    void set_values_00abe6e0(float value, float start_angle, float u0, float u1);
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
    GuiWidgetOwner& owner_;
    GuiSectionRuntimeServices services_;
    GuiSectionFields fields_;
};

// ABE7C9..ABE877 owner/emission companion, called only after the existing
// ABE7B0 current5C gate. Snapshots F8/100 before numeric work, uses the existing
// Section x87 approach, then performs FLD/FSTP argument copies and literal FLD1.
bool update_gui_section_timed_owner_00abe7b0(GuiSectionRuntimeImplementation&,
    GuiTimedEntryStorage&, float delta);
} // namespace bsp
