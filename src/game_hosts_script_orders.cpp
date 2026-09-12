// bsp_game.exe milestone 2m: the mission script's navigator, formation, skill,
// repair and role bindings, run through their recovered bodies.
//
// See include/bsp/game_hosts_script_orders.hpp for the address list and for what
// this file supplies rather than recovers.

#include "bsp/game_hosts_script_orders.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/mission_lua_bindings.hpp"

extern "C" {
#include "lua.h"
}

#include <cstdio>
#include <cstring>

namespace bsp::game {
namespace {

// The eight rows src/lua_binding_navigator.cpp reconstructs, with the binding
// table row address 006b8610 registers each one under. The three names that
// share 008a2f20's body are listed with their own rows because the table does.
struct ScriptOrderBinding {
    const char* name;
    std::uint32_t row_address;
};

constexpr ScriptOrderBinding kScriptOrderBindings[] = {
    {"NavigatorAttackMove", 0x008a30d0u},
    {"NavigatorMoveToRange", 0x008a2f20u},
    {"NavigatorMoveToPos", 0x008a2bc0u},
    {"NavigatorDirectMoveToRange", 0x008a2d70u},
    {"JoinFormation", 0x00899d10u},
    {"SetSkillLevel", 0x00895250u},
    {"RepairEnable", 0x008ad330u},
    {"SetRoleAvailable", 0x008ab850u},
};

const ScriptOrderBinding* find_binding(const char* name) noexcept {
    if (name == nullptr) return nullptr;
    for (const ScriptOrderBinding& row : kScriptOrderBindings) {
        if (std::strcmp(row.name, name) == 0) return &row;
    }
    return nullptr;
}

// bsp::LuaCommandTargetSource and bsp::LuaBindingNavigatorHost index arguments
// the way the native's call frame does: index 0 is Lua stack slot 1.
int stack_slot(int index) noexcept { return index + 1; }

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

// The two command objects the four navigator bindings name. Both constants come
// from include/bsp/unit_commanded_speed.hpp and lua_binding_navigator.hpp.
const char* command_name_of(std::uint32_t object) noexcept {
    if (object == bsp::kCommandObjectAttackMove) return "attackmove";
    if (object == bsp::kCommandObjectMoveTo) return "moveto";
    return "";
}

}  // namespace

GameScriptOrdersHost::GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units)
    : log_(log), units_(units) {}

bool GameScriptOrdersHost::handles(const char* binding_name) noexcept {
    return find_binding(binding_name) != nullptr;
}

// ---------------------------------------------------------------------------
// The argument reads
// ---------------------------------------------------------------------------

void* GameScriptOrdersHost::entity_from_argument(int index) {
    // 00888aa0 BSP_ObjectHandle_FromLuaTable. The native validates the table and
    // converts its `Ptr` to the entity object; this process has no such object,
    // so the resolve is the `ID` field 00928a00 seeds turned into the created
    // instance of that index. Milestone 2l established that identity.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return nullptr;
    if (lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "ID");
    const bool number = lua_type(state_, -1) == LUA_TNUMBER;
    const int id = number ? static_cast<int>(lua_tonumber(state_, -1)) : 0;
    lua_settop(state_, argument_count_);
    if (!number || id <= 0) return nullptr;
    const std::size_t unit = static_cast<std::size_t>(id - 1);
    if (unit >= units_.count()) return nullptr;
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(id));
}

std::size_t GameScriptOrdersHost::index_of(void* entity) const noexcept {
    const std::uintptr_t id = reinterpret_cast<std::uintptr_t>(entity);
    if (id == 0) return static_cast<std::size_t>(-1);
    return static_cast<std::size_t>(id - 1);
}

std::string GameScriptOrdersHost::name_of(void* entity) const {
    const std::size_t index = index_of(entity);
    const GameUnitRow* row = (index < units_.count()) ? units_.unit_row(index) : nullptr;
    return (row != nullptr) ? row->name : std::string();
}

bool GameScriptOrdersHost::argument_id_field_is_nil(int index) {
    // 00b67910 fetching "ID" at 0088a83b, then 00b65fb0 at 0088a84a.
    if (state_ == nullptr) return true;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return true;
    lua_getfield(state_, slot, "ID");
    const bool nil = lua_type(state_, -1) == LUA_TNIL;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaCommandTarget::argument_id_field", "0088a83b");
    return nil;
}

void* GameScriptOrdersHost::argument_entity(int index) {
    void* entity = entity_from_argument(index);
    log_.implemented("LuaBinding::argument_entity", "00888aa0");
    return entity;
}

bool GameScriptOrdersHost::argument_vector3(int index, float out[3]) {
    // 00888760 at 0088a8a1 reads the value as a Vector3. The shipped scripts
    // pass an entity table to every one of these rows, so this arm is reached
    // only for a target that carries no `ID`.
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return false;
    static const char* const kKeys[3] = {"x", "y", "z"};
    bool complete = true;
    for (int lane = 0; lane < 3; ++lane) {
        lua_getfield(state_, slot, kKeys[lane]);
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            out[lane] = static_cast<float>(lua_tonumber(state_, -1));
        } else {
            complete = false;
        }
        lua_settop(state_, argument_count_);
    }
    log_.implemented("LuaCommandTarget::argument_vector3", "00888760");
    return complete;
}

std::uint16_t GameScriptOrdersHost::entity_object_id(void* entity) {
    // entity+174h, the handle-table ordinal. The two tables at 00f89a0c and
    // 00f89a60 are not built here, so the executable's own entity number stands
    // in for it, which is the same substitution GameCommandUnit already makes.
    log_.unimplemented("LuaBinding::entity_object_id", "0088a88e");
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return 0;
    return static_cast<std::uint16_t>(index + 1);
}

int GameScriptOrdersHost::argument_integer(int index) {
    if (state_ == nullptr) return 0;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return 0;
    log_.implemented("LuaBinding::argument_integer", "00b66290");
    return static_cast<int>(lua_tonumber(state_, slot));
}

bool GameScriptOrdersHost::argument_boolean(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return false;
    log_.implemented("LuaBinding::argument_boolean", "00b66250");
    return lua_toboolean(state_, slot) != 0;
}

void* GameScriptOrdersHost::argument_ptr_field(int index) {
    // 00888d20 BSP_LuaTable_GetPtrField: the table's `Ptr` without 00888aa0's
    // entity validation. This process seeds `Ptr` with the entity's own id, so
    // the same identity comes back.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "Ptr");
    void* value = (lua_type(state_, -1) == LUA_TLIGHTUSERDATA)
        ? lua_touserdata(state_, -1) : nullptr;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaBinding::argument_ptr_field", "00888d20");
    return value;
}

// ---------------------------------------------------------------------------
// The effects
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::entity_issue_command(void* entity,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags) {
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return;
    if (!logged_path_) {
        logged_path_ = true;
        log_.notef("the mission script's navigator bindings now issue through the same "
            "path an authored scene command takes: 008a30d0 / 008a2f20 hand 0077d600 a "
            "fixed command object and the descriptor 0088a810 built, and 0077d600 builds "
            "MT_COMMAND and routes it. What it reaches is 00816e30, whose own arms "
            "00816f7c..00817330 hold `moveto` and `attackmove` and are projected nowhere "
            "(docs/CRUISE_COMMAND.md), so the command stops one hop short of a weapon "
            "director slot. That block is packet `entity_command_arms`");
    }
    const char* name = command_name_of(command_object);
    std::string target_name;
    if (target.object != nullptr) {
        target_name = name_of(target.object);
    } else if (target.position_valid != 0) {
        target_name = "(position)";
    }
    std::string source("script:");
    source += (row_ != nullptr) ? row_->binding : std::string("navigator");
    const GameCommandRow* issued = units_.issue_script_command(index, command_object,
        target, flags, source, target_name);
    if (row_ != nullptr) {
        row_->command = name;
        row_->target = target_name;
        row_->issued = issued != nullptr && issued->issued;
        row_->reached_director = issued != nullptr && issued->projected_arm;
        if (issued != nullptr) row_->blocked = issued->blocked;
    }
    if (issued != nullptr && issued->issued) ++summary_.issued;
    if (issued != nullptr && issued->projected_arm) ++summary_.reached_director;
    if (command_object == bsp::kCommandObjectAttackMove) ++summary_.attack_moves;
    if (command_object == bsp::kCommandObjectMoveTo) ++summary_.move_tos;
    bool seen = false;
    for (std::size_t existing : ordered_units_) {
        if (existing == index) { seen = true; break; }
    }
    if (!seen) {
        ordered_units_.push_back(index);
        ++summary_.units_ordered;
    }
}

bool GameScriptOrdersHost::entity_command_is_available(void* entity,
    const char* command_name, void* target) {
    static_cast<void>(entity);
    static_cast<void>(command_name);
    static_cast<void>(target);
    if (!logged_predicate_) {
        logged_predicate_ = true;
        log_.notef("JoinFormation stops at its first question. 0077c8d0 asks the follower's "
            "vtable 16Ch whether it may `follow` the leader, which for MDestroyer is "
            "008162b0; that body was not read by the packet that reconstructed the binding "
            "(docs/LUA_BINDING_NAVIGATOR.md, follow-up 3), so the host answers the neutral "
            "false and 0077c902 ends the routine with no effect. Answering true would "
            "invent the predicate");
    }
    log_.unimplemented("Formation::command_is_available", "0077c8fe");
    ++summary_.formations_refused;
    return false;
}

int GameScriptOrdersHost::entity_route_slot(void* entity) {
    static_cast<void>(entity);
    // entity+1ACh, the index 00905300 counts against. No field of a created
    // instance carries it, so the record answers outside the 0..7 bound 0077c90a
    // tests, which is the value that skips the counter.
    log_.unimplemented("Formation::entity_route_slot", "0077c904");
    return -1;
}

void GameScriptOrdersHost::slot_counter_increment(int slot) {
    static_cast<void>(slot);
    log_.unimplemented("Formation::slot_counter_increment", "00905300");
}

void GameScriptOrdersHost::session_route_formation_message(void* follower,
    std::uint16_t leader_object_id) {
    static_cast<void>(follower);
    static_cast<void>(leader_object_id);
    log_.unimplemented("Formation::route_join_message", "0077c964");
}

void GameScriptOrdersHost::entity_set_skill_level(void* entity, int level) {
    static_cast<void>(entity);
    // entity->vtable[128h] at 0089539a. The leaf body belongs to the unit class
    // family and has no reconstruction, so the value is recorded, not applied.
    log_.unimplemented("UnitInstance::set_skill_level", "0089539a");
    if (row_ != nullptr) row_->skill_level = level;
    ++summary_.skills;
}

bool GameScriptOrdersHost::entity_is_kind_of(void* entity, int class_id) {
    // entity->vtable[5Ch] at 008ad44f, the IsKindOf the corpus uses everywhere.
    // This process runs it over the recovered class chain of the created
    // instance, the same one 0068aca0's 20h arm and the minimap walk use.
    const std::size_t index = index_of(entity);
    log_.implemented("UnitInstance::is_kind_of", "008ad44f");
    if (index >= units_.count()) return false;
    return units_.unit_is_kind_of(index, class_id);
}

void GameScriptOrdersHost::entity_set_repair_enabled_field(void* entity, bool enabled) {
    static_cast<void>(entity);
    // The byte at entity+378h, 008ad4e8. No recovered field of a created
    // instance covers it, so the write is recorded.
    log_.unimplemented("UnitInstance::set_repair_enabled_field", "008ad4e8");
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
}

void GameScriptOrdersHost::session_route_repair_enable_message(void* entity, bool enabled) {
    static_cast<void>(entity);
    log_.unimplemented("Session::route_repair_enable_message", "008ad4cd");
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
}

int GameScriptOrdersHost::game_session_mode() {
    // [00e188a8]+1fe4h, the field every other host in this executable reports as
    // the single-player value.
    log_.implemented("Game::session_mode", "008ab9d1");
    return 0;
}

int GameScriptOrdersHost::game_effective_game_mode() {
    // 004bca50 at 008ab9de. game+614h and game+61Ch resolve a single-player
    // campaign load to 8, which is the value the scene contents pass already
    // runs its gate on.
    log_.implemented("Game::effective_game_mode", "004bca50");
    return 8;
}

void GameScriptOrdersHost::role_owner_set_role_available(void* owner, int role, int value) {
    static_cast<void>(owner);
    log_.unimplemented("RoleOwner::set_role_available", "008aba51");
    if (row_ != nullptr) {
        row_->role = role;
        row_->role_value = value;
    }
    ++summary_.roles;
}

void GameScriptOrdersHost::session_route_role_message(void* owner, int role, int value) {
    static_cast<void>(owner);
    static_cast<void>(role);
    static_cast<void>(value);
    log_.unimplemented("Session::route_role_message", "008aba2f");
}

void GameScriptOrdersHost::game_assign_party_player_slots(int value) {
    static_cast<void>(value);
    log_.unimplemented("Game::assign_party_player_slots", "008aba60");
}

// ---------------------------------------------------------------------------
// The dispatch
// ---------------------------------------------------------------------------

int GameScriptOrdersHost::dispatch(lua_State* state, const char* binding_name,
    int argument_count) {
    const ScriptOrderBinding* binding = find_binding(binding_name);
    if (binding == nullptr) return 0;
    state_ = state;
    argument_count_ = argument_count;

    GameScriptOrderRow row;
    row.binding = binding->name;
    row.row_address = binding->row_address;
    row_ = &row;

    char address[16];
    format_address(binding->row_address, address);
    char method[96];
    std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding->name);
    log_.implemented(method, address);

    void* subject = entity_from_argument(0);
    if (subject == nullptr && std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        subject = argument_ptr_field(0);
    }
    row.unit_index = index_of(subject);
    row.unit = name_of(subject);

    int results = 0;
    if (std::strcmp(binding->name, "NavigatorAttackMove") == 0) {
        results = bsp::lua_binding_navigator_attack_move(*this, *this);
    } else if (std::strcmp(binding->name, "JoinFormation") == 0) {
        void* leader = entity_from_argument(1);
        row.formation_leader = name_of(leader);
        ++summary_.formations_requested;
        results = bsp::lua_binding_join_formation(*this);
    } else if (std::strcmp(binding->name, "SetSkillLevel") == 0) {
        results = bsp::lua_binding_set_skill_level(*this);
    } else if (std::strcmp(binding->name, "RepairEnable") == 0) {
        bsp::RepairEnableArm arm = bsp::RepairEnableArm::kLocalFieldWrite;
        results = bsp::lua_binding_repair_enable(*this, arm);
    } else if (std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        bsp::SetRoleAvailableArm arm = bsp::SetRoleAvailableArm::kDirectCall;
        results = bsp::lua_binding_set_role_available(*this, arm);
    } else {
        // 008a2f20, 008a2bc0 and 008a2d70 are one body with one command object.
        results = bsp::lua_binding_navigator_move_to(*this, *this);
    }

    ++summary_.calls;
    row_ = nullptr;
    rows_.push_back(row);
    state_ = nullptr;
    argument_count_ = 0;
    return results;
}

void GameScriptOrdersHost::report() {
    if (rows_.empty()) return;
    log_.notef("the mission script's own orders, run through the eight reconstructed "
        "binding bodies (docs/LUA_BINDING_NAVIGATOR.md):");
    log_.notef("  %-26s %-16s %-10s %-16s %6s %6s", "binding", "unit", "command",
        "target", "issued", "slot");
    for (const GameScriptOrderRow& row : rows_) {
        if (row.command.empty()) continue;
        log_.notef("  %-26s %-16s %-10s %-16s %6d %6d", row.binding.c_str(),
            row.unit.c_str(), row.command.c_str(), row.target.c_str(),
            row.issued ? 1 : 0, row.reached_director ? 1 : 0);
    }
    log_.notef("summary mission script bindings calls=%zu attackmove=%zu moveto=%zu "
        "issued=%zu reached_director=%zu units=%zu formations=%zu/%zu skills=%zu "
        "repairs=%zu roles=%zu", summary_.calls, summary_.attack_moves,
        summary_.move_tos, summary_.issued, summary_.reached_director,
        summary_.units_ordered, summary_.formations_requested - summary_.formations_refused,
        summary_.formations_requested, summary_.skills, summary_.repairs, summary_.roles);
}

}  // namespace bsp::game
