#pragma once
// Projection of the seventeen attack and terminal command classes that
// docs/COMMAND_CLASSES.md left as `contract: unread`.
//
// Packet cc2_attack_commands. docs/ATTACK_COMMANDS.md and
// reports/attack_commands.json carry the addresses and the evidence per claim.
// Every descriptive name here is a hypothesis, not a recovered symbol.
//
// The headline the packet establishes: the ten attack classes carry no
// behaviour of their own and almost none of it lives in the unit command
// controller either. Two routines hold it. 007EE8F0 decides whether one class
// applies to one target with one loadout, and 007EEC50 runs 007EE8F0 over the
// classes in a fixed preference order to pick one. Everything downstream (the
// bot task factory 0099A170, the per-tick revalidation 009F8160, the AI weight
// lookup 009F9770, the HUD order icon 00534870) reads the class that came out.
//
// Contracts this header does not own and does not duplicate:
//   * the 26 class descriptors and the singleton block 00E08EF8..00E08FC7:
//     include/bsp/entity_orders.hpp (EntityOrderCommandClass).
//   * the ten-slot queue, the override slot, the stages and the kCommand*
//     singleton constants: include/bsp/command_execution.hpp.
//   * the controller object, its permission bytes +220h/+222h/+240h and the
//     fire-target setter 00835860: include/bsp/weapon_director.hpp and
//     docs/WEAPON_DIRECTOR.md.
//   * `cruise` (00E08F70) and the commanded-speed stages: docs/CRUISE_COMMAND.md
//     and docs/UNIT_COMMANDED_SPEED.md (009E1170), a peer's.
//   * `moveonpath` (00E08F80): the path machinery is a peer's; only the two
//     places this packet read it are recorded here.
//   * entity `vtable[5Ch]` is the `IsKindOf(int)` test established by
//     docs/UNIT_TIMED_SUBUPDATES.md and docs/WEAPON_DIRECTOR.md.
#include <cstddef>
#include <cstdint>

#include "bsp/command_execution.hpp"
#include "bsp/entity_orders.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The seventeen singletons this packet read
// ---------------------------------------------------------------------------
// Addresses from the 00E08EF8 + 8*ordinal block; names from
// docs/SCENE_COMMAND_TYPES.md's 26-row table. command_execution.hpp already
// owns kCommandFollow/MoveTo/Cruise/AttackMove/MoveOnPath/Stop/Leave/Disband,
// so those eight are not repeated.

inline constexpr std::uint32_t kAttackCmdArtillery = 0x00e08f10u;
inline constexpr std::uint32_t kAttackCmdTorpedo = 0x00e08f18u;
inline constexpr std::uint32_t kAttackCmdDiveBomb = 0x00e08f20u;
inline constexpr std::uint32_t kAttackCmdLevelBomb = 0x00e08f28u;
inline constexpr std::uint32_t kAttackCmdDropKamikaze = 0x00e08f30u;
inline constexpr std::uint32_t kAttackCmdDepthCharge = 0x00e08f38u;
inline constexpr std::uint32_t kAttackCmdStrafe = 0x00e08f40u;
inline constexpr std::uint32_t kAttackCmdRocket = 0x00e08f48u;
inline constexpr std::uint32_t kAttackCmdKamikaze = 0x00e08f50u;
inline constexpr std::uint32_t kAttackCmdDogfight = 0x00e08f58u;
inline constexpr std::uint32_t kAttackCmdRetreat = 0x00e08f90u;
inline constexpr std::uint32_t kAttackCmdReturnToBase = 0x00e08f98u;
inline constexpr std::uint32_t kAttackCmdLand = 0x00e08fa0u;
inline constexpr std::uint32_t kAttackCmdCloseToShip = 0x00e08fa8u;
inline constexpr std::uint32_t kAttackCmdTutorial = 0x00e08fc0u;

// ---------------------------------------------------------------------------
// The kind ids the feasibility arms test through vtable[5Ch]
// ---------------------------------------------------------------------------
// Every one of these is a literal pushed immediately before a `CALL [vt+5Ch]`
// in 007EE8F0, 008A4F00, 008A50D0, 0099A170 or 00922B10/00922C80. What each
// kind *is* is not recovered; only which arm asks for it.

inline constexpr int kKindSurfaceVessel = 0x02;     // 0099A170's bomb/rocket gate
inline constexpr int kKindKamikazeTargetShip = 0x06;// 007EEB64
inline constexpr int kKindSubmarine = 0x08;         // 007EEABF, the depthcharge gate
inline constexpr int kKindBombExcluded = 0x0e;      // 007EE9B2, 007EEA07
inline constexpr int kKindAircraftA = 0x0f;         // 00922B10, 008A50D0
inline constexpr int kKindLevelBomberSelf = 0x10;   // 007EE95A, 007EE9E2
inline constexpr int kKindDogfightExcludedSelf = 0x16; // 007EEB1A
inline constexpr int kKindKamikazeCapableSelf = 0x17;  // 007EEB53, 008A50D0
inline constexpr int kKindAircraftB = 0x18;         // 00922B10, 008A50D0, 0099A170
inline constexpr int kKindStructure = 0x1c;         // 007EE980, 007EE8F0's side gate
inline constexpr int kKindStrafeFallback = 0x41;    // 007EEBA7
inline constexpr int kKindLandCapableSelf = 0x0c;   // 00816FC0's land gate

// The one side value that is not a team: 007EE928's `CMP EAX,2`.
inline constexpr int kSideNeutral = 2;

// ---------------------------------------------------------------------------
// Ordnance kinds, the values 007B91C0 forwards to descriptor->vtable[8]
// ---------------------------------------------------------------------------
// Each `007ED7E0` family member is the same loop over the weapon controller's
// +3CCh slots; the only thing that differs is the kind it asks for.

inline constexpr int kOrdnanceKindNone = -1;
inline constexpr int kOrdnanceKindGeneralBomb = 0x2a;  // 007B9320's first test
inline constexpr int kOrdnanceKindTorpedo = 0x2b;      // 007B93F0
inline constexpr int kOrdnanceKindDepthCharge = 0x2c;  // 007B94F0
inline constexpr int kOrdnanceKindDropKamikaze = 0x2f; // 007B93E0
inline constexpr int kOrdnanceKindLevelBomb = 0x31;    // 007B9500
inline constexpr int kOrdnanceKindRocket = 0x33;       // 007B9480

// 007B9320 accepts kind 2Ah only when the same descriptor answers none of
// these: they are the kinds that have their own command class.
inline constexpr int kDiveBombExcludedKinds[] = {0x2c, 0x31, 0x2b, 0x33, 0x2d};

// 007EEA6C..007EEA8E, the torpedo depth-band gate. The descriptor field +8 must
// equal 0Ah and the float pair +F0h/+F4h must bracket the constant at 00D7A23C.
inline constexpr int kTorpedoDescriptorTypeValue = 0x0a;
inline constexpr float kTorpedoSurfaceDepth = 0.001f; // 00D7A23C, bytes 6F 12 83 3A

// ---------------------------------------------------------------------------
// The per-class effect row
// ---------------------------------------------------------------------------
// One row per class the packet read. Everything in it is a fact recorded at a
// named address; a field that no arm sets carries the neutral value.

struct AttackCommandEffects {
    std::uint32_t command;
    int category;             // class vtable[0Ch], docs/SCENE_COMMAND_TYPES.md
    int ordnance_kind;        // the 007B91C0 argument, or kOrdnanceKindNone
    bool needs_surface_target;// the byte argument 007EE8F0 tests before the arm
    bool needs_air_target;    // 00922B10 inside the arm
    int self_kind_required;   // vt[5Ch] on weaponController (unit+3D0h), or -1
    int self_kind_forbidden;  // as above, or -1
    int target_kind_required; // vt[5Ch] on the target, or -1
    int target_kind_forbidden;// as above, or -1
    int hud_order_icon;       // 00534870's per-class constant, or -1
    int ai_weight_offset;     // 009F9770's offset into 004D6CE0's block, or -1
    std::uint32_t bot_task_factory;   // 0099A170's call, or 0
    std::uint32_t bot_task_valid_test;// the still-valid predicate, or 0
    std::size_t bot_task_latched_target_offset; // its `this` offset, or 0
};

// The seventeen rows, plus attackmove because five arms rewrite a class into it
// and every still-valid test accepts it.
inline constexpr int kAttackCommandRowCount = 18;
const AttackCommandEffects* attack_command_effects(std::uint32_t command) noexcept;
const AttackCommandEffects* attack_command_row(int index) noexcept;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 009F6D80: the seven category-2 classes, the ones 007EEC50 tries before guns.
bool attack_command_is_ordnance_class(std::uint32_t command) noexcept;

// 00816E30's two rewrites, both taken before the command reaches a queue slot.
// 00817229: artillery is replaced by attackmove outright. 00816FC0: land is
// replaced by attackmove unless the unit answers IsKindOf(0Ch).
std::uint32_t attack_command_normalise_on_issue(std::uint32_t command,
                                                bool self_is_land_capable) noexcept;

// 007EE8F0's prologue, 007EE8F6..007EE932. A target passes only when the unit
// has a weapon controller and the sides disagree; a non-structure target must
// also not be neutral.
bool attack_command_target_side_allows(bool unit_has_weapon_controller,
                                       bool target_present,
                                       bool target_is_structure,
                                       int unit_side,
                                       int target_side) noexcept;

// The inputs one arm of 007EE8F0 reads. Each field names the site that reads it.
struct AttackFeasibilityInputs {
    bool unit_has_weapon_controller = false; // 007EE8FC
    bool target_present = false;             // 007EE8F6
    bool target_is_structure = false;        // 007EE90A, vt[5Ch](1Ch)
    int unit_side = 0;                       // 007EE917, unit+54h
    int target_side = 0;                     // 007EE912, target+54h

    bool target_is_surface = false; // the byte argument, 00922C80's result
    bool target_is_air = false;     // 007EEAF5, 00922B10

    bool self_is_level_bomber = false;    // 007EE95A, vt[5Ch](10h) on unit+3D0h
    bool self_is_kamikaze_capable = false;// 007EEB53, vt[5Ch](17h)
    bool self_is_dogfight_excluded = false;// 007EEB1A, vt[5Ch](16h)

    bool target_is_bomb_excluded = false; // 007EE9B2, vt[5Ch](0Eh)
    bool target_is_submarine = false;     // 007EEABF, vt[5Ch](8)
    bool target_is_kamikaze_ship = false; // 007EEB64, vt[5Ch](6)
    bool target_is_strafe_fallback = false;// 007EEBA7, vt[5Ch](41h)

    bool has_level_bomb_ordnance = false;  // 007ED830 -> 007B9500, kind 31h
    bool has_general_bomb_ordnance = false;// 007ED7E0 -> 007B9320, kind 2Ah
    bool has_drop_kamikaze_ordnance = false;// 007ED880 -> 007B93E0, kind 2Fh
    bool has_torpedo_ordnance = false;     // 007ED8D0 -> 007B93F0, kind 2Bh
    bool has_rocket_ordnance = false;      // 007ED970 -> 007B9480, kind 33h
    bool has_depth_charge_ordnance = false;// 007EDA10 -> 007B94F0, kind 2Ch

    bool torpedo_runs_at_surface = false;  // 007EEA6C..007EEA8E's depth band
    bool torpedo_target_blocked = false;   // 007EEAA9, 00828EC0(target+538h) != 0
    bool guns_available = false;           // 007EEB08/007EEBB7, controller+C24h
    bool guns_suppressed = false;          // 0047B850 on the weapon controller
    bool kamikaze_ship_blocked = false;    // 00827F70(target+538h) and 00604A50
};

// 007EE8F0. True when the named class may be issued against this target.
// A class with no arm (anything outside the eleven the routine names) is false,
// which is 007EEBF6's fall-through.
bool attack_command_applies(std::uint32_t command,
                            const AttackFeasibilityInputs& in) noexcept;

// 007EEC50. Runs the seven ordnance classes in the order levelbomb,
// dropkamikaze, divebomb, torpedo, rocket, kamikaze, depthcharge, then the two
// gun classes dogfight, strafe. Returns 0 when nothing applies.
//   prefer_ordnance: 007EEC90's third argument; when false the gun pass runs
//                    even if an ordnance class matched.
//   allow_guns:      the fourth; when false a gun-only result is discarded.
// The two early exits are kept: no weapon controller yields moveto, and a
// target that fails the side gate yields 0.
std::uint32_t attack_command_choose(const AttackFeasibilityInputs& in,
                                    bool prefer_ordnance,
                                    bool allow_guns) noexcept;

// 009A5DB0 and its eight siblings, byte-identical apart from the class constant
// and the latched-target offset. A bot attack task keeps running while the
// director still reports its own class or attackmove, and while the command's
// resolved target still matches the target the task latched.
bool attack_task_still_valid(std::uint32_t my_command,
                             std::uint32_t current_command,
                             std::uint32_t latched_target,
                             std::uint32_t resolved_target) noexcept;

// 009A2920, closetoship's. Same rule with two differences: it reads the class
// off the entity (vtable[174h]) rather than the director, and it does not
// accept attackmove, because 007EEC50 never produces closetoship.
bool close_to_ship_task_still_valid(std::uint32_t current_command,
                                    std::uint32_t latched_target,
                                    std::uint32_t resolved_target) noexcept;

// 009F8160's revalidation gate. Any class whose category is 1 or 2 is re-tested
// against 007EE8F0 every tick and abandoned when the target is gone or the test
// fails; every other class survives this gate untouched. The depthcharge branch
// at 009F81C6 is not part of it: when depthcharge still applies it sets the bot
// byte +3Dh, it does not end the command.
bool attack_command_survives_retest(int category,
                                    bool target_present,
                                    bool still_applies) noexcept;

// 00811F50, the six-class suppression predicate. The previous packet recorded
// this address as a name table; it is not one. It answers whether the unit's
// current command blocks the caller's action.
bool command_blocks_auto_action(std::uint32_t current_command,
                                bool command_target_valid,
                                bool path_cursor_blocks) noexcept;

// 00534870's chain: the order-category icon the HUD shows for a class.
inline constexpr int kOrderIconNone = -1;
inline constexpr int kOrderIconHold = 0;   // stop, retreat
inline constexpr int kOrderIconMove = 1;   // follow, moveto, moveonpath
inline constexpr int kOrderIconCruise = 2; // cruise
inline constexpr int kOrderIconAttack = 3; // the ten attack classes, attackmove
inline constexpr int kOrderIconLand = 4;   // land
int attack_command_order_icon(std::uint32_t command) noexcept;

// 009F9770's map from the chosen class to a 10h-byte stride offset inside the
// tuning block 004D6CE0 returns. dogfight and strafe share +54h; kamikaze and
// dropkamikaze share +64h.
int attack_command_ai_weight_offset(std::uint32_t command) noexcept;

// 009F3D00's map from a class to the bot sub-controller that runs it. artillery
// and attackmove share one slot, which is 007EE8F0's rewrite seen from the AI
// side. The 217Ch/2254h pair is the branch taken when bot+B0Ch is non-zero.
inline constexpr std::size_t kBotSlotNone = 0;
std::size_t attack_command_bot_slot_offset(std::uint32_t command,
                                           bool bot_has_group,
                                           bool group_predicate_00779aa0) noexcept;

// 0071DEE0, a peer's routine, quoted for the completion table: Leave and
// disband never count as active orders, so they run at issue time and the slot
// they occupy is skipped. Reproduced here only as the predicate this packet
// needs; the queue walk itself stays in command_execution.hpp.
bool terminal_command_runs_at_issue(std::uint32_t command) noexcept;

// ---------------------------------------------------------------------------
// The per-class arms as sequences over an injected host
// ---------------------------------------------------------------------------
// One virtual per native call site. The bodies in attack_commands.cpp perform
// the same calls in the same order as 0099A170 and 007F16D0.

struct AttackCommandHost {
    virtual ~AttackCommandHost() = default;

    // 0099A18B: unit->vtable[114h](), the weapon director.
    virtual std::uint32_t unit_weapon_director(std::uint32_t unit) = 0;
    // 0099A193, 009A5DD0: 0071BE40 BSP_WeaponDirector_CurrentCommand.
    virtual std::uint32_t director_current_command(std::uint32_t director) = 0;
    // 0099A19C, 009A5DEC: 0071EB60, the current command's parameter record.
    virtual std::uint32_t director_current_params(std::uint32_t director) = 0;
    // 0099A1A2, 009A5DF3: 00521EA0 BSP_CommandTarget_ResolveObject.
    virtual std::uint32_t resolve_target(std::uint32_t params) = 0;

    // 0099A1FF and its siblings: target->vtable[5Ch](kind).
    virtual bool entity_is_kind(std::uint32_t entity, int kind) = 0;
    // 0099A2CE, 0099A41E: 009229F0, the shared "target still attackable" test.
    virtual bool target_still_attackable(std::uint32_t target) = 0;

    // 0099A21F: FUN_007EEC50, the chooser, when the class is attackmove.
    virtual std::uint32_t choose_attack_command(std::uint32_t unit,
                                                std::uint32_t target,
                                                bool prefer_ordnance,
                                                bool allow_guns) = 0;
    // 0099A22A: FUN_007F16D0, the returntobase resolver.
    virtual std::uint32_t resolve_return_to_base(std::uint32_t unit) = 0;

    // The per-class factories 0099A170 calls, one address each. The
    // implementation picks by class; the host only has to build the task.
    virtual std::uint32_t create_bot_task(std::uint32_t unit,
                                          std::uint32_t command,
                                          std::uint32_t target) = 0;
    // 0099A46A: 0099A020, install the task the factory returned.
    virtual void install_bot_task(std::uint32_t unit, std::uint32_t task) = 0;

    // 009F3F50 group gate reached from 0099A448's land arm: 006BCD20 then
    // 006C4790 over the director.
    virtual bool land_group_available(std::uint32_t unit) = 0;
};

// 0099A170. The whole routine: read the current class, rewrite land without a
// target into returntobase, resolve attackmove and returntobase into a concrete
// class, then build and install that class's task. Returns the installed task,
// or 0 for every path that returns early.
std::uint32_t bot_install_command_task_0099a170(std::uint32_t unit,
                                                AttackCommandHost& host);

// 007F16D0's outcome, as the three values the routine can leave behind.
enum class ReturnToBaseOutcome : int {
    kIssuedLandAtOwnBase = 0,  // 007F1878, FUN_007F1000(land, unit+404h)
    kIssuedLandAtCarrier = 1,  // 007F18C6, FUN_007EF8B0(land, 00465080(...))
    kBuiltRetreat = 2,         // 007F19xx, *out = retreat
};
ReturnToBaseOutcome return_to_base_outcome(bool has_assigned_base,
                                           bool base_is_reachable,
                                           bool carrier_slot_available) noexcept;

} // namespace bsp
