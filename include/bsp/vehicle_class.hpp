#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_string.hpp"

// The vehicle-class descriptor system: the object 00964790 returns for a scene
// entity's Type property, one per row of the installed VehicleClass Lua table.
//
// Addresses: 00964790 (factory), 00749050 (base descriptor constructor),
//            00963380 (ship base), 00437F50 (registry singleton), 00437BD0
//            (cache resize), 0095BA60 (party bitmap write), 0095BAB0 (party
//            bitmap read), 004E27E0/00506550/00592640 (index-map writers) and
//            the 22 leaf constructors listed in kVehicleClassKindTable.
//
// Evidence: docs/VEHICLE_CLASS_DESCRIPTORS.md. Every descriptive C++ name here
// is a hypothesis; the class-name literals in kVehicleClassKindTable are not.
// They are read out of the image, returned by the descriptor's own vtable slot
// +0Ch, which is pure virtual in the base (00BF698E __purecall).
namespace bsp {

// The kind code the descriptor's vtable slot +1Ch returns, and the argument the
// is-kind predicate at slot +18h takes. Every one of these is a `mov eax, N;
// ret` constant read from the image. 5, 6 and 0Fh are never a leaf's own kind:
// they are the abstract bases, named here for the is-kind tests the factory
// makes (0Fh for the plane naming path, 6 for the ship recursion).
enum class VehicleClassKind : int {
    Base = 5,               // 00749140, the class 00749050 builds
    ShipBase = 6,           // no leaf returns it; 00963380's class
    Destroyer = 7,
    Submarine = 8,
    MotherShip = 9,
    Cruiser = 0x0A,
    Cargo = 0x0B,
    LandingShip = 0x0C,
    BattleShip = 0x0D,
    TorpedoBoat = 0x0E,
    PlaneBase = 0x0F,       // no leaf returns it; 007D1E50's class
    LevelBomber = 0x10,
    TorpedoBomber = 0x11,
    DiveBomber = 0x12,
    Fighter = 0x13,
    ReconPlane = 0x14,
    SmallReconPlane = 0x15,
    LargeReconPlane = 0x16,
    Kamikaze = 0x17,
    LandVehicle = 0x19,
    LandFort = 0x1B,
    CommandBuilding = 0x1C,
    DummyTargetVehicle = 0x35,
    AirField = 0x45,
    Shipyard = 0x46,
};

// One branch of the factory's string chain. Sizes are the operator new / 00470B80
// argument in 00964790; instance_size is the operator new argument inside the
// descriptor's vtable slot +28h, and is zero for the one class that inherits the
// base's `xor eax, eax; ret 4` and allocates nothing.
struct VehicleClassKindRow {
    const char* lua_type;        // the VehicleClass.Type literal, compared with 00425850
    const char* class_name;      // recovered: the vtable +0Ch string literal
    std::uint32_t descriptor_size;
    std::uint32_t instance_size; // 0 when slot +28h is the base's null allocator
    VehicleClassKind kind;
    std::uint32_t constructor;   // the leaf constructor, 0 when inlined into 00964790
    std::uint32_t vtable;        // primary vtable; the secondary at +6Ch is this minus 4
    std::uint32_t allocate_instance; // vtable slot +28h
    std::int32_t shipped_rows;   // rows in the installed vehicleclasses.lua
    std::uint8_t eh_state;       // the SEH state 00964790 records for the branch
};

inline constexpr int kVehicleClassKindCount = 22;
// In the factory's comparison order, which is the order the branches are tested.
extern const VehicleClassKindRow kVehicleClassKindTable[kVehicleClassKindCount];

// The chain at 009648EC..00964EEC: 00425850 against each literal in turn, which
// is a case-insensitive compare (it delegates to _stricmp). Returns the index
// into kVehicleClassKindTable, or kVehicleClassKindUnknown when no branch takes,
// which is the path that returns a null descriptor without caching anything.
inline constexpr int kVehicleClassKindUnknown = -1;
int vehicle_class_kind_index(const char* lua_type) noexcept;
const VehicleClassKindRow* vehicle_class_kind_row(const char* lua_type) noexcept;
const VehicleClassKindRow* vehicle_class_kind_row_for(VehicleClassKind kind) noexcept;

// ---------------------------------------------------------------------------
// Registry singleton 00437F50
// ---------------------------------------------------------------------------
//
// | offset  | field                                                          |
// | ------- | -------------------------------------------------------------- |
// | +4h     | descriptor cache: data pointer                                  |
// | +8h     | descriptor cache: size, grown through 00437BD0                  |
// | +0Ch    | descriptor cache: capacity                                      |
// | +10h    | forward index map, 800h int entries                             |
// | +2010h  | inverse index map, 800h int entries                             |
// | +4010h  | party-requires-class bitmap: data pointer, stride 3             |
// | +4014h  | bitmap size, grown through 004359E0                             |
//
// The two maps are one bijection kept in both directions. 00592640 and 00506550
// reset all 800h pairs to the identity and then rewrite exactly one pair, so a
// Type that no one has remapped resolves to itself. That is why 0095BA60 can
// take an already-resolved class index and map it a second time without harm.
inline constexpr int kVehicleClassIndexMapSize = 0x800;

struct VehicleClassIndexMap {
    std::int32_t forward[kVehicleClassIndexMapSize]{}; // +10h, keyed by Type
    std::int32_t inverse[kVehicleClassIndexMapSize]{}; // +2010h, keyed by class index

    // The loop at 00592652..00592668 and 0050657C..00506592.
    void reset_identity_00592652() noexcept;
    // The pair write at 00592667/0059266E, the only non-identity entry either
    // caller leaves behind.
    void remap_00592667(std::int32_t type_id, std::int32_t class_index) noexcept;

    // singleton[10h + type_id * 4]. Out-of-range ids are the caller's problem in
    // the native, which indexes without a bound check; this returns -1 instead.
    std::int32_t to_class_index(std::int32_t type_id) const noexcept;
    std::int32_t to_type_id(std::int32_t class_index) const noexcept;
};

// 0095BA60 writes byte[base + class_index * 3 + party] = 1 and returns without
// doing anything when party is neither 0 nor 1. 0095BAB0 reads the same byte for
// the local player's party. The third byte of every row is never written by
// either; its reader is not identified.
inline constexpr int kVehicleClassPartyStride = 3;
bool vehicle_class_party_valid_0095ba60(int party) noexcept;
std::size_t vehicle_class_party_slot_0095ba60(int class_index, int party) noexcept;

// The test at 00964F31: Race 1 and Race 4 are party 1, every other value is
// party 0. The installed table authors Race 0, 1, 2, 3, 4 and 5.
int vehicle_class_party_from_race_00964f31(int race) noexcept;

// ---------------------------------------------------------------------------
// The descriptor
// ---------------------------------------------------------------------------
//
// The base is 0138h bytes, built by 00749050. Only the fields below are
// recovered; the rest of the base and every derived extension is opaque here.
// The two vptrs are host-owned, so they are not modelled as members: the base
// writes the primary at +0h and the secondary at +6Ch, and every leaf overwrites
// both with its own pair.
inline constexpr std::uint32_t kVehicleClassDescriptorBaseSize = 0x138;
inline constexpr std::uint32_t kVehicleClassDescriptorClassIndexOffset = 0x70;
inline constexpr std::uint32_t kVehicleClassDescriptorTypeNameOffset = 0x74;
inline constexpr std::uint32_t kVehicleClassDescriptorEngineCountOffset = 0x7C;
inline constexpr std::uint32_t kVehicleClassDescriptorLinkedClassOffset = 0xC0;
inline constexpr std::uint32_t kVehicleClassDescriptorSecondaryVptrOffset = 0x6C;
// 00963380 runs an eh_vector_constructor_iterator over 14h elements of 30h bytes
// starting at +138h, immediately past the base, so the ship extension begins
// exactly where the base ends.
inline constexpr std::uint32_t kVehicleClassShipSubObjectCount = 0x14;
inline constexpr std::uint32_t kVehicleClassShipSubObjectStride = 0x30;

struct VehicleClassDescriptor {
    std::int32_t class_index{-1};        // +70h, written by 00964F5C
    NativeString type_name;              // +74h, the row's Type string
    std::int32_t engine_count{0};        // +7Ch, NumEngines or 0 when the key is nil
    // +C0h, read back at 00965182 and compared against -1. Neither 00749050 nor
    // any leaf constructor writes it: it is zero out of the memset until the
    // vtable +8h load fills it, so a ship whose load does nothing still passes
    // the != -1 test. That is native behaviour, not a modelling choice.
    std::int32_t linked_class_index{0};
    int kind_index{kVehicleClassKindUnknown};
    std::int32_t ref_count{1};           // +4h of the refcounted base
};

// The kind tests 00964790 makes on the freshly built descriptor, through vtable
// slot +18h. Named for what the branch does, not for a recovered symbol.
inline constexpr int kVehicleClassIsPlaneKind = 0x0F;  // 00964FD6, the Class_ name
inline constexpr int kVehicleClassIsLandVehicleKind = 0x19; // 0096507C
inline constexpr int kVehicleClassIsLandFortKind = 0x1B;    // 0096508B
inline constexpr int kVehicleClassIsShipKind = 0x06;        // 0096516B, the recursion

// 00964FBF..0096505B builds a string from three literals: "Class_" always, then
// "own_" or "enemy_" when the descriptor is a plane. 0095BAB0 returns true when
// the local player's party has *not* registered a need for the class, which is
// the enemy case. Nothing else is appended; the native leaves the string at the
// prefix and hands it to 004CACA0 with the class index.
std::string vehicle_class_debug_name_00964fbf(bool is_plane, bool enemy_class);

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
//
// One method per native call site of 00964790, in the order the body reaches
// them. Nothing here has a default: none of it stands in for game behaviour that
// has not been recovered. The Lua half mirrors GuiLuaHost's operations because
// the factory calls the same wrappers (00B67980, 00B67800, 00B67720, 00B662B0,
// 00B66380, 00B65FB0, 00B66290, 00B67700); it is repeated rather than inherited
// because the factory also writes back into the row, which the GUI never does.
struct VehicleClassHost {
    virtual ~VehicleClassHost() = default;

    // 00437F50 singleton, +8h and 00437BD0. The native resizes before the read
    // and again on the way out, so a resolve can never index past the end.
    virtual int cache_size() = 0;
    virtual void resize_cache_00437bd0(int size) = 0;
    virtual VehicleClassDescriptor* cached(int class_index) = 0;
    // 00965119..0096513A: unref the descriptor already in the slot, clear it,
    // then store. The unref is InterlockedDecrement on +4h followed by vtable
    // slot 0 when it reaches zero.
    virtual void store_cached(int class_index, VehicleClassDescriptor* descriptor) = 0;

    // singleton +10h, the forward index map.
    virtual std::int32_t type_to_class_index(std::int32_t type_id) = 0;

    // 00B67980 against the interpreter at (*00E188A8)+1A0Ch.
    virtual GuiLuaRef globals() = 0;
    virtual GuiLuaRef get_by_name(const GuiLuaRef& table, const char* key) = 0; // 00B67800
    virtual GuiLuaRef get_by_index(const GuiLuaRef& table, std::int32_t key) = 0; // 00B67720
    virtual const char* to_string(const GuiLuaRef& object) = 0;   // 00B662B0
    virtual int to_integer_or(const GuiLuaRef& object, int fallback) = 0; // 00B66380
    virtual bool is_nil(const GuiLuaRef& object) = 0;             // 00B65FB0
    virtual int to_integer(const GuiLuaRef& object) = 0;          // 00B66290
    virtual void release(const GuiLuaRef& object) = 0;            // 00B67700
    // 00B67460 and 00B673A0, both taking a native string key built on the stack
    // and released straight after. These are the only writes the factory makes
    // into the Lua table.
    virtual void set_row_int(const GuiLuaRef& row, const char* key, std::int32_t value) = 0;
    virtual void set_row_bool(const GuiLuaRef& row, const char* key, bool value) = 0;

    // 00BF55BE + 00BF79F0 for the two inlined branches, 00470B80 for the other
    // twenty; both give zeroed storage of exactly size bytes. Null is a real
    // outcome the native tests for. The leaf constructor named by the row runs
    // on the result.
    virtual VehicleClassDescriptor* construct_descriptor(int kind_index, std::uint32_t size) = 0;
    virtual void destroy_descriptor(VehicleClassDescriptor* descriptor) = 0;

    // 0095BA60.
    virtual void mark_party_requires_class(std::int32_t class_index, int party) = 0;
    // vtable slot +18h.
    virtual bool descriptor_is_kind(VehicleClassDescriptor* descriptor, int kind) = 0;
    // 0095BAB0 against the local player's party.
    virtual bool class_is_enemy(std::int32_t class_index) = 0;
    // 004CACA0, taking the built name and the class index.
    virtual void register_debug_name(const std::string& name, std::int32_t class_index) = 0;

    // vtable slot +8h, the Lua load. It takes a native string out-parameter the
    // caller destroys; the factory does not read it.
    virtual void load_descriptor(VehicleClassDescriptor* descriptor) = 0;
    virtual void descriptor_activate(VehicleClassDescriptor* descriptor) = 0; // slot +10h, argument 0
    virtual void descriptor_finalize(VehicleClassDescriptor* descriptor) = 0; // slot +14h

    // Storage the descriptor's type-name string allocates through. The native
    // routes it to the sized pool behind 00419CC0.
    virtual NativeStringStorage& string_storage() = 0;
};

// The Lua keys the factory reads or writes, in the order it reaches them.
inline constexpr const char* kVehicleClassTableName = "VehicleClass"; // 00CE5880
inline constexpr const char* kVehicleClassTypeKey = "Type";           // 00CE4780
inline constexpr const char* kVehicleClassLandingShipKey = "LandingShip"; // 00CEB7BC
inline constexpr const char* kVehicleClassIdKey = "ID";               // 00CE59B4, written
inline constexpr const char* kVehicleClassGotKey = "Got";             // 00CE452C, written
inline constexpr const char* kVehicleClassRaceKey = "Race";           // 00CE8EE0
inline constexpr const char* kVehicleClassNumEnginesKey = "NumEngines"; // 00D0651C

// Why a resolve ended where it did. This is the reconstruction's own reporting;
// the native returns only the cache slot.
enum class VehicleClassResolveOutcome {
    Cached,            // 009647F9 found a non-null slot
    Constructed,       // a branch took and the descriptor reached the cache
    UnknownType,       // no literal matched: 00964EA6 returns 0 and caches nothing
    AllocationFailed,  // the allocator returned null: 00964EE8
};

struct VehicleClassResolveResult {
    VehicleClassDescriptor* descriptor{nullptr};
    VehicleClassResolveOutcome outcome{VehicleClassResolveOutcome::UnknownType};
    std::int32_t class_index{-1};
    int recursions{0}; // LandingShip pre-resolves and the ship tail re-entry
};

// 00964790, __fastcall(int type_id in ECX, bool read_race in DL), RET 0,
// returning the cache slot in EAX.
//
// 1. class_index = forward_map[type_id], read twice through two separate
//    00437F50 calls that must agree.
// 2. Resize the cache to class_index + 1 if it is short, then return
//    cache[class_index] when it is not null. The test is compiled as
//    NEG/SBB/TEST EAX,0F8A0BDh, which is `cached != 0` with a dead immediate:
//    there is no second condition hiding in it.
// 3. Otherwise read VehicleClass[class_index]. Type as a string, LandingShip as
//    an integer defaulting to 0; a non-zero LandingShip resolves that class
//    first, with read_race set.
// 4. Write ID = class_index and Got = true back into the row.
// 5. Match Type against the 22 literals in order and run that branch's
//    allocation and constructor.
// 6. With read_race set, read Race (default 0) and mark the party.
// 7. Fill +70h, +74h and +7Ch, build the debug name, run the load, publish into
//    cache[class_index], activate and finalize.
// 8. For a ship whose +C0h is not -1, resolve that class too.
//
// The recursion guard 00F8A098 counts nesting around steps 7 and 8; a nested
// resolve skips the 1Ch-byte object 00964790 builds at 00965095 for the
// outermost one. That object's role is not recovered.
VehicleClassResolveResult resolve_vehicle_class_00964790(VehicleClassHost& host,
                                                         std::int32_t type_id, bool read_race);

} // namespace bsp
