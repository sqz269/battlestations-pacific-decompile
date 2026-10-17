// Reconstruction of the plane squadron entity. See include/bsp/plane_squadron.hpp
// and docs/PLANE_SQUADRON.md for the evidence behind every constant.
//
// Coverage: the constructor 007F2C60 and the mode-1 spawn arm of 007F4580 are
// complete; the mode-2 and mode-3 arms are modelled as "no planes" only, and
// the wave-3 tick 007F3BA0 belongs to include/bsp/tick_element_overrides.hpp.
#include "bsp/plane_squadron.hpp"

#include "bsp/air_operations.hpp"
#include "bsp/cc13_seg41_land.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bsp {

const char kSquadronSingleWingSuffix[] = "|";    // 00CF100C
const char kSquadronMultiWingSuffix[] = "|.-";   // 00CF8038

PlaneSquadronConstructedState plane_squadron_construct_007f2c60() noexcept {
    // Every field is a literal in 007F2C60..007F2E16; the defaults on the
    // struct are the constructor's own values, so the routine is the identity
    // over a fresh instance. Modelling it as a value keeps the seeds one
    // declaration away from the tick, which reads +3C0h, +3C4h and +3E8h.
    return PlaneSquadronConstructedState{};
}

std::int32_t squadron_resolve_wing_count_007f4747(const SquadronSpawnProperties& props) noexcept {
    // 007F473A seeds 3 before the first lookup; the key is read twice, once to
    // test for presence (007F4742) and once for the value (007F4759).
    if (!props.wing_count_present) {
        return kSquadronDefaultWingCount;
    }
    // 007F4761 CMP EAX,1 / 007F4764 MOV [ESP+18h],1 / 007F476C JL: a value
    // below 1 becomes 1. There is no upper clamp.
    if (props.wing_count_raw < 1) {
        return 1;
    }
    return props.wing_count_raw;
}

bool squadron_wing_count_fits_array_007f4b55(std::int32_t wing_count) noexcept {
    // The array at +3D0h is exactly kSquadronMaxPlanes pointers wide and
    // 007F4B55 indexes it without a bound test, so a wing count above five
    // would write +3E4h, +3E8h and +3ECh. The authored enum PlaneWingCount
    // admits only 1..5, which is why the game never reaches that.
    return wing_count >= 1 && wing_count <= kSquadronMaxPlanes;
}

namespace {
// cc13's 00742A70 over a malloc-backed stand-in for the native string
// ({length, data} as two 32-bit words, the Win32 layout cc13's file assumes).
class RomanNativeStrings final : public cc13_seg41::NativeStringHost {
public:
    void assign_0041e870(std::int32_t* string, const char* text) override {
        const std::size_t n = std::strlen(text);
        char* data = static_cast<char*>(std::malloc(n + 1));
        std::memcpy(data, text, n + 1);
        string[0] = static_cast<std::int32_t>(n);
        string[1] = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(data));
    }
    void resize_0041dd40(std::int32_t* string, std::uint32_t size, std::uint32_t) override {
        char* old = reinterpret_cast<char*>(static_cast<std::uintptr_t>(static_cast<std::uint32_t>(string[1])));
        char* data = static_cast<char*>(std::realloc(old, size + 1u));
        data[size] = '\0';
        string[0] = static_cast<std::int32_t>(size);
        string[1] = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(data));
    }
    void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) override {
        std::memcpy(destination, source, bytes);
    }
    void release_00419cc0_00bd1510(void* data, std::size_t) override { std::free(data); }
};
}  // namespace

std::string roman_member_number_00742a70(std::int32_t n) {
    RomanNativeStrings host;
    std::int32_t out[2] = {0, 0};
    cc13_seg41::land_convoy_member_name_00742a70(out, n, host);
    char* data = reinterpret_cast<char*>(static_cast<std::uintptr_t>(static_cast<std::uint32_t>(out[1])));
    std::string text = data != nullptr ? std::string(data, static_cast<std::size_t>(out[0])) : std::string();
    std::free(data);
    return text;
}

int squadron_plane_name_suffix_007f4926(std::int32_t wing_count, std::int32_t index,
                                        char* out, std::size_t capacity) noexcept {
    if (out == nullptr || capacity == 0) {
        return -1;
    }
    if (wing_count == 1) {
        // 007F4926: Assign("|") then one concat with the squadron name.
        if (capacity < sizeof(kSquadronSingleWingSuffix)) {
            return -1;
        }
        out[0] = kSquadronNameSeparator;
        out[1] = '\0';
        return 1;
    }
    // 007F49EF: Assign("|.-"), then 00742A70(index + 1) and a second concat.
    const int written = kRomanMemberNamesBound
        ? std::snprintf(out, capacity, "%s%s", kSquadronMultiWingSuffix,
                        roman_member_number_00742a70(static_cast<int>(index) + 1).c_str())
        : std::snprintf(out, capacity, "%s%d", kSquadronMultiWingSuffix,
                        static_cast<int>(index) + 1);
    if (written < 0 || static_cast<std::size_t>(written) >= capacity) {
        return -1;
    }
    return written;
}

bool squadron_plane_is_leader_007f31a0(std::int32_t slot) noexcept {
    return slot == kSquadronLeaderSlot;
}

SquadronPlanePlacement squadron_plane_placement_007f4813(bool parent_resolved) noexcept {
    SquadronPlanePlacement placement{};
    placement.under_parent = parent_resolved;
    // 007F481D..007F48AE writes a 4x4 identity on the stack (1.0f from
    // DAT_00D7A24C on the diagonal, 0.0f elsewhere) and passes the resolved
    // parent as the hierarchy argument.
    placement.identity_matrix = parent_resolved;
    // 007F48C9 LEA ECX,[ESI+74h]: without a parent the matrix is the
    // squadron's own local transform and the hierarchy argument is 0.
    placement.use_squadron_local = !parent_resolved;
    return placement;
}

PlaneSquadronSpawnResult plane_squadron_spawn_planes_007f4580(
        SquadronSpawnKind kind, std::uint32_t squadron, std::uint32_t squadron_world_node,
        std::uint32_t squadron_spawn_descriptor, PlaneSquadronSpawnHost& host,
        SquadronSpawnStep* steps, std::int32_t max_steps) {
    PlaneSquadronSpawnResult result{};
    result.kind = kind;

    // 007F45A7: the base attach runs first on every arm, before the switch.
    host.attach_lua_self_base_0077e830();

    // 007F45B2 reads the kind from descriptor+4h. Only kind 1 spawns planes;
    // kinds 2 and 3 fill +35Ch, +34Ch/+350h and +364h and return.
    if (kind != SquadronSpawnKind::kPropertyBag) {
        result.behaviour = -1;
        return result;
    }

    const SquadronSpawnProperties props = host.read_spawn_properties_008f2260();
    result.wing_count = squadron_resolve_wing_count_007f4747(props);  // -> +3C8h
    result.plane_class = host.vehicle_class_for_type_007b8a80(props.type_class_id);  // -> +35Ch

    if (props.parent_present) {
        // 007F47A6: the property is used only when its type tag at +4h is 0.
        result.parent_entity = host.resolve_parent_entity_00521e30(props.parent_id);
    }
    if (props.behaviour_present) {
        result.behaviour = props.behaviour;  // -> +364h
    } else {
        result.behaviour = -1;  // the constructor's seed survives
    }

    // 007F47E1: the factory class is fetched a second time from the same id.
    const std::uint32_t factory = host.vehicle_class_for_type_007b8a80(props.type_class_id);

    const bool multi_wing = result.wing_count != 1;
    for (std::int32_t i = 0; i < result.wing_count; ++i) {
        SquadronSpawnStep step{};
        step.index = i;
        step.multi_wing = multi_wing;

        // 007F4811: vehicleClass->vtable[28h](0).
        step.plane = host.create_plane_instance(factory);

        // 007F4817: the parent decides the matrix and the hierarchy argument.
        step.placement = squadron_plane_placement_007f4813(result.parent_entity != 0);
        host.place_plane_in_world(step.plane, result.parent_entity, squadron_world_node,
                                  step.placement);

        // 007F48D6 / 007F48FD / 007F491A: the plane's own spawn descriptor.
        host.clone_spawn_descriptor_00922de0(squadron_spawn_descriptor);

        char suffix[16] = {0};
        squadron_plane_name_suffix_007f4926(result.wing_count, i, suffix, sizeof(suffix));
        host.set_plane_name(step.plane, suffix, i + 1, multi_wing);

        // 007F4B43..007F4B6E, in this order: stamp the spawn index and the back
        // pointer on the plane, append to the array, bump the count, set dirty.
        step.stamped_spawn_index = result.plane_count;
        step.array_slot = result.plane_count;
        step.overflowed_array = step.array_slot >= kSquadronMaxPlanes;
        result.plane_count += 1;
        result.dirty = true;

        if (steps != nullptr && result.step_count < max_steps) {
            steps[result.step_count] = step;
        }
        result.step_count += 1;
    }

    (void)squadron;
    return result;
}

}  // namespace bsp
