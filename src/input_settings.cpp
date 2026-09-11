#include "bsp/input_settings.hpp"

#include <cstddef>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
namespace {

// __stricmp ordering, the comparator BSP_NativeString_LessCaseInsensitive uses.
int compare_case_insensitive(const std::string& left, const std::string& right) {
    const std::size_t count = left.size() < right.size() ? left.size() : right.size();
    for (std::size_t i = 0; i < count; ++i) {
        unsigned int a = static_cast<unsigned char>(left[i]);
        unsigned int b = static_cast<unsigned char>(right[i]);
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a != b) return a < b ? -1 : 1;
    }
    if (left.size() == right.size()) return 0;
    return left.size() < right.size() ? -1 : 1;
}

// 006a7f10 / 006a9453 test one exact Lua type; nothing here relies on
// truthiness, matching the native helpers 00b660a0 and 00b66000.
bool is_nil(lua_State* state, int index) { return lua_type(state, index) == LUA_TNIL; }
bool is_string(lua_State* state, int index) { return lua_type(state, index) == LUA_TSTRING; }
bool is_table(lua_State* state, int index) { return lua_type(state, index) == LUA_TTABLE; }

// 00b662b0 is lua_tolstring with a null length. It rewrites a number in place,
// so every read here works on a copy and leaves the original slot alone.
std::string read_string(lua_State* state, int index) {
    lua_pushvalue(state, index);
    const char* text = lua_tolstring(state, -1, nullptr);
    std::string value = text ? std::string(text) : std::string();
    lua_pop(state, 1);
    return value;
}

// 00b66290: lua_tonumber, spill to float32, truncate to a signed integer.
std::int32_t read_int(lua_State* state, int index) {
    const float value = static_cast<float>(lua_tonumber(state, index));
    return static_cast<std::int32_t>(value);
}

// 00b66270: lua_tonumber spilled to float32 without the truncation.
float read_float(lua_State* state, int index) {
    return static_cast<float>(lua_tonumber(state, index));
}

// Pushes t[key] and returns whether it is non-nil. The caller always pops.
bool push_index(lua_State* state, int table_index, lua_Integer key) {
    lua_pushnumber(state, static_cast<lua_Number>(key));
    lua_gettable(state, table_index < 0 ? table_index - 1 : table_index);
    return !is_nil(state, -1);
}

bool push_field(lua_State* state, int table_index, const char* key) {
    lua_pushstring(state, key);
    lua_gettable(state, table_index < 0 ? table_index - 1 : table_index);
    return !is_nil(state, -1);
}

// KeyboardSetup[d].Inputs, walked at 006a7eff. A bare string contributes only a
// name (006a7f91); a table contributes [1] as the name and [2] as its codes.
bool load_inputs(lua_State* state, DeviceSettings& device, std::string& error) {
    if (!is_table(state, -1)) {
        error = device.name + ".Inputs is not a table";
        return false;
    }
    for (lua_Integer index = 1;; ++index) {
        if (!push_index(state, -1, index)) {
            lua_pop(state, 1);
            break;
        }
        InputEntry entry;
        if (is_string(state, -1)) {
            entry.name = read_string(state, -1);
        } else if (is_table(state, -1)) {
            entry.table_form = true;
            push_index(state, -1, 1);
            entry.name = read_string(state, -1);
            lua_pop(state, 1);
            // 006a8067..006a80b1: resize, preserving existing elements when a
            // case-insensitive input name repeats. Bare labels create no rows.
            device.runtime_bindings[entry.name].resize(2);
            device.runtime_reverse[entry.name].resize(2, false);
            if (push_index(state, -1, 2)) {
                for (lua_Integer code = 1;; ++code) {
                    if (!push_index(state, -1, code)) {
                        lua_pop(state, 1);
                        break;
                    }
                    entry.codes.push_back(read_int(state, -1));
                    lua_pop(state, 1);
                }
            }
            lua_pop(state, 1);
        } else {
            lua_pop(state, 1);
            error = device.name + ".Inputs holds neither a string nor a table";
            return false;
        }
        device.input_names.push_back(entry.name);
        device.inputs.push_back(entry);
        lua_pop(state, 1);
    }
    return true;
}

// KeyboardSetup[d].Sensitivities, walked at 006a8258: {name, value, {codes...}}.
bool load_sensitivities(lua_State* state, DeviceSettings& device, std::string& error) {
    if (!is_table(state, -1)) {
        error = device.name + ".Sensitivities is not a table";
        return false;
    }
    for (lua_Integer index = 1;; ++index) {
        if (!push_index(state, -1, index)) {
            lua_pop(state, 1);
            break;
        }
        SensitivityEntry entry;
        push_index(state, -1, 1);
        entry.name = read_string(state, -1);
        lua_pop(state, 1);
        push_index(state, -1, 2);
        entry.value = read_int(state, -1);
        lua_pop(state, 1);
        if (push_index(state, -1, 3)) {
            for (lua_Integer code = 1;; ++code) {
                if (!push_index(state, -1, code)) {
                    lua_pop(state, 1);
                    break;
                }
                entry.codes.push_back(read_int(state, -1));
                lua_pop(state, 1);
            }
        }
        lua_pop(state, 1);
        device.sensitivities.push_back(entry);
        // 006a8448: the saved runtime multiplier is initially 1.0f, independent
        // of the integer in the Lua sensitivity description.
        device.runtime_sensitivities[entry.name] = 1.0f;
        lua_pop(state, 1);
    }
    return true;
}

// KeyboardSetup[d].BaseSensitivities, two nested `next` walks at 006a8561 and
// 006a85da. A stored zero becomes 1e-8f (DAT_00cf7fe8) at 006a8616.
bool load_base_sensitivities(lua_State* state, DeviceSettings& device, std::string& error) {
    if (!is_table(state, -1)) {
        error = device.name + ".BaseSensitivities is not a table";
        return false;
    }
    lua_pushnil(state);
    while (lua_next(state, -2)) {
        const std::int32_t outer = read_int(state, -2);
        if (!is_table(state, -1)) {
            lua_pop(state, 2);
            error = device.name + ".BaseSensitivities holds a non-table row";
            return false;
        }
        std::map<std::int32_t, float>& row = device.base_sensitivities[outer];
        lua_pushnil(state);
        while (lua_next(state, -2)) {
            const std::int32_t inner = read_int(state, -2);
            const float value = read_float(state, -1);
            row[inner] = value == 0.0f ? 1e-8f : value;
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    return true;
}

// KeyboardSetup[d].AxisPairs, walked at 006a871c: each element is {[1], [2]}.
bool load_axis_pairs(lua_State* state, DeviceSettings& device, std::string& error) {
    if (!is_table(state, -1)) {
        error = device.name + ".AxisPairs is not a table";
        return false;
    }
    for (lua_Integer index = 1;; ++index) {
        if (!push_index(state, -1, index)) {
            lua_pop(state, 1);
            break;
        }
        AxisPair pair;
        push_index(state, -1, 1);
        pair.first = read_string(state, -1);
        lua_pop(state, 1);
        push_index(state, -1, 2);
        pair.second = read_string(state, -1);
        lua_pop(state, 1);
        device.axis_pairs.push_back(pair);
        lua_pop(state, 1);
    }
    return true;
}

// KeyboardSetup[d].Min1SensHacks, guarded by a nil test at 006a89de and
// inserted into a set of ints at 006a8a6d.
void load_min1_sens_hacks(lua_State* state, DeviceSettings& device) {
    if (!is_table(state, -1)) return;
    for (lua_Integer index = 1;; ++index) {
        if (!push_index(state, -1, index)) {
            lua_pop(state, 1);
            break;
        }
        device.min1_sens_hacks.insert(read_int(state, -1));
        lua_pop(state, 1);
    }
}

bool load_devices(lua_State* state, InputSettings& out, std::string& error) {
    lua_getglobal(state, "KeyboardSetup");
    if (!is_table(state, -1)) {
        lua_pop(state, 1);
        error = "KeyboardSetup is not a table";
        return false;
    }
    bool ok = true;
    for (lua_Integer index = 1; ok; ++index) {
        if (!push_index(state, -1, index)) {
            lua_pop(state, 1);
            break;
        }
        DeviceSettings device;
        push_field(state, -1, "Name");
        device.name = read_string(state, -1);
        lua_pop(state, 1);

        push_field(state, -1, "Inputs");
        ok = load_inputs(state, device, error);
        lua_pop(state, 1);
        if (ok) {
            push_field(state, -1, "Sensitivities");
            ok = load_sensitivities(state, device, error);
            lua_pop(state, 1);
        }
        if (ok) {
            push_field(state, -1, "BaseSensitivities");
            ok = load_base_sensitivities(state, device, error);
            lua_pop(state, 1);
        }
        if (ok) {
            push_field(state, -1, "AxisPairs");
            ok = load_axis_pairs(state, device, error);
            lua_pop(state, 1);
        }
        if (ok) {
            push_field(state, -1, "Min1SensHacks");
            load_min1_sens_hacks(state, device);
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
        if (!ok) break;
        out.device_order.push_back(device.name);
        out.devices[device.name] = device;
    }
    lua_pop(state, 1);
    return ok;
}

// Global InputNames, a `next` walk at 006a8c06 over string keys.
bool load_input_names(lua_State* state, InputSettings& out, std::string& error) {
    lua_getglobal(state, "InputNames");
    if (!is_table(state, -1)) {
        lua_pop(state, 1);
        error = "InputNames is not a table";
        return false;
    }
    lua_pushnil(state);
    while (lua_next(state, -2)) {
        InputNameRecord record;
        push_index(state, -1, 1);
        record.primary = read_int(state, -1);
        lua_pop(state, 1);
        push_index(state, -1, 2);
        record.secondary = read_int(state, -1);
        lua_pop(state, 1);
        out.input_names[read_string(state, -2)] = record;
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return true;
}

// Global Conflicts, read as Conflicts.Groups (006a8dbf) then Conflicts.Pairs
// (006a9209). Pairs entries are stored one lower than the Lua table holds them.
bool load_conflicts(lua_State* state, InputSettings& out, std::string& error) {
    lua_getglobal(state, "Conflicts");
    if (!is_table(state, -1)) {
        lua_pop(state, 1);
        error = "Conflicts is not a table";
        return false;
    }
    if (push_field(state, -1, "Groups")) {
        for (lua_Integer group = 1;; ++group) {
            if (!push_index(state, -1, group)) {
                lua_pop(state, 1);
                break;
            }
            std::vector<ConflictGroupEntry> entries;
            for (lua_Integer member = 1;; ++member) {
                if (!push_index(state, -1, member)) {
                    lua_pop(state, 1);
                    break;
                }
                ConflictGroupEntry entry;
                push_index(state, -1, 1);
                entry.first = read_string(state, -1);
                lua_pop(state, 1);
                push_index(state, -1, 2);
                entry.second = read_string(state, -1);
                lua_pop(state, 1);
                entries.push_back(entry);
                lua_pop(state, 1);
            }
            out.conflict_groups.push_back(entries);
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    if (push_field(state, -1, "Pairs")) {
        for (lua_Integer index = 1;; ++index) {
            if (!push_index(state, -1, index)) {
                lua_pop(state, 1);
                break;
            }
            ConflictPair pair;
            push_index(state, -1, 1);
            pair.first = read_int(state, -1) - 1;
            lua_pop(state, 1);
            push_index(state, -1, 2);
            pair.second = read_int(state, -1) - 1;
            lua_pop(state, 1);
            out.conflict_pairs.push_back(pair);
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    lua_pop(state, 1);
    return true;
}

}  // namespace

bool CaseInsensitiveLess::operator()(const std::string& left, const std::string& right) const {
    return compare_case_insensitive(left, right) < 0;
}

bool load_keyboard_setup(lua_State* state, InputSettings& out, std::string& error) {
    if (!state) {
        error = "no Lua state";
        return false;
    }
    // 006a7cee and 006a7d32 clear the device map and the ordered name vector
    // before the walk; nothing else in the object is reset.
    out.devices.clear();
    out.device_order.clear();
    const int top = lua_gettop(state);
    bool ok = load_devices(state, out, error);
    if (ok) ok = load_input_names(state, out, error);
    if (ok) ok = load_conflicts(state, out, error);
    if (ok) {
        // 006a9453 tests the exact boolean type and falls back to false.
        lua_getglobal(state, "DEVINPUTS");
        out.dev_inputs = lua_type(state, -1) == LUA_TBOOLEAN && lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
    }
    lua_settop(state, top);
    return ok;
}

bool load_controller_input_names(lua_State* state, InputSettings& out, std::string& error) {
    if (!state) {
        error = "no Lua state";
        return false;
    }
    const int top = lua_gettop(state);
    lua_getglobal(state, "ControllerInputNames");
    if (!is_table(state, -1)) {
        lua_settop(state, top);
        error = "ControllerInputNames is not a table";
        return false;
    }
    lua_pushnil(state);
    while (lua_next(state, -2)) {
        const std::string device = read_string(state, -2);
        if (!is_table(state, -1)) {
            lua_settop(state, top);
            error = "ControllerInputNames." + device + " is not a table";
            return false;
        }
        std::map<std::int32_t, std::string>& labels = out.controller_input_names[device];
        lua_pushnil(state);
        while (lua_next(state, -2)) {
            labels[read_int(state, -2)] = read_string(state, -1);
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    lua_settop(state, top);
    return true;
}

InputDevice* InputDeviceTable::slot(int device_class, int index) const {
    if (device_class < 0 || device_class >= input_device_class_count) return nullptr;
    if (index < 0 || index >= input_device_slot_count) return nullptr;
    return slots_[device_class][index];
}

bool InputDeviceTable::attach(InputDevice* device, int requested_slot, std::string& error) {
    // 00a904e0 dereferences the device without a null check and never validates
    // the class; both faults are reported here instead.
    if (!device) {
        error = "no device";
        return false;
    }
    const int device_class = device->device_class();
    if (device_class < 0 || device_class >= input_device_class_count) {
        error = "device class out of range";
        return false;
    }
    int index = requested_slot;
    if (requested_slot == -1) {
        index = 0;
        while (slots_[device_class][index] != nullptr) {
            ++index;
            if (index >= input_device_slot_count) break;
        }
        // The native scan leaves the counter at 8 and writes past the row.
        if (index >= input_device_slot_count) {
            error = "no free slot in device class";
            return false;
        }
    }
    if (index < 0 || index >= input_device_slot_count) {
        error = "slot index out of range";
        return false;
    }
    device->assigned_slot = index;
    slots_[device_class][index] = device;
    return true;
}

void InputDeviceTable::reset_all() {
    for (int device_class = 0; device_class < input_device_class_count; ++device_class) {
        for (int index = 0; index < input_device_slot_count; ++index) {
            InputDevice* device = slots_[device_class][index];
            if (device) device->on_slot_reset();
        }
    }
}

int InputDeviceTable::occupied_count() const {
    int count = 0;
    for (int device_class = 0; device_class < input_device_class_count; ++device_class) {
        for (int index = 0; index < input_device_slot_count; ++index) {
            if (slots_[device_class][index]) ++count;
        }
    }
    return count;
}

bool create_direct_input_devices(InputDeviceTable& table,
                                 DirectInputHost& host,
                                 const DirectInputCreateParams& params,
                                 std::string& error) {
    // 00a982d0 ignores the DirectInput8Create HRESULT and calls AddRef on the
    // returned pointer immediately, so a failure faults there.
    if (!host.create_interface(params.version)) {
        error = "DirectInput8Create failed";
        return false;
    }
    host.add_ref();
    host.enum_devices(params.enum_device_type, params.enum_flags);
    for (int index = 0; index < params.pad_device_count; ++index) {
        InputDevice* device = host.create_pad_device(index);
        // The native loop passes a null device straight to 00a904e0 when the
        // 0x240-byte allocation fails.
        if (!device) {
            error = "pad device allocation failed";
            return false;
        }
        if (!table.attach(device, -1, error)) return false;
    }
    return true;
}

const DirectInputDeviceProfile& keyboard_device_profile() {
    // 00a9a3e0: CreateDevice(GUID_SysKeyboard), SetDataFormat(c_dfDIKeyboard at
    // 00d79804), SetCooperativeLevel(window, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE).
    static const DirectInputDeviceProfile profile{
        "{6F1D2B61-D5A0-11CF-BFC7-444553540000}", "00d79804", 0x06u, true, 0x310u};
    return profile;
}

const DirectInputDeviceProfile& mouse_device_profile() {
    // 00a9a290: CreateDevice(GUID_SysMouse), SetDataFormat(00d795fc); the
    // constructor sets no cooperative level.
    static const DirectInputDeviceProfile profile{
        "{6F1D2B60-D5A0-11CF-BFC7-444553540000}", "00d795fc", 0u, false, 0x23cu};
    return profile;
}

const DirectInputDeviceProfile& joystick_device_profile() {
    // 00a99940: CreateDevice(the enumerated instance GUID), SetDataFormat over a
    // format built at device+0x274 from EnumObjects, SetCooperativeLevel(window,
    // DISCL_EXCLUSIVE|DISCL_FOREGROUND).
    static const DirectInputDeviceProfile profile{
        "DIDEVICEINSTANCE.guidInstance", "device+0x274", 0x05u, true, 0xb48u};
    return profile;
}

}  // namespace bsp
