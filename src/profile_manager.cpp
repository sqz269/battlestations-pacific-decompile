#include "bsp/profile_manager.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

constexpr const char* kSaveTable = "BSP_Chk_Save"; // 00CE4124

struct LifetimeLock {
    SingletonLifetimeManager& manager;
    explicit LifetimeLock(SingletonLifetimeManager& value) : manager(value) { manager.lock(); }
    ~LifetimeLock() { manager.unlock(); }
};

template<class T>
T* get_singleton(T* volatile& published, SingletonLifetimeManager& manager) {
    if (!published) {
        LifetimeLock lock(manager);
        if (!published) {
            published = new (std::nothrow) T;
            // The native accessor registers even a null allocation result.
            manager.register_object(published);
        }
    }
    return published;
}

struct StackTop {
    lua_State* state;
    int top;
    explicit StackTop(lua_State* value) : state(value), top(lua_gettop(value)) {}
    ~StackTop() { lua_settop(state, top); }
};

lua_State* require_state(lua_State* state) {
    if (!state) throw std::logic_error("profile refresh requires a live Lua state");
    return state;
}

std::int32_t integer_from_float(float value) {
    // Native wrappers spill float32 before CRT __ftol. Reject the exceptional
    // conversion domain instead of relying on undefined C++ float-to-int casts.
    const double wide = value;
    if (!std::isfinite(wide) || wide < -2147483648.0 || wide >= 2147483648.0)
        throw std::range_error("profile Lua number is outside signed 32-bit conversion");
    return static_cast<std::int32_t>(value);
}

std::int32_t integer_field(lua_State* state, int table, const char* name, std::int32_t fallback) {
    lua_getfield(state, table, name);
    const auto result = lua_type(state, -1) == LUA_TNUMBER
        ? integer_from_float(static_cast<float>(lua_tonumber(state, -1))) : fallback;
    lua_pop(state, 1);
    return result;
}

std::string string_field(lua_State* state, int table, const char* name) {
    lua_getfield(state, table, name);
    // 00B685C0 accepts only actual strings, not Lua's numeric-string coercion.
    std::string result;
    if (lua_type(state, -1) == LUA_TSTRING) result = lua_tostring(state, -1);
    lua_pop(state, 1);
    return result;
}

void copy_entry(lua_State* source, int key, int value,
    lua_State* destination, int table, std::vector<const void*>& ancestors) {
    // 00436C50 ignores nil, light userdata, userdata, functions and threads.
    const int type = lua_type(source, value);
    if (type != LUA_TBOOLEAN && type != LUA_TNUMBER && type != LUA_TSTRING && type != LUA_TTABLE)
        return;
    if (lua_type(source, key) == LUA_TSTRING) {
        // NativeString_Assign uses the C-string prefix, including for names.
        lua_pushstring(destination, lua_tostring(source, key));
    } else {
        lua_pushinteger(destination,
            integer_from_float(static_cast<float>(lua_tonumber(source, key))));
    }
    switch (type) {
    case LUA_TBOOLEAN:
        lua_pushboolean(destination, lua_toboolean(source, value));
        break;
    case LUA_TNUMBER: {
        // 00436E94..00436F61 and 00437132..004371B9: float32, floor,
        // float32 equality test, then integer or float setter. For finite
        // numbers floor(x)==x and trunc(x)==x select the same integer branch.
        const float number = static_cast<float>(lua_tonumber(source, value));
        if (!std::isfinite(number))
            throw std::range_error("nonfinite profile Lua number is outside the host domain");
        if (number == std::floor(number)) lua_pushinteger(destination, integer_from_float(number));
        else lua_pushnumber(destination, number);
        break;
    }
    case LUA_TSTRING:
        lua_pushstring(destination, lua_tostring(source, value));
        break;
    case LUA_TTABLE: {
        const void* identity = lua_topointer(source, value);
        if (std::find(ancestors.begin(), ancestors.end(), identity) != ancestors.end())
            throw std::invalid_argument("cyclic profile Lua tables have no finite native copy");
        ancestors.push_back(identity);
        lua_newtable(destination);
        // Native creates and assigns the child before recursively filling it.
        lua_pushvalue(destination, -2);
        lua_pushvalue(destination, -2);
        lua_settable(destination, table);
        const int child = lua_gettop(destination);
        lua_pushnil(source);
        while (lua_next(source, value)) {
            copy_entry(source, lua_gettop(source) - 1, lua_gettop(source), destination, child, ancestors);
            lua_pop(source, 1);
        }
        ancestors.pop_back();
        lua_pop(destination, 2); // retained key and already-assigned child
        return;
    }
    default:
        throw std::logic_error("unreachable profile Lua value type");
    }
    lua_settable(destination, table);
}

void import_save_table(ProfileResetState& profile, lua_State* source, lua_State* destination,
    int destination_table) {
    if (source == destination)
        throw std::invalid_argument("profile refresh requires distinct game and storage Lua owners");
    StackTop source_stack(source);
    StackTop destination_stack(destination);
    lua_getglobal(source, kSaveTable);
    if (!lua_istable(source, -1)) return;
    const int table = lua_gettop(source);
    std::vector<const void*> ancestors{lua_topointer(source, table)};
    lua_pushnil(source);
    while (lua_next(source, table)) {
        const int value = lua_gettop(source);
        const int key = value - 1;
        copy_entry(source, key, value, destination, destination_table, ancestors);
        // Native proceeds to indexed reads for every outer entry. Its valid
        // archive contract therefore requires each value to be a table.
        if (!lua_istable(source, value))
            throw std::invalid_argument("BSP_Chk_Save entry is not a table");
        ProfileTransientRecord94 record;
        record.words = {
            static_cast<std::uint32_t>(integer_field(source, value, "__difficulty", 0)),
            static_cast<std::uint32_t>(integer_field(source, value, "__unlockFrom", -1)),
            static_cast<std::uint32_t>(integer_field(source, value, "__unlockTo", -1))};
        record.text = string_field(source, value, "__unlockName");
        // A temporary copy avoids changing the lua_next key during number-to-
        // string conversion, as the native iterator has its own key reference.
        lua_pushvalue(source, key);
        const char* name = lua_tostring(source, -1);
        if (!name) throw std::invalid_argument("profile record key has no native C-string representation");
        record.key = name;
        lua_pop(source, 1);
        set_profile_transient_record_007fc2c0(profile, std::move(record));
        lua_pop(source, 1);
    }
}

} // namespace

ProfileManagerOwner* get_profile_manager_00425c20(
    ProfileManagerOwner* volatile& published, SingletonLifetimeManager& lifetime) {
    return get_singleton(published, lifetime);
}
ProfileHintsOwner* get_profile_hints_owner_004c1e90(
    ProfileHintsOwner* volatile& published, SingletonLifetimeManager& lifetime) {
    return get_singleton(published, lifetime);
}
void profile_refresh_storage_completed_00bd53c0() noexcept {}

void set_profile_transient_record_007fc2c0(ProfileResetState& profile, ProfileTransientRecord94 record) {
    const auto found = std::find_if(profile.transient_records_94.begin(), profile.transient_records_94.end(),
        [&](const ProfileTransientRecord94& existing) {
            if (existing.key.empty() || record.key.empty()) return existing.key.empty() == record.key.empty();
            return _stricmp(existing.key.c_str(), record.key.c_str()) == 0;
        });
    if (found == profile.transient_records_94.end()) profile.transient_records_94.push_back(std::move(record));
    else {
        found->words = record.words;
        found->text = std::move(record.text);
    }
}

void refresh_profile_manager_004374f0(ProfileResetState& profile, ProfileManagerRefreshHost& host,
    StorageOperationState& storage, StorageOperationHost& storage_host) {
    lua_State* game = require_state(host.game_lua_1a0c());
    StackTop game_stack(game);
    lua_newtable(game);
    lua_pushvalue(game, -1);
    lua_setglobal(game, kSaveTable); // unconditional replacement, before query
    const int destination_table = lua_gettop(game); // retained across the driver
    if (profile.save_name_34.empty()) return; // 007F8CA0 + empty00436710
    if (!host.storage_query_1c(profile.save_name_34, 2)) return;
    if (host.has_storage_buffer_30()) host.free_and_clear_storage_buffer_30();
    host.close_storage_archive_00b65e80();
    host.request_read_00bd3d70(profile.save_name_34, 2);
    run_storage_operation_006adb50(storage, storage_host, profile_refresh_storage_completed_00bd53c0);
    // Deliberately immediate: 00BD53C0 performs no deferred import.
    import_save_table(profile, require_state(host.storage_lua_38()), game, destination_table);
}

ConcreteProfileArchiveManagerServices::ConcreteProfileArchiveManagerServices(ProfileHintsOwner& hints,
    ProfileManagerRefreshHost& host, StorageOperationState& storage, StorageOperationHost& storage_host) noexcept
    : hints_(hints), host_(host), storage_(storage), storage_host_(storage_host) {}
int ConcreteProfileArchiveManagerServices::hints_owner_field_08_004c1e90() { return hints_.field_08; }
void ConcreteProfileArchiveManagerServices::refresh_profile_manager_004374f0(ProfileResetState& profile) {
    bsp::refresh_profile_manager_004374f0(profile, host_, storage_, storage_host_);
}

} // namespace bsp
