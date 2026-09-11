#pragma once

#include <cstdint>

namespace bsp {
struct NativeRenderBatchStorage;

// Complete B20240. ECX is borrowed actual renderer storage: first read DWORD
// +1D90, then (only when zero) byte +1D8A. EAX is exactly zero or one.
std::uint32_t __fastcall native_render_can_execute_command_00b20240(
    const void* actual_renderer) noexcept;

// Complete B1FE20. Reads DWORD +1998. Only AL is the result; the original
// upper 24 EAX bits and the CMP flags are retained by the hardware entry.
std::uint8_t __fastcall native_render_is_frame_active_00b1fe20(
    const void* actual_renderer) noexcept;

// Complete B20210: the concrete renderer override is only RET 4. This
// stdcall declaration supplies its ignored stack DWORD; native ECX is also
// ignored. No renderer allocation, field interpretation or callback is needed.
void __stdcall native_render_command_hook_no_op_00b20210(
    const void* ignored_argument) noexcept;

// Complete B51B20. ECX is the actual batch; returns its raw +10 count word.
// No bounds check, count normalization or mutation.
std::int32_t __fastcall native_render_batch_count_00b51b20(
    const NativeRenderBatchStorage* actual_batch) noexcept;
} // namespace bsp
