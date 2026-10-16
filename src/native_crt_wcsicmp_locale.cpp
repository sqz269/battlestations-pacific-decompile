#include "bsp/native_crt_wcsicmp_locale.hpp"

#include <cstddef>
#include <limits>
#include <type_traits>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
    "Original CRT cursors and locale/PTD fields are Win32 DWORDs");
static_assert(sizeof(char16_t) == 2 && sizeof(NativeCrtWcsicmpLocaleView) == 8);
static_assert(std::is_standard_layout_v<NativeCrtWcsicmpLocaleView>);
static_assert(std::is_standard_layout_v<NativeCrtWcsicmpLocaleUpdate>);
static_assert(offsetof(NativeCrtWcsicmpLocaleUpdate, locale_00) == 0);
static_assert(offsetof(NativeCrtWcsicmpLocaleUpdate, ptd_08) == 8);
static_assert(offsetof(NativeCrtWcsicmpLocaleUpdate, acquired_0c) == 12);
static_assert(sizeof(NativeCrtWcsicmpLocaleUpdate) == 16);

namespace {

void finish_locale_update(NativeCrtWcsicmpLocaleUpdate& local) {
    if (local.acquired_0c != 0) {
        const auto ptd = reinterpret_cast<std::uintptr_t>(local.ptd_08);
        auto* const own_locale =
            reinterpret_cast<volatile std::uint32_t*>(ptd + 0x70u);
        *own_locale = *own_locale & 0xfffffffdu;
    }
}

std::uint32_t fold_ascii_word(std::uint32_t unit) {
    if (unit >= 0x41u && unit <= 0x5au) unit += 0x20u;
    return unit;
}

} // namespace

std::int32_t compare_native_crt_wide_strings_l_00c0392a(
    NativeCrtWcsicmpLocaleServices& services, const char16_t* actual_left,
    const char16_t* actual_right,
    const NativeCrtWcsicmpLocaleView* requested_locale) {
    // No value initialization: explicit-locale BF7E24 leaves PTD/padding alone.
    NativeCrtWcsicmpLocaleUpdate local;
    services.locale_update_00bf7e24(services.context, local, requested_locale);

    if (actual_left == nullptr || actual_right == nullptr) {
        volatile std::int32_t* const errno_cell =
            services.errno_address_00bffb8b(services.context);
        *errno_cell = 0x16;
        services.invalid_parameter_00bf66ef(services.context, 0, 0, 0, 0, 0);
        finish_locale_update(local);
        return (std::numeric_limits<std::int32_t>::max)();
    }

    auto left = reinterpret_cast<std::uintptr_t>(actual_left);
    auto right = reinterpret_cast<std::uintptr_t>(actual_right);
    const auto locinfo = reinterpret_cast<std::uintptr_t>(local.locale_00.locinfo_00);
    const bool c_locale =
        *reinterpret_cast<const volatile std::uint32_t*>(locinfo + 0x14u) == 0;
    std::uint32_t left_unit;
    std::uint32_t right_unit;
    if (c_locale) {
        do {
            left_unit = fold_ascii_word(
                *reinterpret_cast<const volatile char16_t*>(left));
            right_unit = fold_ascii_word(
                *reinterpret_cast<const volatile char16_t*>(right));
            left += 2u;
            right += 2u;
        } while (left_unit != 0 && left_unit == right_unit);
    } else {
        do {
            const std::uint32_t left_argument =
                *reinterpret_cast<const volatile char16_t*>(left);
            left_unit = static_cast<std::uint16_t>(services.towlower_l_00bfd39f(
                services.context, left_argument, &local.locale_00));
            const std::uint32_t right_argument =
                *reinterpret_cast<const volatile char16_t*>(right);
            left += 2u;
            right_unit = static_cast<std::uint16_t>(services.towlower_l_00bfd39f(
                services.context, right_argument, &local.locale_00));
            right += 2u;
        } while (left_unit != 0 && left_unit == right_unit);
    }
    const auto difference = static_cast<std::int32_t>(left_unit)
        - static_cast<std::int32_t>(right_unit);
    finish_locale_update(local);
    return difference;
}

} // namespace bsp
