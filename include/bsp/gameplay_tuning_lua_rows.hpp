#pragma once

#include "bsp/gameplay_settings.hpp"
#include "bsp/native_lua_objects.hpp"

#include <string>
#include <type_traits>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
namespace gameplay_tuning_lua_detail {

// Adapter for the existing keyed projection, not a second 0083B5E0 body.
// The interpreter and every value belong to the actual ShipGlobals load.
// This adapter lives OUTSIDE the protected Lua C frame: an indexing error may
// longjmp over that frame, but never over this virtual object's lifetime.
class Rows final : public GameplayTuningRowView {
public:
    explicit Rows(bool sse2) : sse2_(sse2) {}

    void start(lua_State* state) {
        state_ = state;
        construct_native_lua_state_00b66bd0(&owner_);
        owner_.state_04 = state;
        lua_getfield(state, LUA_GLOBALSINDEX, "ShipGlobals");
        root_ = lua_gettop(state);
        current_ = root_;
        if (!lua_istable(state, root_)) luaL_error(state, "ShipGlobals is not a table");
    }
    void enter() override {
        lua_settop(state_, root_);
        current_ = root_;
    }
    void enter(const char* a) override {
        enter();
        lua_getfield(state_, current_, a);
        current_ = lua_gettop(state_);
    }
    void enter(const char* a, const char* b) override {
        enter(a);
        lua_getfield(state_, current_, b);
        current_ = lua_gettop(state_);
    }

    float number(const char* key) override { return number(key, 0); }
    float number(const char* key, int index) override {
        return read(key, index, [](const NativeLuaObjectStorage& value) {
            return native_lua_number_00b66270(value);
        });
    }
    float number_or(const char* key, float fallback) override {
        return number_or(key, 0, fallback);
    }
    float number_or(const char* key, int index, float fallback) override {
        return read(key, index, [fallback](const NativeLuaObjectStorage& value) {
            return native_lua_number_or_00b66330(value, fallback);
        });
    }
    std::int32_t integer(const char* key) override { return integer(key, 0); }
    std::int32_t integer(const char* key, int index) override {
        return read(key, index, [this](const NativeLuaObjectStorage& value) {
            return native_lua_integer_00b66290(value, sse2_);
        });
    }
    std::int32_t integer_or(const char* key, std::int32_t fallback) override {
        return integer_or(key, 0, fallback);
    }
    std::int32_t integer_or(const char* key, int index, std::int32_t fallback) override {
        return read(key, index, [this, fallback](const NativeLuaObjectStorage& value) {
            return native_lua_integer_or_00b66380(value, fallback, sse2_);
        });
    }
    bool boolean(const char* key) override { return boolean(key, 0); }
    bool boolean(const char* key, int index) override {
        return read(key, index, [](const NativeLuaObjectStorage& value) {
            return native_lua_boolean_00b66250(value);
        });
    }
    bool boolean_or(const char* key, bool fallback) override {
        return boolean_or(key, 0, fallback);
    }
    bool boolean_or(const char* key, int index, bool fallback) override {
        return read(key, index, [fallback](const NativeLuaObjectStorage& value) {
            return native_lua_boolean_or_00b662f0(value, fallback) != 0;
        });
    }

private:
    template<class Read>
    std::invoke_result_t<Read, const NativeLuaObjectStorage&>
    read(const char* key, int index, Read get) {
        // The section is an untracked borrowed positive stack index. Only the
        // fetched field/element owns a native tracked slot, released before the
        // next getter. Nothing here converts a Lua value through a C++ snapshot.
        NativeLuaObjectStorage section{&owner_, 2, current_, 0, 0, {}};
        NativeLuaObjectStorage field, element;
        native_lua_get_by_name_00b67800(section, &field, key);
        NativeLuaObjectStorage* value = &field;
        if (index != 0) {
            native_lua_get_by_index_00b67720(field, &element, index);
            value = &element;
        }
        const auto result = get(*value);
        if (index != 0) destroy_native_lua_object_00b67700(element);
        destroy_native_lua_object_00b67700(field);
        return result;
    }

    bool sse2_;
    lua_State* state_{};
    int root_{};
    int current_{};
    NativeLuaStateStorage owner_;
};

struct ReadContext {
    Rows& rows;
    GameplayTuningSettings& output;
};

inline int read_protected(lua_State* state) {
    auto& context = *static_cast<ReadContext*>(lua_touserdata(state, lua_upvalueindex(1)));
    context.rows.start(state);
    // Existing keyed 0083B5E0 body; its unkeyed subobjects are not represented.
    load_gameplay_tuning_settings(context.rows, context.output);
    return 0;
}

} // namespace gameplay_tuning_lua_detail

// Balanced, protected C++ boundary, following the existing ship-settings reader.
// The output is an already initialized settings owner (00424A10's projection).
// Failed Lua indexing publishes no partial settings. No Lua owner is closed.
inline bool read_gameplay_tuning_lua(lua_State& state, GameplayTuningSettings& output,
    bool crt_sse2_conversion, std::string& error) {
    GameplayTuningSettings scratch = output;
    gameplay_tuning_lua_detail::Rows rows(crt_sse2_conversion);
    gameplay_tuning_lua_detail::ReadContext context{rows, scratch};
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &context);
    lua_pushcclosure(&state, &gameplay_tuning_lua_detail::read_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message ? message : "ShipGlobals indexing raised a non-string Lua error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    output = scratch;
    error.clear();
    return true;
}

} // namespace bsp
