#pragma once

// Non-gameplay GGame helpers from 004B0000-004CFFFF reconstructed by
// cc13-sweep4B: the eight multiplayer participant-slot records and their side
// counts, the session-side gate, the map-box centre and span, the input-action
// record accessors and the press-start button edge. Names are hypotheses, not
// recovered symbols. Evidence, ABIs and open questions are in
// docs/CC13_BAND_004B_004F.md. None is a drop-in binary replacement: these are
// C++ interfaces over models of the native data.
//
// Where the image dereferences a pointer or indexes an array without a check,
// the model throws std::logic_error instead of reproducing the fault.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp::cc13_band004b {

// One 118h-byte participant record, game+748h + slot*118h. The game also keeps
// eight pointers to them at game+18CCh and the local slot index at game+18ECh
// (include/bsp/entity_orders.hpp, include/bsp/frontend_menus.hpp).
// Only the fields the routines below touch are modelled.
struct ParticipantSlotRecord {
    std::uint8_t occupied_08 = 0;    // set to 1 by 004BB550
    std::uint8_t flag_09 = 0;        // read as "AI-controlled" (provisional)
    std::uint8_t flag_0a = 0;        // read as "AI enabled" (provisional)
    std::int32_t side_28 = 0;        // 0 or 1 in the side counts
    std::uint32_t field_54 = 0;
    std::string name_58;             // char[20h] at +58h (strcpy target)
    std::string tag_78;              // char[8] at +78h (strcpy target)
    std::uint64_t xuid_80 = 0;       // +80h/+84h
    std::uint8_t byte_88 = 0;
    std::uint8_t byte_89 = 0;
    std::uint32_t field_d0 = 0;
    std::uint32_t field_d4 = 0;
    std::uint32_t field_d8 = 0;
    std::uint32_t field_dc = 0;
};

inline constexpr std::size_t kParticipantSlotCount = 8;
inline constexpr std::size_t kParticipantNameCapacity = 0x20; // +58h..+77h
inline constexpr std::size_t kParticipantTagCapacity = 0x08;  // +78h..+7Fh

// game+18CCh: eight record pointers (null modelled as nullptr).
using ParticipantSlotTable = std::array<const ParticipantSlotRecord*, kParticipantSlotCount>;

// 004B54F0, __fastcall(ECX record), AL: +08h != 0 && +09h == 0.
bool participant_slot_is_occupied_human_004b54f0(const ParticipantSlotRecord& record) noexcept;
// 004B5510, __fastcall(ECX record), AL: +08h, +09h and +0Ah all non-zero.
bool participant_slot_is_active_ai_004b5510(const ParticipantSlotRecord& record) noexcept;
// 004B5530, __fastcall(ECX record), AL: +08h != 0 && (+09h == 0 || +0Ah != 0).
bool participant_slot_is_active_004b5530(const ParticipantSlotRecord& record) noexcept;

// 004B4740, __thiscall(ECX game, slot), RET 4: game+18ECh = slot.
void game_set_local_player_slot_004b4740(std::int32_t& local_slot_18ec, std::int32_t slot) noexcept;
// 004B6BC0, __fastcall(ECX game), AL: 0 <= game+18ECh < 8.
bool game_has_local_player_slot_004b6bc0(std::int32_t local_slot_18ec) noexcept;

// 004BB550, __thiscall(ECX game, slot, name, tag, xuid_lo, xuid_hi, b89, b88,
// d54, dD0, dD4, dD8, dDC), RET 30h. The image computes the record address
// without a range check and copies both strings with an unbounded strcpy.
struct ParticipantSlotHost {
    virtual ~ParticipantSlotHost() = default;
    // 004BB5E8: [00F8A2FC] (BSP_XLiveSystem) virtual +28h, no arguments.
    virtual void xlive_system_virtual_28() = 0;
};
struct ParticipantSlotOccupancy {
    std::string name;
    std::string tag;
    std::uint64_t xuid = 0;
    std::uint8_t byte_89 = 0;
    std::uint8_t byte_88 = 0;
    std::uint32_t field_54 = 0;
    std::uint32_t field_d0 = 0;
    std::uint32_t field_d4 = 0;
    std::uint32_t field_d8 = 0;
    std::uint32_t field_dc = 0;
};
void game_occupy_participant_slot_004bb550(
    std::array<ParticipantSlotRecord, kParticipantSlotCount>& records_748, std::int32_t slot,
    const ParticipantSlotOccupancy& occupancy, ParticipantSlotHost& host);

// 004BB690, __thiscall(ECX game, const char* name), RET 4: the first of the
// eight pointers at game+18CCh whose +58h string equals name (inlined strcmp),
// or nullptr. Every pointer is dereferenced without a null check.
const ParticipantSlotRecord* game_find_participant_by_name_004bb690(
    const ParticipantSlotTable& slots_18cc, const std::string& name);

// 004BB770, __thiscall(ECX game, int* side0, int* side1), RET 8: counts the
// records that pass 004B5530 by side +28h (0 or 1; other sides are ignored).
void game_count_active_participants_by_side_004bb770(
    const ParticipantSlotTable& slots_18cc, std::int32_t& side0, std::int32_t& side1);

// 004BB850, __thiscall(ECX game, record), RET 4, AL. interface_manager is
// [00E198C4] != 0.
bool game_participant_side_has_room_004bb850(bool interface_manager,
    const ParticipantSlotTable& slots_18cc, const ParticipantSlotRecord& record);

// 004BB8A0, __fastcall(ECX game), AL. session_mode is game+1FE4h
// (0 local, 1 host, 2 client; include/bsp/session_message_dispatch.hpp).
bool game_session_local_side_check_004bb8a0(std::int32_t session_mode, bool interface_manager,
    const ParticipantSlotTable& slots_18cc, std::int32_t local_slot_18ec);

// 004B61C0, __fastcall(ECX session), AL: [[session+18Ch]+9Ch] is 3 or 4.
bool session_is_client_stage_3_or_4_004b61c0(std::uint32_t client_stage_9c) noexcept;

// The map box on the game object. Field names keep the offsets; the pairing
// below is the one 004B4850 uses for the centre.
struct GameMapBox {
    float f711c = 0.0F;
    float f7120 = 0.0F;
    float f7124 = 0.0F;
    float f7128 = 0.0F;
    float f712c = 0.0F;
    float f7130 = 0.0F;
};

// 004B4850, __thiscall(ECX game, float* out), RET 4:
// out = {(+7128h + +711Ch), (+712Ch + +7120h), (+7130h + +7124h)} * 0.5, each sum
// stored to a float before the multiply by the double at 00D7A280.
std::array<float, 3> game_map_box_center_004b4850(const GameMapBox& box) noexcept;

// 004B6C40, __thiscall(ECX game, axis), RET 4, ST0: axis 0 or 2 -> +7128h - +711Ch,
// any other axis -> +7124h - +7130h, rounded through a float slot.
float game_map_box_span_004b6c40(const GameMapBox& box, std::int32_t axis) noexcept;

// One 30h-byte input action record, [InputManager+4] + action*30h
// (docs/GAME_INPUT_TICK.md).
struct InputActionRecord {
    std::uint8_t previous_down_20 = 0;
    float hold_time_24 = 0.0F;
    std::uint8_t down_28 = 0;
};

// 004C5070, __stdcall(action), RET 4, ST0: record +24h. No range check.
float input_action_hold_time_004c5070(const std::vector<InputActionRecord>& actions, std::int32_t action);
// 004C5090, __stdcall(action), RET 4, AL: +28h set and +24h > 0.0f (COMISS against 00D7A218).
bool input_action_is_held_004c5090(const std::vector<InputActionRecord>& actions, std::int32_t action);
// 004C50D0, __stdcall(action), RET 4, AL: +28h set and +20h clear.
bool input_action_was_pressed_004c50d0(const std::vector<InputActionRecord>& actions, std::int32_t action);

// 004B43B0, __fastcall(ECX screen), AL. any_button_down is the byte
// BSP_InputDeviceTable_AnyDynamicDeviceButtonDown returned; it is stored in
// screen+DDh on both paths. Returns 1 only on a 0 -> non-zero edge.
bool press_start_poll_button_edge_004b43b0(std::uint8_t& latch_dd, std::uint8_t any_button_down) noexcept;

} // namespace bsp::cc13_band004b
