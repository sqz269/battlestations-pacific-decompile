#include "bsp/native_dyn_solver_mode0.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mode0 solver requires MSVC Win32 x87 assembly.
#endif
// Consume the platform stack probe; do not copy a CRT implementation.
extern "C" void _chkstk();
namespace bsp {
namespace {
using U=std::uint32_t;
struct Context {CameraAxesCrtAccess crt;const AvoidZoneDynHullMemory* memory;NativeDynSolverMode0Calls* calls;};
static_assert(std::is_standard_layout_v<Context> && offsetof(Context,crt)==0);
void* __cdecl allocate_bridge(U site,Context* c,U size){return c->calls->allocate_00bf55be(site,size,*c->memory);}
void __cdecl row_free_bridge(U site,Context* c,void* p){c->calls->free_00bf65ac(site,p,*c->memory);}
void __cdecl velocity_free_bridge(U site,Context* c,void* p){c->calls->free_00bf6989(site,p,*c->memory);}
void task_kernel();
void rows_size_kernel();
void store_kernel();
void writeback_kernel();
void friction_kernel();
void normal_kernel();
void warm_kernel();
void build_rows_kernel();
void prestep_kernel();
void abs_kernel();
alignas(8) const std::uint32_t constant_00d7a208=0x80000000U;
alignas(8) const std::uint64_t constant_00d7a270=0x3fa99999a0000000ULL;
alignas(8) const std::uint32_t constant_00d7a288=0x358637bdU;
__declspec(naked) void sqrt_kernel(){
    __asm {
        push ebp
        mov ebp,esp
        and esp,-8
        sub esp,8
        fld dword ptr [ebp+0ch]
        push ecx
        mov ecx,dword ptr [ebp+8]
        call native_crt_sqrt_st0_00bf7030
        pop ecx
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        mov esp,ebp
        pop ebp
        ret 8
    }
}
__declspec(naked) void prestep_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+0ch]
        call prestep_kernel
        ret 8
    }
}
__declspec(naked) void build_rows_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+0ch]
        call build_rows_kernel
        ret 8
    }
}
__declspec(naked) void bridge_004037ea(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 0004037eah
        call row_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_004037fe(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 0004037feh
        call velocity_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c31c4f(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c31c4fh
        call row_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c31c58(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c31c58h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c4f081(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c4f081h
        call velocity_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c4f09c(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c4f09ch
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void task_kernel(){
    __asm {
        push -1 // 00403720
        mov eax, dword ptr fs:[0] // 00403722
        push 0 // 00403728
        push eax // 0040372d
        mov eax, 0eaach // 0040372e
        // 00403733: native FS exception registration omitted; normal-return contract.
        call _chkstk // 0040373a
        push ebp // 0040373f
        push esi // 00403740
        xor ebp, ebp // 00403741
        mov esi, ecx // 00403743
        mov eax, dword ptr [esi + 8] // 00403745
        push edi // 00403748
        mov dword ptr [esp + 0ea78h], ebp // 00403749
        mov dword ptr [esp + 0ea7ch], ebp // 00403750
        mov dword ptr [esp + 0ch], eax // 00403757
        mov dword ptr [esp + 0ea84h], ebp // 0040375b
        mov dword ptr [esp + 0ea80h], ebp // 00403762
        mov dword ptr [esp + 0eac0h], ebp // 00403769
        mov edi, dword ptr [esi + 0ch] // 00403770
        cmp edi, dword ptr [esi + 010h] // 00403773
        jg l_00403806 // 00403776
        push ebx // 0040377c
        lea ebx, [edi + edi*2] // 0040377d
        add ebx, ebx // 00403780
        add ebx, ebx // 00403782
    l_00403784:
        mov eax, dword ptr [esi + 8] // 00403784
        fld dword ptr [esi + 014h] // 00403787
        mov eax, dword ptr [eax + 044ch] // 0040378a
        push ecx // 00403790
        add eax, ebx // 00403791
        fstp dword ptr [esp] // 00403793
        lea ecx, [esp + 014h] // 00403796
        push dword ptr [esp+60112] // Borrowed context, original frame offsets retained.
        call prestep_shim // 0040379a
        mov ecx, dword ptr [esp + 010h] // 0040379f
        mov eax, dword ptr [ecx + 038h] // 004037a3
        cmp eax, ebp // 004037a6
        jle l_004037c3 // 004037a8
        mov ebp, eax // 004037aa
        _emit 08dh // 004037ac
        _emit 064h
        _emit 024h
        _emit 000h
    l_004037b0:
        lea ecx, [esp + 010h] // 004037b0
        call normal_kernel // 004037b4
        call friction_kernel // 004037b9
        sub ebp, 1 // 004037be
        jne l_004037b0 // 004037c1
    l_004037c3:
        lea edx, [esp + 010h] // 004037c3
        call writeback_kernel // 004037c7
        push edx // 004037cc
        call store_kernel // 004037cd
        add edi, 1 // 004037d2
        add ebx, 0ch // 004037d5
        cmp edi, dword ptr [esi + 010h] // 004037d8
        jle l_00403784 // 004037db
        mov eax, dword ptr [esp + 0ea84h] // 004037dd
        cmp eax, ebp // 004037e4
        pop ebx // 004037e6
        je l_004037f2 // 004037e7
        push eax // 004037e9
        push dword ptr [esp+60108] // Borrowed context, original frame offsets retained.
        call bridge_004037ea // 004037ea
        add esp, 4 // 004037ef
    l_004037f2:
        mov eax, dword ptr [esp + 0ea78h] // 004037f2
        cmp eax, ebp // 004037f9
        je l_00403806 // 004037fb
        push eax // 004037fd
        push dword ptr [esp+60108] // Borrowed context, original frame offsets retained.
        call bridge_004037fe // 004037fe
        add esp, 4 // 00403803
    l_00403806:
        mov ecx, dword ptr [esp + 0eab8h] // 00403806
        pop edi // 0040380d
        pop esi // 0040380e
        pop ebp // 0040380f
        // 00403810: native FS exception registration omitted; normal-return contract.
        add esp, 0eab8h // 00403817
        ret 4 // 0040381d
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void rows_size_kernel(){
    __asm {
        cmp dword ptr [esi + 0ea78h], edi // 00c31c30
        jge l_00c31c67 // 00c31c36
        mov eax, dword ptr [esi + 0ea74h] // 00c31c38
        push ebx // 00c31c3e
        mov ebx, edi // 00c31c3f
        imul ebx, ebx, 07ch // 00c31c41
        test eax, eax // 00c31c44
        mov dword ptr [esi + 0ea78h], edi // 00c31c46
        je l_00c31c57 // 00c31c4c
        push eax // 00c31c4e
        push dword ptr [esp+12] // Borrowed context, original frame offsets retained.
        call bridge_00c31c4f // 00c31c4f
        add esp, 4 // 00c31c54
    l_00c31c57:
        push ebx // 00c31c57
        push dword ptr [esp+12] // Borrowed context, original frame offsets retained.
        call bridge_00c31c58 // 00c31c58
        add esp, 4 // 00c31c5d
        mov dword ptr [esi + 0ea74h], eax // 00c31c60
        pop ebx // 00c31c66
    l_00c31c67:
        mov ecx, dword ptr [esi + 0ea74h] // 00c31c67
        lea eax, [edi + edi*2] // 00c31c6d
        shl eax, 4 // 00c31c70
        mov dword ptr [esi + 0ea7ch], ecx // 00c31c73
        add ecx, eax // 00c31c79
        add eax, ecx // 00c31c7b
        mov dword ptr [esi + 0ea84h], eax // 00c31c7d
        lea eax, [eax + edi*4] // 00c31c83
        mov dword ptr [esi + 0ea88h], eax // 00c31c86
        lea eax, [eax + edi*4] // 00c31c8c
        mov dword ptr [esi + 0ea90h], eax // 00c31c8f
        lea eax, [eax + edi*4] // 00c31c95
        mov dword ptr [esi + 0ea8ch], eax // 00c31c98
        lea eax, [eax + edi*4] // 00c31c9e
        mov dword ptr [esi + 0ea94h], eax // 00c31ca1
        lea eax, [eax + edi*4] // 00c31ca7
        mov dword ptr [esi + 0ea9ch], eax // 00c31caa
        lea eax, [eax + edi*4] // 00c31cb0
        mov dword ptr [esi + 0ea80h], ecx // 00c31cb3
        mov dword ptr [esi + 0ea98h], eax // 00c31cb9
        ret 4 // 00c31cbf
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void store_kernel(){
    __asm {
        sub esp, 8 // 00c35020
        push ebx // 00c35023
        push edi // 00c35024
        mov edi, dword ptr [esp + 014h] // 00c35025
        mov eax, dword ptr [edi + 0ea68h] // 00c35029
        mov ecx, dword ptr [eax + 4] // 00c3502f
        xor ebx, ebx // 00c35032
        test ecx, ecx // 00c35034
        jle l_00c3509b // 00c35036
        mov eax, dword ptr [eax] // 00c35038
        push ebp // 00c3503a
        mov dword ptr [esp + 0ch], eax // 00c3503b
        mov dword ptr [esp + 010h], ecx // 00c3503f
        push esi // 00c35043
    l_00c35044:
        mov esi, dword ptr [eax] // 00c35044
        xor edx, edx // 00c35046
        cmp dword ptr [esi + 0c8h], edx // 00c35048
        jle l_00c35087 // 00c3504e
        mov ebp, dword ptr [edi + 0ea94h] // 00c35050
        mov edi, dword ptr [edi + 0ea8ch] // 00c35056
        lea eax, [ebp + ebx*4] // 00c3505c
        lea ecx, [esi + 030h] // 00c35060
        sub edi, ebp // 00c35063
    l_00c35065:
        fld dword ptr [edi + eax] // 00c35065
        add edx, 1 // 00c35068
        fstp dword ptr [ecx - 4] // 00c3506b
        add ebx, 1 // 00c3506e
        fld dword ptr [eax] // 00c35071
        add eax, 4 // 00c35073
        fstp dword ptr [ecx] // 00c35076
        add ecx, 030h // 00c35078
        cmp edx, dword ptr [esi + 0c8h] // 00c3507b
        jl l_00c35065 // 00c35081
        mov edi, dword ptr [esp + 01ch] // 00c35083
    l_00c35087:
        mov eax, dword ptr [esp + 010h] // 00c35087
        add eax, 4 // 00c3508b
        sub dword ptr [esp + 014h], 1 // 00c3508e
        mov dword ptr [esp + 010h], eax // 00c35093
        jne l_00c35044 // 00c35097
        pop esi // 00c35099
        pop ebp // 00c3509a
    l_00c3509b:
        pop edi // 00c3509b
        pop ebx // 00c3509c
        add esp, 8 // 00c3509d
        ret 4 // 00c350a0
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void writeback_kernel(){
    __asm {
        push ebp // 00c37b50
        mov ebp, 1 // 00c37b51
        cmp dword ptr [edx + 0eaa0h], ebp // 00c37b56
        jle l_00c37c37 // 00c37b5c
        push ebx // 00c37b62
        push esi // 00c37b63
        push edi // 00c37b64
        mov esi, 030h // 00c37b65
        lea ebx, [edx + 0ch] // 00c37b6a
        _emit 08dh // 00c37b6d
        _emit 049h
        _emit 000h
    l_00c37b70:
        mov ecx, dword ptr [edx + 0ea6ch] // 00c37b70
        fld dword ptr [ecx + esi] // 00c37b76
        mov edi, dword ptr [ebx] // 00c37b79
        mov eax, dword ptr [edi + 4] // 00c37b7b
        fadd dword ptr [eax] // 00c37b7e
        add ecx, esi // 00c37b80
        add ebp, 1 // 00c37b82
        add ebx, 4 // 00c37b85
        fstp dword ptr [eax] // 00c37b88
        fld dword ptr [ecx + 4] // 00c37b8a
        fadd dword ptr [eax + 4] // 00c37b8d
        fstp dword ptr [eax + 4] // 00c37b90
        fld dword ptr [ecx + 8] // 00c37b93
        fadd dword ptr [eax + 8] // 00c37b96
        fstp dword ptr [eax + 8] // 00c37b99
        mov eax, dword ptr [edx + 0ea6ch] // 00c37b9c
        lea ecx, [esi + eax + 0ch] // 00c37ba2
        mov eax, dword ptr [edi + 4] // 00c37ba6
        fld dword ptr [eax + 0ch] // 00c37ba9
        add eax, 0ch // 00c37bac
        fadd dword ptr [ecx] // 00c37baf
        fstp dword ptr [eax] // 00c37bb1
        fld dword ptr [ecx + 4] // 00c37bb3
        fadd dword ptr [eax + 4] // 00c37bb6
        fstp dword ptr [eax + 4] // 00c37bb9
        fld dword ptr [ecx + 8] // 00c37bbc
        fadd dword ptr [eax + 8] // 00c37bbf
        fstp dword ptr [eax + 8] // 00c37bc2
        mov dword ptr [edi + 058h], 0ffffffffh // 00c37bc5
        mov ecx, dword ptr [edx + 0ea6ch] // 00c37bcc
        mov edi, dword ptr [ebx - 4] // 00c37bd2
        mov eax, dword ptr [edi + 4] // 00c37bd5
        fld dword ptr [eax + 020h] // 00c37bd8
        add eax, 020h // 00c37bdb
        fadd dword ptr [esi + ecx + 018h] // 00c37bde
        lea ecx, [esi + ecx + 018h] // 00c37be2
        fstp dword ptr [eax] // 00c37be6
        fld dword ptr [ecx + 4] // 00c37be8
        fadd dword ptr [eax + 4] // 00c37beb
        fstp dword ptr [eax + 4] // 00c37bee
        fld dword ptr [ecx + 8] // 00c37bf1
        fadd dword ptr [eax + 8] // 00c37bf4
        fstp dword ptr [eax + 8] // 00c37bf7
        mov eax, dword ptr [edx + 0ea6ch] // 00c37bfa
        fld dword ptr [esi + eax + 024h] // 00c37c00
        lea ecx, [esi + eax + 024h] // 00c37c04
        mov eax, dword ptr [edi + 4] // 00c37c08
        fadd dword ptr [eax + 02ch] // 00c37c0b
        add eax, 02ch // 00c37c0e
        add esi, 030h // 00c37c11
        fstp dword ptr [eax] // 00c37c14
        fld dword ptr [ecx + 4] // 00c37c16
        fadd dword ptr [eax + 4] // 00c37c19
        fstp dword ptr [eax + 4] // 00c37c1c
        fld dword ptr [ecx + 8] // 00c37c1f
        fadd dword ptr [eax + 8] // 00c37c22
        fstp dword ptr [eax + 8] // 00c37c25
        cmp ebp, dword ptr [edx + 0eaa0h] // 00c37c28
        jl l_00c37b70 // 00c37c2e
        pop edi // 00c37c34
        pop esi // 00c37c35
        pop ebx // 00c37c36
    l_00c37c37:
        pop ebp // 00c37c37
        ret  // 00c37c38
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void friction_kernel(){
    __asm {
        sub esp, 048h // 00c42230
        mov edx, dword ptr [ecx + 0ea80h] // 00c42233
        push ebp // 00c42239
        mov ebp, dword ptr [ecx + 0eaa8h] // 00c4223a
        push esi // 00c42240
        mov esi, dword ptr [ecx + 0ea7ch] // 00c42241
        lea eax, [ebp + ebp*2] // 00c42247
        shl eax, 4 // 00c4224b
        add esi, eax // 00c4224e
        add edx, eax // 00c42250
        mov eax, ebp // 00c42252
        add eax, dword ptr [ecx + 0eaa4h] // 00c42254
        mov dword ptr [esp + 0ch], esi // 00c4225a
        cmp ebp, eax // 00c4225e
        mov dword ptr [esp + 010h], edx // 00c42260
        mov dword ptr [esp + 01ch], ebp // 00c42264
        jge l_00c42520 // 00c42268
        push ebx // 00c4226e
        add edx, 014h // 00c4226f
        add esi, 014h // 00c42272
        push edi // 00c42275
        jmp l_00c42280 // 00c42276
        _emit 08dh // 00c42278
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        nop  // 00c4227f
    l_00c42280:
        mov eax, dword ptr [ecx + 0ea84h] // 00c42280
        movsx ebx, word ptr [eax + ebp*4 + 2] // 00c42286
        movsx edi, word ptr [eax + ebp*4] // 00c4228b
        lea eax, [eax + ebp*4] // 00c4228f
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42292
        lea ebx, [ebx + ebx*2] // 00c42298
        shl ebx, 4 // 00c4229b
        mov dword ptr [esp + 01ch], ebx // 00c4229e
        mov ebx, dword ptr [ecx + 0ea88h] // 00c422a2
        lea edi, [edi + edi*2] // 00c422a8
        shl edi, 4 // 00c422ab
        fld dword ptr [ebx + ebp*4] // 00c422ae
        mov ebx, dword ptr [esp + 014h] // 00c422b1
        fld dword ptr [esi - 010h] // 00c422b5
        fmul dword ptr [eax + edi + 4] // 00c422b8
        fld dword ptr [eax + edi] // 00c422bc
        fmul dword ptr [ebx] // 00c422bf
        mov ebx, dword ptr [esp + 01ch] // 00c422c1
        faddp st(1), st(0) // 00c422c5
        fld dword ptr [esi - 0ch] // 00c422c7
        fmul dword ptr [eax + edi + 8] // 00c422ca
        faddp st(1), st(0) // 00c422ce
        fstp dword ptr [esp + 020h] // 00c422d0
        fsub dword ptr [esp + 020h] // 00c422d4
        fstp dword ptr [esp + 020h] // 00c422d8
        fld dword ptr [esp + 020h] // 00c422dc
        fld dword ptr [esi - 4] // 00c422e0
        fmul dword ptr [eax + edi + 010h] // 00c422e3
        fld dword ptr [esi - 8] // 00c422e7
        fmul dword ptr [eax + edi + 0ch] // 00c422ea
        faddp st(1), st(0) // 00c422ee
        fld dword ptr [eax + edi + 014h] // 00c422f0
        fmul dword ptr [esi] // 00c422f4
        faddp st(1), st(0) // 00c422f6
        fstp dword ptr [esp + 020h] // 00c422f8
        fsub dword ptr [esp + 020h] // 00c422fc
        fstp dword ptr [esp + 020h] // 00c42300
        fld dword ptr [esp + 020h] // 00c42304
        fld dword ptr [esi + 8] // 00c42308
        fmul dword ptr [ebx + eax + 4] // 00c4230b
        fld dword ptr [esi + 4] // 00c4230f
        fmul dword ptr [ebx + eax] // 00c42312
        faddp st(1), st(0) // 00c42315
        fld dword ptr [esi + 0ch] // 00c42317
        fmul dword ptr [ebx + eax + 8] // 00c4231a
        faddp st(1), st(0) // 00c4231e
        fstp dword ptr [esp + 020h] // 00c42320
        fsub dword ptr [esp + 020h] // 00c42324
        fstp dword ptr [esp + 020h] // 00c42328
        fld dword ptr [esp + 020h] // 00c4232c
        fld dword ptr [esi + 014h] // 00c42330
        fmul dword ptr [ebx + eax + 010h] // 00c42333
        fld dword ptr [esi + 010h] // 00c42337
        fmul dword ptr [ebx + eax + 0ch] // 00c4233a
        faddp st(1), st(0) // 00c4233e
        fld dword ptr [esi + 018h] // 00c42340
        fmul dword ptr [ebx + eax + 014h] // 00c42343
        mov eax, dword ptr [ecx + 0ea98h] // 00c42347
        faddp st(1), st(0) // 00c4234d
        fstp dword ptr [esp + 020h] // 00c4234f
        fsub dword ptr [esp + 020h] // 00c42353
        fstp dword ptr [esp + 020h] // 00c42357
        fld dword ptr [esp + 020h] // 00c4235b
        fmul dword ptr [eax + ebp*4] // 00c4235f
        mov eax, dword ptr [ecx + 0ea8ch] // 00c42362
        fstp dword ptr [esp + 020h] // 00c42368
        fld dword ptr [esp + 020h] // 00c4236c
        fadd dword ptr [eax + ebp*4] // 00c42370
        sub ebp, dword ptr [ecx + 0eaa8h] // 00c42373
        mov eax, dword ptr [ecx + 0ea9ch] // 00c42379
        add ebp, ebp // 00c4237f
        fstp dword ptr [esp + 010h] // 00c42381
        add ebp, ebp // 00c42385
        fld dword ptr [eax + ebp] // 00c42387
        mov eax, dword ptr [ecx + 0ea8ch] // 00c4238a
        fmul dword ptr [eax + ebp] // 00c42390
        fstp dword ptr [esp + 020h] // 00c42393
        fld dword ptr [esp + 020h] // 00c42397
        fld st(0) // 00c4239b
        fchs  // 00c4239d
        fstp dword ptr [esp + 01ch] // 00c4239f
        fld dword ptr [esp + 010h] // 00c423a3
        fld dword ptr [esp + 01ch] // 00c423a7
        fcomip st(0), st(1) // 00c423ab
        jbe l_00c423bb // 00c423ad
        movss xmm0, dword ptr [esp + 01ch] // 00c423af
        fstp st(1) // 00c423b5
        fstp st(0) // 00c423b7
        jmp l_00c423c7 // 00c423b9
    l_00c423bb:
        fcomip st(0), st(1) // 00c423bb
        fstp st(0) // 00c423bd
        jbe l_00c423cd // 00c423bf
        movss xmm0, dword ptr [esp + 020h] // 00c423c1
    l_00c423c7:
        movss dword ptr [esp + 010h], xmm0 // 00c423c7
    l_00c423cd:
        mov ebp, dword ptr [esp + 024h] // 00c423cd
        fld dword ptr [esp + 010h] // 00c423d1
        fsub dword ptr [eax + ebp*4] // 00c423d5
        movss xmm0, dword ptr [esp + 010h] // 00c423d8
        movss dword ptr [eax + ebp*4], xmm0 // 00c423de
        lea eax, [eax + ebp*4] // 00c423e3
        fstp dword ptr [esp + 024h] // 00c423e6
        mov eax, dword ptr [esp + 018h] // 00c423ea
        fld dword ptr [eax] // 00c423ee
        mov eax, dword ptr [ecx + 0ea6ch] // 00c423f0
        fld dword ptr [esp + 024h] // 00c423f6
        add eax, edi // 00c423fa
        fld st(0) // 00c423fc
        fmulp st(2), st(0) // 00c423fe
        fxch st(1) // 00c42400
        fstp dword ptr [esp + 028h] // 00c42402
        fld dword ptr [edx - 010h] // 00c42406
        fmul st(0), st(1) // 00c42409
        fstp dword ptr [esp + 02ch] // 00c4240b
        fld dword ptr [edx - 0ch] // 00c4240f
        fmul st(0), st(1) // 00c42412
        fstp dword ptr [esp + 030h] // 00c42414
        fld dword ptr [eax] // 00c42418
        fadd dword ptr [esp + 028h] // 00c4241a
        fstp dword ptr [eax] // 00c4241e
        fld dword ptr [esp + 02ch] // 00c42420
        fadd dword ptr [eax + 4] // 00c42424
        fstp dword ptr [eax + 4] // 00c42427
        fld dword ptr [eax + 8] // 00c4242a
        fadd dword ptr [esp + 030h] // 00c4242d
        fstp dword ptr [eax + 8] // 00c42431
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42434
        fld dword ptr [edx - 8] // 00c4243a
        lea edi, [eax + edi + 0ch] // 00c4243d
        fmul st(0), st(1) // 00c42441
        fstp dword ptr [esp + 034h] // 00c42443
        fld dword ptr [edx - 4] // 00c42447
        fmul st(0), st(1) // 00c4244a
        fstp dword ptr [esp + 038h] // 00c4244c
        fld st(0) // 00c42450
        fmul dword ptr [edx] // 00c42452
        fstp dword ptr [esp + 03ch] // 00c42454
        fld dword ptr [edi] // 00c42458
        fadd dword ptr [esp + 034h] // 00c4245a
        fstp dword ptr [edi] // 00c4245e
        fld dword ptr [edi + 4] // 00c42460
        fadd dword ptr [esp + 038h] // 00c42463
        fstp dword ptr [edi + 4] // 00c42467
        fld dword ptr [edi + 8] // 00c4246a
        fadd dword ptr [esp + 03ch] // 00c4246d
        fstp dword ptr [edi + 8] // 00c42471
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42474
        fld dword ptr [edx + 4] // 00c4247a
        add eax, ebx // 00c4247d
        fmul st(0), st(1) // 00c4247f
        fstp dword ptr [esp + 040h] // 00c42481
        fld dword ptr [edx + 8] // 00c42485
        fmul st(0), st(1) // 00c42488
        fstp dword ptr [esp + 044h] // 00c4248a
        fld dword ptr [edx + 0ch] // 00c4248e
        fmul st(0), st(1) // 00c42491
        fstp dword ptr [esp + 048h] // 00c42493
        fld dword ptr [eax] // 00c42497
        fadd dword ptr [esp + 040h] // 00c42499
        fstp dword ptr [eax] // 00c4249d
        fld dword ptr [eax + 4] // 00c4249f
        fadd dword ptr [esp + 044h] // 00c424a2
        fstp dword ptr [eax + 4] // 00c424a6
        fld dword ptr [eax + 8] // 00c424a9
        fadd dword ptr [esp + 048h] // 00c424ac
        fstp dword ptr [eax + 8] // 00c424b0
        mov eax, dword ptr [ecx + 0ea6ch] // 00c424b3
        fld dword ptr [edx + 010h] // 00c424b9
        lea ebx, [eax + ebx + 0ch] // 00c424bc
        fmul st(0), st(1) // 00c424c0
        fstp dword ptr [esp + 04ch] // 00c424c2
        fld dword ptr [edx + 014h] // 00c424c6
        fmul st(0), st(1) // 00c424c9
        fstp dword ptr [esp + 050h] // 00c424cb
        fmul dword ptr [edx + 018h] // 00c424cf
        fstp dword ptr [esp + 054h] // 00c424d2
        fld dword ptr [ebx] // 00c424d6
        mov eax, 030h // 00c424d8
        fadd dword ptr [esp + 04ch] // 00c424dd
        add dword ptr [esp + 014h], eax // 00c424e1
        add dword ptr [esp + 018h], eax // 00c424e5
        add esi, eax // 00c424e9
        fstp dword ptr [ebx] // 00c424eb
        add edx, eax // 00c424ed
        fld dword ptr [ebx + 4] // 00c424ef
        add ebp, 1 // 00c424f2
        fadd dword ptr [esp + 050h] // 00c424f5
        mov dword ptr [esp + 024h], ebp // 00c424f9
        fstp dword ptr [ebx + 4] // 00c424fd
        fld dword ptr [esp + 054h] // 00c42500
        fadd dword ptr [ebx + 8] // 00c42504
        fstp dword ptr [ebx + 8] // 00c42507
        mov eax, dword ptr [ecx + 0eaa8h] // 00c4250a
        add eax, dword ptr [ecx + 0eaa4h] // 00c42510
        cmp ebp, eax // 00c42516
        jl l_00c42280 // 00c42518
        pop edi // 00c4251e
        pop ebx // 00c4251f
    l_00c42520:
        pop esi // 00c42520
        pop ebp // 00c42521
        add esp, 048h // 00c42522
        ret  // 00c42525
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void normal_kernel(){
    __asm {
        sub esp, 0c4h // 00c42530
        mov eax, dword ptr [ecx + 0ea7ch] // 00c42536
        push ebx // 00c4253c
        push edi // 00c4253d
        mov edi, dword ptr [ecx + 0ea80h] // 00c4253e
        xor ebx, ebx // 00c42544
        cmp dword ptr [ecx + 0eaa4h], ebx // 00c42546
        mov dword ptr [esp + 018h], eax // 00c4254c
        mov dword ptr [esp + 01ch], edi // 00c42550
        jle l_00c42b66 // 00c42554
        xorps xmm0, xmm0 // 00c4255a
        add edi, 014h // 00c4255d
        add eax, 014h // 00c42560
        push ebp // 00c42563
        mov dword ptr [esp + 018h], eax // 00c42564
        push esi // 00c42568
        _emit 08dh // 00c42569
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c42570:
        mov ebp, dword ptr [esp + 01ch] // 00c42570
        movss xmm1, dword ptr [ebp - 010h] // 00c42574
        mov ebp, dword ptr [esp + 020h] // 00c42579
        mov eax, dword ptr [ecx + 0ea84h] // 00c4257d
        movsx edx, word ptr [eax + ebx*4] // 00c42583
        movss dword ptr [esp + 070h], xmm1 // 00c42587
        movss xmm1, dword ptr [ebp] // 00c4258d
        mov ebp, dword ptr [esp + 01ch] // 00c42592
        movsx esi, word ptr [eax + ebx*4 + 2] // 00c42596
        movss dword ptr [esp + 060h], xmm1 // 00c4259b
        movss xmm1, dword ptr [ebp - 0ch] // 00c425a1
        movss dword ptr [esp + 048h], xmm1 // 00c425a6
        movss xmm1, dword ptr [ebp - 4] // 00c425ac
        movss dword ptr [esp + 050h], xmm1 // 00c425b1
        movss xmm1, dword ptr [ebp - 8] // 00c425b7
        movss dword ptr [esp + 034h], xmm1 // 00c425bc
        movss xmm1, dword ptr [ebp] // 00c425c2
        movss dword ptr [esp + 054h], xmm1 // 00c425c7
        movss xmm1, dword ptr [ebp + 8] // 00c425cd
        movss dword ptr [esp + 03ch], xmm1 // 00c425d2
        movss xmm1, dword ptr [ebp + 4] // 00c425d8
        movss dword ptr [esp + 05ch], xmm1 // 00c425dd
        movss xmm1, dword ptr [ebp + 0ch] // 00c425e3
        lea eax, [eax + ebx*4] // 00c425e8
        mov eax, dword ptr [ecx + 0ea6ch] // 00c425eb
        movss dword ptr [esp + 044h], xmm1 // 00c425f1
        movss xmm1, dword ptr [ebp + 014h] // 00c425f7
        movss dword ptr [esp + 064h], xmm1 // 00c425fc
        movss xmm1, dword ptr [ebp + 010h] // 00c42602
        movss dword ptr [esp + 04ch], xmm1 // 00c42607
        movss xmm1, dword ptr [ebp + 018h] // 00c4260d
        mov ebp, dword ptr [ecx + 0ea88h] // 00c42612
        fld dword ptr [ebp + ebx*4] // 00c42618
        lea edx, [edx + edx*2] // 00c4261c
        shl edx, 4 // 00c4261f
        fld dword ptr [eax + edx + 4] // 00c42622
        lea esi, [esi + esi*2] // 00c42626
        fld dword ptr [esp + 070h] // 00c42629
        shl esi, 4 // 00c4262d
        fld st(0) // 00c42630
        movss dword ptr [esp + 06ch], xmm1 // 00c42632
        fmulp st(2), st(0) // 00c42638
        fld dword ptr [eax + edx] // 00c4263a
        fmul dword ptr [esp + 060h] // 00c4263d
        faddp st(2), st(0) // 00c42641
        fld dword ptr [eax + edx + 8] // 00c42643
        fmul dword ptr [esp + 048h] // 00c42647
        faddp st(2), st(0) // 00c4264b
        fxch st(1) // 00c4264d
        fstp dword ptr [esp + 010h] // 00c4264f
        fld dword ptr [esp + 010h] // 00c42653
        fsubp st(2), st(0) // 00c42657
        fxch st(1) // 00c42659
        fstp dword ptr [esp + 010h] // 00c4265b
        fld dword ptr [esp + 010h] // 00c4265f
        fld dword ptr [eax + edx + 010h] // 00c42663
        fmul dword ptr [esp + 050h] // 00c42667
        fld dword ptr [esp + 034h] // 00c4266b
        fmul dword ptr [eax + edx + 0ch] // 00c4266f
        faddp st(1), st(0) // 00c42673
        fld dword ptr [eax + edx + 014h] // 00c42675
        fmul dword ptr [esp + 054h] // 00c42679
        faddp st(1), st(0) // 00c4267d
        fstp dword ptr [esp + 010h] // 00c4267f
        fsub dword ptr [esp + 010h] // 00c42683
        fstp dword ptr [esp + 010h] // 00c42687
        fld dword ptr [esp + 010h] // 00c4268b
        fld dword ptr [esi + eax + 4] // 00c4268f
        fmul dword ptr [esp + 03ch] // 00c42693
        fld dword ptr [esp + 05ch] // 00c42697
        fmul dword ptr [esi + eax] // 00c4269b
        faddp st(1), st(0) // 00c4269e
        fld dword ptr [esi + eax + 8] // 00c426a0
        fmul dword ptr [esp + 044h] // 00c426a4
        faddp st(1), st(0) // 00c426a8
        fstp dword ptr [esp + 010h] // 00c426aa
        fsub dword ptr [esp + 010h] // 00c426ae
        fstp dword ptr [esp + 010h] // 00c426b2
        fld dword ptr [esp + 010h] // 00c426b6
        fld dword ptr [esi + eax + 010h] // 00c426ba
        fmul dword ptr [esp + 064h] // 00c426be
        fld dword ptr [esi + eax + 0ch] // 00c426c2
        fmul dword ptr [esp + 04ch] // 00c426c6
        faddp st(1), st(0) // 00c426ca
        fld dword ptr [esi + eax + 014h] // 00c426cc
        mov eax, dword ptr [ecx + 0ea98h] // 00c426d0
        fmul dword ptr [esp + 06ch] // 00c426d6
        faddp st(1), st(0) // 00c426da
        fstp dword ptr [esp + 010h] // 00c426dc
        fsub dword ptr [esp + 010h] // 00c426e0
        fstp dword ptr [esp + 010h] // 00c426e4
        fld dword ptr [esp + 010h] // 00c426e8
        fmul dword ptr [eax + ebx*4] // 00c426ec
        mov eax, dword ptr [ecx + 0ea8ch] // 00c426ef
        fstp dword ptr [esp + 010h] // 00c426f5
        fld dword ptr [esp + 010h] // 00c426f9
        fadd dword ptr [eax + ebx*4] // 00c426fd
        fstp dword ptr [esp + 018h] // 00c42700
        fld dword ptr [esp + 018h] // 00c42704
        fldz  // 00c42708
        fcomip st(0), st(1) // 00c4270a
        fstp st(0) // 00c4270c
        jbe l_00c42716 // 00c4270e
        movss dword ptr [esp + 018h], xmm0 // 00c42710
    l_00c42716:
        fld dword ptr [esp + 018h] // 00c42716
        movss xmm1, dword ptr [esp + 018h] // 00c4271a
        fsub dword ptr [eax + ebx*4] // 00c42720
        movss dword ptr [eax + ebx*4], xmm1 // 00c42723
        lea eax, [eax + ebx*4] // 00c42728
        mov eax, dword ptr [esp + 024h] // 00c4272b
        fstp dword ptr [esp + 014h] // 00c4272f
        fld dword ptr [eax] // 00c42733
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42735
        fstp dword ptr [esp + 010h] // 00c4273b
        add eax, edx // 00c4273f
        fld dword ptr [esp + 010h] // 00c42741
        fld st(0) // 00c42745
        fld dword ptr [esp + 014h] // 00c42747
        fld st(0) // 00c4274b
        fmulp st(2), st(0) // 00c4274d
        fxch st(1) // 00c4274f
        fstp dword ptr [esp + 0a4h] // 00c42751
        fld dword ptr [edi - 010h] // 00c42758
        fstp dword ptr [esp + 010h] // 00c4275b
        fld dword ptr [esp + 010h] // 00c4275f
        fld st(0) // 00c42763
        fmul st(0), st(2) // 00c42765
        fstp dword ptr [esp + 0a8h] // 00c42767
        fld dword ptr [edi - 0ch] // 00c4276e
        fstp dword ptr [esp + 010h] // 00c42771
        fld dword ptr [esp + 010h] // 00c42775
        fld st(0) // 00c42779
        fmul st(0), st(3) // 00c4277b
        fstp dword ptr [esp + 0ach] // 00c4277d
        fld dword ptr [esp + 0a4h] // 00c42784
        fadd dword ptr [eax] // 00c4278b
        fstp dword ptr [eax] // 00c4278d
        fld dword ptr [eax + 4] // 00c4278f
        fadd dword ptr [esp + 0a8h] // 00c42792
        fstp dword ptr [eax + 4] // 00c42799
        fld dword ptr [esp + 0ach] // 00c4279c
        fadd dword ptr [eax + 8] // 00c427a3
        fstp dword ptr [eax + 8] // 00c427a6
        mov eax, dword ptr [ecx + 0ea6ch] // 00c427a9
        fld dword ptr [edi - 8] // 00c427af
        lea eax, [eax + edx + 0ch] // 00c427b2
        fstp dword ptr [esp + 010h] // 00c427b6
        fld dword ptr [esp + 010h] // 00c427ba
        fld st(0) // 00c427be
        fmul st(0), st(4) // 00c427c0
        fstp dword ptr [esp + 08ch] // 00c427c2
        fld dword ptr [edi - 4] // 00c427c9
        fstp dword ptr [esp + 028h] // 00c427cc
        fld dword ptr [esp + 028h] // 00c427d0
        fmul st(0), st(4) // 00c427d4
        fstp dword ptr [esp + 090h] // 00c427d6
        fld dword ptr [edi] // 00c427dd
        fstp dword ptr [esp + 058h] // 00c427df
        fld dword ptr [esp + 058h] // 00c427e3
        fmul st(0), st(4) // 00c427e7
        fstp dword ptr [esp + 094h] // 00c427e9
        fld dword ptr [eax] // 00c427f0
        fadd dword ptr [esp + 08ch] // 00c427f2
        fstp dword ptr [eax] // 00c427f9
        fld dword ptr [esp + 090h] // 00c427fb
        fadd dword ptr [eax + 4] // 00c42802
        fstp dword ptr [eax + 4] // 00c42805
        fld dword ptr [esp + 094h] // 00c42808
        fadd dword ptr [eax + 8] // 00c4280f
        fstp dword ptr [eax + 8] // 00c42812
        fld dword ptr [edi + 4] // 00c42815
        fstp dword ptr [esp + 068h] // 00c42818
        fld dword ptr [esp + 068h] // 00c4281c
        fmul st(0), st(4) // 00c42820
        fstp dword ptr [esp + 0bch] // 00c42822
        fld dword ptr [edi + 8] // 00c42829
        fstp dword ptr [esp + 02ch] // 00c4282c
        fld dword ptr [esp + 02ch] // 00c42830
        fmul st(0), st(4) // 00c42834
        fstp dword ptr [esp + 0c0h] // 00c42836
        fld dword ptr [edi + 0ch] // 00c4283d
        fstp dword ptr [esp + 030h] // 00c42840
        fld dword ptr [esp + 030h] // 00c42844
        fmul st(0), st(4) // 00c42848
        fstp dword ptr [esp + 0c4h] // 00c4284a
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42851
        fld dword ptr [eax + esi] // 00c42857
        add eax, esi // 00c4285a
        fadd dword ptr [esp + 0bch] // 00c4285c
        fstp dword ptr [eax] // 00c42863
        fld dword ptr [esp + 0c0h] // 00c42865
        fadd dword ptr [eax + 4] // 00c4286c
        fstp dword ptr [eax + 4] // 00c4286f
        fld dword ptr [eax + 8] // 00c42872
        fadd dword ptr [esp + 0c4h] // 00c42875
        fstp dword ptr [eax + 8] // 00c4287c
        mov eax, dword ptr [ecx + 0ea6ch] // 00c4287f
        fld dword ptr [edi + 010h] // 00c42885
        lea eax, [eax + esi + 0ch] // 00c42888
        fstp dword ptr [esp + 038h] // 00c4288c
        fld dword ptr [esp + 038h] // 00c42890
        fmul st(0), st(4) // 00c42894
        fstp dword ptr [esp + 074h] // 00c42896
        fld dword ptr [edi + 014h] // 00c4289a
        fstp dword ptr [esp + 040h] // 00c4289d
        fld dword ptr [esp + 040h] // 00c428a1
        fmul st(0), st(4) // 00c428a5
        fstp dword ptr [esp + 078h] // 00c428a7
        fld dword ptr [edi + 018h] // 00c428ab
        fstp dword ptr [esp + 010h] // 00c428ae
        fld dword ptr [esp + 010h] // 00c428b2
        fmulp st(4), st(0) // 00c428b6
        fxch st(3) // 00c428b8
        fstp dword ptr [esp + 07ch] // 00c428ba
        fld dword ptr [esp + 074h] // 00c428be
        fadd dword ptr [eax] // 00c428c2
        fstp dword ptr [eax] // 00c428c4
        fld dword ptr [eax + 4] // 00c428c6
        fadd dword ptr [esp + 078h] // 00c428c9
        fstp dword ptr [eax + 4] // 00c428cd
        fld dword ptr [eax + 8] // 00c428d0
        fadd dword ptr [esp + 07ch] // 00c428d3
        fstp dword ptr [eax + 8] // 00c428d7
        mov eax, dword ptr [ecx + 0ea6ch] // 00c428da
        mov ebp, dword ptr [ecx + 0ea6ch] // 00c428e0
        add eax, edx // 00c428e6
        add ebp, esi // 00c428e8
        mov dword ptr [esp + 018h], ebp // 00c428ea
        mov ebp, dword ptr [ecx + 0ea90h] // 00c428ee
        fld dword ptr [ebp + ebx*4] // 00c428f4
        fld dword ptr [eax + 01ch] // 00c428f8
        fmulp st(6), st(0) // 00c428fb
        fld dword ptr [eax + 018h] // 00c428fd
        fmul dword ptr [esp + 060h] // 00c42900
        faddp st(6), st(0) // 00c42904
        fld dword ptr [eax + 020h] // 00c42906
        fmul dword ptr [esp + 048h] // 00c42909
        faddp st(6), st(0) // 00c4290d
        fxch st(5) // 00c4290f
        fstp dword ptr [esp + 014h] // 00c42911
        fld dword ptr [esp + 014h] // 00c42915
        fsubp st(5), st(0) // 00c42919
        fxch st(4) // 00c4291b
        fstp dword ptr [esp + 014h] // 00c4291d
        fld dword ptr [esp + 014h] // 00c42921
        fld dword ptr [eax + 028h] // 00c42925
        fmul dword ptr [esp + 050h] // 00c42928
        fld dword ptr [eax + 024h] // 00c4292c
        fmul dword ptr [esp + 034h] // 00c4292f
        faddp st(1), st(0) // 00c42933
        fld dword ptr [eax + 02ch] // 00c42935
        mov eax, dword ptr [esp + 018h] // 00c42938
        fmul dword ptr [esp + 054h] // 00c4293c
        faddp st(1), st(0) // 00c42940
        fstp dword ptr [esp + 014h] // 00c42942
        fsub dword ptr [esp + 014h] // 00c42946
        fstp dword ptr [esp + 014h] // 00c4294a
        fld dword ptr [esp + 014h] // 00c4294e
        fld dword ptr [eax + 01ch] // 00c42952
        fmul dword ptr [esp + 03ch] // 00c42955
        fld dword ptr [eax + 018h] // 00c42959
        fmul dword ptr [esp + 05ch] // 00c4295c
        faddp st(1), st(0) // 00c42960
        fld dword ptr [eax + 020h] // 00c42962
        fmul dword ptr [esp + 044h] // 00c42965
        faddp st(1), st(0) // 00c42969
        fstp dword ptr [esp + 014h] // 00c4296b
        fsub dword ptr [esp + 014h] // 00c4296f
        fstp dword ptr [esp + 014h] // 00c42973
        fld dword ptr [esp + 014h] // 00c42977
        fld dword ptr [eax + 028h] // 00c4297b
        fmul dword ptr [esp + 064h] // 00c4297e
        fld dword ptr [esp + 04ch] // 00c42982
        fmul dword ptr [eax + 024h] // 00c42986
        faddp st(1), st(0) // 00c42989
        fld dword ptr [eax + 02ch] // 00c4298b
        mov eax, dword ptr [ecx + 0ea98h] // 00c4298e
        fmul dword ptr [esp + 06ch] // 00c42994
        faddp st(1), st(0) // 00c42998
        fstp dword ptr [esp + 014h] // 00c4299a
        fsub dword ptr [esp + 014h] // 00c4299e
        fstp dword ptr [esp + 014h] // 00c429a2
        fld dword ptr [esp + 014h] // 00c429a6
        fmul dword ptr [eax + ebx*4] // 00c429aa
        mov eax, dword ptr [ecx + 0ea94h] // 00c429ad
        fstp dword ptr [esp + 014h] // 00c429b3
        fld dword ptr [esp + 014h] // 00c429b7
        fadd dword ptr [eax + ebx*4] // 00c429bb
        fstp dword ptr [esp + 018h] // 00c429be
        fld dword ptr [esp + 018h] // 00c429c2
        fldz  // 00c429c6
        fcomip st(0), st(1) // 00c429c8
        fstp st(0) // 00c429ca
        jbe l_00c429d4 // 00c429cc
        movss dword ptr [esp + 018h], xmm0 // 00c429ce
    l_00c429d4:
        fld dword ptr [esp + 018h] // 00c429d4
        movss xmm1, dword ptr [esp + 018h] // 00c429d8
        fsub dword ptr [eax + ebx*4] // 00c429de
        lea eax, [eax + ebx*4] // 00c429e1
        movss dword ptr [eax], xmm1 // 00c429e4
        mov eax, dword ptr [ecx + 0ea6ch] // 00c429e8
        fstp dword ptr [esp + 014h] // 00c429ee
        lea eax, [eax + edx + 018h] // 00c429f2
        fld dword ptr [esp + 014h] // 00c429f6
        fld st(0) // 00c429fa
        fmulp st(4), st(0) // 00c429fc
        fxch st(3) // 00c429fe
        fstp dword ptr [esp + 080h] // 00c42a00
        fmul st(0), st(2) // 00c42a07
        fstp dword ptr [esp + 084h] // 00c42a09
        fld st(1) // 00c42a10
        fmulp st(3), st(0) // 00c42a12
        fxch st(2) // 00c42a14
        fstp dword ptr [esp + 088h] // 00c42a16
        fld dword ptr [eax] // 00c42a1d
        fadd dword ptr [esp + 080h] // 00c42a1f
        fstp dword ptr [eax] // 00c42a26
        fld dword ptr [esp + 084h] // 00c42a28
        fadd dword ptr [eax + 4] // 00c42a2f
        fstp dword ptr [eax + 4] // 00c42a32
        fld dword ptr [eax + 8] // 00c42a35
        fadd dword ptr [esp + 088h] // 00c42a38
        fstp dword ptr [eax + 8] // 00c42a3f
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42a42
        lea edx, [eax + edx + 024h] // 00c42a48
        fld st(0) // 00c42a4c
        fmulp st(2), st(0) // 00c42a4e
        fxch st(1) // 00c42a50
        fstp dword ptr [esp + 098h] // 00c42a52
        fld dword ptr [esp + 028h] // 00c42a59
        fmul st(0), st(1) // 00c42a5d
        fstp dword ptr [esp + 09ch] // 00c42a5f
        fld dword ptr [esp + 058h] // 00c42a66
        fmul st(0), st(1) // 00c42a6a
        fstp dword ptr [esp + 0a0h] // 00c42a6c
        fld dword ptr [esp + 098h] // 00c42a73
        fadd dword ptr [edx] // 00c42a7a
        fstp dword ptr [edx] // 00c42a7c
        fld dword ptr [edx + 4] // 00c42a7e
        fadd dword ptr [esp + 09ch] // 00c42a81
        fstp dword ptr [edx + 4] // 00c42a88
        fld dword ptr [esp + 0a0h] // 00c42a8b
        fadd dword ptr [edx + 8] // 00c42a92
        fstp dword ptr [edx + 8] // 00c42a95
        mov edx, dword ptr [ecx + 0ea6ch] // 00c42a98
        fld dword ptr [esp + 068h] // 00c42a9e
        lea eax, [edx + esi + 018h] // 00c42aa2
        fmul st(0), st(1) // 00c42aa6
        fstp dword ptr [esp + 0b0h] // 00c42aa8
        fld dword ptr [esp + 02ch] // 00c42aaf
        fmul st(0), st(1) // 00c42ab3
        fstp dword ptr [esp + 0b4h] // 00c42ab5
        fld dword ptr [esp + 030h] // 00c42abc
        fmul st(0), st(1) // 00c42ac0
        fstp dword ptr [esp + 0b8h] // 00c42ac2
        fld dword ptr [eax] // 00c42ac9
        fadd dword ptr [esp + 0b0h] // 00c42acb
        fstp dword ptr [eax] // 00c42ad2
        fld dword ptr [esp + 0b4h] // 00c42ad4
        fadd dword ptr [eax + 4] // 00c42adb
        fstp dword ptr [eax + 4] // 00c42ade
        fld dword ptr [esp + 0b8h] // 00c42ae1
        fadd dword ptr [eax + 8] // 00c42ae8
        fstp dword ptr [eax + 8] // 00c42aeb
        mov eax, dword ptr [ecx + 0ea6ch] // 00c42aee
        fld dword ptr [esp + 038h] // 00c42af4
        lea esi, [eax + esi + 024h] // 00c42af8
        fmul st(0), st(1) // 00c42afc
        fstp dword ptr [esp + 0c8h] // 00c42afe
        fld dword ptr [esp + 040h] // 00c42b05
        fmul st(0), st(1) // 00c42b09
        fstp dword ptr [esp + 0cch] // 00c42b0b
        fmul dword ptr [esp + 010h] // 00c42b12
        fstp dword ptr [esp + 0d0h] // 00c42b16
        fld dword ptr [esi] // 00c42b1d
        fadd dword ptr [esp + 0c8h] // 00c42b1f
        fstp dword ptr [esi] // 00c42b26
        mov eax, 030h // 00c42b28
        fld dword ptr [esi + 4] // 00c42b2d
        add dword ptr [esp + 020h], eax // 00c42b30
        fadd dword ptr [esp + 0cch] // 00c42b34
        add dword ptr [esp + 01ch], eax // 00c42b3b
        add dword ptr [esp + 024h], eax // 00c42b3f
        add ebx, 1 // 00c42b43
        fstp dword ptr [esi + 4] // 00c42b46
        add edi, eax // 00c42b49
        fld dword ptr [esi + 8] // 00c42b4b
        fadd dword ptr [esp + 0d0h] // 00c42b4e
        fstp dword ptr [esi + 8] // 00c42b55
        cmp ebx, dword ptr [ecx + 0eaa4h] // 00c42b58
        jl l_00c42570 // 00c42b5e
        pop esi // 00c42b64
        pop ebp // 00c42b65
    l_00c42b66:
        pop edi // 00c42b66
        pop ebx // 00c42b67
        add esp, 0c4h // 00c42b68
        ret  // 00c42b6e
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void warm_kernel(){
    __asm {
        sub esp, 068h // 00c42ba0
        push ebp // 00c42ba3
        mov ebp, dword ptr [eax + 0ea8ch] // 00c42ba4
        push esi // 00c42baa
        xor esi, esi // 00c42bab
        cmp dword ptr [eax + 0eaa4h], esi // 00c42bad
        mov dword ptr [esp + 8], ebp // 00c42bb3
        jle l_00c42ebe // 00c42bb7
        push ebx // 00c42bbd
        xor ecx, ecx // 00c42bbe
        push edi // 00c42bc0
    l_00c42bc1:
        mov edx, dword ptr [eax + 0ea84h] // 00c42bc1
        fld dword ptr [ebp] // 00c42bc7
        fstp dword ptr [esp + 014h] // 00c42bca
        movsx edi, word ptr [edx + esi*4] // 00c42bce
        movsx ebx, word ptr [edx + esi*4 + 2] // 00c42bd2
        lea edx, [edx + esi*4] // 00c42bd7
        mov edx, dword ptr [eax + 0ea80h] // 00c42bda
        add edx, ecx // 00c42be0
        lea edi, [edi + edi*2] // 00c42be2
        shl edi, 4 // 00c42be5
        fld dword ptr [edx] // 00c42be8
        lea ebx, [ebx + ebx*2] // 00c42bea
        fld dword ptr [esp + 014h] // 00c42bed
        shl ebx, 4 // 00c42bf1
        fld st(0) // 00c42bf4
        fmulp st(2), st(0) // 00c42bf6
        fxch st(1) // 00c42bf8
        fstp dword ptr [esp + 018h] // 00c42bfa
        fld dword ptr [edx + 4] // 00c42bfe
        fmul st(0), st(1) // 00c42c01
        fstp dword ptr [esp + 01ch] // 00c42c03
        fmul dword ptr [edx + 8] // 00c42c07
        mov edx, dword ptr [eax + 0ea6ch] // 00c42c0a
        add edx, edi // 00c42c10
        fstp dword ptr [esp + 020h] // 00c42c12
        fld dword ptr [edx] // 00c42c16
        fadd dword ptr [esp + 018h] // 00c42c18
        fstp dword ptr [edx] // 00c42c1c
        fld dword ptr [edx + 4] // 00c42c1e
        fadd dword ptr [esp + 01ch] // 00c42c21
        fstp dword ptr [edx + 4] // 00c42c25
        fld dword ptr [edx + 8] // 00c42c28
        fadd dword ptr [esp + 020h] // 00c42c2b
        fstp dword ptr [edx + 8] // 00c42c2f
        mov edx, dword ptr [eax + 0ea80h] // 00c42c32
        fld dword ptr [ebp] // 00c42c38
        lea edx, [edx + ecx + 0ch] // 00c42c3b
        fstp dword ptr [esp + 014h] // 00c42c3f
        fld dword ptr [esp + 014h] // 00c42c43
        fld st(0) // 00c42c47
        fmul dword ptr [edx] // 00c42c49
        fstp dword ptr [esp + 024h] // 00c42c4b
        fld dword ptr [edx + 4] // 00c42c4f
        fmul st(0), st(1) // 00c42c52
        fstp dword ptr [esp + 028h] // 00c42c54
        fmul dword ptr [edx + 8] // 00c42c58
        mov edx, dword ptr [eax + 0ea6ch] // 00c42c5b
        lea edx, [edi + edx + 0ch] // 00c42c61
        fstp dword ptr [esp + 02ch] // 00c42c65
        fld dword ptr [edx] // 00c42c69
        fadd dword ptr [esp + 024h] // 00c42c6b
        fstp dword ptr [edx] // 00c42c6f
        fld dword ptr [edx + 4] // 00c42c71
        fadd dword ptr [esp + 028h] // 00c42c74
        fstp dword ptr [edx + 4] // 00c42c78
        fld dword ptr [edx + 8] // 00c42c7b
        fadd dword ptr [esp + 02ch] // 00c42c7e
        fstp dword ptr [edx + 8] // 00c42c82
        mov edx, dword ptr [eax + 0ea80h] // 00c42c85
        fld dword ptr [ebp] // 00c42c8b
        lea edx, [edx + ecx + 018h] // 00c42c8e
        fstp dword ptr [esp + 014h] // 00c42c92
        fld dword ptr [edx] // 00c42c96
        fld dword ptr [esp + 014h] // 00c42c98
        fld st(0) // 00c42c9c
        fmulp st(2), st(0) // 00c42c9e
        fxch st(1) // 00c42ca0
        fstp dword ptr [esp + 030h] // 00c42ca2
        fld dword ptr [edx + 4] // 00c42ca6
        fmul st(0), st(1) // 00c42ca9
        fstp dword ptr [esp + 034h] // 00c42cab
        fmul dword ptr [edx + 8] // 00c42caf
        mov edx, dword ptr [eax + 0ea6ch] // 00c42cb2
        add edx, ebx // 00c42cb8
        fstp dword ptr [esp + 038h] // 00c42cba
        fld dword ptr [esp + 030h] // 00c42cbe
        fadd dword ptr [edx] // 00c42cc2
        fstp dword ptr [edx] // 00c42cc4
        fld dword ptr [esp + 034h] // 00c42cc6
        fadd dword ptr [edx + 4] // 00c42cca
        fstp dword ptr [edx + 4] // 00c42ccd
        fld dword ptr [edx + 8] // 00c42cd0
        fadd dword ptr [esp + 038h] // 00c42cd3
        fstp dword ptr [edx + 8] // 00c42cd7
        mov edx, dword ptr [eax + 0ea80h] // 00c42cda
        fld dword ptr [ebp] // 00c42ce0
        lea edx, [edx + ecx + 024h] // 00c42ce3
        fstp dword ptr [esp + 014h] // 00c42ce7
        fld dword ptr [edx] // 00c42ceb
        fld dword ptr [esp + 014h] // 00c42ced
        fld st(0) // 00c42cf1
        fmulp st(2), st(0) // 00c42cf3
        fxch st(1) // 00c42cf5
        fstp dword ptr [esp + 03ch] // 00c42cf7
        fld dword ptr [edx + 4] // 00c42cfb
        fmul st(0), st(1) // 00c42cfe
        fstp dword ptr [esp + 040h] // 00c42d00
        fmul dword ptr [edx + 8] // 00c42d04
        mov edx, dword ptr [eax + 0ea6ch] // 00c42d07
        lea edx, [ebx + edx + 0ch] // 00c42d0d
        fstp dword ptr [esp + 044h] // 00c42d11
        fld dword ptr [edx] // 00c42d15
        fadd dword ptr [esp + 03ch] // 00c42d17
        fstp dword ptr [edx] // 00c42d1b
        fld dword ptr [esp + 040h] // 00c42d1d
        fadd dword ptr [edx + 4] // 00c42d21
        fstp dword ptr [edx + 4] // 00c42d24
        fld dword ptr [edx + 8] // 00c42d27
        fadd dword ptr [esp + 044h] // 00c42d2a
        fstp dword ptr [edx + 8] // 00c42d2e
        mov ebp, dword ptr [eax + 0ea94h] // 00c42d31
        mov edx, dword ptr [eax + 0ea80h] // 00c42d37
        fld dword ptr [ebp + esi*4] // 00c42d3d
        fstp dword ptr [esp + 014h] // 00c42d41
        add edx, ecx // 00c42d45
        fld dword ptr [edx] // 00c42d47
        fld dword ptr [esp + 014h] // 00c42d49
        fld st(0) // 00c42d4d
        fmulp st(2), st(0) // 00c42d4f
        fxch st(1) // 00c42d51
        fstp dword ptr [esp + 048h] // 00c42d53
        fld dword ptr [edx + 4] // 00c42d57
        fmul st(0), st(1) // 00c42d5a
        fstp dword ptr [esp + 04ch] // 00c42d5c
        fmul dword ptr [edx + 8] // 00c42d60
        mov edx, dword ptr [eax + 0ea6ch] // 00c42d63
        lea edx, [edi + edx + 018h] // 00c42d69
        fstp dword ptr [esp + 050h] // 00c42d6d
        fld dword ptr [edx] // 00c42d71
        fadd dword ptr [esp + 048h] // 00c42d73
        fstp dword ptr [edx] // 00c42d77
        fld dword ptr [edx + 4] // 00c42d79
        fadd dword ptr [esp + 04ch] // 00c42d7c
        fstp dword ptr [edx + 4] // 00c42d80
        fld dword ptr [esp + 050h] // 00c42d83
        fadd dword ptr [edx + 8] // 00c42d87
        fstp dword ptr [edx + 8] // 00c42d8a
        mov ebp, dword ptr [eax + 0ea94h] // 00c42d8d
        mov edx, dword ptr [eax + 0ea80h] // 00c42d93
        fld dword ptr [ebp + esi*4] // 00c42d99
        fstp dword ptr [esp + 014h] // 00c42d9d
        lea edx, [edx + ecx + 0ch] // 00c42da1
        fld dword ptr [edx] // 00c42da5
        fld dword ptr [esp + 014h] // 00c42da7
        fld st(0) // 00c42dab
        fmulp st(2), st(0) // 00c42dad
        fxch st(1) // 00c42daf
        fstp dword ptr [esp + 054h] // 00c42db1
        fld dword ptr [edx + 4] // 00c42db5
        fmul st(0), st(1) // 00c42db8
        fstp dword ptr [esp + 058h] // 00c42dba
        fmul dword ptr [edx + 8] // 00c42dbe
        mov edx, dword ptr [eax + 0ea6ch] // 00c42dc1
        lea edx, [edi + edx + 024h] // 00c42dc7
        fstp dword ptr [esp + 05ch] // 00c42dcb
        fld dword ptr [edx] // 00c42dcf
        fadd dword ptr [esp + 054h] // 00c42dd1
        fstp dword ptr [edx] // 00c42dd5
        fld dword ptr [esp + 058h] // 00c42dd7
        fadd dword ptr [edx + 4] // 00c42ddb
        fstp dword ptr [edx + 4] // 00c42dde
        fld dword ptr [edx + 8] // 00c42de1
        fadd dword ptr [esp + 05ch] // 00c42de4
        mov ebp, dword ptr [esp + 010h] // 00c42de8
        add ebp, 4 // 00c42dec
        add esi, 1 // 00c42def
        fstp dword ptr [edx + 8] // 00c42df2
        mov edi, dword ptr [eax + 0ea94h] // 00c42df5
        fld dword ptr [edi + esi*4 - 4] // 00c42dfb
        mov edx, dword ptr [eax + 0ea80h] // 00c42dff
        fstp dword ptr [esp + 014h] // 00c42e05
        lea edx, [edx + ecx + 018h] // 00c42e09
        fld dword ptr [esp + 014h] // 00c42e0d
        mov dword ptr [esp + 010h], ebp // 00c42e11
        fld st(0) // 00c42e15
        fmul dword ptr [edx] // 00c42e17
        fstp dword ptr [esp + 060h] // 00c42e19
        fld dword ptr [edx + 4] // 00c42e1d
        fmul st(0), st(1) // 00c42e20
        fstp dword ptr [esp + 064h] // 00c42e22
        fmul dword ptr [edx + 8] // 00c42e26
        mov edx, dword ptr [eax + 0ea6ch] // 00c42e29
        lea edx, [ebx + edx + 018h] // 00c42e2f
        fstp dword ptr [esp + 068h] // 00c42e33
        fld dword ptr [edx] // 00c42e37
        fadd dword ptr [esp + 060h] // 00c42e39
        fstp dword ptr [edx] // 00c42e3d
        fld dword ptr [edx + 4] // 00c42e3f
        fadd dword ptr [esp + 064h] // 00c42e42
        fstp dword ptr [edx + 4] // 00c42e46
        fld dword ptr [esp + 068h] // 00c42e49
        fadd dword ptr [edx + 8] // 00c42e4d
        fstp dword ptr [edx + 8] // 00c42e50
        mov edi, dword ptr [eax + 0ea94h] // 00c42e53
        fld dword ptr [edi + esi*4 - 4] // 00c42e59
        mov edx, dword ptr [eax + 0ea80h] // 00c42e5d
        fstp dword ptr [esp + 014h] // 00c42e63
        lea edx, [edx + ecx + 024h] // 00c42e67
        fld dword ptr [esp + 014h] // 00c42e6b
        add ecx, 030h // 00c42e6f
        fld st(0) // 00c42e72
        fmul dword ptr [edx] // 00c42e74
        fstp dword ptr [esp + 06ch] // 00c42e76
        fld dword ptr [edx + 4] // 00c42e7a
        fmul st(0), st(1) // 00c42e7d
        fstp dword ptr [esp + 070h] // 00c42e7f
        fmul dword ptr [edx + 8] // 00c42e83
        mov edx, dword ptr [eax + 0ea6ch] // 00c42e86
        lea edx, [ebx + edx + 024h] // 00c42e8c
        fstp dword ptr [esp + 074h] // 00c42e90
        fld dword ptr [edx] // 00c42e94
        fadd dword ptr [esp + 06ch] // 00c42e96
        fstp dword ptr [edx] // 00c42e9a
        fld dword ptr [edx + 4] // 00c42e9c
        fadd dword ptr [esp + 070h] // 00c42e9f
        fstp dword ptr [edx + 4] // 00c42ea3
        fld dword ptr [esp + 074h] // 00c42ea6
        fadd dword ptr [edx + 8] // 00c42eaa
        fstp dword ptr [edx + 8] // 00c42ead
        cmp esi, dword ptr [eax + 0eaa4h] // 00c42eb0
        jl l_00c42bc1 // 00c42eb6
        pop edi // 00c42ebc
        pop ebx // 00c42ebd
    l_00c42ebe:
        pop esi // 00c42ebe
        pop ebp // 00c42ebf
        add esp, 068h // 00c42ec0
        ret  // 00c42ec3
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void build_rows_kernel(){
    __asm {
        sub esp, 017ch // 00c4de40
        mov eax, dword ptr [esi + 0ea68h] // 00c4de46
        mov dword ptr [esi + 4], 1 // 00c4de4c
        mov eax, dword ptr [eax + 4] // 00c4de53
        push edi // 00c4de56
        xor edx, edx // 00c4de57
        xor ecx, ecx // 00c4de59
        test eax, eax // 00c4de5b
        lea edi, [eax*4] // 00c4de5d
        mov dword ptr [esp + 024h], edx // 00c4de64
        mov dword ptr [esp + 088h], edi // 00c4de68
        mov dword ptr [esp + 084h], ecx // 00c4de6f
        jle l_00c4f027 // 00c4de76
        xorps xmm1, xmm1 // 00c4de7c
        movss xmm2, dword ptr constant_00d7a208 // 00c4de7f
        push ebx // 00c4de87
        push ebp // 00c4de88
        _emit 08dh // 00c4de89
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c4de90:
        mov edx, dword ptr [esi + 0ea68h] // 00c4de90
        mov eax, dword ptr [edx] // 00c4de96
        mov edx, dword ptr [eax + ecx*4] // 00c4de98
        cmp dword ptr [edx + 0c8h], 0 // 00c4de9b
        mov dword ptr [esp + 050h], edx // 00c4dea2
        je l_00c4efe4 // 00c4dea6
        mov eax, dword ptr [edx + 0cch] // 00c4deac
        test byte ptr [eax + 050h], 1 // 00c4deb2
        mov ebx, dword ptr [esi] // 00c4deb6
        mov ecx, dword ptr [edx + 0d0h] // 00c4deb8
        mov ebx, dword ptr [ebx + 02ch] // 00c4debe
        mov edi, dword ptr [eax + 4] // 00c4dec1
        mov ebp, dword ptr [ecx + 4] // 00c4dec4
        mov dword ptr [esp + 0a4h], eax // 00c4dec7
        mov dword ptr [esp + 088h], ecx // 00c4dece
        mov dword ptr [esp + 048h], ebx // 00c4ded5
        je l_00c4dee7 // 00c4ded9
        mov dword ptr [eax + 05ch], 0 // 00c4dedb
        mov dword ptr [esi + 8], eax // 00c4dee2
        jmp l_00c4df04 // 00c4dee5
    l_00c4dee7:
        cmp dword ptr [eax + 058h], ebx // 00c4dee7
        je l_00c4df04 // 00c4deea
        mov dword ptr [eax + 058h], ebx // 00c4deec
        mov ebx, dword ptr [esi + 4] // 00c4deef
        mov dword ptr [eax + 05ch], ebx // 00c4def2
        mov ebx, dword ptr [esi + 4] // 00c4def5
        mov dword ptr [esi + ebx*4 + 8], eax // 00c4def8
        add dword ptr [esi + 4], 1 // 00c4defc
        mov ebx, dword ptr [esp + 048h] // 00c4df00
    l_00c4df04:
        test byte ptr [ecx + 050h], 1 // 00c4df04
        je l_00c4df16 // 00c4df08
        mov dword ptr [ecx + 05ch], 0 // 00c4df0a
        mov dword ptr [esi + 8], ecx // 00c4df11
        jmp l_00c4df2f // 00c4df14
    l_00c4df16:
        cmp dword ptr [ecx + 058h], ebx // 00c4df16
        je l_00c4df2f // 00c4df19
        mov dword ptr [ecx + 058h], ebx // 00c4df1b
        mov ebx, dword ptr [esi + 4] // 00c4df1e
        mov dword ptr [ecx + 05ch], ebx // 00c4df21
        mov ebx, dword ptr [esi + 4] // 00c4df24
        mov dword ptr [esi + ebx*4 + 8], ecx // 00c4df27
        add dword ptr [esi + 4], 1 // 00c4df2b
    l_00c4df2f:
        cmp dword ptr [edx + 0c8h], 0 // 00c4df2f
        mov ebx, dword ptr [eax + 05ch] // 00c4df36
        mov dword ptr [esp + 0a0h], ebx // 00c4df39
        mov ebx, dword ptr [ecx + 05ch] // 00c4df40
        mov dword ptr [esp + 080h], ebx // 00c4df43
        lea ebx, [edx + 8] // 00c4df4a
        mov dword ptr [esp + 020h], ebx // 00c4df4d
        mov dword ptr [esp + 048h], 0 // 00c4df51
        jle l_00c4efe4 // 00c4df59
        mov ebx, dword ptr [esp + 02ch] // 00c4df5f
        lea edx, [ebx + ebx*2] // 00c4df63
        shl edx, 4 // 00c4df66
        mov dword ptr [esp + 058h], edx // 00c4df69
        mov edx, dword ptr [esp + 090h] // 00c4df6d
        add ebx, edx // 00c4df74
        lea edx, [ebx*4] // 00c4df76
        mov dword ptr [esp + 04ch], edx // 00c4df7d
        mov edx, dword ptr [esp + 020h] // 00c4df81
        lea ebx, [ebx + ebx*2] // 00c4df85
        shl ebx, 4 // 00c4df88
        add edx, 014h // 00c4df8b
        mov dword ptr [esp + 028h], edx // 00c4df8e
        jmp l_00c4dfa6 // 00c4df92
    l_00c4df94:
        mov edx, dword ptr [esp + 028h] // 00c4df94
        mov ecx, dword ptr [esp + 088h] // 00c4df98
        mov eax, dword ptr [esp + 0a4h] // 00c4df9f
    l_00c4dfa6:
        fld dword ptr [edx - 4] // 00c4dfa6
        fstp dword ptr [esp + 0ch] // 00c4dfa9
        fld dword ptr [edx - 8] // 00c4dfad
        fstp dword ptr [esp + 01ch] // 00c4dfb0
        fld dword ptr [edx] // 00c4dfb4
        fstp dword ptr [esp + 018h] // 00c4dfb6
        fld dword ptr [eax + 014h] // 00c4dfba
        fld dword ptr [esp + 0ch] // 00c4dfbd
        fld st(0) // 00c4dfc1
        fmulp st(2), st(0) // 00c4dfc3
        fld dword ptr [eax + 8] // 00c4dfc5
        fld dword ptr [esp + 01ch] // 00c4dfc8
        fld st(0) // 00c4dfcc
        fmulp st(2), st(0) // 00c4dfce
        fxch st(3) // 00c4dfd0
        faddp st(1), st(0) // 00c4dfd2
        fld dword ptr [eax + 020h] // 00c4dfd4
        fld dword ptr [esp + 018h] // 00c4dfd7
        fld st(0) // 00c4dfdb
        fmulp st(2), st(0) // 00c4dfdd
        fxch st(2) // 00c4dfdf
        faddp st(1), st(0) // 00c4dfe1
        fadd dword ptr [eax + 02ch] // 00c4dfe3
        fstp dword ptr [esp + 0170h] // 00c4dfe6
        fld st(2) // 00c4dfed
        fmul dword ptr [eax + 0ch] // 00c4dfef
        fld dword ptr [eax + 018h] // 00c4dff2
        fmul st(0), st(3) // 00c4dff5
        faddp st(1), st(0) // 00c4dff7
        fld dword ptr [eax + 024h] // 00c4dff9
        fmul st(0), st(2) // 00c4dffc
        faddp st(1), st(0) // 00c4dffe
        fadd dword ptr [eax + 030h] // 00c4e000
        fstp dword ptr [esp + 0174h] // 00c4e003
        fld dword ptr [eax + 01ch] // 00c4e00a
        fmulp st(2), st(0) // 00c4e00d
        fld dword ptr [eax + 010h] // 00c4e00f
        fmulp st(3), st(0) // 00c4e012
        fxch st(1) // 00c4e014
        faddp st(2), st(0) // 00c4e016
        fmul dword ptr [eax + 028h] // 00c4e018
        faddp st(1), st(0) // 00c4e01b
        fadd dword ptr [eax + 034h] // 00c4e01d
        fstp dword ptr [esp + 0178h] // 00c4e020
        fld dword ptr [edx + 8] // 00c4e027
        fstp dword ptr [esp + 01ch] // 00c4e02a
        fld dword ptr [edx + 4] // 00c4e02e
        fstp dword ptr [esp + 0ch] // 00c4e031
        fld dword ptr [edx + 0ch] // 00c4e035
        fstp dword ptr [esp + 018h] // 00c4e038
        fld dword ptr [ecx + 8] // 00c4e03c
        fld dword ptr [esp + 0ch] // 00c4e03f
        fld st(0) // 00c4e043
        fmulp st(2), st(0) // 00c4e045
        fld dword ptr [ecx + 014h] // 00c4e047
        fld dword ptr [esp + 01ch] // 00c4e04a
        fld st(0) // 00c4e04e
        fmulp st(2), st(0) // 00c4e050
        fxch st(3) // 00c4e052
        faddp st(1), st(0) // 00c4e054
        fld dword ptr [esp + 018h] // 00c4e056
        fld st(0) // 00c4e05a
        fmul dword ptr [ecx + 020h] // 00c4e05c
        faddp st(2), st(0) // 00c4e05f
        fld dword ptr [ecx + 02ch] // 00c4e061
        faddp st(2), st(0) // 00c4e064
        fxch st(1) // 00c4e066
        fstp dword ptr [esp + 0d0h] // 00c4e068
        fld dword ptr [ecx + 0ch] // 00c4e06f
        fmul st(0), st(2) // 00c4e072
        fld st(3) // 00c4e074
        fmul dword ptr [ecx + 018h] // 00c4e076
        faddp st(1), st(0) // 00c4e079
        fld dword ptr [ecx + 024h] // 00c4e07b
        fmul st(0), st(2) // 00c4e07e
        faddp st(1), st(0) // 00c4e080
        fadd dword ptr [ecx + 030h] // 00c4e082
        fstp dword ptr [esp + 0d4h] // 00c4e085
        fld dword ptr [ecx + 010h] // 00c4e08c
        fmulp st(2), st(0) // 00c4e08f
        fld dword ptr [ecx + 01ch] // 00c4e091
        mov eax, dword ptr [esi] // 00c4e094
        fmulp st(3), st(0) // 00c4e096
        fxch st(1) // 00c4e098
        faddp st(2), st(0) // 00c4e09a
        fmul dword ptr [ecx + 028h] // 00c4e09c
        faddp st(1), st(0) // 00c4e09f
        fadd dword ptr [ecx + 034h] // 00c4e0a1
        mov ecx, dword ptr [esp + 02ch] // 00c4e0a4
        fstp dword ptr [esp + 0d8h] // 00c4e0a8
        fld dword ptr [eax + 020h] // 00c4e0af
        mov eax, dword ptr [esi + 0ea8ch] // 00c4e0b2
        fmul dword ptr [edx + 010h] // 00c4e0b8
        fstp dword ptr [esp + 0ch] // 00c4e0bb
        fld dword ptr [esp + 0ch] // 00c4e0bf
        fstp dword ptr [eax + ecx*4] // 00c4e0c3
        mov eax, dword ptr [esi + 0ea94h] // 00c4e0c6
        fld dword ptr [edx + 014h] // 00c4e0cc
        fstp dword ptr [eax + ecx*4] // 00c4e0cf
        movss xmm0, dword ptr [edx + 018h] // 00c4e0d2
        comiss xmm1, xmm0 // 00c4e0d7
        mov edx, dword ptr [esi] // 00c4e0da
        movss xmm3, dword ptr [edx + 028h] // 00c4e0dc
        movss dword ptr [esp + 0c0h], xmm0 // 00c4e0e1
        movss dword ptr [esp + 0e8h], xmm3 // 00c4e0ea
        jbe l_00c4e0fd // 00c4e0f3
        movss dword ptr [esp + 054h], xmm1 // 00c4e0f5
        jmp l_00c4e11f // 00c4e0fb
    l_00c4e0fd:
        fld dword ptr [esp + 0e8h] // 00c4e0fd
        fld dword ptr [esp + 0c0h] // 00c4e104
        fcomip st(0), st(1) // 00c4e10b
        fstp st(0) // 00c4e10d
        jbe l_00c4e119 // 00c4e10f
        movss dword ptr [esp + 054h], xmm3 // 00c4e111
        jmp l_00c4e11f // 00c4e117
    l_00c4e119:
        movss dword ptr [esp + 054h], xmm0 // 00c4e119
    l_00c4e11f:
        mov eax, dword ptr [esi + 0ea84h] // 00c4e11f
        fld dword ptr [esp + 0170h] // 00c4e125
        movzx edx, word ptr [esp + 0a0h] // 00c4e12c
        mov word ptr [eax + ecx*4], dx // 00c4e134
        mov eax, dword ptr [esi + 0ea84h] // 00c4e138
        movzx edx, word ptr [esp + 080h] // 00c4e13e
        mov word ptr [eax + ecx*4 + 2], dx // 00c4e146
        mov eax, dword ptr [esp + 0a4h] // 00c4e14b
        mov ecx, dword ptr [esp + 058h] // 00c4e152
        movaps xmm0, xmm2 // 00c4e156
        fsub dword ptr [eax + 02ch] // 00c4e159
        fstp dword ptr [esp + 05ch] // 00c4e15c
        fld dword ptr [esp + 0174h] // 00c4e160
        fsub dword ptr [eax + 030h] // 00c4e167
        fstp dword ptr [esp + 060h] // 00c4e16a
        fld dword ptr [esp + 0178h] // 00c4e16e
        fsub dword ptr [eax + 034h] // 00c4e175
        mov eax, dword ptr [esp + 088h] // 00c4e178
        fstp dword ptr [esp + 064h] // 00c4e17f
        fld dword ptr [esp + 0d0h] // 00c4e183
        fsub dword ptr [eax + 02ch] // 00c4e18a
        fstp dword ptr [esp + 074h] // 00c4e18d
        fld dword ptr [esp + 0d4h] // 00c4e191
        fsub dword ptr [eax + 030h] // 00c4e198
        fstp dword ptr [esp + 078h] // 00c4e19b
        fld dword ptr [esp + 0d8h] // 00c4e19f
        fsub dword ptr [eax + 034h] // 00c4e1a6
        mov eax, dword ptr [esp + 020h] // 00c4e1a9
        subss xmm0, dword ptr [eax] // 00c4e1ad
        mov eax, dword ptr [esp + 028h] // 00c4e1b1
        fstp dword ptr [esp + 07ch] // 00c4e1b5
        movss dword ptr [esp + 0128h], xmm0 // 00c4e1b9
        fld dword ptr [esp + 060h] // 00c4e1c2
        mov edx, dword ptr [esp + 0128h] // 00c4e1c6
        fld st(0) // 00c4e1cd
        movaps xmm0, xmm2 // 00c4e1cf
        subss xmm0, dword ptr [eax - 010h] // 00c4e1d2
        fchs  // 00c4e1d7
        fstp dword ptr [esp + 0ach] // 00c4e1d9
        movss dword ptr [esp + 012ch], xmm0 // 00c4e1e0
        fld dword ptr [esp + 064h] // 00c4e1e9
        movaps xmm0, xmm2 // 00c4e1ed
        subss xmm0, dword ptr [eax - 0ch] // 00c4e1f0
        fld st(0) // 00c4e1f5
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e1f7
        fchs  // 00c4e1fd
        mov dword ptr [eax + ecx], edx // 00c4e1ff
        fstp dword ptr [esp + 0b0h] // 00c4e202
        mov edx, dword ptr [esp + 012ch] // 00c4e209
        fld dword ptr [esp + 0ach] // 00c4e210
        mov dword ptr [eax + ecx + 4], edx // 00c4e217
        fld st(0) // 00c4e21b
        add eax, ecx // 00c4e21d
        movss dword ptr [esp + 0130h], xmm0 // 00c4e21f
        mov edx, dword ptr [esp + 0130h] // 00c4e228
        mov dword ptr [eax + 8], edx // 00c4e22f
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e232
        lea edx, [ecx + eax + 0ch] // 00c4e238
        mov eax, dword ptr [esp + 028h] // 00c4e23c
        fmul dword ptr [eax - 0ch] // 00c4e240
        mov dword ptr [esp + 018h], edx // 00c4e243
        fld dword ptr [esp + 0b0h] // 00c4e247
        movaps xmm0, xmm2 // 00c4e24e
        subss xmm0, dword ptr [esp + 05ch] // 00c4e251
        fld st(0) // 00c4e257
        fmul dword ptr [eax - 010h] // 00c4e259
        movss dword ptr [esp + 0a8h], xmm0 // 00c4e25c
        fsubp st(2), st(0) // 00c4e265
        fxch st(1) // 00c4e267
        fstp dword ptr [edx] // 00c4e269
        mov edx, dword ptr [esp + 020h] // 00c4e26b
        fmul dword ptr [edx] // 00c4e26f
        mov edx, dword ptr [esp + 018h] // 00c4e271
        fld dword ptr [eax - 0ch] // 00c4e275
        fld dword ptr [esp + 0a8h] // 00c4e278
        fld st(0) // 00c4e27f
        fmulp st(2), st(0) // 00c4e281
        fxch st(2) // 00c4e283
        fsubrp st(1), st(0) // 00c4e285
        fstp dword ptr [edx + 4] // 00c4e287
        fmul dword ptr [eax - 010h] // 00c4e28a
        mov edx, dword ptr [esp + 020h] // 00c4e28d
        fld dword ptr [edx] // 00c4e291
        mov eax, dword ptr [esp + 018h] // 00c4e293
        fmulp st(2), st(0) // 00c4e297
        fsubrp st(1), st(0) // 00c4e299
        fstp dword ptr [eax + 8] // 00c4e29b
        mov edx, dword ptr [edx] // 00c4e29e
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e2a0
        fld dword ptr [esp + 078h] // 00c4e2a6
        mov dword ptr [ecx + eax + 018h], edx // 00c4e2aa
        fld st(0) // 00c4e2ae
        mov edx, dword ptr [esp + 020h] // 00c4e2b0
        mov edx, dword ptr [edx + 4] // 00c4e2b4
        mov dword ptr [ecx + eax + 01ch], edx // 00c4e2b7
        mov edx, dword ptr [esp + 020h] // 00c4e2bb
        mov edx, dword ptr [edx + 8] // 00c4e2bf
        mov dword ptr [ecx + eax + 020h], edx // 00c4e2c2
        lea eax, [ecx + eax + 018h] // 00c4e2c6
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e2ca
        lea edx, [ecx + eax + 024h] // 00c4e2d0
        mov eax, dword ptr [esp + 028h] // 00c4e2d4
        fmul dword ptr [eax - 0ch] // 00c4e2d8
        mov dword ptr [esp + 0ch], edx // 00c4e2db
        fld dword ptr [esp + 07ch] // 00c4e2df
        fld st(0) // 00c4e2e3
        fmul dword ptr [eax - 010h] // 00c4e2e5
        fsubp st(2), st(0) // 00c4e2e8
        fxch st(1) // 00c4e2ea
        fstp dword ptr [edx] // 00c4e2ec
        mov edx, dword ptr [esp + 020h] // 00c4e2ee
        fld st(0) // 00c4e2f2
        fmul dword ptr [edx] // 00c4e2f4
        mov edx, dword ptr [esp + 0ch] // 00c4e2f6
        fld dword ptr [eax - 0ch] // 00c4e2fa
        fld dword ptr [esp + 074h] // 00c4e2fd
        fld st(0) // 00c4e301
        fmulp st(2), st(0) // 00c4e303
        fxch st(2) // 00c4e305
        fsubrp st(1), st(0) // 00c4e307
        fstp dword ptr [edx + 4] // 00c4e309
        fld dword ptr [eax - 010h] // 00c4e30c
        mov eax, dword ptr [esp + 020h] // 00c4e30f
        fmul st(0), st(1) // 00c4e313
        fld st(3) // 00c4e315
        fmul dword ptr [eax] // 00c4e317
        fsubp st(1), st(0) // 00c4e319
        fstp dword ptr [edx + 8] // 00c4e31b
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e31e
        fld dword ptr [edi + 050h] // 00c4e324
        add eax, ecx // 00c4e327
        fstp dword ptr [esp + 0ch] // 00c4e329
        fld dword ptr [eax] // 00c4e32d
        fld dword ptr [esp + 0ch] // 00c4e32f
        fld st(0) // 00c4e333
        fmulp st(2), st(0) // 00c4e335
        fxch st(1) // 00c4e337
        fstp dword ptr [esp + 0158h] // 00c4e339
        mov edx, dword ptr [esp + 0158h] // 00c4e340
        fld dword ptr [eax + 4] // 00c4e347
        fmul st(0), st(1) // 00c4e34a
        fstp dword ptr [esp + 015ch] // 00c4e34c
        fmul dword ptr [eax + 8] // 00c4e353
        mov eax, dword ptr [esi + 0ea80h] // 00c4e356
        mov dword ptr [eax + ecx], edx // 00c4e35c
        mov edx, dword ptr [esp + 015ch] // 00c4e35f
        mov dword ptr [eax + ecx + 4], edx // 00c4e366
        fstp dword ptr [esp + 0160h] // 00c4e36a
        mov edx, dword ptr [esp + 0160h] // 00c4e371
        add eax, ecx // 00c4e378
        mov dword ptr [eax + 8], edx // 00c4e37a
        fld dword ptr [edi + 06ch] // 00c4e37d
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e380
        fmul dword ptr [ecx + eax + 010h] // 00c4e386
        mov edx, dword ptr [esi + 0ea80h] // 00c4e38a
        fld dword ptr [ecx + eax + 0ch] // 00c4e390
        lea eax, [ecx + eax + 0ch] // 00c4e394
        fmul dword ptr [edi + 060h] // 00c4e398
        lea edx, [edx + ecx + 0ch] // 00c4e39b
        faddp st(1), st(0) // 00c4e39f
        fld dword ptr [edi + 078h] // 00c4e3a1
        fmul dword ptr [eax + 8] // 00c4e3a4
        faddp st(1), st(0) // 00c4e3a7
        fstp dword ptr [edx] // 00c4e3a9
        fld dword ptr [edi + 070h] // 00c4e3ab
        fmul dword ptr [eax + 4] // 00c4e3ae
        fld dword ptr [edi + 064h] // 00c4e3b1
        fmul dword ptr [eax] // 00c4e3b4
        faddp st(1), st(0) // 00c4e3b6
        fld dword ptr [edi + 07ch] // 00c4e3b8
        fmul dword ptr [eax + 8] // 00c4e3bb
        faddp st(1), st(0) // 00c4e3be
        fstp dword ptr [edx + 4] // 00c4e3c0
        fld dword ptr [edi + 074h] // 00c4e3c3
        fmul dword ptr [eax + 4] // 00c4e3c6
        fld dword ptr [edi + 068h] // 00c4e3c9
        fmul dword ptr [eax] // 00c4e3cc
        faddp st(1), st(0) // 00c4e3ce
        fld dword ptr [edi + 080h] // 00c4e3d0
        fmul dword ptr [eax + 8] // 00c4e3d6
        faddp st(1), st(0) // 00c4e3d9
        fstp dword ptr [edx + 8] // 00c4e3db
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e3de
        fld dword ptr [ebp + 050h] // 00c4e3e4
        mov edx, dword ptr [esi + 0ea80h] // 00c4e3e7
        fstp dword ptr [esp + 0ch] // 00c4e3ed
        lea eax, [ecx + eax + 018h] // 00c4e3f1
        fld dword ptr [esp + 0ch] // 00c4e3f5
        fld st(0) // 00c4e3f9
        fmul dword ptr [eax] // 00c4e3fb
        fstp dword ptr [esp + 0f8h] // 00c4e3fd
        fld dword ptr [eax + 4] // 00c4e404
        fmul st(0), st(1) // 00c4e407
        fstp dword ptr [esp + 0fch] // 00c4e409
        fmul dword ptr [eax + 8] // 00c4e410
        lea eax, [edx + ecx + 018h] // 00c4e413
        mov edx, dword ptr [esp + 0f8h] // 00c4e417
        mov dword ptr [eax], edx // 00c4e41e
        fstp dword ptr [esp + 0100h] // 00c4e420
        mov edx, dword ptr [esp + 0fch] // 00c4e427
        mov dword ptr [eax + 4], edx // 00c4e42e
        mov edx, dword ptr [esp + 0100h] // 00c4e431
        mov dword ptr [eax + 8], edx // 00c4e438
        fld dword ptr [ebp + 06ch] // 00c4e43b
        mov eax, dword ptr [esi + 0ea7ch] // 00c4e43e
        fmul dword ptr [ecx + eax + 028h] // 00c4e444
        mov edx, dword ptr [esi + 0ea80h] // 00c4e448
        fld dword ptr [ecx + eax + 024h] // 00c4e44e
        lea eax, [ecx + eax + 024h] // 00c4e452
        fmul dword ptr [ebp + 060h] // 00c4e456
        lea edx, [edx + ecx + 024h] // 00c4e459
        faddp st(1), st(0) // 00c4e45d
        fld dword ptr [ebp + 078h] // 00c4e45f
        fmul dword ptr [eax + 8] // 00c4e462
        faddp st(1), st(0) // 00c4e465
        fstp dword ptr [edx] // 00c4e467
        fld dword ptr [ebp + 070h] // 00c4e469
        fmul dword ptr [eax + 4] // 00c4e46c
        fld dword ptr [ebp + 064h] // 00c4e46f
        fmul dword ptr [eax] // 00c4e472
        faddp st(1), st(0) // 00c4e474
        fld dword ptr [ebp + 07ch] // 00c4e476
        fmul dword ptr [eax + 8] // 00c4e479
        faddp st(1), st(0) // 00c4e47c
        fstp dword ptr [edx + 4] // 00c4e47e
        fld dword ptr [ebp + 074h] // 00c4e481
        fmul dword ptr [eax + 4] // 00c4e484
        fld dword ptr [ebp + 068h] // 00c4e487
        fmul dword ptr [eax] // 00c4e48a
        faddp st(1), st(0) // 00c4e48c
        fld dword ptr [ebp + 080h] // 00c4e48e
        fmul dword ptr [eax + 8] // 00c4e494
        faddp st(1), st(0) // 00c4e497
        fstp dword ptr [edx + 8] // 00c4e499
        fld dword ptr [edi + 010h] // 00c4e49c
        fstp dword ptr [esp + 01ch] // 00c4e49f
        fld dword ptr [edi + 014h] // 00c4e4a3
        fstp dword ptr [esp + 018h] // 00c4e4a6
        fld st(3) // 00c4e4aa
        fmul dword ptr [esp + 01ch] // 00c4e4ac
        fld st(5) // 00c4e4b0
        fmul dword ptr [esp + 018h] // 00c4e4b2
        fsubp st(1), st(0) // 00c4e4b6
        fstp dword ptr [esp + 0c4h] // 00c4e4b8
        fld dword ptr [edi + 0ch] // 00c4e4bf
        fstp dword ptr [esp + 024h] // 00c4e4c2
        fld dword ptr [esp + 05ch] // 00c4e4c6
        fld st(0) // 00c4e4ca
        fmul dword ptr [esp + 018h] // 00c4e4cc
        fld dword ptr [esp + 024h] // 00c4e4d0
        fmulp st(6), st(0) // 00c4e4d4
        fsubrp st(5), st(0) // 00c4e4d6
        fxch st(4) // 00c4e4d8
        fstp dword ptr [esp + 0c8h] // 00c4e4da
        fld dword ptr [esp + 024h] // 00c4e4e1
        fmulp st(5), st(0) // 00c4e4e5
        fld dword ptr [esp + 01ch] // 00c4e4e7
        fmulp st(4), st(0) // 00c4e4eb
        fxch st(4) // 00c4e4ed
        fsubrp st(3), st(0) // 00c4e4ef
        fxch st(2) // 00c4e4f1
        fstp dword ptr [esp + 0cch] // 00c4e4f3
        fld dword ptr [ebp + 010h] // 00c4e4fa
        fstp dword ptr [esp + 0ch] // 00c4e4fd
        fld dword ptr [ebp + 014h] // 00c4e501
        fstp dword ptr [esp + 01ch] // 00c4e504
        fld st(1) // 00c4e508
        fld dword ptr [esp + 0ch] // 00c4e50a
        fld st(0) // 00c4e50e
        fmulp st(2), st(0) // 00c4e510
        fld st(2) // 00c4e512
        fld dword ptr [esp + 01ch] // 00c4e514
        fld st(0) // 00c4e518
        fmulp st(2), st(0) // 00c4e51a
        fxch st(3) // 00c4e51c
        fsubrp st(1), st(0) // 00c4e51e
        fstp dword ptr [esp + 0140h] // 00c4e520
        fld dword ptr [ebp + 0ch] // 00c4e527
        fstp dword ptr [esp + 0ch] // 00c4e52a
        fld st(4) // 00c4e52e
        fmulp st(2), st(0) // 00c4e530
        fld dword ptr [esp + 0ch] // 00c4e532
        fld st(0) // 00c4e536
        fmulp st(5), st(0) // 00c4e538
        fxch st(2) // 00c4e53a
        fsubrp st(4), st(0) // 00c4e53c
        fxch st(3) // 00c4e53e
        fstp dword ptr [esp + 0144h] // 00c4e540
        fmulp st(1), st(0) // 00c4e547
        fxch st(2) // 00c4e549
        fmulp st(1), st(0) // 00c4e54b
        fsubp st(1), st(0) // 00c4e54d
        fstp dword ptr [esp + 0148h] // 00c4e54f
        fld dword ptr [ebp] // 00c4e556
        fstp dword ptr [esp + 0ch] // 00c4e559
        fld dword ptr [esp + 0140h] // 00c4e55d
        fld dword ptr [esp + 0ch] // 00c4e564
        fld st(0) // 00c4e568
        faddp st(2), st(0) // 00c4e56a
        fxch st(1) // 00c4e56c
        fstp dword ptr [esp + 0110h] // 00c4e56e
        fld dword ptr [ebp + 4] // 00c4e575
        fstp dword ptr [esp + 0ch] // 00c4e578
        fld dword ptr [esp + 0144h] // 00c4e57c
        fld dword ptr [esp + 0ch] // 00c4e583
        fld st(0) // 00c4e587
        faddp st(2), st(0) // 00c4e589
        fxch st(1) // 00c4e58b
        fstp dword ptr [esp + 0114h] // 00c4e58d
        fld dword ptr [ebp + 8] // 00c4e594
        fstp dword ptr [esp + 0ch] // 00c4e597
        fld dword ptr [esp + 0148h] // 00c4e59b
        fld dword ptr [esp + 0ch] // 00c4e5a2
        fld st(0) // 00c4e5a6
        faddp st(2), st(0) // 00c4e5a8
        fxch st(1) // 00c4e5aa
        fstp dword ptr [esp + 0118h] // 00c4e5ac
        fld dword ptr [edi] // 00c4e5b3
        fstp dword ptr [esp + 0ch] // 00c4e5b5
        fld dword ptr [esp + 0110h] // 00c4e5b9
        fld dword ptr [esp + 0ch] // 00c4e5c0
        fld st(0) // 00c4e5c4
        fsubp st(2), st(0) // 00c4e5c6
        fxch st(1) // 00c4e5c8
        fstp dword ptr [esp + 0b4h] // 00c4e5ca
        mov eax, dword ptr [esi] // 00c4e5d1
        fld dword ptr [edi + 4] // 00c4e5d3
        mov edx, dword ptr [esp + 020h] // 00c4e5d6
        fstp dword ptr [esp + 018h] // 00c4e5da
        fld dword ptr [esp + 0114h] // 00c4e5de
        fsub dword ptr [esp + 018h] // 00c4e5e5
        fstp dword ptr [esp + 0b8h] // 00c4e5e9
        fld dword ptr [edi + 8] // 00c4e5f0
        fstp dword ptr [esp + 024h] // 00c4e5f3
        fld dword ptr [esp + 0118h] // 00c4e5f7
        fsub dword ptr [esp + 024h] // 00c4e5fe
        fstp dword ptr [esp + 0bch] // 00c4e602
        fld dword ptr [esp + 0b4h] // 00c4e609
        fsub dword ptr [esp + 0c4h] // 00c4e610
        fstp dword ptr [esp + 068h] // 00c4e617
        fld dword ptr [esp + 0b8h] // 00c4e61b
        fsub dword ptr [esp + 0c8h] // 00c4e622
        fstp dword ptr [esp + 06ch] // 00c4e629
        fld dword ptr [esp + 0bch] // 00c4e62d
        fsub dword ptr [esp + 0cch] // 00c4e634
        fstp dword ptr [esp + 070h] // 00c4e63b
        fld dword ptr [eax + 018h] // 00c4e63f
        mov eax, dword ptr [esp + 028h] // 00c4e642
        fmul dword ptr [esp + 054h] // 00c4e646
        fdiv dword ptr [esp + 018ch] // 00c4e64a
        fstp dword ptr [esp + 010h] // 00c4e651
        fld dword ptr [esp + 068h] // 00c4e655
        fmul dword ptr [edx] // 00c4e659
        fld dword ptr [esp + 06ch] // 00c4e65b
        fmul dword ptr [eax - 010h] // 00c4e65f
        faddp st(1), st(0) // 00c4e662
        fld dword ptr [esp + 070h] // 00c4e664
        fmul dword ptr [eax - 0ch] // 00c4e668
        mov eax, dword ptr [esp + 050h] // 00c4e66b
        faddp st(1), st(0) // 00c4e66f
        fstp dword ptr [esp + 0ch] // 00c4e671
        fld dword ptr [esp + 0ch] // 00c4e675
        fmul dword ptr [eax + 4] // 00c4e679
        fadd qword ptr constant_00d7a270 // 00c4e67c
        fstp dword ptr [esp + 01ch] // 00c4e682
        fld dword ptr [esp + 01ch] // 00c4e686
        fldz  // 00c4e68a
        fcomip st(0), st(1) // 00c4e68c
        fstp st(0) // 00c4e68e
        jbe l_00c4e6a3 // 00c4e690
        movss xmm0, dword ptr [esp + 01ch] // 00c4e692
        movss dword ptr [esp + 084h], xmm0 // 00c4e698
        jmp l_00c4e6ac // 00c4e6a1
    l_00c4e6a3:
        movss dword ptr [esp + 084h], xmm1 // 00c4e6a3
    l_00c4e6ac:
        mov edx, dword ptr [esi + 0ea7ch] // 00c4e6ac
        fld dword ptr [esp + 084h] // 00c4e6b2
        fchs  // 00c4e6b9
        lea eax, [ecx + edx] // 00c4e6bb
        fld dword ptr [eax + 010h] // 00c4e6be
        mov edx, dword ptr [esi + 0ea88h] // 00c4e6c1
        fmul dword ptr [edi + 010h] // 00c4e6c7
        movss xmm0, dword ptr [esp + 010h] // 00c4e6ca
        fld dword ptr [eax + 0ch] // 00c4e6d0
        fmul dword ptr [edi + 0ch] // 00c4e6d3
        faddp st(1), st(0) // 00c4e6d6
        fld dword ptr [eax + 014h] // 00c4e6d8
        fmul dword ptr [edi + 014h] // 00c4e6db
        faddp st(1), st(0) // 00c4e6de
        fstp dword ptr [esp + 0ch] // 00c4e6e0
        fld dword ptr [esp + 0ch] // 00c4e6e4
        fld dword ptr [eax + 4] // 00c4e6e8
        fmul dword ptr [esp + 018h] // 00c4e6eb
        fld dword ptr [eax] // 00c4e6ef
        fmulp st(4), st(0) // 00c4e6f1
        faddp st(3), st(0) // 00c4e6f3
        fld dword ptr [eax + 8] // 00c4e6f5
        fmul dword ptr [esp + 024h] // 00c4e6f8
        faddp st(3), st(0) // 00c4e6fc
        fxch st(2) // 00c4e6fe
        fstp dword ptr [esp + 0ch] // 00c4e700
        fld dword ptr [esp + 0ch] // 00c4e704
        faddp st(2), st(0) // 00c4e708
        fld dword ptr [eax + 01ch] // 00c4e70a
        fmulp st(4), st(0) // 00c4e70d
        fld dword ptr [eax + 018h] // 00c4e70f
        fmulp st(5), st(0) // 00c4e712
        fxch st(3) // 00c4e714
        faddp st(4), st(0) // 00c4e716
        fld dword ptr [eax + 020h] // 00c4e718
        fmulp st(2), st(0) // 00c4e71b
        fxch st(3) // 00c4e71d
        faddp st(1), st(0) // 00c4e71f
        fstp dword ptr [esp + 0ch] // 00c4e721
        fld dword ptr [esp + 0ch] // 00c4e725
        faddp st(2), st(0) // 00c4e729
        fld dword ptr [eax + 028h] // 00c4e72b
        fmul dword ptr [ebp + 010h] // 00c4e72e
        fld dword ptr [eax + 024h] // 00c4e731
        fmul dword ptr [ebp + 0ch] // 00c4e734
        faddp st(1), st(0) // 00c4e737
        fld dword ptr [eax + 02ch] // 00c4e739
        mov eax, dword ptr [esp + 02ch] // 00c4e73c
        fmul dword ptr [ebp + 014h] // 00c4e740
        faddp st(1), st(0) // 00c4e743
        fstp dword ptr [esp + 0ch] // 00c4e745
        fld dword ptr [esp + 0ch] // 00c4e749
        faddp st(2), st(0) // 00c4e74d
        fsubrp st(1), st(0) // 00c4e74f
        fstp dword ptr [esp + 0ch] // 00c4e751
        fld dword ptr [esp + 0ch] // 00c4e755
        fstp dword ptr [edx + eax*4] // 00c4e759
        mov edx, dword ptr [esi + 0ea90h] // 00c4e75c
        movss dword ptr [edx + eax*4], xmm0 // 00c4e762
        mov eax, dword ptr [esi + 0ea80h] // 00c4e767
        mov edx, dword ptr [esi + 0ea7ch] // 00c4e76d
        fld dword ptr [ecx + edx + 010h] // 00c4e773
        add eax, ecx // 00c4e777
        fmul dword ptr [eax + 010h] // 00c4e779
        add ecx, edx // 00c4e77c
        fld dword ptr [ecx + 0ch] // 00c4e77e
        fmul dword ptr [eax + 0ch] // 00c4e781
        faddp st(1), st(0) // 00c4e784
        fld dword ptr [ecx + 014h] // 00c4e786
        fmul dword ptr [eax + 014h] // 00c4e789
        faddp st(1), st(0) // 00c4e78c
        fstp dword ptr [esp + 010h] // 00c4e78e
        fld dword ptr [esp + 010h] // 00c4e792
        fld dword ptr [ecx + 4] // 00c4e796
        fmul dword ptr [eax + 4] // 00c4e799
        fld dword ptr [eax] // 00c4e79c
        fmul dword ptr [ecx] // 00c4e79e
        faddp st(1), st(0) // 00c4e7a0
        fld dword ptr [ecx + 8] // 00c4e7a2
        fmul dword ptr [eax + 8] // 00c4e7a5
        faddp st(1), st(0) // 00c4e7a8
        fstp dword ptr [esp + 010h] // 00c4e7aa
        fadd dword ptr [esp + 010h] // 00c4e7ae
        fld dword ptr [ecx + 01ch] // 00c4e7b2
        fmul dword ptr [eax + 01ch] // 00c4e7b5
        fld dword ptr [ecx + 018h] // 00c4e7b8
        fmul dword ptr [eax + 018h] // 00c4e7bb
        faddp st(1), st(0) // 00c4e7be
        fld dword ptr [ecx + 020h] // 00c4e7c0
        fmul dword ptr [eax + 020h] // 00c4e7c3
        faddp st(1), st(0) // 00c4e7c6
        fstp dword ptr [esp + 010h] // 00c4e7c8
        fadd dword ptr [esp + 010h] // 00c4e7cc
        fld dword ptr [ecx + 028h] // 00c4e7d0
        fmul dword ptr [eax + 028h] // 00c4e7d3
        fld dword ptr [ecx + 024h] // 00c4e7d6
        fmul dword ptr [eax + 024h] // 00c4e7d9
        faddp st(1), st(0) // 00c4e7dc
        fld dword ptr [ecx + 02ch] // 00c4e7de
        mov ecx, dword ptr [esp + 02ch] // 00c4e7e1
        fmul dword ptr [eax + 02ch] // 00c4e7e5
        mov eax, dword ptr [esi + 0ea98h] // 00c4e7e8
        faddp st(1), st(0) // 00c4e7ee
        fstp dword ptr [esp + 010h] // 00c4e7f0
        fadd dword ptr [esp + 010h] // 00c4e7f4
        fld1  // 00c4e7f8
        fdivrp st(1), st(0) // 00c4e7fa
        fstp dword ptr [esp + 010h] // 00c4e7fc
        fld dword ptr [esp + 010h] // 00c4e800
        fstp dword ptr [eax + ecx*4] // 00c4e804
        mov edx, dword ptr [esi + 0ea8ch] // 00c4e807
        mov eax, dword ptr [esp + 04ch] // 00c4e80d
        mov ecx, dword ptr [esp + 020h] // 00c4e811
        movss dword ptr [eax + edx], xmm1 // 00c4e815
        fld dword ptr [ecx] // 00c4e81a
        fstp dword ptr [esp + 0ch] // 00c4e81c
        push ecx // 00c4e820
        fld dword ptr [esp + 010h] // 00c4e821
        fstp dword ptr [esp] // 00c4e825
        call abs_kernel // 00c4e828
        mov edx, dword ptr [esp + 028h] // 00c4e82d
        fstp dword ptr [esp + 018h] // 00c4e831
        fld dword ptr [edx - 010h] // 00c4e835
        push ecx // 00c4e838
        fstp dword ptr [esp] // 00c4e839
        mov dword ptr [esp + 028h], 0 // 00c4e83c
        call abs_kernel // 00c4e844
        fstp dword ptr [esp + 01ch] // 00c4e849
        fld dword ptr [esp + 018h] // 00c4e84d
        fld dword ptr [esp + 01ch] // 00c4e851
        fcomip st(0), st(1) // 00c4e855
        fstp st(0) // 00c4e857
        jbe l_00c4e86f // 00c4e859
        movss xmm0, dword ptr [esp + 01ch] // 00c4e85b
        movss dword ptr [esp + 018h], xmm0 // 00c4e861
        mov dword ptr [esp + 024h], 1 // 00c4e867
    l_00c4e86f:
        fld dword ptr [esp + 018h] // 00c4e86f
        mov eax, dword ptr [esp + 028h] // 00c4e873
        fstp qword ptr [esp + 010h] // 00c4e877
        push ecx // 00c4e87b
        fld dword ptr [eax - 0ch] // 00c4e87c
        fstp dword ptr [esp] // 00c4e87f
        call abs_kernel // 00c4e882
        fld qword ptr [esp + 010h] // 00c4e887
        fxch st(1) // 00c4e88b
        fcomip st(0), st(1) // 00c4e88d
        fstp st(0) // 00c4e88f
        jbe l_00c4e89b // 00c4e891
        mov dword ptr [esp + 024h], 2 // 00c4e893
    l_00c4e89b:
        mov ecx, dword ptr [esp + 024h] // 00c4e89b
        mov edx, dword ptr [esp + 020h] // 00c4e89f
        movss xmm0, dword ptr constant_00d7a208 // 00c4e8a3
        mov eax, 1 // 00c4e8ab
        shl eax, cl // 00c4e8b0
        mov ecx, dword ptr [edx] // 00c4e8b2
        mov dword ptr [esp + 03ch], ecx // 00c4e8b4
        mov ecx, dword ptr [edx + 4] // 00c4e8b8
        mov edx, dword ptr [edx + 8] // 00c4e8bb
        mov dword ptr [esp + 044h], edx // 00c4e8be
        mov dword ptr [esp + 040h], ecx // 00c4e8c2
        and eax, 3 // 00c4e8c6
        mov ecx, dword ptr [esp + eax*4 + 03ch] // 00c4e8c9
        lea edx, [esp + eax*4 + 03ch] // 00c4e8cd
        mov dword ptr [esp + 010h], eax // 00c4e8d1
        mov eax, dword ptr [esp + 024h] // 00c4e8d5
        xor dword ptr [esp + eax*4 + 03ch], ecx // 00c4e8d9
        mov ecx, dword ptr [esp + eax*4 + 03ch] // 00c4e8dd
        xor dword ptr [edx], ecx // 00c4e8e1
        mov edx, dword ptr [edx] // 00c4e8e3
        xor dword ptr [esp + eax*4 + 03ch], edx // 00c4e8e5
        mov ecx, dword ptr [esp + 010h] // 00c4e8e9
        subss xmm0, dword ptr [esp + eax*4 + 03ch] // 00c4e8ed
        lea eax, [esp + eax*4 + 03ch] // 00c4e8f3
        mov edx, 1 // 00c4e8f7
        shl edx, cl // 00c4e8fc
        movss dword ptr [eax], xmm0 // 00c4e8fe
        xorps xmm0, xmm0 // 00c4e902
        push ecx // 00c4e905
        and edx, 3 // 00c4e906
        movss dword ptr [esp + edx*4 + 040h], xmm0 // 00c4e909
        fld dword ptr [esp + 044h] // 00c4e90f
        fld dword ptr [esp + 040h] // 00c4e913
        fld dword ptr [esp + 048h] // 00c4e917
        fld st(1) // 00c4e91b
        fmulp st(2), st(0) // 00c4e91d
        fld st(2) // 00c4e91f
        fmulp st(3), st(0) // 00c4e921
        fxch st(1) // 00c4e923
        faddp st(2), st(0) // 00c4e925
        fmul st(0), st(0) // 00c4e927
        faddp st(1), st(0) // 00c4e929
        fstp dword ptr [esp + 014h] // 00c4e92b
        fld dword ptr [esp + 014h] // 00c4e92f
        fstp dword ptr [esp] // 00c4e933
        push dword ptr [esp+404] // Borrowed context, original frame offsets retained.
        call sqrt_kernel // 00c4e936
        fstp dword ptr [esp + 010h] // 00c4e93b
        mov eax, dword ptr [esp + 028h] // 00c4e93f
        fld dword ptr [esp + 03ch] // 00c4e943
        fld dword ptr [esp + 010h] // 00c4e947
        fld st(0) // 00c4e94b
        fdivp st(2), st(0) // 00c4e94d
        fxch st(1) // 00c4e94f
        fstp dword ptr [esp + 03ch] // 00c4e951
        fld dword ptr [esp + 040h] // 00c4e955
        fdiv st(0), st(1) // 00c4e959
        fstp dword ptr [esp + 040h] // 00c4e95b
        fdivr dword ptr [esp + 044h] // 00c4e95f
        fstp dword ptr [esp + 044h] // 00c4e963
        fld dword ptr [eax - 0ch] // 00c4e967
        fstp dword ptr [esp + 010h] // 00c4e96a
        fld dword ptr [eax - 010h] // 00c4e96e
        fstp dword ptr [esp + 01ch] // 00c4e971
        fld dword ptr [esp + 040h] // 00c4e975
        fld st(0) // 00c4e979
        fld dword ptr [esp + 010h] // 00c4e97b
        fld st(0) // 00c4e97f
        fmulp st(2), st(0) // 00c4e981
        fld dword ptr [esp + 044h] // 00c4e983
        fld st(0) // 00c4e987
        fld dword ptr [esp + 01ch] // 00c4e989
        fld st(0) // 00c4e98d
        fmulp st(2), st(0) // 00c4e98f
        fxch st(4) // 00c4e991
        fsubrp st(1), st(0) // 00c4e993
        fstp dword ptr [esp + 094h] // 00c4e995
        fld st(0) // 00c4e99c
        fld dword ptr [esp + 0ch] // 00c4e99e
        fld st(0) // 00c4e9a2
        fmulp st(2), st(0) // 00c4e9a4
        fld dword ptr [esp + 03ch] // 00c4e9a6
        fld st(0) // 00c4e9aa
        fmulp st(5), st(0) // 00c4e9ac
        fxch st(2) // 00c4e9ae
        fsubrp st(4), st(0) // 00c4e9b0
        fxch st(3) // 00c4e9b2
        fstp dword ptr [esp + 098h] // 00c4e9b4
        fld st(0) // 00c4e9bb
        fmulp st(4), st(0) // 00c4e9bd
        fld st(4) // 00c4e9bf
        fmulp st(3), st(0) // 00c4e9c1
        fxch st(3) // 00c4e9c3
        fsubrp st(2), st(0) // 00c4e9c5
        fxch st(1) // 00c4e9c7
        fstp dword ptr [esp + 09ch] // 00c4e9c9
        fld dword ptr [esp + 098h] // 00c4e9d0
        fld dword ptr [esp + 06ch] // 00c4e9d7
        fld st(0) // 00c4e9db
        fmulp st(2), st(0) // 00c4e9dd
        fld dword ptr [esp + 094h] // 00c4e9df
        fld dword ptr [esp + 068h] // 00c4e9e6
        fld st(0) // 00c4e9ea
        fmulp st(2), st(0) // 00c4e9ec
        fxch st(3) // 00c4e9ee
        faddp st(1), st(0) // 00c4e9f0
        fld dword ptr [esp + 09ch] // 00c4e9f2
        fmul dword ptr [esp + 070h] // 00c4e9f9
        faddp st(1), st(0) // 00c4e9fd
        fstp dword ptr [esp + 010h] // 00c4e9ff
        fld dword ptr [esp + 094h] // 00c4ea03
        fld dword ptr [esp + 010h] // 00c4ea0a
        fld st(0) // 00c4ea0e
        fmulp st(2), st(0) // 00c4ea10
        fxch st(1) // 00c4ea12
        fstp dword ptr [esp + 0ech] // 00c4ea14
        fld dword ptr [esp + 098h] // 00c4ea1b
        fmul st(0), st(1) // 00c4ea22
        fstp dword ptr [esp + 0f0h] // 00c4ea24
        fmul dword ptr [esp + 09ch] // 00c4ea2b
        fstp dword ptr [esp + 0f4h] // 00c4ea32
        fmul st(0), st(4) // 00c4ea39
        fld st(3) // 00c4ea3b
        fmulp st(2), st(0) // 00c4ea3d
        faddp st(1), st(0) // 00c4ea3f
        fld st(1) // 00c4ea41
        fmul dword ptr [esp + 070h] // 00c4ea43
        faddp st(1), st(0) // 00c4ea47
        fstp dword ptr [esp + 010h] // 00c4ea49
        fld dword ptr [esp + 010h] // 00c4ea4d
        fld st(0) // 00c4ea51
        fmulp st(3), st(0) // 00c4ea53
        fxch st(2) // 00c4ea55
        fstp dword ptr [esp + 0dch] // 00c4ea57
        fld st(1) // 00c4ea5e
        fmulp st(3), st(0) // 00c4ea60
        fxch st(2) // 00c4ea62
        fstp dword ptr [esp + 0e0h] // 00c4ea64
        fmulp st(1), st(0) // 00c4ea6b
        fstp dword ptr [esp + 0e4h] // 00c4ea6d
        fld dword ptr [esp + 0dch] // 00c4ea74
        fadd dword ptr [esp + 0ech] // 00c4ea7b
        fstp dword ptr [esp + 030h] // 00c4ea82
        fld dword ptr [esp + 0e0h] // 00c4ea86
        fadd dword ptr [esp + 0f0h] // 00c4ea8d
        fstp dword ptr [esp + 034h] // 00c4ea94
        fld dword ptr [esp + 0e4h] // 00c4ea98
        fadd dword ptr [esp + 0f4h] // 00c4ea9f
        fstp dword ptr [esp + 038h] // 00c4eaa6
        fld dword ptr [esp + 030h] // 00c4eaaa
        fld dword ptr [esp + 034h] // 00c4eaae
        fld dword ptr [esp + 038h] // 00c4eab2
        fld st(1) // 00c4eab6
        fmulp st(2), st(0) // 00c4eab8
        fld st(2) // 00c4eaba
        fmulp st(3), st(0) // 00c4eabc
        fxch st(1) // 00c4eabe
        faddp st(2), st(0) // 00c4eac0
        fmul st(0), st(0) // 00c4eac2
        faddp st(1), st(0) // 00c4eac4
        fstp dword ptr [esp + 010h] // 00c4eac6
        fld dword ptr constant_00d7a288 // 00c4eaca
        fld dword ptr [esp + 010h] // 00c4ead0
        fcomi st(0), st(1) // 00c4ead4
        fstp st(1) // 00c4ead6
        jbe l_00c4eb0d // 00c4ead8
        push ecx // 00c4eada
        fstp dword ptr [esp] // 00c4eadb
        push dword ptr [esp+404] // Borrowed context, original frame offsets retained.
        call sqrt_kernel // 00c4eade
        fstp dword ptr [esp + 010h] // 00c4eae3
        fld dword ptr [esp + 030h] // 00c4eae7
        fld dword ptr [esp + 010h] // 00c4eaeb
        fld st(0) // 00c4eaef
        fdivp st(2), st(0) // 00c4eaf1
        fxch st(1) // 00c4eaf3
        fstp dword ptr [esp + 030h] // 00c4eaf5
        fld dword ptr [esp + 034h] // 00c4eaf9
        fdiv st(0), st(1) // 00c4eafd
        fstp dword ptr [esp + 034h] // 00c4eaff
        fdivr dword ptr [esp + 038h] // 00c4eb03
        fstp dword ptr [esp + 038h] // 00c4eb07
        jmp l_00c4eb27 // 00c4eb0b
    l_00c4eb0d:
        mov eax, dword ptr [esp + 03ch] // 00c4eb0d
        fstp st(0) // 00c4eb11
        mov ecx, dword ptr [esp + 040h] // 00c4eb13
        mov edx, dword ptr [esp + 044h] // 00c4eb17
        mov dword ptr [esp + 030h], eax // 00c4eb1b
        mov dword ptr [esp + 034h], ecx // 00c4eb1f
        mov dword ptr [esp + 038h], edx // 00c4eb23
    l_00c4eb27:
        fld dword ptr [esp + 060h] // 00c4eb27
        mov eax, dword ptr [esi + 0ea84h] // 00c4eb2b
        fld st(0) // 00c4eb31
        movzx ecx, word ptr [esp + 0a0h] // 00c4eb33
        fld dword ptr [esp + 038h] // 00c4eb3b
        mov edx, dword ptr [esp + 04ch] // 00c4eb3f
        fld st(0) // 00c4eb43
        movss xmm2, dword ptr constant_00d7a208 // 00c4eb45
        fmulp st(2), st(0) // 00c4eb4d
        mov word ptr [edx + eax], cx // 00c4eb4f
        fld dword ptr [esp + 064h] // 00c4eb53
        mov eax, dword ptr [esi + 0ea84h] // 00c4eb57
        fld st(0) // 00c4eb5d
        movzx ecx, word ptr [esp + 080h] // 00c4eb5f
        fld dword ptr [esp + 034h] // 00c4eb67
        fld st(0) // 00c4eb6b
        mov word ptr [edx + eax + 2], cx // 00c4eb6d
        fmulp st(2), st(0) // 00c4eb72
        mov eax, dword ptr [esi + 0ea7ch] // 00c4eb74
        fxch st(4) // 00c4eb7a
        add eax, ebx // 00c4eb7c
        movaps xmm0, xmm2 // 00c4eb7e
        fsubrp st(1), st(0) // 00c4eb81
        subss xmm0, dword ptr [esp + 030h] // 00c4eb83
        movss dword ptr [esp + 0104h], xmm0 // 00c4eb89
        mov ecx, dword ptr [esp + 0104h] // 00c4eb92
        fstp dword ptr [esp + 011ch] // 00c4eb99
        mov dword ptr [eax], ecx // 00c4eba0
        fld dword ptr [esp + 030h] // 00c4eba2
        movaps xmm0, xmm2 // 00c4eba6
        fld st(0) // 00c4eba9
        subss xmm0, dword ptr [esp + 034h] // 00c4ebab
        fmulp st(2), st(0) // 00c4ebb1
        movss dword ptr [esp + 0108h], xmm0 // 00c4ebb3
        fld dword ptr [esp + 05ch] // 00c4ebbc
        mov ecx, dword ptr [esp + 0108h] // 00c4ebc0
        fld st(0) // 00c4ebc7
        mov dword ptr [eax + 4], ecx // 00c4ebc9
        fmul st(0), st(4) // 00c4ebcc
        movaps xmm0, xmm2 // 00c4ebce
        subss xmm0, dword ptr [esp + 038h] // 00c4ebd1
        movss dword ptr [esp + 010ch], xmm0 // 00c4ebd7
        fsubp st(3), st(0) // 00c4ebe0
        mov ecx, dword ptr [esp + 010ch] // 00c4ebe2
        fxch st(2) // 00c4ebe9
        mov dword ptr [eax + 8], ecx // 00c4ebeb
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ebee
        fstp dword ptr [esp + 0120h] // 00c4ebf4
        lea eax, [eax + ebx + 0ch] // 00c4ebfb
        fld st(3) // 00c4ebff
        movaps xmm0, xmm2 // 00c4ec01
        subss xmm0, dword ptr [esp + 011ch] // 00c4ec04
        fmulp st(2), st(0) // 00c4ec0d
        movss dword ptr [esp + 0134h], xmm0 // 00c4ec0f
        mov ecx, dword ptr [esp + 0134h] // 00c4ec18
        fld st(0) // 00c4ec1f
        mov dword ptr [eax], ecx // 00c4ec21
        fmulp st(5), st(0) // 00c4ec23
        fxch st(1) // 00c4ec25
        fsubrp st(4), st(0) // 00c4ec27
        fxch st(3) // 00c4ec29
        fstp dword ptr [esp + 0124h] // 00c4ec2b
        fld dword ptr [esp + 0120h] // 00c4ec32
        fchs  // 00c4ec39
        fstp dword ptr [esp + 0138h] // 00c4ec3b
        mov ecx, dword ptr [esp + 0138h] // 00c4ec42
        fld dword ptr [esp + 0124h] // 00c4ec49
        mov dword ptr [eax + 4], ecx // 00c4ec50
        fchs  // 00c4ec53
        fstp dword ptr [esp + 013ch] // 00c4ec55
        mov ecx, dword ptr [esp + 013ch] // 00c4ec5c
        fld dword ptr [esp + 078h] // 00c4ec63
        mov dword ptr [eax + 8], ecx // 00c4ec67
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ec6a
        mov ecx, dword ptr [esp + 030h] // 00c4ec70
        lea eax, [eax + ebx + 018h] // 00c4ec74
        mov dword ptr [eax], ecx // 00c4ec78
        mov ecx, dword ptr [esp + 034h] // 00c4ec7a
        mov dword ptr [eax + 4], ecx // 00c4ec7e
        mov ecx, dword ptr [esp + 038h] // 00c4ec81
        mov dword ptr [eax + 8], ecx // 00c4ec85
        fld st(0) // 00c4ec88
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ec8a
        fmul st(0), st(2) // 00c4ec90
        lea eax, [eax + ebx + 024h] // 00c4ec92
        fld dword ptr [esp + 07ch] // 00c4ec96
        fld st(0) // 00c4ec9a
        fmul st(0), st(5) // 00c4ec9c
        fsubp st(2), st(0) // 00c4ec9e
        fxch st(1) // 00c4eca0
        fstp dword ptr [esp + 014ch] // 00c4eca2
        mov ecx, dword ptr [esp + 014ch] // 00c4eca9
        mov dword ptr [eax], ecx // 00c4ecb0
        fmul st(0), st(4) // 00c4ecb2
        fld dword ptr [esp + 074h] // 00c4ecb4
        fld st(0) // 00c4ecb8
        fmulp st(4), st(0) // 00c4ecba
        fxch st(1) // 00c4ecbc
        fsubrp st(3), st(0) // 00c4ecbe
        fxch st(2) // 00c4ecc0
        fstp dword ptr [esp + 0150h] // 00c4ecc2
        mov ecx, dword ptr [esp + 0150h] // 00c4ecc9
        mov dword ptr [eax + 4], ecx // 00c4ecd0
        fxch st(1) // 00c4ecd3
        fmulp st(2), st(0) // 00c4ecd5
        fmulp st(2), st(0) // 00c4ecd7
        fsubrp st(1), st(0) // 00c4ecd9
        fstp dword ptr [esp + 0154h] // 00c4ecdb
        mov ecx, dword ptr [esp + 0154h] // 00c4ece2
        mov dword ptr [eax + 8], ecx // 00c4ece9
        fld dword ptr [edi + 050h] // 00c4ecec
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ecef
        fstp dword ptr [esp + 010h] // 00c4ecf5
        fld dword ptr [esp + 010h] // 00c4ecf9
        add eax, ebx // 00c4ecfd
        fld st(0) // 00c4ecff
        fmul dword ptr [eax] // 00c4ed01
        fstp dword ptr [esp + 0164h] // 00c4ed03
        mov ecx, dword ptr [esp + 0164h] // 00c4ed0a
        fld dword ptr [eax + 4] // 00c4ed11
        fmul st(0), st(1) // 00c4ed14
        fstp dword ptr [esp + 0168h] // 00c4ed16
        fmul dword ptr [eax + 8] // 00c4ed1d
        mov eax, dword ptr [esi + 0ea80h] // 00c4ed20
        mov dword ptr [eax + ebx], ecx // 00c4ed26
        mov ecx, dword ptr [esp + 0168h] // 00c4ed29
        fstp dword ptr [esp + 016ch] // 00c4ed30
        mov dword ptr [eax + ebx + 4], ecx // 00c4ed37
        mov ecx, dword ptr [esp + 016ch] // 00c4ed3b
        mov dword ptr [eax + ebx + 8], ecx // 00c4ed42
        fld dword ptr [edi + 06ch] // 00c4ed46
        mov ecx, dword ptr [esi + 0ea80h] // 00c4ed49
        add eax, ebx // 00c4ed4f
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ed51
        fmul dword ptr [eax + ebx + 010h] // 00c4ed57
        lea eax, [eax + ebx + 0ch] // 00c4ed5b
        fld dword ptr [eax] // 00c4ed5f
        lea ecx, [ecx + ebx + 0ch] // 00c4ed61
        fmul dword ptr [edi + 060h] // 00c4ed65
        faddp st(1), st(0) // 00c4ed68
        fld dword ptr [edi + 078h] // 00c4ed6a
        fmul dword ptr [eax + 8] // 00c4ed6d
        faddp st(1), st(0) // 00c4ed70
        fstp dword ptr [ecx] // 00c4ed72
        fld dword ptr [edi + 070h] // 00c4ed74
        fmul dword ptr [eax + 4] // 00c4ed77
        fld dword ptr [edi + 064h] // 00c4ed7a
        fmul dword ptr [eax] // 00c4ed7d
        faddp st(1), st(0) // 00c4ed7f
        fld dword ptr [edi + 07ch] // 00c4ed81
        fmul dword ptr [eax + 8] // 00c4ed84
        faddp st(1), st(0) // 00c4ed87
        fstp dword ptr [ecx + 4] // 00c4ed89
        fld dword ptr [edi + 074h] // 00c4ed8c
        fmul dword ptr [eax + 4] // 00c4ed8f
        fld dword ptr [edi + 068h] // 00c4ed92
        fmul dword ptr [eax] // 00c4ed95
        faddp st(1), st(0) // 00c4ed97
        fld dword ptr [edi + 080h] // 00c4ed99
        fmul dword ptr [eax + 8] // 00c4ed9f
        faddp st(1), st(0) // 00c4eda2
        fstp dword ptr [ecx + 8] // 00c4eda4
        fld dword ptr [ebp + 050h] // 00c4eda7
        mov eax, dword ptr [esi + 0ea7ch] // 00c4edaa
        fstp dword ptr [esp + 010h] // 00c4edb0
        mov ecx, dword ptr [esi + 0ea80h] // 00c4edb4
        fld dword ptr [esp + 010h] // 00c4edba
        lea eax, [eax + ebx + 018h] // 00c4edbe
        fld st(0) // 00c4edc2
        fmul dword ptr [eax] // 00c4edc4
        fstp dword ptr [esp + 017ch] // 00c4edc6
        fld dword ptr [eax + 4] // 00c4edcd
        fmul st(0), st(1) // 00c4edd0
        fstp dword ptr [esp + 0180h] // 00c4edd2
        fmul dword ptr [eax + 8] // 00c4edd9
        lea eax, [ecx + ebx + 018h] // 00c4eddc
        mov ecx, dword ptr [esp + 017ch] // 00c4ede0
        mov dword ptr [eax], ecx // 00c4ede7
        fstp dword ptr [esp + 0184h] // 00c4ede9
        mov ecx, dword ptr [esp + 0180h] // 00c4edf0
        mov dword ptr [eax + 4], ecx // 00c4edf7
        mov ecx, dword ptr [esp + 0184h] // 00c4edfa
        mov dword ptr [eax + 8], ecx // 00c4ee01
        fld dword ptr [ebp + 06ch] // 00c4ee04
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ee07
        fmul dword ptr [eax + ebx + 028h] // 00c4ee0d
        mov ecx, dword ptr [esi + 0ea80h] // 00c4ee11
        fld dword ptr [eax + ebx + 024h] // 00c4ee17
        lea eax, [eax + ebx + 024h] // 00c4ee1b
        fmul dword ptr [ebp + 060h] // 00c4ee1f
        lea ecx, [ecx + ebx + 024h] // 00c4ee22
        faddp st(1), st(0) // 00c4ee26
        fld dword ptr [ebp + 078h] // 00c4ee28
        fmul dword ptr [eax + 8] // 00c4ee2b
        faddp st(1), st(0) // 00c4ee2e
        fstp dword ptr [ecx] // 00c4ee30
        fld dword ptr [ebp + 064h] // 00c4ee32
        fmul dword ptr [eax] // 00c4ee35
        fld dword ptr [ebp + 070h] // 00c4ee37
        fmul dword ptr [eax + 4] // 00c4ee3a
        faddp st(1), st(0) // 00c4ee3d
        fld dword ptr [ebp + 07ch] // 00c4ee3f
        fmul dword ptr [eax + 8] // 00c4ee42
        faddp st(1), st(0) // 00c4ee45
        fstp dword ptr [ecx + 4] // 00c4ee47
        fld dword ptr [ebp + 068h] // 00c4ee4a
        fmul dword ptr [eax] // 00c4ee4d
        fld dword ptr [ebp + 074h] // 00c4ee4f
        fmul dword ptr [eax + 4] // 00c4ee52
        faddp st(1), st(0) // 00c4ee55
        fld dword ptr [ebp + 080h] // 00c4ee57
        fmul dword ptr [eax + 8] // 00c4ee5d
        faddp st(1), st(0) // 00c4ee60
        fstp dword ptr [ecx + 8] // 00c4ee62
        mov eax, dword ptr [esi + 0ea7ch] // 00c4ee65
        fld dword ptr [eax + ebx + 010h] // 00c4ee6b
        add eax, ebx // 00c4ee6f
        fmul dword ptr [edi + 010h] // 00c4ee71
        fld dword ptr [eax + 0ch] // 00c4ee74
        fmul dword ptr [edi + 0ch] // 00c4ee77
        faddp st(1), st(0) // 00c4ee7a
        fld dword ptr [eax + 014h] // 00c4ee7c
        fmul dword ptr [edi + 014h] // 00c4ee7f
        faddp st(1), st(0) // 00c4ee82
        fstp dword ptr [esp + 010h] // 00c4ee84
        fld dword ptr [esp + 010h] // 00c4ee88
        fld dword ptr [eax + 4] // 00c4ee8c
        fmul dword ptr [edi + 4] // 00c4ee8f
        fld dword ptr [eax] // 00c4ee92
        fmul dword ptr [edi] // 00c4ee94
        faddp st(1), st(0) // 00c4ee96
        fld dword ptr [eax + 8] // 00c4ee98
        fmul dword ptr [edi + 8] // 00c4ee9b
        faddp st(1), st(0) // 00c4ee9e
        fstp dword ptr [esp + 010h] // 00c4eea0
        fadd dword ptr [esp + 010h] // 00c4eea4
        fld dword ptr [eax + 01ch] // 00c4eea8
        fmul dword ptr [ebp + 4] // 00c4eeab
        fld dword ptr [eax + 018h] // 00c4eeae
        fmul dword ptr [ebp] // 00c4eeb1
        faddp st(1), st(0) // 00c4eeb4
        fld dword ptr [eax + 020h] // 00c4eeb6
        fmul dword ptr [ebp + 8] // 00c4eeb9
        faddp st(1), st(0) // 00c4eebc
        mov ecx, dword ptr [esi + 0ea88h] // 00c4eebe
        add dword ptr [esp + 020h], 030h // 00c4eec4
        add dword ptr [esp + 028h], 030h // 00c4eec9
        fstp dword ptr [esp + 010h] // 00c4eece
        add dword ptr [esp + 058h], 030h // 00c4eed2
        fadd dword ptr [esp + 010h] // 00c4eed7
        fld dword ptr [eax + 028h] // 00c4eedb
        fmul dword ptr [ebp + 010h] // 00c4eede
        fld dword ptr [eax + 024h] // 00c4eee1
        fmul dword ptr [ebp + 0ch] // 00c4eee4
        faddp st(1), st(0) // 00c4eee7
        fld dword ptr [eax + 02ch] // 00c4eee9
        fmul dword ptr [ebp + 014h] // 00c4eeec
        faddp st(1), st(0) // 00c4eeef
        fstp dword ptr [esp + 010h] // 00c4eef1
        fadd dword ptr [esp + 010h] // 00c4eef5
        fchs  // 00c4eef9
        fstp dword ptr [esp + 010h] // 00c4eefb
        fld dword ptr [esp + 010h] // 00c4eeff
        fstp dword ptr [edx + ecx] // 00c4ef03
        mov ecx, dword ptr [esi + 0ea7ch] // 00c4ef06
        mov eax, dword ptr [esi + 0ea80h] // 00c4ef0c
        fld dword ptr [ecx + ebx + 4] // 00c4ef12
        fmul dword ptr [eax + ebx + 4] // 00c4ef16
        add eax, ebx // 00c4ef1a
        fld dword ptr [ecx + ebx] // 00c4ef1c
        add ecx, ebx // 00c4ef1f
        fmul dword ptr [eax] // 00c4ef21
        faddp st(1), st(0) // 00c4ef23
        fld dword ptr [ecx + 8] // 00c4ef25
        fmul dword ptr [eax + 8] // 00c4ef28
        faddp st(1), st(0) // 00c4ef2b
        fstp dword ptr [esp + 010h] // 00c4ef2d
        fld dword ptr [esp + 010h] // 00c4ef31
        fld dword ptr [ecx + 010h] // 00c4ef35
        fmul dword ptr [eax + 010h] // 00c4ef38
        fld dword ptr [ecx + 0ch] // 00c4ef3b
        fmul dword ptr [eax + 0ch] // 00c4ef3e
        faddp st(1), st(0) // 00c4ef41
        fld dword ptr [ecx + 014h] // 00c4ef43
        fmul dword ptr [eax + 014h] // 00c4ef46
        faddp st(1), st(0) // 00c4ef49
        fstp dword ptr [esp + 010h] // 00c4ef4b
        fadd dword ptr [esp + 010h] // 00c4ef4f
        fld dword ptr [ecx + 01ch] // 00c4ef53
        fmul dword ptr [eax + 01ch] // 00c4ef56
        fld dword ptr [ecx + 018h] // 00c4ef59
        fmul dword ptr [eax + 018h] // 00c4ef5c
        faddp st(1), st(0) // 00c4ef5f
        fld dword ptr [ecx + 020h] // 00c4ef61
        fmul dword ptr [eax + 020h] // 00c4ef64
        faddp st(1), st(0) // 00c4ef67
        fstp dword ptr [esp + 010h] // 00c4ef69
        fadd dword ptr [esp + 010h] // 00c4ef6d
        fld dword ptr [ecx + 028h] // 00c4ef71
        fmul dword ptr [eax + 028h] // 00c4ef74
        fld dword ptr [ecx + 024h] // 00c4ef77
        fmul dword ptr [eax + 024h] // 00c4ef7a
        faddp st(1), st(0) // 00c4ef7d
        fld dword ptr [ecx + 02ch] // 00c4ef7f
        fmul dword ptr [eax + 02ch] // 00c4ef82
        mov eax, dword ptr [esi + 0ea98h] // 00c4ef85
        faddp st(1), st(0) // 00c4ef8b
        fstp dword ptr [esp + 010h] // 00c4ef8d
        fadd dword ptr [esp + 010h] // 00c4ef91
        fld1  // 00c4ef95
        fdivrp st(1), st(0) // 00c4ef97
        fstp dword ptr [esp + 010h] // 00c4ef99
        fld dword ptr [esp + 010h] // 00c4ef9d
        fstp dword ptr [edx + eax] // 00c4efa1
        mov eax, dword ptr [esp + 050h] // 00c4efa4
        fld dword ptr [eax] // 00c4efa8
        mov eax, dword ptr [esp + 02ch] // 00c4efaa
        mov ecx, dword ptr [esi + 0ea9ch] // 00c4efae
        add dword ptr [esp + 02ch], 1 // 00c4efb4
        fstp dword ptr [ecx + eax*4] // 00c4efb9
        mov eax, dword ptr [esp + 048h] // 00c4efbc
        add eax, 1 // 00c4efc0
        mov dword ptr [esp + 048h], eax // 00c4efc3
        add ebx, 030h // 00c4efc7
        mov ecx, dword ptr [esp + 050h] // 00c4efca
        xorps xmm1, xmm1 // 00c4efce
        add edx, 4 // 00c4efd1
        cmp eax, dword ptr [ecx + 0c8h] // 00c4efd4
        mov dword ptr [esp + 04ch], edx // 00c4efda
        jl l_00c4df94 // 00c4efde
    l_00c4efe4:
        mov ecx, dword ptr [esp + 08ch] // 00c4efe4
        mov edx, dword ptr [esi + 0ea68h] // 00c4efeb
        add ecx, 1 // 00c4eff1
        cmp ecx, dword ptr [edx + 4] // 00c4eff4
        mov dword ptr [esp + 08ch], ecx // 00c4eff7
        jl l_00c4de90 // 00c4effe
        mov eax, dword ptr [esp + 02ch] // 00c4f004
        mov ecx, dword ptr [esp + 090h] // 00c4f008
        pop ebp // 00c4f00f
        pop ebx // 00c4f010
        mov dword ptr [esi + 0eaa4h], eax // 00c4f011
        mov dword ptr [esi + 0eaa8h], ecx // 00c4f017
        pop edi // 00c4f01d
        add esp, 017ch // 00c4f01e
        ret 8 // 00c4f024
    l_00c4f027:
        mov dword ptr [esi + 0eaa8h], edi // 00c4f027
        mov dword ptr [esi + 0eaa4h], edx // 00c4f02d
        pop edi // 00c4f033
        add esp, 017ch // 00c4f034
        ret 8 // 00c4f03a
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void prestep_kernel(){
    __asm {
        push esi // 00c4f040
        push edi // 00c4f041
        mov esi, ecx // 00c4f042
        mov dword ptr [esi + 0ea68h], eax // 00c4f044
        mov edi, dword ptr [eax + 4] // 00c4f04a
        add edi, edi // 00c4f04d
        add edi, edi // 00c4f04f
        add edi, edi // 00c4f051
        push dword ptr [esp+16] // Borrowed context, original frame offsets retained.
        call rows_size_kernel // 00c4f053
        fld dword ptr [esp + 0ch] // 00c4f058
        push ecx // 00c4f05c
        fstp dword ptr [esp] // 00c4f05d
        push dword ptr [esp+20] // Borrowed context, original frame offsets retained.
        call build_rows_shim // 00c4f060
        mov edi, dword ptr [esi + 4] // 00c4f065
        mov dword ptr [esi + 0eaa0h], edi // 00c4f068
        cmp edi, dword ptr [esi + 0ea70h] // 00c4f06e
        jle l_00c4f0b0 // 00c4f074
        mov eax, dword ptr [esi + 0ea6ch] // 00c4f076
        test eax, eax // 00c4f07c
        je l_00c4f089 // 00c4f07e
        push eax // 00c4f080
        push dword ptr [esp+20] // Borrowed context, original frame offsets retained.
        call bridge_00c4f081 // 00c4f081
        add esp, 4 // 00c4f086
    l_00c4f089:
        xor ecx, ecx // 00c4f089
        mov eax, edi // 00c4f08b
        mov edx, 030h // 00c4f08d
        mul edx // 00c4f092
        seto cl // 00c4f094
        neg ecx // 00c4f097
        or ecx, eax // 00c4f099
        push ecx // 00c4f09b
        push dword ptr [esp+20] // Borrowed context, original frame offsets retained.
        call bridge_00c4f09c // 00c4f09c
        add esp, 4 // 00c4f0a1
        mov dword ptr [esi + 0ea6ch], eax // 00c4f0a4
        mov dword ptr [esi + 0ea70h], edi // 00c4f0aa
    l_00c4f0b0:
        xor edx, edx // 00c4f0b0
        cmp dword ptr [esi + 0ea70h], edx // 00c4f0b2
        jle l_00c4f133 // 00c4f0b8
        xorps xmm0, xmm0 // 00c4f0ba
        xor ecx, ecx // 00c4f0bd
        nop  // 00c4f0bf
    l_00c4f0c0:
        mov eax, dword ptr [esi + 0ea6ch] // 00c4f0c0
        movss dword ptr [eax + ecx + 8], xmm0 // 00c4f0c6
        movss dword ptr [eax + ecx + 4], xmm0 // 00c4f0cc
        movss dword ptr [eax + ecx], xmm0 // 00c4f0d2
        add eax, ecx // 00c4f0d7
        mov eax, dword ptr [esi + 0ea6ch] // 00c4f0d9
        movss dword ptr [ecx + eax + 014h], xmm0 // 00c4f0df
        movss dword ptr [ecx + eax + 010h], xmm0 // 00c4f0e5
        movss dword ptr [ecx + eax + 0ch], xmm0 // 00c4f0eb
        lea eax, [ecx + eax + 0ch] // 00c4f0f1
        mov eax, dword ptr [esi + 0ea6ch] // 00c4f0f5
        lea eax, [ecx + eax + 024h] // 00c4f0fb
        movss dword ptr [eax + 8], xmm0 // 00c4f0ff
        movss dword ptr [eax + 4], xmm0 // 00c4f104
        movss dword ptr [eax], xmm0 // 00c4f109
        mov eax, dword ptr [esi + 0ea6ch] // 00c4f10d
        lea eax, [ecx + eax + 018h] // 00c4f113
        add edx, 1 // 00c4f117
        movss dword ptr [eax + 8], xmm0 // 00c4f11a
        movss dword ptr [eax + 4], xmm0 // 00c4f11f
        movss dword ptr [eax], xmm0 // 00c4f124
        add ecx, 030h // 00c4f128
        cmp edx, dword ptr [esi + 0ea70h] // 00c4f12b
        jl l_00c4f0c0 // 00c4f131
    l_00c4f133:
        mov eax, esi // 00c4f133
        call warm_kernel // 00c4f135
        pop edi // 00c4f13a
        pop esi // 00c4f13b
        ret 8 // 00c4f13c
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void abs_kernel(){
    __asm {
        push ebp // 00401170
        mov ebp, esp // 00401171
        and esp, 0fffffff8h // 00401173
        sub esp, 8 // 00401176
        fld dword ptr [ebp + 8] // 00401179
        fabs  // 0040117c
        fstp dword ptr [esp + 4] // 0040117e
        fld dword ptr [esp + 4] // 00401182
        mov esp, ebp // 00401186
        pop ebp // 00401188
        ret 4 // 00401189
    }
}
} // namespace
void* NativeDynSolverMode0Calls::allocate_00bf55be(U,U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void NativeDynSolverMode0Calls::free_00bf65ac(U,void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeDynSolverMode0Calls::free_00bf6989(U,void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void execute_native_dyn_solver_mode0_00403720(void* task,const AvoidZoneDynHullMemory& m,NativeDynSolverMode0Calls& calls,const CameraAxesCrtAccess& crt){
    Context context{crt,&m,&calls};Context* c=&context;
    __asm {push c}
    __asm {mov ecx,task}
    __asm {call task_kernel}
}
NativeDynSolverMode0Runtime::NativeDynSolverMode0Runtime(const AvoidZoneDynHullMemory& m,NativeDynSolverMode0Calls& c,const CameraAxesCrtAccess& crt) noexcept:methods_{reinterpret_cast<std::uintptr_t>(&run)},memory_(m),calls_(&c),crt_(crt){static_assert(std::is_standard_layout_v<NativeDynSolverMode0Runtime>);}
void __fastcall NativeDynSolverMode0Runtime::run(void* task,void*){auto* self=*static_cast<NativeDynSolverMode0Runtime**>(task);execute_native_dyn_solver_mode0_00403720(task,self->memory_,*self->calls_,self->crt_);}
} // namespace bsp
