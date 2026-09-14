#include "bsp/ship_sub_entity_list.hpp"

namespace bsp {
namespace {

// The append the three bodies share: 00432480 and 006D4DD0 call
// 004323D0 BSP_PointerVector_PushBack, 007F44E0 inlines its fast path and falls
// back to 004322D0 when the vector is full. All three copy one 4-byte pointer.
inline void append(const void* entity,
                   const void** out,
                   std::size_t out_capacity,
                   std::size_t& written) noexcept {
    if (written < out_capacity && out != nullptr) {
        out[written] = entity;
    }
    ++written;
}

}  // namespace

bool hangar_contributes_006d4dd0(const AirfieldHangarCandidate& hangar) noexcept {
    // 006D4DF1 MOV EAX,[ESI]; 006D4DF3 TEST EAX,EAX; JZ skip
    // 006D4DF7 MOVSS XMM0,[EAX+370h]; COMISS 00D7A218 (0.0f); JBE skip
    return hangar.record.object != nullptr && hangar.condition > 0.0f;
}

std::size_t sub_entity_list_slot0fc(const SubEntityListInputs& inputs,
                                    const void** out,
                                    std::size_t out_capacity) noexcept {
    std::size_t written = 0;

    switch (inputs.provider) {
    case SubEntityProvider::Self:
        // 00432480: PUSH ECX spills `this` to a local, LEA EAX,[ESP] takes its
        // address and 004323D0 copies the pointer in. One element, unconditional
        // - the native body has no null test and no filter.
        append(inputs.self, out, out_capacity, written);
        break;

    case SubEntityProvider::AirFieldHangars:
        // 006D4DD4-006D4E2F: ESI walks airfield+830h to base + count*0Ch,
        // re-reading both the base and the count every iteration, and appends
        // record[0] for each record that passes the gate.
        for (std::size_t i = 0; i < inputs.hangar_count; ++i) {
            if (inputs.hangars == nullptr) {
                break;
            }
            const AirfieldHangarCandidate& hangar = inputs.hangars[i];
            if (hangar_contributes_006d4dd0(hangar)) {
                append(hangar.record.object, out, out_capacity, written);
            }
        }
        break;

    case SubEntityProvider::SquadronPlanes:
        // 007F44E6-007F4565: EBP counts 0 .. squadron+3CCh, EBX walks
        // squadron+3D0h by 4, and every slot is appended with no null test and
        // no filter. A count of zero or less appends nothing (JLE at 007F44F0).
        for (std::int32_t i = 0; i < inputs.squadron_plane_count; ++i) {
            if (inputs.squadron_planes == nullptr) {
                break;
            }
            append(inputs.squadron_planes[static_cast<std::size_t>(i)],
                   out, out_capacity, written);
        }
        break;
    }

    return written;
}

}  // namespace bsp
