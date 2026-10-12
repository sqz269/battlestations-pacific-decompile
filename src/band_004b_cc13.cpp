#include "bsp/band_004b_cc13.hpp"

#include <stdexcept>

namespace bsp::cc13_band004b {

namespace {

const ParticipantSlotRecord& require_slot(const ParticipantSlotRecord* record)
{
    // The image dereferences each of the eight pointers without a null test.
    if (record == nullptr) {
        throw std::logic_error("participant slot pointer is null; the image would fault");
    }
    return *record;
}

const InputActionRecord& require_action(const std::vector<InputActionRecord>& actions, std::int32_t action)
{
    // 004C5095: LEA ECX,[ECX+ECX*2] / SHL ECX,4 / ADD ECX,[EAX+4], no bounds check.
    if (action < 0 || static_cast<std::size_t>(action) >= actions.size()) {
        throw std::logic_error("input action index outside the action table; the image reads past it");
    }
    return actions[static_cast<std::size_t>(action)];
}

} // namespace

bool participant_slot_is_occupied_human_004b54f0(const ParticipantSlotRecord& record) noexcept
{
    return record.occupied_08 != 0 && record.flag_09 == 0;
}

bool participant_slot_is_active_ai_004b5510(const ParticipantSlotRecord& record) noexcept
{
    return record.occupied_08 != 0 && record.flag_09 != 0 && record.flag_0a != 0;
}

bool participant_slot_is_active_004b5530(const ParticipantSlotRecord& record) noexcept
{
    return record.occupied_08 != 0 && (record.flag_09 == 0 || record.flag_0a != 0);
}

void game_set_local_player_slot_004b4740(std::int32_t& local_slot_18ec, std::int32_t slot) noexcept
{
    local_slot_18ec = slot;
}

bool game_has_local_player_slot_004b6bc0(std::int32_t local_slot_18ec) noexcept
{
    return local_slot_18ec >= 0 && local_slot_18ec < static_cast<std::int32_t>(kParticipantSlotCount);
}

void game_occupy_participant_slot_004bb550(
    std::array<ParticipantSlotRecord, kParticipantSlotCount>& records_748, std::int32_t slot,
    const ParticipantSlotOccupancy& occupancy, ParticipantSlotHost& host)
{
    // 004BB550: record = game + slot*118h + 748h, no range check.
    if (slot < 0 || static_cast<std::size_t>(slot) >= kParticipantSlotCount) {
        throw std::logic_error("participant slot outside the eight records; the image writes past them");
    }
    // Both copies are unbounded strcpy loops (004BB566, 004BB57C).
    if (occupancy.name.size() >= kParticipantNameCapacity || occupancy.tag.size() >= kParticipantTagCapacity) {
        throw std::logic_error("participant name or tag overruns its field; the image's strcpy would overwrite the next fields");
    }
    ParticipantSlotRecord& record = records_748[static_cast<std::size_t>(slot)];
    record.occupied_08 = 1;
    record.name_58 = occupancy.name;
    record.tag_78 = occupancy.tag;
    record.xuid_80 = occupancy.xuid;
    record.byte_89 = occupancy.byte_89;
    record.byte_88 = occupancy.byte_88;
    record.field_54 = occupancy.field_54;
    record.field_d0 = occupancy.field_d0;
    record.field_d4 = occupancy.field_d4;
    record.field_d8 = occupancy.field_d8;
    record.field_dc = occupancy.field_dc;
    host.xlive_system_virtual_28(); // 004BB5E8
}

const ParticipantSlotRecord* game_find_participant_by_name_004bb690(
    const ParticipantSlotTable& slots_18cc, const std::string& name)
{
    for (const ParticipantSlotRecord* slot : slots_18cc) {
        if (require_slot(slot).name_58 == name) {
            return slot;
        }
    }
    return nullptr;
}

void game_count_active_participants_by_side_004bb770(
    const ParticipantSlotTable& slots_18cc, std::int32_t& side0, std::int32_t& side1)
{
    side1 = 0;
    side0 = 0;
    // Two passes of four in the image (004BB781..004BB846); the order does not
    // change the counts.
    for (const ParticipantSlotRecord* slot : slots_18cc) {
        const ParticipantSlotRecord& record = require_slot(slot);
        if (!participant_slot_is_active_004b5530(record)) continue;
        if (record.side_28 == 0) {
            ++side0;
        } else if (record.side_28 == 1) {
            ++side1;
        }
    }
}

bool game_participant_side_has_room_004bb850(bool interface_manager,
    const ParticipantSlotTable& slots_18cc, const ParticipantSlotRecord& record)
{
    if (!interface_manager) return false; // 004BB851
    const std::int32_t side = record.side_28; // read before the count (004BB865)
    std::int32_t side0 = 0;
    std::int32_t side1 = 0;
    game_count_active_participants_by_side_004bb770(slots_18cc, side0, side1);
    if (side == 0) return side0 < 2; // 004BB87B
    if (side == 1) return side1 < 2; // 004BB88E
    return false;
}

bool game_session_local_side_check_004bb8a0(std::int32_t session_mode, bool interface_manager,
    const ParticipantSlotTable& slots_18cc, std::int32_t local_slot_18ec)
{
    if (session_mode == 0) return false;
    if (session_mode == 1) return true;
    if (!interface_manager || session_mode != 2) return false;
    // [game + 18CCh + game+18ECh*4], no range check on the slot.
    if (!game_has_local_player_slot_004b6bc0(local_slot_18ec)) {
        throw std::logic_error("local slot outside the eight pointers; the image reads past them");
    }
    const ParticipantSlotRecord& local = require_slot(slots_18cc[static_cast<std::size_t>(local_slot_18ec)]);
    return game_participant_side_has_room_004bb850(interface_manager, slots_18cc, local);
}

bool session_is_client_stage_3_or_4_004b61c0(std::uint32_t client_stage_9c) noexcept
{
    return client_stage_9c == 3 || client_stage_9c == 4;
}

std::array<float, 3> game_map_box_center_004b4850(const GameMapBox& box) noexcept
{
    // Each sum goes through FSTP float [ESP+n] before FMUL by the double 0.5.
    constexpr double kHalf = 0.5; // 00D7A280
    const float x = box.f7128 + box.f711c;
    const float y = box.f712c + box.f7120;
    const float z = box.f7130 + box.f7124;
    return {static_cast<float>(x * kHalf), static_cast<float>(y * kHalf), static_cast<float>(z * kHalf)};
}

float game_map_box_span_004b6c40(const GameMapBox& box, std::int32_t axis) noexcept
{
    if (axis == 0 || axis == 2) {
        return box.f7128 - box.f711c;
    }
    return box.f7124 - box.f7130;
}

float input_action_hold_time_004c5070(const std::vector<InputActionRecord>& actions, std::int32_t action)
{
    return require_action(actions, action).hold_time_24;
}

bool input_action_is_held_004c5090(const std::vector<InputActionRecord>& actions, std::int32_t action)
{
    const InputActionRecord& record = require_action(actions, action);
    return record.down_28 != 0 && record.hold_time_24 > 0.0F; // COMISS / JBE: NaN is not held
}

bool input_action_was_pressed_004c50d0(const std::vector<InputActionRecord>& actions, std::int32_t action)
{
    const InputActionRecord& record = require_action(actions, action);
    return record.down_28 != 0 && record.previous_down_20 == 0;
}

bool press_start_poll_button_edge_004b43b0(std::uint8_t& latch_dd, std::uint8_t any_button_down) noexcept
{
    const bool edge = latch_dd == 0 && any_button_down != 0;
    latch_dd = any_button_down; // 004B43C7 and 004B43D1 store CL on both paths
    return edge;
}

} // namespace bsp::cc13_band004b
