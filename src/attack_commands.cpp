// Packet cc2_attack_commands. Evidence per claim is in docs/ATTACK_COMMANDS.md
// and reports/attack_commands.json; every address named in a comment here is
// repeated there with its role.
#include "bsp/attack_commands.hpp"

namespace bsp {
namespace {

// One row per class, in the order docs/SCENE_COMMAND_TYPES.md ordinals them.
// The category column is that document's; every other column is a fact read at
// the address named in the header's field comment. A column an arm never sets
// carries -1, false or 0.
constexpr AttackCommandEffects kRows[kAttackCommandRowCount] = {
    // artillery: 00817229 replaces it with attackmove before anything reads it,
    // and 009F3D57 gives it attackmove's bot slot, so it has no arm of its own.
    {kAttackCmdArtillery, 1, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconAttack, -1, 0u, 0u, 0u},
    // torpedo: 007EEA40, factory 009D4E30, getter 009D3280, test 009D3EF0.
    {kAttackCmdTorpedo, 2, kOrdnanceKindTorpedo, false, false, -1, -1, -1, -1,
     kOrderIconAttack, 0x34, 0x009d4e30u, 0x009d3ef0u, 0x4c4},
    // divebomb: 007EE9C5, factory 009C8C70, getter 009C79A0, test 009C8060.
    {kAttackCmdDiveBomb, 2, kOrdnanceKindGeneralBomb, true, false,
     -1, kKindLevelBomberSelf, -1, kKindBombExcluded,
     kOrderIconAttack, 0x14, 0x009c8c70u, 0x009c8060u, 0x440},
    // levelbomb: 007EE946, factory 009B9030, getter 009B7BB0, test 009B81F0.
    {kAttackCmdLevelBomb, 2, kOrdnanceKindLevelBomb, true, false,
     kKindLevelBomberSelf, -1, -1, -1,
     kOrderIconAttack, 0x24, 0x009b9030u, 0x009b81f0u, 0x440},
    // dropkamikaze: 007EEBCC, factory 009AEBE0, getter 009ADDC0, test 009AE140.
    {kAttackCmdDropKamikaze, 2, kOrdnanceKindDropKamikaze, true, false,
     -1, -1, -1, -1, kOrderIconAttack, 0x64, 0x009aebe0u, 0x009ae140u, 0x440},
    // depthcharge: 007EEAB3, factory 009A6970, getter 009A54D0, test 009A5DB0.
    {kAttackCmdDepthCharge, 2, kOrdnanceKindDepthCharge, false, false,
     -1, -1, kKindSubmarine, -1,
     kOrderIconAttack, 0x04, 0x009a6970u, 0x009a5db0u, 0x48c},
    // strafe: 007EEB94, factory 009CD300, getter 009CC3D0, test 009CC850.
    {kAttackCmdStrafe, 1, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconAttack, 0x54, 0x009cd300u, 0x009cc850u, 0x44c},
    // rocket: 007EEA20, factory 007B7FD0, getter 007B6CD0, test 007B7200.
    {kAttackCmdRocket, 2, kOrdnanceKindRocket, true, false,
     -1, -1, -1, kKindBombExcluded,
     kOrderIconAttack, 0x74, 0x007b7fd0u, 0x007b7200u, 0x468},
    // kamikaze: 007EEB36, factory 009AF720, getter 009AF060, test 009AF560.
    {kAttackCmdKamikaze, 2, kOrdnanceKindNone, true, false,
     kKindKamikazeCapableSelf, -1, -1, -1,
     kOrderIconAttack, 0x64, 0x009af720u, 0x009af560u, 0x478},
    // dogfight: 007EEAEC, factory 009AB570, getter 009A99E0, test 009A9CC0.
    {kAttackCmdDogfight, 1, kOrdnanceKindNone, false, true,
     -1, kKindDogfightExcludedSelf, -1, -1,
     kOrderIconAttack, 0x54, 0x009ab570u, 0x009a9cc0u, 0x4c4},
    // attackmove: 007EE938 answers true for it unconditionally; 0099A21F turns
    // it into one of the ten above whenever it carries a target.
    {kCommandAttackMove, 2, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconAttack, -1, 0u, 0u, 0u},
    // moveonpath: 0099A1DC's factory only; the path machinery is a peer's.
    {kCommandMoveOnPath, 3, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconMove, -1, 0x009bdbb0u, 0u, 0u},
    // retreat: 0099A45B's factory; 007F16D0 also builds one when no base exists.
    {kAttackCmdRetreat, 3, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconHold, -1, 0x009ca2b0u, 0u, 0u},
    // returntobase: 0099A22A resolves it through 007F16D0; it never reaches a
    // factory itself.
    {kAttackCmdReturnToBase, 3, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconNone, -1, 0u, 0u, 0u},
    // land: 0099A448's factory 009B41C0; 00816FC0 rewrites it to attackmove for
    // a unit that is not IsKindOf(0Ch).
    {kAttackCmdLand, 3, kOrdnanceKindNone, false, false,
     -1, -1, -1, -1, kOrderIconLand, -1, 0x009b41c0u, 0u, 0u},
    // closetoship: Lua-issued only (008A4960). Factory 009A2F40, getter
    // 009A28B0, test 009A2920, which reads the entity rather than the director.
    {kAttackCmdCloseToShip, 3, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconNone, -1, 0x009a2f40u, 0x009a2920u, 0x438},
    // Leave: 00816EFE, a one-shot that takes an optional IsKindOf(2) target.
    {kCommandLeave, 0, kOrdnanceKindNone, false, false, -1, -1, kKindSurfaceVessel,
     -1, kOrderIconNone, -1, 0u, 0u, 0u},
    // disband: 00816F43 calls 0077CA60 and returns; no target, no slot.
    {kCommandDisband, 0, kOrdnanceKindNone, false, false, -1, -1, -1, -1,
     kOrderIconNone, -1, 0u, 0u, 0u},
};

bool level_bomb_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EE946..007EE9C2. BL is the IsKindOf(10h) answer taken once at
    // 007EE95C; the two halves differ only in which ordnance they accept and
    // which target kind they then demand.
    if (!in.target_is_surface) {
        return false;
    }
    if (!in.self_is_level_bomber) {
        return false;
    }
    if (in.has_level_bomb_ordnance) {
        return in.target_is_structure; // 007EE980, vt[5Ch](1Ch)
    }
    if (!in.has_general_bomb_ordnance) {
        return false; // 007EE9A4, 007ED7E0
    }
    return !in.target_is_bomb_excluded; // 007EE9B2, vt[5Ch](0Eh)
}

bool dive_bomb_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EE9C5..007EEA1D, the exact complement of the level-bomber gate.
    if (!in.target_is_surface) {
        return false;
    }
    if (in.self_is_level_bomber) {
        return false; // 007EE9E6, a level bomber never dive bombs
    }
    if (!in.has_general_bomb_ordnance) {
        return false;
    }
    return !in.target_is_bomb_excluded;
}

bool rocket_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEA20..007EEA3E, which joins divebomb's tail at 007EE9FA.
    if (!in.target_is_surface) {
        return false;
    }
    if (!in.has_rocket_ordnance) {
        return false;
    }
    return !in.target_is_bomb_excluded;
}

bool torpedo_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEA40..007EEAAE. The depth-band test only narrows the surface case:
    // a torpedo that runs at the surface cannot be used on a target the caller
    // did not classify as a surface vessel.
    if (!in.has_torpedo_ordnance) {
        return false;
    }
    if (!in.target_is_surface && in.torpedo_runs_at_surface) {
        return false; // 007EEA94..007EEA9D
    }
    return !in.torpedo_target_blocked; // 007EEAA9, 00828EC0 must answer 0
}

bool depth_charge_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEAB3..007EEAE9. The only arm that never looks at the surface flag.
    if (!in.target_is_submarine) {
        return false;
    }
    return in.has_depth_charge_ordnance;
}

bool dogfight_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEAEC..007EEB31.
    if (!in.target_is_air) {
        return false; // 007EEAF5, 00922B10
    }
    if (!in.guns_available) {
        return false; // 007EEB08, controller+C24h
    }
    if (in.self_is_dogfight_excluded) {
        return false; // 007EEB1A, vt[5Ch](16h)
    }
    return !in.guns_suppressed; // 007EEB2C, 0047B850 must answer 0
}

bool kamikaze_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEB36..007EEB91.
    if (!in.target_is_surface) {
        return false;
    }
    if (!in.self_is_kamikaze_capable) {
        return false; // 007EEB53, vt[5Ch](17h)
    }
    if (in.target_is_kamikaze_ship && in.kamikaze_ship_blocked) {
        return false; // 007EEB6E..007EEB8A
    }
    return true;
}

bool strafe_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEB94..007EEBC7. Unlike every ordnance arm, a non-surface target is
    // not fatal: it only has to answer IsKindOf(41h).
    if (!in.target_is_surface && !in.target_is_strafe_fallback) {
        return false; // 007EEBA7
    }
    if (!in.guns_available) {
        return false;
    }
    return !in.guns_suppressed;
}

bool drop_kamikaze_applies(const AttackFeasibilityInputs& in) noexcept
{
    // 007EEBCC..007EEBE8, the shortest arm.
    if (!in.target_is_surface) {
        return false;
    }
    return in.has_drop_kamikaze_ordnance;
}

} // namespace

const AttackCommandEffects* attack_command_row(int index) noexcept
{
    if (index < 0 || index >= kAttackCommandRowCount) {
        return nullptr;
    }
    return &kRows[index];
}

const AttackCommandEffects* attack_command_effects(std::uint32_t command) noexcept
{
    for (int i = 0; i < kAttackCommandRowCount; ++i) {
        if (kRows[i].command == command) {
            return &kRows[i];
        }
    }
    return nullptr;
}

bool attack_command_is_ordnance_class(std::uint32_t command) noexcept
{
    // 009F6D80, seven compares and nothing else. They are exactly the seven
    // classes 007EEC50 tries before it considers guns.
    return command == kAttackCmdLevelBomb || command == kAttackCmdDropKamikaze
        || command == kAttackCmdDiveBomb || command == kAttackCmdTorpedo
        || command == kAttackCmdKamikaze || command == kAttackCmdRocket
        || command == kAttackCmdDepthCharge;
}

std::uint32_t attack_command_normalise_on_issue(std::uint32_t command,
                                                bool self_is_land_capable) noexcept
{
    if (command == kAttackCmdArtillery) {
        return kCommandAttackMove; // 00817229..00817239
    }
    if (command == kAttackCmdLand && !self_is_land_capable) {
        return kCommandAttackMove; // 00816FC0, vt[5Ch](0Ch) on the unit
    }
    return command;
}

bool attack_command_target_side_allows(bool unit_has_weapon_controller,
                                       bool target_present,
                                       bool target_is_structure,
                                       int unit_side,
                                       int target_side) noexcept
{
    // 007EE8F6..007EE932.
    if (!target_present || !unit_has_weapon_controller) {
        return false;
    }
    if (target_is_structure) {
        return target_side != unit_side; // 007EE917
    }
    if (target_side == unit_side) {
        return false; // 007EE923
    }
    return target_side != kSideNeutral; // 007EE928
}

bool attack_command_applies(std::uint32_t command,
                            const AttackFeasibilityInputs& in) noexcept
{
    if (!attack_command_target_side_allows(in.unit_has_weapon_controller,
                                           in.target_present,
                                           in.target_is_structure,
                                           in.unit_side,
                                           in.target_side)) {
        return false;
    }
    if (command == kCommandAttackMove) {
        return true; // 007EE938, the one arm with no condition
    }
    if (command == kAttackCmdLevelBomb) {
        return level_bomb_applies(in);
    }
    if (command == kAttackCmdDiveBomb) {
        return dive_bomb_applies(in);
    }
    if (command == kAttackCmdRocket) {
        return rocket_applies(in);
    }
    if (command == kAttackCmdTorpedo) {
        return torpedo_applies(in);
    }
    if (command == kAttackCmdDepthCharge) {
        return depth_charge_applies(in);
    }
    if (command == kAttackCmdDogfight) {
        return dogfight_applies(in);
    }
    if (command == kAttackCmdKamikaze) {
        return kamikaze_applies(in);
    }
    if (command == kAttackCmdStrafe) {
        return strafe_applies(in);
    }
    if (command == kAttackCmdDropKamikaze) {
        return drop_kamikaze_applies(in);
    }
    return false; // 007EEBF6, every class the chain does not name
}

std::uint32_t attack_command_choose(const AttackFeasibilityInputs& in,
                                    bool prefer_ordnance,
                                    bool allow_guns) noexcept
{
    if (!in.unit_has_weapon_controller) {
        return kCommandMoveTo; // 007EEC5A, the only non-attack return
    }
    if (!attack_command_target_side_allows(true,
                                           in.target_present,
                                           in.target_is_structure,
                                           in.unit_side,
                                           in.target_side)) {
        return 0u; // 007EEC50's inverted guard, mirroring 007EE8F0's prologue
    }

    std::uint32_t ordnance = 0u;
    if (!in.target_is_air) { // 007EEC8A, 00922B10: an airborne target skips it
        const std::uint32_t order[] = {
            kAttackCmdLevelBomb, kAttackCmdDropKamikaze, kAttackCmdDiveBomb,
            kAttackCmdTorpedo, kAttackCmdRocket, kAttackCmdKamikaze,
            kAttackCmdDepthCharge,
        };
        for (std::uint32_t candidate : order) {
            if (attack_command_applies(candidate, in)) {
                ordnance = candidate;
                break;
            }
        }
    }

    if (prefer_ordnance && ordnance != 0u) {
        return ordnance; // 007EED9F's first exit
    }

    std::uint32_t guns = 0u;
    if (attack_command_applies(kAttackCmdDogfight, in)) {
        guns = kAttackCmdDogfight;
    } else if (attack_command_applies(kAttackCmdStrafe, in)) {
        guns = kAttackCmdStrafe;
    }
    if (!allow_guns) {
        return 0u; // 007EEDB2, a gun-only answer is discarded
    }
    return guns;
}

bool attack_task_still_valid(std::uint32_t my_command,
                             std::uint32_t current_command,
                             std::uint32_t latched_target,
                             std::uint32_t resolved_target) noexcept
{
    // 009A5DB0 and the eight siblings: the class compare, then the attackmove
    // compare, then the latched-target compare, which is skipped when the task
    // never latched one.
    if (current_command != my_command && current_command != kCommandAttackMove) {
        return false;
    }
    if (latched_target != 0u && resolved_target != latched_target) {
        return false;
    }
    return true;
}

bool close_to_ship_task_still_valid(std::uint32_t current_command,
                                    std::uint32_t latched_target,
                                    std::uint32_t resolved_target) noexcept
{
    // 009A2920. No attackmove arm, and the latched target is compared before
    // the null check the other nine make, because 009A2963 reads it first.
    if (current_command != kAttackCmdCloseToShip) {
        return false;
    }
    return resolved_target == latched_target;
}

bool attack_command_survives_retest(int category,
                                    bool target_present,
                                    bool still_applies) noexcept
{
    if (category != 1 && category != 2) {
        return true; // 009F8231, the category gate
    }
    if (!target_present) {
        return false; // 009F823B
    }
    return still_applies; // 009F8248, 007EE8F0 again
}

bool command_blocks_auto_action(std::uint32_t current_command,
                                bool command_target_valid,
                                bool path_cursor_blocks) noexcept
{
    // 00811FAE..0081204A. stop is checked first and falls straight through.
    if (current_command == kCommandStop) {
        return false;
    }
    if (current_command == kAttackCmdRetreat || current_command == kCommandCruise) {
        return true; // 00811FCC, 00811FD8
    }
    if (current_command == kCommandAttackMove || current_command == kCommandMoveTo) {
        return command_target_valid; // 00811FE4..00812018
    }
    if (current_command == kCommandMoveOnPath) {
        return path_cursor_blocks; // 00812038..0081204A
    }
    return false;
}

int attack_command_order_icon(std::uint32_t command) noexcept
{
    const AttackCommandEffects* row = attack_command_effects(command);
    if (row != nullptr && row->hud_order_icon != kOrderIconNone) {
        return row->hud_order_icon;
    }
    // The five movement classes 00534870 names that this packet does not own.
    if (command == kCommandStop) {
        return kOrderIconHold;
    }
    if (command == kCommandFollow || command == kCommandMoveTo
        || command == kCommandMoveOnPath) {
        return kOrderIconMove;
    }
    if (command == kCommandCruise) {
        return kOrderIconCruise;
    }
    return kOrderIconNone;
}

int attack_command_ai_weight_offset(std::uint32_t command) noexcept
{
    const AttackCommandEffects* row = attack_command_effects(command);
    return row != nullptr ? row->ai_weight_offset : -1;
}

std::size_t attack_command_bot_slot_offset(std::uint32_t command,
                                           bool bot_has_group,
                                           bool group_predicate_00779aa0) noexcept
{
    // 009F3D00's chain, in its own order.
    if (command == kCommandMoveTo) {
        return 0xc5c;
    }
    if (command == kAttackCmdLand) {
        return 0xc38;
    }
    if (command == kCommandStop) {
        return 0xbd8;
    }
    if (command == kCommandMoveOnPath) {
        return 0xc64;
    }
    if (command == kCommandFollow) {
        return 0xbe4;
    }
    if (command == kAttackCmdArtillery || command == kCommandAttackMove) {
        if (!bot_has_group) {
            return 0xc70; // 009F3D68, bot+B0Ch == 0
        }
        return group_predicate_00779aa0 ? 0x2254 : 0x217c;
    }
    if (command == kCommandCruise) {
        return 0xbc8;
    }
    return kBotSlotNone;
}

bool terminal_command_runs_at_issue(std::uint32_t command) noexcept
{
    // 0071DF0F, 0071DF16: the two classes 0071DEE0 refuses to call an active
    // order that are not stop or cruise.
    return command == kCommandLeave || command == kCommandDisband;
}

std::uint32_t bot_install_command_task_0099a170(std::uint32_t unit,
                                                AttackCommandHost& host)
{
    const std::uint32_t director = host.unit_weapon_director(unit);
    if (director == 0u) {
        return 0u; // 0099A17E
    }
    std::uint32_t command = host.director_current_command(director);
    if (command == 0u) {
        return 0u; // 0099A199
    }
    std::uint32_t target = host.resolve_target(host.director_current_params(director));

    if (command == kAttackCmdLand && target == 0u) {
        command = kAttackCmdReturnToBase; // 0099A1B4, land without a target
    }
    if (command == kCommandAttackMove) {
        if (target != 0u) {
            command = host.choose_attack_command(unit, target, true, true); // 0099A21F
        }
    } else if (command == kAttackCmdReturnToBase) {
        command = host.resolve_return_to_base(unit); // 0099A22A, 007F16D0
    }

    // 0099A23A onwards: one arm per class, each with its own precondition.
    bool ok = false;
    if (command == 0u || command == kCommandMoveTo || command == kCommandMoveOnPath) {
        ok = true; // 0099A23A, 0099A24B, 0099A25C
    } else if (command == kAttackCmdDiveBomb || command == kAttackCmdLevelBomb
               || command == kAttackCmdDropKamikaze || command == kAttackCmdKamikaze
               || command == kAttackCmdDogfight) {
        ok = target != 0u && host.entity_is_kind(target, kKindSurfaceVessel);
    } else if (command == kAttackCmdRocket) {
        ok = target != 0u && host.entity_is_kind(target, kKindSurfaceVessel)
             && !host.entity_is_kind(target, kKindAircraftB); // 0099A39C
    } else if (command == kAttackCmdTorpedo || command == kAttackCmdCloseToShip) {
        ok = target != 0u && host.target_still_attackable(target); // 009229F0
    } else if (command == kAttackCmdStrafe) {
        ok = target != 0u; // 0099A31B, the only attack arm with no kind test
    } else if (command == kAttackCmdDepthCharge) {
        ok = target != 0u && host.entity_is_kind(target, kKindSubmarine);
    } else if (command == kAttackCmdLand) {
        ok = host.land_group_available(unit); // 0099A424
    } else if (command == kAttackCmdRetreat || command == kCommandStop) {
        ok = true; // 0099A455, 0099A462
    }
    if (!ok) {
        return 0u;
    }

    const std::uint32_t task = host.create_bot_task(unit, command, target);
    if (task != 0u) {
        host.install_bot_task(unit, task); // 0099A46A, 0099A020
    }
    return task;
}

ReturnToBaseOutcome return_to_base_outcome(bool has_assigned_base,
                                           bool base_is_reachable,
                                           bool carrier_slot_available) noexcept
{
    if (has_assigned_base && base_is_reachable) {
        return ReturnToBaseOutcome::kIssuedLandAtOwnBase; // 007F1878
    }
    if (carrier_slot_available) {
        return ReturnToBaseOutcome::kIssuedLandAtCarrier; // 007F18C6
    }
    return ReturnToBaseOutcome::kBuiltRetreat; // the retreat record 007F19xx writes
}

} // namespace bsp
