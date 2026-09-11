#include "bsp/xlive_pipe_preimage.hpp"
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error XLive pipe machine-storage capture requires the audited MSVC Win32 x86 target
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(std::is_trivially_default_constructible_v<XLivePipeProtocolNativeState>);
static_assert(std::is_trivially_default_constructible_v<XLivePipeEncodedWideValue>);

__declspec(noinline) void capture_current_process_xlive_pipe_bytes(
    const void* source, void* destination, std::size_t count) noexcept {
    __asm {
        mov esi, source
        mov edi, destination
        mov ecx, count
        rep movsb
    }
}

XLivePipeProtocolAllocationPreimage CurrentProcessXLivePipePreimageHost::context_allocation_preimage(
    const XLivePipeProtocolNativeState& actual_allocation) {
    XLivePipeProtocolAllocationPreimage observed;
    capture_current_process_xlive_pipe_bytes(&actual_allocation, observed.data(), observed.size());
    return observed;
}
std::array<std::uint8_t, 72> CurrentProcessXLivePipePreimageHost::temporary_wide_preimage(
    const XLivePipeEncodedWideValue& actual_temporary) {
    std::array<std::uint8_t, 72> observed;
    capture_current_process_xlive_pipe_bytes(actual_temporary.bytes.data(), observed.data(), observed.size());
    return observed;
}
} // namespace bsp
