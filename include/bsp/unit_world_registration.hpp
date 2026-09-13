#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

struct VehicleClassDescriptorRow;

// Evidence addresses identify native code and data; they are not callable
// pointers in the rebuilt process. Every row is grounded in its descriptor
// creator, leaf constructor primary-vptr store, and that vtable's +130h slot.
struct UnitWorldRegistration {
    std::uint32_t creator;
    std::uint32_t primary_vtable;
    std::uint32_t native_entry;
    const std::size_t* list_offsets;
    std::size_t list_count;
};

// Exactly the 21 instance creators currently handled by GameUnitsHost's
// kUnitMotionDispatches. Missing descriptors and unsupported creators return
// nullptr; no kind ancestry, default destroyer, or guessed memberships.
const UnitWorldRegistration* unit_world_registration_for_creator(
    std::uint32_t creator) noexcept;
const UnitWorldRegistration* unit_world_registration_for_descriptor(
    const VehicleClassDescriptorRow* descriptor) noexcept;

struct UnitWorldRegistrationHost {
    virtual ~UnitWorldRegistrationHost() = default;

    // Inline native load of [unit+30h]. The producer is the existing placement
    // state in 009258F0. Return the actual borrowed parent/list owner, reloaded
    // on every call; callers must supply a valid placed unit.
    virtual void* parent_0030(void* unit) = 0;

    // 00484540, native ECX=parent+list_offset, stack argument=unit, RET4.
    // Append the direct unit identity to that actual parent's intrusive list.
    // List offsets refer to parent+18h+id*0Ch; this interface does not allocate
    // a world owner or substitute a different list when ownership is missing.
    virtual void push_back_00484540(void* parent, std::uint32_t list_offset,
                                   void* unit) = 0;
};

// Complete 00928560..0092856C: reload parent and append to parent+24h (id1).
// Native ECX=unit, no stack arguments, plain RET. New C++ ABI.
void register_parent_entity_list_00928560(UnitWorldRegistrationHost& host,
                                        void* unit);

// Complete registration schedules of the 21 resolved +130h targets, including
// the shared 00928560 call first. Native ECX=unit, no stack arguments, plain
// RET. The 009288F1 placement caller dispatches only after the parent changes
// and the new [unit+30h] is nonzero. Placement/locking and list allocation are
// existing external contracts, not performed by this sequence.
// Returns false without host calls for an unsupported creator. On success,
// the genuine host owns each append; there is no rollback or deduplication.
// Complete body ranges and original-byte fixture limits:
// docs/UNIT_WORLD_REGISTRATION.md. These are source interfaces, not ABI shims.
bool register_unit_world_lists_for_creator(UnitWorldRegistrationHost& host,
                                          std::uint32_t creator, void* unit);

} // namespace bsp
