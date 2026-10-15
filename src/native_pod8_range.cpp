#include "bsp/native_pod8_range.hpp"

namespace bsp {
__declspec(naked) void* __fastcall copy_native_pod8_range_00419960(
    const void*, const void*, void*,
    std::uint32_t, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        cmp ecx, edx // 00419960: native equality, without pointer subtraction.
        mov eax, dword ptr [esp + 4] // 00419962: destination, even if empty.
        je finished // 00419966.
        push esi // 00419968.
        // 00419969: exact seven-byte LEA ESP,[ESP+00000000], flags unchanged.
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    next_element:
        test eax, eax // 00419970: null applies to each current destination.
        je skip_copy // 00419972: skip both source reads as well as stores.
        mov esi, dword ptr [ecx] // 00419974.
        mov dword ptr [eax], esi // 00419976: can alter next source word.
        mov esi, dword ptr [ecx + 4] // 00419978: actual second reread.
        mov dword ptr [eax + 4], esi // 0041997B.
    skip_copy:
        add ecx, 8 // 0041997E: native unsigned wrap, including skipped copy.
        add eax, 8 // 00419981.
        cmp ecx, edx // 00419984.
        jne next_element // 00419986.
        pop esi // 00419988.
    finished:
        ret 10h // 00419989: destination and three ignored metadata words.
    }
}

__declspec(naked) void __fastcall fill_n_native_pod8_range_0041c7e0(
    void*, std::uint32_t, const void*,
    std::uint32_t, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        test edx, edx // 0041C7E0: TEST clears CF; unsigned JBE means count0.
        jbe finished // 0041C7E2: preserve incoming EAX on the zero path.
        mov eax, dword ptr [esp + 4] // 0041C7E4: actual value pointer.
        push esi // 0041C7E8.
        // 0041C7E9: exact seven-byte LEA ESP,[ESP+00000000].
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    next_element:
        test ecx, ecx // 0041C7F0: null destination skips source reads.
        je skip_fill // 0041C7F2.
        mov esi, dword ptr [eax] // 0041C7F4: reread word0 each iteration.
        mov dword ptr [ecx], esi // 0041C7F6.
        mov esi, dword ptr [eax + 4] // 0041C7F8: after the first store.
        mov dword ptr [ecx + 4], esi // 0041C7FB.
    skip_fill:
        sub edx, 1 // 0041C7FE: unsigned count, no signed-count guard.
        add ecx, 8 // 0041C801: native DWORD address arithmetic.
        test edx, edx // 0041C804.
        ja next_element // 0041C806.
        pop esi // 0041C808.
    finished:
        ret 10h // 0041C809: EAX is intentionally not assigned a new result.
    }
}

__declspec(naked) void* __fastcall fill_n_native_pod8_dispatch_0041e3c0(
    std::uint32_t, std::uint32_t, void*, std::uint32_t, const void*) noexcept {
    __asm {
        push ecx // 0041E3C0: actual metadata preimage, not an invented zero.
        mov edx, dword ptr [esp + 10h] // 0041E3C1: value.
        push esi // 0041E3C5.
        mov esi, dword ptr [esp + 10h] // 0041E3C6: count.
        push edi // 0041E3CA.
        mov edi, dword ptr [esp + 10h] // 0041E3CB: first.
        mov byte ptr [esp + 8], 0 // 0041E3CF: only tag's low byte.
        mov eax, dword ptr [esp + 8] // 0041E3D4.
        push eax // 0041E3D8: tag (ECX preimage & FFFFFF00h).
        mov eax, dword ptr [esp + 1ch] // 0041E3D9: value, reloaded from caller.
        push edx // 0041E3DD: ignored1 = value.
        push ecx // 0041E3DE: ignored0 = unmodified ECX preimage.
        push eax // 0041E3DF: actual value.
        mov edx, esi // 0041E3E0: actual count.
        mov ecx, edi // 0041E3E2: actual destination.
        call fill_n_native_pod8_range_0041c7e0 // 0041E3E4: genuine body.
        lea eax, [edi + esi * 8] // 0041E3E9: modulo2^32, even count0.
        pop edi // 0041E3EC.
        pop esi // 0041E3ED.
        pop ecx // 0041E3EE: masked tag preimage.
        ret 0ch // 0041E3EF: original three stack arguments.
    }
}

__declspec(naked) void* __fastcall copy_native_pod8_dispatch_0041f4e0(
    std::uint32_t, std::uint32_t, const void*, const void*, void*) noexcept {
    __asm {
        push ecx // 0041F4E0: actual metadata preimage.
        mov edx, dword ptr [esp + 10h] // 0041F4E1: destination.
        mov byte ptr [esp], 0 // 0041F4E5: preserve upper24 tag bits.
        mov eax, dword ptr [esp] // 0041F4E9.
        push eax // 0041F4EC: masked tag.
        mov eax, dword ptr [esp + 14h] // 0041F4ED: destination again.
        push edx // 0041F4F1: ignored1 = destination.
        mov edx, dword ptr [esp + 14h] // 0041F4F2: last.
        push ecx // 0041F4F6: ignored0 = unmodified ECX preimage.
        mov ecx, dword ptr [esp + 14h] // 0041F4F7: first.
        push eax // 0041F4FB: actual destination.
        call copy_native_pod8_range_00419960 // 0041F4FC: genuine body.
        pop ecx // 0041F501: masked tag preimage; preserve kernel EAX.
        ret 0ch // 0041F502: original three stack arguments.
    }
}
} // namespace bsp
