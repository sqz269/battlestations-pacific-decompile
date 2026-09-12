#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// The hidden-entity record map at SceneDatabase+18h: the key rule, the record
// layout, the scene-reader branch that fills it (0046CF40, 0046D5E4..0046D7E9)
// and the three native routines that read it (004691B0, 00469480, 0046DC10).
// Evidence, the per-field writer/reader table and the full call-site tables are
// in docs/SCENE_RECORD_MAP.md.
//
// What the map is: the scene database's index of entities the scene file
// authored with the `Hidden` property set. Those entities are parsed but never
// instantiated during the read; the record keeps what is needed to instantiate
// one later by name. The Lua binding name `FindHiddenEntity` (00D0FE10, paired
// with 008A9E10 at 00E0BFF0) is recovered; every C++ name here is a hypothesis.
//
// This is a rule and a sequence over an injected host, not a binary-compatible
// replacement. The red-black tree (00468140, 0046B340, 0046AD70, 004678F0), the
// CRT string and pool helpers, the generation gate 0046C550, the class creators
// and the Lua frame plumbing are contracts and are not ported.

namespace bsp {

// ---------------------------------------------------------------------------
// The record (operator new(5Ch) at 0046D68C, constructed by 004693C0)
// ---------------------------------------------------------------------------

// Offsets are named kSceneHiddenRecord* to stay clear of the kSceneCreateRecord*
// constants include/bsp/scene_entity_create.hpp declares for the same object
// from the consumer side. The two agree; this header adds the fields only the
// producer shows (+0h, +8h, +10h) and the size.
inline constexpr std::uint32_t kSceneHiddenRecordVftable = 0x00;
inline constexpr std::uint32_t kSceneHiddenRecordProperties = 0x04;
inline constexpr std::uint32_t kSceneHiddenRecordName = 0x08;
inline constexpr std::uint32_t kSceneHiddenRecordClassName = 0x0c;
inline constexpr std::uint32_t kSceneHiddenRecordParty = 0x10;
inline constexpr std::uint32_t kSceneHiddenRecordFrame = 0x14;
inline constexpr std::uint32_t kSceneHiddenRecordParentNameLength = 0x54;
inline constexpr std::uint32_t kSceneHiddenRecordParentNameData = 0x58;
inline constexpr std::uint32_t kSceneHiddenRecordSize = 0x5c;

// The single-slot vftable at 00CE5640; the slot holds 004693A0, the scalar
// deleting destructor.
inline constexpr std::uint32_t kSceneHiddenRecordVftableAddress = 0x00ce5640;

// The empty-string global the parent lookup substitutes for a null data
// pointer. kSceneEmptyStringData in scene_entity_create.hpp is the same value
// seen from 0046D930; not redeclared here.
inline constexpr std::uint32_t kSceneHiddenRecordDefaultParentName = 0x00ce3a0c;

// The 5Ch record, one C++ field per native offset. `name` and `class_name` are
// two independent owned strdup'd char* in the native record (00469411 and
// 00469428, freed separately at 00469344 and 00469357), not one string with a
// length: that is the correction this packet makes to the provisional reading
// in docs/SCENE_ENTITY_CREATE.md.
struct SceneHiddenEntityRecord {
    void* properties{nullptr};   // +4h, owned; deleted through its own vtable
    std::string name;            // +8h, the authored entity name and the map key
    std::string class_name;      // +0Ch, keys the class map at SceneDatabase+34h
    std::int32_t party{0};       // +10h, the `Party` property value
    float frame[16]{};           // +14h..+53h, the frame handed to 0046C550 as a4
    std::string parent_name;     // +54h/+58h, empty when the entity has no parent
};

// ---------------------------------------------------------------------------
// The key rule
// ---------------------------------------------------------------------------

// The key type is the bare NativeString {uint32 length at +0h, char* data at
// +4h}. A null data pointer is never dereferenced: 00443D0D and 00443D24 test
// the lengths first, so a default-constructed key behaves exactly like "".
struct SceneRecordKey {
    std::uint32_t length{0};
    const char* data{nullptr};
};

// 00443D00 BSP_NativeString_LessCaseInsensitive, and the same three branches
// inlined at 00468155..0046817A and 0046B361..0046B390.
//
//   if (a.length == 0) return b.length != 0;
//   if (b.length == 0) return false;
//   return _stricmp(a.data, b.data) < 0;
//
// Lookups are therefore case-insensitive: "Carrier1" finds a record authored
// as "carrier1".
bool scene_record_key_less(const SceneRecordKey& a, const SceneRecordKey& b);
bool scene_record_key_less(const std::string& a, const std::string& b);

// The equivalence the map's strict weak ordering induces: neither less.
bool scene_record_key_equivalent(const std::string& a, const std::string& b);

// ---------------------------------------------------------------------------
// The container, as a view rather than a tree
// ---------------------------------------------------------------------------

// Node offsets of the MSVC _Tree node, from the two walks. Declared so a reader
// can match the listing; nothing here walks a native node.
inline constexpr std::uint32_t kSceneRecordMapOffset = 0x18;  // from SceneDatabase
inline constexpr std::uint32_t kSceneRecordMapHead = 0x04;    // _Myhead
inline constexpr std::uint32_t kSceneRecordNodeLeft = 0x00;
inline constexpr std::uint32_t kSceneRecordNodeParent = 0x04;
inline constexpr std::uint32_t kSceneRecordNodeRight = 0x08;
inline constexpr std::uint32_t kSceneRecordNodeKeyLength = 0x0c;
inline constexpr std::uint32_t kSceneRecordNodeKeyData = 0x10;
inline constexpr std::uint32_t kSceneRecordNodeMapped = 0x14;
inline constexpr std::uint32_t kSceneRecordNodeIsNil = 0x19;
// The value_type 00469FC0 builds and 00469A20 copies: key at +0h/+4h, mapped
// pointer at +8h. 12 bytes.
inline constexpr std::uint32_t kSceneRecordEntryMapped = 0x08;
inline constexpr std::uint32_t kSceneRecordEntrySize = 0x0c;

// One entry, as the reconstruction sees the container: an ordered sequence of
// {key, record} with the map's own comparison. The tree shape is not modelled.
struct SceneRecordEntry {
    std::string key;
    const SceneHiddenEntityRecord* record{nullptr};
};

// A view over the map's entries in any order. Lookup applies the key rule, so
// the result does not depend on the order.
struct SceneRecordMapView {
    std::vector<SceneRecordEntry> entries;
};

// 00468CD0 followed by the end check and the mapped read: null when no key is
// equivalent. Models 004691B0's return and the record read in 0046D930 and
// 0046DC10 alike.
const SceneHiddenEntityRecord* scene_record_map_find(const SceneRecordMapView& map,
                                                     const std::string& name);

// 00469480 / the Lua binding FindHiddenEntity: the same lookup, reduced to its
// end test.
bool scene_record_map_contains(const SceneRecordMapView& map, const std::string& name);

// ---------------------------------------------------------------------------
// Filling the map (0046CF40, the branch at 0046D5E4..0046D7E9)
// ---------------------------------------------------------------------------

// The four gates, in the order the native code tests them. Every one must hold
// for a record to exist.
struct SceneHiddenRecordGates {
    bool hidden_property{false};   // 0046D5F4, the `Hidden` value byte at +0Ch
    bool recording_enabled{false}; // 0046D5FA, argument a6 at entry+54h
    bool should_generate{false};   // 0046D65A, 0046C550's result
    bool registration_pass{false}; // 0046D679, the argument at entry+58h; must be false
};

// Why a block produced no record. `Duplicate` is not a gate: the record is
// built and the insert refuses it (0046B447).
enum class SceneHiddenRecordOutcome {
    NotHidden,
    RecordingDisabled,
    GenerationRefused,
    RegistrationPass,
    AllocationFailed,
    Duplicate,
    Inserted,
};

// What the reader has in hand when the gates pass. `name` is the 256-byte stack
// buffer at entry-10Ch, which is also 0046C550's entityName argument and the
// source of the map key: one buffer, three uses.
struct SceneHiddenRecordSource {
    std::string name;
    std::string class_name;
    std::int32_t party{0};
    float frame[16]{};
    std::string parent_name;
    void* properties{nullptr};
};

struct SceneHiddenRecordInsertResult {
    SceneHiddenRecordOutcome outcome{SceneHiddenRecordOutcome::NotHidden};
    // Non-null from AllocationFailed onward: the record the reader built. On
    // Duplicate it is orphaned, because 0046CF40 ignores the insert's bool.
    const SceneHiddenEntityRecord* record{nullptr};
    bool leaked{false};  // Duplicate only
};

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------

// One method per native callee of the branch that fills the map and of the two
// read wrappers, in call order. No default implementations: nothing here stands
// in for unrecovered behaviour. Call sites are in the comments and, one row per
// site, in reports/scene_record_map.json.
struct SceneRecordMapHost {
    virtual ~SceneRecordMapHost() = default;

    // 0046D5EB and 0046D674 FUN_008F2260: read a property out of the bag the
    // block just parsed. The native value is the dword at [result+0Ch]; the
    // `Hidden` test reads its low byte, `Party` the whole dword.
    virtual std::int32_t read_property(void* property_bag, std::uint32_t key_literal) = 0;

    // 0046D655 FUN_0046C550, the per-entity generation predicate. This site is
    // the only one that pushes 1 for the trailing argument (0046D60F); every
    // other caller pushes 0.
    virtual bool should_generate(const SceneHiddenRecordSource& source) = 0;

    // 0046D68C operator new(5Ch). Returning null is tolerated: 0046D6A4 skips
    // the construction and the record stays 0.
    virtual void* allocate_record(std::uint32_t size) = 0;

    // 0046D6B6, the parent object's name through its vtable slot at +10h, with
    // 00CE3A0C substituted when the reader has no parent (0046D6BA).
    virtual std::string parent_name(void* parent) = 0;

    // 0046D6F4 FUN_004693C0. The two char* fields are strdup'd here
    // (00438E40 at 00469408 and 00469414) and the frame is copied by
    // BSP_Matrix_Copy4x4X87 at 0046942B.
    virtual const SceneHiddenEntityRecord* construct_record(const SceneHiddenRecordSource& source) = 0;

    // 0046D75F FUN_0046B340 on SceneDatabase+18h. Returns false when a record
    // with an equivalent key is already present, in which case nothing is
    // inserted (0046B447). The key temporary (0046D71B), the sink pair
    // constructor (0046D729) and its copy (0046D73E) are implementation details
    // of this method, as are the three pool releases at 0046D77F..0046D7E4.
    virtual bool insert_record(const std::string& key, const SceneHiddenEntityRecord* record) = 0;
};

// The branch at 0046D5E4..0046D7E9 as a sequence over the host.
SceneHiddenRecordInsertResult scene_record_map_record_entity(SceneRecordMapHost& host,
                                                            const SceneHiddenRecordGates& gates,
                                                            const SceneHiddenRecordSource& source);

// ---------------------------------------------------------------------------
// The placement override in 0046DC10
// ---------------------------------------------------------------------------

// 0046DC10 differs from 0046D930 in the frame it hands the creator. The three
// caller floats replace elements 12, 13 and 14 (+30h, +34h, +38h) at
// 0046DD62..0046DD7E, and when the heading is under a full turn the 3x3 part is
// rebuilt from the Euler triple (0, heading, 0) by 00467050 at 0046DD9F.
//
// The threshold is the double at 00CE3828, 0x401921FB60000000: (float)6.2831855
// widened to double, i.e. two pi at float precision. A heading at or above it
// leaves the authored basis alone, which is how the caller says "no heading".
inline constexpr double kSceneHiddenRecordHeadingSentinel = 6.2831855773925781;

struct ScenePlacementOverride {
    float position[3]{};
    float heading{0.0f};
};

// Applies the part of the override this packet read: the REP MOVSD of the
// record's frame followed by the three translation stores. The 3x3 rebuild is
// NOT applied. 00467050 belongs to the matrix code and only its first half was
// read here, so reproducing its basis would be an invention; call
// scene_record_heading_applies() to learn whether the native path would also
// have replaced the basis.
void scene_record_apply_placement(const float record_frame[16],
                                  const ScenePlacementOverride& placement,
                                  float out_frame[16]);

// True when 0046DD87 takes the rotation branch.
bool scene_record_heading_applies(float heading);

}  // namespace bsp
