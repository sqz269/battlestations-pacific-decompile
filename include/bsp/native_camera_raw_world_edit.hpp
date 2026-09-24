#pragma once
#include "bsp/native_node_base_destruction.hpp"
#include <cstdint>

namespace bsp {
// Pure immutable host metadata, disjoint from addressed actual storage. Each
// pointer borrows one live initialized caller-owned volatile DWORD array;
// no aggregate object is created over overlapping native scratch. Backing and
// metadata remain stable throughout nested calls. Do not zero preimages.
// These regions describe original local arrays, not saved registers, return
// addresses or a drop-in private-stack ABI.
struct NativeCameraDeriveLocalFrame {
    volatile std::uint32_t* words; // 32 contiguous live DWORDs
    // words[0..15]: inverse, B6E7E0 entryESP-80h, parent branch only
    // words[16..31]: local, B6E7E0 entryESP-40h, parent branch only
};
struct NativeCameraDirectionFrame {
    volatile std::uint32_t* words; // 6 contiguous live DWORDs
    // words[0..2]: position, B70660 entryESP-18h
    // words[3..5]: target, B70660 entryESP-0Ch
};
struct NativeCameraRawWorldEditFrame {
    // Disjoint immutable host metadata borrowing actual scratch. Views
    // permit the original nested overlap: for B71460 entryESP=E, derive begins
    // E-94h and direction begins E-20h (derive offset74h). Derive scratch is
    // dead before direction; only the currently live callee's outputs apply.
    NativeCameraDeriveLocalFrame derive;
    NativeCameraDirectionFrame direction;
};

// Complete normal bodies, new source interfaces. Actual node words are +30
// parent, +34 child, +3C next, +5C flags, +60 inverse, +A0 notification owner,
// +B0 local64 and +F0 world64. Camera extensions use +1A0 target3,
// +1AC direction3 and +2F0 flags. No logical hierarchy or companion adoption.
// Valid addressed storage/acyclic hierarchy, clear DF and the existing matrix
// providers' native domain are required. Partial overlaps follow native order;
// no singular fallback, validation rollback or asynchronous mutation support.
void derive_native_camera_local_00b6e7e0(
    void* actual_node, const NativeCameraDeriveLocalFrame&);

// Read current incoming pointer word once at entry. A0/current3C precedes local
// derivation; current own profile/current40 follows descendant invalidation;
// flags5C=2 is stored after that callback. Required dispatch resolves borrowed
// current tables purely and invokes the genuine exact reached targets.
void set_native_raw_world_matrix_00b6e870(void* actual_node,
    const volatile std::uint32_t& source_argument,
    const NativeCameraDeriveLocalFrame&, NativeNodeBaseWorldDispatch&);

// Exact x87 FLD/FSTP, spill and addition order. Retests current low flags bit2
// after the three direction stores. EDX is this source interface's frame, not
// an original register argument. No native exception-unwind projection.
void __fastcall refresh_native_raw_camera_direction_00b70660(
    void* actual_camera, const NativeCameraDirectionFrame&) noexcept;

// Capture incoming pointer BEFORE clearing current camera flags with FFFFFE4B;
// call raw world setter, then direction refresh, using persistent nested frames.
void set_native_raw_camera_world_00b71460(void* actual_camera,
    const volatile std::uint32_t& source_argument,
    const NativeCameraRawWorldEditFrame&, NativeNodeBaseWorldDispatch&);

// Exceptions from genuine dispatch retain the completed native prefix. These
// bodies have no native EH, allocation or owner credits. Raw B71A80 construction,
// full camera lifetime admission and game integration are outside this packet.
} // namespace bsp
