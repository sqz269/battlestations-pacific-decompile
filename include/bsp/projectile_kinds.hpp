#pragma once
// The projectile ("bullet") class descriptor and the thirteen projectile kinds.
//
// The descriptor is the object the gun reaches as gun[+3F8h][+34h] and whose
// vtable[20h] the gun calls to spawn a shot (docs/PROJECTILE_IMPACT.md). It is
// built by BSP_BulletClass_GetOrCreate 006EA910 from one row of the Lua Bullets
// table, and its Lua field reader is vtable[18h]. Evidence per claim is in
// docs/WEAPON_CLASS_DESCRIPTOR.md and docs/PROJECTILE_KINDS.md; the native call
// sites the host methods stand for are in reports/projectile_kinds.json.
//
// This is not the gun's own class descriptor. That object is gun[+3F4h], its
// reader is 007327B0, and its +80h weapon-type id and +88h/+8Ch rotation rates
// (docs/GUN_AIMING.md, docs/UNIT_WEAPON_DEVICES.md) belong to it, not here:
// +80h..+9Ch of this descriptor are the two payload-icon atlas rectangles.
#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/projectile_impact.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Descriptor layout. Every offset below is settled by its producer: the base
// constructor 006E8320, the base reader 006E8770, or one of the per-kind
// readers. Offsets that projectile_impact.hpp already declares are reconciled
// with static_asserts at the end of this block rather than renamed.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kWeaponClassOffRefCount = 0x04;      // 006E8320, InterlockedIncrement target
inline constexpr std::size_t kWeaponClassOffSubType = 0x08;       // 006E8320 and every derived constructor
inline constexpr std::size_t kWeaponClassOffBulletId = 0x0C;      // 006EAD86, the Bullets table index
inline constexpr std::size_t kWeaponClassOffName = 0x10;          // "Name", 006E8770
inline constexpr std::size_t kWeaponClassOffComment = 0x18;       // "Comment"
inline constexpr std::size_t kWeaponClassOffNoGravity = 0x20;     // "NoGravity", bool
inline constexpr std::size_t kWeaponClassOffExplWaterEfx = 0x24;  // "ExplWaterEfx"
inline constexpr std::size_t kWeaponClassOffExplArmorEfx = 0x28;  // "ExplArmorEfx"
inline constexpr std::size_t kWeaponClassOffExplAirEfx = 0x2C;    // "ExplAirEfx"
inline constexpr std::size_t kWeaponClassOffExplUnderWaterArmorEfx = 0x30;     // "ExplUnderWaterArmorEfx"
inline constexpr std::size_t kWeaponClassOffExplDeepUnderWaterArmorEfx = 0x34; // "ExplDeepUnderWaterArmorEfx"
inline constexpr std::size_t kWeaponClassOffExplGroundEfx = 0x38; // "ExplGroundEfx"
inline constexpr std::size_t kWeaponClassOffExplLightArmorEfx = 0x3C; // "ExplLightArmorEfx"
inline constexpr std::size_t kWeaponClassOffTravelEfx = 0x40;     // "TravelEfx"
inline constexpr std::size_t kWeaponClassOffTravelEfx2 = 0x44;    // "TravelEfx2"
inline constexpr std::size_t kWeaponClassOffWaterTravelEfx = 0x48; // "WaterTravelEfx"
inline constexpr std::size_t kWeaponClassOffMass = 0x4C;          // "Mass"
inline constexpr std::size_t kWeaponClassOffMuzzleSpeed = 0x50;   // "V0"
inline constexpr std::size_t kWeaponClassOffFlyTime = 0x54;       // "FlyTime"
inline constexpr std::size_t kWeaponClassOffFlakMinRange = 0x58;  // "MinRange", flak reader 0070C030 only
inline constexpr std::size_t kWeaponClassOffTimeScale = 0x5C;     // constructor only, 1.0f; no Lua key
inline constexpr std::size_t kWeaponClassOffRange = 0x68;         // "Range"
inline constexpr std::size_t kWeaponClassOffHasBlast = 0x6C;      // the "Blast" sub-table exists
inline constexpr std::size_t kWeaponClassOffBlastRange = 0x70;    // "Blast.BlastRange"
inline constexpr std::size_t kWeaponClassOffExplWaterHit = 0x74;  // "ExplWaterHit", bool
inline constexpr std::size_t kWeaponClassOffSmallPayloadIcon = 0x78; // "SmallPayloadIcon" texture
inline constexpr std::size_t kWeaponClassOffBigPayloadIcon = 0x7C;   // "BigPayloadIcon" texture
inline constexpr std::size_t kWeaponClassOffSmallIconAtlasUV = 0x80; // four floats
inline constexpr std::size_t kWeaponClassOffBigIconAtlasUV = 0x90;   // four floats
inline constexpr std::size_t kWeaponClassOffMesh = 0xA0;          // "Mesh"
inline constexpr std::size_t kWeaponClassOffDamageMin = 0xAC;     // "DamageMin"
inline constexpr std::size_t kWeaponClassOffDamageMax = 0xB0;     // "DamageMax", defaults to DamageMin
inline constexpr std::size_t kWeaponClassOffBlastDamageMin = 0xB4; // "Blast.BlastDamageMin"
inline constexpr std::size_t kWeaponClassOffBlastDamageMax = 0xB8; // "Blast.BlastDamageMax"
inline constexpr std::size_t kWeaponClassOffWaterDamage = 0xBC;   // "WaterDamage"
inline constexpr std::size_t kWeaponClassOffFireDamage = 0xC0;    // "FireDamage"
inline constexpr std::size_t kWeaponClassOffFireChance = 0xC4;    // "FireChance" / 100
inline constexpr std::size_t kWeaponClassOffLandscapeDecal = 0xCC; // "Decals.Landscape", -1 when absent
inline constexpr std::size_t kWeaponClassOffOtherDecal = 0xD0;    // "Decals.Other", -1 when absent
inline constexpr std::size_t kWeaponClassOffTimeOutEfx = 0xD4;    // "TimeOutEfx", bomb reader 006E1BE0
inline constexpr std::size_t kWeaponClassOffFlakFuseTime = 0xD8;  // read by 0070C3E2; no reader writes it

static_assert(kWeaponClassOffSubType == kProjectileClassOffSubType, "sub-type offset");
static_assert(kWeaponClassOffNoGravity == kProjectileClassOffNoGravity, "gravity flag offset");
static_assert(kWeaponClassOffMuzzleSpeed == kProjectileClassOffMuzzleSpeed, "muzzle speed offset");
static_assert(kWeaponClassOffFlyTime == kProjectileClassOffMaxLife, "fly time offset");
static_assert(kWeaponClassOffTimeScale == kProjectileClassOffTimeScale, "time scale offset");
static_assert(kWeaponClassOffHasBlast == kProjectileClassOffExplosion, "blast flag offset");
static_assert(kWeaponClassOffExplWaterHit == kProjectileClassOffTraceVariant, "water hit flag offset");
static_assert(kWeaponClassOffFlakFuseTime == kProjectileClassOffFuseTime, "flak fuse offset");

// Per-kind reader extensions. Each block starts after the base fields; the
// offsets do not overlap because only one kind's reader writes them.
inline constexpr std::size_t kTorpedoClassOffWaterSplashEfx = 0xD8;   // "WaterSplashEfx"
inline constexpr std::size_t kTorpedoClassOffMaxWaterHitVel = 0xDC;   // "MaxWaterHitVel"
inline constexpr std::size_t kTorpedoClassOffMaxFall = 0xE0;          // "MaxFall"
inline constexpr std::size_t kTorpedoClassOffWaterTravelSpeed = 0xE4; // "WaterTravelSpeed"
inline constexpr std::size_t kTorpedoClassOffHeadingTurn = 0xE8;      // "HeadingTurn"
inline constexpr std::size_t kTorpedoClassOffHomingHorzTurnSpeed = 0xF0; // "HomingHorzTurnSpeed"
inline constexpr std::size_t kTorpedoClassOffHomingVertTurnSpeed = 0xF4; // "HomingVertTurnSpeed"

inline constexpr std::size_t kDepthChargeClassOffV0RandomFactor = 0xD8; // "V0RandomFactor"
inline constexpr std::size_t kDepthChargeClassOffDiveSpeed = 0xDC;      // "DiveSpeed"
inline constexpr std::size_t kDepthChargeClassOffDiveMinDepth = 0xE0;   // "DiveMinDepth", default 1.0f
inline constexpr std::size_t kDepthChargeClassOffDiveMaxDepth = 0xE4;   // "DiveMaxDepth", default 1.0f
inline constexpr std::size_t kDepthChargeClassOffWaterSplashEfx = 0xE8; // "WaterSplashEfx"
inline constexpr std::size_t kDepthChargeClassOffMaxWaterHitVel = 0xEC; // "MaxWaterHitVel"
inline constexpr std::size_t kDepthChargeClassOffMaxFall = 0xF4;        // "MaxFall"

inline constexpr std::size_t kRocketClassOffVMax = 0xD8;         // "VMax"
inline constexpr std::size_t kRocketClassOffAcceleration = 0xDC; // "Acceleration"
inline constexpr std::size_t kRocketClassOffIgnitionDelay = 0xE0; // "IgnitionDelay"
inline constexpr std::size_t kRocketClassOffAntiAir = 0xE4;      // "AntiAir", bool

// Torpedo instance field written by the Lua binding 008A2710.
inline constexpr std::size_t kTorpedoInstanceOffSwimDepth = 0x47C;

// ---------------------------------------------------------------------------
// The thirteen classes. Sub-type values are the constant each constructor
// stores at +8h; they are the enum the gun and the impact path switch on.
// ---------------------------------------------------------------------------
enum class ProjectileKind {
    Bullet = 0,
    ArtilleryBullet,
    Bomb,
    Torpedo,
    DepthCharge,
    DummyTarget,
    DummyKamikazePlane,
    DummySubmarine,
    Paratrooper,
    FlakBullet,
    KamikazePlane,
    Rocket,
    WaterMine,
    Unknown,
};

inline constexpr int kProjectileSubTypeBullet = 0x01;
inline constexpr int kProjectileSubTypeArtillery = 0x04;
inline constexpr int kProjectileSubTypeBomb = 0x09;
inline constexpr int kProjectileSubTypeTorpedo = 0x0A;
inline constexpr int kProjectileSubTypeDepthCharge = 0x0B;
inline constexpr int kProjectileSubTypeDummyTarget = 0x0C;
inline constexpr int kProjectileSubTypeDummyKamikazePlane = 0x0D;
inline constexpr int kProjectileSubTypeDummySubmarine = 0x0E;
inline constexpr int kProjectileSubTypeParatrooper = 0x0F;
inline constexpr int kProjectileSubTypeFlak = 0x10;
inline constexpr int kProjectileSubTypeKamikaze = 0x11;
inline constexpr int kProjectileSubTypeRocket = 0x12;
inline constexpr int kProjectileSubTypeWaterMine = 0x13;

// The artillery group is the closed interval the launch bias and the ammo
// provider both test (006E85B4, 0073042F). Only 4 has a producer.
inline constexpr int kProjectileSubTypeArtilleryGroupFirst = 0x04;
inline constexpr int kProjectileSubTypeArtilleryGroupLast = 0x07;

// One row of the native class table. The addresses are the recovered ones;
// the names are hypotheses taken from the strings that follow each vtable.
struct ProjectileClassInfo {
    ProjectileKind kind{ProjectileKind::Unknown};
    const char* lua_type{""};        // the "Type" value 006EA910 compares, case-insensitively
    const char* native_name{""};     // the string that follows the vtable in .rdata
    int sub_type{0};                 // +8h
    std::size_t descriptor_size{0};  // the allocation in 006EA910
    std::size_t instance_size{0};    // the allocation in the create, 0 when unread
    std::uint32_t vtable{0};
    std::uint32_t constructor{0};
    std::uint32_t create{0};          // vtable[20h]
    std::uint32_t read_lua_fields{0}; // vtable[18h]
    std::uint32_t tick_element_vtable{0};
    std::uint32_t tick_advance{0};    // slot 8h of the tick element vtable
    std::uint32_t place_pose{0};      // slot 4h of the tick element vtable
};

// The table in 006EA910's order. Size is kProjectileClassCount.
inline constexpr std::size_t kProjectileClassCount = 13;
const ProjectileClassInfo* projectile_class_table() noexcept;

// The 006EA910 name chain. Case-insensitive, null when nothing matches (the
// routine then stores a null class pointer and the gun cannot fire).
const ProjectileClassInfo* projectile_class_for_lua_type(const std::string& type) noexcept;
const ProjectileClassInfo* projectile_class_for_sub_type(int sub_type) noexcept;
ProjectileKind projectile_kind_for_sub_type(int sub_type) noexcept;

// ---------------------------------------------------------------------------
// Sub-type rules. Each is one native branch, with its site in the comment.
// ---------------------------------------------------------------------------
// 006E85B4: the launch velocity's vertical component carries one half-step of
// gravity for the artillery group.
bool projectile_sub_type_in_artillery_group(int sub_type) noexcept;

// 0072FE0x: in world mode 2 these sub-types return from the fire routine
// without spawning. Flak (10h) and kamikaze (11h) are the two exceptions
// inside the range.
bool projectile_sub_type_suppressed_in_world_mode_2(int sub_type) noexcept;

// 0073039D..00730476: the ammo provider id 00521E70 is asked for before the
// shot. 2 is the default ammunition pool.
int projectile_ammo_provider_slot(int sub_type) noexcept;

// 006E85A3..006E85D6. dir is the unit direction; speed is the descriptor's V0.
float projectile_launch_velocity_y(float speed, float dir_y, int sub_type) noexcept;

// 0070C3B1 and 0070C3E2, on the flak tick after the base tick has run.
bool flak_should_self_destruct(float flight_time, float fly_time) noexcept;
bool flak_fuse_is_armed(float flight_time, float fuse_time) noexcept;

// ---------------------------------------------------------------------------
// The descriptor as the loader fills it. Effects, textures and decals are
// contracts: the loader only carries the handle the host returned.
// ---------------------------------------------------------------------------
struct WeaponClassEffects {
    std::uint32_t expl_water{0};
    std::uint32_t expl_armor{0};
    std::uint32_t expl_air{0};
    std::uint32_t expl_under_water_armor{0};
    std::uint32_t expl_deep_under_water_armor{0};
    std::uint32_t expl_ground{0};
    std::uint32_t expl_light_armor{0};
    std::uint32_t travel{0};
    std::uint32_t travel2{0};
    std::uint32_t water_travel{0};
    std::uint32_t time_out{0};     // bomb family only
    std::uint32_t water_splash{0}; // torpedo and depth charge only
};

struct WeaponClassDescriptor {
    int sub_type{kProjectileSubTypeBullet};
    int bullet_id{0};
    std::string name;
    std::string comment;
    std::string mesh;
    bool no_gravity{false};
    bool expl_water_hit{false};
    bool has_blast{false};
    float mass{0.0f};
    float muzzle_speed{0.0f};
    float fly_time{1.0f};
    float time_scale{1.0f};
    float range{0.0f};
    float blast_range{0.0f};
    float damage_min{0.0f};
    float damage_max{0.0f};
    float blast_damage_min{0.0f};
    float blast_damage_max{0.0f};
    float water_damage{0.0f};
    float fire_damage{0.0f};
    float fire_chance{0.0f};
    int landscape_decal{-1};
    int other_decal{-1};
    std::uint32_t small_payload_icon{0};
    std::uint32_t big_payload_icon{0};
    WeaponClassEffects effects;

    // Torpedo, depth charge and rocket extensions. Only the reader for that
    // kind writes its block; the rest stay at their constructor defaults.
    float max_water_hit_vel{0.0f};
    float max_fall{0.0f};
    float water_travel_speed{0.0f};
    float heading_turn{0.0f};
    float homing_horz_turn_speed{0.0f};
    float homing_vert_turn_speed{0.0f};
    float v0_random_factor{0.0f};
    float dive_speed{0.0f};
    float dive_min_depth{1.0f};
    float dive_max_depth{1.0f};
    float v_max{0.0f};
    float acceleration{0.0f};
    float ignition_delay{0.0f};
    bool anti_air{false};
    float flak_min_range{0.0f};
};

// 006E8320: the constructor's defaults, before any Lua key is read.
WeaponClassDescriptor weapon_class_constructed_defaults() noexcept;

// One Lua row, injected. Each method is one BSP_LuaObject_GetByName plus the
// typed accessor that follows it; a missing key yields the fallback, which is
// what BSP_LuaReference_Get*OrDefault does.
struct WeaponClassRowView {
    virtual ~WeaponClassRowView() = default;
    virtual bool has_key(const char* key) const = 0;
    virtual std::string string_value(const char* key, const char* fallback) const = 0;
    virtual float float_value(const char* key, float fallback) const = 0;
    virtual int integer_value(const char* key, int fallback) const = 0;
    virtual bool boolean_value(const char* key, bool fallback) const = 0;
    // "Blast" and "Decals" are sub-tables; null when the key is nil.
    virtual const WeaponClassRowView* sub_table(const char* key) const = 0;
};

// The three resolvers the reader calls out to. None of them is modelled.
struct WeaponClassLoadHost {
    virtual ~WeaponClassLoadHost() = default;
    // The effect-id to effect-reference lookup repeated for every Expl*/Travel*
    // key (006E8770, the block at 006E87F1 and its copies).
    virtual std::uint32_t acquire_effect(int effect_id) = 0;
    // BSP_GUI_ResolveTextureAndAtlasUV at 006E8AFF and 006E8B4E. atlas_uv
    // receives the four floats the call writes.
    virtual std::uint32_t resolve_payload_icon(const std::string& path, float atlas_uv[4]) = 0;
    // The decal-name to decal-id lookup under "Decals" (006E8C6D, 006E8CD9).
    virtual int resolve_decal(const std::string& name) = 0;
};

// 006E8770, the base reader, in call order.
void load_weapon_class_base_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                                   WeaponClassDescriptor& desc);
// 006E1BE0: the base reader then "TimeOutEfx".
void load_bomb_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                            WeaponClassDescriptor& desc);
// 008566B0, 006FD400, 00809CB0, 0070C030. Each chains its parent first.
void load_torpedo_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                               WeaponClassDescriptor& desc);
void load_depth_charge_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                                    WeaponClassDescriptor& desc);
void load_rocket_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                              WeaponClassDescriptor& desc);
void load_flak_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                            WeaponClassDescriptor& desc);
// Dispatch on the kind, which is what vtable[18h] does.
void load_weapon_class_fields(ProjectileKind kind, const WeaponClassRowView& row,
                              WeaponClassLoadHost& host, WeaponClassDescriptor& desc);

// ---------------------------------------------------------------------------
// 006E8430 BSP_ProjectileClass_CreateProjectile. Seven stack arguments, RET 1Ch.
// Arguments 1 and 7 are pushed by 0072BF10 and never read by the body.
// ---------------------------------------------------------------------------
struct ProjectileCreateArgs {
    int unused_arg1{0};          // pushed 0 at 0072C004
    int owner_id{0};             // [gun[+3F0h]+54h], stored at +54h
    float position[3]{};         // stored at +A4h, +FCh, +1D0h and +1DCh
    float direction[3]{};        // unit direction, scaled by the descriptor's V0
    bool spawn_flag{false};      // argument 5, stored at +278h
    bool near_camera{false};     // argument 6, stored at +1CCh
    int unused_arg7{0};          // gun[+41Ch], pushed at 0072BFE3
};

// What the body writes into the 284h-byte record, in the order it writes it.
struct ProjectileSpawnState {
    int owner_id{0};
    float position[3]{};
    float velocity[3]{};
    bool underwater{false};  // the water-height test at 006E8663
    bool spawn_flag{false};
    bool near_camera{false};
    bool spawn_hook_called{false}; // shot->vtable[3Ch] on the spawn flag
};

// One virtual per native call site inside 006E8430.
struct ProjectileCreateHost {
    virtual ~ProjectileCreateHost() = default;
    // operator_new(size) then memset 0 (006E8452, 006E8462). Null on failure,
    // which the body tolerates: it then constructs at address zero.
    virtual void* allocate_zeroed(std::size_t size) = 0;
    // 006E7B00 BSP_ProjectileTickableEntity_Construct at 006E8478.
    virtual void* construct_tickable_entity(void* storage) = 0;
    // 00923870 at 006E8511, with the scene registry [00E188A8+19CCh] and a
    // scale-1 matrix built on the stack.
    virtual void register_in_world(void* projectile) = 0;
    // 0085DC80 at 006E8625 and the child-node walk that follows it.
    virtual void refresh_child_poses(void* projectile) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld at 006E8659.
    virtual void refresh_world_pose(void* projectile) = 0;
    // 0078CF20 BSP_GameWorld_SampleWaterHeight at 006E8680.
    virtual float sample_water_height(float x, float z) = 0;
    // The shot interface's vtable[28h] 006E6450 and vtable[24h] 006E6410.
    virtual void set_water_mode(void* shot) = 0;
    virtual void set_air_mode(void* shot) = 0;
    // The shot interface's vtable[3Ch] 006E6C20 at 006E86CD.
    virtual void on_spawn_flag(void* shot) = 0;
};

// The body of 006E8430 as a sequence. instance_size and shot_offset come from
// the class row: 284h/170h for MBullet, 298h/170h for MFlakBullet, and the
// bomb family's larger records with the shot interface at 310h.
ProjectileSpawnState create_projectile_006e8430(const ProjectileClassInfo& info,
                                                const WeaponClassDescriptor& desc,
                                                const ProjectileCreateArgs& args,
                                                ProjectileCreateHost& host);

} // namespace bsp
