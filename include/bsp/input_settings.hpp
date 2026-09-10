#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

// Input phase of cSkeletonAppMidway::Init (0073d410), packet app_init_input.
//
// Recovered addresses:
//   005547d0  input-settings singleton getter (0x540 object over the ctor 006ab6b0)
//   006a7be0  data-table load: ControlPresets.lua, KeyboardSetup.lua, ControllerInputNames.lua
//   00a982d0  DirectInput8 object construction and device enumeration
//   00a900f0  3x8 device-slot walk, virtual slot +0x14 on every occupied slot
//
// Evidence, native offsets, calling conventions and uncertainties are in
// docs/APP_INIT_INPUT.md and reports/app_init_input.json. Every name below is a
// hypothesis; none is a recovered symbol. No native global or type is invented:
// the loaders take an already-open Lua state and the device layer takes an
// injected host, so nothing here touches a real device or a real singleton.

struct lua_State;

namespace bsp {

// ---------------------------------------------------------------------------
// Settings model
// ---------------------------------------------------------------------------

// One element of KeyboardSetup[d].Inputs. 006a7be0 accepts two forms at
// 006a7f10: a bare string, or a table whose [1] is the name and whose [2] is an
// array of integer codes. The bare form pushes only the name (006a7f91) and
// leaves the code list empty.
struct InputEntry {
    std::string name;
    std::vector<std::int32_t> codes;
    bool table_form{};
};

// One element of KeyboardSetup[d].Sensitivities: {name, value, {codes...}}.
// 006a82be reads [1] as the name, 006a8369 reads [2] through the integer
// coercion 00b66290, 006a839e walks [3] as an array of integer codes.
struct SensitivityEntry {
    std::string name;
    std::int32_t value{};
    std::vector<std::int32_t> codes;
};

// One element of KeyboardSetup[d].AxisPairs: a two-string table read at
// 006a87da ([1]) and 006a8898 ([2]) into a 16-byte native element, i.e. two
// pooled strings. The roles of the two halves are not established.
struct AxisPair {
    std::string first;
    std::string second;
};

// Native device record: a 132-byte mapped value in the case-insensitive
// std::map at settings+0x08 (node _Isnil at +0x99, so sizeof(value_type)==0x8c
// over an 8-byte pooled-string key).
struct DeviceSettings {
    std::string name;                                   // entry.Name, 006a7dd9
    std::vector<std::string> input_names;               // native devRec+0x30
    std::vector<InputEntry> inputs;                     // entry.Inputs
    std::vector<SensitivityEntry> sensitivities;        // entry.Sensitivities
    // entry.BaseSensitivities, a nested numeric-keyed table walked with the
    // native `next` iterator pair 00b67080/00b67190. A stored zero is replaced
    // by 1e-8f at 006a8616 (DAT_00cf7fe8) so the value can be divided by.
    std::map<std::int32_t, std::map<std::int32_t, float>> base_sensitivities;
    std::vector<AxisPair> axis_pairs;                   // entry.AxisPairs
    std::set<std::int32_t> min1_sens_hacks;             // entry.Min1SensHacks
};

// Mapped value of the case-insensitive InputNames map at settings+0x24 (N2C in
// docs/INPUT_SETTINGS_TREE_CLUSTER.md: 20-byte value at node+0x14). 006a8ce4
// writes table[1] to value+0x00 and 006a8d1f writes table[2] to value+0x0c. The
// three untouched dwords keep their default-constructed value; the meaning of
// both written fields is provisional.
struct InputNameRecord {
    std::int32_t primary{};    // value+0x00
    std::int32_t secondary{};  // value+0x0c
};

// One element of Conflicts.Groups[g]: a two-string table (006a8fa2, 006a9095)
// written into an 8-byte-stride vector of pooled strings.
struct ConflictGroupEntry {
    std::string first;
    std::string second;
};

// One element of Conflicts.Pairs: a two-integer table. 006a9327 and 006a937d
// both subtract one, so the stored pair is zero-based while the Lua table is
// one-based.
struct ConflictPair {
    std::int32_t first{};
    std::int32_t second{};
};

// Case-insensitive ordering used by the three string-keyed native maps
// (BSP_NativeString_LessCaseInsensitive, reached from 0055c110 for the device
// map and from the N2C/N24 _Lbound bodies for the other two).
struct CaseInsensitiveLess {
    bool operator()(const std::string& left, const std::string& right) const;
};

// Host mirror of the 0x540 settings object built by 006ab6b0 and filled by
// 006a7be0. Only the members this packet proved are modelled.
struct InputSettings {
    // settings+0x08, case-insensitive map keyed by device name.
    std::map<std::string, DeviceSettings, CaseInsensitiveLess> devices;
    // settings+0x14, the device names in KeyboardSetup order.
    std::vector<std::string> device_order;
    // settings+0x24, case-insensitive InputNames map.
    std::map<std::string, InputNameRecord, CaseInsensitiveLess> input_names;
    // settings+0x30, Conflicts.Groups.
    std::vector<std::vector<ConflictGroupEntry>> conflict_groups;
    // settings+0x40, Conflicts.Pairs.
    std::vector<ConflictPair> conflict_pairs;
    // settings+0x50, the global DEVINPUTS flag (006a9453/006a9463).
    bool dev_inputs{};
    // settings+0x60, ControllerInputNames: device name to code-to-label map.
    std::map<std::string, std::map<std::int32_t, std::string>, CaseInsensitiveLess>
        controller_input_names;
};

// ---------------------------------------------------------------------------
// Loaders. Each takes a Lua state in which the matching script has already been
// executed, reads the globals 006a7be0 reads, and appends into `out`. The
// native routine performs no error handling at all; these report a failure
// instead of continuing over a wrong type.
// ---------------------------------------------------------------------------

// Globals KeyboardSetup, InputNames, Conflicts and DEVINPUTS. Clears
// out.devices and out.device_order first, matching 006a7cee and 006a7d32.
bool load_keyboard_setup(lua_State* state, InputSettings& out, std::string& error);

// Global ControllerInputNames, executed after KeyboardSetup in the same state.
bool load_controller_input_names(lua_State* state, InputSettings& out, std::string& error);

// ---------------------------------------------------------------------------
// Device slots: 00a904e0 (attach) and 00a900f0 (reset)
// ---------------------------------------------------------------------------

inline constexpr int input_device_class_count = 3;   // rows, 00a900f0 outer count
inline constexpr int input_device_slot_count = 8;    // columns, 00a900f0 inner count

// The two virtual slots this packet needs. 00a904e0 reads the class through
// vtable+0x08 and 00a900f0 invokes vtable+0x14 with no arguments; the base
// device implements +0x14 as a bare RET at 00a93e80.
class InputDevice {
public:
    virtual ~InputDevice() = default;
    virtual int device_class() const = 0;   // vtable+0x08
    virtual void on_slot_reset() = 0;       // vtable+0x14
    // 00a904e0 stores the resolved column into the device at +0x08.
    int assigned_slot{-1};
};

// The 3x8 pointer array at manager+0x04. Row 0 is keyboard, row 1 mouse and
// row 2 joystick/gamepad, read off the enumeration callback body 00a98030.
class InputDeviceTable {
public:
    InputDevice* slot(int device_class, int index) const;
    // 00a904e0. `requested_slot` of -1 means "first free column in the device's
    // own class row"; the native scan stops at 8 and then writes column 8,
    // which is out of the row. This reproduces the scan and reports the
    // overflow instead of writing past the row.
    bool attach(InputDevice* device, int requested_slot, std::string& error);
    // 00a900f0: every occupied slot in class order, then slot order.
    void reset_all();
    int occupied_count() const;

private:
    InputDevice* slots_[input_device_class_count][input_device_slot_count]{};
};

// ---------------------------------------------------------------------------
// DirectInput bring-up: 00a982d0
// ---------------------------------------------------------------------------

// Parameters 00a982d0 passes, as data. The IID is IID_IDirectInput8A, matched
// byte for byte against DAT_00d78d8c.
struct DirectInputCreateParams {
    std::uint32_t version{0x0800};                    // DIRECTINPUT_VERSION
    std::uint32_t enum_device_type{0};                // DI8DEVCLASS_ALL
    std::uint32_t enum_flags{1};                      // DIEDFL_ATTACHEDONLY
    int pad_device_count{4};                          // 00a98398 loop bound
    std::uint32_t pad_device_size{0x240};             // 00a98360 allocation
};

// Everything 00a982d0 does that reaches outside the object. A test supplies a
// recording implementation, so no DirectInput object is created.
class DirectInputHost {
public:
    virtual ~DirectInputHost() = default;
    // DirectInput8Create(GetModuleHandleA(nullptr), version, IID_IDirectInput8A,
    // &manager+0xe0, nullptr). The native call ignores the HRESULT.
    virtual bool create_interface(std::uint32_t version) = 0;
    virtual void add_ref() = 0;                                     // vtable+0x04
    virtual void enum_devices(std::uint32_t device_type, std::uint32_t flags) = 0;  // vtable+0x10
    // The four fixed devices allocated after the enumeration. `index` is the
    // loop counter passed to the constructor 00a9a5a0.
    virtual InputDevice* create_pad_device(int index) = 0;
};

// Reproduces the ordering of 00a982d0: create, AddRef, EnumDevices, then four
// pad devices attached with a requested slot of -1.
bool create_direct_input_devices(InputDeviceTable& table,
                                 DirectInputHost& host,
                                 const DirectInputCreateParams& params,
                                 std::string& error);

// ---------------------------------------------------------------------------
// Device-creation parameters recovered from the enumeration callback. These are
// facts about the original binary, not a device factory.
// ---------------------------------------------------------------------------

struct DirectInputDeviceProfile {
    const char* device_guid;      // GUID passed to IDirectInput8::CreateDevice
    const char* data_format;      // static or runtime-built DIDATAFORMAT
    std::uint32_t cooperative_level;  // 0 when the constructor sets none
    bool sets_cooperative_level;
    std::uint32_t allocation_size;
};

const DirectInputDeviceProfile& keyboard_device_profile();
const DirectInputDeviceProfile& mouse_device_profile();
const DirectInputDeviceProfile& joystick_device_profile();

}  // namespace bsp
