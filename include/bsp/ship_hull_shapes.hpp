#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/ship_hull_body.hpp"
#include "bsp/world_ocean.hpp"

// Which model nodes become the hull body's collision shapes, and the AABB those shapes
// give the body -- the last input docs/SHIP_HULL_BODY.md left open for the hull inertia.
//
// docs/SHIP_HULL_SHAPES.md carries the addresses, the original ABI and the uncertainty.
// Everything here is a semantic C++ interface for MSVC Win32, not a drop-in binary
// replacement, and every descriptive name is a hypothesis rather than a recovered symbol.
//
// The chain, in the order the game runs it:
//
//   00938F61..0093918C  walk the model's collision-record list and keep every record
//                       whose owning node is one of four: the node named "firstnode",
//                       the node at model+0Ch, the node named "front", the node named
//                       "back". Each kept record contributes one shape descriptor.
//   009391BD            resize the shape-descriptor vector; 00931A10 stamps the default
//                       record, which is where the shape type 4 and the identity
//                       transform come from.
//   009392FA..009396B8  fill one 48h-byte descriptor per kept record and push its
//                       address into the body descriptor's shape vector (desc+78h).
//   009396BA..009399BF  the extra periscope shape, for a class whose +510h and +514h
//                       are both <= 0 and whose model has a node named "periszkop".
//   00C5D8C0..00C5D8E7  Dyn::World::CreateBody hands each descriptor to 00C5C940.
//   00C5C940            allocates the shape for its type and links it into the body's
//                       list at B+70h, threaded through shape+208h.
//   00C57F50            the type-4 constructor, Dyn::ConvexMeshShape.
//   00C57C40            the shape's own AABB from the convex mesh's local box.
//   00C55FC0            the body's AABB (B+38h/B+44h) as the union over the list.
//
// The `fizika_%02d` and `hajobelso` node walks in the same function do NOT feed this
// vector. They build the controller's part and debris group lists (docs/UNIT_PARTS.md)
// and hide interior geometry; no descriptor they produce reaches CreateBody.

namespace bsp {

// ---------------------------------------------------------------------------
// The Dyn shape descriptor
// ---------------------------------------------------------------------------

// 48h bytes. 00939322..009396B8 fills one per collision record and 00C5C940 consumes it
// with the descriptor in ECX and the body in EDX. The field meanings below are settled by
// the type-4 constructor 00C57F50, which is the producer of the shape object's fields, and
// by the shape offsets docs/DYN_COLLISION_PASS.md already recorded
// (bsp/dyn_collision_pass.hpp: type +08h, restitution +24h, friction +28h, group +2Ch,
// mask +30h, list next +208h).
inline constexpr std::size_t kDynShapeDescSize = 0x48;
inline constexpr std::size_t kDynShapeDescOffRestitution = 0x00; // 00C57F85 -> shape+24h
inline constexpr std::size_t kDynShapeDescOffFriction = 0x04;    // 00C57F8B -> shape+28h
inline constexpr std::size_t kDynShapeDescOffGroup = 0x08;       // 00C57F92 -> shape+2Ch
inline constexpr std::size_t kDynShapeDescOffMask = 0x0C;        // 00C57F98 -> shape+30h
inline constexpr std::size_t kDynShapeDescOffType = 0x10;        // 00C5C95E, 00C57F7F
inline constexpr std::size_t kDynShapeDescOffGeometry = 0x14;    // 00C57FB4, dereferenced
inline constexpr std::size_t kDynShapeDescOffTransform = 0x18;   // 00C57FBF, twelve floats

// desc+10h, the switch selector at 00C5C95E/00C5C971. `CMP EAX,5 / JA` at 00C5C968 makes
// anything above 5 a no-op, and case 3 has no entry either, so those descriptors attach
// nothing at all. Each class below is settled the same way: the constructor installs a
// vtable whose slot 0 is the already-named bounds refresher for that class.
enum class DynShapeType : std::uint32_t {
    kSphere = 0,      // 00C585B0, vtable 00D7A058 -> Dyn_SphereShape_vslot0 00C58520
    kBox = 1,         // 00C57B90, vtable 00D7A0C8 -> Dyn_BoxShape_vslot0 00C57880
    kCylinder = 2,    // 00C582E0, vtable 00D7A098 -> Dyn_CylinderShape_vslot0 00C57FF0
    kNoShape = 3,     // no switch entry: 00C5C940 falls through and attaches nothing
    kConvexMesh = 4,  // 00C57F50, vtable 00D7A0AC -> Dyn_ConvexMeshShape_vslot0 00C57C40
    kTerrain = 5,     // 00C58840, vtable 00D7A194 -> Dyn_TerrainShape_vslot0 00C58690
};

// The 48h-byte record as the game fills it. The transform is four rows of three floats,
// the same row-vector layout DynBodyDescriptor uses: three basis rows then the
// translation, at desc+18h, +24h, +30h and +3Ch.
struct DynShapeDescriptor {
    float restitution{0.0f};
    float friction{0.0f};
    std::uint32_t group{0};
    std::uint32_t mask{0};
    DynShapeType type{DynShapeType::kConvexMesh};
    // desc+14h. 00C57FB4 loads the first dword AT this address into shape+210h and
    // 00C57C4C reads the convex mesh's local box off that. The game stores the address of
    // a model collision record's +0Ch field here (009393D1, 00939886), so the value the
    // shape keeps is that field's content. The record's producer is the model loader,
    // which this packet did not read.
    const void* geometry{nullptr};
    float row0[3]{1.0f, 0.0f, 0.0f};
    float row1[3]{0.0f, 1.0f, 0.0f};
    float row2[3]{0.0f, 0.0f, 1.0f};
    float position[3]{0.0f, 0.0f, 0.0f};
};

// 00931A10 builds this 48h-byte prototype on its own stack at 00931A18..00931A8C and
// 00931610 copies it into every element the resize adds. It is why a hull shape is a
// convex mesh with an identity rotation: the fill loop at 009392FA never writes the type
// or the basis rows, only the friction, the group, the mask, the geometry and the
// translation.
DynShapeDescriptor dyn_shape_descriptor_default_00931a10() noexcept;

// ---------------------------------------------------------------------------
// The AABB the shapes produce
// ---------------------------------------------------------------------------

// A min/max pair. Used both for a convex mesh's local box (mesh+18h..+2Ch) and for the
// world boxes at shape+0Ch/+18h and B+38h/+44h.
struct DynAabb {
    OceanVec3 min{};
    OceanVec3 max{};
};

// mesh+18h..+20h is the local minimum and mesh+24h..+2Ch the local maximum, read at
// 00C57C4C..00C57C93 off shape+210h.
inline constexpr std::size_t kDynConvexMeshLocalBoundsOffset = 0x18;

// The double at 00D7A2F8, subtracted from the local minimum and added to the local
// maximum at 00C57C63..00C57CBD before the box is transformed.
inline constexpr float kDynConvexMeshBoundsEpsilon = 0.02f;

// 00C57C40, `__thiscall(shape)`, RET. Vtable slot 0 of Dyn::ConvexMeshShape (00D7A0AC) and
// also a direct call from the constructor at 00C57FCE. Writes shape+0Ch..+14h and
// shape+18h..+20h, then tail-calls 00C55FC0 with the body in ESI.
//
//   lo     = mesh.min - 0.02 ; hi = mesh.max + 0.02
//   centre = (lo + hi) * 0.5            ; the double 0.5 at 00D7A280
//   half   = (hi - lo) * 0.5
//   c'     = centre * R + t             ; R the three basis rows, t the translation
//   h'.x   = |row0.x|*half.x + |row1.x|*half.y + |row2.x|*half.z  (and likewise y, z)
//   out    = { c' - h', c' + h' }
//
// This is the same conservative box rotation as dyn_body_world_bounds_00c5715c; nine
// BSP_Math_AbsFloat calls (00401170) supply the magnitudes.
DynAabb dyn_convex_mesh_shape_bounds_00c57c40(const DynShapeDescriptor& shape,
                                              const DynAabb& mesh_local) noexcept;

// 00C55FC0, the body in ESI, no stack arguments. Both call sites establish the register:
// 00C5CA8D inherits ESI from 00C5C961 (`MOV ESI,EDX`, the body argument) and 00C57F3D
// loads it from shape+04h, which 00C57F7C had set to the constructing body.
//
// B+38h..+40h start at +FLT_MAX (00D7A248) and B+44h..+4Ch at -FLT_MAX (00D7A244); every
// shape on the list at B+70h, threaded through shape+208h, merges both of its corners
// into both ends, so the result is the union of the shapes' own boxes in body space.
// Every caller runs it with at least one shape already linked, so the seed never survives
// in practice; a body that never gets a shape keeps the zeros 00C43CA0 wrote at
// 00C43E5E..00C43E77 instead, which is the zero extent docs/SHIP_HULL_BODY.md assumed.
// The broad-phase proxy refresh that follows, gated on B+50h bit 3 at 00C56182, is not
// modelled here; docs/DYN_COLLISION_PASS.md covers the same math at 00C5715C.
DynAabb dyn_body_local_bounds_00c55fc0(const DynAabb* shape_bounds, std::size_t count) noexcept;

// ---------------------------------------------------------------------------
// The model side: which nodes contribute a shape
// ---------------------------------------------------------------------------

// The four owners a collision record's node is tested against at 00939026, 00939039,
// 00939053 and 0093906C, in that order. A record matching any of them is kept.
enum class ShipHullShapeSource {
    kFirstNodeByName = 0,   // 00938F4E, the model node named "firstnode" (00D1968C)
    kModelPrimaryNode = 1,  // 00938F9D, the node pointer at model+0Ch
    kFrontByName = 2,       // 00938DB9, the model node named "front" (00D196A0)
    kBackByName = 3,        // 00938DE2, the model node named "back" (00D19698)
};

// One entry of the model's collision-record list at model+4Ch (begin +50h, end +54h,
// eight-byte entries). The entry is a pair: the record at +0h and the node that owns it at
// +4h; 0093861B/00938622 in the part loop compares that +4h against a node pointer the
// name search returned, which is what settles it as a node. The walk keeps `record+0Ch`
// (009393CB) and the float3 at `record+14h` (009393FF..00939429).
//
// The record's producer is the model loader, unread here: the file side shows the model
// carries `ConvexObject` chunks and a `GeomMesh` name table, so the mesh bounds below are
// the loader's, not this packet's.
struct ShipHullCollisionRecord {
    ShipHullShapeSource source{ShipHullShapeSource::kModelPrimaryNode};
    // The address stored into desc+14h at 009393D1, i.e. `record + 0Ch`.
    const void* geometry{nullptr};
    // record+14h..+1Ch, copied into desc+3Ch..+44h at 0093946E..0093947A.
    OceanVec3 offset{};
    // The local box of the convex mesh this record points at, mesh+18h..+2Ch. Supplied by
    // the caller because the model loader that writes it is not reconstructed.
    DynAabb mesh_local{};
};

// ---------------------------------------------------------------------------
// The collision group, mask and friction a hull shape gets
// ---------------------------------------------------------------------------

// desc+08h at 009394DD for a hull shape and 009398C4 for the periscope shape.
inline constexpr std::uint32_t kShipHullShapeGroup = 1;
inline constexpr std::uint32_t kShipHullPeriscopeGroup = 2;

// desc+0Ch. A hull shape starts at 0Dh (009394A9) and ORs in one bit chosen by the
// vehicle class's virtual slot 1Ch (009395E2). The periscope shape is 5 flat (009398D6).
inline constexpr std::uint32_t kShipHullShapeMaskBase = 0x0Du;
inline constexpr std::uint32_t kShipHullPeriscopeMask = 0x05u;

// The jump table at 00939C90, entered at 00939507 after `ADD EAX,-7 / CMP EAX,7 / JA`.
// A category outside 7..14 adds no bit at all (EDI stays 0 from 009394F7).
// `class_flag_808` is the byte at class+808h, read at 00939533 and 00939556; a non-zero
// byte doubles the bit for categories 10 and 12.
std::uint32_t ship_hull_shape_category_bit(int unit_category, bool class_flag_808) noexcept;

// desc+04h. The physics material's `Friction`, record+34h, i.e.
// settings+514h + material*38h, read at 00939365 with `FLD [EBX + EDX*8 + 514h]` where EDX
// is the material index times seven. docs/SHIP_HULL_BODY.md names the three records; the
// installed shipglobals.lua authors 0.5 for Ship and TBoat and 1.0 for Submarine.
float ship_hull_shape_friction(ShipPhysicsMaterial material) noexcept;

// ---------------------------------------------------------------------------
// The host: one pure-virtual method per native call site
// ---------------------------------------------------------------------------

// Integration boundary for everything the shape collection reaches outside itself, in the
// order 00937D3F..009399BF calls it. There are no default implementations: nothing here
// stands in for unrecovered game behaviour.
struct ShipHullShapeHost {
    virtual ~ShipHullShapeHost() = default;

    // 0071AD50 with the model in ECX, `__thiscall(model, const char* name)`. Returns the
    // node bound to that exact name in the model's sorted name index at model+7Ch, or
    // null. Called at 00938DB9 ("front"), 00938DE2 ("back"), 00938F4E ("firstnode") and
    // 009396D8 ("periszkop"). Codex owns the model routines; this is a contract, not a
    // reconstruction.
    virtual const void* model_find_node_by_name(const char* name) = 0;

    // The node pointer at model+0Ch, read at 00938F9D. Its producer is the model loader,
    // unread; the walk only compares it against a collision record's owning node.
    virtual const void* model_primary_node() = 0;

    // The model's collision-record list at model+4Ch, walked at 00938FB3..0093917F.
    // The host returns the records whose owning node is one of the four above, already
    // resolved, because 00C5C940 never sees the node itself.
    virtual std::vector<ShipHullCollisionRecord> model_hull_collision_records() = 0;

    // The vehicle class descriptor's virtual slot 1Ch, called at 009394F9 with the class
    // from unit+538h in ECX. The callee's body was not read, so this is the raw category
    // id the jump table at 00939C90 switches on, not a named enum.
    virtual int vehicle_class_category() = 0;

    // The byte at class+808h, read at 00939533 and 00939556.
    virtual bool vehicle_class_flag_808() = 0;

    // class+510h and class+514h, compared against zero at 009396F5 and 00939706. Both must
    // be <= 0 for the periscope shape to be considered. Neither field is named by any doc.
    virtual float vehicle_class_field_510() = 0;
    virtual float vehicle_class_field_514() = 0;

    // 00B6DB60 with the node in ECX, called at 0093988D for the "periszkop" node. Returns
    // the node's local 4x4; 00C336C0 at 0093989A copies it into desc+18h as a 4x3, which
    // is the only hull shape whose rotation is not the identity.
    virtual void periscope_node_local_transform(float row0[3], float row1[3], float row2[3],
                                                float position[3]) = 0;

    // The collision record bound to the "periszkop" node, found by the same list walk at
    // 009397D2..0093985B. 00939883 also stores its `record+0Ch` at controller+18h.
    virtual ShipHullCollisionRecord periscope_collision_record() = 0;
};

// What the walk needs that is not a call.
struct ShipHullShapeInputs {
    // The physics material chosen at 00937CF1..00937D38, the same index the inertia
    // multiplier uses. docs/SHIP_HULL_BODY.md.
    ShipPhysicsMaterial material{ShipPhysicsMaterial::kShip};
    // Whether the model has a node named "periszkop" at all. The gate at 0093970F.
    bool has_periscope_node{false};
};

// The descriptors the body descriptor's shape vector ends up pointing at, in push order,
// and the AABB they give the body once CreateBody has attached them.
struct ShipHullShapeSet {
    std::vector<DynShapeDescriptor> shapes;
    std::vector<DynAabb> shape_bounds;  // shape+0Ch/+18h, one per entry of `shapes`
    DynAabb body_bounds{};              // B+38h/B+44h after 00C55FC0
};

// 00938F61..009399BF as one sequence: collect the records, stamp a descriptor for each,
// add the periscope shape when the gate allows it, then run the two AABB rules the Dyn
// side runs when the shapes are attached. The result's `body_bounds` is exactly the span
// ShipHullBodyInputs::aabb_min / aabb_max wants.
//
// The native routine also resizes its descriptor vector to the hull-node count plus the
// debris group count at controller+330h (0093919D..009391BD) and then fills only the
// hull-node prefix. The surplus records are never pushed into the body descriptor, so
// they are not modelled.
ShipHullShapeSet ship_hull_shapes_collect_00938f61(const ShipHullShapeInputs& in,
                                                   ShipHullShapeHost& host);

// ---------------------------------------------------------------------------
// The node walks that do NOT produce hull shapes
// ---------------------------------------------------------------------------

// 00938042..00938D8F runs twenty times, once per part slot, on `fizika_%02d` (00CEB90C).
inline constexpr int kShipHullPartSlotCount = 20;  // CMP EAX,14h at 00938D88

// The float added to each component of a node's local position before the node is pushed
// out of the world: the double 100000.0 at 00CF81F0, applied to the `roncs_fizika` nodes
// at 00938AC1..00938B0C and to the `hajobelso` nodes at 00938EC1..00938F0C.
inline constexpr float kShipHullPartNodeBanishOffset = 100000.0f;

}  // namespace bsp
