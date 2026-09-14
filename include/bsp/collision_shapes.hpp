#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/hit_narrowphase.hpp"

// Collision shapes: the class set installed into a collision node's inline shape
// array, the shape record layouts, the node-level sphere reject and the install
// sequence. Evidence and the full class table are in docs/COLLISION_SHAPES.md.
//
// Node-side offsets are NOT redeclared here: kCollisionNodeOffShapeArray,
// kCollisionNodeOffShapeCount and kCollisionNodeShapeSlots already live in
// bsp/hit_narrowphase.hpp and this file reuses them.
//
// Descriptive names are hypotheses, not recovered symbols.
namespace bsp {

// ---------------------------------------------------------------------------
// The shape record
// ---------------------------------------------------------------------------

// Common prefix, written by 0098AAB0 (__thiscall(shape; const float3* min,
// const float3* max), RET 8) as six FLD/FSTP pairs.
inline constexpr std::size_t kCollisionShapeOffVTable = 0x00;    // 0071102F
inline constexpr std::size_t kCollisionShapeOffBoundsMin = 0x04; // 0098AAB6
inline constexpr std::size_t kCollisionShapeOffBoundsMax = 0x10; // 0098AACB
inline constexpr std::size_t kCollisionShapeBaseSize = 0x1C;

// The unit part's shape adds three pointers. Size is pinned by the copy
// constructor 00711020 together with 00711460, which allocates a 0x30-byte list
// node whose value begins at +8h: 0x30 - 8 = 0x28.
inline constexpr std::size_t kUnitPartShapeOffNode = 0x1C;  // 00711035, 0071103E
inline constexpr std::size_t kUnitPartShapeOffBody = 0x20;  // 0071104D, 00711053
inline constexpr std::size_t kUnitPartShapeOffOwner = 0x24; // 00711065, 0071106B
inline constexpr std::size_t kUnitPartShapeSize = 0x28;

// The transformed box is embedded at entity+1A4h with the node at entity+1C4h
// (00929E9D, 00929EEF, 00929F3A), so its record ends where the node begins.
inline constexpr std::size_t kTransformedBoxShapeSize = 0x20;

// Both matrices the shape geometry reads live on the owning collision node.
// bsp/spatial_index.hpp names the same two fields kSpatialNodeOffWorldMatrix
// and kSpatialNodeOffInverseMatrix; they are repeated as shape-relative facts
// only in the comments below, never as new constants.
//   unit part shape: through the explicit node pointer at +1Ch (00724525, 0072452C)
//   transformed box: at this+70h and this+B0h, which are node+50h and node+90h

// The non-base group shape list. Complete 00712440 routes matches in damage
// rows >=1 here; its base list at +188 (head+18C) publishes inline pointers.
// Both lists contribute local bounds. See NATIVE_UNIT_PART_COLLISION_AH.md.
inline constexpr std::size_t kCollisionNodeOffShapeListProxy = 0x194; // 0071276E, 0074720C
inline constexpr std::size_t kCollisionNodeOffShapeListHead = 0x198;  // 0071275E, 00747206
inline constexpr std::size_t kCollisionNodeOffShapeSource = 0x160;    // 00712451
inline constexpr std::size_t kCollisionShapeListNodeOffValue = 0x08;  // 0071148C, 00712747
inline constexpr std::size_t kCollisionShapeListNodeSize = 0x30;      // 00711462

// Corrections to docs/EXPLOSION_RADIAL_DAMAGE.md, from 004E6480: the children
// are a heap vector, not an inline array. The count and array offsets are
// already declared in bsp/hit_narrowphase.hpp.
inline constexpr std::size_t kCollisionNodeOffChildCapacity = 0x104; // 004E64ED
inline constexpr int kCollisionNodeInitialChildCapacity = 2;         // 004E64ED
inline constexpr std::size_t kCollisionNodeChildBlockBytes = 8;      // PUSH 8 at 004E64C9

// Bounds of one shape, shape+4h..+18h.
struct CollisionShapeBounds {
    float min[3]{};
    float max[3]{};
};

// Vtable 00CFD768. The only class in the program whose sphere slot is a real
// test rather than XOR AL,AL; RET 0Ch.
struct UnitPartCollisionShapeRecord {
    std::uint32_t vtable{0};      // +0h,  00711044
    CollisionShapeBounds bounds;  // +4h..+18h, 0098AAB0
    std::uint32_t node{0};        // +1Ch, the owning collision node
    std::uint32_t body{0};        // +20h, the physics-library collision body
    std::uint32_t owner{0};       // +24h, the `this` of 00723F80 / 00723E90 / 00723840
};
static_assert(sizeof(CollisionShapeBounds) == 0x18, "shape bounds are six floats");

// ---------------------------------------------------------------------------
// The class set
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kVTableCollisionShapeBase = 0x00CE89DC;
inline constexpr std::uint32_t kVTableSubobjectShape = 0x00CEA050;
inline constexpr std::uint32_t kVTableFactoryShape = 0x00CF8B94;
inline constexpr std::uint32_t kVTableUnitPartShape = 0x00CFD768;
inline constexpr std::uint32_t kVTableTransformedBoxShape = 0x00D194B8;

enum class CollisionShapeClass {
    Base,          // 00CE89DC, both geometry slots __purecall
    Subobject,     // 00CEA050, trace 0087FF80, sphere stub 004F13A0
    Factory,       // 00CF8B94, both slots stubbed
    UnitPart,      // 00CFD768, trace 00724510, sphere 0070F720
    TransformedBox // 00D194B8, trace 00929B80, sphere stub 00929FF0
};

// True only for the one class whose slot 4 is not XOR AL,AL; RET 0Ch. This is
// the whole reason a blast reaches a hull through its parts and not through the
// ship entity's own node.
bool shape_class_answers_sphere(CollisionShapeClass shape_class) noexcept;

// Resolves the class from the vtable word found at shape+0h. Returns false for
// any other value: the enumeration is complete for this program, so an unknown
// vtable means the pointer is not a collision shape.
bool shape_class_from_vtable(std::uint32_t vtable, CollisionShapeClass& out) noexcept;

// ---------------------------------------------------------------------------
// The node-level sphere reject, 0098AB47-0098ABB0
// ---------------------------------------------------------------------------

// 0098AAE0 builds the query sphere's AABB (centre -/+ radius per axis at
// 0098AB01..0098AB43) and runs six FCOMIP tests against the node's own bounds
// before it touches the shape array. Each branch rejects only on a strict
// separation, so touching bounds still overlap, and an unordered compare (a NaN
// input) falls through to the continue side on all six.
//
// Pure: no host, no state. `node_min` and `node_max` are node+13Ch and node+148h.
bool collision_node_sphere_aabb_overlap(const float node_min[3], const float node_max[3],
                                        const float centre[3], float radius) noexcept;

// TEST byte ptr [body+5Ch],2 at 0070F727. The prepare call runs only when the
// bit is clear.
inline constexpr std::uint8_t kCollisionBodyPreparedBit = 0x02; // 0070F727
bool collision_body_needs_prepare(std::uint8_t body_flags) noexcept;

// ---------------------------------------------------------------------------
// Hosts
// ---------------------------------------------------------------------------

// One virtual method per native call site of the unit part's sphere test,
// 0070F720. Nothing here has a default: the physics library at 00B6xxxx and the
// owner callbacks at 00723xxx are contracts, not reimplemented behaviour.
struct UnitPartShapeSphereHost {
    virtual ~UnitPartShapeSphereHost() = default;
    // The prepared flag byte at body+5Ch, read at 0070F727.
    virtual std::uint8_t body_flags(std::uint32_t body) = 0;
    // 00B6DB70 at 0070F72F, this = the body. Gated on the flag bit.
    virtual void prepare_body(std::uint32_t body) = 0;
    // 00B6E0D0 at 0070F749, this = the body; (centre, radius, record). The
    // return is passed on to the owner untouched, so it stays opaque here.
    virtual std::uint32_t body_sphere_query(std::uint32_t body, const float centre[3],
                                            float radius, void* record) = 0;
    // 00723F80 at 0070F759, this = shape+24h; (body+F0h, the query result).
    // Its bool is the return of the whole sphere test.
    virtual bool owner_accept_sphere_hit(std::uint32_t owner, std::uint32_t body_detail,
                                         std::uint32_t query_result) = 0;
};

// 0070F720 as a sequence. Returns what AL carries at 0070F760.
bool unit_part_shape_test_sphere(const UnitPartCollisionShapeRecord& shape,
                                 const float centre[3], float radius, void* record,
                                 UnitPartShapeSphereHost& host) noexcept;

// The state an installer mutates: the node's inline shape array and its count.
// Capacity is kCollisionNodeShapeSlots from bsp/hit_narrowphase.hpp. The native
// store at 006D140A performs no bounds check; `overflowed` records what the
// native code would have written past the array rather than hiding it.
struct CollisionNodeShapeArray {
    std::uint32_t shapes[kCollisionNodeShapeSlots]{};
    int count{0};
    int overflowed{0};
};

// 006D1400, __thiscall(node; shape), RET 4. The canonical append; six further
// install sites inline exactly this body.
void collision_node_add_shape(CollisionNodeShapeArray& array, std::uint32_t shape) noexcept;

// 006D1420, node->shapeCount = 0.
void collision_node_clear_shapes(CollisionNodeShapeArray& array) noexcept;

// One virtual method per native call site of the two installers reconstructed
// here. Both installers are sequences over this host.
struct CollisionShapeInstallHost {
    virtual ~CollisionShapeInstallHost() = default;
    // 0098A920 at 0092B2F9, this = the node; two vectors.
    virtual void set_node_bounds(std::uint32_t node, const float min[3], const float max[3]) = 0;
    // 0042E630 at 0092B305, no arguments.
    virtual std::uint32_t spatial_index() = 0;
    // 0098BA10 at 0092B30C, this = the index; (node, 0, 0, 0).
    virtual void insert_node(std::uint32_t index, std::uint32_t node) = 0;
    // 0098AAB0 at 00712723, this = the list element value; (min, max).
    virtual void set_shape_bounds(std::uint32_t shape, const float min[3], const float max[3]) = 0;
    // The node's std::list of shapes, walked at 0071275A-0071277F and again at
    // 00747200 and 008518C0. Returns the element value addresses in list order,
    // that is listnode+8h, and the count actually produced.
    virtual int shape_list(std::uint32_t node, std::uint32_t* out, int capacity) = 0;
    // The bounds each list element is given before it is published, read from
    // the source records the native code decodes at [node+160h]+3Ch. That decode
    // is unread, so the bounds arrive through the host rather than being made up.
    virtual CollisionShapeBounds shape_source_bounds(std::uint32_t node, int index) = 0;
};

// FUN_0092AAE0's tail, 0092B289-0092B30C: the tickable game entity publishes the
// single transformed box embedded at entity+1A4h into the node at entity+1C4h,
// sets the node bounds and inserts the node into the spatial index.
// `entity` is the native base address; the two offsets are applied here.
void install_tickable_entity_shape(std::uint32_t entity, CollisionNodeShapeArray& array,
                                   const float node_min[3], const float node_max[3],
                                   CollisionShapeInstallHost& host) noexcept;

inline constexpr std::size_t kTickableEntityOffBoxShape = 0x1A4; // 0092B2CB, 00929F3A
inline constexpr std::size_t kTickableEntityOffNode = 0x1C4;     // 0092B2B3, 00929EEF
inline constexpr std::size_t kSubobjectOwnerOffShape = 0x344;    // 00883F94, 004F1206
inline constexpr std::size_t kSubobjectOwnerOffNode = 0x1E4;     // 00883F8E

// FUN_00712440's base-list publication, 00712700-00712758: the unit part's node
// gives the newly appended +188-list element its bounds and publishes its
// address. Returns the number published. coverage: partial, the record decode
// that fills the list is unread.
int install_unit_part_shapes(std::uint32_t node, CollisionNodeShapeArray& array,
                             CollisionShapeInstallHost& host) noexcept;

// ---------------------------------------------------------------------------
// 0098C460, the hit-record append
// ---------------------------------------------------------------------------

// Stride proved by the IMUL 30C30C31h / SAR 4 pair at 0098C477 and 0098C491.
inline constexpr std::size_t kHitRecordStride = 0x54;
inline constexpr std::size_t kHitRecordVectorOffProxy = 0x00;
inline constexpr std::size_t kHitRecordVectorOffFirst = 0x04; // 0098C467
inline constexpr std::size_t kHitRecordVectorOffLast = 0x08;  // 0098C472
inline constexpr std::size_t kHitRecordVectorOffEnd = 0x0C;   // 0098C48C

// The vector's three iterators, in elements rather than bytes.
struct HitRecordVectorState {
    std::uint32_t first{0};
    std::uint32_t last{0};
    std::uint32_t end{0};
};

// Element counts as 0098C460 computes them: (last - first) / 0x54 and
// (end - first) / 0x54, with first == 0 meaning an empty vector.
int hit_record_vector_size(const HitRecordVectorState& vec) noexcept;
int hit_record_vector_capacity(const HitRecordVectorState& vec) noexcept;

// Which of the two branches 0098C460 takes. The append always constructs a new
// element from the source; it never stores the source pointer, which is why the
// caller's 004704B0 at 0098C606 is safe.
enum class HitRecordAppendPath {
    ConstructInPlace, // 0098C4A6-0098C4CD, 0098BDE0 then last += 0x54
    Grow              // 0098C4D8ff, 0098C3B0
};
HitRecordAppendPath hit_record_append_path(const HitRecordVectorState& vec) noexcept;

} // namespace bsp
