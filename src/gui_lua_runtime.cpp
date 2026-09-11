#include "bsp/gui_lua_runtime.hpp"
#include <limits>
#include <stdexcept>
#include <unordered_map>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
struct GuiLua51Host::Impl {
    lua_State* state{luaL_newstate()};
    std::uint32_t serial{};
    std::unordered_map<std::uint32_t, int> refs;
    // The recovered reader releases each returned key before requesting next.
    // A separate registry reference preserves each table's iteration cursor.
    std::unordered_map<std::uint32_t, int> cursors;
    Impl() { if (!state) throw std::bad_alloc(); }
    ~Impl() { lua_close(state); }
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
}
GuiLua51Host::GuiLua51Host() : impl_(std::make_unique<Impl>()) {}
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
GuiLuaRef GuiLua51Host::globals() {
    lua_pushvalue(impl_->state, LUA_GLOBALSINDEX);
    return impl_->capture();
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
    luaL_unref(impl_->state, LUA_REGISTRYINDEX, found->second);
    impl_->refs.erase(found);
}
} // namespace bsp
