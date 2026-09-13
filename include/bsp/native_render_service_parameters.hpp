#pragma once

#include <cstdint>

namespace bsp {

// Borrow the current DWORD at each original constant address. The referents
// may alias the destination, including words changed earlier by this function.
// These are raw MOVSS inputs: no float conversion or constant substitution.
// The reference bindings themselves must remain fixed for the call and must
// not occupy the destination's writable 68h bytes.
struct NativeRenderServiceParameterConstants {
    const volatile std::uint32_t& actual_00ce4788;
    const volatile std::uint32_t& actual_00d5e138;
    const volatile std::uint32_t& actual_00ce3854;
    const volatile std::uint32_t& actual_00d5e134;
    const volatile std::uint32_t& actual_00ce3850;
    const volatile std::uint32_t& actual_00ce3958;
    const volatile std::uint32_t& actual_00cef1b0;
    const volatile std::uint32_t& actual_00ce54a0;
    const volatile std::uint32_t& actual_00ce7804;
    const volatile std::uint32_t& actual_00ce69c8;
    const volatile std::uint32_t& actual_00ced318;
    const volatile std::uint32_t& actual_00cf2548;
    const volatile std::uint32_t& actual_00cef0b8;
    const volatile std::uint32_t& actual_00d7a2f0;
    const volatile std::uint32_t& actual_00d5e130;
    const volatile std::uint32_t& actual_00ce3930;
    const volatile std::uint32_t& actual_00d5e12c;
    const volatile std::uint32_t& actual_00ce380c;
    const volatile std::uint32_t& actual_00ce3800;
    const volatile std::uint32_t& actual_00d7a24c;
    const volatile std::uint32_t& actual_00ce3d34;
};

// Complete 00B0CD80..00B0CEAB, 300 bytes. Native ECX writable 68h subobject,
// zero stack arguments, EAX original receiver, RET. Sole observed caller
// 00B14BEF passes actual render service+2ACh. Preserve all 21 interleaved
// constant loads and 26 nonascending DWORD stores, including register reuse.
// No calls, ownership, parameter meanings, null checks or synchronization are
// added. This new C++ context interface does not construct the larger service
// and is not an original-caller ABI replacement or gameplay validation.
void* initialize_native_render_service_parameters_00b0cd80(void* actual_storage,
    const NativeRenderServiceParameterConstants&) noexcept;

} // namespace bsp
