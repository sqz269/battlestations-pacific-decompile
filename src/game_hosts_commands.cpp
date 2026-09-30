// bsp_game.exe milestone 2l: the authored scene command, run through the path
// the game runs it through.
//
// Milestone 2i turned the `Command = E CommandType : Cruise` token every
// DestroyerGen of usn_2_java.scn carries into one order-ring order of throttle 1
// and rudder 0, and said so. docs/CRUISE_COMMAND.md recovered what the token
// means, and it is a latch rather than an order: when `cruise` becomes the
// unit's current command it captures the ring's ordered pair and the unit's
// heading, and every step afterwards it re-applies what it captured. A ship
// whose ring is still zero therefore holds zero.
//
// What runs here is recovered: 0046aab0's resolve, 0077d600's issue,
// 00816e30's movement fall-through, 0071ecf0's hop, 00721a40's 5Ch arm,
// 008358d0 and 0071e6c0's slot push, 0071be40's current-command read,
// 00835c70's `cruise` arm with the latch 00835ac0, and 009e1170's AI arm.
// What this file supplies, and labels, is listed in
// include/bsp/game_hosts_commands.hpp.

#include "bsp/game_hosts_commands.hpp"
#include "bsp/game_hosts_ship_ai.hpp"

#include "bsp/game_hosts.hpp"

#include "bsp/command_completion.hpp"
#include "bsp/command_execution.hpp"
#include "bsp/entity_orders.hpp"
#include "bsp/entity_command_arms.hpp"
#include "bsp/weapon_director.hpp"
#include "bsp/ship_ai_path_cursor.hpp"
#include "bsp/unit_kind_query.hpp"
#include <algorithm>
#include <array>
#include <cmath>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// Packet cc9_ai_retask, docs/AI_RETASK.md. True: a re-registration of the unit
// table keeps each existing unit's formation pair (unit+284h, 007788B0 /
// 007788D0), so a released escort's idle tail issues `follow` (00836E0D..
// 00836E90). False: the pair is cleared by every later spawn, as before.
constexpr bool kAiRetaskBound = true;

// Packet cc9_ship_natives_2, docs/SHIP_NATIVES_2.md. True: 00836920's per-command
// arms the host only recorded run: the generic arrival 008369A0..00836A81, the
// stage-2 skip 00836A85, and the `follow` arm 00836ADC..00836B40. The `stop`
// arm was already bound, `attackmove` by packet cc9_target_release, `moveonpath`
// by packet cc8_ship_moveonpath, and the override arm 00836D67 cannot fire here
// because nothing writes director+188h. False: the record, as before.
constexpr bool kDirectorCommandArmsBound = true;
// Packet cc9_unresolved_fire_target (docs/GUNNERY_OPEN_ITEMS.md section 33).
// True: 008358D0's forced fire target (00835930 -> 00835860) resolves a
// descriptor whose object is a producer's handle, not one of this host's unit
// records, by its object id (+2h), as resolve_target_00521ea0 does. LABELLED:
// the handle stands in for the entity pointer the image carries. False: such a
// target is dropped and the director keeps its own pick.
// ON by the pairs of 2026-09-28: JM06 moves (the two PlayerSubs fire on their
// commanded targets); USN02, USN04, USN13, USN01 and LOMP06 gameplay identical.
constexpr bool kFireTargetObjectIdBound = true;

// Packet cc9_squadron_set_command (docs/GUNNERY_OPEN_ITEMS.md section 45). True: a
// row answering IsKindOf(0Fh) or IsKindOf(18h) takes the squadron controller's
// SetCommand, slot +60h of 00D0BD98, which is 0071E6C0 alone: no category test and
// no forced fire target 00835930. This host fuses a squadron with its leader plane
// (a 0Fh row), whose ship-style director ran 008358D0; a plane instance has no
// director at all (vtable[114h] 0047F180), so neither kind reaches 00835860 in the
// image. The slot push itself still calls the ship director's +14h
// (00836040) where the squadron's is 0084DD20; that difference is open.
// OFF: every row runs 008358D0 (counted).
constexpr bool kSquadronSetCommandBound = true;
// Packet cc9_scene_command_find_case (routed from cc9-lua17, main c6d14ae2f): the
// scene command's name lookup 0046AB48 -> 00925A90 compares through 009251F0's
// 00438E10 (_stricmp) as the Lua FindEntity does (kFindEntityCaseInsensitiveBound
// in src/game_hosts_lua.cpp). True: an exact miss answers the first unit whose
// name matches ignoring ASCII case. False: exact names only (the case-only
// matches are still counted). ON by the census (GUNNERY_OPEN_ITEMS 62): on the
// sixteen reference-o rows every lookup is an exact hit (case_only=0, no miss), so
// no row can move; the switch only matters where a scene names a unit in other case.
constexpr bool kSceneCommandFindCaseInsensitiveBound = true;

// Packet cc9_land_command_current (docs/SHIP_AI_OPEN_ITEMS.md section 85). 00816E30's
// land arm (00816FC6..00816FD9) asks the unit's own IsKindOf(0Ch), [EDI]+5Ch with
// EDI = the unit (00816E6A MOV EDI,ECX): an MLandingShip keeps `land`, anything
// else gets `attackmove` (00816FD9 MOV EBP,00E08F78). True: the answer comes from
// the unit's recovered class chain. False: every unit answers false, a record, so
// a landing ship's `land` became an attackmove and never reached the land state.
// ON by section 85.4: both JM08 land rows became current (slot=1, curr=1).
constexpr bool kEntityCommandSelfKindBound = true;

// [00e188a8]+1fe4h. The single-player value, which is what every other host in
// this executable already reports for the same field.
constexpr int kSessionModeSinglePlayer = 1;

// The three command objects whose apply takes 00816e30's movement fall-through:
// the test at 00816f5e..00816f7a is false for them, so 00816f7c..00817330 does
// not run and control reaches the tail at 00817334. Every other command has an
// arm in that block, and none of those arms is projected.
bool command_takes_movement_fall_through(std::uint32_t object) noexcept {
    return object == 0x00e08f70u    // cruise
        || object == 0x00e08f88u    // stop
        || object == 0x00e08f80u;   // moveonpath
}

const char* outcome_name(bsp::SceneCommandOutcome outcome) noexcept {
    switch (outcome) {
        case bsp::SceneCommandOutcome::kIssued: return "issued";
        case bsp::SceneCommandOutcome::kUnknownCommandName: return "unknown_command_name";
        case bsp::SceneCommandOutcome::kNullCommandObject: return "null_command_object";
        case bsp::SceneCommandOutcome::kCommandRequiresTarget: return "requires_target";
        case bsp::SceneCommandOutcome::kTargetNotFound: return "target_not_found";
    }
    return "unknown";
}

}  // namespace

// ---------------------------------------------------------------------------
// The weapon director this process owns for one unit
// ---------------------------------------------------------------------------

// Only the fields the command path reads. The 250h-byte native object is
// docs/WEAPON_DIRECTOR.md's; this holds its command slots (director+54h with
// stride 1ch), the command mode at +30h, the stage counter at +48h and the
// three cruise autopilot fields at +243h / +244h / +248h.
struct GameDirector {
    // The existing WeaponDirectorState defaults carry the unconditional
    // 00836724/0083672A/00836730 stores. Project only these three bytes: this
    // does not invoke or claim the full endpoint/subobject constructor.
    GameDirectorAvoidance avoidance = [] {
        const bsp::WeaponDirectorState constructed{};
        return GameDirectorAvoidance{constructed.torpedo_avoidance,
            constructed.ship_collision_avoidance, constructed.land_collision_avoidance};
    }();
    std::uint32_t slot_command[bsp::kDirectorCommandSlotCount]{};
    bsp::SceneCommandTarget slot_target[bsp::kDirectorCommandSlotCount]{};
    bsp::CruiseCommandMode mode{bsp::CruiseCommandMode::None};
    int stage{0};
    bsp::CruiseAutopilotFields cruise{};
    bool latched{false};
    // Milestone 2m: the last default command 00836dc9's idle tail chose for this
    // director. The tail runs every step and re-issues the same command while
    // nothing changes, so the executable records one row per distinct choice
    // rather than one per step.
    std::uint32_t last_idle_command{0};
    // Milestone 2p: director+40h, the auto-target re-acquisition hold
    // 0071DF70 tests first. 00720225 stores -1.0f in the command controller
    // base constructor 00720180, which 008363E0 calls at 00836403 with ECX
    // still the director (docs/DIRECTOR_TARGET_GATE.md), so every director in
    // this process starts there. 0071F314's per-frame `hold -= dt` runs only
    // while the value is at or above 0.0f, so the -1.0f sentinel never moves,
    // and 00817031's 3.0f is the `cleartarget` arm, which this mission never
    // issues.
    float target_hold_0040{-1.0f};
    // Packet cc8_ship_moveonpath: the slot-0 path object of director+1A4h, the
    // one 0071BFF0(director, 0) answers with and the only one both 009E59C0 and
    // 00836920's `moveonpath` arm ever ask for. The native allocates ten of
    // them; this host carries the one that is read.
    bsp::ShipAiPathCursor path_cursor{};
    std::vector<std::array<float, 3>> path_points;   // the path source's points
    std::string path_name;
    int path_follow_mode{0};   // command+8h, 0071C1B0's first store
    int path_start_mode{0};    // command+0Ch, 0071C1B0's second store
    bool path_built{false};
    unsigned long long path_advances{0};
    int path_start_index{0};
    float path_travelled{0.0f};
    float path_last_x{0.0f};
    float path_last_z{0.0f};
    bool path_last_valid{false};
    std::vector<int> path_visited;   // the legs reached, in order
    // Packet cc8_ship_drive. The path the 5Bh message named for the command it
    // queued, held until the director BEGINS that command. 0071F600 resolves
    // the path entity from the slot descriptor itself (0071F63A LEA EBX,
    // [ESI+58h] then 00521EA0), so the source belongs to the slot and not to
    // the message; this host carries the last one named, which is the same path
    // on all 49 repeats a USN04 carrier receives.
    std::vector<std::array<float, 3>> pending_path_points;
    std::string pending_path_name;
    bool pending_path_valid{false};
    // Whether the build has already run for the begin the queue head is in.
    // Cleared as soon as slot 0 stops holding `moveonpath`.
    bool path_begun{false};
    // Packet cc9_director_moveonpath_route: the point vector +40h..+48h of the
    // slot object the last user `moveonpath` was queued with (0071FDE0), and
    // whether slot 0's cursor was built over it (0071F600's descriptor-kind-0
    // arm), which is 0071FC40's "attached". LABELLED: the host keeps one user
    // path per director, the one the last 5Fh named; the image keeps one per
    // slot object.
    std::vector<std::array<float, 3>> user_points;
    bool path_user{false};
    unsigned long long user_points_received{0};
    unsigned long long user_paths_queued{0};
    unsigned long long user_points_dropped{0};
    unsigned long long user_points_outside_map{0};
};

struct GameCommandsHost::Impl {
    explicit Impl(GameHostLog& log_in) : log(log_in) {}

    GameHostLog& log;
    std::vector<GameCommandUnit> units;
    std::vector<GameFireTargetRequest> fire_target_requests;   // cc9_weapon_director_fire_target
    std::vector<GameDirector> directors;
    // Packet cc9_target_release.
    const GameCommandTargetFactsSource* target_facts{nullptr};
    // Packet cc9_director_target_checks: the entity's +5Dh as 0071D712 reads
    // it, one byte per unit, set when 0071DDB0 is delivered for the unit.
    // SUBSTITUTION: the image sets +5Dh at 00926390 / 009263C0 in the fixed
    // step's destroy flush (00875EC9); this host sets it at the gunnery kill,
    // which runs after every director step of the same fixed step and before
    // that flush. No push reaches 0071D6D0 between the two in this process.
    std::vector<unsigned char> released_05d;
    // 00521EA0 on a descriptor, as GameCommandsHost::resolve_command_target_00521ea0.
    std::uint32_t resolve_target_00521ea0(const bsp::SceneCommandTarget& target) const {
        if (target.kind == 0 || target.object_id == 0) return 0u;
        for (const GameCommandUnit& unit : units) {
            if (unit.object_id == target.object_id)
                return static_cast<std::uint32_t>(unit.index) + 1u;
        }
        return 0u;
    }
    bool target_released_05d(std::uint32_t resolved) const {
        return resolved != 0u && resolved - 1u < released_05d.size()
            && released_05d[resolved - 1u] != 0;
    }
    // Packet cc9_command_extra_tests: 0071D71F..0071D76C, the answer 0071D6D0 gives
    // once the target checks passed (or were not required).
    bool command_extra_test_0071d71f(std::uint32_t command,
                                     const bsp::SceneCommandTarget& target) {
        if (command == 0x00e08f18u) {                                 // 0071D71F
            ++summary.torpedo_tests;
            // 0071D729 00521EA0, then 009229F0(entity, 6).
            const std::uint32_t resolved = resolve_target_00521ea0(target);
            bool accept = false;
            if (resolved == 0u || resolved - 1u >= units.size()) {
                ++summary.torpedo_refused_null;                       // 009229F8
            } else {
                const int class_id = units[resolved - 1u].class_id;
                if (class_id < 0) {
                    // LABELLED: no class id to test; accepted and counted.
                    ++summary.torpedo_class_unknown;
                    accept = true;
                } else if (bsp::unit_is_kind_of(class_id, 6)) {      // 00922A00
                    accept = true;
                } else if (bsp::unit_is_kind_of(class_id, 0x1b)) {   // 00922A14
                    // 00922990([class+178h] FakedType, 6). No class row in this
                    // installation authors FakedType (every scripts\ .lua searched,
                    // GUNNERY_OPEN_ITEMS 76.5), so every fort holds the default 1Bh
                    // (00749684), outside the set: refused, as the image does here.
                    ++summary.torpedo_fort_unread;
                } else {
                    ++summary.torpedo_refused_kind;
                }
            }
            if (!kCommandExtraTestsBound) {
                record("WeaponDirector::command_extra_test_torpedo", 0x009229f0u);
                return true;
            }
            done("WeaponDirector::command_extra_test_torpedo", 0x009229f0u);
            return accept;
        }
        if (command == 0x00e08f80u && target.kind != 0) {             // 0071D73E..0071D749
            ++summary.path_tests;
            // 0071D74D 00521EA0, then 007AC9D0: a path kind (47h..4Ah). SUBSTITUTION: this
            // host resolves units only, and no unit is a path kind; a descriptor that names
            // no unit is taken as the authored Path it carries.
            const std::uint32_t resolved = resolve_target_00521ea0(target);
            const bool accept = resolved == 0u;
            if (accept) ++summary.path_non_unit; else ++summary.path_refused_unit;
            if (!kCommandExtraTestsBound) {
                record("WeaponDirector::command_extra_test_path", 0x007ac9d0u);
                return true;
            }
            done("WeaponDirector::command_extra_test_path", 0x007ac9d0u);
            return accept;
        }
        return true;                                                   // 0071D76D
    }
    // Milestone 2m. One navigator parameter block per unit, the 0081f283
    // allocation at *(unit+73Ch). Only the commanded-speed pair at +24h / +28h
    // has a recovered producer, and it is the pair the director's stage reset
    // 00835bf0, its `stop` arm 00836a8b, its idle tail 00836e59 and the cruise
    // state 009e12ac all read. The seven tuning floats at +0h..+18h stay out of
    // this process because 00822b70's only call site here passes the literal 0.
    std::vector<bsp::CruiseSpeedSetting> navigator_params;
    std::vector<GameCommandRow> rows;
    bsp::SceneCommandRegistry registry;
    // Milestone 2q: the `command` event channel's subscriptions, the list
    // 0097C8A0 builds for every `command` event block 0097E360 parses. That
    // parser is not reconstructed and this mission authors no such block, so
    // the list is empty and 00984300 takes its own early return at 009843D8.
    std::vector<bsp::CommandEventSubscription> command_event_subscriptions;
    GameCommandsSummary summary{};
    bool logged_path{false};
    bool logged_block{false};
    bool logged_step{false};
    bool logged_director_step{false};
    bool logged_navigator_params{false};
    bool logged_commanded_step{false};

    // Packet cc8_ship_command: a bounded command-lifetime trace for the two
    // USN04 carriers, against 00836920's stage spine. Every line is one change
    // of (stage, queue head, filled slots) on one unit, tagged with the arm
    // that made it, so "what ended the running command and what began after it"
    // is read off the trace rather than inferred from the summary counters.
    struct CommandLifeState {
        int stage{-1};
        std::uint32_t head{0u};
        int slots{-1};
        bool seen{false};
    };
    std::vector<CommandLifeState> life;
    int life_lines{0};
    // The last clock the director step was given. An issue that arrives between
    // two steps is stamped with the previous step's clock, which is within one
    // fixed step of the truth and is enough to order arrivals against ends.
    float life_clock{0.0f};

    bool life_traced(std::size_t index) const {
        if (index >= units.size()) return false;
        const std::string& name = units[index].name;
        return name == "Yorktown-class01" || name == "Lexington-class01";
    }
    const char* life_command_name(std::uint32_t object) {
        if (object == 0u) return "(none)";
        const bsp::EntityOrderCommandClass* klass = class_of(object);
        return klass != nullptr ? klass->name : "?";
    }
    void life_emit(std::size_t index, const char* arm, float clock,
                   const GameDirector& director, bool force = false) {
        if (!life_traced(index)) return;
        if (life.size() < directors.size()) life.resize(directors.size());
        CommandLifeState& prev = life[index];
        const int slots = command_count(director);
        const std::uint32_t head = director.slot_command[0];
        if (!force && prev.seen && prev.stage == director.stage && prev.head == head
            && prev.slots == slots) {
            return;
        }
        if (life_lines < 4000) {
            ++life_lines;
            log.notef("  cmdlife %8.2fs %-17s %-22s stage %d -> %d   head %s -> %s   "
                "slots %d -> %d", static_cast<double>(clock), units[index].name.c_str(),
                arm, prev.seen ? prev.stage : -1, director.stage,
                prev.seen ? life_command_name(prev.head) : "-",
                life_command_name(head), prev.seen ? prev.slots : -1, slots);
        }
        prev.stage = director.stage;
        prev.head = head;
        prev.slots = slots;
        prev.seen = true;
    }

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
    void record_slot(const char* method, const char* slot) {
        log.unimplemented(method, slot);
    }

    void build_registry() {
        if (!registry.empty()) return;
        // The 26 rows of docs/SCENE_COMMAND_TYPES.md, in registration order,
        // which is the order 0046aab0's first-match walk sees them in. The
        // identity is the native object's address: this process never
        // dereferences it, exactly as the reconstruction's contract says.
        registry.reserve(static_cast<std::size_t>(bsp::kEntityOrderCommandCount));
        for (int i = 0; i < bsp::kEntityOrderCommandCount; ++i) {
            const bsp::EntityOrderCommandClass& klass = bsp::kEntityOrderCommandClasses[i];
            bsp::SceneCommandType type;
            type.identity = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(klass.object_address));
            type.name = klass.name;
            type.requires_target = klass.requires_target;
            registry.push_back(type);
        }
    }

    std::uint32_t object_of(const void* identity) const noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(identity));
    }

    const bsp::EntityOrderCommandClass* class_of(std::uint32_t object) const noexcept {
        return bsp::entity_order_command_class_by_address(object);
    }

    bsp::CruiseSpeedSetting params_of(std::size_t index) const noexcept {
        if (index >= navigator_params.size()) return bsp::CruiseSpeedSetting{};
        return navigator_params[index];
    }

    // The store at 00835c54 and 009e13f8 into the block's +28h.
    void set_commanded_speed_time(std::size_t index, float value) noexcept {
        if (index >= navigator_params.size()) return;
        navigator_params[index].enable = value;
    }

    // Milestone 2q: 0071D810's stage-2 consequence, for both of its call sites.
    // Defined below the two bindings it needs.
    bool route_clear_command(std::size_t unit_index, bool player_controlled);
    // The same round trip for any 5Dh message (0071D900's slot clear included).
    bool route_clear_command(std::size_t unit_index, bool player_controlled,
                             const bsp::ClearCommandMessage& message);

    // Packet cc8_ship_drive: the one-time note that says what the weighted
    // 0071D780 answers next to the unweighted walk every index site runs.
    bool logged_weighted_count{false};
    // Packet cc9_unresolved_fire_target: 00835930 calls whose object is not
    // one of this host's unit records, and how many of them name a unit by the
    // descriptor's object id (+2h) all the same.
    unsigned long long building_arm_hits{0};     // cc9_attackmove_building_arm
    // cc9_scene_command_find_case: 00925A90 lookups, exact hits, and misses that
    // only a case-folded (_stricmp) comparison matches.
    unsigned long long find_lookups{0};
    unsigned long long find_exact{0};
    unsigned long long find_case_only{0};
    unsigned long long building_arm_converts{0};
    int building_arm_traced{0};
    unsigned long long queue_full_tests{0};             // cc9_director_slot_housekeeping
    unsigned long long queue_full_tests_queued_path{0};
    unsigned long long fire_unresolved{0};
    unsigned long long fire_unresolved_by_id{0};
    unsigned long long squadron_set_commands{0};   // plane or squadron rows at 008358D0
    unsigned long long squadron_fire_targets{0};   // their 00835930 calls (0 while ON)
    int fire_unresolved_traced{0};
    bool logged_queue_full{false};
    void note_weighted_command_count(int weighted, int unweighted) {
        if (weighted != unweighted && !logged_weighted_count) {
            logged_weighted_count = true;
            log.notef("0071d780's `moveonpath` weighting is live: the queue-full test at "
                "0071e6c8 sees %d where the unweighted walk every index site runs "
                "(0071e6d6, 0071e56b) sees %d. The test is CMP EAX,0xa, so a push is "
                "refused only once the weighted total reaches 10",
                weighted, unweighted);
        }
        if (weighted >= bsp::kDirectorCommandSlotCount && !logged_queue_full) {
            logged_queue_full = true;
            log.notef("0071e6c8 refused a push: the weighted command count reached %d",
                weighted);
        }
    }

    int command_count(const GameDirector& director) const noexcept {
        // 0071E6D6..0071E6EE and 0071E56B..0071E57E, the unweighted walk from
        // director+54h with stride 1Ch that stops at the first null command:
        // the index of the first empty slot. This is what every site that wants
        // a SLOT INDEX runs. The weighted 0071D780, whose `moveonpath` slots
        // count as their point count, is the queue-full test alone and lives on
        // DirectorBinding::command_count.
        int index = 0;
        while (index < bsp::kDirectorCommandSlotCount
            && director.slot_command[index] != 0) {
            ++index;
        }
        return index;
    }

    // Packet cc9_set_command_queue_delay: the loopback vector session+24Ch
    // (count +250h, insertion pointer +258h) and what one entry carries. The
    // native entry is the message object itself; this host's entry adds what
    // its chain needs at delivery: the command row by index into `rows` (the
    // vector can grow between post and delivery), the unit's order ring and the
    // heading the caller read, and whether the chain's finish tail (the
    // current-command read and 00835C70's arm) runs at the end of this delivery.
    static constexpr std::size_t kNoLoopbackRow = static_cast<std::size_t>(-1);
    enum class LoopbackKind { Command, SetCommand, Clear, UserPathPoint };
    struct LoopbackMessage {
        LoopbackKind kind{LoopbackKind::Command};
        std::size_t unit{0};
        std::size_t row{kNoLoopbackRow};
        const bsp::UnitOrderRing* ring{nullptr};
        float heading{0.0f};
        bool finish{false};
        bool script_issue{false};
        bool nested{false};                 // posted by a delivery: next in the drain
        unsigned long long posted_before_drain{0};
        // A target object is one of this host's unit records; `units` is
        // replaced by register_units, so the entry keeps the one-based index.
        std::size_t target_handle{0};
        bsp::EntityOrderMessage command{};   // MT_COMMAND (0077D600)
        std::uint32_t set_command{0};        // MT_GAMEUNIT_SETCMD (0071C830)
        bsp::SceneCommandTarget set_target{};
        std::uint8_t set_flag{0};
        bool player{false};                  // the 5Dh clear (0071C730 / 0071D900)
        bsp::ClearCommandMessage clear{};
        // The receiver side a Lua binding runs once its order is delivered
        // (GameScriptOrdersHost::after_order_delivery): handed along with the
        // finish tail and run after it.
        std::function<void()> after_delivery;
        float user_point[3]{0.0f, 0.0f, 0.0f};  // MT_GAMEUNIT_ADDUSERPATHPOINT +20h
        bool user_outside_map{false};
    };
    // Where the last post went: `loopback` while it waits for the drain, or the
    // list being drained when a delivery inserted it (+258h). Null when it was
    // delivered at once. The last issue's MT_COMMAND keeps the same pair; a
    // drained list clears it when the list ends.
    std::vector<LoopbackMessage>* last_post_list{nullptr};
    std::size_t last_post_index{kNoLoopbackRow};
    std::vector<LoopbackMessage>* last_issue_list{nullptr};
    std::size_t last_issue_index{kNoLoopbackRow};
    std::vector<LoopbackMessage> loopback;
    std::vector<LoopbackMessage>* loopback_active{nullptr};
    // unit+184h as the last director step of each unit was given it; what the
    // 5Dh receiver's QueueClearBinding answers for a clear posted from hop 1.
    std::vector<unsigned char> player_184;
    bool player_of(std::size_t unit) const noexcept {
        return unit < player_184.size() && player_184[unit] != 0;
    }
    std::size_t loopback_insert{0};
    bool loopback_drain_open{false};
    unsigned long long loopback_drain_serial{0};

    std::size_t unit_handle_of(const void* object) const noexcept {
        if (object == nullptr) return 0;
        for (std::size_t i = 0; i < units.size(); ++i) {
            if (&units[i] == object) return i + 1;
        }
        return 0;
    }
    const void* unit_object_of(std::size_t handle, const void* fallback) const noexcept {
        if (handle == 0 || handle - 1 >= units.size()) return fallback;
        return &units[handle - 1];
    }
    std::size_t row_index_of(const GameCommandRow* row) const noexcept {
        if (row == nullptr || rows.empty()) return kNoLoopbackRow;
        if (row < rows.data() || row >= rows.data() + rows.size()) return kNoLoopbackRow;
        return static_cast<std::size_t>(row - rows.data());
    }
    // 0076E520. Answers the entry's index in `loopback` when it waits there for
    // the next drain, kNoLoopbackRow when a drain delivers it.
    std::size_t post_loopback(LoopbackMessage message);
    void run_loopback_list(std::vector<LoopbackMessage>& list);
    void deliver_loopback(const LoopbackMessage& message);
    // 00721A40's 5Dh arm on delivery, the old body of route_clear_command.
    bool apply_clear_command(std::size_t unit_index, bool player_controlled,
                             const bsp::ClearCommandMessage& message);
    // 00721A40's 5Fh arm with the presence byte set, 007207C0.
    void apply_user_path_point_007207c0(std::size_t unit_index, const float point[3],
                                        bool outside_map);
};

namespace {

// Everything one issued command carries while the three hops run.
struct ChainState {
    GameCommandsHost::Impl& owner;
    GameCommandUnit& unit;
    GameDirector& director;
    GameCommandRow* row{nullptr};
    const bsp::UnitOrderRing* ring{nullptr};
    float heading{0.0f};
    // The message in flight. 007798d0 writes the ordinal at +20h and the flags
    // at +21h; 0071c830 swaps the two bytes, which is why the second hop reads
    // its command from +21h through 007216d0.
    bsp::SceneCommandTarget pending_target{};
    std::uint32_t pending_command{0};
    std::uint8_t pending_flag{0};
    // Milestone 2n: the AI controller's control block, blk = brain+8h, and the
    // two callees the mode switches make. When they are present the three
    // desired-value setters run their reconstructions instead of recording.
    bsp::ShipAiControlBlock* ai_block{nullptr};
    bsp::ShipAiSetterHost* ai_setters{nullptr};
    bsp::ShipAiAvoidanceRequest* avoidance_request{nullptr};
    const bsp::ShipAiCruiseAvoidanceInputs* avoidance_inputs{nullptr};
    // Packet cc9_set_command_queue_delay. The finish tail still owed at the end
    // of the chain's last delivery; a post hands it to the message it posts.
    bool finish_pending{false};
    bool script_issue{false};
    // The `loopback` index of a SETCMD this chain queued for the next drain.
    std::size_t setcmd_post{GameCommandsHost::Impl::kNoLoopbackRow};
    // The same for the chain's MT_COMMAND, and the receiver continuation that
    // travels with the finish tail.
    std::vector<GameCommandsHost::Impl::LoopbackMessage>* command_list{nullptr};
    std::size_t command_index{GameCommandsHost::Impl::kNoLoopbackRow};
    std::function<void()> after_delivery;
};

void publish_cruise_avoidance(ChainState& chain) {
    if (chain.avoidance_request == nullptr) return;
    bsp::ShipAiAvoidanceRequestBlock block{*chain.avoidance_request,
        chain.ai_block->early_out_3f5};
    bsp::ship_ai_cruise_step_request_009e11d6(block, *chain.avoidance_inputs);
    *chain.avoidance_request = block.request;
    chain.ai_block->early_out_3f5 = block.early_out_3f5;
    chain.avoidance_request = nullptr; // each cruise step publishes once
    chain.owner.done("CruiseState::publish_avoidance_request", 0x009e12ebu);
}

// ---------------------------------------------------------------------------
// bsp::SceneDeferredReferenceHost, one method per call site inside 0046aab0
// ---------------------------------------------------------------------------

class SceneResolveBinding final : public bsp::SceneDeferredReferenceHost {
public:
    explicit SceneResolveBinding(ChainState& chain) : chain_(chain) {}

    // 0046aba2, the matched object's vtable[8]. Every derived body is
    // `MOV AL,1 ; RET` or `XOR AL,AL ; RET`, so the registry row carries it.
    bool command_requires_target(void* command) override {
        const bsp::EntityOrderCommandClass* klass
            = chain_.owner.class_of(chain_.owner.object_of(command));
        chain_.owner.done("SceneCommand::command_requires_target", 0x006f7f40u);
        return klass != nullptr && klass->requires_target;
    }

    // 0046aba8: the owner's pose byte +c8h. A created instance is published
    // with its world matrix valid, so the refresh below is not reached.
    bool owner_pose_is_current(void* owner) override {
        static_cast<void>(owner);
        return true;
    }
    void refresh_owner_pose(void* owner) override {
        static_cast<void>(owner);
        chain_.owner.record("SceneCommand::refresh_owner_pose", 0x00414db0u);
    }
    // 0046abb7 / 0046abc5 / 0046abd8: the owner's world position at +fch, +100h
    // and +104h, which is pose row 3 of the instance the placement wrote.
    void owner_world_position(void* owner, float out[3]) override {
        const GameCommandUnit* entity = static_cast<const GameCommandUnit*>(owner);
        for (int lane = 0; lane < 3; ++lane) {
            out[lane] = (entity != nullptr) ? entity->position[lane] : 0.0f;
        }
        chain_.owner.done("SceneCommand::owner_world_position", 0x0046abb7u);
    }

    // 0046ab48: 00925a90 on the scene database at [[00e188a8]+19cch]. That
    // database is the scene-graph owner's, so the lookup is the executable's
    // own walk over the instances the instantiate pass created, and the native
    // routine is recorded.
    void* find_entity_by_name(const std::string& name) override {
        chain_.owner.record("SceneCommand::find_entity_by_name", 0x00925a90u);
        ++chain_.owner.find_lookups;
        for (GameCommandUnit& unit : chain_.owner.units) {
            if (unit.name == name) {
                ++chain_.owner.find_exact;
                return &unit;
            }
        }
        // 00925A90 hands each registry entry to 009251F0, whose name test
        // (0092521E) is 00438E10, a null-guarded CRT _stricmp over the whole name,
        // the same rule kFindEntityCaseInsensitiveBound binds for the Lua
        // FindEntity. LABELLED: the registry's walk order is not modelled; an
        // exact hit wins, then the first case-folded match in unit order.
        for (GameCommandUnit& unit : chain_.owner.units) {
            if (_stricmp(unit.name.c_str(), name.c_str()) == 0) {
                ++chain_.owner.find_case_only;
                if (kSceneCommandFindCaseInsensitiveBound) return &unit;
                break;
            }
        }
        return nullptr;
    }
    // 0046ab8d: the resolved entity's uint16 at +174h.
    std::uint16_t entity_object_id(void* entity) override {
        const GameCommandUnit* unit = static_cast<const GameCommandUnit*>(entity);
        return (unit != nullptr) ? unit->object_id : static_cast<std::uint16_t>(0);
    }

    // 0046ac0b: 0077d600(ECX = owner, command, &target, 1).
    void issue_command(void* owner, void* command, const bsp::SceneCommandTarget& target,
        int flags) override;

    // 0046ac34: the tail JMP to 0046a9f0. The one-record queue this file builds
    // is a local, so the erase is its destruction.
    void clear_queue() override {
        chain_.owner.done("SceneCommand::clear_queue", 0x0046a9f0u);
    }

private:
    ChainState& chain_;
};

// ---------------------------------------------------------------------------
// bsp::EntityOrderHost, one method per call site inside 0077d600
// ---------------------------------------------------------------------------

class EntityIssueBinding final : public bsp::EntityOrderHost {
public:
    explicit EntityIssueBinding(ChainState& chain) : chain_(chain) {}

    // The inline copy of 00521ea0 at 0077d676. The two 16-byte-entry handle
    // tables at 00f89a0c / 00f89a60 with the split at 00f89a10 are the session
    // owner's and are not built here, so the descriptor's own object pointer is
    // what a resolve can answer and the native lookup is recorded.
    void* resolve_target_object(bsp::SceneCommandTarget& target) override {
        chain_.owner.record("EntityOrder::resolve_target_object", 0x00521ea0u);
        return target.object;
    }
    bool object_is_kind_of(void* object, int class_id) override {
        static_cast<void>(object);
        static_cast<void>(class_id);
        chain_.owner.record("EntityOrder::target_is_kind_of", 0x006fe530u);
        return false;
    }
    const char* command_name(void* command) override {
        const bsp::EntityOrderCommandClass* klass
            = chain_.owner.class_of(chain_.owner.object_of(command));
        chain_.owner.done("EntityOrder::command_name", 0x006f8a30u);
        return (klass != nullptr) ? klass->name : "";
    }
    // 0041e870 then 00419cc0 / 00bd1510: the pooled string 0077d600 builds from
    // that name and frees again at 0077d767 without reading it.
    void note_command_name_string(const char* name) override {
        static_cast<void>(name);
        chain_.owner.record("EntityOrder::pool_command_name", 0x0041e870u);
    }
    void* retarget_object(void* object) override {
        static_cast<void>(object);
        chain_.owner.record("EntityOrder::retarget_object", 0x0077d6fau);
        return nullptr;
    }
    std::uint16_t object_id(void* object) override {
        const GameCommandUnit* unit = static_cast<const GameCommandUnit*>(object);
        return (unit != nullptr) ? unit->object_id : static_cast<std::uint16_t>(0);
    }
    int command_ordinal(void* command) override {
        const bsp::EntityOrderCommandClass* klass
            = chain_.owner.class_of(chain_.owner.object_of(command));
        return (klass != nullptr) ? klass->ordinal : 0;
    }
    int attackmove_ordinal() override { return bsp::kEntityOrderAttackMoveOrdinal; }

    // entity+16ch, 0077d787. Nothing in this process writes it: its only
    // producer in the reconstruction is the `AICreateGroup` binding 00a38a50
    // through 00a2dfa0 / 00a2d8e0, and no installed mission script calls it.
    void* entity_ai_group(void* entity) override {
        static_cast<void>(entity);
        return nullptr;
    }
    void* entity_controller(void* entity) override {
        static_cast<void>(entity);
        return nullptr;
    }
    void* controller_owner(void* controller) override {
        static_cast<void>(controller);
        return nullptr;
    }
    // 00a2bd90 at 0077d7a3. Not reached here, because the entity has no group.
    void ai_group_forward_command(void* ai_group, void* command,
        const bsp::SceneCommandTarget& target) override {
        static_cast<void>(ai_group);
        static_cast<void>(command);
        static_cast<void>(target);
        chain_.owner.record("EntityOrder::ai_group_forward_command", 0x00a2bd90u);
        if (chain_.row != nullptr) chain_.row->ai_group_notified = true;
        ++chain_.owner.summary.ai_forwards;
    }
    // 0075b454: the slot index at world+18ech over the eight pointers at
    // +18cch. This process has no session world object, so the slot is 0, which
    // is what session_message_player_slot_0075b454 answers for an empty array.
    std::uint32_t session_player_slot() override {
        const std::uint32_t players[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        return bsp::session_message_player_slot_0075b454(0, players);
    }

    // 0077c2a0 at 0077d7bd with the routing-flag override 0. The router, the
    // local queue on session+24ch and the drain 0076c600 / 00780670 / 00780120
    // all belong to the session owner, so each is recorded and the executable
    // hands the message to the same process synchronously.
    void route_message(void* entity, const bsp::EntityOrderMessage& message) override;

private:
    ChainState& chain_;
};

// ---------------------------------------------------------------------------
// bsp::CruiseCommandHost, one method per call site of the four director routines
// ---------------------------------------------------------------------------

class DirectorBinding final : public bsp::CruiseCommandHost {
public:
    explicit DirectorBinding(ChainState& chain) : chain_(chain) {}

    // -- 00816e30 ----------------------------------------------------------
    std::uint32_t command_object_from_message_ordinal(std::uint8_t ordinal) override {
        // 0077a050 reads the byte at message +20h and walks the registry list
        // at 00e19a70 for the first object whose +4h matches; 0ffh is the
        // "no command" ordinal.
        chain_.owner.done("EntityCommand::command_from_ordinal", 0x0077a050u);
        if (ordinal == bsp::kCruiseCommandNoOrdinal) return 0;
        const bsp::EntityOrderCommandClass* klass
            = bsp::entity_order_command_class_by_ordinal(static_cast<int>(ordinal));
        return (klass != nullptr) ? klass->object_address : 0u;
    }
    void clear_all_commands() override {
        // 0071d880 builds MT_GAMEUNIT_CLEARCMD with +04h = 1, +20h = 1 and
        // +24h = -1 and routes it with flags 7 through the endpoint at
        // director+34h, so the clear is a networked round trip. The receive arm
        // 00721a40's 5Dh case takes 00720ca0 for the negative index, and that
        // body is not projected; the executable performs the "every slot" clear
        // the -1 index names and records both halves.
        ++chain_.owner.summary.clear_all_calls;
        if (kSetCommandClearAllMessageBound) {
            // Packet cc9_set_command_clear_all: 0071D880's message, delivered
            // through 00721A40's 5Dh arm into 00720CA0 (apply_clear_command).
            chain_.owner.done("EntityCommand::clear_all_commands", 0x0071d880u);
            if (chain_.row != nullptr) chain_.row->slots_cleared = true;
            bsp::ClearCommandMessage every{};
            every.arm = 1;
            every.index = -1;
            chain_.owner.route_clear_command(chain_.unit.index,
                chain_.owner.player_of(chain_.unit.index), every);
            return;
        }
        chain_.owner.record("EntityCommand::clear_all_commands", 0x0071d880u);
        chain_.owner.record("GameUnitMessage::clear_every_slot", 0x00720ca0u);
        for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
            chain_.director.slot_command[i] = 0;
            chain_.director.slot_target[i] = bsp::SceneCommandTarget{};
        }
        if (chain_.row != nullptr) chain_.row->slots_cleared = true;
    }
    bool command_accepted(std::uint32_t command) override {
        // director vtable[34h] = 00835e90, only on the flags == 0 arm. Both
        // producers of an authored command push 1, so this is not reached; the
        // rule is here because the interface has no default.
        const int count = chain_.owner.command_count(chain_.director);
        const std::uint32_t top = (count > 0)
            ? chain_.director.slot_command[count - 1] : 0u;
        const bsp::EntityOrderCommandClass* top_class = chain_.owner.class_of(top);
        const bsp::EntityOrderCommandClass* klass = chain_.owner.class_of(command);
        // 0071c0c0: false for a null command, otherwise the first empty slot
        // index is below 10.
        const bool base_allows = command != 0 && count < bsp::kDirectorCommandSlotCount;
        chain_.owner.done("EntityCommand::command_accepted", 0x00835e90u);
        return bsp::cruise_command_accepted_00835e90(base_allows, count, top,
            (top_class != nullptr) ? top_class->category : -1,
            (klass != nullptr) ? klass->category : -1);
    }
    void director_issue_command(std::uint32_t command,
        const bsp::SceneCommandTarget& target) override;

    // -- 0071ecf0 ----------------------------------------------------------
    std::uint32_t endpoint_subject_vtable140() override {
        // [director+34h]->vtable[140h]. The session endpoint is the session
        // owner's object and this process builds none, so the whole AI-group
        // block of 0071ecf0 (0071ed19..0071ed5c) is skipped.
        chain_.owner.record_slot("EntityCommand::endpoint_subject", "00d09ec0+vtable140");
        return 0;
    }
    bool entity_is_kind_of(std::uint32_t entity, int class_id) override {
        static_cast<void>(entity);
        static_cast<void>(class_id);
        chain_.owner.record_slot("EntityCommand::endpoint_is_kind_of", "00d09ec0+vtable5c");
        return false;
    }
    std::uint32_t entity_ai_group(std::uint32_t entity) override {
        static_cast<void>(entity);
        return 0;
    }
    bool entity_controller_is_another_entity(std::uint32_t entity) override {
        static_cast<void>(entity);
        chain_.owner.record("EntityCommand::controller_is_another_entity", 0x007788b0u);
        return false;
    }
    void ai_group_forward_command(std::uint32_t ai_group, std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        static_cast<void>(ai_group);
        static_cast<void>(command);
        static_cast<void>(target);
        chain_.owner.record("EntityCommand::director_ai_group_forward", 0x00a2bd90u);
        if (chain_.row != nullptr) chain_.row->ai_group_forwarded = true;
        ++chain_.owner.summary.ai_forwards;
    }
    bool make_room_for_command(std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        static_cast<void>(target);
        const int count = chain_.owner.command_count(chain_.director);
        const bsp::EntityOrderCommandClass* klass = chain_.owner.class_of(command);
        const std::uint32_t top = (count > 0)
            ? chain_.director.slot_command[count - 1] : 0u;
        const bsp::EntityOrderCommandClass* top_class = chain_.owner.class_of(top);
        const bool dropped = bsp::cruise_make_room_0071e550(
            (klass != nullptr) ? klass->category : -1, count,
            (top_class != nullptr) ? top_class->category : -1);
        if (dropped && count > 0) ++chain_.owner.summary.drop_calls;
        if (dropped && count > 0 && kSetCommandClearAllMessageBound) {
            // Packet cc9_set_command_clear_all: 0071D900(count - 1) posts the
            // 5Dh slot clear, delivered through 00720850.
            chain_.owner.done("EntityCommand::drop_top_slot", 0x0071d900u);
            chain_.owner.route_clear_command(chain_.unit.index,
                chain_.owner.player_of(chain_.unit.index),
                bsp::clear_command_message_for_slot(count - 1));
        } else if (dropped && count > 0) {
            // 0071d900 at 0071e5aa removes the top slot.
            chain_.owner.record("EntityCommand::drop_top_slot", 0x0071d900u);
            chain_.director.slot_command[count - 1] = 0;
            chain_.director.slot_target[count - 1] = bsp::SceneCommandTarget{};
        }
        chain_.owner.done("EntityCommand::make_room", 0x0071e550u);
        return dropped;
    }
    void route_set_command_message(std::uint32_t command,
        const bsp::SceneCommandTarget& target, std::uint8_t flag) override;

    // -- 00721a40's 5Ch arm -------------------------------------------------
    bool message_is_category(int category) override {
        chain_.owner.done("GameUnitMessage::is_category", 0x0071c900u);
        return bsp::gameunit_set_command_message_is_category_0071c900(category);
    }
    bsp::SceneCommandTarget message_target_descriptor() override {
        chain_.owner.done("GameUnitMessage::target_descriptor", 0x00721030u);
        return chain_.pending_target;
    }
    std::uint32_t resolve_target_object(const bsp::SceneCommandTarget& target) override {
        if (target.object == nullptr) return 0;
        chain_.owner.record("GameUnitMessage::resolve_target_object", 0x00521ea0u);
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target.object));
    }
    std::uint32_t message_command_object() override {
        // 007216d0 reads the byte at message +21h and walks the same registry.
        chain_.owner.done("GameUnitMessage::command_from_ordinal", 0x007216d0u);
        return chain_.pending_command;
    }
    bool director_set_command(std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        bsp::SceneCommandTarget local = target;
        const bool squadron = bsp::unit_is_kind_of(chain_.unit.class_id, 0x0f)
            || bsp::unit_is_kind_of(chain_.unit.class_id, 0x18);
        squadron_row_ = squadron;
        if (squadron) ++chain_.owner.squadron_set_commands;
        if (kSquadronSetCommandBound && squadron) {
            // 00D0BDF8: the squadron controller's slot +60h is 0071E6C0.
            const bool pushed = bsp::director_push_command_slot_0071e6c0(*this, command, local);
            chain_.owner.done("WeaponDirector::squadron_set_command_0071e6c0", 0x0071e6c0u);
            return pushed;
        }
        const bool pushed = bsp::director_set_command_008358d0(*this, command, local,
            kSessionModeSinglePlayer);
        squadron_row_ = false;
        chain_.owner.done("WeaponDirector::set_command", 0x008358d0u);
        return pushed;
    }
    void director_queue_command(std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        static_cast<void>(command);
        static_cast<void>(target);
        chain_.owner.record("WeaponDirector::queue_command", 0x0071e7f0u);
    }

    // -- 008358d0 and 0071e6c0 ---------------------------------------------
    int command_category(std::uint32_t command) override {
        const bsp::EntityOrderCommandClass* klass = chain_.owner.class_of(command);
        return (klass != nullptr) ? klass->category : -1;
    }
    void set_fire_target(std::uint32_t target_object) override {
        if (squadron_row_) ++chain_.owner.squadron_fire_targets;
        // 00835930, SetCommand's call with force 1 (00835924 PUSH 1). The target
        // is resolve_target_object's pointer: one of this host's unit records.
        if (!kWeaponDirectorFireTargetBound) {
            chain_.owner.record("WeaponDirector::set_fire_target", 0x00835860u);
            return;
        }
        std::size_t plus_one = 0;
        const void* object = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(target_object));
        for (std::size_t i = 0; i < chain_.owner.units.size(); ++i) {
            if (&chain_.owner.units[i] == object) { plus_one = chain_.owner.units[i].index + 1; break; }
        }
        if (object != nullptr && plus_one == 0) {
            // Packet cc9_unresolved_fire_target, diagnostic only: what the
            // object is, by the descriptor this delivery carries.
            const bsp::SceneCommandTarget& pt = chain_.pending_target;
            const GameCommandUnit* by_id = nullptr;
            if (pt.object_id != 0) {
                for (const GameCommandUnit& u : chain_.owner.units) {
                    if (u.object_id == pt.object_id) { by_id = &u; break; }
                }
            }
            ++chain_.owner.fire_unresolved;
            if (by_id != nullptr) ++chain_.owner.fire_unresolved_by_id;
            // kFireTargetObjectIdBound: the script-order, ship-AI and plane
            // producers hand this host an opaque handle (the entity's object id
            // or index+1) where the image has the entity pointer. 00835860 sets
            // the fire target to that entity, so resolve the handle the way this
            // host resolves every descriptor at 00521EA0: by the object id (+2h).
            if (kFireTargetObjectIdBound && by_id != nullptr) {
                chain_.owner.fire_target_requests.push_back({chain_.unit.index, by_id->index + 1, true});
                chain_.owner.done("WeaponDirector::set_fire_target", 0x00835860u);
                return;
            }
            if (chain_.owner.fire_unresolved_traced < 12) {
                ++chain_.owner.fire_unresolved_traced;
                chain_.owner.log.notef("fire target unresolved (00835930): unit=%s command=%s "
                    "kind=%u object_id=%u object=%p by_id=%s (packet cc9_unresolved_fire_target)",
                    chain_.unit.name.c_str(), chain_.owner.life_command_name(chain_.pending_command),
                    static_cast<unsigned>(pt.kind), static_cast<unsigned>(pt.object_id), object,
                    by_id != nullptr ? by_id->name.c_str() : "-");
            }
            chain_.owner.record("WeaponDirector::set_fire_target_unresolved", 0x00835930u);
            return;
        }
        chain_.owner.fire_target_requests.push_back({chain_.unit.index, plus_one, true});
        chain_.owner.done("WeaponDirector::set_fire_target", 0x00835860u);
    }
    int command_count() override {
        // Packet cc8_ship_drive. 0071D780 has exactly one caller in the image:
        // 0071E6C3, inside 0071E6C0, where its answer meets `CMP EAX,0xa` and
        // nothing else (an exhaustive rel32 plus absolute-dword scan finds the
        // one CALL and no vtable slot). It is the queue-FULL test, never a slot
        // index: 0071E6C0 finds the slot it stores into with its own unweighted
        // walk at 0071E6D6..0071E6EE, and 0071E550 repeats that walk at
        // 0071E56B. So the `moveonpath` weighting belongs here and only here,
        // and Impl::command_count stays unweighted for the three index sites.
        chain_.owner.done("WeaponDirector::command_count", 0x0071d780u);
        int path_point_counts[bsp::kDirectorCommandSlotCount]{};
        // 0071D7AF..0071D7F9 reads slot i's path object at director+1A4h+i*4 and
        // takes (end - begin) / 0Ch off its vtable[8h] answer. This host carries
        // the slot-0 path object alone, which is the one 0071BFF0(director, 0)
        // answers with; slots 1..9 have no path object here and contribute the
        // 1 the native's `MOV EBX,0x1` default gives them. Named hole.
        if (chain_.director.slot_command[0] == 0x00e08f80u
            && chain_.director.path_built) {
            path_point_counts[0] = static_cast<int>(chain_.director.path_points.size());
        }
        // 0071D79C reads only the slot's command pointer, so the queue record
        // this hands over carries the ten commands and nothing else.
        bsp::CommandQueueState state{};
        for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
            state.slots[i].command = chain_.director.slot_command[i];
        }
        const int weighted = bsp::command_queue_count(state, path_point_counts);
        // Packet cc9_director_slot_housekeeping, diagnostic: a queued `moveonpath`
        // in slots 1..9 is where 0071FB90's path objects (0 here) would weigh.
        ++chain_.owner.queue_full_tests;
        for (int i = 1; i < bsp::kDirectorCommandSlotCount; ++i) {
            if (chain_.director.slot_command[i] == 0x00e08f80u) {
                ++chain_.owner.queue_full_tests_queued_path;
                if (chain_.owner.queue_full_tests_queued_path <= 8) {
                    chain_.owner.log.notef("queue full test with a queued moveonpath: unit=%s slot=%d "
                        "weighted=%d head=%s (0071D780, packet cc9_director_slot_housekeeping)",
                        chain_.unit.name.c_str(), i, weighted,
                        chain_.owner.life_command_name(chain_.director.slot_command[0]));
                }
                break;
            }
        }
        chain_.owner.note_weighted_command_count(weighted,
            chain_.owner.command_count(chain_.director));
        return weighted;
    }
    std::uint32_t slot_command(int slot_index) override {
        if (slot_index < 0 || slot_index >= bsp::kDirectorCommandSlotCount) return 0;
        return chain_.director.slot_command[slot_index];
    }
    bool slot_target_matches(int slot_index,
        const bsp::SceneCommandTarget& target) override {
        // 0071e200, body 0071e200-0071e2d9: both descriptors resolve to the
        // same object and their positions are within a squared distance of 1.0,
        // where a descriptor whose position_valid byte is clear contributes the
        // zero vector at 00f87574. The body is read in full in
        // docs/CRUISE_COMMAND.md; this is its projection.
        if (slot_index < 0 || slot_index >= bsp::kDirectorCommandSlotCount) return false;
        const bsp::SceneCommandTarget& slot = chain_.director.slot_target[slot_index];
        chain_.owner.done("WeaponDirector::slot_target_matches", 0x0071e200u);
        if (slot.object != target.object) return false;
        double squared = 0.0;
        for (int lane = 0; lane < 3; ++lane) {
            const double a = (slot.position_valid != 0)
                ? static_cast<double>(slot.position[lane]) : 0.0;
            const double b = (target.position_valid != 0)
                ? static_cast<double>(target.position[lane]) : 0.0;
            squared += (a - b) * (a - b);
        }
        return squared < 1.0;
    }
    bool command_allowed(std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        // 0071d6d0, body 0071d6d0-0071d772: when the command's vtable[8]
        // requires a target and either the descriptor's position_valid byte is
        // clear or the category is 1 or 2, the target must resolve and must not
        // carry the +5dh flag. `torpedo` and `moveonpath` add tests through
        // 009229f0 and 007ac9d0, whose bodies are unread.
        const bsp::EntityOrderCommandClass* klass = chain_.owner.class_of(command);
        chain_.owner.done("WeaponDirector::command_allowed", 0x0071d6d0u);
        if (klass == nullptr) return false;
        if (!klass->requires_target) {
            return chain_.owner.command_extra_test_0071d71f(command, target);  // 0071D6E5
        }
        if (target.position_valid != 0 && klass->category != 1 && klass->category != 2) {
            return chain_.owner.command_extra_test_0071d71f(command, target);  // 0071D6FE
        }
        if (target.object == nullptr) return false;
        // 0071D70B..0071D716: 00521EA0 again, then CMP byte ptr [EAX+5Dh],0;
        // a set byte takes 0071D718 (XOR AL,AL): the push is refused. The byte
        // is the released mirror 0071DDB0's delivery keeps (Impl::released_05d).
        const bool released =
            chain_.owner.target_released_05d(chain_.owner.resolve_target_00521ea0(target));
        if (released) ++chain_.owner.summary.target_refusals;
        if (!kDirectorTargetChecksBound) {
            chain_.owner.record("WeaponDirector::target_refuses_commands", 0x0071d74au);
            return chain_.owner.command_extra_test_0071d71f(command, target);
        }
        chain_.owner.done("WeaponDirector::target_refuses_commands", 0x0071d712u);
        if (released) return false;                                    // 0071D718
        return chain_.owner.command_extra_test_0071d71f(command, target);
    }
    bool normalize_self_target(std::uint32_t command,
        bsp::SceneCommandTarget& target) override {
        // director vtable[14h] = 00836040, body 00836040-008360bc. It rewrites
        // the descriptor in place to an empty one when the descriptor names a
        // target that resolves to the director's own endpoint at director+34h
        // and the command is `cruise` or `stop`. The endpoint is the session
        // owner's and is null here, so no descriptor resolves to it.
        chain_.owner.done("WeaponDirector::normalize_self_target", 0x00836040u);
        if (command != 0x00e08f70u && command != 0x00e08f88u) return false;
        if (target.object == nullptr) return false;
        return false;
    }
    void store_slot(int slot_index, std::uint32_t command,
        const bsp::SceneCommandTarget& target) override {
        if (slot_index < 0 || slot_index >= bsp::kDirectorCommandSlotCount) return;
        chain_.director.slot_command[slot_index] = command;
        chain_.director.slot_target[slot_index] = target;
        chain_.owner.done("WeaponDirector::store_slot", 0x0071e764u);
        if (chain_.row != nullptr) {
            chain_.row->slot_pushed = true;
            chain_.row->slot_index = slot_index;
        }
    }
    void observe_target(std::uint32_t target_object) override {
        static_cast<void>(target_object);
        // 00694A60 adds the pair (target, director+1Ch) to the observer
        // registry; its only effect is 0071DDB0's delivery at the target's
        // release, which GameCommandsHost::release_observed_target_0071ddb0
        // runs for every director (the pair set is every director whose push
        // resolved the entity, so scanning every director's slots is the same).
        if (!kDirectorTargetChecksBound) {
            chain_.owner.record("WeaponDirector::observe_target", 0x00694a60u);
            return;
        }
        chain_.owner.done("WeaponDirector::observe_target", 0x00694a60u);
    }
    bsp::CruiseCommandMode command_mode() override { return chain_.director.mode; }
    void set_command_mode(bsp::CruiseCommandMode mode) override {
        chain_.director.mode = mode;
        chain_.owner.done("WeaponDirector::set_command_mode", 0x0071e795u);
    }
    std::uint32_t override_command() override { return 0; }   // director+188h
    bool session_field_f4h_is_0_or_1() override {
        chain_.owner.record("WeaponDirector::session_echo_gate", 0x006e38e0u);
        return false;
    }
    void echo_command() override {
        chain_.owner.record("WeaponDirector::echo_command", 0x0071e2e0u);
    }

    // -- 00835c70's `cruise` arm -------------------------------------------
    void raise_command_stage(int stage) override {
        // 0071d810: director+48h only ever rises.
        if (stage > chain_.director.stage) chain_.director.stage = stage;
        chain_.owner.done("CruiseCommand::raise_command_stage", 0x0071d810u);
    }
    float unit_heading() override {
        // The unit's primary vtable slot 50h at 00835e46, a RET 0 getter with no
        // reconstruction, so the caller supplies the heading it computes from
        // the instance's own pose row 2 and the native slot is recorded.
        chain_.owner.record_slot("CruiseCommand::unit_heading", "00cfc3d0+vtable50");
        return chain_.heading;
    }
    void store_cruise_fields(const bsp::CruiseAutopilotFields& fields) override {
        chain_.director.cruise = fields;
        chain_.director.latched = true;
        chain_.owner.done("CruiseCommand::latch", 0x00835ac0u);
        if (chain_.row != nullptr) {
            chain_.row->latched = true;
            chain_.row->fields = fields;
        }
    }

    // -- 009e1170's AI arm --------------------------------------------------
    bsp::CruiseAutopilotFields cruise_fields() override {
        chain_.owner.done("CruiseState::cruise_fields", 0x008356f0u);
        return chain_.director.cruise;
    }
    float body_axis_speed() override {
        chain_.owner.done("CruiseState::body_axis_speed", 0x0092d730u);
        return body_speed;
    }
    float reference_speed() override {
        chain_.owner.done("CruiseState::reference_speed", 0x0080fc30u);
        return reference;
    }
    bsp::CruiseSpeedSetting speed_setting() override {
        // *(unit+73ch) +24h and +28h, the navigator parameter block the unit
        // constructor allocates at 0081f283. docs/UNIT_COMMANDED_SPEED.md
        // corrects docs/CRUISE_COMMAND.md's "no producer": there are two, the
        // Lua bindings 00890d30 luaMW_SetShipSpeed and 008a3600
        // luaMW_NavigatorMoveOnPath, and both make the same store. This process
        // owns the block, so the read is the field read the native makes.
        chain_.owner.done("CruiseState::commanded_speed_setting", 0x009e12acu);
        return chain_.owner.params_of(chain_.unit.index);
    }
    void set_desired_steering(float rudder) override {
        // The request stores 009E12EB..009E12FF precede either steering
        // setter, after the speed/commanded-speed reads in the compiled arm.
        publish_cruise_avoidance(chain_);
        // Milestone 2n: 009dffb0, complete in src/ship_ai_states.cpp. It clears
        // the two timers and switches the mode on a change, then stores the
        // argument clamped into [-1,+1] at blk+1D4h.
        if (chain_.ai_block != nullptr && chain_.ai_setters != nullptr) {
            bsp::ship_ai_set_desired_steering_009dffb0(*chain_.ai_block, rudder,
                *chain_.ai_setters);
            chain_.owner.done("CruiseState::set_desired_steering", 0x009dffb0u);
        } else {
            chain_.owner.record("CruiseState::set_desired_steering", 0x009dffb0u);
        }
        desired_rudder = rudder;
    }
    void set_desired_heading(float heading_radians) override {
        publish_cruise_avoidance(chain_);
        if (chain_.ai_block != nullptr && chain_.ai_setters != nullptr) {
            bsp::ship_ai_set_desired_heading_009e0040(*chain_.ai_block, heading_radians,
                *chain_.ai_setters);
            chain_.owner.done("CruiseState::set_desired_heading", 0x009e0040u);
        } else {
            chain_.owner.record("CruiseState::set_desired_heading", 0x009e0040u);
        }
        desired_heading = heading_radians;
    }
    void set_desired_throttle(float throttle) override {
        if (chain_.ai_block != nullptr) {
            bsp::ship_ai_set_desired_throttle_009dbf90(*chain_.ai_block, throttle);
            chain_.owner.done("CruiseState::set_desired_throttle", 0x009dbf90u);
        } else {
            chain_.owner.record("CruiseState::set_desired_throttle", 0x009dbf90u);
        }
        desired_throttle = throttle;
    }

    float body_speed{0.0f};
    float reference{0.0f};
    float desired_rudder{0.0f};
    float desired_heading{0.0f};
    float desired_throttle{0.0f};

private:
    ChainState& chain_;
    bool squadron_row_{false};  // the current 008358D0 runs on a plane or squadron row
};

// ---------------------------------------------------------------------------
// The three hops, written out of line because each enters the next host
// ---------------------------------------------------------------------------

void SceneResolveBinding::issue_command(void* owner, void* command,
    const bsp::SceneCommandTarget& target, int flags) {
    EntityIssueBinding issue(chain_);
    const bsp::EntityOrderIssueResult result
        = bsp::entity_issue_command_0077d600(issue, owner, command, target, flags);
    chain_.owner.done("SceneCommand::issue_command", 0x0077d600u);
    if (chain_.row != nullptr) {
        chain_.row->issued = true;
        chain_.row->ai_group_notified = result.ai_group_notified;
    }
    ++chain_.owner.summary.issued;
}

// ---------------------------------------------------------------------------
// bsp::EntityCommandArmsHost, one method per call site of 00816EA6..0081732E
// ---------------------------------------------------------------------------

class EntityCommandArmsBinding final : public bsp::EntityCommandArmsHost {
public:
    explicit EntityCommandArmsBinding(ChainState& chain) : chain_(chain) {}

    std::uint32_t resolve_target_00521ea0() override {
        // 00521EA0 returns 0 for a descriptor whose kind is 0 and otherwise
        // resolves the uint16 id through the two handle tables at 00f89a54 and
        // 00f89aa8. This process numbers its own entities, which is the
        // substitution this header already declares for entity+174h, so the
        // resolve is that numbering rather than the native tables.
        chain_.owner.done("EntityCommandArm::resolve_target", 0x00521ea0u);
        if (chain_.pending_target.kind == 0) return 0u;
        return chain_.pending_target.object_id;
    }
    bool target_is_kind_of_vtable5c(std::uint32_t, int) override {
        chain_.owner.record_slot("EntityCommandArm::target_is_kind_of", "00cfc3d0+vtable5c");
        return false;
    }
    bool self_is_kind_of_vtable5c(int kind) override {
        if constexpr (kEntityCommandSelfKindBound) {
            // 00816FC6..00816FCF; the land arm is the only caller.
            chain_.owner.done("EntityCommandArm::self_is_kind_of", 0x00816fcfu);
            return bsp::unit_is_kind_of(chain_.unit.class_id, kind);
        } else {
            static_cast<void>(kind);
            chain_.owner.record_slot("EntityCommandArm::self_is_kind_of", "00cfc3d0+vtable5c");
            return false;
        }
    }
    void request_join_formation_0077c8d0(std::uint32_t) override {
        chain_.owner.record("EntityCommandArm::request_join_formation", 0x0077c8d0u);
    }
    void call_0064a8e0() override {
        chain_.owner.record("EntityCommandArm::follow_tail", 0x0064a8e0u);
    }
    void call_0077c980(std::uint32_t) override {
        chain_.owner.record("EntityCommandArm::leave", 0x0077c980u);
    }
    void call_0077ca60() override {
        chain_.owner.record("EntityCommandArm::disband", 0x0077ca60u);
    }
    std::uint32_t path_interface_007ac9d0(std::uint32_t target) override {
        // 007AC9D0 is complete in src/entity_command_arms.cpp; its own host is
        // the entity's IsKindOf, which for a created ship this process answers
        // through the recovered class chain only in GameUnitsHost. The command
        // path holds no class id, so the four kind tests are records and the
        // routine answers "no path interface", which is the ship case: 47h to
        // 4Ah are the path-following kinds of docs/ENTITY_COMMAND_ARMS.md.
        chain_.owner.record("EntityCommandArm::path_interface", 0x007ac9d0u);
        static_cast<void>(target);
        return 0u;
    }
    void set_fire_target_00835860(std::uint32_t, int) override {
        // LABELLED (packet cc9_weapon_director_fire_target): stays a record; this
        // arm (00816E30) is not reached on the reference runs.
        chain_.owner.record("EntityCommandArm::set_fire_target", 0x00835860u);
    }
    bool controller_belongs_to_another_007788b0() override {
        chain_.owner.record("EntityCommandArm::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    void send_clear_commands_0071d880() override {
        if (!kClearOrdersSendBound) {
            chain_.owner.record("EntityCommandArm::send_clear_commands", 0x0071d880u);
            return;
        }
        // 0071D880 -> MT_GAMEUNIT_CLEARCMD (+20h = 1, +24h = -1), the message
        // clear_all_commands posts for 0081733E: routed through
        // route_clear_command and answered by 00721A40's 5Dh arm with 00720CA0.
        chain_.owner.done("EntityCommandArm::send_clear_commands", 0x0071d880u);
        bsp::ClearCommandMessage every{};
        every.arm = 1;
        every.index = -1;
        chain_.owner.route_clear_command(chain_.unit.index,
            chain_.owner.player_of(chain_.unit.index), every);
        ++chain_.owner.summary.clearorders_sends;
    }
    bool call_0080dc70() override {
        chain_.owner.record("EntityCommandArm::free_fire_gate", 0x0080dc70u);
        return false;
    }
    void free_fire_0071bf20() override {
        chain_.owner.record("EntityCommandArm::free_fire", 0x0071bf20u);
    }
    void clear_target_block_00817023() override {
        chain_.owner.record("EntityCommandArm::clear_target_block", 0x00817023u);
    }
    std::uint32_t allocate_zeroed_00470b80(std::uint32_t) override {
        chain_.owner.record("EntityCommandArm::allocate_throwaway", 0x00470b80u);
        return 0u;
    }
    std::uint32_t construct_entity_004e5980(std::uint32_t) override {
        chain_.owner.record("EntityCommandArm::construct_throwaway", 0x004e5980u);
        return 0u;
    }
    void place_entity_vtable98(std::uint32_t, std::uint32_t) override {
        chain_.owner.record_slot("EntityCommandArm::place_throwaway", "00cfc3d0+vtable98");
    }
    std::uint32_t transform_from_position_0059bd20() override {
        chain_.owner.record("EntityCommandArm::throwaway_transform", 0x0059bd20u);
        return 0u;
    }
    void set_entity_transform_006e8040(std::uint32_t, std::uint32_t) override {
        chain_.owner.record("EntityCommandArm::set_throwaway_transform", 0x006e8040u);
    }
    void set_descriptor_target_00464f70(std::uint32_t, float) override {
        chain_.owner.record("EntityCommandArm::set_descriptor_target", 0x00464f70u);
    }
    std::uint32_t session_field_19cc() override {
        chain_.owner.record("EntityCommandArm::session_world", 0x008172e9u);
        return 0u;
    }

private:
    ChainState& chain_;
};

// MT_COMMAND's delivery: 00780120's 58h arm to the ship's vtable[160h], 00816e30.
void deliver_entity_command(ChainState& chain_, const bsp::EntityOrderMessage& message) {
    bsp::EntityCommandMessageView view;
    view.command_ordinal = message.command_ordinal;
    view.flags = message.flags;
    view.target.kind = static_cast<std::uint8_t>(message.target_kind & 0xffu);
    view.target.position_valid = static_cast<std::uint8_t>((message.target_kind >> 8) & 0xffu);
    view.target.object_id = message.target_id;
    view.target.object = const_cast<void*>(message.target_object);
    for (int lane = 0; lane < 3; ++lane) view.target.position[lane] = message.position[lane];
    view.target.trailing = message.trailing;

    const bsp::EntityOrderCommandClass* klass
        = bsp::entity_order_command_class_by_ordinal(
            static_cast<int>(view.command_ordinal));
    // Milestone 2n: the arm cascade 00816ea6..0081732e, reconstructed by packet
    // cc_ship_ai_arms in src/entity_command_arms.cpp. Milestone 2m recorded the
    // whole block at 00816f7c and stopped every scripted moveto and attackmove
    // there; the cascade decides which command singleton the order becomes and
    // hands the survivors to the same tail, so the seven commands this
    // mission's script issues now reach 0071ecf0.
    if (klass != nullptr) {
        EntityCommandArmsBinding arms(chain_);
        const bsp::EntityCommandArmDecision decision
            = bsp::entity_command_arm_cascade_00816ea6(
                static_cast<bsp::EntityCommandArmId>(klass->object_address), view.target,
                arms);
        chain_.owner.done("EntityCommand::arm_cascade", 0x00816ea6u);
        if (decision.result == bsp::EntityCommandArmResult::HandledWithoutQueueing) {
            if (chain_.row != nullptr) {
                chain_.row->blocked = "00816e30's arm for this command did its own work and "
                    "returned; nothing is queued on the director";
            }
            return;
        }
        if (decision.command != bsp::EntityCommandArmId::None
            && static_cast<std::uint32_t>(decision.command) != klass->object_address) {
            // 00816fb4 (moveto -> moveonpath), 00816fd9 (land -> attackmove) and
            // 00817238 (attackmove / artillery -> attackmove): the arm rewrote
            // EBP, and the tail issues what EBP holds.
            const bsp::EntityOrderCommandClass* substituted
                = bsp::entity_order_command_class_by_address(
                    static_cast<std::uint32_t>(decision.command));
            if (substituted != nullptr) {
                view.command_ordinal = static_cast<std::uint8_t>(substituted->ordinal);
                if (chain_.row != nullptr) chain_.row->command = substituted->name;
            }
        }
        if (decision.made_throwaway_target && chain_.row != nullptr) {
            chain_.row->blocked = "00817243..0081732e manufactured a throwaway target entity; "
                "its six call sites are records";
        }
    }
    if (chain_.row != nullptr) chain_.row->projected_arm = true;
    DirectorBinding director(chain_);
    bsp::unit_apply_entity_command_00816e30(director, view);
    chain_.owner.done("EntityCommand::apply", 0x00816e30u);
}

void EntityIssueBinding::route_message(void* entity, const bsp::EntityOrderMessage& message) {
    static_cast<void>(entity);
    // 0077c2a0 at 0077d7bd. The router reads the default routing flags at
    // [00e0af1c], queues a local delivery on session+24ch and 0076c600 drains
    // it; 00780670 then widens the tick window for a category 49h message and
    // 00780120's 58h arm at 00780607 dispatches it to the entity's own
    // vtable[160h], which for a ship is 00816e30.
    chain_.owner.record("EntityOrder::route_message", 0x0077c2a0u);
    chain_.owner.record("Session::drain_local_messages", 0x0076c600u);
    chain_.owner.record("Session::message_tick_window", 0x00780670u);
    chain_.owner.record_slot("Session::dispatch_entity_command", "00cfc530+vtable160");
    ++chain_.owner.summary.loopback_command_posts;
    if (!kSetCommandQueueDelayBound) {
        ++chain_.owner.summary.loopback_delivered_in_place;
        deliver_entity_command(chain_, message);
        return;
    }
    // Packet cc9_set_command_queue_delay: 0077C44D CALL 0076E520.
    GameCommandsHost::Impl::LoopbackMessage posted;
    posted.kind = GameCommandsHost::Impl::LoopbackKind::Command;
    posted.unit = chain_.unit.index;
    posted.row = chain_.owner.row_index_of(chain_.row);
    posted.ring = chain_.ring;
    posted.heading = chain_.heading;
    posted.finish = chain_.finish_pending;
    posted.script_issue = chain_.script_issue;
    posted.target_handle = chain_.owner.unit_handle_of(message.target_object);
    posted.command = message;
    posted.after_delivery = std::move(chain_.after_delivery);
    chain_.after_delivery = nullptr;
    chain_.finish_pending = false;
    chain_.owner.post_loopback(posted);
    chain_.command_list = chain_.owner.last_post_list;
    chain_.command_index = chain_.owner.last_post_index;
}

void DirectorBinding::director_issue_command(std::uint32_t command,
    const bsp::SceneCommandTarget& target) {
    bsp::director_issue_command_0071ecf0(*this, command, target);
    chain_.owner.done("WeaponDirector::issue_command", 0x0071ecf0u);
}

void DirectorBinding::route_set_command_message(std::uint32_t command,
    const bsp::SceneCommandTarget& target, std::uint8_t flag) {
    // 0071c830 at 0071ed6c builds MT_GAMEUNIT_SETCMD with the caller's flag at
    // +20h and the command ordinal at +21h, and 0077c2a0 at 0071ed81 routes it
    // with the routing flags 7 through the endpoint at director+34h. The same
    // session boundary as the first hop, so the executable delivers it here.
    chain_.owner.record("WeaponDirector::build_set_command_message", 0x0071c830u);
    chain_.owner.record("WeaponDirector::route_set_command_message", 0x0077c2a0u);
    chain_.owner.record_slot("Session::dispatch_gameunit_message", "00cfc530+vtable114");
    ++chain_.owner.summary.loopback_setcmd_posts;
    if (!kSetCommandQueueDelayBound) {
        ++chain_.owner.summary.loopback_delivered_in_place;
        chain_.pending_command = command;
        chain_.pending_target = target;
        chain_.pending_flag = flag;
        bsp::gameunit_apply_set_command_00721a40(*this, kSessionModeSinglePlayer, flag);
        chain_.owner.done("GameUnitMessage::apply_set_command", 0x00721a40u);
        return;
    }
    // Packet cc9_set_command_queue_delay: 0077C44D CALL 0076E520. Inside a
    // drain this is the +258h insertion, delivered right after the MT_COMMAND
    // being delivered; from a director step it waits for row 9.
    GameCommandsHost::Impl::LoopbackMessage posted;
    posted.kind = GameCommandsHost::Impl::LoopbackKind::SetCommand;
    posted.unit = chain_.unit.index;
    posted.row = chain_.owner.row_index_of(chain_.row);
    posted.ring = chain_.ring;
    posted.heading = chain_.heading;
    posted.finish = chain_.finish_pending;
    posted.script_issue = chain_.script_issue;
    posted.target_handle = chain_.owner.unit_handle_of(target.object);
    posted.set_command = command;
    posted.set_target = target;
    posted.set_flag = flag;
    posted.after_delivery = std::move(chain_.after_delivery);
    chain_.after_delivery = nullptr;
    chain_.finish_pending = false;
    chain_.setcmd_post = chain_.owner.post_loopback(posted);
}

}  // namespace

// ---------------------------------------------------------------------------
// GameCommandsHost
// ---------------------------------------------------------------------------

namespace {
GameCommandsHost*& live_commands_host() {
    static GameCommandsHost* live = nullptr;
    return live;
}
}  // namespace

GameCommandsHost::GameCommandsHost(GameHostLog& log)
    : impl_(std::make_unique<Impl>(log)) {
    live_commands_host() = this;
}

GameCommandsHost::~GameCommandsHost() {
    if (live_commands_host() == this) live_commands_host() = nullptr;
}

void GameCommandsHost::set_unit_formation(std::size_t index, bool follower,
                                          std::size_t leader) {
    // Packet cc8_ship_follow. The unit group lives on the units host; this is the
    // pair 007788B0 and 007788D0 read off unit+284h, pushed here so 00836920's
    // idle re-issue can answer them.
    Impl& host = *impl_;
    if (index >= host.units.size()) return;
    host.units[index].formation_follower = follower;
    host.units[index].formation_leader = leader;
}

void GameCommandsHost::register_units(std::vector<GameCommandUnit> units) {
    Impl& host = *impl_;
    // Packet cc9_ai_retask, docs/AI_RETASK.md. unit+284h, the group 007788B0 and
    // 007788D0 read, is per-unit state that creating ANOTHER unit does not touch
    // in the image. The rows arriving here carry the defaults, so a re-registration
    // after a formation join used to clear every follower's pair, and 00836920's
    // idle tail then issued `stop` instead of `follow`.
    if (kAiRetaskBound) {
        const std::size_t kept_rows = std::min(host.units.size(), units.size());
        for (std::size_t i = 0; i < kept_rows; ++i) {
            if (!host.units[i].formation_follower || units[i].formation_follower) continue;
            units[i].formation_follower = host.units[i].formation_follower;
            units[i].formation_leader = host.units[i].formation_leader;
        }
    }
    host.units = std::move(units);
    // Packet cc8_ship_drive. This is called again for every batch of units the
    // mission creates - GameScriptOrdersHost calls create_units at its two
    // spawn producers and create_units ends here - and it used to `assign`,
    // which destroyed EVERY unit's director and rebuilt it: the ten command
    // slots at director+54h, the queue mode at +30h, the stage at +48h, the
    // cruise latch and, since packet cc8_ship_moveonpath, the slot-0 path
    // object. In the executable a director is per-unit state created with its
    // unit (00720180 from 008363E0) and destroyed with it; nothing about
    // creating ANOTHER unit touches it. The units are appended, so the indices
    // of the existing ones do not move and `resize` keeps their state and
    // default-constructs only the new rows.
    const std::size_t kept = host.directors.size();
    host.directors.resize(host.units.size());
    if (kept != 0 && kept != host.units.size()) {
        ++host.summary.director_reregistrations;
        if (host.summary.director_reregistrations <= 4) {
            host.log.notef("re-registered the unit table: %zu director(s) kept, %zu new. Before "
                "packet cc8_ship_drive every one of the %zu was destroyed and rebuilt here, "
                "which emptied the command queue of every unit already in the mission each "
                "time one was created", kept, host.units.size() - kept, kept);
        }
    }
    // 0081f273 / 0081f278 leave the pair at -1.0f, which is what
    // CruiseSpeedSetting's own defaults are, so a fresh block is the
    // constructor's state rather than a zeroed one. Same correction: the block
    // holds the commanded speed 008A38D5 and 00890E6F store, and re-registering
    // the unit table was clearing it for every unit already in the mission.
    host.navigator_params.resize(host.units.size(), bsp::CruiseSpeedSetting{});
    host.summary.units = host.units.size();
    host.build_registry();
}

bool GameCommandsHost::director_avoidance(std::size_t unit_index,
    GameDirectorAvoidance& out) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    out = host.directors[unit_index].avoidance;
    return true;
}

bool GameCommandsHost::apply_director_avoidance_message_00835640(
    std::size_t unit_index, const bsp::DirectorCommandMessage& message) {
    Impl& host = *impl_;
    if (unit_index >= host.directors.size() || message.base_kind != 0x5a) return false;
    GameDirectorAvoidance& flags = host.directors[unit_index].avoidance;
    // Exactly the three derived switch arms. Other fields belong to the
    // existing command owner; no dummy WeaponDirectorHost or temporary full
    // state is used to run unrelated base-message behavior.
    switch (static_cast<bsp::DirectorCommandSubKind>(message.sub_kind)) {
    case bsp::DirectorCommandSubKind::TorpedoAvoidance:
        flags.torpedo = message.value != 0; // 00835653
        break;
    case bsp::DirectorCommandSubKind::ShipCollisionAvoidance:
        flags.ship = message.value != 0;    // 00835668
        break;
    case bsp::DirectorCommandSubKind::LandCollisionAvoidance:
        flags.land = message.value != 0;    // 0083567D
        break;
    default:
        return false;
    }
    host.done("WeaponDirector::apply_avoidance_message", 0x00835640u);
    return true;
}

namespace {
// Defined below, after the hop bindings it uses.
const GameCommandRow* finish_issue(GameCommandsHost::Impl& host, ChainState& chain,
    GameCommandRow& row, const bsp::UnitOrderRing& ring);
void finish_issue_in_place(GameCommandsHost::Impl& host, ChainState& chain);
}  // namespace

const GameCommandRow* GameCommandsHost::issue(std::size_t unit_index,
    const std::string& token, const std::string& target_token,
    const bsp::UnitOrderRing& ring, float heading_radians) {
    Impl& host = *impl_;
    if (unit_index >= host.units.size()) return nullptr;
    host.build_registry();

    if (!host.logged_path) {
        host.logged_path = true;
        host.log.notef("authored commands run the recovered path: 0046aab0 resolves the "
            "token against the 26-row registry at 00e19a70, 0077d600 builds MT_COMMAND "
            "(ordinal at +20h, flags 1 at +21h) and routes it, 00816e30 applies it, "
            "0071ecf0 issues it to the weapon director, 00721a40's 5Ch arm receives "
            "MT_GAMEUNIT_SETCMD, and 008358d0 / 0071e6c0 push the command slot. The "
            "session that carries the three messages is a record, so the executable "
            "delivers each one to the same process and says so");
    }

    GameCommandRow row;
    row.unit_index = unit_index;
    row.unit = host.units[unit_index].name;
    row.token = token;
    row.target_token = target_token;

    ChainState chain{host, host.units[unit_index], host.directors[unit_index], &row,
        &ring, heading_radians, bsp::SceneCommandTarget{}, 0u, 0u};

    // 0046aab0 over a one-record queue: the same walk, the same first-match
    // rule and the same tail clear the native runs for the scene's own queue.
    bsp::SceneCommandQueue queue;
    queue.push_back(bsp::scene_command_record_004690d0(&host.units[unit_index],
        token.c_str(), target_token.empty() ? nullptr : target_token.c_str()));
    SceneResolveBinding resolve(chain);
    const bsp::SceneCommandResolution resolution
        = bsp::resolve_scene_command_0046aab0(queue.front(), host.registry, resolve);
    row.resolve_outcome = outcome_name(resolution.outcome);
    if (resolution.command != nullptr) {
        const bsp::EntityOrderCommandClass* klass
            = host.class_of(host.object_of(resolution.command->identity));
        if (klass != nullptr) {
            row.command = klass->name;
            row.ordinal = klass->ordinal;
            row.category = klass->category;
        }
    }
    for (int lane = 0; lane < 3; ++lane) {
        row.descriptor_position[lane] = resolution.target.position[lane];
    }
    if (resolution.outcome == bsp::SceneCommandOutcome::kIssued
        && resolution.command != nullptr) {
        ++host.summary.resolved;
        if (kSetCommandQueueDelayBound) {
            // Packet cc9_set_command_queue_delay. The chain continues at the
            // drain, so its row is stored now and addressed by index; what the
            // caller copies from it on return is the post, not the push.
            host.rows.push_back(row);
            const std::size_t stored = host.rows.size() - 1;
            chain.row = &host.rows[stored];
            chain.finish_pending = true;
            host.last_issue_list = nullptr;
            resolve.issue_command(&host.units[unit_index], resolution.command->identity,
                resolution.target, 1);
            host.last_issue_list = chain.command_list;
            host.last_issue_index = chain.command_index;
            resolve.clear_queue();
            host.done("SceneCommand::resolve_deferred_reference", 0x0046aab0u);
            if (chain.finish_pending) finish_issue_in_place(host, chain);
            return &host.rows[stored];
        }
        resolve.issue_command(&host.units[unit_index], resolution.command->identity,
            resolution.target, 1);
    }
    resolve.clear_queue();
    host.done("SceneCommand::resolve_deferred_reference", 0x0046aab0u);
    return finish_issue(host, chain, row, ring);
}

namespace {

// The tail every issue shares, from 0071be40's current-command read to
// 00835c70's own arm. Extracted at milestone 2m so the navigator bindings'
// issue, which starts at 0077d600 with a fixed command object rather than at
// 0046aab0's registry walk, runs exactly the same hops.
void finish_issue_tail(GameCommandsHost::Impl& host, ChainState& chain,
    GameCommandRow& row, const bsp::UnitOrderRing& ring) {
    const std::size_t unit_index = row.unit_index;
    GameDirector& director = host.directors[unit_index];
    // 0071be40 with the mode 0071e6c0 set: 1 means the current command is slot 0.
    const std::uint32_t current = bsp::director_current_command_0071be40(director.mode,
        director.slot_command[0], 0u);
    host.done("WeaponDirector::current_command", 0x0071be40u);
    const bsp::EntityOrderCommandClass* issued_class
        = (row.ordinal >= 0) ? bsp::entity_order_command_class_by_ordinal(row.ordinal)
                             : nullptr;
    row.current = current != 0 && issued_class != nullptr
        && current == issued_class->object_address;
    if (row.slot_pushed) ++host.summary.pushed;
    if (row.current) ++host.summary.current;

    if (row.current && current == bsp::kCruiseCommandObjectAddress) {
        // 00835c70's `cruise` arm, 00835e0e..00835e5d. 00835c70 has no direct
        // caller: 00d09fd0 and 00d09fd4 are the director vtable's slot 78h and
        // are its only references, so where in a frame the begin-command
        // routine runs is not established and running it here, immediately
        // after the push that made the command current, is the executable's own
        // decision.
        DirectorBinding begin(chain);
        bsp::cruise_command_begin_00835c70(begin, ring);
        host.done("CruiseCommand::begin_command", 0x00835c70u);
        ++host.summary.latched;
        if (director.cruise.thrust != 0.0f) ++host.summary.moving;
        row.blocked = "009dbf90 / 009dffb0 / 009e0040 write the AI controller block at "
            "[state]+8 (+1d0h throttle, +1d4h rudder, +1d8h heading, +1c4h mode) and the "
            "hop from that block to unit+0fc4h / unit+0fdch under the unit+61h gate has no "
            "recovered writer";
    } else if (row.current && current == 0x00e08f88u) {
        // `stop` reaches the same arm and raises the stage, but 00835e17 tests
        // the command against 00e08f70 before the latch, so only `cruise`
        // latches. What a `stop` then asks of the ship belongs to another state
        // of the 00d21598 class family, which has no reconstruction.
        DirectorBinding begin(chain);
        begin.raise_command_stage(1);
        host.record("CruiseCommand::stop_state_step", 0x009e1170u);
        row.blocked = "`stop` raises the command stage and latches nothing (00835e17 "
            "compares the command against 00e08f70); its per-step state is another class "
            "of the 00d21598 family and has no reconstruction";
    }

    if (host.life_traced(unit_index)) {
        // The arrival, so a slot that appears is attributed to its producer.
        // NOT forced: a repeat the queue refuses changes nothing and would
        // otherwise drown the trace, and the run's own per-command table
        // already counts every issue by source.
        char label[80];
        std::snprintf(label, sizeof(label), "issue %.16s/%.16s %s",
            row.source.empty() ? "?" : row.source.c_str(),
            row.command.empty() ? "?" : row.command.c_str(),
            row.slot_pushed ? "pushed" : "REFUSED");
        host.life_emit(unit_index, label, host.life_clock, director);
    }
}

const GameCommandRow* finish_issue(GameCommandsHost::Impl& host, ChainState& chain,
    GameCommandRow& row, const bsp::UnitOrderRing& ring) {
    finish_issue_tail(host, chain, row, ring);
    host.rows.push_back(row);
    host.summary.command_name = row.command;
    return &host.rows.back();
}

// Packet cc9_set_command_queue_delay: the same tail on a row already stored,
// at the end of the delivery that ends the chain.
void finish_issue_in_place(GameCommandsHost::Impl& host, ChainState& chain) {
    chain.finish_pending = false;
    if (chain.row == nullptr || chain.ring == nullptr) return;
    finish_issue_tail(host, chain, *chain.row, *chain.ring);
    host.summary.command_name = chain.row->command;
    if (chain.script_issue && !chain.row->projected_arm) ++host.summary.script_blocked;
}

}  // namespace

// ---------------------------------------------------------------------------
// Packet cc9_set_command_queue_delay: 0076E520 and 0076C600
// ---------------------------------------------------------------------------

std::size_t GameCommandsHost::Impl::post_loopback(LoopbackMessage message) {
    message.posted_before_drain = loopback_drain_serial;
    // Env-gated: BSP_LOOPBACK_TRACE=<n> prints the first n posts with the drain
    // state they met. Prints nothing when unset.
    static const long trace_limit = [] {
        char* text = nullptr;
        std::size_t length = 0;
        long limit = 0;
        if (_dupenv_s(&text, &length, "BSP_LOOPBACK_TRACE") == 0 && text != nullptr) {
            limit = std::strtol(text, nullptr, 10);
        }
        std::free(text);
        return limit;
    }();
    static long traced = 0;
    if (traced < trace_limit) {
        ++traced;
        log.notef("  loopback post %ld: kind=%d unit=%zu row=%lld finish=%d active=%d open=%d "
            "queue=%zu serial=%llu clock=%.2f", traced, static_cast<int>(message.kind),
            message.unit, message.row == kNoLoopbackRow ? -1LL
                : static_cast<long long>(message.row), message.finish ? 1 : 0,
            loopback_active != nullptr ? 1 : 0, loopback_drain_open ? 1 : 0,
            loopback.size(), loopback_drain_serial, static_cast<double>(life_clock));
    }
    if (loopback_active != nullptr) {
        // 0076E5F5..0076E6D0: insert at +258h and move it past the new entry.
        message.nested = true;
        loopback_active->insert(loopback_active->begin()
            + static_cast<std::ptrdiff_t>(loopback_insert), message);
        last_post_list = loopback_active;
        last_post_index = loopback_insert;
        ++loopback_insert;
        return kNoLoopbackRow;
    }
    last_post_list = nullptr;
    last_post_index = kNoLoopbackRow;
    if (loopback_drain_open) {
        // A post from the pump's own after-row-9 order delivery, which is one
        // entry of the same drain in the image: delivered now, then its posts.
        message.nested = true;
        std::vector<LoopbackMessage> once(1, message);
        run_loopback_list(once);
        return kNoLoopbackRow;
    }
    // 0076E5D1..0076E5EC: +258h is null outside a drain, so the entry appends.
    loopback.push_back(message);
    last_post_list = &loopback;
    last_post_index = loopback.size() - 1;
    return loopback.size() - 1;
}

void GameCommandsHost::Impl::run_loopback_list(std::vector<LoopbackMessage>& list) {
    std::vector<LoopbackMessage>* const outer = loopback_active;
    const std::size_t outer_insert = loopback_insert;
    loopback_active = &list;
    // 0076C634..0076C70B: the end is re-read on every pass.
    for (std::size_t i = 0; i < list.size(); ++i) {
        loopback_insert = i + 1;                     // 0076C639
        const LoopbackMessage message = list[i];
        if (message.nested) {
            ++summary.loopback_delivered_nested;
        } else {
            ++summary.loopback_delivered_queued;
        }
        deliver_loopback(message);
    }
    loopback_active = outer;
    loopback_insert = outer_insert;
    if (last_issue_list == &list) last_issue_list = nullptr;
    if (last_post_list == &list) last_post_list = nullptr;
}

void GameCommandsHost::Impl::deliver_loopback(const LoopbackMessage& message) {
    if (message.unit >= units.size() || message.unit >= directors.size()) return;
    GameCommandRow* row = (message.row < rows.size()) ? &rows[message.row] : nullptr;
    ChainState chain{*this, units[message.unit], directors[message.unit], row, message.ring,
        message.heading, bsp::SceneCommandTarget{}, 0u, 0u};
    chain.finish_pending = message.finish;
    chain.script_issue = message.script_issue;
    chain.after_delivery = message.after_delivery;
    switch (message.kind) {
    case LoopbackKind::Command: {
        bsp::EntityOrderMessage delivered = message.command;
        delivered.target_object = unit_object_of(message.target_handle,
            delivered.target_object);
        deliver_entity_command(chain, delivered);
        break;
    }
    case LoopbackKind::SetCommand: {
        chain.pending_command = message.set_command;
        chain.pending_target = message.set_target;
        chain.pending_target.object = const_cast<void*>(
            unit_object_of(message.target_handle, message.set_target.object));
        chain.pending_flag = message.set_flag;
        DirectorBinding binding(chain);
        bsp::gameunit_apply_set_command_00721a40(binding, kSessionModeSinglePlayer,
            message.set_flag);
        done("GameUnitMessage::apply_set_command", 0x00721a40u);
        break;
    }
    case LoopbackKind::Clear:
        apply_clear_command(message.unit, message.player, message.clear);
        break;
    case LoopbackKind::UserPathPoint:
        apply_user_path_point_007207c0(message.unit, message.user_point,
                                       message.user_outside_map);
        break;
    }
    if (chain.finish_pending) finish_issue_in_place(*this, chain);
    if (chain.after_delivery) {
        // Still held: this delivery ended the chain (no post took it on).
        std::function<void()> after = std::move(chain.after_delivery);
        chain.after_delivery = nullptr;
        after();
    }
}

void GameCommandsHost::post_user_path_point_0071d340(std::size_t unit_index,
    const float point[3], bool outside_map) {
    Impl& host = *impl_;
    if (unit_index >= host.units.size() || unit_index >= host.directors.size()) return;
    host.done("PathObject::send_user_path_point_0071d340", 0x0071d340u);
    if (kSetCommandQueueDelayBound) {
        Impl::LoopbackMessage message;
        message.kind = Impl::LoopbackKind::UserPathPoint;
        message.unit = unit_index;
        for (int i = 0; i < 3; ++i) message.user_point[i] = point[i];
        message.user_outside_map = outside_map;
        host.post_loopback(message);
        return;
    }
    host.apply_user_path_point_007207c0(unit_index, point, outside_map);
}

void GameCommandsHost::Impl::apply_user_path_point_007207c0(std::size_t unit_index,
    const float point[3], bool outside_map) {
    GameDirector& director = directors[unit_index];
    ++director.user_points_received;
    const int count = command_count(director);
    // 0071DC80: 1 (start a new path) for an empty queue, a last command that is
    // not `moveonpath`, or one whose descriptor names an object of type 12h; 0
    // (append) for a `moveonpath` with descriptor kind 0 (0071DCB4) or one whose
    // object resolves to another type. LABELLED: a kind-1 `moveonpath` (an
    // authored Path) is taken as a new path; its object type was not resolved.
    bool start_new = true;
    if (count > 0 && director.slot_command[count - 1] == 0x00e08f80u
        && director.slot_target[count - 1].kind == 0) {
        start_new = false;
    }
    std::array<float, 3> p{point[0], point[1], point[2]};
    if (outside_map) {
        // 0071FE9A..0071FF22: an out-of-map point also pushes the border
        // crossing 004BBDD0 finds from the previous point. LABELLED: not
        // reproduced, the point alone is pushed and the case is counted.
        ++director.user_points_outside_map;
        record("PathObject::user_point_outside_map_004bbdd0", 0x004bbdd0u);
    }
    ChainState chain{*this, units[unit_index], director, nullptr, nullptr, 0.0f,
        bsp::SceneCommandTarget{}, 0u, 0u};
    DirectorBinding binding(chain);
    if (start_new) {
        // 007207CC: director vtable[34h] 00835E90 must accept `moveonpath`.
        if (!binding.command_accepted(0x00e08f80u)) {
            record("WeaponDirector::user_path_refused_00835e90", 0x007207dcu);
            return;
        }
        // 0071FDE0(point, 1, count): the slot object's vector is erased first.
        director.user_points.clear();
        director.user_points.push_back(p);
        // 0071FFA5: director vtable[60h] 008358D0 with `moveonpath` and the
        // zeroed descriptor (kind 0, no position, no object).
        bsp::SceneCommandTarget empty{};
        binding.director_set_command(0x00e08f80u, empty);
        ++director.user_paths_queued;
        done("GameUnitMessage::add_user_path_point_new_007207c0", 0x007207c0u);
    } else {
        // 0071FDE0(point, 0, count - 1): refused when more than 7 points already
        // lie ahead of the slot's +18h.
        const int ahead = static_cast<int>(director.user_points.size())
            - (director.path_user ? director.path_cursor.index_08 : 0);
        if (ahead > 7) {
            ++director.user_points_dropped;
            record("PathObject::user_point_dropped_0071fe24", 0x0071fe24u);
            return;
        }
        director.user_points.push_back(p);
        done("GameUnitMessage::add_user_path_point_append_007207c0", 0x0072081fu);
    }
    // 0071FF4D, 007B1FB0([slot+18h]): a followed path takes the new points.
    if (director.path_user && director.path_begun && director.slot_command[0] == 0x00e08f80u) {
        director.path_points = director.user_points;
    }
    // 0071FFBF..00720040: the session-mode echo (unit vtable[13Ch](2, 0) and a
    // 57h message) is multiplayer bookkeeping.
    record("PathObject::user_path_echo_57", 0x0071ffbfu);
}

bool GameCommandsHost::user_path_attached_0071fc40(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    const GameDirector& director = host.directors[unit_index];
    return director.slot_command[0] == 0x00e08f80u && director.path_begun
        && director.path_built && director.path_user;
}

int GameCommandsHost::user_path_remaining_0071d2a0(std::size_t unit_index) const {
    // 0071D2A0 -> 0071D2E0: [slot+18h] when attached, else -1. LABELLED: +18h
    // is taken as the follower's current index (the listener at +10h is how the
    // follower reports it; its body was not read).
    if (!user_path_attached_0071fc40(unit_index)) return -1;
    return impl_->directors[unit_index].path_cursor.index_08;
}

void GameCommandsHost::begin_loopback_drain_0076c600() {
    if (!kSetCommandQueueDelayBound) return;
    impl_->loopback_drain_open = true;
}

std::size_t GameCommandsHost::finish_loopback_drain_0076c600() {
    if (!kSetCommandQueueDelayBound) return 0;
    Impl& host = *impl_;
    const unsigned long long before = host.summary.loopback_delivered_nested
        + host.summary.loopback_delivered_queued;
    std::vector<Impl::LoopbackMessage> batch;
    batch.swap(host.loopback);
    host.last_issue_list = nullptr;
    if (!batch.empty()) {
        ++host.summary.loopback_drains;
        host.run_loopback_list(batch);
    }
    host.loopback_drain_open = false;
    ++host.loopback_drain_serial;
    host.done("Session::drain_loopback_queue", 0x0076c600u);
    return static_cast<std::size_t>(host.summary.loopback_delivered_nested
        + host.summary.loopback_delivered_queued - before);
}

bool GameCommandsHost::after_last_issue_delivery(std::function<void()>& fn) {
    if (!kSetCommandQueueDelayBound) return false;
    Impl& host = *impl_;
    if (host.last_issue_list == nullptr
        || host.last_issue_index >= host.last_issue_list->size()) {
        return false;
    }
    std::function<void()>& slot = (*host.last_issue_list)[host.last_issue_index].after_delivery;
    if (slot) {
        std::function<void()> first = std::move(slot);
        slot = [first = std::move(first), next = std::move(fn)]() {
            first();
            next();
        };
    } else {
        slot = std::move(fn);
    }
    fn = nullptr;
    return true;
}

bool commands_after_last_issue_delivery(std::function<void()>& fn) {
    GameCommandsHost* live = live_commands_host();
    return live != nullptr && live->after_last_issue_delivery(fn);
}

void commands_begin_loopback_drain_0076c600() {
    if (GameCommandsHost* live = live_commands_host()) live->begin_loopback_drain_0076c600();
}

std::size_t commands_finish_loopback_drain_0076c600() {
    GameCommandsHost* live = live_commands_host();
    return live != nullptr ? live->finish_loopback_drain_0076c600() : 0;
}

const GameCommandRow* GameCommandsHost::issue_command_object(std::size_t unit_index,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags,
    const std::string& source, const std::string& target_name,
    const bsp::UnitOrderRing& ring, float heading_radians) {
    Impl& host = *impl_;
    if (unit_index >= host.units.size()) return nullptr;
    host.build_registry();

    GameCommandRow row;
    row.unit_index = unit_index;
    row.unit = host.units[unit_index].name;
    row.source = source;
    row.target_token = target_name;
    row.resolve_outcome = "fixed_command_object";
    const bsp::EntityOrderCommandClass* klass = host.class_of(command_object);
    if (klass != nullptr) {
        row.command = klass->name;
        row.token = klass->name;
        row.ordinal = klass->ordinal;
        row.category = klass->category;
    }
    for (int lane = 0; lane < 3; ++lane) row.descriptor_position[lane] = target.position[lane];

    ChainState chain{host, host.units[unit_index], host.directors[unit_index], &row,
        &ring, heading_radians, bsp::SceneCommandTarget{}, 0u, 0u};
    SceneResolveBinding resolve(chain);
    if (kSetCommandQueueDelayBound) {
        // Packet cc9_set_command_queue_delay, as in issue().
        host.rows.push_back(row);
        const std::size_t stored = host.rows.size() - 1;
        chain.row = &host.rows[stored];
        chain.finish_pending = true;
        chain.script_issue = true;
        host.last_issue_list = nullptr;
        resolve.issue_command(&host.units[unit_index],
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(command_object)), target,
            flags);
        host.last_issue_list = chain.command_list;
        host.last_issue_index = chain.command_index;
        ++host.summary.script_issues;
        if (chain.finish_pending) finish_issue_in_place(host, chain);
        return &host.rows[stored];
    }
    resolve.issue_command(&host.units[unit_index],
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(command_object)), target, flags);
    ++host.summary.script_issues;
    const GameCommandRow* stored = finish_issue(host, chain, row, ring);
    if (stored != nullptr && !stored->projected_arm) ++host.summary.script_blocked;
    return stored;
}

void GameCommandsHost::store_commanded_speed_00890e6f(std::size_t unit_index,
    float requested, float mission_clock) {
    Impl& host = *impl_;
    if (unit_index >= host.navigator_params.size()) return;
    const bool was_active
        = bsp::navigator_commanded_speed_active(host.navigator_params[unit_index]);
    host.navigator_params[unit_index]
        = bsp::navigator_commanded_speed_store_00890e6f(requested, mission_clock);
    if (!was_active) ++host.summary.commanded_speeds;
    if (!host.logged_navigator_params) {
        host.logged_navigator_params = true;
        host.log.notef("commanded speed stored on the navigator parameter block at "
            "*(unit+73Ch): +24h = max(requested, 0) and +28h = the mission clock "
            "DAT_00F876A4, the store 00890e6f and 008a38d5 both make. +28h is a timestamp, "
            "not an enable: 00835c28 measures its age against 1.0f, and 00836e59 is what "
            "turns an active pair into a `cruise` instead of a `stop`");
    }
    host.done("NavigatorParams::store_commanded_speed", 0x00890e6fu);
}

bsp::CruiseSpeedSetting GameCommandsHost::commanded_speed(std::size_t unit_index) const {
    return impl_->params_of(unit_index);
}

namespace {

// ---------------------------------------------------------------------------
// bsp::WeaponDirectorStageHost, one method per call site inside 00836920
// ---------------------------------------------------------------------------
//
// Milestone 2m. The director's stage ladder is what decides, every step, that a
// unit with nothing to do should be told to `stop` or to `cruise`, and the
// commanded-speed pair is the field that separates the two. Nothing here is a
// reconstruction: the pre-pass, the `stop` arm, the idle tail and the stage
// reset are docs/UNIT_COMMANDED_SPEED.md's routines.
class DirectorStageBinding final : public bsp::WeaponDirectorStageHost {
public:
    DirectorStageBinding(ChainState& chain, float mission_clock, bool player_controlled)
        : chain_(chain), clock_(mission_clock), player_(player_controlled),
          post_reset(chain.owner.params_of(chain.unit.index)) {}

    bool queue_advanced{false};

    int director_filled_command_slots_0071be60() override {
        chain_.owner.done("WeaponDirector::filled_command_slots", 0x0071be60u);
        return chain_.owner.command_count(chain_.director);
    }
    void director_raise_primary_stage_0071d810(int stage) override {
        // 0071D810 is monotonic (0071D818 returns when the stored stage is
        // already at least the requested one).
        if (!bsp::stage_raise_applies(chain_.director.stage, stage)) {
            chain_.owner.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
            return;
        }
        chain_.director.stage = stage;
        chain_.owner.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
        ++chain_.owner.summary.stage_raises;
        // Milestone 2q: reaching stage 2 is what sends MT_GAMEUNIT_CLEARCMD,
        // and the queue only advances when that message is received. In a local
        // session this process is both ends, so it routes its own message back
        // into 00721A40's 5Dh arm here rather than recording the send.
        if (!bsp::stage_raise_sends_message(stage, kSessionModeSinglePlayer)) return;
        if (chain_.owner.route_clear_command(chain_.unit.index, player_)) {
            queue_advanced = true;
        }
    }
    void director_raise_secondary_stage_0071d9e0(int stage) override {
        if (stage > secondary_stage_) secondary_stage_ = stage;
        chain_.owner.done("WeaponDirector::raise_secondary_stage", 0x0071d9e0u);
    }
    void director_clear_command_slot_0071c130(bool primary) override {
        if (primary) {
            chain_.director.stage = 0;
        } else {
            secondary_stage_ = 0;
        }
        chain_.owner.done("WeaponDirector::clear_command_stage_pair", 0x0071c130u);
    }
    void navigator_params_reset_from_tuning_00822b70(bool reset) override {
        // 00835c65 passes the literal 0, and 00822b70's own body is skipped
        // entirely when its char argument is zero, so the call does nothing.
        // The projection makes it because the native makes it.
        static_cast<void>(reset);
        chain_.owner.done("NavigatorParams::reset_from_tuning", 0x00822b70u);
    }
    void set_navigator_commanded_speed_time(float value) override {
        chain_.owner.set_commanded_speed_time(chain_.unit.index, value);
    }
    void director_reset_command_stage_vtable6c(bool primary) override {
        // 00d09fc4, the derived director's vtable slot 6Ch, which is 00835bf0.
        post_reset = bsp::weapon_director_reset_command_stage_00835bf0(primary,
            chain_.owner.params_of(chain_.unit.index), clock_, *this);
        chain_.owner.done("WeaponDirector::reset_command_stage", 0x00835bf0u);
    }
    bool unit_controller_belongs_to_another_007788b0() override {
        // 007788B0 BSP_Unit_IsFormationFollower, read whole by packet
        // cc8_ship_follow: g = [unit+284h]; g && [g+14h] != unit. unit+284h is
        // the unit GROUP, not a controller back-pointer - 0077FB22 tests it for
        // zero and calls 0070DB20 UnitGroup_Create to fill it - and this process
        // now has one, so the answer is read rather than recorded.
        chain_.owner.done("WeaponDirector::is_formation_follower", 0x007788b0u);
        return chain_.unit.formation_follower;
    }
    std::uint32_t unit_controller_owner_007788d0() override {
        // 007788D0 BSP_Unit_FormationLeader: [[unit+284h]+14h].
        chain_.owner.done("WeaponDirector::formation_leader", 0x007788d0u);
        if (!chain_.unit.formation_follower) return 0;
        return static_cast<std::uint32_t>(chain_.unit.formation_leader + 1u);
    }
    std::uint32_t make_command_target_00465080(std::uint32_t object, float range) override {
        static_cast<void>(range);
        target_ = bsp::SceneCommandTarget{};
        if (object != 0) {
            // 00465080 builds a kind-1 descriptor naming `object`, the one-based
            // handle its callers pass: the unit itself for `cruise` / `stop`
            // (00836E6C, 00836E84) and the formation leader for `follow`
            // (00836E28..00836E32). Packet cc9_ship_natives_2: this used to name
            // the unit on every arm, so a follow order targeted its own follower,
            // which the follow arm 00836B23 reads and ends.
            const GameCommandUnit* named = &chain_.unit;
            if (kDirectorCommandArmsBound && object - 1u < chain_.owner.units.size()) {
                named = &chain_.owner.units[object - 1u];
            }
            target_.kind = 1;
            target_.object = const_cast<GameCommandUnit*>(named);
            target_.object_id = named->object_id;
        }
        chain_.owner.done("WeaponDirector::make_command_target", 0x00465080u);
        return object;
    }
    void director_issue_command_0071ecf0(std::uint32_t command,
        std::uint32_t target) override {
        static_cast<void>(target);
        DirectorBinding director(chain_);
        director.director_issue_command(command, target_);
    }

    bsp::CruiseSpeedSetting post_reset{};

private:
    ChainState& chain_;
    float clock_{0.0f};
    bool player_{false};
    int secondary_stage_{0};
    bsp::SceneCommandTarget target_{};
};

}  // namespace

namespace {

// ---------------------------------------------------------------------------
// Milestone 2q: the controller's command state, as the two reconstructions see
// it
// ---------------------------------------------------------------------------
//
// GameDirector is milestone 2l's lighter projection of the same native storage:
// slot_command[i] is the slot's +0h command object and slot_target[i] its
// 18h-byte parameter record, which 00720850 and 0071E430 know as
// CommandParams. The two records have the same shape and the same offsets, so
// the copy is field for field, not a conversion.
bsp::CommandParams params_of_target(const bsp::SceneCommandTarget& target) noexcept {
    bsp::CommandParams params{};
    params.has_target_entity = target.kind != 0;
    params.has_position = target.position_valid != 0;
    params.target_entity_id = target.object_id;
    params.target_entity = target.object != nullptr ? 1u : 0u;
    params.position_x = target.position[0];
    params.position_y = target.position[1];
    params.position_z = target.position[2];
    return params;
}

bsp::SceneCommandTarget target_of_params(const bsp::CommandParams& params,
                                         void* object) noexcept {
    bsp::SceneCommandTarget target{};
    target.kind = params.has_target_entity ? 1u : 0u;
    target.position_valid = params.has_position ? 1u : 0u;
    target.object_id = params.target_entity_id;
    target.object = params.target_entity != 0 ? object : nullptr;
    target.position[0] = params.position_x;
    target.position[1] = params.position_y;
    target.position[2] = params.position_z;
    return target;
}

bsp::CommandQueueState queue_state_of(const GameDirector& director) {
    bsp::CommandQueueState state{};
    state.mode = static_cast<bsp::CommandMode>(static_cast<int>(director.mode));
    state.queue_stage = director.stage;
    state.queue_accepted = director.latched;
    for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
        state.slots[i].command = director.slot_command[i];
        state.slots[i].params = params_of_target(director.slot_target[i]);
    }
    return state;
}

void queue_state_back(const bsp::CommandQueueState& state, GameDirector& director) {
    director.mode = static_cast<bsp::CruiseCommandMode>(static_cast<int>(state.mode));
    director.stage = state.queue_stage;
    director.latched = state.queue_accepted;
    for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
        void* const object = director.slot_target[i].object;
        director.slot_command[i] = state.slots[i].command;
        director.slot_target[i] = target_of_params(state.slots[i].params, object);
    }
}

// bsp::CommandExecutionHost, for 00720850 only. Every method that the clear
// path does not reach on this mission records its own address; the ones it does
// reach are concrete.
class QueueClearBinding final : public bsp::CommandExecutionHost {
public:
    QueueClearBinding(GameCommandsHost::Impl& owner, GameCommandUnit& unit,
                      GameDirector& director, bool player_controlled)
        : owner_(owner), unit_(unit), director_(director), player_(player_controlled) {}

    const char* command_name(std::uint32_t command) override {
        const bsp::EntityOrderCommandClass* klass = owner_.class_of(command);
        owner_.done("WeaponDirector::command_name", 0x00720889u);
        return klass != nullptr ? klass->name : "EmptyCommand";
    }
    int command_category(std::uint32_t command) override {
        const bsp::EntityOrderCommandClass* klass = owner_.class_of(command);
        owner_.done("WeaponDirector::command_category", 0x008358f0u);
        return klass != nullptr ? klass->category : 0;
    }
    bool command_accepts_target(std::uint32_t, const bsp::CommandParams&) override {
        owner_.record("WeaponDirector::command_accepts_target", 0x0071d6d0u);
        return false;
    }
    std::uint32_t resolve_target(const bsp::CommandParams& params) override {
        owner_.done("WeaponDirector::resolve_target", 0x00521ea0u);
        if (!params.has_target_entity || params.target_entity_id == 0) return 0u;
        for (const GameCommandUnit& unit : owner_.units) {
            if (unit.object_id == params.target_entity_id) {
                return static_cast<std::uint32_t>(unit.index) + 1u;
            }
        }
        return 0u;
    }
    void refresh_target_pose(std::uint32_t) override {
        owner_.record("WeaponDirector::refresh_target_pose", 0x00414db0u);
    }
    void read_target_position(std::uint32_t target, float& x, float& y, float& z) override {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        const std::size_t index = static_cast<std::size_t>(target - 1u);
        if (index >= owner_.units.size()) return;
        x = owner_.units[index].position[0];
        y = owner_.units[index].position[1];
        z = owner_.units[index].position[2];
        owner_.done("WeaponDirector::read_target_position", 0x00720949u);
    }
    void register_target_observer(std::uint32_t) override {
        owner_.record("WeaponDirector::register_target_observer", 0x00694a60u);
    }
    void unregister_target_observer(std::uint32_t) override {
        owner_.record("WeaponDirector::unregister_target_observer", 0x006952a0u);
    }
    std::uint32_t create_path_object() override {
        owner_.record("WeaponDirector::create_path_object", 0x0071fb90u);
        return 0u;
    }
    void destroy_path_object(std::uint32_t) override {
        owner_.record("WeaponDirector::destroy_path_object", 0x00720aa1u);
    }
    void invalidate_path_object(std::uint32_t) override {
        owner_.record("WeaponDirector::invalidate_path_object", 0x0071bdb0u);
    }
    std::uint32_t session_trace_value() override {
        owner_.record("WeaponDirector::session_trace_value", 0x007208a3u);
        return 0u;
    }
    void trace_clear_primary_command(const char* command_name_in, int index, int mode,
                                     std::uint32_t) override {
        // 004254B0's varargs trace. The executable keeps the three values the
        // literal prints rather than formatting a native trace line.
        cleared_command = command_name_in != nullptr ? command_name_in : "";
        cleared_index = index;
        cleared_mode = mode;
        owner_.done("WeaponDirector::trace_clear_primary_command", 0x004254b0u);
    }
    void send_command_message(const char*, std::uint32_t, const bsp::CommandParams&) override {
        owner_.record("WeaponDirector::send_command_message", 0x00984300u);
    }
    void notify_command_target(std::uint32_t) override {
        owner_.record("WeaponDirector::notify_command_target", 0x00984800u);
    }
    void raise_queue_stage(int stage) override {
        if (stage > director_.stage) director_.stage = stage;
        owner_.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
    }
    void raise_override_stage(int) override {
        owner_.record("WeaponDirector::raise_secondary_stage", 0x0071d9e0u);
    }
    void set_fire_target(std::uint32_t target, bool force) override {
        // 00835E07, BeginCurrentCommand's call; `target` is a unit index + 1 here.
        if (!kWeaponDirectorFireTargetBound) {
            owner_.record("WeaponDirector::set_fire_target", 0x00835860u);
            return;
        }
        std::size_t plus_one = 0;
        if (target != 0u && target - 1u < owner_.units.size()) {
            plus_one = owner_.units[target - 1u].index + 1;
        }
        owner_.fire_target_requests.push_back({unit_.index, plus_one, force});
        owner_.done("WeaponDirector::set_fire_target", 0x00835860u);
    }
    std::uint32_t fire_target() override {
        owner_.record("WeaponDirector::fire_target", 0x008364e0u);
        return 0u;
    }
    void on_command_changed(bool) override {
        // 00720B56, vtable[6Ch] = 00835BF0, whose head is 0071C130.
        director_.stage = 0;
        director_.latched = false;
        owner_.done("WeaponDirector::clear_command_stage_pair", 0x0071c130u);
    }
    std::uint32_t group_leader() override {
        owner_.record("WeaponDirector::group_leader", 0x007788d0u);
        return 0u;
    }
    bool attack_move_gate() override {
        owner_.record("WeaponDirector::attack_move_gate", 0x00521e70u);
        return false;
    }
    bool controller_belongs_to_another() override {
        owner_.record("WeaponDirector::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    bool begin_command_base(bool) override {
        owner_.record("WeaponDirector::begin_command_base", 0x0071f600u);
        return false;
    }
    bool push_command_slot(std::uint32_t, const bsp::CommandParams&) override {
        owner_.record("WeaponDirector::push_command_slot", 0x0071e6c0u);
        return false;
    }
    bool set_command(std::uint32_t, const bsp::CommandParams&) override {
        owner_.record("WeaponDirector::set_command", 0x008358d0u);
        return false;
    }
    void default_command_position(float& x, float& y, float& z) override {
        // 00F87574, past .data's raw size, so three zeroes at load.
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        owner_.done("WeaponDirector::default_command_position", 0x00f87574u);
    }
    int session_mode() override { return kSessionModeSinglePlayer; }
    bool unit_flag_184h() override {
        static_cast<void>(unit_);
        return player_;
    }

    std::string cleared_command;
    int cleared_index{-1};
    int cleared_mode{-1};

private:
    GameCommandsHost::Impl& owner_;
    GameCommandUnit& unit_;
    GameDirector& director_;
    bool player_{false};
};

// bsp::CommandCompletionHost, the six methods 0071E430 and its stage ladder
// reach. The one that matters is raise_queue_stage: reaching stage 2 is what
// sends MT_GAMEUNIT_CLEARCMD, and in a local session this process is both the
// sender and the receiver, so the message is routed straight back into
// 00721A40's 5Dh arm here.
class CompletionBinding final : public bsp::CommandCompletionHost {
public:
    CompletionBinding(GameCommandsHost::Impl& owner, std::size_t unit_index,
                      GameDirector& director, bsp::CommandQueueState& state,
                      QueueClearBinding& exec, bool player_controlled)
        : owner_(owner), unit_index_(unit_index), director_(director), state_(state),
          exec_(exec), player_(player_controlled) {}

    int command_category(std::uint32_t command) override {
        const bsp::EntityOrderCommandClass* klass = owner_.class_of(command);
        owner_.done("WeaponDirector::command_category", 0x0071e440u);
        return klass != nullptr ? klass->category : 0;
    }

    void raise_queue_stage(int stage) override {
        // 0071D810: monotonic, then the message when the new stage is 2 and
        // this machine originates it. The send itself is deferred by one
        // statement to the caller, which is where 0071E430 returns anyway: arm
        // B is CALL 0071D810 followed by RET 8.
        if (!bsp::stage_raise_applies(state_.queue_stage, stage)) {
            owner_.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
            return;
        }
        state_.queue_stage = stage;
        director_.stage = stage;
        owner_.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
        if (bsp::stage_raise_sends_message(stage, kSessionModeSinglePlayer)) {
            sends_clear_command = true;
        }
    }

    void raise_override_stage(int stage) override {
        if (!bsp::stage_raise_applies(override_stage_, stage)) return;
        override_stage_ = stage;
        owner_.done("WeaponDirector::raise_secondary_stage", 0x0071d9e0u);
        if (bsp::stage_raise_sends_message(stage, kSessionModeSinglePlayer)) {
            owner_.record("WeaponDirector::clear_override_command", 0x0071e610u);
        }
    }

    void route_clear_command_message(const bsp::ClearCommandMessage&) override {
        sends_clear_command = true;
    }

    void assign_status_text(const char*) override {
        owner_.record("WeaponDirector::assign_status_text", 0x0041e870u);
    }
    std::uint32_t make_command_target(std::uint32_t) override {
        owner_.record("WeaponDirector::make_command_target", 0x004f1830u);
        return 0u;
    }
    void report_command_event(std::uint32_t, std::uint32_t, std::uint32_t,
                              const char*) override {
        owner_.record("WeaponDirector::report_command_event", 0x00984300u);
    }
    void report_target_event(std::uint32_t, std::uint32_t) override {
        owner_.record("WeaponDirector::report_target_event", 0x00984800u);
    }
    bool controller_belongs_to_another() override {
        owner_.record("WeaponDirector::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    std::uint32_t controlling_entity() override {
        owner_.record("WeaponDirector::controlling_entity", 0x007788d0u);
        return 0u;
    }
    void issue_command(std::uint32_t, std::uint32_t) override {
        owner_.record("WeaponDirector::issue_command", 0x0071ecf0u);
    }
    void on_command_changed(bool primary) override {
        const bsp::StageResetResult reset = bsp::reset_command_stage_0071c130(state_, primary);
        static_cast<void>(reset);
        director_.stage = state_.queue_stage;
        owner_.done("WeaponDirector::clear_command_stage_pair", 0x0071c130u);
    }

    bool sends_clear_command{false};

private:
    GameCommandsHost::Impl& owner_;
    std::size_t unit_index_{0};
    GameDirector& director_;
    bsp::CommandQueueState& state_;
    QueueClearBinding& exec_;
    bool player_{false};
    int override_stage_{0};
};

}  // namespace

// 0071D810's stage-2 tail, shared by both of its call sites: 0071C730 builds
// MT_GAMEUNIT_CLEARCMD(1, 0), the router hands it to the session, and in a local
// session this process is the receiver, so 00721A40's 5Dh arm runs here and
// takes the 00721BB7 branch into 00720850 with index 0.
bool GameCommandsHost::Impl::route_clear_command(std::size_t unit_index,
    bool player_controlled) {
    if (unit_index >= units.size()) return false;
    done("WeaponDirector::build_clear_command", 0x0071c730u);
    return route_clear_command(unit_index, player_controlled,
                               bsp::clear_command_message_for_queue_stage_done());
}

bool GameCommandsHost::Impl::route_clear_command(std::size_t unit_index,
    bool player_controlled, const bsp::ClearCommandMessage& message) {
    if (unit_index >= units.size()) return false;
    ++summary.clear_messages;
    ++summary.loopback_clear_posts;
    if (!kSetCommandQueueDelayBound) {
        ++summary.loopback_delivered_in_place;
        return apply_clear_command(unit_index, player_controlled, message);
    }
    // Packet cc9_set_command_queue_delay: the 5Dh message waits for the drain,
    // so the raising step's own later reads still see the finished command.
    LoopbackMessage posted;
    posted.kind = LoopbackKind::Clear;
    posted.unit = unit_index;
    posted.player = player_controlled;
    posted.clear = message;
    post_loopback(posted);
    return false;
}

bool GameCommandsHost::Impl::apply_clear_command(std::size_t unit_index,
    bool player_controlled, const bsp::ClearCommandMessage& message) {
    if (unit_index >= units.size()) return false;
    GameDirector& director = directors[unit_index];
    ++summary.clear_receives;
    done("GameUnitMessage::apply_clear_command", 0x00721a40u);
    if (kSetCommandClearAllMessageBound
        && bsp::clear_command_action(message) == bsp::ClearCommandAction::kClearAllSlots) {
        // 00721BA8 -> 00720CA0, src/command_execution.cpp's reconstruction:
        // 00720850 on each occupied slot from 9 down to 0.
        bsp::CommandQueueState state = queue_state_of(director);
        QueueClearBinding exec(*this, units[unit_index], director, player_controlled);
        for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
            if (state.slots[i].command != 0) ++summary.clear_all_slot_clears;
        }
        bsp::clear_all_command_slots_00720ca0(state, exec);
        queue_state_back(state, director);
        done("GameUnitMessage::clear_every_slot", 0x00720ca0u);
        return true;
    }
    if (bsp::clear_command_action(message) != bsp::ClearCommandAction::kClearSlot) {
        return false;
    }
    bsp::CommandQueueState state = queue_state_of(director);
    QueueClearBinding exec(*this, units[unit_index], director, player_controlled);
    bsp::clear_command_slot_00720850(state, message.index, exec);
    done("WeaponDirector::clear_primary_command", 0x00720850u);
    queue_state_back(state, director);
    ++summary.queue_advances;
    if (summary.queue_advances <= 8) {
        const bsp::EntityOrderCommandClass* promoted = class_of(director.slot_command[0]);
        log.notef("  command finished: %s cleared `%s` from slot %d (mode %d); the queue now "
            "holds `%s` and 0071c130 left the stage pair at 0",
            units[unit_index].name.c_str(),
            exec.cleared_command.empty() ? "EmptyCommand" : exec.cleared_command.c_str(),
            exec.cleared_index, exec.cleared_mode,
            promoted != nullptr ? promoted->name : "(none)");
    }
    return true;
}

std::size_t GameCommandsHost::report_command_event_00984300(std::size_t unit_index,
    std::uint32_t command_object, const char* status) {
    Impl& host = *impl_;
    if (unit_index >= host.units.size()) return 0;
    ++host.summary.command_events;
    const bsp::EntityOrderCommandClass* klass = host.class_of(command_object);
    bsp::CommandEventParameters params;
    params.unit_id = host.units[unit_index].object_id;
    params.target_id = 0;
    params.command_name = (klass != nullptr) ? klass->name : "";
    params.status = (status != nullptr) ? status : "";
    // 00984394..009843D8: the channel named `command` is looked up on the
    // mission event director's map at reporter+F8h and the body returns at once
    // when the channel holds no subscription. 0097E360, the parser that would
    // put one there, is not reconstructed, and this mission authors none:
    // usn_2_java.scn declares no event block and usn_2_java.lua registers no
    // `command` handler. The lookup therefore finds an empty channel, which is
    // the native's own early return, not a substitution for it.
    host.record("MissionEvents::command_event_block", 0x0097e360u);
    const std::vector<std::string> callbacks
        = bsp::command_event_callbacks(host.command_event_subscriptions, params);
    host.done("MissionEvents::report_command", 0x00984300u);
    host.summary.command_event_callbacks += callbacks.size();
    if (!callbacks.empty()) {
        // 00984710, 00887E50 BSP_MissionLuaHost_CallNamedThreadSafe on the host
        // at *(00E188A8)+1A08h. No subscription exists, so no name is ever
        // queued; the hop is recorded with its address rather than being left
        // unmentioned.
        host.record("MissionEvents::call_named_threadsafe", 0x00887e50u);
    }
    return callbacks.size();
}

GameCommandCompletion GameCommandsHost::end_command_0071e430(std::size_t unit_index,
    std::uint32_t command_object, bool terminal, bool player_controlled,
    const bsp::UnitOrderRing& ring, float heading_radians) {
    Impl& host = *impl_;
    GameCommandCompletion out;
    if (unit_index >= host.units.size()) return out;
    GameDirector& director = host.directors[unit_index];
    GameCommandUnit& unit = host.units[unit_index];

    bsp::CommandQueueState state = queue_state_of(director);
    const int stage_before = state.queue_stage;

    QueueClearBinding exec(host, unit, director, player_controlled);
    CompletionBinding completion(host, unit_index, director, state, exec, player_controlled);
    const bsp::EndCommandTrace trace
        = bsp::run_end_command_0071e430(state, command_object, terminal, completion);
    host.done("WeaponDirector::end_command", 0x0071e430u);
    ++host.summary.end_commands;

    out.ran = true;
    out.requested_stage = trace.requested_stage;
    out.raised_queue_stage = trace.raised_queue_stage;
    out.restarted_head = trace.restart.applies;
    switch (trace.arm) {
    case bsp::EndCommandArm::kQueueStageByCategory: out.arm = "queue_stage_by_category"; break;
    case bsp::EndCommandArm::kQueueStageByMode: out.arm = "queue_stage_by_mode"; break;
    case bsp::EndCommandArm::kOverrideStageByMode: out.arm = "override_stage_by_mode"; break;
    case bsp::EndCommandArm::kRestartQueueHead: out.arm = "restart_queue_head"; break;
    case bsp::EndCommandArm::kNothing: out.arm = "nothing"; break;
    }
    if (trace.restart.applies) ++host.summary.restarts;

    queue_state_back(state, director);
    if (director.stage > stage_before) ++host.summary.stage_raises;
    if (completion.sends_clear_command) {
        out.message_routed = true;
        out.queue_advanced = host.route_clear_command(unit_index, player_controlled);
    }
    out.promoted_command = director.slot_command[0];
    out.stage_after = director.stage;
    if (host.life_traced(unit_index)) {
        char label[64];
        std::snprintf(label, sizeof(label), "end 0071e430 %.28s",
            out.arm.empty() ? "?" : out.arm.c_str());
        host.life_emit(unit_index, label, host.life_clock, director, true);
    }
    static_cast<void>(ring);
    static_cast<void>(heading_radians);
    return out;
}

void GameCommandsHost::bind_command_target_facts(
    const GameCommandTargetFactsSource* source) noexcept {
    impl_->target_facts = source;
}

GameDirectorStepOutcome GameCommandsHost::director_step_00836920(std::size_t unit_index,
    bool player_controlled, float mission_clock, const bsp::UnitOrderRing& ring,
    float heading_radians) {
    Impl& host = *impl_;
    GameDirectorStepOutcome outcome;
    if (unit_index >= host.units.size()) return outcome;
    if (!host.logged_director_step) {
        host.logged_director_step = true;
        host.log.notef("weapon director step 00836920 runs once per unit per fixed "
            "simulation step: the pre-pass 00836941, the `stop` arm 00836a8b and the idle "
            "tail 00836dc9 that re-issues a default command. Its own caller is the unit "
            "update's director block, which this process does not reach, so the position "
            "in the step is the executable's decision and is recorded as one. The `follow`, "
            "`attackmove` and `moveonpath` arms 00836adc..00836d66 are read in "
            "docs/UNIT_COMMANDED_SPEED.md and projected nowhere, so they are records");
    }
    if (!kDirectorCommandArmsBound) {
        host.record("WeaponDirector::step_command_arms", 0x00836adcu);
    }

    GameDirector& director = host.directors[unit_index];
    if (host.player_184.size() < host.units.size()) host.player_184.resize(host.units.size(), 0);
    host.player_184[unit_index] = player_controlled ? 1 : 0;
    GameCommandRow row;
    row.unit_index = unit_index;
    row.unit = host.units[unit_index].name;
    row.source = "director idle tail";
    row.resolve_outcome = "idle_reissue";

    ChainState chain{host, host.units[unit_index], director, &row, &ring, heading_radians,
        bsp::SceneCommandTarget{}, 0u, 0u};
    DirectorStageBinding stage(chain, mission_clock, player_controlled);

    bsp::WeaponDirectorCommandState state;
    state.primary_stage = director.stage;
    state.secondary_stage = 0;
    state.primary_command = director.slot_command[0];
    state.secondary_command = 0;
    state.unit = static_cast<std::uint32_t>(unit_index + 1);
    state.unit_player_controlled = player_controlled;

    outcome.ran = true;
    ++host.summary.director_steps;
    host.life_clock = mission_clock;
    host.life_emit(unit_index, "step entry 00836920", mission_clock, director);
    outcome.prepass_flag = bsp::weapon_director_step_prepass_00836941(state, stage);
    host.done("WeaponDirector::step_prepass", 0x00836941u);
    state.primary_stage = director.stage;

    // Packet cc9_ship_natives_2: 008369A0..00836A81, the generic arrival. With
    // more than one command queued (CMP EDI,EBP; JLE), the stage below 1, the head
    // command's category (vtable[0Ch]) neither 1 nor 2 and the LAST queued
    // command's 1 or 2, the head ends once the unit is within 2000 of that last
    // command's target: 00427EB0 on both, x and z, the sum stored to float at
    // 00836A64 and compared against the double 4000000.0 at 00D09FE8.
    if (kDirectorCommandArmsBound && host.target_facts != nullptr) {
        const int count = host.command_count(director);
        if (count > 1 && director.stage < 1) {
            const bsp::EntityOrderCommandClass* head = host.class_of(director.slot_command[0]);
            const bsp::EntityOrderCommandClass* last =
                host.class_of(director.slot_command[count - 1]);
            const int head_category = head != nullptr ? head->category : -1;
            const int last_category = last != nullptr ? last->category : -1;
            if (bsp::command_arrival_gate_open(count, director.stage, head_category,
                                               last_category)) {
                const std::uint32_t target =
                    resolve_command_target_00521ea0(director.slot_target[count - 1]);
                GameCommandTargetFacts tf{};
                GameCommandTargetFacts uf{};
                if (target != 0u && host.target_facts->command_target_facts(target - 1u, tf)
                    && host.target_facts->command_target_facts(unit_index, uf)) {
                    const float dx = uf.position_x - tf.position_x;
                    const float dz = uf.position_z - tf.position_z;
                    const float squared = static_cast<float>(
                        static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
                    if (bsp::kCommandArrivalRadiusSquared > static_cast<double>(squared)) {
                        stage.director_raise_primary_stage_0071d810(2); // 00836A7C
                        state.primary_stage = director.stage;
                        outcome.arrival_raised = true;
                    }
                }
            }
        }
        host.done("WeaponDirector::generic_arrival", 0x00836a6cu);
    }
    // 00836962..00836985. The prepass is the only arm that can end a running
    // `moveonpath` short of arrival: it raises the stage when the filled-slot
    // count is above one, or when the unit is the controlled one.
    host.life_emit(unit_index, "prepass 00836941", mission_clock, director);

    outcome.stop_arm_raised = bsp::weapon_director_stop_arm_00836a8b(state,
        host.params_of(unit_index), stage);
    host.done("WeaponDirector::stop_arm", 0x00836a8bu);
    state.primary_stage = director.stage;
    host.life_emit(unit_index, "stop arm 00836a8b", mission_clock, director);

    // Packet cc9_target_release: the `attackmove` arm, 00836B45..00836BEB, when
    // slot 0 holds the attackmove command object (00836B45 CMP EAX,0E08F78h).
    // 00836A81 CMP [ESI+48h],2; JE 00836DC9: a finished stage skips every arm.
    const bool arms_run = !kDirectorCommandArmsBound || director.stage != 2;
    if (arms_run && kDirectorCommandArmsBound
        && director.slot_command[0] == bsp::kCommandedSpeedFollowObject) {
        // 00836ADC..00836B40, the `follow` arm. unit+284h and 007788D0 are the
        // pair the units host publishes (set_unit_formation): a follower's group
        // has a leader that is not the unit. A unit with no published pair is
        // either in no group or leads its own, and both end the command.
        const GameCommandUnit& unit = host.units[unit_index];
        bsp::WeaponDirectorFollowArmInputs in;
        in.has_group = unit.formation_follower;
        in.leader = unit.formation_follower
            ? static_cast<std::uint32_t>(unit.formation_leader + 1u) : 0u;
        in.unit = static_cast<std::uint32_t>(unit_index + 1u);
        in.command_target = resolve_command_target_00521ea0(director.slot_target[0]);
        in.filled_command_slots = host.command_count(director);
        if (bsp::weapon_director_follow_arm_00836adc(in)) {
            stage.director_raise_primary_stage_0071d810(2); // 00836B3B
            state.primary_stage = director.stage;
            outcome.follow_arm_raised = true;
        }
        host.done("WeaponDirector::follow_arm", 0x00836adcu);
    }
    if (arms_run && host.target_facts != nullptr
        && director.slot_command[0] == bsp::kCommandAttackMove) {
        bool raise = false;
        const char* why = "";
        // 00836B50..00836B5C: 00521EA0 on director+58h, the slot-0 descriptor.
        const std::uint32_t target = resolve_command_target_00521ea0(director.slot_target[0]);
        GameCommandTargetFacts facts{};
        GameCommandTargetFacts own{};
        if (target == 0u || !host.target_facts->command_target_facts(target - 1u, facts)) {
            raise = true;                                  // 00836B5C JE 00836BB2
            why = "target gone";
        } else if (facts.nav_point_41) {
            // 00836B6B JNZ 00836D67: nothing.
        } else if (facts.command_building_1c) {
            // 00836B80..00836BAD: +5Eh set or the session's own side converts the
            // command to `moveto` (00465080, 0071ECF0) and then raises stage 2.
            // The conversion is not issued here, so the branch stays a record.
            // LABELLED: the arm is reached (USN13 1002, USN01 8 on reference i), but
            // the conversion's test never holds there: every building is another
            // party's and not destroyed, so the image keeps the attack-move too
            // (docs/GUNNERY_OPEN_ITEMS.md section 35). A captured or destroyed
            // building target would need the conversion, which is not issued.
            // Packet cc9_attackmove_building_arm, diagnostic: 00836B80 converts only when
            // +5Eh is set, or 00836B86..00836B8F finds the building's party +54h equal
            // to the unit's ([director+34h]+54h); otherwise JNZ 00836D67 keeps it.
            ++host.building_arm_hits;
            const bool own_ok = host.target_facts->command_target_facts(unit_index, own);
            const bool convert = facts.flag_05e || (own_ok && own.side_0054 == facts.side_0054);
            if (convert) {
                ++host.building_arm_converts;
                if (host.building_arm_traced < 12) {
                    ++host.building_arm_traced;
                    host.log.notef("attackmove building arm 00836b95: unit=%zu \"%s\" target=%u "
                        "flag_05e=%d target_side=%d own_side=%d at %.2f s (packet "
                        "cc9_attackmove_building_arm)", unit_index,
                        host.units[unit_index].name.c_str(), target, facts.flag_05e ? 1 : 0,
                        facts.side_0054, own.side_0054, static_cast<double>(mission_clock));
                }
            }
            host.record("WeaponDirector::attackmove_arm_building_moveto_00836b95", 0x00836b95u);
        } else if (!facts.live_0043f080) {
            raise = true;                                  // 00836BC9 JE 00836BB2
            why = "target not live (0043f080)";
        } else if (host.target_facts->command_target_facts(unit_index, own)) {
            // 00836BCB..00836BDC: 005457C0(ECX = [director+24Ch], target+54h),
            // `unit+54h != side && side != 2`, false raises stage 2.
            const bool hostile = own.side_0054 != facts.side_0054 && facts.side_0054 != 2;
            if (!hostile) {
                raise = true;
                why = "target not hostile (005457c0)";
            }
        }
        host.done("WeaponDirector::attackmove_arm", 0x00836b45u);
        if (raise) {
            outcome.attackmove_arm_raised = true;
            const std::size_t before = static_cast<std::size_t>(host.command_count(director));
            stage.director_raise_primary_stage_0071d810(2); // 00836BB6 / 00836BE6
            state.primary_stage = director.stage;
            host.log.notef("attackmove arm 00836b45: unit=%zu \"%s\" target=%u %s at %.2f s; "
                "stage 2, queue %zu -> %d", unit_index, host.units[unit_index].name.c_str(),
                target, why, static_cast<double>(mission_clock), before,
                host.command_count(director));
        }
        host.life_emit(unit_index, "attackmove arm 00836b45", mission_clock, director);
    }

    outcome.reissued = bsp::weapon_director_idle_reissue_00836dc9(state,
        outcome.prepass_flag, stage.post_reset, stage);
    host.done("WeaponDirector::idle_reissue", 0x00836dc9u);

    const std::uint32_t reissued_object = static_cast<std::uint32_t>(outcome.reissued);
    if (outcome.reissued != bsp::DirectorDefaultCommand::None
        && reissued_object != director.last_idle_command) {
        director.last_idle_command = reissued_object;
        ++host.summary.idle_reissues;
        switch (outcome.reissued) {
        case bsp::DirectorDefaultCommand::Stop: ++host.summary.idle_stop; break;
        case bsp::DirectorDefaultCommand::Cruise: ++host.summary.idle_cruise; break;
        case bsp::DirectorDefaultCommand::Follow: ++host.summary.idle_follow; break;
        case bsp::DirectorDefaultCommand::None: break;
        }
        const bsp::EntityOrderCommandClass* klass = host.class_of(reissued_object);
        if (klass != nullptr) {
            row.command = klass->name;
            row.token = klass->name;
            row.ordinal = klass->ordinal;
            row.category = klass->category;
        }
        if (kSetCommandQueueDelayBound && chain.setcmd_post < host.loopback.size()) {
            // Packet cc9_set_command_queue_delay. 0071ED81 queued the SETCMD for
            // row 9, so the push and the finish tail happen at its delivery.
            host.rows.push_back(row);
            Impl::LoopbackMessage& posted = host.loopback[chain.setcmd_post];
            posted.row = host.rows.size() - 1;
            posted.finish = true;
        } else {
            finish_issue(host, chain, row, ring);
        }
    }
    host.life_emit(unit_index, "idle tail 00836dc9", mission_clock, director);
    return outcome;
}

// ---------------------------------------------------------------------------
// Packet cc8_ship_moveonpath: the slot-0 path cursor
// ---------------------------------------------------------------------------

bool GameCommandsHost::set_path_follow_pair_0071c1b0(std::size_t unit_index,
    int follow_mode, int start_mode) {
    Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    GameDirector& director = host.directors[unit_index];
    director.path_follow_mode = follow_mode;   // 0071C1D2, [slot+8h]
    director.path_start_mode = start_mode;     // 0071C1D8, [slot+0Ch]
    host.done("Navigator::path_object_set_follow_mode", 0x0071c1b0u);
    return true;
}

// Packet cc8_ship_drive. 0071F600 is not reached from the 5Bh message. Its only
// caller is 00835D33, inside 00835C70 BSP_WeaponDirector_BeginCurrentCommand,
// and 0071F62F takes `MOV EBP,[ESI+54h]`: the command it begins is SLOT 0's, the
// queue head. The 49 repeats a USN04 carrier's script sends therefore do not
// each build a path - 0071E6C0 refuses a push that repeats the slot below
// (0071E70C/0071E721), so they queue nothing new and begin nothing.
//
// Before this gate the host rebuilt on every message, which reset the cursor's
// join index, `advances` and `travelled` every 3.06 s. That is the whole of the
// previous packet's "0 advances in 900 tries" and its `travelled 48.30`: 48.30 m
// is one re-issue interval of motion, i.e. the Yorktown steaming at 15.8 m/s.
std::vector<GameFireTargetRequest> GameCommandsHost::take_fire_target_requests() {
    std::vector<GameFireTargetRequest> out;
    out.swap(impl_->fire_target_requests);
    return out;
}

bool GameCommandsHost::begin_current_command_00835c70(std::size_t unit_index,
    float unit_x, float unit_z) {
    Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    GameDirector& director = host.directors[unit_index];
    if (director.slot_command[0] != 0x00e08f80u) {
        // 00835C70 begins whatever the head is; a `moveonpath` that is not the
        // head has not begun, and the next time it does it begins afresh.
        director.path_begun = false;
        return false;
    }
    if (director.path_begun) return director.path_built && !director.path_points.empty();
    if (director.slot_target[0].kind == 0 && !director.user_points.empty()) {
        // Packet cc9_director_moveonpath_route. 0071F6A5's descriptor-kind-0 arm:
        // slot 0's vtable[8] resets it, BSP_EntityPathSource_CreateForSlot wraps
        // its own point vector, and the cursor starts with the pair {1, 5}
        // (0071F6B8/0071F6C0) instead of 0071C1B0's.
        director.path_begun = true;
        director.path_user = true;
        director.pending_path_name = "<user path>";
        director.pending_path_points = director.user_points;
        director.path_follow_mode = 1;
        director.path_start_mode = 5;
        host.done("WeaponDirector::begin_user_path_0071f6a5", 0x0071f6a5u);
        return build_path_object_0071f600(unit_index, unit_x, unit_z);
    }
    director.path_user = false;
    if (!director.pending_path_valid) return false;
    director.path_begun = true;
    return build_path_object_0071f600(unit_index, unit_x, unit_z);
}

bool GameCommandsHost::begin_path_command_0071f600(std::size_t unit_index,
    const std::string& path_name, const std::vector<std::array<float, 3>>& points,
    float unit_x, float unit_z) {
    Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    GameDirector& director = host.directors[unit_index];
    director.pending_path_name = path_name;
    director.pending_path_points = points;
    director.pending_path_valid = true;
    // The command may already be the head - the first one a carrier receives
    // lands under an authored `cruise`, but a later one can begin at once - so
    // the begin is offered here as well as from the director step.
    begin_current_command_00835c70(unit_index, unit_x, unit_z);
    return !points.empty();
}

bool GameCommandsHost::build_path_object_0071f600(std::size_t unit_index,
    float unit_x, float unit_z) {
    Impl& host = *impl_;
    GameDirector& director = host.directors[unit_index];
    const std::string& path_name = director.pending_path_name;
    const std::vector<std::array<float, 3>>& points = director.pending_path_points;
    // 0071F6C6/0071F6CD answer null for an entity with no path interface and
    // 007B22A0 still builds the source, so the build always happens and a
    // zero-point source is what makes 007ADC30 true on the first state step.
    // `path_built` therefore means "a build ran", and the empty test is separate.
    director.path_points = points;
    director.path_name = path_name;
    director.path_built = true;
    director.path_advances = 0;
    director.path_travelled = 0.0f;
    director.path_last_valid = false;
    director.path_visited.clear();
    if (points.empty()) {
        director.path_cursor = bsp::ShipAiPathCursor{};
        director.path_cursor.follow_mode_0c = director.path_follow_mode;
        director.path_start_index = 0;
        return false;
    }
    // 007B11F0's head: the nearest point, then its neighbour in the travel
    // direction as a second candidate. The projection test that chooses between
    // them (007B1263 onwards) was not read, so this takes the nearest alone.
    std::vector<float> flat;
    flat.reserve(points.size() * 3);
    for (const std::array<float, 3>& point : points) {
        flat.push_back(point[0]);
        flat.push_back(point[1]);
        flat.push_back(point[2]);
    }
    const int joined = bsp::ship_ai_path_nearest_index_007b1100(flat.data(),
        static_cast<int>(points.size()), unit_x, unit_z);
    // 007B1C50. `random_forward` is only consulted for PATH_SM_JOIN_RANDOM_DIR,
    // and the draw 00BD2F10 makes is not reproduced: this host keeps forward,
    // which is what start modes 5 and 6 give anyway and what every USN04 call
    // asks for (the eight sites pass three arguments, so the start mode is the
    // 008A3734 default 5).
    bsp::ship_ai_path_cursor_start_007b1c50(director.path_cursor,
        director.path_follow_mode, director.path_start_mode, joined,
        /*random_forward=*/true);
    director.path_start_index = director.path_cursor.index_08;
    director.path_visited.push_back(director.path_cursor.index_08);
    host.done("WeaponDirector::path_build_0071f600", 0x0071f600u);
    return true;
}

bool GameCommandsHost::path_cursor_has_no_legs_007adc30(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return true;
    const GameDirector& director = host.directors[unit_index];
    // 007ADC30: no path object, or a point count that is not positive.
    return !director.path_built || director.path_points.empty();
}

bool GameCommandsHost::path_cursor_on_final_leg_007adc60(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return true;
    const GameDirector& director = host.directors[unit_index];
    return bsp::ship_ai_path_on_final_leg_007adc60(director.path_cursor,
        director.path_built, static_cast<int>(director.path_points.size()));
}

bool GameCommandsHost::path_cursor_point(std::size_t unit_index, int leg,
    float& x, float& z) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    const GameDirector& director = host.directors[unit_index];
    if (!director.path_built || director.path_points.empty()) return false;
    const int count = static_cast<int>(director.path_points.size());
    int index = leg < 0 ? director.path_cursor.index_08 : leg;
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    x = director.path_points[static_cast<std::size_t>(index)][0];
    z = director.path_points[static_cast<std::size_t>(index)][2];
    return true;
}

bool GameCommandsHost::advance_path_cursor_00836bf0(std::size_t unit_index,
    float unit_x, float unit_z, float unit_radius, float turn_radius) {
    Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    GameDirector& director = host.directors[unit_index];
    if (!director.path_built || director.path_points.empty()) return false;
    // 00836BF0 CMP EAX,0xe08f80: the arm runs only while the director's current
    // command is `moveonpath`.
    if (director.slot_command[0] != 0x00e08f80u) return false;

    if (director.path_last_valid) {
        const float mx = unit_x - director.path_last_x;
        const float mz = unit_z - director.path_last_z;
        director.path_travelled += std::sqrt(mx * mx + mz * mz);
    }
    director.path_last_x = unit_x;
    director.path_last_z = unit_z;
    director.path_last_valid = true;

    // 00836C01-00836C39. The two doubles are read at their own width:
    // 00CE3DE0 = 2.5 and 00CEC160 = 1.2000000476837158.
    const double scaled_hull = static_cast<double>(unit_radius) * 2.5;
    const double scaled_turn = static_cast<double>(turn_radius) * 1.2000000476837158;
    const float radius = static_cast<float>(scaled_hull < scaled_turn ? scaled_hull
                                                                      : scaled_turn);

    const int count = static_cast<int>(director.path_points.size());
    int index = director.path_cursor.index_08;
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    const std::array<float, 3>& point = director.path_points[static_cast<std::size_t>(index)];
    const float dx = point[0] - unit_x;
    const float dz = point[2] - unit_z;
    // 007ADDDE FMUL ST0 squares the radius and 007ADE4D-007ADE55 squares the
    // planar delta, so the comparison is between squares and never takes a root.
    const bool within = (dx * dx + dz * dz) <= (radius * radius);
    host.done("WeaponDirector::path_follow_00836bf0", 0x00836bf0u);
    if (!within) return false;

    // 007ADFAC-007ADFD4. A refusal here is the ONLY way 007ADD70 answers true,
    // because every advance clears the flag 007ADFFE tests.
    if (!bsp::ship_ai_path_advance_allowed_007adfac(director.path_cursor,
            director.path_built, count)) {
        return true;
    }
    // 007ADFDF and 007ADFE9. NOT PROJECTED: 007ADD70's loop can take several
    // legs in one step, with the count coming from the remaining-leg arithmetic
    // at 007ADD88-007ADDD3 and the atan2 lookahead at 007ADE5D-007ADF80 that
    // decides how far to skip. This host takes one leg per director step.
    const int next = bsp::ship_ai_path_next_index_007adcc0(director.path_cursor, count);
    director.path_cursor.index_08 = next;
    ++director.path_advances;
    if (director.path_visited.size() < 512) director.path_visited.push_back(next);
    host.done("WeaponDirector::path_cursor_advance_007adcc0", 0x007adcc0u);
    return false;
}

std::vector<GamePathCursorRow> GameCommandsHost::path_cursor_rows() const {
    const Impl& host = *impl_;
    std::vector<GamePathCursorRow> rows;
    for (std::size_t i = 0; i < host.directors.size(); ++i) {
        const GameDirector& director = host.directors[i];
        if (!director.path_built) continue;
        GamePathCursorRow row;
        row.unit_index = i;
        row.unit = i < host.units.size() ? host.units[i].name : std::string();
        row.path = director.path_name;
        row.points = director.path_points.size();
        row.follow_mode = director.path_follow_mode;
        row.start_mode = director.path_start_mode;
        row.start_index = director.path_start_index;
        row.index = director.path_cursor.index_08;
        row.forward = director.path_cursor.forward_10;
        row.final_leg = bsp::ship_ai_path_on_final_leg_007adc60(director.path_cursor,
            director.path_built, static_cast<int>(director.path_points.size()));
        row.advances = director.path_advances;
        row.travelled = director.path_travelled;
        std::string visited;
        const std::size_t shown = director.path_visited.size() < 24
            ? director.path_visited.size() : 24;
        for (std::size_t k = 0; k < shown; ++k) {
            if (k != 0) visited += ">";
            visited += std::to_string(director.path_visited[k]);
        }
        if (director.path_visited.size() > shown) visited += ">...";
        row.visited = visited;
        rows.push_back(row);
    }
    return rows;
}

bool GameCommandsHost::holds_cruise(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    const GameDirector& director = host.directors[unit_index];
    return director.latched
        && bsp::director_current_command_0071be40(director.mode, director.slot_command[0], 0u)
            == bsp::kCruiseCommandObjectAddress;
}

std::uint32_t GameCommandsHost::current_command_0071be40(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return 0u;
    const GameDirector& director = host.directors[unit_index];
    return bsp::director_current_command_0071be40(director.mode, director.slot_command[0], 0u);
}

// ---------------------------------------------------------------------------
// Milestone 2p: what the ship AI brain reads off the same director
// ---------------------------------------------------------------------------

bool GameCommandsHost::active_command_descriptor_0071eb60(std::size_t unit_index,
    bsp::SceneCommandTarget& out, int& mode) const {
    const Impl& host = *impl_;
    mode = 0;
    if (unit_index >= host.directors.size()) return false;
    const GameDirector& director = host.directors[unit_index];
    mode = static_cast<int>(director.mode);
    // 0071EB60's own three-way select, read from the body: mode 1 hands back
    // director+58h, which 0071E6C0 filled with slot 0's descriptor; mode 2 the
    // override descriptor at director+18Ch; anything else the lazily built
    // empty singleton at 00E19B98, whose +1h byte and +14h handle are zero.
    if (director.mode == bsp::CruiseCommandMode::QueuedSlots) {
        out = director.slot_target[0];
        return true;
    }
    if (director.mode == bsp::CruiseCommandMode::Override) {
        // 0071E7F0 BSP_WeaponDirector_SetOverrideCommand (0071E89E) is the
        // writer of director+188h / +18Ch that matters here (00835C92 is a LEA
        // that READS +18Ch; packet cc9_ship_natives_3), and this process reaches
        // it through no path: its call from 00721A40 is a record here
        // (WeaponDirector::queue_command). The descriptor is therefore the one
        // a fresh director carries, and the caller is told which arm it got.
        out = bsp::SceneCommandTarget{};
        return true;
    }
    return false;
}

std::uint32_t GameCommandsHost::resolve_command_target_00521ea0(
    const bsp::SceneCommandTarget& target) const {
    // 00521EA0 BSP_CommandTarget_ResolveObject reads the descriptor's +0h kind,
    // +2h object id and +4h object. This process numbers its own entities
    // because the two handle tables at 00f89a0c / 00f89a60 are not built, so
    // the id is matched against the register_units table and the answer is a
    // one-based created-instance handle.
    const Impl& host = *impl_;
    if (target.kind == 0 || target.object_id == 0) return 0u;
    for (const GameCommandUnit& unit : host.units) {
        if (unit.object_id != target.object_id) continue;
        return static_cast<std::uint32_t>(unit.index) + 1u;
    }
    return 0u;
}

std::size_t GameCommandsHost::release_observed_target_0071ddb0(
    const GameReleasedTarget& released) {
    // 0071DDB0, body 0071DDB0-0071DEA7 (RET 4, INT3 from 0071DEAA; no Ghidra
    // function), ECX = director+1Ch, the stack argument the released entity.
    // Reached through the observer vtable D09EA8: slot +8 directly (00696340 from
    // 00925C90, 00926390's death delivery) and slot +4 0071C1A0 (body
    // 0071C1A0-0071C1A6, `MOV EAX,[ECX]; MOV EAX,[EAX+8]; JMP EAX`, no Ghidra
    // function) from 00696330 (009263C0's removal). The second delivery finds
    // every matching descriptor already rewritten, so it does nothing.
    Impl& host = *impl_;
    if (released.unit >= host.units.size()) return 0;
    if (host.released_05d.size() != host.units.size())
        host.released_05d.assign(host.units.size(), 0);
    host.released_05d[released.unit] = 1;          // 00926390's +5Dh store
    ++host.summary.release_deliveries;
    // 0071DDC1..0071DDCD: [[00E188A8]+5D4h] >= 0Ch. The game state is 0Dh
    // (GameStateId::kInMission) whenever a unit can die in this process.
    // 0071DDD5..0071DE0D, the override descriptor at director+18Ch: nothing in
    // this process writes director+188h (override_command answers 0), so no
    // override descriptor resolves.
    const std::uint32_t handle = static_cast<std::uint32_t>(released.unit) + 1u;
    std::size_t matches = 0;
    for (std::size_t d = 0; d < host.directors.size(); ++d) {
        GameDirector& director = host.directors[d];
        // 0071DE12..0071DE9C: i = 0..9, the command at director+54h+1Ch*i and
        // its descriptor at +58h+1Ch*i; a null command is skipped, not a stop.
        for (int i = 0; i < bsp::kDirectorCommandSlotCount; ++i) {
            if (director.slot_command[i] == 0) continue;               // 0071DE2D
            bsp::SceneCommandTarget& target = director.slot_target[i];
            if (host.resolve_target_00521ea0(target) != handle) continue;  // 0071DE47
            ++matches;
            ++host.summary.release_slot_matches;
            if (released.is_plane) ++host.summary.release_plane_matches;
            // 0071DE57 director vtable[70h] = 0071EDD0(descriptor, 1). For an
            // aircraft (vtable[5Ch](0Fh)) with a squadron at +9D4h it moves the
            // descriptor to the LAST live member among the squadron's first five
            // (0071EEEF compares d^2 against FLT_MAX, 00D7A278, so every live
            // member qualifies) and answers 0. Otherwise it rewrites the
            // descriptor to the position branch at the entity's +FCh (kind 0,
            // +1h = 1, no object) and answers 1.
            // SUBSTITUTION: this host holds no squadron member list, so an
            // aircraft takes the position branch; release_plane_matches counts
            // every descriptor that would have been offered the retarget.
            const bsp::EntityOrderCommandClass* klass = host.class_of(director.slot_command[i]);
            const int category = klass != nullptr ? klass->category : -1;
            if (i == 0) {
                ++host.summary.release_head_ends;
            } else if (category == 1 || category == 2) {   // 0071DEB0, i > 0
                ++host.summary.release_slot_clears;
            } else {
                ++host.summary.release_slot_kept;
            }
            if (!kDirectorTargetChecksBound) continue;
            target = bsp::SceneCommandTarget{};
            target.kind = 0;
            target.position_valid = 1;
            for (int lane = 0; lane < 3; ++lane) target.position[lane] = released.position[lane];
            host.done("WeaponDirector::release_retarget_0071edd0", 0x0071edd0u);
            // 0071DE5D: [[00E188A8]+1FE4h] == 2 skips; this process is mode 1.
            const bool player = released.controlled_unit == d;
            if (i == 0) {
                // 0071DE72 0071D810(2), monotonic, then the 5Dh round trip.
                if (bsp::stage_raise_applies(director.stage, 2)) {
                    director.stage = 2;
                    ++host.summary.stage_raises;
                    host.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
                    if (bsp::stage_raise_sends_message(2, kSessionModeSinglePlayer)) {
                        host.done("WeaponDirector::build_clear_command", 0x0071c730u);
                        host.route_clear_command(d, player,
                            bsp::clear_command_message_for_queue_stage_done());
                    }
                }
            } else if (category == 1 || category == 2) {
                // 0071DE7B..0071DE91: 0071DEB0(command, i) true, 0071D900(i).
                host.done("WeaponDirector::send_clear_command_slot", 0x0071d900u);
                host.route_clear_command(d, player, bsp::clear_command_message_for_slot(i));
            }
            host.done("WeaponDirector::release_observed_target", 0x0071ddb0u);
        }
    }
    return matches;
}

bool GameCommandsHost::command_accepts_target_0071d6d0(std::uint32_t command,
                                                       std::uint32_t handle) {
    Impl& host = *impl_;
    bsp::SceneCommandTarget target{};                  // 00465080
    if (handle != 0u && handle - 1u < host.units.size()) {
        target.kind = 1;
        target.object_id = host.units[handle - 1u].object_id;
        target.object = &host.units[handle - 1u];
    }
    const bsp::EntityOrderCommandClass* klass = host.class_of(command);
    if (klass == nullptr) return false;
    if (!klass->requires_target)                       // 0071D6E5
        return host.command_extra_test_0071d71f(command, target);
    if (target.position_valid != 0 && klass->category != 1 && klass->category != 2)
        return host.command_extra_test_0071d71f(command, target);  // 0071D6EB..0071D6FE
    const std::uint32_t resolved = host.resolve_target_00521ea0(target);  // 0071D702
    if (resolved == 0u) return false;                  // 0071D709
    if (host.target_released_05d(resolved)) return false;  // 0071D712
    return host.command_extra_test_0071d71f(command, target);
}

float GameCommandsHost::director_target_hold_0040(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return 0.0f;
    return host.directors[unit_index].target_hold_0040;
}

int GameCommandsHost::director_leading_slot_categories_0071df83(std::size_t unit_index,
    int* out, int max_out) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size() || out == nullptr || max_out <= 0) return 0;
    const GameDirector& director = host.directors[unit_index];
    // 0071DF83..0071DF9E: walk director+54h in 1Ch steps from index 0 and stop
    // at the first null command pointer, capped at ten slots. A gap hides every
    // slot behind it, which is what 0071E6C0's fill-the-first-null and
    // 00720850's shift-the-tail-down together guarantee.
    int written = 0;
    for (int i = 0; i < bsp::kDirectorCommandSlotCount && written < max_out; ++i) {
        const std::uint32_t command = director.slot_command[i];
        if (command == 0u) break;
        const bsp::EntityOrderCommandClass* klass = host.class_of(command);
        out[written++] = (klass != nullptr) ? klass->category : -1;
    }
    return written;
}

std::uint32_t GameCommandsHost::director_slot_command(std::size_t unit_index,
    int slot_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return 0u;
    if (slot_index < 0 || slot_index >= bsp::kDirectorCommandSlotCount) return 0u;
    return host.directors[unit_index].slot_command[slot_index];
}

std::uint64_t GameCommandsHost::director_head_key(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return 0u;
    const GameDirector& director = host.directors[unit_index];
    if (director.slot_command[0] == 0u) return 0u;
    return (static_cast<std::uint64_t>(director.slot_command[0]) << 16)
        | static_cast<std::uint64_t>(director.slot_target[0].object_id);
}

const char* GameCommandsHost::command_name_of(std::uint32_t command_object) const {
    const bsp::EntityOrderCommandClass* klass = impl_->class_of(command_object);
    return (klass != nullptr) ? klass->name : "";
}

bool GameCommandsHost::cruise_step(std::size_t unit_index, bool player_controlled,
    float body_axis_speed, float reference_speed, bsp::CruiseOrderedValues& out,
    bsp::ShipAiControlBlock* blk, bsp::ShipAiSetterHost* setters,
    bsp::ShipAiAvoidanceRequest* request,
    const bsp::ShipAiCruiseAvoidanceInputs* avoidance_inputs) {
    Impl& host = *impl_;
    if ((request == nullptr) != (avoidance_inputs == nullptr) || (request && !blk))
        return false;
    if (!holds_cruise(unit_index)) return false;
    bsp::ShipAiCruiseAvoidanceInputs live_inputs{};
    if (request != nullptr) {
        live_inputs = *avoidance_inputs;
        live_inputs.unit_player_controlled = player_controlled;
        const auto arm = bsp::ship_ai_cruise_step_arm_009e11a5(live_inputs);
        if (arm != bsp::ShipAiCruiseAvoidanceArm::CruiseRule) {
            bsp::ShipAiAvoidanceRequestBlock block{*request, blk->early_out_3f5};
            bsp::ship_ai_cruise_step_request_009e11d6(block, live_inputs);
            *request = block.request;
            blk->early_out_3f5 = block.early_out_3f5;
            if (arm == bsp::ShipAiCruiseAvoidanceArm::HelmHeldByPlayer) {
                host.done("CruiseState::publish_helm_avoidance_request", 0x009e13b6u);
                host.record("CruiseState::helm_drive_arm", 0x009e13b4u);
            } else {
                host.done("CruiseState::publish_player_avoidance_request", 0x009e11d6u);
                host.record("CruiseState::player_controlled_arm", 0x009e11e8u);
            }
            return false;
        }
    }
    if (player_controlled) {
        // 009e11e8..009e1262: with unit+184h set 009e1170 forwards the ring's
        // confirmed pair at unit+998h / unit+994h to 009dffb0 and 009dbf90 and
        // never reads a cruise field. docs/CRUISE_COMMAND.md reads that arm and
        // projects none of it, so it is a record rather than a substitution.
        host.record("CruiseState::player_controlled_arm", 0x009e11e8u);
        return false;
    }
    if (!host.logged_step) {
        host.logged_step = true;
        // Milestone 2n corrects milestone 2l here. 009e1170 is the `cruise`
        // state's vtable +0Ch, and 009f5186 calls it only on a re-plan tick, at
        // most once every state->vtable[28h]() * 0.05f seconds; the controller
        // 009f50e0 is what schedules it, and this process now runs that
        // controller once per unit per fixed simulation step.
        host.log.notef("cruise state step 009e1170 runs on a re-plan tick of the ship AI "
            "controller 009f50e0, not on every step: 009f3dd0 keeps the state in sync with "
            "0071be40's answer and 009f519e reloads the interval from the state's own "
            "vtable +28h (009dac30, 2.0 ticks of 0.05 s for `cruise`)");
        host.log.notef("the three desired-value setters 009dbf90 / 009dffb0 / 009e0040 now "
            "write the AI control block blk = brain+8h through their reconstructions, and "
            "009ed6b0's direct-control arm and 009f4d10 carry what they wrote into the "
            "unit's own 84-byte AI order slot (docs/SHIP_AI_STATES.md, "
            "docs/UNIT_AUTOPILOT_PAIR.md)");
    }
    ChainState chain{host, host.units[unit_index], host.directors[unit_index], nullptr,
        nullptr, 0.0f, bsp::SceneCommandTarget{}, 0u, 0u, blk, setters,
        request, request != nullptr ? &live_inputs : nullptr};
    DirectorBinding binding(chain);
    binding.body_speed = body_axis_speed;
    binding.reference = reference_speed;
    out = bsp::cruise_state_step_009e1170(binding);
    host.done("ShipAiState::cruise_step", 0x009e1170u);
    const bsp::CruiseSpeedSetting speed = host.params_of(unit_index);
    if (bsp::navigator_commanded_speed_active(speed) && !host.logged_commanded_step) {
        host.logged_commanded_step = true;
        host.log.notef("cruise state with an active commanded speed on \"%s\": "
            "009e12bd divided +24h %.3f m/s by 0080fc30's reference %.3f and asked the AI "
            "controller for throttle %.6f, steer mode %d, value %.4f. That triple reaches "
            "009dbf90 / 009dffb0 / 009e0040 and stops there: the hop from the controller "
            "block to unit+0fc4h / unit+0fdch has no recovered writer",
            host.units[unit_index].name.c_str(), static_cast<double>(speed.speed),
            static_cast<double>(reference_speed), static_cast<double>(out.throttle),
            static_cast<int>(out.mode), static_cast<double>(out.steer_or_heading));
    }
    ++host.summary.steps;
    for (GameCommandRow& row : host.rows) {
        if (row.unit_index == unit_index) ++row.steps;
    }
    return true;
}

const std::vector<GameCommandRow>& GameCommandsHost::rows() const noexcept {
    return impl_->rows;
}

const GameCommandsSummary& GameCommandsHost::summary() const noexcept {
    return impl_->summary;
}

void GameCommandsHost::report() {
    Impl& host = *impl_;
    if (host.rows.empty()) return;
    host.log.notef("  %-20s %-11s %-20s %4s %4s %5s %5s %5s %9s %9s %9s", "unit", "command",
        "source", "ord", "cat", "issue", "slot", "curr", "latch", "steer", "thrust");
    for (const GameCommandRow& row : host.rows) {
        host.log.notef("  %-20s %-11s %-20s %4d %4d %5d %5d %5d %9s %9.3f %9.3f",
            row.unit.c_str(), row.command.empty() ? row.token.c_str() : row.command.c_str(),
            row.source.c_str(),
            row.ordinal, row.category, row.issued ? 1 : 0, row.slot_pushed ? 1 : 0,
            row.current ? 1 : 0,
            row.latched ? (row.fields.is_heading ? "heading" : "rudder") : "-",
            static_cast<double>(row.fields.steer_or_heading),
            static_cast<double>(row.fields.thrust));
    }
    // The token census. A token the 26-row registry does not carry ends its
    // record at 0046ab13 and issues nothing, which is what the scene's own
    // default does: `properties Command` in universe/library/commandunit.props
    // declares `Command = E CommandType : None`, and `None` is that enum's
    // value 1 rather than a command class.
    struct TokenTally {
        std::string token;
        std::string resolved;
        std::size_t count{0};
    };
    std::vector<TokenTally> tally;
    for (const GameCommandRow& row : host.rows) {
        bool found = false;
        for (TokenTally& entry : tally) {
            if (entry.token == row.token && entry.resolved == row.command) {
                ++entry.count;
                found = true;
                break;
            }
        }
        if (!found) tally.push_back(TokenTally{row.token, row.command, 1});
    }
    for (const TokenTally& entry : tally) {
        if (entry.resolved.empty()) {
            host.log.notef("  authored token \"%s\" x%zu resolves to no command class: "
                "0046aab0's case-insensitive first-match walk over the registry at "
                "00e19a70 finds nothing and ends the record at 0046ab13, so no MT_COMMAND "
                "is built for those units", entry.token.c_str(), entry.count);
        } else {
            host.log.notef("  authored token \"%s\" x%zu resolves to command class \"%s\"",
                entry.token.c_str(), entry.count, entry.resolved.c_str());
        }
    }
    // Packet cc8_ship_moveonpath: the slot-0 path cursor of every unit that was
    // ordered onto an authored path. `mode` is PATH_FM_* and `start` PATH_SM_*,
    // this installation's scripts/global/luamw_init.lua 224-232.
    {
        const std::vector<GamePathCursorRow> cursors = path_cursor_rows();
        if (!cursors.empty()) {
            host.log.notef("  path cursors (0071BFF0 slot 0, 009E59C0 reads, 00836BF0 moves)");
            host.log.notef("   %-20s %-16s pts mode start from  at dir final advances "
                "travelled legs", "unit", "path");
            for (const GamePathCursorRow& row : cursors) {
                host.log.notef("   %-20s %-16s %3zu %4d %5d %4d %3d %3s %5s %8llu %9.2f %s",
                    row.unit.c_str(), row.path.c_str(), row.points, row.follow_mode,
                    row.start_mode, row.start_index, row.index, row.forward ? "fwd" : "rev",
                    row.final_leg ? "yes" : "no", row.advances,
                    static_cast<double>(row.travelled), row.visited.c_str());
            }
        }
    }
    host.log.notef("summary mission commands units=%zu resolved=%zu issued=%zu pushed=%zu "
        "current=%zu latched=%zu with_thrust=%zu ai_groups=%zu ai_forwards=%zu steps=%llu",
        host.summary.units, host.summary.resolved, host.summary.issued, host.summary.pushed,
        host.summary.current, host.summary.latched, host.summary.moving,
        host.summary.ai_groups, host.summary.ai_forwards, host.summary.steps);
    host.log.notef("summary mission scene command find lookups=%llu exact=%llu case_only=%llu "
        "bound=%d (0046AB48 -> 00925A90 -> 009251F0 _stricmp, packet cc9_scene_command_find_case)",
        host.find_lookups, host.find_exact, host.find_case_only,
        kSceneCommandFindCaseInsensitiveBound ? 1 : 0);
    host.log.notef("summary mission director attackmove building arm hits=%llu converts=%llu "
        "(00836B95, packet cc9_attackmove_building_arm)", host.building_arm_hits,
        host.building_arm_converts);
    host.log.notef("summary mission director queue full tests=%llu with_queued_moveonpath=%llu "
        "(0071D780 weights; 0071FB90 path objects, packet cc9_director_slot_housekeeping)",
        host.queue_full_tests, host.queue_full_tests_queued_path);
    host.log.notef("summary mission director fire target unresolved=%llu by_object_id=%llu "
        "(00835930, packet cc9_unresolved_fire_target)", host.fire_unresolved,
        host.fire_unresolved_by_id);
    host.log.notef("summary mission director squadron set command rows=%llu "
        "forced_fire_targets=%llu bound=%d (00D0BDF8 -> 0071E6C0, packet "
        "cc9_squadron_set_command)", host.squadron_set_commands,
        host.squadron_fire_targets, kSquadronSetCommandBound ? 1 : 0);
    host.log.notef("summary mission director steps=%llu idle_reissues=%llu stop=%zu "
        "cruise=%zu follow=%zu script_issues=%zu blocked_at_00816f7c=%zu commanded_speeds=%zu",
        host.summary.director_steps, host.summary.idle_reissues, host.summary.idle_stop,
        host.summary.idle_cruise, host.summary.idle_follow, host.summary.script_issues,
        host.summary.script_blocked, host.summary.commanded_speeds);
    // Milestone 2q: the completion round trip, counted at every hop.
    host.log.notef("summary mission director completion end_commands=%llu stage_raises=%llu "
        "clear_messages=%llu clear_receives=%llu queue_advances=%llu restarts=%llu "
        "command_events=%llu event_callbacks=%llu",
        host.summary.end_commands, host.summary.stage_raises, host.summary.clear_messages,
        host.summary.clear_receives, host.summary.queue_advances, host.summary.restarts,
        host.summary.command_events, host.summary.command_event_callbacks);
    host.log.notef("summary mission director release bound=%d deliveries=%llu "
        "slot_matches=%llu head_ends=%llu slot_clears=%llu slot_kept=%llu "
        "plane_matches=%llu refusals=%llu (packet cc9_director_target_checks, 0071DDB0 / "
        "0071D712)", kDirectorTargetChecksBound ? 1 : 0,
        host.summary.release_deliveries, host.summary.release_slot_matches,
        host.summary.release_head_ends, host.summary.release_slot_clears,
        host.summary.release_slot_kept, host.summary.release_plane_matches,
        host.summary.target_refusals);
    host.log.notef("summary mission director extra tests bound=%d torpedo=%llu null=%llu kind=%llu "
        "fort_unread=%llu class_unknown=%llu path=%llu path_unit=%llu path_non_unit=%llu "
        "(0071D71F, 009229F0 / 007AC9D0, packet cc9_command_extra_tests)",
        kCommandExtraTestsBound ? 1 : 0, host.summary.torpedo_tests,
        host.summary.torpedo_refused_null, host.summary.torpedo_refused_kind,
        host.summary.torpedo_fort_unread, host.summary.torpedo_class_unknown,
        host.summary.path_tests, host.summary.path_refused_unit, host.summary.path_non_unit);
    host.log.notef("summary mission director loopback bound=%d command_posts=%llu "
        "setcmd_posts=%llu clear_posts=%llu in_place=%llu nested=%llu queued=%llu "
        "drains=%llu (packet cc9_set_command_queue_delay, 0076E520 / 0076C600)",
        kSetCommandQueueDelayBound ? 1 : 0, host.summary.loopback_command_posts,
        host.summary.loopback_setcmd_posts, host.summary.loopback_clear_posts,
        host.summary.loopback_delivered_in_place, host.summary.loopback_delivered_nested,
        host.summary.loopback_delivered_queued, host.summary.loopback_drains);
    host.log.notef("summary mission director clear-all bound=%d clear_all=%llu drops=%llu "
        "slot_clears_00720ca0=%llu (packet cc9_set_command_clear_all, 0071D880 / 0071D900)",
        kSetCommandClearAllMessageBound ? 1 : 0, host.summary.clear_all_calls,
        host.summary.drop_calls, host.summary.clear_all_slot_clears);
    {
        unsigned long long received = 0, queued = 0, dropped = 0, outside = 0;
        for (const GameDirector& d : host.directors) {
            received += d.user_points_received;
            queued += d.user_paths_queued;
            dropped += d.user_points_dropped;
            outside += d.user_points_outside_map;
        }
        host.log.notef("summary mission director user path points=%llu queued=%llu "
            "dropped=%llu outside_map=%llu clearorders_bound=%d clearorders=%llu (007207C0 / "
            "0071D880, packet cc9_director_moveonpath_route)", received, queued, dropped,
            outside, kClearOrdersSendBound ? 1 : 0, host.summary.clearorders_sends);
    }
}

// Packet cc9_formation_join_follow: 00720CD0 as the join 0077F940 calls it at 0077FAB8,
// directly, with no MT_COMMAND, no SETCMD and no 0071ECF0. The clear is
// apply_clear_command's kClearAllSlots walk (00720CA0); the push is the same
// director_set_command DirectorBinding runs (008358D0, so kSquadronSetCommandBound
// applies unchanged). Inert until a caller wires it (kFormationJoinFollowBound).
bool GameCommandsHost::issue_follow_command_00720cd0(std::size_t unit_index,
    std::size_t target_index) {
    Impl& host = *impl_;
    if (unit_index >= host.units.size() || target_index >= host.units.size()
        || unit_index >= host.directors.size()) {
        return false;
    }
    GameDirector& director = host.directors[unit_index];
    {
        // 00720CD8..00720CF8: 00720850 on each occupied slot from 9 down to 0.
        bsp::CommandQueueState state = queue_state_of(director);
        QueueClearBinding exec(host, host.units[unit_index], director, false);
        bsp::clear_all_command_slots_00720ca0(state, exec);
        queue_state_back(state, director);
        host.done("WeaponDirector::issue_target_command_clear", 0x00720ca0u);
    }
    GameCommandRow row;
    row.unit_index = unit_index;
    row.unit = host.units[unit_index].name;
    row.token = "follow";
    row.target_token = host.units[target_index].name;
    row.command = "follow";
    // 00720D2A..00720D5C: kind 1 (has_target_entity), no position, the id
    // [target+174h] (index + 1 in this host) and the zero floats at 00F87574.
    bsp::SceneCommandTarget target{};
    target.kind = 1u;
    target.position_valid = 0u;
    target.object_id = static_cast<std::uint16_t>(target_index + 1);
    target.object = &host.units[target_index];
    ChainState chain{host, host.units[unit_index], director, &row, nullptr, 0.0f,
        target, bsp::kCommandFollow, 0u};
    DirectorBinding binding(chain);
    const bool pushed = binding.director_set_command(bsp::kCommandFollow, target);  // 00720D6E
    row.issued = pushed;
    row.slot_pushed = pushed;
    host.rows.push_back(row);
    host.log.notef("  follow issued (00720CD0, source join 0077F940): \"%s\" -> \"%s\" "
        "pushed=%d", row.unit.c_str(), row.target_token.c_str(), pushed ? 1 : 0);
    host.done("WeaponDirector::issue_target_command", 0x00720cd0u);
    return pushed;
}

}  // namespace bsp::game
