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
};

struct GameCommandsHost::Impl {
    explicit Impl(GameHostLog& log_in) : log(log_in) {}

    GameHostLog& log;
    std::vector<GameCommandUnit> units;
    std::vector<GameDirector> directors;
    std::vector<GameCommandRow> rows;
    bsp::SceneCommandRegistry registry;
    GameCommandsSummary summary{};
    bool logged_path{false};
    bool logged_block{false};
    bool logged_step{false};

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
        // *(unit+73ch) +24h and +28h. 00822b70 copies the tuning singleton's
        // +160h..+178h into the block's +0h..+18h and writes neither of these
        // two, 009e11c6 stores the -1.0f that disables the override, and no
        // direct-displacement store to +24h exists in the image
        // (docs/CRUISE_COMMAND.md, follow-up `cruise_speed_setting`). So the
        // block a created unit carries has the override off.
        chain_.owner.record("CruiseState::commanded_speed_setting", 0x00822b70u);
        return bsp::CruiseSpeedSetting{};
    }
    void set_desired_steering(float rudder) override {
        chain_.owner.record("CruiseState::set_desired_steering", 0x009dffb0u);
        desired_rudder = rudder;
    }
    void set_desired_heading(float heading_radians) override {
        chain_.owner.record("CruiseState::set_desired_heading", 0x009e0040u);
        desired_heading = heading_radians;
    }
    void set_desired_throttle(float throttle) override {
        chain_.owner.record("CruiseState::set_desired_throttle", 0x009dbf90u);
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
    if (klass != nullptr && !command_takes_movement_fall_through(klass->object_address)) {
        // 00816f7c..00817330 holds the `follow`, `land`, `disband`, `settarget`,
        // `cleartarget`, `clearorders`, `moveto`, `attackmove` and `artillery`
        // arms. docs/CRUISE_COMMAND.md reads them in pseudocode and projects
        // none of them, so a command that selects one stops here.
        chain_.owner.record("EntityCommand::non_movement_arm", 0x00816f7cu);
        if (chain_.row != nullptr) {
            chain_.row->blocked = "00816e30's own arm for this command "
                "(00816f7c..00817330) is not projected";
        }
        return;
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
    host.summary.units = host.units.size();
    host.build_registry();
}

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

bool GameCommandsHost::holds_cruise(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.directors.size()) return false;
    const GameDirector& director = host.directors[unit_index];
    return director.latched
        && bsp::director_current_command_0071be40(director.mode, director.slot_command[0], 0u)
            == bsp::kCruiseCommandObjectAddress;
}

bool GameCommandsHost::cruise_step(std::size_t unit_index, bool player_controlled,
    float body_axis_speed, float reference_speed, bsp::CruiseOrderedValues& out) {
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
        host.log.notef("cruise state step 009e1170 runs once per unit per fixed simulation "
            "step: its own scheduler is the ship AI state class family at 00d21598, whose "
            "009f3dd0 keeps the state in sync with 0071be40's answer and which has no "
            "reconstruction, so where it runs is the executable's decision");
        host.log.notef("the latched pair reaches no order ring: 009dbf90, 009dffb0 and "
            "009e0040 write the AI controller block at [state]+8, and the hop from that "
            "block to unit+0fc4h / unit+0fdch under the unit+61h gate that 00825f20 reads "
            "at 008266c1 has no recovered writer (docs/CRUISE_COMMAND.md, follow-up "
            "`unit_autopilot_pair`), so nothing this rule decides reaches the motion path");
    }
    host.record("ShipAiState::sync_with_current_command", 0x009f3dd0u);
    ChainState chain{host, host.units[unit_index], host.directors[unit_index], nullptr,
        nullptr, 0.0f, bsp::SceneCommandTarget{}, 0u, 0u};
    DirectorBinding binding(chain);
    binding.body_speed = body_axis_speed;
    binding.reference = reference_speed;
    out = bsp::cruise_state_step_009e1170(binding);
    host.done("ShipAiState::cruise_step", 0x009e1170u);
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
    host.log.notef("  %-20s %-11s %4s %4s %5s %5s %5s %9s %9s %9s", "unit", "command",
        "ord", "cat", "issue", "slot", "curr", "latch", "steer", "thrust");
    for (const GameCommandRow& row : host.rows) {
        host.log.notef("  %-20s %-11s %4d %4d %5d %5d %5d %9s %9.3f %9.3f",
            row.unit.c_str(), row.command.empty() ? row.token.c_str() : row.command.c_str(),
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
}

}  // namespace bsp::game
