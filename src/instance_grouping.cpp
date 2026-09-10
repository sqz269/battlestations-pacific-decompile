#include "bsp/instance_grouping.hpp"
#include "bsp/instance_geometry.hpp"
#include <cmath>
#include <limits>

namespace bsp {
float compute_stream_threshold_fade_00b73770(float input, float fraction,
    float threshold) noexcept {
    float width, result;
    //00b73790..b9: width and final result are the only arithmetic float stores.
    __asm {
        fld threshold
        fld st(0)
        fmul fraction
        fstp width
        fld input
        fld width
        fld st(0)
        fsubp st(3), st(0)
        fxch st(1)
        fsubrp st(2), st(0)
        fld1
        fld st(0)
        fdivrp st(2), st(0)
        fxch st(2)
        fmulp st(1), st(0)
        fsubp st(1), st(0)
        fstp result
    }
    if (result < 0.0f) return 0.0f;
    if (result > 1.0f) return 1.0f;
    return result; // Both native comparisons pass unordered values through.
}

bool apply_instance_visibility_00b1dff0_fragment(InstanceRenderEntry& entry,
    const InstanceVisibilityInputs& input, bool& rejected, std::string& error) {
    rejected = false;
    if (!input.enabled) return true;
    std::array<float, 3> delta;
    for (std::size_t i = 0; i < delta.size(); ++i) {
        delta[i] = input.model_world_position[i] - input.camera_world_position[i];
        if (!std::isfinite(delta[i])) {
            error = "Instance visibility requires finite refreshed positions"; return false;
        }
    }
    const float x = delta[0], y = delta[1], z = delta[2];
    float xx, yy, zz, distance;
    //00419440 stores each square, adds y*y+x*x in x87, then z*z, stores
    //the sum before the CRT sqrt. FSQRT is the finite nonnegative core.
    __asm {
        fld x
        fmul st(0), st(0)
        fstp xx
        fld y
        fmul st(0), st(0)
        fstp yy
        fld z
        fmul st(0), st(0)
        fstp zz
        fld yy
        fadd xx
        fadd zz
        fstp distance
        fld distance
        fsqrt
        fstp distance
    }
    float distance_fade = 1.0f;
    if (((1u << (input.camera_mode & 31u)) & 0x17u) != 0) {
        const float limit = input.camera_distance_limit;
        const double divisor = 200.0; //00ce4d70, used as an x87 double operand.
        __asm {
            fld limit
            fsub distance
            fdiv divisor
            fstp distance_fade
        }
        if (distance_fade < 0.0f) distance_fade = 0.0f;
        else if (distance_fade > 1.0f) distance_fade = 1.0f;
    }
    const float threshold_fade = compute_stream_threshold_fade_00b73770(
        entry.leading_value, input.descriptor_fraction, input.stream_threshold);
    const float prior_visibility = entry.visibility;
    float visibility;
    __asm {
        fld threshold_fade
        fmul prior_visibility
        fmul distance_fade
        fstp visibility
    }
    entry.visibility = visibility;
    //00ceb690 double is exactly the promoted float 0.03, not double0.03.
    rejected = static_cast<double>(visibility) < 0.029999999329447746;
    return true;
}

std::uint32_t instance_category_00b1e1c7(float visibility) noexcept {
    unsigned short saved_control, truncate_control;
    std::int64_t converted;
    __asm { fnstcw saved_control }
    truncate_control = static_cast<unsigned short>(saved_control | 0x0c00u);
    __asm {
        fldcw truncate_control
        fld visibility
        fistp converted
        fldcw saved_control
    }
    const auto low = static_cast<std::uint32_t>(converted);
    return low == 0 ? 1u : 0u;
}

bool group_instance_render_entry_00b1dff0_fragment(InstanceGroupingState& state,
    InstanceRenderEntry& entry, const InstanceVisibilityInputs& visibility,
    std::shared_ptr<const InstanceGroupingBinding> binding, InstanceGroupFactory& factory,
    const std::vector<InstanceRenderQueue*>& queues, InstanceGroupingResult& result,
    std::string& error) {
    bool rejected;
    if (!apply_instance_visibility_00b1dff0_fragment(entry, visibility, rejected, error)) return false;
    if (rejected) { result = InstanceGroupingResult::rejected; return true; }
    if (!entry.section) { error = "Instance grouping requires its actual section"; return false; }
    if (!binding) {
        const auto index = entry.visibility < 1.0f ? 1u : entry.section->material_queue_index;
        if (index >= queues.size() || !queues[index]) {
            error = "Instance grouping selected an unavailable render queue"; return false;
        }
        if (!append_instance_render_entry_00b51cb0(*queues[index], entry, error)) return false;
        result = InstanceGroupingResult::queued_directly; return true;
    }
    if (!binding->generator || binding->id >= static_cast<std::uint32_t>(
        (std::numeric_limits<std::int32_t>::max)())) {
        error = "Instance binding requires a generator and a valid signed ID"; return false;
    }
    if (state.by_binding.size() <= binding->id)
        state.by_binding.resize(static_cast<std::size_t>(binding->id) + 1);
    auto& owned = state.by_binding[binding->id];
    const auto category = instance_category_00b1e1c7(entry.visibility);
    if (!owned) {
        owned = std::make_unique<OwnedInstanceGroup>();
        owned->binding = std::move(binding);
        owned->upload.generator = owned->binding->generator.get();
        // Native initializes six colors but selects id%5, so cyan is unreachable.
        constexpr std::array<std::array<float, 4>, 5> colors{{
            {{1,0,0,1}}, {{0,1,0,1}}, {{0,0,1,1}}, {{1,1,0,1}}, {{1,0,1,1}}}};
        for (std::uint32_t i = 0; i < 2; ++i) {
            auto& bucket = owned->upload.categories[i];
            bucket.output_entry = &owned->output_entries[i];
            if (!factory.create(entry, *owned->binding, i,
                colors[owned->binding->id % colors.size()], owned->models[i],
                owned->geometries[i], error)) return false;
            if (!owned->geometries[i] || owned->models[i].geometry != owned->geometries[i].get()) {
                error = "Instance factory did not return its actual retained geometry"; return false;
            }
            if (owned->models[i].context && !owned->models[i].context_owner) {
                error = "Instance factory must retain its model callback context"; return false;
            }
            bucket.generated_model = &owned->models[i];
            bucket.source_entries.reserve(8); //00b1e526 initial native capacity.
        }
        owned->upload.categories[category].instance_count = 1;
        state.ordered_groups.push_back(&owned->upload);
    } else {
        auto& count = owned->upload.categories[category].instance_count;
        if (count == (std::numeric_limits<std::uint32_t>::max)()) {
            error = "Instance group count would overflow"; return false;
        }
        ++count;
    }
    owned->upload.categories[category].source_entries.push_back(&entry);
    result = InstanceGroupingResult::grouped;
    return true;
}
}
