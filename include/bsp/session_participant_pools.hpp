#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "bsp/mission_load_path.hpp"
#include "bsp/mission_state_entry.hpp"

namespace bsp {

// Partial ownership projection of the two native 8 * 118h participant pools:
// player records at game+748h, mission records at game+1008h, and the current
// pointer table at game+18CCh. No vtables, strings or other record fields are
// synthesized. Evidence: docs/SESSION_PARTICIPANT_AI_FLAG.md.
enum class ParticipantPool : std::uint8_t { Player, Mission };

struct ParticipantRecordId {
    ParticipantPool pool{ParticipantPool::Player};
    std::size_t index{kSceneSlotRecordCount}; // no represented record
};

struct ParticipantByte {
    std::uint8_t value{0};
    bool available{false};
};

struct ParticipantRecordBytes {
    ParticipantByte claimed_08;
    ParticipantByte ai_held_09;
    ParticipantByte device_bound_0e;
};

enum class ParticipantPoolInitialization : std::uint8_t {
    Unavailable,
    // Explicit provenance: 0073E163 memset(game,0,71A0h), before the two
    // 004D6BA0 constructor arrays. Those constructors retain these three bytes.
    ZeroedGameAllocation,
};

enum class ParticipantClaimResult : std::uint8_t { Claimed, Full, Unavailable };

class SessionParticipantPools {
public:
    explicit SessionParticipantPools(ParticipantPoolInitialization initial =
        ParticipantPoolInitialization::Unavailable) noexcept;

    // Seeds an explicitly observed record projection, e.g. a fixture or a
    // real restored owner. The caller supplies availability for each byte.
    bool adopt_record_bytes(ParticipantRecordId record,
                            const ParticipantRecordBytes& bytes) noexcept;
    bool read_record_bytes(ParticipantRecordId record,
                           ParticipantRecordBytes& out) const noexcept;

    // Partial 004BB160..004BB3DB, ECX game, RET: current pointers and +8/+9
    // stores only. With a scene, signed counts <=0 populate no records; >8
    // is outside this bounded representation and returns false untouched.
    // No scene populates all eight. Trailing mission +9 and every +0E retain
    // their previous values AND availability. Player +8/+9 become known zero.
    bool reset_flags_004bb160(bool have_scene, std::int32_t scene_slot_count) noexcept;

    // Partial 004BB440 (ECX game, nine stack words, RET24h): the first-free
    // selection and +8=1 store only. Unknown earlier +8 cannot be skipped.
    // Names, user callbacks and every other record field remain outside scope.
    // out is written only for Claimed; +9 and +0E are retained.
    ParticipantClaimResult claim_player_flags_004bb440(ParticipantRecordId& out) noexcept;

    // The common current-pointer store. Explicit identity replaces a native
    // pointer; this does not assign a unit role or copy any Party field.
    bool publish_active_record(std::size_t slot, ParticipantRecordId record) noexcept;
    bool active_record(std::size_t slot, ParticipantRecordId& out) const noexcept;

    // Partial 004BB630: only its first current-pointer store (ECX game,
    // record and slot stack args, RET8); +20/+24/+28 are not represented.
    bool install_record_004bb630(std::size_t slot, ParticipantRecordId record) noexcept;
    // Partial 004BB660: require the old non-null identity, then restore the
    // corresponding mission identity. Old record+20=-1 is outside scope.
    bool restore_mission_record_004bb660(std::size_t slot) noexcept;

    // Exact relevant order at 004DFD18..004DFD5C: reset, first-free claim,
    // then current[0] publication. This is a flag/identity fragment, not the
    // full load or claim service. Does not manufacture names or Party values.
    ParticipantClaimResult reset_and_claim_local_flags(bool have_scene,
        std::int32_t scene_slot_count, ParticipantRecordId& out) noexcept;

    // Complete five-instruction 00927F10's field read over represented inputs.
    // Native stdcall(slot), RET4, raw byte in AL; ECX is not an input.
    // Native has no range/null/claimed test. This typed interface reports
    // unavailable for a missing identity/byte or slot outside 0..7, including8.
    // Caller-owned output remains untouched on every unavailable read.
    bool try_ai_held_00927f10(std::int32_t slot, std::uint8_t& out) const noexcept;

    // Carries an actual +0E writer without asserting when its delivery occurs.
    bool write_device_bound(ParticipantRecordId record, std::uint8_t value) noexcept;
    // Existing entry consumer view; excluded is exactly this same +9 byte.
    // No output changes unless all eight selected +9/+0E pairs are available.
    bool try_entry_slots(MissionEntryPlayerSlot (&out)[kLocalPlayerSlotCount]) const noexcept;

private:
    ParticipantRecordBytes* record_bytes(ParticipantRecordId record) noexcept;
    const ParticipantRecordBytes* record_bytes(ParticipantRecordId record) const noexcept;
    std::array<ParticipantRecordBytes, kSceneSlotRecordCount> players_{};
    std::array<ParticipantRecordBytes, kSceneSlotRecordCount> mission_{};
    std::array<ParticipantRecordId, kSceneSlotRecordCount> active_{};
};

} // namespace bsp
