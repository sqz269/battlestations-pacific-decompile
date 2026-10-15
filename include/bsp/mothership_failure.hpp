#pragma once

// The carrier's runway and hangar failures (docs/OCEAN_WAVE_FIELD.md sections
// 34 and 36). Packet p11_carrier_runway_failure.
//
// The mothership (vtable 00D01630) overrides three failure virtuals of the ship
// base, and each tests the two air-ops names before falling through:
//   +194h 00758750 (Lua SetFailure, 0088F14C): "RunwayFailure" (_stricmp
//     00BF7FBF against 00CF8E24) builds message 72h and routes it with
//     0077C2A0(msg, 7, 0); "HangarFailure" (00425850, case-blind, against
//     00CF8E14) builds 74h. The duration argument is not read. Other names go
//     to the ship base 008132C0.
//   +21Ch 00758890 (the section path, 0093BED0 / 0093C300): the same 72h / 74h
//     arms; other names go to 00827B90.
//   +19Ch 00758A50 (0093C520's expiry): the same names build 73h / 75h; other
//     names go to 00813660.
// The mothership message handler 007583A0 (72h -> unit+11A4h = 1, 74h ->
// unit+11A5h = 1) and 007583F0 (73h / 75h clear them) write the air-ops block
// the constructor 00758550 builds at unit+1188h, so the bytes are block+1Ch /
// block+1Dh, the ones 006BF620 IsReadyToSendPlanes refuses on.
// The airfield (006D2210) sends the same messages, but its handlers only clear
// (CC13_SEG30 006D0D50 / 006D0D90), so these arms are for IsKindOf(09h) only.
// SUBSTITUTION (labelled): the message is applied at the call, not on the
// session pump.

#include <cctype>
#include <string>

#include "bsp/air_operations.hpp"

namespace bsp {

// False: the two names are counted as "other" on a carrier, and no failure
// ever closes a carrier's launch gates, as before.
// ON by mechanism (OCEAN_WAVE_FIELD 36): ESMP03 Zuiho's SetFailure lands on
// its deck (arm 10); ESMP03, USN04 and ESMP08 pairs are gameplay-identical.
inline constexpr bool kCarrierRunwayFailureBound = true;

// IsKindOf(09h), MMothership.
inline constexpr int kMothershipKindOfClassId = 0x09;

enum class MothershipFailureArm { none = 0, runway = 1, hangar = 2 };

inline bool mothership_failure_name_is(const std::string& name, const char* literal) noexcept {
    std::size_t i = 0;
    for (; literal[i] != '\0'; ++i) {
        if (i >= name.size()) return false;
        if (std::tolower(static_cast<unsigned char>(name[i]))
            != std::tolower(static_cast<unsigned char>(literal[i]))) return false;
    }
    return i == name.size();
}

// The name test shared by 00758750, 00758890 and 00758A50: runway first.
inline MothershipFailureArm mothership_failure_arm(const std::string& name) noexcept {
    if (mothership_failure_name_is(name, "RunwayFailure")) return MothershipFailureArm::runway;
    if (mothership_failure_name_is(name, "HangarFailure")) return MothershipFailureArm::hangar;
    return MothershipFailureArm::none;
}

// 007583A0 (set = true, messages 72h / 74h) and 007583F0 (set = false, 73h /
// 75h) on block+1Ch / block+1Dh. A null deck (no block in this process) is a
// no-op; the arm is still taken, as the image returns 1 without testing it.
inline void mothership_apply_failure_message(AirOpsDeck* deck, MothershipFailureArm arm,
                                             bool set) noexcept {
    if (deck == nullptr) return;
    if (arm == MothershipFailureArm::runway) deck->runway_failure = set;
    else if (arm == MothershipFailureArm::hangar) deck->hangar_failure = set;
}

} // namespace bsp
