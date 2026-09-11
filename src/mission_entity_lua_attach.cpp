// Mission entity Lua self-table attach. docs/MISSION_ENTITY_LUA_ATTACH.md.
//
// Addresses: 00928A00 00928C80 00928F50 0077E830 00779AF0 00951F30 00CE6290.
//
// Each routine below is the native control flow of one virtual slot, with every
// native call expressed as one host method. Nothing here allocates a Lua state,
// formats a NativeString or takes a critical section itself: those are the
// host's contracts. The only executable rules are the branch predicates and the
// key conversion.

#include "bsp/mission_entity_lua_attach.hpp"

#include <string>

namespace bsp {

bool mission_entity_keeps_existing_self_table(bool descriptor_present,
                                              std::int32_t descriptor_kind) noexcept {
    // 00928AAC CMP EAX,EBP / JZ 00928ABE takes the normal path when the
    // descriptor is null, and only then does 00928AB4 compare the kind.
    return descriptor_present &&
           descriptor_kind == static_cast<std::int32_t>(EntityDescriptorKind::ScriptOwned);
}

bool mission_entity_reads_resource_usage(bool descriptor_present,
                                         std::int32_t descriptor_kind) noexcept {
    // 0077E83F TEST EAX,EAX / JZ 0077E88B, then 0077E843 CMP [EAX+4],1 / JNZ.
    return descriptor_present &&
           descriptor_kind == static_cast<std::int32_t>(EntityDescriptorKind::ResourceOwner);
}

bool mission_entity_resource_usage_needs_apply(float resource_usage) noexcept {
    // 0077E86A FLDZ / FXCH / FUCOMIP / FSTP / LAHF / TEST AH,0x44 / JNP.
    // TEST AH,44h isolates ZF and PF; JNP skips when the parity flag is clear,
    // which an unordered compare sets. So a NaN falls through to the call and
    // only an ordered equality with zero skips it.
    return !(resource_usage == 0.0F);
}

std::string mission_entity_lua_key(std::uint16_t network_id) {
    return mission_entity_self_key(network_id);
}

void mission_entity_attach_lua_self_00928a00(const MissionEntityLuaRecord& record,
                                             const void* entity,
                                             MissionEntityLuaAttachHost& host) {
    host.prepare_00927050();  // 00928A1E

    // 00928A2F..00928A6A. The key is formatted and cached on the entity before
    // any Lua work, and before the kind-3 test, so even an entity that keeps
    // its existing table has its key refreshed from the current network id.
    const std::string key = host.format_key(record.network_id);
    host.cache_key_on_entity(key);

    // 00928A9F. The slot object is fetched once and every field write below
    // targets it, including the one on the kind-3 path.
    LuaObject& self = host.self_object();

    if (!mission_entity_keeps_existing_self_table(record.descriptor_present,
                                                  record.descriptor_kind)) {
        // 00928AC2. A slot that is already a table survives; only a non-nil
        // slot is explicitly cleared first, which is the stale-entry drop.
        if (!host.self_object_is_nil(self)) {
            host.clear_stale_slot(key);  // 00928AFF
        }
        host.assign_fresh_table(key);       // 00928B53
        host.set_id_string(self, key);      // 00928BA5, a string, not a number
        host.set_dead_false(self);          // 00928BC7
    }

    // 00928BEE..00928C2F. The kind-3 jump lands here, so `Ptr` is the one field
    // every entity gets on every attach: it is rebound to the live pointer even
    // when the table itself is left alone.
    host.set_ptr_lightuserdata(self, entity);
}

void mission_entity_attach_lua_self_0077e830(const MissionEntityLuaRecord& record,
                                             const void* entity,
                                             MissionEntityLuaAttachHost& host,
                                             float* resource_usage) {
    // 0077E834. The override calls the base implementation of the same slot
    // first, unconditionally, with ECX unchanged.
    mission_entity_attach_lua_self_00928a00(record, entity, host);

    if (!mission_entity_reads_resource_usage(record.descriptor_present,
                                             record.descriptor_kind)) {
        return;
    }

    // 0077E849 FLDZ pushes the default before the name, so the property read is
    // (name, 0.0f) and the result is stored at +304h whatever it is.
    const float value = host.read_resource_usage_property(record.descriptor_owner, 0.0F);
    if (resource_usage != nullptr) {
        *resource_usage = value;
    }
    if (mission_entity_resource_usage_needs_apply(value)) {
        // 0077E878 reloads the descriptor from +C0h rather than reusing EAX, so
        // the owner is read a second time and could differ in principle.
        host.apply_resource_usage_property(record.descriptor_owner);
    }
}

void mission_entity_on_killed_00928c80(const MissionEntityLuaRecord& record,
                                       MissionEntityLuaAttachHost& host) {
    // 00928C9B..00928CA2. The whole Lua half is gated on the cached key being
    // non-empty, which is the mark that an attach has run for this entity.
    if (!record.key_string_empty) {
        host.enter_entity_lock();

        LuaObject& self = host.self_object();   // 00928CDC
        host.set_ptr_null(self);                // 00928D04, lightuserdata(0)

        LuaObject& last = host.make_last_position_table(self);  // 00928D49/00928D81
        if (record.pose_stale) {
            host.refresh_world_pose();  // 00928D97, only when +C8h is zero
        }
        host.set_number(last, "x", record.pose_x);  // 00928DE6
        host.set_number(last, "y", record.pose_y);  // 00928E32
        host.set_number(last, "z", record.pose_z);  // 00928E7E

        host.leave_entity_lock();
    }

    // 00928ED9..00928F14 run outside the gate and outside the lock.
    host.log_entity_killed(record.network_id);
}

void mission_entity_set_party_race_00928f50(const MissionEntityLuaRecord& record,
                                            MissionEntityLuaAttachHost& host,
                                            std::int32_t a, std::int32_t b,
                                            std::int32_t c) {
    host.base_set_party_race_00923b80(a, b, c);  // 00928F7D

    // 00928F89. The mirror reads the entity fields the base call has just
    // written, not the arguments: 00928FC7 and 00929034 both load from ESI.
    LuaObject& self = host.self_object();
    host.set_integer(self, "Race", record.race);    // 00928FD9
    host.set_integer(self, "Party", record.party);  // 00929046
}

}  // namespace bsp
