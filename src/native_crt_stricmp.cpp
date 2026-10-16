#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_crt_ascii_stricmp.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::int32_t) == 4,
    "Original __stricmp uses Win32 pointers and DWORD cells");

std::int32_t compare_native_crt_strings_00bf7fbf(const char* actual_left,
    const char* actual_right, const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& callees) {
    if (actual_locale_changed_0109de1c != 0)
        return callees.compare_locale_00bf7eec(actual_left, actual_right, nullptr);

    if (actual_left != nullptr && actual_right != nullptr)
        return compare_native_crt_ascii_strings_00bf7eb7(actual_left, actual_right);

    volatile std::int32_t* const actual_errno = callees.errno_cell_00bffb8b();
    *actual_errno = 0x16;
    callees.invalid_parameter_00bf66ef(0, 0, 0, 0, 0);
    return 0x7fffffff;
}

} // namespace bsp
