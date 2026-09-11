#pragma once
// The 0x1188-byte unit instance that a vehicle-class descriptor's vtable slot
// +28h allocates, and the six-level constructor chain behind it.
// docs/UNIT_INSTANCE_LAYOUT.md. Addresses: 006fe590, 006fe460, 0081ed40,
// 0095cc90, 0087b670, 0077eed0, 00928630, 00925ce0, 009553d0, 00928860,
// 009258f0, 006fe620, 00928560.
//
// Names here are hypotheses, not recovered symbols. The field offsets already
// carried by include/bsp/unit_instance.hpp are not repeated; this header adds
// the ones the constructor chain and the placement path establish.
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Sizes and vtables
// ---------------------------------------------------------------------------

// PUSH 1188h at 006FE5A8 and again at 006FE5B4 for the memset.
inline constexpr std::size_t kUnitInstanceAllocationSize = 0x1188;

// Highest field 0081ED40 writes is +1184h (param_1[0x461]), four bytes short of
// the allocation, so the level-5 base owns the whole object and 006FE460 adds
// no fields of its own.
inline constexpr std::size_t kUnitInstanceBaseHighWater = 0x1184;

// ---------------------------------------------------------------------------
// The constructor chain
// ---------------------------------------------------------------------------

// Six constructors run in this order, each writing its own depth to +C4h.
// The depth value 3 is never written by any of them.
inline constexpr int kUnitConstructorLevels = 6;

struct UnitConstructorLevel {
    std::uint32_t address;   // the constructor
    std::uint32_t call_site; // where the level below calls it, 0 for the deepest
    int class_id;            // the value it stores at +C4h
};

// Innermost first. 00925CE0 is a scene-graph node, not a vehicle.
inline constexpr UnitConstructorLevel kUnitConstructorChain[kUnitConstructorLevels + 1] = {
    {0x00925ce0, 0x00928651, 0}, // scene node: links, matrix, name, gate bytes
    {0x00928630, 0x0077eef9, 1}, // game entity: the u16 id at +174h, the world lock
    {0x0077eed0, 0x0087b694, 2}, // three 0x34-byte slots at +1E8h, +21Ch, +250h
    {0x0087b670, 0x0095ccb3, 4}, // fixed-step tick node at +310h
    {0x0095cc90, 0x0081ed6a, 5}, // +538h class reference cleared, two arrays
    {0x0081ed40, 0x006fe468, 6}, // the vehicle base: everything up to +1184h
    {0x006fe460, 0x006fe5dd, 7}, // MDestroyer: eight vptrs and the class id
};

// Value of the dword at +C4h (kUnitOffClassId in unit_instance.hpp) after the
// chain has run down to `level`, counting 0 for 00925CE0. Returns -1 outside
// the chain. Pure projection of the seven MOV [this+C4h], imm instructions.
int unit_constructor_class_id(int level) noexcept;

// The eight vtable pointers 006FE460 installs at 006FE46D..006FE4B3. Every
// level of the chain rewrites the same eight offsets; only the last set
// survives. Offsets and values are read from the listing, in write order.
inline constexpr int kUnitVptrSlotCount = 8;
inline constexpr std::size_t kUnitVptrSlotOffsets[kUnitVptrSlotCount] = {
    0x000, 0x010, 0x024, 0x170, 0x1e4, 0x310, 0x38c, 0x72c,
};
inline constexpr std::uint32_t kUnitVptrSlotValues[kUnitVptrSlotCount] = {
    0x00cfc3d0, 0x00cfc3b8, 0x00cfc3b0, 0x00cfc3ac,
    0x00cfc3a4, 0x00cfc38c, 0x00cfc388, 0x00cfc384,
};

// ---------------------------------------------------------------------------
// Fields this packet establishes
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitLayoutOffWeakOwner = 0x024;      // 00925D34, ctor 00925490
inline constexpr std::size_t kUnitLayoutOffHierarchyParent = 0x03c; // 00925935
inline constexpr std::size_t kUnitLayoutOffHierarchyPrev = 0x040;  // 00925948
inline constexpr std::size_t kUnitLayoutOffHierarchyNext = 0x044;  // 0092594E
inline constexpr std::size_t kUnitLayoutOffChildHead = 0x048;      // 009258F0 reads, 00925CE0 zeroes
inline constexpr std::size_t kUnitLayoutOffChildTail = 0x04c;
inline constexpr std::size_t kUnitLayoutOffChildCount = 0x050;
inline constexpr std::size_t kUnitLayoutOffLocalMatrix = 0x074;    // 00925DCA identity, 009259C4 copy
inline constexpr std::size_t kUnitLayoutOffDeferredA = 0x0b4;      // 009259A0
inline constexpr std::size_t kUnitLayoutOffDeferredB = 0x0b8;      // 0092598E
inline constexpr std::size_t kUnitLayoutOffAttachedFlag = 0x0bc;   // 009258F6 reads, 009259EE sets
inline constexpr std::size_t kUnitLayoutOffPropertyBag = 0x0c0;    // docs/SCENE_ENTITY_CREATE.md
inline constexpr std::size_t kUnitLayoutOffAttachClearedByte = 0x10c; // 009259D4
inline constexpr std::size_t kUnitLayoutOffNameCapacity = 0x15c;   // 00925CE0
inline constexpr std::size_t kUnitLayoutOffEntityId = 0x174;       // 009286ED, a u16
inline constexpr std::size_t kUnitLayoutOffRoleSlot = 0x180;       // 00928707, value 9
inline constexpr std::size_t kUnitLayoutOffTickNode = 0x310;       // 0087B6A9
inline constexpr std::size_t kUnitLayoutOffTickGroup = 0x324;      // node +14h, 1 after 0081ED40
inline constexpr std::size_t kUnitLayoutOffTickPayload = 0x338;    // node +28h, the instance
inline constexpr std::size_t kUnitLayoutOffOrderRing = 0x838;      // 0081EE95, BSP_UnitOrderRing_Construct
inline constexpr std::size_t kUnitLayoutOffSlotCounter = 0x109c;   // 0081F1A8

// The 4x4 local frame at +74h: sixteen floats, 004134F0 copies the whole block.
inline constexpr std::size_t kUnitLayoutLocalMatrixFloats = 16;

// The fixed-step group index 0081ED40 leaves at +324h. 00875890 constructs the
// node with group 0 (docs/FIXED_STEP_JOB_WAVES.md); 0081ED40 then overwrites it.
inline constexpr int kUnitTickGroupIndex = 1;

// The dword the game-entity level parks at +180h and passes to vtable[148h].
inline constexpr int kUnitRoleSlotValue = 9;
// Companion literal in the same vtable[148h] call at 009286F4. 0077F360 was not
// read, so this is a transcribed argument, not an interpreted one.
inline constexpr int kUnitRoleSlotMask = 0x1ff;

// The nine dwords 00928630 fills with 8 at +1ACh..+1CCh.
inline constexpr std::size_t kUnitLayoutOffRoleTable = 0x1ac;
inline constexpr int kUnitRoleTableEntries = 9;
inline constexpr int kUnitRoleTableFill = 8;

// The two u16 id registries 00928630 chooses between at 009286BC..009286D8.
// 0077EED0 passes 1, so a unit instance always takes the first.
inline constexpr std::uint32_t kUnitEntityIdRegistryPrimary = 0x00f89a08;
inline constexpr std::uint32_t kUnitEntityIdRegistryAlternate = 0x00f89a5c;
std::uint32_t unit_entity_id_registry(int level_argument) noexcept;

// ---------------------------------------------------------------------------
// The construction-time global slot counter
// ---------------------------------------------------------------------------

// 0081F1A2..0081F1C3: the instance stores the current byte at 00F87151 into
// +109Ch, then the global is incremented and reset to 0 once it exceeds 11.
// Twelve consecutive instances therefore take the values 0..11 in order.
inline constexpr int kUnitSlotCounterLimit = 11;

struct UnitSlotCounterStep {
    int stored;    // what this instance receives at +109Ch
    int next;      // what 00F87151 holds afterwards
};
UnitSlotCounterStep unit_slot_counter_step(int global_counter) noexcept;

// ---------------------------------------------------------------------------
// Placement: vtable[98h] -> 006DFE40 -> 00928860 -> 009258F0
// ---------------------------------------------------------------------------

// The six intrusive lists of the world node that registration pushes onto.
// The first is 00928560's; the other five are 006FE620's, in call order.
inline constexpr int kUnitParentListCount = 6;
inline constexpr std::size_t kUnitParentListOffsets[kUnitParentListCount] = {
    0x24, 0x30, 0x48, 0x54, 0x60, 0x6c,
};

// The detach mirrors the registration list for list: 00928570 takes +24h,
// 006D3620 takes +30h and +48h, 006DFFC0 takes +54h and +60h, and 006FE670
// itself takes +6Ch. Lists are {count +0h, head +4h, tail +8h} and nodes are
// {prev +0h, next +4h, value +8h}; each remover scans the head and erases the
// node whose +8h is the instance through 004837D0.
inline constexpr std::size_t kUnitParentListDetachedLast = 0x6c;

// Vtable slots of 00CFC3D0 used on the placement path.
inline constexpr std::size_t kUnitVtableSlotPlace = 0x098;         // 006DFE40 -> 00928860
inline constexpr std::size_t kUnitVtableSlotEnteredParent = 0x130; // 006FE620
inline constexpr std::size_t kUnitVtableSlotLeavingParent = 0x134; // 006FE670
inline constexpr std::size_t kUnitVtableSlotRoleInit = 0x148;      // 0077F360, not read

// What 00928860 decides before it calls anything, from the single SETNZ at
// 009288AA whose result is reused at 009288DC.
struct UnitPlacementDecision {
    bool node_changed{false};   // new world node differs from [instance+30h]
    bool leave_old_parent{false}; // node_changed and the old node was non-null
    bool enter_new_parent{false}; // node_changed and the new node is non-null
};
UnitPlacementDecision unit_placement_decision(std::uint32_t current_node,
                                              std::uint32_t new_node) noexcept;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One virtual per native call site on the creation path. Nothing here has a
// default implementation: none of these is a stand-in for game behaviour.
struct UnitInstanceCreationHost {
    virtual ~UnitInstanceCreationHost() = default;

    // 006FE5AF: 00BF55BE operator new, cdecl, PUSH size. 0 on failure.
    virtual std::uint32_t allocate(std::size_t size) = 0;
    // 006FE5BE: 00BF79F0 memset(block, 0, size).
    virtual void zero_fill(std::uint32_t block, std::size_t size) = 0;

    // 00925D34: 00925490, the weak-owner sub-object at instance+24h.
    virtual void construct_weak_owner(std::uint32_t sub_object) = 0;
    // 00925DCA and 00925EA0: 004134F0 BSP_Matrix_Copy4x4X87 with a stack
    // identity. Both calls target instance+74h; the second overwrites the first.
    virtual void copy_identity_matrix(std::uint32_t destination) = 0;
    // 00925EA9: 0041DD40 BSP_NativeString_Resize(instance+154h, 0, 0).
    virtual void reset_name_string(std::uint32_t string_object) = 0;
    // 00928695 and 0092887C: 00928240, the registry singleton whose +4h is the
    // CRITICAL_SECTION guarding entity registration.
    virtual std::uint32_t entity_registry() = 0;
    // 009286AE / 00928774 and 00928895 / 009288FC: [00CE2218] and [00CE2210],
    // EnterCriticalSection / LeaveCriticalSection, around a counter at +18h.
    virtual void enter_world_lock(std::uint32_t critical_section) = 0;
    virtual void leave_world_lock(std::uint32_t critical_section) = 0;
    // 009286E4: 009517C0(registry, flag, instance) -> u16.
    virtual std::uint16_t allocate_entity_id(std::uint32_t registry,
                                             std::uint32_t flag,
                                             std::uint32_t instance) = 0;
    // 009286F4: instance->vtable[148h](1FFh, 9). 0077F360 was not read.
    virtual void initialise_role_slot(std::uint32_t instance, int mask, int slot) = 0;
    // 00928760: 00926BE0, this-pointer not resolved.
    virtual void entity_registered_hook(std::uint32_t instance) = 0;
    // 0087B6A9: 00875890 BSP_TickRegistration_Construct(instance+310h,
    // instance, 0). docs/FIXED_STEP_JOB_WAVES.md owns the node.
    virtual void construct_tick_node(std::uint32_t node,
                                     std::uint32_t payload,
                                     int group) = 0;
    // 0081ED7B, 0081EE95, 0081EF47, 0081F03D, 0081F15F and the three vector
    // iterators: the sub-objects of the vehicle base, by sub-object address.
    // Their interiors were not read.
    virtual void construct_base_sub_object(std::uint32_t sub_object,
                                           std::uint32_t constructor) = 0;
    // 0081F200: 00BF681B operator new(2Ch); the block lands at +73Ch.
    virtual std::uint32_t allocate_side_block(std::size_t size) = 0;
    // 006FE5F3: 009553D0, the refcounted setter for +538h.
    virtual void set_vehicle_class(std::uint32_t instance, std::uint32_t descriptor) = 0;
};

// Fields of the instance that this packet can account for after creation.
// Everything else in the 0x1188 bytes is either an unread sub-object or a
// field a later path writes.
struct UnitInstanceCreationResult {
    std::uint32_t instance{0};         // 0 when operator new failed
    std::uint32_t vptrs[kUnitVptrSlotCount]{};
    int class_id{-1};                  // +C4h
    std::uint16_t entity_id{0};        // +174h
    std::uint32_t descriptor_owning{0}; // +538h
    std::uint32_t descriptor_back{0};  // +354h
    int tick_group{0};                 // +324h
    std::uint32_t tick_payload{0};     // +338h
    int slot_counter{-1};              // +109Ch
    int role_slot{0};                  // +180h
    bool zeroed{false};                // the memset ran
};

// 006FE590 BSP_VehicleClassDestroyer_CreateInstance, __thiscall(descriptor,
// int flag), RET 4. Returns the instance; the native code returns the
// allocation unchanged, including the null case, and 009553D0 is then called
// with a null this, which faults. That is reproduced here as a null-instance
// result with the setter still reported to the host.
UnitInstanceCreationResult create_unit_instance(UnitInstanceCreationHost& host,
                                                std::uint32_t descriptor,
                                                std::uint32_t flag,
                                                int global_slot_counter) noexcept;

// One virtual per native call site on the placement path.
struct UnitInstancePlacementHost {
    virtual ~UnitInstancePlacementHost() = default;

    virtual std::uint32_t entity_registry() = 0;              // 0092887C: 00928240
    virtual void enter_world_lock(std::uint32_t cs) = 0;      // 00928895
    virtual void leave_world_lock(std::uint32_t cs) = 0;      // 009288FC
    // 009288C7: instance->vtable[134h], 006FE670 for 00CFC3D0.
    virtual void leaving_parent(std::uint32_t instance) = 0;
    // 009288F1: instance->vtable[130h], 006FE620 for 00CFC3D0.
    virtual void entered_parent(std::uint32_t instance) = 0;
    // 009259B6: 009245A0(instance, deferred_b, deferred_a).
    virtual void flush_deferred(std::uint32_t instance,
                                std::uint32_t deferred_b,
                                std::uint32_t deferred_a) = 0;
    // 009259C4: 004134F0 BSP_Matrix_Copy4x4X87(instance+74h, matrix).
    virtual void copy_local_matrix(std::uint32_t destination,
                                   std::uint32_t matrix) = 0;
    // 009259E2: 0042ED50 on every child reached through +44h.
    virtual void notify_child(std::uint32_t child) = 0;
    // Appends to the list object at `list`. 00484540 BSP_UnitList_PushBack
    // belongs to the cc_unit_lists packet and is a contract here.
    virtual void push_parent_list(std::uint32_t list, std::uint32_t instance) = 0;
};

// The scene-node fields 009258F0 writes, as a projection. `attached` is the
// +BCh gate: when it is already set on entry the routine writes nothing.
struct UnitAttachState {
    std::uint32_t world_node{0};        // +30h
    std::uint32_t hierarchy_parent{0};  // +3Ch
    std::uint32_t deferred_a{0};        // +B4h
    std::uint32_t deferred_b{0};        // +B8h
    bool attached{false};               // +BCh
    bool pose_valid{true};              // +C8h, cleared by the attach
    bool attach_flag_10c{true};         // +10Ch, cleared by the attach
};

// 009258F0, __thiscall(instance, hierarchyParent, worldNode, matrix), RET 0Ch.
// Returns true when the body ran. The list splices themselves are the host's:
// this reports the two link targets it chose.
struct UnitAttachOutcome {
    bool ran{false};
    bool linked_to_hierarchy_parent{false}; // arg1 non-null
    std::uint32_t hierarchy_list_owner{0};  // arg1, or the world node's +8h list
};
UnitAttachOutcome attach_unit_instance(UnitInstancePlacementHost& host,
                                       UnitAttachState& state,
                                       std::uint32_t instance,
                                       std::uint32_t hierarchy_parent,
                                       std::uint32_t world_node,
                                       std::uint32_t matrix) noexcept;

// 00928860, __thiscall(instance, hierarchyParent, worldNode, matrix), RET 0Ch,
// reached through the five-byte thunk 006DFE40 at vtable[98h].
UnitPlacementDecision place_unit_instance(UnitInstancePlacementHost& host,
                                          UnitAttachState& state,
                                          std::uint32_t instance,
                                          std::uint32_t hierarchy_parent,
                                          std::uint32_t world_node,
                                          std::uint32_t matrix) noexcept;

// 006FE620 plus the 00928560 it calls first: six pushes onto the world node's
// lists, in native order.
void register_unit_instance(UnitInstancePlacementHost& host,
                            std::uint32_t instance,
                            std::uint32_t world_node) noexcept;

// ---------------------------------------------------------------------------
// The slot +28h creators
// ---------------------------------------------------------------------------

struct UnitClassCreatorRow {
    const char* type;                 // the VehicleClass.Type string
    std::uint32_t descriptor_vtable;  // MOV [ESI],<vtable> in the descriptor ctor
    std::uint32_t creator;            // the dword at vtable+28h
    std::uint32_t instance_size;      // PUSH <size> at creator+18h, 0 for the stub
    std::uint32_t instance_ctor;      // the call after the memset, 0 when unread
    bool calls_setter_out_of_line;    // false when 009553D0 is inlined
};

inline constexpr int kUnitClassCreatorCount = 22;
extern const UnitClassCreatorRow kUnitClassCreators[kUnitClassCreatorCount];

// True for the eight ship classes whose instance constructor is 0081ED40.
bool unit_class_uses_vehicle_base(std::uint32_t instance_ctor) noexcept;

} // namespace bsp
