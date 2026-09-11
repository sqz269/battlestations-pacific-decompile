#include "bsp/lua_numeric.hpp"
#include "bsp/native_render_batch_keys.hpp"

namespace bsp {
float lua_object_number_00b66270(GuiLuaHost& host, GuiLuaRef object) {
    return lua_number_float32_00b66270(host.to_number(object));
}
float lua_number_float32_00b66270(double number) noexcept {
    float narrowed;
    __asm {
        fld number
        fstp narrowed
    }
    return narrowed;
}
std::int32_t lua_object_integer_00b66290(GuiLuaHost& host, GuiLuaRef object,
    const bool& crt_sse2_conversion) {
    return lua_number_integer_00b66290(host.to_number(object), crt_sse2_conversion);
}
std::int32_t lua_number_integer_00b66290(double number,
    const bool& crt_sse2_conversion) noexcept {
    float narrowed;
    double spill;
    std::int32_t result;
    const bool* const mode = &crt_sse2_conversion;
    __asm {
        fld number
        fstp narrowed
        fld narrowed
        mov edx, mode
        cmp byte ptr [edx], 0
        je x87_path
        fstp spill
        cvttsd2si eax, spill
        jmp converted
    x87_path:
        call native_x87_truncate_st0_00bf7456
    converted:
        mov result, eax
    }
    return result;
}
std::int32_t lua_float_index_00bd5790(float number) noexcept {
    std::int32_t result;
    __asm {
        cvttss2si eax, number
        mov result, eax
    }
    return result;
}
} // namespace bsp
