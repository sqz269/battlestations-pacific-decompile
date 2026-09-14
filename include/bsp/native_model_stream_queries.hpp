#pragma once

#include <cstdint>

namespace bsp {

// Original ECX is the actual logical stream, the index is at [ESP+4], and
// EAX=[stream+50h]+(index<<5) with 32-bit wrap; RET4. The additional ignored
// EDX DWORD in this free-function source ABI keeps the index on the stack.
// This returns a borrowed address, even when the current +50h backing is null;
// no bounds, backing, or stream check is performed.
const void* __fastcall get_native_stream_decode_record_00b61e10(
    const void* actual_stream, std::uint32_t ignored_edx,
    std::uint32_t element_index) noexcept;

// Original ECX is the actual model; EAX is signed DWORD+188h; plain RET.
// The caller of B42350 tests the count with signed JLE.
std::int32_t __fastcall get_native_model_bone_count_00b8ff00(
    const void* actual_model) noexcept;

// Original ECX is the actual model, index at [ESP+4], EAX the borrowed node
// loaded from [[model+184h]+index*4], and RET4. The extra EDX DWORD is ignored
// and preserves that stack placement in the source free-function ABI.
// Neither the table nor the selected entry is checked or retained.
void* __fastcall get_native_model_bone_node_00b90620(
    const void* actual_model, std::uint32_t ignored_edx,
    std::uint32_t bone_index) noexcept;

} // namespace bsp
