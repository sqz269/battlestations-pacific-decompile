#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Opaque four-DWORD rows: this packet establishes their byte operations only.
// No member initializer, constructor, destructor, or pointed-object ownership.
struct NativeFrameTargetVectorRow {
    std::uint32_t words[4];
};
struct NativeFrameTargetVectorStorage {
    NativeFrameTargetVectorRow* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeFrameTargetVectorRow) == 0x10);
static_assert(sizeof(NativeFrameTargetVectorStorage) == 0x0c);
static_assert(offsetof(NativeFrameTargetVectorStorage, data_00) == 0);
static_assert(offsetof(NativeFrameTargetVectorStorage, count_04) == 4);
static_assert(offsetof(NativeFrameTargetVectorStorage, capacity_08) == 8);

// B1F970: ECX actual header, stack signed capacity, RET4. Clamp request to1;
// signed growth comparison; allocate wrapped DWORD(request*16) through the
// shared BF55BE/BF681B domain. Copy current rows as four ordered DWORDs, free
// current data, then publish replacement data/capacity. Count is not changed.
void reserve_native_frame_target_vector_00b1f970(
    NativeFrameTargetVectorStorage&, std::int32_t capacity);

// B1F9F0: ECX actual header, stack signed count, RET4. Reserve if needed,
// zero four DWORDs of each added row, decrement count on shrink, then store
// requested count. Shrink does not clear bytes or destroy row contents.
void resize_native_frame_target_vector_00b1f9f0(
    NativeFrameTargetVectorStorage&, std::int32_t count);

// Full B1FB90 through B1FBA6: ECX actual header, RET. Resize0, then free its
// current data through BF6989/BF65AC. Preserve dangling data and capacity.
void destroy_native_frame_target_vector_00b1fb90(NativeFrameTargetVectorStorage&);

// New MSVC Win32 C++ interfaces over the native header at FrameTargets+1C.
// Preserve DWORD wrapping, reached null-destination checks and individual
// unaligned DWORD accesses. Every reached nonnull address requires backed
// memory; overflow is not validated and may create insufficient allocation.
// Actual shared CRT/new-handler callbacks may throw or change the header;
// there is no rollback, owner lifecycle, arbitrary fault recovery or ABI shim.
} // namespace bsp
