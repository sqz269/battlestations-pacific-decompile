#pragma once

#include <cstdint>

namespace bsp {

// Borrowed fields of the existing unit and dummy owners. These views neither
// initialize nor own storage. +6B8 is UNIT DummyObjectID, not the unrelated
// descriptor DamageThreshold at the same offset. The two float fields are
// copied as words, including NaN payloads, by the native MOVSS schedule.
struct UnitDummyBindingFields {
    std::int32_t& dummy_object_id_06b8;
    float& visibility_02f4;
    float& visibility_02f8;
};

struct UnitDummySourceFields {
    const std::int32_t& dummy_object_id_0484;
    const std::uint8_t& visibility_override_02f0;
    const float& visibility_02f4;
    const float& visibility_02f8;
};

struct UnitDummyBindingHost {
    virtual ~UnitDummyBindingHost() = default;

    // Pure borrowed-storage access. All identities and fields must be valid
    // for the native operation's lifetime; missing owners are not empty lists.
    virtual UnitDummyBindingFields unit_fields(void* unit) = 0;
    virtual UnitDummySourceFields dummy_fields(void* dummy) = 0;

    // [game=00E188A8]+19CC -> world; list45 is world+234 (head+238).
    // Nodes have next+4 and the direct dummy identity at+8. Reload next only
    // after an index mismatch; stop at the first matching dummy.
    virtual void* world_list45_head() = 0;
    virtual void* list_value_0008(void* node) = 0;
    virtual void* list_next_0004(void* node) = 0;

    virtual void* frontend_00e198c4() = 0;
    virtual std::int32_t frontend_mode_0020(void* frontend) = 0;
    // [[frontend+8C]+4C], then +398 if nonnull, otherwise nullptr.
    // The +8C owner itself is required when mode==26h.
    virtual void* selected_dummy_0398(void* frontend) = 0;
    virtual void* unit_virtual_0140(void* unit) = 0;
    virtual void* hud_root_0040(void* frontend) = 0;
    // Native ECX=HUD root, one stack unit, RET4. The actual HUD selection
    // service, not a local assignment to the selected-dummy projection.
    virtual void set_spectated_unit_00647300(void* hud, void* unit) = 0;

    // Native global byte is loaded separately for the two visibility stores.
    virtual std::uint8_t visibility_override_00f87152() = 0;
    // Native ECX=dummy, one stack boolean, RET4; writes dummy+459 and updates
    // the attached visibility object when present.
    virtual void set_dummy_visible_006e0b40(void* dummy, bool visible) = 0;
    // Reload [game=00E188A8]+1FE4 after the visibility callback.
    virtual std::int32_t session_mode_1fe4() = 0;
    // Native ECX=dummy, one stack cause, RET4. Existing queued kill service
    // includes its lock, child recursion, +5F transition and pending list.
    virtual void kill_entity_00926d90(void* dummy, std::int32_t cause) = 0;
};

// Complete control/store/call schedule 00953A80..00953B7E. Native ECX=unit,
// one signed stack ID, RET4 at00953AC4 and00953B7C. New C++ source ABI.
// Stores the requested ID even if list45 is empty or has no match. Required
// services are live contracts; this does not provide a second flags owner.
// Constructor -1 at0095CDE9 and base+5E=0 at00925E11 are only initial state.
void bind_unit_dummy_object_00953a80(UnitDummyBindingHost& host, void* unit,
                                   std::int32_t dummy_object_id);

} // namespace bsp
