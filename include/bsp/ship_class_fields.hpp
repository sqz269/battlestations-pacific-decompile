// Ship class descriptor field reader.
//
// Addresses: 00831840 (the ship family's override of descriptor vtable slot +8h),
// and the game-side helpers it reaches: 0082FD20 / 0049F9B0 / 0049D4C0 (the
// Traffic record vector), 0082F8E0 / 0082E1F0 (the Idle record vector), 00499030
// and 00444BE0 (the two per-Idle weight maps), 004D9C00 (the effect-handle
// append), 004674F0 (the ExplosionTypes array growth) and 00441F30 (the integer
// list growth). 00870CD0, 00964790, 004B1400, 0048E840, 0048E8D0, 0048E960,
// 00424C40, 0082B170, 0082AD20, 0082E160, 004D17C0 and the BSP_LuaObject_*
// wrappers are read, not reconstructed.
//
// Evidence: the listing exports/bsp/functions/00831840/assembly.txt. Every
// address in the tables below is a call site or a store the listing makes; see
// docs/SHIP_CLASS_FIELDS.md for the per-key table and the shipped-file counts,
// and reports/ship_class_fields.json for the same facts as data. The names are
// hypotheses drawn from the Lua key that fills each slot, not recovered symbols.
//
// This supersedes the provisional kVehicleClassShipFieldSchema table in
// vehicle_class_fields.hpp, which tabulated 00831840's key strings without
// decoding the nested walks; that table's offsets for every nested key are
// wrong. Nothing declared there is redefined here.
//
// State: analysed and reconstructed, build-tested and installed-file-checked;
// not a binary-compatible replacement and not validated against the running
// game. The reader sequence is expressed over an injected host with one method
// per native Lua call site, in the style of bsp::VehicleClassFieldHost.

#ifndef BSP_SHIP_CLASS_FIELDS_HPP
#define BSP_SHIP_CLASS_FIELDS_HPP

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/vehicle_class_fields.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Key schema as data
// ---------------------------------------------------------------------------

// How 00831840 turns the Lua value into the bytes it stores. The plain wrappers
// are the same ones vehicle_class_fields.hpp names; the compound forms below
// exist because the ship reader post-processes the wrapper's answer.
enum class ShipClassConversion : std::uint8_t {
    Number,           // 00B66270, written unconditionally
    NumberOr,         // 00B66330 with a literal float default
    NumberOrSlot,     // 00B66270 after an IsNil test; the default is another slot
    NumberOrSettings, // 00B66330 with the default taken from the 00424C40 settings object
    NumberToInt,      // 00B66270 then the CRT float-to-int at 00BF7420
    NumberReciprocal, // 00B66270, stored as 1.0f / value
    Integer,          // 00B66290
    IntegerOr,        // 00B66380 with a literal default
    IntegerOrZero,    // 00B66290 after an IsNil test; the slot is zeroed when absent
    IntegerOrAsFloat, // 00B66380 then CVTSI2SS; the slot is a float
    IntegerMinusOne,  // 00B66290 then value - 1, a one-based Lua index
    EffectHandle,     // an integer id, then 00870CD0 wraps it in the refcounted handle
    EffectHandleList, // as above, appended to a handle vector by 004D9C00
    IndexedEffectList,   // the integer key lands in one vector, the handle in another
    ClassPointerOr,      // an integer id resolved by 00964790, stored only when valid
    ClassWeightMap,      // a name -> float map whose key resolves to a class pointer
    NameWeightMap,       // a name -> float map keyed by the raw string
    IntegerList,         // key/value or indexed walk into a {begin,end,capacity} vector
    IntegerListMinusOne, // as above, each value stored minus one
    IntegerArray,        // a {data,count,capacity} array grown by 004674F0
    RecordVector,        // a vector of fixed-size records the reader resizes and fills
    Vector3Array8,       // exactly eight 12-byte slots
    Vector4,             // four floats plus a present flag
    StringPairList,      // a list of two-element string arrays
    CamoArray,           // an integer-keyed array of heap camo records
    Table,               // opened and walked, never converted
};

// kShipClassNoOffset marks a key that only opens a sub-table.
inline constexpr std::uint32_t kShipClassNoOffset = 0xFFFFFFFFu;

struct ShipClassFieldSpec {
    const char* path;            // the dotted key path, "[]" for an array step
    ShipClassConversion conversion;
    std::uint32_t offset;        // descriptor offset, or an offset inside the owning record
    std::uint32_t site;          // the BSP_LuaObject_GetByName / GetByIndex call site
    std::uint32_t store;         // the instruction that writes the slot, 0 when it is a walk
    const char* fallback;        // the default, in the form the listing shows it
    const char* note;            // provenance, never empty for a nested or post-processed key
};

// 00831840 in body order: 67 keys, counting each nested path once.
extern const ShipClassFieldSpec kShipClassFieldSchema[];
std::size_t ship_class_field_count() noexcept;

// A linear search over the schema; returns nullptr when the path is not consumed.
const ShipClassFieldSpec* ship_class_find_field(const char* path) noexcept;

// ---------------------------------------------------------------------------
// Descriptor layout beyond the vehicle base
// ---------------------------------------------------------------------------

// Offsets 00831840 writes into the ship descriptor. vehicle_class_fields.hpp
// already names +0h..+134h; nothing there is repeated. Grouped in a struct so
// the member names cannot collide with any other header's constants.
struct ShipClassDescriptorOffsets {
    // The 20-record HoD block, +138h..+4F7h. See ShipClassHoDOffsets.
    static constexpr std::uint32_t kHoDRecords = 0x138;
    static constexpr std::uint32_t kHoDRecordCount = 20;

    // Motion, 00831882..00831998.
    static constexpr std::uint32_t kMaxRotAngle = 0x4F8;
    static constexpr std::uint32_t kMaxRotAngleChangeRatio = 0x4FC;
    static constexpr std::uint32_t kMaxSpeed = 0x500;
    static constexpr std::uint32_t kMaxAccel = 0x504;
    static constexpr std::uint32_t kRetardation = 0x508;

    // Collision and kamikaze, 008319C0..00831AAC.
    static constexpr std::uint32_t kCollisionMaxSpeedDamage = 0x50C;
    static constexpr std::uint32_t kKamikazeDamage = 0x510;
    static constexpr std::uint32_t kKamikazeBlastDamage = 0x514;
    static constexpr std::uint32_t kKamikazeBlastRange = 0x518;

    static constexpr std::uint32_t kExplosionEfx = 0x51C;   // 00831CF4
    static constexpr std::uint32_t kCaptainCameraHeight = 0x538;
    static constexpr std::uint32_t kCameraDistanceFront = 0x53C;
    static constexpr std::uint32_t kCameraDistanceSide = 0x540;
    static constexpr std::uint32_t kCameraDistanceVertical = 0x544;
    static constexpr std::uint32_t kCameraMinHeight = 0x548;

    // Sinking, 00831FFC..008320EB.
    static constexpr std::uint32_t kDamageToDeath = 0x54C;
    static constexpr std::uint32_t kTimeToDeath = 0x550;
    static constexpr std::uint32_t kPumpTimeToEmpty = 0x554;
    static constexpr std::uint32_t kWaterForceMultiplier = 0x558;

    static constexpr std::uint32_t kDeathEfx = 0x55C;               // 00831BE8
    static constexpr std::uint32_t kFireDamagePerFireTick = 0x574;  // 00833CB9

    // Wake and propeller particles.
    static constexpr std::uint32_t kBowParticle = 0x5AC;   // 00832167
    static constexpr std::uint32_t kBowWave = 0x630;       // 0083222B
    static constexpr std::uint32_t kWaveStern = 0x634;     // 008322EF
    static constexpr std::uint32_t kRotorParticle = 0x65C; // 008323D4

    // DamageSmoke.
    static constexpr std::uint32_t kDamageSmokeMaxNumber = 0x660;  // 00832495
    static constexpr std::uint32_t kDamageSmokeEffects = 0x664;    // vector base, 00832571

    static constexpr std::uint32_t kSmoke = 0x674;        // 00832BB6
    static constexpr std::uint32_t kExplosionTypes = 0x690;        // data
    static constexpr std::uint32_t kExplosionTypesCount = 0x694;
    static constexpr std::uint32_t kExplosionTypesCapacity = 0x698;
    static constexpr std::uint32_t kRotorSpdTurnDiff = 0x69C;      // 00831AF1
    static constexpr std::uint32_t kRotorSpd = 0x6A0;              // 00831B41
    static constexpr std::uint32_t kSmokeSecondary = 0x6A4;        // the same id, a second handle
    static constexpr std::uint32_t kSmokeTertiary = 0x6A8;         // and a third
    static constexpr std::uint32_t kUnderwaterArmour = 0x6B4;      // 00831D99
    static constexpr std::uint32_t kDamageThreshold = 0x6B8;       // 00831DE5

    // Traffic: a vector of 50h-byte records; the listing reads +6E0h as begin and
    // +6E4h as end, so +6DCh is the vector's first member.
    static constexpr std::uint32_t kTraffic = 0x6DC;
    static constexpr std::uint32_t kTrafficBegin = 0x6E0;
    static constexpr std::uint32_t kTrafficEnd = 0x6E4;

    // Idle: a vector of 1Ch-byte records, read the same way.
    static constexpr std::uint32_t kIdle = 0x6FC;
    static constexpr std::uint32_t kIdleBegin = 0x700;
    static constexpr std::uint32_t kIdleEnd = 0x704;

    static constexpr std::uint32_t kCamosBegin = 0x710;  // 0083360A bounds trap
    static constexpr std::uint32_t kCamosEnd = 0x714;

    static constexpr std::uint32_t kHullWaterLineRatio = 0x71C;  // 00832D9A
    static constexpr std::uint32_t kHullSegments = 0x720;        // 00832DE8, an int
    static constexpr std::uint32_t kLSClassId = 0x724;           // 00833A76
    static constexpr std::uint32_t kLSPoints = 0x728;            // eight vec3, +728h..+787h
    static constexpr std::uint32_t kLSReloadRate = 0x788;        // 1 / LSReload
    static constexpr std::uint32_t kLandingShipClass = 0x78C;    // a class pointer, 0 when unset
    static constexpr std::uint32_t kLandingShipAmount = 0x790;
    static constexpr std::uint32_t kLandingShipCoolDown = 0x794;
    static constexpr std::uint32_t kHackShipRotationAdd = 0x798;
    static constexpr std::uint32_t kHackShipHeightAdd = 0x79C;
    static constexpr std::uint32_t kMaxTorpedoStock = 0x7A0;

    // DamagedGFXRemove, three {begin,end,capacity} integer vectors.
    static constexpr std::uint32_t kRemoveSlots = 0x7A8;
    static constexpr std::uint32_t kRemoveFunnels = 0x7B8;
    static constexpr std::uint32_t kRemoveFlags = 0x7C8;

    // InnerExplosionEfx: the keys in one vector, the handles in another.
    static constexpr std::uint32_t kInnerExplosionSlots = 0x7D8;
    static constexpr std::uint32_t kInnerExplosionEffects = 0x7E4;
    static constexpr std::uint32_t kStructuralDamageEffects = 0x7F4;

    static constexpr std::uint32_t kCapturePower = 0x804;  // a float, CVTSI2SS at 0083453C
};

// One HoD record. The listing's cursor is record+24h (LEA EBX,[EDI+0x15c] at
// 00832695 for record 0), and the record stride is 30h (ADD EBX,0x30 in the
// loop tail). Only the three lists the reader fills are named; +0Ch..+13h and
// +20h are untouched here and stay unknown.
struct ShipClassHoDOffsets {
    static constexpr std::uint32_t kStride = 0x30;
    static constexpr std::uint32_t kFlags = 0x00;   // filled by 00442190 at 0083277A
    static constexpr std::uint32_t kSmokeBegin = 0x14;
    static constexpr std::uint32_t kSmokeEnd = 0x18;
    static constexpr std::uint32_t kSmokeCapacity = 0x1C;
    static constexpr std::uint32_t kIdleBegin = 0x24;
    static constexpr std::uint32_t kIdleEnd = 0x28;
    static constexpr std::uint32_t kIdleCapacity = 0x2C;
};

// One Traffic record. Constructed by 0049F9B0, appended by the resize at
// 0082FD20; only pathID is written here, and 0049D4C0 at 00832F54 fills the rest
// from the same Lua element, so the other 4Ch bytes are not attributed.
struct ShipClassTrafficOffsets {
    static constexpr std::uint32_t kStride = 0x50;
    static constexpr std::uint32_t kPathId = 0x4C;  // pathID - 1, stored at 00832F3A
};

// One Idle record. Constructed by 0082E1F0, appended by the resize at 0082F8E0.
struct ShipClassIdleOffsets {
    static constexpr std::uint32_t kStride = 0x1C;
    static constexpr std::uint32_t kTemplates = 0x00;  // 00499030 returns the float slot
    static constexpr std::uint32_t kAnims = 0x0C;      // 00444BE0 returns the float slot
    static constexpr std::uint32_t kPosId = 0x18;      // posID - 1, stored at 008330E3
};

// The three defaults 00831840 takes from the object 00424C40 returns rather than
// from a literal. The offsets are the FLD operands at 008344A6, 008344FE and
// 00833C77.
struct ShipClassSettingsOffsets {
    static constexpr std::uint32_t kHackShipRotationAdd = 0x238;
    static constexpr std::uint32_t kHackShipHeightAdd = 0x23C;
    static constexpr std::uint32_t kFireDamagePerFireTick = 0x74C;
};

// One camo record, allocated by operator new at 008335BD with the refcounted
// vtable at 00D099B0 and its three trailing pointers zeroed.
struct ShipClassCamoOffsets {
    static constexpr std::uint32_t kGunColorPresent = 0x18;  // a byte, 1 or 0
    static constexpr std::uint32_t kGunColor = 0x1C;         // four floats, +1Ch..+28h
};

// ---------------------------------------------------------------------------
// Conversion rules, as pure functions over the existing Lua value view
// ---------------------------------------------------------------------------

// A Lua array index the reader stores zero-based. 00832F30 and 008330D9 both
// subtract one unconditionally, so a shipped 0 becomes -1 rather than clamping.
std::int32_t ship_class_zero_based_index_00832f30(std::int32_t lua_index) noexcept;

// LSReload lands as its reciprocal (00833B17 loads 1.0 and 00833B19 divides), so a
// shipped 0 produces an infinity rather than being rejected.
float ship_class_reload_rate_00833b19(float lua_reload) noexcept;

// The five camera slots chain their defaults: CameraDistanceFront falls back to
// the base's Length at +A0h, Side and Vertical to Front, and CameraMinHeight to
// CaptainCameraHeight. Each test is IsNil, so a present but non-numeric value
// still goes through 00B66270 rather than the fallback.
struct ShipClassCameraInputs {
    bool has_captain_camera_height{false};
    float captain_camera_height{0.0f};
    bool has_distance_front{false};
    float distance_front{0.0f};
    bool has_distance_side{false};
    float distance_side{0.0f};
    bool has_distance_vertical{false};
    float distance_vertical{0.0f};
    bool has_min_height{false};
    float min_height{0.0f};
    float base_length{0.0f};              // descriptor+A0h
    float captain_camera_global{0.0f};    // DAT_00CE38B8, read at 00831E41
};

struct ShipClassCameraFields {
    float captain_camera_height{0.0f};
    float distance_front{0.0f};
    float distance_side{0.0f};
    float distance_vertical{0.0f};
    float min_height{0.0f};
};

// 00831E0D..00831FEC in body order, so each fallback sees the slot the previous
// key just wrote.
ShipClassCameraFields ship_class_camera_00831e0d(const ShipClassCameraInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Reconstructed values
// ---------------------------------------------------------------------------

struct ShipClassHoDFields {
    std::vector<std::int32_t> flags;   // HoD[i].Flag
    std::vector<std::int32_t> smoke;   // HoD[i].Smoke
    std::vector<std::int32_t> idle;    // HoD[i].Idle
};

struct ShipClassTrafficFields {
    std::int32_t path_id{-1};  // pathID - 1; -1 when the element has no pathID
};

struct ShipClassIdleFields {
    std::int32_t pos_id{-1};
    // The template key resolves to a class: is_soldier says which table answered.
    struct Template {
        std::string name;
        bool is_soldier{false};  // SoldierTypes rather than LandVehicleclasses
        float weight{0.0f};
    };
    std::vector<Template> templates;
    std::map<std::string, float> anims;
};

struct ShipClassCamoFields {
    std::vector<std::pair<std::string, std::string>> texture_remaps;
    bool has_gun_color{false};
    float gun_color[4]{0.0f, 0.0f, 0.0f, 0.0f};
};

struct ShipClassFields {
    float max_rot_angle{0.0f};
    float max_rot_angle_change_ratio{0.0f};
    float max_speed{0.0f};
    float max_accel{0.0f};
    float retardation{0.0f};

    float collision_max_speed_damage{0.0f};
    float kamikaze_damage{0.0f};
    float kamikaze_blast_damage{0.0f};
    float kamikaze_blast_range{0.0f};
    float rotor_spd_turn_diff{0.0f};
    float rotor_spd{15.0f};

    std::int32_t death_efx{0};
    std::int32_t explosion_efx{0};
    float underwater_armour{0.0f};
    float damage_threshold{100.0f};

    ShipClassCameraFields camera;

    float damage_to_death{-1.0f};
    float time_to_death{-1.0f};
    float pump_time_to_empty{-1.0f};
    float water_force_multiplier{1.0f};

    std::int32_t bow_particle{0};
    std::int32_t bow_wave{0};
    std::int32_t wave_stern{0};
    std::int32_t rotor_particle{0};

    std::int32_t damage_smoke_max_number{0};
    std::vector<std::int32_t> damage_smoke_effects;

    ShipClassHoDFields hod[ShipClassDescriptorOffsets::kHoDRecordCount];
    std::vector<std::int32_t> explosion_types;
    std::int32_t smoke{0};

    float hull_water_line_ratio{0.0f};
    std::int32_t hull_segments{0};

    std::vector<ShipClassTrafficFields> traffic;
    std::vector<ShipClassIdleFields> idle;
    std::vector<ShipClassCamoFields> camos;

    std::int32_t ls_class_id{0};
    float ls_points[8][3]{};
    bool has_ls_reload{false};
    float ls_reload_rate{0.0f};
    std::int32_t landing_ship_class{0};  // 0 when the id did not resolve
    std::int32_t landing_ship_amount{0};
    std::int32_t landing_ship_cool_down{60};
    float fire_damage_per_fire_tick{0.0f};
    std::int32_t max_torpedo_stock{0};

    std::vector<std::int32_t> remove_slots;
    std::vector<std::int32_t> remove_funnels;  // already minus one
    std::vector<std::int32_t> remove_flags;    // already minus one

    std::vector<std::int32_t> inner_explosion_slots;
    std::vector<std::int32_t> inner_explosion_effects;
    std::vector<std::int32_t> structural_damage_effects;

    float hack_ship_rotation_add{0.0f};
    float hack_ship_height_add{0.0f};
    float capture_power{10.0f};
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site 00831840 makes that is not a Lua wrapper the
// base reader already covers. The Lua half is not repeated: the reader sequence
// takes a VehicleClassFieldHost for that and this host for the rest.
struct ShipClassFieldHost {
    virtual ~ShipClassFieldHost() = default;

    // 00870CD0, the refcounted effect handle around an id. The reader only calls
    // it after the value passed IsInteger or came from GetIntegerOrDefault.
    virtual std::int32_t make_effect_handle(std::int32_t effect_id) = 0;

    // 0048E960 then 0048E8D0: does the named table hold this key? The reader
    // asks "LandVehicleclasses" first and falls back to "SoldierTypes".
    virtual bool class_table_contains(const char* table, const char* name) = 0;
    // 0048E840 then 00964790 (BSP_VehicleClass_GetOrCreate) with DL = 1.
    virtual bool resolve_vehicle_class(const char* name) = 0;
    // 0048E840 then 004B1400, the SoldierTypes path.
    virtual bool resolve_soldier_type(const char* name) = 0;
    // 00964790 again, by id, followed by the virtual +18h validity test at
    // 00833B9C. Returns 0 when the class is absent or the test fails.
    virtual std::int32_t resolve_landing_ship_class(std::int32_t class_id) = 0;

    // 00424C40, the settings object the three defaults come from: +238h for
    // HackShipRotationAdd, +23Ch for HackShipHeightAdd, +74Ch for
    // Fire.FireDamagePerFireTick.
    virtual float settings_float(std::uint32_t offset) = 0;
    // DAT_00CE38B8, the CaptainCameraHeight fallback read at 00831E41.
    virtual float default_captain_camera_height() = 0;
};

// 00831840, __thiscall(descriptor, LuaObject* row), RET 4. The row object is the
// receiver of every BSP_LuaObject_GetByName the body makes, the same way
// 00960230 takes it; the body chains to 00960230 at 0083186E before its first
// key, which this function does not repeat.
//
// base_length is the descriptor's +A0h Length that 00960230 has already written,
// because CameraDistanceFront falls back to it.
//
// This reproduces the walk in body order and reports what it read. It does not
// reproduce the refcount traffic, the descriptor's raw layout or the SEH states.
void read_ship_class_fields_00831840(VehicleClassFieldHost& lua,
                                     ShipClassFieldHost& host,
                                     const GuiLuaRef& row,
                                     float base_length,
                                     ShipClassFields& out);

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

// Counts from the shipped Scripts/datatables/autoload/vehicleclasses.lua, parsed
// read-only by an independent Lua-subset parser. A ship row is one whose Type is
// BattleShip, Cargo, Cruiser, Destroyer, LandingShip, MotherShip, Submarine or
// TorpedoBoat. See docs/SHIP_CLASS_FIELDS.md for the method.
inline constexpr int kShipClassShippedRows = 633;
inline constexpr int kShipClassShippedShipRows = 160;
inline constexpr int kShipClassShippedShipKeyPaths = 211;

// The shipped-row count for one schema path, in schema order.
struct ShipClassKeyCount {
    const char* path;
    std::int32_t shipped_ship_rows;
};

extern const ShipClassKeyCount kShipClassKeyCounts[];
std::size_t ship_class_key_count_count() noexcept;

// Schema paths no shipped ship row provides.
extern const char* const kShipClassUnprovidedPaths[];
std::size_t ship_class_unprovided_path_count() noexcept;

// The busiest shipped ship keys that neither this reader nor the base reader
// consumes by name. The whole list is in reports/ship_class_fields.json.
extern const ShipClassKeyCount kShipClassUnconsumedPaths[];
std::size_t ship_class_unconsumed_path_count() noexcept;

}  // namespace bsp

#endif  // BSP_SHIP_CLASS_FIELDS_HPP
