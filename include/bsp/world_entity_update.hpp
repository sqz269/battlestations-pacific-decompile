#pragma once
#include <cstdint>
#include <vector>

#include "bsp/unit_instance.hpp"

// The world's per-frame entity pass and the matrix-interpolator list it closes with.
// Addresses: 00904BF0, 00904600, 00905080, 008ADE00, 009037F0, 004CB030, 0042ED50.
// Evidence, uncertainty and the original ABI of each routine are in
// docs/WORLD_ENTITY_UPDATE.md and docs/WORLD_TIMED_ATTACHMENTS.md.
//
// Nothing here is a binary-compatible replacement. The native routines are
// __thiscall members of classes whose layouts are only partly recovered; the
// structs below project the fields the reconstructed rules touch. The walk
// itself, 00904BF0, is already reconstructed as
// bsp::update_world_entities_00904bf0 in bsp/unit_instance.hpp and is NOT
// duplicated here: this header adds the chain header the walk starts from and
// the interpolator pass it ends with.

namespace bsp {

// ---------------------------------------------------------------------------
// The chain header at world+4h (009037F0).
// ---------------------------------------------------------------------------
//
// bsp/world_construct.hpp records kWorldActivationChainHeadOffset = 0x04 as
// "memset only". 009037F0, which 004DE610 runs on the world immediately after
// 004CB030, allocates two zeroed 0x0C-byte blocks and stores them at world+4h
// and world+8h; 00904BF4 loads the entity chain head as [[world+4]], so the
// dword the walk reads is the first field of that block, not a field of the
// world object.

inline constexpr std::size_t kWorldChainHeaderSize = 0x0C;          // 009037FA, PUSH 0Ch
inline constexpr std::size_t kWorldSecondChainHeaderOffset = 0x08;  // 0090383A
inline constexpr std::size_t kWorldChainHeadFieldOffset = 0x00;     // 00904BF7
inline constexpr std::size_t kWorldActiveFlagOffset = 0x4A4;        // 00903802, from arg 1
inline constexpr std::size_t kWorldPendingCountOffset = 0x4A8;      // 009037FC, zeroed

// The two bytes 004DE69C passes. Both are literal 1 at the only call site.
inline constexpr int kWorldInitActiveFlag = 1; // 004DE68B, the byte stored at +4A4h

// ---------------------------------------------------------------------------
// The matrix interpolator record (00905080, 00904600).
// ---------------------------------------------------------------------------
//
// The list base is world+4B0h, already declared in bsp/world_construct.hpp as
// kWorldListOffset / kWorldListHeadOffset / kWorldListSizeOffset. It is an MSVC
// std::list with _SECURE_SCL on: node = {_Next, _Prev, value}, value at node+8h.
// The value type is 0x64 bytes and the node 0x6Ch, established twice: the
// max_size guard constant 0x28F5C28 in 00904E50 (0xFFFFFFFF / 100) and the
// PUSH 6Ch in the buy-node 009045C2.

inline constexpr std::size_t kMatrixInterpolatorRecordSize = 0x64;  // 00904E70
inline constexpr std::size_t kMatrixInterpolatorNodeSize = 0x6C;    // 009045C2
inline constexpr std::size_t kMatrixInterpolatorNodeValueOffset = 0x08; // 00904659

// Offsets within the record. Every one is settled by the producer 00905080,
// which writes the whole record; 00904600 only reads it.
inline constexpr std::size_t kMatrixInterpolatorEntityOffset = 0x00;      // 009050D3
inline constexpr std::size_t kMatrixInterpolatorTranslationOffset = 0x04; // 009050F8
inline constexpr std::size_t kMatrixInterpolatorRotationOffset = 0x10;    // 00905138
inline constexpr std::size_t kMatrixInterpolatorDurationOffset = 0x1C;    // 00905178
inline constexpr std::size_t kMatrixInterpolatorBaseMatrixOffset = 0x20;  // 0090519D
inline constexpr std::size_t kMatrixInterpolatorStartTimeOffset = 0x60;   // 009051CF

// The base matrix is copied out of the entity at +74h by 004134F0 at 0090519D,
// and 00955970 (unit vtable slot 0D8h) reads the same 16 dwords, so entity+74h
// is the entity's own local 4x4.
inline constexpr std::size_t kEntityLocalMatrixOffset = 0x74; // 00905196, 009559C8

// Entity virtuals the pass uses. kUnitOffActiveByte (entity+5Ch) and the update
// slot 0DCh are already declared in bsp/unit_instance.hpp.
inline constexpr std::size_t kEntitySetLocalMatrixVtableSlot = 0x88; // 00904ADD
inline constexpr std::size_t kEntityRefreshVtableSlot = 0xD8;        // 00904B24

// 0042ED50 clears these two bytes on a node and on every descendant. The first
// is kUnitOffPoseValid; the second has no prior name.
inline constexpr std::size_t kEntityDerivedCacheValidOffset = 0x10C; // 00904AFE
inline constexpr std::size_t kEntityChildHeadOffset = 0x48;          // 00904AF2
inline constexpr std::size_t kEntityChildNextOffset = 0x44;          // 00904B0E

// 00D7A24C, the upper clamp of the interpolation phase.
inline constexpr float kMatrixInterpolatorPhaseCeiling = 1.0f; // 00904B6D

struct MatrixInterpolatorRecord {
    std::uint32_t entity{0};          // +00h, the scene entity being moved
    float translation[3]{};           // +04h..+0Fh, the full offset at phase 1
    float rotation[3]{};              // +10h..+1Bh, x, y and z angles in radians
    float duration{0.0f};             // +1Ch, seconds
    float base[16]{};                 // +20h..+5Fh, entity+74h at registration
    float start_time{0.0f};           // +60h, DAT_00F876A4 at registration
};

// 00905080, void __thiscall(world, entity, float tx, float ty, float tz,
// float rx, float ry, float rz, float duration), RET 20h. Appends one record at
// the back of the world+4B0h list. The argument count is the RET immediate, not
// the pushes: eight dwords after the entity pointer's slot.
//
// The reconstruction takes the base matrix and the clock as parameters because
// natively they are read through the entity (004134F0 on entity+74h) and the
// global mission clock at 00F876A4.
void add_matrix_interpolator_00905080(std::vector<MatrixInterpolatorRecord>& records,
                                      std::uint32_t entity, const float translation[3],
                                      const float rotation[3], float duration,
                                      const float entity_local_matrix[16], float clock) noexcept;

// 00904670..00904B84. phase = clamp((clock - start) / duration, 0, 1). The
// native order is the division first, then the lower clamp against 0.0f with
// FCOMIP/JBE and the upper clamp against 1.0f with COMISS/JBE. Both branches
// are taken on an unordered compare, so a NaN quotient (duration 0 with the
// numerator also 0) passes through both clamps unchanged rather than becoming
// 0.0f; the reconstruction keeps that shape.
float matrix_interpolator_phase_00904670(float clock, float start, float duration) noexcept;

// The three rotation builders the pass calls, each `void __fastcall(float* dst,
// const float* angle)` natively, writing a row-major 4x4. Row 3 is left at
// {0,0,0,1} and the negated entry is computed as `-0.0f - sin` (00D7A208).
// 00B64780 is also reconstructed as build_gui_rotation_z_00b64780 in
// bsp/gui_widget_owner.hpp; this copy exists so that this module does not pull
// the GUI widget graph in.
void matrix_interpolator_rotation_x_00b64640(float dst[16], float angle) noexcept;
void matrix_interpolator_rotation_y_00b646e0(float dst[16], float angle) noexcept;
void matrix_interpolator_rotation_z_00b64780(float dst[16], float angle) noexcept;

// 009046B5..00904AD8, the composition the pass hands to the entity's vtable
// slot 88h. In native operand order:
//
//   out = ((RotZ(-phase*rz) * RotY(-phase*ry)) * RotX(-phase*rx))
//       * Translate(phase * translation) * record.base
//
// Every angle is negated after the scale (FCHS at 009047AB, 00904889, 00904991)
// and the translation is not. The four multiplies go through 00413920, whose
// byte-exact port is multiply_native_camera_matrices_00413920; this routine
// calls it, so the arithmetic is the native x87 schedule, not a re-derivation.
void matrix_interpolator_compose_009046b5(const MatrixInterpolatorRecord& record, float phase,
                                          float out[16]) noexcept;

// ---------------------------------------------------------------------------
// The pass itself.
// ---------------------------------------------------------------------------

// One method per native call site inside 00904600, in call order.
struct MatrixInterpolatorHost {
    virtual ~MatrixInterpolatorHost() = default;
    // The byte at entity+5Ch, read twice per record (0090465C and 00904B39).
    virtual bool entity_active(std::uint32_t entity) = 0;
    // entity->vtable[88h](matrix), 00904AE3.
    virtual void entity_set_local_matrix(std::uint32_t entity, const float matrix[16]) = 0;
    // 00904AF7..00904B13: clears entity+0C8h and +10Ch and runs 0042ED50 over
    // every descendant, which clears the same two bytes on each.
    virtual void entity_invalidate_subtree_pose(std::uint32_t entity) = 0;
    // entity->vtable[0D8h](), 00904B2A, no argument.
    virtual void entity_refresh(std::uint32_t entity) = 0;
};

struct MatrixInterpolatorPassResult {
    std::size_t visited{0};  // records the walk reached
    std::size_t applied{0};  // records whose entity was active on the read at 0090465C
    std::size_t retired{0};  // records erased and freed
};

// 00904600, void __thiscall(world, float), RET 4, body 00904600..00904BE2.
//
// The float argument is dead: the frame delta 00904BF0 pushes at 00904C28 is
// never read in the body, which works entirely off the mission clock at
// 00F876A4. `clock` is that value.
//
// A retired record is erased in place and the walk continues (the continue is
// in the 12 bytes at 00904BCA..00904BD5 that Ghidra leaves undisassembled after
// the _free call), so any number of records can expire in one pass.
MatrixInterpolatorPassResult run_matrix_interpolator_pass_00904600(
    MatrixInterpolatorHost& host, std::vector<MatrixInterpolatorRecord>& records, float clock);

} // namespace bsp
