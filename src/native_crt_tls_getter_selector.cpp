#include "bsp/native_crt_tls_getter_selector.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32.
#endif

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeCrtActualTlsSetValue) == 4);

void* __cdecl select_native_crt_ptd_getter_00c05070(
    const NativeCrtPointerDecodeContext& actual_decode_context,
    const volatile std::uint32_t& actual_encoded_getter_0109de38,
    NativeCrtActualTlsSetValue const volatile& actual_tls_set_value_iat_00ce20b4) {
    const std::uint32_t initial_index =
        actual_decode_context.getter_tls_index_00e15b00;
    const auto actual_getter = actual_decode_context.tls_get_value_iat_00ce20bc;
    void* const cached = actual_getter(initial_index);
    if (cached != nullptr) return cached;

    const std::uint32_t encoded = actual_encoded_getter_0109de38;
    void* const decoded = native_crt_decode_pointer_00c04fde(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(encoded)),
        actual_decode_context);
    const std::uint32_t current_index =
        actual_decode_context.getter_tls_index_00e15b00;
    const auto actual_setter = actual_tls_set_value_iat_00ce20b4;
    (void)actual_setter(current_index, decoded);
    return decoded;
}

} // namespace bsp
