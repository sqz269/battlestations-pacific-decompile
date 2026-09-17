#include "bsp/native_lua_color.hpp"
extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {
// Keep both the integer conversion and byte publication before FLDCW. A C++
// cast would have different overflow behavior and could use SSE conversion.
void store_channel(double number, std::uint8_t* output) {
    std::uint16_t saved_control;
    std::uint16_t truncate_control;
    std::int32_t converted;
    __asm {
        fld number
        fnstcw saved_control
        mov ax, saved_control
        or ax, 0c00h
        mov truncate_control, ax
        fldcw truncate_control
        fistp converted
        mov eax, converted
        mov ecx, output
        mov byte ptr [ecx], al
        fldcw saved_control
    }
}
} // namespace

std::uint8_t* read_native_lua_color_00b67f10(
    NativeLuaObjectStorage& input, std::uint8_t* output) {
    NativeLuaObjectStorage temporary;
    constexpr std::uint32_t offsets[]{2, 1, 0, 3};
    for (std::int32_t index = 1; index <= 4; ++index) {
        native_lua_get_by_index_00b67720(input, &temporary, index);
        // The original calls lua_tonumber directly, not B66270 (float32).
        const double number = lua_tonumber(temporary.owner_00->state_04, temporary.index_08);
        store_channel(number, output + offsets[index - 1]);
        if (temporary.kind_04 != 0) {
            release_native_lua_tracked_object_00b66de0(
                temporary.owner_00, temporary, temporary.index_08, 1);
        }
    }
    return output;
}
} // namespace bsp
