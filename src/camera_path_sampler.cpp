#include "bsp/camera_path_sampler.hpp"
#include "bsp/camera_frame_state.hpp"
#include "bsp/material_effect_plane.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Camera path reconstruction requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
const double half = 0.5; // 00D7A280
const double three = 3.0; // 00D7A2B0
const double four = 4.0; // 00D7A328
const double six = 6.0; // 00CE6628

// Native signed address subtraction/SAR2, including null-base size zero.
std::int32_t count(CameraPathView& path, void** begin) {
    if (begin == nullptr) return 0;
    const auto bytes = reinterpret_cast<std::uint32_t>(path.knots_end_0c)
        - reinterpret_cast<std::uint32_t>(begin);
    return static_cast<std::int32_t>(bytes) >> 2;
}
void check(CameraPathView& path, void** begin, std::uint32_t index, CameraPathHost& host) {
    if (begin == nullptr || index >= static_cast<std::uint32_t>(count(path, begin)))
        host.range_error_00bf6713();
}
float* words(CameraPathView& path, std::uint32_t index, CameraPathHost& host) {
    return host.resolve_path_knot_words(path.knots_begin_08[index]);
}
void copy_float(float& destination, const float& source) {
    float* output = &destination;
    const float* input = &source;
    __asm {
        mov eax, input
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
bool reaches_segment(float parameter, const float* start_record, const float* duration_record) {
    unsigned char take;
    __asm {
        mov eax, start_record
        mov edx, duration_record
        fld dword ptr [edx + 0x1c]
        fadd dword ptr [eax + 0x18]
        fld parameter
        fcomip st(0), st(1)
        fstp st(0)
        setbe take
    }
    return take != 0;
}
float local_parameter(float parameter, const float* start_record, const float* duration_record) {
    float result;
    __asm {
        mov eax, start_record
        mov edx, duration_record
        fld parameter
        fsub dword ptr [eax + 0x18]
        fdiv dword ptr [edx + 0x1c]
        fstp result
    }
    return result;
}

// Numeric body007AFFD9..007B03B7. ECX/EDX select actual current/next
// records; stack parameter,position,direction,endpoint,previous. Scratch offsets
// deliberately match native. endpoint replaces only the observed index gates.
__declspec(naked) void __fastcall cubic_kernel(const float*, const float*,
    float, float*, float*, unsigned, const float*) {
    __asm {
        sub esp, 0x58
        push esi
        push ebx
        push edi
        mov edi, ecx
        mov eax, edx
        mov edx, dword ptr [esp + 0x78]
        cmp dword ptr [esp + 0x74], 0
        fld dword ptr [edi + 0x1c]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [esp + 0x68]
        fsub dword ptr [edi + 0x18]
        fld dword ptr [esp + 0xc]
        fld st(0) // 007affd9
        fdivp st(2),st(0) // 007affdb
        fxch // 007affdd
        fstp dword ptr [esp + 0xc] // 007affdf
        fld dword ptr [edx + 0xc] // 007affe3
        fadd dword ptr [edi + 0xc] // 007affe6
        fstp dword ptr [esp + 0x1c] // 007affe9
        fld dword ptr [edx + 0x10] // 007affed
        fadd dword ptr [edi + 0x10] // 007afff0
        fstp dword ptr [esp + 0x20] // 007afff3
        fld dword ptr [edx + 0x14] // 007afff7
        fadd dword ptr [edi + 0x14] // 007afffa
        fstp dword ptr [esp + 0x24] // 007afffd
        fld qword ptr [half] // 007b0001
        fmul st(0), st(1) // 007b0007
        fstp dword ptr [esp + 0x68] // 007b0009
        fld dword ptr [esp + 0x68] // 007b000d
        fst dword ptr [esp + 0x68] // 007b0011
        fld dword ptr [esp + 0x1c] // 007b0015
        fld dword ptr [esp + 0x68] // 007b0019
        fld st(0) // 007b001d
        fmulp st(2), st(0) // 007b001f
        fxch // 007b0021
        fstp dword ptr [esp + 0x10] // 007b0023
        fld dword ptr [esp + 0x20] // 007b0027
        fmul st(0), st(1) // 007b002b
        fstp dword ptr [esp + 0x14] // 007b002d
        fmul dword ptr [esp + 0x24] // 007b0031
        fstp dword ptr [esp + 0x18] // 007b0035
        fld dword ptr [edi + 0xc] // 007b0039
        fadd dword ptr [eax + 0xc] // 007b003c
        fstp dword ptr [esp + 0x28] // 007b003f
        fld dword ptr [edi + 0x10] // 007b0043
        fadd dword ptr [eax + 0x10] // 007b0046
        fstp dword ptr [esp + 0x2c] // 007b0049
        fld dword ptr [edi + 0x14] // 007b004d
        fadd dword ptr [eax + 0x14] // 007b0050
        fstp dword ptr [esp + 0x30] // 007b0053
        fstp dword ptr [esp + 0x68] // 007b0057
        fld dword ptr [esp + 0x28] // 007b005b
        fld dword ptr [esp + 0x68] // 007b005f
        fld st(0) // 007b0063
        fmulp st(2), st(0) // 007b0065
        fxch // 007b0067
        fstp dword ptr [esp + 0x1c] // 007b0069
        fld dword ptr [esp + 0x2c] // 007b006d
        fmul st(0), st(1) // 007b0071
        fstp dword ptr [esp + 0x20] // 007b0073
        fmul dword ptr [esp + 0x30] // 007b0077
        fstp dword ptr [esp + 0x24] // 007b007b
        jz label_007b0106 // 007b007f
        cmp dword ptr [esp + 0x74],0x1 // 007b0085
        jnz label_007b00d3 // 007b0087
        fld dword ptr [edi + 0xc] // 007b0089
        fmul st(0), st(1) // 007b008c
        fstp dword ptr [esp + 0x28] // 007b008e
        fld dword ptr [edi + 0x10] // 007b0092
        fmul st(0), st(1) // 007b0095
        fstp dword ptr [esp + 0x2c] // 007b0097
        fmul dword ptr [edi + 0x14] // 007b009b
        fstp dword ptr [esp + 0x30] // 007b009e
        fld dword ptr [esp + 0x28] // 007b00a2
        fstp dword ptr [esp + 0x10] // 007b00a6
        fld dword ptr [esp + 0x2c] // 007b00aa
        fstp dword ptr [esp + 0x14] // 007b00ae
        fld dword ptr [esp + 0x30] // 007b00b2
        fstp dword ptr [esp + 0x18] // 007b00b6
        jmp label_007b0108 // 007b00ba
    label_007b00d3:
        fld dword ptr [edi + 0xc] // 007b00d3
        fmul st(0), st(1) // 007b00d6
        fstp dword ptr [esp + 0x28] // 007b00d8
        fld dword ptr [edi + 0x10] // 007b00dc
        fmul st(0), st(1) // 007b00df
        fstp dword ptr [esp + 0x2c] // 007b00e1
        fmul dword ptr [edi + 0x14] // 007b00e5
        fstp dword ptr [esp + 0x30] // 007b00e8
        fld dword ptr [esp + 0x28] // 007b00ec
        fstp dword ptr [esp + 0x1c] // 007b00f0
        fld dword ptr [esp + 0x2c] // 007b00f4
        fstp dword ptr [esp + 0x20] // 007b00f8
        fld dword ptr [esp + 0x30] // 007b00fc
        fstp dword ptr [esp + 0x24] // 007b0100
        jmp label_007b0108 // 007b0104
    label_007b0106:
        fstp st(0) // 007b0106
    label_007b0108:
        fld dword ptr [esp + 0xc] // 007b0108
        fld st(0) // 007b010c
        fmul st(0), st(0) // 007b010e
        fld st(1) // 007b0110
        fmul st(0), st(1) // 007b0112
        fld st(0) // 007b0114
        fsubrp st(2),st(0) // 007b0116
        fxch // 007b0118
        fstp dword ptr [esp + 0x74] // 007b011a
        fld dword ptr [esp + 0x74] // 007b011e
        fld st(0) // 007b0122
        fld dword ptr [esp + 0x1c] // 007b0124
        fld st(0) // 007b0128
        fmulp st(2), st(0) // 007b012a
        fxch // 007b012c
        fstp dword ptr [esp + 0x58] // 007b012e
        fld dword ptr [esp + 0x20] // 007b0132
        fmul st(0), st(2) // 007b0136
        fstp dword ptr [esp + 0x5c] // 007b0138
        fld dword ptr [esp + 0x24] // 007b013c
        fmulp st(2), st(0) // 007b0140
        fxch // 007b0142
        fstp dword ptr [esp + 0x60] // 007b0144
        fld st(2) // 007b0148
        fadd st(0),st(0) // 007b014a
        fld st(3) // 007b014c
        fmul st(0), st(1) // 007b014e
        fsub st(3),st(0) // 007b0150
        fld st(4) // 007b0152
        faddp st(4),st(0) // 007b0154
        fxch st(3) // 007b0156
        fstp dword ptr [esp + 0x74] // 007b0158
        fld dword ptr [esp + 0x74] // 007b015c
        fld st(0) // 007b0160
        fmul dword ptr [esp + 0x10] // 007b0162
        fstp dword ptr [esp + 0x4c] // 007b0166
        fld dword ptr [esp + 0x14] // 007b016a
        fmul st(0), st(1) // 007b016e
        fstp dword ptr [esp + 0x50] // 007b0170
        fmul dword ptr [esp + 0x18] // 007b0174
        fstp dword ptr [esp + 0x54] // 007b0178
        fld st(3) // 007b017c
        fmul qword ptr [three] // 007b017e
        fmul st(0), st(4) // 007b0184
        fld st(4) // 007b0186
        fmulp st(4), st(0) // 007b0188
        fld st(0) // 007b018a
        fsub st(0),st(4) // 007b018c
        fstp dword ptr [esp + 0x74] // 007b018e
        fld dword ptr [esp + 0x74] // 007b0192
        fld st(0) // 007b0196
        fmul dword ptr [eax] // 007b0198
        fstp dword ptr [esp + 0x34] // 007b019a
        fld dword ptr [eax + 0x4] // 007b019e
        fmul st(0), st(1) // 007b01a1
        fstp dword ptr [esp + 0x38] // 007b01a3
        fmul dword ptr [eax + 0x8] // 007b01a7
        fstp dword ptr [esp + 0x3c] // 007b01aa
        fsub st(3),st(0) // 007b01ae
        fld1 // 007b01b0
        fadd st(4),st(0) // 007b01b2
        fxch st(4) // 007b01b4
        fstp dword ptr [esp + 0x74] // 007b01b6
        fld dword ptr [edi] // 007b01ba
        fld dword ptr [esp + 0x74] // 007b01bc
        fld st(0) // 007b01c0
        fmulp st(2), st(0) // 007b01c2
        fxch // 007b01c4
        fstp dword ptr [esp + 0x28] // 007b01c6
        fld st(0) // 007b01ca
        fmul dword ptr [edi + 0x4] // 007b01cc
        fstp dword ptr [esp + 0x2c] // 007b01cf
        fmul dword ptr [edi + 0x8] // 007b01d3
        fstp dword ptr [esp + 0x30] // 007b01d6
        fld dword ptr [esp + 0x28] // 007b01da
        fadd dword ptr [esp + 0x34] // 007b01de
        fstp dword ptr [esp + 0x40] // 007b01e2
        fld dword ptr [esp + 0x2c] // 007b01e6
        fadd dword ptr [esp + 0x38] // 007b01ea
        fstp dword ptr [esp + 0x44] // 007b01ee
        fld dword ptr [esp + 0x30] // 007b01f2
        fadd dword ptr [esp + 0x3c] // 007b01f6
        mov ecx,dword ptr [esp + 0x6c] // 007b01fa
        fstp dword ptr [esp + 0x48] // 007b01fe
        fld dword ptr [esp + 0x40] // 007b0202
        fadd dword ptr [esp + 0x4c] // 007b0206
        fstp dword ptr [esp + 0x34] // 007b020a
        fld dword ptr [esp + 0x44] // 007b020e
        fadd dword ptr [esp + 0x50] // 007b0212
        fstp dword ptr [esp + 0x38] // 007b0216
        fld dword ptr [esp + 0x48] // 007b021a
        fadd dword ptr [esp + 0x54] // 007b021e
        fstp dword ptr [esp + 0x3c] // 007b0222
        fld dword ptr [esp + 0x34] // 007b0226
        fadd dword ptr [esp + 0x58] // 007b022a
        fstp dword ptr [esp + 0x4c] // 007b022e
        fld dword ptr [esp + 0x38] // 007b0232
        fadd dword ptr [esp + 0x5c] // 007b0236
        fstp dword ptr [esp + 0x50] // 007b023a
        fld dword ptr [esp + 0x3c] // 007b023e
        fadd dword ptr [esp + 0x60] // 007b0242
        fstp dword ptr [esp + 0x54] // 007b0246
        fld dword ptr [esp + 0x4c] // 007b024a
        fstp dword ptr [ecx] // 007b024e
        fld dword ptr [esp + 0x50] // 007b0250
        fstp dword ptr [ecx + 0x4] // 007b0254
        fld dword ptr [esp + 0x54] // 007b0257
        fstp dword ptr [ecx + 0x8] // 007b025b
        mov ecx,dword ptr [esp + 0x70] // 007b025e
        test ecx,ecx // 007b0262
        jz label_007b03a5 // 007b0264
        fld st(0) // 007b026a
        fsubrp st(2),st(0) // 007b026c
        fxch // 007b026e
        fstp dword ptr [esp + 0x74] // 007b0270
        fld dword ptr [esp + 0x74] // 007b0274
        fld st(0) // 007b0278
        fmulp st(3), st(0) // 007b027a
        fxch st(2) // 007b027c
        fstp dword ptr [esp + 0x28] // 007b027e
        fld dword ptr [esp + 0x20] // 007b0282
        fmul st(0), st(2) // 007b0286
        fstp dword ptr [esp + 0x2c] // 007b0288
        fld dword ptr [esp + 0x24] // 007b028c
        fmulp st(2), st(0) // 007b0290
        fxch // 007b0292
        fstp dword ptr [esp + 0x30] // 007b0294
        fld st(2) // 007b0298
        fmul qword ptr [four] // 007b029a
        fsubp st(1), st(0) // 007b02a0
        faddp st(1), st(0) // 007b02a2
        fstp dword ptr [esp + 0x74] // 007b02a4
        fld dword ptr [esp + 0x74] // 007b02a8
        fld st(0) // 007b02ac
        fmul dword ptr [esp + 0x10] // 007b02ae
        fstp dword ptr [esp + 0x34] // 007b02b2
        fld dword ptr [esp + 0x14] // 007b02b6
        fmul st(0), st(1) // 007b02ba
        fstp dword ptr [esp + 0x38] // 007b02bc
        fmul dword ptr [esp + 0x18] // 007b02c0
        fstp dword ptr [esp + 0x3c] // 007b02c4
        fld qword ptr [six] // 007b02c8
        fmul st(0), st(1) // 007b02ce
        fmul st(1), st(0) // 007b02d0: DCC9; Ghidra's single ST1 omits destination.
        fld st(0) // 007b02d2
        fsub st(0),st(2) // 007b02d4
        fstp dword ptr [esp + 0x74] // 007b02d6
        fld dword ptr [esp + 0x74] // 007b02da
        fld st(0) // 007b02de
        fmul dword ptr [eax] // 007b02e0
        fstp dword ptr [esp + 0x4c] // 007b02e2
        fld dword ptr [eax + 0x4] // 007b02e6
        fmul st(0), st(1) // 007b02e9
        fstp dword ptr [esp + 0x50] // 007b02eb
        fmul dword ptr [eax + 0x8] // 007b02ef
        fstp dword ptr [esp + 0x54] // 007b02f2
        fsubp st(1), st(0) // 007b02f6
        fstp dword ptr [esp + 0x74] // 007b02f8
        fld dword ptr [edi] // 007b02fc
        fld dword ptr [esp + 0x74] // 007b02fe
        fld st(0) // 007b0302
        fmulp st(2), st(0) // 007b0304
        fxch // 007b0306
        fstp dword ptr [esp + 0x58] // 007b0308
        fld st(0) // 007b030c
        fmul dword ptr [edi + 0x4] // 007b030e
        fstp dword ptr [esp + 0x5c] // 007b0311
        fmul dword ptr [edi + 0x8] // 007b0315
        fstp dword ptr [esp + 0x60] // 007b0318
        fld dword ptr [esp + 0x58] // 007b031c
        fadd dword ptr [esp + 0x4c] // 007b0320
        fstp dword ptr [esp + 0x40] // 007b0324
        fld dword ptr [esp + 0x5c] // 007b0328
        fadd dword ptr [esp + 0x50] // 007b032c
        fstp dword ptr [esp + 0x44] // 007b0330
        fld dword ptr [esp + 0x60] // 007b0334
        fadd dword ptr [esp + 0x54] // 007b0338
        fstp dword ptr [esp + 0x48] // 007b033c
        fld dword ptr [esp + 0x40] // 007b0340
        fadd dword ptr [esp + 0x34] // 007b0344
        fstp dword ptr [esp + 0x58] // 007b0348
        fld dword ptr [esp + 0x44] // 007b034c
        fadd dword ptr [esp + 0x38] // 007b0350
        fstp dword ptr [esp + 0x5c] // 007b0354
        fld dword ptr [esp + 0x48] // 007b0358
        fadd dword ptr [esp + 0x3c] // 007b035c
        fstp dword ptr [esp + 0x60] // 007b0360
        fld dword ptr [esp + 0x58] // 007b0364
        fadd dword ptr [esp + 0x28] // 007b0368
        fstp dword ptr [esp + 0x4c] // 007b036c
        fld dword ptr [esp + 0x5c] // 007b0370
        fadd dword ptr [esp + 0x2c] // 007b0374
        fstp dword ptr [esp + 0x50] // 007b0378
        pop edi // 007b037c
        fld dword ptr [esp + 0x5c] // 007b037d
        pop ebx // 007b0381
        fadd dword ptr [esp + 0x28] // 007b0382
        pop esi // 007b0386
        fstp dword ptr [esp + 0x48] // 007b0387
        fld dword ptr [esp + 0x40] // 007b038b
        fstp dword ptr [ecx] // 007b038f
        fld dword ptr [esp + 0x44] // 007b0391
        fstp dword ptr [ecx + 0x4] // 007b0395
        fld dword ptr [esp + 0x48] // 007b0398
        fstp dword ptr [ecx + 0x8] // 007b039c
        add esp,0x58 // 007b039f
        ret 0x14 // 007b03a2
    label_007b03a5:
        fstp st(4) // 007b03a5
        pop edi // 007b03a7
        fstp st(3) // 007b03a8
        pop ebx // 007b03aa
        fstp st(1) // 007b03ab
        pop esi // 007b03ad
        fstp st(1) // 007b03ae
        fstp st(0) // 007b03b0
        add esp,0x58 // 007b03b2
        ret 0x14 // 007b03b5
    }
}

// Exact arithmetic and stack argument layout of007AE200; EDX unused.
__declspec(naked) float* __fastcall lerp_kernel(float*, void*,
    float, float, float, float, float, float, float) {
    __asm {
        sub esp,0xc // 007ae200
        fld dword ptr [esp + 0x1c] // 007ae203
        mov eax,ecx // 007ae207
        fld dword ptr [esp + 0x10] // 007ae209
        fld st(0) // 007ae20d
        fsubp st(2),st(0) // 007ae20f
        fxch // 007ae211
        fstp dword ptr [esp] // 007ae213
        fld dword ptr [esp + 0x20] // 007ae216
        fld dword ptr [esp + 0x14] // 007ae21a
        fld st(0) // 007ae21e
        fsubp st(2),st(0) // 007ae220
        fxch // 007ae222
        fstp dword ptr [esp + 0x4] // 007ae224
        fld dword ptr [esp + 0x24] // 007ae228
        fld dword ptr [esp + 0x18] // 007ae22c
        fld st(0) // 007ae230
        fsubp st(2),st(0) // 007ae232
        fxch // 007ae234
        fstp dword ptr [esp + 0x8] // 007ae236
        fld dword ptr [esp] // 007ae23a
        fld dword ptr [esp + 0x28] // 007ae23d
        fld st(0) // 007ae241
        fmulp st(2), st(0) // 007ae243
        fxch // 007ae245
        fstp dword ptr [esp + 0x1c] // 007ae247
        fld dword ptr [esp + 0x4] // 007ae24b
        fmul st(0), st(1) // 007ae24f
        fstp dword ptr [esp + 0x20] // 007ae251
        fmul dword ptr [esp + 0x8] // 007ae255
        fstp dword ptr [esp + 0x24] // 007ae259
        fld dword ptr [esp + 0x1c] // 007ae25d
        faddp st(3),st(0) // 007ae261
        fxch st(2) // 007ae263
        fstp dword ptr [eax] // 007ae265
        fadd dword ptr [esp + 0x20] // 007ae267
        fstp dword ptr [eax + 0x4] // 007ae26b
        fadd dword ptr [esp + 0x24] // 007ae26e
        fstp dword ptr [eax + 0x8] // 007ae272
        add esp,0xc // 007ae275
        ret 0x1c // 007ae278
    }
}

//007B053B..007B0599, retaining four FLD/FSTP copies, rounded reciprocal,
// three staged products, then ordered output stores.
__declspec(naked) void __fastcall divide_position_kernel(float*, const float*) {
    __asm {
        sub esp, 0x20
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [edx + 8]
        fstp dword ptr [esp + 8]
        fld dword ptr [edx + 0xc]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [esp + 0xc]
        fld1
        fdivrp st(1), st(0)
        fstp dword ptr [esp + 0x10]
        fld dword ptr [esp]
        fld dword ptr [esp + 0x10]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 0x14]
        fld dword ptr [esp + 4]
        fmul st(0), st(1)
        fstp dword ptr [esp + 0x18]
        fmul dword ptr [esp + 8]
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [esp + 0x14]
        fstp dword ptr [ecx]
        fld dword ptr [esp + 0x18]
        fstp dword ptr [ecx + 4]
        fld dword ptr [esp + 0x1c]
        fstp dword ptr [ecx + 8]
        add esp, 0x20
        ret
    }
}


} // namespace

std::array<float, 3>* lerp_camera_path_vector_007ae200(std::array<float, 3>& destination,
    std::array<float, 3> from, std::array<float, 3> to, float parameter) {
    lerp_kernel(destination.data(), nullptr, from[0], from[1], from[2],
        to[0], to[1], to[2], parameter);
    return &destination;
}

void sample_camera_path_linear_007afac0(CameraPathView& path, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction, CameraPathHost& host) {
    std::uint32_t index = 0;
    void** begin;
    for (;;) {
        check(path, path.knots_begin_08, index, host);
        float* first = words(path, index, host);
        begin = path.knots_begin_08;
        check(path, begin, index, host);
        begin = path.knots_begin_08;
        float* second = words(path, index, host);
        if (reaches_segment(parameter, first, second)) break;
        ++index;
    }
    const auto next = static_cast<std::int32_t>(index) < count(path, begin) - 1 ? index + 1u : 0u;
    check(path, begin, index, host);
    float* current = words(path, index, host);
    check(path, path.knots_begin_08, index, host);
    float* duration_record = words(path, index, host);
    begin = path.knots_begin_08;
    float t = local_parameter(parameter, current, duration_record);
    copy_float(t, t); // Native by-value parameter copy at 007AFB8E.
    check(path, begin, next, host);
    float* next_record = words(path, next, host);
    std::array<float, 3> to;
    for (unsigned axis = 0; axis != 3; ++axis) copy_float(to[axis], next_record[axis]);
    check(path, path.knots_begin_08, index, host);
    current = words(path, index, host);
    std::array<float, 3> from, result;
    for (unsigned axis = 0; axis != 3; ++axis) copy_float(from[axis], current[axis]);
    lerp_camera_path_vector_007ae200(result, from, to, t);
    for (unsigned axis = 0; axis != 3; ++axis) copy_float(position[axis], result[axis]);
    if (direction != nullptr) {
        check(path, path.knots_begin_08, index, host);
        current = words(path, index, host);
        for (unsigned axis = 0; axis != 3; ++axis) copy_float((*direction)[axis], current[axis + 3]);
    }
}

void sample_camera_path_local_007afe80(CameraPathView& path, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction,
    std::uint32_t flags, CameraPathHost& host) {
    if (count(path, path.knots_begin_08) < 3) {
        float copied_parameter;
        copy_float(copied_parameter, parameter);
        sample_camera_path_linear_007afac0(path, copied_parameter, position, direction, host);
        return;
    }
    std::uint32_t index = 0;
    void** begin;
    for (;;) {
        check(path, path.knots_begin_08, index, host);
        float* first = words(path, index, host);
        begin = path.knots_begin_08;
        check(path, begin, index, host);
        begin = path.knots_begin_08;
        if (reaches_segment(parameter, first, words(path, index, host))) break;
        ++index;
    }
    check(path, begin, index, host);
    begin = path.knots_begin_08;
    float* current = words(path, index, host);
    const auto previous_index = static_cast<std::int32_t>(index) > 0 ? index - 1u
        : static_cast<std::uint32_t>(count(path, begin)) - (path.wrap_byte_24 != 0 ? 2u : 1u);
    check(path, begin, previous_index, host);
    float* previous = words(path, previous_index, host);
    begin = path.knots_begin_08;
    const auto next_index = static_cast<std::int32_t>(index) < count(path, begin) - 1
        ? index + 1u : (path.wrap_byte_24 != 0 ? 1u : 0u);
    check(path, begin, next_index, host);
    begin = path.knots_begin_08;
    float* next = words(path, next_index, host);
    // Byte flag and first/penultimate index gates are distinct from wrap_byte.
    unsigned endpoint = 0;
    if ((flags & 0xffu) != 0) {
        if (index == 0) endpoint = 1;
        else if (index == static_cast<std::uint32_t>(count(path, begin)) - 2u) endpoint = 2;
    }
    cubic_kernel(current, next, parameter, position.data(),
        direction == nullptr ? nullptr : direction->data(), endpoint, previous);
}

void sample_camera_path_world_007b04c0(CameraPathView& path, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction,
    std::uint32_t flags, CameraPathHost& host) {
    float copied_parameter;
    copy_float(copied_parameter, parameter);
    sample_camera_path_local_007afe80(path, copied_parameter, position, direction, flags, host);
    auto& pose = host.resolve_pose(path.parent_14);
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    CameraPlane input, transformed;
    std::memcpy(input.data(), position.data(), 3 * sizeof(float)); // Native MOVSS.
    input[3] = 1.0f; // 00D7A24C verified 3F800000.
    transform_camera_plane_00b65ba0(transformed, input, pose.world_cc);
    divide_position_kernel(position.data(), transformed.data());
    if (direction != nullptr) {
        auto& direction_pose = host.resolve_pose(path.parent_14);
        if (direction_pose.world_valid_c8 == 0) refresh_pose_00414db0(direction_pose);
        std::array<float, 3> result;
        transform_effect_direction_0042d0d0_no_normalize(result, *direction, direction_pose.world_cc);
        for (unsigned axis = 0; axis != 3; ++axis) copy_float((*direction)[axis], result[axis]);
    }
}

} // namespace bsp
