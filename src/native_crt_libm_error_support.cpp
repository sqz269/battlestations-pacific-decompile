#include "bsp/native_crt_libm_error_support.hpp"

#include "bsp/legacy_crt_math.hpp"
#include "bsp/native_crt_pointer_decode.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT libm error support requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CameraAxesCrtException) == 32);
static_assert(offsetof(CameraAxesCrtException, type) == 0);
static_assert(offsetof(CameraAxesCrtException, name) == 4);
static_assert(offsetof(CameraAxesCrtException, argument1) == 8);
static_assert(offsetof(CameraAxesCrtException, argument2) == 16);
static_assert(offsetof(CameraAxesCrtException, result) == 24);
static_assert(sizeof(NativeCrtLibmErrorSupportContext) == 20);
static_assert(sizeof(NativeCrtLibmNameIdentities) == 56);
static_assert(offsetof(NativeCrtLibmNameIdentities, tan_00d571a0) == 0);
static_assert(offsetof(NativeCrtLibmNameIdentities, sin_00d571b4) == 4);
static_assert(offsetof(NativeCrtLibmNameIdentities, pow_00d571c4) == 8);
static_assert(offsetof(NativeCrtLibmNameIdentities, modf_00d571c8) == 12);
static_assert(offsetof(NativeCrtLibmNameIdentities, log_00d571d0) == 16);
static_assert(offsetof(NativeCrtLibmNameIdentities, log10_00d571d4) == 20);
static_assert(offsetof(NativeCrtLibmNameIdentities, floor_00d571f4) == 24);
static_assert(offsetof(NativeCrtLibmNameIdentities, exp_00d571fc) == 28);
static_assert(offsetof(NativeCrtLibmNameIdentities, cos_00d57204) == 32);
static_assert(offsetof(NativeCrtLibmNameIdentities, ceil_00d57210) == 36);
static_assert(offsetof(NativeCrtLibmNameIdentities, atan_00d57218) == 40);
static_assert(offsetof(NativeCrtLibmNameIdentities, asin_00d57228) == 44);
static_assert(offsetof(NativeCrtLibmNameIdentities, acos_00d57230) == 48);
static_assert(offsetof(NativeCrtLibmNameIdentities, exp10_00d6a968) == 52);
static_assert(offsetof(LegacyCrtMathRuntime, errno_location_00bffb8b) == 4);
} // namespace
// Defined below from compiled in-body label offsets; the immutable COFF proof
// checks all thirteen relocations and final targets against the complete body.
extern const unsigned char* const libm_selector_table[13];

// Address labels are retained for complete emitted-instruction evidence.
#pragma warning(push)
#pragma warning(disable: 4102)

__declspec(naked) int __cdecl native_crt_default_matherr_00c28545(
    CameraAxesCrtException*) {
    __asm {
        xor eax, eax // 00c28545
        ret          // 00c28547
    }
}

__declspec(naked) void __cdecl native_crt_libm_error_support_00c0f0e4(
    const double*, const double*, double*, std::int32_t,
    const NativeCrtLibmErrorSupportContext&) {
    __asm {
    l00c0f0e4:
        push ebp // 00c0f0e4
    l00c0f0e5:
        mov ebp,esp // 00c0f0e5
    l00c0f0e7:
        sub esp,028h // 00c0f0e7
    l00c0f0ea:
        xor eax,eax // 00c0f0ea
    l00c0f0ec:
        mov ecx, dword ptr [ebp + 18h] // 00c0f0ec
        mov ecx, dword ptr [ecx] // 00c0f0ec
        cmp dword ptr [ecx], eax // 00c0f0ec
    l00c0f0f2:
        push ebx // 00c0f0f2
    l00c0f0f3:
        mov ebx,dword ptr [ebp + 0ch] // 00c0f0f3
    l00c0f0f6:
        push esi // 00c0f0f6
    l00c0f0f7:
        mov esi,dword ptr [ebp + 010h] // 00c0f0f7
    l00c0f0fa:
        push edi // 00c0f0fa
    l00c0f0fb:
        mov edi,dword ptr [ebp + 08h] // 00c0f0fb
    l00c0f0fe:
        mov byte ptr [ebp - 08h],al // 00c0f0fe
    l00c0f101:
        mov byte ptr [ebp - 07h],al // 00c0f101
    l00c0f104:
        mov byte ptr [ebp - 06h],al // 00c0f104
    l00c0f107:
        mov byte ptr [ebp - 05h],al // 00c0f107
    l00c0f10a:
        mov byte ptr [ebp - 04h],al // 00c0f10a
    l00c0f10d:
        mov byte ptr [ebp - 03h],al // 00c0f10d
    l00c0f110:
        mov byte ptr [ebp - 02h],al // 00c0f110
    l00c0f113:
        mov byte ptr [ebp - 01h],al // 00c0f113
    l00c0f116:
        jz l00c0f126 // 00c0f116
    l00c0f118:
        mov ecx, dword ptr [ebp + 18h] // 00c0f118
        mov edx, dword ptr [ecx + 4] // 00c0f118
        push dword ptr [ecx + 8] // 00c0f118
        push dword ptr [edx] // 00c0f118
    l00c0f11e:
        call native_crt_decode_pointer_00c04fde // 00c0f11e
    l00c0f123:
        pop ecx // 00c0f123
        pop ecx // 00c0f123
    l00c0f124:
        jmp l00c0f12b // 00c0f124
    l00c0f126:
        mov eax, offset native_crt_default_matherr_00c28545 // 00c0f126
    l00c0f12b:
        mov ecx,dword ptr [ebp + 014h] // 00c0f12b
    l00c0f12e:
        mov edx,0a6h // 00c0f12e
    l00c0f133:
        cmp ecx,edx // 00c0f133
    l00c0f135:
        jg l00c0f2af // 00c0f135
    l00c0f13b:
        jz l00c0f29c // 00c0f13b
    l00c0f141:
        cmp ecx,019h // 00c0f141
    l00c0f144:
        jg l00c0f242 // 00c0f144
    l00c0f14a:
        jz l00c0f239 // 00c0f14a
    l00c0f150:
        mov edx,ecx // 00c0f150
    l00c0f152:
        push 02h // 00c0f152
    l00c0f154:
        pop ecx // 00c0f154
    l00c0f155:
        sub edx,ecx // 00c0f155
    l00c0f157:
        jz l00c0f22a // 00c0f157
    l00c0f15d:
        dec edx // 00c0f15d
    l00c0f15e:
        jz l00c0f221 // 00c0f15e
    l00c0f164:
        sub edx,05h // 00c0f164
    l00c0f167:
        jz l00c0f212 // 00c0f167
    l00c0f16d:
        dec edx // 00c0f16d
    l00c0f16e:
        jz l00c0f1fa // 00c0f16e
    l00c0f174:
        sub edx,05h // 00c0f174
    l00c0f177:
        jz l00c0f1ea // 00c0f177
    l00c0f179:
        dec edx // 00c0f179
    l00c0f17a:
        jz l00c0f1c1 // 00c0f17a
    l00c0f17c:
        sub edx,09h // 00c0f17c
    l00c0f17f:
        jnz l00c0f359 // 00c0f17f
    l00c0f185:
        mov dword ptr [ebp - 028h],03h // 00c0f185
    l00c0f18c:
        mov edx, dword ptr [ebp + 18h] // 00c0f18c
        mov edx, dword ptr [edx + 10h] // 00c0f18c
        mov edx, dword ptr [edx + 8] // 00c0f18c
        mov dword ptr [ebp - 24h], edx // 00c0f18c
    l00c0f193:
        fld qword ptr [edi] // 00c0f193
    l00c0f195:
        lea ecx,[ebp - 028h] // 00c0f195
    l00c0f198:
        fstp qword ptr [ebp - 020h] // 00c0f198
    l00c0f19b:
        push ecx // 00c0f19b
    l00c0f19c:
        fld qword ptr [ebx] // 00c0f19c
    l00c0f19e:
        fstp qword ptr [ebp - 018h] // 00c0f19e
    l00c0f1a1:
        fld qword ptr [esi] // 00c0f1a1
    l00c0f1a3:
        fstp qword ptr [ebp - 010h] // 00c0f1a3
    l00c0f1a6:
        call eax // 00c0f1a6
    l00c0f1a8:
        test eax,eax // 00c0f1a8
    l00c0f1aa:
        pop ecx // 00c0f1aa
    l00c0f1ab:
        jnz l00c0f354 // 00c0f1ab
    l00c0f1b1:
        mov eax, dword ptr [ebp + 18h] // 00c0f1b1
        mov eax, dword ptr [eax + 0Ch] // 00c0f1b1
        call dword ptr [eax + 4] // 00c0f1b1
    l00c0f1b6:
        mov dword ptr [eax],022h // 00c0f1b6
    l00c0f1bc:
        jmp l00c0f354 // 00c0f1bc
    l00c0f1c1:
        mov edx, dword ptr [ebp + 18h] // 00c0f1c1
        mov edx, dword ptr [edx + 10h] // 00c0f1c1
        mov edx, dword ptr [edx + 28] // 00c0f1c1
        mov dword ptr [ebp - 24h], edx // 00c0f1c1
    l00c0f1c8:
        fld qword ptr [edi] // 00c0f1c8
    l00c0f1ca:
        lea ecx,[ebp - 028h] // 00c0f1ca
    l00c0f1cd:
        fstp qword ptr [ebp - 020h] // 00c0f1cd
    l00c0f1d0:
        push ecx // 00c0f1d0
    l00c0f1d1:
        fld qword ptr [ebx] // 00c0f1d1
    l00c0f1d3:
        mov dword ptr [ebp - 028h],04h // 00c0f1d3
    l00c0f1da:
        fstp qword ptr [ebp - 018h] // 00c0f1da
    l00c0f1dd:
        fld qword ptr [esi] // 00c0f1dd
    l00c0f1df:
        fstp qword ptr [ebp - 010h] // 00c0f1df
    l00c0f1e2:
        call eax // 00c0f1e2
    l00c0f1e4:
        pop ecx // 00c0f1e4
    l00c0f1e5:
        jmp l00c0f354 // 00c0f1e5
    l00c0f1ea:
        mov dword ptr [ebp - 028h],03h // 00c0f1ea
    l00c0f1f1:
        mov edx, dword ptr [ebp + 18h] // 00c0f1f1
        mov edx, dword ptr [edx + 10h] // 00c0f1f1
        mov edx, dword ptr [edx + 28] // 00c0f1f1
        mov dword ptr [ebp - 24h], edx // 00c0f1f1
    l00c0f1f8:
        jmp l00c0f193 // 00c0f1f8
    l00c0f1fa:
        mov edx, dword ptr [ebp + 18h] // 00c0f1fa
        mov edx, dword ptr [edx + 10h] // 00c0f1fa
        mov edx, dword ptr [edx + 20] // 00c0f1fa
        mov dword ptr [ebp - 24h], edx // 00c0f1fa
    l00c0f201:
        fld qword ptr [edi] // 00c0f201
    l00c0f203:
        fstp qword ptr [ebp - 020h] // 00c0f203
    l00c0f206:
        fld qword ptr [ebx] // 00c0f206
    l00c0f208:
        fstp qword ptr [ebp - 018h] // 00c0f208
    l00c0f20b:
        fld qword ptr [esi] // 00c0f20b
    l00c0f20d:
        jmp l00c0f334 // 00c0f20d
    l00c0f212:
        mov dword ptr [ebp - 028h],ecx // 00c0f212
    l00c0f215:
        mov edx, dword ptr [ebp + 18h] // 00c0f215
        mov edx, dword ptr [edx + 10h] // 00c0f215
        mov edx, dword ptr [edx + 20] // 00c0f215
        mov dword ptr [ebp - 24h], edx // 00c0f215
    l00c0f21c:
        jmp l00c0f193 // 00c0f21c
    l00c0f221:
        mov edx, dword ptr [ebp + 18h] // 00c0f221
        mov edx, dword ptr [edx + 10h] // 00c0f221
        mov edx, dword ptr [edx + 16] // 00c0f221
        mov dword ptr [ebp - 24h], edx // 00c0f221
    l00c0f228:
        jmp l00c0f201 // 00c0f228
    l00c0f22a:
        mov dword ptr [ebp - 028h],ecx // 00c0f22a
    l00c0f22d:
        mov edx, dword ptr [ebp + 18h] // 00c0f22d
        mov edx, dword ptr [edx + 10h] // 00c0f22d
        mov edx, dword ptr [edx + 16] // 00c0f22d
        mov dword ptr [ebp - 24h], edx // 00c0f22d
    l00c0f234:
        jmp l00c0f193 // 00c0f234
    l00c0f239:
        mov edx, dword ptr [ebp + 18h] // 00c0f239
        mov edx, dword ptr [edx + 10h] // 00c0f239
        mov edx, dword ptr [edx + 8] // 00c0f239
        mov dword ptr [ebp - 24h], edx // 00c0f239
    l00c0f240:
        jmp l00c0f1c8 // 00c0f240
    l00c0f242:
        sub ecx,01ah // 00c0f242
    l00c0f245:
        jz l00c0f295 // 00c0f245
    l00c0f247:
        dec ecx // 00c0f247
    l00c0f248:
        jz l00c0f289 // 00c0f248
    l00c0f24a:
        dec ecx // 00c0f24a
    l00c0f24b:
        jz l00c0f27d // 00c0f24b
    l00c0f24d:
        dec ecx // 00c0f24d
    l00c0f24e:
        jz l00c0f270 // 00c0f24e
    l00c0f250:
        sub ecx,01dh // 00c0f250
    l00c0f253:
        jz l00c0f267 // 00c0f253
    l00c0f255:
        sub ecx,03h // 00c0f255
    l00c0f258:
        jnz l00c0f359 // 00c0f258
    l00c0f25e:
        mov edx, dword ptr [ebp + 18h] // 00c0f25e
        mov edx, dword ptr [edx + 10h] // 00c0f25e
        mov edx, dword ptr [edx + 44] // 00c0f25e
        mov dword ptr [ebp - 24h], edx // 00c0f25e
    l00c0f265:
        jmp l00c0f201 // 00c0f265
    l00c0f267:
        mov edx, dword ptr [ebp + 18h] // 00c0f267
        mov edx, dword ptr [edx + 10h] // 00c0f267
        mov edx, dword ptr [edx + 48] // 00c0f267
        mov dword ptr [ebp - 24h], edx // 00c0f267
    l00c0f26e:
        jmp l00c0f201 // 00c0f26e
    l00c0f270:
        mov edx, dword ptr [ebp + 18h] // 00c0f270
        mov edx, dword ptr [edx + 10h] // 00c0f270
        mov edx, dword ptr [edx + 8] // 00c0f270
        mov dword ptr [ebp - 24h], edx // 00c0f270
    l00c0f277:
        fld qword ptr [edi] // 00c0f277
    l00c0f279:
        fstp qword ptr [esi] // 00c0f279
    l00c0f27b:
        jmp l00c0f201 // 00c0f27b
    l00c0f27d:
        mov edx, dword ptr [ebp + 18h] // 00c0f27d
        mov edx, dword ptr [edx + 10h] // 00c0f27d
        mov edx, dword ptr [edx + 8] // 00c0f27d
        mov dword ptr [ebp - 24h], edx // 00c0f27d
    l00c0f284:
        jmp l00c0f201 // 00c0f284
    l00c0f289:
        mov dword ptr [ebp - 028h],02h // 00c0f289
    l00c0f290:
        jmp l00c0f18c // 00c0f290
    l00c0f295:
        fld1 // 00c0f295
    l00c0f297:
        jmp l00c0f357 // 00c0f297
    l00c0f29c:
        mov dword ptr [ebp - 028h],03h // 00c0f29c
    l00c0f2a3:
        mov edx, dword ptr [ebp + 18h] // 00c0f2a3
        mov edx, dword ptr [edx + 10h] // 00c0f2a3
        mov edx, dword ptr [edx + 52] // 00c0f2a3
        mov dword ptr [ebp - 24h], edx // 00c0f2a3
    l00c0f2aa:
        jmp l00c0f193 // 00c0f2aa
    l00c0f2af:
        add ecx,0fffffc18h // 00c0f2af
    l00c0f2b5:
        cmp ecx,0ch // 00c0f2b5
    l00c0f2b8:
        ja l00c0f359 // 00c0f2b8
    l00c0f2be:
        jmp dword ptr [libm_selector_table + ecx*4] // 00c0f2be
    l00c0f2c5:
        mov edx, dword ptr [ebp + 18h] // 00c0f2c5
        mov edx, dword ptr [edx + 10h] // 00c0f2c5
        mov edx, dword ptr [edx + 16] // 00c0f2c5
        mov dword ptr [ebp - 24h], edx // 00c0f2c5
    l00c0f2cc:
        jmp l00c0f277 // 00c0f2cc
    l00c0f2ce:
        mov edx, dword ptr [ebp + 18h] // 00c0f2ce
        mov edx, dword ptr [edx + 10h] // 00c0f2ce
        mov edx, dword ptr [edx + 20] // 00c0f2ce
        mov dword ptr [ebp - 24h], edx // 00c0f2ce
    l00c0f2d5:
        jmp l00c0f277 // 00c0f2d5
    l00c0f2d7:
        mov edx, dword ptr [ebp + 18h] // 00c0f2d7
        mov edx, dword ptr [edx + 10h] // 00c0f2d7
        mov edx, dword ptr [edx + 28] // 00c0f2d7
        mov dword ptr [ebp - 24h], edx // 00c0f2d7
    l00c0f2de:
        jmp l00c0f277 // 00c0f2de
    l00c0f2e0:
        mov edx, dword ptr [ebp + 18h] // 00c0f2e0
        mov edx, dword ptr [edx + 10h] // 00c0f2e0
        mov edx, dword ptr [edx + 40] // 00c0f2e0
        mov dword ptr [ebp - 24h], edx // 00c0f2e0
    l00c0f2e7:
        jmp l00c0f277 // 00c0f2e7
    l00c0f2e9:
        mov edx, dword ptr [ebp + 18h] // 00c0f2e9
        mov edx, dword ptr [edx + 10h] // 00c0f2e9
        mov edx, dword ptr [edx + 36] // 00c0f2e9
        mov dword ptr [ebp - 24h], edx // 00c0f2e9
    l00c0f2f0:
        jmp l00c0f277 // 00c0f2f0
    l00c0f2f2:
        mov edx, dword ptr [ebp + 18h] // 00c0f2f2
        mov edx, dword ptr [edx + 10h] // 00c0f2f2
        mov edx, dword ptr [edx + 24] // 00c0f2f2
        mov dword ptr [ebp - 24h], edx // 00c0f2f2
    l00c0f2f9:
        jmp l00c0f277 // 00c0f2f9
    l00c0f2fe:
        mov edx, dword ptr [ebp + 18h] // 00c0f2fe
        mov edx, dword ptr [edx + 10h] // 00c0f2fe
        mov edx, dword ptr [edx + 12] // 00c0f2fe
        mov dword ptr [ebp - 24h], edx // 00c0f2fe
    l00c0f305:
        jmp l00c0f277 // 00c0f305
    l00c0f30a:
        mov edx, dword ptr [ebp + 18h] // 00c0f30a
        mov edx, dword ptr [edx + 10h] // 00c0f30a
        mov edx, dword ptr [edx + 4] // 00c0f30a
        mov dword ptr [ebp - 24h], edx // 00c0f30a
    l00c0f311:
        jmp l00c0f323 // 00c0f311
    l00c0f313:
        mov edx, dword ptr [ebp + 18h] // 00c0f313
        mov edx, dword ptr [edx + 10h] // 00c0f313
        mov edx, dword ptr [edx + 32] // 00c0f313
        mov dword ptr [ebp - 24h], edx // 00c0f313
    l00c0f31a:
        jmp l00c0f323 // 00c0f31a
    l00c0f31c:
        mov edx, dword ptr [ebp + 18h] // 00c0f31c
        mov edx, dword ptr [edx + 10h] // 00c0f31c
        mov edx, dword ptr [edx + 0] // 00c0f31c
        mov dword ptr [ebp - 24h], edx // 00c0f31c
    l00c0f323:
        fld qword ptr [edi] // 00c0f323
    l00c0f325:
        fmul qword ptr [ebp - 08h] // 00c0f325
    l00c0f328:
        fst qword ptr [esi] // 00c0f328
    l00c0f32a:
        fld qword ptr [edi] // 00c0f32a
    l00c0f32c:
        fstp qword ptr [ebp - 020h] // 00c0f32c
    l00c0f32f:
        fld qword ptr [ebx] // 00c0f32f
    l00c0f331:
        fstp qword ptr [ebp - 018h] // 00c0f331
    l00c0f334:
        lea ecx,[ebp - 028h] // 00c0f334
    l00c0f337:
        fstp qword ptr [ebp - 010h] // 00c0f337
    l00c0f33a:
        push ecx // 00c0f33a
    l00c0f33b:
        mov dword ptr [ebp - 028h],01h // 00c0f33b
    l00c0f342:
        call eax // 00c0f342
    l00c0f344:
        test eax,eax // 00c0f344
    l00c0f346:
        pop ecx // 00c0f346
    l00c0f347:
        jnz l00c0f354 // 00c0f347
    l00c0f349:
        mov eax, dword ptr [ebp + 18h] // 00c0f349
        mov eax, dword ptr [eax + 0Ch] // 00c0f349
        call dword ptr [eax + 4] // 00c0f349
    l00c0f34e:
        mov dword ptr [eax],021h // 00c0f34e
    l00c0f354:
        fld qword ptr [ebp - 010h] // 00c0f354
    l00c0f357:
        fstp qword ptr [esi] // 00c0f357
    l00c0f359:
        pop edi // 00c0f359
    l00c0f35a:
        pop esi // 00c0f35a
    l00c0f35b:
        pop ebx // 00c0f35b
    l00c0f35c:
        leave // 00c0f35c
    l00c0f35d:
        ret // 00c0f35d
    }
}
#pragma warning(pop)

// Actual C0F360 selector1000..1012 identities relocated to the corresponding
// in-body labels above. Offsets were extracted from MSVC Win32 emitted labels,
// then checked again against the complete final object and linked build.
// These are static readonly DIR32 function+offset relocations: no initializer
// function, runtime table publication, extra lookup or comparison chain.
// Any emitted-layout/toolchain change requires full table-target revalidation.
const unsigned char* const libm_selector_table[13] = {
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x239, // 1000: l00c0f2c5
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x247, // 1001: l00c0f2ce
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x255, // 1002: l00c0f2d7
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x266, // 1003: l00c0f2e0
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x277, // 1004: l00c0f2e9
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x288, // 1005: l00c0f2f2
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x1e7, // 1006: l00c0f27d
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x299, // 1007: l00c0f2fe
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x1c1, // 1008: l00c0f267
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x1b0, // 1009: l00c0f25e
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x2aa, // 1010: l00c0f30a
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x2b8, // 1011: l00c0f313
    reinterpret_cast<const unsigned char*>(&native_crt_libm_error_support_00c0f0e4) + 0x2c6, // 1012: l00c0f31c
};
} // namespace bsp
