// Packet cc13-ver9, the strike-approach verification. See
// include/bsp/strike_approach_ver_cc13.hpp and docs/CC13_VER_9.md.
// Reconstructed and build-tested; not wired into the host.
#include "bsp/strike_approach_ver_cc13.hpp"

namespace bsp {

float strike_ctl_cruising_alt_394(const StrikeCtlCruiseInputs& in) noexcept {
    if (in.script_alt_set) return in.script_alt_394;  // 0089F550
    switch (in.task) {
        case StrikeCruiseTask::kDiveBomb:
            return in.dive_cruising_alt_4c0;           // 009C8920 step 4
        case StrikeCruiseTask::kTorpedo:
            return in.torpedo_cruising_alt_430;        // 009D4A70 step 4
        case StrikeCruiseTask::kNone:
            break;
    }
    return in.general_cruising_alt_53c;                // 007F2BE8
}

PlaneCruiseAltitudeResult strike_cruise_altitude_with_ctl_cap_009fba9b(
    PlaneCruiseAltitudeInputs cin, const StrikeCtlCruiseInputs& ctl) {
    // 009FBA93 TEST EAX,EAX / 009FBA99 JE: the leg runs only with a block.
    cin.has_squadron = ctl.has_control_block_0c;
    cin.squadron_limit = strike_ctl_cruising_alt_394(ctl);  // 009FBA9B
    return cruise_altitude_command_009fba50(cin);
}

}  // namespace bsp
