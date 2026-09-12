#pragma once
// Weapon devices ("guns") on a unit instance, and the thirteen mission Lua
// bindings of the table 00E0B7B8 that read and drive them.
//
// Addresses: 008BE440 GunForceFire, 008BE600 GunForceFireWithAngle,
// 008BE7E0 HasFired, 008A6950 UnitFreeFire, 008A6AC0 UnitHoldFire,
// 008A6490 UnitSetFireStance, 0089C590 ArtilleryEnable, 0089C8F0 TorpedoEnable,
// 0089EEE0 ShipSetTorpedoStock, 008BED80 GetShipTorpedoes, 0089A8B0 SetFireTarget,
// 0089C360 GetFireTarget, 0088DC10 GetFirepower, 008CF350 GetGun,
// 00730160 gun Fire (vtable+1D8h), 0072CF00 set barrel reload timer,
// 0072D520 re-arm one barrel, 0072E6D0 gun setup from descriptor,
// 0071BE80 set stance, 0071BED0 hold fire, 0071BF20 free fire,
// 0071DFD0 artillery enable message, 0071E0D0 torpedo enable message,
// 0081F8B0 set torpedo stock, 0080E150 unit->weapon director,
// 007298D0 next ready barrel.
//
// Evidence: docs/UNIT_WEAPON_DEVICES.md. Every descriptive name here is a
// hypothesis, not a recovered symbol.
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Where the weapon devices live on a unit instance
// ---------------------------------------------------------------------------
// Guns are scene-graph nodes parented under the unit, not entries of a flat
// array. 008CF350 (GetGun) seeds a worklist from a fixed five-slot pointer
// array on the unit and walks each node's child chain; 0081F8B0 walks only the
// unit's direct children. Both test each node with the class virtual +5Ch.
inline constexpr std::size_t kUnitOffDeviceRootCount = 0x3cc; // int, 008CF35E-ish read at piVar2[0F3h]
inline constexpr std::size_t kUnitOffDeviceRootArray = 0x3d0; // up to five node pointers
inline constexpr int kUnitDeviceRootArrayCapacity = 5;        // clamp at 008CF350's `index < 5`
inline constexpr std::size_t kUnitOffWeaponDirector = 0x738;  // 0080E150: `mov eax,[ecx+738h]; ret`
inline constexpr std::size_t kUnitOffTorpedoStockSpare = 0x104c; // 0081F8B0 writes it

// Scene-graph node links used by both walks (008CF350, 0081F8B0).
inline constexpr std::size_t kNodeOffFirstChild = 0x48;
inline constexpr std::size_t kNodeOffNextSibling = 0x44;

// Arguments seen at the class-test virtual +5Ch. Only these three are read by
// this packet; the predicate body itself is a contract.
inline constexpr int kClassTestUnit = 0x18; // 008CF350 gate before seeding the walk
inline constexpr int kClassTestGun = 0x20;  // 008CF350 and 0081F8B0 select guns
inline constexpr int kClassTestGunMuzzleQuery = 0x24; // 00730259 inside Fire

// ---------------------------------------------------------------------------
// The gun device record
// ---------------------------------------------------------------------------
// Offsets whose names come from the class's own debug-dump routine 0072ADC0,
// which prints each field next to a literal name string in .rdata.
inline constexpr std::size_t kGunOffBarrelRecords = 0x3d4;   // -> array, stride 14h
inline constexpr std::size_t kGunOffBarrelNodeVector = 0x3e4; // per-barrel attached objects
inline constexpr std::size_t kGunOffAmmoProvider = 0x3f0;
inline constexpr std::size_t kGunOffWeaponClass = 0x3f4;     // descriptor; +80h is the type id
inline constexpr std::size_t kGunOffFireParams = 0x3f8;      // +4h spread, +28h/+2Ch reload range, +30h barrel delay
inline constexpr std::size_t kGunOffReloadTimers = 0x414;    // float[barrelNum]
inline constexpr std::size_t kGunOffReloadDurations = 0x418; // float[barrelNum], full-scale copy
inline constexpr std::size_t kGunOffDelayGroupSize = 0x43c;  // "delayGroupSize"
inline constexpr std::size_t kGunOffDelayGroupIndex = 0x440; // "delayGroupIndex"
inline constexpr std::size_t kGunOffDelayGroupCount = 0x444; // "delayGroupCnt"
inline constexpr std::size_t kGunOffBarrelNum = 0x448;       // "barrelNum"
inline constexpr std::size_t kGunOffNextFireBarrel = 0x44c;  // "nextFireBarrel"
inline constexpr std::size_t kGunOffBarrelDelayTime = 0x450; // "barrelDelayTime"
inline constexpr std::size_t kGunOffContFiring = 0x454;      // "contFiring"
inline constexpr std::size_t kGunOffThrowA = 0x400;          // "throwA"
inline constexpr std::size_t kGunOffThrowB = 0x404;          // "throwB"

// The per-barrel record at kGunOffBarrelRecords. Stride is the `add ebp,14h`
// of the dump loop at 0072B051; three of the five dwords are named there.
inline constexpr std::size_t kGunBarrelRecordStride = 0x14;
inline constexpr std::size_t kGunBarrelOffDist = 0x04; // "dist"  (00CFD71C)
inline constexpr std::size_t kGunBarrelOffSpeed = 0x08; // "speed" (00CE6748)
inline constexpr std::size_t kGunBarrelOffDeltaTime = 0x0c; // "DT" (00CFDF48)

// Weapon type ids read from the descriptor at kGunOffWeaponClass + 80h. Only
// the torpedo id is established by this packet: 0081F8B0 re-arms a gun only
// when that dword is 7, and the binding it serves is ShipSetTorpedoStock.
inline constexpr int kWeaponTypeTorpedo = 7;

// The gun class's own vtable. 00CFE0A8 is written by the constructors 0072D950
// and 0072E510; slot 1D8h of it holds 00730160, the Fire method.
inline constexpr std::uint32_t kGunVtableBase = 0x00cfe0a8;
inline constexpr std::size_t kGunVtableSlotFire = 0x1d8;

// Projection of one barrel. Field names follow the dump routine; the two
// unnamed dwords at +0h and +10h are carried so the stride stays honest.
struct GunBarrelRecord {
    std::uint32_t unknown_00{0};
    float dist{0.0f};
    float speed{0.0f};
    float delta_time{0.0f};
    std::uint32_t unknown_10{0};
};

// Projection of the mutable firing state of one gun device. This is the subset
// the thirteen bindings and 00730160 read or write, not the whole 458h+ record.
struct GunDeviceState {
    int barrel_num{0};        // +448h
    int next_fire_barrel{0};  // +44Ch
    int delay_group_size{0};  // +43Ch
    int delay_group_index{0}; // +440h
    int delay_group_count{0}; // +444h
    float barrel_delay_time{0.0f}; // +450h
    float throw_a{0.0f};      // +400h
    float throw_b{0.0f};      // +404h
    int weapon_type{0};       // descriptor +80h
    bool cont_firing{false};  // +454h
};

// ---------------------------------------------------------------------------
// Reload timers
// ---------------------------------------------------------------------------
// 0072D520's no-ammo branch writes the float at 00CFDBF8 into the timer, and
// 0081F8B0 treats a timer at or above the threshold float 00D7A278 as "this
// barrel is empty". 0072CF00 skips its rate divide for values at or above the
// same threshold, so the sentinel is the one value that never decays. The
// literals themselves are data this packet did not read out of the image.
inline constexpr std::uint32_t kGunReloadSentinelAddress = 0x00cfdbf8;
inline constexpr std::uint32_t kGunReloadEmptyThresholdAddress = 0x00d7a278;

// 007298D0: scan forward from `start`, modulo `barrel_num`, for the first
// barrel whose reload timer has run out. Returns `start` when none is ready,
// which is what 00730160 then fires anyway. `timers` must hold `barrel_num`
// entries; a non-positive `barrel_num` yields 0.
int next_ready_barrel_007298d0(const float* timers, int barrel_num, int start) noexcept;

// 0072CF00's write-back rule, without its effect-spawn tail. `scaled_rate` is
// the divisor the caller resolved (1.0f when the gameplay modifier is off);
// `set_full` mirrors the value into the duration array as 0072D520 does when
// it re-arms a barrel after a shot.
struct BarrelReloadWrite {
    float timer{0.0f};
    float duration{0.0f};
    bool wrote_duration{false};
};
BarrelReloadWrite barrel_reload_write_0072cf00(float time, float sentinel_threshold,
                                               float scaled_rate, bool set_full) noexcept;

// ---------------------------------------------------------------------------
// Fire stance
// ---------------------------------------------------------------------------
// 0071BE80 asks the director two questions about the stance and applies the
// answers through two more virtuals. The four values are the constants the
// shipped scripts define in scripts/global/luamw_init.lua, and the table in
// scripts/global/commandhelpers.lua annotates each with its fire/move pair.
enum class FireStance : int {
    HoldFire = 0,   // hold fire, hold move
    FreeFire = 1,   // free fire, hold move  (also STANCE_GUARD)
    FreeAttack = 2, // free fire, free move
    MoveOnly = 3,   // hold fire, free move
};

// The two predicates the director answers at its vtable +24h and +28h. The
// predicate bodies were not read; this is the decoding the script table states,
// and it is the only reading consistent with 0071BED0 passing 0 for hold-fire
// and 0071BF20 passing 1 for free-fire.
bool stance_allows_fire(FireStance stance) noexcept;
bool stance_allows_move(FireStance stance) noexcept;

// ---------------------------------------------------------------------------
// Enable messages
// ---------------------------------------------------------------------------
// ArtilleryEnable and TorpedoEnable never touch the director's own fields:
// both build a session message of base kind 5Ah with a sub-kind dword and route
// it. The only difference between 0071DFD0 and 0071E0D0 is the sub-kind.
inline constexpr int kSessionMessageKindWeaponEnable = 0x5a;
inline constexpr int kWeaponEnableSubKindArtillery = 3; // 0071DFD0
inline constexpr int kWeaponEnableSubKindTorpedo = 5;   // 0071E0D0
inline constexpr int kSessionRouteChannel = 7;          // second argument at both sites

struct WeaponEnableMessage {
    int base_kind{kSessionMessageKindWeaponEnable};
    int sub_kind{0};
    bool enabled{false};
};
WeaponEnableMessage artillery_enable_message_0071dfd0(bool enabled) noexcept;
WeaponEnableMessage torpedo_enable_message_0071e0d0(bool enabled) noexcept;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------
// One virtual per native call site. Nothing here has a default body: none of
// these stands in for unrecovered game behaviour. Opaque native pointers stay
// opaque.
using NativeHandle = std::uintptr_t;

struct UnitWeaponHost {
    virtual ~UnitWeaponHost() = default;

    // -- mission binding argument plumbing (docs/LUA_BINDING_CORE.md) --------
    virtual NativeHandle entity_from_lua_argument(int index) = 0; // 00888AA0
    virtual NativeHandle entity_from_lua_ptr_field(int index) = 0; // 00888D20
    virtual int lua_argument_int(int index) = 0;   // 00B66290
    virtual bool lua_argument_bool(int index) = 0; // 00B66250
    virtual float lua_argument_number(int index) = 0; // 00B66270
    virtual const char* lua_argument_string(int index) = 0; // 00B667xx, HasFired arg 1
    virtual bool lua_argument_is_nil(int index) = 0;  // 00B66xxx via 0089A8B0
    virtual bool lua_argument_is_entity(int index) = 0; // 008889C0
    virtual void lua_argument_vector3(int index, float out[3]) = 0; // 0089A8B0's third branch
    virtual int lua_argument_count() = 0;             // 00B663F0
    virtual void lua_push_boolean(bool value) = 0;    // 00B66xxx
    virtual void lua_push_number(float value) = 0;    // 0088DC10 tail
    virtual void lua_push_nil() = 0;                  // 00B663xx
    virtual void lua_push_entity(NativeHandle entity) = 0; // thisTable[id] + pushvalue
    virtual void lua_new_result_table() = 0;          // 00B66xxx NewTable, 008BED80
    virtual void lua_result_table_set(int key, NativeHandle entity) = 0; // 00B666C0
    virtual int lua_result_count() = 0;               // 00B66400

    // -- unit and device lookup ---------------------------------------------
    virtual bool class_test(NativeHandle object, int class_id) = 0; // vtable +5Ch
    virtual NativeHandle weapon_director(NativeHandle unit) = 0;    // vtable +114h -> 0080E150
    virtual int device_root_count(NativeHandle unit) = 0;           // unit +3CCh
    virtual NativeHandle device_root(NativeHandle unit, int slot) = 0; // unit +3D0h[slot]
    virtual NativeHandle node_first_child(NativeHandle node) = 0;   // node +48h
    virtual NativeHandle node_next_sibling(NativeHandle node) = 0;  // node +44h

    // -- gun device reads ----------------------------------------------------
    virtual int gun_barrel_num(NativeHandle gun) = 0;        // +448h
    virtual int gun_next_fire_barrel(NativeHandle gun) = 0;  // +44Ch
    virtual float gun_reload_timer(NativeHandle gun, int barrel) = 0; // +414h[barrel]
    virtual float gun_throw_a(NativeHandle gun) = 0;         // +400h
    virtual float gun_throw_b(NativeHandle gun) = 0;         // +404h
    virtual int gun_weapon_type(NativeHandle gun) = 0;       // descriptor +80h
    virtual float gun_reload_empty_threshold() = 0;          // 00D7A278

    // -- gun device writes ---------------------------------------------------
    // 0072CF00(gun, barrel, time, set_full).
    virtual void gun_set_reload_timer(NativeHandle gun, int barrel, float time,
                                      bool set_full) = 0;
    // 0072D520(gun, barrel, full): consume a round and re-arm that barrel.
    virtual void gun_rearm_barrel(NativeHandle gun, int barrel, bool full) = 0;
    // Gun vtable +1D8h -> 00730160.
    virtual void gun_fire(NativeHandle gun, int use_explicit_throw, float throw_a,
                          float throw_b) = 0;

    // -- weapon director -----------------------------------------------------
    virtual bool director_stance_allows_fire(NativeHandle director, int stance) = 0; // +24h
    virtual bool director_stance_allows_move(NativeHandle director, int stance) = 0; // +28h
    virtual void director_apply_fire_permission(NativeHandle director, bool allow) = 0; // +40h
    virtual void director_apply_move_permission(NativeHandle director, bool allow) = 0; // +44h
    virtual NativeHandle director_fire_target(NativeHandle director) = 0; // +2Ch
    // 0089A8B0's three branches; the setter bodies are unread by this packet.
    virtual void director_clear_fire_target(NativeHandle director) = 0;
    virtual void director_set_fire_target_entity(NativeHandle director,
                                                 NativeHandle target) = 0;
    virtual void director_set_fire_target_position(NativeHandle director,
                                                   const float position[3]) = 0;
    // HasFired's fallback owner at unit +9D4h, whose director is asked instead.
    virtual NativeHandle unit_secondary_owner(NativeHandle unit) = 0;
    // 006E8250(director, weapon_name, window): HasFired's predicate.
    virtual bool director_has_fired(NativeHandle director, const char* weapon_name,
                                    float window) = 0;

    // -- torpedo stock -------------------------------------------------------
    virtual int torpedo_count(NativeHandle unit) = 0;      // 00810E90
    virtual void torpedo_spawn_one(NativeHandle unit) = 0; // 0081DCB0
    virtual void unit_set_torpedo_spare(NativeHandle unit, int spare) = 0; // +104Ch
    virtual int live_torpedo_count() = 0; // registry at (00E188A8)+19CCh +21Ch
    virtual NativeHandle live_torpedo_at(int index) = 0; // list walk, node +8h
    virtual bool torpedo_is_dead(NativeHandle torpedo) = 0; // +5Eh
    virtual NativeHandle torpedo_owner(NativeHandle torpedo) = 0; // +3BCh

    // -- session and firepower ----------------------------------------------
    virtual void route_weapon_enable(const WeaponEnableMessage& message) = 0; // 00BE-session
    virtual float unit_firepower(NativeHandle unit) = 0; // 0088DC10 tail, body unread
};

// ---------------------------------------------------------------------------
// The bindings, as sequences over the host
// ---------------------------------------------------------------------------
int lua_gun_force_fire_008be440(UnitWeaponHost& host);
int lua_gun_force_fire_with_angle_008be600(UnitWeaponHost& host);
int lua_has_fired_008be7e0(UnitWeaponHost& host);
int lua_unit_free_fire_008a6950(UnitWeaponHost& host);
int lua_unit_hold_fire_008a6ac0(UnitWeaponHost& host);
int lua_unit_set_fire_stance_008a6490(UnitWeaponHost& host);
int lua_artillery_enable_0089c590(UnitWeaponHost& host);
int lua_torpedo_enable_0089c8f0(UnitWeaponHost& host);
int lua_ship_set_torpedo_stock_0089eee0(UnitWeaponHost& host);
int lua_get_ship_torpedoes_008bed80(UnitWeaponHost& host);
int lua_set_fire_target_0089a8b0(UnitWeaponHost& host);
int lua_get_fire_target_0089c360(UnitWeaponHost& host);
int lua_get_firepower_0088dc10(UnitWeaponHost& host);

// 008CF350: the nth gun under `unit`, one-based, or 0 when there are fewer.
NativeHandle find_gun_008cf350(UnitWeaponHost& host, NativeHandle unit, int index);

// 0071BE80 / 0071BED0 / 0071BF20.
void director_set_stance_0071be80(UnitWeaponHost& host, NativeHandle director, int stance);

// 0081F8B0: reconcile the live torpedo count with `stock`, then re-arm every
// empty barrel of every direct-child torpedo gun.
void set_torpedo_stock_0081f8b0(UnitWeaponHost& host, NativeHandle unit, int stock);

} // namespace bsp
