#include "bsp/native_traffic_dword_insert.hpp"

#include "bsp/native_traffic_dword_copy_fill.hpp"
#include "bsp/native_traffic_vector_failure_children.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Readable whole-body assembly preserves actual mutable argument cells,
// borrowed addresses, all reloads, wrap/SAR/flags and child call schedules.
// Calls bind to genuine registered Source providers; singleton_lifetime_free
// is the existing real current std::free boundary, not a Native CRT stub.
__declspec(naked) void* __fastcall insert_native_traffic_dwords_00493120(
    void* /* raw_header */, std::uint32_t /* source_unused_edx */,
    std::uint32_t /* opaque_first_word */, void* /* position */,
    std::uint32_t /* count */, const void* /* value_cell */) {
    __asm {
        mov eax, dword ptr [esp + 0x10] // 00493120
        push ebx // 00493124
        push esi // 00493125
        mov esi, ecx // 00493126
        mov ecx, dword ptr [eax] // 00493128
        mov dword ptr [esp + 0x18], ecx // 0049312a
        mov ecx, dword ptr [esi + 4] // 0049312e
        test ecx, ecx // 00493131
        push edi // 00493133
        jne native_0049313a // 00493134
        xor edi, edi // 00493136
        jmp native_00493142 // 00493138
    native_0049313a:
        mov edi, dword ptr [esi + 0xc] // 0049313a
        sub edi, ecx // 0049313d
        sar edi, 2 // 0049313f
    native_00493142:
        mov ebx, dword ptr [esp + 0x18] // 00493142
        test ebx, ebx // 00493146
        je native_004932ce // 00493148
        test ecx, ecx // 0049314e
        jne native_00493156 // 00493150
        xor eax, eax // 00493152
        jmp native_0049315e // 00493154
    native_00493156:
        mov eax, dword ptr [esi + 8] // 00493156
        sub eax, ecx // 00493159
        sar eax, 2 // 0049315b
    native_0049315e:
        mov edx, 0x3fffffff // 0049315e
        sub edx, eax // 00493163
        cmp edx, ebx // 00493165
        jae native_0049316e // 00493167
        call throw_native_traffic_vector_length_00490240 // 00493169
    native_0049316e:
        test ecx, ecx // 0049316e
        jne native_00493176 // 00493170
        xor eax, eax // 00493172
        jmp native_0049317e // 00493174
    native_00493176:
        mov eax, dword ptr [esi + 8] // 00493176
        sub eax, ecx // 00493179
        sar eax, 2 // 0049317b
    native_0049317e:
        add eax, ebx // 0049317e
        cmp edi, eax // 00493180
        push ebp // 00493182
        jae native_0049323c // 00493183
        mov eax, edi // 00493189
        shr eax, 1 // 0049318b
        mov edx, 0x3fffffff // 0049318d
        sub edx, eax // 00493192
        cmp edx, edi // 00493194
        jae native_0049319c // 00493196
        xor edi, edi // 00493198
        jmp native_0049319e // 0049319a
    native_0049319c:
        add edi, eax // 0049319c
    native_0049319e:
        test ecx, ecx // 0049319e
        jne native_004931a6 // 004931a0
        xor eax, eax // 004931a2
        jmp native_004931ae // 004931a4
    native_004931a6:
        mov eax, dword ptr [esi + 8] // 004931a6
        sub eax, ecx // 004931a9
        sar eax, 2 // 004931ab
    native_004931ae:
        add eax, ebx // 004931ae
        cmp edi, eax // 004931b0
        jae native_004931c7 // 004931b2
        test ecx, ecx // 004931b4
        jne native_004931bc // 004931b6
        xor eax, eax // 004931b8
        jmp native_004931c4 // 004931ba
    native_004931bc:
        mov eax, dword ptr [esi + 8] // 004931bc
        sub eax, ecx // 004931bf
        sar eax, 2 // 004931c1
    native_004931c4:
        lea edi, [eax + ebx] // 004931c4
    native_004931c7:
        xor edx, edx // 004931c7
        mov ecx, edi // 004931c9
        call allocate_native_traffic_vector_0047e970 // 004931cb
        mov ecx, dword ptr [esi + 4] // 004931d0
        mov ebp, eax // 004931d3
        mov eax, dword ptr [esp + 0x18] // 004931d5
        push ebp // 004931d9
        push eax // 004931da
        push ecx // 004931db
        mov ecx, esi // 004931dc
        call copy_native_traffic_dwords_0048d660 // 004931de
        lea edx, [esp + 0x20] // 004931e3
        push edx // 004931e7
        push ebx // 004931e8
        push eax // 004931e9
        mov ecx, esi // 004931ea
        call fill_n_native_traffic_dwords_0048e4a0 // 004931ec
        mov ecx, dword ptr [esp + 0x18] // 004931f1
        push eax // 004931f5
        mov eax, dword ptr [esi + 8] // 004931f6
        push eax // 004931f9
        push ecx // 004931fa
        mov ecx, esi // 004931fb
        call copy_native_traffic_dwords_0048d660 // 004931fd
        mov eax, dword ptr [esi + 4] // 00493202
        test eax, eax // 00493205
        jne native_0049320d // 00493207
        xor ecx, ecx // 00493209
        jmp native_00493215 // 0049320b
    native_0049320d:
        mov ecx, dword ptr [esi + 8] // 0049320d
        sub ecx, eax // 00493210
        sar ecx, 2 // 00493212
    native_00493215:
        add ebx, ecx // 00493215
        test eax, eax // 00493217
        je native_00493224 // 00493219
        push eax // 0049321b
        call singleton_lifetime_free // 0049321c
        add esp, 4 // 00493221
    native_00493224:
        lea edx, [ebp + edi*4] // 00493224
        lea eax, [ebp + ebx*4] // 00493228
        mov dword ptr [esi + 4], ebp // 0049322c
        pop ebp // 0049322f
        pop edi // 00493230
        mov dword ptr [esi + 0xc], edx // 00493231
        mov dword ptr [esi + 8], eax // 00493234
        pop esi // 00493237
        pop ebx // 00493238
        ret 0x10 // 00493239
    native_0049323c:
        mov ebp, dword ptr [esi + 8] // 0049323c
        mov edi, dword ptr [esp + 0x18] // 0049323f
        mov ecx, ebp // 00493243
        sub ecx, edi // 00493245
        sar ecx, 2 // 00493247
        lea eax, [ebx*4] // 0049324a
        cmp ecx, ebx // 00493251
        mov dword ptr [esp + 0x1c], eax // 00493253
        mov ecx, esi // 00493257
        jae native_004932a1 // 00493259
        add eax, edi // 0049325b
        push eax // 0049325d
        push ebp // 0049325e
        push edi // 0049325f
        call copy_native_traffic_dwords_0048d660 // 00493260
        mov eax, dword ptr [esi + 8] // 00493265
        mov ecx, eax // 00493268
        sub ecx, edi // 0049326a
        sar ecx, 2 // 0049326c
        lea edx, [esp + 0x20] // 0049326f
        push edx // 00493273
        sub ebx, ecx // 00493274
        push ebx // 00493276
        push eax // 00493277
        mov ecx, esi // 00493278
        call fill_n_native_traffic_dwords_0048e4a0 // 0049327a
        mov eax, dword ptr [esp + 0x1c] // 0049327f
        add dword ptr [esi + 8], eax // 00493283
        mov esi, dword ptr [esi + 8] // 00493286
        lea edx, [esp + 0x20] // 00493289
        push edx // 0049328d
        sub esi, eax // 0049328e
        push esi // 00493290
        push edi // 00493291
        call fill_native_traffic_dwords_00486800 // 00493292
        add esp, 0xc // 00493297
        pop ebp // 0049329a
        pop edi // 0049329b
        pop esi // 0049329c
        pop ebx // 0049329d
        ret 0x10 // 0049329e
    native_004932a1:
        push ebp // 004932a1
        mov ebx, ebp // 004932a2
        sub ebx, eax // 004932a4
        push ebp // 004932a6
        push ebx // 004932a7
        call copy_native_traffic_dwords_0048d660 // 004932a8
        push ebp // 004932ad
        push ebx // 004932ae
        push edi // 004932af
        mov dword ptr [esi + 8], eax // 004932b0
        call copy_native_traffic_dwords_backward_00486820 // 004932b3
        mov ecx, dword ptr [esp + 0x28] // 004932b8
        lea eax, [esp + 0x2c] // 004932bc
        push eax // 004932c0
        add ecx, edi // 004932c1
        push ecx // 004932c3
        push edi // 004932c4
        call fill_native_traffic_dwords_00486800 // 004932c5
        add esp, 0x18 // 004932ca
        pop ebp // 004932cd
    native_004932ce:
        pop edi // 004932ce
        pop esi // 004932cf
        pop ebx // 004932d0
        ret 0x10 // 004932d1
    }
}

} // namespace bsp
