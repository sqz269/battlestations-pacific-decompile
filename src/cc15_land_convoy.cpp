// Land convoy bodies (packet cc15-rD1). Evidence and ABI:
// include/bsp/cc15_land_convoy.hpp and docs/CC15_LAND_CONVOY.md.
#include "bsp/cc15_land_convoy.hpp"

namespace bsp::cc15_land_convoy {
namespace {

// CMP ESI,[EDI+8] / JC / CALL 00BF6713: the slot must lie before last, with
// last re-read.
void check_dereferenceable(std::uint8_t* convoy, std::size_t i, LandConvoyMemberSlots& host) {
    if (!(i < host.member_slot_count(convoy))) host.invalid_parameter_noinfo_00bf6713();
}

} // namespace

// 00742180-0074220D. Entry: ESI = first, EDI = convoy+394h; the first <= last
// assert at 0074218B and the per-pass first <= last (007421A3) and same
// container (007421AD, CMP EDI,EDI, never fires) asserts cannot fail on a
// slot count, so they are not called out.
void land_convoy_kill_members_00742180(std::uint8_t* convoy, LandConvoyKillHost& host) {
    for (std::size_t i = 0;; ++i) {
        // 007421A0: last re-read at the top of every pass; exit on equality.
        if (i == host.member_slot_count(convoy)) break;          // 007421B6
        check_dereferenceable(convoy, i, host);                    // 007421BA
        if (host.member_slot(convoy, i) != nullptr) {              // 007421C4
            check_dereferenceable(convoy, i, host);                // 007421C9
            host.clear_member_convoy_738(host.member_slot(convoy, i));  // 007421D5
            // 007421DF CMP, 007421E2 cause read, 007421E5 JC: the cause is read
            // after the clear and between the bound compare and its assert.
            const bool in_bounds = i < host.member_slot_count(convoy);
            const std::int32_t cause = host.convoy_kill_cause_70(convoy);
            if (!in_bounds) host.invalid_parameter_noinfo_00bf6713();  // 007421E7
            host.mission_entity_kill_00926d90(host.member_slot(convoy, i), cause);  // 007421EF
        }
        check_dereferenceable(convoy, i, host);                    // 007421F4
    }
    // 00742209 JMP 00779AF0, ECX = convoy: tail call.
    host.mission_entity_on_killed_00779af0(convoy);
}

// 007422A0-0074232A. The member argument is [ESP+14h] after four pushes,
// re-read every pass (constant). The search stops at the first slot equal to
// it, so a null member matches the first slot an earlier removal nulled.
std::size_t land_convoy_remove_member_007422a0(std::uint8_t* convoy, std::uint8_t* member,
                                              LandConvoyRemoveHost& host) {
    std::size_t removed = kMemberNotFound;
    for (std::size_t i = 0;; ++i) {
        if (i == host.member_slot_count(convoy)) break;           // 007422C0, 007422D6
        check_dereferenceable(convoy, i, host);                    // 007422DA
        if (host.member_slot(convoy, i) == member) {               // 007422E8
            check_dereferenceable(convoy, i, host);                // 007422FB
            host.null_member_slot(convoy, i);                      // 00742305
            host.set_live_count_3bc(convoy, host.live_count_3bc(convoy) - 1);  // 0074230B
            removed = i;
            break;
        }
        check_dereferenceable(convoy, i, host);                    // 007422EC
    }
    // 00742312: reached from both the found and the not-found exit. Exact
    // zero only; a negative count does not kill.
    if (host.live_count_3bc(convoy) == 0) {
        host.mission_entity_kill_00926d90(convoy, kConvoyEmptiedKillCause);  // 00742320
    }
    return removed;
}

// 007420B0-00742178. Everything after the base call, the member broadcast
// included, sits behind the property bag gate (both JZ/JNZ go to 00742177).
void land_convoy_bind_motion_007420b0(std::uint8_t* convoy, LandConvoyBindHost& host) {
    host.unit_owner_activate_and_register_0077f0e0(convoy);       // 007420B3
    void* bag = host.property_bag_c0(convoy);                      // 007420B8
    if (bag == nullptr) return;                                    // 007420C0
    if (host.property_bag_state(bag) != 1) return;                 // 007420C9 SUB 1 / JNZ

    void* property = host.scene_property_bag_find_008f2260(host.property_bag_records(bag),
                                                           kPathKey);  // 007420DC
    const char* name = host.scene_property_string_008f0df0(property);  // 007420E3
    void* entity = host.entity_registry_find_by_name_00925a90(host.entity_registry_30(convoy),
                                                              name);  // 007420EC
    void* path = entity != nullptr ? host.entity_path_interface_for_kind_007ac9d0(entity)  // 007420F7
                                   : nullptr;                                             // 007420FE
    host.set_path_interface_344(convoy, path);                     // 00742104

    const std::int32_t skill = host.property_bag_get_skill_00927a80(convoy);  // 0074210A
    // 00742115: ESI = first; the entry first <= last assert (00742118) and
    // the per-pass ones (00742128, 00742132) cannot fail on a slot count.
    for (std::size_t i = 0;; ++i) {
        if (i == host.member_slot_count(convoy)) break;           // 00742125, 0074213B
        check_dereferenceable(convoy, i, host);                    // 0074213F
        if (host.member_slot(convoy, i) != nullptr) {              // 00742149
            check_dereferenceable(convoy, i, host);                // 0074214E
            host.member_set_skill_vt128(host.member_slot(convoy, i), skill);  // 00742163
        }
        check_dereferenceable(convoy, i, host);                    // 00742165
    }
}

} // namespace bsp::cc15_land_convoy
