#include "bsp/session_participant_pools.hpp"

namespace bsp {
namespace {
ParticipantByte known_byte(std::uint8_t value) noexcept { return {value, true}; }
}

SessionParticipantPools::SessionParticipantPools(ParticipantPoolInitialization initial) noexcept {
    if (initial == ParticipantPoolInitialization::ZeroedGameAllocation) {
        // This is an explicit allocation provenance, not a claim that
        // 004D6BA0 itself writes any of these bytes. Active native pointers
        // are null until the reset/store sequence publishes them.
        const ParticipantRecordBytes zero{known_byte(0), known_byte(0), known_byte(0)};
        players_.fill(zero);
        mission_.fill(zero);
    }
}

ParticipantRecordBytes* SessionParticipantPools::record_bytes(ParticipantRecordId record) noexcept {
    if (record.index >= kSceneSlotRecordCount) return nullptr;
    switch (record.pool) {
    case ParticipantPool::Player: return &players_[record.index];
    case ParticipantPool::Mission: return &mission_[record.index];
    }
    return nullptr;
}

const ParticipantRecordBytes* SessionParticipantPools::record_bytes(ParticipantRecordId record) const noexcept {
    if (record.index >= kSceneSlotRecordCount) return nullptr;
    switch (record.pool) {
    case ParticipantPool::Player: return &players_[record.index];
    case ParticipantPool::Mission: return &mission_[record.index];
    }
    return nullptr;
}

bool SessionParticipantPools::adopt_record_bytes(ParticipantRecordId record,
    const ParticipantRecordBytes& bytes) noexcept {
    ParticipantRecordBytes* target = record_bytes(record);
    if (!target) return false;
    *target = bytes;
    return true;
}

bool SessionParticipantPools::read_record_bytes(ParticipantRecordId record,
    ParticipantRecordBytes& out) const noexcept {
    const ParticipantRecordBytes* target = record_bytes(record);
    if (!target) return false;
    out = *target;
    return true;
}

bool SessionParticipantPools::reset_flags_004bb160(bool have_scene,
    std::int32_t scene_slot_count) noexcept {
    if (have_scene && scene_slot_count > static_cast<std::int32_t>(kSceneSlotRecordCount)) {
        return false;
    }
    // 004BB169..004BB1C3: all eight current pointers change before record writes.
    for (std::size_t slot = 0; slot < kSceneSlotRecordCount; ++slot) {
        active_[slot] = {ParticipantPool::Mission, slot};
    }
    const std::size_t count = !have_scene ? kSceneSlotRecordCount
        : scene_slot_count > 0 ? static_cast<std::size_t>(scene_slot_count) : 0;
    // 004BB21C/220, or 004BB36F/373 in the no-scene arm.
    for (std::size_t slot = 0; slot < count; ++slot) {
        mission_[slot].claimed_08 = known_byte(1);
        mission_[slot].ai_held_09 = known_byte(1);
    }
    // 004BB2D0 clears ONLY +8 of the remaining mission records.
    for (std::size_t slot = count; slot < kSceneSlotRecordCount; ++slot) {
        mission_[slot].claimed_08 = known_byte(0);
    }
    // 004BB30B/30E: the other pool is reset to unclaimed, non-AI records.
    for (ParticipantRecordBytes& record : players_) {
        record.claimed_08 = known_byte(0);
        record.ai_held_09 = known_byte(0);
    }
    return true;
}

ParticipantClaimResult SessionParticipantPools::claim_player_flags_004bb440(
    ParticipantRecordId& out) noexcept {
    for (std::size_t slot = 0; slot < kSceneSlotRecordCount; ++slot) {
        ParticipantByte& claimed = players_[slot].claimed_08;
        if (!claimed.available) return ParticipantClaimResult::Unavailable;
        if (claimed.value == 0) {
            claimed = known_byte(1); // 004BB47D; +9 is not modified by this routine.
            out = {ParticipantPool::Player, slot};
            return ParticipantClaimResult::Claimed;
        }
    }
    return ParticipantClaimResult::Full;
}

bool SessionParticipantPools::publish_active_record(std::size_t slot,
    ParticipantRecordId record) noexcept {
    if (slot >= kSceneSlotRecordCount || !record_bytes(record)) return false;
    active_[slot] = record;
    return true;
}

bool SessionParticipantPools::active_record(std::size_t slot,
    ParticipantRecordId& out) const noexcept {
    if (slot >= kSceneSlotRecordCount || !record_bytes(active_[slot])) return false;
    out = active_[slot];
    return true;
}

bool SessionParticipantPools::install_record_004bb630(std::size_t slot,
    ParticipantRecordId record) noexcept {
    return publish_active_record(slot, record); // 004BB638
}

bool SessionParticipantPools::restore_mission_record_004bb660(std::size_t slot) noexcept {
    ParticipantRecordId previous;
    if (!active_record(slot, previous)) return false;
    // 004BB66B first dereferences old pointer for +20=-1 (unrepresented),
    // then 004BB681 publishes this mission record. The old bytes retain.
    active_[slot] = {ParticipantPool::Mission, slot};
    return true;
}

ParticipantClaimResult SessionParticipantPools::reset_and_claim_local_flags(bool have_scene,
    std::int32_t scene_slot_count, ParticipantRecordId& out) noexcept {
    if (!reset_flags_004bb160(have_scene, scene_slot_count)) {
        return ParticipantClaimResult::Unavailable;
    }
    ParticipantRecordId claimed;
    const ParticipantClaimResult result = claim_player_flags_004bb440(claimed);
    if (result == ParticipantClaimResult::Claimed) {
        publish_active_record(0, claimed); // 004DFD5C, after 004DFD57 returned.
        out = claimed;
    }
    return result;
}

bool SessionParticipantPools::try_ai_held_00927f10(std::int32_t slot,
    std::uint8_t& out) const noexcept {
    if (slot < 0 || slot >= static_cast<std::int32_t>(kSceneSlotRecordCount)) return false;
    const ParticipantRecordBytes* record = record_bytes(active_[static_cast<std::size_t>(slot)]);
    if (!record || !record->ai_held_09.available) return false;
    out = record->ai_held_09.value; // 00927F21: raw AL, no +8 test.
    return true;
}

bool SessionParticipantPools::write_device_bound(ParticipantRecordId record,
    std::uint8_t value) noexcept {
    ParticipantRecordBytes* target = record_bytes(record);
    if (!target) return false;
    target->device_bound_0e = known_byte(value);
    return true;
}

bool SessionParticipantPools::try_entry_slots(
    MissionEntryPlayerSlot (&out)[kLocalPlayerSlotCount]) const noexcept {
    static_assert(kLocalPlayerSlotCount == kSceneSlotRecordCount, "same native pointer table");
    MissionEntryPlayerSlot pending[kLocalPlayerSlotCount];
    for (std::size_t slot = 0; slot < kLocalPlayerSlotCount; ++slot) {
        const ParticipantRecordBytes* record = record_bytes(active_[slot]);
        if (!record || !record->ai_held_09.available || !record->device_bound_0e.available) return false;
        pending[slot].excluded = record->ai_held_09.value != 0;
        pending[slot].device_bound = record->device_bound_0e.value != 0;
    }
    for (std::size_t slot = 0; slot < kLocalPlayerSlotCount; ++slot) out[slot] = pending[slot];
    return true;
}

} // namespace bsp
