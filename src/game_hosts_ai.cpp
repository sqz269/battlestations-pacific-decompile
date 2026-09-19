// The AI coordinator's fixed-step tick, bound to this process's units.
// docs/AI_COORDINATOR_TICK.md. See include/bsp/game_hosts_ai.hpp for the
// addresses and for every stand-in this file makes.

#include "bsp/game_hosts_ai.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <utility>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bsp/ai_command_lifetime.hpp"
#include "bsp/ai_command_object.hpp"
#include "bsp/ai_close_attack_tick.hpp"
#include "bsp/ai_command_tick.hpp"
#include "bsp/ai_group_think.hpp"
#include "bsp/ai_planners.hpp"
#include "bsp/ai_tuning_globals.hpp"
#include "bsp/ai_target_weights.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/plane_squadron_entity.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/unit_gunnery_pass.hpp"

namespace bsp::game {

void GameObjectiveSets::reset() noexcept {
    for (std::size_t i = 0; i < kSlotCount; ++i) slots[i].clear();
    adds = unit_adds = unit_removes = rejected = 0;
}

GameObjectiveSets::Objective* GameObjectiveSets::add_objective(int slot,
    const std::string& name) {
    if (slot < 0 || static_cast<std::size_t>(slot) >= kSlotCount || name.empty()) {
        ++rejected;
        return nullptr;
    }
    std::vector<Objective>& list = slots[static_cast<std::size_t>(slot)];
    // 008DF2B0's lookup is a case-insensitive name compare over the set's list;
    // 008E1F80 creates only when the walk found nothing.
    for (Objective& o : list) {
        if (o.name.size() == name.size() &&
            _stricmp(o.name.c_str(), name.c_str()) == 0) {
            return &o;
        }
    }
    Objective made;
    made.name = name;
    list.push_back(std::move(made));
    ++adds;
    return &list.back();
}

bool GameObjectiveSets::add_unit(int slot, const std::string& name, std::size_t unit) {
    Objective* o = add_objective(slot, name);
    if (o == nullptr) return false;
    if (std::find(o->units.begin(), o->units.end(), unit) != o->units.end()) return false;
    o->units.push_back(unit);
    ++unit_adds;
    return true;
}

bool GameObjectiveSets::remove_unit(int slot, const std::string& name, std::size_t unit) {
    if (slot < 0 || static_cast<std::size_t>(slot) >= kSlotCount) return false;
    for (Objective& o : slots[static_cast<std::size_t>(slot)]) {
        if (!name.empty() && !(o.name.size() == name.size() &&
                               _stricmp(o.name.c_str(), name.c_str()) == 0)) {
            continue;
        }
        auto it = std::find(o.units.begin(), o.units.end(), unit);
        if (it != o.units.end()) {
            o.units.erase(it);
            ++unit_removes;
            return true;
        }
    }
    return false;
}

std::vector<std::size_t> GameObjectiveSets::units_in_slot(int slot) const {
    std::vector<std::size_t> out;
    if (slot < 0 || static_cast<std::size_t>(slot) >= kSlotCount) return out;
    for (const Objective& o : slots[static_cast<std::size_t>(slot)]) {
        for (const std::size_t unit : o.units) {
            if (std::find(out.begin(), out.end(), unit) == out.end()) out.push_back(unit);
        }
    }
    return out;
}

std::size_t GameObjectiveSets::total_units() const noexcept {
    std::size_t n = 0;
    for (std::size_t i = 0; i < kSlotCount; ++i) {
        for (const Objective& o : slots[i]) n += o.units.size();
    }
    return n;
}

GameObjectiveSets& game_objective_sets() noexcept {
    static GameObjectiveSets sets;
    return sets;
}

void GameAiWeaponFacts::reset() noexcept { units.clear(); }

const GameAiWeaponFacts::Unit* GameAiWeaponFacts::row(std::size_t unit) const noexcept {
    if (unit >= units.size()) return nullptr;
    return units[unit].known ? &units[unit] : nullptr;
}

GameAiWeaponFacts::Unit& GameAiWeaponFacts::row_for_write(std::size_t unit) {
    if (units.size() <= unit) units.resize(unit + 1u);
    units[unit].known = true;
    return units[unit];
}

std::size_t GameAiWeaponFacts::known_units() const noexcept {
    std::size_t n = 0;
    for (const Unit& u : units) {
        if (u.known) ++n;
    }
    return n;
}

GameAiWeaponFacts& game_ai_weapon_facts() noexcept {
    static GameAiWeaponFacts facts;
    return facts;
}

namespace {

// bsp::AiTargetWeightModelHost over the process-wide weapon-facts table, so
// 00A08460 BSP_Ai_TargetWeight runs for real as soon as something publishes a
// row. Every method names the native site it stands at. The entity pointers
// the key carries are this process's unit handles, index + 1.
// docs/AI_TARGET_WEIGHT_TERMS.md term 2.
class AiWeightModelBinding final : public bsp::AiTargetWeightModelHost {
public:
    // The target group is a callback because 009FE270 asks the target itself,
    // so the answer cannot be baked into the published row.
    using TargetGroupFn = std::function<bsp::AiAccuracyTargetGroup(std::size_t)>;
    AiWeightModelBinding(const GameAiWeaponFacts& facts, const bsp::AiModeTuning& tuning,
                         const bsp::AiTuningBlock& block, TargetGroupFn group)
        : facts_(facts), tuning_(tuning), block_(block), group_(std::move(group)) {}

    // 00A03B90 and 00A079B0, the memo map at 00F8A734. A memo only caches, so
    // skipping it changes no answer; the census counts the queries instead.
    bool memo_lookup(const bsp::AiTargetWeightKey&, float&) override { return false; }
    void memo_store(const bsp::AiTargetWeightKey&, float) override {}
    // 00A31DB0 at 00A08540. The forced rules come from the globals loader's
    // ForcedTargetWeightValues tail, which this process does not run.
    bool forced_rule_weight(const bsp::AiTargetWeightKey&, float&) override { return false; }
    const bsp::AiModeTuning& mode_tuning() override { return tuning_; }
    // Entity vtable +18h and +1Ch. Neither is the +5Ch class test, and neither
    // has a producer here, so the model takes its no-bonus arms.
    bool entity_is_type(const void*, int) override { return false; }
    int entity_kind(const void*) override { return 0; }

    float target_hit_points(const void* target) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(target));
        return row != nullptr ? row->hit_points : 0.0f;
    }
    float target_capture_state(const void* target) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(target));
        return row != nullptr ? row->capture_state : 0.0f;
    }
    // 00A095E3's subsystem walk is flattened into one barrel list per unit, so
    // the attacker has a single subsystem carrying every barrel.
    int subsystem_count(const void* attacker) override {
        return facts_.row(index_of(attacker)) != nullptr ? 1 : 0;
    }
    int barrel_count(const void* subsystem) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(subsystem));
        return row != nullptr ? static_cast<int>(row->barrels.size()) : 0;
    }
    float barrel_reload(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->reload : 0.0f;
    }
    int barrel_shots(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->shots : 0;
    }
    // 009FE270 at 00A094E6. Not a stored value: the published row carries the
    // bullet class record's +8h selector and the answer is a lookup into the AI
    // mode tuning record by (that selector, the target's class group).
    // docs/AI_TARGET_WEIGHT_TERMS.md.
    float barrel_accuracy(const void* subsystem, int barrel, const void* target) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        if (b == nullptr || !b->accuracy_resolved || !group_) return 0.0f;
        bool resolved = false;
        const std::uint32_t offset = bsp::ai_bullet_type_accuracy_offset_009fe270(
            b->bullet_sub_type, group_(index_of(target)), resolved);
        // A zero offset is the reject arm, which is a real answer of no
        // accuracy, and 00A094F5 then skips the barrel exactly as the native
        // does.
        if (!resolved || offset == 0u) return 0.0f;
        return block_.at(offset);
    }
    // 009FE200 at 00A09578 and 00424C40+3B0h at 00A09624, both unread, so both
    // stand in neutral.
    //
    // This one used to `return a`, on the reading that `a` was the value being
    // scaled and returning it kept the value undiminished. It is not: the call
    // site multiplies by the answer and passes `(0, 0, 0, 0)`, four distance
    // arguments this projection has not recovered. So the stub answered 0 and
    // annihilated the term, `best` could never leave 0, and the barrel loop
    // contributed nothing to `total` even once the subsystem handle was fixed.
    // A falloff is a multiplier, so its neutral value is 1.0f. Labelled: the
    // real falloff is a function of the four distances and diminishes with
    // range, so this over-states a distant barrel.
    float distance_falloff(float, float, float, float) override { return 1.0f; }
    float capture_scale() override { return 1.0f; }

private:
    static std::size_t index_of(const void* entity) {
        return reinterpret_cast<std::size_t>(entity) - 1u;
    }
    const GameAiWeaponFacts::Barrel* barrel_at(const void* subsystem, int barrel) const {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(subsystem));
        if (row == nullptr || barrel < 0) return nullptr;
        if (static_cast<std::size_t>(barrel) >= row->barrels.size()) return nullptr;
        return &row->barrels[static_cast<std::size_t>(barrel)];
    }

    const GameAiWeaponFacts& facts_;
    const bsp::AiModeTuning& tuning_;
    const bsp::AiTuningBlock& block_;
    TargetGroupFn group_;
};

// 004BCA50 BSP_Game_GetEffectiveGameMode returns [world+614h], remapped by the
// 004BCA5F/004BCA68 arms. This process runs a single-player campaign mission,
// which is mode 0, and mode 0 is what makes ai_party_think_mode answer
// GroupWalk and 009FFE50 admit party slots 0 and 4 only. The value is a
// labelled substitution: the world singleton's +614h has no producer here.
constexpr int kCampaignGameMode = 0;

// 00A2C790's per-member chain reaches the member's own weapon director through
// the ENTITY's vtable[+114h] and reports its state back to the group's AI
// command, whose vt+24h (00A0FC90) discards it. Neither 00A10890 nor 00A109B0
// builds a scene command, and no class in the block acts on the report, so
// issuing a real order at order_attack stays this file's substitution for the
// per-class tick at vt+0Ch, which was not read.

}  // namespace

struct GameAiCoordinatorHost::Impl : public bsp::AiGroupThinkHost,
                                     public bsp::AiPlannerHost,
                                     public bsp::AiCommandMemberPassHost,
                                     public bsp::AiCommandTickHost,
                                     public bsp::AiCloseAttackTickHost {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in) : log(log_in), units(units_in) {}

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

    // --- the records every void* in the two host interfaces points at -------

    struct Planner;

    struct Group {
        int team{0};                  // group+5638h, the seed entity's +54h
        int party{0};                 // the party slot this process files it under
        std::vector<std::size_t> members;   // the +563Ch list, by unit index
        Planner* claimed_by{nullptr}; // group+5654h
        // group+564Ch. Every group carries one from its constructor, so the
        // object is held by value and `has_command` is not a separate state.
        bsp::AiCommandObject command{};
        float member_pass_due{0.0f};  // group+5650h, 0.0f from the constructor
        bool emptied{false};
        bool destroyed{false};
        std::string tag;              // the planner spawn tag, when one made it
        float leader_position[3]{0.0f, 0.0f, 0.0f};  // the +C8h cache
        double leader_order_key{0.0}; // 009FFD70 on the leader
        const std::vector<Group*>* walk_list{nullptr};  // the list a walk found it in
        std::size_t walk_at{0};
    };

    struct Planner {
        bsp::AiPlannerKind kind{bsp::AiPlannerKind::Siege};
        int slot{0};
        std::vector<Group*> owned;
    };

    struct Brain {
        int party_slot{0};            // brain+20h
        int world_set{0};             // brain+24h
        std::array<Planner, 8> planners{};  // brain+0h..+1Ch
    };

    // Groups are held by pointer so a void* handed to a sequence stays valid
    // across a phase that appends.
    std::vector<std::unique_ptr<Group>> groups;
    std::vector<Group*> registry;                  // 00F8AA70, every live group
    std::array<std::vector<Group*>, 3> by_team;    // g_aiGroupsByTeam, 00F8AA48 + t*0Ch
    std::vector<Group*> emptied;                   // 00F8AA7C / 00F8AA80
    std::array<std::unique_ptr<Brain>, bsp::kAiGroupPartySlotCount> brains{};
    std::array<float, bsp::kAiGroupPartySlotCount> next_think{};
    std::array<bool, bsp::kAiGroupPartySlotCount> party_record{};
    int current_party{-1};           // 00E0E344
    float clock_seconds{0.0f};       // 00F876A4
    std::uint32_t rng{0x2545F491u};  // 00BD2F10's stream, one per run
    bool created{false};

    // Which group holds a unit, so entity_has_group answers entity+16Ch.
    std::vector<Group*> group_of_unit;

    // The plane squadron layer. The native scene creates a PlaneSquadronGen
    // (004F0AD0) and its slot-39 attach 007F4580 fills the five-slot member
    // array at +3D0h with WingCount planes; the AI then groups the SQUADRON,
    // because 009FE080 admits class 18h and never the plane base 0Fh. This
    // process creates one unit per scene entity, so each aircraft entity
    // yields a squadron of one wing: the member array is real, the extra
    // wings are not spawned because unit creation is owned elsewhere.
    // Labelled substitution for 007F4580's per-wing loop.
    struct Squadron {
        bsp::PlaneSquadronEntity entity;
        std::vector<std::size_t> member_units;  // unit indices in +3D0h order
    };
    std::vector<Squadron> squadrons;
    // A plane the squadron owns is NOT an AI candidate of its own. The native
    // seeds the scene's PlaneSquadronGen entities; the planes exist only in the
    // member array at +3D0h, and 009FE080 would refuse them anyway (009FE0A5
    // pushes the ship base 6, never the plane base 0Fh). Keeping both in a
    // group double-orders the same aircraft.
    std::vector<bool> unit_owned_by_squadron;

    // The native 0077D600 REPLACES an entity's outstanding order; this
    // process's order ring appends, so the follower pass 00A10DC0, which calls
    // 00A02020 on every fixed step and whose only native gate is the squared
    // distance at 00A0205C, would append one order per member per step for the
    // whole mission. What is suppressed here is the DUPLICATE, never a change:
    // a new token, a new target or a point that moved more than a metre is
    // always issued. Labelled substitution for the ring's replace semantics,
    // not a native rule, and the census counts what it suppressed.
    struct LastOrder {
        std::string token;
        std::string target;
        float point[3]{0.0f, 0.0f, 0.0f};
        bool valid{false};
    };
    std::vector<LastOrder> last_order;
    static constexpr float kOrderRepeatEpsilonSquared = 1.0f;  // one metre
    bool order_is_repeat(std::size_t index, const std::string& token,
                         const std::string& target, const float point[3]) {
        if (last_order.size() <= index) last_order.resize(index + 1u);
        LastOrder& prev = last_order[index];
        const bool same = prev.valid && prev.token == token && prev.target == target &&
            (point == nullptr ||
             ((prev.point[0] - point[0]) * (prev.point[0] - point[0]) +
              (prev.point[1] - point[1]) * (prev.point[1] - point[1]) +
              (prev.point[2] - point[2]) * (prev.point[2] - point[2]))
                 < kOrderRepeatEpsilonSquared);
        if (same) {
            ++summary.orders_suppressed;
            return true;
        }
        prev.valid = true;
        prev.token = token;
        prev.target = target;
        if (point != nullptr) {
            prev.point[0] = point[0];
            prev.point[1] = point[1];
            prev.point[2] = point[2];
        } else {
            prev.point[0] = prev.point[1] = prev.point[2] = 0.0f;
        }
        return false;
    }
    bool seed_admits(std::size_t index) const {
        if (index >= unit_owned_by_squadron.size()) return true;
        return !unit_owned_by_squadron[index];
    }
    std::size_t next_admitted_seed(std::size_t from) const {
        while (from < candidate_count() && !seed_admits(from)) ++from;
        return from;
    }

    // Candidate indices [0, units.count()) are units; the squadrons follow.
    std::size_t candidate_count() const {
        return units.count() + squadrons.size();
    }
    bool is_squadron(std::size_t index) const {
        return index >= units.count() && index < candidate_count();
    }
    Squadron* squadron_of(std::size_t index) {
        if (!is_squadron(index)) return nullptr;
        return &squadrons[index - units.count()];
    }
    const Squadron* squadron_of(std::size_t index) const {
        if (!is_squadron(index)) return nullptr;
        return &squadrons[index - units.count()];
    }
    // Every geometric, team and liveness query on a squadron answers from
    // its FLIGHT LEADER, the plane at +3D0h that 007ED610 rotates into slot
    // 0; that is the point the native's formation and leader reads take.
    std::size_t proxy(std::size_t index) const {
        const Squadron* s = squadron_of(index);
        if (s == nullptr || s->member_units.empty()) return index;
        return s->member_units.front();
    }
    std::size_t proxy(void* entity) const { return proxy(unit_index_of(entity)); }

    // 007EDA90's three reads, taken on the squadron's flight leader.
    bsp::PlaneSquadronLeadPlaneFacts squadron_lead_facts(const Squadron& s) const {
        bsp::PlaneSquadronLeadPlaneFacts lead;
        if (s.member_units.empty()) return lead;          // 007EDA99 JZ
        const std::size_t leader = s.member_units.front();
        lead.has_lead_plane = true;
        // 007EDAA0 PUSH 17h through the leader's vtable[+5Ch].
        lead.lead_is_kamikaze_17 =
            units.unit_is_kind_of(leader, bsp::kPlaneSquadronKamikazeKindId);
        // 007EDAAA, the byte at leader+C24h. docs/ATTACK_GATE_TAILS.md
        // establishes it as the authored `PilotFires`, written once at load
        // by FUN_007CD930 from BSP_Plane_ReadPropertyBag. This process has
        // no reader for it, so it stays clear and is labelled here.
        lead.lead_pilot_fires_0c24 = false;
        return lead;
    }

    // 00A335D0's record for this run's mode, read back through 00A371A0.
    bsp::AiTuningBlock tuning{};

    // The AiModeTuning record 00A08460 reads, projected out of the same block
    // 00A335D0 filled. Only the two fields the model's barrel arithmetic uses
    // are carried: MaxTargetKillRatio at record +05Ch and DamageCalcTime at
    // +060h (include/bsp/ai_target_weights.hpp). Everything else keeps its
    // default, which is what an unfilled record holds natively too.
    // 009FE270's target axis, read from arm 009FE2A2 and the torpedo arm
    // 009FE3F5: PUSH 0Fh the plane base, else PUSH 6 the ship base split by
    // 00827F70, else the fall-through. PUSH 8, the submarine, is asked first
    // only by the torpedo arm; everywhere else a submarine is ship-base and not
    // small surface, which is the same slot this returns for it.
    //
    // Labelled substitution on the query family, not on the codes: the native
    // asks the VEHICLE CLASS descriptor's vtable[+18h] and this asks the
    // instance's vtable[+5Ch]. The two id spaces coincide - VehicleClassKind in
    // vehicle_class.hpp and the entity class ids in docs/ENTITY_CLASS_IDS.md
    // both number ShipBase 6, PlaneBase 0Fh, Submarine 8, LandingShip 0Ch and
    // TorpedoBoat 0Eh - and both are selected from the same VehicleClass.Type,
    // so the codes below are the native's. The vtables are not the same vtable.
    // Packet cc8_ai_target_choice_observed. The buffer 00A13B60's per-member
    // loop fills, and the chosen-class table the mission summary prints.
    void* choice_member{nullptr};
    std::vector<std::pair<std::size_t, float>> choice_weights;
    std::map<int, unsigned long long> chosen_class_counts;
    std::map<int, unsigned long long> runnerup_class_counts;
    std::map<int, int> class_sample_counts;
    unsigned long long choice_samples_logged{0};

    static bool ai_weight_model_enabled() {
        // _dupenv_s rather than getenv, which is a /W4 /WX error under MSVC;
        // the same form native_frame_job_lifetime.cpp already uses.
        static const bool enabled = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            if (_dupenv_s(&text, &bytes, "BSP_AI_WEIGHT_MODEL") != 0) return true;
            const bool on = text == nullptr || text[0] != '0';
            std::free(text);
            return on;
        }();
        return enabled;
    }

    // One row per (bullet sub-type, target group) pair the accuracy lookup is
    // asked for, with the time factor and the barrel contribution 00A08460
    // would have built from it. Observation only.
    struct AccuracyCensusRow {
        unsigned long long lookups{0};
        unsigned long long zero_accuracy{0};
        unsigned long long unresolved{0};
        float accuracy{0.0f};        // one value per pair, so the last wins
        double contribution_sum{0.0};
    };
    static constexpr int kCensusSubTypes = 0x14;
    static constexpr int kCensusGroups = 5;
    AccuracyCensusRow accuracy_census[kCensusSubTypes][kCensusGroups]{};

    // 00A085AD PUSH 0Fh on EBP, the attacker vehicle class, stored to
    // [ESP+37h]; 00A08619 JE 00A09228 skips 00A0861F..00A09222 when it is
    // clear. So a PLANE attacker takes the big unprojected branch and everyone
    // else falls through to the subsystem-and-barrel walk this process does
    // project. Counting the split says whether IJN01's zero weights are the
    // barrel path answering honestly or the wrong path being walked at all.
    unsigned long long census_plane_attacker{0};
    unsigned long long census_other_attacker{0};
    std::map<int, unsigned long long> unresolved_by_attacker_class;

    void census_barrel_accuracy(const GameAiWeaponFacts::Unit* attacker_row,
                                std::size_t attacker_unit, std::size_t target) {
        if (units.unit_is_kind_of(attacker_unit, 0x0F)) {
            ++census_plane_attacker;
        } else {
            ++census_other_attacker;
        }
        if (attacker_row == nullptr || attacker_row->barrels.empty()) return;
        const bsp::AiAccuracyTargetGroup group = accuracy_target_group(target);
        const int g = static_cast<int>(group);
        if (g < 0 || g >= kCensusGroups) return;
        const float damage_calc_time = tuning.at(bsp::kAiTuningDamageCalcTime);
        for (const GameAiWeaponFacts::Barrel& barrel : attacker_row->barrels) {
            const int s = barrel.bullet_sub_type;
            if (s < 0 || s >= kCensusSubTypes) continue;
            AccuracyCensusRow& row = accuracy_census[s][g];
            ++row.lookups;
            // Packet cc8_ai_target_choice_observed item 3: sub-type 0 is a
            // barrel whose bullet class never resolved, which is what the
            // 37600-candidate admission shortfall traced back to. Counting the
            // ATTACKER's class for those says which units carry them, which the
            // gunnery-side census cannot be asked for from here because that
            // file is leased elsewhere.
            if (s == 0) ++unresolved_by_attacker_class[units.unit_class_id(attacker_unit)];
            bool resolved = false;
            const std::uint32_t offset =
                bsp::ai_bullet_type_accuracy_offset_009fe270(s, group, resolved);
            if (!resolved) {
                ++row.unresolved;
                continue;
            }
            const float accuracy = offset == 0u ? 0.0f : tuning.at(offset);
            row.accuracy = accuracy;
            if (!(accuracy > 0.0f)) ++row.zero_accuracy;
            // ai_barrel_time_factor and ai_barrel_damage, the two the model
            // multiplies the accuracy through at 00A09544 and 00A09548.
            const float factor =
                bsp::ai_barrel_time_factor(damage_calc_time, barrel.reload);
            row.contribution_sum +=
                static_cast<double>(bsp::ai_barrel_damage(factor, accuracy, barrel.shots));
        }
    }

    bsp::AiAccuracyTargetGroup accuracy_target_group(std::size_t unit) {
        if (units.unit_is_kind_of(unit, 0x0F)) {
            return bsp::AiAccuracyTargetGroup::Plane;
        }
        if (units.unit_is_kind_of(unit, 0x06)) {
            if (units.unit_is_kind_of(unit, 0x08)) {
                return bsp::AiAccuracyTargetGroup::Submarine;
            }
            // 00827F70, read from its bytes: 00827F78 PUSH 0Eh TorpedoBoat,
            // then 00827F89 PUSH 0Ch LandingShip with 00827F95 requiring the
            // byte at class+808h, BigLandingShip, to be zero. The two codes are
            // exact; the +808h byte is the one thing this host does not hold,
            // so a BIG landing ship is classed small here where the native
            // would class it big. Labelled.
            if (units.unit_is_kind_of(unit, 0x0E) ||
                units.unit_is_kind_of(unit, 0x0C)) {
                return bsp::AiAccuracyTargetGroup::SmallShip;
            }
            return bsp::AiAccuracyTargetGroup::BigShip;
        }
        return bsp::AiAccuracyTargetGroup::Other;
    }

    bsp::AiModeTuning mode_tuning_record() const {
        bsp::AiModeTuning record{};
        record.max_target_kill_ratio = tuning.at(0x05Cu);
        record.damage_calc_time = tuning.at(0x060u);
        return record;
    }

    GameAiSummary summary{};
    std::vector<GameAiPartyRow> parties;

    // Every void* this file hands a sequence is a Group*, never a separate
    // cursor object, because the two sequences mix list walks with handles they
    // were given directly (a planner's owned group, the emptied-list head) and
    // a caller cannot tell the two apart. The walk position therefore lives on
    // the group: `first_*` publishes it, `next_group` reads it back.
    //
    // Nested walks over one list are safe: phase 4 runs an inner walk over the
    // same party list as the outer, and a group re-published by the inner walk
    // gets the same list and the same index it already held. Walks over
    // different lists never overlap, because a group is in exactly one
    // g_aiGroupsByTeam list and the party think's inner walk is over the enemy
    // team's list.
    void* open(const std::vector<Group*>& list) {
        return advance(list, 0);
    }
    void* advance(const std::vector<Group*>& list, std::size_t at) {
        while (at < list.size() && list[at]->destroyed) ++at;
        if (at >= list.size()) return nullptr;
        Group* g = list[at];
        g->walk_list = &list;
        g->walk_at = at;
        return g;
    }
    static Group* group_at(void* handle) { return static_cast<Group*>(handle); }

    GameAiPartyRow& party_row(int party) {
        for (GameAiPartyRow& row : parties) {
            if (row.party == party) return row;
        }
        GameAiPartyRow row;
        row.party = party;
        parties.push_back(row);
        return parties.back();
    }

    float random_00bd2f10(float low, float high) {
        rng = rng * 1664525u + 1013904223u;
        const float unit = static_cast<float>((rng >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }

    // --- AiGroupThinkHost ---------------------------------------------------

    bool high_level_ai_enabled() override {
        // 00E0E34C's absolute address occurs once in the image, at the read
        // 00A32D52, and its static value is 1, so the gate is always open.
        return true;
    }

    std::uint32_t emptied_group_count() override {
        return static_cast<std::uint32_t>(emptied.size());
    }
    void* first_emptied_group() override {
        return emptied.empty() ? nullptr : static_cast<void*>(emptied.front());
    }
    void* first_group_of_global_registry() override { return open(registry); }
    void* first_group_of_proximity_list() override { return open(by_team[2]); }

    void release_group_reference(void* holder, void* group) override {
        // 00A2B8F0 __thiscall(group+24h)(emptyGroup), contract: unread. The
        // only reference this process holds between groups is a command target.
        Group* h = group_at(holder);
        Group* g = group_at(group);
        if (h != nullptr && h->command.target_group == static_cast<void*>(g)) {
            // Natively the released group leaves a dangling command+1Ch. This
            // process cannot hold one, so the command reverts to the class the
            // constructor installed. Labelled substitution, not a native rule.
            h->command = initial_command_for(h);
        }
        record("AiGroups::release_group_reference", 0x00a2b8f0u);
    }
    void unlink_and_free_emptied_node(void* group) override {
        Group* g = group_at(group);
        emptied.erase(std::remove(emptied.begin(), emptied.end(), g), emptied.end());
        done("AiGroups::unlink_emptied_node", 0x00a2e7bfu);
    }
    void destroy_group(void* group) override {
        // slot 0 of 00D23084 is 00A2D8C0, which calls 00A2D440, the unregister.
        // contract: partial. The unregister is the list removals below.
        Group* g = group_at(group);
        if (g == nullptr || g->destroyed) return;
        g->destroyed = true;
        registry.erase(std::remove(registry.begin(), registry.end(), g), registry.end());
        for (std::vector<Group*>& list : by_team) {
            list.erase(std::remove(list.begin(), list.end(), g), list.end());
        }
        for (const std::unique_ptr<Brain>& brain : brains) {
            if (brain == nullptr) continue;
            for (Planner& planner : brain->planners) {
                planner.owned.erase(std::remove(planner.owned.begin(), planner.owned.end(), g),
                    planner.owned.end());
            }
        }
        for (std::size_t i = 0; i < group_of_unit.size(); ++i) {
            if (group_of_unit[i] == g) group_of_unit[i] = nullptr;
        }
        ++summary.groups_destroyed;
        record("AiGroups::destroy_group", 0x00a2d8c0u);
    }

    void evict_invalid_members(void* group) override {
        // 00A2DDE0, body read in full: a member whose gate bytes fail, or whose
        // party or team no longer matches the group's, leaves the list.
        Group* g = group_at(group);
        if (g == nullptr) return;
        const std::size_t before = g->members.size();
        std::vector<std::size_t> kept;
        kept.reserve(before);
        for (const std::size_t unit : g->members) {
            bsp::AiGroupCandidateFlags flags = unit_flags(unit);
            const int side = units.unit_side_0054(proxy(unit));
            if (bsp::ai_group_member_still_belongs(flags, side, g->party, side, g->team)) {
                kept.push_back(unit);
            } else if (unit < group_of_unit.size()) {
                group_of_unit[unit] = nullptr;
            }
        }
        summary.members_evicted += before - kept.size();
        g->members.swap(kept);
        if (g->members.empty() && !g->emptied) {
            g->emptied = true;
            emptied.push_back(g);
        }
        done("AiGroups::evict_invalid_members", 0x00a2dde0u);
    }

    void split_detached_members(void* group) override {
        // 00A2E260, body read in full: build the subset of members for which
        // 009FE080 holds and move it into a NEW group, but only when the subset
        // is non-empty and strictly smaller than the population (00A2E334 JBE,
        // 00A2E342 JNC, both unsigned). This is the group multiplier. Leaving
        // it a no-op, as packet cc8_ai_coordinator_tick did, is why that
        // packet measured one group per team and therefore one attack order:
        // with a single enemy group the planner re-picks the same target every
        // think and 00A2CBD0's second test skips it.
        ++summary.splits;
        Group* g = group_at(group);
        if (g == nullptr) return;
        std::vector<std::size_t> groupable;
        for (const std::size_t unit : g->members) {
            if (bsp::ai_entity_is_groupable_combatant_009fe080(combatant_facts(unit))) {
                groupable.push_back(unit);
            }
        }
        if (!bsp::ai_group_split_runs_00a2e260(groupable.size(), g->members.size())) {
            done("AiGroups::split_detached_members", 0x00a2e260u);
            return;
        }
        std::vector<std::size_t> kept;
        for (const std::size_t unit : g->members) {
            if (std::find(groupable.begin(), groupable.end(), unit) == groupable.end()) {
                kept.push_back(unit);
            }
        }
        g->members.swap(kept);
        // 00A2E42A: the first split member creates the new group with 00A2DFA0,
        // the rest are added with 00A2D8E0.
        Group* made = static_cast<Group*>(create_group(handle(groupable.front())));
        for (std::size_t i = 1; i < groupable.size(); ++i) attach(made, groupable[i]);
        ++summary.splits_taken;
        done("AiGroups::split_detached_members", 0x00a2e260u);
    }

    void* create_group(void* first_member) override {
        const std::size_t unit = unit_index_of(first_member);
        groups.push_back(std::make_unique<Group>());
        Group* g = groups.back().get();
        g->team = units.unit_side_0054(proxy(unit));
        if (g->team < 0 || g->team > bsp::kAiGroupMaxSeedTeam) g->team = 0;
        g->party = g->team;   // this process files a group under its own team
        registry.push_back(g);
        // 00A2E086-00A2E0BD: every group appends itself to
        // g_aiGroupsByTeam[group+5638h] unconditionally.
        by_team[static_cast<std::size_t>(g->team)].push_back(g);
        // group+564Ch, the eight-byte NONCONTROL or IDLE instance the
        // constructor installs before the first member is added.
        g->command = initial_command_for(g);
        ++summary.groups_created;
        attach(g, unit);
        done("AiGroups::create_group", 0x00a2dfa0u);
        return g;
    }

    void add_group_member(void* group, void* entity) override {
        Group* g = group_at(group);
        if (g == nullptr) return;
        attach(g, unit_index_of(entity));
        done("AiGroups::add_group_member", 0x00a2d8e0u);
    }

    // The group constructor's install at group+564Ch. 009FFE50 admits party
    // slots 0 and 4 in campaign mode, which ai_party_think_mode already knows.
    bsp::AiCommandObject initial_command_for(Group* g) const {
        bsp::AiCommandObject cmd;
        const int slot = g->party;
        cmd.type = bsp::ai_command_initial_type(
            slot, bsp::ai_party_ai_enabled(kCampaignGameMode, slot));
        cmd.owner_group = g;
        return cmd;
    }

    // 00A2C6C0, the ship half of the merge family gate: the member list through
    // 009FE120, a tail forward of entity->vtable[+5Ch](6).
    bool group_has_ship(Group* g) {
        for (const std::size_t unit : g->members) {
            if (is_squadron(unit)) continue;   // never the 009FE0A5 arm
            if (units.unit_is_kind_of(unit, bsp::kUnitGunneryKindShipBase)) return true;
        }
        return false;
    }
    // 00A2C660, the air half: the member list through 009FE0F0, which asks
    // IsKindOf(0Fh) then IsKindOf(18h).
    bool group_has_air(Group* g) {
        for (const std::size_t unit : g->members) {
            if (is_squadron(unit)) return true;   // 009FE0F0's IsKindOf(18h)
            if (units.unit_is_kind_of(unit, 0x0F) || units.unit_is_kind_of(unit, 0x18)) {
                return true;
            }
        }
        return false;
    }

    // 00A2C600 over a group's members, through 009FE0B0's three class ids.
    bool group_matches_009fe0b0(Group* g) {
        for (const std::size_t unit : g->members) {
            if (is_squadron(unit)) continue;   // 18h is none of 1Bh/45h/46h
            if (bsp::ai_entity_class_matches_009fe0b0(units.unit_is_kind_of(unit, 0x1B),
                                                      units.unit_is_kind_of(unit, 0x45),
                                                      units.unit_is_kind_of(unit, 0x46))) {
                return true;
            }
        }
        return false;
    }

    bool can_auto_merge(void* into, void* from) override {
        // 00A2C8D0's own gates live in ai_group_can_auto_merge; what this
        // method answers is the delegated half, from+564Ch's vtable[+14h].
        Group* a = group_at(into);
        Group* b = group_at(from);
        // 00A2C8D0's own gates. The sequence does not apply them, so the whole
        // routine is this method's contract: a null or self argument, an empty
        // population on either side, and the family test. The +5648h
        // grouping-enabled byte is 1 from the group constructor and this
        // process has no producer that clears it.
        if (a == nullptr || b == nullptr || a == b || a->members.empty() ||
            b->members.empty()) {
            done("AiGroups::can_auto_merge", 0x00a2c8d0u);
            return false;
        }
        // 00A2C6C0 HasShip walks the members through 009FE120, IsKindOf(6);
        // 00A2C660 HasAir walks them through 009FE0F0, IsKindOf(0Fh) or (18h).
        if (!bsp::ai_group_families_may_merge(group_has_ship(a), group_has_air(a),
                                              group_has_ship(b), group_has_air(b))) {
            done("AiGroups::can_auto_merge", 0x00a2c8d0u);
            return false;
        }
        bsp::AiCommandMergeFacts facts;
        facts.type = b->command.type;              // the absorbed group's command
        facts.other_group_present = true;          // `into` is the argument
        facts.other_population = static_cast<std::uint32_t>(a->members.size());
        facts.own_population = static_cast<std::uint32_t>(b->members.size());
        facts.own_has_groupable_combatant = group_has_groupable_combatant(from);
        facts.other_has_groupable_combatant = group_has_groupable_combatant(into);
        if (facts.type == bsp::AiCommandType::Idle) {
            facts.own_matches_009fe0b0 = group_matches_009fe0b0(b);
            facts.other_matches_009fe0b0 = group_matches_009fe0b0(a);
        }
        facts.own_leader_weight = static_cast<float>(group_leader_order_key(from));
        facts.other_leader_weight = static_cast<float>(group_leader_order_key(into));
        // 00A10C60 substitutes the zero vector at 00F87574 for an empty group.
        static const float kOrigin[3] = {0.0f, 0.0f, 0.0f};
        const float* own_leader = group_leader_position(from);
        const float* other_leader = group_leader_position(into);
        if (own_leader == nullptr) own_leader = kOrigin;
        if (other_leader == nullptr) other_leader = kOrigin;
        for (int i = 0; i < 3; ++i) {
            facts.own_leader_position[i] = own_leader[i];
            facts.other_leader_position[i] = other_leader[i];
        }
        facts.auto_merge_dist = tuning.at(bsp::kAiTuningAutoMergeMergeDist);
        const bool ok = bsp::ai_command_can_merge_with(facts);
        done("AiGroups::can_auto_merge", 0x00a2c8d0u);
        return ok;
    }

    void merge_group(void* into, void* from) override {
        Group* a = group_at(into);
        Group* b = group_at(from);
        if (a == nullptr || b == nullptr || a == b) return;
        // 00A2DBC1-00A2DD0E, phase A: every command in the registry that is an
        // ATTACK aimed at the absorbed group keeps its class and is rebound to
        // the absorber through 00A2BD00.
        for (Group* g : registry) {
            if (g->members.empty()) continue;
            if (!bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::Attack)) continue;
            if (g->command.target_group != static_cast<void*>(b)) continue;
            g->command.target_group = a;
            ++summary.commands_retargeted;
        }
        for (const std::size_t unit : b->members) attach(a, unit);
        b->members.clear();
        if (!b->emptied) {
            b->emptied = true;
            emptied.push_back(b);
        }
        ++summary.proximity_merges;
        done("AiGroups::merge_group", 0x00a2db80u);
    }

    void group_member_pass(void* group) override {
        // 00A2C790, body 00A2C790-00A2C8C6, read in full. The members report
        // their weapon directors' state to the group's AI command, whose vt+24h
        // is 00A0FC90 in all sixteen classes and discards it; then the command
        // ticks on its own 2-4 s schedule.
        Group* g = group_at(group);
        if (g == nullptr) return;
        const bsp::AiCommandMemberPassResult pass =
            bsp::ai_command_member_pass_00a2c790(*this, group);
        summary.member_passes += pass.members_walked;
        summary.member_reports += pass.notifications_sent;
        if (pass.ticked) ++summary.command_ticks;
        done("AiGroups::group_member_pass", 0x00a2c790u);
    }

    // ---- bsp::AiCommandMemberPassHost, one method per native call ----------
    // group_population and fixed_step_clock are the AiGroupThinkHost overrides
    // further down; the two interfaces declare them identically.
    std::size_t group_member_count(void* group) override {
        Group* g = group_at(group);
        return g == nullptr ? 0u : g->members.size();
    }
    void* group_member_at(void* group, std::size_t index) override {
        Group* g = group_at(group);
        if (g == nullptr || index >= g->members.size()) return nullptr;
        return handle(g->members[index]);
    }
    void* group_command(void* group) override {
        Group* g = group_at(group);
        return g == nullptr ? nullptr : static_cast<void*>(&g->command);
    }
    void* member_weapon_director(void* member) override {
        // member->vtable[+114h]. This process has no director object to hand
        // back, so a live unit stands in for a non-null director and the two
        // readers below take the unit index. Labelled substitution.
        if (member == nullptr) return nullptr;
        if (!units.unit_active(proxy(member))) return nullptr;
        return member;
    }
    void* director_target_descriptor(void* director) override {
        // 0071EB60. The descriptor is read for its side effect on the census
        // only; the notification that would consume it is discarded natively.
        const std::size_t unit = unit_index_of(director);
        bsp::SceneCommandTarget target;
        int mode = 0;
        if (!units.active_command_descriptor_0071eb60(unit, target, mode)) return nullptr;
        ++summary.member_descriptors;
        return director;
    }
    void* director_current_command(void* director) override {
        const std::size_t unit = unit_index_of(director);  // 0071BE40
        const std::uint32_t command = units.director_current_command_0071be40(unit);
        if (command != 0u) ++summary.member_scene_commands;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(command));
    }
    void command_notify_member(void* command, void* current_command,
                               void* target_descriptor) override {
        // vtable[+24h] is 00A0FC90, `RET 8`, in every one of the sixteen
        // vtables between 00D22968 and 00D22C68. Nothing is done with either
        // argument, natively or here.
        (void)command;
        (void)current_command;
        (void)target_descriptor;
    }
    void command_tick(void* command) override {
        // vtable[+0Ch]. The five bodies packet cc8_ai_command_tick read run
        // here; NONCONTROL and IDLE still reach 00A10EC0, which was not read.
        bsp::AiCommandObject* cmd = static_cast<bsp::AiCommandObject*>(command);
        if (cmd == nullptr) return;
        // 00A12A90's gate, instrumented: MOVETOATTACK promotes to CLOSEATTACK
        // only when the leader stops being a groupable combatant or the two
        // leader points close to within CloseAttack_CollectDist. A group that
        // never closes never reaches 00A13B60 at all.
        if (cmd->type == bsp::AiCommandType::MoveToAttack && diag_moveto_lines < 20) {
            ++diag_moveto_lines;
            Group* g = group_at(cmd->owner_group);
            float own[3] = {0.0f, 0.0f, 0.0f};
            float tgt[3] = {0.0f, 0.0f, 0.0f};
            if (g != nullptr) tick_leader_point(g, own);
            if (cmd->target_group != nullptr) tick_leader_point(cmd->target_group, tgt);
            log.notef("  ai diag movetoattack leader=%s dist=%.1f collect=%.1f groupable=%d "
                "members=%zu",
                (g == nullptr || g->members.empty())
                    ? "" : unit_name(g->members.front()).c_str(),
                static_cast<double>(tick_horizontal_distance(own, tgt)),
                static_cast<double>(tuning.at(bsp::kAiTuningCloseAttackCollectDist)),
                (g == nullptr || g->members.empty()) ? 0
                    : (bsp::ai_entity_is_groupable_combatant_009fe080(
                           combatant_facts(g->members.front())) ? 1 : 0),
                g == nullptr ? std::size_t{0} : g->members.size());
        }
        const bsp::AiCommandTickResult tick = bsp::ai_command_tick_vt000c(*this, *cmd);
        // 00A15490 and 00A15500 both end in 00A13B60, with the target group's
        // leader point and 1.5f for CLOSEATTACK and the own group's and 1.0f
        // for DEFENDPOSITION.
        if (cmd->type == bsp::AiCommandType::CloseAttack ||
            cmd->type == bsp::AiCommandType::DefendPosition) {
            const bool close = cmd->type == bsp::AiCommandType::CloseAttack;
            float centre[3] = {0.0f, 0.0f, 0.0f};
            void* centre_group = close ? cmd->target_group : cmd->owner_group;
            if (centre_group != nullptr) tick_leader_point(centre_group, centre);
            const bsp::AiCloseAttackTickResult close_tick =
                bsp::ai_close_attack_tick_00a13b60(*this, *cmd, close ? 1.5f : 1.0f, centre);
            summary.close_members_served += close_tick.members_served;
            summary.close_attack_move_orders += close_tick.attack_move_orders;
            summary.close_set_target_orders += close_tick.set_target_orders;
            summary.close_fallback_movetos += close_tick.fallback_movetos;
            summary.close_candidates_scored += close_tick.candidates_scored;
            done("AiCommand::close_attack_tick", 0x00a13b60u);
        }
        summary.tick_orders += tick.orders_issued;
        summary.tick_formation_requests += tick.formation_requests;
        summary.tick_followers += tick.followers_walked;
        if (tick.promoted) ++summary.command_promotions;
        if (cmd->type == bsp::AiCommandType::NonControl ||
            cmd->type == bsp::AiCommandType::Idle) {
            record("AiCommand::tick_000c", 0x00a10ec0u);
        } else {
            done("AiCommand::tick_000c", 0x00a02020u);
        }
    }

    // ---- bsp::AiCloseAttackTickHost, one method per native call -----------
    std::size_t close_member_count(void* group) override { return group_member_count(group); }
    void* close_member_at(void* group, std::size_t index) override {
        void* member = group_member_at(group, index);
        if (member != nullptr) diag_close_member(member);
        return member;
    }
    bool close_member_is_ship_base(void* member) override {
        return tick_member_is_ship_base(member);
    }
    bool close_member_is_plane_squadron(void* member) override {
        return tick_member_is_plane_squadron(member);
    }
    bool close_member_squadron_excluded(void* member) override {
        return tick_squadron_excluded_007eda90(member);
    }
    // 00A143ED PUSH 18h, 00A14402 PUSH 17h, 00A1440C the +C24h byte, 00A14413
    // JE, then the other arm's 00A14427 PUSH 6 and 00A1443D vtable[+2Ch]. One
    // line per member with every input's value on this run.
    void diag_close_member(void* member) {
        if (diag_member_lines >= 48) return;
        ++diag_member_lines;
        const std::size_t index = unit_index_of(member);
        const bool squadron = tick_member_is_plane_squadron(member);
        const bool excluded = squadron && tick_squadron_excluded_007eda90(member);
        const bool ship = tick_member_is_ship_base(member);
        log.notef("  ai diag close member=%s squadron_18h=%d excluded_007eda90=%d "
            "ship_base_6=%d busy=%d served=%d",
            unit_name(index).c_str(), squadron ? 1 : 0, excluded ? 1 : 0, ship ? 1 : 0, 0,
            bsp::ai_close_attack_member_served(squadron, excluded, ship, false) ? 1 : 0);
    }
    bool close_member_controller_busy(void* member) override {
        // member+538h through its vtable[+2Ch] at 00A1443D; contract unread.
        (void)member;
        record("AiCommand::close_controller_busy", 0x00a1443du);
        return false;
    }
    bool close_member_position(void* member, float out[3]) override {
        return tick_member_position(member, out);
    }
    std::size_t close_candidate_count() override { return units.count(); }
    void* close_candidate_at(std::size_t index) override { return handle(index); }
    bool close_candidate_position(void* candidate, float out[3]) override {
        return tick_member_position(candidate, out);
    }
    int close_candidate_team(void* candidate) override {
        return units.unit_side_0054(proxy(candidate));
    }
    bool close_candidate_alive(void* candidate) override {
        return units.unit_alive_and_visible(proxy(candidate));
    }
    float close_target_weight(void* member, void* candidate) override {
        // 00A0F810, body 00A0F810..00A0F961, read in full with the stack slots
        // normalised by tools/stack_frame_walk.py --indirect-pops 4. Its four
        // multipliers run here; only its innermost term, 00A08460's own weight,
        // is still stood in for by the candidate's class weight from 009FDF30,
        // because 00A08460 needs the per-barrel reload, accuracy and shot count
        // that live in the gunnery host and this coordinator does not hold one.
        // Labelled: the shape and the three class and objective multipliers are
        // the native's; the base term is not.
        const std::size_t target = unit_index_of(candidate);
        bsp::AiCandidateTargetWeightInputs in;
        // 00A0F843's base term. The real model 00A08460 runs as soon as the
        // weapon-facts table carries a row for the attacker AND the target;
        // until the gunnery host publishes one it has neither, and the
        // candidate's class weight from 009FDF30 stands in exactly as before.
        // docs/AI_TARGET_WEIGHT_TERMS.md term 2.
        const GameAiWeaponFacts& facts = game_ai_weapon_facts();
        const std::size_t attacker_unit = proxy(member);
        const GameAiWeaponFacts::Unit* attacker_row = facts.row(attacker_unit);
        const GameAiWeaponFacts::Unit* target_row = facts.row(target);
        // Packet cc8_ai_target_weight_zero. An OBSERVATION pass: it reads what
        // 009FE270 would answer for every (attacker barrel, this target) pair
        // and changes nothing, so it runs while inputs_complete is still false
        // and the stand-in is still what scores. Without this the census would
        // need the model switched on, which is the regression it is meant to
        // diagnose.
        census_barrel_accuracy(attacker_row, attacker_unit, target);
        // One build, two columns: BSP_AI_WEIGHT_MODEL=0 keeps the class
        // stand-in so a before run and an after run come from the same binary
        // and differ in nothing else. Default is on.
        if (ai_weight_model_enabled() && attacker_row != nullptr && target_row != nullptr
            && attacker_row->inputs_complete && target_row->inputs_complete) {
            bsp::AiTargetWeightKey key;
            key.attacker = handle(attacker_unit);
            key.attacker_class = units.unit_class_id(attacker_unit);
            key.target = handle(target);
            key.target_is_neutral = 0;   // target record +1Ch, no producer here
            const bsp::AiModeTuning record = mode_tuning_record();
            AiWeightModelBinding model(facts, record, tuning,
                [this](std::size_t unit) { return accuracy_target_group(unit); });
            in.base_weight = bsp::ai_target_weight_00a08460(model, key);
            ++summary.weight_model_runs;
        } else {
            // 00A08460 with none of its inputs. 1.0f is the identity of the
            // product it feeds, and the class weight that used to stand here
            // has moved to target_scale, which is where the native keeps it.
            in.base_weight = 1.0f;
            ++summary.weight_class_stand_ins;
        }
        // 00A0F84C tests the attacker record's +1Ch, which 00A04560 fills from
        // 00A04568 CMP [entity+54h],2 / SETGE: the entity's SIDE being two or
        // more, a neutral or third party. That this process does reach.
        in.attacker_record_flag_1c = units.unit_side_0054(attacker_unit) >= 2;
        // 00A0F859 then asks the attacker's vtable[+18h] with 1Ch. That slot is
        // NOT the +5Ch class test the other three tests in 00A0F810 use, so the
        // 1Ch is a type-group code and not the MCommandBuilding class id it
        // resembles; the same slot is AiTargetWeightModelHost::entity_is_type.
        // Unread, so this stays false and the arm never fires. The arm can only
        // REMOVE weight, so leaving it off can admit a candidate the native
        // would score zero and never reject one it would keep.
        in.attacker_is_command_building = false;
        // 00A0F86A and 00A0F872, the two record +18h factors. 00A00020 fills
        // +18h from its fifth argument, which 00A04560 computed at 00A045EA
        // through 00A371A0's [record+50h] and [record+54h] and the
        // interpolation 00419010, and which 00A00083 optionally re-rolls
        // through 00BD2F10 when 00A00058's COMISS finds it negative. The two
        // tuning offsets are unread, so both factors keep the identity 1.0f.
        in.attacker_factor = 1.0f;
        in.target_factor = 1.0f;
        // 00A0F89B's target +14h. 00A00020 fills it from its third argument,
        // which 00A0460F produced as 009FDF30's class weight for the entity's
        // +C4h class id MULTIPLIED by 00A04240(entity). The class-weight half
        // is exactly what this host computes, so it moves here from the base
        // term where it used to stand; 00A04240 is unread and its factor is
        // the identity. docs/AI_TARGET_WEIGHT_TERMS.md term 3.
        in.target_scale = unit_class_weight(target);
        // 00A0F87E picks objective set 0 when the local player's party equals
        // the attacker's +54h and set 4 otherwise, then 00A0F8B5 asks 008DDF90.
        // The sets are the ones the mission Lua fills; see
        // docs/MISSION_OBJECTIVES.md for why they hold no unit on these
        // missions, which makes this false throughout.
        const int attacker_side = units.unit_side_0054(attacker_unit);
        const int local_party = 0;   // [00E188A8]+18CCh, slot 0's +28h
        const int objective_set = attacker_side == local_party ? 0 : 4;
        const std::vector<std::size_t> set =
            game_objective_sets().units_in_slot(objective_set);
        in.target_is_objective =
            std::find(set.begin(), set.end(), target) != set.end();
        if (in.target_is_objective) ++summary.weight_objective_hits;
        // 00A0F8F4 / 00A0F903 / 00A0F912, the 009FE0B0 trio, and 00A0F92F's
        // command-building test.
        in.target_matches_009fe0b0 = bsp::ai_entity_class_matches_009fe0b0(
            units.unit_is_kind_of(target, 0x1B), units.unit_is_kind_of(target, 0x45),
            units.unit_is_kind_of(target, 0x46));
        in.target_is_command_building = units.unit_is_kind_of(target, 0x1C);
        if (in.target_matches_009fe0b0) {
            ++summary.weight_fort_targets;
            if (!in.target_is_command_building) ++summary.weight_non_command_targets;
        }
        // 00A0F8CE 00923BE0(target), subtracted from 2.0. The routine is read
        // in full now (docs/AI_TARGET_WEIGHT_TERMS.md): the torn-down byte
        // +5Dh answers 0, otherwise the class fraction is clamped into [0, 1]
        // and cached at +164h. This process reaches the torn-down byte through
        // the scene node flags, but not the fraction, which is
        // `unit+370h / unit+36Ch` and lives with the gunnery host; so a live
        // candidate takes the full-health value 1.0f and the term is 1.0, not
        // the 2.0 an earlier reading of this packet left. Labelled: the clamp
        // and the torn-down arm are the native's, the fraction is not.
        bsp::SceneNodeFlags target_node;
        const bool have_node = units.unit_scene_node_flags(target, target_node);
        in.target_term = bsp::ai_unit_health_00923be0(
            have_node && target_node.torn_down, 0.0f, false);
        // The torn-down arm cannot actually be reached from here: 00A13B60's
        // candidate loop already drops a candidate that fails
        // close_candidate_alive, so every candidate scored is live. The arm is
        // modelled because the native has it, and the census counts any hit.
        if (in.target_term == 0.0f) ++summary.weight_torn_down_targets;
        ++summary.weight_queries;
        done("AiCommand::close_target_weight", 0x00a0f810u);
        const float weight = bsp::ai_candidate_target_weight_00a0f810(in);
        // Packet cc8_ai_target_choice_observed. 00A13B60 scores every candidate
        // of one member and then issues one order, so a buffer cleared on a
        // change of member holds exactly that member's candidates when
        // close_issue_order runs and can name the chosen target's weight and
        // the runner-up's. Observation only.
        if (member != choice_member) {
            choice_member = member;
            choice_weights.clear();
        }
        choice_weights.emplace_back(target, weight);
        return weight;
    }
    bool close_in_target_group(void* target_group, void* candidate) override {
        // 00A2C720 walks the group's +563Ch list for the entity.
        Group* g = group_at(target_group);
        if (g == nullptr) return false;
        const std::size_t unit = unit_index_of(candidate);
        return std::find(g->members.begin(), g->members.end(), unit) != g->members.end();
    }
    void* close_member_current_target(void* member) override {
        // The member's vtable[+114h] then 0071EB60, then 00521EA0 to resolve
        // the descriptor into an instance.
        // 00A2C790 reads the member's vtable[+114h]; for a squadron that is
        // 007ECFD0, `MOV EAX,[ECX+348h]`, the 0x22C block 007F4FE6 allocates
        // and 007F5009 stores. This process holds no such block, so the
        // flight leader's own descriptor stands in. Labelled.
        const std::size_t unit = proxy(member);
        bsp::SceneCommandTarget target;
        int mode = 0;
        if (!units.active_command_descriptor_0071eb60(unit, target, mode)) return nullptr;
        const std::uint32_t resolved = units.resolve_command_target_00521ea0(target);
        if (resolved == 0u) return nullptr;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(resolved));
    }
    bool close_issue_order(void* member, std::uint32_t command_class, void* target) override {
        // 0077D600 with a kind-1 descriptor naming the target. This process
        // reaches the same routine through the registry path, which resolves
        // the object by name instead of carrying the pointer; labelled.
        const std::size_t unit = unit_index_of(member);
        const std::string token =
            command_class == bsp::kAiSceneCommandAttackMove ? "attackmove" : "settarget";
        const std::string target_name = unit_name(unit_index_of(target));
        if (target_name.empty() || !issue_order(unit, token, target_name)) {
            ++summary.commands_refused;
            return false;
        }
        ++summary.commands_issued;
        if (current_party >= 0) ++party_row(current_party).commands_issued;
        if (summary.first_command_seconds < 0.0f) summary.first_command_seconds = clock_seconds;
        observe_target_choice(member, target);
        done("AiCommand::close_issue_order", 0x0077d600u);
        return true;
    }

    // Packet cc8_ai_target_choice_observed. What no run before this one
    // recorded: WHICH target the weight actually picked. The chosen entry is
    // looked up by target in the member's buffer, and the runner-up is the
    // highest-weight entry that is not the chosen one. Labelled: 00A149A8
    // orders candidates by SCORE, which is the weight times the range factor
    // and the two multipliers, so this runner-up is the runner-up by weight
    // and can differ from the one the score would have named.
    void observe_target_choice(void* member, void* target) {
        if (member != choice_member) return;
        const std::size_t chosen = unit_index_of(target);
        float chosen_weight = 0.0f;
        bool chosen_found = false;
        std::size_t runner = 0;
        float runner_weight = -1.0f;
        for (const std::pair<std::size_t, float>& entry : choice_weights) {
            if (entry.first == chosen) {
                chosen_weight = entry.second;
                chosen_found = true;
            } else if (entry.second > runner_weight) {
                runner_weight = entry.second;
                runner = entry.first;
            }
        }
        if (!chosen_found) return;
        const int chosen_class = units.unit_class_id(chosen);
        ++chosen_class_counts[chosen_class];
        if (runner_weight >= 0.0f) ++runnerup_class_counts[units.unit_class_id(runner)];
        // A bounded sample PER CHOSEN CLASS rather than the first 24 overall,
        // which only ever caught submarines and left the fort, the bomber and
        // the fighter unmeasured. Three of each is enough to check the
        // arithmetic and short enough not to flood the log.
        if (class_sample_counts[chosen_class] < 3) {
            ++class_sample_counts[chosen_class];
            ++choice_samples_logged;
            const GameAiWeaponFacts::Unit* row = game_ai_weapon_facts().row(chosen);
            const GameAiWeaponFacts::Unit* runner_row =
                runner_weight >= 0.0f ? game_ai_weapon_facts().row(runner) : nullptr;
            log.notef("  ai target choice chosen_class=%02Xh(%s) weight=%.6f hp=%.1f "
                "trio=%d command_building=%d | runner_class=%02Xh(%s) weight=%.6f hp=%.1f",
                chosen_class, ai_class_name(chosen_class),
                static_cast<double>(chosen_weight),
                static_cast<double>(row != nullptr ? row->hit_points : 0.0f),
                units.unit_is_kind_of(chosen, 0x1B) || units.unit_is_kind_of(chosen, 0x45) ||
                    units.unit_is_kind_of(chosen, 0x46) ? 1 : 0,
                units.unit_is_kind_of(chosen, 0x1C) ? 1 : 0,
                runner_weight >= 0.0f ? units.unit_class_id(runner) : 0,
                runner_weight >= 0.0f ? ai_class_name(units.unit_class_id(runner)) : "none",
                static_cast<double>(runner_weight),
                static_cast<double>(runner_row != nullptr ? runner_row->hit_points : 0.0f));
            // The barrel terms behind that weight, so the sample decomposes
            // into the listing's own arithmetic rather than being taken on
            // trust: damage = (DamageCalcTime / reload) * accuracy * shots, and
            // total = best + min(sum, DamageCalcTime).
            const std::size_t attacker_unit = proxy(member);
            const GameAiWeaponFacts::Unit* attacker_row =
                game_ai_weapon_facts().row(attacker_unit);
            if (attacker_row != nullptr) {
                const bsp::AiAccuracyTargetGroup group = accuracy_target_group(chosen);
                const float damage_calc_time = tuning.at(bsp::kAiTuningDamageCalcTime);
                double sum = 0.0;
                double best = 0.0;
                for (const GameAiWeaponFacts::Barrel& b : attacker_row->barrels) {
                    bool resolved = false;
                    const std::uint32_t offset =
                        bsp::ai_bullet_type_accuracy_offset_009fe270(
                            b.bullet_sub_type, group, resolved);
                    const float accuracy =
                        (!resolved || offset == 0u) ? 0.0f : tuning.at(offset);
                    const float factor =
                        bsp::ai_barrel_time_factor(damage_calc_time, b.reload);
                    const float damage = bsp::ai_barrel_damage(factor, accuracy, b.shots);
                    if (accuracy > 0.0f) {
                        sum += damage;
                        if (damage > best) best = damage;
                    }
                    log.notef("      barrel subtype=%02Xh accuracy=%.4f reload=%.3f "
                        "shots=%d factor=%.4f damage=%.4f",
                        b.bullet_sub_type, static_cast<double>(accuracy),
                        static_cast<double>(b.reload), b.shots,
                        static_cast<double>(factor), static_cast<double>(damage));
                }
                const double clamped = sum > damage_calc_time ? damage_calc_time : sum;
                log.notef("      attacker_class=%02Xh(%s) barrels=%zu best=%.4f "
                    "sum=%.4f clamped=%.4f total=%.4f model=total/hp=%.6f",
                    units.unit_class_id(attacker_unit),
                    ai_class_name(units.unit_class_id(attacker_unit)),
                    attacker_row->barrels.size(), best, sum, clamped, best + clamped,
                    row != nullptr && row->hit_points > 0.0f
                        ? (best + clamped) / static_cast<double>(row->hit_points)
                        : 0.0);
            }
        }
    }

    static const char* ai_class_name(int class_id) {
        switch (class_id) {
        case 0x07: return "Destroyer";
        case 0x08: return "Submarine";
        case 0x09: return "MotherShip";
        case 0x0A: return "Cruiser";
        case 0x0B: return "Cargo";
        case 0x0C: return "LandingShip";
        case 0x0D: return "BattleShip";
        case 0x0E: return "TorpedoBoat";
        case 0x10: return "LevelBomber";
        case 0x11: return "TorpedoBomber";
        case 0x12: return "DiveBomber";
        case 0x13: return "Fighter";
        case 0x14: return "ReconPlane";
        case 0x17: return "Kamikaze";
        case 0x19: return "LandVehicle";
        case 0x1B: return "LandFort";
        case 0x1C: return "CommandBuilding";
        case 0x45: return "AirField";
        case 0x46: return "Shipyard";
        default:   return "other";
        }
    }
    bool close_issue_moveto(void* member, const float point[3]) override {
        return tick_issue_moveto(member, point);
    }
    float close_tuning_field(std::uint32_t offset) override { return tuning.at(offset); }
    int close_own_team(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? g->team : 0;
    }

    // 009FDF30 alone, without 009FFD80's ship and group-class multipliers.
    float unit_class_weight(std::size_t unit) {
        if (is_squadron(unit)) {
            // 009FFD80 on a squadron: its +C4h is 18h and 009FE0A5's ship
            // test is false, so only the class weight for 18h applies.
            const std::uint32_t squadron_offset =
                bsp::ai_entity_class_weight_offset_009fdf30(
                    bsp::kPlaneSquadronClassId);
            return bsp::ai_entity_leader_weight_009ffd80(
                squadron_offset == 0xFFFFFFFFu ? 1.0f : tuning.at(squadron_offset),
                false, false);
        }
        const std::uint32_t offset =
            bsp::ai_entity_class_weight_offset_009fdf30(units.unit_class_id(unit));
        return offset == 0xFFFFFFFFu ? 1.0f : tuning.at(offset);
    }

    // ---- bsp::AiCommandTickHost, one method per native call ----------------
    std::uint32_t tick_group_population(void* group) override {
        return group_population(group);
    }
    std::size_t tick_member_count(void* group) override { return group_member_count(group); }
    void* tick_member_at(void* group, std::size_t index) override {
        return group_member_at(group, index);
    }
    bool tick_leader_point(void* group, float out[3]) override {
        // 00A10C20: the first member's +FCh pose, or 00F87574 when empty.
        Group* g = group_at(group);
        out[0] = out[1] = out[2] = 0.0f;
        if (g == nullptr ||
            bsp::ai_group_leader_point_is_origin_00a10c20(
                static_cast<std::uint32_t>(g->members.size()))) {
            return false;
        }
        const float* p = group_leader_position(group);
        if (p == nullptr) return false;
        out[0] = p[0];
        out[1] = p[1];
        out[2] = p[2];
        return true;
    }
    bool tick_member_position(void* member, float out[3]) override {
        out[0] = out[1] = out[2] = 0.0f;
        if (member == nullptr) return false;
        float y = 0.0f;
        units.unit_position_00fc(proxy(member), out[0], y, out[2]);
        out[1] = y;
        return true;
    }
    bool tick_member_is_ship_base(void* member) override {
        if (member == nullptr) return false;
        // 009FE0A0's tail is unreachable for a squadron: its IsKindOf(18h)
        // already answered, so a squadron is never a ship base.
        if (is_squadron(unit_index_of(member))) return false;
        return units.unit_is_kind_of(unit_index_of(member),
                                     bsp::kUnitGunneryKindShipBase);
    }
    bool tick_member_is_plane_squadron(void* member) override {
        if (member == nullptr) return false;
        if (is_squadron(unit_index_of(member))) return true;   // +C4h == 18h
        return units.unit_is_kind_of(unit_index_of(member), 0x18);
    }
    bool tick_squadron_excluded_007eda90(void* member) override {
        if (member == nullptr) return false;
        const bool excluded =
            bsp::ai_squadron_excluded_007eda90(combatant_facts(unit_index_of(member)));
        if (excluded) ++summary.squadron_excluded;
        return excluded;
    }
    bool tick_squadron_excluded_009ffeb0(void* member) override {
        // 009FFEB0 is a second squadron-carrier exclusion, not 007EDA90: it
        // returns false outright when the byte at 00E17BF2 is set, and that
        // byte is 00 in the image, so the carrier arm is what runs. That arm
        // was not read. This process holds no carrier link, so the answer here
        // matches what 007EDA90 answers, which is false.
        record("AiCommand::squadron_excluded_009ffeb0", 0x009ffeb0u);
        return tick_squadron_excluded_007eda90(member);
    }
    bool tick_member_is_groupable_combatant(void* member) override {
        if (member == nullptr) return false;
        return bsp::ai_entity_is_groupable_combatant_009fe080(
            combatant_facts(unit_index_of(member)));
    }
    bool tick_issue_moveto(void* member, const float position[3]) override {
        // 00A02020's tail: 0077D600 with the `moveto` class descriptor
        // 00E08F68 and a position descriptor, flags 1.
        if (member == nullptr) return false;
        const std::size_t unit = unit_index_of(member);
        bsp::SceneCommandTarget target;
        target.kind = 0;             // a position (squadron arm below)
        target.position_valid = 1;   // 00A0214B stores 0x0100 over the pair
        target.object_id = 0;
        target.object = nullptr;
        target.position[0] = position[0];
        target.position[1] = position[1];
        target.position[2] = position[2];
        target.trailing = 0.0f;
        if (order_is_repeat(unit, "moveto", std::string(), position)) return true;
        std::size_t placed = 0;
        if (const std::vector<std::size_t>* members = squadron_member_units(unit)) {
            // 007ECF80's shape again: one moveto per live member plane.
            for (const std::size_t plane : *members) {
                if (units.issue_script_command(plane, bsp::kAiSceneCommandMoveTo, target,
                                               bsp::kAiSceneCommandFlags, "ai_command_tick",
                                               unit_name(plane)) != nullptr) {
                    ++placed;
                    ++summary.squadron_member_orders;
                }
            }
            if (placed != 0) ++summary.squadron_commands;
        } else if (units.issue_script_command(unit, bsp::kAiSceneCommandMoveTo, target,
                                              bsp::kAiSceneCommandFlags, "ai_command_tick",
                                              unit_name(unit)) != nullptr) {
            placed = 1;
        }
        if (placed == 0) {
            ++summary.commands_refused;
            return false;
        }
        ++summary.commands_issued;
        if (current_party >= 0) ++party_row(current_party).commands_issued;
        if (summary.first_command_seconds < 0.0f) summary.first_command_seconds = clock_seconds;
        done("AiCommand::issue_moveto", 0x00a02020u);
        return true;
    }
    bool tick_request_join_formation(void* follower, void* leader) override {
        // 0077C8D0 BSP_Entity_RequestJoinFormation, contract: unread. This
        // process has no formation ring to join, so the request is recorded.
        (void)follower;
        (void)leader;
        record("AiCommand::request_join_formation", 0x0077c8d0u);
        return true;
    }
    bool tick_avoid_zone_point(void* member, const float target[3], float out[2]) override {
        // 00417B10 BSP_AvoidZoneGroup_OffsetPointSequential, contract: unread.
        // Without it a ship is sent at the requested point itself.
        (void)member;
        out[0] = target[0];
        out[1] = target[2];
        record("AiCommand::avoid_zone_offset_point", 0x00417b10u);
        return false;
    }
    float tick_tuning_field(std::uint32_t offset) override { return tuning.at(offset); }
    float tick_horizontal_distance(const float a[3], const float b[3]) override {
        // 009FFC10 BSP_Math_HorizontalLength on the difference, x and z only.
        const float dx = a[0] - b[0];
        const float dz = a[2] - b[2];
        return std::sqrt(dx * dx + dz * dz);
    }
    void tick_replace_command(void* group, bsp::AiCommandType type) override {
        // 00A12C1B: new(20h) + 00A10710(group, target), vtable 00D22C44, then
        // 00A2BD00 deletes the outgoing command and stores the replacement.
        Group* g = group_at(group);
        if (g == nullptr) return;
        if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
        float own[3] = {0.0f, 0.0f, 0.0f};
        float tgt[3] = {0.0f, 0.0f, 0.0f};
        tick_leader_point(g, own);
        if (g->command.target_group != nullptr) tick_leader_point(g->command.target_group, tgt);
        bool groupable = false;
        if (!g->members.empty()) {
            groupable = bsp::ai_entity_is_groupable_combatant_009fe080(
                combatant_facts(g->members.front()));
        }
        log.notef("  ai command promote group members=%zu leader=%s groupable=%d "
            "dist=%.1f collect=%.1f", g->members.size(),
            g->members.empty() ? "" : unit_name(g->members.front()).c_str(),
            groupable ? 1 : 0,
            static_cast<double>(tick_horizontal_distance(own, tgt)),
            static_cast<double>(tuning.at(bsp::kAiTuningCloseAttackCollectDist)));
        g->command.type = type;
        g->command.owner_group = g;
        done("AiCommand::promote_to_close_attack", 0x00a2bd00u);
    }
    void command_pass_interval(void* command, float* low, float* high) override {
        (void)command;  // 00A0FC50, shared by all sixteen classes
        *low = bsp::kAiCommandMemberPassIntervalLow;
        *high = bsp::kAiCommandMemberPassIntervalHigh;
    }
    float member_pass_due(void* group) override {
        Group* g = group_at(group);
        return g == nullptr ? 0.0f : g->member_pass_due;
    }
    void store_member_pass_due(void* group, float when) override {
        Group* g = group_at(group);
        if (g != nullptr) g->member_pass_due = when;
    }
    float random_interval(float low, float high) override {
        return random_00bd2f10(low, high);  // the same stream 00BD2F10 drives
    }

    float auto_merge_dist() override {
        // 00A371A0()+208h, AutoMerge_MergeDist, the block 00A335D0 loads.
        done("AiGroups::auto_merge_dist", 0x00a371a0u);
        return tuning.at(bsp::kAiTuningAutoMergeMergeDist);
    }

    int game_mode() override { return kCampaignGameMode; }

    // Phase 3's five world collections. This process has one flat unit list, so
    // collection 0 yields every created unit and 1 to 4 are empty.
    void* first_seed_candidate(int collection) override {
        if (collection != 0) return nullptr;
        record("AiGroups::seed_collection", 0x00a2e835u);
        seed_cursor = next_admitted_seed(0);
        return seed_cursor < candidate_count() ? &seed_cursor : nullptr;
    }
    void* next_seed_candidate(void* cursor) override {
        if (cursor != &seed_cursor) return nullptr;
        seed_cursor = next_admitted_seed(seed_cursor + 1u);
        return seed_cursor < candidate_count() ? &seed_cursor : nullptr;
    }
    void* seed_candidate_entity(void* cursor) override {
        if (cursor != &seed_cursor) return nullptr;
        ++summary.seed_candidates;
        return handle(seed_cursor);
    }
    std::size_t seed_cursor{0};
    // Packet cc8_ai_squadron_served: a bounded diagnostic of the inputs the
    // close-attack member gate 00A143ED and the planner's owned-group walk
    // read. Bounded so a 3000-step run cannot flood the log.
    int diag_planner_lines{0};
    int diag_member_lines{0};
    int diag_order_lines{0};
    int diag_moveto_lines{0};

    bsp::AiGroupCandidateFlags entity_flags(void* entity) override {
        return unit_flags(unit_index_of(entity));
    }
    bool entity_has_group(void* entity) override {
        const std::size_t unit = unit_index_of(entity);
        return unit < group_of_unit.size() && group_of_unit[unit] != nullptr;
    }
    int entity_team(void* entity) override {
        return units.unit_side_0054(proxy(entity));
    }

    double group_leader_order_key(void* group) override {
        // 009FFD70 is an adjustor thunk onto 009FDF30 with the leader's +C4h;
        // contract: unread. The key only has to order a pair consistently, so
        // this process uses the leader's unit index.
        Group* g = group_at(group);
        record("AiGroups::group_leader_order_key", 0x009ffd70u);
        return (g == nullptr || g->members.empty())
            ? 0.0 : static_cast<double>(g->members.front());
    }
    const float* group_leader_position(void* group) override {
        Group* g = group_at(group);
        if (g == nullptr || g->members.empty()) return nullptr;
        float y = 0.0f;
        units.unit_position_00fc(g->members.front(), g->leader_position[0], y,
            g->leader_position[2]);
        g->leader_position[1] = y;
        return g->leader_position;
    }

    float fixed_step_clock() override { return clock_seconds; }
    float random_think_interval(float lo, float hi) override {
        return random_00bd2f10(lo, hi);
    }
    void* party_brain(int party_slot) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return nullptr;
        return brains[static_cast<std::size_t>(party_slot)].get();
    }
    void store_party_brain(int party_slot, void* brain) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return;
        if (brain == nullptr) brains[static_cast<std::size_t>(party_slot)].reset();
    }
    float next_think_time(int party_slot) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return 0.0f;
        return next_think[static_cast<std::size_t>(party_slot)];
    }
    void store_next_think_time(int party_slot, float when) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return;
        next_think[static_cast<std::size_t>(party_slot)] = when;
    }
    bool party_record_enabled(int party_slot) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return false;
        return party_record[static_cast<std::size_t>(party_slot)];
    }
    bool brain_wants_immediate_think(void* brain) override {
        // 00A15970's four mode arms read brain+10h..+1Ch vtable[+30h], the
        // replan virtuals. No planner in this process sets a replan request.
        (void)brain;
        record("AiGroups::brain_wants_immediate_think", 0x00a15970u);
        return false;
    }
    void* create_party_brain(int party_slot) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return nullptr;
        auto brain = std::make_unique<Brain>();
        brain->party_slot = party_slot;
        brain->world_set = party_slot;
        for (int slot = 0; slot < 8; ++slot) {
            brain->planners[static_cast<std::size_t>(slot)].slot = slot;
            brain->planners[static_cast<std::size_t>(slot)].kind = planner_kind_for_slot(slot);
        }
        Brain* raw = brain.get();
        brains[static_cast<std::size_t>(party_slot)] = std::move(brain);
        party_row(party_slot).brain_created = true;
        record("AiGroups::create_party_brain", 0x00a15a70u);
        return raw;
    }
    void destroy_party_brain(void* brain) override {
        (void)brain;
        record("AiGroups::destroy_party_brain", 0x00a16490u);
    }
    void set_current_party(int party_slot) override { current_party = party_slot; }
    void party_brain_think(void* brain) override {
        ++summary.parties_thought;
        Brain* b = static_cast<Brain*>(brain);
        if (b != nullptr) ++party_row(b->party_slot).thinks;
        bsp::ai_party_brain_think_00a181a0(*this, brain);
        done("AiParties::party_brain_think", 0x00a181a0u);
    }

    void* brain_planner(void* brain, int slot) override {
        Brain* b = static_cast<Brain*>(brain);
        if (b == nullptr || slot < 0 || slot >= 8) return nullptr;
        return &b->planners[static_cast<std::size_t>(slot)];
    }
    int brain_party_slot(void* brain) override {
        Brain* b = static_cast<Brain*>(brain);
        return b != nullptr ? b->party_slot : 0;
    }
    int brain_world_set_index(void* brain) override {
        Brain* b = static_cast<Brain*>(brain);
        return b != nullptr ? b->world_set : 0;
    }
    void planner_tick(void* planner) override {
        Planner* p = static_cast<Planner*>(planner);
        if (p == nullptr) return;
        ++summary.planner_ticks;
        if (current_party >= 0) ++party_row(current_party).planner_ticks;
        bsp::AiModePlannerTickInputs in;
        in.kind = p->kind;
        in.owned_group_count = static_cast<std::uint32_t>(p->owned.size());
        in.first_owned_group = p->owned.empty() ? nullptr : static_cast<void*>(p->owned.front());
        in.own_team = current_party;
        ticking_planner = p;
        if (diag_planner_lines < 24) {
            ++diag_planner_lines;
            Group* first = p->owned.empty() ? nullptr : p->owned.front();
            log.notef("  ai diag planner kind=%d owned=%zu first_group_members=%zu "
                "first_leader=%s first_command=%d enemy_groups=%u",
                static_cast<int>(p->kind), p->owned.size(),
                first == nullptr ? std::size_t{0} : first->members.size(),
                (first == nullptr || first->members.empty())
                    ? "" : unit_name(first->members.front()).c_str(),
                first == nullptr ? -1 : static_cast<int>(first->command.type),
                enemy_team_group_count(bsp::ai_enemy_team_index(current_party)));
        }
        bsp::ai_mode_planner_tick(*this, in);
        ticking_planner = nullptr;
        record("AiPlanners::planner_tick", 0x00a26510u);
    }
    void* first_group_of_party(int party_slot) override {
        if (party_slot < 0 || party_slot >= static_cast<int>(by_team.size())) return nullptr;
        return open(by_team[static_cast<std::size_t>(party_slot)]);
    }
    void* next_group(void* cursor) override {
        Group* g = group_at(cursor);
        if (g == nullptr || g->walk_list == nullptr) return nullptr;
        return advance(*g->walk_list, g->walk_at + 1u);
    }
    std::uint32_t group_population(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? static_cast<std::uint32_t>(g->members.size()) : 0u;
    }
    void* group_claiming_planner(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? static_cast<void*>(g->claimed_by) : nullptr;
    }
    bool group_has_groupable_combatant(void* group) override {
        // 00A2C5A0 walks the member list at group+563Ch and calls 009FE080 on
        // each member's +8h (00A2C5CE), so the group answer is the disjunction
        // of the SAME predicate the split uses. It does not admit the plane
        // base 0Fh: only a ship base, or a plane squadron 18h whose carrier
        // link fails 007EDA90.
        Group* g = group_at(group);
        if (g == nullptr) return false;
        for (const std::size_t unit : g->members) {
            if (bsp::ai_entity_is_groupable_combatant_009fe080(combatant_facts(unit))) {
                done("AiParties::group_has_groupable_combatant", 0x00a2c5a0u);
                return true;
            }
        }
        done("AiParties::group_has_groupable_combatant", 0x00a2c5a0u);
        return false;
    }
    bool group_has_member_in_world_set(void* group, int set_index) override {
        // 00A2C450, body read in full: it walks the group's member list at
        // +563Ch/+5640h and calls 008DDF90 BSP_SzurkeNyil_ContainsUnit on each
        // member against the set [00E188A8] + set_index*4 + 21A4h, returning
        // true at 00A2C4AB on the first member found and false at 00A2C4B4
        // when the walk ends.
        //
        // What those eight sets ARE is settled by packet cc8_ai_world_sets:
        // 004DF90F..004DF959 in BSP_Game_ConstructWorld builds exactly eight
        // of them, `operator new(30h)` each, constructed by 008DF900(set, i)
        // with the loop index and then 008DA160; 004DE20A..004DE234 in
        // BSP_Game_ConstructActualStorage only nulls the eight slots. They are
        // the per-PLAYER-SLOT objective sets (class string "SzurkeNyil" at
        // 00D16100, vtable 00D1610C), one per player, holding Objective
        // records whose own +20h unit lists carry the units.
        // docs/OBJECTIVE_UNIT_LIST.md and docs/LOCAL_PLAYER_UNIT_LISTS.md.
        //
        // Their producer is the mission Lua: 008CD440 Objectives_Add and
        // 008CDD60 Objectives_AddUnit. BOTH ARE UNIMPLEMENTED in this process
        // (the run log carries "MissionLuaNative::Objectives_Add [008cd440]
        // UNIMPLEMENTED"), so no objective and no objective unit exists, every
        // set is empty, and 008DDF90 finds nothing for any member. The native
        // routine's answer here is therefore its 00A2C4B4 walk-ended arm for
        // every group, which is what this returns. The lookup is wired through
        // the table below so that when Objectives_Add lands the answer becomes
        // real without another change here.
        ++summary.world_set_queries;
        Group* g = group_at(group);
        record("AiParties::group_has_member_in_world_set", 0x00a2c450u);
        if (g == nullptr || set_index < 0 ||
            static_cast<std::size_t>(set_index) >= GameObjectiveSets::kSlotCount) {
            return false;
        }
        const std::vector<std::size_t> set =
            game_objective_sets().units_in_slot(set_index);
        if (set.empty()) return false;          // 00A2C4B4 when the slot is empty
        for (const std::size_t member : g->members) {
            // 00A2C486 takes the node's +8h, the member entity, and 00A2C491
            // asks the set. A squadron answers for its flight leader, because
            // that is the unit an objective would name.
            const std::size_t unit = proxy(member);
            if (std::find(set.begin(), set.end(), unit) != set.end()) {
                ++summary.world_set_hits;
                return true;                    // 00A2C4AB
            }
        }
        return false;
    }
    void planner_claim_group(void* planner, void* group) override {
        Planner* p = static_cast<Planner*>(planner);
        Group* g = group_at(group);
        if (p == nullptr || g == nullptr || g->claimed_by != nullptr) return;
        g->claimed_by = p;
        p->owned.push_back(g);
        ++summary.planner_claims;
        if (current_party >= 0) ++party_row(current_party).groups_claimed;
        done("AiParties::planner_claim_group", 0x00a22750u);
    }
    void party_brain_plan_tail(void* brain) override {
        // 00A179E0, the brain's engagement pass. contract: unread, body
        // 00A179E0-00A18195.
        (void)brain;
        record("AiParties::party_brain_plan_tail", 0x00a179e0u);
    }

    // --- AiPlannerHost ------------------------------------------------------

    Planner* ticking_planner{nullptr};

    std::uint32_t enemy_team_group_count(int enemy_team) override {
        if (enemy_team < 0 || enemy_team >= static_cast<int>(by_team.size())) return 0u;
        return static_cast<std::uint32_t>(by_team[static_cast<std::size_t>(enemy_team)].size());
    }
    void* first_enemy_team_group(int enemy_team) override {
        if (enemy_team < 0 || enemy_team >= static_cast<int>(by_team.size())) return nullptr;
        return open(by_team[static_cast<std::size_t>(enemy_team)]);
    }
    bool group_is_end(int enemy_team, void* node) override {
        (void)enemy_team;
        return node == nullptr;
    }
    bsp::AiCommandType group_command_type(void* group) override {
        Group* g = group_at(group);
        // 00A2CBD0 installs 00A10890 (MoveToAttack, class 7) or 00A109B0
        // (CautiousAttack, class 8); the planner's already-on-it test asks the
        // command's vtable +4h for its type. NonControl is what a group with
        // no command answers.
        if (g == nullptr) return bsp::AiCommandType::NonControl;
        return g->command.type;
    }
    void* group_command_target(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? g->command.target_group : nullptr;
    }
    void clear_group_target_cache(void* group) override {
        (void)group;
        done("AiPlanners::clear_group_target_cache", 0x00a1cb80u);
    }
    float candidate_base_weight(void* group) override {
        // 00A0F970 weighs the candidate by what it holds. This process weighs
        // it by population, which is the only member fact it can supply.
        Group* g = group_at(group);
        record("AiPlanners::candidate_base_weight", 0x00a0f970u);
        return g != nullptr ? static_cast<float>(g->members.size()) : 0.0f;
    }
    float squared_planar_distance(void* a, void* b) override {
        if (group_leader_position(a) == nullptr) return 0.0f;
        if (group_leader_position(b) == nullptr) return 0.0f;
        Group* ga = group_at(a);
        Group* gb = group_at(b);
        if (ga == nullptr || gb == nullptr) return 0.0f;
        const float dx = ga->leader_position[0] - gb->leader_position[0];
        const float dz = ga->leader_position[2] - gb->leader_position[2];
        return dx * dx + dz * dz;
    }
    float near_radius_squared() override { return bsp::kAiEngagementRadiusSquaredValue; }
    float tuning_field(std::uint32_t offset) override {
        // 00A371A0 + offset. The six fields this packet reconstructed carry the
        // shipped values; every other slot is the zero an unloaded block holds,
        // so an unreconstructed field answers what it answered before.
        done("AiPlanners::tuning_field", 0x00a371a0u);
        return tuning.at(offset);
    }
    float range_interpolation(float near_value, float far_value, float distance) override {
        const float span = bsp::kAiEngagementRadiusSquaredValue;
        if (span <= 0.0f) return near_value;
        float t = distance / span;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return near_value + (far_value - near_value) * t;
    }
    bool candidate_has_member_in_own_set(void* group, int own_team) override {
        Group* g = group_at(group);
        return g != nullptr && g->team == own_team;
    }
    void order_attack(void* group, void* target, float aggressive) override;
    void quick_spawn_named_group(const char* tag) override {
        // 00A25B90 spawns a tagged group for a planner that owns nothing. This
        // process creates no unit at run time, so the spawn is recorded and the
        // planner keeps owning nothing.
        (void)tag;
        ++summary.planner_spawn_arms;
        record("AiPlanners::quick_spawn_named_group", 0x00a25b90u);
    }
    void publish_spawn_bias(float bias) override {
        (void)bias;
        record("AiPlanners::publish_spawn_bias", 0x00a16450u);
    }
    bool reset_target_flag() override {
        // [00F8A9E0] == 3. Nothing in this process writes it.
        return false;
    }

    // --- helpers ------------------------------------------------------------

    void* handle(std::size_t unit) { return reinterpret_cast<void*>(unit + 1u); }
    static std::size_t unit_index_of(void* entity) {
        return reinterpret_cast<std::size_t>(entity) - 1u;
    }

    static bsp::AiPlannerKind planner_kind_for_slot(int slot) {
        switch (slot) {
        case 0: return bsp::AiPlannerKind::Attack;
        case 1: return bsp::AiPlannerKind::Defend;
        case 2: return bsp::AiPlannerKind::Capture;
        case 3: return bsp::AiPlannerKind::Duel;
        case 4: return bsp::AiPlannerKind::Escort;
        case 5: return bsp::AiPlannerKind::Siege;
        default: return bsp::AiPlannerKind::Competitive;
        }
    }

    // 009FE080's inputs. The squadron arm's three reads have no producer in
    // this process: nothing creates a PlaneSquadronGen, so is_plane_squadron is
    // false for every unit and the ship-base tail is the whole answer.
    bsp::AiGroupableCombatantFacts combatant_facts(std::size_t unit) {
        if (const Squadron* s = squadron_of(unit)) {
            // The 009FE092 arm, on a real squadron for the first time in
            // this process: 009FE088's IsKindOf(18h) holds, so the answer is
            // the negation of 007EDA90 and the ship tail is never reached.
            return bsp::plane_squadron_combatant_facts(squadron_lead_facts(*s));
        }
        bsp::AiGroupableCombatantFacts facts;
        facts.is_plane_squadron = units.unit_is_kind_of(unit, 0x18);
        facts.is_ship_base = units.unit_is_kind_of(unit, bsp::kUnitGunneryKindShipBase);
        return facts;
    }

    bsp::AiGroupCandidateFlags unit_flags(std::size_t unit) {
        // A squadron is as alive as its flight leader: 007F3970 removes a
        // dead plane from +3D0h and 007F3A45 flags the last one out.
        unit = proxy(unit);
        bsp::AiGroupCandidateFlags flags;
        bsp::SceneNodeFlags node;
        bool pending = false;
        if (units.unit_scene_node_flags(unit, node) &&
            units.unit_pending_destroy_0060(unit, pending)) {
            flags.active = node.active;
            flags.flag_5d = node.torn_down;
            flags.flag_5e = node.destroyed;
            flags.flag_60 = pending;
        }
        if (!units.unit_active(unit)) flags.active = false;
        return flags;
    }

    void attach(Group* g, std::size_t unit) {
        if (unit >= candidate_count()) return;
        if (std::find(g->members.begin(), g->members.end(), unit) != g->members.end()) return;
        g->members.push_back(unit);
        // 00A2D8E0's sorted insert, now on the native key: the list is ordered
        // by DESCENDING 009FFD80 leader weight, so the first member is the one
        // every tick and every merge test reads as the leader. Ties keep unit
        // order, which is what the native's stop-on-JA insert also does.
        std::stable_sort(g->members.begin(), g->members.end(),
            [this](std::size_t a, std::size_t b) {
                return bsp::ai_group_member_sorts_before_00a2d8e0(
                    unit_leader_weight(a), unit_leader_weight(b));
            });
        if (group_of_unit.size() < candidate_count())
            group_of_unit.resize(candidate_count(), nullptr);
        group_of_unit[unit] = g;
        ++summary.members_added;
    }

    // 009FFD80 BSP_Entity_AiLeaderWeight: the class weight 009FDF30 reads out
    // of the tuning block, times the ship or group-class multiplier.
    float unit_leader_weight(std::size_t unit) {
        const std::uint32_t offset =
            bsp::ai_entity_class_weight_offset_009fdf30(units.unit_class_id(unit));
        const float class_weight =
            offset == 0xFFFFFFFFu ? 1.0f : tuning.at(offset);  // 009FDFED is FLD1
        return bsp::ai_entity_leader_weight_009ffd80(
            class_weight,
            units.unit_is_kind_of(unit, bsp::kUnitGunneryKindShipBase),
            bsp::ai_entity_class_matches_009fe0b0(units.unit_is_kind_of(unit, 0x1B),
                                                  units.unit_is_kind_of(unit, 0x45),
                                                  units.unit_is_kind_of(unit, 0x46)));
    }

    std::string unit_name(std::size_t unit) const {
        const GameUnitRow* row = units.unit_row(proxy(unit));
        return row != nullptr ? row->name : std::string();
    }

    // 007ECF80, vtable 00D087C0 slot +128h: the squadron walks its own
    // member array and calls the same slot on every live plane. This is the
    // shape a squadron-level order takes down to its planes; the native's
    // order fan-out itself (the arms of the +164h dispatcher 007F0030 other
    // than BCh and BEh) is contract: unread, so this is a labelled
    // substitution for it, not a reconstruction of it.
    const std::vector<std::size_t>* squadron_member_units(std::size_t index) const {
        const Squadron* s = squadron_of(index);
        if (s == nullptr || s->member_units.empty()) return nullptr;
        return &s->member_units;
    }

    void issue_to_member(std::size_t member, const std::string& token,
        const std::string& target_name, int party);
    bool issue_named_order(std::size_t unit, const std::string& token,
        const std::string& target_name);
    std::size_t fan_out_to_members(std::size_t member, const std::string& token,
        const std::string& target_name);
    bool issue_order(std::size_t member, const std::string& token,
        const std::string& target_name);
    void build_squadrons();
};

bool GameAiCoordinatorHost::Impl::issue_named_order(std::size_t unit,
    const std::string& token, const std::string& target_name) {
    const std::string name = unit_name(unit);
    if (name.empty() || target_name.empty()) return false;
    // The same 0046AAB0 registry resolve and 0077D600 message hop a scripted
    // order takes, so the plane task machinery and the ship order ring see the
    // AI's decision as a native order. docs/ENTITY_LUA_ORDER_PATH.md.
    return units.issue_player_command(token, target_name, name);
}

std::size_t GameAiCoordinatorHost::Impl::fan_out_to_members(std::size_t member,
    const std::string& token, const std::string& target_name) {
    // 007ECF80, vtable 00D087C0 slot +128h: the squadron walks its own +3D0h
    // array up to +3CCh and calls the same slot on every live member. The
    // squadron itself has no scene-registry name in this process, so what
    // reaches 0077D600 is one order per member plane. Labelled substitution
    // for the unread order arms of the +164h dispatcher 007F0030.
    const std::vector<std::size_t>* members = squadron_member_units(member);
    if (members == nullptr) return 0;
    std::size_t reached = 0;
    for (const std::size_t plane : *members) {
        if (issue_named_order(plane, token, target_name)) {
            ++reached;
            ++summary.squadron_member_orders;
        }
    }
    if (reached != 0) ++summary.squadron_commands;
    return reached;
}

bool GameAiCoordinatorHost::Impl::issue_order(std::size_t member,
    const std::string& token, const std::string& target_name) {
    if (order_is_repeat(member, token, target_name, nullptr)) return true;
    if (is_squadron(member)) return fan_out_to_members(member, token, target_name) != 0;
    return issue_named_order(member, token, target_name);
}

void GameAiCoordinatorHost::Impl::issue_to_member(std::size_t member,
    const std::string& token, const std::string& target_name, int party) {
    if (!issue_order(member, token, target_name)) {
        ++summary.commands_refused;
        if (party >= 0) ++party_row(party).commands_refused;
        return;
    }
    ++summary.commands_issued;
    if (party >= 0) ++party_row(party).commands_issued;
    if (summary.first_command_seconds < 0.0f) summary.first_command_seconds = clock_seconds;
    done("AiPlanners::issue_member_order", 0x0077d600u);
}

void GameAiCoordinatorHost::Impl::order_attack(void* group, void* target, float aggressive) {
    // 00A2CBD0, body 00A2CBD0-00A2CCE8, read in full:
    //   if (group+5644h == 0) return
    //   if (command->IsType(ATTACK) && command+1Ch == target) return
    //   member = first member; if (member->vtable[5Ch](6) == 0 && ... &&
    //       00BD2F40() > aggressive) CAUTIOUSATTACK else MOVETOATTACK
    Group* g = group_at(group);
    Group* t = group_at(target);
    if (g == nullptr || t == nullptr) return;
    if (g->members.empty()) return;                      // +5644h == 0
    // command->vtable[+8h](ATTACK) and command+1Ch == target.
    if (bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::Attack) &&
        g->command.target_group == static_cast<void*>(t)) {
        return;  // already on it
    }
    // A higher aggressive ratio makes CAUTIOUSATTACK less likely, because
    // 00BD2F40's uniform draw is compared against it.
    const bool cautious = random_00bd2f10(0.0f, 1.0f) > aggressive;
    // 00A2BD00: the outgoing command is destroyed through its vtable[+0h] with
    // flag 1 and the new pointer is stored at group+564Ch.
    if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
    bsp::AiCommandObject replacement;
    replacement.type = cautious ? bsp::AiCommandType::CautiousAttack
                                : bsp::AiCommandType::MoveToAttack;
    replacement.owner_group = g;
    replacement.target_group = t;
    g->command = replacement;
    if (diag_order_lines < 12) {
        ++diag_order_lines;
        log.notef("  ai diag order_attack group_members=%zu group_leader=%s "
            "target_members=%zu target_leader=%s cautious=%d",
            g->members.size(),
            g->members.empty() ? "" : unit_name(g->members.front()).c_str(),
            t->members.size(),
            t->members.empty() ? "" : unit_name(t->members.front()).c_str(),
            cautious ? 1 : 0);
    }
    ++summary.attack_orders;
    if (cautious) ++summary.attack_cautious;
    else ++summary.attack_movetoattack;
    if (current_party >= 0) ++party_row(current_party).attack_orders;
    done("AiPlanners::order_attack", 0x00a2cbd0u);

    // The substitution for 00A2C790's unread member->vtable[+114h]: each member
    // gets the order as a scene command. The token is the member's own weapon
    // kind, because the 26-row registry has no single "attack": a plane takes
    // `dogfight` against a plane group and `attackmove` against a surface one,
    // and a ship takes `artillery`. docs/ENTITY_LUA_ORDER_PATH.md.
    const std::string target_name = t->members.empty() ? std::string()
        : unit_name(t->members.front());
    const bool target_is_air = !t->members.empty() &&
        units.unit_is_kind_of(t->members.front(), bsp::kUnitGunneryKindPlaneBase);
    for (const std::size_t member : g->members) {
        const bool member_is_air = units.unit_is_kind_of(member,
            bsp::kUnitGunneryKindPlaneBase);
        const char* token = "artillery";
        if (member_is_air) token = target_is_air ? "dogfight" : "attackmove";
        issue_to_member(member, token, target_name, current_party);
    }
}

GameAiCoordinatorHost::GameAiCoordinatorHost(GameHostLog& log, GameUnitsHost& units)
    : impl_(std::make_unique<Impl>(log, units)) {}

GameAiCoordinatorHost::~GameAiCoordinatorHost() = default;

void GameAiCoordinatorHost::create_00a32350() {
    Impl& host = *impl_;
    if (host.created) return;
    host.created = true;
    host.build_squadrons();
    host.group_of_unit.assign(host.candidate_count(), nullptr);
    host.next_think.fill(0.0f);
    host.party_record.fill(false);
    // The party records the mission's `SetParty` binding fills. This process
    // has no party record block, so a party slot is enabled when at least one
    // created unit carries that side. record +0h is the native gate.
    for (std::size_t i = 0; i < host.units.count(); ++i) {
        const int side = host.units.unit_side_0054(i);
        if (side >= 0 && side < bsp::kAiGroupPartySlotCount) {
            host.party_record[static_cast<std::size_t>(side)] = true;
        }
    }
    host.summary.game_mode = host.game_mode();
    // 00A335D0. 009FFC80 picks the record: an effective game mode of 0 takes
    // the 009FFC9E arm, which clamps 00A15950's difficulty into the three
    // IslandCapture records. Nothing in this process produces a difficulty, so
    // it is 0; all three IslandCapture rows carry the same values for the six
    // fields this packet reconstructed, so the choice does not change a number.
    {
        bsp::AiTuningAuthoredReader reader;
        const bsp::AiTuningMode mode = bsp::ai_tuning_mode_009ffc80(host.game_mode(), 0);
        bsp::ai_tuning_load_00a335d0(reader, mode, host.tuning);
        host.summary.tuning_mode = static_cast<int>(mode);
        host.summary.tuning_merge_dist = host.tuning.at(bsp::kAiTuningAutoMergeMergeDist);
        host.summary.tuning_near_dist = host.tuning.at(bsp::kAiTuningFreeAttackNearDist);
        host.summary.tuning_far_dist = host.tuning.at(bsp::kAiTuningFreeAttackFarDist);
        host.summary.tuning_sticky = host.tuning.at(bsp::kAiTuningFreeAttackExistingTargetMul);
        host.done("AiController::load_tuning", 0x00a335d0u);
    }
    for (int party = 0; party < bsp::kAiGroupPartySlotCount; ++party) {
        if (!host.party_record[static_cast<std::size_t>(party)]) continue;
        GameAiPartyRow& row = host.party_row(party);
        row.record_enabled = true;
        // 009FFE50 BSP_Ai_IsPartyAiEnabled: in game modes 0 to 3 only slots 0
        // and 4 are enabled; above 3 every slot is.
        row.ai_enabled = bsp::ai_party_ai_enabled(host.game_mode(), party);
    }
    host.record("AiController::create", 0x00a32350u);
    host.record("AiController::construct", 0x00a31730u);
}

void GameAiCoordinatorHost::Impl::build_squadrons() {
    // 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen allocates the 0x414 block
    // and 007F2C60 stamps +C4h = 18h and zeroes the five member slots at
    // 007F2DA3..007F2DBB; the slot-39 attach 007F4580 then reads `WingCount`
    // (007F4735 default 3, 007F4754 max(1, authored)) and runs the per-wing
    // loop whose tail 007F4B43..007F4B6E fills +3D0h and bumps +3CCh.
    //
    // This process creates exactly one unit per scene entity, and a scene
    // aircraft entity IS a PlaneSquadronGen, so the unit the loader made is the
    // squadron's first wing. What is built here is therefore the squadron
    // object and a one-wing member array; the remaining wings are NOT spawned,
    // because unit creation belongs to the units host. Labelled substitution
    // for 007F4580's loop, complete for the member array and the class id.
    squadrons.clear();
    unit_owned_by_squadron.assign(units.count(), false);
    // A name-to-unit map, because a SCENE-ROW squadron's registry record never
    // gets its member units filled. Only the air-ops path fills them
    // (game_hosts_script_orders.cpp, `squadron.member_units.push_back(unit)`);
    // the scene path clears the array and then fills only `member_names` and
    // `member_spawn_index`, queueing the wings into `pending_squadron_members`,
    // and the `resolve_member_units` the registry's header says makes
    // find_by_member_unit valid does not exist anywhere. Measured on USN04 with
    // the unit lookup alone: 7 squadrons over 15 planes, which is its four
    // air-ops wings grouped correctly and its one scene wing seeding three
    // squadrons of its own. Resolving by name closes that.
    std::map<std::string, std::size_t> unit_by_name;
    for (std::size_t u = 0; u < units.count(); ++u) {
        const std::string named = unit_name(u);
        if (!named.empty()) unit_by_name.emplace(named, u);
    }
    // Fill the registry's +3D0h array for the routes that only named their
    // wings. This is the write-back itself, not a local workaround: it makes
    // find_by_member_unit answer for a scene-row squadron for EVERY caller,
    // which matters beyond this function because the torpedo release-order
    // binding reads +3CCh and +3D0h through it and answered zero members for a
    // scene row such as USN01's Mavs.
    const std::size_t resolved = bsp::plane_squadron_registry().resolve_member_units(
        [&unit_by_name](const std::string& named) -> std::size_t {
            const auto found = unit_by_name.find(named);
            return found != unit_by_name.end() ? found->second
                                               : bsp::kPlaneSquadronNoUnit;
        });
    if (resolved != 0) {
        log.notef("plane squadrons: resolved %zu member unit(s) from member names "
            "(+3D0h, the write-back the scene and GenerateObject routes never did; "
            "the air-ops route fills its own slots and is left alone)", resolved);
    }
    for (std::size_t unit = 0; unit < units.count(); ++unit) {
        // 009FE0F0's air test: the plane base 0Fh. A squadron's own members are
        // planes, and nothing else in the scene produces one.
        if (!units.unit_is_kind_of(unit, bsp::kPlaneSquadronMemberKindId)) continue;
        // Packet cc8_plane_squadron_host (15563fdf9) spawns a squadron's real
        // WingCount wingmen, so a plane can now be a MEMBER of a squadron
        // rather than a squadron in its own right. Seeding one squadron per
        // member is exactly the double-order the comment on
        // unit_owned_by_squadron forbids - "keeping both in a group
        // double-orders the same aircraft" - and it showed as 15 AI squadrons
        // on a USN04 that has 5. The registry's back pointer is this process's
        // stand-in for plane+9D4h: a plane whose squadron names another unit as
        // its flight leader is a wingman and is not a seed.
        // The name fallback this used to carry is deleted: after the write-back
        // above, member_units is filled for every route, so a name lookup could
        // only ever agree with the unit lookup or disagree with it, and a second
        // answer that can disagree is worse than one that cannot.
        const bsp::PlaneSquadronHostRecord* owner =
            bsp::plane_squadron_registry().find_by_member_unit(unit);
        // The wing in +3D0h order, skipping the slots whose plane never became
        // a unit; live_count()'s rule.
        std::vector<std::size_t> wing_units;
        if (owner != nullptr) {
            for (const std::size_t member : owner->member_units) {
                if (member != bsp::kPlaneSquadronNoUnit) wing_units.push_back(member);
            }
        }
        // A record that resolves to nothing is no owner: without this the
        // leader check would fail for every plane in it and drop them all.
        if (wing_units.empty()) {
            owner = nullptr;
            wing_units.assign(1, unit);
        }
        // 007EDA91 reads slot 0 whatever the count: only the flight leader seeds.
        if (wing_units.front() != unit) continue;
        Squadron s;
        // 007F4778 stores the wing count at +3C8h. Take the authored WingCount
        // the registry carries when there is one; the absent-key arm 007F4735,
        // which defaults to 3, is only right for a plane in no squadron.
        s.entity.wing_count = owner != nullptr
            ? owner->wing_count
            : bsp::plane_squadron_wing_count_007f4754(false, 0);
        // 007F4B43 is still the rule that fills +3D0h and bumps +3CCh. Only the
        // SOURCE of the members changes: the registry's array in array order
        // for a real squadron, and this unit alone otherwise.
        bool attached = false;
        for (const std::size_t plane : wing_units) {
            int spawn_index = 0;
            if (!bsp::plane_squadron_attach_plane_007f4b43(
                    s.entity, handle(plane), &spawn_index)) {
                continue;
            }
            s.member_units.push_back(plane);
            if (unit_owned_by_squadron.size() <= plane) {
                unit_owned_by_squadron.resize(plane + 1u, false);
            }
            unit_owned_by_squadron[plane] = true;
            ++summary.squadron_members;
            attached = true;
        }
        if (!attached) continue;
        squadrons.push_back(std::move(s));
        ++summary.squadrons_built;
    }
    if (!squadrons.empty()) {
        log.notef("ai squadrons: %llu PlaneSquadronGen objects built over %llu member "
            "planes (004F0AD0 + 007F2C60 + 007F4580's +3D0h tail); each carries class "
            "id 18h, so 009FE080's 009FE088 arm now answers for them and its 009FE092 "
            "negation of 007EDA90 is what admits them",
            static_cast<unsigned long long>(summary.squadrons_built),
            static_cast<unsigned long long>(summary.squadron_members));
    } else {
        log.notef("ai squadrons: this mission created no unit answering IsKindOf(0Fh), "
            "so no PlaneSquadronGen is built and 009FE080 falls to its ship tail");
    }
    record("SceneUnit::create_plane_squadron_gen", 0x004f0ad0u);
    record("PlaneSquadron::construct", 0x007f2c60u);
    record("PlaneSquadron::attach_planes", 0x007f4580u);
}

void GameAiCoordinatorHost::fixed_step(float step_seconds) {
    Impl& host = *impl_;
    if (!host.created) return;
    host.clock_seconds += step_seconds;
    ++host.summary.compose_passes;
    ++host.summary.party_think_calls;
    // 00A32D50: the gate, then 00A2E720, then 00A182C0. The float is discarded.
    bsp::ai_coordinator_fixed_step_00a32d50(host);
    host.done("AiController::fixed_step", 0x00a32d50u);
}

const GameAiSummary& GameAiCoordinatorHost::summary() const noexcept {
    return impl_->summary;
}

const std::vector<GameAiPartyRow>& GameAiCoordinatorHost::party_rows() const noexcept {
    return impl_->parties;
}

void GameAiCoordinatorHost::report() {
    Impl& host = *impl_;
    const GameAiSummary& s = host.summary;
    std::size_t with_task = 0;
    for (const Impl::Group* g : host.registry) {
        if (g != nullptr && bsp::ai_group_command_is_engaged(g->command.type, 0.0f, 0.0f)) {
            with_task += g->members.size();
        }
    }
    host.summary.units_with_task = static_cast<unsigned long long>(with_task);
    host.log.notef("summary mission ai coordinator game_mode=%d compose=%llu seeds=%llu "
        "groups_created=%llu destroyed=%llu members_added=%llu evicted=%llu splits=%llu "
        "splits_taken=%llu auto_merges=%llu prox_merges=%llu member_passes=%llu "
        "tick_orders=%llu tick_followers=%llu formation_requests=%llu promotions=%llu "
        "collect_dist=%.1f served=%llu attackmove=%llu settarget=%llu fallback=%llu "
        "scored=%llu",
        s.game_mode, s.compose_passes, s.seed_candidates, s.groups_created,
        s.groups_destroyed, s.members_added, s.members_evicted, s.splits,
        s.splits_taken, s.auto_merges, s.proximity_merges, s.member_passes,
        s.tick_orders, s.tick_followers, s.tick_formation_requests,
        s.command_promotions,
        static_cast<double>(host.tuning.at(bsp::kAiTuningCloseAttackCollectDist)),
        s.close_members_served, s.close_attack_move_orders, s.close_set_target_orders,
        s.close_fallback_movetos, s.close_candidates_scored);
    host.log.notef("summary mission ai tuning mode=%d (%s) merge_dist=%.1f "
        "near=%.1f far=%.1f sticky=%.2f",
        s.tuning_mode, bsp::ai_tuning_mode_table_name(
            static_cast<bsp::AiTuningMode>(s.tuning_mode)),
        static_cast<double>(s.tuning_merge_dist),
        static_cast<double>(s.tuning_near_dist),
        static_cast<double>(s.tuning_far_dist),
        static_cast<double>(s.tuning_sticky));
    host.log.notef("summary mission ai parties calls=%llu thought=%llu planner_ticks=%llu "
        "claims=%llu spawn_arms=%llu attack_orders=%llu cautious=%llu movetoattack=%llu "
        "commands=%llu refused=%llu units_with_task=%llu first_command=%.2f s",
        s.party_think_calls, s.parties_thought, s.planner_ticks, s.planner_claims,
        s.planner_spawn_arms, s.attack_orders, s.attack_cautious, s.attack_movetoattack,
        s.commands_issued, s.commands_refused, s.units_with_task,
        static_cast<double>(s.first_command_seconds));
    {
        // How many group members are squadrons, counted over the live registry.
        unsigned long long squadron_group_members = 0;
        for (const Impl::Group* g : host.registry) {
            if (g == nullptr) continue;
            for (const std::size_t member : g->members) {
                if (host.is_squadron(member)) ++squadron_group_members;
            }
        }
        host.summary.squadron_group_members = squadron_group_members;
    }
    host.log.notef("summary mission ai squadrons built=%llu members=%llu "
        "group_members=%llu excluded_007eda90=%llu squadron_commands=%llu "
        "member_orders=%llu (004F0AD0 / 007F2C60 / 007F4580 +3D0h; 009FE080's "
        "18h arm; 007ECF80 fan-out)",
        host.summary.squadrons_built, host.summary.squadron_members,
        host.summary.squadron_group_members, host.summary.squadron_excluded,
        host.summary.squadron_commands, host.summary.squadron_member_orders);
    host.log.notef("summary mission ai order dedupe suppressed=%llu (duplicate re-issues "
        "of the same token/target/point; 0077D600 replaces, this ring appends)",
        host.summary.orders_suppressed);
    host.summary.objective_set_units =
        static_cast<unsigned long long>(game_objective_sets().total_units());
    host.log.notef("summary mission ai world sets queries=%llu hits=%llu objective_units=%llu "
        "(00A2C450 over game+21A4h..+21C0h, the eight per-player-slot SzurkeNyil objective "
        "sets; their producer 008CD440 Objectives_Add is unimplemented here, so every set is "
        "empty and the native's 00A2C4B4 arm is the answer)",
        host.summary.world_set_queries, host.summary.world_set_hits,
        host.summary.objective_set_units);
    host.log.notef("summary mission ai target weight queries=%llu objective_hits=%llu "
        "fort_targets=%llu non_command=%llu (00A0F810's four multipliers: 10.0 objective, "
        "0.1 for the 009FE0B0 trio, 0.01 when that trio is not a command building)",
        host.summary.weight_queries, host.summary.weight_objective_hits,
        host.summary.weight_fort_targets, host.summary.weight_non_command_targets);
    host.log.notef("summary mission ai target weight health torn_down_targets=%llu "
        "(00923BE0's +5Dh arm; the fraction unit+370h/unit+36Ch has no producer here, so a live "
        "candidate takes the full-health 1.0 and slot D is 1.0)",
        host.summary.weight_torn_down_targets);
    {
        // A row is incomplete only when it carries a Rocket barrel, whose
        // small/big split 009FE4F1 makes through unread target-state
        // predicates, so the gap between the two counts is the rocket cost.
        std::size_t complete = 0;
        for (const GameAiWeaponFacts::Unit& row : game_ai_weapon_facts().units) {
            if (row.known && row.inputs_complete) ++complete;
        }
        host.log.notef("summary mission ai target weight base model_runs=%llu "
            "class_stand_ins=%llu weapon_rows=%zu complete_rows=%zu (00A08460 runs when the "
            "weapon-facts table carries a COMPLETE row for both the attacker and the target; "
            "an incomplete row is one with a Rocket barrel, sub-type 12h)",
            host.summary.weight_model_runs, host.summary.weight_class_stand_ins,
            game_ai_weapon_facts().known_units(), complete);
    }
    {
        // Packet cc8_ai_target_weight_zero: one line per (bullet sub-type,
        // target group) pair actually asked for. `accuracy` is what 009FE270
        // answers for the pair and `contribution` the sum of
        // time_factor * accuracy * shots over every lookup, which is what
        // 00A08460 accumulates. A pair with lookups and a zero contribution is
        // a barrel that can never move the weight.
    {
        // Packet cc8_ai_target_choice_observed: which class the weight actually
        // picked, over the whole mission. This is the line no earlier run in
        // this stream carried, and the one that turns the preference from a
        // calculation into a measurement.
        host.log.notef("summary mission ai target choice model=%d chosen_classes=%zu "
            "runnerup_classes=%zu (00A14A6E's order, the target 00A149A8 left in the slot)",
            GameAiCoordinatorHost::Impl::ai_weight_model_enabled() ? 1 : 0,
            host.chosen_class_counts.size(), host.runnerup_class_counts.size());
        for (const std::pair<const int, unsigned long long>& entry : host.chosen_class_counts) {
            unsigned long long as_runner = 0;
            const auto found = host.runnerup_class_counts.find(entry.first);
            if (found != host.runnerup_class_counts.end()) as_runner = found->second;
            host.log.notef("  ai target choice class=%02Xh(%s) chosen=%llu runnerup=%llu",
                entry.first,
                GameAiCoordinatorHost::Impl::ai_class_name(entry.first),
                entry.second, as_runner);
        }
        for (const std::pair<const int, unsigned long long>& entry : host.runnerup_class_counts) {
            if (host.chosen_class_counts.find(entry.first) != host.chosen_class_counts.end()) {
                continue;
            }
            host.log.notef("  ai target choice class=%02Xh(%s) chosen=0 runnerup=%llu",
                entry.first,
                GameAiCoordinatorHost::Impl::ai_class_name(entry.first), entry.second);
        }
    }
        host.log.notef("summary mission ai target weight path plane_attacker=%llu "
            "other_attacker=%llu (00A085AD PUSH 0Fh on the attacker vehicle class, stored to "
            "[ESP+37h]; 00A08619 JE 00A09228 sends a NON-plane attacker to the subsystem and "
            "barrel walk this process projects, and a plane attacker into 00A0861F..00A09222, "
            "which it does not)",
            host.census_plane_attacker, host.census_other_attacker);
        for (const std::pair<const int, unsigned long long>& entry :
             host.unresolved_by_attacker_class) {
            host.log.notef("  ai target weight unresolved_bullet_class attacker=%02Xh(%s) "
                "barrel_lookups=%llu (gun.bullet_class < 0, or a Bullets row whose Type matches "
                "none of 006EA910's thirteen literals)",
                entry.first, GameAiCoordinatorHost::Impl::ai_class_name(entry.first),
                entry.second);
        }
        static const char* const kGroupNames[] = {
            "plane", "submarine", "smallship", "bigship", "other"};
        static const char* const kSubTypeNames[] = {
            "?0", "bullet_raw", "machinegun", "machinegun_aa", "artillery_raw",
            "artillery_light", "artillery_medium", "artillery_heavy", "?8", "bomb",
            "torpedo", "depthcharge", "dummytarget", "dummykamikaze", "dummysub",
            "paratrooper", "flak", "kamikaze", "rocket", "watermine"};
        for (int sub = 0; sub < GameAiCoordinatorHost::Impl::kCensusSubTypes; ++sub) {
            for (int grp = 0; grp < GameAiCoordinatorHost::Impl::kCensusGroups; ++grp) {
                const auto& row = host.accuracy_census[sub][grp];
                if (row.lookups == 0) continue;
                host.log.notef("summary mission ai target weight accuracy "
                    "subtype=%02Xh(%s) group=%s lookups=%llu accuracy=%.4f "
                    "zero=%llu unresolved=%llu contribution=%.3f",
                    sub, kSubTypeNames[sub], kGroupNames[grp], row.lookups,
                    static_cast<double>(row.accuracy), row.zero_accuracy,
                    row.unresolved, row.contribution_sum);
            }
        }
    }
    for (const GameAiPartyRow& row : host.parties) {
        host.log.notef("  ai party %d record=%d ai_enabled=%d brain=%d thinks=%llu "
            "claims=%llu planner_ticks=%llu attacks=%llu commands=%llu refused=%llu",
            row.party, row.record_enabled ? 1 : 0, row.ai_enabled ? 1 : 0,
            row.brain_created ? 1 : 0, row.thinks, row.groups_claimed, row.planner_ticks,
            row.attack_orders, row.commands_issued, row.commands_refused);
    }
    for (const Impl::Group* g : host.registry) {
        if (g == nullptr) continue;
        host.log.notef("  ai group team=%d party=%d members=%zu claimed=%d command=%s "
            "target=%d leader=%s weight=%.3f", g->team, g->party, g->members.size(),
            g->claimed_by != nullptr ? 1 : 0,
            bsp::ai_command_type_name(g->command.type),
            g->command.target_group != nullptr ? 1 : 0,
            g->members.empty() ? "" : host.unit_name(g->members.front()).c_str(),
            g->members.empty() ? 0.0
                : static_cast<double>(host.unit_leader_weight(g->members.front())));
    }
}

}  // namespace bsp::game
