#pragma once
#include "bsp/native_logical_buffer_device_restore.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Borrow the same actual global, physical mapping services and four immutable
// physical profiles used by device restore. No buffer, lock or array is owned.
struct NativeLogicalBufferMappingContext {
    const void* volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativePhysicalBufferLockContext& actual_physical_lock;
    const NativeLogicalBufferDeviceRestoreProfiles& actual_physical_profiles;
};

// Complete native normal/returning-failure paths. Original ECX actual logical
// stream; stack count, offset, readonly DWORD (low byte consumed), RET0Ch.
// These host bindings add the existing context in EDX. Native DWORD products
// wrap. No HRESULT translation, count clamp, resource retention or rollback.
// Vertex publishes physical Lock result to actual+08, then captures it before
// guard leave. No physical owner returns the existing+08 unchanged. Index
// returns null for absent physical; its output offset is actual+0C.
void* __fastcall lock_native_logical_vertex_stream_00b49980(void* actual_logical,
    NativeLogicalBufferMappingContext&, std::uint32_t count,
    std::uint32_t offset, std::uint8_t read_only);
void* __fastcall lock_native_logical_index_stream_00b49b60(void* actual_logical,
    NativeLogicalBufferMappingContext&, std::uint32_t count,
    std::uint32_t offset, std::uint8_t read_only);

// Complete ECX actual logical stream/no stack arguments/RET; host context EDX.
// Reload physical after guard entry. Vertex clears+08 after physical Unlock;
// index has no cached mapping field to clear. No extra release or allocation.
void __fastcall unlock_native_logical_vertex_stream_00b49a80(void* actual_logical,
    NativeLogicalBufferMappingContext&);
void __fastcall unlock_native_logical_index_stream_00b49c70(void* actual_logical,
    NativeLogicalBufferMappingContext&);

// Entry captures renderer only if current mode enables entry. Cleanup uses
// that renderer and CURRENT mode/lock, including during C++ unwinding. An
// entry-disabled/exit-enabled transition reads an uninitialized native guard
// and is outside the valid domain. Four observed physical profiles only;
// numeric profile entries select existing implementations, never callbacks.
} // namespace bsp
