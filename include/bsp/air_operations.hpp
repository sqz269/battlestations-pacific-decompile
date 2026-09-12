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

// State values observed at slot+2Ch. Only these three appear in the routines
// this packet read; the remaining values of the enum are unread.
enum class AirOpsSlotState : std::int32_t {
    kCooldown = 1, // 006C65B0 and 006CD40F leave the slot here with kTimer set
    kLaunching = 2, // LaunchAirBaseSlot refuses to re-launch a slot in this state
    kReady = 5,    // the only state 006CD350 dispatches its launch branch from
};

struct AirOpsSlot {
    std::uint32_t vehicle_class{0};
    std::int32_t assigned_count{0};
    std::int32_t requested_count{0};
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

} // namespace bsp
