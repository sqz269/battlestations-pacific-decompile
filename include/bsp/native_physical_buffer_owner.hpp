#pragma once

#include "bsp/native_resource_support.hpp"
#include "bsp/native_string.hpp"
#include "bsp/d3d9_startup.hpp"

namespace bsp {

// Borrowed actual services and initialized pool storage. The two pool objects
// are distinct native globals, never replacement owners of the same slots.
struct NativePhysicalBufferOwnerContext {
    NativeStringStorage& actual_string_storage;
    NativeResourceSupportStorage* volatile& actual_support_0108fedc;
    // Raw mode borrows the same application's support and AA0 cells; existing
    // semantic callers retain their original domain through the adapter.
    NativeResourceSupportLifetime actual_lifetime_01090aa0;
    void* actual_index_pool_0108fda8;
    void* actual_vertex_pool_0108fde0;
};

// Complete native ECX constructors, EAX same address, RET. Borrow a 2Ch object.
// Both initialize intrusive count1 and install POOLED profiles D61E58/D61E7C.
void* construct_native_physical_index_buffer_00b4bb60(void* actual_owner) noexcept;
void* construct_native_physical_vertex_buffer_00b4bbb0(void* actual_owner) noexcept;

// Native ECX owner, stack COM/flags/byte capacity, RET Ch; no stable result.
// Publish new COM+28 before captured AddRef/Release; preserve diagnostic strings,
// actual support access, native guards and flags+14/capacity+18 ordering.
void attach_native_physical_index_buffer_00b4c250(void* actual_owner,
    IDirect3DIndexBuffer9*, std::uint32_t flags, std::uint32_t byte_capacity,
    NativePhysicalBufferOwnerContext&);
void attach_native_physical_vertex_buffer_00b4c370(void* actual_owner,
    IDirect3DVertexBuffer9*, std::uint32_t flags, std::uint32_t byte_capacity,
    NativePhysicalBufferOwnerContext&);

// Complete actual 12-byte diagnostic record cleanup: borrowed COM at+0 remains;
// current eight-byte string at+4 releases without clearing. Native ECX/RET.
void destroy_native_buffer_diagnostic_record_00b3f4c0(void* actual_record,
    NativeStringStorage&) noexcept;

// Complete native ECX/RET base and derived destructors. The derived entries
// install PRIVATE physical profiles D61E10/D61E34 even for pooled objects, call
// actual support, reload COM+28, release/clear it, then destroy the raw +08 array.
void destroy_native_physical_index_buffer_base_00b4b490(void* actual_owner);
void destroy_native_physical_vertex_buffer_base_00b4b530(void* actual_owner);
void destroy_native_physical_index_buffer_00b4b900(void* actual_owner,
    NativePhysicalBufferOwnerContext&);
void destroy_native_physical_vertex_buffer_00b4bab0(void* actual_owner,
    NativePhysicalBufferOwnerContext&);

// Complete ECX owner/stack flags/EAX original address/RET4 deleting slots.
// Bit0 selects scalar free for PRIVATE objects, or the exact actual pool for
// POOLED objects. The returned address may already be freed or reusable.
void* delete_native_private_index_buffer_00b4bb20(void*, std::uint32_t flags,
    NativePhysicalBufferOwnerContext&);
void* delete_native_private_vertex_buffer_00b4bb40(void*, std::uint32_t flags,
    NativePhysicalBufferOwnerContext&);
void* delete_native_pooled_index_buffer_00b4c210(void*, std::uint32_t flags,
    NativePhysicalBufferOwnerContext&);
void* delete_native_pooled_vertex_buffer_00b4c230(void*, std::uint32_t flags,
    NativePhysicalBufferOwnerContext&);

// Complete ECX actual pool/stack raw30h slot/RET4. Slot+2C contains its slab
// index. Borrow initialized pool+0C critical section, +24 depth, +28 slab array,
// +34 earliest available slab; each slab has32 slots, free indices+600/count+640.
void return_native_physical_buffer_slot_00b49500(void* actual_pool, void* raw_slot);

// Actual 0Ch (12-byte) array header: data+00/count+04/capacity+08.
// Native ECX header/stack requested DWORD/RET4. Signed comparisons, wrapped
// byte arithmetic, current header reloads and post-free publication survive.
void reserve_native_physical_buffer_pointer_array_00b496a0(void* actual_header,
    std::uint32_t requested_capacity);
void resize_native_physical_buffer_pointer_array_00b49700(void* actual_header,
    std::uint32_t requested_count);

// New C++ service interfaces; numeric native table identities are data tokens,
// not callable host vtables. No full pool initialization or game ABI claim.
} // namespace bsp
