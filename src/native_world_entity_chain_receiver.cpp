#include "bsp/native_world_entity_chain_receiver.hpp"
#include "bsp/native_world_entity_chain_retirement.hpp"

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

__declspec(naked) void __fastcall retire_native_world_entities_00904390(void*) {
    __asm {
        mov ecx, dword ptr [ecx + 4]         // 00904390
        jmp retire_native_world_entity_chain_009041a0 // 00904393
    }
}

} // namespace bsp

#endif
