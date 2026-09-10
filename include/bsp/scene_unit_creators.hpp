#pragma once
// The shared creation path behind the ten `Type`-keyed scene classes.
// Addresses: 004f0520, 004f05f0, 004f06c0, 004f0790, 004f0860, 004f0930,
//            004f0a00, 004f0ad0, 004f0fb0, 004f10b0, 004e96d0, 004e98e0,
//            004e98f0, 004e9900, 004e9910, 004e6bc0, 004e6b30, 004f03c0,
//            00964790, 0095ba60, 00470b80, 006fe590.
//
// Every name here is a hypothesis, not a recovered symbol. The one exception is
// `MDestroyer`, which the image itself carries at 00D1AD28, right behind the
// class descriptor's vtable. docs/SCENE_UNIT_CREATORS.md holds the evidence,
// including the listing the calling convention was taken from: Ghidra drops the
// stack arguments of all ten creators because none of them is prototyped.
//
// Two objects, not one. 00964790 returns a *vehicle-class descriptor*, one per
// row of the installed `VehicleClass` Lua table, cached by class index. The
// descriptor's vtable slot +28h then allocates the *unit instance*. The scene
// creator only ever holds the second one.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/scene_file.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The ten scene classes (004F2800's `via unit` rows)
// ---------------------------------------------------------------------------

struct SceneUnitCreatorRow {
    int class_id;                       // descriptor +0
    const char* scene_class;            // the 004F2800 registration key
    std::uint32_t create_address;       // descriptor +4, the instantiate pass
    std::uint32_t register_address;     // descriptor +8, the registration pass
    // The size the creator allocates itself, or 0 when the instance comes from
    // descriptor->vtable[28h]. Only PlaneSquadronGen has one.
    std::uint32_t own_instance_size;
    bool applies_command_property;      // calls 004E6B30
    const char* extra_key;              // the second bag key it reads, or nullptr
    std::uint32_t authored_entities;    // .scn count from docs/SCENE_ENTITY_FACTORY.md
};

inline constexpr int kSceneUnitCreatorCount = 10;
const SceneUnitCreatorRow* scene_unit_creator_table() noexcept;
const SceneUnitCreatorRow* find_scene_unit_creator(int class_id) noexcept;
const SceneUnitCreatorRow* find_scene_unit_creator(const std::string& scene_class) noexcept;

// ---------------------------------------------------------------------------
// The 22 `VehicleClass.Type` literals (00964790's comparison chain)
// ---------------------------------------------------------------------------

// Descriptor sizes are the `MOV ECX,<size> ; CALL 00470B80` argument, except the
// first three, which the compiler inlined as `PUSH <size> ; CALL 00BF55BE`.
// 00470B80 is operator new plus a memset 0 of the same size.
struct VehicleClassKindRow {
    const char* type_name;              // the literal, compared with 00425850
    std::uint32_t descriptor_size;      // bytes allocated and zeroed
    std::uint32_t constructor_address;  // run on the zeroed block
    int seh_state;                      // 0Ah for Destroyer .. 1Fh for DummyTargetVehicle
    // Rows carrying this Type in Scripts/datatables/autoload/vehicleclasses.lua
    // as shipped. `ReconPlane` has a branch and no row.
    int installed_rows;
};

inline constexpr int kVehicleClassKindCount = 22;
const VehicleClassKindRow* vehicle_class_kind_table() noexcept;
// Comparison is 00425850 in the factory's own order; the first match wins.
const VehicleClassKindRow* find_vehicle_class_kind(const std::string& type_name) noexcept;

// The unit instance MDestroyer's descriptor produces. 006FE590 is the +28h slot:
// operator new(1188h), memset 0, 006FE460(instance, flag), 009553D0 and a second
// back-pointer at +354h.
inline constexpr std::uint32_t kDestroyerDescriptorVtable = 0x00d1acf8;
inline constexpr std::uint32_t kDestroyerInstanceSize = 0x1188;
inline constexpr std::uint32_t kDestroyerInstanceVtable = 0x00cfc3d0;
// Fields of a unit instance this packet established.
inline constexpr std::uint32_t kUnitInstanceNameLengthOffset = 0x154;
inline constexpr std::uint32_t kUnitInstanceNameBufferOffset = 0x158;
inline constexpr std::uint32_t kUnitInstanceClassBackPointerOffset = 0x354;
inline constexpr std::uint32_t kUnitInstanceClassRefOffset = 0x538;
// The vtable slot the creator calls to place the instance, and the two hooks
// 00928860 runs around it.
inline constexpr std::uint32_t kUnitInstancePlaceSlot = 0x98;
inline constexpr std::uint32_t kUnitInstanceEnteredParentSlot = 0x130;
inline constexpr std::uint32_t kUnitInstanceLeavingParentSlot = 0x134;

// ---------------------------------------------------------------------------
// The vehicle-class registry singleton (00437F50)
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kVehicleClassCacheDataOffset = 0x04;
inline constexpr std::uint32_t kVehicleClassCacheCapacityOffset = 0x08;
inline constexpr std::uint32_t kVehicleClassIndexTableOffset = 0x10;
// (4010h - 10h) / 4: the `Type` value is an index into a fixed 4096-entry table.
inline constexpr std::uint32_t kVehicleClassIndexTableEntries = 0x1000;
inline constexpr std::uint32_t kVehicleClassPartyArrayOffset = 0x4010;
inline constexpr int kVehicleClassPartyStride = 3;   // only bytes 0 and 1 are written
inline constexpr int kVehicleClassPartyCount = 2;    // 0095BA60 returns for any other party

// The Lua fields 00964790 and 004E96D0 read out of one `VehicleClass` row.
struct VehicleClassLuaRow {
    std::string type_name;    // VehicleClass[i].Type, a string
    int landing_ship{0};      // VehicleClass[i].LandingShip; non-zero recurses first
    int race{0};              // VehicleClass[i].Race, read only when read_race
    int launched_class{-1};   // VehicleClass[i].Catapult.LaunchedClass, default -1
};

// The three arrays of the singleton, as data. `descriptor_cache` holds opaque
// handles; the reconstruction never dereferences them.
struct VehicleClassRegistry {
    std::vector<int> type_to_class_index;
    std::vector<void*> descriptor_cache;
    std::vector<std::array<unsigned char, kVehicleClassPartyStride>> party_required;
};

// 009647BB..009647C4: classIndex = singleton[10h + typeId*4]. Returns -1 when the
// id is outside the 4096-entry table; the native does not bounds check and would
// read past it.
int vehicle_class_index_from_type(const VehicleClassRegistry& registry, int type_id) noexcept;

// 009647F9: `NEG EAX ; SBB EAX,EAX ; TEST EAX,0F8A0BDh ; JNZ`. That is
// `cached != 0` with a junk immediate; there is no second condition, so a
// descriptor is built at most once per class index.
bool vehicle_class_cache_hit(const VehicleClassRegistry& registry, int class_index) noexcept;

// 0095BA60. Returns false without touching anything when the party is not 0 or 1.
// Otherwise grows the array as 004359E0 does and sets byte[classIndex][party].
bool mark_party_requires_class_0095ba60(VehicleClassRegistry& registry, int type_id,
                                        int party) noexcept;

// ---------------------------------------------------------------------------
// `Type` resolution
// ---------------------------------------------------------------------------

inline constexpr const char* kSceneUnitTypeKey = "Type";       // 00CE4780
inline constexpr const char* kSceneUnitPartyKey = "Party";     // 00CE5804
inline constexpr const char* kSceneUnitStationaryKey = "Stationary";  // 00CEA044
inline constexpr const char* kSceneUnitLaunchClassKey = "LaunchClassID";  // 00CE9270
inline constexpr const char* kSceneUnitCommandKey = "Command";        // 00CE8A08
inline constexpr const char* kSceneUnitCommandTargetKey = "CommandTarget";  // 00CE89F8
inline constexpr const char* kSceneHierarchyFlagKey = "SetHierarchy";     // 00CEA028
inline constexpr const char* kSceneHierarchyParentKey = "HierarchyParent";  // 00CEA018
inline constexpr const char* kSceneHierarchyMatrixKey = "HierarchyMatrix";  // 00CEA008

// `Type = E ShipClasses : PACK3_Pennsylvania ;`. The property descriptor declares
// the type and the reader stores the resolved integer at +0Ch, so the creator sees
// a number. The enum symbol tables are not in any shipped data file, so the
// reconstruction can only hand back the token; the resolved id is an input.
struct SceneEnumProperty {
    bool present{false};
    std::string table;   // `ShipClasses`, `PlaneClasses`, `Races`, ...
    std::string symbol;  // `PACK3_Pennsylvania`
};
SceneEnumProperty scene_enum_property(const ScenePropertyBlock& bag,
                                      const std::string& key) noexcept;

// The Lua path 004E96D0 builds when `LaunchClassID` is absent or negative:
// "VehicleClass." .. tostring(typeId) .. ".Catapult.LaunchedClass".
std::string vehicle_class_launch_lua_path(int type_id);

// ---------------------------------------------------------------------------
// The `Command` sub-block (004E6B30)
// ---------------------------------------------------------------------------

// The native requires the outer `Command` property to carry type code 6, the
// nested-block code, and then reads two keys out of that sub-bag. When the inner
// `Command` key is missing nothing is queued, which is the case for every
// `"Command" { CommandTarget = R "" ; }` block in the shipped files.
struct SceneUnitCommand {
    bool queued{false};
    std::string command;
    std::string target;  // "" (00CE3A0C) when CommandTarget is missing or empty
};
SceneUnitCommand scene_unit_command_004e6b30(const ScenePropertyBlock& bag);

// ---------------------------------------------------------------------------
// The deferred hierarchy write (004F03C0)
// ---------------------------------------------------------------------------

// What 004F03C0 leaves in the property bag. It erases the three keys first, so
// re-running it is idempotent.
struct SceneHierarchyDeferral {
    bool set_hierarchy{true};       // always written as 1
    bool wrote_parent{false};       // only with a non-null hierarchy parent
    std::string parent_name;        // 00926420(parent)+4, or "" at 00E18B5D
    std::array<float, 16> matrix{}; // the 16 floats, pushed with REP MOVSD ECX=10h
};
SceneHierarchyDeferral defer_hierarchy_004f03c0(bool has_parent,
                                                const std::string& parent_name,
                                                const float local_frame[16]) noexcept;
void apply_hierarchy_deferral(ScenePropertyBlock& bag, const SceneHierarchyDeferral& deferral);

// ---------------------------------------------------------------------------
// The registration pass (004E96D0 and 004E6BC0)
// ---------------------------------------------------------------------------

struct SceneUnitRegistrationInputs {
    int type_id{-1};
    int party{-1};
    bool has_launch_class_id{false};  // `LaunchClassID` present, type code 0
    int launch_class_id{-1};
    int lua_launched_class{-1};       // the Catapult fallback, -1 when absent
};

struct SceneUnitRegistrationResult {
    bool marked_type{false};          // 0095BA60(type, party)
    int launch_class{-1};             // the class the catapult launches, -1 for none
    bool marked_launch_class{false};  // 0095BA60(launchClass, party)
    bool registered_hidden_stock{false};  // 0046BE90
};

// 004E96D0, the pass-2 body the four plain ship classes reach through the thunk
// at 004E98E0 and the three base classes reach before their own stock walk.
SceneUnitRegistrationResult register_scene_unit_004e96d0(const SceneUnitRegistrationInputs& inputs,
                                                        VehicleClassRegistry& registry);

// 004E6BC0, the squadron's pass-2 body: no LaunchClassID, no Lua, and the type
// itself is registered as hidden stock.
SceneUnitRegistrationResult register_plane_squadron_004e6bc0(int type_id, int party,
                                                             VehicleClassRegistry& registry);

// ---------------------------------------------------------------------------
// The instantiate pass
// ---------------------------------------------------------------------------

// The parsed entity block as 0046CF40 hands it to the creator at 0046D5A4.
// `class_id` arrives in ECX and is dead in all ten bodies; the fourth stack
// argument is never read either, but RET 10h still pops it.
struct SceneUnitCreationInputs {
    int class_id{0};
    std::string entity_name;                        // EDX, a C string
    void* hierarchy_parent{nullptr};                // stack arg 1, may be null
    const float* local_frame{nullptr};              // stack arg 2, 16 floats
    const ScenePropertyBlock* properties{nullptr};  // stack arg 3
    int type_id{-1};                                // the resolved `Type` value
};

// Everything a creator reaches outside the parsed entity block. One method per
// native call site, in call order. There are no default implementations: nothing
// here stands in for unrecovered game behaviour.
struct SceneUnitCreatorHost {
    virtual ~SceneUnitCreatorHost() = default;

    // 00964790 with ECX = the `Type` value and DL = 1. Null when the row's Type
    // string matches none of the 22 literals.
    virtual void* vehicle_class_descriptor(int type_id, bool read_race) = 0;

    // descriptor->vtable[28h](0). For MDestroyer this is 006FE590.
    virtual void* create_instance_from_descriptor(void* descriptor) = 0;

    // 00748C40(descriptor), the LandFort variant taken when `Stationary` is set.
    virtual void* create_stationary_instance(void* descriptor) = 0;

    // operator new(size) + memset 0 + 007F2C60(instance, 0), the squadron.
    virtual void* create_squadron_instance(std::uint32_t size) = 0;

    // 004C1130()+4, vtable[10h]. Non-zero takes the 004F03C0 branch instead of
    // placing the instance. Which branch a shipped load takes is unproven.
    virtual bool placement_deferred() = 0;

    // *(*(00E188A8)+19CCh), the parent node the placement call actually uses.
    virtual void* world_parent_node() = 0;

    // instance->vtable[98h](hierarchy_parent, world_parent, local_frame).
    virtual void place_instance(void* instance, void* hierarchy_parent, void* world_parent,
                                const float local_frame[16]) = 0;

    // 00926420(parent) then +4, or "" at 00E18B5D. Only reached on the deferred
    // branch, and only with a non-null hierarchy parent.
    virtual std::string hierarchy_parent_name(void* hierarchy_parent) = 0;

    // 0041DD40(instance+154h, length, 0) then memcpy into +158h. A null or empty
    // name resizes to 0 and copies nothing.
    virtual void set_instance_name(void* instance, const std::string& name) = 0;

    // 00469610(*(00E18680), instance, command, target) through 004E6B30.
    virtual void queue_entity_command(void* instance, const std::string& command,
                                      const std::string& target) = 0;
};

struct SceneUnitCreationResult {
    void* instance{nullptr};
    void* descriptor{nullptr};
    bool hierarchy_deferred{false};
    bool used_stationary_variant{false};
    SceneHierarchyDeferral deferral{};
    SceneUnitCommand command{};
};

// 004F0520 in full, and the six creators that are instruction-for-instruction the
// same, plus the two that differ only by a flag on the row: `LandFort` reads
// `Stationary` first and neither `LandFort` nor `CommandBuilding` applies the
// `Command` property.
SceneUnitCreationResult create_scene_unit_004f0520(const SceneUnitCreatorRow& row,
                                                   const SceneUnitCreationInputs& inputs,
                                                   SceneUnitCreatorHost& host);

// 004F0AD0. The instance is the squadron's own 414h-byte object; 00964790 is
// still called with the entity's `Type` and its result discarded, so the plane
// class exists in the cache before the squadron is placed.
inline constexpr std::uint32_t kPlaneSquadronInstanceSize = 0x414;
SceneUnitCreationResult create_plane_squadron_004f0ad0(const SceneUnitCreationInputs& inputs,
                                                       SceneUnitCreatorHost& host);

}  // namespace bsp
