// The tails around an entity leaving the world: what the scalar deleting destructor
// unregisters, the three writers of the +5Ch..+70h gate bytes, and the objective
// marker drops that hang off the same observer edges.
//
// Packet cc2_entity_lifecycle_tails, worktree agent/cc2-entity-lifecycle-tails.
// Ghidra was read-only for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/ENTITY_LIFECYCLE_TAILS.md carries the evidence and
// reports/entity_lifecycle_tails.json the call rows.
//
// SceneNodeFlags (+5Ch..+5Fh) comes from bsp/hit_narrowphase.hpp and is not redeclared.
// The Objective record and the ObjectiveSet offsets come from bsp/objective_units.hpp.
//
// Not modelled: the MSVC std::list and _Tree node churn (008DE6A0 is
// std::set<Entity*>::insert and 009269B0's rollback tail is the compiler's strong
// guarantee, both library contracts), and the levels of the unit destructor above
// 009287B0, whose Ghidra body is short of the real one.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/hit_narrowphase.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Offsets the release chain reads. Each cites the instruction that uses it.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kEntityOffObserverEdgeArray = 0x04;   // 00695760, free(param_1[1])
inline constexpr std::size_t kEntityOffObserverList = 0x08;        // 00695760, the detach gate
inline constexpr std::size_t kEntityOffCallbackOwner = 0x10;       // 0092589D, ECX = ESI+0x10
inline constexpr std::size_t kEntityOffWeakOwner = 0x24;           // 00925891, ECX = EBP = ESI+0x24
inline constexpr std::size_t kEntityOffWorld = 0x30;               // 009257F5, 0092581A, 0092582E
inline constexpr std::size_t kEntityOffUpdateLinkPrev = 0x34;      // 00903F34
inline constexpr std::size_t kEntityOffUpdateLinkNext = 0x38;      // 00903F3B
// The parent pointer at +3Ch is already declared as kEntityOffParent in
// bsp/gun_bot_ticks.hpp with the same value; 00925801 and 00925811 read and clear it.
inline constexpr std::size_t kEntityOffSiblingLinkPrev = 0x40;     // 00924714
inline constexpr std::size_t kEntityOffSiblingLinkNext = 0x44;     // 0092471B, 00922FFD
inline constexpr std::size_t kEntityOffChildListHead = 0x48;       // 009257E0, 0092580C, 00922FD4
inline constexpr std::size_t kEntityOffChildCount = 0x50;          // 009257D4, 009257EF
inline constexpr std::size_t kEntityOffDeadMeatMark = 0x70;        // 009272F3
inline constexpr std::size_t kEntityOffWorldListCounter = 0xB8;    // 00925825, 009245A0
inline constexpr std::size_t kEntityOffSpawnDescriptor = 0xC0;     // 00927080
inline constexpr std::size_t kEntityOffOwnedRef = 0x168;           // 009257BA
inline constexpr std::size_t kEntityOffIdWord = 0x174;             // 009287EB, a u16
inline constexpr std::size_t kEntityOffNameLength = 0x178;         // 00928824
inline constexpr std::size_t kEntityOffNamePointer = 0x17C;        // 00928815

// The intrusive list heads the release unlinks from, relative to the world object.
inline constexpr std::size_t kWorldOffUpdateChainHead = 0x04;  // 009257F8 MOV ECX,[ECX+0x4]
inline constexpr std::size_t kWorldOffRootListHead = 0x08;     // 0092581D MOV ECX,[EDX+0x8]
inline constexpr std::size_t kWorldOffUnitListHead = 0x0C;     // 00925832 ADD ECX,0xc

// A head object of either intrusive chain: first, last and the count 00903F30 and
// 00924710 both decrement at [ECX+8h].
inline constexpr std::size_t kChainHeadOffFirst = 0x00;  // 00903F56, 00924736
inline constexpr std::size_t kChainHeadOffLast = 0x04;   // 00903F6A, 0092474A
inline constexpr std::size_t kChainHeadOffCount = 0x08;  // 00903F7B, 0092475B

// The two entity id registries and the bound that chooses between them.
inline constexpr std::uint32_t kEntityIdRegistryLowAddress = 0x00F89A08u;   // 00928804
inline constexpr std::uint32_t kEntityIdRegistryHighAddress = 0x00F89A5Cu;  // 0092880B
inline constexpr std::uint32_t kEntityIdRegistrySplitAddress = 0x00F89A10u; // 009287F5

// The marker-manager map keyed by unit, and the per-entry field 006DE3F0 tests.
inline constexpr std::size_t kMarkerManagerOffUnitMap = 0x14;   // 006DE3FF LEA EBX,[ECX+0x14]
inline constexpr std::size_t kMarkerEntryOffKeepFlag = 0x14;    // 006DE47A
// The std::set<Entity*> the objective add path inserts into.
inline constexpr std::size_t kObjectiveSetOffUnitSet = 0x18;    // 008DF3D4 LEA ECX,[EBX+0x18]

// ---------------------------------------------------------------------------
// 00922FD0 Kill, 009273A0's pending flush and 00927050's deadMeat write, as rules
// over the SceneNodeFlags of bsp/hit_narrowphase.hpp.
// ---------------------------------------------------------------------------

// 009272EF and 009272F3: the mission Lua property, applied at creation. Returns the
// value written to entity+70h alongside the torn-down byte.
bool spawn_apply_dead_meat_009272ef(SceneNodeFlags& node, bool dead_meat) noexcept;

// 009274CE..009274DA, the pending flush's teardown write. Distinct from
// scene_node_kill_00922fd0: it also sets the removed byte and never touches +6Ch.
void pending_flush_mark_teardown_009274ce(SceneNodeFlags& node) noexcept;

// 00927086: the descriptor kind at *(entity[+C0h] + 4h) selects the branch.
enum class SpawnDescriptorKind : std::uint8_t {
    none = 0,          // entity[+C0h] == 0, 00927080 falls through to the return
    property_bag = 1,  // 00927089, the Race/Party/GuiName/SetHierarchy branch
    lua_table = 3,     // 0092721B, the _entity branch
    other = 0xFF,      // 0092721E, any other value returns without doing anything
};
SpawnDescriptorKind spawn_descriptor_kind_00927086(bool descriptor_present, std::int32_t kind) noexcept;

// ---------------------------------------------------------------------------
// The release. One enumerator per step of the rule table in
// docs/ENTITY_LIFECYCLE_TAILS.md section 1, in native order.
// ---------------------------------------------------------------------------
enum class EntityReleaseStep : std::uint8_t {
    release_entity_id,          // 00928810, 009516D0
    free_name_string,           // 00928831
    release_owned_ref,          // 009257CF, 00740270
    destroy_children,           // 009257ED, the +50h loop
    unlink_update_chain,        // 009257FC, 00903F30
    unlink_from_parent,         // 0092580C, 00924710 with ECX = parent+48h
    unlink_from_world_roots,    // 00925820, 00924710 with ECX = *(world+8h)
    remove_from_world_unit_list,// 00925835, 004845A0
    free_pooled_string_160,     // 00925856
    free_pooled_string_158,     // 0092587E
    destroy_weak_owner,         // 00925891, 00925540
    destroy_callback_owner,     // 0092589D, 00695870
    detach_observer_edges,      // 009258AC, 00695760 -> 006953C0
    free_observer_edge_array,   // inside 00695760, free(entity[+4h])
    free_instance,              // 006FE580, gated on the flags argument
};

// The state the chain branches on. Every field names the instruction that reads it.
struct EntityReleaseState {
    bool is_game_entity{true};        // the 009287B0 level is in the chain
    bool has_name_string{true};       // 0092881B, entity[+17Ch] != 0
    bool has_owned_ref{true};         // 009257BA, entity[+168h]
    bool has_children{false};         // 009257D4, entity[+50h] != 0
    bool has_parent{false};           // 00925804, entity[+3Ch] != 0
    std::int32_t world_list_counter{0};  // 00925825, entity[+B8h]
    bool has_pooled_string_160{false};   // 00925840
    bool has_pooled_string_158{false};   // 00925868
    bool has_observer_list{true};        // 00695760, entity[+8h] != 0
    bool has_observer_edge_array{true};  // 00695760, entity[+4h] != 0
    bool free_flag{true};                // 006FE578, TEST byte ptr [ESP+8],1
};

// The ordered steps 006FE570 performs for `state`. Pure: no host, no storage.
std::vector<EntityReleaseStep> entity_release_steps_006fe570(const EntityReleaseState& state);

// 009287F5..0092880B: which id registry the release returns the id to.
bool entity_id_uses_low_registry_009287f5(std::uint16_t id, std::uint32_t split) noexcept;

// ---------------------------------------------------------------------------
// 00903F30 and 00924710: the same unlink over two different link pairs.
// ---------------------------------------------------------------------------
struct IntrusiveChainNode {
    std::size_t prev{0};  // +34h / +40h, 0 when this node is first
    std::size_t next{0};  // +38h / +44h, 0 when this node is last
};

// One chain: `first` and `last` are node handles (0 = empty) and `count` is the
// value at head+8h. Nodes are addressed by a caller-chosen handle, which stands in
// for the entity pointer the native code links.
struct IntrusiveChain {
    std::size_t first{0};
    std::size_t last{0};
    std::int32_t count{0};
};

// 00903F30 / 00924710 exactly, including the early-out at 00903F40: a node that is
// unlinked on both sides is still unlinked when the chain holds more than one entry,
// because the listing only skips the work when count <= 1.
void chain_unlink_00903f30(IntrusiveChain& chain, std::size_t node,
                           std::vector<IntrusiveChainNode>& nodes) noexcept;

// ---------------------------------------------------------------------------
// 006DE3F0, the marker drop
// ---------------------------------------------------------------------------
struct MarkerDropOutcome {
    bool destroyed_slot_object{false};  // 006DE424, slot->vtable[0](1)
    bool cleared_slot{false};           // 006DE426, *slot = 0
    bool unregistered_pair{false};      // 006DE4AC, 006952A0
};

// `slot_has_object` is *slot at 006DE418; `entry_keep_flags` are the +14h fields of
// the per-unit container in iteration order; `pair_registered` is 00694AF0's answer.
// The loop returns on the FIRST entry with a set flag (006DE47E), so a later cleared
// entry cannot re-enable the unregister.
MarkerDropOutcome marker_drop_unit_006de3f0(bool slot_has_object,
                                            const std::vector<bool>& entry_keep_flags,
                                            bool pair_registered) noexcept;

// ---------------------------------------------------------------------------
// 008DC3E0, the objective unit-list erase
// ---------------------------------------------------------------------------
struct ObjectiveUnitEntryHandle {
    std::size_t unit{0};  // record+0h; 0 is a position entry
};

struct ObjectiveEraseResult {
    std::vector<ObjectiveUnitEntryHandle> remaining;
    std::int32_t erased{0};      // one per freed node
    std::int32_t size_after{0};  // the value left at objective+28h
};

// Erases EVERY record whose +0h equals `unit` and decrements _Mysize once per erase
// (008DC473 ADD dword ptr [EBP+8],-1, read from the PE bytes because the listing
// omits the run after the free at 008DC46B).
ObjectiveEraseResult objective_erase_unit_entries_008dc3e0(
    const std::vector<ObjectiveUnitEntryHandle>& entries, std::size_t unit,
    std::int32_t size_before);

// ---------------------------------------------------------------------------
// The release as a sequence over an injected host: one method per native call site.
// Nothing here stands in for unrecovered behaviour; a step the host cannot perform
// is simply not called.
// ---------------------------------------------------------------------------
class EntityReleaseHost {
  public:
    virtual ~EntityReleaseHost() = default;

    // 00928810 -> 009516D0. `low_registry` is the 009287F5 choice.
    virtual void release_entity_id(std::uint16_t id, bool low_registry) = 0;
    // 00928831 -> 00419CC0 then 00BD1510, and the two twins at 0092584F / 00925877.
    virtual void free_pooled_string(std::size_t field_offset) = 0;
    // 009257CF -> 00740270, the InterlockedDecrement release of entity+168h.
    virtual void release_owned_ref() = 0;
    // 009257ED, entity[+48h]->vtable[0](1), repeated while entity[+50h] != 0.
    virtual void destroy_next_child() = 0;
    // 009257FC -> 00903F30 with ECX = *(world+4h).
    virtual void unlink_update_chain() = 0;
    // 0092580C / 00925820 -> 00924710. `parent_head` is false for the world roots.
    virtual void unlink_sibling_chain(bool parent_head) = 0;
    // 00925811, the store that follows the parent unlink.
    virtual void clear_parent() = 0;
    // 00925835 -> 004845A0 with ECX = world+0Ch.
    virtual void remove_from_world_unit_list() = 0;
    // 00925891 -> 00925540 and 0092589D -> 00695870.
    virtual void destroy_weak_owner() = 0;
    virtual void destroy_callback_owner() = 0;
    // 009258AC -> 00695760, which calls 006953C0 under the observer lock.
    virtual void detach_observer_edges() = 0;
    virtual void free_observer_edge_array() = 0;
    // 006FE580, the scalar deleting destructor's own free.
    virtual void free_instance() = 0;
};

// Runs the rule table against `host`. `child_count` is entity[+50h] at entry and is
// the number of destroy_next_child calls the native loop makes.
void entity_release_006fe570(EntityReleaseHost& host, const EntityReleaseState& state,
                             std::uint16_t entity_id, std::uint32_t id_split,
                             std::int32_t child_count);

}  // namespace bsp
