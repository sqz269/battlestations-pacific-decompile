#include "bsp/native_scene_property_reference_set.hpp"
#include "bsp/native_reference_payload_replace.hpp"

#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Whole Original58B SHA256:
// 8dac017d064d307eb6bd6f93de1476f5882325716cb81fd81a6b65ec1c707328
// The OFF arm rebinds only CALL operands [12,16) and [31,35); its other
// 50 bytes retain the original schedule.
// 00BF6989 -> 00BF65AC binds current canonical singleton_lifetime_free.
// Physical 00438E40 binds the accepted ECX/RET0 duplicate, including null.
__declspec(naked) void* __fastcall set_native_scene_property_reference_008f0420(
    void*, void*, const char*, std::uint32_t, std::uint32_t) {
    // bsp.py show 008F0420 --asm, 008F0423..008F044E, and
    // bsp.py show 008F0310 --asm: record+18h is exactly the latter's
    // scalar/+4-owned-string payload. Reuse that one existing body for
    // the free, clear, duplicate, pointer store and scalar store. This
    // is source composition of the inlined sequence, not a recovered
    // Original CALL. The live scene merge supplies the actual type-5
    // record and a separate parser-owned token; no second replacement
    // runs. Stack words are stable in this entry's declared nonaliasing,
    // nonreentrant domain. Keep the kind write and EAX record result.
    __asm {
        push esi
        mov esi, ecx
        push dword ptr [esp + 0ch] // Actual scalar, after saved ESI.
        push dword ptr [esp + 0ch] // Actual text, after scalar push.
        lea ecx, [esi + 18h]
        call replace_native_reference_payload_008f0310 // RET8.
        mov ecx, dword ptr [esp + 10h]
        mov dword ptr [esi + 8], ecx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

} // namespace bsp
