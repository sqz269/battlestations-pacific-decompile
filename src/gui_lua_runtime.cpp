#include "bsp/gui_lua_runtime.hpp"
#include <limits>
#include <cmath>
#include <cstring>
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
    enum class RefKind { tracked, globals, borrowed_index };
    struct Reference { int value; RefKind kind; };
    std::unordered_map<std::uint32_t, Reference> refs;
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
        refs.emplace(id, Reference{LUA_GLOBALSINDEX, RefKind::globals});
        return {id};
    }
    GuiLuaRef borrow_index(int index) {
        if (serial == std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Lua host handle space exhausted");
        const auto id = ++serial;
        refs.emplace(id, Reference{index, RefKind::borrowed_index});
        return {id};
    }
    GuiLuaRef capture() {
        if (serial == std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Lua host handle space exhausted");
        const auto id = ++serial;
        const int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        try { refs.emplace(id, Reference{ref, RefKind::tracked}); }
        catch (...) { luaL_unref(state, LUA_REGISTRYINDEX, ref); throw; }
        return {id};
    }
    Reference reference(GuiLuaRef object) const {
        const auto found = refs.find(object.id);
        if (found == refs.end())
            throw std::invalid_argument("Lua operation requires a bound object");
        return found->second;
    }
    void push(const GuiLuaRef& object) {
        const auto found = refs.find(object.id);
        if (found == refs.end())
            throw std::invalid_argument("Lua operation requires a bound object");
        if (found->second.kind != RefKind::tracked) lua_pushvalue(state, found->second.value);
        else if (found->second.value == LUA_REFNIL) lua_pushnil(state);
        else lua_rawgeti(state, LUA_REGISTRYINDEX, found->second.value);
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
GuiLuaRef GuiLua51Host::copy_ref_00b66fa0(GuiLuaRef object) {
    const auto found = impl_->refs.find(object.id);
    if (found == impl_->refs.end()) return {};
    if (found->second.kind == Impl::RefKind::globals)
        return impl_->globals();
    if (found->second.kind == Impl::RefKind::borrowed_index)
        return impl_->borrow_index(found->second.value);
    StackTop restore(impl_->state);
    impl_->push(object);
    return impl_->capture();
}
GuiLuaRef GuiLua51Host::get_by_name(const GuiLuaRef& table, const char* key) {
    auto* state = impl_->state;
    const auto source = impl_->reference(table);
    StackTop restore(state);
    const bool tracked = source.kind == Impl::RefKind::tracked;
    if (tracked) impl_->push(table);
    lua_pushlstring(state, key, std::strlen(key));
    lua_gettable(state, tracked ? -2 : source.value);
    return impl_->capture();
}
GuiLuaRef GuiLua51Host::get_by_index(const GuiLuaRef& table, std::int32_t key) {
    const auto found = impl_->refs.find(table.id);
    if (found != impl_->refs.end() && found->second.kind == Impl::RefKind::globals) {
        //00B677B7..D4: non-kind2 wrapper returns a borrowed kind2 stack-index
        // alias. In particular globals+2 denotes LUA_REGISTRYINDEX. No lookup.
        const auto bits = static_cast<std::uint32_t>(found->second.value)
            + static_cast<std::uint32_t>(key);
        std::int32_t index;
        std::memcpy(&index, &bits, sizeof index);
        return impl_->borrow_index(index);
    }
    auto* state = impl_->state;
    const auto source = impl_->reference(table);
    StackTop restore(state);
    const bool tracked = source.kind == Impl::RefKind::tracked;
    if (tracked) impl_->push(table);
    lua_pushnumber(state, static_cast<lua_Number>(key));
    lua_gettable(state, tracked ? -2 : source.value);
    return impl_->capture();
}
bool GuiLua51Host::next(const GuiLuaRef& table, GuiLuaRef& key,
    GuiLuaRef& value, bool restart) {
    auto* state = impl_->state;
    const auto source = impl_->reference(table);
    StackTop restore(state);
    if (restart) impl_->end_cursor(table.id);
    const bool tracked = source.kind == Impl::RefKind::tracked;
    if (tracked) impl_->push(table);
    const auto cursor = impl_->cursors.find(table.id);
    if (cursor == impl_->cursors.end()) lua_pushnil(state);
    else lua_rawgeti(state, LUA_REGISTRYINDEX, cursor->second);
    // Resolve a borrowed relative index AFTER the key push, as native does.
    //00A683A0 dereferences an unchecked table union; don't report a false end.
    const int table_index = tracked ? -2 : source.value;
    if (lua_type(state, table_index) != LUA_TTABLE)
        throw std::invalid_argument("Lua iteration requires a table");
    if (!lua_next(state, table_index)) {
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
    if (impl_->refs.find(object.id) == impl_->refs.end()) return GuiLuaType::None;
    const auto source = impl_->reference(object);
    if (source.kind != Impl::RefKind::tracked)
        return static_cast<GuiLuaType>(lua_type(impl_->state, source.value));
    StackTop restore(impl_->state);
    impl_->push(object);
    return static_cast<GuiLuaType>(lua_type(impl_->state, -1));
}
bool GuiLua51Host::to_boolean(const GuiLuaRef& object) {
    const auto source = impl_->reference(object);
    if (source.kind != Impl::RefKind::tracked)
        return lua_toboolean(impl_->state, source.value) != 0;
    StackTop restore(impl_->state);
    impl_->push(object);
    return lua_toboolean(impl_->state, -1) != 0;
}
double GuiLua51Host::to_number(const GuiLuaRef& object) {
    const auto source = impl_->reference(object);
    if (source.kind != Impl::RefKind::tracked)
        return lua_tonumber(impl_->state, source.value);
    StackTop restore(impl_->state);
    impl_->push(object);
    return lua_tonumber(impl_->state, -1);
}
const char* GuiLua51Host::to_string(const GuiLuaRef& object) {
    const auto source = impl_->reference(object);
    if (source.kind != Impl::RefKind::tracked)
        return lua_tostring(impl_->state, source.value);
    StackTop restore(impl_->state);
    impl_->push(object);
    const char* result = lua_tostring(impl_->state, -1);
    // lua_tolstring may replace a number with a string. Anchor the replacement
    // in the object's registry entry so the returned pointer survives the pop.
    const auto found = impl_->refs.find(object.id);
    if (result && found != impl_->refs.end()
        && found->second.kind == Impl::RefKind::tracked && found->second.value >= 0) {
        lua_pushvalue(impl_->state, -1);
        lua_rawseti(impl_->state, LUA_REGISTRYINDEX, found->second.value);
    }
    return result;
}
void* GuiLua51Host::to_userdata(const GuiLuaRef& object) {
    const auto source = impl_->reference(object);
    if (source.kind != Impl::RefKind::tracked)
        return lua_touserdata(impl_->state, source.value);
    StackTop restore(impl_->state);
    impl_->push(object);
    return lua_touserdata(impl_->state, -1);
}
void GuiLua51Host::release(const GuiLuaRef& object) {
    impl_->end_cursor(object.id);
    const auto found = impl_->refs.find(object.id);
    if (found == impl_->refs.end()) return;
    // Native00b66de6 returns immediately for an untracked global. In particular
    // do not touch an already closed borrowed Lua state when retiring that root.
    if (found->second.kind == Impl::RefKind::tracked && found->second.value >= 0)
        luaL_unref(impl_->state, LUA_REGISTRYINDEX, found->second.value);
    impl_->refs.erase(found);
}
} // namespace bsp
