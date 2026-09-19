#pragma once
#include <cstddef>
#include <cstdint>
namespace bsp {
// Native custom container, not the STL three-pointer vector layout.
struct NativeWordArrayStorage {std::uint16_t* data_00;std::int32_t size_04,capacity_08;};
static_assert(sizeof(NativeWordArrayStorage)==12);
static_assert(offsetof(NativeWordArrayStorage,size_04)==4);
static_assert(offsetof(NativeWordArrayStorage,capacity_08)==8);
// Original ECX=header; reserve/resize take signed DWORD target and RET4.
// Reserve clamps its request to >=1, compares signed capacity, allocates
// 32-bit-wrapping target*2 bytes, copies the live size, frees old data,
// then publishes data/capacity. It has no local allocation cleanup guard.
void reserve_native_word_array_005296a0(NativeWordArrayStorage*,std::int32_t);
// Native signed resize does not clamp to zero. Caller supplies valid backing
// for its original unchecked accesses; shrinking retains capacity/storage.
void resize_native_word_array_00529980(NativeWordArrayStorage*,std::int32_t);
// Original no stack args/RET: resize0, then free; dangling data and capacity
// remain in the header. These are explicit source interfaces.
void destroy_native_word_array_0052ad30(NativeWordArrayStorage*);
} // namespace bsp
