#include "bsp/gui_lua_runtime.hpp"
#include <limits>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
struct GuiLua51Host::Impl {
    lua_State* state;
    bool owns_state;
    std::uint32_t serial{};
    std::unordered_map<std::uint32_t, int> refs;
    // The recovered reader releases each returned key before requesting next.
    // A separate registry reference preserves each table's iteration cursor.
    std::unordered_map<std::uint32_t, int> cursors;
    Impl() : state(luaL_newstate()), owns_state(true) {
        if (!state) throw std::bad_alloc();
    }
    explicit Impl(lua_State& borrowed) : state(&borrowed), owns_state(false) {}
    ~Impl() { if (owns_state) lua_close(state); }
    GuiLuaRef globals() {
        if (serial == std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Lua host handle space exhausted");
        const auto id = ++serial;
        refs.emplace(id, LUA_GLOBALSINDEX);
        return {id};
    }
    GuiLuaRef capture() {
        if (serial == std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Lua host handle space exhausted");
        const auto id = ++serial;
        const int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        try { refs.emplace(id, ref); }
        catch (...) { luaL_unref(state, LUA_REGISTRYINDEX, ref); throw; }
        return {id};
    }
    void push(const GuiLuaRef& object) {
        const auto found = refs.find(object.id);
        if (found == refs.end() || found->second == LUA_REFNIL) lua_pushnil(state);
        else if (found->second == LUA_GLOBALSINDEX) lua_pushvalue(state, LUA_GLOBALSINDEX);
        else lua_rawgeti(state, LUA_REGISTRYINDEX, found->second);
    }
    void end_cursor(std::uint32_t id) {
        const auto found = cursors.find(id);
        if (found != cursors.end()) {
            luaL_unref(state, LUA_REGISTRYINDEX, found->second);
            cursors.erase(found);
        }
    }
};
namespace {
struct StackTop {
    lua_State* state;
    int top;
    explicit StackTop(lua_State* s) : state(s), top(lua_gettop(s)) {}
    ~StackTop() { lua_settop(state, top); }
};
struct TableSnapshot {
    lua_State* state;
    std::unordered_map<const void*, std::shared_ptr<GuiTable>> tables;
    std::unordered_set<const void*> active;
    std::size_t entries{};

    GuiValue value(int index, std::size_t depth) {
        switch (lua_type(state, index)) {
        case LUA_TNIL: return {};
        case LUA_TBOOLEAN: return GuiValue(lua_toboolean(state, index) != 0);
        case LUA_TNUMBER: return GuiValue(lua_tonumber(state, index));
        case LUA_TSTRING: {
            std::size_t length{};
            const char* text = lua_tolstring(state, index, &length);
            return GuiValue(std::string(text, length));
        }
        case LUA_TTABLE: return GuiValue(table(index, depth));
        default: throw std::runtime_error("GUI table contains a non-data Lua value");
        }
    }
    std::shared_ptr<const GuiTable> table(int index, std::size_t depth) {
        StackTop restore(state);
        if (depth > 128) throw std::runtime_error("GUI table exceeds host snapshot depth");
        if (!lua_checkstack(state, 8))
            throw std::runtime_error("GUI table snapshot could not extend the Lua stack");
        const void* identity = lua_topointer(state, index);
        if (active.count(identity)) throw std::runtime_error("GUI table contains a cycle");
        const auto previous = tables.find(identity);
        if (previous != tables.end()) return previous->second;
        if (lua_getmetatable(state, index))
            throw std::runtime_error("GUI table metatables require a live widget reader");
        lua_pushvalue(state, index);
        const int source = lua_gettop(state);
        auto result = std::make_shared<GuiTable>();
        tables.emplace(identity, result);
        active.insert(identity);
        lua_pushnil(state);
        while (lua_next(state, source)) {
            if (++entries > 100000)
                throw std::runtime_error("GUI table exceeds host snapshot size");
            if (lua_type(state, -2) == LUA_TSTRING) {
                std::size_t length{};
                const char* key = lua_tolstring(state, -2, &length);
                if (std::char_traits<char>::length(key) != length)
                    throw std::runtime_error("GUI table key contains an embedded NUL");
                std::string name(key, length);
                result->named.emplace_back(std::move(name), value(-1, depth + 1));
            } else if (lua_type(state, -2) == LUA_TNUMBER) {
                const double key = lua_tonumber(state, -2);
                if (!std::isfinite(key) || key < 1 || key > 100000 || std::floor(key) != key)
                    throw std::runtime_error("GUI numeric key is outside the retained array domain");
                const auto slot = static_cast<std::size_t>(key - 1);
                if (result->array.size() <= slot) result->array.resize(slot + 1);
                result->array[slot] = value(-1, depth + 1);
            } else {
                throw std::runtime_error("GUI table contains an unsupported key type");
            }
            lua_pop(state, 1);
        }
        active.erase(identity);
        return result;
    }
};
}
GuiLua51Host::GuiLua51Host() : impl_(std::make_unique<Impl>()) {}
GuiLua51Host::GuiLua51Host(lua_State& borrowed) : impl_(std::make_unique<Impl>(borrowed)) {}
GuiLua51Host::~GuiLua51Host() = default;
bool GuiLua51Host::execute_archive(std::string_view text, std::string& error) {
    auto* state = impl_->state;
    StackTop restore(state);
    int status = luaL_loadbuffer(state, text.data(), text.size(), "profile archive");
    if (!status) status = lua_pcall(state, 0, 0, 0);
    if (status) {
        const char* message = lua_tostring(state, -1);
        error = message ? message : "Lua archive execution failed";
        return false;
    }
    error.clear();
    return true;
}
std::shared_ptr<const GuiTable> GuiLua51Host::snapshot_table(const GuiLuaRef& object) {
    StackTop restore(impl_->state);
    impl_->push(object);
    if (lua_isnil(impl_->state, -1)) return {};
    if (!lua_istable(impl_->state, -1))
        throw std::runtime_error("GuiScreen is not a Lua table");
    TableSnapshot snapshot{impl_->state, {}, {}, 0};
    return snapshot.table(-1, 0);
}
GuiLuaRef GuiLua51Host::globals() {
    return impl_->globals();
}
GuiLuaRef GuiLua51Host::get_by_name(const GuiLuaRef& table, const char* key) {
    auto* state = impl_->state;
    StackTop restore(state);
    impl_->push(table);
    if (lua_type(state, -1) != LUA_TTABLE) lua_pushnil(state);
    else {
        lua_pushstring(state, key ? key : "");
        lua_gettable(state, -2);
    }
    return impl_->capture();
}
GuiLuaRef GuiLua51Host::get_by_index(const GuiLuaRef& table, std::int32_t key) {
    auto* state = impl_->state;
    StackTop restore(state);
    impl_->push(table);
    if (lua_type(state, -1) != LUA_TTABLE) lua_pushnil(state);
    else {
        lua_pushnumber(state, static_cast<lua_Number>(key));
        lua_gettable(state, -2);
    }
    return impl_->capture();
}
bool GuiLua51Host::next(const GuiLuaRef& table, GuiLuaRef& key,
    GuiLuaRef& value, bool restart) {
    auto* state = impl_->state;
    StackTop restore(state);
    if (restart) impl_->end_cursor(table.id);
    impl_->push(table);
    if (lua_type(state, -1) != LUA_TTABLE) return false;
    const auto cursor = impl_->cursors.find(table.id);
    if (cursor == impl_->cursors.end()) lua_pushnil(state);
    else lua_rawgeti(state, LUA_REGISTRYINDEX, cursor->second);
    if (!lua_next(state, -2)) {
        impl_->end_cursor(table.id);
        key = {}; value = {};
        return false;
    }
    value = impl_->capture(); // leaves table/key
    lua_pushvalue(state, -1);
    const int next_cursor = luaL_ref(state, LUA_REGISTRYINDEX);
    impl_->end_cursor(table.id);
    impl_->cursors[table.id] = next_cursor;
    key = impl_->capture();
    return true;
}
GuiLuaType GuiLua51Host::type_of(const GuiLuaRef& object) {
    StackTop restore(impl_->state);
    impl_->push(object);
    return static_cast<GuiLuaType>(lua_type(impl_->state, -1));
}
bool GuiLua51Host::to_boolean(const GuiLuaRef& object) {
    StackTop restore(impl_->state);
    impl_->push(object);
    return lua_toboolean(impl_->state, -1) != 0;
}
double GuiLua51Host::to_number(const GuiLuaRef& object) {
    StackTop restore(impl_->state);
    impl_->push(object);
    return lua_tonumber(impl_->state, -1);
}
const char* GuiLua51Host::to_string(const GuiLuaRef& object) {
    StackTop restore(impl_->state);
    impl_->push(object);
    const char* result = lua_tostring(impl_->state, -1);
    // lua_tolstring may replace a number with a string. Anchor the replacement
    // in the object's registry entry so the returned pointer survives the pop.
    const auto found = impl_->refs.find(object.id);
    if (result && found != impl_->refs.end() && found->second >= 0) {
        lua_pushvalue(impl_->state, -1);
        lua_rawseti(impl_->state, LUA_REGISTRYINDEX, found->second);
    }
    return result;
}
void GuiLua51Host::release(const GuiLuaRef& object) {
    impl_->end_cursor(object.id);
    const auto found = impl_->refs.find(object.id);
    if (found == impl_->refs.end()) return;
    // Native00b66de6 returns immediately for an untracked global. In particular
    // do not touch an already closed borrowed Lua state when retiring that root.
    if (found->second >= 0) luaL_unref(impl_->state, LUA_REGISTRYINDEX, found->second);
    impl_->refs.erase(found);
}
} // namespace bsp
