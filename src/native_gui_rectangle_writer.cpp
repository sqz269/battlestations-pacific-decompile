#include "bsp/native_gui_rectangle_writer.hpp"

namespace bsp {
// Call-free whole mapped writer; no invented globals or unresolved providers.
__declspec(naked) void __stdcall write_native_gui_rectangle_00acf260(
    void*, volatile std::uint32_t*, const void*, const void*) noexcept {
    __asm {
        sub esp, 0xc // 00acf260
        mov eax, dword ptr [esp + 0x10] // 00acf263
        mov edx, dword ptr [esp + 0x14] // 00acf267
        mov ecx, dword ptr [esp + 0x18] // 00acf26b
        movss xmm0, dword ptr [ecx] // 00acf26f
        movss xmm1, dword ptr [ecx + 4] // 00acf273
        push ebx // 00acf278
        push esi // 00acf279
        mov esi, dword ptr [edx] // 00acf27a
        imul esi, dword ptr [eax + 0xc] // 00acf27c
        add esi, dword ptr [eax + 0x10] // 00acf280
        push edi // 00acf283
        add esi, dword ptr [eax + 8] // 00acf284
        movss dword ptr [esi], xmm0 // 00acf287
        movss dword ptr [esi + 4], xmm1 // 00acf28b
        xorps xmm0, xmm0 // 00acf290
        movss dword ptr [esi + 8], xmm0 // 00acf293
        mov ebx, dword ptr [edx] // 00acf298
        lea esi, [ebx + 1] // 00acf29a
        mov dword ptr [edx], esi // 00acf29d
        mov edi, dword ptr [eax + 0xc] // 00acf29f
        mov esi, dword ptr [esp + 0x28] // 00acf2a2
        imul edi, ebx // 00acf2a6
        add edi, dword ptr [eax + 0x28] // 00acf2a9
        movss xmm1, dword ptr [esi] // 00acf2ac
        movss xmm2, dword ptr [esi + 4] // 00acf2b0
        add edi, dword ptr [eax + 8] // 00acf2b5
        movss dword ptr [edi], xmm1 // 00acf2b8
        movss dword ptr [edi + 4], xmm2 // 00acf2bc
        fld dword ptr [ecx + 8] // 00acf2c1
        fadd dword ptr [ecx] // 00acf2c4
        mov edi, dword ptr [edx] // 00acf2c6
        imul edi, dword ptr [eax + 0xc] // 00acf2c8
        fstp dword ptr [esp + 0xc] // 00acf2cc
        fld dword ptr [esp + 0xc] // 00acf2d0
        add edi, dword ptr [eax + 0x10] // 00acf2d4
        movss xmm1, dword ptr [ecx + 4] // 00acf2d7
        add edi, dword ptr [eax + 8] // 00acf2dc
        fstp dword ptr [edi] // 00acf2df
        movss dword ptr [edi + 4], xmm1 // 00acf2e1
        movss dword ptr [edi + 8], xmm0 // 00acf2e6
        mov ebx, dword ptr [edx] // 00acf2eb
        lea edi, [ebx + 1] // 00acf2ed
        mov dword ptr [edx], edi // 00acf2f0
        fld dword ptr [esi + 8] // 00acf2f2
        fadd dword ptr [esi] // 00acf2f5
        mov edi, dword ptr [eax + 0xc] // 00acf2f7
        movss xmm1, dword ptr [esi + 4] // 00acf2fa
        imul edi, ebx // 00acf2ff
        fstp dword ptr [esp + 0xc] // 00acf302
        fld dword ptr [esp + 0xc] // 00acf306
        add edi, dword ptr [eax + 0x28] // 00acf30a
        add edi, dword ptr [eax + 8] // 00acf30d
        fstp dword ptr [edi] // 00acf310
        movss dword ptr [edi + 4], xmm1 // 00acf312
        mov edi, dword ptr [edx] // 00acf317
        fld dword ptr [ecx + 8] // 00acf319
        fadd dword ptr [ecx] // 00acf31c
        imul edi, dword ptr [eax + 0xc] // 00acf31e
        fstp dword ptr [esp + 0xc] // 00acf322
        fld dword ptr [ecx + 4] // 00acf326
        fadd dword ptr [ecx + 0xc] // 00acf329
        add edi, dword ptr [eax + 0x10] // 00acf32c
        fstp dword ptr [esp + 0x10] // 00acf32f
        fld dword ptr [esp + 0xc] // 00acf333
        add edi, dword ptr [eax + 8] // 00acf337
        fstp dword ptr [edi] // 00acf33a
        movss dword ptr [edi + 8], xmm0 // 00acf33c
        fld dword ptr [esp + 0x10] // 00acf341
        fstp dword ptr [edi + 4] // 00acf345
        mov ebx, dword ptr [edx] // 00acf348
        lea edi, [ebx + 1] // 00acf34a
        mov dword ptr [edx], edi // 00acf34d
        fld dword ptr [esi + 8] // 00acf34f
        fadd dword ptr [esi] // 00acf352
        mov edi, dword ptr [eax + 0xc] // 00acf354
        imul edi, ebx // 00acf357
        fstp dword ptr [esp + 0xc] // 00acf35a
        fld dword ptr [esi + 4] // 00acf35e
        fadd dword ptr [esi + 0xc] // 00acf361
        fstp dword ptr [esp + 0x10] // 00acf364
        add edi, dword ptr [eax + 0x28] // 00acf368
        add edi, dword ptr [eax + 8] // 00acf36b
        fld dword ptr [esp + 0xc] // 00acf36e
        fstp dword ptr [edi] // 00acf372
        fld dword ptr [esp + 0x10] // 00acf374
        fstp dword ptr [edi + 4] // 00acf378
        mov edi, dword ptr [edx] // 00acf37b
        imul edi, dword ptr [eax + 0xc] // 00acf37d
        add edi, dword ptr [eax + 0x10] // 00acf381
        movss xmm1, dword ptr [ecx] // 00acf384
        movss xmm2, dword ptr [ecx + 4] // 00acf388
        add edi, dword ptr [eax + 8] // 00acf38d
        movss dword ptr [edi], xmm1 // 00acf390
        movss dword ptr [edi + 4], xmm2 // 00acf394
        movss dword ptr [edi + 8], xmm0 // 00acf399
        mov ebx, dword ptr [edx] // 00acf39e
        lea edi, [ebx + 1] // 00acf3a0
        mov dword ptr [edx], edi // 00acf3a3
        mov edi, dword ptr [eax + 0xc] // 00acf3a5
        movss xmm1, dword ptr [esi] // 00acf3a8
        imul edi, ebx // 00acf3ac
        add edi, dword ptr [eax + 0x28] // 00acf3af
        movss xmm2, dword ptr [esi + 4] // 00acf3b2
        add edi, dword ptr [eax + 8] // 00acf3b7
        movss dword ptr [edi], xmm1 // 00acf3ba
        movss dword ptr [edi + 4], xmm2 // 00acf3be
        fld dword ptr [ecx + 8] // 00acf3c3
        fadd dword ptr [ecx] // 00acf3c6
        mov edi, dword ptr [edx] // 00acf3c8
        imul edi, dword ptr [eax + 0xc] // 00acf3ca
        fstp dword ptr [esp + 0xc] // 00acf3ce
        fld dword ptr [ecx + 4] // 00acf3d2
        fadd dword ptr [ecx + 0xc] // 00acf3d5
        fstp dword ptr [esp + 0x10] // 00acf3d8
        add edi, dword ptr [eax + 0x10] // 00acf3dc
        fld dword ptr [esp + 0xc] // 00acf3df
        add edi, dword ptr [eax + 8] // 00acf3e3
        fstp dword ptr [edi] // 00acf3e6
        movss dword ptr [edi + 8], xmm0 // 00acf3e8
        fld dword ptr [esp + 0x10] // 00acf3ed
        fstp dword ptr [edi + 4] // 00acf3f1
        mov ebx, dword ptr [edx] // 00acf3f4
        lea edi, [ebx + 1] // 00acf3f6
        mov dword ptr [edx], edi // 00acf3f9
        fld dword ptr [esi + 8] // 00acf3fb
        fadd dword ptr [esi] // 00acf3fe
        mov edi, dword ptr [eax + 0xc] // 00acf400
        imul edi, ebx // 00acf403
        fstp dword ptr [esp + 0xc] // 00acf406
        fld dword ptr [esi + 4] // 00acf40a
        fadd dword ptr [esi + 0xc] // 00acf40d
        fstp dword ptr [esp + 0x10] // 00acf410
        add edi, dword ptr [eax + 0x28] // 00acf414
        fld dword ptr [esp + 0xc] // 00acf417
        add edi, dword ptr [eax + 8] // 00acf41b
        fstp dword ptr [edi] // 00acf41e
        fld dword ptr [esp + 0x10] // 00acf420
        fstp dword ptr [edi + 4] // 00acf424
        movss xmm1, dword ptr [ecx] // 00acf427
        fld dword ptr [ecx + 4] // 00acf42b
        fadd dword ptr [ecx + 0xc] // 00acf42e
        mov ecx, dword ptr [edx] // 00acf431
        imul ecx, dword ptr [eax + 0xc] // 00acf433
        fstp dword ptr [esp + 0x10] // 00acf437
        fld dword ptr [esp + 0x10] // 00acf43b
        add ecx, dword ptr [eax + 0x10] // 00acf43f
        add ecx, dword ptr [eax + 8] // 00acf442
        fstp dword ptr [ecx + 4] // 00acf445
        movss dword ptr [ecx], xmm1 // 00acf448
        movss dword ptr [ecx + 8], xmm0 // 00acf44c
        mov edi, dword ptr [edx] // 00acf451
        lea ecx, [edi + 1] // 00acf453
        mov dword ptr [edx], ecx // 00acf456
        fld dword ptr [esi + 4] // 00acf458
        mov ecx, dword ptr [eax + 0xc] // 00acf45b
        fadd dword ptr [esi + 0xc] // 00acf45e
        movss xmm0, dword ptr [esi] // 00acf461
        imul ecx, edi // 00acf465
        fstp dword ptr [esp + 0x10] // 00acf468
        add ecx, dword ptr [eax + 0x28] // 00acf46c
        add ecx, dword ptr [eax + 8] // 00acf46f
        movss dword ptr [ecx], xmm0 // 00acf472
        fld dword ptr [esp + 0x10] // 00acf476
        pop edi // 00acf47a
        pop esi // 00acf47b
        fstp dword ptr [ecx + 4] // 00acf47c
        pop ebx // 00acf47f
        add esp, 0xc // 00acf480
        ret 0x10 // 00acf483
    }
}
} // namespace bsp
