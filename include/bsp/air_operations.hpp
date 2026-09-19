#pragma once
// Airbase and carrier air operations: the plane stock, the launch slots, the
// catapult launch and the landing/rearm rules. docs/AIR_OPERATIONS.md.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The four
// Lua key spellings in kCatapultClassKeys are recovered strings (00D1AC88,
// 00D1AC7C, 00D1AC68, 00CF69AC) and are confirmed by the shipped
// scripts/datatables/autoload/vehicleclasses.lua.
//
// Nothing here is a drop-in binary replacement: the native objects are opaque
// handles behind AirOperationsHost, and the records below are projections of the
// fields this packet read, not full native layouts.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Where the air-operations block lives on the owning entity
// ---------------------------------------------------------------------------
// 006BCD20 selects the block by class test (vtable slot 5Ch, the class-id test of
// docs/ENTITY_CLASS_IDS.md): class 09h MMothership -> owner+1188h, class 45h
// MAirfield -> owner+72Ch. 007F1B70 reaches the same two offsets independently
// (007F1BA7 LEA ECX,[ESI+1188h] after a 5Ch(9) test, 007F1BE7 LEA ECX,[ESI+72Ch]
// after a 5Ch(45h) test), which is the corroboration for both constants.
//
// Caution: docs/UNIT_INSTANCE_SUBOBJECTS.md documents a different +72Ch, the
// owned-ref slot of the *ship* layout. MAirfield is not a ship; the two blocks
// share an offset and nothing else.
inline constexpr std::uint32_t kAirOpsClassIdMothership = 0x09;
inline constexpr std::uint32_t kAirOpsClassIdAirfield = 0x45;
inline constexpr std::size_t kAirOpsBlockOffsetMothership = 0x1188;
inline constexpr std::size_t kAirOpsBlockOffsetAirfield = 0x72C;

// Returns the air-operations block offset for a class id, or 0 when the entity
// is neither a carrier nor an airfield (006BCD20 returns null there).
std::size_t air_ops_block_offset_006bcd20(std::uint32_t class_id) noexcept;

// Second parameter of 006BCD20 (DL). When set, the block is refused unless the
// owner at block+7Ch is present and its byte at +5Dh is clear: the caller wants a
// block that may still act. The +5Dh byte is the owner's "gone" flag and is not
// reconstructed here (contract: unread).
bool air_ops_block_is_usable_006bcd20(bool require_active, bool owner_present,
                                      bool owner_retired_flag) noexcept;

// ---------------------------------------------------------------------------
// The air-operations block: field offsets read by this packet
// ---------------------------------------------------------------------------
struct AirOpsBlockOffsets {
    // Critical section pointer used by every list/slot walk (006CD35F, 006BD403).
    static constexpr std::size_t kLock = 0x0C;
    // std::list<AirOpsStockEntry> object; +44h is _Myhead, +48h the size.
    static constexpr std::size_t kStockList = 0x40;
    static constexpr std::size_t kStockListHead = 0x44;
    // Slot array: pointer, live count, capacity. Records are 58h bytes
    // (006CADD0 grows the array by 2n+2 and strides 58h; the Lua bindings index
    // it as (slot - 1) * 58h, so script slot numbers are 1-based).
    static constexpr std::size_t kSlotArray = 0x4C;
    static constexpr std::size_t kSlotCount = 0x50;
    static constexpr std::size_t kSlotCapacity = 0x54;
    // Plane limit for the whole base, from the scene property bag in 006CADD0
    // and from SetAirBasePlaneLimit 00897720.
    static constexpr std::size_t kPlaneLimit = 0x58;
    // Owning entity, the receiver of every session message and of the stock
    // notification 00984EB0 (006CA883, 006CA899).
    static constexpr std::size_t kOwner = 0x7C;
};
inline constexpr std::size_t kAirOpsSlotStride = 0x58;

// One stock entry, the list node payload built on the stack at 006CA7E8-006CA7F3
// and inserted by 006BFAE0. The list node itself prefixes next/prev, so the
// payload starts at node+8h.
struct AirOpsStockEntryOffsets {
    static constexpr std::size_t kVehicleClass = 0x00; // node+8h
    static constexpr std::size_t kCount = 0x04;        // node+0Ch
    static constexpr std::size_t kInitialFive = 0x08;  // node+10h, literal 5
};

struct AirOpsStockEntry {
    std::uint32_t vehicle_class{0}; // BSP_VehicleClass_GetOrCreate 00964790 result
    std::int32_t count{0};
    // 006CA7F3 stores the literal 5 in the third payload word. No routine this
    // packet read ever loads it back. contract: unread.
    std::int32_t unread_initial_five{5};
};

// ---------------------------------------------------------------------------
// A launch slot (58h bytes)
// ---------------------------------------------------------------------------
struct AirOpsSlotOffsets {
    static constexpr std::size_t kVehicleClass = 0x04;   // 006BC6F0 writes +4h
    static constexpr std::size_t kAssignedCount = 0x08;  // 006BC6F0 writes +8h
    static constexpr std::size_t kRequestedCount = 0x0C; // upper bound, 006CD40B
    static constexpr std::size_t kClassField134 = 0x10;  // copy of class+134h, 006BC6F0
    static constexpr std::size_t kLaunchedSquadron = 0x28; // entity, 006C65C5/00896990
    static constexpr std::size_t kState = 0x2C;
    static constexpr std::size_t kTimer = 0x30;          // float seconds
    static constexpr std::size_t kLaunchRequested = 0x34; // byte, set by 00896750
};

// State values at slot+2Ch. An exhaustive store census of the offset (every MOV
// dword imm/reg, disp8 and disp32, over .text) finds six values written, and the
// tick 006C0510 now closes the cycle, so the enum names what a writer plus a
// reader agree on. Writers, by address. Each one below was shown to address the
// slot array, by the block+4Ch load with the 58h stride or by taking the slot
// itself in ECX:
//   1  006C6603 (the landing release 006C65B0, with the 5.0 timer), 006CD3FC and
//      006CCE3A (leaving state 5 once the launch count is taken), 006CC61C (the
//      state-2 cancel 006CC5C0), 006C57A7 (the landing reassign 006C56D0),
//      006BD3A3 (006BD360)
//   2  006CA66B (006CA640, the queue arm 006CC690 takes at 006CC72C) and
//      006CA91A (006CA8E0, which then calls the launch start itself)
//   3  006C74E0 (the launch start 006C7490), 006CD0EC (006CCDA0's state-4 arm),
//      006C57F5 (006C56D0), 006C776E (006C7680, which takes the slot in ECX; its
//      one caller 006C8800 was not read)
//   4  006CCF9D (006CCDA0, order 2 against a state-3 slot: recall) and 006CD4A9
//      (the same code inlined into 006CD350)
//   5  006C058F, the tick 006C0510, and nowhere else
//   6  006CB28B (the scene loader's `FakeAllocated` slot) and 006C8326 (006C80C0)
// Four further stores to a +2Ch (006BA4E0, 006BC751, 006C4F5C, 006C7C3F) are in
// the census but were not shown to sit on a slot, so they are not claimed here.
// docs/AIROPS_LAUNCH_TICK.md, docs/AIROPS_LAUNCH_START.md,
// docs/AIROPS_LOAD_FROM_SCENE.md.
enum class AirOpsSlotState : std::int32_t {
    // 006C7210 picks a slot in state 1 or 5 to launch from and 006BF230 counts
    // the stock reserved by a slot in state 1 or 5, so both are "this slot holds
    // planes on the deck". kCooldown keeps its name because 006C65B0 and 006CD40F
    // arrive here with the 5.0 timer, but the timer is an accumulator, not a
    // countdown, and nothing gates on it leaving state 1.
    kCooldown = 1,
    kLaunching = 2, // LaunchAirBaseSlot refuses to re-launch a slot in this state
    kReady = 5,    // the only state 006CD350 dispatches its launch branch from
    // 006C74E0 writes it and 006C0510 reads it: a slot whose squadron is away.
    // It leaves only when slot+28h is zero again, which is what makes the tick
    // and the landing release 006C65B0 the two ends of one cycle.
    kLaunched = 3,
    // 006CCF9D writes it after issuing the recall command to the squadron, and
    // 006C0510 treats it exactly as it treats 3.
    kRecalled = 4,
};

struct AirOpsSlot {
    std::uint32_t vehicle_class{0};
    std::int32_t assigned_count{0};
    std::int32_t requested_count{0};
    // slot+10h, the copy of class+134h that 006BC6F0 writes. Four independent
    // spellings of the one field: the scene authors it as `Arm` (006CB213), the
    // Lua reader publishes it as `equipment` (006C6811 names the key, 006C681F
    // loads the field), the squadron property bag carries it as `Equipment`
    // (006C5050), and the class holds its default at +134h.
    // docs/MISSION_LUA_GETPROPERTY.md, docs/AIROPS_LOAD_FROM_SCENE.md,
    // docs/AIROPS_LAUNCH_START.md.
    std::int32_t class_field_134{0};
    std::uint32_t launched_squadron{0};
    AirOpsSlotState state{AirOpsSlotState::kCooldown};
    float timer{0.0F};
    bool launch_requested{false};
};

// 00CE3850, the float stored into slot+30h by 006C65B0 (006C65D5) and by the
// launch branch of 006CD350. Bytes 00 00 A0 40.
inline constexpr float kAirOpsSlotCooldownSeconds = 5.0F;

// ---------------------------------------------------------------------------
// The stock rules
// ---------------------------------------------------------------------------
// 006CA770 AddAirBaseStock: find the entry whose class matches, add to its count,
// or push_back a new {class, count, 5}. Returns the entry's count after the add,
// which is what the replication message and the notification carry.
struct AirOpsStockAddResult {
    std::int32_t entry_count{0}; // the entry's count after the add
    bool created_entry{false};
};
AirOpsStockAddResult air_base_stock_add_006ca770(AirOpsStockEntry* entries, int entry_count,
                                                 int capacity, std::uint32_t vehicle_class,
                                                 std::int32_t added, AirOpsStockEntry* created);

// 006BF100: sum the counts of every entry whose class+70h equals a key. class+70h
// is the grouping key the notification 00984EB0 is keyed by; what it spells is
// not established here (contract: unread).
std::int32_t air_base_stock_count_by_category_006bf100(const AirOpsStockEntry* entries,
                                                       const std::uint32_t* entry_category_keys,
                                                       int entry_count,
                                                       std::uint32_t category_key) noexcept;

// 006BD3F0: planes already committed across every slot. A slot that has launched
// counts its squadron's live plane count (entity+3CCh); a slot that has not
// counts its assigned count.
std::int32_t air_base_committed_planes_006bd3f0(const AirOpsSlot* slots,
                                                const std::int32_t* launched_plane_counts,
                                                int slot_count) noexcept;

// ---------------------------------------------------------------------------
// The launch rule of 006CD350 (the kReady branch, 006CD3A8-006CD434)
// ---------------------------------------------------------------------------
struct AirOpsLaunchInputs {
    std::int32_t plane_limit{0};       // block+58h
    std::int32_t committed_planes{0};  // 006BD3F0
    std::int32_t class_stock{0};       // 006BF330 for the slot's class
    std::int32_t slot_requested{0};    // slot+0Ch
    bool launch_requested{false};      // slot+34h
};

struct AirOpsLaunchDecision {
    std::int32_t launch_count{0};
    bool clear_slot{false}; // nothing can be launched: slot+4h and +10h go to 0
    AirOpsSlotState next_state{AirOpsSlotState::kCooldown};
    float next_timer{0.0F};
};

AirOpsLaunchDecision air_ops_slot_launch_006cd350(const AirOpsLaunchInputs& in) noexcept;

// ---------------------------------------------------------------------------
// MCatapult (class id 28h, vtable 00CFAAB8)
// ---------------------------------------------------------------------------
// Overridden slots relative to the base gun vtable 00CFE0A8, established by
// differencing the two vtables word for word.
inline constexpr std::uint32_t kCatapultVtable = 0x00CFAAB8;
inline constexpr std::uint32_t kCatapultVtableSlotCanFire = 0x1D0;  // 006EBD30
inline constexpr std::uint32_t kCatapultVtableSlotFire = 0x1D8;     // 006EC8E0
inline constexpr std::uint32_t kCatapultVtableSlotDebugDump = 0x0A8; // 006EC140
inline constexpr std::uint32_t kCatapultVtableSlotOnLaunchMessage = 0x1F0; // 006ECB50

// Field names are recovered from the debug dump 006EC140, which prints each with
// its literal key, so these four are not hypotheses.
struct CatapultInstanceOffsets {
    static constexpr std::size_t kDisabled = 0x3B8;   // byte, gate 1 of CanFire
    static constexpr std::size_t kOwner = 0x3F0;      // owning unit
    static constexpr std::size_t kWeaponDesc = 0x3F4; // +E0h is the launch interval
    static constexpr std::size_t kReloadTimerPtr = 0x414; // *ptr > 0 blocks the launch
    static constexpr std::size_t kAmmo = 0x480;       // "ammo"
    static constexpr std::size_t kOriginalAmmo = 0x484; // "orgAmmo"
    static constexpr std::size_t kStartLaunch = 0x488;  // "startLaunch", bool
    static constexpr std::size_t kLaunchTimeSeconds = 0x48C; // set from the message
    static constexpr std::size_t kLaunchTick = 0x490; // "launchTick"
    static constexpr std::size_t kLaunchedRefBlock = 0x494; // observer pair
    static constexpr std::size_t kLaunchedSquadron = 0x4A8; // block+14h
    static constexpr std::size_t kNodeMatrixValid = 0x0C8;  // byte
    static constexpr std::size_t kNodeMatrix = 0x0CC;       // 4x4 float, 40h bytes
};

// Ship-side catapult bookkeeping, on the owning unit rather than on the gun.
struct ShipCatapultOffsets {
    // Ten launch slots of 18h bytes: 550h, 568h, 580h, 598h, 5B0h, 5C8h, 5E0h,
    // 5F8h, 610h, 628h. 00953920 counts the ones whose first word is non-zero.
    static constexpr std::size_t kSlotArray = 0x550;
    static constexpr std::size_t kSlotStride = 0x18;
    static constexpr int kSlotCount = 10;
    static constexpr std::size_t kLastCatapultedIndex = 0x630; // -1 when none
    static constexpr std::size_t kCatapultStock = 0x638;
    // Per-instance launch overrides; -1 means "take the class default".
    static constexpr std::size_t kLaunchedClassOverride = 0x530;
    static constexpr std::size_t kEquipmentOverride = 0x534;
};

// The class-side block is already reconstructed: see VehicleClassFieldOffsets in
// include/bsp/vehicle_class_fields.hpp (kLaunchedClass C0h, kCatapultEquipment
// C4h, kLaunchStock C8h, kMaxLaunchedPlanes CCh). These are the Lua keys the
// reader 00960230 takes them from, inside the "Catapult" sub-table (00CE46F8).
struct CatapultClassKeys {
    const char* launched_class;      // 00D1AC88, default -1
    const char* launch_stock;        // 00D1AC7C, default 0
    const char* max_launched_planes; // 00D1AC68, default 0
    const char* equipment;           // 00CF69AC, default 0
};
inline constexpr CatapultClassKeys kCatapultClassKeys{"LaunchedClass", "LaunchStock",
                                                      "MaxLaunchedPlanes", "Equipment"};

// 009539A0 SetCatapultStock: refuses entirely when the class allows no stock,
// clamps to [0, class LaunchStock].
std::int32_t ship_catapult_stock_clamp_009539a0(std::int32_t current, std::int32_t requested,
                                                std::int32_t class_launch_stock) noexcept;

// 006EBD30, the CanFire override in vtable slot 1D0h.
struct CatapultFireGate {
    bool disabled{false};             // this+3B8h
    std::int32_t ammo{0};             // this+480h
    std::int32_t catapulted_now{0};   // 00953920 over the owner's ten slots
    std::int32_t max_launched{0};     // owner class +CCh
    float reload_timer{0.0F};         // *(this+414h)
};
bool catapult_can_fire_006ebd30(const CatapultFireGate& gate) noexcept;

// 006EC070, run after a launch on both the launching side (006ECB0F) and the
// receiving side (006ECBCB). The barrel timer is the class interval minus the
// time already elapsed since the launch tick the message carried.
inline constexpr std::uint32_t kCatapultSecondsPerTickAddress = 0x00D0DE84;
struct CatapultPostLaunch {
    std::int32_t launch_tick{0};
    float barrel_reload_seconds{0.0F};
    std::int32_t ammo{0};
};
CatapultPostLaunch catapult_after_launch_006ec070(std::int32_t current_tick,
                                                  std::int32_t launch_tick,
                                                  float class_launch_interval,
                                                  float seconds_per_tick,
                                                  std::int32_t ammo_before) noexcept;

// The property bag MCatapult::Fire fills before it calls the squadron creator.
// Keys are recovered strings; the values are what 006EC95D-006ECA56 push.
struct CatapultSpawnProperties {
    std::uint32_t type{0};          // "Type" 00CE4780, the launched class
    std::int32_t wing_count{1};     // "WingCount" 00CF8840, literal 1
    std::int32_t skill{0};          // "Skill" 00CF8838, owner vtable[12Ch]()
    std::int32_t race{0};           // "Race" 00CE8EE0, owner+58h
    std::int32_t party{0};          // "Party" 00CE5804, owner+54h
    std::int32_t owner_player{0};   // "OwnerPlayer" 00CF882C, owner+188h
    std::int32_t state{3};          // "State" 00CF8818, literal 3
    std::int32_t plane_parent_id{0}; // "PlaneParentID" 00CFACC8, owner+174h u16
    std::int32_t equipment{0};      // "Equipment" 00CF69AC
    std::int32_t behaviour{1};      // "Behaviour" 00CFACBC, literal 1
    float resource_usage{0.0F};     // "ResourceUsage" 00CFACAC, launched class +124h
};

// The per-instance override rule both launch inputs of Fire use (006EC90B and
// 006EC923): a negative field means "take the class default".
std::int32_t catapult_launch_input_006ec8ff(std::int32_t instance_override,
                                            std::int32_t class_default) noexcept;

CatapultSpawnProperties catapult_spawn_properties_006ec8e0(std::int32_t launched_class,
                                                           std::int32_t equipment,
                                                           std::int32_t skill, std::int32_t race,
                                                           std::int32_t party,
                                                           std::int32_t owner_player,
                                                           std::uint16_t owner_entity_id,
                                                           std::int32_t launched_class_resource);

// ---------------------------------------------------------------------------
// Recovery and rearm
// ---------------------------------------------------------------------------
// The squadron's planes live in the generic child array the unit layout already
// declares: kUnitOffDeviceRootCount 3CCh and kUnitOffDeviceRootArray 3D0h of
// include/bsp/unit_weapons.hpp (kSquadronPlaneOffset in
// include/bsp/local_player_unit_lists.hpp names the same 3D0h for a squadron).
// 008A2232 bounds the index at 4, so at most five planes are readable.
inline constexpr int kSquadronMaxPlanes = 5;
inline constexpr std::size_t kSquadronDirtyByteOffset = 0x3EC;  // set by 007ED67C, 007F3970
inline constexpr std::size_t kSquadronLastPlaneFlagOffset = 0x36A;
inline constexpr std::size_t kPlaneSquadronBackPointerOffset = 0x9D4;
inline constexpr std::int32_t kSquadronLandKillReason = 5; // 008A223B PUSH 5

// 008A20E0 SquadronLandAndKill walks the planes from the last index down; for
// index > 4 the plane pointer is replaced by zero before the two calls.
struct SquadronLandStep {
    int index{0};
    bool plane_readable{false}; // index <= 4
};
int squadron_land_and_kill_plan_008a20e0(int plane_count, SquadronLandStep* steps,
                                         int max_steps) noexcept;

// 007F3970: remove one plane from the squadron's array, compacting the tail.
// Returns the new plane count, or -1 when the plane does not belong to the
// squadron (its back pointer at +9D4h is null).
std::int32_t squadron_remove_plane_007f3970(std::uint32_t* planes, std::int32_t plane_count,
                                            std::uint32_t plane, bool plane_has_back_pointer,
                                            bool killed_by_landing, bool* out_last_plane_flag);

// ---------------------------------------------------------------------------
// The host: one virtual method per native call site
// ---------------------------------------------------------------------------
struct AirOperationsHost {
    virtual ~AirOperationsHost() = default;

    // 006CA87E: build the stock-change session message from class and count.
    virtual std::uint32_t build_stock_message(std::uint32_t vehicle_class,
                                              std::int32_t count) = 0;
    // 006CA891 -> 0077C7B0: hand a message to every non-local peer.
    virtual void send_message_to_peers(std::uint32_t owner, std::uint32_t message) = 0;
    // 006CA8C0 -> 00984EB0: the "stock" notification, keyed by the class grouping
    // key at class+70h and carrying the category total from 006BF100.
    virtual void notify_stock_changed(std::uint32_t owner, std::uint32_t category_key,
                                      std::int32_t category_total) = 0;

    // 0075B430 with the message id, then 0077C2A0: the two command messages the
    // Lua bindings route rather than acting directly.
    // id 7Bh from 00891F05 (ShipUseCatapult), id 83h from 00896915
    // (LaunchAirBaseSlot).
    virtual void route_command_message(std::int32_t message_id, std::int32_t slot_index,
                                       float value, bool flag) = 0;

    // 00896983 -> 006CDC70: run the air-operations update once so a launch
    // requested from script takes effect before the binding returns.
    virtual void run_air_ops_update(std::uint32_t block) = 0;

    // 006ECA96 -> 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen, the creator the
    // catapult shares with the scene loader. The matrix is the catapult node's
    // world transform copied from this+CCh.
    virtual std::uint32_t create_plane_squadron(std::uint32_t launched_class,
                                                const char* class_name, const float* world_matrix,
                                                const CatapultSpawnProperties& props) = 0;
    // 006ECAC3 -> 00694A60: register the observer pair that keeps this+4A8h in
    // step with the squadron's lifetime.
    virtual void bind_launched_squadron(std::uint32_t catapult, std::uint32_t squadron) = 0;
    // 006ECAE9 -> 00922E20: the 0Ch-byte spawn record stored at squadron+C0h.
    virtual std::uint32_t attach_spawn_record(std::uint32_t squadron,
                                              const CatapultSpawnProperties& props) = 0;
    // 006EC07F -> BSP_Gun_SetBarrelReloadTimer.
    virtual void set_barrel_reload_timer(std::uint32_t catapult, float seconds) = 0;

    // 007ED667 -> 007C1D80 and its vtable[200h] call: rearm one plane's bomb
    // platforms. The walk selects devices that answer the class test with 25h,
    // which covers MBombPlatform 25h and MMultipleBombPlatform 26h.
    virtual void reload_plane_bomb_platforms(std::uint32_t plane) = 0;
    // 007F1BAD and 007F1BED -> 006C65B0: tell one air-operations block that a
    // squadron is gone, for every carrier and airfield in the scene.
    virtual void release_squadron_slot(std::uint32_t air_ops_block,
                                       std::uint32_t squadron) = 0;
    // 008A223F -> 00926D90 BSP_MissionEntity_Kill, reason 5.
    virtual void kill_plane(std::uint32_t plane, std::int32_t reason) = 0;
};

// The two routines as sequences over the host.
void plane_reload_bomb_platforms_0089f080(AirOperationsHost& host, const std::uint32_t* planes,
                                          std::int32_t plane_count, bool* out_dirty);
void squadron_land_and_kill_008a20e0(AirOperationsHost& host, std::uint32_t squadron,
                                     const std::uint32_t* air_ops_blocks, int block_count,
                                     std::uint32_t* planes, std::int32_t* plane_count);

// ---------------------------------------------------------------------------
// 006CADD0 mode 1: the deck as the scene authors it
// ---------------------------------------------------------------------------
// 006CADD0 is a three-mode serializer over the object at its argument: the mode
// is argument+4h and the context argument+8h (006CADF9 tests 1, 006CB2C6 tests 2,
// 006CB894 and 006CC35C test 3). Only mode 1, the scene property-bag load, is
// reconstructed here; docs/AIROPS_LOAD_FROM_SCENE.md says what is known of the
// other two.
//
// Mode 1 reads, in this order:
//   006CAE37  `NumSlots`      the slot count, after which the existing array is
//                             cleared and block+50h zeroed (006CAE7E)
//   006CB0B3  `MaxInAirPlanes`  straight into block+58h (006CB0C9)
//   006CB0D5  `PlaneStock %d` 1-based, each a sub-block of `Type`, `Count` and
//                             `SquadLimit`
//   006CB1B2  `Slot %d`       1-based, each a sub-block of `Type`, `Count`,
//                             `Arm` and the optional boolean `FakeAllocated`
// A sub-block key must answer with property type 6 (006CB0FB, 006CB1E0) and a
// non-null payload, otherwise the loop stops.
// The scene class ids of the two classes that own a deck, from the rows in
// src/scene_entity_factory.cpp. They happen to carry the same numbers as the
// vtable+5Ch class ids above, and they are a different id space: these select a
// scene creator, those answer 006BCD20's class test.
inline constexpr int kAirOpsSceneClassIdMothership = 0x09; // MotherShipGen
inline constexpr int kAirOpsSceneClassIdAirfield = 0x45;   // AirField

struct AirOpsSceneSlot {
    std::string type;       // `Type`, resolved through 007B8A80
    std::int32_t count{0};  // `Count`
    std::int32_t arm{0};    // `Arm`, stored at slot+10h (006CB277)
    bool fake_allocated{false}; // `FakeAllocated`, property type 3, byte at +0Ch
};

struct AirOpsSceneStock {
    std::string type;            // `Type`
    std::int32_t count{0};       // `Count`
    std::int32_t squad_limit{0}; // `SquadLimit`
};

struct AirOpsSceneDeck {
    std::int32_t num_slots{0};
    std::int32_t max_in_air_planes{0};
    std::vector<AirOpsSceneStock> stock;
    std::vector<AirOpsSceneSlot> slots;
};

// The block mode 1 leaves behind. `slots` is block+4Ch with its count at
// block+50h; `max_in_air_planes` is block+58h.
struct AirOpsDeck {
    std::vector<AirOpsSlot> slots;
    std::vector<AirOpsStockEntry> stock;
    std::int32_t max_in_air_planes{0};

    // The fields the launch gates read, from 006BF620 and 006CC690. The two
    // failure flags are named by 006CADD0 mode 3, whose `LEA` descriptors for
    // block+1Ch and block+1Dh sit immediately before the terminators that carry
    // `runwayFailure` (00CBE35) and `hangarFailure` (00CBE77). Neither is a
    // scene key, so both start clear.
    bool runway_failure{false};  // block+1Ch
    bool hangar_failure{false};  // block+1Dh
    // block+38h. CORRECTED: it does have a writer, 006C6589 in the sub-update
    // 006C6540. That routine runs only while block+38h is zero and the list at
    // block+D8h is not empty; it takes the head node's payload at node+8h,
    // unregisters the old observer pair at block+24h, stores the entity at
    // block+38h (the pair's own observed slot is [block+24h]+14h), registers the
    // new pair, and calls 00922F30 on it to enable its scene node (the pushed 0
    // is that routine's second argument, forwarded to vtable[68h]; 00922F4B sets
    // node+5Ch to 1 either way, so this makes the entity visible). So
    // the field is the one entity the deck has pulled out of its queue and is
    // holding, hidden, and the three readers follow: 006BF620 refuses readiness
    // while one is held, 006CC690 queues at 006CC715 instead of starting, and
    // 006C5050 refuses at 006C5078. The name is kept because "a plane is already
    // being spotted" is what the writer shows; what the queue at block+D8h is
    // filled by has not been read. docs/AIROPS_LAUNCH_TICK.md.
    std::uint32_t launch_in_progress{0};
    // block+7Ch is the OWNING ENTITY, named from 006C5050, which reads its
    // virtual at +12Ch for `Skill`, its +54h for `Party`, its +58h, its +188h
    // for `OwnerPlayer` and passes the pointer itself as `HomeBase`. 006BF620
    // requires it non-null and the byte at its +5Dh clear. This process has no
    // such entity object, so `owner_present` is a labelled substitution: a deck
    // the scene loaded reports it present.
    bool owner_present{true};
    bool owner_blocked{false};
    // What 006C5050 reads off the owner to fill the squadron's bag. The scene
    // knows the party and the name; the rest have no authored source this thread
    // has found, so they stay at their defaults and are contracts.
    std::string owner_name;
    std::int32_t owner_party{0};
    std::int32_t owner_skill{0};
    std::int32_t owner_race{0};
    std::int32_t owner_player{0};
    // The entity side of the gate, not the block's: 00895E4B tests the class
    // through vtable+5Ch against 45h and 00895E51 the byte at entity+720h.
    // block+74h with its count at block+78h: the queue 006CC7B0 pushes and
    // 006C58A0 drains, which is the arm a campaign session takes. Each entry is
    // a squadron with the class and count 006C56D0 matches a free slot on.
    struct AssignQueueEntry {
        std::uint32_t squadron{0};
        std::uint32_t vehicle_class{0};  // the squadron's own +35Ch
        std::int32_t plane_count{0};     // its +3CCh
    };
    std::vector<AssignQueueEntry> assign_queue;
    bool is_airfield{false};
    bool airfield_blocked{false};
};

// 00895D20 IsReadyToSendPlanes. The whole rule: an airfield whose entity+720h
// byte is set is never ready, and everything else is 006BF620 over the block.
bool air_ops_is_ready_to_send_planes_00895d20(const AirOpsDeck& deck) noexcept;

// 006C7210: the first slot whose state is 1 or 5, or, when none is, the index
// one past the end, which is where 006CADD0's 2n+2 growth puts a new record.
int air_ops_pick_launch_slot_006c7210(const AirOpsDeck& deck) noexcept;

struct AirOpsLaunchRequest {
    std::uint32_t vehicle_class{0};
    std::int32_t count{0};
    std::int32_t arm{0};
    // 0089E3C0 defaults the arm to class+134h and replaces it only when the
    // binding was given a fourth argument (the 00B663F0 argument-count test
    // against 4).
    bool arm_given{false};
    std::int32_t class_default_arm{0};
};

struct AirOpsLaunchResult {
    int slot_index{-1};  // what 0089E3C0 returns, before its +1
    bool started{false}; // 006C7490 ran
    bool queued{false};  // the stock went back and 006CA640 ran instead
};

// ---------------------------------------------------------------------------
// The squadron 006C5050 builds
// ---------------------------------------------------------------------------
// 006C5050 allocates nothing itself. It fills a scene property bag and hands it
// to 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen, the same creator a
// PlaneSquadronGen scene row uses, which is also how the squadron reaches the
// mission script's entity table: it becomes an ordinary created unit. Every key
// below is a recovered string. docs/AIROPS_LAUNCH_TICK.md.
struct AirOpsSquadronRequest {
    std::uint32_t type{0};          // `Type`, the class's +70h
    std::int32_t wing_count{0};     // `WingCount`, slot+8h
    std::int32_t skill{0};          // `Skill`, the owner's virtual at +12Ch
    std::int32_t race{0};           // the key at 00CE8EE0, the owner's +58h
    std::int32_t party{0};          // `Party`, the owner's +54h
    std::int32_t owner_player{0};   // `OwnerPlayer`, or the owner's +188h when 9
    std::int32_t state{1};          // `State`, 1 normally and 7 on the flag arm
    std::int32_t equipment{0};      // `Equipment`, slot+10h, only when positive
    bool has_auto_attack_target{false};
    std::int32_t auto_attack_target{0}; // `AutoAttackTarget`, the +174h at slot+4Ch
    std::string home_base;          // `HomeBase`, a reference to the owner itself
    int slot_number{0};             // 006C7490 passes the slot index plus one
};

// The seam for the creator. 004F0AD0's counterpart in this process is driven
// from the scene contents pass, so making a squadron appear mid-mission is a
// units-host operation and cannot live here. A deck with no factory creates
// nothing and leaves slot+28h zero, which is what this process did before.
class AirOpsSquadronFactory {
public:
    virtual ~AirOpsSquadronFactory() = default;
    // Returns the new unit's entity id, or 0 when nothing was created.
    virtual std::uint32_t create_squadron(const AirOpsSquadronRequest& request) = 0;
};

// Not a native structure: where this process keeps the factory, if it has one.
void set_air_ops_squadron_factory(AirOpsSquadronFactory* factory) noexcept;
AirOpsSquadronFactory* air_ops_squadron_factory() noexcept;

// 006C7490, the launch start 006CC690 calls. It builds the squadron through
// 006C5050 and puts it in slot+28h, sets the slot to state 3 with a zero timer,
// carries the launch-requested byte into the 5.0 cooldown, and moves the
// observer pair to the new squadron. It does NOT write block+38h.
// docs/AIROPS_LAUNCH_START.md.
void air_ops_launch_start_006c7490(AirOpsDeck& deck, int slot_index) noexcept;

// 006C65B0 BSP_AirOps_ReleaseSquadronSlot, __thiscall(block, squadron). The far
// end of the cycle the launch start opens. It walks every slot without an early
// exit (006C6612 advances by 58h and 006C661B counts block+50h down), and for
// each whose +28h is that squadron it unregisters the observer pair, zeroes +28h
// and +8h, writes state 1, sets the timer to 5.0 and clears the +34h byte.
// Nothing is returned to the stock list. Its own caller is
// 007F1B70 BSP_Squadron_ReleaseFromAllAirBases, which calls it twice.
// Returns the number of slots it released. docs/AIROPS_LAUNCH_TICK.md.
std::size_t air_ops_release_squadron_slot_006c65b0(AirOpsDeck& deck,
                                                   std::uint32_t squadron) noexcept;

// ---------------------------------------------------------------------------
// The two queues a squadron reaches its deck through, and which one runs
// ---------------------------------------------------------------------------
// 007F1C00 BSP_PlaneSquadron_SetHomeAirBase holds the squadron's home base at
// squadron+404h (its observer pair is at squadron+3F0h, observed slot
// [squadron+3F0h]+14h) and then hands it to one of two queues on the block. The
// branch is read from the bytes, because it inverts the obvious guess:
//
//   007f1c4b: CMP dword ptr [ECX + 0x1fe4],0x0
//   007f1c55: JZ  0x007f1c69          ; ZERO -> 006CC7B0, the block+74h queue
//   007f1c57: CALL 0x006cc760         ; non-zero -> the block+C0h spotting queue
//
// So the **spotting** queue, the one whose drain 006C6540 puts an aircraft in
// block+38h and so makes 006BF620 refuse readiness, is the NON-campaign arm.
// A campaign session takes 006CC7B0 into block+74h, which 006C58A0 drains
// before 006CDC70's game-state gate straight into a slot through 006C56D0.
//
// This process asserts a campaign session (`game_non_campaign_flag()` is 0,
// with two more assertions of the same word in src/game_hosts_mission.cpp), so
// **the campaign arm is the only one it can take, and in a campaign the deck has
// no readiness brake at all**: nothing writes block+38h. What paces a campaign
// launch is the mission script's own gate, `stloPlaneNum < 2` for the American
// carriers and `< 4` for the Japanese ones, and that gate only works because the
// tick keeps slot+28h filled. docs/USN04_STRIKE_CLASS.md.
//
// Only the campaign arm is reconstructed below. The spotting arm is left out
// rather than written unreachable.
void air_ops_push_assign_queue_006cc7b0(AirOpsDeck& deck, std::uint32_t squadron,
                                        std::uint32_t vehicle_class,
                                        std::int32_t plane_count);

// 006C56D0's arrival rule, the one 006C58A0 applies to each drained entry.
// Returns the slot it used, or the slot count when none matched.
std::size_t air_ops_arrive_squadron_006c56d0(AirOpsDeck& deck, std::uint32_t squadron,
                                             std::uint32_t vehicle_class,
                                             std::int32_t plane_count) noexcept;

// 006C58A0, sub-update 1 of 006CDC70 and the only one before the game-state
// gate. Returns how many entries it placed.
std::size_t air_ops_drain_assign_queue_006c58a0(AirOpsDeck& deck);

// 006C56D0, the other end: a squadron coming back to the deck. It walks the
// slots for the one whose +28h is that squadron and otherwise takes a free slot,
// where free is state 1 OR state 6 and the class and count match the squadron's
// own +35Ch and +3CCh. It leaves the slot at state 1 with the timer pair, calls
// 006C0F00 to reassign the class and count, and ends at state 3.
//
// That pairing is what gives state 6 a meaning: 006CADD0 mode 1 writes it for a
// `FakeAllocated` slot and 006C56D0 treats it as free, so it is a parked slot
// rather than an active one. Reconstructed for its state rule only; the
// reassignment and the squadron fields it matches on are not modelled.
// docs/AIROPS_LAUNCH_TICK.md.
bool air_ops_slot_is_free_006c56d0(const AirOpsSlot& slot) noexcept;

// 006CC690, which 0089E3C0 delegates to and whose result it pushes plus one.
AirOpsLaunchResult air_ops_launch_squadron_006cc690(AirOpsDeck& deck,
                                                    const AirOpsLaunchRequest& request);

// ---------------------------------------------------------------------------
// The tick: 006C0510 per slot, 006C0DA0 over the deck, 006CDC70 over the block
// ---------------------------------------------------------------------------
// 006BF230 BSP_AirOps_StockAvailable, __thiscall(block, out pair, class). The
// pair is {available, total}: the total sums the stock list at block+44h for
// that class, and the available figure then subtracts, for every slot whose
// class matches AND whose state is 1 or 5 (006BF2A8, 006BF2B1), the squadron's
// +3CCh when slot+28h is set and slot+8h when it is not. 006BF330 returns the
// first word alone. The state test is what stops a launched slot reserving
// stock. docs/AIROPS_LAUNCH_TICK.md.
struct AirOpsStockAvailable {
    std::int32_t available{0};
    std::int32_t total{0};
};
AirOpsStockAvailable air_ops_stock_available_006bf230(
    const AirOpsDeck& deck, std::uint32_t vehicle_class,
    const std::int32_t* launched_plane_counts) noexcept;

struct AirOpsSlotTickResult {
    bool dirty{false};        // 006C0510's own return: 006C0DFB then replicates
    bool became_ready{false}; // the state 3/4 -> 5 refill at 006C058F
    std::int32_t refilled_count{0};
};

// 006C0510, __thiscall(slot, float step). The whole of the slot's own clock and
// the only writer of state 5. In order:
//   006C051A  slot+30h += step. The timer ACCUMULATES; this routine never
//             compares it and there is no cooldown here.
//   006C0525  when slot+34h is set, raise the timer to at least 5.0 (00CE3850,
//             COMISS/JA at 006C0534 is max) and clear the byte.
//   006C0549  slot+28h non-zero -> the slot only tracks the squadron's live
//             plane count at +3CCh into slot+8h and returns.
//   006C054E  otherwise, state 3 or 4 only: zero slot+8h, then take
//             min(block+58h - 006BD3F0, 006BF230(class).available, slot+0Ch),
//             store it and write state 5.
// The zero at 006C055C happens BEFORE 006BD3F0 runs, so the slot's own count is
// not in the committed total it is then measured against; that is why this takes
// the deck rather than a pre-computed input. `launched_plane_counts` is one
// entry per slot, the squadron's +3CCh, and may be null (read as zero).
AirOpsSlotTickResult air_ops_slot_tick_006c0510(
    AirOpsDeck& deck, std::size_t slot_index, float step_seconds,
    const std::int32_t* launched_plane_counts) noexcept;

// The squadron's live plane count, entity+3CCh. This process has no squadron
// object, so whoever creates squadrons installs the reader. A null hook reads
// every squadron as zero planes, which is what an unbound deck did before.
using AirOpsSquadronPlaneCount = std::int32_t (*)(std::uint32_t squadron, void* context);
void set_air_ops_squadron_plane_count(AirOpsSquadronPlaneCount reader, void* context) noexcept;

struct AirOpsDeckTickResult {
    std::size_t slots{0};
    std::size_t dirty{0};
    std::size_t became_ready{0};
    std::size_t tracking{0};   // slots whose +28h holds a squadron
    std::size_t assigned{0};   // entries 006C58A0 placed through 006C56D0
};

// 006C0DA0, __thiscall(block, float step): every slot of block+4Ch in index
// order, and 006C0DFB routes 006BD520's message for each slot the tick returned
// true for. The message is not modelled; the count is kept.
AirOpsDeckTickResult air_ops_deck_update_006c0da0(AirOpsDeck& deck, float step_seconds);

// Not a native structure. 006CDC70 BSP_AirOps_Update is the block's own update
// and runs from the owning unit's motion pass (00758270 for the mother ship,
// 006D2510 for the airfield); this process keeps the blocks in one table, so the
// walk over the table lives here. Only 006C0DA0's sub-update is reconstructed:
// the stock regeneration 006CD240, the ready-plane pull 006C6540, the elevator
// 006C64B0 and the AI launch 006CD810 are not.
AirOpsDeckTickResult air_ops_update_decks_006cdc70(float step_seconds);

// `resolve_type` stands in for 007B8A80, which turns the authored `Type` token
// into the class id the slot carries at +4h. A resolver that returns 0 leaves
// the slot's class unset, which is what an unresolvable token does.
using AirOpsTypeResolver = std::uint32_t (*)(const std::string& type, void* context);
AirOpsDeck air_ops_load_from_scene_006cadd0(const AirOpsSceneDeck& authored,
                                            AirOpsTypeResolver resolve_type, void* context);

// ---------------------------------------------------------------------------
// Where this process keeps the decks it built
// ---------------------------------------------------------------------------
// Not a native structure. The executable hangs the block off the entity; this
// process has no entity object, so the decks live in one process-wide table
// keyed by the unit name the scene authored, with the mission Lua's entity id
// bound to that name once the unit list exists (id is the unit index plus one,
// which is the same convention the objective bindings read back as ID - 1).
class AirOpsDeckRegistry {
public:
    void clear() noexcept;
    void set(const std::string& unit_name, AirOpsDeck deck);
    void bind_entity_id(int entity_id, const std::string& unit_name);
    const AirOpsDeck* find(const std::string& unit_name) const noexcept;
    const AirOpsDeck* find_by_entity_id(int entity_id) const noexcept;
    AirOpsDeck* find_mutable_by_entity_id(int entity_id) noexcept;
    std::size_t size() const noexcept;
    // For the table-wide update; the executable reaches each block from its own
    // unit instead.
    AirOpsDeck* mutable_at(std::size_t index) noexcept;
    const std::string& name_at(std::size_t index) const noexcept;

private:
    std::vector<std::pair<std::string, AirOpsDeck>> decks_;
    std::vector<std::pair<int, std::string>> entity_ids_;
};

AirOpsDeckRegistry& air_ops_decks() noexcept;

} // namespace bsp
