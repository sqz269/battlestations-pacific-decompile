#include "bsp/game_recon_sensor_class_registry.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/lua_numeric.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

extern "C" {
#include <lua.h>
}

#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp::game {
namespace {

// bsp.py show 00808F90 / 008082A0: use ordinary gettable/settable accesses,
// exact Lua iteration order, and 00B66270's float32 spill before conversion.
// These are C++ service adapters, not original register-ABI entry points.
class SensorClassBinding final : public SensorClassRegistryHost, public SensorTableLuaHost {
public:
    using Records = std::map<int, std::unique_ptr<SensorClassTable>>;

    SensorClassBinding(lua_State& state, Records& records)
        : state_(&state), records_(records), entry_top_(lua_gettop(state_)),
          sse2_conversion_(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE) {}
    ~SensorClassBinding() override { lua_settop(state_, entry_top_); }

    void reserve_registry_00808fd1(int id) override {
        // A keyed, stable slot replaces the native ref-pointer array's growth.
        if (id < 0) throw std::out_of_range("negative ReconClass id");
        records_.try_emplace(id);
    }
    SensorClassTable* cached_record_00808fe4(int id) override {
        return records_.at(id).get();
    }
    void open_recon_class_global_00809041() override {
        lua_getfield(state_, LUA_GLOBALSINDEX, kSensorReconClassGlobal);
        global_ = lua_gettop(state_);
    }
    void open_class_row_0080905e(int id) override {
        // 00B67720 pushes a numeric key and uses lua_gettable, not rawgeti.
        lua_pushnumber(state_, static_cast<lua_Number>(id));
        lua_gettable(state_, global_);
        row_ = lua_gettop(state_);
    }
    void set_row_id_00809085(int id) override {
        lua_pushnumber(state_, static_cast<lua_Number>(id));
        lua_setfield(state_, row_, kSensorReconClassIdKey);
    }
    void set_row_got_008090c9(bool value) override {
        lua_pushboolean(state_, value);
        lua_setfield(state_, row_, kSensorReconClassGotKey);
    }
    SensorClassTable* allocate_record_0080910d() override {
        pending_ = std::make_unique<SensorClassTable>(construct_sensor_class_table_00808cb0());
        return pending_.get();
    }
    void load_from_row_00809147(SensorClassTable& table) override {
        load_sensor_class_table_008082a0(table, *this);
    }
    void publish_record_0080915f(int id, SensorClassTable* record) override {
        if (record != pending_.get()) throw std::logic_error("sensor record publication mismatch");
        records_.at(id) = std::move(pending_);
    }

    bool row_is_table_008082cd() override { return lua_istable(state_, row_) != 0; }
    bool open_details_008082e9() override {
        lua_getfield(state_, row_, kSensorDetailsKey);
        details_ = lua_gettop(state_);
        return lua_istable(state_, details_) != 0;
    }
    std::vector<std::string> details_keys_0080834a() override { return keys(details_); }
    std::vector<std::string> observer_group_keys_008084b5(const std::string& key) override {
        lua_settop(state_, details_);
        lua_getfield(state_, details_, key.c_str());
        observer_ = lua_gettop(state_);
        auto result = keys(observer_);
        gui_keys_.clear();
        gui_key_ = 0;
        for (const auto& candidate : result) {
            if (_stricmp(candidate.c_str(), kSensorGuiRangeKey) == 0) gui_keys_.push_back(candidate);
        }
        return result;
    }
    std::vector<SensorLuaEntryRow> subject_rows_0080858c(const std::string& key) override {
        std::vector<SensorLuaEntryRow> result;
        each_row(key, [&](int index) {
            SensorLuaEntryRow row;
            row.dist = number(index, kSensorEntryDistKey);
            row.gain = number(index, kSensorEntryGainKey);
            lua_getfield(state_, index, kSensorEntryMaxLevelKey);
            // 00B66380 accepts exact NUMBER; numeric strings take default 2.
            row.max_level = lua_type(state_, -1) == LUA_TNUMBER
                ? lua_number_integer_00b66290(lua_tonumber(state_, -1), sse2_conversion_)
                : kSensorMaxLevelDefault;
            lua_pop(state_, 1);
            row.raw_type = integer(index, kSensorEntryRawTypeKey);
            angle(index, row.has_angle, row.angle);
            result.push_back(row);
        });
        return result;
    }
    std::vector<SensorLuaRingRow> gui_range_rows_00808786() override {
        std::vector<SensorLuaRingRow> result;
        // The native loader still holds the iterated value at this call. Keep
        // each actual case-insensitive GUIRange key, including duplicate case
        // variants, rather than refetching a hard-coded spelling.
        const std::string& key = gui_keys_.at(gui_key_++);
        each_row(key, [&](int index) {
            SensorLuaRingRow row;
            row.range = number(index, kSensorRingRangeKey);
            row.raw_type = integer(index, kSensorEntryRawTypeKey);
            angle(index, row.has_angle, row.angle);
            lua_getfield(state_, index, kSensorRingColorKey);
            // Use the existing whole 00B67C40 body on a view of this actual
            // Color table. Its four tracked children are released by that
            // body; no tracking reference to the view itself is registered.
            NativeLuaStateStorage owner;
            construct_native_lua_state_00b66bd0(&owner);
            owner.state_04 = state_;
            NativeLuaObjectStorage value{&owner, 2, lua_gettop(state_), 0, 0, {}};
            float color[4];
            native_lua_get_float4_00b67c40(value, color);
            std::memcpy(row.color.data(), color, sizeof(color));
            lua_pop(state_, 1);
            result.push_back(row);
        });
        return result;
    }

private:
    std::vector<std::string> keys(int table) {
        std::vector<std::string> result;
        if (lua_isnil(state_, table)) return result;
        lua_pushnil(state_);
        while (lua_next(state_, table) != 0) {
            // Details/observer keys are native GetString inputs. Do not coerce
            // a numeric iterator key, which would invalidate lua_next's key.
            if (lua_type(state_, -2) != LUA_TSTRING)
                throw std::runtime_error("ReconClass Details/observer key is not a string");
            std::size_t length = 0;
            const char* text = lua_tolstring(state_, -2, &length);
            result.emplace_back(text, length);
            lua_pop(state_, 1);
        }
        return result;
    }
    template<class Read> void each_row(const std::string& key, Read&& read) {
        lua_settop(state_, observer_);
        lua_getfield(state_, observer_, key.c_str());
        const int table = lua_gettop(state_);
        if (!lua_isnil(state_, table)) {
            lua_pushnil(state_);
            while (lua_next(state_, table) != 0) {
                read(lua_gettop(state_));
                lua_pop(state_, 1);
            }
        }
        lua_pop(state_, 1);
    }
    double number(int table, const char* key) {
        lua_getfield(state_, table, key);
        const float value = lua_number_float32_00b66270(lua_tonumber(state_, -1));
        lua_pop(state_, 1);
        return value;
    }
    int integer(int table, const char* key) {
        lua_getfield(state_, table, key);
        const auto value = lua_number_integer_00b66290(lua_tonumber(state_, -1), sse2_conversion_);
        lua_pop(state_, 1);
        return value;
    }
    void angle(int table, bool& present, double& value) {
        lua_getfield(state_, table, kSensorEntryAngleKey);
        present = lua_isnil(state_, -1) == 0;
        value = present ? lua_number_float32_00b66270(lua_tonumber(state_, -1)) : 0.0;
        lua_pop(state_, 1);
    }

    lua_State* state_;
    Records& records_;
    int entry_top_;
    bool sse2_conversion_;
    int global_{0}, row_{0}, details_{0}, observer_{0};
    std::unique_ptr<SensorClassTable> pending_;
    std::vector<std::string> gui_keys_;
    std::size_t gui_key_{0};
};

} // namespace

std::unique_ptr<SensorClassRegistryHost> GameReconSensorClassRegistry::bind(lua_State& state) {
    return std::make_unique<SensorClassBinding>(state, records_);
}

} // namespace bsp::game
