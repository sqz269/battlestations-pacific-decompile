// bsp_game.exe milestone 2t: the gun chain.
//
// See include/bsp/game_hosts_gunnery.hpp for the address list and for what this
// process does not hold. Nothing here reconstructs native code: every step is a
// call of a bsp:: rule or host already on main, or a recorded gap.

#include "bsp/game_hosts_gunnery.hpp"

#include <map>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/gun_gravity_arc.hpp"
#include "bsp/bullet_engagement_range.hpp"
#include "bsp/gun_heading_snap.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/gun_bot_remainder.hpp"
#include "bsp/gun_bot_ticks.hpp"
#include "bsp/hit_narrowphase.hpp"
#include "bsp/kill_credit.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/ship_hit_record.hpp"
#include "bsp/unit_damage.hpp"
#include "bsp/unit_hit_path.hpp"
#include "bsp/unit_weapons.hpp"

namespace bsp::game {
namespace {

constexpr int kMaxPlatformScan = 64;   // the `Platforms` keys this process scans
constexpr int kMaxWindowScan = 8;      // the `Windows` entries per platform
constexpr float kAngleScale = 1000000.0f;
constexpr float kMilliScale = 1000.0f;
constexpr float kHalfPi = 1.57079637050628662109375f;
constexpr float kQuarterPi = 0.785398185253143310546875f;  // 00CEB5A8
constexpr float kGravity = 9.8100004196166992187500f;      // 00CF9058

// 00470470's reset leaves +34h at -1, which is the hull segment a direct
// segment hit keeps: all three known shapes write -1 (docs/HIT_NARROWPHASE.md).
constexpr int kDirectHitHullSegment = -1;

float dot3(const float a[3], const float b[3]) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float length3(const float v[3]) noexcept {
    return std::sqrt(dot3(v, v));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameGunneryHost::Impl {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in, GameMissionLuaHost& lua_in)
        : log(log_in), units(units_in), lua(lua_in) {}

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }

    GameHostLog& log;
    GameUnitsHost& units;
    GameMissionLuaHost& lua;
    GameShipAiHost* ship_ai{nullptr};

    // One entry per created unit, index aligned with GameUnitsHost.
    struct UnitState {
        GameGunneryUnitRow row;
        // The gunnery pass object at unit+6DCh, as its live fields.
        bool attached{false};
        bool enabled{false};             // pass+58h
        float throttle{0.0f};            // pass+6Ch
        bool sweep_suppressed{false};    // pass+59h
        bsp::UnitGunneryCategoryState category{};
        int bridge_countdown{0};         // adapter+8h
        bool allow_fire_cache{false};    // adapter+0Ch
        // The visibility cache at pass+68h: one entry per candidate, 0Ch bytes.
        struct VisibilityEntry {
            std::size_t target{0};
            bool visible{false};
            float ttl{0.0f};
        };
        std::vector<VisibilityEntry> visibility;
        // Per class descriptor, read once out of the authored row.
        float hull_length{0.0f};
        float hull_width{0.0f};
        float hull_height{0.0f};
        float armour{0.0f};
        float max_health{0.0f};
        float health{0.0f};
        // The twelve category records at unit+394h and the ranges at unit+430h.
        std::array<std::vector<std::size_t>, bsp::kUnitGunneryCategoryCount> category_guns{};
        std::array<float, bsp::kUnitGunneryCategoryCount> category_ranges{};
        bool dead{false};
        // The kill attribution block at victim+2C4h..+2E8h, as 0077CE60 leaves it.
        bsp::KillAttributionFields attribution{};
        std::size_t last_attacker{0};   // victim+2C4h, one based
        // The per-step scratch the pass fills.
        std::vector<bsp::GunneryCandidate> candidates;
        std::vector<int> order;
        std::vector<std::size_t> candidate_units;
        std::size_t fire_target{0};      // one based
        std::size_t command_target{0};   // one based
    };

    std::vector<UnitState> unit_state;
    std::vector<GameGunRow> guns;
    std::vector<GameDeviceClassRow> devices;
    std::vector<GameBulletClassRow> bullets;
    std::vector<GameProjectileRow> shots;
    GameGunnerySummary summary{};

    // 00E19BF8, built by 00727BD0 from the authored lists at 00E092C8.
    std::vector<int> rank_table;
    float think_time{bsp::kInstalledWeaponDirectorThinkTime};
    float clock_seconds{0.0f};
    unsigned long long step_index{0};
    // 00BD2F10's stream. One sequence for the whole run so a rerun repeats.
    std::uint32_t rng{0x9E3779B9u};

    float random_range_00bd2f10(float low, float high) {
        rng = rng * 1664525u + 1013904223u;
        const float unit = static_cast<float>((rng >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }

    const GameDeviceClassRow* device(int id) const {
        for (const GameDeviceClassRow& row : devices) {
            if (row.id == id) return &row;
        }
        return nullptr;
    }
    const GameBulletClassRow* bullet(int id) const {
        for (const GameBulletClassRow& row : bullets) {
            if (row.id == id) return &row;
        }
        return nullptr;
    }

    // --- authored data, through the live interpreter ------------------------
    void flatten_class_tables(const std::vector<int>& class_ids);
    int flat(int class_id, const char* key, int fallback) {
        return lua.read_vehicle_class_integer(class_id, "BSPGun", key, fallback);
    }
    float flat_scaled(int class_id, const char* key, float scale, float fallback) {
        const int raw = flat(class_id, key, 0x7FFFFFFF);
        if (raw == 0x7FFFFFFF) return fallback;
        return static_cast<float>(raw) / scale;
    }

    void build_guns();
    void build_rank_table();
    void attach_passes();

    void run_gunnery_pass(std::size_t index, float dt);
    void run_gun_aim_and_fire(float dt);
    void run_projectiles(float dt);
    void apply_hit(std::size_t shooter, std::size_t gun_row, std::size_t victim,
        const float point[3], const float direction[3]);
    void kill_unit(std::size_t victim);

    void unit_pose(std::size_t index, float right[3], float up[3], float forward[3],
        float origin[3]) const {
        if (!units.unit_pose(index, right, up, forward, origin)) {
            for (int i = 0; i < 3; ++i) right[i] = up[i] = forward[i] = origin[i] = 0.0f;
            right[0] = 1.0f;
            up[1] = 1.0f;
            forward[2] = 1.0f;
        }
    }

    void unit_velocity(std::size_t index, float out[3]) const {
        float right[3], up[3], forward[3], origin[3];
        unit_pose(index, right, up, forward, origin);
        const GameUnitRow* row = units.unit_row(index);
        const float speed = row != nullptr ? row->forward_speed : 0.0f;
        for (int i = 0; i < 3; ++i) out[i] = forward[i] * speed;
    }

    // The aim point 00864D90 builds: the target's position raised by the class
    // Height at [target+538h]+0A8h.
    void unit_aim_point(std::size_t index, float out[3]) const {
        float right[3], up[3], forward[3], origin[3];
        unit_pose(index, right, up, forward, origin);
        for (int i = 0; i < 3; ++i) out[i] = origin[i];
        if (index < unit_state.size()) out[1] += unit_state[index].hull_height;
    }
};

// ---------------------------------------------------------------------------
// The authored tables, flattened through the live Lua state
// ---------------------------------------------------------------------------

void GameGunneryHost::Impl::flatten_class_tables(const std::vector<int>& class_ids) {
    // The twelve `Function` spellings 007327B0 compares against, emitted from
    // the reconstruction so the literal list stays on the C++ side and the
    // chunk only performs the compare.
    std::string chunk;
    chunk.reserve(6144);
    chunk += "local F = {";
    for (int i = 0; i < bsp::kUnitGunneryCategoryCount; ++i) {
        const char* name = bsp::gunnery_category_function_name(i);
        chunk += "\"";
        chunk += (name != nullptr ? name : "");
        chunk += "\",";
    }
    chunk += "}\nlocal ids = {";
    for (std::size_t i = 0; i < class_ids.size(); ++i) {
        char number[16];
        std::snprintf(number, sizeof(number), "%d,", class_ids[i]);
        chunk += number;
    }
    chunk +=
        "}\n"
        "local function cat(s)\n"
        "  if type(s) ~= 'string' then return -1 end\n"
        "  local u = string.upper(s)\n"
        "  for i = 1, table.getn(F) do if F[i] == u then return i - 1 end end\n"
        "  return -1\n"
        "end\n"
        "local function num(v, s)\n"
        "  if type(v) ~= 'number' then return nil end\n"
        "  return math.floor(v * s + 0.5)\n"
        "end\n"
        "local think = 2\n"
        "if type(Globals) == 'table' and type(Globals.WeaponSystems) == 'table'\n"
        "   and type(Globals.WeaponSystems.WeaponDirectorThinkTime) == 'number' then\n"
        "  think = Globals.WeaponSystems.WeaponDirectorThinkTime\n"
        "end\n"
        "for _, id in ipairs(ids) do\n"
        "  local row = type(VehicleClass) == 'table' and VehicleClass[id] or nil\n"
        "  if type(row) == 'table' then\n"
        "    local f = {}\n"
        "    f.think = num(think, 1000) or 2000\n"
        "    f.hp = num(row.HP, 1000) or 0\n"
        "    f.armour = num(row.Armour, 1000) or 0\n"
        "    f.length = num(row.Length, 1000) or 0\n"
        "    f.width = num(row.Width, 1000) or 0\n"
        "    f.height = num(row.Height, 1000) or 0\n"
        "    local n = 0\n"
        "    local plats = row.Platforms\n"
        "    if type(plats) == 'table' then\n"
        "      for k = 1, 64 do\n"
        "        local p = plats[k]\n"
        "        if type(p) == 'table' and type(p.Gun) == 'table'\n"
        "           and type(p.Gun[1]) == 'number' then\n"
        "          local dev = type(DeviceClass) == 'table' and DeviceClass[p.Gun[1]] or nil\n"
        "          if type(dev) == 'table' then\n"
        "            n = n + 1\n"
        "            local q = 'p' .. n .. '_'\n"
        "            f[q .. 'key'] = k\n"
        "            f[q .. 'dev'] = p.Gun[1]\n"
        "            f[q .. 'cat'] = cat(dev.Function)\n"
        "            f[q .. 'hrs'] = num(dev.HorzRotSpeed, 1000) or 0\n"
        "            f[q .. 'vrs'] = num(dev.VertRotSpeed, 1000) or 0\n"
        "            local bn = 0\n"
        "            local b1 = nil\n"
        "            if type(dev.Bullet) == 'table' then\n"
        "              for bi = 1, 16 do\n"
        "                if type(dev.Bullet[bi]) == 'table' then\n"
        "                  bn = bn + 1\n"
        "                  if b1 == nil then b1 = dev.Bullet[bi] end\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            f[q .. 'barrels'] = bn\n"
        "            if type(b1) == 'table' then\n"
        "              f[q .. 'bullet'] = num(b1.Bullet, 1) or -1\n"
        // LATENT GUARD. 007313E0 reads ReloadTime as a PAIR into +28h/+2Ch and
        // 00BD2F10 draws between them per shot; when a row authors a scalar the
        // reader writes it to both ends, which is every row in this installation.
        // But num() returns nil for a table, so a paired ReloadTime would leave
        // `reload` at 0 here and the gun would fire every fixed step. Nothing
        // authors a pair today, so this changes no current behaviour.
        // docs/GUN_SHOT_CADENCE.md divergence 9.
        "              local rt1 = b1.ReloadTime\n"
        "              if type(rt1) == 'table' then rt1 = rt1[1] end\n"
        "              f[q .. 'reload'] = num(rt1, 1000) or 0\n"
        "              f[q .. 'bdelay'] = num(b1.BarrelDelayTime, 1000) or 0\n"
        "              f[q .. 'throw'] = num(b1.Throw, 1000000) or 0\n"
        // bulletclasses.lua publishes the arcade or realistic table under the
        // global `Bullets`; `BulletClass` is accepted as well so a differently
        // named installation still reads.
        "              local BT = type(Bullets) == 'table' and Bullets\n"
        "                 or (type(BulletClass) == 'table' and BulletClass or nil)\n"
        "              local bc = BT and BT[b1.Bullet] or nil\n"
        "              if type(bc) == 'table' then\n"
        "                f[q .. 'v0'] = num(bc.V0, 1000) or 0\n"
        "                f[q .. 'range'] = num(bc.Range, 1000) or 0\n"
        "                f[q .. 'dmin'] = num(bc.DamageMin, 1000) or 0\n"
        "                f[q .. 'dmax'] = num(bc.DamageMax, 1000) or 0\n"
        "                f[q .. 'wdmg'] = num(bc.WaterDamage, 1000) or 0\n"
        "                f[q .. 'fdmg'] = num(bc.FireDamage, 1000) or 0\n"
        "                f[q .. 'fchance'] = num(bc.FireChance, 1000) or 0\n"
        "                f[q .. 'mass'] = num(bc.Mass, 1000) or 0\n"
        "                if type(bc.Blast) == 'table' then\n"
        "                  f[q .. 'bdmax'] = num(bc.Blast.BlastDamageMax, 1000) or 0\n"
        "                  f[q .. 'brange'] = num(bc.Blast.BlastRange, 1000) or 0\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            local wn = 0\n"
        "            if type(p.Windows) == 'table' then\n"
        "              for wi = 1, 8 do\n"
        "                local w = p.Windows[wi]\n"
        "                if type(w) == 'table' then\n"
        "                  wn = wn + 1\n"
        "                  local r = q .. 'w' .. wn .. '_'\n"
        "                  f[r .. 'nf'] = (w.Nofire and 1) or 0\n"
        "                  f[r .. 'minh'] = num(w.MinHorzAngle, 1000000) or 0\n"
        "                  f[r .. 'maxh'] = num(w.MaxHorzAngle, 1000000) or 0\n"
        "                  f[r .. 'minv'] = num(w.MinVertAngle, 1000000) or 0\n"
        "                  f[r .. 'maxv'] = num(w.MaxVertAngle, 1000000) or 0\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            f[q .. 'wn'] = wn\n"
        "            if type(p.RestAngles) == 'table' then\n"
        "              f[q .. 'rh'] = num(p.RestAngles[1], 1000000) or 0\n"
        "              f[q .. 'rv'] = num(p.RestAngles[2], 1000000) or 0\n"
        "            end\n"
        "          end\n"
        "        end\n"
        "      end\n"
        "    end\n"
        "    f.n = n\n"
        "    row.BSPGun = f\n"
        "  end\n"
        "end\n";

    const int loaded = lua.luaL_loadbuffer(chunk.c_str(),
        static_cast<int>(chunk.size()), "bsp_gunnery_flatten");
    if (loaded != 0) {
        log.note("gunnery: the authored-table flatten chunk did not compile; no gun exists");
        lua.lua_settop(-2);
        return;
    }
    const int called = lua.lua_pcall(0, 0, 0);
    if (called != 0) {
        log.note("gunnery: the authored-table flatten chunk failed; no gun exists");
        lua.lua_settop(-2);
        return;
    }
    done("Gunnery::vehicle_class_platforms_009610f6", 0x009610f6u);
    done("Gunnery::device_class_function_007327b0", 0x007327b0u);
}

// ---------------------------------------------------------------------------
// 00727BD0 and the per-unit gun lists
// ---------------------------------------------------------------------------

void GameGunneryHost::Impl::build_rank_table() {
    rank_table.assign(static_cast<std::size_t>(bsp::kUnitGunneryCategoryCount)
        * static_cast<std::size_t>(bsp::kUnitGunneryClassIdCount), 0);
    bsp::build_rank_table_00727bd0(bsp::kGunneryPreferenceLists, rank_table.data());
    done("Gunnery::build_target_rank_table_00727bd0", 0x00727bd0u);
}

void GameGunneryHost::Impl::build_guns() {
    const std::size_t count = units.count();
    unit_state.resize(count);

    // The distinct class ids the scene created, for one flatten pass.
    std::vector<int> class_ids;
    for (std::size_t i = 0; i < count; ++i) {
        const GameUnitRow* row = units.unit_row(i);
        if (row == nullptr || row->type_id < 0) continue;
        if (std::find(class_ids.begin(), class_ids.end(), row->type_id) == class_ids.end()) {
            class_ids.push_back(row->type_id);
        }
    }
    flatten_class_tables(class_ids);

    bool think_read = false;
    for (std::size_t i = 0; i < count; ++i) {
        UnitState& state = unit_state[i];
        const GameUnitRow* row = units.unit_row(i);
        state.row.unit_index = i;
        state.row.name = row != nullptr ? row->name : std::string();
        state.row.side = units.unit_side_0054(i);
        state.row.type_id = row != nullptr ? row->type_id : -1;
        state.category = bsp::unit_gunnery_initial_category_state_00864580();
        if (row == nullptr || row->type_id < 0) continue;
        const int type_id = row->type_id;

        if (!think_read) {
            const float value = flat_scaled(type_id, "think", kMilliScale, -1.0f);
            if (value > 0.0f) {
                think_time = value;
                think_read = true;
                done("Gunnery::weapon_director_think_time_0087e16b", 0x0087e16bu);
            }
        }
        state.max_health = flat_scaled(type_id, "hp", kMilliScale, 0.0f);
        state.health = state.max_health;
        state.armour = flat_scaled(type_id, "armour", kMilliScale, 0.0f);
        state.hull_length = flat_scaled(type_id, "length", kMilliScale, 0.0f);
        state.hull_width = flat_scaled(type_id, "width", kMilliScale, 0.0f);
        state.hull_height = flat_scaled(type_id, "height", kMilliScale, 0.0f);
        state.row.health = state.health;
        state.row.max_health = state.max_health;

        const int platforms = flat(type_id, "n", 0);
        for (int p = 1; p <= platforms && p <= kMaxPlatformScan; ++p) {
            char key[32];
            auto make = [&key, p](const char* leaf) {
                std::snprintf(key, sizeof(key), "p%d_%s", p, leaf);
                return key;
            };
            GameGunRow gun;
            gun.unit_index = i;
            gun.unit_name = state.row.name;
            gun.platform_key = flat(type_id, make("key"), 0);
            gun.device_class = flat(type_id, make("dev"), -1);
            gun.category = flat(type_id, make("cat"), -1);
            if (gun.category < 0 || gun.category >= bsp::kUnitGunneryCategoryCount) continue;
            const char* function = bsp::gunnery_category_function_name(gun.category);
            gun.function = function != nullptr ? function : "";
            char label[32];
            std::snprintf(label, sizeof(label), "Platform %d", gun.platform_key);
            gun.platform_name = label;
            gun.speeds.horz = flat_scaled(type_id, make("hrs"), kMilliScale, 0.0f);
            gun.speeds.vert = flat_scaled(type_id, make("vrs"), kMilliScale, 0.0f);
            gun.barrel_num = std::max(1, flat(type_id, make("barrels"), 1));
            gun.bullet_class = flat(type_id, make("bullet"), -1);
            gun.reload_time = flat_scaled(type_id, make("reload"), kMilliScale, 0.0f);
            gun.barrel_delay_time = flat_scaled(type_id, make("bdelay"), kMilliScale, 0.0f);
            gun.muzzle_speed = flat_scaled(type_id, make("v0"), kMilliScale, 0.0f);
            gun.max_range = flat_scaled(type_id, make("range"), kMilliScale, 0.0f);
            // 00731020 answers with descriptor+60h, NOT the authored Lua `Range`:
            // 006E8770 puts `Range` at +68h and never writes +60h. The finalise
            // hook 006E9890 derives +60h from the class sub-type - artillery
            // (4..7) keeps `Range`, the gun group (1,2,3,10h) takes FlyTime * V0,
            // 0Bh takes 240 and everything else 3000 - and MTorpedo overrides it
            // at 00855A90.
            //
            // Only 32 of the 119 bullet classes in this installation author a
            // `Range` at all. The other 87 were reading 0, which collapsed their
            // category to category_engagement_range_00956d63's 10.0f seed and made
            // 00863990 refuse every candidate: on IJN01 that silenced 82 PLANEGUN,
            // 295 AAMACHINEGUN, 60 FLAK, 12 TORPEDO and 10 DEPTHCHARGE guns.
            // docs/BULLET_ENGAGEMENT_RANGE.md.
            //
            // Unauthored fields are left at the struct's defaults, which are the
            // constructor's and the reader's, so filling only what the row
            // authored reproduces native state. `FlyTime` defaults to FLT_MAX.
            if (gun.bullet_class >= 0) {
                bsp::WeaponClassFinaliseInput fin;
                const std::string bullet_type =
                    lua.read_bullet_class_string(gun.bullet_class, "Type");
                fin.sub_type = bsp::weapon_class_sub_type_for_lua_type(bullet_type);
                // docs/ORDNANCE_KIND_IDENTITY.md: the Type string names the
                // projectile descriptor whose vtable[8] the 007ED7E0 family
                // queries, and its answer set is the entity class-id space.
                gun.ordnance = bsp::ordnance_kinds_for_bullet_type(bullet_type.c_str());
                fin.range = lua.read_bullet_class_number(
                    gun.bullet_class, "Range", 0.0f);
                fin.muzzle_speed = lua.read_bullet_class_number(
                    gun.bullet_class, "V0", 0.0f);
                fin.fly_time = lua.read_bullet_class_number(
                    gun.bullet_class, "FlyTime", bsp::kWeaponClassFlyTimeDefault);
                fin.damage_min = lua.read_bullet_class_number(
                    gun.bullet_class, "DamageMin", 0.0f);
                fin.blast_damage_min = lua.read_bullet_class_number(
                    gun.bullet_class, "Blast.BlastDamageMin", 0.0f);
                fin.name = lua.read_bullet_class_string(gun.bullet_class, "Name");
                fin.water_travel_speed = lua.read_bullet_class_number(
                    gun.bullet_class, "WaterTravelSpeed", 0.0f);
                fin.max_fall = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxFall", 0.0f);
                fin.flak_min_range = lua.read_bullet_class_number(
                    gun.bullet_class, "MinRange", 0.0f);
                const bsp::WeaponClassFinaliseResult finalised =
                    bsp::weapon_class_derive_engagement_range(fin);
                if (finalised.engagement_range > 0.0f) {
                    gun.max_range = finalised.engagement_range;
                    ++summary.bullet_ranges_derived;
                }
                if (finalised.swim_speed > 0.0f) {
                    gun.water_travel_speed = fin.water_travel_speed;
                    gun.swim_speed = finalised.swim_speed;
                    ++summary.torpedo_ranges_derived;
                }
            }
            gun.rest_horz = flat_scaled(type_id, make("rh"), kAngleScale, 0.0f);
            gun.rest_vert = flat_scaled(type_id, make("rv"), kAngleScale, 0.0f);

            if (gun.bullet_class >= 0 && bullet(gun.bullet_class) == nullptr) {
                GameBulletClassRow b;
                b.id = gun.bullet_class;
                b.found = true;
                b.muzzle_speed = gun.muzzle_speed;
                b.range = gun.max_range;
                b.water_travel_speed = gun.water_travel_speed;
                b.swim_speed = gun.swim_speed;
                b.damage_min = flat_scaled(type_id, make("dmin"), kMilliScale, 0.0f);
                b.damage_max = flat_scaled(type_id, make("dmax"), kMilliScale, 0.0f);
                b.water_damage = flat_scaled(type_id, make("wdmg"), kMilliScale, 0.0f);
                b.fire_damage = flat_scaled(type_id, make("fdmg"), kMilliScale, 0.0f);
                b.fire_chance = flat_scaled(type_id, make("fchance"), kMilliScale, 0.0f);
                b.mass = flat_scaled(type_id, make("mass"), kMilliScale, 0.0f);
                b.blast_damage_max = flat_scaled(type_id, make("bdmax"), kMilliScale, 0.0f);
                b.blast_range = flat_scaled(type_id, make("brange"), kMilliScale, 0.0f);
                bullets.push_back(b);
            }
            if (gun.device_class >= 0 && device(gun.device_class) == nullptr) {
                GameDeviceClassRow d;
                d.id = gun.device_class;
                d.found = true;
                d.function = gun.function;
                d.category = gun.category;
                d.horz_rot_speed = gun.speeds.horz;
                d.vert_rot_speed = gun.speeds.vert;
                d.bullet_class = gun.bullet_class;
                d.reload_time = gun.reload_time;
                d.barrel_delay_time = gun.barrel_delay_time;
                d.barrel_num = gun.barrel_num;
                devices.push_back(d);
            }

            // 007F5A10 treats the window list as a partition of the circle and
            // dereferences its search result with no null guard, so the list
            // must already hold a covering window before the first authored
            // insert. docs/GUN_PLATFORM_ARC.md records that seed as "somewhere
            // this packet did not find". Its flags are settled by the authored
            // data: `Turret A` of class 265 authors [-145,0] and [0,145] and
            // `Turret D` authors [30,180] and [-180,-30], each leaving the
            // sector its own superstructure blocks uncovered, so an uncovered
            // heading must refuse both tests and the seed carries flags 0.
            const int windows = flat(type_id, make("wn"), 0);
            bsp::GunFiringArc seed;
            seed.flags = 0;
            seed.min_horz = -bsp::kGunArcBoundLimit;
            seed.max_horz = bsp::kGunArcBoundLimit;
            seed.min_vert = -kHalfPi;
            seed.max_vert = kHalfPi;
            if (windows <= 0) {
                // A platform that authors no `Windows` has nothing to carve the
                // seed with, so the seed itself is what the gun traverses and
                // fires through. This is the executable's reading of an absent
                // key, not a recovered default.
                seed.flags = static_cast<std::uint8_t>(bsp::kGunArcFlagTraverse
                    | bsp::kGunArcFlagFire);
            }
            gun.arcs.push_back(seed);
            bool traverses = false;
            for (int w = 1; w <= windows && w <= kMaxWindowScan; ++w) {
                char wkey[40];
                auto wmake = [&wkey, p, w](const char* leaf) {
                    std::snprintf(wkey, sizeof(wkey), "p%d_w%d_%s", p, w, leaf);
                    return wkey;
                };
                bsp::GunFiringArc arc;
                const bool nofire = flat(type_id, wmake("nf"), 0) != 0;
                arc.flags = bsp::kGunArcFlagTraverse;
                if (!nofire) arc.flags = static_cast<std::uint8_t>(
                    arc.flags | bsp::kGunArcFlagFire);
                arc.min_horz = flat_scaled(type_id, wmake("minh"), kAngleScale, 0.0f);
                arc.max_horz = flat_scaled(type_id, wmake("maxh"), kAngleScale, 0.0f);
                arc.min_vert = flat_scaled(type_id, wmake("minv"), kAngleScale, 0.0f);
                arc.max_vert = flat_scaled(type_id, wmake("maxv"), kAngleScale, 0.0f);
                if (arc.min_horz != 0.0f || arc.max_horz != 0.0f ||
                    arc.min_vert != 0.0f || arc.max_vert != 0.0f) {
                    traverses = true;
                }
                bsp::gun_add_authored_arc_007f6b10(gun.arcs, arc);
            }
            // The guard below drops a gun whose authored windows produced no arc
            // beyond the seed, on the reading that the arc data failed. That is
            // right for a weapon that traverses and wrong for one that does not.
            // A BOMBPLATFORM authors exactly one window with all four bounds at
            // zero - measured: every one of the 81 guns this used to drop in
            // USN01 was category 0Ah, each with `arcs=1 windows=1` and a window
            // of minh=maxh=minv=maxv=0 - and 007F6B10 correctly declines to add
            // a zero-span arc, so the seed alone IS the complete answer for it.
            // Distinguishing on the authored data rather than on the category
            // keeps this a statement about what the data says.
            if (gun.arcs.size() <= 1 && windows > 0 && traverses) continue;
            done("Gunnery::add_authored_arc_007f6b10", 0x007f6b10u);

            gun.angles.horz = gun.rest_horz;
            gun.angles.vert = gun.rest_vert;
            gun.angles.target_horz = gun.rest_horz;
            gun.angles.target_vert = gun.rest_vert;
            gun.fire.barrel_timers.assign(static_cast<std::size_t>(gun.barrel_num), 0.0f);
            guns.push_back(gun);
        }
    }

    // 00956C20: the twelve category lists at unit+394h, the all-guns list at
    // unit+424h and the ranges at unit+430h, over the device list this process
    // built. Run through the reconstruction's own sequence.
    for (std::size_t i = 0; i < count; ++i) {
        UnitState& state = unit_state[i];
        std::vector<std::size_t> owned;
        for (std::size_t g = 0; g < guns.size(); ++g) {
            if (guns[g].unit_index == i) owned.push_back(g);
        }
        if (owned.empty()) continue;

        struct RebuildBinding final : bsp::UnitWeaponCategoryIndexHost {
            RebuildBinding(Impl& owner_in, UnitState& state_in,
                const std::vector<std::size_t>& owned_in)
                : owner(owner_in), state(state_in), owned(owned_in) {}
            void clear_torpedo_flag() override { torpedo = false; }
            void clear_list_00955eb0(int record) override {
                if (record < 0) {
                    all.clear();
                } else if (record < bsp::kUnitGunneryCategoryCount) {
                    state.category_guns[static_cast<std::size_t>(record)].clear();
                }
            }
            int device_count() override { return static_cast<int>(owned.size()); }
            bsp::GunneryRebuildDevice device_at(int index) override {
                bsp::GunneryRebuildDevice out;
                if (index < 0 || static_cast<std::size_t>(index) >= owned.size()) return out;
                const std::size_t slot = owned[static_cast<std::size_t>(index)];
                const GameGunRow& gun = owner.guns[slot];
                out.device = reinterpret_cast<void*>(slot + 1);
                out.dead = false;
                out.is_gun = true;
                out.operational = true;   // 00729F10 over an undamaged gun
                out.weapon_function = gun.category;
                out.max_range = gun.max_range;  // 00731020
                out.flak_alternate_range = gun.max_range;
                const GameBulletClassRow* b = owner.bullet(gun.bullet_class);
                out.has_blast = b != nullptr && b->blast_range > 0.0f;
                out.blast_inner = b != nullptr ? b->blast_range : 0.0f;
                out.blast_outer = b != nullptr ? b->blast_range : 0.0f;
                return out;
            }
            void append_to_category(int category, void* gun) override {
                if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return;
                state.category_guns[static_cast<std::size_t>(category)].push_back(
                    reinterpret_cast<std::size_t>(gun) - 1);
            }
            void append_to_all_guns(void* gun) override {
                all.push_back(reinterpret_cast<std::size_t>(gun) - 1);
            }
            void set_torpedo_flag() override { torpedo = true; }
            void store_category_range(int category, float range) override {
                if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return;
                state.category_ranges[static_cast<std::size_t>(category)] = range;
            }
            void store_category_blast_sum(int, float) override {}
            void store_artillery_max_range(float) override {}
            void store_any_weapon_max_range(float) override {}

            Impl& owner;
            UnitState& state;
            const std::vector<std::size_t>& owned;
            std::vector<std::size_t> all;
            bool torpedo{false};
        };

        RebuildBinding binding(*this, state, owned);
        bsp::rebuild_weapon_category_index_00956c20(binding);
        done("Gunnery::rebuild_weapon_category_index_00956c20", 0x00956c20u);
        state.row.guns = owned.size();
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            const std::size_t slot = static_cast<std::size_t>(c);
            state.row.category_guns[slot]
                = static_cast<int>(state.category_guns[slot].size());
            state.row.category_ranges[slot] = state.category_ranges[slot];
        }
    }
}

void GameGunneryHost::Impl::attach_passes() {
    for (std::size_t i = 0; i < unit_state.size(); ++i) {
        UnitState& state = unit_state[i];
        if (state.row.guns == 0) continue;
        // 00864BD0: the base attach onto the unit's tick element unit+310h, the
        // visibility cache, the two policy objects, the twelve masks and enable
        // bytes, then the director bridge 00863A80 which runs 008624C0 forced.
        state.attached = true;
        state.enabled = true;
        bsp::GunneryThrottle throttle;
        bsp::gunnery_throttle_prime_00864c1d(throttle, think_time);
        state.throttle = throttle.accumulator;
        state.category = bsp::unit_gunnery_initial_category_state_00864580();
        state.bridge_countdown = 0;
        state.allow_fire_cache = false;
        state.row.pass_attached = true;
        state.row.pass_enabled = true;
        ++summary.passes_attached;
        done("Gunnery::pass_attach_00864bd0", 0x00864bd0u);
        done("Gunnery::pass_construct_00864580", 0x00864580u);
        done("Gunnery::install_policies_008636a0", 0x008636a0u);
        done("Gunnery::create_director_bridge_00863a80", 0x00863a80u);
    }
}

// ---------------------------------------------------------------------------
// 00864FE0: the two-second think
// ---------------------------------------------------------------------------

namespace {

// The bsp::UnitGunneryPassHost binding for one unit, for one call.
class GunneryPassBinding final : public bsp::UnitGunneryPassHost {
public:
    GunneryPassBinding(GameGunneryHost::Impl& owner, std::size_t unit_index)
        : owner_(owner), unit_(unit_index), state_(owner.unit_state[unit_index]) {}

    bool unit_present() override { return true; }
    bool unit_is_dead() override { return state_.dead; }
    float throttle_threshold_00432650() override { return owner_.think_time; }
    bool unit_allows_sweep() override {
        // unit+61h. Milestone 2s: no writer outside the constructor.
        return owner_.units.unit_flag_0061(unit_);
    }

    void age_visibility_cache_00862c30(float elapsed) override {
        std::vector<GameGunneryHost::Impl::UnitState::VisibilityEntry> kept;
        kept.reserve(state_.visibility.size());
        for (auto& entry : state_.visibility) {
            float ttl = entry.ttl;
            if (bsp::visibility_entry_survives_00862c30(ttl, elapsed)) {
                entry.ttl = ttl;
                kept.push_back(entry);
            }
        }
        state_.visibility.swap(kept);
        owner_.done("Gunnery::age_visibility_cache_00862c30", 0x00862c30u);
    }

    bool has_director_bridge() override { return true; }

    void apply_director_stance_008624c0() override {
        // 008363E0 defaults director+3Ch and +3Dh to 1 for a unit of neither
        // class 0Bh nor class 9, and 007202FD sets all four of +220h..+223h to
        // 1 (docs/DIRECTOR_UPDATE_ARMS.md). No producer in this process moves
        // them, so the stance is the defaulted one.
        bsp::DirectorGunneryStance stance;
        const bool plane = owner_.units.unit_is_kind_of(unit_,
            bsp::kUnitGunneryKindPlaneBase);
        state_.category = bsp::apply_director_stance_008624c0(state_.category, stance,
            plane, false, state_.bridge_countdown, state_.allow_fire_cache);
        ++owner_.summary.bridge_applies;
        owner_.done("Gunnery::director_bridge_apply_008624c0", 0x008624c0u);
    }

    void update_target_records_00862cd0() override {
        // The this+B0h per-target engagement list. 00862CD0 and 00864880 are
        // contract: unread (docs/UNIT_GUNNERY_PASS.md section 10), so the list
        // is not built and nothing reads one back.
        owner_.record("Gunnery::update_target_records_00862cd0", 0x00862cd0u);
    }

    bool category_record_present(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        return !state_.category_guns[static_cast<std::size_t>(category)].empty();
    }
    bool category_enabled(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        const std::size_t slot = static_cast<std::size_t>(category);
        return state_.category.enabled[slot] && state_.category.mask[slot] != 0;
    }
    bool torpedo_category_enabled() override { return state_.category.torpedo_enable; }
    bool torpedo_may_take_fire_target() override { return true; }  // pass+7Dh, ctor 1
    bool category_gate_slot4(int) override {
        // 008636A0 installs the default gate 00D0D31C on a unit that is not
        // class 8; its vtable[4h] is 00861BE0, `mov al,1`.
        return true;
    }

    bool sweep_suppressed() override { return state_.sweep_suppressed; }

    int recon_contact_count_008053c0() override {
        // [recon+DE8h], the side's published enemy contact list. This process
        // builds no recon slot object; the stand-in is documented in the header.
        contacts_.clear();
        const int own_side = owner_.units.unit_side_0054(unit_);
        const std::size_t count = owner_.units.count();
        for (std::size_t i = 0; i < count; ++i) {
            if (i == unit_) continue;
            ++owner_.summary.contact_considered;
            if (owner_.units.unit_side_0054(i) == own_side) {
                ++owner_.summary.contact_reject_side;
                continue;
            }
            // docs/RECON_SLOT_LISTS.md rule (a): the class must be one the scan
            // visits. rule (b): +5Ch set, +5Dh / +5Eh / +60h clear.
            if (!owner_.units.unit_alive_and_visible(i)) {
                ++owner_.summary.contact_reject_visible;
                continue;
            }
            if (i < owner_.unit_state.size() && owner_.unit_state[i].dead) {
                ++owner_.summary.contact_reject_dead;
                continue;
            }
            // Rule (a) is "a class the scan visits", and the native scan
            // 00806480 visits seven PLANE leaf ids under IsKindOf(02h) as well
            // as the ship bases. Admitting only ship bases is why nothing ever
            // shot at an aircraft: on IJN01 the seven A7M fighters took zero
            // hits and zero damage while 295 AAMACHINEGUN and 60 FLAK guns sat
            // idle. docs/PLANE_UNIT_TICK.md.
            const bool ship_base =
                owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase);
            const bool plane_base =
                owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindPlaneBase);
            if (!ship_base && !plane_base) {
                ++owner_.summary.contact_reject_kind;
                continue;
            }
            if (plane_base) ++owner_.summary.contact_admit_plane;
            else ++owner_.summary.contact_admit_ship;
            contacts_.push_back(i);
        }
        ++owner_.summary.recon_sweeps;
        owner_.record("Gunnery::recon_slot_contacts_008053c0", 0x008053c0u);
        return static_cast<int>(contacts_.size());
    }
    void* recon_contact(int index) override {
        if (index < 0 || static_cast<std::size_t>(index) >= contacts_.size()) return nullptr;
        return handle(contacts_[static_cast<std::size_t>(index)]);
    }

    bool score_candidate_00863990(int category, void* target, float& distance) override {
        const std::size_t other = unit_of(target);
        if (other >= owner_.units.count()) return false;
        // 008633D0 then 00862820, then the range gate and the plane penalty.
        const bool is_plane = owner_.units.unit_is_kind_of(other,
            bsp::kUnitGunneryKindPlaneBase);
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        const std::size_t slot = static_cast<std::size_t>(category);
        bsp::GunneryTargetLiveness liveness;
        liveness.registered = owner_.units.unit_active(other);
        liveness.dead = owner_.unit_state[other].dead;
        if (!bsp::target_is_engageable_00862820(liveness)) return false;
        if (owner_.units.unit_class_id(other) < 0) return false;
        if (bsp::gunnery_rank(owner_.rank_table.data(), category,
                owner_.units.unit_class_id(other)) == 0) {
            return false;
        }
        if (!bsp::category_mask_admits_target_008633d0(state_.category.mask[slot],
                is_plane)) {
            return false;
        }
        float mine[3], theirs[3];
        owner_.unit_aim_point(unit_, mine);
        owner_.unit_aim_point(other, theirs);
        const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
            theirs[2] - mine[2]};
        bsp::GunneryScoreInputs in;
        in.distance = length3(delta);
        in.category_range = state_.category_ranges[slot];
        in.target_is_plane = is_plane;
        in.target_lacks_follow_target = false;
        const bsp::GunneryScoreResult out = bsp::score_candidate_00863990(in);
        distance = out.distance;
        owner_.done("Gunnery::score_candidate_00863990", 0x00863990u);
        if (out.distance > 0.0f
            && (state_.row.nearest_enemy <= 0.0f || out.distance < state_.row.nearest_enemy)) {
            state_.row.nearest_enemy = out.distance;
        }
        if (in.category_range > state_.row.best_range) {
            state_.row.best_range = in.category_range;
        }
        if (out.accepted) ++accepted_;
        else ++rejected_;
        return out.accepted;
    }

    bool unit_ai_suppresses_00862440(void* target) override {
        const std::size_t other = unit_of(target);
        // entity+1D4h, the script-set untouchable flag. No binding in this
        // mission sets it, so the proxy answers clear.
        const bool untouchable = false;
        owner_.done("Gunnery::untouchable_gate_00862440", 0x00862440u);
        return bsp::entity_suppresses_gunnery_00862440(other < owner_.units.count(),
            untouchable);
    }

    bool visible_00864d90(void* target) override {
        const std::size_t other = unit_of(target);
        for (const auto& entry : state_.visibility) {
            if (entry.target == other) return entry.visible;
        }
        // 00864680, the line-of-sight test itself, is contract: unread. Over
        // open water with no terrain in this process the answer is yes.
        owner_.record("Gunnery::line_of_sight_00864680", 0x00864680u);
        GameGunneryHost::Impl::UnitState::VisibilityEntry entry;
        entry.target = other;
        entry.visible = true;
        entry.ttl = owner_.random_range_00bd2f10(0.0f,
            bsp::kInstalledLosVisibleTimeOut);
        state_.visibility.push_back(entry);
        owner_.done("Gunnery::visibility_cache_append_00864d90", 0x00864d90u);
        return true;
    }

    int target_rank(int category, void* target) override {
        const std::size_t other = unit_of(target);
        if (other >= owner_.units.count()) return 0;
        return bsp::gunnery_rank(owner_.rank_table.data(), category,
            owner_.units.unit_class_id(other));
    }

    void* director_fire_target_slot4() override {
        return state_.fire_target != 0 ? handle(state_.fire_target - 1) : nullptr;
    }
    void* director_command_target_0071ebf0() override {
        return state_.command_target != 0 ? handle(state_.command_target - 1) : nullptr;
    }

    std::array<float, 3> unit_world_position_00427eb0() override {
        float point[3];
        owner_.unit_aim_point(unit_, point);
        return {point[0], point[1], point[2]};
    }
    int sub_entities_slot0fc(void* target) override {
        // target->vtable[0FCh]. The base implementation 00432480 is what 92 of
        // the 94 entity-hierarchy vtables carry in this slot - every ship, gun
        // platform and projectile - and it appends the entity itself, once and
        // unconditionally: PUSH ECX / MOV [ESP],ECX / MOV ECX,[ESP+8] /
        // PUSH EAX / CALL 004323D0 BSP_PointerVector_PushBack / RET 4.
        // The two overrides are MAirfield 006D4DD0 (intact hangars) and the
        // plane squadron 007F44E0 (live planes); neither is a unit this
        // process creates, so a target here always takes the base.
        // docs/SHIP_SUB_ENTITY_LIST.md.
        owner_.record("Gunnery::target_sub_entities_slot0fc", 0x008654acu);
        sub_entity_ = target;
        // Provenance for the diagnostic counters: this method is reached only from
        // step 8.7's two arms, so anything it hands back came from the director's
        // targets rather than from the recon sweep.
        if (target != nullptr) arm_entities_.insert(target);
        return target != nullptr ? 1 : 0;
    }
    void* sub_entity(int index) override {
        return index == 0 ? sub_entity_ : nullptr;
    }
    std::array<float, 3> entity_world_position(void* entity) override {
        const std::size_t other = unit_of(entity);
        float point[3] = {0.0f, 0.0f, 0.0f};
        if (other < owner_.units.count()) owner_.unit_aim_point(other, point);
        return {point[0], point[1], point[2]};
    }
    float vector_length_0042b2f0(const std::array<float, 3>& v) override {
        const float raw[3] = {v[0], v[1], v[2]};
        return length3(raw);
    }

    int category_gun_count(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return 0;
        const int guns = static_cast<int>(
            state_.category_guns[static_cast<std::size_t>(category)].size());
        if (guns > 0) ++owner_.summary.assignment_passes;
        return guns;
    }
    void* category_gun(int category, int index) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return nullptr;
        const auto& list = state_.category_guns[static_cast<std::size_t>(category)];
        if (index < 0 || static_cast<std::size_t>(index) >= list.size()) return nullptr;
        return reinterpret_cast<void*>(list[static_cast<std::size_t>(index)] + 1);
    }
    bsp::GunneryGunInputs gun_inputs(void* gun, void* target) override {
        bsp::GunneryGunInputs in;
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        if (slot >= owner_.guns.size()) return in;
        const GameGunRow& row = owner_.guns[slot];
        in.weapon_sub_type = row.category;
        in.is_torpedo_class_launcher = row.category == bsp::kUnitGunneryTorpedoCategory;
        in.minimum_air_range = 0.0f;
        // 00729BC0: the projectile-kind dispatch onto one of the six bot slots,
        // then that slot's vtable[1Ch]. The slot exists when the category maps
        // to one, and the range gate is the ammunition record's own +60h.
        const std::size_t other = unit_of(target);
        bool in_range = false;
        if (other < owner_.units.count()) {
            float mine[3], theirs[3];
            owner_.unit_aim_point(unit_, mine);
            owner_.unit_aim_point(other, theirs);
            const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
                theirs[2] - mine[2]};
            in_range = length3(delta) <= row.max_range;
        }
        in.slot_accepts_target = in_range;
        ++owner_.summary.gun_evaluations;
        if (!in_range) ++owner_.summary.gun_slot_rejects;
        owner_.done("Gunnery::bot_slot_accepts_target_00729bc0", 0x00729bc0u);
        return in;
    }
    bool target_is_plane(void* target) override {
        const std::size_t other = unit_of(target);
        return other < owner_.units.count()
            && owner_.units.unit_is_kind_of(other, bsp::kUnitGunneryKindPlaneBase);
    }
    void set_bot_fire_target_00727f10(void* gun, void* target, bool is_fire_target) override {
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        const std::size_t other = unit_of(target);
        if (slot >= owner_.guns.size()) return;
        GameGunRow& row = owner_.guns[slot];
        row.target_unit = other + 1;
        row.target_name = other < owner_.unit_state.size()
            ? owner_.unit_state[other].row.name : std::string();
        row.target_is_fire_target = is_fire_target;
        ++row.assigns;
        ++state_.row.assigns;
        ++owner_.summary.assigns;
        {
            const bool from_arm = arm_entities_.count(target) != 0;
            float fraction = 0.0f;
            if (other < owner_.units.count() && row.max_range > 0.0f) {
                float mine[3], theirs[3];
                owner_.unit_aim_point(unit_, mine);
                owner_.unit_aim_point(other, theirs);
                const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
                    theirs[2] - mine[2]};
                fraction = length3(delta) / row.max_range;
            }
            if (from_arm) {
                ++owner_.summary.assigns_from_arm;
                owner_.summary.arm_reach_fraction_sum += fraction;
                if (fraction > 0.5f) ++owner_.summary.arm_assigns_beyond_half;
            } else {
                ++owner_.summary.assigns_from_recon;
                owner_.summary.recon_reach_fraction_sum += fraction;
                if (fraction > 0.5f) ++owner_.summary.recon_assigns_beyond_half;
            }
        }
        owner_.done("Gunnery::set_bot_fire_target_00727f10", 0x00727f10u);
    }
    void add_gun_to_target_record_00864ca0(void*, void*) override {
        owner_.record("Gunnery::add_gun_to_target_record_00864ca0", 0x00864ca0u);
    }
    void clear_bot_fire_target_00728000(void* gun) override {
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        if (slot >= owner_.guns.size()) return;
        GameGunRow& row = owner_.guns[slot];
        ++row.clears;
        ++state_.row.clears;
        ++owner_.summary.clears;
        row.target_unit = 0;
        row.target_name.clear();
        row.target_is_fire_target = false;
        owner_.done("Gunnery::clear_bot_fire_target_00728000", 0x00728000u);
    }

    std::size_t candidates_seen() const noexcept { return accepted_; }
    std::size_t candidates_rejected() const noexcept { return rejected_; }

private:
    void* handle(std::size_t unit_index) const {
        return reinterpret_cast<void*>(unit_index + 1);
    }
    std::size_t unit_of(void* pointer) const {
        return pointer != nullptr ? reinterpret_cast<std::size_t>(pointer) - 1
                                  : owner_.units.count();
    }

    GameGunneryHost::Impl& owner_;
    std::size_t unit_;
    GameGunneryHost::Impl::UnitState& state_;
    std::vector<std::size_t> contacts_;
    std::size_t accepted_{0};
    std::size_t rejected_{0};
    void* sub_entity_{nullptr};   // the one entry 00432480 appends: the target itself
    std::set<void*> arm_entities_;   // entities handed out by step 8.7 this pass
};

}  // namespace

void GameGunneryHost::Impl::run_gunnery_pass(std::size_t index, float dt) {
    UnitState& state = unit_state[index];
    if (!state.attached) return;

    // The director's two targets, as this process holds them: 00863640 reads
    // director+238h, which the automatic target think 009F5DA0 wrote through
    // 00835860, and 0071EBF0 takes the newest queued command's target.
    state.fire_target = 0;
    state.command_target = 0;
    if (ship_ai != nullptr) {
        const std::vector<GameShipAiRow>& rows = ship_ai->rows();
        if (index < rows.size()) {
            const GameShipAiRow& row = rows[index];
            for (std::size_t i = 0; i < units.count(); ++i) {
                const GameUnitRow* candidate = units.unit_row(i);
                if (candidate == nullptr) continue;
                if (!row.fire_target.empty() && candidate->name == row.fire_target) {
                    state.fire_target = i + 1;
                }
                // NOT row.brain_target_name. That field is brain+0B20h, the
                // navigation goal vector's target, which sits beside the goal
                // position brain+0B2Ch..0B34h and names whatever the ship is
                // steering toward - frequently the ship itself. 0071EBF0 answers
                // with the newest QUEUED COMMAND's target, an order's target,
                // which is a different thing entirely.
                //
                // Using the goal target here made every gun engage its own hull:
                // step 8.7 appends the target itself (00432480), and neither the
                // native 00863990 nor this host applies a party or self test -
                // verified in the listing at 00863990..00863A73, which gates only
                // on the category mask 008633D0, an owner vtable[5Ch](5) test, the
                // per-category range at owner+category*4+430h and a plane penalty.
                // The native never meets the case because a queued command's
                // target is an enemy. See docs/GAME_EXECUTABLE.md.
                //
                // No command-target producer is wired in this process, and this
                // run queues no orders, so the faithful answer is "no command
                // target" - which is what the native would return here.
            }
        }
    }
    state.row.fire_target = state.fire_target != 0
        ? unit_state[state.fire_target - 1].row.name : std::string();
    state.row.command_target = state.command_target != 0
        ? unit_state[state.command_target - 1].row.name : std::string();
    done("Gunnery::director_fire_target_00863640", 0x00863640u);
    done("Gunnery::director_newest_command_target_0071ebf0", 0x0071ebf0u);

    GunneryPassBinding binding(*this, index);
    const float before = state.throttle;
    bool enabled = state.enabled;
    bsp::unit_gunnery_pass_tick_00864fe0(binding, dt, state.throttle, enabled);
    state.enabled = enabled;
    state.row.pass_enabled = enabled;
    ++state.row.ticks;
    ++summary.pass_ticks;
    if (state.throttle < before) {
        ++state.row.think_bodies;
        ++summary.pass_bodies;
        state.row.candidates += binding.candidates_seen();
        summary.candidates += binding.candidates_seen();
        summary.candidates_rejected += binding.candidates_rejected();
    }
    done("Gunnery::pass_tick_00864fe0", 0x00864fe0u);
}

// ---------------------------------------------------------------------------
// The aim ticks, the trigger latch and the 0ADh fire message
// ---------------------------------------------------------------------------

namespace {

class FireRequestBinding final : public bsp::GunFireRequestHost {
public:
    FireRequestBinding(GameGunneryHost::Impl& owner, std::size_t gun_slot)
        : owner_(owner), gun_(gun_slot) {}

    bool has_owning_unit() const override { return true; }          // gun+3F0h
    bool unit_suppressed() const override {                         // unit+5Dh
        const std::size_t unit = owner_.guns[gun_].unit_index;
        return unit < owner_.unit_state.size() && owner_.unit_state[unit].dead;
    }
    bool gun_suppressed() const override { return false; }          // gun+5Dh
    void release_fire_target_ref_006952a0() override {
        owner_.record("Gun::release_fire_target_ref_006952a0", 0x006952a0u);
    }
    bool is_rapid_fixed_slave_006e3d50(int) override { return false; }
    void send_fixed_slave_fire_message_0077c7b0(bool) override {
        owner_.record("Gun::fixed_slave_fire_message_0077c7b0", 0x0077c7b0u);
    }
    float random_stagger_00bd2f10(float lo, float hi) override {
        owner_.done("Gun::fire_stagger_00bd2f10", 0x00bd2f10u);
        return owner_.random_range_00bd2f10(lo, hi);
    }
    void stop_firing_0072b4c0() override {
        owner_.record("Gun::stop_firing_0072b4c0", 0x0072b4c0u);
    }
    void release_effect_ref() override {}
    // 0072D18C, the first thing the gun does each step: age the list at
    // gun+120h and drop the records whose countdown has gone strictly negative.
    // docs/GUN_BASE_TICK.md. The list has no producer in this reconstruction,
    // so the sweep is a no-op today; the counter says so out loud rather than
    // letting an always-zero look like a working path.
    void base_tick_0072ad40(float dt) override {
        GameGunRow& row = owner_.guns[gun_];
        owner_.summary.gun_pending_timers_expired +=
            bsp::gun_age_pending_timers_0072ad40(row.pending_timers, dt);
        owner_.summary.gun_pending_timers_live += row.pending_timers.size();
        owner_.done("Gun::base_tick_0072ad40", 0x0072ad40u);
    }
    void set_barrel_reload_timer_0072cf00(int index, float value) override {
        GameGunRow& row = owner_.guns[gun_];
        if (index < 0 || static_cast<std::size_t>(index) >= row.fire.barrel_timers.size()) {
            return;
        }
        const bsp::BarrelReloadWrite write = bsp::barrel_reload_write_0072cf00(value,
            1.0e9f, 1.0f, false);
        row.fire.barrel_timers[static_cast<std::size_t>(index)] = write.timer;
        owner_.done("Gun::set_barrel_reload_timer_0072cf00", 0x0072cf00u);
    }
    bool unit_fire_blocked() const override { return false; }   // unit+720h
    bool gun_disabled() const override { return false; }        // gun+3B8h
    void send_fire_message_0ad(std::uint16_t) override {
        ++owner_.guns[gun_].fire_messages;
        ++owner_.summary.fire_messages;
        sent_ = true;
        owner_.done("Gun::fire_message_0ad_0072d290", 0x0072d290u);
    }

    bool sent() const noexcept { return sent_; }

private:
    GameGunneryHost::Impl& owner_;
    std::size_t gun_;
    bool sent_{false};
};

}  // namespace

void GameGunneryHost::Impl::run_gun_aim_and_fire(float dt) {
    for (std::size_t g = 0; g < guns.size(); ++g) {
        GameGunRow& gun = guns[g];
        const std::size_t owner_unit = gun.unit_index;
        if (owner_unit >= unit_state.size()) continue;
        UnitState& state = unit_state[owner_unit];
        if (state.dead) continue;

        // Which of the five aim bots this gun's weapon sub-type selects.
        const bsp::GunBotSlotAssignment slots
            = bsp::gun_bot_slots_for_subtype_0072c6a0(gun.category, true, false);
        (void)slots;

        float want_horz = gun.rest_horz;
        float want_vert = gun.rest_vert;
        bool have_target = false;
        std::size_t target = 0;
        if (gun.target_unit != 0 && gun.target_unit - 1 < unit_state.size()) {
            target = gun.target_unit - 1;
            if (unit_state[target].dead || !units.unit_alive_and_visible(target)) {
                // 008FFA20's target-validity test: a dead target is dropped.
                gun.target_unit = 0;
                gun.target_name.clear();
                done("GunBot::target_still_valid_008ffa20", 0x008ffa20u);
            } else {
                have_target = true;
            }
        }

        float right[3], up[3], forward[3], origin[3];
        unit_pose(owner_unit, right, up, forward, origin);
        // PLACEHOLDER with no native counterpart, and it is the reason the recovered
        // ballistic arc 00955630 currently buys nothing. docs/GUN_MOUNT_POSITIONS.md:
        // the native muzzle origin is
        //   TransformAffinePoint(class->muzzleOffsets[gun+44Ch], gun[+3CCh]->worldMatrix)
        // at 007307A0/007307D3, where gun+3CCh is the model node named "barrel" or
        // "base" and the offsets come from the model's "fire" node group. `Height`
        // (class+0A8h) is the HULL height - ship-motion draft and a hit-slab half
        // extent - and nothing on the firing path reads it. Raising one shared origin
        // by it gives a whole battery the same muzzle point and makes
        // h = aim.y - muzzle.y near zero, which is exactly the case in which the arc
        // degenerates to the asin(g*R/v^2)/2 pre-estimate it replaced
        // (tools/gun_arc_pre_estimate_compare.py). Real per-gun origins need the model
        // node transforms, which this process does not yet build.
        const float muzzle[3] = {origin[0], origin[1] + state.hull_height, origin[2]};

        bool arc_solved = true;
        if (have_target) {
            float theirs[3];
            unit_aim_point(target, theirs);
            float velocity[3];
            unit_velocity(target, velocity);
            const std::array<float, 3> shooter{muzzle[0], muzzle[1], muzzle[2]};
            const std::array<float, 3> at{theirs[0], theirs[1], theirs[2]};
            const std::array<float, 3> v{velocity[0], velocity[1], velocity[2]};
            std::array<float, 3> lead = at;
            float pitch = 0.0f;

            if (gun.category == bsp::kUnitGunneryTorpedoCategory) {
                // The torpedo bot runs its own intercept solver and its round
                // does not fall, so no gravity term is added.
                // 0090022B loads [[gun+3F8h]+34h]+0E4h, WaterTravelSpeed, for
                // the torpedo bot's solver, while the AA flak bot's call at
                // 009031CF takes +50h (V0) - the choice is deliberate and the
                // host had been passing V0. For the Mark 15 that is 13 against
                // 51.444, and at 13 m/s the quadratic's leading coefficient
                // turns positive and no aspect has a solution against an 18.78
                // m/s destroyer, so the solver answered nothing on every launch.
                // docs/TORPEDO_LAUNCH_ACCURACY.md.
                const float solver_speed = gun.water_travel_speed > 0.0f
                    ? gun.water_travel_speed : gun.muzzle_speed;
                if (solver_speed > 0.0f
                    && bsp::torpedo_intercept_point_008fbb00(shooter, at,
                        solver_speed, v, lead)) {
                    done("GunBot::intercept_point_008fbb00", 0x008fbb00u);
                } else {
                    lead = at;
                }
            } else if (gun.muzzle_speed > 0.0f) {
                // 006DF520 steps 5, 6 and 7, the muzzle bot's own gravity
                // pre-estimate (docs/GUN_BOT_TICKS.md section 6.3):
                //   s     = distance * 9.81 / v^2              006DF8BF
                //   pitch = min(asin(s) * 0.5, pi/4)           00CEB5A8 clamps
                //   the aim point is pushed out along the target velocity by
                //   distance / (v * cos(pitch) * [muzzle+5Ch])
                // and step 7 refuses the shot when s > 1. The refinement
                // 00955630 makes on top of that is a contract here, so the
                // pre-estimate is the whole of the elevation this run uses.
                const float span[3] = {at[0] - muzzle[0], at[1] - muzzle[1],
                    at[2] - muzzle[2]};
                const float distance = length3(span);
                const float speed_squared = gun.muzzle_speed * gun.muzzle_speed;
                const float s = distance * kGravity / speed_squared;
                arc_solved = s <= 1.0f;
                pitch = std::min(std::asin(std::min(s, 1.0f)) * 0.5f, kQuarterPi);
                // [muzzle+5Ch] has no recovered producer; the divisor is taken
                // as one, which makes the push-out the plain time of flight.
                const float cosine = std::cos(pitch);
                const float flight = cosine > 0.0f ? distance / (gun.muzzle_speed * cosine)
                                                   : 0.0f;
                for (int i = 0; i < 3; ++i) lead[i] = at[i] + v[i] * flight;
                // Step 8, 006DFAD4: the recovered solve 00955630 replaces the
                // pre-estimate for the FINAL elevation. The pre-estimate above
                // still sets the time-of-flight push-out, which is what step 6
                // uses it for. docs/GUN_GRAVITY_ARC.md:
                //   k = g*R^2 / 2v^2,  D = R^2 - 4k(k + h),
                //   tan(pitch) = (R - sqrt(D)) / 2k          the low flat root
                // R is the HORIZONTAL distance and h the height difference, not
                // the slant range the pre-estimate used.
                bsp::GunGravityArcQuery arc_query;
                arc_query.aim_point = {lead[0], lead[1], lead[2]};
                arc_query.muzzle_position = {muzzle[0], muzzle[1], muzzle[2]};
                arc_query.muzzle_speed = gun.muzzle_speed;
                // mount_frame == nullptr is 0085B8DE's deliberate world-frame
                // path: the pair comes back without the local-frame round trip.
                arc_query.mount_frame = nullptr;
                const bsp::GunGravityArcSolution arc =
                    bsp::solve_gun_gravity_arc_00955630(arc_query);
                arc_solved = arc_solved && arc.solved;
                // angles.vert is *outPitch, the elevation above horizontal, and
                // the 009557F8 negate applies to *outYaw only. want_vert below
                // is asin(direct line) + pitch, so what belongs in `pitch` is
                // the superelevation above the direct line, not the total.
                const float led[3] = {lead[0] - muzzle[0], lead[1] - muzzle[1],
                    lead[2] - muzzle[2]};
                const float led_horizontal =
                    std::sqrt(led[0] * led[0] + led[2] * led[2]);
                const float direct_line = std::atan2(led[1], led_horizontal);
                pitch = arc.angles.vert - direct_line;
                record("GunBot::ballistic_arc_00955630", 0x00955630u);
                done("GunBot::gravity_pre_estimate_006df8bf", 0x006df8bfu);
            }

            const float delta[3] = {lead[0] - muzzle[0], lead[1] - muzzle[1],
                lead[2] - muzzle[2]};
            const float distance = length3(delta);
            if (distance > 0.0f) {
                const float unit_delta[3] = {delta[0] / distance, delta[1] / distance,
                    delta[2] / distance};
                // 008FDAF0: the world direction as a hull-relative angle pair.
                want_horz = std::atan2(dot3(unit_delta, right), dot3(unit_delta, forward));
                const float vertical = std::max(-1.0f,
                    std::min(1.0f, dot3(unit_delta, up)));
                want_vert = std::asin(vertical) + pitch;
                if (gun.category == bsp::kUnitGunneryTorpedoCategory) {
                    // 008FFF20 step 8 at 00900380 does NOT refuse a heading that
                    // falls outside a firing window: 0085AB50 -> 007F6190 snaps
                    // it up to pi/4 onto the nearest window edge and fires along
                    // that edge, abandoning the shot only when no window's
                    // horizontal bounds hold the heading at all or the snap
                    // exceeds the limit. The host had been handing the raw
                    // heading to gun_set_target_angles_0085aba0, which refuses
                    // outright, so a torpedo mount almost never opened a launch
                    // window. docs/TORPEDO_LAUNCH_ACCURACY.md.
                    const bsp::GunPlatformArcs snap_arcs{gun.arcs.data(),
                        gun.arcs.size()};
                    const float snapped = bsp::gun_snap_heading_to_fire_window_007f6190(
                        snap_arcs, want_horz, kQuarterPi);
                    if (bsp::gun_heading_snap_failed(snapped)) {
                        have_target = false;          // 00900392..009003A2
                    } else {
                        want_horz = snapped;
                        ++summary.torpedo_heading_snaps;
                    }
                    done("GunBot::snap_heading_to_fire_window_007f6190", 0x007f6190u);
                    // 009003DD commands a hard 0.0f vertical for the torpedo bot.
                    // Every torpedo platform in this installation's
                    // vehicleclasses.lua authors its one window with
                    // MinVertAngle == MaxVertAngle == 0, so any computed
                    // depression is refused outright by
                    // gun_set_target_angles_0085aba0 and the mount never opens a
                    // launch window. docs/TORPEDO_LAUNCH_ACCURACY.md.
                    want_vert = 0.0f;
                }

                done("GunBot::angles_from_world_direction_008fdaf0", 0x008fdaf0u);
            }
            if (!arc_solved) {
                ++gun.arc_unsolved;
                ++summary.arc_unsolved;
                have_target = false;   // 006DFA60's gate: no shot is armed
            }
        }

        const bsp::GunPlatformArcs arcs{gun.arcs.data(), gun.arcs.size()};
        float accepted_mark = 0.0f;
        const bool accepted = bsp::gun_set_target_angles_0085aba0(gun.angles, arcs,
            gun.speeds, want_horz, want_vert, accepted_mark);
        if (accepted) {
            ++gun.angle_sets;
            ++summary.angle_sets;
        } else {
            ++gun.angle_refusals;
            ++summary.angle_refusals;
            // angle_sets + angle_refusals is exactly guns * mission_ticks - every
            // gun, every tick, with no target gate - so the refusal count carries
            // no information about targets and must never be read as one. Verified
            // on IJN01: 230442 + 72558 = 303000 = 606 * 500 to the digit.
            // docs/AA_VERTICAL_WINDOW.md. These split out the ticks where the gun
            // actually held a target.
            if (have_target) ++summary.angle_refusals_targeted;
        }
        done("GunBot::set_target_angles_0085aba0", 0x0085aba0u);

        const bsp::GunArcRouteOutcome route = bsp::gun_arc_route_deltas_007f6530(arcs,
            gun.angles.horz, gun.angles.vert, gun.angles.target_horz,
            gun.angles.target_vert);
        done("Gun::arc_route_deltas_007f6530", 0x007f6530u);
        if (bsp::gun_step_aim_0085ad80(gun.angles, gun.speeds, route.deltas, dt, false)) {
            ++gun.aim_steps;
            ++summary.aim_steps;
        }
        done("Gun::step_aim_0085ad80", 0x0085ad80u);

        // 006DF520 step 12 arms the trigger through 006DEE40 against
        // *00CF9054 = 0.1 degree, not the stepper's 0.01-degree dead band that
        // gun_aim_settled_0085ae4a carries. Using the latter as a fire gate made
        // the host ten times stricter per axis than the native.
        // docs/GUN_SHOT_CADENCE.md divergence 2. This is a faithfulness fix and
        // is NOT expected to raise the shot count materially: the packet measures
        // it at 13% of targeted refusals, and the count is held down by the
        // authored 17.5 s reload, not by the settle test.
        // Only the CONSTANT differs from gun_aim_settled_0085ae4a: the wrapped
        // difference and the strict comparison are kept, because a plain
        // subtraction changes the semantics across the +/-pi wrap. An earlier
        // revision of this line dropped the wrap and was wrong for that reason.
        const bool settled =
            std::fabs(bsp::wrapped_angle_subtract_00438b10(
                gun.angles.target_horz, gun.angles.horz)) < bsp::kGunFireSettleBand &&
            std::fabs(bsp::wrapped_angle_subtract_00438b10(
                gun.angles.target_vert, gun.angles.vert)) < bsp::kGunFireSettleBand;
        const bool may_fire_here = bsp::gun_fire_allowed_007f60a0(arcs, gun.angles.horz,
            gun.angles.vert);
        done("Gun::fire_window_007f60a0", 0x007f60a0u);
        if (have_target && settled && !may_fire_here) {
            ++gun.arc_blocks;
            ++summary.arc_blocks;
        }
        // unit+634h, the scripted per-group fire inhibit. No mission-script
        // action in this mission writes it, so every bit is clear.
        const bool inhibited = false;
        // Which conjunct of want_fire fails on a tick that had a target. A
        // refusal here is not automatically a defect: a beam mount cannot train
        // astern, so check the commanded bearing against the platform's windows
        // before reading a non-zero want_fire_no_accept as one.
        // DEFINITIONAL, not independent evidence: `accepted` is the return of the
        // same call that increments angle_refusals, so on a targeted tick this is
        // the same event as angle_refusals_targeted and the two always match. Kept
        // only so the decomposition below reads completely; it is the counter to
        // drop first. docs/AA_VERTICAL_WINDOW.md.
        if (have_target && !accepted) ++summary.want_fire_no_accept;
        if (have_target && accepted && !settled) ++summary.want_fire_no_settle;
        if (have_target && accepted && settled && !may_fire_here) {
            ++summary.want_fire_no_window;
        }
        const bool want_fire = have_target && accepted && settled && may_fire_here
            && !inhibited;

        FireRequestBinding fire_host(*this, g);
        const bool before = gun.fire.fire_requested;
        const bool latched = bsp::gun_set_fire_request_0072d2c0(gun.fire, fire_host,
            want_fire);
        done("Gun::set_fire_request_0072d2c0", 0x0072d2c0u);
        if (latched && !before) {
            ++gun.trigger_rises;
            ++summary.trigger_rises;
        }

        const bool sent = bsp::gun_fixed_step_tick_0072d130(gun.fire, fire_host, dt);
        done("Gun::fixed_step_tick_0072d130", 0x0072d130u);
        if (!sent) continue;

        // 0072D860, the 0ADh arm: gun->vtable[1DCh] FireIfReady 00727E30, which
        // asks CanFire and fires on a yes.
        ++summary.fire_if_ready;
        bsp::GunFireGateInputs gate;
        gate.fire_params_armed = true;
        gate.disabled = false;
        gate.damage_counter = 0;
        gate.barrel_delay_time = gun.fire.barrel_delay_time;
        gate.secondary_delay = gun.fire.fire_stagger;
        gate.unit_cooldown_applies = false;
        gate.weapon_type_id = gun.category;
        gate.muzzle_world_y = muzzle[1];
        gate.muzzle_submerged = false;
        gate.muzzle_blocked = false;
        gate.barrel_count = gun.barrel_num;
        gate.reload_timers = gun.fire.barrel_timers.data();
        const bool can_fire = bsp::gun_can_fire_turning_0085a830(inhibited, arcs,
            gun.angles, gate, true);
        done("Gun::can_fire_0085a830", 0x0085a830u);
        if (!can_fire) {
            ++gun.can_fire_refusals;
            ++summary.can_fire_refusals;
            continue;
        }

        // 00730160 BSP_Gun_Fire: the next ready barrel, one projectile, then
        // the two timers.
        const int barrel = bsp::next_ready_barrel_007298d0(gun.fire.barrel_timers.data(),
            gun.barrel_num, gun.next_fire_barrel);
        gun.next_fire_barrel = (barrel + 1) % std::max(1, gun.barrel_num);
        if (barrel >= 0 && static_cast<std::size_t>(barrel) < gun.fire.barrel_timers.size()) {
            gun.fire.barrel_timers[static_cast<std::size_t>(barrel)] = gun.reload_time;
        }
        gun.fire.barrel_delay_time = gun.barrel_delay_time;
        ++gun.shots;
        ++state.row.shots;
        ++summary.shots;
        if (gun.first_shot_seconds < 0.0f) gun.first_shot_seconds = clock_seconds;
        if (summary.first_shot_seconds < 0.0f) {
            summary.first_shot_seconds = clock_seconds;
            float aim[3] = {0.0f, 0.0f, 0.0f};
            if (target < unit_state.size()) unit_aim_point(target, aim);
            const float span[3] = {aim[0] - muzzle[0], aim[1] - muzzle[1],
                aim[2] - muzzle[2]};
            log.notef("gunnery: first shot at t=%.2f s, %s platform %d (%s, bullet %d) at "
                "%s, range %.0f m, horz %.1f deg, vert %.1f deg",
                static_cast<double>(clock_seconds), gun.unit_name.c_str(),
                gun.platform_key, gun.function.c_str(), gun.bullet_class,
                gun.target_name.c_str(), static_cast<double>(length3(span)),
                static_cast<double>(gun.angles.horz * 57.2957795f),
                static_cast<double>(gun.angles.vert * 57.2957795f));
        }
        done("Gun::fire_00730160", 0x00730160u);
        done("Gun::next_ready_barrel_007298d0", 0x007298d0u);

        // 0072BF10 then the factory 006E8430: the muzzle direction with the
        // class `Throw` spread, and the launch velocity.
        const float horz = gun.angles.horz;
        const float vert = gun.angles.vert;
        const float flat_component = std::cos(vert);
        float direction[3];
        for (int i = 0; i < 3; ++i) {
            direction[i] = forward[i] * (flat_component * std::cos(horz))
                + right[i] * (flat_component * std::sin(horz))
                + up[i] * std::sin(vert);
        }
        GameProjectileRow shot;
        shot.gun_row = g;
        shot.owner_unit = owner_unit + 1;
        shot.owner_side = units.unit_side_0054(owner_unit);
        shot.bullet_class = gun.bullet_class;
        shot.alive = true;
        for (int i = 0; i < 3; ++i) shot.position[i] = muzzle[i];
        const bsp::TickPoint3 launch_direction{direction[0], direction[1], direction[2]};
        const bsp::TickPoint3 velocity = bsp::projectile_launch_velocity_006e8430(
            gun.muzzle_speed, launch_direction, 0);
        shot.flight.velocity = velocity;
        shot.flight.snapshot_current = bsp::TickPoint3{muzzle[0], muzzle[1], muzzle[2]};
        shot.flight.local_position = shot.flight.snapshot_current;
        shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
        shot.flight.class_disables_gravity = false;
        shots.push_back(shot);
        ++summary.projectiles;
        done("Projectile::launch_velocity_006e8430", 0x006e8430u);
        done("Projectile::spawn_0072bf10", 0x0072bf10u);
    }
}

// ---------------------------------------------------------------------------
// Projectile flight, the sweep and the impact
// ---------------------------------------------------------------------------

namespace {

class SegmentBinding final : public bsp::SegmentQueryHost {
public:
    SegmentBinding(GameGunneryHost::Impl& owner, std::size_t exclude)
        : owner_(owner), exclude_(exclude) {}

    float grid_cell_size() override { return 0.0f; }  // no spatial grid here
    const void* cell_first_node(int, int) override { return nullptr; }
    const void* cell_next_node(const void*) override { return nullptr; }
    const void* cell_node_entity(const void*) override { return nullptr; }
    int loose_entity_count() override { return static_cast<int>(owner_.units.count()); }
    const void* loose_entity(int slot) override {
        return reinterpret_cast<const void*>(static_cast<std::size_t>(slot) + 1);
    }
    const void* entity_owner(const void* entity) override { return entity; }
    bool entity_is_kind(const void*, int) override { return true; }
    bsp::HitQueryBounds entity_bounds(const void* entity) override {
        bsp::HitQueryBounds bounds;
        const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
        float centre[3], half[3];
        if (!box_of(index, centre, half)) return bounds;
        bounds.min.x = centre[0] - half[0];
        bounds.min.y = centre[1] - half[1];
        bounds.min.z = centre[2] - half[2];
        bounds.max.x = centre[0] + half[0];
        bounds.max.y = centre[1] + half[1];
        bounds.max.z = centre[2] + half[2];
        return bounds;
    }
    int shape_count(const void* entity) override {
        const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
        if (index == exclude_) return 0;
        if (index >= owner_.unit_state.size()) return 0;
        if (owner_.unit_state[index].dead) return 0;
        if (!owner_.units.unit_alive_and_visible(index)) return 0;
        return 1;
    }
    bool shape_trace_segment(const void* entity, int, const bsp::HitQueryPoint& from,
        const bsp::HitQueryPoint& to, bsp::HitRecordFill& record) override;
    int child_count(const void*) override { return 0; }
    const void* child_entity(const void*, int) override { return nullptr; }

    std::size_t hit_unit{0};   // one based

private:
    bool box_of(std::size_t index, float centre[3], float half[3]) const;

    GameGunneryHost::Impl& owner_;
    std::size_t exclude_;
};

bool SegmentBinding::box_of(std::size_t index, float centre[3], float half[3]) const {
    if (index >= owner_.unit_state.size()) return false;
    const GameGunneryHost::Impl::UnitState& state = owner_.unit_state[index];
    float right[3], up[3], forward[3], origin[3];
    owner_.unit_pose(index, right, up, forward, origin);
    const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
        state.hull_length * 0.5f};
    if (extents[0] <= 0.0f || extents[2] <= 0.0f) return false;
    for (int i = 0; i < 3; ++i) {
        centre[i] = origin[i];
        half[i] = std::fabs(right[i]) * extents[0] + std::fabs(up[i]) * extents[1]
            + std::fabs(forward[i]) * extents[2];
    }
    return true;
}

bool SegmentBinding::shape_trace_segment(const void* entity, int,
    const bsp::HitQueryPoint& from, const bsp::HitQueryPoint& to,
    bsp::HitRecordFill& record) {
    const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
    float centre[3], half[3];
    if (!box_of(index, centre, half)) return false;
    // The hull box in its own frame: a slab test along the three pose rows.
    float right[3], up[3], forward[3], origin[3];
    owner_.unit_pose(index, right, up, forward, origin);
    const GameGunneryHost::Impl::UnitState& state = owner_.unit_state[index];
    const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
        state.hull_length * 0.5f};
    const float world_from[3] = {from.x, from.y, from.z};
    const float world_to[3] = {to.x, to.y, to.z};
    const float rel[3] = {world_from[0] - origin[0], world_from[1] - origin[1],
        world_from[2] - origin[2]};
    const float span[3] = {world_to[0] - world_from[0], world_to[1] - world_from[1],
        world_to[2] - world_from[2]};
    const float* axes[3] = {right, up, forward};
    float local_origin[3];
    float local_span[3];
    for (int i = 0; i < 3; ++i) {
        local_origin[i] = dot3(rel, axes[i]);
        local_span[i] = dot3(span, axes[i]);
    }
    float enter = 0.0f;
    float leave = 1.0f;
    for (int i = 0; i < 3; ++i) {
        const float low = -extents[i];
        const float high = extents[i];
        if (std::fabs(local_span[i]) < 1.0e-6f) {
            if (local_origin[i] < low || local_origin[i] > high) return false;
            continue;
        }
        float t0 = (low - local_origin[i]) / local_span[i];
        float t1 = (high - local_origin[i]) / local_span[i];
        if (t0 > t1) std::swap(t0, t1);
        enter = std::max(enter, t0);
        leave = std::min(leave, t1);
        if (enter > leave) return false;
    }
    bsp::HitQueryPoint point;
    point.x = world_from[0] + span[0] * enter;
    point.y = world_from[1] + span[1] * enter;
    point.z = world_from[2] + span[2] * enter;
    bsp::shape_hit_fill_0087fec0(record, point, entity);
    record.shape_kind = 0x0A;
    record.hull_segment = kDirectHitHullSegment;
    bsp::hit_record_set_entity_00470370(record, entity);
    hit_unit = index + 1;
    return true;
}

}  // namespace

void GameGunneryHost::Impl::run_projectiles(float dt) {
    for (GameProjectileRow& shot : shots) {
        if (!shot.alive) continue;
        const float from[3] = {shot.position[0], shot.position[1], shot.position[2]};
        shot.flight = bsp::projectile_flight_step(shot.flight, dt);
        shot.position[0] = shot.flight.local_position.x;
        shot.position[1] = shot.flight.local_position.y;
        shot.position[2] = shot.flight.local_position.z;
        shot.flight.snapshot_current = shot.flight.local_position;
        shot.life += dt;
        ++summary.projectile_steps;
        done("Projectile::flight_step_006e7670", 0x006e7670u);

        const bsp::TickPoint3 a{from[0], from[1], from[2]};
        const bsp::TickPoint3 b{shot.position[0], shot.position[1], shot.position[2]};
        if (!bsp::projectile_segment_is_sweepable(a, b)) continue;
        ++summary.sweeps;

        // 0084BF00 step 2: the entity sweep. The static trace is the water
        // surface, which 0078CF20 answers at height zero for open sea.
        SegmentBinding query(*this, shot.owner_unit - 1);
        bsp::SegmentQueryArgs args;
        args.from = bsp::HitQueryPoint{from[0], from[1], from[2]};
        args.to = bsp::HitQueryPoint{shot.position[0], shot.position[1], shot.position[2]};
        args.exclude_entity = reinterpret_cast<const void*>(shot.owner_unit);
        bsp::HitRecordFill record;
        bsp::hit_record_reset_00470470(record);
        const bool hit = bsp::query_segment_0098add0(query, args, record);
        done("Projectile::sweep_entities_0098add0", 0x0098add0u);

        if (hit && query.hit_unit != 0) {
            const float point[3] = {record.position.x, record.position.y,
                record.position.z};
            const float direction[3] = {shot.flight.velocity.x, shot.flight.velocity.y,
                shot.flight.velocity.z};
            ++summary.impacts_entity;
            done("Projectile::on_impact_0084bc60", 0x0084bc60u);
            apply_hit(shot.owner_unit - 1, shot.gun_row, query.hit_unit - 1, point,
                direction);
            shot.alive = false;
            continue;
        }

        // The water crossing 0078D1B0 solves and the ocean sampler 0078CF20
        // behind it: open sea is height zero.
        if (shot.position[1] <= 0.0f && from[1] > 0.0f) {
            ++summary.water_crossings;
            // A torpedo does not die at the surface: it enters its swim. The
            // round's record carries a swim speed at +470h, written at
            // 0085786D..00857875 as WaterTravelSpeed * the double at 00D0C5E0
            // (0.5999994277954102), and 00855A90 derives the engagement range as
            // that speed times FlyTime - so the authored range only makes sense
            // if the round actually travels at it underwater. Before this, every
            // torpedo was killed on the frame it touched the sea and 12 of 16
            // launches ended as `water`. docs/TORPEDO_LAUNCH_ACCURACY.md.
            const GameBulletClassRow* const entry = bullet(shot.bullet_class);
            const float swim = entry != nullptr ? entry->swim_speed : 0.0f;
            if (swim > 0.0f && !shot.swimming) {
                shot.swimming = true;
                // Level the round onto the surface plane at the swim speed,
                // keeping the heading the launch gave it. Gravity is turned off
                // through the flight state's own classDesc[+20h] flag rather
                // than by stepping the round outside the recovered
                // projectile_flight_step, so the swim still runs through the
                // reconstructed rule.
                shot.flight.class_disables_gravity = true;
                const float vx = shot.flight.velocity.x;
                const float vz = shot.flight.velocity.z;
                const float horizontal = std::sqrt(vx * vx + vz * vz);
                if (horizontal > 0.0f) {
                    shot.flight.velocity.x = vx / horizontal * swim;
                    shot.flight.velocity.z = vz / horizontal * swim;
                }
                shot.flight.velocity.y = 0.0f;
                shot.position[1] = 0.0f;
                shot.flight.local_position.y = 0.0f;
                shot.flight.snapshot_current.y = 0.0f;
                ++summary.torpedo_swims_started;
                continue;
            }
            ++summary.impacts_static;
            done("Projectile::water_crossing_0078d1b0", 0x0078d1b0u);
            shot.alive = false;
            continue;
        }
        const GameBulletClassRow* row = bullet(shot.bullet_class);
        const float range = row != nullptr ? row->range : 0.0f;
        // Once swimming, the round travels at the swim speed, which is the
        // speed 00855A90's range was derived against (swim * FlyTime), so the
        // life bound only lands on FlyTime if the same speed is used here.
        const float cruise = shot.swimming && row != nullptr && row->swim_speed > 0.0f
            ? row->swim_speed
            : (row != nullptr && row->muzzle_speed > 0.0f ? row->muzzle_speed : 1.0f);
        const float speed = cruise;
        if (range > 0.0f && shot.life * speed > range) {
            ++summary.expired;
            shot.alive = false;
        }
    }
    shots.erase(std::remove_if(shots.begin(), shots.end(),
        [](const GameProjectileRow& row) { return !row.alive; }), shots.end());
}

// ---------------------------------------------------------------------------
// The hit record, the damage and the kill credit
// ---------------------------------------------------------------------------

namespace {

class ShipHitBinding final : public bsp::ShipHitRecordHost {
public:
    ShipHitBinding(GameGunneryHost::Impl& owner, std::size_t victim, std::size_t shooter,
        const GameBulletClassRow* weapon, const float direction[3])
        : owner_(owner), victim_(victim), shooter_(shooter), weapon_(weapon) {
        for (int i = 0; i < 3; ++i) direction_[i] = direction[i];
    }

    bool hull_is_submarine() override { return false; }
    float hull_armour() override { return owner_.unit_state[victim_].armour; }
    float class_armour_virtual() override { return owner_.unit_state[victim_].armour; }
    float class_mass() override {
        // [this+538h]+B0h, the `Mass` key. 00826FD0 compares it against 100 and
        // 500 to pick which of the three part-damage arms runs.
        return owner_.units.unit_hull_mass_00b0(victim_);
    }
    void clear_hit_accumulator() override {}

    float hull_damage(float armour) override {
        return bsp::hull_damage_00470510(hit_, armour);
    }
    float part_damage(float armour, int part_hit_index) override {
        return bsp::part_damage_004705c0(hit_, armour, part_hit_index);
    }

    int session_mode() override { return 0; }
    bool unit_is_local_players() override { return false; }
    unsigned campaign_difficulty_index() override { return 0; }
    float difficulty_multiplier(unsigned) override {
        owner_.record("ShipHit::difficulty_multiplier_008270ad", 0x008270adu);
        return 1.0f;
    }

    void apply_part_damage(int, const float[3], float damage) override {
        applied_ += damage;
        ++owner_.summary.part_damages;
        owner_.done("ShipHit::part_health_0092d1f0", 0x0092d1f0u);
    }
    void impact_direction(float out[3]) override {
        for (int i = 0; i < 3; ++i) out[i] = direction_[i];
    }

    void roll_axis(float out[3]) override { out[0] = 0.0f; out[1] = 1.0f; out[2] = 0.0f; }
    float settings_roll_torque_scale() override { return 0.0f; }
    float settings_roll_mass_root() override { return 0.0f; }
    void route_add_hull_torque(const bsp::ShipRollTorque&) override {
        owner_.record("ShipHit::add_hull_torque_00827312", 0x00827312u);
    }

    float weapon_water_damage() override {
        return weapon_ != nullptr ? weapon_->water_damage : 0.0f;
    }
    float weapon_fire_damage() override {
        return weapon_ != nullptr ? weapon_->fire_damage : 0.0f;
    }
    float weapon_fire_chance() override {
        return weapon_ != nullptr ? weapon_->fire_chance : 0.0f;
    }
    float random_unit_float() override { return owner_.random_range_00bd2f10(0.0f, 1.0f); }
    void record_flood_rate(float rate) override {
        if (rate > 0.0f) {
            ++owner_.unit_state[victim_].row.floods_started;
            ++owner_.summary.flood_messages_9e;
        }
    }
    void record_fire_rate(float rate) override {
        if (rate > 0.0f) {
            ++owner_.unit_state[victim_].row.fires_started;
            ++owner_.summary.fire_messages_9e;
        }
    }
    void route_set_damage_channel(bsp::ShipDamageChannel, float, bool) override {
        owner_.done("ShipHit::damage_channel_message_9e_0080fa50", 0x0080fa50u);
    }

    void roll_component_failure(float) override {
        owner_.record("ShipHit::component_failure_0093bed0", 0x0093bed0u);
    }

    bool is_local_players_unit() override { return false; }
    bool shooter_world_position(float out[3]) override {
        float right[3], up[3], forward[3], origin[3];
        owner_.unit_pose(shooter_, right, up, forward, origin);
        for (int i = 0; i < 3; ++i) out[i] = origin[i];
        return true;
    }
    void push_damage_direction(const float[3]) override {}
    void route_hull_impact_effect(int, const float[3]) override {}
    void route_part_impact_effect(int, const float[3]) override {}

    bool apply_base_hit_record() override {
        // 008777D0, the base hit record: the hull pass and the part pass. The
        // hull damage reaches the unit's own health through 00879070.
        const float armour = owner_.unit_state[victim_].armour;
        const float damage = bsp::hull_damage_00470510(hit_, armour);
        owner_.done("ShipHit::base_hit_record_008777d0", 0x008777d0u);
        if (damage <= 0.0f) return false;
        applied_ += damage;
        bsp::UnitHealth health;
        health.current_health = owner_.unit_state[victim_].health;
        health.max_health = owner_.unit_state[victim_].max_health;
        bsp::UnitDamageGates gates;
        const bsp::UnitDamageOutcome outcome = bsp::apply_damage_00879070(health, gates,
            damage);
        owner_.done("ShipHit::apply_damage_00879070", 0x00879070u);
        if (outcome.refused) return false;
        const bsp::UnitHealthWrite write = bsp::set_health_00877b90(health,
            outcome.new_health, bsp::UnitSessionMode::campaign, 0, false);
        owner_.done("ShipHit::set_health_00877b90", 0x00877b90u);
        if (write.wrote) owner_.unit_state[victim_].health = write.stored_health;
        ++owner_.summary.hull_damages;
        return true;
    }

    void set_hit(const bsp::HitRecord& hit) noexcept { hit_ = hit; }
    float applied() const noexcept { return applied_; }

private:
    GameGunneryHost::Impl& owner_;
    std::size_t victim_;
    std::size_t shooter_;
    const GameBulletClassRow* weapon_;
    float direction_[3]{};
    bsp::HitRecord hit_{};
    float applied_{0.0f};
};

}  // namespace

void GameGunneryHost::Impl::apply_hit(std::size_t shooter, std::size_t gun_row,
    std::size_t victim, const float point[3], const float direction[3]) {
    if (victim >= unit_state.size() || shooter >= unit_state.size()) return;
    UnitState& target = unit_state[victim];
    if (target.dead) return;
    const GameGunRow& gun = guns[gun_row];
    const GameBulletClassRow* weapon = bullet(gun.bullet_class);

    // 00926E80 queues the record, 009239A0 dispatches it: the entity gates, the
    // OnHit delivery and the attribution listener.
    ++summary.queued_hits;
    done("Projectile::queue_hit_00926e80", 0x00926e80u);

    // 00470350 HitRecord::SetShot: record+14h = shot->vtable[54h](), which is
    // 006E7C60, a uniform draw between the weapon class's DamageMin (+ACh) and
    // DamageMax (+B0h) through 00BD2F10. A direct segment hit gets no part
    // damage: +28h stays zero (docs/HIT_NARROWPHASE.md).
    bsp::HitRecord hit;
    const float low = weapon != nullptr ? weapon->damage_min : 0.0f;
    const float high = weapon != nullptr ? weapon->damage_max : 0.0f;
    hit.hull_damage_base = random_range_00bd2f10(low, high);
    hit.part_damage_base = 0.0f;
    hit.falloff_range = 0.0f;
    hit.ignore_falloff = false;
    hit.armour_selector = 0.0f;
    hit.hull_segment = kDirectHitHullSegment;
    hit.weapon_scale = 1.0f;   // shot->vtable[58h] = 006E7C90, gun+43Ch, default 1.0f
    hit.owner_modifier = 1.0f; // 008E6430 over an empty modifier list
    hit.part_hits = nullptr;
    hit.part_hit_count = 0;
    done("Projectile::hit_record_set_shot_00470350", 0x00470350u);
    done("Projectile::shot_damage_base_006e7c60", 0x006e7c60u);

    bsp::ShipHitRecordView view;
    view.segment_kind = 0x0A;
    for (int i = 0; i < 3; ++i) view.impact_point[i] = point[i];
    view.shot_present = true;
    view.shot_is_depth_charge = false;
    view.shot_is_torpedo = gun.category == bsp::kUnitGunneryTorpedoCategory;
    view.weapon_present = weapon != nullptr;

    ShipHitBinding binding(*this, victim, shooter, weapon, direction);
    binding.set_hit(hit);
    const float before = target.health;
    bsp::apply_ship_hit_record_00826f10(binding, hit, view);
    ++summary.ship_hit_records;
    ++summary.dispatched_hits;
    done("ShipHit::apply_hit_record_00826f10", 0x00826f10u);
    done("Projectile::dispatch_queued_hit_009239a0", 0x009239a0u);

    const float applied = before - target.health;
    target.row.hits_taken += 1;
    target.row.damage_taken += applied;
    target.row.health = target.health;
    unit_state[shooter].row.hits_dealt += 1;
    unit_state[shooter].row.damage_dealt += applied;
    summary.damage_total += applied;
    if (summary.first_hit_seconds < 0.0f) summary.first_hit_seconds = clock_seconds;

    // 0077CE60, step 7 of 009239A0: the attribution block on the victim.
    bsp::KillAttributionSource source;
    source.ordnance_kind = gun.category;
    source.ordnance_category = bsp::kill_credit_ordnance_category_00779a00(gun.category);
    source.owner_side = units.unit_side_0054(shooter);
    source.origin_slot = units.unit_side_0054(shooter);
    source.owner_id = static_cast<int>(shooter) + 1;
    source.has_owner_unit = true;
    bsp::KillAttributionAttacker attacker;
    attacker.unit_class = units.unit_class_id(shooter);
    attacker.owning_slot = units.unit_side_0054(shooter);
    attacker.type_id = unit_state[shooter].row.type_id;
    attacker.is_ship = true;
    bsp::KillAttributionKamikaze kamikaze;
    target.attribution = bsp::kill_attribution_0077ce60(target.attribution, true, source,
        attacker, kamikaze, true);
    target.last_attacker = shooter + 1;
    ++summary.attributions;
    done("Hit::attribution_0077ce60", 0x0077ce60u);

    bsp::UnitHealth health;
    health.current_health = target.health;
    health.max_health = target.max_health;
    if (bsp::unit_is_dead(health)) kill_unit(victim);
}

void GameGunneryHost::Impl::kill_unit(std::size_t victim) {
    UnitState& target = unit_state[victim];
    if (target.dead) return;
    target.dead = true;
    target.row.sunk = true;
    target.row.sunk_seconds = clock_seconds;
    target.enabled = false;
    target.row.pass_enabled = false;
    ++summary.deaths;
    done("Death::entity_kill_00926d90", 0x00926d90u);
    record("Death::unit_sink_008110f0", 0x008110f0u);

    if (target.last_attacker != 0) {
        target.row.killed_by = unit_state[target.last_attacker - 1].row.name;
        ++unit_state[target.last_attacker - 1].row.kill_credits;
    }

    struct KillBinding final : bsp::KillCreditHost {
        explicit KillBinding(Impl& owner_in) : owner(owner_in) {}
        int session_mode() override { return 0; }
        bool skip_friendly_losses() override { return false; }
        int local_player_side() override { return -1; }
        bool root_entity_already_scored() override { return false; }
        void add_loss(int, bool, const std::string&) override { ++losses; }
        void add_type_kill(int, std::size_t, int) override { ++type_kills; }
        int& kill_tree_leaf(int, std::size_t, const bsp::ScoringKillKey&) override {
            return leaf;
        }
        int add_named_counter(int, const std::string&) override { return ++counter; }
        int award_threshold(const std::string&) override {
            owner.record("KillCredit::award_threshold_0050fc30", 0x0050fc30u);
            return 0;
        }
        void grant_award(int, const std::string&, int) override { ++awards; }
        void update_kill_list(int, std::uint32_t, int, int, bool) override { ++entries; }

        Impl& owner;
        int leaf{0};
        int counter{0};
        int losses{0};
        int type_kills{0};
        int awards{0};
        int entries{0};
    };

    KillBinding binding(*this);
    bsp::UnitKillInputs in;
    in.victim_name = target.row.name;
    in.victim_type_id = target.row.type_id;
    in.victim_owning_slot = target.row.side;
    in.victim_is_root = true;
    in.has_attacker = target.last_attacker != 0;
    in.attacker_is_ship_base = true;
    in.ordnance_kind = target.attribution.ordnance_kind;
    in.sole_attacker = target.attribution.sole_attacker;
    if (target.last_attacker != 0) {
        in.attacker_type_id = unit_state[target.last_attacker - 1].row.type_id;
    }
    in.attribution.attacker_side = target.attribution.attacker_side;
    in.attribution.credited_slot = target.attribution.credited_slot;
    in.attribution.originating_slot = target.attribution.origin_slot;
    in.attribution.attacker_class = target.attribution.attacker_class;
    in.attribution.victim_class = units.unit_class_id(victim);
    in.attribution.victim_side = target.row.side;
    bsp::kill_credit_record_unit_kill_0091bda0(binding, in);
    ++summary.kill_credits;
    done("KillCredit::record_unit_kill_0091bda0", 0x0091bda0u);
}

// ---------------------------------------------------------------------------
// GameGunneryHost
// ---------------------------------------------------------------------------

GameGunneryHost::GameGunneryHost(GameHostLog& log, GameUnitsHost& units,
    GameMissionLuaHost& lua)
    : impl_(std::make_unique<Impl>(log, units, lua)) {}

GameGunneryHost::~GameGunneryHost() = default;

void GameGunneryHost::set_ship_ai(GameShipAiHost* ai) noexcept { impl_->ship_ai = ai; }

void GameGunneryHost::attach_00864bd0() {
    Impl& host = *impl_;
    host.build_rank_table();
    host.build_guns();
    host.attach_passes();
    host.summary.guns = host.guns.size();
    host.summary.device_rows = host.devices.size();
    host.summary.bullet_rows = host.bullets.size();
    for (const Impl::UnitState& state : host.unit_state) {
        if (state.row.guns > 0) ++host.summary.units_with_guns;
    }
    host.log.notef("gunnery: %zu unit(s) carry %zu gun(s) from %zu authored device class "
        "row(s) and %zu bullet class row(s); the weapon director think time is %.3f s "
        "(Globals.WeaponSystems.WeaponDirectorThinkTime, 0087e16b)",
        host.summary.units_with_guns, host.summary.guns, host.summary.device_rows,
        host.summary.bullet_rows, static_cast<double>(host.think_time));
}

void GameGunneryHost::fixed_step(float step_seconds) {
    Impl& host = *impl_;
    if (host.guns.empty()) return;
    host.clock_seconds += step_seconds;
    ++host.step_index;
    for (std::size_t i = 0; i < host.unit_state.size(); ++i) {
        host.run_gunnery_pass(i, step_seconds);
    }
    host.run_gun_aim_and_fire(step_seconds);
    host.run_projectiles(step_seconds);
}

const std::vector<GameGunRow>& GameGunneryHost::guns() const noexcept {
    return impl_->guns;
}

const std::vector<GameGunneryUnitRow>& GameGunneryHost::unit_rows() const noexcept {
    static std::vector<GameGunneryUnitRow> rows;
    rows.clear();
    rows.reserve(impl_->unit_state.size());
    for (const Impl::UnitState& state : impl_->unit_state) rows.push_back(state.row);
    return rows;
}

const GameGunnerySummary& GameGunneryHost::summary() const noexcept {
    return impl_->summary;
}

void GameGunneryHost::log_sample(unsigned long long step_index,
    unsigned long long interval) {
    Impl& host = *impl_;
    if (interval == 0 || step_index % interval != 0) return;
    if (host.guns.empty()) return;
    host.log.notef("  gunnery step %llu t=%.2f assigns=%llu shots=%llu shots_in_flight=%zu "
        "hits=%llu damage=%.1f deaths=%llu", step_index,
        static_cast<double>(host.clock_seconds), host.summary.assigns, host.summary.shots,
        host.shots.size(), host.summary.dispatched_hits,
        static_cast<double>(host.summary.damage_total), host.summary.deaths);
}

void GameGunneryHost::report() {
    Impl& host = *impl_;
    const GameGunnerySummary& s = host.summary;
    host.log.notef("summary mission gunnery units=%zu guns=%zu passes=%zu ticks=%llu "
        "bodies=%llu bridge=%llu sweeps=%llu candidates=%llu rejected=%llu "
        "assignment_passes=%llu gun_evaluations=%llu slot_rejects=%llu assigns=%llu "
        "clears=%llu",
        s.units_with_guns, s.guns, s.passes_attached, s.pass_ticks, s.pass_bodies,
        s.bridge_applies, s.recon_sweeps, s.candidates, s.candidates_rejected,
        s.assignment_passes, s.gun_evaluations, s.gun_slot_rejects, s.assigns,
        s.clears);
    host.log.notef("summary mission gunnery source arm=%llu recon=%llu "
        "arm_mean_reach=%.3f recon_mean_reach=%.3f arm_beyond_half=%llu "
        "recon_beyond_half=%llu",
        s.assigns_from_arm, s.assigns_from_recon,
        s.assigns_from_arm ? s.arm_reach_fraction_sum / double(s.assigns_from_arm) : 0.0,
        s.assigns_from_recon ? s.recon_reach_fraction_sum / double(s.assigns_from_recon) : 0.0,
        s.arm_assigns_beyond_half, s.recon_assigns_beyond_half);
    {
        // The ordnance inventory 007EEC50's AttackFeasibilityInputs need, per
        // unit, aggregated over that unit's guns exactly as the 007ED7E0 family
        // aggregates over the weapon controller's slots.
        // docs/ORDNANCE_KIND_IDENTITY.md.
        std::size_t torpedo = 0, general_bomb = 0, drop_kamikaze = 0, paratrooper = 0;
        std::map<std::size_t, int> per_unit;
        for (const GameGunRow& gun : host.guns) {
            int& bits = per_unit[gun.unit_index];
            if (bsp::ordnance_has_torpedo_2bh(gun.ordnance)) bits |= 1;
            if (bsp::ordnance_has_general_bomb_2ah(gun.ordnance)) bits |= 2;
            if (bsp::ordnance_has_drop_kamikaze_2fh(gun.ordnance)) bits |= 4;
            if (bsp::ordnance_has_paratrooper_31h(gun.ordnance)) bits |= 8;
        }
        for (const std::pair<const std::size_t, int>& row : per_unit) {
            if (row.second & 1) ++torpedo;
            if (row.second & 2) ++general_bomb;
            if (row.second & 4) ++drop_kamikaze;
            if (row.second & 8) ++paratrooper;
        }
        host.log.notef("summary mission gunnery ordnance units_with torpedo=%zu "
            "general_bomb=%zu drop_kamikaze=%zu paratrooper=%zu (of %zu units with guns)",
            torpedo, general_bomb, drop_kamikaze, paratrooper, per_unit.size());
    }
    host.log.notef("summary mission gunnery torpedo_ranges_derived=%llu "
        "swims_started=%llu snaps=%llu bullet_ranges_derived=%llu "
        "base_tick_timers_live=%llu expired=%llu",
        s.torpedo_ranges_derived, s.torpedo_swims_started,
        s.torpedo_heading_snaps, s.bullet_ranges_derived,
        s.gun_pending_timers_live, s.gun_pending_timers_expired);
    host.log.notef("summary mission gunnery contacts considered=%llu side=%llu "
        "invisible=%llu dead=%llu kind=%llu admit_ship=%llu admit_plane=%llu",
        s.contact_considered, s.contact_reject_side, s.contact_reject_visible,
        s.contact_reject_dead, s.contact_reject_kind, s.contact_admit_ship,
        s.contact_admit_plane);
    host.log.notef("summary mission gunnery targeted refusals=%llu no_accept=%llu "
        "no_settle=%llu no_window=%llu", s.angle_refusals_targeted,
        s.want_fire_no_accept, s.want_fire_no_settle, s.want_fire_no_window);
    host.log.notef("summary mission gunnery aim angle_sets=%llu refusals=%llu steps=%llu "
        "arc_blocks=%llu arc_unsolved=%llu trigger_rises=%llu fire_messages=%llu "
        "fire_if_ready=%llu can_fire_refusals=%llu shots=%llu first_shot=%.2f s",
        s.angle_sets, s.angle_refusals, s.aim_steps, s.arc_blocks, s.arc_unsolved,
        s.trigger_rises,
        s.fire_messages, s.fire_if_ready, s.can_fire_refusals, s.shots,
        static_cast<double>(s.first_shot_seconds));
    host.log.notef("summary mission gunnery projectiles created=%llu steps=%llu sweeps=%llu "
        "entity_impacts=%llu water=%llu expired=%llu in_flight=%zu",
        s.projectiles, s.projectile_steps, s.sweeps, s.impacts_entity, s.water_crossings,
        s.expired, host.shots.size());
    host.log.notef("summary mission gunnery damage queued_hits=%llu dispatched=%llu "
        "hit_records=%llu hull=%llu part=%llu fires=%llu floods=%llu attributions=%llu "
        "deaths=%llu kill_credits=%llu total_damage=%.1f first_hit=%.2f s",
        s.queued_hits, s.dispatched_hits, s.ship_hit_records, s.hull_damages,
        s.part_damages, s.fire_messages_9e, s.flood_messages_9e, s.attributions, s.deaths,
        s.kill_credits, static_cast<double>(s.damage_total),
        static_cast<double>(s.first_hit_seconds));

    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_guns{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_assigns{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_shots{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_refusals{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_blocks{};
    for (const GameGunRow& gun : host.guns) {
        if (gun.category < 0 || gun.category >= bsp::kUnitGunneryCategoryCount) continue;
        const std::size_t slot = static_cast<std::size_t>(gun.category);
        ++cat_guns[slot];
        cat_assigns[slot] += gun.assigns;
        cat_shots[slot] += gun.shots;
        cat_refusals[slot] += gun.angle_refusals;
        cat_blocks[slot] += gun.arc_blocks;
    }
    host.log.note("  cat  Function              guns   assigns     shots  no_window  "
        "arc_blocked");
    for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
        const std::size_t slot = static_cast<std::size_t>(c);
        if (cat_guns[slot] == 0) continue;
        const char* name = bsp::gunnery_category_function_name(c);
        host.log.notef("  %3d  %-20s %5llu %9llu %9llu %10llu %12llu", c,
            name != nullptr ? name : "?", cat_guns[slot], cat_assigns[slot],
            cat_shots[slot], cat_refusals[slot], cat_blocks[slot]);
    }

    std::vector<const GameGunRow*> ordered;
    ordered.reserve(host.guns.size());
    for (const GameGunRow& gun : host.guns) {
        if (gun.shots > 0) ordered.push_back(&gun);
    }
    std::sort(ordered.begin(), ordered.end(),
        [](const GameGunRow* a, const GameGunRow* b) { return a->shots > b->shots; });
    if (!ordered.empty()) {
        host.log.note("  gun                       plat  cat  range  assigns  shots  "
            "first_shot  target");
        std::size_t shown = 0;
        for (const GameGunRow* gun : ordered) {
            if (shown++ >= 20) break;
            host.log.notef("  %-24s %5d %4d %6.0f %8llu %6llu %11.2f  %s",
                gun->unit_name.c_str(), gun->platform_key, gun->category,
                static_cast<double>(gun->max_range), gun->assigns, gun->shots,
                static_cast<double>(gun->first_shot_seconds),
                gun->target_name.empty() ? "-" : gun->target_name.c_str());
        }
    }

    host.log.note("  unit                 side  guns  cats                 range  nearest"
        "  shots  hits   dealt   taken   health   sunk_at  killed_by");
    for (const Impl::UnitState& state : host.unit_state) {
        if (state.row.guns == 0 && state.row.hits_taken == 0) continue;
        std::string categories;
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            if (state.row.category_guns[static_cast<std::size_t>(c)] == 0) continue;
            char text[24];
            std::snprintf(text, sizeof(text), "%d:%d ", c,
                state.row.category_guns[static_cast<std::size_t>(c)]);
            categories += text;
        }
        host.log.notef("  %-20s %4d %5zu  %-18s %6.0f %8.0f %6llu %5llu %7.0f %7.0f "
            "%8.0f %9.2f  %s",
            state.row.name.c_str(), state.row.side, state.row.guns, categories.c_str(),
            static_cast<double>(state.row.best_range),
            static_cast<double>(state.row.nearest_enemy),
            state.row.shots, state.row.hits_taken,
            static_cast<double>(state.row.damage_dealt),
            static_cast<double>(state.row.damage_taken),
            static_cast<double>(state.row.health),
            static_cast<double>(state.row.sunk_seconds),
            state.row.killed_by.empty() ? "-" : state.row.killed_by.c_str());
    }
}

}  // namespace bsp::game
