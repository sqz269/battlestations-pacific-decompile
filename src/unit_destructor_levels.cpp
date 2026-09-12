// Reconstruction of the unit's own destructor levels above GameEntity.
// Packet cc2_unit_destructor_levels. Evidence: docs/UNIT_DESTRUCTOR_LEVELS.md.
// Ghidra was read-only; every name is a hypothesis, not a recovered symbol.

#include "bsp/unit_destructor_levels.hpp"

namespace bsp {
namespace {

// The eight ref slots level 5 releases at 0081F5CB..0081F6EA, in native order:
// the descending run +BC0h, +BBCh, +BB8h, +BB4h, +BB0h, +BACh, +BA8h, +BA4h.
constexpr std::size_t kRefSlotsBa4ToBc0[8] = {0xBC0, 0xBBC, 0xBB8, 0xBB4,
                                              0xBB0, 0xBAC, 0xBA8, 0xBA4};

// The four level 5 releases at 0081F77F..0081F802: +9F4h, +9F0h, +9ECh, +9E8h.
constexpr std::size_t kRefSlots9e8To9f4[4] = {0x9F4, 0x9F0, 0x9EC, 0x9E8};

}  // namespace

std::vector<UnitDestructorLevel> unit_destructor_chain_006fe570() {
    return {UnitDestructorLevel::most_derived,      UnitDestructorLevel::unit_vehicle_base,
            UnitDestructorLevel::unit_game_object,  UnitDestructorLevel::unit_tickable,
            UnitDestructorLevel::unit_owner_entity, UnitDestructorLevel::game_entity,
            UnitDestructorLevel::entity_root};
}

std::int32_t recursive_critical_section_leave_count_0081f583(std::int32_t enter_count) noexcept {
    // 0081F583 CMP dword ptr [EDI+0x18],EBP with EBP = 0 and 0081F586 JLE: the loop
    // is skipped entirely unless the count is already positive, so a zero or negative
    // count leaves the section zero times.
    return enter_count > 0 ? enter_count : 0;
}

int claim_participant_record_004bb440(std::array<std::uint8_t, 8>& claimed_bytes) noexcept {
    // 004BB446 LEA ESI,[ECX+0x748]; 004BB450 CMP byte ptr [ESI+8],BL (BL = 0);
    // 004BB458 ADD ESI,0x118; 004BB45E CMP EAX,8; 004BB461 JL.
    for (int index = 0; index < 8; ++index) {
        if (claimed_bytes[static_cast<std::size_t>(index)] == 0) {
            claimed_bytes[static_cast<std::size_t>(index)] = 1;  // 004BB47D
            return index;
        }
    }
    return -1;  // 004BB463 POP ESI; 004BB464 XOR EAX,EAX; RET 0x24
}

bool participant_slot_accepted_008cdf58(std::uint8_t byte8, std::uint8_t byte9) noexcept {
    // 008CDF58 CMP byte ptr [EAX+0x8],0x0 rejects on zero;
    // 008CDF5E CMP byte ptr [EAX+0x9],0x0 rejects on non-zero.
    return byte8 != 0 && byte9 == 0;
}

std::vector<UnitReleaseStep> unit_release_steps_0081f3a0(const UnitReleaseState& state,
                                                         bool free_flag) {
    std::vector<UnitReleaseStep> steps;

    // ----- Level 5, 0081F3A0 -------------------------------------------------
    steps.push_back(UnitReleaseStep::l5_rewrite_vptrs);

    // 0081F412 JBE skips the loop when the count is zero; 0081F47A re-reads the
    // count every iteration, so the bound is the field, not a cached copy.
    for (std::uint32_t i = 0; i < state.point_effect_count; ++i) {
        steps.push_back(UnitReleaseStep::l5_stop_and_release_point_effects);
    }

    // 0081F482 loads [unit+1018h] into ECX and calls 0092CFD0 unconditionally; the
    // null test at 0081F493 only guards the destroy and the free.
    steps.push_back(UnitReleaseStep::l5_release_motion_physics_proxy);
    if (state.has_motion_controller) {
        steps.push_back(UnitReleaseStep::l5_destroy_motion_controller);
        steps.push_back(UnitReleaseStep::l5_free_motion_controller);
    }
    if (state.has_field_73c) {
        steps.push_back(UnitReleaseStep::l5_free_field_73c);
    }
    if (state.has_field_740) {
        steps.push_back(UnitReleaseStep::l5_destroy_field_740);
    }
    if (state.has_field_117c) {
        steps.push_back(UnitReleaseStep::l5_free_field_117c);
    }
    if (state.has_ref_range_1114) {
        steps.push_back(UnitReleaseStep::l5_release_ref_range_1114);
        steps.push_back(UnitReleaseStep::l5_free_ref_range_buffer_1114);
    }
    steps.push_back(UnitReleaseStep::l5_destroy_subobject_10d4);
    steps.push_back(UnitReleaseStep::l5_destroy_subobject_10a0);
    steps.push_back(UnitReleaseStep::l5_release_live_effect_refs_ffc);
    steps.push_back(UnitReleaseStep::l5_rewrite_subobject_vptr_bd0);

    if (state.has_critical_section_bd4) {
        const std::int32_t leaves =
            recursive_critical_section_leave_count_0081f583(state.critical_section_enter_count);
        for (std::int32_t i = 0; i < leaves; ++i) {
            steps.push_back(UnitReleaseStep::l5_leave_critical_section_bd4);
        }
        steps.push_back(UnitReleaseStep::l5_delete_critical_section_bd4);
        steps.push_back(UnitReleaseStep::l5_free_critical_section_bd4);
    }

    for (unsigned slot = 0; slot < 8; ++slot) {
        if ((state.live_ref_slots_ba4_to_bc0 >> slot) & 1u) {
            steps.push_back(UnitReleaseStep::l5_release_ref_slots_ba4_to_bc0);
        }
    }
    steps.push_back(UnitReleaseStep::l5_destroy_ref_slot_array_b54);
    steps.push_back(UnitReleaseStep::l5_destroy_ref_slot_array_b44);
    steps.push_back(UnitReleaseStep::l5_destroy_repair_task_a20);
    steps.push_back(UnitReleaseStep::l5_release_live_effect_refs_a14);
    steps.push_back(UnitReleaseStep::l5_destroy_ref_slot_array_a00);
    for (unsigned slot = 0; slot < 4; ++slot) {
        if ((state.live_ref_slots_9e8_to_9f4 >> slot) & 1u) {
            steps.push_back(UnitReleaseStep::l5_release_ref_slots_9e8_to_9f4);
        }
    }

    // 0081F808 gates the second +758h pass on the buffer pointer. The release half
    // of that pass can never fire: step 2 nulled every slot it released
    // (0081F467 and 0081F46E), so only the free at 0081F85F does work.
    if (state.has_point_effect_array) {
        steps.push_back(UnitReleaseStep::l5_free_point_effect_array_758);
    }
    if (state.has_field_74c) {
        steps.push_back(UnitReleaseStep::l5_free_field_74c);
    }
    steps.push_back(UnitReleaseStep::l5_reset_owned_ref_slot_72c);

    // ----- Level 4, 00959940 -----------------------------------------------
    steps.push_back(UnitReleaseStep::l4_rewrite_vptrs);
    if (state.has_field_538) {
        steps.push_back(UnitReleaseStep::l4_release_field_538);
    }
    // 009599D3 returns early on a null +4A4h, so neither the global clear nor the
    // release runs; 009599D5 then compares the field with [00E188DC].
    if (state.has_field_4a4) {
        if (state.field_4a4_is_current_global) {
            steps.push_back(UnitReleaseStep::l4_clear_current_object_global);
        }
        steps.push_back(UnitReleaseStep::l4_release_field_4a4);
    }
    if (state.has_field_6f4) {
        steps.push_back(UnitReleaseStep::l4_destroy_field_6f4);
    }
    if (state.has_field_724) {
        steps.push_back(UnitReleaseStep::l4_destroy_field_724);
    }
    for (std::uint32_t i = 0; i < state.list_424_nodes; ++i) {
        steps.push_back(UnitReleaseStep::l4_drain_list_424);
    }
    for (std::uint32_t nodes : state.gun_category_nodes) {
        for (std::uint32_t i = 0; i < nodes; ++i) {
            steps.push_back(UnitReleaseStep::l4_drain_gun_category_lists_394);
        }
    }
    if (state.has_pooled_string_718) {
        steps.push_back(UnitReleaseStep::l4_return_pooled_string_718);
    }
    if (state.has_pooled_string_6d0) {
        steps.push_back(UnitReleaseStep::l4_return_pooled_string_6d0);
    }
    if (state.has_field_670) {
        steps.push_back(UnitReleaseStep::l4_release_field_670);
    }
    if (state.has_field_664) {
        steps.push_back(UnitReleaseStep::l4_free_field_664);
    }
    steps.push_back(UnitReleaseStep::l4_destroy_record_array_53c);
    steps.push_back(UnitReleaseStep::l4_clear_gun_category_list_424);
    steps.push_back(UnitReleaseStep::l4_destroy_gun_category_array_394);

    // ----- Level 3, 0087A410 -----------------------------------------------
    steps.push_back(UnitReleaseStep::l3_rewrite_vptrs);
    if (state.has_vector_380) {
        steps.push_back(UnitReleaseStep::l3_free_vector_380);
    }
    if (state.has_parts_descriptor_vector) {
        steps.push_back(UnitReleaseStep::l3_free_parts_descriptor_vector_348);
    }
    steps.push_back(UnitReleaseStep::l3_destroy_tick_element_310);

    // ----- Level 2, 0077E380 -----------------------------------------------
    steps.push_back(UnitReleaseStep::l2_rewrite_vptrs);
    if (state.session_message_gate) {
        steps.push_back(UnitReleaseStep::l2_send_release_session_message);
    }
    steps.push_back(UnitReleaseStep::l2_erase_from_global_container);
    steps.push_back(UnitReleaseStep::l2_destroy_subobject_2b0);
    steps.push_back(UnitReleaseStep::l2_clear_list_2a4);
    steps.push_back(UnitReleaseStep::l2_free_list_buffer_2a8);
    steps.push_back(UnitReleaseStep::l2_clear_list_298);
    steps.push_back(UnitReleaseStep::l2_destroy_recon_detection_records_1e8);

    steps.push_back(UnitReleaseStep::delegate_to_game_entity);
    if (free_flag) {
        steps.push_back(UnitReleaseStep::l6_free_instance);
    }
    return steps;
}

void run_unit_release_0081f3a0(UnitReleaseHost& host, const UnitReleaseState& state,
                               bool free_flag, std::uint32_t dead_meat_mark) {
    std::uint32_t point_effect_index = 0;
    unsigned ba4_slot = 0;
    unsigned r9e8_slot = 0;
    std::size_t gun_list = 0;
    std::uint32_t gun_list_drained = 0;

    for (const UnitReleaseStep step : unit_release_steps_0081f3a0(state, free_flag)) {
        switch (step) {
            case UnitReleaseStep::l5_rewrite_vptrs:
                host.rewrite_vptrs(UnitDestructorLevel::unit_vehicle_base);
                break;
            case UnitReleaseStep::l5_stop_and_release_point_effects:
                host.stop_and_release_point_effect(point_effect_index++);
                break;
            case UnitReleaseStep::l5_release_motion_physics_proxy:
                host.release_motion_physics_proxy();
                break;
            case UnitReleaseStep::l5_destroy_motion_controller:
                host.destroy_motion_controller();
                break;
            case UnitReleaseStep::l5_free_motion_controller:
                host.free_motion_controller();
                break;
            case UnitReleaseStep::l5_free_field_73c:
                host.free_field(0x73C);
                break;
            case UnitReleaseStep::l5_destroy_field_740:
                host.destroy_through_vtable(0x740, 0x10);
                break;
            case UnitReleaseStep::l5_free_field_117c:
                host.free_field(0x117C);
                break;
            case UnitReleaseStep::l5_release_ref_range_1114:
                host.release_ref_range_1114();
                break;
            case UnitReleaseStep::l5_free_ref_range_buffer_1114:
                host.free_field(0x1118);
                break;
            case UnitReleaseStep::l5_destroy_subobject_10d4:
                host.destroy_subobject(0x10D4);
                break;
            case UnitReleaseStep::l5_destroy_subobject_10a0:
                host.destroy_subobject(0x10A0);
                break;
            case UnitReleaseStep::l5_release_live_effect_refs_ffc:
                host.release_live_effect_refs(0xFFC);
                break;
            case UnitReleaseStep::l5_rewrite_subobject_vptr_bd0:
                // 0081F56C is a plain store of 00D09480; the host models it as the
                // level whose subobject it belongs to rather than a call.
                host.rewrite_vptrs(UnitDestructorLevel::unit_vehicle_base);
                break;
            case UnitReleaseStep::l5_leave_critical_section_bd4:
                host.leave_critical_section_bd4();
                break;
            case UnitReleaseStep::l5_delete_critical_section_bd4:
                host.delete_critical_section_bd4();
                break;
            case UnitReleaseStep::l5_free_critical_section_bd4:
                host.free_field(0xBD4);
                break;
            case UnitReleaseStep::l5_release_ref_slots_ba4_to_bc0:
                while (ba4_slot < 8 && !((state.live_ref_slots_ba4_to_bc0 >> ba4_slot) & 1u)) {
                    ++ba4_slot;
                }
                if (ba4_slot < 8) {
                    host.release_ref_slot(kRefSlotsBa4ToBc0[ba4_slot++]);
                }
                break;
            case UnitReleaseStep::l5_destroy_ref_slot_array_b54:
                host.destroy_ref_slot_array(0xB54, 5);
                break;
            case UnitReleaseStep::l5_destroy_ref_slot_array_b44:
                host.destroy_ref_slot_array(0xB44, 4);
                break;
            case UnitReleaseStep::l5_destroy_repair_task_a20:
                host.destroy_repair_task_a20();
                break;
            case UnitReleaseStep::l5_release_live_effect_refs_a14:
                host.release_live_effect_refs(0xA14);
                break;
            case UnitReleaseStep::l5_destroy_ref_slot_array_a00:
                host.destroy_ref_slot_array(0xA00, 4);
                break;
            case UnitReleaseStep::l5_release_ref_slots_9e8_to_9f4:
                while (r9e8_slot < 4 && !((state.live_ref_slots_9e8_to_9f4 >> r9e8_slot) & 1u)) {
                    ++r9e8_slot;
                }
                if (r9e8_slot < 4) {
                    host.release_ref_slot(kRefSlots9e8To9f4[r9e8_slot++]);
                }
                break;
            case UnitReleaseStep::l5_free_point_effect_array_758:
                host.free_field(0x758);
                break;
            case UnitReleaseStep::l5_free_field_74c:
                host.free_field(0x74C);
                break;
            case UnitReleaseStep::l5_reset_owned_ref_slot_72c:
                host.reset_owned_ref_slot_72c();
                break;

            case UnitReleaseStep::l4_rewrite_vptrs:
                host.rewrite_vptrs(UnitDestructorLevel::unit_game_object);
                break;
            case UnitReleaseStep::l4_release_field_538:
                host.release_ref_slot(0x538);
                break;
            case UnitReleaseStep::l4_clear_current_object_global:
                host.clear_current_object_global();
                break;
            case UnitReleaseStep::l4_release_field_4a4:
                host.release_ref_slot(0x4A4);
                break;
            case UnitReleaseStep::l4_destroy_field_6f4:
                host.destroy_through_vtable(0x6F4, 0x4);
                break;
            case UnitReleaseStep::l4_destroy_field_724:
                host.destroy_through_vtable(0x724, 0x0);
                break;
            case UnitReleaseStep::l4_drain_list_424:
                host.drain_list_node(0x428);
                break;
            case UnitReleaseStep::l4_drain_gun_category_lists_394:
                // The twelve records are 0Ch apart and the head is at record+4h, so
                // 00959A8F starts EDI at +398h and steps 0Ch (00959ADD).
                while (gun_list < state.gun_category_nodes.size() &&
                       gun_list_drained >= state.gun_category_nodes[gun_list]) {
                    ++gun_list;
                    gun_list_drained = 0;
                }
                if (gun_list < state.gun_category_nodes.size()) {
                    host.drain_list_node(0x398 + gun_list * 0x0C);
                    ++gun_list_drained;
                }
                break;
            case UnitReleaseStep::l4_return_pooled_string_718:
                host.return_pooled_string(0x718);
                break;
            case UnitReleaseStep::l4_return_pooled_string_6d0:
                host.return_pooled_string(0x6D0);
                break;
            case UnitReleaseStep::l4_release_field_670:
                host.release_ref_slot(0x670);
                break;
            case UnitReleaseStep::l4_free_field_664:
                host.free_field(0x664);
                break;
            case UnitReleaseStep::l4_destroy_record_array_53c:
                host.destroy_record_array(0x53C, 0xA, 0x18);
                break;
            case UnitReleaseStep::l4_clear_gun_category_list_424:
                host.clear_gun_category_list(0x424);
                break;
            case UnitReleaseStep::l4_destroy_gun_category_array_394:
                host.destroy_record_array(0x394, 0xC, 0xC);
                break;

            case UnitReleaseStep::l3_rewrite_vptrs:
                host.rewrite_vptrs(UnitDestructorLevel::unit_tickable);
                break;
            case UnitReleaseStep::l3_free_vector_380:
                host.free_field(0x380);
                break;
            case UnitReleaseStep::l3_free_parts_descriptor_vector_348:
                host.free_field(0x348);
                break;
            case UnitReleaseStep::l3_destroy_tick_element_310:
                host.destroy_tick_element_310();
                break;

            case UnitReleaseStep::l2_rewrite_vptrs:
                host.rewrite_vptrs(UnitDestructorLevel::unit_owner_entity);
                break;
            case UnitReleaseStep::l2_send_release_session_message:
                host.send_release_session_message(dead_meat_mark);
                break;
            case UnitReleaseStep::l2_erase_from_global_container:
                host.erase_from_global_container();
                break;
            case UnitReleaseStep::l2_destroy_subobject_2b0:
                host.destroy_subobject(0x2B0);
                break;
            case UnitReleaseStep::l2_clear_list_2a4:
                host.clear_list(0x2A4);
                break;
            case UnitReleaseStep::l2_free_list_buffer_2a8:
                host.free_field(0x2A8);
                break;
            case UnitReleaseStep::l2_clear_list_298:
                host.clear_list(0x298);
                break;
            case UnitReleaseStep::l2_destroy_recon_detection_records_1e8:
                host.destroy_record_array(0x1E8, 3, 0x34);
                break;

            case UnitReleaseStep::delegate_to_game_entity:
                host.delegate_to_game_entity();
                break;
            case UnitReleaseStep::l6_free_instance:
                host.free_instance();
                break;
        }
    }
}

}  // namespace bsp
