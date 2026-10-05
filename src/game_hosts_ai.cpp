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
#include <limits>
#include <utility>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "bsp/ai_command_lifetime.hpp"
#include "bsp/attack_commands.hpp"
#include "bsp/ai_command_object.hpp"
#include "bsp/ai_close_attack_tick.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/ai_command_tick.hpp"
#include "bsp/ai_group_think.hpp"
#include "bsp/ai_planners.hpp"
#include "bsp/ai_planner_tails.hpp"
#include "bsp/ai_tuning_globals.hpp"
#include "bsp/building_pads.hpp"
#include "bsp/ai_target_weights.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_commands.hpp"
#include "bsp/ordnance_kinds.hpp"
#include "bsp/plane_squadron_entity.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/unit_gunnery_pass.hpp"

namespace bsp::game {

// Packet cc9_planner_join_loopback (docs/SHIP_AI_OPEN_ITEMS.md section 94). The
// planner's follower pass 00A10DC0 asks 0077C8D0 (at 00A10E67), which routes the
// same 76h join as JoinFormation (0077C964 -> 0077C2A0 flags 7 -> 0076E520): the
// join reaches 0077F940 at its turn in the next 0076C600 drain, not at the call.
// True: posted through commands_post_loopback_callback_0076e520, as section 92
// does for the script's joins. False: joined at the call, as before.
// ON by section 94.4: posted equals delivered in JM08 36000 (39), USNOS (10) and
// USN02 (7); USN02 exit 1, USNOS and JM08 exit 3 by spread (base objects).
inline constexpr bool kPlannerJoinLoopbackBound = true;

// Packet cc9_group_transport_moves (docs/SHIP_AI_OPEN_ITEMS.md section 96).
// 00A11B80, the middle call of CLOSEATTACK (00A15490, with the target group),
// DEFENDPOSITION (00A15500) and PATROLTO (00A15690, both with none): each
// member that is a ship (vt+5Ch(6)) whose class carries landing craft
// (00827FB0) is either ready (008128E0: an enemy list-28 site within its
// LandingRange with a free pad), when 94h MT_SHIP_STARTLANDING is routed for
// it, or moved by 00A02020 to the point 0.75 x CaptureRange from the anchor
// (the target group's first kind-1Ch member, or with no target
// group the nearest list-28 entity of another team). True: the moves are
// issued. False: counted only. Either way 94h is counted here; its two
// receivers are delivered by the two switches below.
// ON by section 96.4: JM08 36000, JM08, IJN01, JM06 and LOMP06 exit 1; USNOS
// and USNOS long exit 3 with no death row flipped (convoys steer for CB2).
inline constexpr bool kAiGroupTransportMovesBound = true;

// Packet cc9_transport_moves_order_bridge (docs/SHIP_AI_OPEN_ITEMS.md 107). True:
// 00A11B80's move (00A11F23 CALL 00A02020) passes the order bridge as the command
// ticks' leader and follower orders do: the class gate, the 80 m gate (00A0206E)
// and a ship's 00417B10 avoid-zone point (30 m margin, y = 0). False: the point on
// the 0.75 x CaptureRange circle is issued directly, as before. ON by section 107.5:
// JM08 36000's transports hold in deep water; JM08 long and USNOS (long) identical.
inline constexpr bool kAiTransportMovesOrderBridgeBound = true;

// Packet cc9_close_fallback_order_bridge (docs/SHIP_AI_OPEN_ITEMS.md 112). True:
// 00A13B60's no-candidate fallback (00A14D48 CALL 00A02020, ECX = member,
// EDX = the 0.4/0.6 point) passes the order bridge as 00A11B80's move does: the
// class gate (a squadron unless 007EDA90, else IsKindOf(6)), the 80 m gate and
// a ship's 00417B10 avoid-zone point (30 m margin, y = 0). False: the point is
// issued directly, as before. Counted both ways. ON: section 119 (USN13 moves
// paths only; the other pairs are gameplay-identical).
inline constexpr bool kCloseFallbackOrderBridgeBound = true;

// Packet cc9_startlanding_94h (docs/SHIP_AI_OPEN_ITEMS.md section 97). The 94h
// that 00A11B80 routes for a ready member reaches 00821F61 -> vt+238h: on an
// MLandingShip (vtable 00CFFA30) that is 0074A4C0, the landing request that
// takes the nearest free pad of the site and begins the landing (0A5h ->
// 0074A990). True: delivered to kind-0Ch members at the call. False: counted.
// The troop transport's 008206F0 (vtable 00CFA778, the craft launch) stays
// counted either way.
// ON by section 97.4: JM08 36000 begins 3 landings (LST 01, 03, 02) and exits 3;
// the ramp missed by 0.35 s to the defenders; USNOS identical.
inline constexpr bool kLandingShipStartLandingBound = true;

// Packet cc9_landing_craft_launch (docs/SHIP_AI_OPEN_ITEMS.md section 100). The
// same 94h on the troop transport (vtable 00CFA778) reaches 008206F0, which
// launches one landing craft per free pad of the site from the rings around the
// hull and sets ship+1124h, the cooldown 008128E0 tests. True: delivered at the
// call (GameShipAiHost::transport_launch_craft_008206f0) and the +1124h test
// applies. False: counted, and +1124h reads 0.0.
// ON by section 100.4: JM08 36000 launches 8 crafts from USTroopTransport 01 at
// t=632.75, which hold every pad (0074A4C0 begins 3 -> 0); 3 ramps lower, no
// flip; exit 3. USNOS gameplay-identical.
inline constexpr bool kLandingCraftLaunchBound = true;

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

GameObjectiveSets::Objective* GameObjectiveSets::add_objective(int slot,
    const std::string& name, int kind) {
    const unsigned long long before = adds;
    Objective* o = add_objective(slot, name);
    // 008E1F80 constructs the record with the kind as 008DD5C0's third
    // argument (+18h at 008DD684). An existing record keeps its own.
    if (o != nullptr && adds != before) o->kind = kind;
    return o;
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

bool GameObjectiveSets::set_status(int slot, const std::string& name, int status) {
    if (slot < 0 || static_cast<std::size_t>(slot) >= kSlotCount) return false;
    for (Objective& o : slots[static_cast<std::size_t>(slot)]) {
        if (!(o.name.size() == name.size() && _stricmp(o.name.c_str(), name.c_str()) == 0)) {
            continue;
        }
        // 008E216F 008E1D30, the announcement: sound only, not modelled here.
        // 008E2177 008DFE50: every live unit of the objective goes to 008DFC00.
        // SUBSTITUTION, labelled: the walk's early return for a hidden
        // objective (+18h == 2) is not applied, because this host does not
        // record the kind; on the measured missions no objective holds a unit.
        if (kObjectiveKindBound && o.kind == 2) {
            // Packet cc9_objective_kind: 008DFE50 returns at 008DFE6E for a
            // hidden objective, so its units stay listed.
            ++hidden_status_holds;
            o.state = status;                             // 008E2181
            ++status_sets;
            return true;
        }
        status_unit_drops += o.units.size();
        unit_removes += o.units.size();
        o.units.clear();
        o.state = status;                                 // 008E2181
        ++status_sets;
        return true;
    }
    ++status_misses;
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
// Packet cc9_planner_kate_targeting: the planner's range factor as 00A1CD95
// forms it. docs/PLANNER_KATE_TARGETING.md. ON (2026-09-24, packet
// cc9_group_composition): the R1 torpedo loss was a frozen dead leader point, fixed by the
// plane death flags; the H0/H1 pair's moves are the range law re-weighting the ship group's
// picks.
constexpr bool kPlannerRangeInterpBound = true;
// DIAGNOSTIC, packet cc9_yorktown_order_split: past the 20-line cap, every
// kAiMovetoDiagEvery-th MOVETOATTACK tick is still logged with the target
// group's leader, so the 00A12A90 collect-distance promotion can be timed.
// Off (0) in the landed build.
constexpr int kAiMovetoDiagEvery = 0;
// Packet cc9_ai_tick_strafe: the leader point of a group whose first member is
// a plane squadron. 00A10C20 (00A10C44-00A10C54) refreshes and returns the
// member's own +FCh, which for a squadron this host answers from its flight
// leader (proxy(), as tick_member_position already does and as GetPosition's
// 008A7C3C read of the same field does). OFF, the squadron's candidate index
// falls outside the unit rows and the point reads (0,0,0): on ESMP08 the US
// strike group measured 15712 to Zuikaku from the origin and never closed, and
// its followers were sent to the origin. docs/SQUADRON_LAND_TASK.md 5cg.
// ON (2026-09-30): controls identical, USNOS and ESMP08 moved; the ESMP08
// promotion miss is recorded in 5cg.
constexpr bool kAiSquadronLeaderPointBound = true;
// Packet cc9_ai_tick_plane_retask: a plane that receives the AI command tick's
// moveto (00A02020's tail, 0077D600 00E08F68 through the squadron fan-out) is
// re-tasked from it. In the image the plane's bot tick sees the command change
// (0099AE7E -> 0099A4C0): a strafe head (vtable 00D210E0) keeps itself only
// while 009CC850 finds the current command settarget 00E08F40 or 00E08F78 on
// the same target, so a moveto retires it and 0099A170's moveto arm
// (0099A23A) builds the kind-7 task 009C3BE0; a kind-7 head is kept when
// 009C31B0 finds the new point within 100 m (squared 10000, 00CE3D64) of its
// own. OFF, this host placed the moveto on the director and left the old task
// flying: ESMP08's strike leader kept strafing its script target and circled
// 3.2-4 km off Zuikaku. The install runs at the delivery, one bot tick early,
// as PilotMoveTo's does (SENTITY_INIT_ATTACH_ORDER 22.7). docs/SQUADRON_LAND_TASK.md 5ch.
// ON (2026-09-30): controls identical; ESMP08 now promotes to CLOSEATTACK (5ch).
constexpr bool kAiTickMovetoRetasksPlaneBound = false;   // OFF again: SQUADRON_LAND_TASK 5co

// Packet cc9_plane_attacker_weight, docs/SQUADRON_LAND_TASK.md 5cq. 00A08460's
// class type queries (vtable[+18h]) were a stub answering false, so a plane
// attacker never took the type-0Fh branch and the non-plane walk ran without
// its target gates.
// kAiWeightBarrelGatesBound true: the type queries answer through the unit's
// own kind test (LABELLED: the class descriptor's slot, asked of the unit), and
// the walk applies 00A0924C..00A0943D's per-target gates and 00A09362..00A093A8's
// platform filters.
// kAiPlaneAttackerWeightBound true: a 0Fh attacker takes the plane arm
// 00A0861F..00A09222 (no-loadout arm: kamikaze, strafe, dogfight and tail-gun
// options) and the 00A09771 x3 bonus. Both false: the earlier substitution.
constexpr bool kAiWeightBarrelGatesBound = false;
constexpr bool kAiPlaneAttackerWeightBound = false;

// Packet cc9_ship_natives_2, docs/SHIP_NATIVES_2.md. True: 009FFD70
// BSP_Entity_AiClassWeight (ECX = [leader+0C4h], JMP 009FDF30) is the group
// leader's class weight out of the tuning block, read at 00A2EB97/00A2EBA2
// (the auto-merge pair order) and 00A10D8B/00A10D96 (the merge leader test).
// False: the leader's unit index, the previous stand-in.
constexpr bool kAiLeaderOrderKeyBound = true;

// Packet cc9_generated_squadron_brain_membership, docs/GENERATED_SHIP_AI.md
// section 7. The image's compose phase 3 (00A2E835..00A2EA5A) walks the live
// entity lists hung off world+19CCh on every pass, so a PlaneSquadronGen created
// after load (SpawnNew, GenerateObject, an air-ops launch) is a seed candidate
// from the pass after it is registered, like a loaded one. True: squadrons whose
// flight leader was created after build_squadrons are built by the same seed
// body, and the squadron candidates keep their identity when the unit count
// grows (their index is units.count() + i, so every stored squadron index is
// shifted by the growth). False: the squadron list is the load-time one, and a
// stored squadron index that the unit growth overtakes reads as a unit.
// Pairs: USN04 and USN13 moved (brain orders reach the generated squadrons),
// USN01 and USN02 identical (docs/GENERATED_SHIP_AI.md section 7). ON after the
// order-ring read (docs/ORDER_RING_REPLACE.md): with both switches ON, USN13 and
// USN04 are identical to the membership-only rows, so the moves are this rule's.
constexpr bool kGeneratedSquadronBrainBound = true;
// Packet cc9_party_replan_flag, docs/SHIP_AI_OPEN_ITEMS.md section 3. The
// planner's replan byte +2Ch: the claim 00A22750 sets it at 00A227A9 when
// 00A1C8B0 finds the group not yet in the planner's own list, and 00A15970
// (from 00A182C0) answers, outside effective game modes 4..7, the OR of
// 00A18480 over brain+0h..+0Ch, which reads and clears it. True: the byte and
// its query are live, so a claim makes the party think again on the next call.
// False: 00A15970 answers false (the previous stand-in).
constexpr bool kAiPartyReplanFlagBound = true;  // ON: pairs held, spread misses recorded (docs/SHIP_AI_OPEN_ITEMS.md section 3)

// Packet cc9_order_ring_replace, docs/ORDER_RING_REPLACE.md. True: an AI order
// that repeats the entity's previous one (same token and target, point within
// a metre) is issued again, as the image issues it: 0077D600 -> 00816E30 with
// flags 1 clears the queue at 0081733E and issues at 0081735D, so the command
// restarts. False: the host's duplicate filter drops it. ON: USN13/USN04 small
// moves alone, none on top of the squadron membership; USN01 identical; USN02
// moved after its 212.91 s failure (docs/ORDER_RING_REPLACE.md section 4).
constexpr bool kAiOrderReissueBound = true;

// Packet cc9_planner_close_attack_choice, docs/PLANNER_TASK_CHOICE.md. True:
// brain slot i holds planner kind i, as 00A15A70 builds it: 00A15C31 Defend ->
// [ESI], 00A15C61 Attack -> +4h, 00A15C92 Sell -> +8h, 00A15CC3 Capture -> +0Ch,
// 00A15ADA Duel -> +10h (mode 4), then Escort, Siege, Competitive. False: the
// permuted table (Attack, Defend, Capture, Duel, Escort, Siege, Competitive).
// ON: USN02, USN13 and USN04 identical (docs/PLANNER_TASK_CHOICE.md section 4).
constexpr bool kAiPlannerSlotKindsBound = true;

// Packet cc9_planner_defend_capture_thinks, docs/PLANNER_TASK_CHOICE.md section 6.
// True: the Attack kind runs 00A1CF90 (00A1CB80 per owned group with the party's
// aggressive ratio, reset 0) and the Capture kind runs 00A29FD0's no-target path
// (release every owned group and hand it to brain+4h, or brain+8h beside an own
// CommandBuilding). A Capture think that has an enemy CommandBuilding still runs
// the Siege-shape stand-in. False: every kind runs the Siege shape. ON: USN02,
// USN04, USN13, USN01 identical (docs/PLANNER_TASK_CHOICE.md section 6.4).
constexpr bool kAiCaptureThinkBound = true;

// Packet cc9_planner_defend_capture_thinks part 2, docs/PLANNER_TASK_CHOICE.md
// section 8. True: a Capture think that has an enemy CommandBuilding runs
// 00A29FD0's target path: the 00A1E250 records (00A2A120-00A2A1E7), the
// per-group pass (00A2A263-00A2AA90), the assignment loop (00A2AAA0-00A2AF40,
// 00A1A720 at 00A2AD77), the 00A1D010 merges (00A2B12F), the hand-off of every
// unassigned group and the spawn arm as a record. False: that think runs the
// Siege-shape stand-in and counts a target_fallback. ON: USN13 and USN01 moved
// (Enterprise's group is ordered at the CommandBuilding group and draws
// CAUTIOUSATTACK against 0.5), USN02 and USN04 identical
// (docs/PLANNER_TASK_CHOICE.md section 8.4).
constexpr bool kAiCaptureTargetPathBound = true;

// Packet cc9_planner_defend_capture_thinks part 3, docs/PLANNER_TASK_CHOICE.md
// section 9. True: the Sell kind (brain+8h) runs 00A22800: 00A2E4C0 splits the
// air members off each owned group into a new group that 00A22750 claims, then
// every owned group whose command is not SELLING (00A2BE10, IsType(0Eh)) gets
// one (vtable 00D229B8, 00A2BD00). False: the Siege-shape stand-in. ON: USN13
// and USN01 identical, no group reaches brain+8h there (section 9.4).
constexpr bool kAiSellThinkBound = true;

// Packet cc9_planner_defend_capture_thinks part 3, docs/PLANNER_TASK_CHOICE.md
// section 10. True: the Defend kind (brain+0h) runs 00A28A60's no-record path.
// When no owned group holds a defend candidate (00A2DEF0: a member that is
// IsKindOf(1Ch) or in a SzurkeNyil set), the record list is empty, so every
// owned group without a groupable combatant, or with a world-set member, gets
// DEFENDPOSITION (00A2BE20), and every other one is released and claimed by
// brain+0Ch. With a candidate the records path (00A243D0 scores, 00A28300,
// the anchors, the merge pass and the spawn tail) is not reconstructed and the
// Siege-shape stand-in runs, counted as record_fallbacks. False: the stand-in.
// ON: USN13 and USN01 moved (the Storage LandFort group holds DEFENDPOSITION,
// and the RNG shift gives Enterprise MOVETOATTACK), USN02 and USN04 identical
// (docs/PLANNER_TASK_CHOICE.md section 10.4).
constexpr bool kAiDefendThinkBound = true;

// Packet cc9_defend_records_path, docs/PLANNER_TASK_CHOICE.md section 12. True:
// 00A28A60 builds its 00A243D0 records and, when one has enemy weight inside
// Defend_CollectEnemiesDist, runs the records path (anchors, DEFENDPOSITION or
// PATROLTO per group, the Capture pairs, the merge pass, the spawn tail as a
// record). False: any defend candidate runs the Siege-shape stand-in. ON:
// LOMP07 moved in its unit table, LOMP10 and the reference four identical; no
// record was kept on any run (docs/PLANNER_TASK_CHOICE.md section 12.4).
constexpr bool kAiDefendRecordsPathBound = true;

// Packet cc9_selling_tick, docs/PLANNER_TASK_CHOICE.md section 13. True: a
// SELLING command (the Sell think's) runs 00A11FF0 every command tick: a group
// without an air member moves its leader to the nearest own list-28 entity (or
// holds at its own leader point inside 0.8 x CaptureRange) and runs the
// follower pass; an air group sends `returntobase` to each squadron. False:
// SELLING has no arm and the group is not moved. ON: LOMP07 moved (Salt Lake
// City sent toward its CommandBuilding), LOMP10 orders only, the reference four
// identical (docs/PLANNER_TASK_CHOICE.md section 13.4).
constexpr bool kSellingTickBound = true;

// Packet cc9_capture_accessors, docs/PLANNER_TASK_CHOICE.md section 14. True:
// 00A03760's radius is the CommandBuilding's authored CaptureRange
// (GameUnitsHost::command_building_capture_range_07a0, 006F2780) and a
// squadron's speed its plane class MaxSpd (plane_class_max_speed_0188,
// [unit+35Ch]+188h); SELLING's stop radius takes the same CaptureRange.
// False: the 500 and 0 stand-ins. ON: USN13, USN01 and LOMP07 identical
// (docs/PLANNER_TASK_CHOICE.md section 14.2).
constexpr bool kCaptureAccessorsBound = true;

// Packet cc9_capture_group_value, docs/SHIP_AI_OPEN_ITEMS.md section 70. True:
// the Capture target path's per-group weight 00A250A0 (called at 00A2A380) is
// 00A0C650 over the group's member records (00A07E40) and the target's defender
// records (00A24870: list-2 entities of the enemy side, IsKindOf 6, 18h or 1Bh,
// within Capture_CollectDefendersDist, else the target alone), the composition
// kPlannerGroupTargetValueBound already runs for 00A0F970. A group whose pairs
// all weigh 0 (no barrels) then fails the 00A2AC02 `w > 0` test, stays
// unassigned and is handed to brain+4h or brain+8h. False: members x defenders,
// always positive, so every group is assigned. ON (2026-09-30): USN13 and USN01
// moved as predicted (28 and 3 Maru/convoy groups handed to Attack), USN04 and
// JM05 gameplay-identical (section 70.1).
constexpr bool kCaptureGroupValueBound = true;

// Packet cc9_planner_group_target_value (rank 10), docs/SHIP_AI_OPEN_ITEMS.md
// section 7. True: the planner's candidate base term is 00A0F970's group target
// value, (AttackSumMul * attack sum + attack maxes sum) / ReferenceWeight over
// every (own member, candidate member) pair, with the penalties zeroed because
// 00A1CC61 passes 0 as the first argument (00A0CE71). False: the candidate's
// population, the stand-in this host has carried since the planner landed. The
// census line `summary mission ai group target value` runs in both states.
// ON (2026-09-28): the orders moved as predicted on USN02, JM06 and LOMP06
// (exit 3); USN04 and USN01 gameplay-identical (section 7, the pairs).
constexpr bool kPlannerGroupTargetValueBound = true;

// bsp::AiTargetWeightModelHost over the process-wide weapon-facts table, so
// 00A08460 BSP_Ai_TargetWeight runs for real as soon as something publishes a
// row. Every method names the native site it stands at. The entity pointers
// the key carries are this process's unit handles, index + 1.
// docs/AI_TARGET_WEIGHT_TERMS.md term 2.
// Packet cc9_ai_target_weight_damage_terms: the census of the bound barrel loop
// and the sums of the model's answers at its two call sites, in both states.
struct AiDamageTermsCensus {
    unsigned long long barrels{0};
    unsigned long long armour_refusals{0};
    unsigned long long zero_per_hit{0};
    double per_hit_sum{0.0};
    double close_weight_sum{0.0};
    double group_weight_sum{0.0};
};
AiDamageTermsCensus& ai_damage_terms_census() {
    static AiDamageTermsCensus census;
    return census;
}

// Packet cc9_plane_attacker_weight: the type queries the model asks, by code,
// and how often the unit's own kind test answers true (whether or not the
// answer is used); the plane arm's options; the walk's gate refusals.
struct AiPlaneWeightCensus {
    unsigned long long queries[0x30]{};
    unsigned long long true_answers[0x30]{};
    unsigned long long stub_divergences{0};   // answered false, the kind test says true
    unsigned long long plane_arm_calls{0};
    unsigned long long plane_arm_no_options{0};
    unsigned long long plane_arm_loadout{0};
    unsigned long long options[3]{};          // kamikaze, strafe, dogfight
    unsigned long long positive_options[3]{};
    double option_value_sum[3]{};
    unsigned long long gate_refusals[0x14]{};
    unsigned long long gate_admits{0};
    unsigned long long platform_refusals{0};
};
AiPlaneWeightCensus& ai_plane_weight_census() {
    static AiPlaneWeightCensus census;
    return census;
}

// 0099A170's inputs for a plane the AI tick has just ordered, answered from
// GameUnitsHost as ScriptOrderAttackCommandHost does for the script orders.
class AiTickMovetoBotHost final : public bsp::AttackCommandHost {
public:
    explicit AiTickMovetoBotHost(bsp::game::GameUnitsHost& units) : units_(units) {}
    std::uint32_t unit_weapon_director(std::uint32_t unit) override {
        return index_for(unit) < units_.count() ? unit : 0u;   // 0099A18B
    }
    std::uint32_t director_current_command(std::uint32_t director) override {
        return units_.director_current_command_0071be40(index_for(director));
    }
    std::uint32_t director_current_params(std::uint32_t director) override {
        return director;
    }
    std::uint32_t resolve_target(std::uint32_t) override {
        return 0u;   // a position descriptor (00A0214B): no object
    }
    bool entity_is_kind(std::uint32_t entity, int kind) override {
        return units_.unit_is_kind_of(index_for(entity), kind);
    }
    bool target_still_attackable(std::uint32_t) override { return false; }
    std::uint32_t choose_attack_command(std::uint32_t, std::uint32_t, bool, bool) override {
        return 0u;
    }
    std::uint32_t resolve_return_to_base(std::uint32_t) override { return 0u; }
    std::uint32_t create_bot_task(std::uint32_t, std::uint32_t command,
                                  std::uint32_t) override {
        built_command_ = command;
        return 1u;
    }
    void install_bot_task(std::uint32_t, std::uint32_t) override {}
    bool land_group_available(std::uint32_t) override { return false; }
    std::uint32_t built_command() const noexcept { return built_command_; }

private:
    static std::size_t index_for(std::uint32_t token) noexcept {
        return token > 0u ? static_cast<std::size_t>(token - 1u)
                          : ~static_cast<std::size_t>(0);
    }
    bsp::game::GameUnitsHost& units_;
    std::uint32_t built_command_{0};
};

class AiWeightModelBinding final : public bsp::AiTargetWeightModelHost {
public:
    // The target group is a callback because 009FE270 asks the target itself,
    // so the answer cannot be baked into the published row.
    using TargetGroupFn = std::function<bsp::AiAccuracyTargetGroup(std::size_t)>;
    // The unit's own kind test (vtable[5Ch]), standing in for the class
    // descriptor's vtable[+18h] (packet cc9_plane_attacker_weight).
    using KindFn = std::function<bool(std::size_t, int)>;
    AiWeightModelBinding(const GameAiWeaponFacts& facts, const bsp::AiModeTuning& tuning,
                         const bsp::AiTuningBlock& block, TargetGroupFn group,
                         KindFn kind = KindFn())
        : facts_(facts), tuning_(tuning), block_(block), group_(std::move(group)),
          kind_(std::move(kind)) {}

    // 00A03B90 and 00A079B0, the memo map at 00F8A734. A memo only caches, so
    // skipping it changes no answer; the census counts the queries instead.
    bool memo_lookup(const bsp::AiTargetWeightKey&, float&) override { return false; }
    void memo_store(const bsp::AiTargetWeightKey&, float) override {}
    // 00A31DB0 at 00A08540. The forced rules come from the globals loader's
    // ForcedTargetWeightValues tail, which this process does not run.
    bool forced_rule_weight(const bsp::AiTargetWeightKey&, float&) override { return false; }
    const bsp::AiModeTuning& mode_tuning() override { return tuning_; }
    // Entity vtable +18h and +1Ch. +18h is the class descriptor's type query;
    // packet cc9_plane_attacker_weight answers it with the unit's own kind test
    // (LABELLED) while either of its switches is ON. OFF it stays the stub's
    // false, and the census counts each false the kind test would answer true.
    bool entity_is_type(const void* entity, int code) override {
        const bool real = kind_ ? kind_(index_of(entity), code) : false;
        AiPlaneWeightCensus& c = ai_plane_weight_census();
        if (code >= 0 && code < 0x30) {
            ++c.queries[code];
            if (real) ++c.true_answers[code];
        }
        if (kAiWeightBarrelGatesBound || kAiPlaneAttackerWeightBound) return real;
        if (real) ++c.stub_divergences;
        return false;
    }
    int entity_kind(const void*) override { return 0; }

    float target_hit_points(const void* target) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(target));
        return row != nullptr ? row->hit_points : 0.0f;
    }
    float target_armour(const void* target) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(target));
        return row != nullptr ? row->armour : 0.0f;
    }
    float target_underwater_armour(const void* target) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(target));
        return row != nullptr ? row->underwater_armour : 0.0f;
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
    // Packet cc9_ai_target_weight_damage_terms. 009FE200 at 00A09578 is not a
    // distance falloff, as this binding once labelled it: its four arguments
    // are the barrel's damage range, the target's armour and hit points, and
    // it answers the expected damage per hit (bsp::ai_expected_hit_damage_009fe200).
    // 00424C40+3B0h at 00A09624 is WaterTickDamage, scaling the WaterDamage sum.
    // With kAiTargetWeightDamageTermsBound OFF the model keeps 1.0 for both.
    int barrel_sub_type(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->bullet_sub_type : 0;
    }
    float barrel_damage_low(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->damage_low : 0.0f;
    }
    float barrel_damage_high(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->damage_high : 0.0f;
    }
    float barrel_water_damage(const void* subsystem, int barrel) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        return b != nullptr ? b->water_damage : 0.0f;
    }
    float water_damage_scale() override { return facts_.water_damage_scale; }
    bool damage_terms_bound() override { return kAiTargetWeightDamageTermsBound; }

    // Packet cc9_plane_attacker_weight.
    bool barrel_target_gates_bound() override { return kAiWeightBarrelGatesBound; }
    bool barrel_platform_admitted(const void* subsystem, int barrel) override {
        // 00A09362 +38h != -1 holds for every row (a platform with a Gun[1]
        // takes it as its DefaultGun at 0096149E); 00A0936C +18h == 1 and
        // 00A093A4's 25h refusal are the published platform fields.
        const GameAiWeaponFacts::Barrel* b = barrel_at(subsystem, barrel);
        const bool ok = b != nullptr && b->platform_gun_count == 1 && !b->device_bomb_platform;
        if (!ok) ++ai_plane_weight_census().platform_refusals;
        return ok;
    }
    bool plane_arm_bound() override { return kAiPlaneAttackerWeightBound; }
    // The attacker class's +94h/+98h platforms, one published row each.
    // LABELLED: a platform whose device has no Function category, or whose
    // bullet never resolved, has no row, and the null slots of a gapped
    // Platforms table are not kept; neither changes which options are built.
    int plane_platform_count(const void* attacker) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(attacker));
        return row != nullptr ? static_cast<int>(row->barrels.size()) : 0;
    }
    bool plane_platform(const void* attacker, int index,
                        bsp::AiPlanePlatformFacts& out) override {
        const GameAiWeaponFacts::Barrel* b = barrel_at(attacker, index);
        if (b == nullptr) return false;
        out.present = true;
        out.has_default_gun = true;
        out.gun_count = b->platform_gun_count;
        out.device_is_gun = !b->device_bomb_platform;
        out.device_function = b->category;
        out.pilot_fires = b->platform_pilot_fires;
        out.reload = b->reload;
        out.bullet.present = b->bullet_sub_type != 0;
        out.bullet.sub_type = b->bullet_sub_type;
        out.bullet.damage_min = b->damage_min;
        out.bullet.damage_max = b->damage_max;
        return true;
    }
    bool kamikaze_bullet(const void* attacker, bsp::AiPlaneBulletFacts& out) override {
        const GameAiWeaponFacts::Unit* row = facts_.row(index_of(attacker));
        if (row == nullptr || !row->kamikaze.present) return false;
        out.present = true;
        out.sub_type = row->kamikaze.sub_type;
        out.damage_min = row->kamikaze.damage_min;
        out.damage_max = row->kamikaze.damage_max;
        out.blast_min = row->kamikaze.blast_min;
        out.blast_max = row->kamikaze.blast_max;
        return true;
    }
    // 009FE270 by sub-type, the lookup barrel_accuracy makes for a barrel.
    float bullet_accuracy(const void*, int sub_type, const void* target) override {
        if (!group_) return 0.0f;
        bool resolved = false;
        const std::uint32_t offset = bsp::ai_bullet_type_accuracy_offset_009fe270(
            sub_type, group_(index_of(target)), resolved);
        if (!resolved || offset == 0u) return 0.0f;
        return block_.at(offset);
    }
    void note_plane_option(std::uint32_t descriptor, float value) override {
        AiPlaneWeightCensus& c = ai_plane_weight_census();
        const int slot = descriptor == bsp::kAiPlaneOptionKamikaze ? 0
                         : descriptor == bsp::kAiPlaneOptionStrafe ? 1 : 2;
        ++c.options[slot];
        if (value > 0.0f) ++c.positive_options[slot];
        c.option_value_sum[slot] += value;
    }
    void note_plane_arm(bool loadout_arm, bool no_options) override {
        AiPlaneWeightCensus& c = ai_plane_weight_census();
        ++c.plane_arm_calls;
        if (loadout_arm) ++c.plane_arm_loadout;
        if (no_options) ++c.plane_arm_no_options;
    }
    void note_barrel_gate(int sub_type, bool admitted) override {
        AiPlaneWeightCensus& c = ai_plane_weight_census();
        if (admitted) ++c.gate_admits;
        else if (sub_type >= 0 && sub_type < 0x14) ++c.gate_refusals[sub_type];
    }
    void note_damage_terms_barrel(bool refused, float per_hit) override {
        AiDamageTermsCensus& c = ai_damage_terms_census();
        ++c.barrels;
        if (refused) ++c.armour_refusals;
        else if (!(per_hit > 0.0f)) ++c.zero_per_hit;
        else c.per_hit_sum += per_hit;
    }

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
    KindFn kind_;
};

// 004BCA50 BSP_Game_GetEffectiveGameMode returns [world+614h], remapped by the
// 004BCA5F/004BCA68 arms. This process runs a single-player campaign mission,
// which is mode 0, and mode 0 is what makes ai_party_think_mode answer
// GroupWalk and 009FFE50 admit party slots 0 and 4 only. The value is a
// labelled substitution: the world singleton's +614h has no producer here.
constexpr int kCampaignGameMode = 0;

// Packet cc9_ai_party_gate, docs/SHIP_AI_OPEN_ITEMS.md section 60. True: the
// single-player rule of the image. game+61Ch, the forced-mode byte, is 0 in a
// campaign (its writers are 004BC890, forced only by the 004E27E0 command-line
// switches, and the constructor 004DDFB7), so 004BCA50 answers 8 and
// 009FFE50 takes its unforced arm: in single player every party slot except 0
// is planned for (009FFE8D..009FFE91, slot != 0). 009FFD20 files a unit whose
// +180h is unauthored (9) under slot 0 when its +54h is the local player's
// team ([game+18CCh + 4*[game+18ECh]]+28h, Player1's Party) and under slot 4
// otherwise, and a brain's team (brain+24h, 00A15A90 CALL 009FFD60) is its
// slot record's +28h, PlayerN's Party with N = slot + 1. So the local side's
// groups get NONCONTROL (00A2E124) and no brain, and the other side is
// planned by the slot-4 brain under Player5's Party. False: the forced arm
// in mode 0 with party = team, the earlier substitution: team 0 is planned,
// team 1 is NONCONTROL. LABELLED: every unit's +180h is taken as 9 (the few
// OwnerPlayer "AI control" entries some scenes author are not modelled), and
// the local slot index game+18ECh is 0. Flipped ON in 60.5, reverted to OFF
// for the section 60.6 re-read, flipped ON again after reference q (60.7).
constexpr bool kAiPartyGateUnforcedBound = true;

// Packet cc9_ai_owner_player_slot, docs/SHIP_AI_OPEN_ITEMS.md section 71. True:
// 009FFD20 reads each unit's +180h, the scene's `OwnerPlayer` resolved through
// global.enums `Players` (0077F1F9, 9 when absent): 8 ("AI control") answers
// -1, so a group led by such a unit is NONCONTROL (00A2E124) and any other
// group evicts it (00A2DE40); 0..7 names that slot. False: +180h is 9 for every
// unit (section 60's labelled substitution). Needs kAiPartyGateUnforcedBound.
// ON (2026-09-30): JM05 and USN12 moved, with no death row changed; LOMP10,
// USNOS, JM08, USN04 and USN13 gameplay-identical (section 71.3).
constexpr bool kAiOwnerPlayerSlotBound = true;

// Packet cc9_group_score_list_release, docs/SHIP_AI_OPEN_ITEMS.md section 61.
// 00A2B8F0 (00A2B8F0-00A2B94D, RET 4, ECX = group+24h) removes the emptied
// group's record from a group's candidate score list (up to 128 records of
// 0ACh at group+24h, count at group+5624h, keyed at record+0A8h). The list's
// writers are 00A2B950 (inserts), 00A2BCC0 (marks record+0A4h), the planner
// clears at 00A1CB9C / 00A2A148 / 00A2A29B and the constructor 00A2DFFF; its
// only record reader is 00A2CDD0 <- 00A2D580 <- 00A31000, a text overlay
// gated on [[00E198C4]+54h]+5 that no rel32 or abs32 in the image reaches.
// No gameplay decision reads it, so this process keeps no list. True: the
// removal is the release loop's concrete no-op. False: it stays a record.
constexpr bool kAiGroupScoreListReleaseBound = true;
// 004BCA50 in a single-player campaign: not forced, no session, and a mode
// other than 8 or 9 becomes 8 (004BCA72).
constexpr int kSinglePlayerEffectiveGameMode = 8;

std::array<int, 8> g_scene_slot_parties{-1, -1, -1, -1, -1, -1, -1, -1};
bool g_scene_slot_parties_published = false;

// Packet cc9_ai_owner_player_slot: unit+180h by scene entity name, for the
// authored values other than 9. LABELLED: keyed by name because this process
// has no entity object; a name authored with two different values is a
// conflict and reads as 9.
std::map<std::string, int> g_scene_owner_players;
std::set<std::string> g_scene_owner_player_conflicts;
// Packet cc9_landing_craft_owner_player: +180h by unit index for run-time units.
std::map<std::size_t, int> g_unit_owner_players;

// 00A2C790's per-member chain reaches the member's own weapon director through
// the ENTITY's vtable[+114h] and reports its state back to the group's AI
// command, whose vt+24h (00A0FC90) discards it. Neither 00A10890 nor 00A109B0
// builds a scene command, and no class in the block acts on the report, so
// issuing a real order at order_attack stays this file's substitution for the
// per-class tick at vt+0Ch, which was not read.

}  // namespace

void ai_publish_scene_slot_parties(const std::array<int, 8>& parties) {
    g_scene_slot_parties = parties;
    g_scene_slot_parties_published = true;
}

void ai_publish_scene_owner_players(const std::vector<std::pair<std::string, int>>& owners) {
    g_scene_owner_players.clear();
    g_scene_owner_player_conflicts.clear();
    g_unit_owner_players.clear();
    for (const auto& row : owners) {
        const auto it = g_scene_owner_players.find(row.first);
        if (it != g_scene_owner_players.end() && it->second != row.second) {
            g_scene_owner_player_conflicts.insert(row.first);
        }
        g_scene_owner_players[row.first] = row.second;
    }
}

void ai_set_unit_owner_player(std::size_t unit, int owner) {
    g_unit_owner_players[unit] = owner;
}

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
        // 00A1F44C / 00A1F451 / 00A1F45E, the Capture constructor: +38h (planner
        // age) and +3Ch (time since the last spawn) start at 0, +40h (the last
        // think's clock) at -1.0 (00D7A260). 00A29FD0 advances them.
        float age_0038{0.0f};
        float since_spawn_003c{0.0f};
        float last_clock_0040{-1.0f};
        // +2Ch, the replan byte: set by the claim 00A22750 (00A227A9), read and
        // cleared by 00A18480. Packet cc9_party_replan_flag.
        bool replan_002c{false};
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
    // g_aiGroupsByParty, 00F8A9E8 + p*0Ch (00A2E06E), keyed by group+5634h.
    // Read only with kAiPartyGateUnforcedBound; OFF walks by_team as before.
    std::array<std::vector<Group*>, bsp::kAiGroupPartySlotCount> by_party;
    std::vector<Group*> emptied;                   // 00F8AA7C / 00F8AA80
    std::array<std::unique_ptr<Brain>, bsp::kAiGroupPartySlotCount> brains{};
    std::array<float, bsp::kAiGroupPartySlotCount> next_think{};
    std::array<bool, bsp::kAiGroupPartySlotCount> party_record{};
    int current_party{-1};           // 00E0E344
    // The ticking brain's team, brain+24h (planner +30h). OFF: the party slot.
    int current_team{-1};
    // Packet cc9_ai_party_gate census.
    unsigned long long party_gate_local_groups{0};
    unsigned long long party_gate_other_groups{0};
    float clock_seconds{0.0f};       // 00F876A4
    std::uint32_t rng{0x2545F491u};  // 00BD2F10's stream, one per run
    bool created{false};
    // Packet cc9_party_replan_flag: the census of the replan byte, on both sides.
    unsigned long long replan_sets{0};         // claims that set +2Ch (image rule)
    unsigned long long replan_immediate{0};    // 00A15970 answers true
    unsigned long long replan_foreign_claims{0};  // the group held another planner

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
        // Built from a registry record, so 007F3970's leave at death (packet
        // cc9_val_squadron_registry) shrinks the live +3D0h array under it.
        bool registry_backed{false};
    };
    std::vector<Squadron> squadrons;
    // A plane the squadron owns is NOT an AI candidate of its own. The native
    // seeds the scene's PlaneSquadronGen entities; the planes exist only in the
    // member array at +3D0h, and 009FE080 would refuse them anyway (009FE0A5
    // pushes the ship base 6, never the plane base 0Fh). Keeping both in a
    // group double-orders the same aircraft.
    std::vector<bool> unit_owned_by_squadron;

    // Packet cc9_order_ring_replace (docs/ORDER_RING_REPLACE.md) corrects the
    // premise below: this process's director queue is NOT appended to. Every
    // AI order goes through the reconstructed 0077D600 -> 00816E30 chain with
    // flags 1, whose tail clears the queue first, exactly as the image does. The
    // filter is the only departure, and kAiOrderReissueBound removes it.
    // Original note: the native 0077D600 REPLACES an entity's outstanding order; this
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
            // kAiOrderReissueBound: the image has no duplicate filter. The
            // re-issue goes through 0077D600 with flags 1, and 00816E30's tail
            // clears the queue (0081733E) and issues it again (0081735D), so the
            // command restarts. The count above then reads "re-issued".
            if constexpr (!kAiOrderReissueBound) return true;
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
        if (s == nullptr) return index;
        const std::size_t lead = lead_member(*s);
        return lead == bsp::kPlaneSquadronNoUnit ? index : lead;
    }
    // +3D0h[0]. With the leave bound, a member that died has left the
    // registry's array (007BCAA0 -> 007F3970), so slot 0 is the first member
    // still listed there, and an emptied squadron has none (007EDA99 JZ).
    std::size_t lead_member(const Squadron& s) const {
        if (s.member_units.empty()) return bsp::kPlaneSquadronNoUnit;
        if constexpr (bsp::kPlaneSquadronLeaveOnDeathBound) {
            if (s.registry_backed) {
                for (const std::size_t m : s.member_units) {
                    if (bsp::plane_squadron_registry().find_by_member_unit(m) != nullptr) {
                        return m;
                    }
                }
                return bsp::kPlaneSquadronNoUnit;
            }
        }
        return s.member_units.front();
    }
    std::size_t proxy(void* entity) const { return proxy(unit_index_of(entity)); }

    // 007EDA90's three reads, taken on the squadron's flight leader.
    bsp::PlaneSquadronLeadPlaneFacts squadron_lead_facts(const Squadron& s) const {
        bsp::PlaneSquadronLeadPlaneFacts lead;
        const std::size_t leader = lead_member(s);
        if (leader == bsp::kPlaneSquadronNoUnit) return lead;   // 007EDA99 JZ
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

    static bool squad_tick_trace_enabled() {
        static const bool enabled = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            if (_dupenv_s(&text, &bytes, "BSP_AI_SQUAD_TICK_TRACE") != 0) return false;
            const bool on = text != nullptr && text[0] == '1';
            std::free(text);
            return on;
        }();
        return enabled;
    }

    // Packet cc9_plane_attacker_weight: every 00A0F970 call whose own group is
    // led by a squadron or plane, with each candidate's value. Env-gated
    // (BSP_AI_GROUP_VALUE_TRACE=1), observation only.
    static bool group_value_trace_enabled() {
        static const bool enabled = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            if (_dupenv_s(&text, &bytes, "BSP_AI_GROUP_VALUE_TRACE") != 0) return false;
            const bool on = text != nullptr && text[0] == '1';
            std::free(text);
            return on;
        }();
        return enabled;
    }

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

    // Packet cc9_big_landing_ship: class+808h through the ship AI host, which
    // read it at load; false when the switch is off.
    bool big_landing_ship_0808(std::size_t unit) {
        GameShipAiHost* ship_ai = units.ship_ai();
        const bool big = ship_ai != nullptr && ship_ai->unit_big_landing_ship_0808(unit);
        if (big) ++big_landing_ship_reads;   // counted on both sides
        return kShipAiBigLandingShipBound && big;
    }
    unsigned long long big_landing_ship_reads{0};

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
            // Packet cc9_big_landing_ship: ON reads +808h from the ship AI host.
            if (units.unit_is_kind_of(unit, 0x0E) ||
                (units.unit_is_kind_of(unit, 0x0C) && !big_landing_ship_0808(unit))) {
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
        // 00A2B8F0 __thiscall(group+24h)(emptyGroup): the score-list removal
        // (kAiGroupScoreListReleaseBound). The only reference this process
        // holds between groups is a command target, counted here.
        Group* h = group_at(holder);
        Group* g = group_at(group);
        if (h != nullptr && h->command.target_group == static_cast<void*>(g)) {
            ++summary.target_group_releases;
            const bsp::AiCommandObject birth = initial_command_for(h);
            if (birth.type != bsp::AiCommandType::Idle) {
                ++summary.target_group_releases_non_idle_birth;
            }
            // OFF: the earlier host rule, the command reverts to the class the
            // constructor installed. ON: destroy_group replaces it with IDLE,
            // as 00A10040 does when 00A2D440 notifies (kAiTargetGroupDestroyedIdleBound).
            if (!kAiTargetGroupDestroyedIdleBound) h->command = birth;
        }
        if constexpr (kAiGroupScoreListReleaseBound) {
            done("AiGroups::release_group_reference", 0x00a2b8f0u);
        } else {
            record("AiGroups::release_group_reference", 0x00a2b8f0u);
        }
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
        if (kAiTargetGroupDestroyedIdleBound) {
            // 00A2D440 -> 00696330: each ATTACK command observing g runs 00A10040,
            // which builds IDLE (new(8), vtable 00D229E0, +4h = its own group) and
            // installs it with 00A2BD00, deleting the attack command.
            for (Group* h : registry) {
                if (h == nullptr || h == g) continue;
                if (h->command.target_group != static_cast<void*>(g)) continue;
                bsp::AiCommandObject idle;
                idle.type = bsp::AiCommandType::Idle;
                idle.owner_group = h;
                h->command = idle;
                done("AiCommand::attack_target_destroyed_00a10040", 0x00a10040u);
            }
        }
        registry.erase(std::remove(registry.begin(), registry.end(), g), registry.end());
        for (std::vector<Group*>& list : by_team) {
            list.erase(std::remove(list.begin(), list.end(), g), list.end());
        }
        for (std::vector<Group*>& list : by_party) {
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
            if (bsp::ai_group_member_still_belongs(flags, unit_slot_009ffd20(unit), g->party,
                                                   side, g->team)) {
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
        // OFF: this process files a group under its own team. ON: group+5634h
        // is 009FFD20 of the first member (00A2DFA0).
        g->party = kAiOwnerPlayerSlotBound ? unit_slot_009ffd20(unit)
                                           : party_slot_of_team(g->team);
        if constexpr (kAiPartyGateUnforcedBound) {
            if (g->party == 0) ++party_gate_local_groups; else ++party_gate_other_groups;
        }
        registry.push_back(g);
        // 00A2E086-00A2E0BD: every group appends itself to
        // g_aiGroupsByTeam[group+5638h] unconditionally.
        by_team[static_cast<std::size_t>(g->team)].push_back(g);
        if (g->party >= 0 && g->party < bsp::kAiGroupPartySlotCount) {
            by_party[static_cast<std::size_t>(g->party)].push_back(g);
        }
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

    // Packet cc9_ai_party_gate. A slot record's +28h (009FFD60): PlayerN's
    // Party, N = slot + 1. Unpublished or unauthored, the slot itself stands
    // in (the OFF identity).
    int slot_team(int slot) const {
        if (slot < 0 || slot >= 8) return slot;
        const int party = g_scene_slot_parties[static_cast<std::size_t>(slot)];
        return (g_scene_slot_parties_published && party >= 0) ? party : slot;
    }
    // 009FFD20 BSP_Unit_LossCountingSlot (009FFD20-009FFD5C, live decompile):
    // +180h == 8 answers -1, +180h < 8 answers itself, otherwise the single-
    // player team rule below. kAiOwnerPlayerSlotBound OFF takes +180h as 9 for
    // every unit (section 60's labelled substitution).
    int unit_slot_009ffd20(std::size_t unit) const {
        const int team_slot = party_slot_of_team(units.unit_side_0054(proxy(unit)));
        if constexpr (!kAiOwnerPlayerSlotBound) return team_slot;
        const int owner = unit_owner_player_0180(unit);
        if (owner == 8) return -1;
        if (owner >= 0 && owner < 8) return owner;
        return team_slot;
    }
    // unit+180h as 0077F1F9 stores it from the scene's `OwnerPlayer`, else 9.
    int unit_owner_player_0180(std::size_t unit) const {
        for (const std::size_t key : {unit, proxy(unit)}) {
            const auto by_index = g_unit_owner_players.find(key);
            if (by_index != g_unit_owner_players.end()) return by_index->second;
        }
        const GameUnitRow* row = units.unit_row(unit);
        if (row == nullptr) row = units.unit_row(proxy(unit));
        if (row == nullptr) return 9;
        if (g_scene_owner_player_conflicts.count(row->name) != 0) return 9;
        const auto it = g_scene_owner_players.find(row->name);
        return it != g_scene_owner_players.end() ? it->second : 9;
    }
    // 009FFD20 for a unit whose +180h is 9, in single player: 0 on the local
    // player's team, else 4 (009FFD4B SUB / NEG / SBB / AND EAX,4).
    int party_slot_of_team(int team) const {
        if constexpr (kAiPartyGateUnforcedBound) {
            return team == slot_team(0) ? 0 : 4;
        } else {
            return team;
        }
    }
    bool party_admitted(int slot) const {
        if constexpr (kAiPartyGateUnforcedBound) {
            return bsp::ai_party_ai_enabled_009ffe50(false, false,
                kSinglePlayerEffectiveGameMode, slot);
        } else {
            return bsp::ai_party_ai_enabled(kCampaignGameMode, slot);
        }
    }

    // The group constructor's install at group+564Ch. 009FFE50 admits party
    // slots 0 and 4 in campaign mode, which ai_party_think_mode already knows.
    bsp::AiCommandObject initial_command_for(Group* g) const {
        bsp::AiCommandObject cmd;
        const int slot = g->party;
        cmd.type = bsp::ai_command_initial_type(slot, party_admitted(slot));
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
        // Packet cc9_ai_tick_strafe: every MOVETOATTACK / CLOSEATTACK tick of a
        // group whose first member is a plane squadron, with 00A12A90's two
        // inputs. Env-gated (BSP_AI_SQUAD_TICK_TRACE=1), observation only.
        if (squad_tick_trace_enabled() &&
            (cmd->type == bsp::AiCommandType::MoveToAttack ||
             cmd->type == bsp::AiCommandType::CloseAttack)) {
            Group* g = group_at(cmd->owner_group);
            if (g != nullptr && !g->members.empty() && is_squadron(g->members.front())) {
                Group* tg = group_at(cmd->target_group);
                float own[3] = {0.0f, 0.0f, 0.0f};
                float tgt[3] = {0.0f, 0.0f, 0.0f};
                tick_leader_point(g, own);
                if (cmd->target_group != nullptr) tick_leader_point(cmd->target_group, tgt);
                log.notef("  ai squadtick t=%.2f cmd=%s leader=%s members=%zu target=%s "
                    "dist=%.1f collect=%.1f groupable=%d own=(%.0f %.0f %.0f) "
                    "tgt=(%.0f %.0f %.0f)",
                    static_cast<double>(clock_seconds),
                    cmd->type == bsp::AiCommandType::MoveToAttack ? "MOVETOATTACK" : "CLOSEATTACK",
                    unit_name(g->members.front()).c_str(), g->members.size(),
                    (tg == nullptr || tg->members.empty())
                        ? "" : unit_name(tg->members.front()).c_str(),
                    static_cast<double>(tick_horizontal_distance(own, tgt)),
                    static_cast<double>(tuning.at(bsp::kAiTuningCloseAttackCollectDist)),
                    bsp::ai_entity_is_groupable_combatant_009fe080(
                        combatant_facts(g->members.front())) ? 1 : 0,
                    static_cast<double>(own[0]), static_cast<double>(own[1]),
                    static_cast<double>(own[2]), static_cast<double>(tgt[0]),
                    static_cast<double>(tgt[1]), static_cast<double>(tgt[2]));
            }
        }
        if constexpr (kAiMovetoDiagEvery > 0) {
            if (cmd->type == bsp::AiCommandType::MoveToAttack) {
                ++diag_moveto_ticks;
                if ((diag_moveto_ticks % kAiMovetoDiagEvery) == 0) {
                    Group* g = group_at(cmd->owner_group);
                    Group* tg = group_at(cmd->target_group);
                    float own[3] = {0.0f, 0.0f, 0.0f};
                    float tgt[3] = {0.0f, 0.0f, 0.0f};
                    if (g != nullptr) tick_leader_point(g, own);
                    if (tg != nullptr) tick_leader_point(tg, tgt);
                    log.notef("  ai diag2 movetoattack leader=%s target=%s dist=%.1f "
                        "tgt=(%.0f %.0f %.0f)",
                        (g == nullptr || g->members.empty())
                            ? "" : unit_name(g->members.front()).c_str(),
                        (tg == nullptr || tg->members.empty())
                            ? "" : unit_name(proxy(tg->members.front())).c_str(),
                        static_cast<double>(tick_horizontal_distance(own, tgt)),
                        static_cast<double>(tgt[0]), static_cast<double>(tgt[1]),
                        static_cast<double>(tgt[2]));
                }
            }
        }
        bsp::AiCommandTickResult tick = bsp::ai_command_tick_vt000c(*this, *cmd);
        if (tick.route_ran) {
            // Packet cc9_director_moveonpath_route: the census of 00A14DD0.
            Group* rg = group_at(cmd->owner_group);
            const std::size_t leader = (rg != nullptr && !rg->members.empty())
                ? proxy(rg->members.front()) : units.count();
            if (tick.route.moveto) ++summary.cautious_route_movetos;
            if (tick.route.waited) ++summary.cautious_route_waits;
            if (tick.route.built) {
                ++summary.cautious_route_builds;
                if (leader < units.count()) {
                    log_cautious_route(leader, "build", tick.route.points);
                }
            }
        }
        if (tick.wedge_ran) {
            // Packet cc9_cautious_wedge: the census of 00A11690.
            ++cautious_wedge_calls;
            if (tick.wedge.ran) {
                ++cautious_wedge_runs;
                cautious_wedge_placed += static_cast<unsigned long long>(tick.wedge.placed);
                cautious_wedge_no_record +=
                    static_cast<unsigned long long>(tick.wedge.no_record);
                if (tick.wedge.frame_u[0] == 0.0f && tick.wedge.frame_u[1] == 0.0f) {
                    ++cautious_wedge_zero_frame;
                }
                log_cautious_wedge(cmd->owner_group, tick.wedge);
                done("AiCommand::formation_wedge_00a11690", 0x00a11690u);
            }
        }
        if constexpr (kSellingTickBound) {
            if (cmd->type == bsp::AiCommandType::Selling) {
                const bsp::AiCommandTickResult sell = selling_tick_00a11ff0(*cmd);
                tick.orders_issued += sell.orders_issued;
                tick.formation_requests += sell.formation_requests;
                tick.followers_walked += sell.followers_walked;
            }
        }
        if (cmd->type == bsp::AiCommandType::PatrolTo) {
            // 00A15671-00A156C8: 00A13B60(1.0, &+8h, 0, 0) when near, 00A11B80(0)
            // (not bound, as for CLOSEATTACK), then the leader moveto when far
            // and the pass found no target.
            bool found = false;
            if (tick.patrol_near) {
                const bsp::AiCloseAttackTickResult close_tick =
                    bsp::ai_close_attack_tick_00a13b60(*this, *cmd, 1.0f, cmd->target_position);
                found = close_tick.candidate_found;
                summary.close_members_served += close_tick.members_served;
                summary.close_attack_move_orders += close_tick.attack_move_orders;
                summary.close_set_target_orders += close_tick.set_target_orders;
                summary.close_fallback_movetos += close_tick.fallback_movetos;
                summary.close_candidates_scored += close_tick.candidates_scored;
                ++patrol_close_passes;
            }
            transport_moves_00a11b80(cmd->owner_group, nullptr);          // 00A15690
            const bsp::AiCommandTickResult tail =
                bsp::ai_command_patrol_to_tail_00a15695(*this, *cmd, tick.patrol_far, found);
            tick.orders_issued += tail.orders_issued;
            tick.leader_ordered = tick.leader_ordered || tail.leader_ordered;
            ++patrol_ticks;
            done("AiCommand::patrol_to_tick", 0x00a15570u);
        }
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
            // 00A154E8 (CLOSEATTACK, cmd+1Ch) / 00A15564 (DEFENDPOSITION, 0).
            transport_moves_00a11b80(cmd->owner_group, close ? cmd->target_group : nullptr);
            if constexpr (kShipDirectorEnablesBound) {
                if (close) {
                    // 00A154F7 JMP 00A11AF0 (packet cc9_ship_torpedo_mask_read):
                    // every node of the OWN group's +563Ch list whose entity
                    // answers IsKindOf(6) gets director->0071E0D0(1), every
                    // tick; 00A11B80 before it is not bound.
                    if (Group* own = group_at(cmd->owner_group)) {
                        for (const std::size_t m : own->members) {
                            if (m >= units.count() || !units.unit_is_kind_of(m, 0x06)) continue;
                            const GameUnitRow* row = units.unit_row(m);
                            if (row == nullptr) continue;
                            ++scene_director_torpedo_writes().close_attack_sends;
                            if (scene_director_enables_set_torpedo(row->name, true)) {
                                log.notef("  close attack torpedo enable: %s director+222h=1 "
                                    "(00A11AF0 -> 0071E0D0)", row->name.c_str());
                            }
                        }
                    }
                    done("AiCommand::close_attack_torpedo_enable", 0x00a11af0u);
                }
            }
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
    bool close_member_lands_troops(void* member) override {
        // 00A14435 MOV ECX,[ESI+538h]; 00A1443D CALL [EDX+2Ch]; a true answer skips the
        // member (00A14444 JNE 00A14D4D). The troop-landing trait.
        const bool trait = units.unit_class_lands_troops_vtable_2c(unit_index_of(member));
        if (trait) {
            ++summary.close_troop_landers;
            diag_troop_lander("close", unit_index_of(member));
        }
        if (!bsp::game::kTroopLandingTraitBound) {
            record("AiCommand::close_member_lands_troops", 0x00a1443du);
            return false;
        }
        done("AiCommand::close_member_lands_troops", 0x00a1443du);
        return trait;
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
            // 00A0F83C reads record+10h, which 00A04560 always leaves 0; only
            // the plane arm reads it (packet cc9_plane_attacker_weight).
            key.attacker_class = kAiPlaneAttackerWeightBound ? 0
                                 : units.unit_class_id(attacker_unit);
            key.target = handle(target);
            // Target record +1Ch, 00A04568's side >= 2. Only the gates read it,
            // so it is published with them (packet cc9_plane_attacker_weight).
            key.target_is_neutral =
                (kAiWeightBarrelGatesBound && units.unit_side_0054(target) >= 2) ? 1 : 0;
            const bsp::AiModeTuning record = mode_tuning_record();
            AiWeightModelBinding model(facts, record, tuning,
                [this](std::size_t unit) { return accuracy_target_group(unit); },
                [this](std::size_t unit, int code) { return units.unit_is_kind_of(unit, code); });
            in.base_weight = bsp::ai_target_weight_00a08460(model, key);
            ai_damage_terms_census().close_weight_sum += in.base_weight;
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
        if constexpr (kAiSquadronSetTargetIntakeBound) {
            // Packet cc9_ai_squadron_settarget_intake (docs/SQUADRON_LAND_TASK.md
            // 5ck): the order goes to the squadron, whose intake 007F1940 issues
            // 007EEC50's class in its place (007F1AD6-007F1B24) or nothing.
            const std::vector<std::size_t>* members = squadron_member_units(unit);
            if (is_squadron(unit) && members != nullptr && !target_name.empty()) {
                if (order_is_repeat(unit, token, target_name, nullptr)) return true;
                ++summary.squadron_commands;
                script_orders_squadron_intake_007f1940(members->front(), *members,
                                                       unit_index_of(target));
                ++summary.commands_issued;
                if (current_party >= 0) ++party_row(current_party).commands_issued;
                if (summary.first_command_seconds < 0.0f)
                    summary.first_command_seconds = clock_seconds;
                observe_target_choice(member, target);
                done("AiCommand::close_issue_order", 0x0077d600u);
                return true;
            }
        }
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
        // 00A14D48 CALL 00A02020 (SHIP_AI 112). The census reads the gates
        // without their counters, so OFF moves no other summary line.
        ++summary.close_bridge_calls;
        float position[3] = {0.0f, 0.0f, 0.0f};
        const bool have_position = tick_member_position(member, position);
        const bool squadron = tick_member_is_plane_squadron(member);
        const bool excluded = squadron
            && bsp::ai_squadron_excluded_007eda90(combatant_facts(unit_index_of(member)));
        const bool ship = tick_member_is_ship_base(member);
        if (!bsp::ai_order_bridge_accepts_00a02020(squadron, excluded, ship)) {
            ++summary.close_bridge_class_refused;
        } else if (have_position) {
            const float dx = position[0] - point[0];
            const float dz = position[2] - point[2];
            if (dx * dx + dz * dz < bsp::kAiOrderIssueDistanceSquared) {
                ++summary.close_bridge_near_refused;
            } else if (ship) {
                ++summary.close_bridge_ships;
            }
        }
        if constexpr (kCloseFallbackOrderBridgeBound) {
            float issued_at[3] = {point[0], point[1], point[2]};
            const bool issued = issue_moveto_bridge_00a02020(member, point, issued_at);
            if (issued && ship && (issued_at[0] != point[0] || issued_at[2] != point[2])) {
                ++summary.close_bridge_zone_moved;
            }
            return issued;
        } else {
            return tick_issue_moveto(member, point);
        }
    }

    // 00A02020 whole for one member (ai_command_tick.cpp's order_leader sequence):
    // the class gate, the 80 m gate, a ship's 00417B10 point, then the tail.
    bool issue_moveto_bridge_00a02020(void* member, const float point[3],
                                      float* issued_at = nullptr) {
        float position[3] = {0.0f, 0.0f, 0.0f};
        if (!tick_member_position(member, position)) return false;
        float avoid[2] = {point[0], point[2]};
        const bool squadron = tick_member_is_plane_squadron(member);
        const bool excluded = tick_squadron_excluded_007eda90(member);
        const bool ship = tick_member_is_ship_base(member);
        if (bsp::ai_order_bridge_takes_zone_point_00a02020(squadron, excluded, ship, position,
                                                            point)) {
            tick_avoid_zone_point(member, point, avoid);
        }
        const bsp::AiOrderBridgeResult bridge = bsp::ai_order_bridge_00a02020(
            squadron, excluded, ship, position, point, avoid);
        if (!bridge.issued) return false;
        if (issued_at != nullptr) {
            for (int k = 0; k < 3; ++k) issued_at[k] = bridge.position[k];
        }
        return tick_issue_moveto(member, bridge.position);
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
        // (009FFEC6..009FFF16) was read whole in docs/SHIP_AI_OPEN_ITEMS.md
        // section 36 and is bound as rtb_exclusion_arm below (section 40,
        // kAiSquadronRtbExclusionBound). With the switch OFF the answer is
        // 007EDA90's, the stand-in used before the arm was read.
        record("AiCommand::squadron_excluded_009ffeb0", 0x009ffeb0u);
        if (lua_device_reload_enabled_00e17bf2()) return false;   // 009FFEB0, [00E17BF2] set
        rtb_exclusion_census(member);
        const bool stand_in = tick_squadron_excluded_007eda90(member);
        if (stand_in) ++rtb_exclusion_stand_in_true;
        if (!kAiSquadronRtbExclusionBound) return stand_in;
        return rtb_exclusion_arm(member);
    }
    // Packet cc9_squadron_rtb_exclusion_bind: 009FFEC6..009FFF16 with the
    // units host's 007EDAD0 and 007F16D0 + 0077D600 entries.
    bool rtb_exclusion_arm(void* member) {
        if (member == nullptr) return false;
        const std::size_t index = unit_index_of(member);
        const Squadron* s = squadron_of(index);
        if (s == nullptr) return false;
        const std::size_t head = lead_member(*s);                  // [sq+3D0h]
        if (head == bsp::kPlaneSquadronNoUnit) return false;
        const int cls = units.unit_class_id(head);                  // 009FFECD +0C4h
        // The units host keys squadrons by a member plane; this host's squadron
        // index lies past units.count(), so the head plane names the record.
        if (units.squadron_ammo_type_007edad0(head) != 0) return false;   // 009FFED3
        if (cls != 0x10 && cls != 0x11 && cls != 0x12) return false;      // 009FFEDC..
        done("AiCommand::squadron_excluded_009ffeb0", 0x009ffeb0u);
        const std::size_t placed =
            units.issue_return_to_base_007f16d0(head, "ai squadron rtb exclusion 009FFF09");
        ++rtb_exclusion_issues;
        if (placed == 0u) return false;   // 009FFEF9, null descriptor: answers 0
        ++rtb_exclusion_excluded;
        if (rtb_exclusion_excluded <= 8u) {
            log.notef("  ai rtb exclusion: squadron %s head class %02Xh spent, returntobase "
                "placed on %zu planes (009FFF09, packet cc9_squadron_rtb_exclusion_bind)",
                unit_name(index).c_str(), cls, placed);
        }
        return true;
    }
    // Packet cc9_squadron_rtb_exclusion census, no behaviour: 009FFEC6..
    // 009FFF16 read. With [00E17BF2] clear the arm takes the head plane's class
    // id [[sq+3D0h]+C4h] (009FFECD); when 007EDAD0 (the squadron's first
    // ordnance kind over its +3D0h planes) answers 0 and the id is 10h, 11h or
    // 12h, 007F16D0 resolves the squadron's returntobase into a command record
    // and a non-null descriptor is issued through 0077D600(desc, &record, 1)
    // (009FFF09); that returns true. Counted here: bomber-class calls, and the
    // calls where the ordnance reader answers 0 (the arm would resolve).
    void rtb_exclusion_census(void* member) {
        if (member == nullptr) return;
        const std::size_t index = unit_index_of(member);
        const Squadron* s = squadron_of(index);
        if (s == nullptr) return;
        ++rtb_exclusion_squadron_calls;
        const std::size_t head = lead_member(*s);
        if (head == bsp::kPlaneSquadronNoUnit) return;
        const int cls = units.unit_class_id(head);
        if (cls != 0x10 && cls != 0x11 && cls != 0x12) return;
        ++rtb_exclusion_bomber_calls;
        // 007EDAD0 over the planes still listed: 2Bh, 2Ch, 33h, 31h, 2Fh, then
        // 007B9320's general bomb; any one answers non-zero. LABELLED: the
        // host's mask is the union over a plane's guns, cleared by drops.
        for (const std::size_t plane : s->member_units) {
            if constexpr (bsp::kPlaneSquadronLeaveOnDeathBound) {
                if (s->registry_backed &&
                    bsp::plane_squadron_registry().find_by_member_unit(plane) == nullptr) {
                    continue;
                }
            }
            const bsp::OrdnanceKindSet set{units.unit_ordnance(plane)};
            if (set.contains(0x2b) || set.contains(0x2c) || set.contains(0x33) ||
                set.contains(0x31) || set.contains(0x2f) ||
                bsp::ordnance_has_general_bomb_2ah(set)) {
                return;
            }
        }
        ++rtb_exclusion_spent_calls;
        if (rtb_exclusion_spent_calls <= 8u) {
            log.notef("  ai rtb exclusion census: squadron %s head class %02Xh has no "
                "ordnance (009FFED3 007EDAD0 = 0), 007F16D0 would resolve (packet "
                "cc9_squadron_rtb_exclusion)", unit_name(index).c_str(), cls);
        }
    }
    bool tick_member_is_groupable_combatant(void* member) override {
        if (member == nullptr) return false;
        return bsp::ai_entity_is_groupable_combatant_009fe080(
            combatant_facts(unit_index_of(member)));
    }
    // Packet cc9_ai_tick_plane_retask (kAiTickMovetoRetasksPlaneBound).
    struct PlaneRetask {
        bool valid{false};
        float point[3]{0.0f, 0.0f, 0.0f};
    };
    std::map<std::size_t, PlaneRetask> plane_retask;
    unsigned long long retask_installed{0};
    unsigned long long retask_kept{0};
    unsigned long long retask_replaced_other{0};
    unsigned long long retask_not_current{0};
    unsigned long long retask_refused{0};
    void retask_plane_on_moveto(std::size_t plane, std::uint32_t before,
                                const float point[3]) {
        if (units.director_current_command_0071be40(plane) != bsp::kCommandMoveTo) {
            ++retask_not_current;   // a deferred delivery: no current moveto yet
            return;
        }
        PlaneRetask& r = plane_retask[plane];
        if (before == bsp::kCommandMoveTo && r.valid) {
            // 009C31B0: the kind-7 head stays when the new point is within
            // 100 m planar of the one it was built for.
            const float dx = point[0] - r.point[0];
            const float dz = point[2] - r.point[2];
            if (dx * dx + dz * dz < 10000.0f) {
                ++retask_kept;
                return;
            }
        }
        AiTickMovetoBotHost bot(units);
        const std::uint32_t task = bsp::bot_install_command_task_0099a170(
            static_cast<std::uint32_t>(plane + 1u), bot);
        if (task == 0u || bot.built_command() != bsp::kCommandMoveTo) {
            ++retask_refused;
            return;
        }
        if (before != bsp::kCommandMoveTo) ++retask_replaced_other;
        units.store_unit_attack_command_class(plane, bsp::kCommandMoveTo);
        units.store_unit_moveto_range(plane, 0.0f);
        units.store_unit_moveto_target(plane, ~static_cast<std::size_t>(0));
        units.store_unit_moveto_point(plane, point);
        r.valid = true;
        r.point[0] = point[0];
        r.point[1] = point[1];
        r.point[2] = point[2];
        ++retask_installed;
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
                const std::uint32_t before = kAiTickMovetoRetasksPlaneBound
                    ? units.director_current_command_0071be40(plane) : 0u;
                if (units.issue_script_command(plane, bsp::kAiSceneCommandMoveTo, target,
                                               bsp::kAiSceneCommandFlags, "ai_command_tick",
                                               unit_name(plane)) != nullptr) {
                    ++placed;
                    ++summary.squadron_member_orders;
                    if constexpr (kAiTickMovetoRetasksPlaneBound) {
                        // The MT_COMMAND waits for the session pump's drain
                        // (cc9_set_command_queue_delay); 0099A4C0 runs in the
                        // plane's bot tick after it, so the install follows the
                        // delivery, as the script orders' installs do.
                        const std::array<float, 3> at{position[0], position[1], position[2]};
                        std::function<void()> install = [this, plane, before, at]() {
                            retask_plane_on_moveto(plane, before, at.data());
                        };
                        if (!commands_after_last_issue_delivery(install)) install();
                    }
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
        // 0077C8D0 BSP_Entity_RequestJoinFormation asks the follower's
        // vtable[16Ch] first (008162B0 for MDestroyer) and ends at 0077C902 with
        // no effect when the answer is false. Packet cc8_ship_follow read that
        // chain: for the `follow` token the answer is decided entirely by
        // 008162BF's call to 00779D50, whose 78 instructions are now
        // transcribed as bsp::entity_may_follow_target_00779d50.
        // docs/SHIP_UNIT_GROUP_FOLLOW.md section 8.
        //
        // The host used to answer a neutral false here, which is why every one
        // of USN01's 306 requests stopped at the first question.
        ++formation_requests_seen;
        if (follower == nullptr || leader == nullptr) {
            ++formation_requests_refused;
            record("AiCommand::request_join_formation", 0x0077c8d0u);
            return false;                                      // 00779D76
        }
        const std::size_t follower_index = unit_index_of(follower);
        const std::size_t leader_index = unit_index_of(leader);
        bsp::EntityFollowFacts facts;
        facts.follower_flag_005d = units.unit_flag_005d(follower_index);
        facts.target_present = true;
        facts.target_flag_005d = units.unit_flag_005d(leader_index);
        facts.target_kind_02 = units.unit_is_kind_of(leader_index, 0x02);
        facts.follower_kind_06 =
            units.unit_is_kind_of(follower_index, bsp::kUnitGunneryKindShipBase);
        facts.follower_kind_08 = units.unit_is_kind_of(follower_index, 0x08);
        facts.target_kind_06 =
            units.unit_is_kind_of(leader_index, bsp::kUnitGunneryKindShipBase);
        facts.target_kind_08 = units.unit_is_kind_of(leader_index, 0x08);
        facts.follower_party_0054 = units.unit_side_0054(follower_index);
        facts.target_party_0054 = units.unit_side_0054(leader_index);
        // 00779820 whole: the same entity, or already sharing a unit group.
        const std::int32_t follower_group =
            units.unit_formation_group_0284(follower_index);
        facts.same_entity_or_group_00779820 =
            (follower == leader)
            || (follower_group >= 0
                && follower_group == units.unit_formation_group_0284(leader_index));
        // 00779DB4's OwnerPlayer arm: +188h has no producer in this process
        // (game_hosts_scene_contents.cpp:1364), so it is skipped rather than
        // guessed. It can only ever admit a follow between two differently
        // owned ships that the image would refuse.
        facts.owner_player_known = false;
        const bool available = bsp::entity_may_follow_target_00779d50(facts, true);
        if (available) {
            ++formation_requests_available;
            // 0077C964 routes a type-76h message whose only payload is the
            // leader's 16-bit id, and 0077FE80's four-instruction arm resolves it
            // and calls 0077F940. In this process the route is local, so the join
            // runs here rather than through the session; the wire hop is the
            // part that is not modelled, not the merge.
            bool posted = false;
            if constexpr (kPlannerJoinLoopbackBound) {
                ++formation_joins_posted;
                posted = commands_post_loopback_callback_0076e520(follower_index,
                    [this, follower_index, leader_index] {
                        ++formation_joins_delivered;
                        if (units.formation_join_0077f940(follower_index, leader_index)) {
                            ++formation_joins_made;
                        }
                    });
            }
            if (!posted && units.formation_join_0077f940(follower_index, leader_index)) {
                ++formation_joins_made;
            }
        } else {
            ++formation_requests_refused;
        }
        if (diag_follow_lines < 12) {
            ++diag_follow_lines;
            log.notef("  ai diag follow %s -> %s available=%d (ship %d/%d, kind2=%d, "
                "party %d/%d, alive %d/%d)",
                unit_name(follower_index).c_str(), unit_name(leader_index).c_str(),
                available ? 1 : 0,
                facts.follower_kind_06 ? 1 : 0, facts.target_kind_06 ? 1 : 0,
                facts.target_kind_02 ? 1 : 0,
                facts.follower_party_0054, facts.target_party_0054,
                facts.follower_flag_005d ? 0 : 1, facts.target_flag_005d ? 0 : 1);
        }
        done("AiCommand::request_join_formation", 0x0077c8d0u);
        return available;
    }
    bool tick_avoid_zone_point(void* member, const float target[3], float out[2]) override {
        // 00A020BE / 00A020F0: 0082ADA0([unit+538h], 0) then 00417B10(group, &out,
        // &{x, z}, 30.0f, 1). The ticks call this only where the image reaches
        // 00A020F0. The runtime is the ship-AI host's (packet
        // cc9_ai_command_avoid_zone_point). OFF, or with no runtime or no group,
        // the ship is sent at the requested point itself.
        ++avoid_zone_asks;
        out[0] = target[0];
        out[1] = target[2];
        if (kAiCommandAvoidZonePointBound) {
            GameShipAiHost* ship_ai = units.ship_ai();
            const std::size_t unit = unit_index_of(member);
            const float in[2] = {target[0], target[2]};
            if (ship_ai != nullptr && unit < units.count() &&
                ship_ai->avoid_zone_offset_point_00a020f0(unit, in,
                    bsp::kAiOrderAvoidZoneMargin, out)) {
                ++avoid_zone_answers;
                if (out[0] != in[0] || out[1] != in[1]) ++avoid_zone_moved;
                done("AiCommand::avoid_zone_offset_point", 0x00417b10u);
                return true;
            }
        }
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
    // ---- packet cc9_director_moveonpath_route -------------------------------
    bsp::AiCommandObject* tick_command_state(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? &g->command : nullptr;
    }
    int tick_group_team(void* group) override {
        Group* g = group_at(group);
        return g != nullptr ? g->team : 0;
    }
    // The leader's director, 00778860. LABELLED: a squadron leader has no
    // director in this host (its planes each have one), so it answers no slot.
    bool route_unit(void* leader, std::size_t& unit) const {
        if (leader == nullptr) return false;
        unit = unit_index_of(leader);
        return unit < units.count();
    }
    bool tick_route_slot_present(void* leader) override {
        std::size_t unit = 0;
        const bool present = route_unit(leader, unit);
        if (!present) ++summary.cautious_route_no_slot;
        return present;
    }
    bool tick_route_attached_0071fc40(void* leader) override {
        std::size_t unit = 0;
        if (!route_unit(leader, unit)) return false;
        return units.commands().user_path_attached_0071fc40(unit);
    }
    int tick_route_remaining_0071d2a0(void* leader) override {
        std::size_t unit = 0;
        if (!route_unit(leader, unit)) return -1;
        return units.commands().user_path_remaining_0071d2a0(unit);
    }
    bool tick_issue_clearorders(void* leader) override {
        std::size_t unit = 0;
        if (!route_unit(leader, unit)) return false;
        // 00A14E8A: 004F1830(1) builds the descriptor (kind 0), then 0077D600.
        bsp::SceneCommandTarget target{};
        ++summary.cautious_route_clears;
        log_cautious_route(unit, "clearorders", 0);
        done("AiCommand::cautious_clearorders", 0x00a14ea7u);
        return units.issue_script_command(unit, bsp::kAiSceneCommandClearOrders, target,
            bsp::kAiSceneCommandFlags, "ai_cautious_route", unit_name(unit)) != nullptr;
    }
    float tick_danger_cost_00a010f0(const float point[3], int team) override {
        // 00A010F0: over [[00E188A8]+19CCh]+58h, the entities whose +54h is
        // `team`, the class weight 009FDF30 times 00419010(PartyPresence_
        // DistanceMin (+190h) -> 1, PartyPresence_DistanceMax (+194h) -> 0, the
        // x/z distance), summed when that factor is above zero. LABELLED: the
        // list is this host's active units in index order.
        // LABELLED SUBSTITUTION: this host's tuning block carries only the
        // reconstructed key subset, so +190h/+194h read zero. The values are this
        // installation's highlvlaiglobals.lua (mtime 2024-07-13), 2000 and 4000 in
        // all seven mode tables (lines 91/92, 279/280, ..., 1130/1131).
        const float near_distance = tuning.at(0x190) != 0.0f ? tuning.at(0x190) : 2000.0f;
        const float far_distance = tuning.at(0x194) != 0.0f ? tuning.at(0x194) : 4000.0f;
        float total = 0.0f;
        for (std::size_t unit = 0; unit < units.count(); ++unit) {
            if (!units.unit_active(unit)) continue;
            if (units.unit_side_0054(unit) != team) continue;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            units.unit_position_00fc(unit, x, y, z);
            const float dx = static_cast<float>(static_cast<double>(point[0]) - x);
            const float dz = static_cast<float>(static_cast<double>(point[2]) - z);
            const double d2 = static_cast<double>(dz) * dz + static_cast<double>(dx) * dx + 0.0;
            const float distance = d2 <= 1.0e-10 ? 0.0f : static_cast<float>(std::sqrt(d2));
            const float factor = bsp::clamped_interpolate_00419010(near_distance, 1.0f,
                far_distance, 0.0f, distance);
            if (0.0f < factor) {
                total = static_cast<float>(static_cast<double>(unit_class_weight(unit)) * factor
                                           + total);
            }
        }
        done("AiCommand::danger_cost_00a010f0", 0x00a010f0u);
        return total;
    }
    void tick_route_send_point_0071d340(void* leader, const float point[3]) override {
        std::size_t unit = 0;
        if (!route_unit(leader, unit)) return;
        float min_x = 0.0f, max_x = 0.0f, min_z = 0.0f, max_z = 0.0f;
        bool outside = false;
        if (units.world_bounds_box_00e188a8(min_x, max_x, min_z, max_z)) {
            outside = point[0] < min_x || point[0] > max_x || point[2] < min_z
                || point[2] > max_z;
        }
        ++summary.cautious_route_points;
        units.commands().post_user_path_point_0071d340(unit, point, outside);
    }
    void log_cautious_route(std::size_t unit, const char* what, int points) {
        if (cautious_route_logs >= 60) return;
        ++cautious_route_logs;
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units.unit_position_00fc(unit, x, y, z);
        log.notef("  ai cautious route: t=%.2f leader=%s %s points=%d at=(%.0f, %.0f) "
            "(00A14DD0, packet cc9_director_moveonpath_route)", clock_seconds,
            unit_name(unit).c_str(), what, points, static_cast<double>(x),
            static_cast<double>(z));
    }
    int cautious_route_logs{0};

    // ---- packet cc9_cautious_wedge (kCautiousWedgeBound) --------------------
    bool tick_member_forward_row(void* member, float out[3]) override {
        out[0] = out[1] = out[2] = 0.0f;
        if (member == nullptr) return false;
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        return units.unit_pose(unit_index_of(member), right, up, out, translation);
    }
    bool tick_member_has_formation_0284(void* member) override {
        if (member == nullptr) return false;
        return units.unit_formation_group_0284(unit_index_of(member)) >= 0;
    }
    int tick_formation_shape0_0070efd0(void* leader) override {
        // 0070EFD0(0) (body 0070EFD0-0070F08C): every record whose +0h is set
        // gets column 0 rewritten, the formation leader's to (0, 0) (0070F03A)
        // and every other one's to table 00E08F18 + slot*8 read as floats. At
        // run time those words are the registered command objects' vtable and
        // ordinal (bsp/entity_orders.hpp), so the products with
        // FormationShipDist are near 1e-38 m. LABELLED: written as 0.0f.
        if (leader == nullptr) return 0;
        const std::size_t unit = unit_index_of(leader);
        const std::int32_t group = units.unit_formation_group_0284(unit);
        if (group < 0) return 0;
        int rewritten = 0;
        const std::int32_t count = units.formation_member_count(group);
        for (std::int32_t slot = 0; slot < count; ++slot) {
            const std::size_t member = units.formation_member_unit(group, slot);
            if (member == static_cast<std::size_t>(-1)) continue;
            if (units.set_formation_member_offset_0070d080(unit, member, 0, 0.0f, 0.0f)) {
                ++rewritten;
            }
        }
        done("AiCommand::formation_shape0_0070efd0", 0x0070efd0u);
        return rewritten;
    }
    bool tick_formation_set_column0_0070d080(void* leader, void* member, float lateral,
                                             float axial) override {
        if (leader == nullptr || member == nullptr) return false;
        return units.set_formation_member_offset_0070d080(unit_index_of(leader),
            unit_index_of(member), 0, lateral, axial);
    }
    void log_cautious_wedge(void* group, const bsp::AiCautiousWedgeResult& w) {
        if (cautious_wedge_logs >= 60) return;
        ++cautious_wedge_logs;
        Group* g = group_at(group);
        const std::string leader = (g != nullptr && !g->members.empty())
            ? unit_name(g->members.front()) : std::string();
        log.notef("  ai cautious wedge: t=%.2f leader=%s samples=%d threat=(%.1f %.1f %.1f) "
            "cost=%.1f u=(%.4f %.4f) v=(%.4f %.4f) dist=%.0f shape0=%d placed=%d not_ship=%d "
            "no_record=%d first=(%.1f %.1f) last=(%.1f %.1f) (00A11690, packet cc9_cautious_wedge)",
            clock_seconds, leader.c_str(), w.samples, static_cast<double>(w.threat[0]),
            static_cast<double>(w.threat[1]), static_cast<double>(w.threat[2]),
            static_cast<double>(w.threat_cost), static_cast<double>(w.frame_u[0]),
            static_cast<double>(w.frame_u[1]), static_cast<double>(w.frame_v[0]),
            static_cast<double>(w.frame_v[1]), static_cast<double>(w.unit_dist),
            w.shape0_records, w.placed, w.not_ship, w.no_record,
            static_cast<double>(w.first_offset[0]), static_cast<double>(w.first_offset[1]),
            static_cast<double>(w.last_offset[0]), static_cast<double>(w.last_offset[1]));
    }
    int cautious_wedge_logs{0};
    unsigned long long cautious_wedge_calls{0};
    unsigned long long cautious_wedge_runs{0};
    unsigned long long cautious_wedge_placed{0};
    unsigned long long cautious_wedge_no_record{0};
    unsigned long long cautious_wedge_zero_frame{0};

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

    int game_mode() override {
        return kAiPartyGateUnforcedBound ? kSinglePlayerEffectiveGameMode : kCampaignGameMode;
    }
    bool party_ai_enabled(int party_slot) override { return party_admitted(party_slot); }

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
    long long diag_moveto_ticks{0};
    // Packet cc8_ship_follow: what 0077C8D0's first question now answers.
    int diag_follow_lines{0};
    unsigned long long formation_requests_seen{0};
    unsigned long long formation_requests_available{0};
    unsigned long long formation_requests_refused{0};
    unsigned long long formation_joins_made{0};
    unsigned long long formation_joins_posted{0};     // cc9_planner_join_loopback
    unsigned long long formation_joins_delivered{0};
    // Packet cc9_group_transport_moves: 00A11B80's census.
    struct TransportMoveCounts {
        unsigned long long calls{0};
        unsigned long long landers{0};      // vt+5Ch(6) and 00827FB0
        unsigned long long ready{0};        // 008128E0 true: 94h routed
        unsigned long long anchors{0};      // anchor searches that found one
        unsigned long long no_anchor{0};    // not ready, no anchor
        unsigned long long inside{0};       // d <= 0.75 x CaptureRange
        unsigned long long movetos{0};      // 00A02020 issued (bound only)
        unsigned long long landing_ships{0};   // 94h for a kind-0Ch member
        unsigned long long begun{0};           // 0074A4C0 answered 1
        unsigned long long refused{0};         // 0074A4C0 answered <= 0
        unsigned long long transports{0};      // 94h for 008206F0
        unsigned long long launches{0};        // 008206F0 answered > 0
        unsigned long long crafts{0};          // the crafts those launched
        unsigned long long launch_refused{0};  // 008206F0 answered <= 0
        int diag_lines{0};
    } transport_moves;

    static bool lander_diag_enabled() {
        static const bool enabled = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            if (_dupenv_s(&text, &bytes, "BSP_LANDER_DIAG") != 0) return false;
            const bool on = text != nullptr && text[0] != '\0' && text[0] != '0';
            std::free(text);
            return on;
        }();
        return enabled;
    }

    // 008128E0's site half: 006F2C30(&unit+FCh, unit+54h, 2) walks list 28 for
    // the nearest entity of another party within its +7C4h LandingRange (3-D
    // squared, 006F2D1C..006F2D34), then 006F2A50(site, 0) asks for a pad of
    // +794h with a null occupant (006AC220). The +1124h test (008128FC, above
    // 0.0 is not ready) reads the launch's cooldown when kLandingCraftLaunchBound
    // is on, else 0.0: its writers are 008206F0 and its 95h only.
    bool transport_ready_008128e0(std::size_t unit) {
        if constexpr (kLandingCraftLaunchBound) {
            GameShipAiHost* ship_ai = units.ship_ai();
            if (ship_ai != nullptr && ship_ai->transport_cooldown_1124(unit) > 0.0f) return false;
        }
        float ux = 0.0f, uy = 0.0f, uz = 0.0f;
        units.unit_position_00fc(unit, ux, uy, uz);
        const int side = units.unit_side_0054(unit);
        std::size_t site = static_cast<std::size_t>(-1);
        float best = 0.0f;
        for (std::size_t i = 0; i < units.world_list_size(28); ++i) {
            const std::size_t e = units.world_list_entry(28, i);
            if (e >= units.count() || units.unit_side_0054(e) == side) continue;
            float ex = 0.0f, ey = 0.0f, ez = 0.0f;
            units.unit_position_00fc(e, ex, ey, ez);
            const float dx = ex - ux, dy = ey - uy, dz = ez - uz;
            const float d2 = dx * dx + dy * dy + dz * dz;
            const float range = units.command_building_landing_range_07c4(e);
            if (d2 > range * range) continue;
            if (site == static_cast<std::size_t>(-1) || d2 < best) {
                site = e;
                best = d2;
            }
        }
        if (site == static_cast<std::size_t>(-1)) return false;
        const bsp::BuildingPadModel& pads = bsp::building_pad_model();
        for (const int pad : pads.pads_of(static_cast<int>(site))) {
            const bsp::BuildingPadModel::Pad* p = pads.pad(pad);
            if (p != nullptr && p->occupant < 0) return true;
        }
        return false;
    }

    // 00A11B80, body 00A11B80-00A11F63, __thiscall(command)(AiGroup* other).
    void transport_moves_00a11b80(void* own_group, void* other_group) {
        Group* g = group_at(own_group);
        if (g == nullptr) return;
        TransportMoveCounts& c = transport_moves;
        ++c.calls;
        bool searched = false;                        // [esp+13h]
        std::size_t anchor = static_cast<std::size_t>(-1);  // [esp+14h]
        const std::vector<std::size_t> members = g->members;
        for (const std::size_t m : members) {
            if (m >= units.count()) continue;
            if (!units.unit_is_kind_of(m, 0x06)) continue;                // 00A11C0E
            if (!units.unit_class_lands_troops_vtable_2c(m)) continue;    // 00A11C27
            ++c.landers;
            if (transport_ready_008128e0(m)) {                            // 00A11C3E
                // 00A11C44..00A11C82: 94h through 0077C2A0(msg, 2, 0).
                ++c.ready;
                if (units.unit_is_kind_of(m, 0x0c)) {
                    ++c.landing_ships;
                    if constexpr (kLandingShipStartLandingBound) {
                        // 00821F61 -> vt+238h = 0074A4C0, at the call (LABELLED).
                        GameShipAiHost* ship_ai = units.ship_ai();
                        const int answer = ship_ai != nullptr
                            ? ship_ai->landing_ship_request_landing_0074a4c0(m) : 0;
                        if (answer == 1) ++c.begun; else ++c.refused;
                    }
                } else {
                    ++c.transports;                 // vt+238h = 008206F0
                    if constexpr (kLandingCraftLaunchBound) {
                        GameShipAiHost* ship_ai = units.ship_ai();
                        const int answer = ship_ai != nullptr
                            ? ship_ai->transport_launch_craft_008206f0(m) : 0;
                        if (answer > 0) {
                            ++c.launches;
                            c.crafts += static_cast<unsigned long long>(answer);
                        } else {
                            ++c.launch_refused;
                        }
                    }
                }
                if (lander_diag_enabled() && c.diag_lines < 60) {
                    ++c.diag_lines;
                    log.notef("transport move diag: t=%.2f leader=%s member=%s ready=1 "
                        "94h routed (launch bound=%d)", clock_seconds,
                        unit_name(g->members.front()).c_str(), unit_name(m).c_str(),
                        kLandingCraftLaunchBound ? 1 : 0);
                }
                continue;
            }
            if (!searched) {
                searched = true;
                if (Group* other = group_at(other_group)) {
                    // 00A11CC0..00A11CFA: the first member answering vt+5Ch(1Ch).
                    for (const std::size_t o : other->members) {
                        if (o < units.count() && units.unit_is_kind_of(o, 0x1c)) {
                            anchor = o;
                            break;
                        }
                    }
                } else {
                    // 00A11D9C..00A11E87: list 28, another team than group+5638h,
                    // XZ squared distance to the first member strictly below the
                    // best, seeded with (tuning+1F4h)^2.
                    const float reach = tuning.at(bsp::kAiTuningCloseAttackCollectDist);
                    float best = reach * reach;
                    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
                    if (!g->members.empty()) {
                        units.unit_position_00fc(g->members.front(), rx, ry, rz);
                    }
                    for (std::size_t i = 0; i < units.world_list_size(28); ++i) {
                        const std::size_t e = units.world_list_entry(28, i);
                        if (e >= units.count() || units.unit_side_0054(e) == g->team) continue;
                        float ex = 0.0f, ey = 0.0f, ez = 0.0f;
                        units.unit_position_00fc(e, ex, ey, ez);
                        const float dx = ex - rx, dz = ez - rz;
                        const float d2 = dx * dx + dz * dz;
                        if (best > d2) {
                            best = d2;
                            anchor = e;
                        }
                    }
                }
                if (anchor != static_cast<std::size_t>(-1)) ++c.anchors;
            }
            if (anchor == static_cast<std::size_t>(-1)) {                 // 00A11D06
                ++c.no_anchor;
                continue;
            }
            float mx = 0.0f, my = 0.0f, mz = 0.0f, ax = 0.0f, ay = 0.0f, az = 0.0f;
            units.unit_position_00fc(m, mx, my, mz);
            units.unit_position_00fc(anchor, ax, ay, az);
            const float dx = mx - ax, dz = mz - az;                       // 00A11D2E
            const float d2 = dx * dx + dz * dz;
            // 00A11D6E: d = 0 when d^2 <= the double 1.0e-10 at 00CE3820.
            const float d = static_cast<double>(d2) > 1.0e-10
                ? static_cast<float>(std::sqrt(static_cast<double>(d2))) : 0.0f;
            // 00A11E9D FILD [anchor+7A0h], FMUL double 0.75 (00CEC9D8).
            const float r = static_cast<float>(
                static_cast<double>(units.command_building_capture_range_07a0(anchor)) * 0.75);
            if (!(d > r)) {                                              // 00A11EB7
                ++c.inside;
                continue;
            }
            const float k = d / r;                                       // 00A11EC0
            const float point[3] = {dx / k + ax, ay, dz / k + az};        // 00A11EC6..00A11F1F
            if (lander_diag_enabled() && c.diag_lines < 60) {
                ++c.diag_lines;
                log.notef("transport move diag: t=%.2f leader=%s member=%s ready=0 anchor=%s "
                    "d=%.1f r=%.1f point=(%.1f %.1f) bound=%d", clock_seconds,
                    unit_name(g->members.front()).c_str(), unit_name(m).c_str(),
                    unit_name(anchor).c_str(), static_cast<double>(d),
                    static_cast<double>(r), static_cast<double>(point[0]),
                    static_cast<double>(point[2]), kAiGroupTransportMovesBound ? 1 : 0);
            }
            if constexpr (kAiGroupTransportMovesBound) {
                // 00A11F23 CALL 00A02020 (SHIP_AI 107).
                const bool issued = kAiTransportMovesOrderBridgeBound
                    ? issue_moveto_bridge_00a02020(handle(m), point)
                    : tick_issue_moveto(handle(m), point);
                if (issued) ++c.movetos;
            }
        }
        done("AiCommand::transport_moves_00a11b80", 0x00a11b80u);
    }
    // Packet cc9_ai_command_avoid_zone_point: 00A020F0 reached (both sides), and
    // answered by the ship-AI host's runtime / with a moved point (ON only).
    unsigned long long avoid_zone_asks{0};
    unsigned long long avoid_zone_answers{0};
    unsigned long long avoid_zone_moved{0};

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
        // 009FFD70 BSP_Entity_AiClassWeight: `MOV ECX,[ECX+0C4h]; JMP 009FDF30`,
        // the leader's class id into the class-weight switch, which FLDs a float
        // out of the tuning block (FLD1 when the id has no row). The callers pass
        // the first node of the group's +5640h list, the leader (00A2EB6D..
        // 00A2EB95). A squadron's +0C4h is 18h (unit_class_weight's arm).
        Group* g = group_at(group);
        if (!kAiLeaderOrderKeyBound) {
            record("AiGroups::group_leader_order_key", 0x009ffd70u);
            return (g == nullptr || g->members.empty())
                ? 0.0 : static_cast<double>(g->members.front());
        }
        done("AiGroups::group_leader_order_key", 0x009ffd70u);
        if (g == nullptr || g->members.empty()) return 0.0;
        return static_cast<double>(unit_class_weight(g->members.front()));
    }
    const float* group_leader_position(void* group) override {
        Group* g = group_at(group);
        if (g == nullptr || g->members.empty()) return nullptr;
        float y = 0.0f;
        const std::size_t leader = kAiSquadronLeaderPointBound
            ? proxy(g->members.front()) : g->members.front();
        units.unit_position_00fc(leader, g->leader_position[0], y,
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
        // 00A15970. Effective game modes 4..7 ask the one mode planner at
        // brain+10h..+1Ch through vtable[+30h] (00A1D110..00A1D1A0); every other
        // mode calls vtable[+30h] on all four of brain+0h..+0Ch in order
        // (00A159E8..00A15A44) and answers their OR (00A15A46..00A15A6A). For
        // those four planners the slot is 00A18480, which reads and clears +2Ch.
        // CORRECTION (packet cc9_party_replan_flag): the earlier comment read
        // only the mode arms, and a claim does set the byte.
        if (!kAiPartyReplanFlagBound) {
            record("AiGroups::brain_wants_immediate_think", 0x00a15970u);
            return false;
        }
        Brain* b = static_cast<Brain*>(brain);
        const int mode = game_mode();
        if (b == nullptr || (mode >= 4 && mode <= 7)) {
            // The mode planners' +30h bodies are not bound in this process,
            // which runs the campaign's mode 0.
            record("AiGroups::brain_wants_immediate_think", 0x00a15970u);
            return false;
        }
        bool any = false;
        for (int slot = 0; slot < 4; ++slot) {
            Planner& p = b->planners[static_cast<std::size_t>(slot)];
            const bool flag = p.replan_002c;  // 00A18480 MOV AL,[ECX+2Ch]
            p.replan_002c = false;            // 00A18483 MOV byte [ECX+2Ch],0
            any = any || flag;
        }
        done("AiGroups::brain_wants_immediate_think", 0x00a15970u);
        if (any) ++replan_immediate;
        return any;
    }
    void* create_party_brain(int party_slot) override {
        if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return nullptr;
        auto brain = std::make_unique<Brain>();
        brain->party_slot = party_slot;
        brain->world_set = kAiPartyGateUnforcedBound ? slot_team(party_slot) : party_slot;
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
    void set_current_party(int party_slot) override {
        current_party = party_slot;
        current_team = (kAiPartyGateUnforcedBound && party_slot >= 0) ? slot_team(party_slot)
                                                                     : party_slot;
    }
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
        in.own_team = current_team;
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
                enemy_team_group_count(bsp::ai_enemy_team_index(current_team)));
        }
        if constexpr (kAiCaptureThinkBound && kAiPlannerSlotKindsBound) {
            if (p->kind == bsp::AiPlannerKind::Attack) {
                attack_think_00a1cf90(*p);
                ticking_planner = nullptr;
                return;
            }
            if (p->kind == bsp::AiPlannerKind::Capture && capture_think_00a29fd0(*p)) {
                ticking_planner = nullptr;
                return;
            }
            if constexpr (kAiDefendThinkBound) {
                if (p->kind == bsp::AiPlannerKind::Defend && defend_think_00a28a60(*p)) {
                    ticking_planner = nullptr;
                    return;
                }
            }
            if constexpr (kAiSellThinkBound) {
                if (p->kind == bsp::AiPlannerKind::Sell) {
                    sell_think_00a22800(*p);
                    ticking_planner = nullptr;
                    return;
                }
            }
        }
        bsp::ai_mode_planner_tick(*this, in);
        ticking_planner = nullptr;
        record("AiPlanners::planner_tick", 0x00a26510u);
    }

    unsigned long long attack_thinks{0};
    unsigned long long capture_thinks{0};
    unsigned long long capture_target_fallbacks{0};
    unsigned long long capture_handoffs{0};
    // 00CE3800, the value 00A32F29 resets every party's aggressive ratio to.
    static constexpr float kPartyAggressiveRatio = 0.5f;
    // Packet cc9_planner_defend_capture_thinks. 00A1CF90, body 00A1CF90-00A1D00F:
    // for every owned group, 00A1CB80(group, [00F8A8D0 + party*1Ch], 0)
    // (00A1CFAB MOVSS the party's aggressive ratio; 00A1CFEE PUSH 0; 00A1CFF7).
    // The ratio has no host store (the AIEnable table branch is unbound and no
    // installed mission script sets aggressiveRatio), so the reset value
    // 00A32F29 writes, 0.5 at 00CE3800, stands.
    void attack_think_00a1cf90(Planner& p) {
        ++attack_thinks;
        const std::vector<Group*> owned = p.owned;
        for (Group* g : owned) {
            if (g == nullptr || g->destroyed) continue;
            bsp::ai_planner_choose_attack_target(*this, g, current_team,
                bsp::ai_enemy_team_index(current_team), kPartyAggressiveRatio, false);
        }
        done("AiPlanners::attack_think_00a1cf90", 0x00a1cf90u);
    }
    // 00A29FD0, the Capture think. Its targets are list 28 (world+19CCh -> +16Ch,
    // the CommandBuilding list, docs/UNIT_WORLD_REGISTRATION.md) entities not on
    // the planner's side (00A2A13B CMP [e+54h],[planner+30h]). The assignment
    // loop runs only while both the target and the group records are non-empty
    // (`while (targets != 0 && groups != 0)`), so with no enemy CommandBuilding
    // no 00A1A720 order is issued. Every group left unassigned is then released
    // through vtable+24h (00A1E210: observer unregister, erase from +20h,
    // group+5654h = 0) and claimed by brain+4h, Attack, when its record's
    // nearest own list-28 entity (node+1Ch) is null (00A2AFC0), or by brain+8h
    // when it is set (00A2B1CF / 00A2B2AF). With no target the think then frees
    // and returns (local_17c == 0) before the spawn arm.
    // Returns false when an enemy CommandBuilding exists: the target path
    // (00A1E250 scoring, the assignment loop, the merges and the spawn arm) is
    // not reconstructed yet and the previous Siege-shape stand-in runs.
    bool capture_think_00a29fd0(Planner& p) {
        const int side = current_team;
        const int slot = current_party;
        bool have_target = false;
        bool own_building = false;
        for (std::size_t unit = 0; unit < units.count(); ++unit) {
            if (!units.unit_is_kind_of(unit, 0x1C)) continue;          // MCommandBuilding
            if (!units.unit_active(unit)) continue;
            if (units.unit_side_0054(unit) != side) have_target = true;
            else own_building = true;
        }
        if (have_target) {
            if constexpr (kAiCaptureTargetPathBound) {
                Brain* owner = (slot >= 0 && slot < bsp::kAiGroupPartySlotCount)
                    ? brains[static_cast<std::size_t>(slot)].get() : nullptr;
                capture_target_path_00a29fd0(p, side, owner);
                return true;
            }
            // BSP_CAPTURE_DIAG=1: print what the target path would do, and do
            // nothing with it.
            if (capture_diag_enabled()) capture_diag(capture_plan_00a29fd0(p, side), side, false);
            ++capture_target_fallbacks;
            record("AiPlanners::capture_target_path_00a2a130", 0x00a2a130u);
            return false;
        }
        ++capture_thinks;
        Brain* brain = (slot >= 0 && slot < bsp::kAiGroupPartySlotCount)
            ? brains[static_cast<std::size_t>(slot)].get() : nullptr;
        if (brain == nullptr) return true;
        const std::vector<Group*> owned = p.owned;
        for (Group* g : owned) {
            if (g == nullptr || g->destroyed) continue;
            // 00A1E210: release from the Capture planner.
            p.owned.erase(std::remove(p.owned.begin(), p.owned.end(), g), p.owned.end());
            if (g->claimed_by == &p) g->claimed_by = nullptr;
            // node+1Ch: the nearest own-side list-28 entity. With own buildings
            // present the record takes one; the host does not rank them, and a
            // group with an own building nearby goes to brain+8h.
            const int to_slot = own_building ? 2 : 1;
            Planner& next = brain->planners[static_cast<std::size_t>(to_slot)];
            if (g->claimed_by == nullptr) {
                g->claimed_by = &next;
                next.owned.push_back(g);
                ++capture_handoffs;
            }
        }
        done("AiPlanners::capture_think_00a29fd0", 0x00a29fd0u);
        return true;
    }

    // ---- Packet cc9_planner_defend_capture_thinks part 2: the target path ----
    // docs/PLANNER_TASK_CHOICE.md section 8 carries the listings.

    // The Capture_ keys 00A335D0 stores (loader +19Ch..+1CCh, which 00A371A0's
    // reader sees 4 bytes lower), as this installation's
    // scripts/datatables/highlvlaiglobals.lua (mtime 2024-07-13) authors them in
    // IslandCaptureParams_Rookie lines 94-103. Regular and Veteran author the
    // same scoring values and differ only in Capture_SpawnDelay ({15,5} and
    // {5,1}), which feeds the spawn arm this host keeps as a record. The shared
    // tuning block does not load these keys, so the host carries them here.
    struct CaptureTuning {
        float arrive_to_range_time{30.0f};    // reader +198h Capture_ArriveToRangeTime
        float point_value{35.0f};             // +19Ch Capture_CapturePointResourceValue
        float min_resource{200.0f};           // +1A0h Capture_MinimalResource
        float collect_defenders{3000.0f};     // +1A4h Capture_CollectDefendersDist
        float act_mul_2{1.15f};               // +1A8h Capture_ActAttackTargetWeightMul[2]
        float act_mul_1{15.0f};               // +1ACh Capture_ActAttackTargetWeightMul[1]
        float act_dist_1{3000.0f};            // +1B0h Capture_ActAttackTargetWeightMulDist[1]
        float act_dist_2{4500.0f};            // +1B4h Capture_ActAttackTargetWeightMulDist[2]
        float min_cb_target_weight{0.5f};     // +1B8h Capture_MinimalCBTargetWeight
        float cb_strategic_mul{0.8f};         // +1BCh Capture_CommandBuildingStrategicWeightMul
        float spawn_delay_1{60.0f};           // +1C0h Capture_SpawnDelay[1]
        float spawn_delay_2{45.0f};           // +1C4h Capture_SpawnDelay[2]
        float spawn_delay_time{150.0f};       // +1C8h Capture_SpawnDelayTime
    };
    CaptureTuning capture_tuning() const {
        CaptureTuning t;
        if (tuning.mode == bsp::AiTuningMode::IslandCaptureRegular) {
            t.spawn_delay_1 = 15.0f;
            t.spawn_delay_2 = 5.0f;
        } else if (tuning.mode == bsp::AiTuningMode::IslandCaptureVeteran) {
            t.spawn_delay_1 = 5.0f;
            t.spawn_delay_2 = 1.0f;
        }
        return t;
    }

    // 00A1E6D9 / 00A1E701 -> 00946FC0 BSP_AiParty_AvailableResources:
    // [00E0CFB4] * 0.5 - sum(unit+304h) - 009469F0(team). [00E0CFB4] holds
    // 2400.0 in the image's .data (and 00CE396C, the 2400.0 005E3262 stores
    // whenever 004BCA50 answers above 3, which a campaign mission does).
    // unit+304h is the entity's `ResourceUsage` (0077E864), which nothing in
    // this installation's scripts or reference scenes authors, so every unit
    // holds 0. LABELLED: 009469F0 sums 009467B0 over the [00F89B3C] records
    // of the team, which this process never creates (the spawn arm is a
    // record), so it is 0.
    static constexpr float kCaptureAvailableResources = 2400.0f * 0.5f;
    // 00A2C530, the group's resource: sum(member+304h). 0 for every group on
    // this installation (see above), so no assignment ever lowers a price.
    static constexpr float kCaptureGroupResource = 0.0f;
    // LABELLED: the CommandBuilding's +7A0h CaptureRange (006F2780, default
    // 500 at 006F27E5). This installation's four reference CommandBuildings
    // (USN01 CB2, USN13 CB2 / CB4 / CBT) author 100; the host has no property
    // bag reader for it here, so the default stands.
    static constexpr float kCaptureRangeStandIn = 500.0f;

    // 00A03510, the unit's CaptureWeight, switched on entity+C4h: Destroyer 7
    // -> 2.0 (00CE3958); Submarine 8, TorpedoBoat 0Eh, PlaneSquadron 18h -> 1.0;
    // MotherShip 9 -> 3.0 (00CE3854); Cruiser 0Ah -> 4.0 (00CE3D34); BattleShip
    // 0Dh -> 5.0 (00CE3850); CommandBuilding 1Ch -> the Lua `CaptureWeight`,
    // GetFloatOrDefault(1.0), unauthored on this installation; any other id 0.
    // Cargo 0Bh answers 3.0 when [unit+538h]->vtable[+2Ch]() is true and 0
    // otherwise: the troop-landing trait, behind kTroopLandingTraitBound. The strafeable
    // troop transports (classes 224 and 234, docs/SHIP_AI_OPEN_ITEMS.md 19) answer true. LandingShip 0Ch answers
    // 0.1 (00D7A2F0) when 00827F70 is true, which for a class-0Ch ship is the
    // BigLandingShip byte +808h being clear (its default), else 1.0 (00A0360B
    // CALL 00827F70, JZ 00A03636). `big` is +808h (packet cc9_big_landing_ship,
    // false with the switch off, so the 0.1 arm stands as before).
    static float capture_weight_00a03510(int class_id, bool lands_troops = false,
                                         bool big = false) {
        switch (class_id) {
        case 0x07: return 2.0f;
        case 0x08: case 0x0E: case 0x18: return 1.0f;
        case 0x09: return 3.0f;
        case 0x0A: return 4.0f;
        case 0x0B: return (bsp::game::kTroopLandingTraitBound && lands_troops) ? 3.0f : 0.0f;
        case 0x0C: return big ? 1.0f : 0.1f;
        case 0x0D: return 5.0f;
        case 0x1C: return 1.0f;
        default: return 0.0f;
        }
    }

    bool cargo_lands_troops(int class_id, std::size_t unit) {
        if (class_id != 0x0B) return false;
        const bool trait = units.unit_class_lands_troops_vtable_2c(unit);
        if (trait) {
            ++summary.cargo_troop_landers;
            diag_troop_lander("cargo", unit);
        }
        return trait;
    }
    // The first answer per unit and site, for the troop-landing pairs.
    std::vector<std::pair<std::size_t, char>> troop_lander_seen;
    void diag_troop_lander(const char* site, std::size_t unit) {
        const std::pair<std::size_t, char> key{unit, site[0]};
        for (const auto& k : troop_lander_seen) if (k == key) return;
        if (troop_lander_seen.size() >= 32) return;
        troop_lander_seen.push_back(key);
        log.notef("ai troop landing trait site=%s unit=%s class=%d", site,
            unit_name(unit).c_str(), units.unit_class_id(unit));
    }

    float unit_xz_distance(std::size_t a, std::size_t b) const {
        float ax = 0.0f, ay = 0.0f, az = 0.0f, bx = 0.0f, by = 0.0f, bz = 0.0f;
        units.unit_position_00fc(a, ax, ay, az);
        units.unit_position_00fc(b, bx, by, bz);
        const float v[3] = {ax - bx, ay - by, az - bz};
        return bsp::ai_tail_horizontal_length(v);   // 009FFC10
    }

    // 00A1E250 over this process's world lists, one method per native site.
    struct CaptureScoreHost final : bsp::AiCaptureScoreHost {
        struct WorldUnit {
            std::size_t unit;
            int class_id;
        };
        Impl& h;
        const CaptureTuning& t;
        std::vector<WorldUnit> world;
        std::vector<std::size_t> near_list;
        CaptureScoreHost(Impl& host, const CaptureTuning& tuning_in) : h(host), t(tuning_in) {
            // 00A1E2A0 [+64h] list 6 (every ship), 00A1E2F1 [+13Ch] list 24
            // (plane squadrons, 007F10B0). 00A1E343 [+358h] list 69 (AirField)
            // and 00A1E395 [+364h] list 70 (Shipyard) take 00A03510's default
            // arm, weight 0, and 00A03760 then answers 0 in or out of range, so
            // they are not walked.
            for (std::size_t i = 0; i < h.units.world_list_size(6); ++i) {
                const std::size_t u = h.units.world_list_entry(6, i);
                if (u < h.units.count()) world.push_back({u, h.units.unit_class_id(u)});
            }
            for (std::size_t i = 0; i < h.units.world_list_size(24); ++i) {
                const std::size_t u = h.units.world_list_entry(24, i);
                if (u < h.units.count()) world.push_back({u, 0x18});
            }
            // 00A1E410-00A1E64E: [+16Ch] list 28, the CommandBuildings.
            for (std::size_t i = 0; i < h.units.world_list_size(28); ++i) {
                const std::size_t u = h.units.world_list_entry(28, i);
                if (u < h.units.count()) near_list.push_back(u);
            }
        }
        int world_unit_count() override { return static_cast<int>(world.size()); }
        int world_unit_team(int i) override {
            return h.units.unit_side_0054(world[static_cast<std::size_t>(i)].unit);
        }
        float unit_arrival_value(int i, void* target) override {
            // 00A03760.
            const WorldUnit& w = world[static_cast<std::size_t>(i)];
            bsp::AiTailArrivalValueInputs in;
            in.capture_weight = capture_weight_00a03510(w.class_id,
                h.cargo_lands_troops(w.class_id, w.unit), h.big_landing_ship_0808(w.unit));
            in.distance = h.unit_xz_distance(w.unit, unit_index_of(target));
            in.capture_radius = static_cast<float>(static_cast<int>(
                kCaptureAccessorsBound
                    ? h.units.command_building_capture_range_07a0(unit_index_of(target))
                    : kCaptureRangeStandIn));   // 00A037CF FILD target+7A0h
            // IsType(6): [unit+538h]+500h MaxSpeed. IsType(18h): [unit+35Ch]+188h,
            // the plane class MaxSpd. LABELLED: the units host exposes no plane
            // MaxSpd, so a squadron's speed is 0 and it counts only inside the
            // radius (contract in docs/PLANNER_TASK_CHOICE.md section 8).
            if (w.class_id != 0x18 &&
                h.units.unit_is_kind_of(w.unit, bsp::kUnitGunneryKindShipBase)) {
                in.speed = h.units.unit_class_max_speed_0500(w.unit);
            } else if (w.class_id == 0x18 && kCaptureAccessorsBound) {
                in.speed = h.units.plane_class_max_speed_0188(w.unit);   // 00A03819
            } else {
                in.speed = 0.0f;
            }
            in.arrive_to_range_time = t.arrive_to_range_time;
            return bsp::ai_tail_unit_arrival_value(in);
        }
        int near_unit_count() override { return static_cast<int>(near_list.size()); }
        int near_unit_team(int i) override {
            return h.units.unit_side_0054(near_list[static_cast<std::size_t>(i)]);
        }
        float near_unit_distance(int i, void* target) override {
            return h.unit_xz_distance(near_list[static_cast<std::size_t>(i)], unit_index_of(target));
        }
        float available_resources(int team) override {
            (void)team;
            return kCaptureAvailableResources;
        }
        float tuning(std::uint32_t offset) override {
            if (offset == bsp::kAiTailTuningCapturePointValue) return t.point_value;
            if (offset == bsp::kAiTailTuningMinCbTargetWeight) return t.min_cb_target_weight;
            return 0.0f;
        }
        float strategic_gain(void* target) override {
            // 00A1E7A5-00A1E7DA: the target's Lua `StrategicGain`, default 0.0.
            // No reference scene or installed script authors it.
            (void)target;
            return 0.0f;
        }
    };

    static constexpr std::size_t kCaptureNone = static_cast<std::size_t>(-1);
    struct CaptureTargetRec {
        std::size_t unit{kCaptureNone};
        bsp::AiTailCaptureScoreTerms terms{};
        float price{0.0f};            // rec+1Ch
        std::size_t defenders{0};     // 00A24870's record count, for the stand-in
        std::vector<std::size_t> defender_units;   // 00A24870's entities, in list-2 order
        bool live{true};
        std::vector<Group*> assigned; // 00A27AE0's per-target list
    };
    struct CaptureGroupRec {
        Group* g{nullptr};
        float resource{0.0f};                 // value[1], 00A2C530
        std::size_t current{kCaptureNone};    // value[2], 00A1A7A0
        std::size_t near_own{kCaptureNone};   // value[3]
        float near_d2{1.0e10f};               // value[4], seeded from 00CE4970
        std::vector<float> weight;            // value[5], the 00A22D10 map
        std::vector<char> has_weight;
        bool live{true};
    };
    struct CapturePlan {
        std::vector<CaptureTargetRec> targets;
        std::vector<CaptureGroupRec> groups;
        std::vector<std::pair<std::size_t, std::size_t>> assignments;  // (group rec, target rec)
        std::vector<float> assignment_scores;
    };

    // 00A1A7A0(planner)(group): the first list-28 entity not on the planner's
    // side that the group's command is already working on: 00A2BDB0 (an ATTACK
    // whose +1Ch is the entity's +16Ch group), 00A2C150 (a MOVE-family point),
    // 00A2C230 (PATROLTO) or 00A2C1C0 (REGROUPINGMOVE) within 1.0 squared of
    // the entity's +FCh in x and z.
    std::size_t capture_current_target_00a1a7a0(Group* g, int side) const {
        const bsp::AiCommandObject& cmd = g->command;
        for (std::size_t i = 0; i < units.world_list_size(28); ++i) {
            const std::size_t e = units.world_list_entry(28, i);
            if (e >= units.count() || units.unit_side_0054(e) == side) continue;
            Group* eg = e < group_of_unit.size() ? group_of_unit[e] : nullptr;
            if (bsp::ai_command_is_type(cmd.type, bsp::AiCommandType::Attack) &&
                cmd.target_group == static_cast<void*>(eg)) {
                return e;
            }
            float ex = 0.0f, ey = 0.0f, ez = 0.0f;
            units.unit_position_00fc(e, ex, ey, ez);
            const float dx = cmd.target_position[0] - ex;
            const float dz = cmd.target_position[2] - ez;
            const bool at_point = dz * dz + dx * dx < 1.0f;
            if (at_point && (bsp::ai_command_is_type(cmd.type, bsp::AiCommandType::Move) ||
                             bsp::ai_command_is_type(cmd.type, bsp::AiCommandType::PatrolTo) ||
                             bsp::ai_command_is_type(cmd.type, bsp::AiCommandType::RegroupingMove))) {
                return e;
            }
        }
        return kCaptureNone;
    }

    // LABELLED stand-in for 00A250A0 (00A07E40 builds the group's records,
    // 00A24870 the target's, and 00A0C650 BSP_AiGroup_ComposeAttackValue scores
    // them). 00A24870 collects, from world list 2, every entity with +5Dh clear,
    // +54h == planner+34h (the enemy side, 00A1EEB8 SETZ) and IsKindOf 6, 18h or
    // 1Bh within Capture_CollectDefendersDist (3-D, 00A1A660) of the target, or
    // the target alone when there is none. 00A0C650's pair terms are not
    // reconstructed (docs/PLANNER_KATE_TARGETING.md section 3), so this counts
    // the attacker-defender pairs: members x defenders, always positive
    // (kCaptureGroupValueBound replaces the count with 00A0C650's value).
    std::size_t capture_defenders_00a24870(std::size_t target, int enemy_side,
                                            const CaptureTuning& t,
                                            std::vector<std::size_t>* collected = nullptr) const {
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        units.unit_position_00fc(target, tx, ty, tz);
        const float r2 = t.collect_defenders * t.collect_defenders;
        std::size_t n = 0;
        for (std::size_t i = 0; i < units.world_list_size(2); ++i) {
            const std::size_t u = units.world_list_entry(2, i);
            if (u >= units.count()) continue;
            if (!units.unit_active(u)) continue;   // LABELLED: +5Dh read as the active row
            if (units.unit_side_0054(u) != enemy_side) continue;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            units.unit_position_00fc(u, x, y, z);
            const float dx = x - tx, dy = y - ty, dz = z - tz;
            if (r2 < dz * dz + dy * dy + dx * dx) continue;   // 00A1A660 answers 1: skip
            if (units.unit_is_kind_of(u, 0x06) || units.unit_is_kind_of(u, 0x18) ||
                units.unit_is_kind_of(u, 0x1B)) {
                ++n;
                if (collected != nullptr) collected->push_back(u);
            }
        }
        if (n == 0 && collected != nullptr) collected->push_back(target);   // 00A248FD
        return n == 0 ? 1u : n;   // 00A248FD: the target's own record
    }

    // 00A250A0, __thiscall(planner)(group, entity), returns ST0:
    // 00A0C650(00A07E40(group), 00A24870(entity), 0, -1.0 [00D7A260], 0, 0, 1.0).
    // Unlike 00A0F970 it has no empty-group test; an empty attacker list sums 0.
    float capture_group_value_00a250a0(const Group* g, const CaptureTargetRec& rec) {
        std::vector<GroupValueRecord> attackers;
        std::vector<GroupValueRecord> defenders;
        for (const std::size_t m : g->members) attackers.push_back(group_value_record_00a04560(m));
        for (const std::size_t u : rec.defender_units) {
            defenders.push_back(group_value_record_00a04560(u));
        }
        float attack_sum = 0.0f;
        float maxes_sum = 0.0f;
        const float value = compose_attack_value_00a0c650(attackers, defenders, attack_sum,
                                                          maxes_sum);
        ++capture_value_calls;
        if (value == 0.0f) ++capture_value_zero;
        if (capture_diag_enabled() && capture_value_lines < 450) {
            ++capture_value_lines;
            std::size_t barrels = 0;
            for (const GroupValueRecord& a : attackers) {
                const GameAiWeaponFacts::Unit* row = game_ai_weapon_facts().row(a.unit);
                if (row != nullptr) barrels += row->barrels.size();
            }
            log.notef("    capture value t=%.2f leader=%s members=%zu barrels=%zu target=%s "
                      "defenders=%zu value=%.4f sum=%.4f maxes=%.4f stand_in=%zu",
                      static_cast<double>(clock_seconds),
                      g->members.empty() ? "-" : unit_name(proxy(g->members.front())).c_str(),
                      g->members.size(), barrels, unit_name(rec.unit).c_str(),
                      rec.defender_units.size(),
                      static_cast<double>(value), static_cast<double>(attack_sum),
                      static_cast<double>(maxes_sum), g->members.size() * rec.defenders);
        }
        return value;
    }
    unsigned long long capture_value_calls{0};
    unsigned long long capture_value_zero{0};
    unsigned long long capture_value_lines{0};

    std::size_t group_alloc_order(const Group* g) const {
        for (std::size_t i = 0; i < groups.size(); ++i) {
            if (groups[i].get() == g) return i;
        }
        return groups.size();
    }

    // 00A2A0C1-00A2AF40 as a pure plan: nothing is ordered here.
    CapturePlan capture_plan_00a29fd0(Planner& p, int side) {
        CapturePlan plan;
        const CaptureTuning t = capture_tuning();
        const int enemy = bsp::ai_tail_enemy_team(side);   // planner+34h
        CaptureScoreHost score_host(*this, t);
        // 00A2A120-00A2A1E7. The records sit in a map keyed by the entity
        // pointer; LABELLED: this process orders them by unit index (creation
        // order) for want of the image's allocation addresses.
        std::vector<std::size_t> list28;
        for (std::size_t i = 0; i < units.world_list_size(28); ++i) {
            const std::size_t e = units.world_list_entry(28, i);
            if (e < units.count()) list28.push_back(e);
        }
        std::vector<std::size_t> enemy_cbs;
        for (const std::size_t e : list28) {
            if (units.unit_side_0054(e) != side) enemy_cbs.push_back(e);   // 00A2A13B
        }
        std::sort(enemy_cbs.begin(), enemy_cbs.end());
        enemy_cbs.erase(std::unique(enemy_cbs.begin(), enemy_cbs.end()), enemy_cbs.end());
        for (const std::size_t e : enemy_cbs) {
            CaptureTargetRec rec;
            rec.unit = e;
            // 00A2A184: ECX = [brain+24h], the planner's side.
            rec.terms = bsp::ai_capture_target_score(score_host, handle(e), side);
            // 00A2A18B-00A2A1E2, the target's CaptureWeight being the Lua 1.0.
            rec.price = bsp::ai_tail_capture_record_price(rec.terms, capture_weight_00a03510(0x1C),
                                                          t.point_value, t.min_resource);
            rec.defenders = capture_defenders_00a24870(e, enemy, t, &rec.defender_units);
            plan.targets.push_back(std::move(rec));
        }
        // 00A2A263-00A2AA90, one record per owned group, keyed by the group
        // pointer; LABELLED: ordered by the host's allocation order.
        std::vector<Group*> owned;
        for (Group* g : p.owned) {
            if (g != nullptr && !g->destroyed) owned.push_back(g);
        }
        std::stable_sort(owned.begin(), owned.end(), [this](const Group* a, const Group* b) {
            return group_alloc_order(a) < group_alloc_order(b);
        });
        for (Group* g : owned) {
            CaptureGroupRec gr;
            gr.g = g;
            gr.resource = kCaptureGroupResource;                           // 00A2A2C7
            gr.current = capture_current_target_00a1a7a0(g, side);         // 00A2A2D2
            gr.weight.assign(plan.targets.size(), 0.0f);
            gr.has_weight.assign(plan.targets.size(), 0);
            float leader[3] = {0.0f, 0.0f, 0.0f};
            tick_leader_point(g, leader);   // group+5640h's first member, or 00F87574
            for (const std::size_t e : list28) {
                if ((e < group_of_unit.size() ? group_of_unit[e] : nullptr) == g) continue;  // 00A2A319
                float ex = 0.0f, ey = 0.0f, ez = 0.0f;
                units.unit_position_00fc(e, ex, ey, ez);
                const float dx = leader[0] - ex;
                const float dz = leader[2] - ez;
                if (units.unit_side_0054(e) == side) {
                    // 00A2A84E..: the nearest own list-28 entity.
                    const float d2 = dx * dx + dz * dz;
                    if (d2 < gr.near_d2) {
                        gr.near_own = e;
                        gr.near_d2 = d2;
                    }
                    continue;
                }
                std::size_t ti = 0;
                while (ti < plan.targets.size() && plan.targets[ti].unit != e) ++ti;
                if (ti == plan.targets.size()) continue;
                // 00A2A380: the base weight, 00A250A0(group, entity), or the
                // members x defenders stand-in with kCaptureGroupValueBound OFF.
                // BSP_CAPTURE_DIAG=1 logs the image's value in both states.
                if (!kCaptureGroupValueBound && capture_diag_enabled()) {
                    (void)capture_group_value_00a250a0(g, plan.targets[ti]);
                }
                const float base = kCaptureGroupValueBound
                    ? capture_group_value_00a250a0(g, plan.targets[ti])
                    : static_cast<float>(g->members.size()) *
                          static_cast<float>(plan.targets[ti].defenders);
                // 00A2A393: 1.0 (00D7A24C) unless this is the group's current
                // target, which takes 00419010(+1B0h, +1ACh, +1B4h, +1A8h, d).
                float mul = 1.0f;
                if (e == gr.current) {
                    const double d2 = static_cast<double>(dz * dz + dx * dx);
                    const float d = d2 <= 1.0e-11 ? 0.0f : static_cast<float>(std::sqrt(d2));
                    mul = bsp::ai_tail_capture_attack_weight(d, t.act_dist_1, t.act_mul_1,
                                                             t.act_dist_2, t.act_mul_2);
                }
                gr.weight[ti] = mul * base;   // 00A2A660-00A2A66E
                gr.has_weight[ti] = 1;
            }
            plan.groups.push_back(std::move(gr));
        }
        // 00A2AAA0-00A2AF40, while both maps are non-empty.
        std::size_t live_targets = plan.targets.size();
        std::size_t live_groups = plan.groups.size();
        while (live_targets != 0 && live_groups != 0) {
            float best = -1.0e10f;                   // 00CE4ADC
            std::size_t best_t = kCaptureNone, best_g = kCaptureNone;
            for (std::size_t ti = 0; ti < plan.targets.size(); ++ti) {
                if (!plan.targets[ti].live) continue;
                const float s = plan.targets[ti].terms.total;   // rec+4h
                for (std::size_t gi = 0; gi < plan.groups.size(); ++gi) {
                    const CaptureGroupRec& gr = plan.groups[gi];
                    if (!gr.live) continue;
                    const float w = gr.has_weight[ti] ? gr.weight[ti] : 0.0f;
                    if (!(w > 0.0f)) continue;           // 00A2AC02 COMISS 00D7A218, JBE
                    // 00A2AC11-00A2AC3E: (1 - k) * w as a double, + k * s, to float.
                    const double k = static_cast<double>(t.cb_strategic_mul);
                    const float score = static_cast<float>(
                        k * static_cast<double>(s) + (1.0 - k) * static_cast<double>(w));
                    if (score > best) {                 // 00A2AC4A FCOMIP, JBE
                        best = score;
                        best_t = ti;
                        best_g = gi;
                    }
                }
            }
            if (best_t == kCaptureNone) break;          // 00A2ACC1
            plan.assignments.push_back({best_g, best_t});
            plan.assignment_scores.push_back(best);
            CaptureTargetRec& tr = plan.targets[best_t];
            tr.assigned.push_back(plan.groups[best_g].g);
            tr.price -= plan.groups[best_g].resource;   // 00A2ADD7-00A2ADE4
            plan.groups[best_g].live = false;
            --live_groups;
            if (tr.price < 0.0f) {                      // <= 0 and != 0
                tr.live = false;
                --live_targets;
            }
        }
        return plan;
    }

    // 00A1A720 BSP_AiPlanner_OrderCaptureGroup (docs/AI_PLANNER_TAILS.md 3).
    // Answers 0 attack, 1 defendposition, 2 patrolto, 3 kept.
    int order_capture_group_00a1a720(Group* g, std::size_t target) {
        Group* tg = target < group_of_unit.size() ? group_of_unit[target] : nullptr;
        if (tg != nullptr) {
            // target+16Ch set: 00A2CBD0 with [00F8A8D0 + party*1Ch].
            order_attack(g, tg, kPartyAggressiveRatio);
            return 0;
        }
        if (group_matches_009fe0b0(g)) {
            // 00A2BE20: returns when the command already answers IsType(11).
            if (bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::DefendPosition)) return 3;
            if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
            bsp::AiCommandObject c;
            c.type = bsp::AiCommandType::DefendPosition;   // vtable 00D22A38
            c.owner_group = g;
            g->command = c;
            ++capture_orders_defend;
            return 1;
        }
        // 00A2C310 with the target's +FCh: nothing when 00A2C230 finds the
        // command already a PATROLTO within 1.0 of it; otherwise new(14h) with
        // the PATROLTO vtable 00D22B3C and the point at +8h.
        float ex = 0.0f, ey = 0.0f, ez = 0.0f;
        units.unit_position_00fc(target, ex, ey, ez);
        if (bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::PatrolTo)) {
            const float dx = g->command.target_position[0] - ex;
            const float dz = g->command.target_position[2] - ez;
            if (dz * dz + dx * dx < 1.0f) return 3;
        }
        if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
        bsp::AiCommandObject c;
        c.type = bsp::AiCommandType::PatrolTo;
        c.owner_group = g;
        c.target_position[0] = ex;
        c.target_position[1] = ey;
        c.target_position[2] = ez;
        g->command = c;
        ++capture_orders_patrol;
        return 2;
    }

    // 00A1D010(a, b), from 00A2B12F with the earlier and the later group of
    // one target's list: both populated (+5644h), both grouping-enabled
    // (+5648h, 1 from the constructor), same team (+5638h), neither answering
    // 00A2C600, fewer than four members together, leaders closer than 1000
    // (3-D squared against the double 1.0e6 at 00CE4C08) and both first
    // members alike under IsKindOf(6): then 00A2DB80, a absorbs b.
    bool capture_merge_00a1d010(Group* a, Group* b) {
        if (a == nullptr || b == nullptr || a == b) return false;
        if (a->members.empty() || b->members.empty()) return false;
        if (a->team != b->team) return false;
        if (group_matches_009fe0b0(a) || group_matches_009fe0b0(b)) return false;
        if (a->members.size() + b->members.size() >= 4u) return false;
        float pa[3] = {0.0f, 0.0f, 0.0f};
        float pb[3] = {0.0f, 0.0f, 0.0f};
        tick_leader_point(a, pa);
        tick_leader_point(b, pb);
        const float d[3] = {pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]};
        const float len2 = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
        if (!(static_cast<double>(len2) < 1.0e6)) return false;
        const bool a_ship = !is_squadron(a->members.front()) &&
            units.unit_is_kind_of(a->members.front(), bsp::kUnitGunneryKindShipBase);
        const bool b_ship = !is_squadron(b->members.front()) &&
            units.unit_is_kind_of(b->members.front(), bsp::kUnitGunneryKindShipBase);
        if (a_ship != b_ship) return false;
        merge_group(a, b);
        ++capture_merges;
        return true;
    }

    // 00A1E210 (planner vtable +24h, no Ghidra function; body 00A1E210-00A1E246):
    // when the planner owns the group, unregister the observer, erase it from
    // +20h and clear group+5654h. Then 00A2B1E3 / 00A2B2C3 claim it for the
    // next planner when that one does not own it already.
    void capture_hand_off(Planner& from, Group* g, Planner& to) {
        if (std::find(from.owned.begin(), from.owned.end(), g) != from.owned.end()) {
            from.owned.erase(std::remove(from.owned.begin(), from.owned.end(), g), from.owned.end());
            if (g->claimed_by == &from) g->claimed_by = nullptr;
        }
        if (g->claimed_by == nullptr) {
            g->claimed_by = &to;
            to.owned.push_back(g);
            ++capture_handoffs;
        }
    }

    const char* capture_order_name(Group* g, std::size_t target) {
        Group* tg = target < group_of_unit.size() ? group_of_unit[target] : nullptr;
        if (tg != nullptr) return "attack";
        return group_matches_009fe0b0(g) ? "defendposition" : "patrolto";
    }

    void capture_diag(const CapturePlan& plan, int side, bool applying) {
        if (diag_capture_lines >= 60) return;
        ++diag_capture_lines;
        log.notef("  capture diag %s t=%.2f party=%d targets=%zu groups=%zu assignments=%zu",
            applying ? "on" : "off-plan", static_cast<double>(clock_seconds), side,
            plan.targets.size(), plan.groups.size(), plan.assignments.size());
        for (const CaptureTargetRec& tr : plan.targets) {
            Group* tg = tr.unit < group_of_unit.size() ? group_of_unit[tr.unit] : nullptr;
            log.notef("    target %s side=%d total=%.3f a=%.3f b=%.3f c=%.3f d=%.3f e=%.3f "
                "price=%.1f defenders=%zu group_leader=%s group_members=%zu",
                unit_name(tr.unit).c_str(), units.unit_side_0054(tr.unit),
                static_cast<double>(tr.terms.total), static_cast<double>(tr.terms.own_value),
                static_cast<double>(tr.terms.enemy_value), static_cast<double>(tr.terms.own_reach),
                static_cast<double>(tr.terms.enemy_reach), static_cast<double>(tr.terms.strategic_gain),
                static_cast<double>(tr.price), tr.defenders,
                (tg == nullptr || tg->members.empty()) ? "-" : unit_name(proxy(tg->members.front())).c_str(),
                tg == nullptr ? std::size_t{0} : tg->members.size());
        }
        for (const CaptureGroupRec& gr : plan.groups) {
            log.notef("    group leader=%s members=%zu command=%d current=%s near_own=%s",
                gr.g->members.empty() ? "-" : unit_name(proxy(gr.g->members.front())).c_str(),
                gr.g->members.size(), static_cast<int>(gr.g->command.type),
                gr.current == kCaptureNone ? "-" : unit_name(gr.current).c_str(),
                gr.near_own == kCaptureNone ? "-" : unit_name(gr.near_own).c_str());
        }
        for (std::size_t i = 0; i < plan.assignments.size(); ++i) {
            const CaptureGroupRec& gr = plan.groups[plan.assignments[i].first];
            const CaptureTargetRec& tr = plan.targets[plan.assignments[i].second];
            log.notef("    assign leader=%s -> %s score=%.3f order=%s",
                gr.g->members.empty() ? "-" : unit_name(proxy(gr.g->members.front())).c_str(),
                unit_name(tr.unit).c_str(), static_cast<double>(plan.assignment_scores[i]),
                capture_order_name(gr.g, tr.unit));
        }
    }

    static bool capture_diag_enabled() {
        static const bool enabled = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            if (_dupenv_s(&text, &bytes, "BSP_CAPTURE_DIAG") != 0) return false;
            const bool on = text != nullptr && text[0] != '0';
            std::free(text);
            return on;
        }();
        return enabled;
    }

    // 00A29FD0 with an enemy CommandBuilding (kAiCaptureTargetPathBound).
    void capture_target_path_00a29fd0(Planner& p, int side, Brain* brain) {
        ++capture_path_thinks;
        const CaptureTuning t = capture_tuning();
        // 00A2A046-00A2A0C1: dt from +40h, then +38h and +3Ch advance, and the
        // spawn gate compares +3Ch with the ramp over +38h.
        float dt = 0.0f;
        if (p.last_clock_0040 >= 0.0f && clock_seconds - p.last_clock_0040 >= 0.0f) {
            dt = clock_seconds - p.last_clock_0040;
        }
        p.last_clock_0040 = clock_seconds;
        p.age_0038 += dt;
        p.since_spawn_003c += dt;
        const bool spawn_due = bsp::ai_tail_capture_spawn_due(p.age_0038, p.since_spawn_003c,
            t.spawn_delay_1, t.spawn_delay_2, t.spawn_delay_time);
        CapturePlan plan = capture_plan_00a29fd0(p, side);
        if (capture_diag_enabled()) capture_diag(plan, side, true);
        // 00A1A720 at 00A2AD77, in the loop's own order.
        for (const auto& a : plan.assignments) {
            ++capture_assignments;
            const int kind = order_capture_group_00a1a720(plan.groups[a.first].g,
                                                          plan.targets[a.second].unit);
            if (kind == 0) ++capture_orders_attack;
            else if (kind == 3) ++capture_orders_kept;
        }
        // 00A2AF60-00A2B15C: every target whose list holds more than one group
        // offers each earlier group every later one.
        for (CaptureTargetRec& tr : plan.targets) {
            if (tr.assigned.size() < 2u) continue;
            for (std::size_t i = 0; i < tr.assigned.size(); ++i) {
                for (std::size_t j = i + 1; j < tr.assigned.size(); ++j) {
                    capture_merge_00a1d010(tr.assigned[i], tr.assigned[j]);
                }
            }
        }
        // 00A2AFC0: the unassigned groups, split by value[3], are released and
        // handed to brain+4h (none) or brain+8h (an own list-28 entity).
        if (brain != nullptr) {
            std::vector<Group*> to_attack, to_sell;
            for (const CaptureGroupRec& gr : plan.groups) {
                if (!gr.live) continue;
                (gr.near_own == kCaptureNone ? to_attack : to_sell).push_back(gr.g);
            }
            for (Group* g : to_attack) capture_hand_off(p, g, brain->planners[1]);
            for (Group* g : to_sell) capture_hand_off(p, g, brain->planners[2]);
        }
        // 00A2B3F0: no live target left frees and returns. Otherwise the spawn
        // arm (00A2B400-00A2B7EB): gated by 00946970(brain+20h) <= 0 and the
        // ramp, then the budget; it quick-spawns "[capture]<id>" at the best
        // target. LABELLED: a record, like every host quick-spawn.
        bool any_live = false;
        for (const CaptureTargetRec& tr : plan.targets) any_live = any_live || tr.live;
        if (any_live && spawn_due) {
            ++capture_spawn_due;
            record("AiPlanners::capture_spawn_arm_00a2b400", 0x00a2b400u);
        }
        done("AiPlanners::capture_target_path_00a29fd0", 0x00a29fd0u);
    }
    // 00A2E4C0 BSP_AiGroup_SplitAirMembers (__fastcall group, returns the new
    // group or 0): the members BSP_Entity_IsPlaneOrSquadron accepts, when there
    // are some but fewer than the population, leave the group (0077BEA0, their
    // +16Ch cleared, 006956A0) and form a new one: the first through
    // BSP_AiGroup_Construct, the rest through BSP_AiGroup_AddEntity.
    Group* split_air_members_00a2e4c0(Group* g) {
        std::vector<std::size_t> air;
        for (const std::size_t unit : g->members) {
            if (is_squadron(unit) || units.unit_is_kind_of(unit, 0x0F) ||
                units.unit_is_kind_of(unit, 0x18)) {
                air.push_back(unit);
            }
        }
        if (air.empty() || air.size() >= g->members.size()) return nullptr;
        std::vector<std::size_t> kept;
        for (const std::size_t unit : g->members) {
            if (std::find(air.begin(), air.end(), unit) == air.end()) kept.push_back(unit);
            else if (unit < group_of_unit.size() && group_of_unit[unit] == g) group_of_unit[unit] = nullptr;
        }
        g->members.swap(kept);
        Group* made = static_cast<Group*>(create_group(handle(air.front())));
        for (std::size_t i = 1; i < air.size(); ++i) attach(made, air[i]);
        ++sell_splits;
        return made;
    }
    // 00A22800 BSP_AiPlanner_SellThink, body 00A22800-00A228D2, read in full.
    // The first walk claims each split group (00A2284D, 00A22750: appended to
    // +24h, so the walk reaches it too and finds nothing more to split). The
    // second walk installs SELLING on every owned group not already answering
    // IsType(0Eh). LABELLED: the SELLING tick 00A11FF0 (nearest own list-28
    // entity, then 00A02020) has no host arm, so a SELLING group is not moved.
    void sell_think_00a22800(Planner& p) {
        ++sell_thinks;
        for (std::size_t i = 0; i < p.owned.size(); ++i) {
            Group* g = p.owned[i];
            if (g == nullptr || g->destroyed) continue;
            Group* made = split_air_members_00a2e4c0(g);
            if (made != nullptr &&
                std::find(p.owned.begin(), p.owned.end(), made) == p.owned.end()) {
                made->claimed_by = &p;
                p.owned.push_back(made);
            }
        }
        for (Group* g : p.owned) {
            if (g == nullptr || g->destroyed) continue;
            if (bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::Selling)) continue;
            if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
            bsp::AiCommandObject c;
            c.type = bsp::AiCommandType::Selling;
            c.owner_group = g;
            g->command = c;
            ++sell_orders;
        }
        done("AiPlanners::sell_think_00a22800", 0x00a22800u);
    }
    // ---- Packet cc9_defend_records_path (docs/PLANNER_TASK_CHOICE.md section 12) ----
    struct DefendRecord {
        std::size_t entity{kCaptureNone};
        float threat{0.0f};      // out[0], T = (b - c) - a
        float weight{0.0f};      // out[1], gain * T
        float own_value{0.0f};   // out[2], a
        float enemy_value{0.0f}; // out[3], b
        float excluded{0.0f};    // out[4], c
        float gain{0.0f};        // out[5]
        float requirement{0.0f}; // out[6]
        float remaining{0.0f};   // rec+30h, seeded from out[6]
    };
    // IslandCaptureParams_Rookie lines 105-110 (the three IslandCapture blocks
    // author the same values): reader +1DCh Defend_MergeTargetDist 500, +1E0h
    // Defend_MergeGroupsDist 300, +1E4h Defend_CollectEnemiesDist 4000, +1E8h
    // Defend_AgainstEnemyResourceMul 1.5, +1ECh Defend_MinimalResource 0. The
    // loader 00A335D0 stores them at +1E0h..+1F0h, 4 bytes above the reader.
    static constexpr float kDefendMergeTargetDist = 500.0f;
    static constexpr float kDefendMergeGroupsDist = 300.0f;
    static constexpr float kDefendCollectEnemiesDist = 4000.0f;
    static constexpr float kDefendAgainstEnemyResourceMul = 1.5f;
    static constexpr float kDefendMinimalResource = 0.0f;

    // LABELLED stand-in for 007EDAD0 BSP_PlaneSquadron_AmmoType (the first
    // ordnance kind of the squadron's planes): the host has no ordnance
    // reader, so the leader plane's class answers: TorpedoBomber 11h -> 2,
    // DiveBomber 12h -> 1, LevelBomber 10h -> 5, Kamikaze 17h -> 6, any other
    // (fighters, recon) -> 0.
    int squadron_ammo_type_stand_in(std::size_t squadron_candidate) const {
        const std::size_t lead = proxy(squadron_candidate);
        if (units.unit_is_kind_of(lead, 0x11)) return 2;
        if (units.unit_is_kind_of(lead, 0x12)) return 1;
        if (units.unit_is_kind_of(lead, 0x10)) return 5;
        if (units.unit_is_kind_of(lead, 0x17)) return 6;
        return 0;
    }

    // 00A243D0(ECX = own side, EDX = candidate)(out[7], explain), RET 8, read in
    // full. It walks world list 2 ([+34h]): an entity answering
    // vtable[+5Ch](1Ch) is skipped (00A24458); w = 00A03510(entity) must be
    // above 0; in = x/z distance squared < Defend_CollectEnemiesDist squared.
    // Own side: a member of a group whose command answers neither IsType(6)
    // nor IsType(2) (00A2450B / 00A2451A) adds w to a when in range. The other
    // side ((own == 0), SETZ): in range, w adds to b, and to c as well for a
    // TorpedoBoat (+C4h == 0Eh) or a squadron (18h) whose 007EDAD0 is 0, 2 or
    // 3. T = (b - c) - a; gain = Lua StrategicGain + 1.0 (00D7A210); the
    // requirement is max(+1E8h * +19Ch * b, +1ECh).
    DefendRecord defend_score_00a243d0(std::size_t target, int own) {
        DefendRecord r;
        r.entity = target;
        const int enemy = own == 0 ? 1 : 0;
        const float limit2 = kDefendCollectEnemiesDist * kDefendCollectEnemiesDist;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        units.unit_position_00fc(target, tx, ty, tz);
        auto visit = [&](std::size_t candidate, int class_id) {
            const std::size_t body = proxy(candidate);
            if (class_id == 0x1C) return;
            const float w = capture_weight_00a03510(class_id,
                cargo_lands_troops(class_id, candidate), big_landing_ship_0808(candidate));
            if (!(0.0f < w)) return;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            units.unit_position_00fc(body, x, y, z);
            const float dx = tx - x;
            const float dz = tz - z;
            const bool in = dx * dx + dz * dz < limit2;
            const int side = units.unit_side_0054(body);
            if (side == own) {
                Group* g = candidate < group_of_unit.size() ? group_of_unit[candidate] : nullptr;
                if (g == nullptr) return;
                const bool idle = !bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::Attack) &&
                                  !bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::Move);
                if (idle && in) r.own_value += w;
            } else if (side == enemy && in) {
                r.enemy_value += w;
                bool excluded = class_id == 0x0E;
                if (class_id == 0x18) {
                    const int ammo = squadron_ammo_type_stand_in(candidate);
                    excluded = ammo == 0 || ammo == 3 || ammo == 2;
                }
                if (excluded) r.excluded += w;
            }
        };
        // LABELLED: list 2's squadron entities are this host's squadron
        // candidates; the planes in the list take 00A03510's default arm.
        for (std::size_t i = 0; i < units.world_list_size(2); ++i) {
            const std::size_t u = units.world_list_entry(2, i);
            if (u < units.count()) visit(u, units.unit_class_id(u));
        }
        for (std::size_t s = 0; s < squadrons.size(); ++s) visit(units.count() + s, 0x18);
        r.threat = (r.enemy_value - r.excluded) - r.own_value;
        r.gain = 0.0f + 1.0f;   // StrategicGain, unauthored, + 00D7A210
        r.weight = r.gain * r.threat;
        r.requirement = kDefendAgainstEnemyResourceMul * capture_tuning().point_value * r.enemy_value;
        if (r.requirement < kDefendMinimalResource) r.requirement = kDefendMinimalResource;
        r.remaining = r.requirement;
        return r;
    }

    // 00A2C230 on a group: its command is PATROLTO with +8h/+10h within 1.0
    // squared of the entity's +FCh.
    bool group_patrols_at_00a2c230(Group* g, std::size_t entity) const {
        if (!bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::PatrolTo)) return false;
        float ex = 0.0f, ey = 0.0f, ez = 0.0f;
        units.unit_position_00fc(entity, ex, ey, ez);
        const float dx = g->command.target_position[0] - ex;
        const float dz = g->command.target_position[2] - ez;
        return dz * dz + dx * dx < 1.0f;
    }

    // 00A1CF10 BSP_AiGroup_PickAnchorEntity(group, candidates): a group with no
    // groupable combatant answers its first member; any other answers the
    // first candidate its command patrols at, or none.
    std::size_t defend_anchor_00a1cf10(Group* g, const std::vector<std::size_t>& candidates) {
        if (!group_has_groupable_combatant(g)) {
            return g->members.empty() ? kCaptureNone : proxy(g->members.front());
        }
        for (const std::size_t e : candidates) {
            if (group_patrols_at_00a2c230(g, e)) return e;
        }
        return kCaptureNone;
    }

    // 00A28A60 BSP_AiPlanner_DefendThink, body 00A28A60-00A29E2A.
    // Pass A (00A28AE5 00A2DEF0, 00A28C2F 00A243D0): the candidates are the
    //   owned groups' members answering vtable[+5Ch](1Ch) or in a SzurkeNyil
    //   set; one record per candidate, kept only when b > 0 (00A28C3C COMISS).
    // Pass B (00A28D31 / 00A28D44 / 00A28D59): a groupable combatant group
    //   with no world-set member whose anchor is not a record entity is queued.
    // Pass C (records empty, 00A28E69 / 00A28E7C / 00A28EBB): a group without a
    //   combatant, or with a world-set member, gets DEFENDPOSITION; any other
    //   is queued.
    // 00A28F22 / 00A28F38 / 00A28F49: each queued group is released and claimed
    //   by brain+0Ch. Records empty: free and return.
    // The records path (00A290F5..): 00A28300 sorts the records; for each owned
    //   group, 00A2919D anchor; with none, the first enemy list-28 entity the
    //   group patrols at queues a (group, entity) pair, else the nearest record
    //   entity (x/z, strict <, seed 1.0e10) is the anchor. The anchor's record
    //   loses the group's 00A2C530 resource; a group without a combatant gets
    //   DEFENDPOSITION, any other PATROLTO to the anchor (00A2C310), and the
    //   group joins the anchor's list (00A27A00). Each pair is released,
    //   claimed by brain+0Ch and ordered through 00A1A720. The merge pass
    //   (00A29860-00A29BE7) runs per list, then the spawn tail
    //   (00A29B8E-00A29E2A) quick-spawns "[defend]<id>" (a record here).
    // LABELLED: the 00A28300 comparator (00A25750 / 00A256B0) was not read,
    // so the records keep their build order; the maps keyed by pointer are
    // ordered by unit index.
    bool defend_think_00a28a60(Planner& p) {
        const int side = current_team;
        Brain* brain = (current_party >= 0 && current_party < bsp::kAiGroupPartySlotCount)
            ? brains[static_cast<std::size_t>(current_party)].get() : nullptr;
        if (brain == nullptr) return false;
        if constexpr (!kAiDefendRecordsPathBound) {
            // The records path is not bound: any candidate runs the stand-in.
            for (Group* g : p.owned) {
                if (g == nullptr || g->destroyed) continue;
                bool candidate = group_has_member_in_world_set(g, brain->world_set);
                for (const std::size_t unit : g->members) {
                    if (!is_squadron(unit) && units.unit_is_kind_of(unit, 0x1C)) candidate = true;
                }
                if (candidate) {
                    ++defend_record_fallbacks;
                    record("AiPlanners::defend_records_path_00a28b00", 0x00a28b00u);
                    // BSP_CAPTURE_DIAG=1: the records the bound path would keep.
                    if (capture_diag_enabled() && diag_capture_lines < 60) {
                        ++diag_capture_lines;
                        std::size_t owned_groups = 0, combatant_groups = 0;
                        for (Group* og : p.owned) {
                            if (og == nullptr || og->destroyed) continue;
                            ++owned_groups;
                            if (group_has_groupable_combatant(og)) ++combatant_groups;
                        }
                        log.notef("  defend diag off-plan t=%.2f party=%d owned=%zu combatant=%zu",
                            static_cast<double>(clock_seconds), side, owned_groups, combatant_groups);
                        for (Group* og : p.owned) {
                            if (og == nullptr || og->destroyed) continue;
                            for (const std::size_t unit : og->members) {
                                if (is_squadron(unit) || !units.unit_is_kind_of(unit, 0x1C)) continue;
                                const DefendRecord r = defend_score_00a243d0(unit, side);
                                log.notef("    candidate %s group_leader=%s members=%zu command=%d "
                                    "T=%.3f a=%.3f b=%.3f c=%.3f req=%.1f kept=%d",
                                    unit_name(unit).c_str(),
                                    og->members.empty() ? "-" : unit_name(proxy(og->members.front())).c_str(),
                                    og->members.size(), static_cast<int>(og->command.type),
                                    static_cast<double>(r.threat), static_cast<double>(r.own_value),
                                    static_cast<double>(r.enemy_value), static_cast<double>(r.excluded),
                                    static_cast<double>(r.requirement), 0.0f < r.enemy_value ? 1 : 0);
                            }
                        }
                    }
                    return false;
                }
            }
        }
        ++defend_thinks;
        // Pass A.
        std::vector<std::size_t> candidates;
        std::vector<DefendRecord> records;
        for (Group* g : p.owned) {
            if (g == nullptr || g->destroyed) continue;
            for (const std::size_t unit : g->members) {
                if (is_squadron(unit) || !units.unit_is_kind_of(unit, 0x1C)) continue;
                candidates.push_back(unit);
                DefendRecord r = defend_score_00a243d0(unit, side);
                if (0.0f < r.enemy_value) records.push_back(r);
            }
        }
        auto is_record_entity = [&records](std::size_t e) {
            for (const DefendRecord& r : records) if (r.entity == e) return true;
            return false;
        };
        // Pass B and, with no record, pass C.
        std::vector<Group*> release;
        const std::vector<Group*> owned = p.owned;
        for (Group* g : owned) {
            if (g == nullptr || g->destroyed) continue;
            if (group_has_groupable_combatant(g) &&
                !group_has_member_in_world_set(g, brain->world_set)) {
                const std::size_t anchor = defend_anchor_00a1cf10(g, candidates);
                if (anchor != kCaptureNone && !is_record_entity(anchor)) release.push_back(g);
            }
        }
        if (records.empty()) {
            for (Group* g : owned) {
                if (g == nullptr || g->destroyed) continue;
                if (!group_has_groupable_combatant(g) ||
                    group_has_member_in_world_set(g, brain->world_set)) {
                    defend_issue_position_00a2be20(g, side);
                } else {
                    release.push_back(g);
                }
            }
        }
        for (Group* g : release) capture_hand_off(p, g, brain->planners[3]);
        if (records.empty()) {
            done("AiPlanners::defend_think_00a28a60", 0x00a28a60u);
            return true;
        }
        ++defend_record_thinks;
        defend_records_seen += records.size();
        if (capture_diag_enabled() && diag_capture_lines < 60) {
            ++diag_capture_lines;
            for (const DefendRecord& r : records) {
                log.notef("  defend diag t=%.2f party=%d record %s T=%.3f a=%.3f b=%.3f c=%.3f req=%.1f",
                    static_cast<double>(clock_seconds), side, unit_name(r.entity).c_str(),
                    static_cast<double>(r.threat), static_cast<double>(r.own_value),
                    static_cast<double>(r.enemy_value), static_cast<double>(r.excluded),
                    static_cast<double>(r.requirement));
            }
        }
        // The records path.
        std::vector<std::pair<Group*, std::size_t>> pairs;
        std::map<std::size_t, std::vector<Group*>> lists;
        const std::vector<Group*> still = p.owned;
        for (Group* g : still) {
            if (g == nullptr || g->destroyed) continue;
            std::size_t anchor = defend_anchor_00a1cf10(g, candidates);
            if (anchor == kCaptureNone) {
                bool paired = false;
                for (std::size_t i = 0; i < units.world_list_size(28) && !paired; ++i) {
                    const std::size_t e = units.world_list_entry(28, i);
                    if (e >= units.count() || units.unit_side_0054(e) == side) continue;
                    if (group_patrols_at_00a2c230(g, e)) {
                        pairs.push_back({g, e});
                        paired = true;
                    }
                }
                if (paired) continue;
                float leader[3] = {0.0f, 0.0f, 0.0f};
                tick_leader_point(g, leader);
                float best = 1.0e10f;   // 00CE4970
                for (const DefendRecord& r : records) {
                    float ex = 0.0f, ey = 0.0f, ez = 0.0f;
                    units.unit_position_00fc(r.entity, ex, ey, ez);
                    const float dx = ex - leader[0];
                    const float dz = ez - leader[2];
                    const float d2 = dz * dz + dx * dx;
                    if (d2 < best) {
                        best = d2;
                        anchor = r.entity;
                    }
                }
                if (anchor == kCaptureNone) continue;
            }
            for (DefendRecord& r : records) {
                if (r.entity == anchor) r.remaining -= kCaptureGroupResource;   // 00A2C530
            }
            if (!group_has_groupable_combatant(g)) {
                defend_issue_position_00a2be20(g, side);
            } else {
                defend_issue_patrol_00a2c310(g, anchor);
            }
            lists[anchor].push_back(g);
        }
        for (const auto& pr : pairs) {
            capture_hand_off(p, pr.first, brain->planners[3]);
            order_capture_group_00a1a720(pr.first, pr.second);
            ++defend_capture_pairs;
        }
        // 00A29860-00A29BE7, the merge pass.
        for (auto& entry : lists) {
            std::vector<Group*>& list = entry.second;
            if (list.size() <= 1u) continue;
            float defended[3] = {0.0f, 0.0f, 0.0f};
            units.unit_position_00fc(entry.first, defended[0], defended[1], defended[2]);
            for (std::size_t i = 0; i < list.size(); ++i) {
                float a[3] = {0.0f, 0.0f, 0.0f};
                tick_leader_point(list[i], a);
                if (!bsp::ai_tail_defend_group_near_target(a, defended, kDefendMergeTargetDist)) continue;
                for (std::size_t j = i + 1; j < list.size();) {
                    float b[3] = {0.0f, 0.0f, 0.0f};
                    tick_leader_point(list[j], b);
                    if (bsp::ai_tail_defend_groups_mergeable(a, b, kDefendMergeGroupsDist)) {
                        merge_group(list[i], list[j]);   // 00A2DB80
                        list.erase(list.begin() + static_cast<std::ptrdiff_t>(j));   // 00A1D1D0
                        ++defend_merges;
                    } else {
                        ++j;
                    }
                }
            }
        }
        // 00A29B8E-00A29E2A: the first record with a positive remainder gets a
        // "[defend]" quick-spawn when the budget allows. A record here.
        for (const DefendRecord& r : records) {
            if (0.0f < r.remaining) {
                ++defend_spawn_arms;
                record("AiPlanners::defend_spawn_tail_00a29b8e", 0x00a29b8eu);
                break;
            }
        }
        done("AiPlanners::defend_think_00a28a60", 0x00a28a60u);
        return true;
    }
    // 00A2BE20: nothing when the command already answers IsType(11).
    void defend_issue_position_00a2be20(Group* g, int side) {
        if (bsp::ai_command_is_type(g->command.type, bsp::AiCommandType::DefendPosition)) return;
        if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
        bsp::AiCommandObject c;
        c.type = bsp::AiCommandType::DefendPosition;   // vtable 00D22A38
        c.owner_group = g;
        g->command = c;
        ++defend_positions;
        if (capture_diag_enabled() && diag_capture_lines < 60) {
            ++diag_capture_lines;
            std::string names;
            for (const std::size_t unit : g->members) {
                if (!names.empty()) names += ", ";
                names += unit_name(proxy(unit));
            }
            log.notef("  defend diag t=%.2f party=%d defendposition members=%zu [%s]",
                static_cast<double>(clock_seconds), side, g->members.size(), names.c_str());
        }
    }
    // 00A2C310 with the anchor's +FCh: nothing when 00A2C230 already holds.
    void defend_issue_patrol_00a2c310(Group* g, std::size_t anchor) {
        if (group_patrols_at_00a2c230(g, anchor)) return;
        float ex = 0.0f, ey = 0.0f, ez = 0.0f;
        units.unit_position_00fc(anchor, ex, ey, ez);
        if (bsp::ai_command_install_deletes_previous(true)) ++summary.commands_replaced;
        bsp::AiCommandObject c;
        c.type = bsp::AiCommandType::PatrolTo;   // vtable 00D22B3C
        c.owner_group = g;
        c.target_position[0] = ex;
        c.target_position[1] = ey;
        c.target_position[2] = ez;
        g->command = c;
        ++defend_patrols;
    }
    unsigned long long defend_record_thinks{0};
    unsigned long long defend_records_seen{0};
    unsigned long long defend_patrols{0};
    unsigned long long defend_capture_pairs{0};
    unsigned long long defend_merges{0};
    unsigned long long defend_spawn_arms{0};
    unsigned long long defend_thinks{0};
    unsigned long long defend_record_fallbacks{0};
    unsigned long long defend_positions{0};
    // 00A11FF0, SELLING's vt+0Ch, body 00A11FF0-00A1242D, read in full.
    // 00A1201D 00A2C660 (an air member?):
    //  no  - 00A12109..: the nearest (3-D squared, strict <, seed 1.0e10 at
    //        00CE4970) list-28 entity whose +54h is the group's +5638h team.
    //        With one: R = (float)((int)entity+7A0h * 0.8) (00A12226 FILD, FMUL
    //        double 00CE3D40); d = 009FFC10(entity+FCh - leader); 00A122EE FCOMIP
    //        / JBE: d <= R takes 00A10C20, the group's own leader point, else the
    //        entity's +FCh; 00A12342 00A02020(first member, point), then 00A10DC0
    //        and 00A11070 (not read). 00A123A0..00A123F8: each member with
    //        +308h == 0.0 that passes 005F98F0 routes session message 51h
    //        through 0077C2A0 (a record here).
    //  yes - 00A12087 / 00A12090: each member answering vtable[+5Ch](18h) with
    //        +361h and +3B0h clear gets 0077D600(00E08F98 `returntobase`, a zero
    //        position descriptor, 1) at 00A120EB.
    // LABELLED: CaptureRange is kCaptureRangeStandIn (the authored value has no
    // reader here); +361h and +3B0h are read as clear.
    bsp::AiCommandTickResult selling_tick_00a11ff0(const bsp::AiCommandObject& cmd) {
        bsp::AiCommandTickResult result;
        Group* g = group_at(cmd.owner_group);
        if (g == nullptr) return result;
        ++selling_ticks;
        if (!group_has_air(g)) {
            float leader[3] = {0.0f, 0.0f, 0.0f};
            tick_leader_point(g, leader);
            std::size_t best = kCaptureNone;
            float best_d2 = 1.0e10f;
            for (std::size_t i = 0; i < units.world_list_size(28); ++i) {
                const std::size_t e = units.world_list_entry(28, i);
                if (e >= units.count() || units.unit_side_0054(e) != g->team) continue;
                float x = 0.0f, y = 0.0f, z = 0.0f;
                units.unit_position_00fc(e, x, y, z);
                const float dx = leader[0] - x, dy = leader[1] - y, dz = leader[2] - z;
                const float d2 = dz * dz + dx * dx + dy * dy;
                if (d2 < best_d2) {
                    best_d2 = d2;
                    best = e;
                }
            }
            if (best == kCaptureNone) return result;
            float cb[3] = {0.0f, 0.0f, 0.0f};
            units.unit_position_00fc(best, cb[0], cb[1], cb[2]);
            const float range = kCaptureAccessorsBound
                ? units.command_building_capture_range_07a0(best) : kCaptureRangeStandIn;
            const float r = static_cast<float>(
                static_cast<double>(static_cast<int>(range)) * 0.8);
            const float v[3] = {cb[0] - leader[0], cb[1] - leader[1], cb[2] - leader[2]};
            const float d = bsp::ai_tail_horizontal_length(v);
            const float* point = d <= r ? leader : cb;
            if (d <= r) ++selling_holds; else ++selling_approaches;
            const bsp::AiCommandTickResult lead =
                bsp::ai_command_order_leader_00a02020(*this, g, point);
            result.orders_issued += lead.orders_issued;
            const bsp::AiCommandTickResult follow = bsp::ai_command_follower_pass_00a10dc0(*this, g);
            result.orders_issued += follow.orders_issued;
            result.formation_requests += follow.formation_requests;
            result.followers_walked += follow.followers_walked;
            record("AiCommand::selling_sell_message_00a123f8", 0x00a123f8u);
        } else {
            for (const std::size_t unit : g->members) {
                if (!is_squadron(unit)) continue;
                const std::vector<std::size_t>* planes = squadron_member_units(unit);
                if (planes == nullptr) continue;
                bsp::SceneCommandTarget target;
                target.kind = 0;
                target.position_valid = 0;
                target.object_id = 0;
                target.object = nullptr;
                target.position[0] = target.position[1] = target.position[2] = 0.0f;
                target.trailing = 0.0f;
                std::size_t placed = 0;
                for (const std::size_t plane : *planes) {
                    if (units.issue_script_command(plane, 0x00E08F98u, target, bsp::kAiSceneCommandFlags,
                                                   "ai_selling_tick", unit_name(plane)) != nullptr) {
                        ++placed;
                    }
                }
                if (placed != 0) {
                    ++selling_returns;
                    ++result.orders_issued;
                }
            }
        }
        done("AiCommand::selling_tick", 0x00a11ff0u);
        return result;
    }
    unsigned long long selling_ticks{0};
    unsigned long long selling_holds{0};
    unsigned long long selling_approaches{0};
    unsigned long long selling_returns{0};
    unsigned long long rtb_exclusion_squadron_calls{0};   // packet cc9_squadron_rtb_exclusion
    unsigned long long rtb_exclusion_bomber_calls{0};
    unsigned long long rtb_exclusion_spent_calls{0};
    unsigned long long rtb_exclusion_stand_in_true{0};   // packet cc9_squadron_rtb_exclusion_bind
    unsigned long long rtb_exclusion_issues{0};
    unsigned long long rtb_exclusion_excluded{0};
    unsigned long long sell_thinks{0};
    unsigned long long sell_splits{0};
    unsigned long long sell_orders{0};
    unsigned long long capture_path_thinks{0};
    unsigned long long patrol_ticks{0};
    unsigned long long patrol_close_passes{0};
    unsigned long long capture_assignments{0};
    unsigned long long capture_orders_attack{0};
    unsigned long long capture_orders_defend{0};
    unsigned long long capture_orders_patrol{0};
    unsigned long long capture_orders_kept{0};
    unsigned long long capture_merges{0};
    unsigned long long capture_spawn_due{0};
    int diag_capture_lines{0};
    void* first_group_of_party(int party_slot) override {
        if constexpr (kAiPartyGateUnforcedBound) {
            if (party_slot < 0 || party_slot >= bsp::kAiGroupPartySlotCount) return nullptr;
            return open(by_party[static_cast<std::size_t>(party_slot)]);
        } else {
            if (party_slot < 0 || party_slot >= static_cast<int>(by_team.size())) return nullptr;
            return open(by_team[static_cast<std::size_t>(party_slot)]);
        }
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
        if (p == nullptr || g == nullptr) return;
        // 00A22762, 00A1C8B0: the planner's own list already holds the group,
        // and the claim returns without touching +2Ch. Otherwise 00A227A9 sets
        // it after the push. The image does not test group+5654h first; this
        // host still refuses a group another planner holds (below), so that
        // case is counted and its ownership transfer is not modelled.
        const bool owned = std::find(p->owned.begin(), p->owned.end(), g) != p->owned.end();
        if (!owned) {
            ++replan_sets;
            if (kAiPartyReplanFlagBound) p->replan_002c = true;
            if (g->claimed_by != nullptr && g->claimed_by != p) ++replan_foreign_claims;
        }
        if (g->claimed_by != nullptr) return;
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
    float candidate_base_weight(void* own_group, void* candidate) override {
        // 00A0F970(ECX = the planner's group, EDX = the candidate), called at
        // 00A1CC65 with (0, -1.0 [00D7A260], 0, 0, 1.0). The census runs in both
        // states; the switch decides which value the planner multiplies.
        Group* g = group_at(candidate);
        const float value = group_target_value_00a0f970(group_at(own_group), g);
        if (kPlannerGroupTargetValueBound) {
            done("AiPlanners::candidate_base_weight", 0x00a0f970u);
            return value;
        }
        // The stand-in: the candidate's population.
        record("AiPlanners::candidate_base_weight", 0x00a0f970u);
        return g != nullptr ? static_cast<float>(g->members.size()) : 0.0f;
    }

    // ---- Packet cc9_planner_group_target_value: 00A0F970 and its callees ----
    // docs/SHIP_AI_OPEN_ITEMS.md section 7. Every name is a hypothesis.

    // The 32-byte record 00A04560 builds through 00A00020, projected onto what
    // 00A0C330 and 00A0C3C0 read.
    struct GroupValueRecord {
        std::size_t member{0};   // the group member (a unit or a squadron)
        std::size_t unit{0};     // proxy(member): the plane leader for a squadron
        float x{0.0f};           // +4h, the pose X (00A04582, entity+FCh)
        float z{0.0f};           // +8h, the pose Z (00A0459D, entity+104h)
        float fix_weight{1.0f};  // +14h, 009FDF30's class weight x 00A04240
        float spread{1.0f};      // +18h, the ValueRandomMul interpolation
        bool third_party{false}; // +1Ch, 00A04568 CMP [entity+54h],2 / SETGE
        // record+0h's class answers vtable[+18h](6) (a ship class, 00A0C3F5) or
        // (0Fh) (a plane class, 00A0C51D); 0 when neither.
        int class_query{0};
        float class_max_speed{0.0f};  // class+500h (ship) or class+188h (plane)
    };

    // ValueRandomMul [1] and [2], tuning +50h/+54h. This installation's
    // highlvlaiglobals.lua (2024-07-13) authors {0.95, 1.05} in the three
    // IslandCapture tables (lines 37, 225, 411) and {0.85, 1.1} in the other
    // four. The per-entity spread is 00419010(0, [1], 78.0 [00D21D04], [2],
    // entity pointer mod 79) (00A0458A-00A045E5). LABELLED stand-in: this
    // process has no stable entity addresses, so the argument is the midpoint
    // 39, which gives every record the same spread. A constant spread scales
    // every candidate's value by the same square, so it moves no pick; the
    // per-entity +-5% the image draws is what is lost.
    float group_value_spread() const {
        const bool island = tuning.mode == bsp::AiTuningMode::IslandCaptureRookie ||
                            tuning.mode == bsp::AiTuningMode::IslandCaptureRegular ||
                            tuning.mode == bsp::AiTuningMode::IslandCaptureVeteran;
        const float lo = island ? 0.95f : 0.85f;
        const float hi = island ? 1.05f : 1.1f;
        return bsp::clamped_interpolate_00419010(0.0f, lo, 78.0f, hi, 39.0f);
    }

    // 00A04560 BSP_Ai_EntityRecordBuild, __fastcall(ECX = out, EDX = entity),
    // RET 4, called from 00A07E40 with 1.
    GroupValueRecord group_value_record_00a04560(std::size_t member) {
        GroupValueRecord r;
        r.member = member;
        r.unit = proxy(member);
        float y = 0.0f;
        units.unit_position_00fc(r.unit, r.x, y, r.z);
        r.third_party = units.unit_side_0054(r.unit) >= 2;
        // 00A045EE-00A04604: 009FDF30's weight for the entity's +C4h class id,
        // times 00A04240(entity). 00A04240 multiplies the two hint-weight maps
        // at 00F8A740 (global) and 00F8A750 + party*0Ch, default 1.0
        // [00D7A24C]. Only the Lua native SetHintWeight (00A07F60 / 00A07F80)
        // fills them, and in this installation only the multiplayer
        // competitive scripts call it, so on these rows the factor is 1.0.
        r.fix_weight = unit_class_weight(member);
        r.spread = group_value_spread();
        // 00A04652 IsKindOf(5) takes [entity+538h], 00A0466E IsKindOf(18h)
        // [entity+35Ch], the squadron's plane class. LABELLED: this host asks
        // the unit's own kind for the class's vtable[+18h] answer.
        if (is_squadron(member) || units.unit_is_kind_of(r.unit, 0x0F)) {
            r.class_query = 0x0F;
            r.class_max_speed = units.plane_class_max_speed_0188(r.unit);
        } else if (units.unit_is_kind_of(r.unit, 0x06)) {
            r.class_query = 0x06;
            r.class_max_speed = units.unit_class_max_speed_0500(r.unit);
        }
        return r;
    }

    // 00A0C3C0 BSP_AiEntityRecord_PairAttackValue, __fastcall(ECX = attacker
    // record, EDX = target record, arg1, distance, debug, rnd flag), RET 10h,
    // with the distance -1.0 and the rnd flag 1 on this path.
    float group_value_pair_00a0c3c0(const GroupValueRecord& a, const GroupValueRecord& t) {
        // 00A0C330 BSP_AiEntityRecord_PairBaseValue: 00A08460(ECX = a+0h,
        // EDX = a+10h, t+0h, t+1Ch) x (a+18h x t+18h) x t+14h.
        float weight = 1.0f;
        const GameAiWeaponFacts& facts = game_ai_weapon_facts();
        const GameAiWeaponFacts::Unit* attacker_row = facts.row(a.unit);
        const GameAiWeaponFacts::Unit* target_row = facts.row(t.unit);
        if (ai_weight_model_enabled() && attacker_row != nullptr && target_row != nullptr &&
            attacker_row->inputs_complete && target_row->inputs_complete) {
            bsp::AiTargetWeightKey key;
            key.attacker = handle(a.unit);
            // a+10h: 0 for a ship record (00A04650); the model reads it only
            // as a memo key, and this binding keeps no memo.
            key.attacker_class = kAiPlaneAttackerWeightBound ? 0   // record+10h
                                 : units.unit_class_id(a.unit);
            key.target = handle(t.unit);
            key.target_is_neutral = t.third_party ? 1 : 0;
            const bsp::AiModeTuning record = mode_tuning_record();
            AiWeightModelBinding model(facts, record, tuning,
                [this](std::size_t unit) { return accuracy_target_group(unit); },
                [this](std::size_t unit, int code) { return units.unit_is_kind_of(unit, code); });
            weight = bsp::ai_target_weight_00a08460(model, key);
            ai_damage_terms_census().group_weight_sum += weight;
            ++group_value_census.model_pairs;
        } else {
            // LABELLED: 00A08460 with none of its inputs, the identity the
            // close-attack weight 00A0F810 stands in with too.
            ++group_value_census.stand_in_pairs;
        }
        // 00A0C34E: an attacker record with +1Ch set whose class answers
        // vtable[+18h](1Ch) scores 0. That query is unread here (the same slot
        // 00A0F859 asks), so the arm never fires; it can only remove value.
        float value = a.spread * t.spread * weight * t.fix_weight;
        // 00A0C3F5 / 00A0C51D: the distance multiplier, ship or plane class.
        if (a.class_query == 0x06 || a.class_query == 0x0F) {
            const bool ship = a.class_query == 0x06;
            // 00A0C415-00A0C439: the records' +0Ch bytes are both 1 (00A0468F),
            // so the distance is 00414C60 over the pose delta, 0 under 1e-10.
            const float dx = a.x - t.x;
            const float dz = a.z - t.z;
            const float d2 = dx * dx + dz * dz;
            const float d = static_cast<double>(d2) > 1.0e-10
                                ? static_cast<float>(std::sqrt(static_cast<double>(d2)))
                                : 0.0f;
            // 00A0C457 / 00A0C57F: less the arrive distance, +D8h or +ECh.
            const float arrive = ship ? kGroupValueShipArriveDist : group_value_plane_arrive();
            const float beyond = static_cast<float>(static_cast<double>(d) - arrive);
            if (beyond > 0.0f) {
                // 00A0C477 FDIV [class+500h] / 00A0C59F FDIV [class+188h]. A
                // zero speed divides to +inf, which the clamp takes to [2].
                const float time = a.class_max_speed != 0.0f
                                       ? beyond / a.class_max_speed
                                       : std::numeric_limits<float>::infinity();
                // 00419010(x0 = TravelTime[1], y0 = WeightMul[1], x1 =
                // TravelTime[2], y1 = WeightMul[2], x = time), the pushes at
                // 00A0C4AE-00A0C4DE (+DCh, +E8h, +E0h, +E4h) and their plane
                // twins (+F0h, +FCh, +F4h, +F8h).
                const float mul = ship
                    ? bsp::clamped_interpolate_00419010(60.0f, 1.0f, 300.0f, 0.1f, time)
                    : bsp::clamped_interpolate_00419010(0.0f, 1.0f,
                                                        group_value_plane_travel_far(), 1.0f,
                                                        time);
                value *= mul;
                if (mul < 1.0f) ++group_value_census.distance_cut_pairs;
            }
        }
        return value;
    }

    // ShipDistWeight_AriveDist (+D8h), 3000 in all seven mode tables of this
    // installation's highlvlaiglobals.lua (lines 46, 234, ...); TravelTime
    // {60, 300} and WeightMul {1.0, 0.1} likewise uniform.
    static constexpr float kGroupValueShipArriveDist = 3000.0f;
    // PlaneDistWeight_AriveDist (+ECh) and TravelTime[2] (+F4h): 1500 and 60 in
    // the three IslandCapture tables, 4000 and 90 in the other four. WeightMul
    // is {1.0, 1.0} everywhere, so a plane attacker's multiplier is 1.0 at any
    // range.
    float group_value_plane_arrive() const {
        return group_value_island_mode() ? 1500.0f : 4000.0f;
    }
    float group_value_plane_travel_far() const {
        return group_value_island_mode() ? 60.0f : 90.0f;
    }
    bool group_value_island_mode() const {
        return tuning.mode == bsp::AiTuningMode::IslandCaptureRookie ||
               tuning.mode == bsp::AiTuningMode::IslandCaptureRegular ||
               tuning.mode == bsp::AiTuningMode::IslandCaptureVeteran;
    }

    // 00A0F970 BSP_AiGroup_TargetValueAgainstGroup, __fastcall(ECX = attacker
    // group, EDX = target group, arg1, distance, debug, flag, scale), RET 14h.
    float group_target_value_00a0f970(Group* own, Group* target) {
        ++group_value_census.calls;
        // 00A0F98B / 00A0F99A: either group's +5644h count at 0 answers 0.
        if (own == nullptr || target == nullptr || own->members.empty() ||
            target->members.empty()) {
            ++group_value_census.empty_groups;
            return 0.0f;
        }
        // 00A07E40 twice: one record per member of the +563Ch list, in order.
        std::vector<GroupValueRecord> attackers;
        std::vector<GroupValueRecord> targets;
        for (const std::size_t m : own->members) attackers.push_back(group_value_record_00a04560(m));
        for (const std::size_t m : target->members) targets.push_back(group_value_record_00a04560(m));
        float attack_sum = 0.0f;
        float maxes_sum = 0.0f;
        const float result = compose_attack_value_00a0c650(attackers, targets, attack_sum, maxes_sum);
        if (result == 0.0f) ++group_value_census.zero_results;
        if (group_value_trace_enabled() &&
            (is_squadron(own->members.front()) ||
             units.unit_is_kind_of(proxy(own->members.front()), 0x0F))) {
            log.notef("  ai group value trace t=%.2f own_lead=%s own_team=%d own_members=%zu "
                      "target_lead=%s target_team=%d target_members=%zu value=%.4f sum=%.4f "
                      "maxes=%.4f leader_dist=%.0f",
                      static_cast<double>(clock_seconds), unit_name(own->members.front()).c_str(),
                      own->team, own->members.size(), unit_name(target->members.front()).c_str(),
                      target->team, target->members.size(), static_cast<double>(result),
                      static_cast<double>(attack_sum), static_cast<double>(maxes_sum),
                      std::sqrt(static_cast<double>(squared_planar_distance(own, target))));
        }
        const float population = static_cast<float>(target->members.size());
        if (group_value_census.samples < kGroupValueSampleLimit) {
            ++group_value_census.samples;
            // The planner's range factor for the same pair, as 00A1CB80 forms
            // it, so a census line predicts the pick without the ON run.
            const float leader_d2 = squared_planar_distance(own, target);
            const float range = range_interpolation(
                tuning.at(bsp::kAiPlannerTuningRangeNear),
                tuning.at(bsp::kAiPlannerTuningRangeFar),
                bsp::ai_target_scoring_distance(leader_d2, near_radius_squared()));
            log.notef("  ai group target value own_lead=%s own_members=%zu target_lead=%s "
                      "target_members=%zu value=%.4f population=%.0f sum=%.4f maxes=%.4f "
                      "leader_dist=%.0f range=%.3f",
                      unit_name(own->members.front()).c_str(), own->members.size(),
                      unit_name(target->members.front()).c_str(), target->members.size(),
                      static_cast<double>(result), static_cast<double>(population),
                      static_cast<double>(attack_sum), static_cast<double>(maxes_sum),
                      std::sqrt(static_cast<double>(leader_d2)), static_cast<double>(range));
        }
        return result;
    }
    // 00A0C650 BSP_AiGroup_ComposeAttackValue with (0, -1.0 [00D7A260], 0, 0,
    // 1.0), the arguments both 00A0F970 (00A1CC61) and 00A250A0 pass.
    float compose_attack_value_00a0c650(const std::vector<GroupValueRecord>& attackers,
                                        const std::vector<GroupValueRecord>& targets,
                                        float& attack_sum, float& maxes_sum) {
        // The pair loop sums every pair (attack sum) and keeps each attacker's
        // best (00A0C8xx); the maxes vector starts at 0.0 (the 004A8F10 resize
        // with FLDZ).
        attack_sum = 0.0f;
        maxes_sum = 0.0f;
        for (const GroupValueRecord& a : attackers) {
            float best = 0.0f;
            for (const GroupValueRecord& t : targets) {
                const float v = group_value_pair_00a0c3c0(a, t);
                attack_sum = v + attack_sum;
                if (best < v) best = v;
            }
            // 00A0CC82-00A0CC9B: the maxes are summed in attacker order.
            maxes_sum = best + maxes_sum;
        }
        // 00A0CDC6-00A0CDE3: (+218h AttackSumMul x sum + maxes) / +214h
        // ReferenceWeight, 0.33 and 5.0 in all seven tables (lines 139-140).
        const float base = (kGroupValueAttackSumMul * attack_sum + maxes_sum) /
                           kGroupValueReferenceWeight;
        // 00A0CE26-00A0CE6B: the speed bonus min(+22Ch x 00A07C10(group), +228h x
        // base). Both are authored 0 (lines 144-145), so it is 0 for any finite
        // group speed; 00A07C10 is not called here.
        const float speed_bonus = 0.0f;
        // 00A0CE71: the three penalties are zeroed when arg1 is 0, which it is
        // from 00A1CC61 and from 00A250A0. Then 00A0CEB0-ish clamps a negative
        // result to 0.
        float result = speed_bonus + base;
        if (result < 0.0f) result = 0.0f;
        return result;
    }
    static constexpr float kGroupValueAttackSumMul = 0.33f;
    static constexpr float kGroupValueReferenceWeight = 5.0f;
    static constexpr unsigned long long kGroupValueSampleLimit = 400;
    struct GroupValueCensus {
        unsigned long long calls{0};
        unsigned long long empty_groups{0};
        unsigned long long model_pairs{0};
        unsigned long long stand_in_pairs{0};
        unsigned long long distance_cut_pairs{0};
        unsigned long long zero_results{0};
        unsigned long long samples{0};
    };
    GroupValueCensus group_value_census{};
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
        if (kPlannerRangeInterpBound) {
            // Packet cc9_planner_kate_targeting: 00A1CD53-00A1CD95 pushes
            // 00419010(x0 = +1D0h FreeAttack_NearDist, y0 = 1.0 (FLD1),
            // x1 = +1D4h FreeAttack_FarDist, y1 = 0.1 [00D7A2F0], x = the planar
            // range). The two tuning fields are DISTANCES, not the two values.
            // docs/PLANNER_KATE_TARGETING.md 2.
            if (far_value == near_value) return 1.0f;
            const float t = (distance - near_value) / (far_value - near_value);
            const float v = 1.0f + (0.1f - 1.0f) * t;
            if (v > 1.0f) return 1.0f;
            if (v < 0.1f) return 0.1f;
            return v;
        }
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
        if constexpr (kAiPlannerSlotKindsBound) {
            // AiPlannerKind is declared in brain-slot order (src/ai_planners.cpp's
            // table carries the offsets 00h..1Ch).
            if (slot >= 0 && slot < bsp::kAiPlannerKindCount)
                return static_cast<bsp::AiPlannerKind>(slot);
            return bsp::AiPlannerKind::Competitive;
        } else {
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
    void seed_squadron_for_unit(std::size_t unit);
    // Packet cc9_generated_squadron_brain_membership.
    void admit_generated_squadrons();
    std::size_t squadron_units_built{0};
    unsigned long long generated_squadrons{0};
    unsigned long long squadron_index_shifts{0};
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

    // Packet cc8_ship_command. `00A2C790`'s `vtable[+114h]` is now read, and it
    // is not an order arm. The member pass calls it three times per member
    // (00A2C7F5, 00A2C805, 00A2C81A) and the answer feeds
    // `0071EB60 BSP_EntityCommand_ActiveTargetDescriptor` and
    // `0071BE40` current-command, whose pair goes to the group command's own
    // `vtable[+24h]` at 00A2C839: the pass READS every member's director and
    // reports it upward. `00A2CBD0`'s callee list carries neither `00A02020`
    // nor `0077D600`, so the attack order itself reaches no member either.
    //
    // A member is reached only by the command's `vt+0Ch` tick, through
    // `00A02020`, the one bridge (docs/AI_COMMAND_TICK.md). There the split is
    // by kind, not by token: `00A10DC0`'s follower pass hands a SHIP follower
    // `0077C8D0 BSP_Entity_RequestJoinFormation` and pushes no command at all
    // (00A10E3E, 00A10E61), and only the leader and squadron followers reach
    // `00A02020`, whose one descriptor is `00E08F68 moveto`.
    //
    // So a SHIP member must receive no scene command here, leader or follower:
    // a follower's only contact is `0077C8D0`, and a ship leader's order is the
    // `moveto` the class tick already issues through `00A02020`
    // (`ai_command_tick`'s `tick_orders`).
    //
    // Issuing an `artillery` on top is what ended the USN04 Yorktown's
    // `moveonpath`, and the measured mechanism is blunter than a stage raise: a
    // scene command REPLACES the whole queue. The trace row is
    //   349.29s issue scene/attackmove  slots 3 -> 1  moveonpath -> attackmove
    // - three filled slots become one and the new command is the head, so the
    // path order is discarded outright rather than terminated. That is the same
    // replace semantics 0077D600 has in the image, which is exactly why the
    // image never sends a scene command to a ship follower.
    // docs/SHIP_COMMAND_LIFETIME.md.
    //
    // The plane tokens stay: a squadron follower IS reached by `00A02020`, and
    // the 26-row registry has no single "attack", so the member's own weapon
    // kind is still the labelled substitution for the token
    // (docs/ENTITY_LUA_ORDER_PATH.md) - a stand-in, and named as one.
    const std::string target_name = t->members.empty() ? std::string()
        : unit_name(t->members.front());
    const bool target_is_air = !t->members.empty() &&
        units.unit_is_kind_of(t->members.front(), bsp::kUnitGunneryKindPlaneBase);
    for (const std::size_t member : g->members) {
        const bool member_is_air = units.unit_is_kind_of(member,
            bsp::kUnitGunneryKindPlaneBase);
        if (units.unit_is_kind_of(member, bsp::kUnitGunneryKindShipBase)) {
            ++summary.ship_members_not_ordered;
            continue;  // 00A10E3E / 00A2CBD0: no scene command reaches a ship
        }
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
        // ON: the slot 009FFD20 files the unit under (cc9_ai_party_gate).
        const int slot = side >= 0 ? host.unit_slot_009ffd20(i) : side;
        if (slot >= 0 && slot < bsp::kAiGroupPartySlotCount) {
            host.party_record[static_cast<std::size_t>(slot)] = true;
        }
    }
    host.summary.game_mode = host.game_mode();
    host.log.notef("ai party gate bound=%d published=%d local_team=%d slot_teams=%d,%d,%d,%d,%d,%d,%d,%d "
        "game_mode=%d (009FFE50 unforced single-player arm, 009FFD20, 009FFD60; packet "
        "cc9_ai_party_gate)", kAiPartyGateUnforcedBound ? 1 : 0,
        g_scene_slot_parties_published ? 1 : 0, host.slot_team(0),
        g_scene_slot_parties[0], g_scene_slot_parties[1], g_scene_slot_parties[2],
        g_scene_slot_parties[3], g_scene_slot_parties[4], g_scene_slot_parties[5],
        g_scene_slot_parties[6], g_scene_slot_parties[7], host.game_mode());
    {
        // Packet cc9_ai_owner_player_slot: the units whose +180h is not 9.
        std::size_t authored = 0, ai_control = 0, named_slot = 0;
        for (std::size_t i = 0; i < host.units.count(); ++i) {
            const int owner = host.unit_owner_player_0180(i);
            if (owner == 9) continue;
            ++authored;
            if (owner == 8) ++ai_control; else if (owner >= 0 && owner < 8) ++named_slot;
            host.log.notef("  ai owner player unit=%s owner=%d slot=%d side=%d",
                host.units.unit_row(i) != nullptr ? host.units.unit_row(i)->name.c_str() : "-",
                owner, host.unit_slot_009ffd20(i), host.units.unit_side_0054(i));
        }
        host.log.notef("ai owner player bound=%d published=%zu conflicts=%zu units=%zu "
            "ai_control=%zu named_slot=%zu (009FFD20 +180h, 0077F1F9; packet "
            "cc9_ai_owner_player_slot)", kAiOwnerPlayerSlotBound ? 1 : 0,
            g_scene_owner_players.size(), g_scene_owner_player_conflicts.size(), authored,
            ai_control, named_slot);
    }
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
        row.ai_enabled = host.party_admitted(party);
    }
    host.record("AiController::create", 0x00a32350u);
    host.record("AiController::construct", 0x00a31730u);
}

// One unit's squadron seed, the body build_squadrons ran per unit. Packet
// cc9_generated_squadron_brain_membership moved it here unchanged.
void GameAiCoordinatorHost::Impl::seed_squadron_for_unit(std::size_t unit) {
    // 009FE0F0's air test: the plane base 0Fh. A squadron's own members are
    // planes, and nothing else in the scene produces one.
    if (!units.unit_is_kind_of(unit, bsp::kPlaneSquadronMemberKindId)) return;
    // Packet cc8_plane_squadron_host (15563fdf9) spawns a squadron's real
    // WingCount wingmen, so a plane can now be a MEMBER of a squadron
    // rather than a squadron in its own right. Seeding one squadron per
    // member is exactly the double-order the comment on
    // unit_owned_by_squadron forbids - "keeping both in a group
    // double-orders the same aircraft" - and it showed as 15 AI squadrons
    // on a USN04 that has 5. The registry's back pointer is this process's
    // stand-in for plane+9D4h: a plane whose squadron names another unit as
    // its flight leader is a wingman and is not a seed.
    // The find_by_member_name fallback this carried is GONE: create_units
    // now calls resolve_plane_squadron_members before constructing this
    // host, so the +3D0h array is filled by the time this runs and one
    // route answers instead of two that could disagree.
    const bsp::PlaneSquadronHostRecord* owner =
        bsp::plane_squadron_registry().find_by_member_unit(unit);
    // The wing in +3D0h order, skipping slots whose plane never became a
    // unit, which is live_count()'s rule.
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
    if (wing_units.front() != unit) return;
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
    if (!attached) return;
    s.registry_backed = owner != nullptr;
    squadrons.push_back(std::move(s));
    ++summary.squadrons_built;
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
    for (std::size_t unit = 0; unit < units.count(); ++unit) {
        seed_squadron_for_unit(unit);
    }
    squadron_units_built = units.count();
    if (!squadrons.empty()) {
        log.notef("ai squadrons: %llu PlaneSquadronGen objects built over %llu member "
            "planes (004F0AD0 + 007F2C60 + 007F4580's +3D0h tail); each carries class "
            "id 18h, so 009FE080's 009FE088 arm now answers for them and its 009FE092 "
            "negation of 007EDA90 is what admits them",
            static_cast<unsigned long long>(summary.squadrons_built),
            static_cast<unsigned long long>(summary.squadron_members));
        // The +3C8h each squadron ended up with, which no line carried before.
        // It is the discriminator for whether the wing count came from the
        // authored WingCount or from 007F4735's absent-key default of 3: they
        // coincide at 3 for USN04's movieval and for every air-ops launch, and
        // differ on USN01, whose five rows author 1.
        std::map<int, std::size_t> wing_count_histogram;
        for (const Squadron& built : squadrons) {
            ++wing_count_histogram[built.entity.wing_count];
        }
        for (const std::pair<const int, std::size_t>& entry : wing_count_histogram) {
            log.notef("  ai squadron wing_count +3C8h=%d over %zu squadron(s)",
                entry.first, entry.second);
        }
        // A squadron of exactly one plane that the registry does know about is
        // the ungrouped case: USN04 ends at 7 squadrons over 15 planes because
        // one air-ops launch's three planes each seeded one. Name them so the
        // next run says which launch rather than leaving it to arithmetic.
        for (const Squadron& built : squadrons) {
            if (built.member_units.size() != 1) continue;
            const std::size_t only = built.member_units.front();
            const bsp::PlaneSquadronHostRecord* record =
                bsp::plane_squadron_registry().find_by_member_name(unit_name(only));
            if (record == nullptr) continue;
            // A one-wing squadron IS one plane, so "built one member" is only
            // wrong when the record has more than one live member to give. The
            // first version of this check lacked the test and fired on all five
            // of USN01's correctly grouped Mavs, whose WingCount is 1.
            if (record->live_count() <= 1) continue;
            log.notef("  ai squadron UNGROUPED %s: the registry knows it as a member of "
                "%s (+3D0h holds %zu slot(s), %d live), so it should not have seeded a "
                "squadron of its own",
                unit_name(only).c_str(), record->name.c_str(),
                record->member_units.size(), record->live_count());
        }
        // The other half of that question: a lone plane the registry does NOT
        // know. Printing its name separates "no record yet" from "the plane has
        // no name yet", which the silent case could not.
        for (const Squadron& built : squadrons) {
            if (built.member_units.size() != 1) continue;
            const std::size_t only = built.member_units.front();
            if (bsp::plane_squadron_registry().find_by_member_name(unit_name(only))
                != nullptr) {
                continue;
            }
            log.notef("  ai squadron LONE name=\"%s\" (no registry record names this "
                "plane at census time; an empty name here would mean the unit row is "
                "not named yet rather than that the record is missing)",
                unit_name(only).c_str());
        }
    } else {
        log.notef("ai squadrons: this mission created no unit answering IsKindOf(0Fh), "
            "so no PlaneSquadronGen is built and 009FE080 falls to its ship tail");
    }
    record("SceneUnit::create_plane_squadron_gen", 0x004f0ad0u);
    record("PlaneSquadron::construct", 0x007f2c60u);
    record("PlaneSquadron::attach_planes", 0x007f4580u);
}

void GameAiCoordinatorHost::Impl::admit_generated_squadrons() {
    const std::size_t old_units = squadron_units_built;
    const std::size_t new_units = units.count();
    if (new_units <= old_units) return;
    const std::size_t delta = new_units - old_units;
    // Keep every stored squadron candidate on its squadron: indices at or past
    // the old unit count named squadron (index - old_units) and now sit delta
    // further on. Unit indices below it are unchanged.
    auto shift = [&](std::size_t index) {
        return index >= old_units ? index + delta : index;
    };
    for (const std::unique_ptr<Group>& g : groups) {
        for (std::size_t& member : g->members) {
            if (member >= old_units) {
                member += delta;
                ++squadron_index_shifts;
            }
        }
    }
    {
        std::vector<Group*> moved(new_units + squadrons.size(), nullptr);
        for (std::size_t index = 0; index < group_of_unit.size(); ++index) {
            const std::size_t to = shift(index);
            if (to < moved.size()) moved[to] = group_of_unit[index];
        }
        group_of_unit.swap(moved);
    }
    if (last_order.size() > old_units) {
        last_order.insert(last_order.begin() + static_cast<std::ptrdiff_t>(old_units),
                          delta, LastOrder{});
    }
    if (seed_cursor >= old_units) seed_cursor += delta;
    squadron_units_built = new_units;
    // Seed the squadrons whose flight leader is new, through the load-time body.
    const std::size_t before = squadrons.size();
    for (std::size_t unit = old_units; unit < new_units; ++unit) {
        seed_squadron_for_unit(unit);
    }
    if (unit_owned_by_squadron.size() < new_units) {
        unit_owned_by_squadron.resize(new_units, false);
    }
    if (group_of_unit.size() < candidate_count()) {
        group_of_unit.resize(candidate_count(), nullptr);
    }
    for (std::size_t i = before; i < squadrons.size(); ++i) {
        ++generated_squadrons;
        const Squadron& built = squadrons[i];
        log.notef("ai squadron generated after load: leader=%s members=%zu wing_count=%d "
            "(00A2E835's live world lists; packet cc9_generated_squadron_brain_membership)",
            unit_name(built.member_units.front()).c_str(), built.member_units.size(),
            built.entity.wing_count);
    }
}

void GameAiCoordinatorHost::fixed_step(float step_seconds) {
    Impl& host = *impl_;
    if (!host.created) return;
    if constexpr (kGeneratedSquadronBrainBound) host.admit_generated_squadrons();
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
    host.log.notef("summary mission ai cautious route bound=%d builds=%llu points=%llu "
        "clears=%llu waits=%llu movetos=%llu no_slot=%llu party_presence=%.0f/%.0f "
        "(00A14DD0, packet cc9_director_moveonpath_route)", bsp::kCautiousRouteBound ? 1 : 0,
        host.summary.cautious_route_builds, host.summary.cautious_route_points,
        host.summary.cautious_route_clears, host.summary.cautious_route_waits,
        host.summary.cautious_route_movetos, host.summary.cautious_route_no_slot,
        static_cast<double>(host.tuning.at(0x190)), static_cast<double>(host.tuning.at(0x194)));
    host.log.notef("summary mission ai cautious wedge bound=%d calls=%llu runs=%llu placed=%llu "
        "no_record=%llu zero_frame=%llu (00A11690, packet cc9_cautious_wedge)",
        bsp::kCautiousWedgeBound ? 1 : 0, host.cautious_wedge_calls, host.cautious_wedge_runs,
        host.cautious_wedge_placed, host.cautious_wedge_no_record,
        host.cautious_wedge_zero_frame);
    host.log.notef("summary mission ai coordinator game_mode=%d compose=%llu seeds=%llu "
        "groups_created=%llu destroyed=%llu members_added=%llu evicted=%llu splits=%llu "
        "splits_taken=%llu auto_merges=%llu prox_merges=%llu member_passes=%llu "
        "tick_orders=%llu tick_followers=%llu formation_requests=%llu promotions=%llu "
        "collect_dist=%.1f served=%llu attackmove=%llu settarget=%llu fallback=%llu "
        "scored=%llu ship_members_not_ordered=%llu target_releases=%llu "
        "target_releases_non_idle_birth=%llu release_idle_bound=%d",
        s.game_mode, s.compose_passes, s.seed_candidates, s.groups_created,
        s.groups_destroyed, s.members_added, s.members_evicted, s.splits,
        s.splits_taken, s.auto_merges, s.proximity_merges, s.member_passes,
        s.tick_orders, s.tick_followers, s.tick_formation_requests,
        s.command_promotions,
        static_cast<double>(host.tuning.at(bsp::kAiTuningCloseAttackCollectDist)),
        s.close_members_served, s.close_attack_move_orders, s.close_set_target_orders,
        s.close_fallback_movetos, s.close_candidates_scored,
        s.ship_members_not_ordered, s.target_group_releases,
        s.target_group_releases_non_idle_birth, kAiTargetGroupDestroyedIdleBound ? 1 : 0);
    host.log.notef("summary mission ai troop landing trait close_landers=%llu cargo_landers=%llu "
        "bound=%d (00A1443D, 00A03510 Cargo arm; packet cc9_close_member_class_trait)",
        s.close_troop_landers, s.cargo_troop_landers, bsp::game::kTroopLandingTraitBound ? 1 : 0);
    // Packet cc8_ship_follow: 0077C8D0's first question, 008162B0 -> 00779D50.
    host.log.notef("summary mission ai follow requests=%llu available=%llu refused=%llu "
        "joins=%llu (00779D50: a live ship may follow a live ship of its own side; the "
        "+188h OwnerPlayer arm is skipped, this process has no producer for it)",
        host.formation_requests_seen, host.formation_requests_available,
        host.formation_requests_refused, host.formation_joins_made);
    host.log.notef("summary mission ai follow join loopback posted=%llu delivered=%llu bound=%d "
        "(00A10E67 -> 0077C8D0 -> 0076E520 -> 0076C600 -> 0077F940, packet "
        "cc9_planner_join_loopback)", host.formation_joins_posted,
        host.formation_joins_delivered, kPlannerJoinLoopbackBound ? 1 : 0);
    host.log.notef("summary mission ai group transport moves bound=%d calls=%llu landers=%llu "
        "ready=%llu startlanding_94h=%llu anchors=%llu no_anchor=%llu inside=%llu movetos=%llu "
        "(00A11B80, 008128E0; 94h not delivered, 008206F0 not modelled; packet "
        "cc9_group_transport_moves)", kAiGroupTransportMovesBound ? 1 : 0,
        host.transport_moves.calls, host.transport_moves.landers, host.transport_moves.ready,
        host.transport_moves.ready, host.transport_moves.anchors,
        host.transport_moves.no_anchor, host.transport_moves.inside,
        host.transport_moves.movetos);
    host.log.notef("summary mission ai startlanding 94h bound=%d landing_ships=%llu begun=%llu "
        "refused=%llu transports=%llu launch_bound=%d launches=%llu crafts=%llu "
        "launch_refused=%llu (00821F61 -> vt+238h: 0074A4C0 on 00CFFA30, 008206F0 on "
        "00CFA778; packets cc9_startlanding_94h, cc9_landing_craft_launch)",
        kLandingShipStartLandingBound ? 1 : 0, host.transport_moves.landing_ships,
        host.transport_moves.begun, host.transport_moves.refused,
        host.transport_moves.transports, kLandingCraftLaunchBound ? 1 : 0,
        host.transport_moves.launches, host.transport_moves.crafts,
        host.transport_moves.launch_refused);
    host.log.notef("summary mission ai close fallback bridge calls=%llu class_refused=%llu "
        "near_refused=%llu ships=%llu zone_moved=%llu bound=%d (00A14D48 -> 00A02020, "
        "packet cc9_close_fallback_order_bridge)", s.close_bridge_calls,
        s.close_bridge_class_refused, s.close_bridge_near_refused, s.close_bridge_ships,
        s.close_bridge_zone_moved, kCloseFallbackOrderBridgeBound ? 1 : 0);
    host.log.notef("summary mission ai command zone point asks=%llu answers=%llu moved=%llu "
        "bound=%d (00A020BE / 00A020F0, margin 30.0, packet cc9_ai_command_avoid_zone_point)",
        host.avoid_zone_asks, host.avoid_zone_answers, host.avoid_zone_moved,
        bsp::game::kAiCommandAvoidZonePointBound ? 1 : 0);
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
    host.log.notef("summary mission ai replan flag bound=%d sets=%llu immediate=%llu "
        "foreign_claims=%llu (00A22750 +2Ch, 00A15970 / 00A18480, packet cc9_party_replan_flag)",
        kAiPartyReplanFlagBound ? 1 : 0, host.replan_sets, host.replan_immediate,
        host.replan_foreign_claims);
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
    if constexpr (kAiTickMovetoRetasksPlaneBound) {
        host.log.notef("summary mission ai tick plane retask installed=%llu kept=%llu "
            "replaced_other=%llu not_current=%llu refused=%llu (0099A4C0 -> 0099A170 "
            "009C3BE0 on the tick's moveto, packet cc9_ai_tick_plane_retask)",
            host.retask_installed, host.retask_kept, host.retask_replaced_other,
            host.retask_not_current, host.retask_refused);
    }
    if constexpr (kAiCaptureThinkBound) {
        host.log.notef("summary mission ai capture thinks=%llu target_fallbacks=%llu handoffs=%llu "
            "attack_thinks=%llu (packet cc9_planner_defend_capture_thinks)", host.capture_thinks,
            host.capture_target_fallbacks, host.capture_handoffs, host.attack_thinks);
    }
    if constexpr (kAiDefendThinkBound) {
        host.log.notef("summary mission ai defend thinks=%llu record_fallbacks=%llu "
            "defendposition=%llu (00A28A60 no-record path, packet cc9_planner_defend_capture_thinks)",
            host.defend_thinks, host.defend_record_fallbacks, host.defend_positions);
        if constexpr (kAiDefendRecordsPathBound) {
            host.log.notef("summary mission ai defend records thinks=%llu records=%llu patrolto=%llu "
                "capture_pairs=%llu merges=%llu spawn_arms=%llu (00A28A60 records path, packet "
                "cc9_defend_records_path)", host.defend_record_thinks, host.defend_records_seen,
                host.defend_patrols, host.defend_capture_pairs, host.defend_merges,
                host.defend_spawn_arms);
        }
    }
    host.log.notef("summary mission ai squadron rtb exclusion squadron_calls=%llu "
        "bomber_calls=%llu spent_calls=%llu (009FFEB0 census, packet cc9_squadron_rtb_exclusion)",
        host.rtb_exclusion_squadron_calls, host.rtb_exclusion_bomber_calls,
        host.rtb_exclusion_spent_calls);
    host.log.notef("summary mission ai squadron rtb exclusion bind stand_in_true=%llu issues=%llu "
        "excluded=%llu bound=%d (009FFF09, packet cc9_squadron_rtb_exclusion_bind)",
        host.rtb_exclusion_stand_in_true, host.rtb_exclusion_issues, host.rtb_exclusion_excluded,
        kAiSquadronRtbExclusionBound ? 1 : 0);
    host.log.notef("summary mission ai big landing ship reads=%llu bound=%d (00827F95 via "
        "009FE2D4.. and 00A0360B, packet cc9_big_landing_ship)", host.big_landing_ship_reads,
        kShipAiBigLandingShipBound ? 1 : 0);
    if constexpr (kSellingTickBound) {
        host.log.notef("summary mission ai selling ticks=%llu holds=%llu approaches=%llu "
            "returntobase=%llu (00A11FF0, packet cc9_selling_tick)", host.selling_ticks,
            host.selling_holds, host.selling_approaches, host.selling_returns);
    }
    if constexpr (kAiSellThinkBound) {
        host.log.notef("summary mission ai sell thinks=%llu splits=%llu selling=%llu "
            "(00A22800, packet cc9_planner_defend_capture_thinks)",
            host.sell_thinks, host.sell_splits, host.sell_orders);
    }
    if constexpr (kAiCaptureTargetPathBound) {
        host.log.notef("summary mission ai capture path thinks=%llu assignments=%llu attack=%llu "
            "defendposition=%llu patrolto=%llu kept=%llu merges=%llu spawn_due=%llu "
            "patrol_ticks=%llu patrol_close_passes=%llu "
            "(00A29FD0 target path, packet cc9_planner_defend_capture_thinks)",
            host.capture_path_thinks, host.capture_assignments, host.capture_orders_attack,
            host.capture_orders_defend, host.capture_orders_patrol, host.capture_orders_kept,
            host.capture_merges, host.capture_spawn_due, host.patrol_ticks,
            host.patrol_close_passes);
        host.log.notef("summary mission ai capture group value bound=%d calls=%llu zero=%llu "
            "(00A250A0 at 00A2A380, packet cc9_capture_group_value)",
            kCaptureGroupValueBound ? 1 : 0, host.capture_value_calls, host.capture_value_zero);
    }
    if constexpr (kGeneratedSquadronBrainBound) {
        host.log.notef("summary mission ai generated squadrons=%llu index_shifts=%llu "
            "(packet cc9_generated_squadron_brain_membership)",
            host.generated_squadrons, host.squadron_index_shifts);
    }
    host.log.notef("summary mission ai order dedupe suppressed=%llu (duplicate re-issues "
        "of the same token/target/point; 0077D600 replaces, this ring appends)",
        host.summary.orders_suppressed);
    host.summary.objective_set_units =
        static_cast<unsigned long long>(game_objective_sets().total_units());
    host.log.notef("summary mission ai world sets queries=%llu hits=%llu objective_units=%llu "
        "(00A2C450 over game+21A4h..+21C0h, the eight per-player-slot SzurkeNyil objective "
        "sets; their producers 008CD440 Objectives_Add, 008CDD60 Objectives_AddUnit and "
        "008CE510 Objectives_RemoveUnit are bound (src/game_hosts_lua.cpp); objective_units "
        "is what the sets hold at the end, and 00A2C4B4 answers for an empty set)",
        host.summary.world_set_queries, host.summary.world_set_hits,
        host.summary.objective_set_units);
    {
        const GameObjectiveSets& sets = game_objective_sets();
        host.log.notef("summary mission objective status (packet cc9_objectives_completed, "
            "008BD340/008BD900 -> 008E20D0/008E2200): sets=%llu misses=%llu unit_drops=%llu",
            sets.status_sets, sets.status_misses, sets.status_unit_drops);
        int hidden = 0;
        for (std::size_t k = 0; k < GameObjectiveSets::kSlotCount; ++k) {
            for (const GameObjectiveSets::Objective& o : sets.slots[k]) {
                if (o.kind == 2) ++hidden;
            }
        }
        host.log.notef("summary mission objective kind bound=%d hidden_objectives=%d "
            "hidden_status_holds=%llu (008DBF40 -> +18h, 008DFE6E, packet cc9_objective_kind)",
            kObjectiveKindBound ? 1 : 0, hidden, sets.hidden_status_holds);
    }
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
    host.log.notef("summary mission ai group target value bound=%d calls=%llu "
        "empty_groups=%llu model_pairs=%llu stand_in_pairs=%llu distance_cut_pairs=%llu "
        "zero_results=%llu (00A0F970 at 00A1CC65; packet cc9_planner_group_target_value)",
        kPlannerGroupTargetValueBound ? 1 : 0, host.group_value_census.calls,
        host.group_value_census.empty_groups, host.group_value_census.model_pairs,
        host.group_value_census.stand_in_pairs, host.group_value_census.distance_cut_pairs,
        host.group_value_census.zero_results);
    {
        const AiDamageTermsCensus& c = ai_damage_terms_census();
        host.log.notef("summary mission ai target weight damage terms bound=%d barrels=%llu "
            "armour_refusals=%llu zero_per_hit=%llu per_hit_sum=%.1f close_weight_sum=%.3f "
            "group_weight_sum=%.3f water_scale=%.3f (00A094D5 gate, 009FE200 at 00A09578, "
            "00A095A9 / 00A09624; packet cc9_ai_target_weight_damage_terms)",
            kAiTargetWeightDamageTermsBound ? 1 : 0, c.barrels, c.armour_refusals,
            c.zero_per_hit, c.per_hit_sum, c.close_weight_sum, c.group_weight_sum,
            static_cast<double>(game_ai_weapon_facts().water_damage_scale));
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
        {
            const AiPlaneWeightCensus& c = ai_plane_weight_census();
            host.log.notef("summary mission ai plane weight gates=%d plane_arm=%d "
                "stub_divergences=%llu plane_arm_calls=%llu no_options=%llu loadout_arm=%llu "
                "kamikaze=%llu/%llu(%.1f) strafe=%llu/%llu(%.1f) dogfight=%llu/%llu(%.1f) "
                "gate_admits=%llu platform_refusals=%llu (00A08460 vtable[+18h] queries; "
                "options counted positive/all(sum); packet cc9_plane_attacker_weight)",
                kAiWeightBarrelGatesBound ? 1 : 0, kAiPlaneAttackerWeightBound ? 1 : 0,
                c.stub_divergences, c.plane_arm_calls, c.plane_arm_no_options,
                c.plane_arm_loadout, c.positive_options[0], c.options[0], c.option_value_sum[0],
                c.positive_options[1], c.options[1], c.option_value_sum[1],
                c.positive_options[2], c.options[2], c.option_value_sum[2], c.gate_admits,
                c.platform_refusals);
            for (int code = 0; code < 0x30; ++code) {
                if (c.queries[code] == 0) continue;
                host.log.notef("  ai plane weight type query code=%02Xh queries=%llu "
                    "kind_true=%llu", code, c.queries[code], c.true_answers[code]);
            }
            for (int sub = 0; sub < 0x14; ++sub) {
                if (c.gate_refusals[sub] == 0) continue;
                host.log.notef("  ai plane weight gate refusals subtype=%02Xh count=%llu",
                    sub, c.gate_refusals[sub]);
            }
        }
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
    host.log.notef("summary mission ai party gate bound=%d local_team=%d slot4_team=%d "
        "local_slot_groups=%llu other_slot_groups=%llu (009FFE50 unforced arm, packet "
        "cc9_ai_party_gate)", kAiPartyGateUnforcedBound ? 1 : 0, host.slot_team(0),
        host.slot_team(4), host.party_gate_local_groups, host.party_gate_other_groups);
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
