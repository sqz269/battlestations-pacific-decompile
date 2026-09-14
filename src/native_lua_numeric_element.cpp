#include "bsp/native_lua_numeric_element.hpp"

extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua numeric element requires MSVC Win32.
#endif

namespace bsp {

static_assert(sizeof(lua_Number) == 8);
static_assert(offsetof(NativeLuaObjectStorage, owner_00) == 0);
static_assert(offsetof(NativeLuaObjectStorage, index_08) == 8);
static_assert(offsetof(NativeLuaStateStorage, state_04) == 4);

// Keep the original public stack words live until each native x87 load.
// In particular, the value's OWNER load precedes FLD, while its STATE load
// follows FLD. FILD for the key precedes both owner/state loads. Ordinary
// C++ numeric casts or entry-time argument copies lose these distinctions.
// Only the four Lua call boundaries adapt the private register conventions
// to the linked C API; each bridge restores ESP before the next native read.
__declspec(naked) void __fastcall set_native_lua_numeric_element_00b66580(
    NativeLuaObjectStorage*, void*, std::int32_t, float) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [eax + 4]
        push 2
        push ecx
        call lua_checkstack
        add esp, 8

        fild dword ptr [esp + 8]
        mov ecx, dword ptr [esi]
        mov ecx, dword ptr [ecx + 4]
        sub esp, 8
        fstp qword ptr [esp]
        push ecx
        call lua_pushnumber
        add esp, 12

        mov edx, dword ptr [esi]
        fld dword ptr [esp + 12]
        mov ecx, dword ptr [edx + 4]
        sub esp, 8
        fstp qword ptr [esp]
        push ecx
        call lua_pushnumber
        add esp, 12

        mov eax, dword ptr [esi]
        mov edx, dword ptr [esi + 8]
        mov ecx, dword ptr [eax + 4]
        push edx
        push ecx
        call lua_settable
        add esp, 8
        pop esi
        ret 8
    }
}

} // namespace bsp
