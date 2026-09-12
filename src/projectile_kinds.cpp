// Reconstruction of the projectile class descriptor, its Lua field reader and
// the thirteen projectile kinds. Evidence per claim is in
// docs/WEAPON_CLASS_DESCRIPTOR.md and docs/PROJECTILE_KINDS.md; the native call
// sites the host methods stand for are in reports/projectile_kinds.json.
//
// Coverage is not uniform. The class table, the sub-type rules, the base reader
// 006E8770 and the four per-kind reader extensions are complete. 006E8430 is
// modelled for its argument handling, its record writes and the water-entry
// branch; its matrix build and its node walk stay with the host. The per-kind
// ticks are not modelled: only the two flak predicates are, and every other
// tick body is listed as a contract in the doc.
#include "bsp/projectile_kinds.hpp"

#include <cctype>

namespace bsp {
namespace {

constexpr ProjectileClassInfo kClassTable[kProjectileClassCount] = {
    // kind, lua type, native name, sub-type, descriptor size, instance size,
    // vtable, constructor, create, reader, tick vtable, tick, place pose.
    {ProjectileKind::Bullet, "Bullet", "MBullet", kProjectileSubTypeBullet, 0xD4, 0x284,
     0x00cfa138, 0x006e8320, 0x006e8430, 0x006e8770, 0x00cf9d78, 0x006e6490, 0x006e6750},
    {ProjectileKind::ArtilleryBullet, "Artillery", "MArtilleryBullet", kProjectileSubTypeArtillery,
     0xD4, 0x284, 0x00cfa440, 0x006ea1c0, 0x006e8430, 0x006e8770, 0x00cf9d78, 0x006e6490,
     0x006e6750},
    {ProjectileKind::KamikazePlane, "Kamikaze", "MKamikazePlane", kProjectileSubTypeKamikaze, 0xD4,
     0, 0x00cfa478, 0x006ea200, 0x006ea230, 0x006e8770, 0, 0, 0},
    {ProjectileKind::Bomb, "Bomb", "MBomb", kProjectileSubTypeBomb, 0xD8, 0x468, 0x00cfa4b0,
     0x006ea260, 0x006e2c00, 0x006e1be0, 0x00cf93f0, 0x006e1300, 0x006e1060},
    {ProjectileKind::Torpedo, "Torpedo", "MTorpedo", kProjectileSubTypeTorpedo, 0xFC, 0x51c,
     0x00cfa56c, 0x006ea4f0, 0x00856420, 0x008566b0, 0x00d0c35c, 0x006e1300, 0x006e1060},
    {ProjectileKind::DepthCharge, "DepthCharge", "MDepthCharge", kProjectileSubTypeDepthCharge,
     0xF8, 0x474, 0x00cfa508, 0x006ea3a0, 0x006fd210, 0x006fd400, 0x00cfb9f8, 0x006e1300,
     0x006e1060},
    {ProjectileKind::Rocket, "Rocket", "MRocket", kProjectileSubTypeRocket, 0xE8, 0x498,
     0x00cfa4dc, 0x006ea330, 0x0080ade0, 0x00809cb0, 0x00d09060, 0x006e1300, 0x006e1060},
    {ProjectileKind::FlakBullet, "Flak", "MFlakBullet", kProjectileSubTypeFlak, 0xDC, 0x298,
     0x00cfa53c, 0x006ea470, 0x0070cc30, 0x0070c030, 0x00cfd554, 0x0070c370, 0x006e6750},
    {ProjectileKind::WaterMine, "WaterMine", "MWaterMine", kProjectileSubTypeWaterMine, 0xEC, 0,
     0x00cfa59c, 0x006ea5e0, 0x0085f310, 0x0085f500, 0, 0, 0},
    {ProjectileKind::DummyTarget, "DummyTarget", "MDummyTarget", kProjectileSubTypeDummyTarget,
     0xE4, 0, 0x00cfa5cc, 0x006ea6b0, 0x00700be0, 0x00701060, 0, 0, 0},
    {ProjectileKind::Paratrooper, "Paratrooper", "MParatrooper", kProjectileSubTypeParatrooper,
     0x100, 0, 0x00cfa604, 0x006ea720, 0x007ac2c0, 0x007ac780, 0, 0, 0},
    {ProjectileKind::DummyKamikazePlane, "DummyKamikazePlane", "MDummyKamikazePlane",
     kProjectileSubTypeDummyKamikazePlane, 0xE0, 0, 0x00cfa63c, 0x006ea7f0, 0x006fef10, 0x006ff170,
     0, 0, 0},
    {ProjectileKind::DummySubmarine, "DummySubmarine", "MDummySubmarine",
     kProjectileSubTypeDummySubmarine, 0xE0, 0, 0x00cfa678, 0x006ea870, 0x006ffdf0, 0x00700050, 0,
     0, 0},
};

bool equals_insensitive(const std::string& lhs, const char* rhs) noexcept {
    // BSP_NativeString_EqualsCStringInsensitive 00425850.
    std::size_t i = 0;
    for (; i < lhs.size(); ++i) {
        const unsigned char r = static_cast<unsigned char>(rhs[i]);
        if (r == 0) {
            return false;
        }
        const unsigned char l = static_cast<unsigned char>(lhs[i]);
        if (std::tolower(l) != std::tolower(r)) {
            return false;
        }
    }
    return rhs[i] == 0;
}

// 006E87F1 and its copies: read the integer key, ask the host for the effect,
// and keep the previous handle when the key is absent.
std::uint32_t read_effect(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                          const char* key) {
    return host.acquire_effect(row.integer_value(key, 0));
}

} // namespace

const ProjectileClassInfo* projectile_class_table() noexcept { return kClassTable; }

const ProjectileClassInfo* projectile_class_for_lua_type(const std::string& type) noexcept {
    // 006EAAE6..006EACE0: the chain compares "Bullet" first and "WaterMine"
    // last, then stores a null class pointer.
    for (const ProjectileClassInfo& info : kClassTable) {
        if (equals_insensitive(type, info.lua_type)) {
            return &info;
        }
    }
    return nullptr;
}

const ProjectileClassInfo* projectile_class_for_sub_type(int sub_type) noexcept {
    for (const ProjectileClassInfo& info : kClassTable) {
        if (info.sub_type == sub_type) {
            return &info;
        }
    }
    return nullptr;
}

ProjectileKind projectile_kind_for_sub_type(int sub_type) noexcept {
    const ProjectileClassInfo* info = projectile_class_for_sub_type(sub_type);
    return info == nullptr ? ProjectileKind::Unknown : info->kind;
}

bool projectile_sub_type_in_artillery_group(int sub_type) noexcept {
    return sub_type >= kProjectileSubTypeArtilleryGroupFirst &&
           sub_type <= kProjectileSubTypeArtilleryGroupLast;
}

bool projectile_sub_type_suppressed_in_world_mode_2(int sub_type) noexcept {
    // 0072FE0x. The ten values the fire routine returns on: 8..0Fh, 12h, 13h.
    // 10h flak and 11h kamikaze stay live in world mode 2.
    if (sub_type >= 0x08 && sub_type <= 0x0F) {
        return true;
    }
    return sub_type == kProjectileSubTypeRocket || sub_type == kProjectileSubTypeWaterMine;
}

int projectile_ammo_provider_slot(int sub_type) noexcept {
    // 0073039D..00730476, in the order the listing tests them.
    if (sub_type == kProjectileSubTypeTorpedo) {
        return 5;
    }
    if (sub_type == kProjectileSubTypeDepthCharge) {
        return 7;
    }
    if (sub_type == kProjectileSubTypeFlak) {
        return 3;
    }
    if (projectile_sub_type_in_artillery_group(sub_type)) {
        return 4;
    }
    return 2;
}

float projectile_launch_velocity_y(float speed, float dir_y, int sub_type) noexcept {
    float v = speed * dir_y;
    if (projectile_sub_type_in_artillery_group(sub_type)) {
        v -= kProjectileLaunchBiasStep * static_cast<float>(kProjectileGravity);
    }
    return v;
}

bool flak_should_self_destruct(float flight_time, float fly_time) noexcept {
    return fly_time < flight_time; // 0070C3B1
}

bool flak_fuse_is_armed(float flight_time, float fuse_time) noexcept {
    return fuse_time < flight_time; // 0070C3E2
}

WeaponClassDescriptor weapon_class_constructed_defaults() noexcept {
    // 006E8320. Only +4h (1), +8h (1), +54h and +5Ch (1.0f) and the four
    // atlas-rectangle diagonals are non-zero; everything else is zero.
    WeaponClassDescriptor desc;
    desc.sub_type = kProjectileSubTypeBullet;
    desc.fly_time = 1.0f;
    desc.time_scale = 1.0f;
    return desc;
}

void load_weapon_class_base_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                                   WeaponClassDescriptor& desc) {
    // 006E8770, in call order.
    desc.name = row.string_value("Name", "");
    desc.comment = row.string_value("Comment", "");
    desc.mesh = row.string_value("Mesh", "");

    desc.effects.expl_water = read_effect(row, host, "ExplWaterEfx");
    desc.effects.expl_ground = read_effect(row, host, "ExplGroundEfx");
    desc.effects.expl_air = read_effect(row, host, "ExplAirEfx");
    desc.effects.expl_armor = read_effect(row, host, "ExplArmorEfx");
    desc.effects.expl_light_armor = read_effect(row, host, "ExplLightArmorEfx");
    if (desc.effects.expl_light_armor == 0) {
        desc.effects.expl_light_armor = desc.effects.expl_armor; // 006E8A0C
    }
    desc.effects.travel = read_effect(row, host, "TravelEfx");
    desc.effects.travel2 = read_effect(row, host, "TravelEfx2");
    desc.effects.water_travel = read_effect(row, host, "WaterTravelEfx");
    desc.effects.expl_under_water_armor = read_effect(row, host, "ExplUnderWaterArmorEfx");
    if (desc.effects.expl_under_water_armor == 0) {
        desc.effects.expl_under_water_armor = desc.effects.expl_armor; // 006E8A9B
    }
    desc.effects.expl_deep_under_water_armor =
        read_effect(row, host, "ExplDeepUnderWaterArmorEfx");
    if (desc.effects.expl_deep_under_water_armor == 0) {
        desc.effects.expl_deep_under_water_armor = desc.effects.expl_under_water_armor;
    }

    desc.expl_water_hit = row.boolean_value("ExplWaterHit", false);
    desc.no_gravity = row.boolean_value("NoGravity", false);
    desc.fly_time = row.float_value("FlyTime", desc.fly_time);

    float atlas[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const std::string small_icon = row.string_value("SmallPayloadIcon", "");
    desc.small_payload_icon = small_icon.empty() ? 0 : host.resolve_payload_icon(small_icon, atlas);
    const std::string big_icon = row.string_value("BigPayloadIcon", "");
    desc.big_payload_icon = big_icon.empty() ? 0 : host.resolve_payload_icon(big_icon, atlas);

    desc.damage_min = row.float_value("DamageMin", 0.0f);
    desc.damage_max = row.float_value("DamageMax", desc.damage_min);
    desc.water_damage = row.float_value("WaterDamage", 0.0f);
    desc.fire_damage = row.float_value("FireDamage", 0.0f);
    desc.fire_chance = row.float_value("FireChance", 0.0f) / 100.0f; // 006E8C0A
    desc.mass = row.float_value("Mass", 0.0f);
    desc.muzzle_speed = row.float_value("V0", 0.0f);
    desc.range = row.float_value("Range", 0.0f);

    const WeaponClassRowView* blast = row.sub_table("Blast");
    desc.has_blast = blast != nullptr;
    if (blast != nullptr) {
        // BlastRange and BlastDamageMin are read with the plain number
        // accessor, so an absent key leaves the constructor's zero.
        desc.blast_range = blast->float_value("BlastRange", 0.0f);
        desc.blast_damage_min = blast->float_value("BlastDamageMin", 0.0f);
        desc.blast_damage_max = blast->float_value("BlastDamageMax", desc.blast_damage_min);
    }

    const WeaponClassRowView* decals = row.sub_table("Decals");
    if (decals != nullptr) {
        desc.landscape_decal = host.resolve_decal(decals->string_value("Landscape", ""));
        desc.other_decal = host.resolve_decal(decals->string_value("Other", ""));
    } else {
        desc.landscape_decal = -1; // 006E8D0C
        desc.other_decal = -1;
    }
}

void load_bomb_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                            WeaponClassDescriptor& desc) {
    load_weapon_class_base_fields(row, host, desc); // 006E1C02
    desc.effects.time_out = read_effect(row, host, "TimeOutEfx");
}

void load_torpedo_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                               WeaponClassDescriptor& desc) {
    load_bomb_class_fields(row, host, desc); // 008566D3
    desc.max_water_hit_vel = row.float_value("MaxWaterHitVel", 0.0f);
    desc.max_fall = row.float_value("MaxFall", 0.0f);
    desc.water_travel_speed = row.float_value("WaterTravelSpeed", 0.0f);
    desc.heading_turn = row.float_value("HeadingTurn", 0.0f);
    desc.effects.water_splash = read_effect(row, host, "WaterSplashEfx");
    desc.homing_horz_turn_speed = row.float_value("HomingHorzTurnSpeed", 0.0f);
    desc.homing_vert_turn_speed = row.float_value("HomingVertTurnSpeed", 0.0f);
}

void load_depth_charge_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                                    WeaponClassDescriptor& desc) {
    load_bomb_class_fields(row, host, desc); // 006FD422
    desc.max_water_hit_vel = row.float_value("MaxWaterHitVel", 0.0f);
    desc.max_fall = row.float_value("MaxFall", 0.0f);
    desc.v0_random_factor = row.float_value("V0RandomFactor", 0.0f);
    desc.dive_speed = row.float_value("DiveSpeed", 0.0f);
    desc.dive_min_depth = row.float_value("DiveMinDepth", 1.0f);
    desc.dive_max_depth = row.float_value("DiveMaxDepth", 1.0f);
    desc.effects.water_splash = read_effect(row, host, "WaterSplashEfx");
}

void load_rocket_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                              WeaponClassDescriptor& desc) {
    load_bomb_class_fields(row, host, desc); // 00809CD2
    desc.v_max = row.float_value("VMax", 0.0f);
    desc.acceleration = row.float_value("Acceleration", 0.0f);
    desc.ignition_delay = row.float_value("IgnitionDelay", 0.0f);
    desc.anti_air = row.boolean_value("AntiAir", false);
}

void load_flak_class_fields(const WeaponClassRowView& row, WeaponClassLoadHost& host,
                            WeaponClassDescriptor& desc) {
    load_weapon_class_base_fields(row, host, desc); // 0070C051, the base not the bomb
    desc.effects.time_out = 0;                      // +D4h is cleared at 0070C04B
    desc.flak_min_range = static_cast<float>(row.integer_value("MinRange", 0));
}

void load_weapon_class_fields(ProjectileKind kind, const WeaponClassRowView& row,
                              WeaponClassLoadHost& host, WeaponClassDescriptor& desc) {
    switch (kind) {
    case ProjectileKind::Torpedo:
        load_torpedo_class_fields(row, host, desc);
        return;
    case ProjectileKind::DepthCharge:
        load_depth_charge_class_fields(row, host, desc);
        return;
    case ProjectileKind::Rocket:
        load_rocket_class_fields(row, host, desc);
        return;
    case ProjectileKind::FlakBullet:
        load_flak_class_fields(row, host, desc);
        return;
    case ProjectileKind::Bomb:
    case ProjectileKind::WaterMine:
    case ProjectileKind::DummyTarget:
    case ProjectileKind::Paratrooper:
    case ProjectileKind::DummyKamikazePlane:
    case ProjectileKind::DummySubmarine:
        // All six chain 006E1BE0; the four dummy readers add keys this packet
        // did not read, so their extensions are a contract, not a gap here.
        load_bomb_class_fields(row, host, desc);
        return;
    case ProjectileKind::Bullet:
    case ProjectileKind::ArtilleryBullet:
    case ProjectileKind::KamikazePlane:
    case ProjectileKind::Unknown:
        break;
    }
    load_weapon_class_base_fields(row, host, desc);
}

ProjectileSpawnState create_projectile_006e8430(const ProjectileClassInfo& info,
                                                const WeaponClassDescriptor& desc,
                                                const ProjectileCreateArgs& args,
                                                ProjectileCreateHost& host) {
    ProjectileSpawnState state;

    void* storage = host.allocate_zeroed(info.instance_size);
    void* projectile = host.construct_tickable_entity(storage);
    host.register_in_world(projectile);

    state.owner_id = args.owner_id;
    for (int i = 0; i < 3; ++i) {
        state.position[i] = args.position[i];
    }

    // 006E8598..006E85F1: the descriptor's V0 scales the direction, and the
    // vertical component loses one half-step of gravity for the artillery
    // group. Both the physics velocity at +178h and the snapshot at +94h take
    // the same three floats.
    state.velocity[0] = desc.muzzle_speed * args.direction[0];
    state.velocity[1] =
        projectile_launch_velocity_y(desc.muzzle_speed, args.direction[1], desc.sub_type);
    state.velocity[2] = desc.muzzle_speed * args.direction[2];

    host.refresh_child_poses(projectile);
    host.refresh_world_pose(projectile);

    // 006E8680: the water/air mode is decided once, at the muzzle, and holds
    // for the whole flight.
    const float water_height = host.sample_water_height(state.position[0], state.position[2]);
    state.underwater = state.position[1] <= water_height;
    if (state.underwater) {
        host.set_water_mode(projectile);
    } else {
        host.set_air_mode(projectile);
    }

    state.spawn_flag = args.spawn_flag;
    state.near_camera = args.near_camera;
    if (state.spawn_flag) {
        host.on_spawn_flag(projectile);
        state.spawn_hook_called = true;
    }
    return state;
}

} // namespace bsp
