#include "bsp/game_native_host_crt_stricmp_callees.hpp"

#include <corecrt.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Host CRT stricmp services require MSVC Win32.
#endif
#if !defined(_DLL) || defined(_DEBUG)
#error Host CRT stricmp services require the current Release /MD CRT domain.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::int32_t) == 4);
static_assert(std::is_same_v<std::int32_t, int>);
using ErrnoEndpoint = int* (__cdecl*)();
using InvalidParameterEndpoint = void (__cdecl*)();
using CompareLocaleEndpoint = int (__cdecl*)(const char*, const char*, _locale_t);
static_assert(std::is_same_v<decltype(&::_errno), ErrnoEndpoint>);
static_assert(std::is_same_v<decltype(&::_invalid_parameter_noinfo), InvalidParameterEndpoint>);
static_assert(std::is_same_v<decltype(&::_stricmp_l), CompareLocaleEndpoint>);
} // namespace

volatile std::int32_t* GameNativeHostCrtStricmpCallees::errno_cell_00bffb8b() {
    return ::_errno();
}

void GameNativeHostCrtStricmpCallees::invalid_parameter_00bf66ef(
    std::uint32_t expression, std::uint32_t function, std::uint32_t file,
    std::uint32_t line, std::uint32_t reserved) {
    if (expression || function || file || line || reserved)
        throw std::invalid_argument("Host CRT invalid-parameter adapter requires five zero words");
    ::_invalid_parameter_noinfo();
}

std::int32_t GameNativeHostCrtStricmpCallees::compare_locale_00bf7eec(
    const char* left, const char* right, void* actual_locale) {
    if (actual_locale)
        throw std::invalid_argument("Host CRT stricmp adapter requires a null opaque locale");
    return ::_stricmp_l(left, right, nullptr);
}

} // namespace bsp::game
