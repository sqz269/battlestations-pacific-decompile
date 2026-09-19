// bsp_game.exe milestone 2t: the gun chain.
//
// See include/bsp/game_hosts_gunnery.hpp for the address list and for what this
// process does not hold. Nothing here reconstructs native code: every step is a
// call of a bsp:: rule or host already on main, or a recorded gap.

#include "bsp/game_hosts_gunnery.hpp"

#include <array>
#include <map>
#include <memory>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "bsp/ai_tuning_globals.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_ai.hpp"
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
#include "bsp/attack_target_classify.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/recon_sensor_pass.hpp"
#include "bsp/sensor_table_data.hpp"
#include "bsp/ship_hit_record.hpp"
#include "bsp/submarine_model.hpp"
#include "bsp/unit_kind_query.hpp"
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
// FLD double ptr [00D7A270] at 0084BE63: the burst centre is backed off from
// the impact point along the impact direction by this much. The value reads
// 0.05 (tools/pe_const_read.py d:00d7a270), and the direction it scales is the
// **unit** direction 0084BF00 stores at buffer+10h, so this is 5 cm out of the
// struck surface and not a time step.
constexpr float kBlastCentreBackOff = 0.05f;

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
        float artillery_max_range{0.0f};  // unit+490h, 00956E43
        float any_weapon_max_range{0.0f}; // unit+494h, 00956E59
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
    // Packet cc8_torpedo_closest_approach: one row per swimming round, kept
    // after the round is erased.
    std::vector<GameTorpedoApproachRow> torpedo_approaches;
    // Packet cc8_dive_glide: one row per released bomb, kept after the erase.
    std::vector<GameBombImpactRow> bomb_impacts;
    // Packet cc8_torpedo_aim_census: the ordered-target half of one record.
    void fill_ordered_fields(GameTorpedoApproachRow& rec,
                             const GameProjectileRow& row) const {
        rec.ordered_name = unit_name_or_index(row.ordered_target);
        rec.ordered_min_distance = row.ordered_min_distance;
        rec.ordered_min_time = row.ordered_min_time;
        rec.crossing_angle = row.crossing_angle;
        rec.drop_owner_heading = row.drop_owner_heading;
        rec.drop_target_heading = row.drop_target_heading;
        rec.drop_crossing_angle = row.drop_crossing_angle;
        if (row.ordered_target != 0 && row.ordered_min_distance >= 0.0f) {
            const float dx = row.target_pos_at_min[0] - row.target_pos_release[0];
            const float dz = row.target_pos_at_min[2] - row.target_pos_release[2];
            rec.target_travel = std::sqrt(dx * dx + dz * dz);
        }
    }
    std::string unit_name_or_index(std::size_t one_based) const {
        if (one_based == 0) return std::string("-");
        const std::size_t i = one_based - 1;
        if (i < unit_state.size() && !unit_state[i].row.name.empty()) {
            return unit_state[i].row.name;
        }
        return std::string("unit#") + std::to_string(one_based);
    }
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

    // Packet cc8_gunnery_host: both take the first unit index to build, so a
    // later spawn batch registers only the units it added. `guns` is appended
    // to and never cleared, so re-running either over an already-built unit
    // would duplicate its guns and re-seed its throttle, health and category
    // state. See GameGunneryHost::register_new_units_00864bd0.
    void build_guns(std::size_t first_unit);
    void build_rank_table();
    void attach_passes(std::size_t first_unit);
    // How many units of the units host this object has already registered.
    // The units host only ever appends, so [0, built_units) is settled.
    std::size_t built_units{0};
    void refresh_build_summary();

    void run_gunnery_pass(std::size_t index, float dt);
    void refresh_command_targets();
    // 0071EBF0 per unit, one based; 0 when the unit has no current command
    // target. Rebuilt only when the command row count or the unit count
    // changes, which is at load and on a new order, not every tick.
    std::vector<std::size_t> command_target_by_unit;
    std::size_t command_rows_resolved{static_cast<std::size_t>(-1)};
    // The second half of the cache key: a row flipping current without the
    // vector growing has to re-resolve too. See refresh_command_targets.
    std::size_t command_rows_current{static_cast<std::size_t>(-1)};
    std::size_t command_targets_resolved{0};
    void run_gun_aim_and_fire(float dt);
    void run_projectiles(float dt);
    // Publishes the rows 00A08460 reads into the process-wide table the AI
    // coordinator holds. docs/AI_TARGET_WEIGHT_TERMS.md term 2.
    void publish_ai_weapon_facts();
    void apply_hit(std::size_t shooter, std::size_t gun_row, std::size_t victim,
        const float point[3], const float direction[3],
        const bsp::HitRecord* blast_record = nullptr);
    // 0084BC60 step 7 (0084BE28..0084BEE3) and the gather behind 0084BAD0: the
    // radial burst every class with a Blast table makes on impact, and the one
    // that carries a torpedo's warhead. docs/TORPEDO_WARHEAD.md.
    void apply_impact_blast(std::size_t shooter, std::size_t gun_row,
        const float point[3], const float direction[3]);
    void kill_unit(std::size_t victim);

    // --- rule (c), the sensor pass ------------------------------------------
    // docs/RECON_SENSOR_PASS_BINDING.md. 008073C0 runs once per tick over every
    // side, not once per firing unit, so the state and its per-class sensor
    // records live on the host and the step runs in fixed_step.

    // One 008082A0 record per distinct `ReconClass` id seen, plus the 56
    // addressable rows 008048A0 indexes with sensor_list_index_008085aa. The
    // record is held by pointer so that adding a class never moves the entry
    // arrays a live GunneryReconSensorRow points at.
    struct ReconClassRecord {
        bsp::SensorClassTable table;
        std::array<std::vector<bsp::ReconSensorEntry>, bsp::kSensorListCount> entries;
        std::array<bsp::GunneryReconSensorRow, bsp::kSensorListCount> rows;
    };
    std::map<int, std::unique_ptr<ReconClassRecord>> recon_classes;
    // Per unit, resolved once from the authored row: the class's `ReconClass`
    // record and the SQUARE of its `ReconModifier`, which is what class+B8h
    // holds (009623CF FMUL ST0,ST0 before the 009623D9 store).
    std::vector<const ReconClassRecord*> unit_recon_record;
    std::vector<float> unit_recon_modifier_sq;
    bsp::ReconSensorPassState recon_pass;
    // [00F874B8], the refresh countdown, and slot+2Ch, the last-pass stamp the
    // pass subtracts from the clock to get its own dt. Starting the countdown
    // at zero makes the first tick run a pass, as a freshly zeroed native
    // timer does.
    float recon_refresh_timer{0.0f};
    float recon_last_pass_seconds{0.0f};
    unsigned long long recon_classes_missing{0};   // `ReconClass` absent from the row
    unsigned long long recon_modifier_absent{0};   // `ReconModifier` absent from the row

    // The published level per observing side, summed over every tick and every
    // (side, target) pair the pass covered. It sums to the state's own flat
    // detected_* counters; the split is what attributes a contact drop to a
    // side rather than to the run as a whole.
    struct ReconSideCensus {
        int side{0};
        unsigned long long none_level{0};
        unsigned long long blip{0};
        unsigned long long identified{0};
    };
    std::vector<ReconSideCensus> recon_side_census;

    const ReconClassRecord* recon_record_for_class(int recon_class_id);
    void resolve_recon_inputs();
    void step_recon_sensor_pass_008073c0(float dt);

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
        "                  f[q .. 'bdmin'] = num(bc.Blast.BlastDamageMin, 1000) or 0\n"
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

void GameGunneryHost::Impl::build_guns(std::size_t first_unit) {
    const std::size_t count = units.count();
    if (first_unit >= count) return;
    // resize, not assign: the units host appends, so the states already built
    // keep their health, timers and category state and the new units come in
    // default-constructed behind them.
    unit_state.resize(count);

    // The distinct class ids the scene created, for one flatten pass. Only the
    // new units': flatten writes `VehicleClass[id].BSPGun` into the Lua state,
    // which outlives this call, so a class an earlier batch flattened is
    // already there and re-flattening it would rewrite the same table.
    std::vector<int> class_ids;
    for (std::size_t i = first_unit; i < count; ++i) {
        const GameUnitRow* row = units.unit_row(i);
        if (row == nullptr || row->type_id < 0) continue;
        if (std::find(class_ids.begin(), class_ids.end(), row->type_id) == class_ids.end()) {
            class_ids.push_back(row->type_id);
        }
    }
    flatten_class_tables(class_ids);

    bool think_read = false;
    for (std::size_t i = first_unit; i < count; ++i) {
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
                // The hook's other product, which this call already computed and
                // used to discard: +8h after the 006E9968 rewrite. 009FE270
                // switches on it for the AI target-weight accuracy.
                gun.bullet_sub_type = finalised.sub_type;
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
                // 008568E0's two water-entry limits, read from the same Bullets
                // row the range derivation above reads. docs/TORPEDO_TICK.md.
                b.max_water_hit_vel = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxWaterHitVel", 0.0f);
                b.max_fall = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxFall", 0.0f);
                b.damage_min = flat_scaled(type_id, make("dmin"), kMilliScale, 0.0f);
                b.damage_max = flat_scaled(type_id, make("dmax"), kMilliScale, 0.0f);
                b.water_damage = flat_scaled(type_id, make("wdmg"), kMilliScale, 0.0f);
                b.fire_damage = flat_scaled(type_id, make("fdmg"), kMilliScale, 0.0f);
                b.fire_chance = flat_scaled(type_id, make("fchance"), kMilliScale, 0.0f);
                b.mass = flat_scaled(type_id, make("mass"), kMilliScale, 0.0f);
                b.blast_damage_min = flat_scaled(type_id, make("bdmin"), kMilliScale, 0.0f);
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

    // The ordnance inventory 007EEC50's AttackFeasibilityInputs need, handed to
    // the units host because the script-order host owns those inputs and cannot
    // see the guns. Aggregated over the unit's guns exactly as the 007ED7E0
    // family aggregates over the weapon controller's slots - the union of the
    // answer sets, since the family asks "does any slot carry kind N".
    // docs/ORDNANCE_KIND_IDENTITY.md. Stored at load, not at report time,
    // because the mission script issues its orders during luaStageInit.
    // Recomputed over the whole gun list rather than the new range, because the
    // mask is an aggregate: this is a re-store of the same value for a unit an
    // earlier batch built, since `guns` only ever gained rows since then.
    {
        std::vector<std::uint64_t> masks(count, 0u);
        for (const GameGunRow& gun : guns) {
            if (gun.unit_index < masks.size()) masks[gun.unit_index] |= gun.ordnance.mask;
        }
        for (std::size_t i = 0; i < count; ++i) units.store_unit_ordnance(i, masks[i]);
    }

    // 00956C20: the twelve category lists at unit+394h, the all-guns list at
    // unit+424h and the ranges at unit+430h, over the device list this process
    // built. Run through the reconstruction's own sequence. New units only: an
    // already-built unit's gun set did not change, and the binding below
    // rebuilds every gun as `dead=false operational=true`, so re-running it
    // over an existing unit would put a gun this run has destroyed back into
    // its category lists.
    for (std::size_t i = first_unit; i < count; ++i) {
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
            void store_artillery_max_range(float range) override {
                // 00956E43, unit+490h. Packet cc8_ship_ai_firepower_inputs.
                state.artillery_max_range = range;
            }
            void store_any_weapon_max_range(float range) override {
                // 00956E59, unit+494h, the maximum over every category.
                state.any_weapon_max_range = range;
            }

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
        state.row.artillery_max_range = state.artillery_max_range;
        state.row.any_weapon_max_range = state.any_weapon_max_range;
    }
}

void GameGunneryHost::Impl::attach_passes(std::size_t first_unit) {
    // New units only: the body below re-primes the throttle accumulator, the
    // category state, the bridge countdown and the fire cache, which is the
    // load-time seed and not something an already-ticking unit may be given a
    // second time.
    for (std::size_t i = first_unit; i < unit_state.size(); ++i) {
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
            // docs/RECON_SLOT_LISTS.md rule (c): the level the sensor pass
            // published for (own side, this target). `none` means the entry
            // drained out of every published list, so the sweep must not see
            // it. A side the pass never covered keeps
            // kReconDetectionUnknownLevel, the permissive answer the tree used
            // before rule (c) ran.
            const bsp::ReconDetectionLevel level = owner_.recon_pass.level(own_side, i);
            if (level == bsp::ReconDetectionLevel::none) {
                ++owner_.summary.contact_reject_recon_level;
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
        // Packet cc8_torpedo_gun_assignment: the per-reason funnel for the
        // torpedo category only. Every `return false` below gets its own
        // counter, so the run says which guard refuses and not merely that one
        // did. The counters are instrumentation; no native address produces them.
        const bool torpedo_cat = category == bsp::kUnitGunneryTorpedoCategory;
        if (torpedo_cat) ++owner_.summary.torpedo_cat_score_calls;
        const std::size_t other = unit_of(target);
        if (other >= owner_.units.count()) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_unknown;
            return false;
        }
        // 008633D0 then 00862820, then the range gate and the plane penalty.
        const bool is_plane = owner_.units.unit_is_kind_of(other,
            bsp::kUnitGunneryKindPlaneBase);
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        const std::size_t slot = static_cast<std::size_t>(category);
        bsp::GunneryTargetLiveness liveness;
        liveness.registered = owner_.units.unit_active(other);
        liveness.dead = owner_.unit_state[other].dead;
        if (!bsp::target_is_engageable_00862820(liveness)) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_liveness;
            return false;
        }
        if (owner_.units.unit_class_id(other) < 0) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_class;
            return false;
        }
        if (bsp::gunnery_rank(owner_.rank_table.data(), category,
                owner_.units.unit_class_id(other)) == 0) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_rank;
            return false;
        }
        if (!bsp::category_mask_admits_target_008633d0(state_.category.mask[slot],
                is_plane)) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_mask;
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
        if (torpedo_cat) {
            // Everything before this point passed, so the only guard left inside
            // 00863990 is the range test against the category range at
            // owner+category*4+430h. Recording the two values on the first few
            // rejections is what turns "the range refused" into a number.
            if (out.accepted) {
                ++owner_.summary.torpedo_cat_score_accepted;
            } else {
                ++owner_.summary.torpedo_cat_reject_range;
                if (owner_.summary.torpedo_cat_reject_range <= 3) {
                    owner_.log.notef("gunnery: torpedo category refused a target at "
                        "%.0f m against a category range of %.0f m (00863990)",
                        static_cast<double>(in.distance),
                        static_cast<double>(in.category_range));
                }
            }
        }
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

// The per-unit answer to 0071EBF0, resolved once rather than per unit per tick.
// A first cut did the name match inside run_gunnery_pass and cost O(commands x
// units) every unit every tick - 77 units against 83 current commands - which
// took a 40-second mission past ten minutes. The semantics are unchanged; only
// the placement is.
// 0071EBF0's rule, which is NOT "the last current row wins".
//
// With [this+30h] == 1 the image scans the unit's own slot array at +54h - ten
// entries of 0x1Ch, the scan stopping at the first NULL (0071EC10-0071EC1D) -
// then walks BACKWARD from the last occupied entry (0071EC1F `LEA ESI,[EAX-1]`,
// stepping at 0071EC4D/0071EC50) and takes the first whose `vtable[+0Ch]`
// answers 1 or 2 (0071EC43 `CMP EAX,1` and 0071EC48 `CMP EAX,2`, both to
// 0071ECC6). A command of any other category is stepped OVER, not taken. When
// none answers, 0071EC57 falls to the lazily-built static at 00E19BB4, which is
// a neutral record and never another unit. ([this+30h] == 2 returns this+18Ch
// instead; this host has no such mode.)
//
// The host's rows carry both fields the rule needs: `slot_index` is the entry
// 0071E6C0 pushed and `category` is what vtable[+0Ch] answers. Taking rows in
// vector order and ignoring the category is what let a `moveto` row (category
// 3) appearing mid-mission take the dive bomber's target off its `divebomb` row
// (category 2) and re-point it at a unit whose position equalled the aircraft's,
// which showed up as approach+BCh = exactly 0.0 m in local/usn04_aim.log.
// docs/DIVE_BOMB_TASK.md, "The aim run".
//
// Resolving the accepted row's token is a SECOND step on purpose. The image
// returns the accepted slot's target field whatever it holds, so an accepting
// row whose token names nothing leaves the unit with no target - it does not
// fall through to an older row.
void GameGunneryHost::Impl::refresh_command_targets() {
    const std::vector<GameCommandRow>& command_rows = units.commands().rows();
    // The cache key is the row count AND how many of them are current, so a row
    // going current or stale without the vector growing still re-resolves. Both
    // are O(commands), not the O(commands x units) the name match costs.
    std::size_t current_rows = 0;
    for (const GameCommandRow& command : command_rows) {
        if (command.current) ++current_rows;
    }
    if (command_rows_resolved == command_rows.size()
        && command_rows_current == current_rows
        && command_target_by_unit.size() == units.count()) {
        return;
    }
    command_rows_resolved = command_rows.size();
    command_rows_current = current_rows;
    command_target_by_unit.assign(units.count(), 0);
    std::map<std::string, std::size_t> by_name;
    for (std::size_t i = 0; i < units.count(); ++i) {
        const GameUnitRow* row = units.unit_row(i);
        if (row != nullptr && !row->name.empty()) by_name.emplace(row->name, i + 1);
    }
    // Step one, the backward walk: per unit, the accepting row that sits in the
    // highest slot. A row the host never pushed carries slot_index -1; those
    // are ordered behind every pushed row and among themselves by vector
    // position, which is the best standing this host has for them.
    std::vector<const GameCommandRow*> accepted(units.count(), nullptr);
    std::vector<long long> accepted_rank(units.count(), -1);
    long long position = 0;
    for (const GameCommandRow& command : command_rows) {
        ++position;
        if (!command.current) continue;
        if (command.unit_index >= accepted.size()) continue;
        // 0071EC43 and 0071EC48: only these two categories answer.
        if (command.category != 1 && command.category != 2) continue;
        const long long rank = command.slot_index >= 0
            ? (static_cast<long long>(command.slot_index) << 32) + position
            : position;
        if (rank <= accepted_rank[command.unit_index]) continue;
        accepted_rank[command.unit_index] = rank;
        accepted[command.unit_index] = &command;
    }
    // Step two: resolve only that row's token.
    for (std::size_t i = 0; i < accepted.size(); ++i) {
        if (accepted[i] == nullptr || accepted[i]->target_token.empty()) continue;
        const std::map<std::string, std::size_t>::const_iterator found =
            by_name.find(accepted[i]->target_token);
        if (found != by_name.end()) command_target_by_unit[i] = found->second;
        // Packet cc8_torpedo_aim_census. 0071EBF0's rule picks a row per unit
        // and resolves its token BY NAME; "command_targets units_with=N" said
        // how many resolved and never which, so the ship a bomber is actually
        // attacking was unnamed in every run this stream has taken.
        log.notef("  command target 0071EBF0: unit=%s token=\"%s\" -> %s",
            (i < unit_state.size() ? unit_state[i].row.name.c_str() : "?"),
            accepted[i]->target_token.c_str(),
            found != by_name.end()
                ? (found->second != 0 && found->second - 1 < unit_state.size()
                       ? unit_state[found->second - 1].row.name.c_str()
                       : "(index out of range)")
                : "(no unit of that name)");
    }
    command_targets_resolved = 0;
    for (std::size_t i = 0; i < command_target_by_unit.size(); ++i) {
        if (command_target_by_unit[i] != 0) ++command_targets_resolved;
        // The plane's control path needs the same answer, and taking it from
        // here rather than resolving names again is what keeps the two from
        // drifting apart.
        units.store_unit_command_target(i, command_target_by_unit[i]);
    }
}

// ---------------------------------------------------------------------------
// docs/RECON_SLOT_LISTS.md rule (c), the sensor pass. 008073C0 over 00806840.
// ---------------------------------------------------------------------------

// scripts/datatables/autoload/reconclasses.lua takes `RealisticTable` only when
// the global `GameMode` is 1 and falls through to `ArcadeTable` otherwise. The
// installed scripts tree carries no gamemode.lua for that autoload's DoFile to
// define `GameMode` from, so the else arm is the one this process can reach.
inline constexpr bsp::ReconTableVariant kReconVariant = bsp::ReconTableVariant::arcade;

const GameGunneryHost::Impl::ReconClassRecord*
GameGunneryHost::Impl::recon_record_for_class(int recon_class_id) {
    auto found = recon_classes.find(recon_class_id);
    if (found != recon_classes.end()) return found->second.get();
    auto made = std::make_unique<ReconClassRecord>();
    // 008082A0 on the authored rows. An id outside 1..12 gives the empty record
    // the loader leaves behind, which 008048A0 then reads as "no entries".
    made->table = bsp::build_sensor_class_table_008082a0(kReconVariant, recon_class_id);
    for (std::size_t i = 0; i < bsp::kSensorListCount; ++i) {
        const std::vector<bsp::SensorTableEntry>& src = made->table.lists[i];
        std::vector<bsp::ReconSensorEntry>& dst = made->entries[i];
        dst.reserve(src.size());
        for (const bsp::SensorTableEntry& row : src) {
            // Field for field onto the 1Ch entry 008048A0 walks. The loader has
            // already squared Dist (008085F1), halved Gain (00808626) and
            // turned MaxLevel into the cap (0080866B), so nothing is re-derived
            // here: this is the record layout change only.
            bsp::ReconSensorEntry entry;
            entry.authored_distance = row.dist;             // +0h
            entry.max_normalized_distance_sq = row.dist_sq; // +4h
            entry.gain_per_second = row.gain_per_second;    // +8h
            entry.cap = row.max_value;                      // +0Ch
            entry.mask_bit = row.raw_type;                  // +10h
            entry.max_bearing_error = row.half_angle;       // +14h
            entry.bearing_limited = row.bearing_limited;    // +18h
            dst.push_back(entry);
        }
        made->rows[i].entries = dst.empty() ? nullptr : dst.data();
        made->rows[i].count = dst.size();
    }
    const ReconClassRecord* raw = made.get();
    recon_classes.emplace(recon_class_id, std::move(made));
    return raw;
}

void GameGunneryHost::Impl::resolve_recon_inputs() {
    const std::size_t count = units.count();
    if (unit_recon_record.size() == count) return;
    unit_recon_record.assign(count, nullptr);
    unit_recon_modifier_sq.assign(count, bsp::kGunneryReconModifierSqDefault);
    for (std::size_t i = 0; i < count; ++i) {
        // The `VehicleClass` table index, which is GameUnitRow::type_id and NOT
        // GameUnitsHost::unit_class_id: the latter is the entity class id the
        // IsKindOf chain walks (06h ship base, 0Fh plane base), and indexing
        // the authored table with it resolves every unit to one wrong row.
        // build_guns takes type_id for the same reason.
        const GameUnitRow* row = units.unit_row(i);
        const int class_id = row != nullptr ? row->type_id : -1;
        if (class_id < 0) continue;
        // `ReconClass` is the integer field 00962... reads next to
        // `ReconModifier`; the row selects which ReconClass[] record the unit's
        // class+B4h table pointer ends up at. 637 of the installed rows carry
        // it, across the twelve ids the shipped table defines.
        const int recon_class = lua.read_vehicle_class_integer(class_id, "ReconClass",
            nullptr, -1);
        if (recon_class < 0) {
            ++recon_classes_missing;
        } else {
            unit_recon_record[i] = recon_record_for_class(recon_class);
        }
        // 009623A9 with the 009623AE FLD1 default, squared at 009623CF.
        const float sentinel = -1.0f;
        const float modifier = lua.read_vehicle_class_number(class_id, "ReconModifier",
            sentinel);
        if (modifier == sentinel) {
            ++recon_modifier_absent;
            unit_recon_modifier_sq[i] = bsp::kGunneryReconModifierSqDefault;
        } else {
            unit_recon_modifier_sq[i] = modifier * modifier;
        }
    }
}

namespace {

// Everything 008048A0 and 00806840 read, answered out of the gunnery host's own
// unit state. One method per native read; nothing is defaulted silently.
class GunneryReconSensorPassHost final : public bsp::ReconSensorPassHost {
public:
    explicit GunneryReconSensorPassHost(GameGunneryHost::Impl& owner) : owner_(owner) {}

    std::size_t unit_count() override { return owner_.units.count(); }

    bool unit_present(std::size_t index) override {
        // Rule (b), the same 0043F080 gate bytes the contact sweep uses, plus
        // the world-registry membership the scan walks.
        if (!owner_.units.unit_alive_and_visible(index)) return false;
        if (!owner_.units.unit_active(index)) return false;
        if (index < owner_.unit_state.size() && owner_.unit_state[index].dead) return false;
        return true;
    }

    int unit_side(std::size_t index) override { return owner_.units.unit_side_0054(index); }

    bool unit_is_observer_class(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQueryUnit);
    }
    bool unit_is_unit_base(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQueryUnit);
    }
    bool unit_is_submarine(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQuerySubmarine);
    }
    bool unit_is_surface_target(std::size_t index) override {
        // 00922DC0's thunk into 00922C80, filled from the same facts the ship
        // AI's target_is_surface_00922dc0 fills. The set branch 008DDF90 is not
        // built in this process, so its tail is taken, as there.
        bsp::EntityTargetFacts tf;
        tf.present = true;
        tf.not_engageable = owner_.units.unit_flag_005d(index);
        tf.is_plane = owner_.units.unit_is_kind_of(index, 0x0f);
        tf.is_plane_squadron = owner_.units.unit_is_kind_of(index, 0x18);
        tf.is_ship_family = owner_.units.unit_is_kind_of(index, 0x06);
        tf.is_submarine = owner_.units.unit_is_kind_of(index, 0x08);
        tf.is_airfield = owner_.units.unit_is_kind_of(index, 0x45);
        tf.is_shipyard = owner_.units.unit_is_kind_of(index, 0x46);
        tf.is_command_building = owner_.units.unit_is_kind_of(index, 0x1c);
        tf.is_dummy_target = owner_.units.unit_is_kind_of(index, 0x35);
        tf.is_land_fort = owner_.units.unit_is_kind_of(index, 0x1b);
        float px = 0.0f, py = 0.0f, pz = 0.0f;
        owner_.units.unit_position_00fc(index, px, py, pz);
        tf.world_y = py;
        const bsp::SurfaceTargetAnswer answer = bsp::entity_is_surface_target_00922c80(tf);
        if (answer == bsp::SurfaceTargetAnswer::kUnreadSetBranch) {
            return bsp::entity_surface_target_tail_00922c80(tf, true);
        }
        return answer == bsp::SurfaceTargetAnswer::kYes;
    }

    bsp::SensorCategory unit_sensor_category(std::size_t index) override {
        // unit->[+1E4h]->vtable[1](). The installed getters are constants, so
        // the answer is the unit's kind: 0074E190 air for the plane base,
        // 006DFD20 surface for the ship base and its three land siblings
        // (0074DD50, 006D1D30, 006F57B0), 004F1740 unclassified otherwise.
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitKindQuerySubmarine)) {
            // 00852B90's three states depend on the periscope and on depth
            // bands this process does not read; the periscope half is
            // reconstructed and answers periscope_in for a stowed periscope,
            // which is the state a submarine that nothing has raised is in.
            owner_.record("Recon::submarine_sensor_state_00852b90", 0x00852b90u);
            return bsp::submarine_periscope_sensor_state(false);
        }
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitGunneryKindPlaneBase)) {
            return bsp::kSensorCategoryPlane_0074e190;
        }
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitGunneryKindShipBase) ||
            owner_.units.unit_is_kind_of(index, 0x45) ||
            owner_.units.unit_is_kind_of(index, 0x1b)) {
            return bsp::kSensorCategoryShip_006dfd20;
        }
        return bsp::kSensorCategoryDefault_004f1740;
    }

    void unit_world_xz(std::size_t index, float& x, float& z) override {
        float y = 0.0f;
        owner_.units.unit_position_00fc(index, x, y, z);
    }

    float unit_heading(std::size_t index) override {
        return owner_.units.unit_heading_radians(index);
    }

    float unit_recon_modifier_sq(std::size_t index) override {
        if (index >= owner_.unit_recon_modifier_sq.size()) {
            return bsp::kGunneryReconModifierSqDefault;
        }
        return owner_.unit_recon_modifier_sq[index];
    }

    const bsp::GunneryReconSensorRow* unit_sensor_rows(std::size_t index,
        std::size_t& count) override {
        count = 0;
        if (index >= owner_.unit_recon_record.size()) return nullptr;
        const GameGunneryHost::Impl::ReconClassRecord* record =
            owner_.unit_recon_record[index];
        if (record == nullptr) return nullptr;  // 008048C1's early false
        count = bsp::kSensorListCount;
        return record->rows.data();
    }

    float unit_environment_factor(std::size_t) override {
        // 008E6430(0Ch, observer) is gated on [00E0C978] and [[00F88C30]+118h].
        // This process builds no gameplay-modifier list, which is the same
        // empty-list 1.0f the hit path already takes at 008E6430.
        return bsp::kReconSensorDefaultFactor;
    }

    bool unit_detection_forced(std::size_t) override { return false; }
    bsp::ReconDetectionLevel unit_forced_level(std::size_t) override {
        return bsp::ReconDetectionLevel::none;
    }

    float simplified_recon_multiplier() override {
        // [game+21C4h]+74h, 1.0f from 00444D20 unless a mission script called
        // 008B24A0. Neither USN01 nor USN02 does.
        return 1.0f;
    }
    float simplified_sonar_multiplier() override { return 1.0f; }

    int network_role() override {
        // [00E188A8]+1FE4h. This process runs the single originating session,
        // so the pass is never the kGunneryReconNonOriginatingRole skip.
        return 0;
    }

private:
    GameGunneryHost::Impl& owner_;
};

}  // namespace

void GameGunneryHost::Impl::step_recon_sensor_pass_008073c0(float frame_dt) {
    // 008079B0 BSP_Recon_ServicePeriodicRefresh's countdown at [00F874B8]:
    // subtract the frame delta, return while it is still positive, otherwise
    // reload by adding 3.0 (00D7A2B0), clamped at zero, and run the pass.
    // Stepping 008073C0 every frame instead would publish `none` for every
    // target, because 00805BE0 zeroes each record's value before the pass and
    // one 20 Hz frame of the shipped gain cannot reach the blip threshold.
    recon_refresh_timer -= frame_dt;
    if (recon_refresh_timer > 0.0f) return;
    recon_refresh_timer += bsp::kReconSensorPassRefreshPeriod;
    if (recon_refresh_timer < 0.0f) recon_refresh_timer = 0.0f;
    // 008073C1/008073CC/008073D6: dt is the measured gap since this slot's
    // previous rebuild, [00F876A4] minus slot+2Ch, not the frame delta.
    const float dt = clock_seconds - recon_last_pass_seconds;
    recon_last_pass_seconds = clock_seconds;
    resolve_recon_inputs();
    GunneryReconSensorPassHost host(*this);
    bsp::recon_sensor_pass_step_008073c0(recon_pass, dt, host);
    // The published answer per observing side, tallied over the pairs the pass
    // covered this tick.
    const std::size_t count = units.count();
    for (std::size_t observer = 0; observer < count; ++observer) {
        const int side = units.unit_side_0054(observer);
        if (!recon_pass.side_covered(side)) continue;
        bool seen = false;
        for (const ReconSideCensus& row : recon_side_census) {
            if (row.side == side) { seen = true; break; }
        }
        if (seen) continue;
        ReconSideCensus row;
        row.side = side;
        recon_side_census.push_back(row);
    }
    for (ReconSideCensus& row : recon_side_census) {
        for (std::size_t target = 0; target < count; ++target) {
            if (units.unit_side_0054(target) == row.side) continue;
            if (!host.unit_present(target)) continue;
            switch (recon_pass.level(row.side, target)) {
                case bsp::ReconDetectionLevel::none: ++row.none_level; break;
                case bsp::ReconDetectionLevel::blip: ++row.blip; break;
                case bsp::ReconDetectionLevel::identified: ++row.identified; break;
            }
        }
    }
    done("Recon::sensor_pass", 0x008073c0u);
    done("Recon::evaluate_sensors", 0x008048a0u);
}

void GameGunneryHost::Impl::run_gunnery_pass(std::size_t index, float dt) {
    UnitState& state = unit_state[index];
    if (!state.attached) return;

    // The director's two targets, as this process holds them: 00863640 reads
    // director+238h, which the automatic target think 009F5DA0 wrote through
    // 00835860, and 0071EBF0 takes the newest queued command's target.
    state.fire_target = 0;
    state.command_target = 0;
    // 0071EBF0 answers with the newest QUEUED COMMAND's target. Until now this
    // host had no queued commands to answer with, so the field stayed 0 and
    // step 8.7's first arm was always skipped. The script-order path now issues
    // real commands that reach a weapon director, so the answer exists: take the
    // unit's current command row and resolve its target by name, exactly as the
    // fire-target arm below resolves its own.
    refresh_command_targets();
    if (index < command_target_by_unit.size()) {
        state.command_target = command_target_by_unit[index];
    }
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

    // Packet cc8_torpedo_gun_assignment. The torpedo category is cut out of the
    // recon sweep by the image itself at 008651F5 (`CMP ESI,7 / JE 00865442`),
    // so on a unit that carries a torpedo-category gun the only way a candidate
    // can appear is step 8.7's two director targets. Counting how often either
    // exists is the first question, before any guard inside 00863990 matters.
    if (!state.category_guns[
            static_cast<std::size_t>(bsp::kUnitGunneryTorpedoCategory)].empty()) {
        ++summary.torpedo_cat_pass_ticks;
        if (state.command_target != 0) ++summary.torpedo_cat_with_command_target;
        if (state.fire_target != 0) ++summary.torpedo_cat_with_fire_target;
    }

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
        // Packet cc8_torpedo_release_spawn. The torpedo gate census: the same
        // conjuncts as `want_fire` below, restricted to guns whose round can
        // swim. Read as a funnel - the first counter that collapses names the
        // gate that keeps torpedoes out of the water.
        const bool torpedo_gun = gun.swim_speed > 0.0f;
        if (torpedo_gun) {
            ++summary.torpedo_gun_ticks;
            if (have_target) {
                ++summary.torpedo_gun_targeted;
                if (accepted) {
                    ++summary.torpedo_gun_accepted;
                    if (settled) {
                        ++summary.torpedo_gun_settled;
                        if (may_fire_here) ++summary.torpedo_gun_window;
                    }
                }
            }
        }
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
        if (torpedo_gun && sent) ++summary.torpedo_gun_sent;
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
        if (torpedo_gun) ++summary.torpedo_gun_shots;
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

        // TRACE, packet cc8_torpedo_swim item 1. Additive logging only: follow
        // every dropped torpedo from its drop line's id until it leaves the
        // list, and name the exit that took it. Nothing here changes a value.
        const bool trace = shot.torpedo_trace_id != 0;
        if (trace) {
            log.notef("  torpedo trace %llu t=%.2f life=%.2f from_y=%.2f "
                "pos=(%.1f,%.2f,%.1f) vel=(%.1f,%.2f,%.1f) swimming=%d",
                shot.torpedo_trace_id, static_cast<double>(clock_seconds),
                static_cast<double>(shot.life), static_cast<double>(from[1]),
                static_cast<double>(shot.position[0]),
                static_cast<double>(shot.position[1]),
                static_cast<double>(shot.position[2]),
                static_cast<double>(shot.flight.velocity.x),
                static_cast<double>(shot.flight.velocity.y),
                static_cast<double>(shot.flight.velocity.z),
                shot.swimming ? 1 : 0);
        }

        // Packet cc8_torpedo_closest_approach. A swimming round carries no
        // target - GameProjectileRow has an owner and no victim, and the swim
        // keeps its launch heading - so the closest approach is measured
        // against every unit of another side rather than against an assumed
        // target. Horizontal only: the round is levelled onto the surface
        // plane at 009D0CE0's swim and a ship's y is its waterline.
        if (shot.swimming) {
            const std::size_t unit_count = units.count();
            for (std::size_t i = 0; i < unit_count; ++i) {
                if (i + 1 == shot.owner_unit) continue;
                if (units.unit_side_0054(i) == shot.owner_side) continue;
                float ux = 0.0f, uy = 0.0f, uz = 0.0f;
                units.unit_position_00fc(i, ux, uy, uz);
                const float dx = shot.position[0] - ux;
                const float dz = shot.position[2] - uz;
                const float d = std::sqrt(dx * dx + dz * dz);
                if (shot.min_enemy_distance < 0.0f || d < shot.min_enemy_distance) {
                    shot.min_enemy_distance = d;
                    shot.min_enemy_time = shot.life;
                    shot.min_enemy_unit = i + 1;
                }
                // Packet cc8_torpedo_aim_census: the same minimum against the
                // ORDERED target, kept separately because the nearest unit and
                // the aimed-at unit need not be the same ship.
                if (shot.ordered_target == i + 1 &&
                    (shot.ordered_min_distance < 0.0f ||
                     d < shot.ordered_min_distance)) {
                    shot.ordered_min_distance = d;
                    shot.ordered_min_time = shot.life;
                    shot.target_pos_at_min[0] = ux;
                    shot.target_pos_at_min[1] = uy;
                    shot.target_pos_at_min[2] = uz;
                    // The round's track against the target's course. Both are
                    // the game's compass convention (0 = +Z, pi/2 = +X), so the
                    // difference is taken with the same wrap the planner uses.
                    const float track = std::atan2(shot.flight.velocity.x,
                                                   shot.flight.velocity.z);
                    const float course = units.unit_heading_radians(i);
                    shot.crossing_angle =
                        std::fabs(bsp::wrapped_angle_subtract_00438b10(track, course));
                }
            }
        }

        const bsp::TickPoint3 a{from[0], from[1], from[2]};
        const bsp::TickPoint3 b{shot.position[0], shot.position[1], shot.position[2]};
        if (!bsp::projectile_segment_is_sweepable(a, b)) {
            if (trace) {
                log.notef("  torpedo trace %llu skip=not_sweepable",
                    shot.torpedo_trace_id);
            }
            continue;
        }
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
            if (trace) {
                log.notef("  torpedo trace %llu exit=entity_impact hit=%s "
                    "at=(%.1f,%.2f,%.1f) life=%.2f",
                    shot.torpedo_trace_id,
                    unit_name_or_index(query.hit_unit).c_str(),
                    static_cast<double>(point[0]), static_cast<double>(point[1]),
                    static_cast<double>(point[2]),
                    static_cast<double>(shot.life));
            }
            done("Projectile::on_impact_0084bc60", 0x0084bc60u);
            apply_hit(shot.owner_unit - 1, shot.gun_row, query.hit_unit - 1, point,
                direction);
            // 0084BC60 step 7: the impact also spawns the burst that carries a
            // torpedo's warhead. Without it a torpedo does its DamageMin draw
            // and nothing else, which a carrier's Armour cancels exactly.
            //
            // The image's gate is the class descriptor's +6Ch and nothing else,
            // so this is NOT torpedo-only: every class with a Blast table
            // bursts on this step, and this loop carries bombs and shells too.
            // apply_impact_blast returns without doing anything when the row
            // has no Blast, which is that same gate.
            apply_impact_blast(shot.owner_unit - 1, shot.gun_row, point, direction);
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
            // 008568E0's first limit: the round breaks up if it hits the sea
            // faster than `MaxWaterHitVel` (classDesc+0DCh). This is why a
            // torpedo bomber has to release low and slow. The second limit,
            // `shot[+0Ch] < -classDesc[+0ECh]`, is the MaxFall-derived fall
            // speed against the round's own depth field and is NOT applied
            // here: this host has no such field on the shot.
            // docs/TORPEDO_TICK.md, docs/TORPEDO_RELEASE_SPAWN.md.
            const float entry_velocity[3] = {shot.flight.velocity.x,
                shot.flight.velocity.y, shot.flight.velocity.z};
            const float entry_speed = length3(entry_velocity);
            const float hit_limit = entry != nullptr ? entry->max_water_hit_vel : 0.0f;
            if (trace) {
                log.notef("  torpedo trace %llu water_crossing from_y=%.3f to_y=%.3f "
                    "life=%.2f swim=%.1f hit_limit=%.1f entry_speed=%.1f swimming=%d",
                    shot.torpedo_trace_id, static_cast<double>(from[1]),
                    static_cast<double>(shot.position[1]),
                    static_cast<double>(shot.life), static_cast<double>(swim),
                    static_cast<double>(hit_limit),
                    static_cast<double>(entry_speed), shot.swimming ? 1 : 0);
            }
            if (swim > 0.0f && !shot.swimming && hit_limit > 0.0f
                && entry_speed > hit_limit) {
                ++summary.water_entry_breakups;
                if (summary.water_entry_breakups <= 4) {
                    log.notef("gunnery: water entry broke the round up at %.1f m/s, "
                        "MaxWaterHitVel %.1f m/s (008568E0)",
                        static_cast<double>(entry_speed),
                        static_cast<double>(hit_limit));
                }
                ++summary.impacts_static;
                shot.alive = false;
                continue;
            }
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
                if (trace) {
                    log.notef("  torpedo trace %llu swim_started life=%.2f "
                        "pos=(%.1f,%.2f,%.1f)", shot.torpedo_trace_id,
                        static_cast<double>(shot.life),
                        static_cast<double>(shot.position[0]),
                        static_cast<double>(shot.position[1]),
                        static_cast<double>(shot.position[2]));
                }
                continue;
            }
            ++summary.impacts_static;
            if (trace) {
                log.notef("  torpedo trace %llu exit=water_static life=%.2f",
                    shot.torpedo_trace_id, static_cast<double>(shot.life));
            }
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
            if (trace) {
                log.notef("  torpedo trace %llu exit=expired life=%.2f speed=%.1f "
                    "range=%.1f travelled=%.1f", shot.torpedo_trace_id,
                    static_cast<double>(shot.life), static_cast<double>(speed),
                    static_cast<double>(range),
                    static_cast<double>(shot.life * speed));
            }
            shot.alive = false;
        }
    }
    // Packet cc8_torpedo_closest_approach: keep a dying swimming round's closest
    // approach before the row is erased, because nothing else outlives it.
    for (const GameProjectileRow& row : shots) {
        if (row.alive || !row.swimming) continue;
        GameTorpedoApproachRow rec;
        rec.owner_name = unit_name_or_index(row.owner_unit);
        rec.nearest_name = unit_name_or_index(row.min_enemy_unit);
        rec.min_distance = row.min_enemy_distance;
        rec.min_time = row.min_enemy_time;
        rec.life_at_end = row.life;
        fill_ordered_fields(rec, row);
        torpedo_approaches.push_back(rec);
    }
    // Packet cc8_dive_glide, the same shape one hunk up and for the same
    // reason: a bomb's impact has to be taken before the row is erased.
    for (const GameProjectileRow& row : shots) {
        if (row.alive || !row.is_bomb) continue;
        GameBombImpactRow rec;
        rec.owner_name = unit_name_or_index(row.owner_unit);
        for (int i = 0; i < 3; ++i) {
            rec.predicted[i] = row.predicted_impact[i];
            rec.actual[i] = row.position[i];
            rec.target_release[i] = row.target_pos_release[i];
        }
        const float pdx = rec.actual[0] - rec.predicted[0];
        const float pdz = rec.actual[2] - rec.predicted[2];
        rec.predicted_error = std::sqrt(pdx * pdx + pdz * pdz);
        if (row.ordered_target != 0) {
            const float tdx = rec.actual[0] - rec.target_release[0];
            const float tdz = rec.actual[2] - rec.target_release[2];
            rec.target_error = std::sqrt(tdx * tdx + tdz * tdz);
        }
        rec.life = row.life;
        rec.died_above_water = row.position[1] > 0.0f;
        bomb_impacts.push_back(rec);
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

    // One AddDamage, the `0.0 < damage` test at 008778C6 / 00877A2E and the
    // 00879070 -> 00877B90 write both passes share.
    bool add_damage(float damage) {
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
        return true;
    }

    bool apply_base_hit_record() override {
        // 008777D0, the base hit record: the hull pass and the part pass. The
        // hull damage reaches the unit's own health through 00879070.
        //
        // The image gates the hull pass on `hit+34h != -1` (OR EDI,0FFFFFFFFh at
        // 008777DA, CMP [EBX+34h],EDI and JZ 008778D8 at 008777DD/E2). This host
        // does NOT reproduce that gate, deliberately: every hull segment index
        // the reconstruction has read is -1, so the gate as read would zero all
        // gunnery damage. docs/EXPLOSION_RADIAL_DAMAGE.md marks the producer of
        // a non-negative +34h as a labelled gap. Because the hull pass here is
        // ungated, a record that is meant to carry its damage in the part pass
        // must leave +14h at zero; apply_torpedo_blast does.
        const float armour = owner_.unit_state[victim_].armour;
        const float damage = bsp::hull_damage_00470510(hit_, armour);
        owner_.done("ShipHit::base_hit_record_008777d0", 0x008777d0u);
        bool applied_any = add_damage(damage);
        if (applied_any) ++owner_.summary.hull_damages;

        // The part pass, 008778D8..00877A43: one 004705C0 per entry of the
        // array at +3Ch, each floored at zero inside the formula and added
        // through the same 00879070 the hull pass uses (AddDamage at 00877A37).
        // A blast record carries its damage here, in +28h.
        for (int i = 0; i < hit_.part_hit_count; ++i) {
            const float part = bsp::part_damage_004705c0(hit_, armour, i);
            owner_.done("ShipHit::part_damage_004705c0", 0x004705c0u);
            if (add_damage(part)) applied_any = true;
        }
        return applied_any;
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
    std::size_t victim, const float point[3], const float direction[3],
    const bsp::HitRecord* blast_record) {
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

    // A blast record replaces the direct-hit one whole: 0084BAD0 builds its own
    // record per gathered entity (+28h the burst damage, +24h the radius, +2Ch
    // the ignore-falloff byte) and the direct DamageMin/DamageMax draw plays no
    // part in it.
    if (blast_record != nullptr) hit = *blast_record;

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

// 0084BC60 step 7, read from the listing at 0084BE25..0084BEE3.
//
//   MOV EAX,[ESI+8]            ; the projectile's weapon class descriptor
//   CMP byte ptr [EAX+6Ch],0   ; the "has blast" flag
//   JZ  0084BEE8               ; no Blast table, no burst
//   FLD  float ptr [EAX+0B8h]  ; BlastDamageMax
//   FLD  float ptr [EAX+0B4h]  ; BlastDamageMin
//   CALL 00BD2F10              ; a uniform draw between them
//   FSTP float ptr [ESP+84h]   ; -> the &damage the 0084BAD0 contract takes
//   LEA  EDX,[EAX+70h]         ; -> &radius, BlastRange, one float, not a draw
//   ... hitPos - direction * [00D7A270]   ; the burst centre, 00D7A270 = 0.05
//   CALL 0084BAD0
//
// So a torpedo's warhead is its Blast sub-table, not its DamageMin/DamageMax.
// For bullet class 69 (the Kate's "17.7 Type 91 Mod3 airplane torpedo") this
// installation's arcade table gives BlastDamageMin = BlastDamageMax = 1200 and
// BlastRange = 50, against a DamageMin/DamageMax of 50 -- and the Lexington's
// Armour is 50, so the contact damage is exactly (50 - 50) = 0 and the whole
// warhead is in the burst. docs/TORPEDO_WARHEAD.md.
//
// 0084BAD0 gathers one record per collision node inside the sphere (00904470
// -> 0098C630) and queues each with +28h = damage and +24h = radius. Two
// stand-ins here, both labelled: the gather is over this host's hull boxes
// rather than the image's shape tree, and the per-record distance that feeds
// 004705C0's falloff has no read producer in the image at all -- the array at
// +3Ch is docs/EXPLOSION_RADIAL_DAMAGE.md's labelled gap. The falloff
// arithmetic is 004705C0's; the distance handed to it is this host's.
void GameGunneryHost::Impl::apply_impact_blast(std::size_t shooter,
    std::size_t gun_row, const float point[3], const float direction[3]) {
    if (gun_row >= guns.size() || shooter >= unit_state.size()) return;
    const GameGunRow& gun = guns[gun_row];
    const GameBulletClassRow* weapon = bullet(gun.bullet_class);
    if (weapon == nullptr || weapon->blast_range <= 0.0f) return;

    const float damage = random_range_00bd2f10(weapon->blast_damage_min,
        weapon->blast_damage_max);
    // The image backs the burst centre off along buffer+10h, which 0084BF00
    // writes as the **unit** direction of the swept segment (delta scaled by
    // the reciprocal of BSP_Vector3f_Length; docs/PROJECTILE_IMPACT.md's field
    // table calls it "the segment direction, normalised"). The caller here
    // hands over the shot's velocity, so it must be normalised first: at a
    // shell's 700 m/s the raw vector would put the burst 35 m back down the
    // flight path instead of 5 cm out of the surface.
    float centre[3] = {point[0], point[1], point[2]};
    const float speed = std::sqrt(direction[0] * direction[0]
        + direction[1] * direction[1] + direction[2] * direction[2]);
    if (speed > 0.0f) {
        for (int i = 0; i < 3; ++i) {
            centre[i] -= (direction[i] / speed) * kBlastCentreBackOff;
        }
    }

    for (std::size_t i = 0; i < unit_state.size(); ++i) {
        if (i == shooter) continue;  // 0084BBF9 skips the burst's own source
        UnitState& state = unit_state[i];
        if (state.dead) continue;

        // The distance from the burst centre to the unit's hull box, in the
        // hull's own frame: the same slab SegmentBinding::shape_trace_segment
        // sweeps, so a round that struck the hull bursts at distance zero.
        float right[3], up[3], forward[3], origin[3];
        unit_pose(i, right, up, forward, origin);
        const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
            state.hull_length * 0.5f};
        if (extents[0] <= 0.0f || extents[2] <= 0.0f) continue;
        const float rel[3] = {centre[0] - origin[0], centre[1] - origin[1],
            centre[2] - origin[2]};
        const float* axes[3] = {right, up, forward};
        float outside = 0.0f;
        for (int a = 0; a < 3; ++a) {
            const float excess = std::fabs(dot3(rel, axes[a])) - extents[a];
            if (excess > 0.0f) outside += excess * excess;
        }
        const float distance = std::sqrt(outside);
        if (distance > weapon->blast_range) continue;

        bsp::HitPartEntry entry;
        entry.kind = 0;
        entry.part_index = 0;
        entry.distance = distance;
        bsp::HitRecord blast;
        blast.hull_damage_base = 0.0f;  // the burst's damage is +28h, see above
        blast.part_damage_base = damage;
        blast.falloff_range = weapon->blast_range;
        blast.ignore_falloff = false;
        blast.armour_selector = 0.0f;
        blast.hull_segment = kDirectHitHullSegment;
        blast.weapon_scale = 1.0f;
        blast.owner_modifier = 1.0f;
        blast.part_hits = &entry;
        blast.part_hit_count = 1;

        const float before = state.health;
        apply_hit(shooter, gun_row, i, point, direction, &blast);
        log.notef("  impact blast bullet=%d on %s dist=%.1f base=%.1f range=%.1f "
            "armour=%.1f took=%.1f health=%.1f",
            gun.bullet_class,
            unit_name_or_index(i + 1).c_str(), static_cast<double>(distance),
            static_cast<double>(damage),
            static_cast<double>(weapon->blast_range),
            static_cast<double>(state.armour),
            static_cast<double>(before - unit_state[i].health),
            static_cast<double>(unit_state[i].health));
    }
    done("Projectile::blast_radial_damage_0084bad0", 0x0084bad0u);
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

void GameGunneryHost::set_ship_ai(GameShipAiHost* ai) noexcept {
    impl_->ship_ai = ai;
    // Packet cc8_ship_ai_firepower_inputs: the reverse edge. 0095EB40 reads the
    // tables 00956C20 built (unit+394h, +430h, +494h and the category lists),
    // and this host is the only thing in the process that runs 00956C20.
    if (ai != nullptr) ai->bind_gunnery(this);
}

void GameGunneryHost::Impl::refresh_build_summary() {
    summary.guns = guns.size();
    summary.device_rows = devices.size();
    summary.bullet_rows = bullets.size();
    // A count over the current rows, not an accumulator, so it is recomputed
    // rather than added to when a later batch registers.
    summary.units_with_guns = 0;
    for (const UnitState& state : unit_state) {
        if (state.row.guns > 0) ++summary.units_with_guns;
    }
    built_units = units.count();
}

void GameGunneryHost::attach_00864bd0() {
    Impl& host = *impl_;
    host.build_rank_table();
    host.build_guns(0);
    host.attach_passes(0);
    host.refresh_build_summary();
    host.log.notef("gunnery: %zu unit(s) carry %zu gun(s) from %zu authored device class "
        "row(s) and %zu bullet class row(s); the weapon director think time is %.3f s "
        "(Globals.WeaponSystems.WeaponDirectorThinkTime, 0087e16b)",
        host.summary.units_with_guns, host.summary.guns, host.summary.device_rows,
        host.summary.bullet_rows, static_cast<double>(host.think_time));
}

void GameGunneryHost::register_new_units_00864bd0() {
    Impl& host = *impl_;
    const std::size_t first = host.built_units;
    const std::size_t count = host.units.count();
    if (count <= first) return;
    const std::size_t guns_before = host.guns.size();
    // The rank table is built from the compiled-in preference lists alone and
    // carries no per-unit state, so it is not rebuilt here.
    host.build_guns(first);
    host.attach_passes(first);
    host.refresh_build_summary();
    host.log.notef("gunnery: spawn batch registered unit(s) %zu..%zu on the existing host, "
        "%zu new gun(s) (%zu total); the summary, hit records and in-flight rounds of the "
        "%zu unit(s) already built are untouched",
        first, count - 1, host.guns.size() - guns_before, host.guns.size(), first);
}

void GameGunneryHost::fixed_step(float step_seconds) {
    Impl& host = *impl_;
    if (host.guns.empty()) return;
    host.clock_seconds += step_seconds;
    ++host.step_index;
    // 008073C0 runs once over every side before the per-unit pass, because it
    // is O(sides * observers * targets); running it inside the contact sweep
    // would repeat the whole pass once per firing unit.
    host.step_recon_sensor_pass_008073c0(step_seconds);
    for (std::size_t i = 0; i < host.unit_state.size(); ++i) {
        host.run_gunnery_pass(i, step_seconds);
    }
    host.run_gun_aim_and_fire(step_seconds);
    host.run_projectiles(step_seconds);
    // The rows are complete for this step here, after every per-unit pass and
    // the aim, fire and projectile passes have run. 00A08460 BSP_Ai_TargetWeight
    // reads the target's hit points and the attacker's barrels, and the AI
    // coordinator holds neither host, so the values are published into the
    // process-wide table it reads. docs/AI_TARGET_WEIGHT_TERMS.md term 2.
    host.publish_ai_weapon_facts();
}

void GameGunneryHost::Impl::publish_ai_weapon_facts() {
    GameAiWeaponFacts& facts = game_ai_weapon_facts();
    facts.reset();
    // target+48h, the hit points 00A08593 reads and 00A09737's epilogue divides
    // the accumulated damage by. The class maximum is what makes that a ratio.
    for (const UnitState& state : unit_state) {
        GameAiWeaponFacts::Unit& row = facts.row_for_write(state.row.unit_index);
        row.hit_points = state.row.max_health;
        // target+4Ch, read at 00A085A8. Nothing in this process produces a
        // capture state, so it stays zero and the model's capture accumulator
        // contributes nothing.
        row.capture_state = 0.0f;
    }
    // 00A095E3 walks the attacker's subsystems at +94h/+98h and their 48h-stride
    // barrel entries at +74h/+78h. This process has one gun row per gun and a
    // barrel count on it, so the barrels are flattened into one list per unit.
    for (const GameGunRow& gun : guns) {
        // A gun whose bullet class never resolved is not a weapon and must not
        // reach 00A08460's barrel walk. The case measured on IJN01 is the
        // CATAPULT: it is gunnery category 0Bh, one of the twelve, so a gun row
        // IS built for it here, but of the 416 device classes in this
        // installation the 20 CATAPULT rows are the only ones that author no
        // `Bullet` block, so gun.bullet_class stays -1 and the sub-type stays 0.
        // The image says the same thing from the other side: the authored
        // preference row for category 0Bh at 00E0A374 is EMPTY, so a catapult
        // targets nothing. Measured before this skip: 21020 barrel lookups on
        // Cruiser and 21020 on BattleShip, the two member classes in IJN01 that
        // mount one.
        //
        // This changes no weight. A sub-type 0 barrel answered accuracy 0 and
        // 00A094F5 already skipped it, so the totals are identical; what goes
        // away is a phantom barrel in the row and in the census.
        if (gun.bullet_sub_type == 0) {
            ++summary.ai_barrels_unresolved_skipped;
            continue;
        }
        GameAiWeaponFacts::Unit& row = facts.row_for_write(gun.unit_index);
        GameAiWeaponFacts::Barrel barrel;
        barrel.reload = gun.reload_time;
        // 0072AB80 BSP_GunClass_MuzzleCount at 00A09501 is the shots argument,
        // and BSP_Gun_SetupFromDescriptor stores its answer to gun+448h at
        // 0072E71A, which is exactly the field carried here as barrel_num. So
        // this one IS available; the earlier reading that it was not came from
        // mistaking the muzzle count for a barrel count.
        barrel.shots = gun.barrel_num > 0 ? gun.barrel_num : 1;
        // 009FE270 at 00A094E6 is not a stored accuracy: it is a lookup by
        // (bullet sub-type, target class group) into the AI mode tuning record,
        // so what the row owes is the selector and the lookup runs per target.
        // docs/AI_TARGET_WEIGHT_TERMS.md.
        barrel.bullet_sub_type = gun.bullet_sub_type;
        {
            // Resolvable is a property of the sub-type alone: every group of a
            // resolvable sub-type answers either an offset or a legitimate
            // zero. Asked with one group here purely to read the flag back.
            bool resolved = false;
            (void)bsp::ai_bullet_type_accuracy_offset_009fe270(
                barrel.bullet_sub_type, bsp::AiAccuracyTargetGroup::BigShip,
                resolved);
            barrel.accuracy_resolved = resolved;
        }
        row.barrels.push_back(barrel);
    }
    // A row is complete when every input 00A08460 reads is published. The
    // reload and the shot count always are; the accuracy is now reachable for
    // every sub-type but Rocket (12h), whose small/big split 009FE4F1 makes
    // through unread target-state predicates. A unit carrying any rocket barrel
    // therefore stays on the stand-in rather than scoring that barrel at zero.
    // Vacuously true for a unit with no guns: the barrels are the ATTACKER's
    // side of 00A08460 and the hit points above are the TARGET's, and the gate
    // asks both rows, so requiring barrels here would refuse every gunless
    // target and the model would never run on the static installations that
    // make up most of IJN01.
    for (GameAiWeaponFacts::Unit& row : facts.units) {
        // What the row itself now owes IS published: the reload, the shot count
        // and the bullet sub-type, for every sub-type but Rocket.
        bool row_inputs_published = true;
        for (const GameAiWeaponFacts::Barrel& barrel : row.barrels) {
            if (!barrel.accuracy_resolved) row_inputs_published = false;
        }
        // ... and the model is still NOT switched on, for a reason measured
        // rather than assumed. With `row_inputs_published` assigned straight to
        // the flag, IJN01 answers a zero weight for 460600 of its 465500
        // candidates - exactly its `fort_targets` count - so `scored` falls to
        // 4900 and `attackmove` 2250 and `settarget` 141 both go to 0 with all
        // 2450 served members taking the fallback moveto. The AI stops
        // attacking altogether. That is a regression against the stand-in and
        // it is 00A08460's own coverage, not this row's: its
        // attacker-is-type-0Fh branch 00A0861F..00A09222 is unprojected and
        // AiWeightModelBinding::entity_is_type and entity_kind are stubs.
        //
        // A first reading blamed the target hit points and was REFUTED by a
        // second run: gating on `hit_points > 0` left complete_rows at 321 and
        // changed no other number, so every row has real health.
        //
        // The collapse that held this at false is now traced and fixed: it was
        // ai_target_weights.cpp handing the barrel walk a null subsystem, so
        // barrel_count answered 0 and the loop never ran. Enabled here, and the
        // run that justifies it is in docs/AI_TARGET_WEIGHT_TERMS.md.
        row.inputs_complete = row_inputs_published;
    }
}

const std::vector<std::size_t>* GameGunneryHost::unit_category_guns(
    std::size_t unit_index, int category) const noexcept {
    if (unit_index >= impl_->unit_state.size()) return nullptr;
    if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return nullptr;
    return &impl_->unit_state[unit_index].category_guns[static_cast<std::size_t>(category)];
}

const GameBulletClassRow* GameGunneryHost::bullet_class_row(int id) const noexcept {
    return impl_->bullet(id);
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

bool GameGunneryHost::release_ordnance_drop(std::size_t unit_index) {
    Impl& h = *impl_;
    // The unit's torpedo-capable gun rows. `swim_speed > 0` is the same test the
    // water crossing uses to decide that a round swims instead of dying at the
    // surface, so a row selected here is exactly a row whose round can reach the
    // swim model.
    const GameGunRow* chosen = nullptr;
    for (const GameGunRow& gun : h.guns) {
        if (gun.unit_index != unit_index) continue;
        if (gun.swim_speed <= 0.0f) continue;
        chosen = &gun;
        break;
    }
    if (chosen == nullptr) {
        ++h.summary.torpedo_drop_refusals;
        return false;
    }

    float right[3], up[3], forward[3], origin[3];
    h.unit_pose(unit_index, right, up, forward, origin);
    // 0092D730 over the unit's body axis and linear velocity, not the cached
    // GameUnitRow field the Impl's own unit_velocity reads: that field is zero
    // for a plane, which made the first drop a pure free fall.
    const float speed = h.units.unit_forward_speed_0092d730(unit_index);
    const float velocity[3] = {forward[0] * speed, forward[1] * speed,
        forward[2] * speed};

    GameProjectileRow shot;
    shot.gun_row = static_cast<std::size_t>(chosen - h.guns.data());
    shot.owner_unit = unit_index + 1;
    shot.owner_side = h.units.unit_side_0054(unit_index);
    shot.bullet_class = chosen->bullet_class;
    shot.alive = true;
    for (int i = 0; i < 3; ++i) shot.position[i] = origin[i];
    // SUBSTITUTION, labelled: the release geometry. The native mount node and
    // the platform's own release slot are unread, so the round leaves from the
    // plane's origin along its forward axis at the plane's own forward speed.
    // A drop inherits the aircraft's velocity; it is not given a muzzle speed,
    // which is why `projectile_launch_velocity_006e8430` is not used here.
    shot.flight.velocity = bsp::TickPoint3{velocity[0], velocity[1], velocity[2]};
    shot.flight.snapshot_current = bsp::TickPoint3{origin[0], origin[1], origin[2]};
    shot.flight.local_position = shot.flight.snapshot_current;
    shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
    shot.flight.class_disables_gravity = false;
    // Packet cc8_torpedo_aim_census: pin the ordered target and where it was at
    // the drop, before the round is filed. Nothing downstream can recover this:
    // the round carries no victim and the target keeps moving.
    {
        const std::size_t owner = shot.owner_unit - 1;
        if (owner < h.command_target_by_unit.size()) {
            shot.ordered_target = h.command_target_by_unit[owner];
        }
        if (shot.ordered_target != 0) {
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            h.units.unit_position_00fc(shot.ordered_target - 1, tx, ty, tz);
            shot.target_pos_release[0] = tx;
            shot.target_pos_release[1] = ty;
            shot.target_pos_release[2] = tz;
            // Packet cc8_torpedo_retire item 5. The crossing geometry at the
            // DROP, in the same convention as the closest-approach one above:
            // unit_heading_radians is the unit's vtable[50h] heading, the hull
            // POSE row 2, for the aircraft as well as for the ship. For an
            // aircraft it is the same number the torpedo aim census prints as
            // yaw_C6C - that census prints yaw_C6C and hull_1050 side by side
            // and they agree to four decimals - so the aircraft's run-in
            // heading and the target's course are commensurable here, which
            // they would NOT be against the ship-ai step heading.
            shot.drop_owner_heading =
                h.units.unit_heading_radians(shot.owner_unit - 1);
            shot.drop_target_heading =
                h.units.unit_heading_radians(shot.ordered_target - 1);
            shot.drop_crossing_angle =
                std::fabs(bsp::wrapped_angle_subtract_00438b10(
                    shot.drop_owner_heading, shot.drop_target_heading));
        }
    }
    // TRACE, packet cc8_torpedo_swim item 1: the id on this round's own drop
    // line, carried on the row so the per-tick trace is keyed to it.
    shot.torpedo_trace_id = h.summary.torpedo_drops + 1;
    h.log.notef("  torpedo trace %llu spawn by %s at=(%.1f,%.2f,%.1f) "
        "vel=(%.1f,%.2f,%.1f) bullet=%d", shot.torpedo_trace_id,
        chosen->unit_name.c_str(), static_cast<double>(shot.position[0]),
        static_cast<double>(shot.position[1]),
        static_cast<double>(shot.position[2]),
        static_cast<double>(velocity[0]), static_cast<double>(velocity[1]),
        static_cast<double>(velocity[2]), chosen->bullet_class);
    h.shots.push_back(shot);
    ++h.summary.projectiles;
    ++h.summary.torpedo_drops;
    // Packet cc8_torpedo_breakoff: RESTORED, as half two of the spent-bomber
    // fix. A drop clears the owner's kind 2Bh bit, so approach+132h (== task+52Ah,
    // section 10.2) goes false on the next approach update.
    //
    // This was tried alone in 1e7c0f2f2 and reverted in c5235a9c6. The revert's
    // stated reason -- "009D3F60's entry chooser sends a task whose +52Ah is clear
    // straight to kDone" -- is WITHDRAWN by section 12: 009D3F60 has one call site
    // that cannot run after a drop (10.1), and the route that run took to `done`
    // is unexplained, not diagnosed. What the run did prove is that the clear
    // alone makes things worse, and section 12.4 gives the reason: the image
    // retires a spent bomber through 009D4C10, whose ordnance arm at 009D4C5C
    // (`CMP byte [ESI+52Ah],0` / `JNZ 009D4C1D`) is one of TWO halves. With
    // should_break_off still pinned false the clear had nothing to feed. It is
    // restored here only together with that binding in src/game_hosts_units.cpp.
    //
    // What the image consumes on a drop is still a HYPOTHESIS and stays one:
    // 009D34C5 refreshes approach+132h from 007B93F0(0) = 007B91C0(2Bh, 0), which
    // walks the controller's devices at ctl+974h, takes the one carrying kind 2Bh
    // and asks the ordnance object it returns `vtable[8](0)`. THAT body is unread.
    // The supporting evidence is a sibling predicate at 007B9426 testing a live
    // round count at ordnance+E0h, and the fact that 009D4030's goaway branch
    // reaches `done` only with the byte clear.
    //
    // The smallest faithful model: a drop consumes the unit's torpedo loadout, so
    // the kind bit clears. This host models no per-device round count, so it
    // cannot decrement one -- the count and its producer 006E3500 are unread here,
    // and "one torpedo per aircraft" is the assumption this makes explicit rather
    // than hides. USN01's Mavs release once each, which is consistent with it and
    // does not prove it. docs/TORPEDO_AFTER_THE_DROP.md sections 9, 12 and 13.
    {
        const std::size_t owner = shot.owner_unit - 1;
        const std::uint64_t mask = h.units.unit_ordnance(owner);
        const std::uint64_t torpedo_bit = std::uint64_t(1) << (0x2b - 0x08);
        if ((mask & torpedo_bit) != 0) {
            h.units.store_unit_ordnance(owner, mask & ~torpedo_bit);
            ++h.summary.torpedo_loadout_cleared;
        }
    }
    if (h.summary.torpedo_drops <= 4) {
        h.log.notef("gunnery: torpedo drop %llu by %s at %.0f m, speed %.1f m/s, "
            "bullet %d, swim %.1f m/s",
            h.summary.torpedo_drops, chosen->unit_name.c_str(),
            static_cast<double>(origin[1]), static_cast<double>(speed),
            chosen->bullet_class, static_cast<double>(chosen->swim_speed));
    }
    return true;
}

// Packet cc8_dive_glide, edited under the integrator's hunk arbitration of
// 2026-09-19. The bomb twin of release_ordnance_drop above: the same 0072F830
// spawn and the same substituted release geometry, with the kind 2Ah selection
// the dive bomber needs in place of the torpedo `swim_speed > 0` one. See the
// header for why the predicate was the whole defect.
bool GameGunneryHost::release_bomb_drop(std::size_t unit_index,
                                        const float predicted_impact[3]) {
    Impl& h = *impl_;
    const GameGunRow* chosen = nullptr;
    for (const GameGunRow& gun : h.guns) {
        if (gun.unit_index != unit_index) continue;
        if (!bsp::ordnance_has_general_bomb_2ah(gun.ordnance)) continue;
        chosen = &gun;
        break;
    }
    if (chosen == nullptr) {
        ++h.summary.bomb_drop_refusals;
        return false;
    }

    float right[3], up[3], forward[3], origin[3];
    h.unit_pose(unit_index, right, up, forward, origin);
    // Same substitution as the torpedo drop, and for the same reason: the
    // native mount node and the platform's release slot are unread, so the
    // round leaves from the plane's origin along its forward axis at the
    // plane's own forward speed. A bomb inherits the aircraft's velocity - it
    // is not given a muzzle speed - which is exactly the assumption 007BCC80's
    // fall time makes when it advances the aircraft by its own velocity.
    const float speed = h.units.unit_forward_speed_0092d730(unit_index);
    const float velocity[3] = {forward[0] * speed, forward[1] * speed,
        forward[2] * speed};

    GameProjectileRow shot;
    shot.gun_row = static_cast<std::size_t>(chosen - h.guns.data());
    shot.owner_unit = unit_index + 1;
    shot.owner_side = h.units.unit_side_0054(unit_index);
    shot.bullet_class = chosen->bullet_class;
    shot.alive = true;
    shot.is_bomb = true;
    for (int i = 0; i < 3; ++i) {
        shot.position[i] = origin[i];
        shot.predicted_impact[i] = predicted_impact[i];
    }
    shot.flight.velocity = bsp::TickPoint3{velocity[0], velocity[1], velocity[2]};
    shot.flight.snapshot_current = bsp::TickPoint3{origin[0], origin[1], origin[2]};
    shot.flight.local_position = shot.flight.snapshot_current;
    shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
    shot.flight.class_disables_gravity = false;
    {
        const std::size_t owner = shot.owner_unit - 1;
        if (owner < h.command_target_by_unit.size()) {
            shot.ordered_target = h.command_target_by_unit[owner];
        }
        if (shot.ordered_target != 0) {
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            h.units.unit_position_00fc(shot.ordered_target - 1, tx, ty, tz);
            shot.target_pos_release[0] = tx;
            shot.target_pos_release[1] = ty;
            shot.target_pos_release[2] = tz;
        }
    }
    h.shots.push_back(shot);
    ++h.summary.projectiles;
    ++h.summary.bomb_drops;
    // NOT DONE, deliberately: no kind 2Ah twin of the torpedo drop's 2Bh clear.
    // The torpedo side can clear its bit because one drop is its whole loadout;
    // a dive bomber's is a salvo of up to two per pass out of a stock this host
    // does not model per device (006E3500 is unread, the same hole
    // kDiveBombCarriedRoundsSubstitute names). Clearing 2Ah here would make
    // 009C7AFE's HasGeneralBombOrdnance false after the FIRST bomb and take the
    // aimdive's second release away with it - and the torpedo side's own
    // history is the warning: the bare clear was tried in 1e7c0f2f2 and
    // reverted in c5235a9c6. The unit-side `dive_bomb_rounds_remaining` is what
    // counts the stock down today.
    if (h.summary.bomb_drops <= 6) {
        h.log.notef("gunnery: bomb drop %llu by %s at %.0f m, speed %.1f m/s, "
            "bullet %d | predicted impact %.0f %.0f %.0f",
            h.summary.bomb_drops, chosen->unit_name.c_str(),
            static_cast<double>(origin[1]), static_cast<double>(speed),
            chosen->bullet_class,
            static_cast<double>(predicted_impact[0]),
            static_cast<double>(predicted_impact[1]),
            static_cast<double>(predicted_impact[2]));
    }
    return true;
}

const bsp::ReconSensorPassState& GameGunneryHost::recon_sensor_pass_state() const noexcept {
    return impl_->recon_pass;
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
    {
        // docs/RECON_SLOT_LISTS.md rule (c). The pass's own counters over the
        // run, then the published level per observing side, then the contact
        // drops rule (c) is responsible for.
        const bsp::ReconSensorPassState& pass = host.recon_pass;
        host.log.notef("summary mission recon sensor_pass passes=%llu observers=%llu "
            "targets=%llu forced=%llu no_table=%llu blip=%llu identified=%llu none=%llu "
            "classes=%zu class_missing=%llu modifier_absent=%llu suppressed=%d "
            "contact_reject_level=%llu",
            pass.passes, pass.observers_admitted, pass.targets_tested,
            pass.targets_skipped_forced, pass.no_sensor_table, pass.detected_blip,
            pass.detected_identified, pass.detected_none, host.recon_classes.size(),
            host.recon_classes_missing, host.recon_modifier_absent,
            pass.suppressed_by_network_role ? 1 : 0, s.contact_reject_recon_level);
        for (const Impl::ReconSideCensus& row : host.recon_side_census) {
            host.log.notef("  recon side %d levels none=%llu blip=%llu identified=%llu",
                row.side, row.none_level, row.blip, row.identified);
        }
        for (const auto& entry : host.recon_classes) {
            // Which ReconClass[] record each id resolved to, and how many of
            // the 56 lists 008082A0 actually filled. An id whose record is
            // empty is the loader's own answer for a class outside 1..12.
            std::size_t filled = 0, rows = 0;
            for (std::size_t i = 0; i < bsp::kSensorListCount; ++i) {
                if (entry.second->rows[i].count != 0) {
                    ++filled;
                    rows += entry.second->rows[i].count;
                }
            }
            host.log.notef("  recon class %d lists=%zu entries=%zu", entry.first,
                filled, rows);
        }
    }
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
    host.log.notef("summary mission gunnery command_targets units_with=%zu "
        "(0071EBF0's answer, step 8.7's first arm; 0 means that arm never runs)",
        host.command_targets_resolved);
    host.log.notef("summary mission gunnery torpedo_ranges_derived=%llu "
        "swims_started=%llu snaps=%llu bullet_ranges_derived=%llu "
        "base_tick_timers_live=%llu expired=%llu",
        s.torpedo_ranges_derived, s.torpedo_swims_started,
        s.torpedo_heading_snaps, s.bullet_ranges_derived,
        s.gun_pending_timers_live, s.gun_pending_timers_expired);
    {
        // Packet cc8_torpedo_release_spawn: the funnel that says where a round
        // that could swim actually stops. Counted over gun rows, so a unit with
        // several tubes contributes one entry per tube.
        unsigned long long torpedo_guns = 0;
        for (const GameGunRow& row : host.guns) {
            if (row.swim_speed > 0.0f) ++torpedo_guns;
        }
        host.log.notef("summary mission gunnery torpedo_gate guns=%llu ticks=%llu "
            "targeted=%llu accepted=%llu settled=%llu window=%llu sent=%llu shots=%llu",
            torpedo_guns, s.torpedo_gun_ticks, s.torpedo_gun_targeted,
            s.torpedo_gun_accepted, s.torpedo_gun_settled, s.torpedo_gun_window,
            s.torpedo_gun_sent, s.torpedo_gun_shots);
        host.log.notef("summary mission gunnery torpedo_drop drops=%llu refusals=%llu "
            "water_entry_breakups=%llu",
            s.torpedo_drops, s.torpedo_drop_refusals, s.water_entry_breakups);
        host.log.notef("summary mission gunnery torpedo_loadout_cleared=%llu "
            "(drops that cleared the owner's kind 2Bh bit, so approach+132h goes false)",
            s.torpedo_loadout_cleared);
        // TRACE, packet cc8_torpedo_swim item 1: a traced round still in the
        // list at mission end took no exit at all, which is its own answer.
        for (const GameProjectileRow& row : host.shots) {
            if (row.torpedo_trace_id == 0) continue;
            host.log.notef("  torpedo trace %llu STILL IN FLIGHT at mission end "
                "life=%.2f pos=(%.1f,%.2f,%.1f) vel=(%.1f,%.2f,%.1f) swimming=%d",
                row.torpedo_trace_id, static_cast<double>(row.life),
                static_cast<double>(row.position[0]),
                static_cast<double>(row.position[1]),
                static_cast<double>(row.position[2]),
                static_cast<double>(row.flight.velocity.x),
                static_cast<double>(row.flight.velocity.y),
                static_cast<double>(row.flight.velocity.z),
                row.swimming ? 1 : 0);
        }
    // Packet cc8_dive_glide: the bomb drops, and each round's impact against
    // the point approach+D8h/+E0h predicted for it at the release tick. The
    // predicted error is the one that scores the CCIP chain; the target error
    // scores the whole attack. Both are planar and centre to centre.
    {
        host.log.notef("summary mission gunnery bomb_drops=%llu refusals=%llu "
            "(kind 2Ah rows; a refusal means the unit carried no bomb platform)",
            s.bomb_drops, s.bomb_drop_refusals);
        std::vector<GameBombImpactRow> rows = host.bomb_impacts;
        host.log.notef("summary mission gunnery bomb_impacts=%zu", rows.size());
        for (const GameBombImpactRow& r : rows) {
            host.log.notef("  bomb from %-12s impact %.0f %.0f %.0f after %.2f s "
                "| vs predicted 009C7D71 %.0f %.0f %.0f = %.1f m "
                "| vs target at release = %.1f m | died %s",
                r.owner_name.c_str(),
                static_cast<double>(r.actual[0]), static_cast<double>(r.actual[1]),
                static_cast<double>(r.actual[2]), static_cast<double>(r.life),
                static_cast<double>(r.predicted[0]),
                static_cast<double>(r.predicted[1]),
                static_cast<double>(r.predicted[2]),
                static_cast<double>(r.predicted_error),
                static_cast<double>(r.target_error),
                r.died_above_water ? "above water (entity sweep)"
                                   : "at the sea surface");
        }
        for (const GameProjectileRow& row : host.shots) {
            if (!row.is_bomb) continue;
            host.log.notef("  bomb from %-12s still in flight at alt %.0f m",
                host.unit_name_or_index(row.owner_unit).c_str(),
                static_cast<double>(row.position[1]));
        }
    }
    // Packet cc8_torpedo_closest_approach: the measurement the torpedo stream
    // has owed since docs/TORPEDO_AFTER_THE_DROP.md section 2. Distances are
    // CENTRE TO CENTRE and horizontal - this host has no oriented hull box - so
    // read each against the target's own Length (Northampton's class row is
    // 180.0 m), not as a miss distance from the plating.
    {
        std::vector<GameTorpedoApproachRow> rows = host.torpedo_approaches;
        for (const GameProjectileRow& row : host.shots) {
            if (!row.swimming) continue;
            GameTorpedoApproachRow rec;
            rec.owner_name = host.unit_name_or_index(row.owner_unit);
            rec.nearest_name = host.unit_name_or_index(row.min_enemy_unit);
            rec.min_distance = row.min_enemy_distance;
            rec.min_time = row.min_enemy_time;
            rec.life_at_end = row.life;
            host.fill_ordered_fields(rec, row);
            rows.push_back(rec);
        }
        host.log.notef("summary mission gunnery torpedo_closest_approach swims=%zu "
            "(centre to centre, horizontal)", rows.size());
        for (const GameTorpedoApproachRow& r : rows) {
            host.log.notef("  torpedo from %-12s nearest %-14s min=%.1f m at t=%.2f s "
                "of %.2f s run | ordered %-14s min=%.1f m at t=%.2f s "
                "target_moved=%.1f m crossing=%.3f rad "
                "| at the drop: own_pose=%.4f target_pose=%.4f "
                "crossing=%.4f rad (%.1f deg)",
                r.owner_name.c_str(), r.nearest_name.c_str(),
                static_cast<double>(r.min_distance), static_cast<double>(r.min_time),
                static_cast<double>(r.life_at_end), r.ordered_name.c_str(),
                static_cast<double>(r.ordered_min_distance),
                static_cast<double>(r.ordered_min_time),
                static_cast<double>(r.target_travel),
                static_cast<double>(r.crossing_angle),
                static_cast<double>(r.drop_owner_heading),
                static_cast<double>(r.drop_target_heading),
                static_cast<double>(r.drop_crossing_angle),
                static_cast<double>(r.drop_crossing_angle) * 180.0 / 3.14159265358979323846);
        }
    }
    {
        // Packet cc8_torpedo_gun_assignment. Which category a swim-capable round
        // is actually mounted in, and whether the units that own a
        // TORPEDO-category gun ever get the director target that is their only
        // candidate source. Both questions are answered per run rather than
        // assumed from the category name.
        std::array<int, bsp::kUnitGunneryCategoryCount> cat_swim{};
        std::vector<std::size_t> torpedo_units;
        for (const GameGunRow& row : host.guns) {
            if (row.category < 0 || row.category >= bsp::kUnitGunneryCategoryCount) continue;
            const std::size_t slot = static_cast<std::size_t>(row.category);
            if (row.swim_speed > 0.0f) ++cat_swim[slot];
            if (row.category == bsp::kUnitGunneryTorpedoCategory
                && std::find(torpedo_units.begin(), torpedo_units.end(), row.unit_index)
                    == torpedo_units.end()) {
                torpedo_units.push_back(row.unit_index);
            }
        }
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            if (cat_swim[static_cast<std::size_t>(c)] == 0) continue;
            const char* name = bsp::gunnery_category_function_name(c);
            host.log.notef("  swim-capable guns in category %2d %-20s %d", c,
                name != nullptr ? name : "?", cat_swim[static_cast<std::size_t>(c)]);
        }
        std::size_t with_command = 0;
        for (const std::size_t u : torpedo_units) {
            if (u < host.command_target_by_unit.size()
                && host.command_target_by_unit[u] != 0) {
                ++with_command;
            }
        }
        host.log.notef("  TORPEDO-category owners=%zu, of which with a command "
            "target=%zu (their only candidate source: 008651F5 cuts category 7 "
            "out of the recon sweep)", torpedo_units.size(), with_command);
    }
        host.log.notef("summary mission gunnery torpedo_candidates pass_ticks=%llu "
            "command_target=%llu fire_target=%llu scored=%llu accepted=%llu "
            "reject unknown=%llu liveness=%llu class=%llu rank=%llu mask=%llu range=%llu",
            s.torpedo_cat_pass_ticks, s.torpedo_cat_with_command_target,
            s.torpedo_cat_with_fire_target, s.torpedo_cat_score_calls,
            s.torpedo_cat_score_accepted, s.torpedo_cat_reject_unknown,
            s.torpedo_cat_reject_liveness, s.torpedo_cat_reject_class,
            s.torpedo_cat_reject_rank, s.torpedo_cat_reject_mask,
            s.torpedo_cat_reject_range);
    }
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
