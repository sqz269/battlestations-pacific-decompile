#include "bsp/native_unit_wake_decomposition.hpp"
#include "bsp/camera_decomposition.hpp"

#include <cstdint>

extern "C" double __cdecl _CIsqrt();

namespace bsp {
namespace {
// Immutable image payloads: D7A248 is FLT_MAX; CE3820 is binary64 1e-10.
// Integer storage spells the exact bytes consumed by MOVSS and FLD.
const std::uint32_t nearest_limit_bits = 0x7f7fffffu;
const std::uint64_t residual_cutoff_bits = 0x3ddb7cdfd9d7bdbbull;

// ECX carries actual_entity; the unused EDX argument leaves point/across/along
// in the native three stack slots. The private RET 0xC kernel preserves all
// 523 original instructions, including x87 intermediate precision, float
// spills, rereads, unordered comparisons, and across-before-along stores.
__declspec(naked) void __fastcall decompose_wake_kernel(const void*, void*,
    const float*, float*, float*) {
    __asm {
        sub esp,0x34                                                       // 00811180
        mov eax,dword ptr [esp + 0x38]                                     // 00811183
        movss xmm0,dword ptr [nearest_limit_bits]                          // 00811187
        push ebx                                                           // 0081118f
        movss dword ptr [esp + 0x8],xmm0                                   // 00811190
        movss xmm0,dword ptr [eax]                                         // 00811196
        movss dword ptr [esp + 0xc],xmm0                                   // 0081119a
        movss xmm0,dword ptr [eax + 0x4]                                   // 008111a0
        fld dword ptr [esp + 0xc]                                          // 008111a5
        push esi                                                           // 008111a9
        mov esi,ecx                                                        // 008111aa
        movss dword ptr [esp + 0x14],xmm0                                  // 008111ac
        movss xmm0,dword ptr [eax + 0x8]                                   // 008111b2
        fld dword ptr [esp + 0x14]                                         // 008111b7
        mov eax,dword ptr [esi + 0xf98]                                    // 008111bb
        push edi                                                           // 008111c1
        or edi,0xffffffff                                                  // 008111c2
        movss dword ptr [esp + 0x1c],xmm0                                  // 008111c5
        fld dword ptr [esp + 0x1c]                                         // 008111cb
        lea ecx,[eax + 0x27]                                               // 008111cf
        lea ebx,[edi + 0x3]                                                // 008111d2
        mov dword ptr [esp + 0x24],ecx                                     // 008111d5
        push ebp                                                           // 008111d9
    native_008111da:
        lea eax,[ecx + 0x1]                                                // 008111da
        cdq                                                                // 008111dd
        mov ebp,0x28                                                       // 008111de
        idiv ebp                                                           // 008111e3
        lea eax,[edx + edx*0x2]                                            // 008111e5
        fld dword ptr [esi + eax*0x8 + 0xbd8]                              // 008111e8
        lea eax,[esi + eax*0x8 + 0xbd8]                                    // 008111ef
        fstp dword ptr [esp + 0x2c]                                        // 008111f6
        fld dword ptr [eax + 0x4]                                          // 008111fa
        fstp dword ptr [esp + 0x30]                                        // 008111fd
        fld dword ptr [eax + 0x8]                                          // 00811201
        fstp dword ptr [esp + 0x34]                                        // 00811204
        fld st(2)                                                          // 00811208
        fsub dword ptr [esp + 0x2c]                                        // 0081120a
        fstp dword ptr [esp + 0x38]                                        // 0081120e
        fld st(1)                                                          // 00811212
        fsub dword ptr [esp + 0x30]                                        // 00811214
        fstp dword ptr [esp + 0x3c]                                        // 00811218
        fld st(0)                                                          // 0081121c
        fsub dword ptr [esp + 0x34]                                        // 0081121e
        fstp dword ptr [esp + 0x40]                                        // 00811222
        fld dword ptr [esp + 0x3c]                                         // 00811226
        fld dword ptr [esp + 0x38]                                         // 0081122a
        fld dword ptr [esp + 0x40]                                         // 0081122e
        fld st(1)                                                          // 00811232
        fmulp st(2), st(0)                                                 // 00811234
        fld st(2)                                                          // 00811236
        fmulp st(3), st(0)                                                 // 00811238
        fxch st(1)                                                         // 0081123a
        faddp st(2),st(0)                                                  // 0081123c
        fmul st(0), st(0)                                                  // 0081123e
        faddp st(1), st(0)                                                 // 00811240
        fstp dword ptr [esp + 0x10]                                        // 00811242
        fld dword ptr [esp + 0x10]                                         // 00811246
        fld dword ptr [esp + 0x14]                                         // 0081124a
        fcomip st(0),st(1)                                                 // 0081124e
        fstp st(0)                                                         // 00811250
        jbe native_00811263                                                // 00811252
        movss xmm0,dword ptr [esp + 0x10]                                  // 00811254
        movss dword ptr [esp + 0x14],xmm0                                  // 0081125a
        lea edi,[ebx + -0x2]                                               // 00811260
    native_00811263:
        mov eax,ecx                                                        // 00811263
        cdq                                                                // 00811265
        idiv ebp                                                           // 00811266
        lea edx,[edx + edx*0x2]                                            // 00811268
        fld dword ptr [esi + edx*0x8 + 0xbd8]                              // 0081126b
        lea eax,[esi + edx*0x8 + 0xbd8]                                    // 00811272
        fstp dword ptr [esp + 0x2c]                                        // 00811279
        fld dword ptr [eax + 0x4]                                          // 0081127d
        fstp dword ptr [esp + 0x30]                                        // 00811280
        fld dword ptr [eax + 0x8]                                          // 00811284
        fstp dword ptr [esp + 0x34]                                        // 00811287
        fld st(2)                                                          // 0081128b
        fsub dword ptr [esp + 0x2c]                                        // 0081128d
        fstp dword ptr [esp + 0x38]                                        // 00811291
        fld st(1)                                                          // 00811295
        fsub dword ptr [esp + 0x30]                                        // 00811297
        fstp dword ptr [esp + 0x3c]                                        // 0081129b
        fld st(0)                                                          // 0081129f
        fsub dword ptr [esp + 0x34]                                        // 008112a1
        fstp dword ptr [esp + 0x40]                                        // 008112a5
        fld dword ptr [esp + 0x3c]                                         // 008112a9
        fld dword ptr [esp + 0x38]                                         // 008112ad
        fld dword ptr [esp + 0x40]                                         // 008112b1
        fld st(1)                                                          // 008112b5
        fmulp st(2), st(0)                                                 // 008112b7
        fld st(2)                                                          // 008112b9
        fmulp st(3), st(0)                                                 // 008112bb
        fxch st(1)                                                         // 008112bd
        faddp st(2),st(0)                                                  // 008112bf
        fmul st(0), st(0)                                                  // 008112c1
        faddp st(1), st(0)                                                 // 008112c3
        fstp dword ptr [esp + 0x10]                                        // 008112c5
        fld dword ptr [esp + 0x10]                                         // 008112c9
        fld dword ptr [esp + 0x14]                                         // 008112cd
        fcomip st(0),st(1)                                                 // 008112d1
        fstp st(0)                                                         // 008112d3
        jbe native_008112e6                                                // 008112d5
        movss xmm0,dword ptr [esp + 0x10]                                  // 008112d7
        movss dword ptr [esp + 0x14],xmm0                                  // 008112dd
        lea edi,[ebx + -0x1]                                               // 008112e3
    native_008112e6:
        lea eax,[ecx + -0x1]                                               // 008112e6
        cdq                                                                // 008112e9
        idiv ebp                                                           // 008112ea
        lea eax,[edx + edx*0x2]                                            // 008112ec
        fld dword ptr [esi + eax*0x8 + 0xbd8]                              // 008112ef
        lea eax,[esi + eax*0x8 + 0xbd8]                                    // 008112f6
        fstp dword ptr [esp + 0x2c]                                        // 008112fd
        fld dword ptr [eax + 0x4]                                          // 00811301
        fstp dword ptr [esp + 0x30]                                        // 00811304
        fld dword ptr [eax + 0x8]                                          // 00811308
        fstp dword ptr [esp + 0x34]                                        // 0081130b
        fld st(2)                                                          // 0081130f
        fsub dword ptr [esp + 0x2c]                                        // 00811311
        fstp dword ptr [esp + 0x38]                                        // 00811315
        fld st(1)                                                          // 00811319
        fsub dword ptr [esp + 0x30]                                        // 0081131b
        fstp dword ptr [esp + 0x3c]                                        // 0081131f
        fld st(0)                                                          // 00811323
        fsub dword ptr [esp + 0x34]                                        // 00811325
        fstp dword ptr [esp + 0x40]                                        // 00811329
        fld dword ptr [esp + 0x3c]                                         // 0081132d
        fld dword ptr [esp + 0x38]                                         // 00811331
        fld dword ptr [esp + 0x40]                                         // 00811335
        fld st(1)                                                          // 00811339
        fmulp st(2), st(0)                                                 // 0081133b
        fld st(2)                                                          // 0081133d
        fmulp st(3), st(0)                                                 // 0081133f
        fxch st(1)                                                         // 00811341
        faddp st(2),st(0)                                                  // 00811343
        fmul st(0), st(0)                                                  // 00811345
        faddp st(1), st(0)                                                 // 00811347
        fstp dword ptr [esp + 0x10]                                        // 00811349
        fld dword ptr [esp + 0x10]                                         // 0081134d
        fld dword ptr [esp + 0x14]                                         // 00811351
        fcomip st(0),st(1)                                                 // 00811355
        fstp st(0)                                                         // 00811357
        jbe native_00811369                                                // 00811359
        movss xmm0,dword ptr [esp + 0x10]                                  // 0081135b
        movss dword ptr [esp + 0x14],xmm0                                  // 00811361
        mov edi,ebx                                                        // 00811367
    native_00811369:
        lea eax,[ecx + -0x2]                                               // 00811369
        cdq                                                                // 0081136c
        idiv ebp                                                           // 0081136d
        lea edx,[edx + edx*0x2]                                            // 0081136f
        fld dword ptr [esi + edx*0x8 + 0xbd8]                              // 00811372
        lea eax,[esi + edx*0x8 + 0xbd8]                                    // 00811379
        fstp dword ptr [esp + 0x2c]                                        // 00811380
        fld dword ptr [eax + 0x4]                                          // 00811384
        fstp dword ptr [esp + 0x30]                                        // 00811387
        fld dword ptr [eax + 0x8]                                          // 0081138b
        fstp dword ptr [esp + 0x34]                                        // 0081138e
        fld st(2)                                                          // 00811392
        fsub dword ptr [esp + 0x2c]                                        // 00811394
        fstp dword ptr [esp + 0x38]                                        // 00811398
        fld st(1)                                                          // 0081139c
        fsub dword ptr [esp + 0x30]                                        // 0081139e
        fstp dword ptr [esp + 0x3c]                                        // 008113a2
        fld st(0)                                                          // 008113a6
        fsub dword ptr [esp + 0x34]                                        // 008113a8
        fstp dword ptr [esp + 0x40]                                        // 008113ac
        fld dword ptr [esp + 0x3c]                                         // 008113b0
        fld dword ptr [esp + 0x38]                                         // 008113b4
        fld dword ptr [esp + 0x40]                                         // 008113b8
        fld st(1)                                                          // 008113bc
        fmulp st(2), st(0)                                                 // 008113be
        fld st(2)                                                          // 008113c0
        fmulp st(3), st(0)                                                 // 008113c2
        fxch st(1)                                                         // 008113c4
        faddp st(2),st(0)                                                  // 008113c6
        fmul st(0), st(0)                                                  // 008113c8
        faddp st(1), st(0)                                                 // 008113ca
        fstp dword ptr [esp + 0x10]                                        // 008113cc
        fld dword ptr [esp + 0x10]                                         // 008113d0
        fld dword ptr [esp + 0x14]                                         // 008113d4
        fcomip st(0),st(1)                                                 // 008113d8
        fstp st(0)                                                         // 008113da
        jbe native_008113ed                                                // 008113dc
        movss xmm0,dword ptr [esp + 0x10]                                  // 008113de
        movss dword ptr [esp + 0x14],xmm0                                  // 008113e4
        lea edi,[ebx + 0x1]                                                // 008113ea
    native_008113ed:
        lea eax,[ecx + -0x3]                                               // 008113ed
        cdq                                                                // 008113f0
        idiv ebp                                                           // 008113f1
        lea eax,[edx + edx*0x2]                                            // 008113f3
        fld dword ptr [esi + eax*0x8 + 0xbd8]                              // 008113f6
        lea eax,[esi + eax*0x8 + 0xbd8]                                    // 008113fd
        fstp dword ptr [esp + 0x2c]                                        // 00811404
        fld dword ptr [eax + 0x4]                                          // 00811408
        fstp dword ptr [esp + 0x30]                                        // 0081140b
        fld dword ptr [eax + 0x8]                                          // 0081140f
        fstp dword ptr [esp + 0x34]                                        // 00811412
        fld st(2)                                                          // 00811416
        fsub dword ptr [esp + 0x2c]                                        // 00811418
        fstp dword ptr [esp + 0x38]                                        // 0081141c
        fld st(1)                                                          // 00811420
        fsub dword ptr [esp + 0x30]                                        // 00811422
        fstp dword ptr [esp + 0x3c]                                        // 00811426
        fld st(0)                                                          // 0081142a
        fsub dword ptr [esp + 0x34]                                        // 0081142c
        fstp dword ptr [esp + 0x40]                                        // 00811430
        fld dword ptr [esp + 0x3c]                                         // 00811434
        fld dword ptr [esp + 0x38]                                         // 00811438
        fld dword ptr [esp + 0x40]                                         // 0081143c
        fld st(1)                                                          // 00811440
        fmulp st(2), st(0)                                                 // 00811442
        fld st(2)                                                          // 00811444
        fmulp st(3), st(0)                                                 // 00811446
        fxch st(1)                                                         // 00811448
        faddp st(2),st(0)                                                  // 0081144a
        fmul st(0), st(0)                                                  // 0081144c
        faddp st(1), st(0)                                                 // 0081144e
        fstp dword ptr [esp + 0x10]                                        // 00811450
        fld dword ptr [esp + 0x10]                                         // 00811454
        fld dword ptr [esp + 0x14]                                         // 00811458
        fcomip st(0),st(1)                                                 // 0081145c
        fstp st(0)                                                         // 0081145e
        jbe native_00811471                                                // 00811460
        movss xmm0,dword ptr [esp + 0x10]                                  // 00811462
        movss dword ptr [esp + 0x14],xmm0                                  // 00811468
        lea edi,[ebx + 0x2]                                                // 0081146e
    native_00811471:
        add ebx,0x5                                                        // 00811471
        lea edx,[ebx + -0x2]                                               // 00811474
        sub ecx,0x5                                                        // 00811477
        cmp edx,ebp                                                        // 0081147a
        jl native_008111da                                                 // 0081147c
        test edi,edi                                                       // 00811482
        fstp st(1)                                                         // 00811484
        fstp st(0)                                                         // 00811486
        jnz native_00811494                                                // 00811488
        fstp st(0)                                                         // 0081148a
        lea ecx,[edi + 0x1]                                                // 0081148c
        jmp native_00811587                                                // 0081148f
    native_00811494:
        cmp edi,0x27                                                       // 00811494
        jnz native_008114a5                                                // 00811497
        mov ecx,edi                                                        // 00811499
        fstp st(0)                                                         // 0081149b
        lea edi,[ecx + -0x1]                                               // 0081149d
        jmp native_00811587                                                // 008114a0
    native_008114a5:
        mov ecx,dword ptr [esi + 0xf98]                                    // 008114a5
        fld st(0)                                                          // 008114ab
        sub ecx,edi                                                        // 008114ad
        lea eax,[ecx + 0x27]                                               // 008114af
        cdq                                                                // 008114b2
        mov ebx,ebp                                                        // 008114b3
        idiv ebx                                                           // 008114b5
        lea eax,[edx + edx*0x2]                                            // 008114b7
        fsub dword ptr [esi + eax*0x8 + 0xbd8]                             // 008114ba
        mov edx,dword ptr [esp + 0x48]                                     // 008114c1
        lea eax,[esi + eax*0x8 + 0xbd8]                                    // 008114c5
        fstp dword ptr [esp + 0x38]                                        // 008114cc
        fld dword ptr [edx + 0x4]                                          // 008114d0
        fstp dword ptr [esp + 0x48]                                        // 008114d3
        fld dword ptr [esp + 0x48]                                         // 008114d7
        fld st(0)                                                          // 008114db
        fsub dword ptr [eax + 0x4]                                         // 008114dd
        fstp dword ptr [esp + 0x3c]                                        // 008114e0
        fld dword ptr [edx + 0x8]                                          // 008114e4
        fstp dword ptr [esp + 0x48]                                        // 008114e7
        fld dword ptr [esp + 0x48]                                         // 008114eb
        fld st(0)                                                          // 008114ef
        fsub dword ptr [eax + 0x8]                                         // 008114f1
        lea eax,[ecx + 0x29]                                               // 008114f4
        cdq                                                                // 008114f7
        mov ecx,ebx                                                        // 008114f8
        idiv ecx                                                           // 008114fa
        fstp dword ptr [esp + 0x40]                                        // 008114fc
        lea edx,[edx + edx*0x2]                                            // 00811500
        fld dword ptr [esi + edx*0x8 + 0xbd8]                              // 00811503
        lea eax,[esi + edx*0x8 + 0xbd8]                                    // 0081150a
        fsubp st(3),st(0)                                                  // 00811511
        fxch st(2)                                                         // 00811513
        fstp dword ptr [esp + 0x2c]                                        // 00811515
        fsub dword ptr [eax + 0x4]                                         // 00811519
        fstp dword ptr [esp + 0x30]                                        // 0081151c
        fsub dword ptr [eax + 0x8]                                         // 00811520
        fstp dword ptr [esp + 0x34]                                        // 00811523
        fld dword ptr [esp + 0x3c]                                         // 00811527
        fld dword ptr [esp + 0x38]                                         // 0081152b
        fld dword ptr [esp + 0x40]                                         // 0081152f
        fld dword ptr [esp + 0x30]                                         // 00811533
        fld dword ptr [esp + 0x2c]                                         // 00811537
        fld dword ptr [esp + 0x34]                                         // 0081153b
        fld st(4)                                                          // 0081153f
        fmulp st(5), st(0)                                                 // 00811541
        fld st(5)                                                          // 00811543
        fmulp st(6), st(0)                                                 // 00811545
        fxch st(4)                                                         // 00811547
        faddp st(5),st(0)                                                  // 00811549
        fld st(2)                                                          // 0081154b
        fmulp st(3), st(0)                                                 // 0081154d
        fxch st(4)                                                         // 0081154f
        faddp st(2),st(0)                                                  // 00811551
        fxch st(1)                                                         // 00811553
        fstp dword ptr [esp + 0x48]                                        // 00811555
        fld dword ptr [esp + 0x48]                                         // 00811559
        fld st(3)                                                          // 0081155d
        fmulp st(4), st(0)                                                 // 0081155f
        fld st(1)                                                          // 00811561
        fmulp st(2), st(0)                                                 // 00811563
        fxch st(3)                                                         // 00811565
        faddp st(1), st(0)                                                 // 00811567
        fld st(1)                                                          // 00811569
        fmulp st(2), st(0)                                                 // 0081156b
        faddp st(1), st(0)                                                 // 0081156d
        fstp dword ptr [esp + 0x48]                                        // 0081156f
        fld dword ptr [esp + 0x48]                                         // 00811573
        fcomip st(0),st(1)                                                 // 00811577
        fstp st(0)                                                         // 00811579
        jbe native_00811582                                                // 0081157b
        lea ecx,[edi + 0x1]                                                // 0081157d
        jmp native_00811587                                                // 00811580
    native_00811582:
        mov ecx,edi                                                        // 00811582
        sub edi,0x1                                                        // 00811584
    native_00811587:
        mov ebx,dword ptr [esi + 0xf98]                                    // 00811587
        mov eax,ebx                                                        // 0081158d
        sub eax,ecx                                                        // 0081158f
        add eax,0x28                                                       // 00811591
        cdq                                                                // 00811594
        mov ecx,ebp                                                        // 00811595
        idiv ecx                                                           // 00811597
        mov eax,ebx                                                        // 00811599
        sub eax,edi                                                        // 0081159b
        add eax,0x28                                                       // 0081159d
        mov ebx,ebp                                                        // 008115a0
        lea edx,[edx + edx*0x2]                                            // 008115a2
        lea ecx,[esi + edx*0x8 + 0xbd8]                                    // 008115a5
        cdq                                                                // 008115ac
        idiv ebx                                                           // 008115ad
        lea eax,[edx + edx*0x2]                                            // 008115af
        fld dword ptr [esi + eax*0x8 + 0xbd8]                              // 008115b2
        lea eax,[esi + eax*0x8 + 0xbd8]                                    // 008115b9
        fsub dword ptr [ecx]                                               // 008115c0
        fstp dword ptr [esp + 0x38]                                        // 008115c2
        fld dword ptr [eax + 0x4]                                          // 008115c6
        fsub dword ptr [ecx + 0x4]                                         // 008115c9
        fstp dword ptr [esp + 0x3c]                                        // 008115cc
        fld dword ptr [eax + 0x8]                                          // 008115d0
        fsub dword ptr [ecx + 0x8]                                         // 008115d3
        lea ecx,[esp + 0x38]                                               // 008115d6
        fstp dword ptr [esp + 0x40]                                        // 008115da
        call normalize_camera_basis_0042b260                               // 008115de
        mov ecx,dword ptr [esi + 0xf98]                                    // 008115e3
        mov eax,ecx                                                        // 008115e9
        sub eax,edi                                                        // 008115eb
        add eax,0x28                                                       // 008115ed
        cdq                                                                // 008115f0
        idiv ebx                                                           // 008115f1
        lea edx,[edx + edx*0x2]                                            // 008115f3
        fld dword ptr [esi + edx*0x8 + 0xbd8]                              // 008115f6
        lea eax,[esi + edx*0x8 + 0xbd8]                                    // 008115fd
        fsub dword ptr [esp + 0x18]                                        // 00811604
        lea edx,[edi + 0x1]                                                // 00811608
        fstp dword ptr [esp + 0x2c]                                        // 0081160b
        fld dword ptr [eax + 0x4]                                          // 0081160f
        fsub dword ptr [esp + 0x1c]                                        // 00811612
        fstp dword ptr [esp + 0x30]                                        // 00811616
        fld dword ptr [eax + 0x8]                                          // 0081161a
        xor eax,eax                                                        // 0081161d
        cmp edx,0x4                                                        // 0081161f
        fsub dword ptr [esp + 0x20]                                        // 00811622
        fstp dword ptr [esp + 0x34]                                        // 00811626
        fld dword ptr [esp + 0x30]                                         // 0081162a
        fld st(0)                                                          // 0081162e
        fmul dword ptr [esp + 0x3c]                                        // 00811630
        fld dword ptr [esp + 0x2c]                                         // 00811634
        fld st(0)                                                          // 00811638
        fld dword ptr [esp + 0x38]                                         // 0081163a
        fld st(0)                                                          // 0081163e
        fmulp st(2), st(0)                                                 // 00811640
        fxch st(3)                                                         // 00811642
        faddp st(1), st(0)                                                 // 00811644
        fld dword ptr [esp + 0x34]                                         // 00811646
        fld st(0)                                                          // 0081164a
        fld dword ptr [esp + 0x40]                                         // 0081164c
        fld st(0)                                                          // 00811650
        fmulp st(2), st(0)                                                 // 00811652
        fxch st(3)                                                         // 00811654
        faddp st(1), st(0)                                                 // 00811656
        fstp dword ptr [esp + 0x48]                                        // 00811658
        fld dword ptr [esp + 0x48]                                         // 0081165c
        fstp dword ptr [esp + 0x20]                                        // 00811660
        jl native_008116f1                                                 // 00811664
        mov ecx,dword ptr [esp + 0x28]                                     // 0081166a
        lea ebx,[edi + 0x1]                                                // 0081166e
        shr ebx,0x2                                                        // 00811671
        lea eax,[ebx*0x4 + 0x0]                                            // 00811674
        mov dword ptr [esp + 0x28],eax                                     // 0081167b
    native_0081167f:
        lea eax,[ecx + 0x1]                                                // 0081167f
        cdq                                                                // 00811682
        mov ebp,0x28                                                       // 00811683
        idiv ebp                                                           // 00811688
        mov eax,ecx                                                        // 0081168a
        add edx,0x7f                                                       // 0081168c
        lea edx,[edx + edx*0x2]                                            // 0081168f
        fld dword ptr [esi + edx*0x8]                                      // 00811692
        cdq                                                                // 00811695
        idiv ebp                                                           // 00811696
        fadd dword ptr [esp + 0x48]                                        // 00811698
        fstp dword ptr [esp + 0x48]                                        // 0081169c
        fld dword ptr [esp + 0x48]                                         // 008116a0
        add edx,0x7f                                                       // 008116a4
        lea eax,[edx + edx*0x2]                                            // 008116a7
        fadd dword ptr [esi + eax*0x8]                                     // 008116aa
        lea eax,[ecx + -0x1]                                               // 008116ad
        cdq                                                                // 008116b0
        idiv ebp                                                           // 008116b1
        fstp dword ptr [esp + 0x48]                                        // 008116b3
        fld dword ptr [esp + 0x48]                                         // 008116b7
        lea eax,[ecx + -0x2]                                               // 008116bb
        sub ecx,0x4                                                        // 008116be
        add edx,0x7f                                                       // 008116c1
        lea edx,[edx + edx*0x2]                                            // 008116c4
        fadd dword ptr [esi + edx*0x8]                                     // 008116c7
        cdq                                                                // 008116ca
        idiv ebp                                                           // 008116cb
        fstp dword ptr [esp + 0x48]                                        // 008116cd
        fld dword ptr [esp + 0x48]                                         // 008116d1
        add edx,0x7f                                                       // 008116d5
        sub ebx,0x1                                                        // 008116d8
        lea eax,[edx + edx*0x2]                                            // 008116db
        fadd dword ptr [esi + eax*0x8]                                     // 008116de
        fstp dword ptr [esp + 0x48]                                        // 008116e1
        jnz native_0081167f                                                // 008116e5
        mov ecx,dword ptr [esi + 0xf98]                                    // 008116e7
        mov eax,dword ptr [esp + 0x28]                                     // 008116ed
    native_008116f1:
        cmp eax,edi                                                        // 008116f1
        pop ebp                                                            // 008116f3
        jg native_00811723                                                 // 008116f4
        sub ecx,eax                                                        // 008116f6
        sub edi,eax                                                        // 008116f8
        add ecx,0x28                                                       // 008116fa
        add edi,0x1                                                        // 008116fd
    native_00811700:
        mov eax,ecx                                                        // 00811700
        cdq                                                                // 00811702
        mov ebx,0x28                                                       // 00811703
        idiv ebx                                                           // 00811708
        sub ecx,0x1                                                        // 0081170a
        add edx,0x7f                                                       // 0081170d
        sub edi,0x1                                                        // 00811710
        lea edx,[edx + edx*0x2]                                            // 00811713
        fld dword ptr [esi + edx*0x8]                                      // 00811716
        fadd dword ptr [esp + 0x44]                                        // 00811719
        fstp dword ptr [esp + 0x44]                                        // 0081171d
        jnz native_00811700                                                // 00811721
    native_00811723:
        xorps xmm0,xmm0                                                    // 00811723
        fld st(1)                                                          // 00811726
        fmul st(0), st(3)                                                  // 00811728
        pop edi                                                            // 0081172a
        fld st(1)                                                          // 0081172b
        pop esi                                                            // 0081172d
        fmul st(0), st(5)                                                  // 0081172e
        pop ebx                                                            // 00811730
        fsubp st(1), st(0)                                                 // 00811731
        fstp dword ptr [esp + 0x14]                                        // 00811733
        fld dword ptr [esp + 0x14]                                         // 00811737
        fldz                                                               // 0081173b
        fcomip st(0),st(1)                                                 // 0081173d
        fstp st(0)                                                         // 0081173f
        jbe native_0081174d                                                // 00811741
        mov dword ptr [esp + 0x4],0xffffffff                               // 00811743
        jmp native_00811768                                                // 0081174b
    native_0081174d:
        movss xmm1,dword ptr [esp + 0x14]                                  // 0081174d
        comiss xmm1,xmm0                                                   // 00811753
        mov dword ptr [esp + 0x4],0x1                                      // 00811756
        ja native_00811768                                                 // 0081175e
        mov dword ptr [esp + 0x4],0x0                                      // 00811760
    native_00811768:
        fld dword ptr [esp + 0x10]                                         // 00811768
        fld st(0)                                                          // 0081176c
        fmulp st(5), st(0)                                                 // 0081176e
        fxch st(4)                                                         // 00811770
        fstp dword ptr [esp + 0x1c]                                        // 00811772
        fld dword ptr [esp + 0x2c]                                         // 00811776
        fmul st(0), st(4)                                                  // 0081177a
        fstp dword ptr [esp + 0x20]                                        // 0081177c
        fxch st(1)                                                         // 00811780
        fmulp st(3), st(0)                                                 // 00811782
        fxch st(2)                                                         // 00811784
        fstp dword ptr [esp + 0x24]                                        // 00811786
        fsub dword ptr [esp + 0x1c]                                        // 0081178a
        fstp dword ptr [esp + 0x28]                                        // 0081178e
        fld dword ptr [esp + 0x20]                                         // 00811792
        fsubp st(2),st(0)                                                  // 00811796
        fxch st(1)                                                         // 00811798
        fstp dword ptr [esp + 0x2c]                                        // 0081179a
        fsub dword ptr [esp + 0x24]                                        // 0081179e
        fstp dword ptr [esp + 0x30]                                        // 008117a2
        fld dword ptr [esp + 0x2c]                                         // 008117a6
        fld dword ptr [esp + 0x28]                                         // 008117aa
        fld dword ptr [esp + 0x30]                                         // 008117ae
        fld st(1)                                                          // 008117b2
        fmulp st(2), st(0)                                                 // 008117b4
        fld st(2)                                                          // 008117b6
        fmulp st(3), st(0)                                                 // 008117b8
        fxch st(1)                                                         // 008117ba
        faddp st(2),st(0)                                                  // 008117bc
        fmul st(0), st(0)                                                  // 008117be
        faddp st(1), st(0)                                                 // 008117c0
        fstp dword ptr [esp + 0x18]                                        // 008117c2
        fld qword ptr [residual_cutoff_bits]                               // 008117c6
        fld dword ptr [esp + 0x18]                                         // 008117cc
        fcomi st(0),st(1)                                                  // 008117d0
        fstp st(1)                                                         // 008117d2
        jbe native_008117e9                                                // 008117d4
        call _CIsqrt                                                       // 008117d6
        fstp dword ptr [esp + 0x18]                                        // 008117db
        fld dword ptr [esp + 0x18]                                         // 008117df
        fstp dword ptr [esp + 0x14]                                        // 008117e3
        jmp native_008117f1                                                // 008117e7
    native_008117e9:
        fstp st(0)                                                         // 008117e9
        movss dword ptr [esp + 0x14],xmm0                                  // 008117eb
    native_008117f1:
        fild dword ptr [esp + 0x4]                                         // 008117f1
        mov eax,dword ptr [esp + 0x3c]                                     // 008117f5
        mov ecx,dword ptr [esp + 0x40]                                     // 008117f9
        movss xmm0,dword ptr [esp + 0x38]                                  // 008117fd
        fmul dword ptr [esp + 0x14]                                        // 00811803
        fstp dword ptr [eax]                                               // 00811807
        movss dword ptr [ecx],xmm0                                         // 00811809
        add esp,0x34                                                       // 0081180d
        ret 0xc                                                            // 00811810
    }
}
} // namespace

void decompose_native_unit_wake_00811180(const void* actual_entity,
    const float* point, float* across, float* along) {
    decompose_wake_kernel(actual_entity, nullptr, point, across, along);
}

} // namespace bsp
