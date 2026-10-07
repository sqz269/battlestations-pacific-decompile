#include "bsp/native_land_state_entries.hpp"
#include "bsp/observer_edges.hpp"
#include <stdexcept>

namespace bsp {
namespace {
// Primary recovered fixed numeric contract. Pointer operands preserve the
// actual current parameter+8 payload and ambient x87 control/exception state.
__declspec(noinline) void copy_follow_amplitude(const float volatile* source,
    float volatile* destination) {
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov ecx, destination
        fstp dword ptr [ecx]
    }
}
}

void enter_native_land_follow_009bed80(
    const NativeLandFollowEntryView& state, NativeLandStateEntryServices& services) {
    const void* const source = services.follow_tuning_block_380_0042e740(); // BED84
    void* const destination = state.parameters_6c;             // BED89
    services.copy_follow_parameters_009be150(destination, source); // BED92
    const void* const current_parameters = state.parameters_6c; // BED9A
    const void* const captured_approach = state.approach_04;     // BED9D
    state.field_94 = 0.0f;                                    // BEDA0
    state.field_90 = 0.0f;                                    // BEDA8
    copy_follow_amplitude(services.parameter_amplitude_address_08(current_parameters),
        &state.amplitude_88);                                 // BEDB0/BB
    state.flag_84 = 0;                                        // BEDC3
    state.field_8c = 1.0f;                                    // BEDC9
    const void* const initial_squadron =
        services.approach_squadron_cell_0c(captured_approach);   // BEDD1
    if (initial_squadron != nullptr) {
        auto initial = services.squadron_entry_fields(initial_squadron);
        initial.formation_shape_3e4 = 1u;                     // BEDDA
        services.assign_formation_indices_007ed260(initial.entity); // BEDE4
        const void* const fresh_approach = state.approach_04;   // BEDE9
        const void* const fresh_squadron =
            services.approach_squadron_cell_0c(fresh_approach); // BEDEC
        auto fresh = services.squadron_entry_fields(fresh_squadron);
        const void* const new_leader = fresh.leader_3d0;        // BEDEF
        const void* const old_leader = state.watched_leader_2c; // BEDF5
        if (old_leader != new_leader) {
            if (old_leader != nullptr) {
                services.observer_lifetime().unregister_pair_006952a0(
                    services.plane_observed_endpoint(old_leader), state.callback_18); // BEE05
            }
            state.watched_leader_2c = new_leader;              // BEE0C
            if (new_leader != nullptr) {
                register_observer_pair_00694a60(
                    services.plane_observed_endpoint(new_leader), state.callback_18,
                    services.observer_lifetime());            // BEE15
            }
        }
    }
    state.flag_85 = 0;                                        // BEE1C
}

void enter_native_land_park_009b21a0(const NativeLandParkEntryView& state) noexcept {
    state.field_1c = 0.0f; // 009B21A3, actual binary32 positive zero
    state.flag_18 = 0;     // 009B21B0
    state.timer_28 = 3.0f; // 009B21B4, primary verified00CE3854=40400000
}

void enter_native_land_moveto_007b3db0() noexcept {} // actual soleRET

NativeLandTaskStateEntryConstructorCalls::NativeLandTaskStateEntryConstructorCalls(
    const NativeLandTaskConstructorView& task, NativeLandInitialExecutableProfiles profiles) noexcept
    : task_(task), profiles_(profiles) {}

void NativeLandTaskStateEntryConstructorCalls::enter_state_04(const void* state) {
    const void* expected_profile;
    if (state == task_.moveto_4c4) expected_profile = profiles_.moveto_00d20aec;
    else if (state == task_.follow_500) expected_profile = profiles_.follow_00d20ab8;
    else if (state == task_.park_620) expected_profile = profiles_.park_00d1ff60;
    else throw std::logic_error("unsupported source land initial state identity");
    if (state_profile_cell_00(state) != expected_profile)
        throw std::logic_error("unsupported source land initial state profile");
    if (state == task_.moveto_4c4) enter_native_land_moveto_007b3db0();
    else if (state == task_.follow_500)
        enter_native_land_follow_009bed80(follow_entry_fields(state), *this);
    else enter_native_land_park_009b21a0(park_entry_fields(state));
}

} // namespace bsp
