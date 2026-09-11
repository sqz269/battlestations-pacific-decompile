#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/world_entity_update.hpp"

// The entity's local 4x4 at +74h: the two vtable slot 88h setters that write it,
// and the vtable slot 0D8h refresh that publishes the result to the scene node.
//
// Addresses: 006E00A0, 00431410, 00955970, 0042B9A0, 0042D700.
// Evidence, original ABI and uncertainty: docs/ENTITY_LOCAL_MATRIX.md and
// docs/ENTITY_REFRESH_VIRTUAL.md.
//
// None of the three routines has a Ghidra function; all three were read from the
// disk listing and their boundaries are in the `no_ghidra_function` table of the
// two docs. Nothing here is a binary-compatible replacement: the natives are
// __thiscall members of classes whose layouts are only partly recovered, and the
// structs below project only the fields the reconstructed rules touch.
//
// Offsets already declared elsewhere are reused, not redeclared:
//   entity+74h  kEntityLocalMatrixOffset        (bsp/world_entity_update.hpp)
//   entity+0C8h kUnitOffPoseValid               (bsp/unit_instance.hpp)
//   entity+0CCh kUnitOffPoseBlock               (bsp/unit_instance.hpp)
//   entity+10Ch kEntityDerivedCacheValidOffset  (bsp/world_entity_update.hpp)
//   entity+48h  kEntityChildHeadOffset          (bsp/world_entity_update.hpp)
//   entity+44h  kEntityChildNextOffset          (bsp/world_entity_update.hpp)
//   entity+4A4h kUnitOffSceneNode               (bsp/unit_instance.hpp)
//   slot 88h    kEntitySetLocalMatrixVtableSlot (bsp/world_entity_update.hpp)
//   slot 0D8h   kEntityRefreshVtableSlot        (bsp/world_entity_update.hpp)

namespace bsp {

// ---------------------------------------------------------------------------
// Layout of the two 4x4s.
// ---------------------------------------------------------------------------
//
// 00414DB0 BSP_EntityPose_RefreshWorld is the producer of the cached matrix at
// entity+0CCh, and it settles the storage order of both matrices in one
// expression (00414DD2..00414DF1):
//
//     entity+0CCh = entity+74h * parent+0CCh
//
// A child-times-parent product is the row-vector (row-major, translation in the
// last row) convention; the column-vector convention needs parent * child. The
// translation row is corroborated independently by
// bsp/mission_entity_lua_attach.hpp, whose kEntityPoseXOffset / Y / Z are 0FCh,
// 100h and 104h, that is +0CCh plus 30h, the fourth row.

inline constexpr std::size_t kEntityMatrixFloatCount = 16; // 009559CB, MOV ECX,10h
inline constexpr std::size_t kEntityMatrixByteSize = 0x40; // 16 dwords, 009559D4 REP MOVSD
inline constexpr std::size_t kEntityMatrixRowStride = 0x10;

// Row 3 of a row-major 4x4 is the translation. In entity coordinates:
// local  translation at 74h + 30h = 0A4h, world translation at 0CCh + 30h = 0FCh.
inline constexpr std::size_t kEntityMatrixTranslationRowOffset = 0x30; // 00414DD2 chain
inline constexpr std::size_t kEntityLocalMatrixTranslationOffset = 0x0A4;

// Index of the first float of the translation row inside a 16-float matrix.
inline constexpr std::size_t kEntityMatrixTranslationIndex = 12;

// The remaining entity fields this module reads.
inline constexpr std::size_t kEntityParentOffset = 0x03C;           // 00955986, 00414DBF
inline constexpr std::size_t kEntityLocalMatrixOwnerOffset = 0x310; // 006E00D4

// Entity virtuals beyond the two the interpolator pass already names.
inline constexpr std::size_t kEntityHasDerivedLocalMatrixVtableSlot = 0x8C; // 009559B8
inline constexpr std::size_t kEntityDerivedLocalMatrixVtableSlot = 0x90;    // 009559E0
inline constexpr std::size_t kEntityLocalMatrixOwnerVtableSlot = 0x0C;      // 006E00DA

// Scene-node virtuals the refresh publishes through.
inline constexpr std::size_t kSceneNodeSetPoseVtableSlot = 0x34;        // 009559A9
inline constexpr std::size_t kSceneNodeSetWorldMatrixVtableSlot = 0x38; // 00955A0B, 00955A27

// ---------------------------------------------------------------------------
// Slot 88h: the local-matrix setter.
// ---------------------------------------------------------------------------
//
// There are exactly two implementations across the sixteen entity vtables that
// carry this interface. 00431410 is the base: copy, clear the two cache bytes,
// invalidate every child subtree. 006E00A0 is byte-for-byte the same and then
// notifies one further object. The table of vtables is in
// docs/ENTITY_LOCAL_MATRIX.md.

// One method per native call site inside the two setters, in call order.
struct EntityLocalMatrixHost {
    virtual ~EntityLocalMatrixHost() = default;
    // 004134F0 BSP_Matrix_Copy4x4X87 with ECX = entity+74h and the source
    // pushed. 006E00AC and 0043141B.
    virtual void copy_local_matrix(std::uint32_t entity, const float source[16]) = 0;
    // MOV byte [entity+0C8h],0 then MOV byte [entity+10Ch],0. The stores are
    // scheduled between the copy and the child walk in both bodies
    // (006E00B6/006E00BD, 00431420/00431427).
    virtual void clear_pose_cache_flags(std::uint32_t entity) = 0;
    // [entity+48h], the first child. 006E00B1 and 0043142E.
    virtual std::uint32_t first_child(std::uint32_t entity) = 0;
    // [child+44h], the next sibling. 006E00CD and 0043143C.
    virtual std::uint32_t next_sibling(std::uint32_t child) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose(child). 006E00C8, 00431437.
    virtual void invalidate_subtree_pose(std::uint32_t child) = 0;
    // ([entity+310h])->vtable[0Ch]() with ECX = entity+310h, not [entity+310h].
    // 006E00DD..006E00E3. Only the 006E00A0 variant reaches this.
    virtual void notify_local_matrix_owner(std::uint32_t entity) = 0;
};

// What one setter call did, for a host that wants to assert the sequence.
struct EntityLocalMatrixSetResult {
    std::size_t children_invalidated{0};
    bool owner_notified{false};
};

// 00431410, void __thiscall(entity, const float* matrix), RET 4,
// body 00431410..00431444. The base implementation, in seven of the sixteen
// vtables.
EntityLocalMatrixSetResult set_entity_local_matrix_00431410(EntityLocalMatrixHost& host,
                                                            std::uint32_t entity,
                                                            const float matrix[16]);

// 006E00A0, void __thiscall(entity, const float* matrix), RET 4,
// body 006E00A0..006E00E7. The base sequence plus the owner notification, in
// the other nine vtables, MDestroyer's 00CFC3D0 among them.
EntityLocalMatrixSetResult set_entity_local_matrix_006e00a0(EntityLocalMatrixHost& host,
                                                            std::uint32_t entity,
                                                            const float matrix[16]);

// ---------------------------------------------------------------------------
// Slot 0D8h: the refresh the interpolator pass ends each record with.
// ---------------------------------------------------------------------------

// Which of the four exits 00955970 took. The names are the branch, not a
// recovered symbol.
enum class EntityRefreshOutcome {
    kNoSceneNode,   // 00955980, [entity+4A4h] is null: nothing happens
    kParentedPose,  // 009559A9, the +0CCh world matrix goes to slot 34h
    kDerivedLocal,  // 00955A13, slot 90h composed with the local, to slot 38h
    kPlainLocal,    // 00955A2E, entity+74h itself goes to slot 38h
};

// One method per native call site inside 00955970, in call order.
struct EntityRefreshHost {
    virtual ~EntityRefreshHost() = default;
    // [entity+4A4h], the scene node. 00955979. Zero means no node.
    virtual std::uint32_t scene_node(std::uint32_t entity) = 0;
    // [entity+3Ch], the transform parent. 00955986.
    virtual std::uint32_t parent(std::uint32_t entity) = 0;
    // byte [entity+0C8h]. 0095598C.
    virtual bool world_matrix_valid(std::uint32_t entity) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld with ECX = entity. 00955995.
    // Reconstructed as refresh_pose_00414db0 in bsp/pose_refresh.hpp.
    virtual void refresh_world_matrix(std::uint32_t entity) = 0;
    // node->vtable[34h](entity+0CCh). 009559AC. The argument is the address of
    // the entity's cached world matrix, passed by pointer, not a copy.
    virtual void scene_node_set_pose(std::uint32_t node, std::uint32_t world_matrix_address) = 0;
    // entity->vtable[8Ch]() -> AL. 009559BE. 0042B9A0 (`XOR AL,AL; RET`) in all
    // sixteen vtables, so `false` is the only value the shipped image produces.
    virtual bool has_derived_local_matrix(std::uint32_t entity) = 0;
    // The 16 dwords at entity+74h, copied to a stack temporary by REP MOVSD.
    // 009559C8..009559D4.
    virtual void read_local_matrix(std::uint32_t entity, float out[16]) = 0;
    // entity->vtable[90h](&out) -> &out. 009559F0. A by-value 4x4 return: the
    // single stack argument is the caller's return buffer and the callee hands
    // it back in EAX. 0042D700 in all sixteen vtables, which writes identity.
    virtual void derived_local_matrix(std::uint32_t entity, float out[16]) = 0;
    // 00413920 BSP_Matrix_Multiply4x4, ECX = left, stack (dst, right).
    // 009559F4, with left = the slot 90h result and right = the local copy.
    virtual void multiply_matrices(float dst[16], const float left[16],
                                   const float right[16]) = 0;
    // node->vtable[38h](matrix). 00955A13 and 00955A2E.
    virtual void scene_node_set_world_matrix(std::uint32_t node, const float matrix[16]) = 0;
};

// 00955970, void __thiscall(entity), RET (no stack arguments),
// body 00955970..00955A37. Called as entity->vtable[0D8h]() at 00904B2A.
//
// The routine has no Ghidra function; the enclosing candidate 00955830 ends at
// 00955965 and 00955966..0095596F is INT3 padding.
EntityRefreshOutcome refresh_entity_00955970(EntityRefreshHost& host, std::uint32_t entity);

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 0042D700, the slot 90h body in every one of the sixteen vtables:
// `Matrix4x4 __thiscall(Matrix4x4* out)`, RET 4, body 0042D700..0042D75E. It
// ignores ECX and writes a row-major identity with SSE scalar stores, the 1.0f
// coming from 00D7A24C.
void write_identity_matrix_0042d700(float out[16]) noexcept;

// 0042B9A0, the slot 8Ch body in every one of the sixteen vtables:
// `bool __thiscall()`, `XOR AL,AL; RET`, body 0042B9A0..0042B9A2.
constexpr bool entity_has_derived_local_matrix_0042b9a0() noexcept { return false; }

} // namespace bsp
