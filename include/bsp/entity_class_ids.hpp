#pragma once
// The entity class-id space: the dword every constructor stamps at +C4h, the
// class test in vtable slot 5Ch that reads it, and the id -> class -> parent
// table the whole engine selects on.
// docs/ENTITY_CLASS_IDS.md. Addresses: 0042B8F0, 0047F190, 00480930, 006D20E0,
// 006E2790, 006E3D50, 006FE050, 0064B720, 006508B0 (class tests read byte for
// byte); 87 class tests in all, listed in reports/entity_class_ids.json.
//
// Class names are recovered string literals from the class-descriptor records
// and the scene registration table, not hypotheses; the id -> parent edges are
// decoded from the compiled compare chains. Nothing here is an ABI-compatible
// replacement: the native test is a __thiscall virtual, this is a pure function
// over the recovered table.
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Where the id lives
// ---------------------------------------------------------------------------

// The dword every constructor level writes after installing its vtable. The
// same offset as kUnitOffClassId in unit_instance.hpp, restated here because
// the field is not specific to units: scene objects, ordnance and guns carry it
// too. It holds the MOST DERIVED class id, not a depth -- the destroyer chain
// writes 01, 02, 04, 05, 06, 07 and skips 03, which exists as its own class.
inline constexpr std::size_t kEntityClassIdFieldOffset = 0x0C4;

// The vtable slot that answers the class test. Same slot as
// kUnitVtableIsKindOf; restated for the non-unit branches of the tree.
inline constexpr std::size_t kEntityClassTestVtableSlot = 0x05C;

// The class test is __thiscall with one stack argument and RET 4.
inline constexpr int kEntityClassTestStackBytes = 4;

// ---------------------------------------------------------------------------
// The id space
// ---------------------------------------------------------------------------

// Highest class id with a compiled class test is 60h (007810F0). The world
// bucket array of 0088B1D8 runs from bucket -6 to bucket 5Ah, which is class id
// 0 to class id 60h, so 60h is the top of the space and not just the top of
// what was decoded.
inline constexpr int kEntityClassIdMax = 0x60;
inline constexpr std::size_t kEntityClassIdCount = kEntityClassIdMax + 1;

// The Lua entity-lookup bucket index is the class id minus six
// (0088B1D8 MOV EBX,0FFFFFFFAh for class 0, 0088B2CA CMP EBX,5Bh for class 60h).
inline constexpr int kEntityKindBucketBias = 6;

// The root class: every class test answers true for a query of 0.
inline constexpr int kEntityClassIdRoot = 0x00;

// Sentinel in EntityClassRow::parent for the root and for ids with no class
// test in the image.
inline constexpr int kEntityClassNoParent = -1;

// ---------------------------------------------------------------------------
// The table
// ---------------------------------------------------------------------------

struct EntityClassRow {
    int id;             // the value at +C4h
    int parent;         // immediate base, or kEntityClassNoParent
    const char* name;   // recovered literal, or nullptr when none was found
};

// Indexed by class id, 0 .. kEntityClassIdMax. Rows whose parent is
// kEntityClassNoParent and whose id is not 0 have no class test in the image.
extern const EntityClassRow kEntityClassTable[kEntityClassIdCount];

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

// nullptr when id is outside 0 .. kEntityClassIdMax.
const EntityClassRow* entity_class_row(int id) noexcept;

// Recovered class name, or nullptr when the id is unnamed or out of range.
const char* entity_class_name(int id) noexcept;

// Immediate base, or kEntityClassNoParent.
int entity_class_parent(int id) noexcept;

// Number of edges from id up to the root; -1 when id is out of range or has no
// class test.
int entity_class_depth(int id) noexcept;

// The class test as a pure function: true when `query` is `dynamic_id` itself
// or any class on its parent chain, including the root 0.
//
// The native routine compiles the chain of the class that OWNS the vtable into
// a run of CMP EAX,imm and adds one CMP EAX,[ECX+C4h] for the dynamic id, so a
// derived class that does not override slot 5Ch is still recognised by exact
// identity through that last compare. The two forms agree for every class that
// has its own test; entity_is_kind_of_inherited models the other case.
bool entity_is_kind_of(int dynamic_id, int query) noexcept;

// The native rule when the object's class does not override slot 5Ch:
// `owner_id` is the class whose vtable supplies the test. True when query is on
// owner_id's chain, or equals dynamic_id exactly.
bool entity_is_kind_of_inherited(int owner_id, int dynamic_id, int query) noexcept;

// Class id for a Lua entity-lookup bucket index, and back. No range check on
// the bucket beyond the id staying inside the table.
int entity_class_id_for_bucket(int bucket) noexcept;
int entity_bucket_for_class_id(int id) noexcept;

}  // namespace bsp
