#include "bsp/lua_numeric.hpp"
#include "bsp/native_render_batch_keys.hpp"

namespace bsp {
float lua_object_number_00b66270(GuiLuaHost& host, GuiLuaRef object) {
    const double number = host.to_number(object);
    float narrowed;
    __asm {
        fld number
        fstp narrowed
    }
    return narrowed;
}
std::int32_t lua_object_integer_00b66290(GuiLuaHost& host, GuiLuaRef object,
    const bool& crt_sse2_conversion) {
    const double number = host.to_number(object);
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
} // namespace bsp
