#include "bsp/native_locale_manager_base_scalar.hpp"
#include "bsp/native_locale_manager_base_retirement.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale base scalar requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

void* delete_native_locale_manager_base_00737010(void* actual_receiver,
    const volatile std::uint32_t& actual_flags,
    const NativeLocaleManagerConstructorContext& context) {
    void* const original_receiver = actual_receiver;
    destroy_native_locale_manager_base_007365e0(original_receiver, context);
    _ReadWriteBarrier();
    const unsigned char low_flags =
        *reinterpret_cast<const volatile unsigned char*>(&actual_flags);
    _ReadWriteBarrier();
    if ((low_flags & 1u) != 0) singleton_lifetime_free(original_receiver);
    return original_receiver;
}
} // namespace bsp
