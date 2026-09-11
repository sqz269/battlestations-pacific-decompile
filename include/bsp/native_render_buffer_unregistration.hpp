#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Actual array storage: data DWORD+0, count DWORD+4. Only these eight bytes
// are touched; an enclosing capacity DWORD+8 and stale last slot are retained.
// The second input points to one actual DWORD, read only if unsigned begin<end.
// Native ECX array, stack pointer-to-value, RET4; full EAX is 0 or 1.
std::uint32_t remove_native_renderer_vertex_pointer_00b25300(
    void* actual_array, const void* actual_value_word) noexcept;
std::uint32_t remove_native_renderer_index_pointer_00b25370(
    void* actual_array, const void* actual_value_word) noexcept;
std::uint32_t remove_native_renderer_texture_pointer_00b25580(
    void* actual_array, const void* actual_value_word) noexcept;
std::uint32_t remove_native_physical_logical_pointer_00b4b2e0(
    void* actual_array, const void* actual_value_word) noexcept;

// Native ECX renderer, stack DWORD value, RET4/EAX0 or1. Operate on actual
// arrays at renderer+1AAC, +1AB8 and +1B00 respectively. No pointee lifetime.
std::uint32_t unregister_native_renderer_vertex_stream_00b268e0(
    void* actual_renderer, std::uint32_t raw_stream) noexcept;
std::uint32_t unregister_native_renderer_index_stream_00b26900(
    void* actual_renderer, std::uint32_t raw_stream) noexcept;
std::uint32_t unregister_native_renderer_texture_00b27d40(
    void* actual_renderer, std::uint32_t raw_texture) noexcept;

// Native ECX physical owner, stack DWORD stream, RET4; EAX is not a stable
// result across optional leave. Borrow the actual global renderer pointer at
// 00F8D394 and globals at 0108D6DC; never take ownership or initialize them.
// No EH/RAII cleanup exists. If entry mode was zero, a later nonzero mode
// would consume native uninitialized saved state: that execution is excluded.
void unregister_native_physical_index_stream_00b4b390(void* actual_physical,
    std::uint32_t raw_stream, const void* volatile& actual_renderer_global,
    NativeRendererSynchronizationGlobals&);
void unregister_native_physical_vertex_stream_00b4b3f0(void* actual_physical,
    std::uint32_t raw_stream, const void* volatile& actual_renderer_global,
    NativeRendererSynchronizationGlobals&);

} // namespace bsp
