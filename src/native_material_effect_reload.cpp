#include "bsp/native_material_effect_reload.hpp"
#include "bsp/native_instance_collection.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void release_derived(NativeMaterialEffectStorage& effect,
    NativeMaterialEffectDestructionAccess& lifetime) {
    // B45EE0 may have installed its retained callable binding on this SAME
    // descriptor. A nonnull numeric-profile context does not override that
    // current table. Keep both existing source dispatch domains explicit.
    const void* const descriptor = effect.descriptor_c4;
    const bool numeric = descriptor &&
        *static_cast<const volatile std::uint32_t*>(descriptor) == 0x00d61a44;
    if (numeric && !lifetime.actual_descriptor)
        throw std::logic_error("raw descriptor reload requires its actual terminal context");
    if (numeric)
        release_native_material_effect_owners_00b41b10(effect,
            lifetime.retained_owners, *lifetime.actual_descriptor);
    else
        release_native_material_effect_owners_00b41b10(effect, lifetime.retained_owners);
}
} // namespace

void reload_native_material_effect_00b469a0(NativeMaterialEffectStorage& effect,
    NativeMaterialEffectReloadContext& context, NativeMaterialEffectReloadFrame& frame) {
    using Phase = NativeMaterialEffectReloadFrame::Phase;
    if (frame.phase != Phase::fresh)
        throw std::logic_error("native effect reload frame is one-shot");
    if (!context.reload_enabled_0108d6f1) {
        frame.phase = Phase::complete;
        return;
    }
    auto& programs = context.programs;
    try {
        // B172D0 returns the actual B8h header. The diagnostic at 4254B0 is
        // exactly RET, so its format/name arguments have no external effect.
        const auto* log_header = native_effect_name_00b172d0(&effect);
        const auto* log_data = *reinterpret_cast<const char* const volatile*>(
            static_cast<const std::byte*>(log_header) + 4);
        (void)log_data;
        frame.phase = Phase::release_derived; frame.native_site = 0x00b469e9;
        release_derived(effect, programs.lifetime);
        frame.phase = Phase::release_base; frame.native_site = 0x00b469f0;
        release_native_material_effect_base_owners_00b187a0(effect.base,
            programs.lifetime.retained_owners);
        // Deliberately reacquire the name header AFTER both cleanup calls.
        const void* const name = native_effect_name_00b172d0(&effect);
        frame.phase = Phase::load_name; frame.native_site = 0x00b469ff;
        if (!load_native_material_effect_variants_00b46950(effect, name,
            programs, frame.loads[0])) {
            frame.phase = Phase::release_failed_derived; frame.native_site = 0x00b46a0a;
            release_derived(effect, programs.lifetime);
            frame.phase = Phase::release_failed_base; frame.native_site = 0x00b46a11;
            release_native_material_effect_base_owners_00b187a0(effect.base,
                programs.lifetime.retained_owners);
            frame.phase = Phase::make_fallback; frame.native_site = 0x00b46a1f;
            const auto* literal = static_cast<const char*>(
                context.original_data.data_at(0x00d61c88, 27));
            frame.fallback_name.assign_0041e870(programs.strings, literal);
            frame.fallback_live = true; frame.exception_state = 0;
            frame.phase = Phase::load_fallback; frame.native_site = 0x00b46a33;
            (void)load_native_material_effect_variants_00b46950(effect,
                &frame.fallback_name, programs, frame.loads[1]);
            // Original captures data, disarms FH3 state, then returns length+1
            // through 419CC0/BD1510. The native header is left stale afterward.
            char* const data = frame.fallback_name.data();
            frame.exception_state = 0xffffffffu; frame.fallback_live = false;
            if (data) programs.strings.release(data, frame.fallback_name.length() + 1u);
        }
        frame.native_site = 0; frame.phase = Phase::complete;
    } catch (...) {
        if (frame.fallback_live) {
            frame.exception_state = 0xffffffffu; frame.fallback_live = false;
            destroy_native_string_header_0041dd20(&frame.fallback_name, programs.strings);
        }
        frame.phase = Phase::failed;
        throw;
    }
}
} // namespace bsp
