#pragma once

// Leftovers after the cc13 band sweeps (cc13-leftovers1, docs/CC13_LEFTOVERS_1.md): the
// MGeomMesh triangle AABB tree build that the GeomMesh resource parser runs at load
// (00725380 -> 00724CE0 -> 007240E0, sorting through the radix sort of bsp/band_0067_cc13.hpp),
// and the unreferenced node counter 00721C60. New C++ interfaces, not the native register ABI.
// Names are hypotheses, not recovered symbols; each declaration states the original address
// and ABI. Reconstructed and build-tested only: not ABI-compatible, not fixture- or
// game-validated.

#include <array>
#include <cstdint>
#include <vector>

#include "bsp/band_0067_cc13.hpp"

namespace bsp::leftovers_cc13 {

using band_0067_cc13::Record16;
using band_0067_cc13::Vec3;

// One 2Ch-byte part of the mesh's part vector (mesh+0Ch). Only the fields the build touches.
struct GeomMeshPart {
    std::vector<std::uint16_t> triangle_list; // +14h/+18h: triangle numbers, reordered by the build
    std::vector<Vec3> node_min;               // +20h: operator_new(capacity * 0Ch)
    std::vector<Vec3> node_max;               // +24h: operator_new(capacity * 0Ch)
    std::uint32_t node_capacity{};            // +28h
};

// The MGeomMesh fields the build reads (vtable ending at 00CFDBB8, class string "MGeomMesh"
// at 00CFDBBC). A part's back-pointer at part+0Ch names this object.
struct GeomMesh {
    std::vector<GeomMeshPart> parts;                    // +0Ch/+10h, 2Ch-byte elements
    std::vector<Vec3> positions;                        // +1Ch/+20h, 0Ch-byte elements
    std::vector<std::array<std::int16_t, 3>> triangles; // +2Ch/+30h, 6-byte elements, read with MOVSX
    Vec3 bounds_min;                                    // +38h
    Vec3 bounds_max;                                    // +44h
};

// The image calls 00BF6713 (the CRT invalid-parameter report of a checked vector index; it does
// not return) when an index is out of range. The C++ stops at the same point and reports it.
enum class GeomMeshBuild { ok, invalid_parameter };

// 00724CE0's radix key: the IEEE bits of v, sign bit flipped for v >= +0 and every bit flipped
// for a set sign (00724F36..00724F48: SHR 1Fh, NOT, ADD 1, OR 80000000h, XOR). The unsigned
// order of the keys is the float order, negatives included, so the unsigned radix sort of
// 00722210 orders coordinates correctly (-0.0 sorts just below +0.0).
std::uint32_t float_radix_key_00724ce0(float v) noexcept;

// 00721C60, register ABI: ECX count, EDX leaf size, EAX result, plain RET. Returns 1 when
// count <= leaf, else f(count - (count SAR 1)) + f(count SAR 1) + 1, the node count of the tree
// that 007240E0 builds for that leaf size. No reference in the image (rel32 and absolute scan);
// Ghidra had split its tail into a bogus function at 00721C6E (repaired, see the doc). A
// negative leaf size with count in (leaf, 0] recurses without end in the image as here.
int count_tree_nodes_00721c60(int count, int leaf) noexcept;

// 007240E0, __thiscall(ECX part, node, start, count), RET 0Ch. Writes the bounds of records
// [start, start + count) to node_min/node_max[node]; when count > 20 picks the axis from the
// truncated extents, sorts the range through 007230F0 (__stdcall(axis, start, count) RET 0Ch,
// records at [00E19BD8] + start*10h, scratch at [00E19BDC]) and recurses into nodes
// 2*node + 1 (first count/2 records) and 2*node + 2 (the rest, as a loop in the image).
// records holds the n records followed by the n-record scratch area.
GeomMeshBuild build_tree_node_007240e0(const GeomMesh& mesh, GeomMeshPart& part, std::vector<Record16>& records,
                                       int node, int start, int count);

// 00724CE0, __fastcall(ECX part). Capacity = n*4/20 + 1 nodes (n = triangle_list size), both
// arrays filled with FLT_MAX / -FLT_MAX, one record per triangle (keys of the float vertex
// sums (v0 + v1) + v2, triangle number at +0Ch), 007240E0(0, 0, n), then the sorted triangle
// numbers are written back to triangle_list and the record buffer is freed.
GeomMeshBuild build_part_tree_00724ce0(const GeomMesh& mesh, GeomMeshPart& part);

// 00725380, __fastcall(ECX mesh), returns EAX of the last 00724CE0 (no meaning). Mesh bounds
// over all positions from FLT_MAX / -FLT_MAX, then 00724CE0 for every part in order. Called
// by BSP_GeomMeshResourceParser_ParsePayload (00727310).
GeomMeshBuild build_mesh_trees_00725380(GeomMesh& mesh);

// --- Landscape patch index lists (00880A60..00881A70) -------------------------------------------
// A terrain patch is a 33 x 33 vertex grid (index = row * 21h + column, rows and columns 0..32).
// For each detail level res (cells per side) the step is s = 32 / res (IDIV; res = 0 would trap).
// The builders append u16 triangle indices (three per triangle, every value masked to 16 bits)
// to one growable list; the per-edge builders take a stitch flag that drops the edge midpoints
// so the edge matches a neighbour with half the detail. Earlier docs called these lists
// "vertex-element lists"; they are index lists.

// The list object at landscape+3F4h holds a pointer to a block {data, size, capacity} whose data
// lives 16-byte aligned inside the same allocation (004E7690).
struct PatchIndexList {
    std::vector<std::uint16_t> indices; // data[0, size)
    std::int32_t capacity{};            // block+8
    bool allocated{};                   // the block pointer is non-null
};

// 004E7690, __thiscall(ECX list, new_capacity), RET 4: when capacity < new_capacity, allocates
// new_capacity*2 + 18h bytes, copies capacity*2 bytes (the old CAPACITY, not the size), keeps
// the size and frees the old block; otherwise only stores new_capacity.
void reserve_patch_index_list_004e7690(PatchIndexList& list, std::int32_t new_capacity);

// 00880A60, __thiscall(ECX list, const u16* value), RET 4: when size == capacity grows to
// capacity + capacity/4 (signed, toward zero), at least 32 when that is <= 32, then appends.
void push_patch_index_00880a60(PatchIndexList& list, std::uint16_t value);

// 00880D10, __thiscall(ECX landscape, res), RET 4, list at ECX+3F4h: the interior cells (rows and
// columns s .. s*(res-1)), two triangles per cell: (r,c) (r,c+s) (r+s,c) and (r+s,c) (r,c+s)
// (r+s,c+s). Nothing for res <= 2.
void emit_patch_interior_00880d10(PatchIndexList& list, int res);

// 00880FF0 / 00881180 / 00881350 / 00880E50, __thiscall(ECX landscape, res, char stitch),
// RET 8: the row-0, column-32, row-32 and column-0 edge strips, two cells per step.
void emit_patch_edge_row0_00880ff0(PatchIndexList& list, int res, bool stitch);
void emit_patch_edge_col32_00881180(PatchIndexList& list, int res, bool stitch);
void emit_patch_edge_row32_00881350(PatchIndexList& list, int res, bool stitch);
void emit_patch_edge_col0_00880e50(PatchIndexList& list, int res, bool stitch);

// 00881510, __thiscall(ECX landscape, res, flags), RET 8. Unreferenced: 00881A70 carries an
// inlined copy. res == 1 emits the quad 0, 20h, 420h / 420h, 20h, 440h; otherwise interior,
// then the edges with stitch bits 1 (row 0), 2 (column 32), 4 (row 32), 8 (column 0).
void emit_patch_lod_00881510(PatchIndexList& list, int res, std::uint32_t flags);

// The landscape fields 00881A70 writes.
struct LandscapeTerrainState {
    std::uint8_t byte_3e0{};
    std::uint32_t dword_3e8{};
    std::uint8_t byte_3ec{};
    std::uint8_t byte_3ed{};
    float float_3f0{};       // seeded from 00D0E658 = 9.375f
    PatchIndexList list_3f4; // appended to, never cleared here
    std::uint32_t dword_40c{};
    float float_410{};       // seeded from 00CE3AE8 = 300.0f
};

// The stitch-flag order of 00881A70: 14 of the 16 combinations (never both opposite edges, 5 or
// 10), read from the call sequence of its pseudocode (local/lo1_combos.py).
inline constexpr std::array<std::uint8_t, 14> kPatchStitchOrder_00881a70{0, 1, 2, 4, 8, 3, 6, 12, 9, 7, 14, 13, 11, 15};

// 00881A70 BSP_Landscape_ResetTerrainState, __fastcall(ECX landscape). Resets the fields, then for
// res = 2, 4, 8, 16, 32 appends the 14 stitch variants in kPatchStitchOrder_00881a70 (interior,
// row 0, column 32, row 32, column 0 each). Its res == 1 arm (14 copies of the 00881510 quad)
// is unreachable because res starts at 2.
void reset_terrain_state_00881a70(LandscapeTerrainState& state);

} // namespace bsp::leftovers_cc13
