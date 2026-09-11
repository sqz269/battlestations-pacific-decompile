// Vehicle class Lua load: the parent-most base, the settings tuning block and
// the eight per-ship-class leaf overrides of descriptor vtable slot +8h.
//
// Addresses: 0087C640 (the parent-most base constructor, so descriptor +0h..+6Bh
// is finally attributed), 00877FA0 (its tree head node allocator), 00837DE0 (the
// settings tuning block selector) and the eight leaf overrides of slot +8h:
// 006E00F0 MBattleship, 006EB4A0 MCargo, 006FB550 MCruiser, 006FE6A0 MDestroyer,
// 0074C630 MLandingShip, 00759590 MMothership, 00854230 MSubmarine and
// 00857F40 MTorpedoBoat. 00424C40 (the gameplay settings singleton), 00870CD0,
// 00964790, 004B1400, 0047B3C0, 00425850, 00B646E0, 00B64640, 00413920 and
// 004134F0 are read as contracts, not reconstructed.
//
// The two readers this layer sits on are already reconstructed elsewhere and
// nothing of theirs is repeated here: 00960230, the descriptor base reader, is
// bsp/vehicle_class_fields.hpp, and 00831840, the ship family reader every leaf
// below chains to first, is bsp/ship_class_fields.hpp.
//
// Evidence: the Ghidra listings of each routine. Every address in the tables
// below is a call site or a store the listing makes. The eight class names are
// recovered data, the literals that follow each leaf vtable (MDestroyer at
// 00D1AD28 after vtable 00D1ACF8, and the seven siblings at the same +30h);
// every other descriptive name here is a hypothesis, not a recovered symbol.
// See docs/VEHICLE_CLASS_LUA_LOAD.md and reports/vehicle_class_lua_load.json.
//
// State: analysed and reconstructed, build-tested and installed-file-checked
// against the shipped Scripts/datatables/autoload/vehicleclasses.lua. Not a
// binary-compatible replacement and not validated against the running game.

#ifndef BSP_VEHICLE_CLASS_LUA_LOAD_HPP
#define BSP_VEHICLE_CLASS_LUA_LOAD_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/vehicle_class_fields.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 0087C640, the parent-most base
// ---------------------------------------------------------------------------

// __fastcall(void* this), returns this, SEH handler 00C9686F. It is the base
// BSP_VehicleClass_ConstructBase 00749050 calls, and it is the class whose
// vtable 00D0E13C carries the shared damageable pre-reader 0087CA80 at slot
// +8h, which is why 00960230 calls 0087CA80 before its own first key.
//
// It writes its own grandparent's vptr 00CEB130 first (00887C65A) and its own
// 00D0E13C second (0087C66E), so its grandparent's constructor is inlined and
// the refcount at +4h belongs to that grandparent.
//
// Grouped in a struct so the member names cannot collide with another header's.
struct DamageableClassBaseOffsets {
    static constexpr std::uint32_t kVptr = 0x00;          // 0087C65A / 0087C66E
    static constexpr std::uint32_t kRefCount = 0x04;      // = 1 at 0087C665
    static constexpr std::uint32_t kExplosionType = 0x40; // = -1 at 0087C699
    static constexpr std::uint32_t kFlagByte = 0x44;      // = 0 at 0087C6A0
    static constexpr std::uint32_t kSectionTree = 0x60;   // the 0Ch tree at 0087C6AC
    static constexpr std::uint32_t kSectionTreeHead = 0x64;
    static constexpr std::uint32_t kSectionTreeSize = 0x68;
    static constexpr std::uint32_t kSecondaryVptr = 0x6C; // written by 00749050, not here
};

// The dwords 0087C640 clears to zero, in store order. Listed so a reader can
// tell an initialised slot from one that is only zero because the factory
// memsets the allocation.
extern const std::uint32_t kDamageableClassBaseZeroedOffsets[];
std::size_t damageable_class_base_zeroed_count() noexcept;

// The offsets inside +0h..+6Bh that 0087C640 leaves alone. Four of the six are
// exactly the scalars the Lua pre-reader 0087CA80 fills (+34h Unique, +48h HP,
// +4Ch Armour, +54h Name); +8h and +18h stay unattributed.
extern const std::uint32_t kDamageableClassBaseUntouchedOffsets[];
std::size_t damageable_class_base_untouched_count() noexcept;

// The MSVC red-black tree head node 00877FA0 hands back, which 0087C640 then
// closes into a one-node ring. Node size 58h.
struct DamageableSectionTreeNodeOffsets {
    static constexpr std::uint32_t kLeft = 0x00;   // = node at 0087C6CF
    static constexpr std::uint32_t kParent = 0x04; // = node at 0087C6C9
    static constexpr std::uint32_t kRight = 0x08;  // = node at 0087C6D4
    static constexpr std::uint32_t kValue = 0x0C;  // 48h bytes, never touched here
    static constexpr std::uint32_t kColor = 0x54;  // = 1 by 00877FA0
    static constexpr std::uint32_t kIsNil = 0x55;  // = 1 at 0087C6C2
    static constexpr std::uint32_t kNodeSize = 0x58;
};

// ---------------------------------------------------------------------------
// 00837DE0, the settings tuning block selector
// ---------------------------------------------------------------------------

// __fastcall(settings) with no stack arguments, RET 0, no callees. It returns
// settings + 0F0h when the game object's session mode dword at
// (*00E188A8) + 1FE4h is non-zero and settings + 080h otherwise (00837DE5,
// 00837DEC, 00837DF4). Every one of its nine call sites re-fetches the settings
// singleton from 00424C40 immediately before calling it.
struct ShipTuningBlockSelector {
    static constexpr std::uint32_t kSessionModeOffset = 0x1FE4; // in the game object
    static constexpr std::uint32_t kBlockWhenModeZero = 0x080;  // in the settings object
    static constexpr std::uint32_t kBlockOtherwise = 0x0F0;
    static constexpr std::uint32_t kBlockStride = 0x070; // the two blocks are 70h apart
};

// The pure rule 00837DE0 implements.
std::uint32_t ship_tuning_block_offset(std::int32_t session_mode) noexcept;

// ---------------------------------------------------------------------------
// The eight ship leaf classes
// ---------------------------------------------------------------------------

// Recovered from the literal that follows each leaf vtable. The vtable address
// is the one whose slot +8h holds the override.
enum class ShipLeafClass : std::uint8_t {
    Battleship,  // 006E00F0, vtable 00D1ADF8, "MBattleship"
    Cargo,       // 006EB4A0, vtable 00D1ADBC, "MCargo"
    Cruiser,     // 006FB550, vtable 00D1AD38, "MCruiser"
    Destroyer,   // 006FE6A0, vtable 00D1ACF8, "MDestroyer"
    LandingShip, // 0074C630, vtable 00D1AD78, "MLandingShip"
    Mothership,  // 00759590, vtable 00D1AEBC, "MMothership"
    Submarine,   // 00854230, vtable 00D1AE38, "MSubmarine"
    TorpedoBoat, // 00857F40, vtable 00D1AE78, "MTorpedoBoat"
};

struct ShipLeafClassInfo {
    ShipLeafClass leaf;
    const char* class_name;   // the recovered literal
    const char* lua_type;     // the row Type that selects it, from key co-occurrence
    std::uint32_t reader;     // the slot +8h override
    std::uint32_t vtable;
    std::uint32_t name_literal; // where the class-name literal sits
    std::int32_t shipped_rows;  // rows of that Type in the shipped file
};

extern const ShipLeafClassInfo kShipLeafClasses[];
std::size_t ship_leaf_class_count() noexcept;

// ---------------------------------------------------------------------------
// The four-slot tuning array every leaf fills
// ---------------------------------------------------------------------------

// Every one of the eight overrides ends by copying two dwords out of the block
// 00837DE0 selected. Seven of them write one value into all four of
// +560h..+56Ch; only MSubmarine writes four different ones, which is why the
// slots are read here as an array of four rather than as one scalar.
struct ShipLeafTuningOffsets {
    static constexpr std::uint32_t kTuningArray = 0x560; // four dwords, +560h..+56Ch
    static constexpr std::uint32_t kTuningArrayCount = 4;
    static constexpr std::uint32_t kTuningScalar = 0x570;
};

// Where each leaf reads its pair from inside the 70h tuning block. Two leaves
// choose between two pairs on a boolean the same override just read.
struct ShipLeafTuningSource {
    ShipLeafClass leaf;
    const char* variant;       // "" when the leaf has only one pair
    std::uint32_t array_source; // block offset feeding +560h..+56Ch
    std::uint32_t scalar_source; // block offset feeding +570h
    std::uint32_t site;        // the first store the leaf makes into the array
};

extern const ShipLeafTuningSource kShipLeafTuningSources[];
std::size_t ship_leaf_tuning_source_count() noexcept;

// MSubmarine's four array slots come from four different block offsets.
extern const std::uint32_t kSubmarineTuningArraySources[]; // 60h, 64h, 68h, 6Ch
inline constexpr std::uint32_t kSubmarineTuningScalarSource = 0x5C;

// ---------------------------------------------------------------------------
// Leaf key schema as data
// ---------------------------------------------------------------------------

// How a leaf override turns the Lua value into the bytes it stores. These are
// the forms the eight leaves use; the richer set the base and ship readers need
// is in vehicle_class_fields.hpp and ship_class_fields.hpp.
enum class ShipLeafConversion : std::uint8_t {
    Number,          // 00B66270, written unconditionally
    NumberOr,        // 00B66330 with a literal float default
    IntegerOr,       // 00B66380 with a literal integer default
    IntegerOrZero,   // 00B66290 after an IsNil test, zero when absent
    BooleanOr,       // 00B662F0 with a literal boolean default
    EffectHandle,    // 00B66290 after IsInteger, then 00870CD0
    StringEquals,    // 00B662B0 then 00425850 against a literal, stored as a byte
    Vector3,         // 00B67A80 into three consecutive floats
    NegatedAngle,    // 00B66270 then FCHS, consumed by the camera basis
    NumberOrPrior,   // 00B66330, but only read when a prior slot stayed negative
};

// kShipLeafNoOffset marks a key that only opens a sub-table or feeds a rule
// rather than one descriptor slot.
inline constexpr std::uint32_t kShipLeafNoOffset = 0xFFFFFFFFu;

struct ShipLeafFieldSpec {
    ShipLeafClass leaf;
    const char* path;          // the dotted key path
    std::uint32_t key_string;  // where the key literal sits
    ShipLeafConversion conversion;
    std::uint32_t offset;      // descriptor offset, or kShipLeafNoOffset
    std::uint32_t site;        // the BSP_LuaObject_GetByName call site
    std::uint32_t store;       // the instruction that writes the slot
    const char* fallback;      // the default, in the form the listing shows it
    const char* note;
};

// The 32 key paths the eight leaves read, in per-leaf body order.
extern const ShipLeafFieldSpec kShipLeafFieldSchema[];
std::size_t ship_leaf_field_count() noexcept;

// A linear search over the schema; nullptr when the leaf does not read the path.
const ShipLeafFieldSpec* ship_leaf_find_field(ShipLeafClass leaf,
                                              const char* path) noexcept;

// ---------------------------------------------------------------------------
// Per-leaf descriptor layout past the ship base
// ---------------------------------------------------------------------------

// The ship base 00963380 is 0804h bytes, so every offset below is the leaf's own
// storage. The same offset means different things in different leaves; nothing
// here may be read without its ShipLeafClass.
struct ShipLeafDescriptorOffsets {
    // MBattleship, MCruiser: one boolean.
    static constexpr std::uint32_t kBattleshipIsBattlecruiser = 0x808;
    static constexpr std::uint32_t kCruiserIsHeavyCruiser = 0x808;

    // MCargo: two SubType booleans.
    static constexpr std::uint32_t kCargoIsJunk = 0x808;
    static constexpr std::uint32_t kCargoIsTroopTransport = 0x809;

    // MLandingShip.
    static constexpr std::uint32_t kLandingShipIsBig = 0x808;
    static constexpr std::uint32_t kLandingShipIsRocketer = 0x809;
    static constexpr std::uint32_t kLandingShipLandedDamage = 0x80C;
    static constexpr std::uint32_t kLandingShipLandedCapturePower = 0x810;
    static constexpr std::uint32_t kLandingShipTroopType = 0x814;
    static constexpr std::uint32_t kLandingShipSubTypeLcvp = 0x818;
    static constexpr std::uint32_t kLandingShipSubTypeDefault = 0x819;
    static constexpr std::uint32_t kLandingShipSubTypeLsm = 0x81A;

    // MMothership.
    static constexpr std::uint32_t kMothershipRunwayWidth = 0x820;
    static constexpr std::uint32_t kMothershipRunwayLength = 0x824;
    static constexpr std::uint32_t kMothershipMaxLandingPlanes = 0x828;
    static constexpr std::uint32_t kMothershipIsCarrierEscort = 0x82C;
    static constexpr std::uint32_t kMothershipDeckCameraBasis = 0x830;  // 30h bytes
    static constexpr std::uint32_t kMothershipDeckCameraPosition = 0x860; // three floats

    // MSubmarine.
    static constexpr std::uint32_t kSubmarinePeriscopeWave = 0x80C;
    static constexpr std::uint32_t kSubmarinePeriscopeDepth = 0x810;
    static constexpr std::uint32_t kSubmarineSwimDepth2 = 0x814;
    static constexpr std::uint32_t kSubmarineSwimDepth3 = 0x818;
    static constexpr std::uint32_t kSubmarinePeriscopeMoveRange = 0x81C;
    static constexpr std::uint32_t kSubmarineUpDownAccel = 0x824;
    static constexpr std::uint32_t kSubmarineUpDownRotation = 0x828;
    static constexpr std::uint32_t kSubmarineUpDownStopTime = 0x82C;
    static constexpr std::uint32_t kSubmarineUpSpeed = 0x830;
    static constexpr std::uint32_t kSubmarineDownSpeed = 0x834;
    static constexpr std::uint32_t kSubmarineAirRunOutTime = 0x838;
    static constexpr std::uint32_t kSubmarineAirReloadTime = 0x83C;

    // MTorpedoBoat.
    static constexpr std::uint32_t kTorpedoBoatTurboTime = 0x808;
    static constexpr std::uint32_t kTorpedoBoatTurboStrength = 0x80C;
    static constexpr std::uint32_t kTorpedoBoatTurboRechargingTime = 0x810;

    // MDestroyer reads no key of its own; it writes only the tuning slots.
};

// ---------------------------------------------------------------------------
// The literal defaults, as floats
// ---------------------------------------------------------------------------

// Each is the dword the listing loads before the GetFloatOrDefault call.
struct ShipLeafDefaults {
    static constexpr float kAbsentDepth = -1.0F;        // 00D7A260
    static constexpr float kPeriscopeMoveRange = 10.0F; // 00CE38B8
    static constexpr float kUpDownStopTime = 5.0F;      // 00CE3850
    static constexpr float kAirReloadTime = 5.0F;       // 00CE3850
    static constexpr float kVerticalSpeed = 1.2F;       // 00CE3814
    static constexpr float kUpDownAccel = 0.25F;        // 00CE3868
    static constexpr float kUpDownRotation = 0.034906585F; // 00D0C26C, two degrees
    static constexpr float kAirRunOutTime = 120.0F;     // 00D05804
    static constexpr float kTurboTime = 10.0F;          // 00CE38B8
    static constexpr float kTurboStrength = 3.0F;       // 00CE3854
    static constexpr float kTurboRechargingTime = 10.0F; // 00CE38B8
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// MSubmarine at 00854332: COMISS against zero then JBE. SwimDepth1 is read only
// when PeriscopeDepth left the slot negative, so it is the legacy alias for the
// same slot rather than a second field.
bool submarine_reads_swim_depth1(float periscope_depth_slot) noexcept;

// MLandingShip at 0074CAE1..0074CBCE. Exactly one of the three bytes is set;
// a SubType that is absent, not a string, or neither literal takes the default.
struct LandingShipSubType {
    bool lcvp = false;
    bool lsm = false;
    bool other = true;
};
LandingShipSubType landing_ship_sub_type(const char* sub_type) noexcept;

// MCargo at 006EB50E..006EB571. Both bytes are cleared first, so a SubType that
// is not a string leaves both false.
struct CargoSubType {
    bool junk = false;
    bool troop_transport = false;
};
CargoSubType cargo_sub_type(const char* sub_type) noexcept;

// MMothership at 0075979B..00759806. Both angles are negated before the basis is
// built; the horizontal one drives 00B646E0 and the vertical one 00B64640, and
// 00413920 multiplies them in that order.
struct DeckCameraAngles {
    float horizontal_radians = 0.0F;
    float vertical_radians = 0.0F;
};
DeckCameraAngles deck_camera_angles(float horz_angle, float vert_angle) noexcept;

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site the leaves make that is not a Lua wrapper
// bsp::VehicleClassFieldHost already covers. The Lua half is not repeated.
struct ShipLeafFieldHost {
    virtual ~ShipLeafFieldHost() = default;

    // 00424C40, the lazily built 76Ch gameplay settings singleton at 00F8753C,
    // then 00837DE0 on it. Returns the dword at block + offset. Every leaf
    // re-fetches the singleton before each of its two or five reads.
    virtual std::uint32_t tuning_block_dword(std::uint32_t block_offset) = 0;

    // (*00E188A8) + 1FE4h, the session mode 00837DE0 branches on.
    virtual std::int32_t session_mode() = 0;

    // 00870CD0, the refcounted effect handle around an id. MSubmarine only calls
    // it after PeriscopeWave passed IsInteger at 00854272.
    virtual std::int32_t make_effect_handle(std::int32_t effect_id) = 0;

    // MLandingShip's LandingTroopClasses walk, 0074C796..0074CA53. The globals
    // table's LandingTroopClasses[LandingTroopType].Classes is iterated and each
    // element's Type decides the branch: "Soldier" resolves ClassName through
    // 004B1400 and "Vehicle" resolves ClassId through 00964790 then 0047B3C0.
    // Neither result reaches a descriptor slot this packet decoded.
    virtual void register_landing_troop_soldier(const char* class_name) = 0;
    virtual void register_landing_troop_vehicle(std::int32_t class_id) = 0;

    // 00B67980 then GetByName "LandingTroopClasses" and GetByIndex: the globals
    // table the walk above starts from, which is not the row. Returns the
    // Classes table for the given LandingTroopType, or an invalid ref.
    virtual GuiLuaRef landing_troop_classes(std::int32_t troop_type) = 0;

    // 00B646E0, 00B64640, 00413920 and 004134F0: the two axis rotations and
    // their product, copied into descriptor +830h. The 30h bytes are opaque here.
    virtual void store_deck_camera_basis(const DeckCameraAngles& angles) = 0;
};

// ---------------------------------------------------------------------------
// What a leaf establishes
// ---------------------------------------------------------------------------

struct ShipLeafTuning {
    std::uint32_t array[4] = {0, 0, 0, 0}; // +560h..+56Ch
    std::uint32_t scalar = 0;              // +570h
    std::uint32_t array_source = 0;        // the block offset it came from
    std::uint32_t scalar_source = 0;
};

struct ShipLeafFields {
    ShipLeafClass leaf = ShipLeafClass::Destroyer;
    ShipLeafTuning tuning;

    // MBattleship, MCruiser.
    bool is_battlecruiser = false;
    bool is_heavy_cruiser = false;

    // MCargo.
    CargoSubType cargo;

    // MLandingShip.
    bool landing_ship_is_big = false;
    bool landing_ship_is_rocketer = false;
    std::int32_t landed_damage = 0;
    std::int32_t landed_capture_power = 0;
    std::int32_t landing_troop_type = 1;
    LandingShipSubType landing_ship_sub_type;

    // MMothership.
    float runway_width = 0.0F;
    float runway_length = 0.0F;
    std::int32_t max_landing_planes = 0;
    bool is_carrier_escort = false;
    DeckCameraAngles deck_camera;
    float deck_camera_position[3] = {0.0F, 0.0F, 0.0F};

    // MSubmarine.
    std::int32_t periscope_wave_handle = 0;
    float periscope_depth = ShipLeafDefaults::kAbsentDepth;
    float swim_depth2 = ShipLeafDefaults::kAbsentDepth;
    float swim_depth3 = ShipLeafDefaults::kAbsentDepth;
    float periscope_move_range = ShipLeafDefaults::kPeriscopeMoveRange;
    float up_down_accel = ShipLeafDefaults::kUpDownAccel;
    float up_down_rotation = ShipLeafDefaults::kUpDownRotation;
    float up_down_stop_time = ShipLeafDefaults::kUpDownStopTime;
    float up_speed = ShipLeafDefaults::kVerticalSpeed;
    float down_speed = ShipLeafDefaults::kVerticalSpeed;
    float air_run_out_time = ShipLeafDefaults::kAirRunOutTime;
    float air_reload_time = ShipLeafDefaults::kAirReloadTime;

    // MTorpedoBoat.
    float turbo_time = ShipLeafDefaults::kTurboTime;
    float turbo_strength = ShipLeafDefaults::kTurboStrength;
    float turbo_recharging_time = ShipLeafDefaults::kTurboRechargingTime;
};

// The eight overrides, each __thiscall(descriptor, LuaObject* row) with RET 4,
// each chaining to 00831840 before its own first key. The chain is not repeated
// here: call read_ship_class_fields_00831840 first, then the leaf.
//
// None of these reproduces the refcount traffic or the SEH states.
void read_battleship_class_fields_006e00f0(VehicleClassFieldHost& lua,
                                           ShipLeafFieldHost& host,
                                           const GuiLuaRef& row,
                                           ShipLeafFields& out);
void read_cargo_class_fields_006eb4a0(VehicleClassFieldHost& lua,
                                      ShipLeafFieldHost& host,
                                      const GuiLuaRef& row,
                                      ShipLeafFields& out);
void read_cruiser_class_fields_006fb550(VehicleClassFieldHost& lua,
                                        ShipLeafFieldHost& host,
                                        const GuiLuaRef& row,
                                        ShipLeafFields& out);
void read_destroyer_class_fields_006fe6a0(ShipLeafFieldHost& host,
                                          ShipLeafFields& out);
void read_landing_ship_class_fields_0074c630(VehicleClassFieldHost& lua,
                                             ShipLeafFieldHost& host,
                                             const GuiLuaRef& row,
                                             ShipLeafFields& out);
void read_mothership_class_fields_00759590(VehicleClassFieldHost& lua,
                                           ShipLeafFieldHost& host,
                                           const GuiLuaRef& row,
                                           ShipLeafFields& out);
void read_submarine_class_fields_00854230(VehicleClassFieldHost& lua,
                                          ShipLeafFieldHost& host,
                                          const GuiLuaRef& row,
                                          ShipLeafFields& out);
void read_torpedo_boat_class_fields_00857f40(VehicleClassFieldHost& lua,
                                             ShipLeafFieldHost& host,
                                             const GuiLuaRef& row,
                                             ShipLeafFields& out);

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

// Counts from the shipped Scripts/datatables/autoload/vehicleclasses.lua, parsed
// read-only by local/leafkeys.py in the worktree. Rows and top-level key counts
// agree with the two earlier packets' independent parsers.
inline constexpr int kShipLeafShippedRows = 633;
inline constexpr int kShipLeafShippedTopLevelKeys = 197;
inline constexpr int kShipLeafRowLevelKeyPaths = 29; // DeckCamera counted once

struct ShipLeafKeyCount {
    const char* path;
    std::int32_t shipped_rows;
};

extern const ShipLeafKeyCount kShipLeafKeyCounts[];
std::size_t ship_leaf_key_count_count() noexcept;

// Leaf keys the readers ask for that no shipped row provides: these always take
// the literal default above.
extern const char* const kShipLeafUnprovidedKeys[];
std::size_t ship_leaf_unprovided_key_count() noexcept;

}  // namespace bsp

#endif  // BSP_VEHICLE_CLASS_LUA_LOAD_HPP
