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

#include "bsp/game_hosts.hpp"

#include "bsp/entity_orders.hpp"
#include "bsp/entity_command_arms.hpp"
#include "bsp/weapon_director.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

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
};

struct GameCommandsHost::Impl {
    explicit Impl(GameHostLog& log_in) : log(log_in) {}

    GameHostLog& log;
    std::vector<GameCommandUnit> units;
    std::vector<GameDirector> directors;
    // Milestone 2m. One navigator parameter block per unit, the 0081f283
    // allocation at *(unit+73Ch). Only the commanded-speed pair at +24h / +28h
    // has a recovered producer, and it is the pair the director's stage reset
    // 00835bf0, its `stop` arm 00836a8b, its idle tail 00836e59 and the cruise
    // state 009e12ac all read. The seven tuning floats at +0h..+18h stay out of
    // this process because 00822b70's only call site here passes the literal 0.
    std::vector<bsp::CruiseSpeedSetting> navigator_params;
    std::vector<GameCommandRow> rows;
    bsp::SceneCommandRegistry registry;
    GameCommandsSummary summary{};
    bool logged_path{false};
    bool logged_block{false};
    bool logged_step{false};
    bool logged_director_step{false};
    bool logged_navigator_params{false};
    bool logged_commanded_step{false};

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

    int command_count(const GameDirector& director) const noexcept {
        // 0071d780, __fastcall(director), body 0071d780-0071d807: the index of
        // the first empty slot. The `moveonpath` variant, where a slot whose
        // path object at director+1a4h+i*4 reports a non-empty point vector
        // contributes that point count instead of 1, needs a path object this
        // process does not build.
        int index = 0;
        while (index < bsp::kDirectorCommandSlotCount
            && director.slot_command[index] != 0) {
            ++index;
        }
        return index;
    }
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
};

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
        for (GameCommandUnit& unit : chain_.owner.units) {
            if (unit.name == name) return &unit;
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
        if (dropped && count > 0) {
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
        const bool pushed = bsp::director_set_command_008358d0(*this, command, local,
            kSessionModeSinglePlayer);
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
        static_cast<void>(target_object);
        chain_.owner.record("WeaponDirector::set_fire_target", 0x00835860u);
    }
    int command_count() override {
        chain_.owner.done("WeaponDirector::command_count", 0x0071d780u);
        return chain_.owner.command_count(chain_.director);
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
        if (command == 0x00e08f18u || command == 0x00e08f80u) {
            chain_.owner.record("WeaponDirector::command_allowed_extra_test",
                (command == 0x00e08f18u) ? 0x009229f0u : 0x007ac9d0u);
        }
        if (!klass->requires_target) return true;
        if (target.position_valid != 0 && klass->category != 1 && klass->category != 2) {
            return true;
        }
        if (target.object == nullptr) return false;
        // The resolved target's +5dh flag. Nothing in this process sets it on a
        // created instance, so the test answers the clear byte.
        chain_.owner.record("WeaponDirector::target_refuses_commands", 0x0071d74au);
        return true;
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
        chain_.owner.record("WeaponDirector::observe_target", 0x00694a60u);
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
    bool self_is_kind_of_vtable5c(int) override {
        chain_.owner.record_slot("EntityCommandArm::self_is_kind_of", "00cfc3d0+vtable5c");
        return false;
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
        chain_.owner.record("EntityCommandArm::set_fire_target", 0x00835860u);
    }
    bool controller_belongs_to_another_007788b0() override {
        chain_.owner.record("EntityCommandArm::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    void send_clear_commands_0071d880() override {
        chain_.owner.record("EntityCommandArm::send_clear_commands", 0x0071d880u);
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

    bsp::EntityCommandMessageView view;
    view.command_ordinal = message.command_ordinal;
    view.flags = message.flags;
    view.target.kind = static_cast<std::uint8_t>(message.target_kind & 0xffu);
    view.target.position_valid = static_cast<std::uint8_t>((message.target_kind >> 8) & 0xffu);
    view.target.object_id = message.target_id;
    view.target.object = const_cast<void*>(message.target_object);
    for (int lane = 0; lane < 3; ++lane) view.target.position[lane] = message.position[lane];
    view.target.reserved = message.trailing;

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
    chain_.pending_command = command;
    chain_.pending_target = target;
    chain_.pending_flag = flag;
    bsp::gameunit_apply_set_command_00721a40(*this, kSessionModeSinglePlayer, flag);
    chain_.owner.done("GameUnitMessage::apply_set_command", 0x00721a40u);
}

}  // namespace

// ---------------------------------------------------------------------------
// GameCommandsHost
// ---------------------------------------------------------------------------

GameCommandsHost::GameCommandsHost(GameHostLog& log)
    : impl_(std::make_unique<Impl>(log)) {}

GameCommandsHost::~GameCommandsHost() = default;

void GameCommandsHost::register_units(std::vector<GameCommandUnit> units) {
    Impl& host = *impl_;
    host.units = std::move(units);
    host.directors.assign(host.units.size(), GameDirector{});
    // 0081f273 / 0081f278 leave the pair at -1.0f, which is what
    // CruiseSpeedSetting's own defaults are, so a fresh block is the
    // constructor's state rather than a zeroed one.
    host.navigator_params.assign(host.units.size(), bsp::CruiseSpeedSetting{});
    host.summary.units = host.units.size();
    host.build_registry();
}

namespace {
// Defined below, after the hop bindings it uses.
const GameCommandRow* finish_issue(GameCommandsHost::Impl& host, ChainState& chain,
    GameCommandRow& row, const bsp::UnitOrderRing& ring);
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
const GameCommandRow* finish_issue(GameCommandsHost::Impl& host, ChainState& chain,
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

    host.rows.push_back(row);
    host.summary.command_name = row.command;
    return &host.rows.back();
}

}  // namespace

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
    DirectorStageBinding(ChainState& chain, float mission_clock)
        : chain_(chain), clock_(mission_clock),
          post_reset(chain.owner.params_of(chain.unit.index)) {}

    int director_filled_command_slots_0071be60() override {
        chain_.owner.done("WeaponDirector::filled_command_slots", 0x0071be60u);
        return chain_.owner.command_count(chain_.director);
    }
    void director_raise_primary_stage_0071d810(int stage) override {
        if (stage > chain_.director.stage) chain_.director.stage = stage;
        // The stage-2 completion message 0071c730 builds and 0077c2a0 routes is
        // the same session boundary every other hop in this file records.
        if (stage == bsp::kDirectorCommandStageFinished) {
            chain_.owner.record("WeaponDirector::route_stage_completion", 0x0071c730u);
        }
        chain_.owner.done("WeaponDirector::raise_primary_stage", 0x0071d810u);
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
        // entity+284h, the controller back-pointer. No controller object exists
        // in this process, which is the same boundary 0071ecf0's own AI-group
        // block reports, so the answer is recorded rather than read.
        chain_.owner.record("WeaponDirector::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    std::uint32_t unit_controller_owner_007788d0() override {
        chain_.owner.record("WeaponDirector::controller_owner", 0x007788d0u);
        return 0;
    }
    std::uint32_t make_command_target_00465080(std::uint32_t object, float range) override {
        static_cast<void>(range);
        target_ = bsp::SceneCommandTarget{};
        if (object != 0) {
            target_.kind = 1;
            target_.object = &chain_.unit;
            target_.object_id = chain_.unit.object_id;
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
    int secondary_stage_{0};
    bsp::SceneCommandTarget target_{};
};

}  // namespace

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
    host.record("WeaponDirector::step_command_arms", 0x00836adcu);

    GameDirector& director = host.directors[unit_index];
    GameCommandRow row;
    row.unit_index = unit_index;
    row.unit = host.units[unit_index].name;
    row.source = "director idle tail";
    row.resolve_outcome = "idle_reissue";

    ChainState chain{host, host.units[unit_index], director, &row, &ring, heading_radians,
        bsp::SceneCommandTarget{}, 0u, 0u};
    DirectorStageBinding stage(chain, mission_clock);

    bsp::WeaponDirectorCommandState state;
    state.primary_stage = director.stage;
    state.secondary_stage = 0;
    state.primary_command = director.slot_command[0];
    state.secondary_command = 0;
    state.unit = static_cast<std::uint32_t>(unit_index + 1);
    state.unit_player_controlled = player_controlled;

    outcome.ran = true;
    ++host.summary.director_steps;
    outcome.prepass_flag = bsp::weapon_director_step_prepass_00836941(state, stage);
    host.done("WeaponDirector::step_prepass", 0x00836941u);
    state.primary_stage = director.stage;

    outcome.stop_arm_raised = bsp::weapon_director_stop_arm_00836a8b(state,
        host.params_of(unit_index), stage);
    host.done("WeaponDirector::stop_arm", 0x00836a8bu);
    state.primary_stage = director.stage;

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
        finish_issue(host, chain, row, ring);
    }
    return outcome;
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
        // 00835C92 is the only writer of director+18Ch and this process reaches
        // it through no path: 0071E7F0's queue arm is a record here
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

const char* GameCommandsHost::command_name_of(std::uint32_t command_object) const {
    const bsp::EntityOrderCommandClass* klass = impl_->class_of(command_object);
    return (klass != nullptr) ? klass->name : "";
}

bool GameCommandsHost::cruise_step(std::size_t unit_index, bool player_controlled,
    float body_axis_speed, float reference_speed, bsp::CruiseOrderedValues& out,
    bsp::ShipAiControlBlock* blk, bsp::ShipAiSetterHost* setters) {
    Impl& host = *impl_;
    if (!holds_cruise(unit_index)) return false;
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
        nullptr, 0.0f, bsp::SceneCommandTarget{}, 0u, 0u, blk, setters};
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
    host.log.notef("summary mission commands units=%zu resolved=%zu issued=%zu pushed=%zu "
        "current=%zu latched=%zu with_thrust=%zu ai_groups=%zu ai_forwards=%zu steps=%llu",
        host.summary.units, host.summary.resolved, host.summary.issued, host.summary.pushed,
        host.summary.current, host.summary.latched, host.summary.moving,
        host.summary.ai_groups, host.summary.ai_forwards, host.summary.steps);
    host.log.notef("summary mission director steps=%llu idle_reissues=%llu stop=%zu "
        "cruise=%zu follow=%zu script_issues=%zu blocked_at_00816f7c=%zu commanded_speeds=%zu",
        host.summary.director_steps, host.summary.idle_reissues, host.summary.idle_stop,
        host.summary.idle_cruise, host.summary.idle_follow, host.summary.script_issues,
        host.summary.script_blocked, host.summary.commanded_speeds);
}

}  // namespace bsp::game
