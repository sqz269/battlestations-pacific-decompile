// Plane class descriptor field reader.
//
// Addresses: 007D1F70 (the plane family's override of descriptor vtable slot
// +8h) and the game-side helpers it reaches: 0042E740 (the luaMW_init tuning
// singleton two keys are scaled by), 006EA910 (the KamikazeBulletClass lookup),
// 004B3D20 (the Wreck class lookup), 007C3510 and 007CCCA0 (the 44h-byte engine
// effect record and its vector), 007D1D30 (the 10h-byte BowWaves record vector)
// and 00871BA0 (the named-effect lookup that resolves the "LowPlaneAlt"
// literal). 00870CD0, 004D9C00, 009536B0 (the per-leaf IsKindOf) and the
// BSP_LuaObject_* wrappers are read, not reconstructed.
//
// Evidence: the listing exports/bsp/functions/007D1F70/assembly.txt. Every
// address in the tables below is a call site or a store the listing makes; see
// docs/PLANE_CLASS_FIELDS.md for the per-key table and the shipped-file counts,
// and reports/plane_class_fields.json for the same facts as data. The names are
// hypotheses drawn from the Lua key that fills each slot, not recovered symbols.
//
// This decodes the plane rows docs/VEHICLE_CLASS_FIELDS.md tabulated but did not
// verify. Nothing declared in vehicle_class_fields.hpp or ship_class_fields.hpp
// is redefined here; the Lua half of the walk reuses bsp::VehicleClassFieldHost
// and bsp::VehicleClassLuaValue, and the conversion wrappers those declare.
//
// State: analysed and reconstructed, build-tested and installed-file-checked;
// not a binary-compatible replacement and not validated against the running
// game.

#ifndef BSP_PLANE_CLASS_FIELDS_HPP
#define BSP_PLANE_CLASS_FIELDS_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/vehicle_class_fields.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Key schema as data
// ---------------------------------------------------------------------------

// How 007D1F70 turns the Lua value into the bytes it stores. The plain wrappers
// are the same ones vehicle_class_fields.hpp names; the compound forms exist
// because the plane reader post-processes the wrapper's answer.
enum class PlaneClassConversion : std::uint8_t {
    NativeString,      // 00B685C0 then the descriptor's own string at +138h
    Number,            // 00B66270, written unconditionally
    NumberOr,          // 00B66330 with a literal float default
    NumberToInt,       // 00B66270 then the CRT float-to-int at 00BF7420
    NumberScaled,      // 00B66270, then multiplied in place by two tuning floats
    NumberAndProduct,  // 00B66270 into one slot, then a tuning product into the next
    NumberReciprocalOrZero,  // FLD1/FDIVRP after an IsNil test; absent stores 0.0f
    NumberOrKindDefault,     // 00B66330 whose default is 2.0f or 4.0f by IsKindOf
    NumberGated,       // written only when the ground gate at 007D2A4B passed
    BooleanOr,         // 00B662F0 with a literal default
    BooleanOrFalse,    // IsNil, then 00B66250; absent clears the byte
    IntegerAndClass,   // 00B66290 into one slot, then 006EA910 into the next
    WreckClassOrNull,  // 00B662B0 then 004B3D20; absent stores 0
    EffectHandle,      // IsNil, then 00B66290 and 00870CD0; absent leaves the slot
    EffectHandleVector,   // an indexed walk appending handles through 004D9C00
    EngineEffectVector,   // an indexed walk appending 44h-byte records via 007CCCA0
    EffectRecordVector,   // an indexed walk appending 10h-byte records via 007D1D30
    PartAnim,             // a three-element array folded into a 10h-byte record
    Table,                // opened and walked, never converted
};

// kPlaneClassNoOffset marks a key that only opens a sub-table.
inline constexpr std::uint32_t kPlaneClassNoOffset = 0xFFFFFFFFu;

struct PlaneClassFieldSpec {
    const char* path;             // the dotted key path, "[]" for an array step
    PlaneClassConversion conversion;
    std::uint32_t offset;         // descriptor offset, or kPlaneClassNoOffset
    std::uint32_t site;           // the BSP_LuaObject_GetByName call site
    const char* fallback;         // the default, in the form the listing shows it
    const char* note;             // provenance, never empty for a compound key
};

// 007D1F70 in body order: 70 key paths over 77 GetByName sites, because seven
// keys are fetched twice, once for the presence test and once to convert.
extern const PlaneClassFieldSpec kPlaneClassFieldSchema[];
std::size_t plane_class_field_count() noexcept;

// A linear search over the schema; returns nullptr when the path is not consumed.
const PlaneClassFieldSpec* plane_class_find_field(const char* path) noexcept;

// ---------------------------------------------------------------------------
// Descriptor layout beyond the vehicle base
// ---------------------------------------------------------------------------

// Offsets 007D1F70 writes into the plane descriptor, whose size is 60Ch for
// every plane leaf (docs/VEHICLE_CLASS_DESCRIPTORS.md). vehicle_class_fields.hpp
// already names +0h..+134h and nothing there is repeated. Grouped in a struct so
// the member names cannot collide with any other header's constants.
struct PlaneClassDescriptorOffsets {
    // The ShortName native string, {size, data} written at 007D1FE9.
    static constexpr std::uint32_t kShortNameSize = 0x138;
    static constexpr std::uint32_t kShortNameData = 0x13C;

    static constexpr std::uint32_t kNumEngines = 0x140;  // 007D2064
    static constexpr std::uint32_t kJetEngines = 0x144;  // a byte, 007D20A8

    static constexpr std::uint32_t kBombControlLimit = 0x15C;
    static constexpr std::uint32_t kCarrierBased = 0x160;  // a byte, 007D2B78

    // Aerodynamics, 007D20C6..007D2295.
    static constexpr std::uint32_t kAccel = 0x164;
    static constexpr std::uint32_t kKameraMogotte = 0x168;
    static constexpr std::uint32_t kKameraFolotte = 0x16C;
    static constexpr std::uint32_t kYDrag = 0x170;
    static constexpr std::uint32_t kXDrag = 0x174;
    static constexpr std::uint32_t kExtRotAccel = 0x178;
    static constexpr std::uint32_t kStallRotAccel = 0x17C;
    static constexpr std::uint32_t kWaterRotAccel = 0x180;

    // Speeds, 007D2351..007D251F.
    static constexpr std::uint32_t kStallSpd = 0x184;
    static constexpr std::uint32_t kMaxSpd = 0x188;
    static constexpr std::uint32_t kTravelSpeed = 0x18C;
    // 007D2406: TravelSpeed times the tuning float at +334h. No key names it.
    static constexpr std::uint32_t kTravelSpeedScaled = 0x190;
    static constexpr std::uint32_t kSwimHeight = 0x194;
    static constexpr std::uint32_t kMinWaterSpd = 0x198;
    static constexpr std::uint32_t kMaxWaterSpd = 0x19C;
    static constexpr std::uint32_t kWaterDecel = 0x1A0;
    static constexpr std::uint32_t kWaterUnSpring = 0x1A4;

    // Rates, 007D2530..007D288A.
    static constexpr std::uint32_t kRollSpd = 0x1A8;
    static constexpr std::uint32_t kPitchSpd = 0x1AC;
    static constexpr std::uint32_t kYawSpd = 0x1B0;
    static constexpr std::uint32_t kYawRollRatio = 0x1B4;
    static constexpr std::uint32_t kSlideRatio = 0x1B8;
    static constexpr std::uint32_t kRollAccel = 0x1BC;
    static constexpr std::uint32_t kPitchAccel = 0x1C0;
    static constexpr std::uint32_t kYawAccel = 0x1C4;
    static constexpr std::uint32_t kTurnRollSpd = 0x1C8;
    static constexpr std::uint32_t kYawLimitAngle = 0x1CC;
    static constexpr std::uint32_t kPitchLimitAngle = 0x1D0;
    static constexpr std::uint32_t kDragPitchRatio = 0x1D4;
    static constexpr std::uint32_t kNegativePitchRatio = 0x1D8;
    static constexpr std::uint32_t kAirBrakeDrag = 0x1DC;
    static constexpr std::uint32_t kWheelBrake = 0x1E0;

    // Ordnance and ground contact, 007D2B93..007D2B38.
    static constexpr std::uint32_t kDropAngle = 0x1F0;
    static constexpr std::uint32_t kBombDelay = 0x1F4;
    // A byte set to 1 at 007D2A5E only when WheelHeight and GroundPitch are both
    // present; the failing path writes nothing, so the slot keeps its old value.
    static constexpr std::uint32_t kGroundDataPresent = 0x1F8;
    static constexpr std::uint32_t kWheelHeight = 0x1FC;
    static constexpr std::uint32_t kGroundPitch = 0x200;
    static constexpr std::uint32_t kWaterPitch = 0x204;
    static constexpr std::uint32_t kGlideRate = 0x208;

    // Resources.
    static constexpr std::uint32_t kKamikazeBulletClassId = 0x20C;  // 007D2BE9
    static constexpr std::uint32_t kKamikazeBulletClass = 0x210;    // refcounted
    static constexpr std::uint32_t kExplosionEfx = 0x214;
    // 007D3191: the effect named by the "LowPlaneAlt" literal, read from no key.
    static constexpr std::uint32_t kLowPlaneAltEfx = 0x218;
    static constexpr std::uint32_t kWreckClass = 0x21C;
    static constexpr std::uint32_t kEngineFireEfx = 0x220;
    // The DamageSmokeEfx handle vector, {begin, end, capacity} from 007D32EB.
    static constexpr std::uint32_t kDamageSmokeEfx = 0x224;
    static constexpr std::uint32_t kShellsEfx = 0x234;
    // The EngineEfxes record vector, {data, count, capacity} from 007D2FC8.
    static constexpr std::uint32_t kEngineEfxes = 0x250;

    static constexpr std::uint32_t kTurnRoll = 0x25C;
    static constexpr std::uint32_t kTurnRollLeader = 0x260;
    static constexpr std::uint32_t kTurnCircleRadius = 0x268;
    static constexpr std::uint32_t kRollMaxforceLimit = 0x274;
    static constexpr std::uint32_t kPitchMaxforceLimit = 0x278;

    // The BowWaves record vector. 007D3677 passes +550h and 007D1D30 reads its
    // begin, end and capacity from +4h, +8h and +0Ch of that address.
    static constexpr std::uint32_t kBowWaves = 0x550;

    static constexpr std::uint32_t kWingTipEfx = 0x5A4;

    // The row-level BayDoor block, 007D34A5..007D3543, defaulted at 007D3550.
    static constexpr std::uint32_t kBayDoorOpenAngle = 0x5A8;
    static constexpr std::uint32_t kBayDoorClosedAngle = 0x5AC;
    static constexpr std::uint32_t kBayDoorTimeToOpen = 0x5B0;

    static constexpr std::uint32_t kParaReloadRate = 0x5B4;  // 1.0f / ParaReload

    // The three PartAnims records, 10h bytes each. See PlanePartAnimOffsets.
    static constexpr std::uint32_t kPartAnimGears = 0x5C8;
    static constexpr std::uint32_t kPartAnimWings = 0x5D8;
    static constexpr std::uint32_t kPartAnimBayDoor = 0x5E8;

    static constexpr std::uint32_t kGearsPullTime = 0x5F8;
    static constexpr std::uint32_t kTurboTime = 0x5FC;
    static constexpr std::uint32_t kTurboRechargingTime = 0x600;
    static constexpr std::uint32_t kTurboStrength = 0x604;
    static constexpr std::uint32_t kTurboControlLimit = 0x608;

    static constexpr std::uint32_t kDescriptorSize = 0x60C;
};

// One PartAnims record. The reader reads elements [1] and [2] of the sub-table,
// stores them, then stores their difference; element [3], when it is a number,
// overwrites that difference. The stores are 007D38C3..007D38EF for Gears.
struct PlanePartAnimOffsets {
    static constexpr std::uint32_t kStride = 0x10;
    static constexpr std::uint32_t kFirst = 0x00;     // element [1]
    static constexpr std::uint32_t kSecond = 0x04;    // element [2]
    static constexpr std::uint32_t kDuration = 0x08;  // |[1] - [2]|, or element [3]
    static constexpr std::uint32_t kPresent = 0x0C;   // a dword whose low byte is 1
};

// The two tuning floats 0042E740's singleton supplies. That object is 6D0h bytes
// built by 007E2A20 from Scripts\global\luaMW_init.lua; only the offsets this
// reader touches are named, and their meaning is not established.
struct PlaneClassTuningOffsets {
    static constexpr std::uint32_t kAccelFactor = 0x31C;      // 007D20F8
    static constexpr std::uint32_t kAccelMultiplier = 0x320;  // 007D2115
    static constexpr std::uint32_t kTravelSpeedFactor = 0x334;  // 007D23F5
};

// The two IsKindOf codes the GearsPullTime default tests, from the PUSH operands
// at 007D3767 and 007D377E. docs/VEHICLE_CLASS_DESCRIPTORS.md maps them to the
// LevelBomber and LargeReconPlane leaves.
struct PlaneClassKindCodes {
    static constexpr std::int32_t kLevelBomber = 0x10;
    static constexpr std::int32_t kLargeReconPlane = 0x16;
};

// ---------------------------------------------------------------------------
// Conversion rules, as pure functions over the existing Lua value view
// ---------------------------------------------------------------------------

// 007D20F3..007D213C. When the tuning factor is greater than 1.0 the slot is
// multiplied by the product of the two tuning floats; otherwise the reader
// clamps the tuning factor itself up to 1.0 and leaves Accel alone. The clamp is
// a write back into the shared tuning object, so it is returned rather than
// applied here.
struct PlaneClassAccelResult {
    float accel{0.0f};
    float clamped_accel_factor{0.0f};  // what the reader stores back at +31Ch
    bool clamped{false};
};
PlaneClassAccelResult plane_class_accel_007d20f3(float lua_accel, float accel_factor,
                                                 float accel_multiplier) noexcept;

// 007D3743: ParaReload lands as its reciprocal, and an absent key stores 0.0f
// rather than an infinity. A shipped 0 does produce an infinity.
float plane_class_para_reload_rate_007d3743(bool present, float lua_para_reload) noexcept;

// 007D375A..007D3790: the GearsPullTime default is 2.0f, or 4.0f when the
// descriptor's IsKindOf accepts LevelBomber or LargeReconPlane.
float plane_class_gears_pull_default_007d375a(bool is_level_bomber,
                                              bool is_large_recon_plane) noexcept;

// 007D3883..007D38B5: the third slot of a PartAnims record is |[1] - [2]|. The
// listing negates through a -0.0f subtraction, which maps a shipped -0.0f to
// +0.0f and leaves a NaN a NaN.
float plane_class_part_anim_duration_007d3883(float first, float second) noexcept;

// 007D3DA4: a TurboStrength that is not strictly greater than 1.0f zeroes
// TurboTime, so the 1.0f default disables turbo on its own.
bool plane_class_turbo_enabled_007d3da4(float turbo_strength) noexcept;

// ---------------------------------------------------------------------------
// Reconstructed values
// ---------------------------------------------------------------------------

struct PlanePartAnimFields {
    bool present{false};
    float first{0.0f};
    float second{0.0f};
    float duration{0.0f};
};

struct PlaneClassFields {
    std::string short_name;
    std::int32_t num_engines{0};
    bool jet_engines{false};

    float accel{0.0f};
    float kamera_mogotte{10.0f};
    float kamera_folotte{2.0f};
    float y_drag{0.0f};
    float x_drag{0.0f};
    float ext_rot_accel{0.0f};
    float stall_rot_accel{0.0f};
    float water_rot_accel{0.0f};
    float air_brake_drag{0.0f};
    float wheel_brake{0.0f};
    float bomb_control_limit{0.0f};
    float bomb_delay{0.0f};

    float stall_spd{0.0f};
    float max_spd{0.0f};
    float travel_speed{0.0f};
    float travel_speed_scaled{0.0f};
    float swim_height{0.0f};
    float min_water_spd{0.0f};
    float max_water_spd{0.0f};
    float water_decel{0.0f};
    float water_un_spring{0.0f};

    float roll_spd{0.0f};
    float pitch_spd{0.0f};
    float yaw_spd{0.0f};
    float yaw_roll_ratio{0.0f};
    float slide_ratio{0.0f};
    float roll_accel{0.0f};
    float pitch_accel{0.0f};
    float yaw_accel{0.0f};
    float turn_roll_spd{0.0f};
    float yaw_limit_angle{0.0f};
    float pitch_limit_angle{0.0f};
    float drag_pitch_ratio{0.0f};
    float negative_pitch_ratio{0.0f};

    float turn_roll{0.0f};
    float turn_roll_leader{0.0f};
    float turn_circle_radius{0.0f};
    float roll_maxforce_limit{0.0f};
    float pitch_maxforce_limit{0.0f};

    // The gate at 007D2A4B. When it fails the three fields below keep whatever
    // the constructor left, which this reconstruction models as "not written".
    bool ground_data_present{false};
    float wheel_height{0.0f};
    float ground_pitch{0.0f};
    float water_pitch{0.0f};
    float glide_rate{0.0f};

    bool carrier_based{false};
    float drop_angle{0.0f};

    std::int32_t kamikaze_bullet_class_id{0};
    std::int32_t kamikaze_bullet_class{0};  // 0 when 006EA910 did not resolve

    std::int32_t wreck_class{0};  // 0 when the key is absent

    // An effect key is read only when it is not nil; the slot otherwise keeps
    // whatever the constructor left, which these flags record.
    bool has_explosion_efx{false};
    std::int32_t explosion_efx{0};
    bool has_shells_efx{false};
    std::int32_t shells_efx{0};
    bool has_engine_fire_efx{false};
    std::int32_t engine_fire_efx{0};
    bool has_wing_tip_efx{false};
    std::int32_t wing_tip_efx{0};
    // Resolved from the "LowPlaneAlt" literal, not from a key.
    std::int32_t low_plane_alt_efx{0};

    std::vector<std::int32_t> engine_efxes;      // one 44h-byte record each
    std::vector<std::int32_t> damage_smoke_efx;  // handle vector at +224h
    std::vector<std::int32_t> bow_waves;         // one 10h-byte record each

    float bay_door_open_angle{-0.6981317f};
    float bay_door_closed_angle{0.6981317f};
    float bay_door_time_to_open{0.1f};

    float para_reload_rate{0.0f};

    PlanePartAnimFields part_anim_gears;
    PlanePartAnimFields part_anim_wings;
    PlanePartAnimFields part_anim_bay_door;

    float gears_pull_time{2.0f};
    float turbo_time{0.0f};
    float turbo_recharging_time{0.0f};
    float turbo_strength{1.0f};
    float turbo_control_limit{1.0f};

    // What the reader wrote back into the shared tuning object at +31Ch, and
    // whether it did. 007D212F only fires when the factor was not above 1.0f.
    bool clamped_accel_factor{false};
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site 007D1F70 makes that is not a Lua wrapper the
// base reader already covers. The Lua half is not repeated: the reader sequence
// takes a VehicleClassFieldHost for that and this host for the rest.
struct PlaneClassFieldHost {
    virtual ~PlaneClassFieldHost() = default;

    // 0042E740, the luaMW_init tuning singleton. The reader reads +31Ch, +320h
    // and +334h, and writes +31Ch back when the clamp at 007D213C fires.
    virtual float tuning_float(std::uint32_t offset) = 0;
    virtual void set_tuning_float(std::uint32_t offset, float value) = 0;

    // The descriptor's own vtable slot +18h, IsKindOf(kind), called at 007D3773
    // and 007D3782 with the two codes in PlaneClassKindCodes.
    virtual bool is_kind_of(std::int32_t kind) = 0;

    // 00870CD0, the refcounted effect handle around an id.
    virtual std::int32_t make_effect_handle(std::int32_t effect_id) = 0;
    // 00871BA0 then 00871B50, the effect looked up by name. The reader calls it
    // once, with the "LowPlaneAlt" literal built at 007D315C.
    virtual std::int32_t find_effect_by_name(const char* name) = 0;

    // 006EA910, which resolves an index into the Lua global Bullets table and
    // hands back a refcounted class pointer. 0 when the id does not resolve.
    virtual std::int32_t resolve_bullet_class(std::int32_t bullet_id) = 0;
    // 004B3D20, the same shape against the Lua global WreckClass table, keyed by
    // name. 0 when the name does not resolve.
    virtual std::int32_t resolve_wreck_class(const char* name) = 0;
};

// 007D1F70, __thiscall(descriptor, LuaObject* row), RET 4 at 007D3E4F. The row
// arrives on the stack (MOV EDI,[ESP+16Ch] at 007D1F8F) and the descriptor in
// ECX (MOV ESI,ECX at 007D1F99); the row is the receiver of every
// BSP_LuaObject_GetByName the body makes. The body forwards the row to
// 00960230 at 007D1F9F before its own first key, which this function does not
// repeat.
//
// This reproduces the walk in body order and reports what it read. It does not
// reproduce the refcount traffic, the descriptor's raw layout or the SEH states.
void read_plane_class_fields_007d1f70(VehicleClassFieldHost& lua,
                                      PlaneClassFieldHost& host,
                                      const GuiLuaRef& row,
                                      PlaneClassFields& out);

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

// Counts from the shipped Scripts/datatables/autoload/vehicleclasses.lua, parsed
// read-only by an independent Lua-subset parser. A plane row is one whose Type
// is ReconPlane, SmallReconPlane, LargeReconPlane, Fighter, DiveBomber,
// TorpedoBomber, Kamikaze or LevelBomber; that set is the one
// docs/VEHICLE_CLASS_DESCRIPTORS.md ties to the eight plane constructors, and
// the 72 rows it selects match that table's per-leaf row counts exactly.
inline constexpr int kPlaneClassShippedRows = 633;
inline constexpr int kPlaneClassShippedPlaneRows = 72;
inline constexpr int kPlaneClassShippedPlaneKeyPaths = 165;

// The shipped-row count for one schema path, in schema order.
struct PlaneClassKeyCount {
    const char* path;
    std::int32_t shipped_plane_rows;
};

extern const PlaneClassKeyCount kPlaneClassKeyCounts[];
std::size_t plane_class_key_count_count() noexcept;

// Schema paths no shipped plane row provides.
extern const char* const kPlaneClassUnprovidedPaths[];
std::size_t plane_class_unprovided_path_count() noexcept;

// Shipped plane keys that neither this reader nor the base or shared reader
// consumes by name. The whole list is in reports/plane_class_fields.json.
extern const PlaneClassKeyCount kPlaneClassUnconsumedPaths[];
std::size_t plane_class_unconsumed_path_count() noexcept;

}  // namespace bsp

#endif  // BSP_PLANE_CLASS_FIELDS_HPP
