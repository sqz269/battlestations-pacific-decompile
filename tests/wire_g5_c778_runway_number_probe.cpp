#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/pool_pool13_cc13.hpp"
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("c778 runway number conversion failed");
}

// Fixture only: actual Lua 5.1.1 values, with no renderer or game startup.
// The independent numeric oracle uses Lua's tonumber and the native float32
// tail. inspect deliberately still exposes booleans/strings, so reverting the
// runway body to the permissive semantic converter fails this regression.
class Fields final : public bsp::VehicleClassFieldHost {
public:
    explicit Fields(bsp::GuiLua51Host& lua) : lua_(lua) {}
    bsp::GuiLuaRef get_by_name(const bsp::GuiLuaRef& row, const char* key) override {
        return lua_.get_by_name(row, key);
    }
    bsp::GuiLuaRef get_by_index(const bsp::GuiLuaRef& row, std::int32_t index) override {
        return lua_.get_by_index(row, index);
    }
    void release(const bsp::GuiLuaRef& object) override { lua_.release(object); }
    float number_00b66270(const bsp::GuiLuaRef& object) override {
        ++number_reads;
        return bsp::lua_number_float32_00b66270(lua_.to_number(object));
    }
    bsp::VehicleClassLuaValue inspect(const bsp::GuiLuaRef& object) override {
        bsp::VehicleClassLuaValue value;
        switch (lua_.type_of(object)) {
        case bsp::GuiLuaType::Boolean:
            value.kind = bsp::VehicleClassValueKind::Boolean;
            value.boolean = lua_.to_boolean(object);
            break;
        case bsp::GuiLuaType::Number:
            value.kind = bsp::VehicleClassValueKind::Number;
            value.number = lua_.to_number(object);
            break;
        case bsp::GuiLuaType::String:
            value.kind = bsp::VehicleClassValueKind::String;
            value.string = lua_.to_string(object);
            break;
        case bsp::GuiLuaType::Table:
            value.kind = bsp::VehicleClassValueKind::Table;
            break;
        default: break;
        }
        return value;
    }
    bool iterate_first(const bsp::GuiLuaRef&, bsp::GuiLuaRef&, bsp::GuiLuaRef&) override {
        throw std::logic_error("unexpected fixture iteration");
    }
    bool iterate_next(const bsp::GuiLuaRef&, bsp::GuiLuaRef&, bsp::GuiLuaRef&) override {
        throw std::logic_error("unexpected fixture iteration");
    }
    void* load_small_map_icon(const char*) override { ++base_loads; return nullptr; }
    void* load_cockpit_mesh(const char*) override { return unexpected(); }
    void* make_sound_handle(std::int32_t) override { return unexpected(); }
    void* load_ambient_sound(const char*) override { return unexpected(); }
    std::int32_t resolve_recon_class(std::int32_t) override {
        throw std::logic_error("unexpected fixture recon class");
    }
    void* load_flag_texture(const char*) override { return unexpected(); }
    void reserve_platform_slots(std::int32_t) override { unexpected(); }
    int number_reads{0}, base_loads{0};
private:
    static void* unexpected() { throw std::logic_error("unexpected fixture resource"); }
    bsp::GuiLua51Host& lua_;
};
}

int main() {
    lua_State* state = luaL_newstate();
    require(state != nullptr);
    {
        bsp::GuiLua51Host lua(*state);
        std::string error;
        require(lua.execute_archive(
            "Rows={{RunwayWidth=125.5,RunwayLength=900},"
            "{RunwayWidth=true,RunwayLength='12m'},"
            "{RunwayWidth='12m',RunwayLength=true},"
            "{RunwayWidth=' 12.5 ',RunwayLength='-7.25'}}", error));
        const auto globals = lua.globals();
        const auto rows = lua.get_by_name(globals, "Rows");
        Fields fields(lua);
        const float expected[4][2] = {{125.5f, 900.0f}, {0, 0}, {0, 0}, {12.5f, -7.25f}};
        const int top = lua_gettop(state);
        for (int i = 0; i != 4; ++i) {
            const auto row = lua.get_by_index(rows, i + 1);
            bsp::VehicleClassBaseFields base;
            bsp::pool_pool13_cc13::RunwayClassFields runway;
            bsp::pool_pool13_cc13::runway_class_read_lua_fields_006d0b80(fields, row, base, runway);
            require(runway.width_138 == expected[i][0] && runway.length_13c == expected[i][1]);
            lua.release(row);
            require(lua_gettop(state) == top);
            require(fields.base_loads == i + 1 && fields.number_reads == 2 * (i + 1));
        }
        lua.release(rows);
        lua.release(globals);
    }
    lua_close(state);
    std::cout << "PASS: runway numbers/numeric strings, boolean and invalid-string rejection, "
                 "both lanes, one base load and balanced Lua stack\n";
}
