#include "bsp/native_crt_osplatform_getter.hpp"

#include "bsp/native_crt_pointer_decode_support.hpp"
#include "bsp/legacy_crt_math.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT OS-platform getter requires MSVC Win32.
#endif

namespace bsp {
std::int32_t get_native_crt_osplatform_00bfbab2(
    std::uint32_t* output, const NativeCrtPointerDecodeSupportContext& context) {
    auto* const captured_output = output;
    if (captured_output) {
        // BFBADA loads once; BFBAE3 stores the same tested EAX value.
        const auto platform = context.osplatform_0109dd84;
        if (platform != 0) {
            *captured_output = platform;
            return 0;
        }
    }
    auto* const current_errno = context.owning_crt.errno_location_00bffb8b();
    *current_errno = 22;
    context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
    return 22;
}
} // namespace bsp
