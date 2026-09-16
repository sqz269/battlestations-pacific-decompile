#include "bsp/native_particle_text_helpers.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle text helpers require MSVC Win32.
#endif

namespace bsp {
namespace {
const char* token_text(const void* token) noexcept {
    const char* value;
    std::memcpy(&value, token, sizeof value);
    return value;
}
} // namespace

float parse_native_particle_token_float_00aedf60(const void* token) {
    const char* const text = token_text(token);
    // Call directly into the CRT's ST0-returning ABI: a C++ double temporary
    // would introduce an extra binary64 store before the native binary32 store.
    using Atof = double (__cdecl*)(const char*);
    Atof const convert = &std::atof;
    float rounded;
    __asm {
        push text
        call convert
        fstp rounded
        add esp, 4
    }
    return rounded;
}

std::int32_t count_native_particle_token_fields_00af3e90(const void* token) noexcept {
    auto* const text = reinterpret_cast<const volatile std::int8_t*>(token_text(token));
    if (text == nullptr) return 0;
    std::uint32_t length = 0;
    while (text[length] != 0) ++length;
    std::uint32_t index = 0;
    std::uint32_t spaces = 0;
    if (text[0] < 0x20) return 1;
    for (;;) {
        const auto current = text[index];
        if (current >= 0x7f || static_cast<std::int32_t>(index) >= static_cast<std::int32_t>(length))
            return static_cast<std::int32_t>(spaces + 1u);
        std::int8_t lower_bound;
        if (current == 0x20) {
            ++spaces;
            for (;;) {
                lower_bound = text[index];
                // AF3ED4 -> AF3EE6 reuses this CMP's signed flags when the
                // run ends on a nonspace; do not reread that byte.
                if (lower_bound != 0x20) break;
                ++index;
                if (static_cast<std::int32_t>(index) >= static_cast<std::int32_t>(length)) {
                    lower_bound = text[index];
                    break;
                }
            }
        } else {
            ++index;
            lower_bound = text[index];
        }
        if (lower_bound < 0x20) return static_cast<std::int32_t>(spaces + 1u);
    }
}
} // namespace bsp
