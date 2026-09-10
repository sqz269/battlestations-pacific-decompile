// Vehicle class descriptor field reader.
//
// Addresses: 00960230 (the descriptor base's vtable slot +8h, the Lua field
// reader), 0087CA80 (the shared damageable-class pre-reader it calls first),
// and the five leaf overrides that call it: 006D0B80, 00700E40, 00749210,
// 0074D4A0, 007D1F70, 00831840.
//
// Evidence: the listing of each routine (see docs/VEHICLE_CLASS_FIELDS.md for the
// per-key address table). The descriptor base and the 22-kind table live in
// vehicle_class.hpp; nothing there is redefined here. Every offset below is a
// store the listing makes into the descriptor, not a recovered symbol; the names
// are hypotheses drawn from the Lua key that fills the slot.
//
// State: analysed and reconstructed; not build-tested against the game and not a
// binary-compatible replacement. The reader sequence is expressed over an
// injected host with one method per native Lua call site, in the style of
// bsp::run_application_frame and bsp::VehicleClassHost.

#ifndef BSP_VEHICLE_CLASS_FIELDS_HPP
#define BSP_VEHICLE_CLASS_FIELDS_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/vehicle_class.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Key schema as data
// ---------------------------------------------------------------------------

// How the reader converts the Lua value once it has it. The native wrappers are
// BSP_LuaObject_GetNumber 00B66270, GetInteger 00B66290, GetBoolean 00B66250,
// GetString 00B662B0 and the four "or default" forms
// BSP_LuaReference_GetFloatOrDefault 00B66330, GetIntegerOrDefault 00B66380,
// GetBooleanOrDefault 00B662F0 and the string form 00B685C0.
enum class VehicleClassFieldConversion : std::uint8_t {
    Number,       // 00B66270, no default; the slot is written unconditionally
    NumberOr,     // 00B66330, default in default_number
    Integer,      // 00B66290
    IntegerOr,    // 00B66380, default in default_integer
    Boolean,      // 00B66250
    BooleanOr,    // 00B662F0, default in default_integer (0 or 1)
    String,       // 00B662B0
    StringOr,     // 00B685C0, default in default_string
    Table,        // the value is walked, not converted
    SoundId,      // integer, then 00870CD0 builds the refcounted sound handle
    Vector3,      // three floats read out of a Lua vector object (00B680A0)
};

// Where the value lands. Offsets are byte offsets into the descriptor unless the
// note says otherwise; kNoOffset marks a key whose value feeds a container or a
// nested object rather than a fixed slot.
inline constexpr std::uint32_t kVehicleClassFieldNoOffset = 0xFFFFFFFFu;

struct VehicleClassFieldSpec {
    const char* key;                       // the Lua key literal
    const char* container;                 // "" for the row, else the owning table path
    VehicleClassFieldConversion conversion;
    std::uint32_t offset;                  // descriptor offset, or kVehicleClassFieldNoOffset
    std::uint32_t site;                    // the address of the GetByName call site
    float default_number;                  // meaningful for NumberOr
    std::int32_t default_integer;          // meaningful for IntegerOr / BooleanOr
    const char* default_string;            // meaningful for StringOr
    const char* note;                      // provenance or a caveat, never empty
};

// 00960230 in body order. 22 row-level keys plus the nested tables they open.
extern const VehicleClassFieldSpec kVehicleClassBaseFieldSchema[];
std::size_t vehicle_class_base_field_count() noexcept;

// 0087CA80, the pre-reader 00960230 calls before its own first key.
extern const VehicleClassFieldSpec kVehicleClassSharedFieldSchema[];
std::size_t vehicle_class_shared_field_count() noexcept;

// 00831840, the ship family's slot +8h override, after it chains to 00960230.
extern const VehicleClassFieldSpec kVehicleClassShipFieldSchema[];
std::size_t vehicle_class_ship_field_count() noexcept;

// 007D1F70, the plane family's override.
extern const VehicleClassFieldSpec kVehicleClassPlaneFieldSchema[];
std::size_t vehicle_class_plane_field_count() noexcept;

// 00749210 (building/fort), 0074D4A0, 00700E40 and 006D0B80, the three small ones.
extern const VehicleClassFieldSpec kVehicleClassStructureFieldSchema[];
std::size_t vehicle_class_structure_field_count() noexcept;

// A linear search over one schema; returns nullptr when the key is not consumed.
const VehicleClassFieldSpec* vehicle_class_find_field(const VehicleClassFieldSpec* schema,
                                                     std::size_t count,
                                                     const char* container,
                                                     const char* key) noexcept;

// ---------------------------------------------------------------------------
// Descriptor layout beyond the base
// ---------------------------------------------------------------------------

// Offsets the two readers write. vehicle_class.hpp already names +70h, +74h,
// +7Ch, +6Ch and +C0h; those are not repeated. Grouped in a struct so the member
// names cannot collide with the constants that header exports.
struct VehicleClassFieldOffsets {
    // 0087CA80, the shared pre-reader. These fall inside the 0h..6Bh window the
    // descriptor doc could not attribute.
    static constexpr std::uint32_t kUnique = 0x34;          // 0087CAFC, byte
    static constexpr std::uint32_t kExplosionType = 0x40;   // 0087CD25
    static constexpr std::uint32_t kHitPoints = 0x48;       // 0087CC98, float, default 100
    static constexpr std::uint32_t kArmour = 0x4C;          // 0087CCBF, float, default 0
    static constexpr std::uint32_t kName = 0x54;            // 0087CAB8, char*

    // 00960230.
    static constexpr std::uint32_t kCockpitMesh = 0x80;     // 00960620
    static constexpr std::uint32_t kCockpitCameraX = 0x84;  // 00960679
    static constexpr std::uint32_t kCockpitCameraY = 0x88;  // 0096068E
    static constexpr std::uint32_t kCockpitCameraZ = 0x8C;  // 00960697
    static constexpr std::uint32_t kSpecRole = 0x90;        // 009606B1 / 0096072B
    static constexpr std::uint32_t kPlatformSlots = 0x94;   // 00961B6C, {T** data; int size}
    static constexpr std::uint32_t kPlatformSlotCount = 0x98;
    static constexpr std::uint32_t kLength = 0xA0;          // 0096039F, float
    static constexpr std::uint32_t kWidth = 0xA4;           // 00960368, float
    static constexpr std::uint32_t kHeight = 0xA8;          // 009603D6, float
    static constexpr std::uint32_t kTotalHeight = 0xAC;     // 00960417, default = Height
    static constexpr std::uint32_t kMass = 0xB0;            // 00960454, default 1
    static constexpr std::uint32_t kReconClass = 0xB4;      // 00962434
    static constexpr std::uint32_t kReconModifier = 0xB8;   // 009623D9, float
    static constexpr std::uint32_t kMovieCameras = 0xBC;    // 00962527, cleared at 00962D8F
    static constexpr std::uint32_t kLaunchedClass = 0xC0;   // 009604C6, default -1
    static constexpr std::uint32_t kCatapultEquipment = 0xC4;   // 0096057D
    static constexpr std::uint32_t kLaunchStock = 0xC8;         // 00960503
    static constexpr std::uint32_t kMaxLaunchedPlanes = 0xCC;   // 00960540
    static constexpr std::uint32_t kRepair = 0xD0;              // 00962E16, byte
    static constexpr std::uint32_t kSmallMapIcon = 0xD4;        // 009602C6
    static constexpr std::uint32_t kEngineSounds = 0xE8;        // 009607CD, a vector
    static constexpr std::uint32_t kContinuousFireSounds = 0xEC; // 0096088E, a vector
    static constexpr std::uint32_t kDeadMeatSound = 0xF0;       // 00960964
    static constexpr std::uint32_t kWindSound = 0xF4;           // 00960A53
    static constexpr std::uint32_t kTooFastSound = 0xF8;        // 00960B42
    static constexpr std::uint32_t kPropellerSound = 0xFC;      // 00960C31, HajoCsavarEfx
    static constexpr std::uint32_t kBowWaveSound = 0x100;       // 00960D20, OrrHullamEfx
    static constexpr std::uint32_t kElevatorSound = 0x104;      // 00960E0F
    static constexpr std::uint32_t kSonarPingSound = 0x108;     // 00960EFE
    static constexpr std::uint32_t kAmbientSound = 0x10C;       // 00961007
    static constexpr std::uint32_t kFlags = 0x110;              // 00962EC9, a 14h-stride vector
    static constexpr std::uint32_t kCost = 0x124;               // 009610D4, default 0
    static constexpr std::uint32_t kEquipmentsBegin = 0x12C;    // 009622CF
    static constexpr std::uint32_t kEquipmentsEnd = 0x130;      // 009622D5
    static constexpr std::uint32_t kDefaultEquipment = 0x134;   // 00961F45, default 0
};

// The 98h-byte weapon platform 00960230 builds for each Platforms entry
// (operator new 98h at 00961170, constructed by 007F7110). Only the fields the
// reader writes are named.
struct VehicleClassPlatformOffsets {
    static constexpr std::uint32_t kSize = 0x98;
    static constexpr std::uint32_t kSlotIndex = 0x08;   // 009611B7, the Lua key
    static constexpr std::uint32_t kPilotFires = 0x0C;  // 009611EF, byte
    static constexpr std::uint32_t kUseBayDoor = 0x0D;  // 0096127A, byte
    static constexpr std::uint32_t kMainPlatform = 0x0E; // 009612C0, byte
    static constexpr std::uint32_t kDefaultGun = 0x38;  // 0096152B and 009615C3
    static constexpr std::uint32_t kForwardAim = 0x48;  // 00961225, float
    static constexpr std::uint32_t kName = 0x8C;        // 0096130E, a duplicated char*
    static constexpr std::uint32_t kRestAngleB = 0x90;  // 00961B3A, RestAngles[2], default 0
    static constexpr std::uint32_t kRestAngleA = 0x94;  // 00961ADF, RestAngles[1], default FLT_MAX
};

// The 14h-byte flag element of the vector at descriptor+110h.
struct VehicleClassFlagOffsets {
    static constexpr std::uint32_t kStride = 0x14;
    static constexpr std::uint32_t kSizeHorizontal = 0x00;  // 00962FA4, default 2.5f
    static constexpr std::uint32_t kSizeVertical = 0x04;    // 00962FE6, default 1.5f
    static constexpr std::uint32_t kTexturePath = 0x08;     // an 8-byte {length, buffer}
    static constexpr std::uint32_t kTextureHandle = 0x10;   // 00963114
};

// ---------------------------------------------------------------------------
// Conversion rules, as pure functions over an abstract Lua value
// ---------------------------------------------------------------------------

// What the wrapper's type predicates report. IsNil is 00B65FB0, IsTable
// 00B661B0, IsInteger 00B66A60; IsUnbound 00B66420 is the iterator's
// end-of-table answer and is not a value kind.
enum class VehicleClassValueKind : std::uint8_t {
    Nil,
    Boolean,
    Integer,
    Number,   // a Lua number that is not integral
    String,
    Table,
    Other,
};

// A read-only view of one Lua value. It carries no ownership and no stack index;
// a host implementation fills it from whatever the wrapper hands back.
struct VehicleClassLuaValue {
    VehicleClassValueKind kind{VehicleClassValueKind::Nil};
    bool boolean{false};
    double number{0.0};
    std::int64_t integer{0};
    const char* string{nullptr};

    bool is_nil() const noexcept { return kind == VehicleClassValueKind::Nil; }
    bool is_table() const noexcept { return kind == VehicleClassValueKind::Table; }
    // 00B66A60 answers true only for a number with no fractional part.
    bool is_integer() const noexcept { return kind == VehicleClassValueKind::Integer; }
};

// The four "or default" wrappers. Each returns the fallback when the value is
// nil and otherwise converts; a wrong-typed value converts the way the Lua
// wrapper does, which for a string is a parse and for a boolean is 0 or 1.
float vehicle_class_number_or_00b66330(const VehicleClassLuaValue& value, float fallback) noexcept;
std::int32_t vehicle_class_integer_or_00b66380(const VehicleClassLuaValue& value,
                                               std::int32_t fallback) noexcept;
bool vehicle_class_boolean_or_00b662f0(const VehicleClassLuaValue& value, bool fallback) noexcept;
const char* vehicle_class_string_or_00b685c0(const VehicleClassLuaValue& value,
                                             const char* fallback) noexcept;

// The bare wrappers, which the reader uses only after an explicit nil or type
// test. They have no fallback and read a nil value as zero.
float vehicle_class_number_00b66270(const VehicleClassLuaValue& value) noexcept;
std::int32_t vehicle_class_integer_00b66290(const VehicleClassLuaValue& value) noexcept;
bool vehicle_class_boolean_00b66250(const VehicleClassLuaValue& value) noexcept;

// 009606BC..0096072B. The row's SpecRole string selects a role code: an exact
// "FlyingControll" is 0, "BomberPilot" is 22h, and anything else, including a
// missing key, leaves 0. The first compare is strcmp (00BF7FBF) and the second
// the case-insensitive helper 00425850, so only the second is case-tolerant.
inline constexpr std::int32_t kVehicleClassSpecRoleDefault = 0;
inline constexpr std::int32_t kVehicleClassSpecRoleBomberPilot = 0x22;
std::int32_t vehicle_class_spec_role_009606bc(const char* spec_role) noexcept;

// 00961B6C. The Platforms key is the slot index, and the descriptor's pointer
// vector at +94h is grown to hold it rather than appended to, so a table with a
// gap leaves a null slot. Returns the size the vector must reach.
std::int32_t vehicle_class_platform_slot_capacity_00961b6c(std::int32_t current_size,
                                                           std::int32_t slot_index) noexcept;

// 00962EFA. The Flags key is one-based and indexes a 14h-stride vector, so the
// element index is key - 1 and the vector is resized to key elements.
std::int32_t vehicle_class_flag_element_index_00962efa(std::int32_t lua_key) noexcept;

// ---------------------------------------------------------------------------
// Reconstructed values
// ---------------------------------------------------------------------------

struct VehicleClassFlagFields {
    float size_horizontal{2.5f};
    float size_vertical{1.5f};
    std::string forced_texture;
    void* texture{nullptr};
};

struct VehicleClassPlatformFields {
    std::int32_t slot_index{0};
    bool pilot_fires{false};
    bool use_bay_door{false};
    bool main_platform{false};
    float forward_aim{0.0f};
    std::string name;
    std::vector<std::int32_t> guns;              // Platforms[i].Gun, a list of weapon class ids
    std::int32_t default_gun{-1};                // Platforms[i].DefaultGun
    std::vector<std::int32_t> director_follows;  // Platforms[i].DirectorFollowPlatforms
    float rest_angle_a{0.0f};                    // RestAngles[1], FLT_MAX when absent
    float rest_angle_b{0.0f};                    // RestAngles[2], 0 when absent
    struct Window {
        bool no_fire{false};
        float min_horz{0.0f};
        float max_horz{0.0f};
        float min_vert{0.0f};
        float max_vert{0.0f};
    };
    std::vector<Window> windows;
};

struct VehicleClassEquipmentFields {
    std::int32_t platform{0};
    std::int32_t ammo{0};
    float reload_time{0.0f};
};

struct VehicleClassSoundFields {
    std::vector<std::int32_t> engine;      // SoundEfx.EngineSoundEfx
    std::vector<std::int32_t> continuous_fire; // SoundEfx.ContFireEfx
    std::int32_t dead_meat{0};
    std::int32_t wind{0};
    std::int32_t too_fast{0};
    std::int32_t propeller{0};   // HajoCsavarEfx
    std::int32_t bow_wave{0};    // OrrHullamEfx
    std::int32_t elevator{0};
    std::int32_t sonar_ping{0};
    std::string ambient;
};

// What 00960230 writes, in the same order the body reaches it.
struct VehicleClassBaseFields {
    void* small_map_icon{nullptr};
    std::string small_map_icon_path;
    float width{0.0f};
    float length{0.0f};
    float height{0.0f};
    float total_height{0.0f};
    float mass{1.0f};
    bool has_catapult{false};
    std::int32_t launched_class{-1};
    std::int32_t launch_stock{0};
    std::int32_t max_launched_planes{0};
    std::int32_t catapult_equipment{0};
    void* cockpit_mesh{nullptr};
    float cockpit_camera[3]{0.0f, 0.0f, 0.0f};
    std::int32_t spec_role{kVehicleClassSpecRoleDefault};
    VehicleClassSoundFields sounds;
    std::int32_t cost{0};
    std::vector<VehicleClassPlatformFields> platforms;
    std::vector<std::vector<std::int32_t>> gun_delay_groups;
    std::int32_t default_equipment{0};
    std::vector<std::vector<VehicleClassEquipmentFields>> equipments;
    float recon_modifier{0.0f};
    std::int32_t recon_class{0};
    bool repair{false};
    std::vector<VehicleClassFlagFields> flags;
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site of 00960230, in the order the body reaches
// them. Nothing has a default: none of it stands in for game behaviour that has
// not been recovered. The Lua half repeats VehicleClassHost's operations because
// the reader calls the same wrappers; it is repeated rather than inherited
// because this reader also iterates and reads booleans and floats, which the
// factory never does.
struct VehicleClassFieldHost {
    virtual ~VehicleClassFieldHost() = default;

    // 00B67800, 00B67720 and 00B67700. A ref handed back by get_* is released
    // exactly once, which is what the native's SEH unwind states enforce.
    virtual GuiLuaRef get_by_name(const GuiLuaRef& table, const char* key) = 0;
    virtual GuiLuaRef get_by_index(const GuiLuaRef& table, std::int32_t index) = 0;
    virtual void release(const GuiLuaRef& object) = 0;

    // The value behind a ref, as the type predicates and getters see it.
    virtual VehicleClassLuaValue inspect(const GuiLuaRef& object) = 0;

    // 00B67080 and 00B67190 against a key/value pair, with 00B66420 as the
    // end test. Returns false once the iterator is unbound.
    virtual bool iterate_first(const GuiLuaRef& table, GuiLuaRef& key, GuiLuaRef& value) = 0;
    virtual bool iterate_next(const GuiLuaRef& table, GuiLuaRef& key, GuiLuaRef& value) = 0;

    // 004C12B0 then 00AA2B30: the GUI manager turns the SMIcon path into the
    // icon the descriptor keeps at +D4h. The native falls back to
    // "gui\\units\\b25.tga" when the key is missing, and the fallback goes
    // through the same call.
    virtual void* load_small_map_icon(const char* path) = 0;
    // 007188A0, the CockpitMesh load.
    virtual void* load_cockpit_mesh(const char* path) = 0;
    // 00870CD0, which wraps a sound id in the refcounted handle the descriptor
    // stores. The reader only calls it when the value is an integer.
    virtual void* make_sound_handle(std::int32_t sound_id) = 0;
    // 00A83FD0, the SoundEfx.Ambient lookup by name.
    virtual void* load_ambient_sound(const char* name) = 0;
    // 00808F90, the ReconClass lookup.
    virtual std::int32_t resolve_recon_class(std::int32_t recon_id) = 0;
    // (*00F8D394)->vtable[64h] with a null second argument, the Flags
    // ForcedTexture load. Only called when the resolved path differs from the
    // empty string the reader starts from.
    virtual void* load_flag_texture(const char* path) = 0;
    // 007F7110 over 98h zeroed bytes, then the descriptor's +94h slot vector is
    // grown so the platform's own key indexes it.
    virtual void reserve_platform_slots(std::int32_t size) = 0;
};

// 00960230, __thiscall(descriptor, LuaObject* row), RET 4. The argument is the
// row object itself: the body passes it straight to BSP_LuaObject_GetByName as
// the receiver, which corrects the descriptor doc's reading of slot +8h as
// taking a string out-parameter.
//
// This reproduces the walk 00960230 makes, in body order, and reports what it
// read. It does not reproduce the refcount traffic, the pooled string storage or
// the SEH states; those belong to the native object graph.
void read_vehicle_class_base_fields_00960230(VehicleClassFieldHost& host,
                                             const GuiLuaRef& row,
                                             VehicleClassBaseFields& out);

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

// The counts recorded from the shipped
// Scripts/datatables/autoload/vehicleclasses.lua, parsed read-only by an
// independent Lua-subset parser. See docs/VEHICLE_CLASS_FIELDS.md for the
// method and reports/vehicle_class_fields.json for the whole table.
inline constexpr int kVehicleClassShippedRows = 633;
inline constexpr int kVehicleClassShippedTopLevelKeys = 198;
inline constexpr int kVehicleClassShippedKeyPaths = 342;

// A key the reader asks for that no shipped row provides, or a shipped key no
// reader consumes.
struct VehicleClassKeyGap {
    const char* container;
    const char* key;
    std::int32_t shipped_rows;  // 0 for a key the reader expects and no row has
    const char* note;
};

extern const VehicleClassKeyGap kVehicleClassUnprovidedKeys[];
std::size_t vehicle_class_unprovided_key_count() noexcept;

extern const VehicleClassKeyGap kVehicleClassUnconsumedKeys[];
std::size_t vehicle_class_unconsumed_key_count() noexcept;

}  // namespace bsp

#endif  // BSP_VEHICLE_CLASS_FIELDS_HPP
