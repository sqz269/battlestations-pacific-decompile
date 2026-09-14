#include "bsp/native_material_effect_descriptor.hpp"
#include "bsp/native_shader_descriptor_owner.hpp"
#include "bsp/native_shader_sampler_owner.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void require_profile(void* owner, std::uint32_t profile,
    const volatile std::uint32_t* actual_slot, std::uint32_t terminal) {
    // These checks qualify the finite source domain. Both native loads occur
    // at dispatch time; neither a callable table nor an owner registry is kept.
    const auto current = *static_cast<const volatile std::uint32_t*>(owner);
    if (current != profile || actual_slot == nullptr || *actual_slot != terminal)
        throw std::logic_error("actual descriptor terminal requires its recovered current profile and slot");
}
} // namespace
void invoke_native_material_effect_descriptor_terminal(void* captured,
    NativeMaterialEffectDescriptorContext& context) {
    require_profile(captured, 0x00d61a44, context.actual_descriptor_profile_00d61a44, 0x00b46930);
    delete_native_shader_descriptor_00b46930(
        static_cast<NativeShaderDescriptorStorage*>(captured), context, 1);
}
void invoke_native_shader_sampler_terminal(void* captured,
    NativeMaterialEffectDescriptorContext& context) {
    require_profile(captured, 0x00d621f4, context.actual_sampler_profile_00d621f4, 0x00b56fc0);
    delete_native_shader_sampler_00b56fc0(static_cast<NativeShaderSamplerStorage*>(captured),
        context.actual_state_list_pool_0108fee4, context.strings, 1);
}
} // namespace bsp
