#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual Win32 header embedded in the native command/group/queue owners. No
// initialization, implicit destructor, replacement vector, or second count.
// The four-byte cells contain borrowed raw identities, never owning wrappers.
struct NativeRenderPointerArrayStorage {
    void** data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderPointerArrayStorage) == 0x0c);
static_assert(offsetof(NativeRenderPointerArrayStorage, data_00) == 0);
static_assert(offsetof(NativeRenderPointerArrayStorage, count_04) == 4);
static_assert(offsetof(NativeRenderPointerArrayStorage, capacity_08) == 8);

// The distinct source-entry specialization embedded twice in a native group.
// B1C4F0: ECX=header, EAX=same header, RET; write the three words in order.
NativeRenderPointerArrayStorage* initialize_native_instance_entry_pointers_00b1c4f0(
    NativeRenderPointerArrayStorage&) noexcept;
void reserve_native_instance_entry_pointers_00b1c500(
    NativeRenderPointerArrayStorage&, std::int32_t capacity);
void resize_native_instance_entry_pointers_00b1c770(
    NativeRenderPointerArrayStorage&, std::int32_t count);
void destroy_native_instance_entry_pointers_00b1d1d0(NativeRenderPointerArrayStorage&);

// Original ECX=actual header; signed capacity/count on stack; RET4. Reserve
// clamps capacity to at least1 and grows only, leaving excess cells unwritten.
// Resize zeroes newly exposed cells and preserves old cells on shrink. Neither
// operation retains/releases pointed objects. Allocation uses the shared actual
// BF55BE/BF6989 boundary; no per-array allocator is cached in the owner.
void reserve_native_render_group_pointers_00b1c660(
    NativeRenderPointerArrayStorage&, std::int32_t capacity);
void resize_native_render_group_pointers_00b1c7c0(
    NativeRenderPointerArrayStorage&, std::int32_t count);
void reserve_native_render_command_pointers_00b1c6c0(
    NativeRenderPointerArrayStorage&, std::int32_t capacity);
void resize_native_render_command_pointers_00b1cc80(
    NativeRenderPointerArrayStorage&, std::int32_t count);

// Original ECX=actual group-pointer header; stack=address of a four-byte pointer
// cell; RET4. Read that cell only AFTER any reserve; then publish the new slot
// and increment count. This raw byte address also permits an alias of data_00
// itself without a C++ pointer-type alias violation. The source address must
// remain valid through reserve; a cell in freed old storage is not valid.
void append_native_render_group_pointer_00b1cbe0(
    NativeRenderPointerArrayStorage&, const void* source_pointer_cell);

// Complete sibling append specializations, ECX actual header, stack address
// of a borrowed pointer cell, RET4. Reload header/count and source after any
// reserve. Native null destination skips the source load/store but increments
// count; unsigned address/count arithmetic preserves the native DWORD wrap.
void append_native_instance_entry_pointer_00b1cb80(
    NativeRenderPointerArrayStorage&, const void* source_pointer_cell);
void append_native_render_command_pointer_00b1cc20(
    NativeRenderPointerArrayStorage&, const void* source_pointer_cell);

// Original ECX=actual embedded group-pointer header; RET. Resize0 then free
// pointer storage. Retain the resulting dangling data pointer and capacity.
// Do not destroy pointed groups or implicitly end the header's typed lifetime.
void destroy_native_ordered_group_pointers_00b1d1f0(NativeRenderPointerArrayStorage&);
void destroy_native_indexed_group_pointers_00b1d260(NativeRenderPointerArrayStorage&);

// Valid native domain: nonnegative count/capacity, count<=capacity, valid spans;
// resize count nonnegative; requested four-byte allocation product and append
// doubling fit signed32. Reserve accepts negative requests through its min1
// clamp. Allocation may throw before publication; no new rollback, clamping of
// corrupt headers, concurrency policy, or owner lifetime is supplied here.
} // namespace bsp
