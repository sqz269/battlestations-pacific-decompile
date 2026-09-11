#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual CPU declaration storage: DWORD profile+00, references+04, scalar+08;
// a 0Ch array header at+0C, fifteen at+18+i*0C, and stride DWORD+CC.
// Each array header is {data pointer, signed count, signed capacity}; each
// element is five raw DWORDs (14h bytes). No companion vector or owner exists.
inline constexpr std::size_t native_vertex_declaration_bytes = 0xd0;
inline constexpr std::size_t native_vertex_declaration_slot_bytes = 0xd4;

// B47910: original ECX header, EAX same header, RET. Writes data/count/capacity
// zero in that order. These declarations otherwise expose new C++ interfaces.
void* initialize_native_vertex_elements_00b47910(void* actual_header) noexcept;
// B47A30/B480F0: original ECX header, signed stack request, RET4. Allocation
// uses the existing shared BF55BE/BF6989 domain. Products/addresses wrap DWORDs;
// only reserve clamps its request to at least one. No type/count sanitization.
void reserve_native_vertex_elements_00b47a30(void* actual_header, std::int32_t capacity);
void resize_native_vertex_elements_00b480f0(void* actual_header, std::int32_t count);
// B48AD0: resize0 then free the current data, including nullptr. Retains the
// dangling data and capacity; no destructor is called on raw element records.
void destroy_native_vertex_elements_00b48ad0(void* actual_header);

// B48AF0: original ECX raw D0h storage, EAX same address, RET. Installs native
// profile D61D1C and initializes the actual fields only. The trailing pool
// DWORD at+D0 must already contain its slab index and remains untouched.
void* construct_native_vertex_declaration_00b48af0(void* actual_storage);
// B488E0: original ECX owner, RET. Clear main then15 usage counts, zero stride,
// then perform the native live-count/type-size pass. Borrow the actual D61CC0
// table, normally18 DWORDs; unknown type values are not clamped or replaced.
void clear_native_vertex_declaration_00b488e0(
    void* actual_owner, const volatile std::uint32_t* actual_type_sizes_00d61cc0);
// B48B70: clear, destroy15 usage arrays in reverse, destroy main, restore base
// profile CEB130. C++ unwind follows the original component/iterator states.
void destroy_native_vertex_declaration_00b48b70(
    void* actual_owner, const volatile std::uint32_t* actual_type_sizes_00d61cc0);
// B48CA0: original ECX owner, stack flags, EAX original address, RET4. Return
// its slot to canonical108FD38 only after successful destruction and flags&1.
void* delete_native_vertex_declaration_00b48ca0(
    void* actual_owner, std::uint32_t flags, void* actual_pool_0108fd38,
    const volatile std::uint32_t* actual_type_sizes_00d61cc0);

// B47950: original ECX pool, stack slot, RET4. Borrow initialized actual pool
// storage: live Win32 CRITICAL_SECTION+0C, recursion+24, slab table+28, first
// free slab+34. Slot+D0 is its slab index; slab+1A80 is the WORD free-index
// array and +1AC0 its WORD count. This routine creates no substitute pool,
// initializes no lock, and installs no host callbacks or virtual table.
void return_native_vertex_declaration_slot_00b47950(
    void* actual_pool_0108fd38, void* actual_slot);
} // namespace bsp
