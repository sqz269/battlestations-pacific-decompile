#include "bsp/native_crt_new_handler.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32.
#endif

namespace bsp {

static_assert(sizeof(void*) == 4);

std::int32_t __cdecl invoke_native_crt_new_handler_00c055b1(
    std::uint32_t requested_bytes,
    const NativeCrtPointerDecodeContext& actual_decode_context,
    const volatile std::uint32_t& actual_encoded_new_handler_0109de44) {
    const std::uint32_t encoded = actual_encoded_new_handler_0109de44;
    void* const decoded = native_crt_decode_pointer_00c04fde(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(encoded)),
        actual_decode_context);
    if (decoded == nullptr) return 0;

    using ActualInstalledHandler = std::int32_t (__cdecl*)(std::uint32_t);
    const auto actual_handler = reinterpret_cast<ActualInstalledHandler>(decoded);
    const std::uint32_t late_source_size =
        *static_cast<const volatile std::uint32_t*>(&requested_bytes);
    const std::int32_t result = actual_handler(late_source_size);
    return result != 0 ? 1 : 0;
}

} // namespace bsp
