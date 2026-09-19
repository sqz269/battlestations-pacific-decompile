#pragma once

#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Actual 20h prefix of the native scene/game/unit object. Reuse the existing
// raw endpoint layout, but retain its two distinct roles and producers. No
// constructors, default initialization, allocations or owned sidecars.
struct NativeUnitObserverPrefixStorage {
    NativeObserverOwnerStorage observed_00;
    NativeObserverOwnerStorage callback_10;
};
static_assert(sizeof(NativeUnitObserverPrefixStorage) == 0x20);
static_assert(offsetof(NativeUnitObserverPrefixStorage, observed_00) == 0);
static_assert(offsetof(NativeUnitObserverPrefixStorage, callback_10) == 0x10);

// Borrowed process mapping, not a native field or another unit owner. Embed
// prefixes in the existing stable unit owner; canonical_unit is that owner's
// existing world-list identity. Both must denote the same live unit. Observer
// edges/node+14/getters use &prefixes.observed_00; callbacks use the separately
// adjusted callback base. Resolve back to canonical_unit through this binding,
// never by casting the semantic unit object's unrelated bytes as a raw prefix.
struct NativeUnitObserverAlias {
    void* canonical_unit;
    NativeUnitObserverPrefixStorage& prefixes;
};

// Partial native 00925CE0 store projections, NOT complete constructors:
// 00925CFF and 00925D0A..D12 set observed table CECCC8 and its three words=0.
// 00925D13..D22 sets callback table CE3CD4 and its three words=0.
// Native ESI=original ECX unit, EBX=0 from00925CFD. Source writes only the
// indicated 10h member. Other prefix/owner bytes and native frame bookkeeping
// are not initialized. Existing arrays must not be live when these are called.
void initialize_unit_observed_prefix_00925cff(NativeUnitObserverPrefixStorage&) noexcept;
void initialize_unit_callback_prefix_00925d13(NativeUnitObserverPrefixStorage&) noexcept;

// Two-store partial projections only. They preserve both edge arrays and
// every unrelated byte. Their complete enclosing constructors perform other
// required work; these functions do not construct a scene node or game unit.
void publish_scene_observer_tables_00925d44(NativeUnitObserverPrefixStorage&) noexcept;
void publish_game_entity_observer_tables_00928662(NativeUnitObserverPrefixStorage&) noexcept;

struct NativeUnitObserverTablePair {
    std::uint32_t creator;
    std::uint32_t observed_table;
    std::uint32_t callback_table;
};

// Exact 21 creator identities already supported by unit_world_registration.
// Metadata comes from each actual leaf constructor's unit+0/+10 stores, not
// from table adjacency or class ancestry. Unsupported creator returns null.
const NativeUnitObserverTablePair* unit_observer_tables_for_creator(
    std::uint32_t creator) noexcept;

// Partial leaf-store projection: set only the two actual vptr words selected
// by that creator. Does not run its allocator/parent constructor/remaining
// stores. Returns false without writes for an unsupported creator. Publish
// only at an explicit corresponding source construction stage; it is not a
// fallback factory or permission to use an incompletely constructed unit.
bool publish_unit_leaf_observer_tables_for_creator(
    NativeUnitObserverPrefixStorage&, std::uint32_t creator) noexcept;

// All table words are native identities, not callable process pointers.
// Primary slot04 is0042B970 for the 21 leaf tables. Callback slot04/08 varies
// by producer and requires its actual provider; no default handler is added.
// Native00925780 destroys callback+10 via00695870 before observed+0 via00695760.
// These are different lifetime operations, neither frees the containing unit.
// Secondary deleting thunks subtract10h before whole-unit deleting methods.
// Notification, full teardown and owning allocation remain externally owned.
} // namespace bsp
