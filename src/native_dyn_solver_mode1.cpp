#include "bsp/native_dyn_solver_mode1.hpp"
#include <type_traits>
#include <intrin.h>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mode1 solver requires MSVC Win32 x87/SSE assembly.
#endif
extern "C" void _chkstk();
namespace bsp {
namespace {
using U=std::uint32_t;
struct Context {CameraAxesCrtAccess crt;const AvoidZoneDynHullMemory* memory;NativeDynSolverMode1Calls* calls;void* const* profile_slot;};
static_assert(std::is_standard_layout_v<Context> && offsetof(Context,crt)==0 && offsetof(Context,profile_slot)==16);
void* __cdecl allocate_bridge(U site,Context* c,U size){return c->calls->allocate_00bf55be(site,size,*c->memory);}
void __cdecl row_free_bridge(U site,Context* c,void* p){c->calls->free_00bf65ac(site,p,*c->memory);}
void __cdecl velocity_free_bridge(U site,Context* c,void* p){c->calls->free_00bf6989(site,p,*c->memory);}
void* __cdecl append_bridge(Context* c,DynProfileNodeStorage* p,const char* name,U id){return dyn_profile_node_append_child_00c50390(*p,name,id,*c->memory);}
std::uint64_t __cdecl timestamp_bridge(U site,Context* c) noexcept {return c->calls->read_timestamp(site);}
void task_kernel();
void rows_size_kernel();
void store_kernel();
void writeback_kernel();
void friction_kernel();
void normal_kernel();
void warm_kernel();
void build_rows_kernel();
void solve_kernel();
void prestep_kernel();
void abs_kernel();
const char name_00d79ef4[]={83,111,108,118,101,114,80,114,101,83,116,101,112,0};
const char name_00d79f04[]={83,111,108,118,101,67,111,110,115,116,114,97,105,110,116,115,0};
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
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call prestep_kernel
        ret 0ch
    }
}
__declspec(naked) void build_rows_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call build_rows_kernel
        ret 0ch
    }
}
__declspec(naked) void append_shim(){
    __asm {
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push esi
        push dword ptr [esp+10h]
        call append_bridge
        add esp,10h
        ret 0ch
    }
}
__declspec(naked) void bridge_004038f1(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 0004038f1h
        call row_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00403905(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000403905h
        call velocity_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c3519e(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3519eh
        call row_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c351a7(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c351a7h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c745(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c745h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c777(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c777h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c7ee(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c7eeh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void bridge_00c5c83d(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5c83dh
        call velocity_free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c5c858(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5c858h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c8fa(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c8fah
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void task_kernel(){
    __asm {
        push ebp // 00403850
        mov ebp, esp // 00403851
        and esp, 0fffffff8h // 00403853
        push -1 // 00403856
        push 0 // 00403858
        mov eax, dword ptr fs:[0] // 0040385d
        push eax // 00403863
        // 00403864: native FS registration omitted; normal-return contract.
        push edx // 0040386b
        mov eax, 0eac8h // 0040386c
        call _chkstk // 00403871
        push ebx // 00403876
        push ebp // 00403877
        push esi // 00403878
        xor eax, eax // 00403879
        mov esi, ecx // 0040387b
        mov ecx, dword ptr [esi + 8] // 0040387d
        push edi // 00403880
        mov dword ptr [esp + 0ea7ch], eax // 00403881
        mov dword ptr [esp + 0ea80h], eax // 00403888
        mov dword ptr [esp + 010h], ecx // 0040388f
        mov dword ptr [esp + 0ea88h], eax // 00403893
        mov dword ptr [esp + 0ea84h], eax // 0040389a
        mov dword ptr [esp + 0eae4h], eax // 004038a1
        mov ebx, dword ptr [esi + 0ch] // 004038a8
        cmp ebx, dword ptr [esi + 010h] // 004038ab
        jg l_0040390d // 004038ae
        lea ebp, [ebx + ebx*2] // 004038b0
        add ebp, ebp // 004038b3
        add ebp, ebp // 004038b5
    l_004038b7:
        mov eax, dword ptr [esi + 8] // 004038b7
        fld dword ptr [esi + 014h] // 004038ba
        push ecx // 004038bd
        mov ecx, dword ptr [eax + 044ch] // 004038be
        fstp dword ptr [esp] // 004038c4
        add ecx, ebp // 004038c7
        push ecx // 004038c9
        lea edi, [esp + 018h] // 004038ca
        push dword ptr [esp+60128] // Borrowed context; original frame offsets retained.
        call prestep_shim // 004038ce
        mov eax, edi // 004038d3
        push dword ptr [esp+60120] // Borrowed context; original frame offsets retained.
        call solve_kernel // 004038d5
        add ebx, 1 // 004038da
        add ebp, 0ch // 004038dd
        cmp ebx, dword ptr [esi + 010h] // 004038e0
        jle l_004038b7 // 004038e3
        mov eax, dword ptr [esp + 0ea84h] // 004038e5
        test eax, eax // 004038ec
        je l_004038f9 // 004038ee
        push eax // 004038f0
        push dword ptr [esp+60124] // Borrowed context; original frame offsets retained.
        call bridge_004038f1 // 004038f1
        add esp, 4 // 004038f6
    l_004038f9:
        mov eax, dword ptr [esp + 0ea7ch] // 004038f9
        test eax, eax // 00403900
        je l_0040390d // 00403902
        push eax // 00403904
        push dword ptr [esp+60124] // Borrowed context; original frame offsets retained.
        call bridge_00403905 // 00403905
        add esp, 4 // 0040390a
    l_0040390d:
        mov ecx, dword ptr [esp + 0eadch] // 0040390d
        pop edi // 00403914
        // 00403915: native FS registration omitted; normal-return contract.
        pop esi // 0040391c
        pop ebp // 0040391d
        pop ebx // 0040391e
        mov esp, ebp // 0040391f
        pop ebp // 00403921
        ret  // 00403922
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void rows_size_kernel(){
    __asm {
        mov eax, dword ptr [esi + 0ea68h] // 00c35160
        mov eax, dword ptr [eax + 4] // 00c35166
        add eax, 3 // 00c35169
        cdq  // 00c3516c
        push ebx // 00c3516d
        and edx, 3 // 00c3516e
        push edi // 00c35171
        add eax, edx // 00c35172
        mov edi, eax // 00c35174
        sar edi, 2 // 00c35176
        add edi, edi // 00c35179
        add edi, edi // 00c3517b
        mov ebx, edi // 00c3517d
        imul ebx, ebx, 0394h // 00c3517f
        cmp dword ptr [esi + 0ea78h], ebx // 00c35185
        jge l_00c351b5 // 00c3518b
        mov eax, dword ptr [esi + 0ea74h] // 00c3518d
        test eax, eax // 00c35193
        mov dword ptr [esi + 0ea78h], ebx // 00c35195
        je l_00c351a6 // 00c3519b
        push eax // 00c3519d
        push dword ptr [esp+16] // Borrowed context; original frame offsets retained.
        call bridge_00c3519e // 00c3519e
        add esp, 4 // 00c351a3
    l_00c351a6:
        push ebx // 00c351a6
        push dword ptr [esp+16] // Borrowed context; original frame offsets retained.
        call bridge_00c351a7 // 00c351a7
        add esp, 4 // 00c351ac
        mov dword ptr [esi + 0ea74h], eax // 00c351af
    l_00c351b5:
        mov ecx, dword ptr [esi + 0ea74h] // 00c351b5
        mov dword ptr [esi + 0ea80h], ecx // 00c351bb
        lea eax, [edi + edi*2] // 00c351c1
        shl eax, 6 // 00c351c4
        add ecx, eax // 00c351c7
        mov dword ptr [esi + 0ea84h], ecx // 00c351c9
        add ecx, eax // 00c351cf
        mov dword ptr [esi + 0ea9ch], ecx // 00c351d1
        add ecx, eax // 00c351d7
        mov dword ptr [esi + 0eaa0h], ecx // 00c351d9
        add ecx, eax // 00c351df
        mov dword ptr [esi + 0ea8ch], ecx // 00c351e1
        mov eax, edi // 00c351e7
        shl eax, 4 // 00c351e9
        add ecx, eax // 00c351ec
        mov dword ptr [esi + 0eaa8h], ecx // 00c351ee
        add ecx, eax // 00c351f4
        mov dword ptr [esi + 0ea90h], ecx // 00c351f6
        add ecx, eax // 00c351fc
        mov dword ptr [esi + 0ea94h], ecx // 00c351fe
        add ecx, eax // 00c35204
        mov dword ptr [esi + 0eaach], ecx // 00c35206
        add ecx, eax // 00c3520c
        mov dword ptr [esi + 0ea88h], ecx // 00c3520e
        lea ecx, [ecx + edi*4] // 00c35214
        mov dword ptr [esi + 0eaa4h], ecx // 00c35217
        lea ecx, [ecx + edi*4] // 00c3521d
        mov dword ptr [esi + 0ea7ch], ecx // 00c35220
        lea ecx, [ecx + edi*4] // 00c35226
        mov dword ptr [esi + 0ea98h], ecx // 00c35229
        lea ecx, [ecx + edi*4] // 00c3522f
        mov dword ptr [esi + 0eab0h], ecx // 00c35232
        lea ecx, [ecx + edi*4] // 00c35238
        mov dword ptr [esi + 0eab4h], ecx // 00c3523b
        add ecx, eax // 00c35241
        mov dword ptr [esi + 0eab8h], ecx // 00c35243
        add ecx, eax // 00c35249
        pop edi // 00c3524b
        mov dword ptr [esi + 0eabch], ecx // 00c3524c
        pop ebx // 00c35252
        ret 4 // 00c35253
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void store_kernel(){
    __asm {
        sub esp, 8 // 00c350c0
        push edi // 00c350c3
        mov edi, dword ptr [esp + 010h] // 00c350c4
        mov eax, dword ptr [edi + 0ea68h] // 00c350c8
        mov ecx, dword ptr [eax + 4] // 00c350ce
        test ecx, ecx // 00c350d1
        jle l_00c3513d // 00c350d3
        mov eax, dword ptr [eax] // 00c350d5
        push ebx // 00c350d7
        push ebp // 00c350d8
        push esi // 00c350d9
        xor ebp, ebp // 00c350da
        mov dword ptr [esp + 010h], eax // 00c350dc
        mov dword ptr [esp + 014h], ecx // 00c350e0
    l_00c350e4:
        mov esi, dword ptr [eax] // 00c350e4
        xor edx, edx // 00c350e6
        cmp dword ptr [esi + 0c8h], edx // 00c350e8
        jle l_00c35125 // 00c350ee
        mov ebx, dword ptr [edi + 0eab8h] // 00c350f0
        mov edi, dword ptr [edi + 0eab4h] // 00c350f6
        lea eax, [ebx + ebp] // 00c350fc
        lea ecx, [esi + 030h] // 00c350ff
        sub edi, ebx // 00c35102
    l_00c35104:
        fld dword ptr [edi + eax] // 00c35104
        add edx, 1 // 00c35107
        fstp dword ptr [ecx - 4] // 00c3510a
        add eax, 4 // 00c3510d
        fld dword ptr [eax - 4] // 00c35110
        add ecx, 030h // 00c35113
        fstp dword ptr [ecx - 030h] // 00c35116
        cmp edx, dword ptr [esi + 0c8h] // 00c35119
        jl l_00c35104 // 00c3511f
        mov edi, dword ptr [esp + 01ch] // 00c35121
    l_00c35125:
        mov eax, dword ptr [esp + 010h] // 00c35125
        add eax, 4 // 00c35129
        add ebp, 010h // 00c3512c
        sub dword ptr [esp + 014h], 1 // 00c3512f
        mov dword ptr [esp + 010h], eax // 00c35134
        jne l_00c350e4 // 00c35138
        pop esi // 00c3513a
        pop ebp // 00c3513b
        pop ebx // 00c3513c
    l_00c3513d:
        pop edi // 00c3513d
        add esp, 8 // 00c3513e
        ret 4 // 00c35141
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void writeback_kernel(){
    __asm {
        push ebp // 00c37c40
        mov ebp, 1 // 00c37c41
        cmp dword ptr [edx + 0eac0h], ebp // 00c37c46
        jle l_00c37d27 // 00c37c4c
        push ebx // 00c37c52
        push esi // 00c37c53
        push edi // 00c37c54
        mov esi, 030h // 00c37c55
        lea ebx, [edx + 0ch] // 00c37c5a
        _emit 08dh // 00c37c5d
        _emit 049h
        _emit 000h
    l_00c37c60:
        mov ecx, dword ptr [edx + 0ea6ch] // 00c37c60
        fld dword ptr [ecx + esi] // 00c37c66
        mov edi, dword ptr [ebx] // 00c37c69
        mov eax, dword ptr [edi + 4] // 00c37c6b
        fadd dword ptr [eax] // 00c37c6e
        add ecx, esi // 00c37c70
        add ebp, 1 // 00c37c72
        add ebx, 4 // 00c37c75
        fstp dword ptr [eax] // 00c37c78
        fld dword ptr [ecx + 4] // 00c37c7a
        fadd dword ptr [eax + 4] // 00c37c7d
        fstp dword ptr [eax + 4] // 00c37c80
        fld dword ptr [ecx + 8] // 00c37c83
        fadd dword ptr [eax + 8] // 00c37c86
        fstp dword ptr [eax + 8] // 00c37c89
        mov eax, dword ptr [edx + 0ea6ch] // 00c37c8c
        lea ecx, [esi + eax + 0ch] // 00c37c92
        mov eax, dword ptr [edi + 4] // 00c37c96
        fld dword ptr [eax + 0ch] // 00c37c99
        add eax, 0ch // 00c37c9c
        fadd dword ptr [ecx] // 00c37c9f
        fstp dword ptr [eax] // 00c37ca1
        fld dword ptr [ecx + 4] // 00c37ca3
        fadd dword ptr [eax + 4] // 00c37ca6
        fstp dword ptr [eax + 4] // 00c37ca9
        fld dword ptr [ecx + 8] // 00c37cac
        fadd dword ptr [eax + 8] // 00c37caf
        fstp dword ptr [eax + 8] // 00c37cb2
        mov dword ptr [edi + 058h], 0ffffffffh // 00c37cb5
        mov ecx, dword ptr [edx + 0ea6ch] // 00c37cbc
        mov edi, dword ptr [ebx - 4] // 00c37cc2
        mov eax, dword ptr [edi + 4] // 00c37cc5
        fld dword ptr [eax + 020h] // 00c37cc8
        add eax, 020h // 00c37ccb
        fadd dword ptr [esi + ecx + 018h] // 00c37cce
        lea ecx, [esi + ecx + 018h] // 00c37cd2
        fstp dword ptr [eax] // 00c37cd6
        fld dword ptr [ecx + 4] // 00c37cd8
        fadd dword ptr [eax + 4] // 00c37cdb
        fstp dword ptr [eax + 4] // 00c37cde
        fld dword ptr [ecx + 8] // 00c37ce1
        fadd dword ptr [eax + 8] // 00c37ce4
        fstp dword ptr [eax + 8] // 00c37ce7
        mov eax, dword ptr [edx + 0ea6ch] // 00c37cea
        fld dword ptr [esi + eax + 024h] // 00c37cf0
        lea ecx, [esi + eax + 024h] // 00c37cf4
        mov eax, dword ptr [edi + 4] // 00c37cf8
        fadd dword ptr [eax + 02ch] // 00c37cfb
        add eax, 02ch // 00c37cfe
        add esi, 030h // 00c37d01
        fstp dword ptr [eax] // 00c37d04
        fld dword ptr [ecx + 4] // 00c37d06
        fadd dword ptr [eax + 4] // 00c37d09
        fstp dword ptr [eax + 4] // 00c37d0c
        fld dword ptr [ecx + 8] // 00c37d0f
        fadd dword ptr [eax + 8] // 00c37d12
        fstp dword ptr [eax + 8] // 00c37d15
        cmp ebp, dword ptr [edx + 0eac0h] // 00c37d18
        jl l_00c37c60 // 00c37d1e
        pop edi // 00c37d24
        pop esi // 00c37d25
        pop ebx // 00c37d26
    l_00c37d27:
        pop ebp // 00c37d27
        ret  // 00c37d28
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void friction_kernel(){
    __asm {
        sub esp, 054h // 00c42ed0
        push esi // 00c42ed3
        push edi // 00c42ed4
        xor esi, esi // 00c42ed5
        xor edi, edi // 00c42ed7
        cmp dword ptr [ebx + 0eac4h], esi // 00c42ed9
        mov dword ptr [esp + 010h], edi // 00c42edf
        jle l_00c431bd // 00c42ee3
        push ebp // 00c42ee9
        mov dword ptr [esp + 018h], esi // 00c42eea
        mov dword ptr [esp + 01ch], esi // 00c42eee
        jmp l_00c42f04 // 00c42ef2
        jmp l_00c42f00 // 00c42ef4
        _emit 08dh // 00c42ef6
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 08dh // 00c42efd
        _emit 049h
        _emit 000h
    l_00c42f00:
        mov esi, dword ptr [esp + 018h] // 00c42f00
    l_00c42f04:
        mov eax, dword ptr [ebx + 0ea98h] // 00c42f04
        movsx ecx, word ptr [eax + edi*4] // 00c42f0a
        movsx edx, word ptr [eax + edi*4 + 2] // 00c42f0e
        mov ebp, dword ptr [ebx + 0eaa0h] // 00c42f13
        lea eax, [eax + edi*4] // 00c42f19
        mov eax, dword ptr [ebx + 0eaa4h] // 00c42f1c
        mov eax, dword ptr [eax + edi*4] // 00c42f22
        mov dword ptr [esp + 0ch], eax // 00c42f25
        mov eax, dword ptr [ebx + 0ea9ch] // 00c42f29
        add eax, esi // 00c42f2f
        add ebp, esi // 00c42f31
        cmp dword ptr [esp + 0ch], 0 // 00c42f33
        mov esi, dword ptr [ebx + 0ea6ch] // 00c42f38
        jle l_00c4319c // 00c42f3e
        lea ecx, [ecx + ecx*2] // 00c42f44
        lea edx, [edx + edx*2] // 00c42f47
        shl ecx, 4 // 00c42f4a
        shl edx, 4 // 00c42f4d
        add ecx, esi // 00c42f50
        add edx, esi // 00c42f52
        mov esi, dword ptr [esp + 01ch] // 00c42f54
        mov dword ptr [esp + 010h], esi // 00c42f58
        lea esi, [ebp + 01ch] // 00c42f5c
        sub ebp, eax // 00c42f5f
        lea edi, [eax + 014h] // 00c42f61
        mov eax, dword ptr [esp + 0ch] // 00c42f64
        mov dword ptr [esp + 028h], eax // 00c42f68
        mov eax, dword ptr [esp + 010h] // 00c42f6c
        mov dword ptr [esp + 02ch], ebp // 00c42f70
        jmp l_00c42f80 // 00c42f74
        _emit 08dh // 00c42f76
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 08dh // 00c42f7d
        _emit 049h
        _emit 000h
    l_00c42f80:
        mov ebp, dword ptr [ebx + 0eaach] // 00c42f80
        fld dword ptr [eax + ebp] // 00c42f86
        mov ebp, dword ptr [ebx + 0eaa8h] // 00c42f89
        fld dword ptr [edi - 010h] // 00c42f8f
        fmul dword ptr [ecx + 4] // 00c42f92
        fld dword ptr [edi - 014h] // 00c42f95
        fmul dword ptr [ecx] // 00c42f98
        faddp st(1), st(0) // 00c42f9a
        fld dword ptr [edi - 0ch] // 00c42f9c
        fmul dword ptr [ecx + 8] // 00c42f9f
        faddp st(1), st(0) // 00c42fa2
        fstp dword ptr [esp + 024h] // 00c42fa4
        fsub dword ptr [esp + 024h] // 00c42fa8
        fld dword ptr [edi - 4] // 00c42fac
        fmul dword ptr [ecx + 010h] // 00c42faf
        fld dword ptr [edi - 8] // 00c42fb2
        fmul dword ptr [ecx + 0ch] // 00c42fb5
        faddp st(1), st(0) // 00c42fb8
        fld dword ptr [ecx + 014h] // 00c42fba
        fmul dword ptr [edi] // 00c42fbd
        faddp st(1), st(0) // 00c42fbf
        fstp dword ptr [esp + 024h] // 00c42fc1
        fsub dword ptr [esp + 024h] // 00c42fc5
        fld dword ptr [edi + 8] // 00c42fc9
        fmul dword ptr [edx + 4] // 00c42fcc
        fld dword ptr [edi + 4] // 00c42fcf
        fmul dword ptr [edx] // 00c42fd2
        faddp st(1), st(0) // 00c42fd4
        fld dword ptr [edi + 0ch] // 00c42fd6
        fmul dword ptr [edx + 8] // 00c42fd9
        faddp st(1), st(0) // 00c42fdc
        fstp dword ptr [esp + 024h] // 00c42fde
        fsub dword ptr [esp + 024h] // 00c42fe2
        fld dword ptr [edi + 014h] // 00c42fe6
        fmul dword ptr [edx + 010h] // 00c42fe9
        fld dword ptr [edi + 010h] // 00c42fec
        fmul dword ptr [edx + 0ch] // 00c42fef
        faddp st(1), st(0) // 00c42ff2
        fld dword ptr [edi + 018h] // 00c42ff4
        fmul dword ptr [edx + 014h] // 00c42ff7
        faddp st(1), st(0) // 00c42ffa
        fstp dword ptr [esp + 024h] // 00c42ffc
        fsub dword ptr [esp + 024h] // 00c43000
        fstp dword ptr [esp + 024h] // 00c43004
        fld dword ptr [esp + 024h] // 00c43008
        fmul dword ptr [eax + ebp] // 00c4300c
        mov ebp, dword ptr [ebx + 0eabch] // 00c4300f
        fstp dword ptr [esp + 024h] // 00c43015
        fld dword ptr [esp + 024h] // 00c43019
        fadd dword ptr [eax + ebp] // 00c4301d
        mov ebp, dword ptr [ebx + 0eab4h] // 00c43020
        fstp dword ptr [esp + 0ch] // 00c43026
        fld dword ptr [eax + ebp] // 00c4302a
        mov eax, dword ptr [ebx + 0eab0h] // 00c4302d
        mov ebp, dword ptr [esp + 014h] // 00c43033
        fmul dword ptr [eax + ebp*4] // 00c43037
        fstp dword ptr [esp + 024h] // 00c4303a
        fld dword ptr [esp + 024h] // 00c4303e
        fld st(0) // 00c43042
        fchs  // 00c43044
        fstp dword ptr [esp + 020h] // 00c43046
        fld dword ptr [esp + 0ch] // 00c4304a
        fld dword ptr [esp + 020h] // 00c4304e
        fcomip st(0), st(1) // 00c43052
        jbe l_00c43062 // 00c43054
        movss xmm0, dword ptr [esp + 020h] // 00c43056
        fstp st(1) // 00c4305c
        fstp st(0) // 00c4305e
        jmp l_00c4306e // 00c43060
    l_00c43062:
        fcomip st(0), st(1) // 00c43062
        fstp st(0) // 00c43064
        jbe l_00c43074 // 00c43066
        movss xmm0, dword ptr [esp + 024h] // 00c43068
    l_00c4306e:
        movss dword ptr [esp + 0ch], xmm0 // 00c4306e
    l_00c43074:
        mov ebp, dword ptr [ebx + 0eabch] // 00c43074
        fld dword ptr [esp + 0ch] // 00c4307a
        mov eax, dword ptr [esp + 010h] // 00c4307e
        fsub dword ptr [ebp + eax] // 00c43082
        add ebp, eax // 00c43086
        movss xmm0, dword ptr [esp + 0ch] // 00c43088
        movss dword ptr [ebp], xmm0 // 00c4308e
        fstp dword ptr [esp + 024h] // 00c43093
        mov ebp, dword ptr [esp + 02ch] // 00c43097
        fld dword ptr [esi - 01ch] // 00c4309b
        fld dword ptr [esp + 024h] // 00c4309e
        fld st(0) // 00c430a2
        fmulp st(2), st(0) // 00c430a4
        fxch st(1) // 00c430a6
        fstp dword ptr [esp + 030h] // 00c430a8
        fld dword ptr [esi - 018h] // 00c430ac
        fmul st(0), st(1) // 00c430af
        fstp dword ptr [esp + 034h] // 00c430b1
        fld dword ptr [esi - 014h] // 00c430b5
        fmul st(0), st(1) // 00c430b8
        fstp dword ptr [esp + 038h] // 00c430ba
        fld dword ptr [esp + 030h] // 00c430be
        fadd dword ptr [ecx] // 00c430c2
        fstp dword ptr [ecx] // 00c430c4
        fld dword ptr [ecx + 4] // 00c430c6
        fadd dword ptr [esp + 034h] // 00c430c9
        fstp dword ptr [ecx + 4] // 00c430cd
        fld dword ptr [esp + 038h] // 00c430d0
        fadd dword ptr [ecx + 8] // 00c430d4
        fstp dword ptr [ecx + 8] // 00c430d7
        fld dword ptr [esi - 010h] // 00c430da
        fmul st(0), st(1) // 00c430dd
        fstp dword ptr [esp + 03ch] // 00c430df
        fld dword ptr [esi - 0ch] // 00c430e3
        fmul st(0), st(1) // 00c430e6
        fstp dword ptr [esp + 040h] // 00c430e8
        fld dword ptr [edi + ebp] // 00c430ec
        fmul st(0), st(1) // 00c430ef
        fstp dword ptr [esp + 044h] // 00c430f1
        fld dword ptr [ecx + 0ch] // 00c430f5
        fadd dword ptr [esp + 03ch] // 00c430f8
        fstp dword ptr [ecx + 0ch] // 00c430fc
        fld dword ptr [ecx + 010h] // 00c430ff
        fadd dword ptr [esp + 040h] // 00c43102
        fstp dword ptr [ecx + 010h] // 00c43106
        fld dword ptr [ecx + 014h] // 00c43109
        fadd dword ptr [esp + 044h] // 00c4310c
        fstp dword ptr [ecx + 014h] // 00c43110
        fld dword ptr [esi - 4] // 00c43113
        fmul st(0), st(1) // 00c43116
        fstp dword ptr [esp + 048h] // 00c43118
        fld dword ptr [esi] // 00c4311c
        fmul st(0), st(1) // 00c4311e
        fstp dword ptr [esp + 04ch] // 00c43120
        fld dword ptr [esi + 4] // 00c43124
        fmul st(0), st(1) // 00c43127
        fstp dword ptr [esp + 050h] // 00c43129
        fld dword ptr [esp + 048h] // 00c4312d
        fadd dword ptr [edx] // 00c43131
        fstp dword ptr [edx] // 00c43133
        fld dword ptr [edx + 4] // 00c43135
        fadd dword ptr [esp + 04ch] // 00c43138
        fstp dword ptr [edx + 4] // 00c4313c
        fld dword ptr [esp + 050h] // 00c4313f
        fadd dword ptr [edx + 8] // 00c43143
        fstp dword ptr [edx + 8] // 00c43146
        fld dword ptr [esi + 8] // 00c43149
        fmul st(0), st(1) // 00c4314c
        fstp dword ptr [esp + 054h] // 00c4314e
        fld dword ptr [esi + 0ch] // 00c43152
        fmul st(0), st(1) // 00c43155
        fstp dword ptr [esp + 058h] // 00c43157
        fmul dword ptr [esi + 010h] // 00c4315b
        fstp dword ptr [esp + 05ch] // 00c4315e
        fld dword ptr [edx + 0ch] // 00c43162
        fadd dword ptr [esp + 054h] // 00c43165
        fstp dword ptr [edx + 0ch] // 00c43169
        fld dword ptr [edx + 010h] // 00c4316c
        fadd dword ptr [esp + 058h] // 00c4316f
        fstp dword ptr [edx + 010h] // 00c43173
        fld dword ptr [edx + 014h] // 00c43176
        fadd dword ptr [esp + 05ch] // 00c43179
        add eax, 4 // 00c4317d
        add edi, 030h // 00c43180
        add esi, 030h // 00c43183
        sub dword ptr [esp + 028h], 1 // 00c43186
        fstp dword ptr [edx + 014h] // 00c4318b
        mov dword ptr [esp + 010h], eax // 00c4318e
        jne l_00c42f80 // 00c43192
        mov edi, dword ptr [esp + 014h] // 00c43198
    l_00c4319c:
        add dword ptr [esp + 018h], 0c0h // 00c4319c
        add dword ptr [esp + 01ch], 010h // 00c431a4
        add edi, 1 // 00c431a9
        cmp edi, dword ptr [ebx + 0eac4h] // 00c431ac
        mov dword ptr [esp + 014h], edi // 00c431b2
        jl l_00c42f00 // 00c431b6
        pop ebp // 00c431bc
    l_00c431bd:
        pop edi // 00c431bd
        pop esi // 00c431be
        add esp, 054h // 00c431bf
        ret  // 00c431c2
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void normal_kernel(){
    __asm {
        sub esp, 0c8h // 00c431d0
        push ebp // 00c431d6
        mov ebp, dword ptr [esp + 0d0h] // 00c431d7
        push edi // 00c431de
        xor edi, edi // 00c431df
        cmp dword ptr [ebp + 0eac4h], edi // 00c431e1
        mov dword ptr [esp + 034h], edi // 00c431e7
        jle l_00c437c0 // 00c431eb
        xorps xmm0, xmm0 // 00c431f1
        push ebx // 00c431f4
        push esi // 00c431f5
        xor esi, esi // 00c431f6
        mov dword ptr [esp + 044h], esi // 00c431f8
        _emit 08dh // 00c431fc
        _emit 064h
        _emit 024h
        _emit 000h
    l_00c43200:
        mov eax, dword ptr [ebp + 0ea7ch] // 00c43200
        movsx ecx, word ptr [eax + edi*4] // 00c43206
        movsx edx, word ptr [eax + edi*4 + 2] // 00c4320a
        mov ebx, dword ptr [ebp + 0ea80h] // 00c4320f
        lea eax, [eax + edi*4] // 00c43215
        mov eax, dword ptr [ebp + 0ea88h] // 00c43218
        mov eax, dword ptr [eax + edi*4] // 00c4321e
        mov dword ptr [esp + 014h], eax // 00c43221
        mov eax, dword ptr [ebp + 0ea84h] // 00c43225
        add ebx, esi // 00c4322b
        add eax, esi // 00c4322d
        cmp dword ptr [esp + 014h], 0 // 00c4322f
        mov dword ptr [esp + 01ch], ebx // 00c43234
        jle l_00c4379d // 00c43238
        mov esi, dword ptr [ebp + 0ea6ch] // 00c4323e
        lea ecx, [ecx + ecx*2] // 00c43244
        lea edx, [edx + edx*2] // 00c43247
        shl ecx, 4 // 00c4324a
        shl edx, 4 // 00c4324d
        add ecx, esi // 00c43250
        add edx, esi // 00c43252
        lea esi, [eax + 01ch] // 00c43254
        shl edi, 4 // 00c43257
        add ebx, 014h // 00c4325a
        sub eax, dword ptr [esp + 01ch] // 00c4325d
        mov dword ptr [esp + 080h], eax // 00c43261
        mov eax, dword ptr [esp + 014h] // 00c43268
        mov dword ptr [esp + 01ch], eax // 00c4326c
    l_00c43270:
        mov eax, dword ptr [ebp + 0ea90h] // 00c43270
        fld dword ptr [edi + eax] // 00c43276
        movss xmm1, dword ptr [ebx - 010h] // 00c43279
        fld dword ptr [ecx + 4] // 00c4327e
        movss dword ptr [esp + 070h], xmm1 // 00c43281
        fld dword ptr [esp + 070h] // 00c43287
        movss xmm1, dword ptr [ebx - 014h] // 00c4328b
        fld st(0) // 00c43290
        movss dword ptr [esp + 060h], xmm1 // 00c43292
        fmulp st(2), st(0) // 00c43298
        movss xmm1, dword ptr [ebx - 0ch] // 00c4329a
        fld dword ptr [ecx] // 00c4329f
        movss dword ptr [esp + 02ch], xmm1 // 00c432a1
        fmul dword ptr [esp + 060h] // 00c432a7
        movss xmm1, dword ptr [ebx - 4] // 00c432ab
        movss dword ptr [esp + 034h], xmm1 // 00c432b0
        movss xmm1, dword ptr [ebx - 8] // 00c432b6
        faddp st(2), st(0) // 00c432bb
        movss dword ptr [esp + 028h], xmm1 // 00c432bd
        fld dword ptr [ecx + 8] // 00c432c3
        movss xmm1, dword ptr [ebx] // 00c432c6
        fmul dword ptr [esp + 02ch] // 00c432ca
        movss dword ptr [esp + 04ch], xmm1 // 00c432ce
        movss xmm1, dword ptr [ebx + 8] // 00c432d4
        movss dword ptr [esp + 030h], xmm1 // 00c432d9
        faddp st(2), st(0) // 00c432df
        movss xmm1, dword ptr [ebx + 4] // 00c432e1
        fxch st(1) // 00c432e6
        movss dword ptr [esp + 054h], xmm1 // 00c432e8
        movss xmm1, dword ptr [ebx + 0ch] // 00c432ee
        fstp dword ptr [esp + 010h] // 00c432f3
        movss dword ptr [esp + 038h], xmm1 // 00c432f7
        fld dword ptr [esp + 010h] // 00c432fd
        movss xmm1, dword ptr [ebx + 014h] // 00c43301
        fsubp st(2), st(0) // 00c43306
        mov eax, dword ptr [ebp + 0ea8ch] // 00c43308
        fld dword ptr [ecx + 010h] // 00c4330e
        movss dword ptr [esp + 05ch], xmm1 // 00c43311
        fmul dword ptr [esp + 034h] // 00c43317
        movss xmm1, dword ptr [ebx + 010h] // 00c4331b
        fld dword ptr [ecx + 0ch] // 00c43320
        movss dword ptr [esp + 040h], xmm1 // 00c43323
        fmul dword ptr [esp + 028h] // 00c43329
        movss xmm1, dword ptr [ebx + 018h] // 00c4332d
        movss dword ptr [esp + 064h], xmm1 // 00c43332
        faddp st(1), st(0) // 00c43338
        fld dword ptr [ecx + 014h] // 00c4333a
        fmul dword ptr [esp + 04ch] // 00c4333d
        faddp st(1), st(0) // 00c43341
        fstp dword ptr [esp + 010h] // 00c43343
        fld dword ptr [esp + 010h] // 00c43347
        fsubp st(2), st(0) // 00c4334b
        fld dword ptr [edx + 4] // 00c4334d
        fmul dword ptr [esp + 030h] // 00c43350
        fld dword ptr [esp + 054h] // 00c43354
        fmul dword ptr [edx] // 00c43358
        faddp st(1), st(0) // 00c4335a
        fld dword ptr [edx + 8] // 00c4335c
        fmul dword ptr [esp + 038h] // 00c4335f
        faddp st(1), st(0) // 00c43363
        fstp dword ptr [esp + 010h] // 00c43365
        fld dword ptr [esp + 010h] // 00c43369
        fsubp st(2), st(0) // 00c4336d
        fld dword ptr [edx + 010h] // 00c4336f
        fmul dword ptr [esp + 05ch] // 00c43372
        fld dword ptr [edx + 0ch] // 00c43376
        fmul dword ptr [esp + 040h] // 00c43379
        faddp st(1), st(0) // 00c4337d
        fld dword ptr [edx + 014h] // 00c4337f
        fmul dword ptr [esp + 064h] // 00c43382
        faddp st(1), st(0) // 00c43386
        fstp dword ptr [esp + 010h] // 00c43388
        fld dword ptr [esp + 010h] // 00c4338c
        fsubp st(2), st(0) // 00c43390
        fxch st(1) // 00c43392
        fstp dword ptr [esp + 010h] // 00c43394
        fld dword ptr [esp + 010h] // 00c43398
        fmul dword ptr [eax + edi] // 00c4339c
        mov eax, dword ptr [ebp + 0eab4h] // 00c4339f
        fstp dword ptr [esp + 010h] // 00c433a5
        fld dword ptr [esp + 010h] // 00c433a9
        fadd dword ptr [eax + edi] // 00c433ad
        fstp dword ptr [esp + 018h] // 00c433b0
        fld dword ptr [esp + 018h] // 00c433b4
        fldz  // 00c433b8
        fcomip st(0), st(1) // 00c433ba
        fstp st(0) // 00c433bc
        jbe l_00c433c6 // 00c433be
        movss dword ptr [esp + 018h], xmm0 // 00c433c0
    l_00c433c6:
        fld dword ptr [esp + 018h] // 00c433c6
        add eax, edi // 00c433ca
        fsub dword ptr [eax] // 00c433cc
        movss xmm1, dword ptr [esp + 018h] // 00c433ce
        movss dword ptr [eax], xmm1 // 00c433d4
        mov eax, dword ptr [esp + 080h] // 00c433d8
        fstp dword ptr [esp + 014h] // 00c433df
        fld dword ptr [esi - 01ch] // 00c433e3
        fstp dword ptr [esp + 010h] // 00c433e6
        fld dword ptr [esp + 010h] // 00c433ea
        fld st(0) // 00c433ee
        fld dword ptr [esp + 014h] // 00c433f0
        fld st(0) // 00c433f4
        fmulp st(2), st(0) // 00c433f6
        fxch st(1) // 00c433f8
        fstp dword ptr [esp + 0a8h] // 00c433fa
        fld dword ptr [esi - 018h] // 00c43401
        fstp dword ptr [esp + 010h] // 00c43404
        fld dword ptr [esp + 010h] // 00c43408
        fld st(0) // 00c4340c
        fmul st(0), st(2) // 00c4340e
        fstp dword ptr [esp + 0ach] // 00c43410
        fld dword ptr [esi - 014h] // 00c43417
        fstp dword ptr [esp + 010h] // 00c4341a
        fld dword ptr [esp + 010h] // 00c4341e
        fld st(0) // 00c43422
        fmul st(0), st(3) // 00c43424
        fstp dword ptr [esp + 0b0h] // 00c43426
        fld dword ptr [ecx] // 00c4342d
        fadd dword ptr [esp + 0a8h] // 00c4342f
        fstp dword ptr [ecx] // 00c43436
        fld dword ptr [esp + 0ach] // 00c43438
        fadd dword ptr [ecx + 4] // 00c4343f
        fstp dword ptr [ecx + 4] // 00c43442
        fld dword ptr [esp + 0b0h] // 00c43445
        fadd dword ptr [ecx + 8] // 00c4344c
        fstp dword ptr [ecx + 8] // 00c4344f
        fld dword ptr [esi - 010h] // 00c43452
        fstp dword ptr [esp + 010h] // 00c43455
        fld dword ptr [esp + 010h] // 00c43459
        fld st(0) // 00c4345d
        fmul st(0), st(4) // 00c4345f
        fstp dword ptr [esp + 090h] // 00c43461
        fld dword ptr [esi - 0ch] // 00c43468
        fstp dword ptr [esp + 048h] // 00c4346b
        fld dword ptr [esp + 048h] // 00c4346f
        fmul st(0), st(4) // 00c43473
        fstp dword ptr [esp + 094h] // 00c43475
        fld dword ptr [ebx + eax] // 00c4347c
        fstp dword ptr [esp + 06ch] // 00c4347f
        fld dword ptr [esp + 06ch] // 00c43483
        fmul st(0), st(4) // 00c43487
        fstp dword ptr [esp + 098h] // 00c43489
        fld dword ptr [esp + 090h] // 00c43490
        fadd dword ptr [ecx + 0ch] // 00c43497
        fstp dword ptr [ecx + 0ch] // 00c4349a
        fld dword ptr [ecx + 010h] // 00c4349d
        fadd dword ptr [esp + 094h] // 00c434a0
        fstp dword ptr [ecx + 010h] // 00c434a7
        fld dword ptr [ecx + 014h] // 00c434aa
        fadd dword ptr [esp + 098h] // 00c434ad
        fstp dword ptr [ecx + 014h] // 00c434b4
        fld dword ptr [esi - 4] // 00c434b7
        fstp dword ptr [esp + 050h] // 00c434ba
        fld dword ptr [esp + 050h] // 00c434be
        fmul st(0), st(4) // 00c434c2
        fstp dword ptr [esp + 0c0h] // 00c434c4
        fld dword ptr [esi] // 00c434cb
        fstp dword ptr [esp + 020h] // 00c434cd
        fld dword ptr [esp + 020h] // 00c434d1
        fmul st(0), st(4) // 00c434d5
        fstp dword ptr [esp + 0c4h] // 00c434d7
        fld dword ptr [esi + 4] // 00c434de
        fstp dword ptr [esp + 058h] // 00c434e1
        fld dword ptr [esp + 058h] // 00c434e5
        fmul st(0), st(4) // 00c434e9
        fstp dword ptr [esp + 0c8h] // 00c434eb
        fld dword ptr [edx] // 00c434f2
        fadd dword ptr [esp + 0c0h] // 00c434f4
        fstp dword ptr [edx] // 00c434fb
        fld dword ptr [esp + 0c4h] // 00c434fd
        fadd dword ptr [edx + 4] // 00c43504
        mov eax, dword ptr [ebp + 0ea94h] // 00c43507
        fstp dword ptr [edx + 4] // 00c4350d
        fld dword ptr [esp + 0c8h] // 00c43510
        fadd dword ptr [edx + 8] // 00c43517
        fstp dword ptr [edx + 8] // 00c4351a
        fld dword ptr [esi + 8] // 00c4351d
        fstp dword ptr [esp + 068h] // 00c43520
        fld dword ptr [esp + 068h] // 00c43524
        fmul st(0), st(4) // 00c43528
        fstp dword ptr [esp + 074h] // 00c4352a
        fld dword ptr [esi + 0ch] // 00c4352e
        fstp dword ptr [esp + 024h] // 00c43531
        fld dword ptr [esp + 024h] // 00c43535
        fmul st(0), st(4) // 00c43539
        fstp dword ptr [esp + 078h] // 00c4353b
        fld dword ptr [esi + 010h] // 00c4353f
        fstp dword ptr [esp + 010h] // 00c43542
        fld dword ptr [esp + 010h] // 00c43546
        fmulp st(4), st(0) // 00c4354a
        fxch st(3) // 00c4354c
        fstp dword ptr [esp + 07ch] // 00c4354e
        fld dword ptr [esp + 074h] // 00c43552
        fadd dword ptr [edx + 0ch] // 00c43556
        fstp dword ptr [edx + 0ch] // 00c43559
        fld dword ptr [edx + 010h] // 00c4355c
        fadd dword ptr [esp + 078h] // 00c4355f
        fstp dword ptr [edx + 010h] // 00c43563
        fld dword ptr [edx + 014h] // 00c43566
        fadd dword ptr [esp + 07ch] // 00c43569
        fstp dword ptr [edx + 014h] // 00c4356d
        fld dword ptr [eax + edi] // 00c43570
        mov eax, dword ptr [ebp + 0ea8ch] // 00c43573
        fld dword ptr [ecx + 01ch] // 00c43579
        fmulp st(6), st(0) // 00c4357c
        fld dword ptr [ecx + 018h] // 00c4357e
        fmul dword ptr [esp + 060h] // 00c43581
        faddp st(6), st(0) // 00c43585
        fld dword ptr [ecx + 020h] // 00c43587
        fmul dword ptr [esp + 02ch] // 00c4358a
        faddp st(6), st(0) // 00c4358e
        fxch st(5) // 00c43590
        fstp dword ptr [esp + 014h] // 00c43592
        fld dword ptr [esp + 014h] // 00c43596
        fsubp st(5), st(0) // 00c4359a
        fld dword ptr [ecx + 028h] // 00c4359c
        fmul dword ptr [esp + 034h] // 00c4359f
        fld dword ptr [ecx + 024h] // 00c435a3
        fmul dword ptr [esp + 028h] // 00c435a6
        faddp st(1), st(0) // 00c435aa
        fld dword ptr [ecx + 02ch] // 00c435ac
        fmul dword ptr [esp + 04ch] // 00c435af
        faddp st(1), st(0) // 00c435b3
        fstp dword ptr [esp + 014h] // 00c435b5
        fld dword ptr [esp + 014h] // 00c435b9
        fsubp st(5), st(0) // 00c435bd
        fld dword ptr [edx + 01ch] // 00c435bf
        fmul dword ptr [esp + 030h] // 00c435c2
        fld dword ptr [edx + 018h] // 00c435c6
        fmul dword ptr [esp + 054h] // 00c435c9
        faddp st(1), st(0) // 00c435cd
        fld dword ptr [edx + 020h] // 00c435cf
        fmul dword ptr [esp + 038h] // 00c435d2
        faddp st(1), st(0) // 00c435d6
        fstp dword ptr [esp + 014h] // 00c435d8
        fld dword ptr [esp + 014h] // 00c435dc
        fsubp st(5), st(0) // 00c435e0
        fld dword ptr [edx + 028h] // 00c435e2
        fmul dword ptr [esp + 05ch] // 00c435e5
        fld dword ptr [edx + 024h] // 00c435e9
        fmul dword ptr [esp + 040h] // 00c435ec
        faddp st(1), st(0) // 00c435f0
        fld dword ptr [edx + 02ch] // 00c435f2
        fmul dword ptr [esp + 064h] // 00c435f5
        faddp st(1), st(0) // 00c435f9
        fstp dword ptr [esp + 014h] // 00c435fb
        fld dword ptr [esp + 014h] // 00c435ff
        fsubp st(5), st(0) // 00c43603
        fxch st(4) // 00c43605
        fstp dword ptr [esp + 014h] // 00c43607
        fld dword ptr [esp + 014h] // 00c4360b
        fmul dword ptr [eax + edi] // 00c4360f
        mov eax, dword ptr [ebp + 0eab8h] // 00c43612
        fstp dword ptr [esp + 014h] // 00c43618
        fld dword ptr [esp + 014h] // 00c4361c
        fadd dword ptr [eax + edi] // 00c43620
        fstp dword ptr [esp + 018h] // 00c43623
        fld dword ptr [esp + 018h] // 00c43627
        fldz  // 00c4362b
        fcomip st(0), st(1) // 00c4362d
        fstp st(0) // 00c4362f
        jbe l_00c43639 // 00c43631
        movss dword ptr [esp + 018h], xmm0 // 00c43633
    l_00c43639:
        fld dword ptr [esp + 018h] // 00c43639
        add eax, edi // 00c4363d
        fsub dword ptr [eax] // 00c4363f
        movss xmm1, dword ptr [esp + 018h] // 00c43641
        movss dword ptr [eax], xmm1 // 00c43647
        add edi, 4 // 00c4364b
        fstp dword ptr [esp + 014h] // 00c4364e
        fld dword ptr [esp + 014h] // 00c43652
        fld st(0) // 00c43656
        fmulp st(4), st(0) // 00c43658
        fxch st(3) // 00c4365a
        fstp dword ptr [esp + 084h] // 00c4365c
        fmul st(0), st(2) // 00c43663
        fstp dword ptr [esp + 088h] // 00c43665
        fld st(1) // 00c4366c
        fmulp st(3), st(0) // 00c4366e
        fxch st(2) // 00c43670
        fstp dword ptr [esp + 08ch] // 00c43672
        fld dword ptr [ecx + 018h] // 00c43679
        fadd dword ptr [esp + 084h] // 00c4367c
        fstp dword ptr [ecx + 018h] // 00c43683
        fld dword ptr [esp + 088h] // 00c43686
        fadd dword ptr [ecx + 01ch] // 00c4368d
        fstp dword ptr [ecx + 01ch] // 00c43690
        fld dword ptr [ecx + 020h] // 00c43693
        fadd dword ptr [esp + 08ch] // 00c43696
        fstp dword ptr [ecx + 020h] // 00c4369d
        fld st(0) // 00c436a0
        fmulp st(2), st(0) // 00c436a2
        fxch st(1) // 00c436a4
        fstp dword ptr [esp + 09ch] // 00c436a6
        fld dword ptr [esp + 048h] // 00c436ad
        fmul st(0), st(1) // 00c436b1
        fstp dword ptr [esp + 0a0h] // 00c436b3
        fld dword ptr [esp + 06ch] // 00c436ba
        fmul st(0), st(1) // 00c436be
        fstp dword ptr [esp + 0a4h] // 00c436c0
        fld dword ptr [esp + 09ch] // 00c436c7
        fadd dword ptr [ecx + 024h] // 00c436ce
        fstp dword ptr [ecx + 024h] // 00c436d1
        fld dword ptr [esp + 0a0h] // 00c436d4
        fadd dword ptr [ecx + 028h] // 00c436db
        fstp dword ptr [ecx + 028h] // 00c436de
        fld dword ptr [esp + 0a4h] // 00c436e1
        fadd dword ptr [ecx + 02ch] // 00c436e8
        fstp dword ptr [ecx + 02ch] // 00c436eb
        fld dword ptr [esp + 050h] // 00c436ee
        fmul st(0), st(1) // 00c436f2
        fstp dword ptr [esp + 0b4h] // 00c436f4
        fld dword ptr [esp + 020h] // 00c436fb
        fmul st(0), st(1) // 00c436ff
        fstp dword ptr [esp + 0b8h] // 00c43701
        fld dword ptr [esp + 058h] // 00c43708
        fmul st(0), st(1) // 00c4370c
        fstp dword ptr [esp + 0bch] // 00c4370e
        fld dword ptr [esp + 0b4h] // 00c43715
        fadd dword ptr [edx + 018h] // 00c4371c
        fstp dword ptr [edx + 018h] // 00c4371f
        fld dword ptr [edx + 01ch] // 00c43722
        fadd dword ptr [esp + 0b8h] // 00c43725
        fstp dword ptr [edx + 01ch] // 00c4372c
        fld dword ptr [esp + 0bch] // 00c4372f
        fadd dword ptr [edx + 020h] // 00c43736
        fstp dword ptr [edx + 020h] // 00c43739
        fld dword ptr [esp + 068h] // 00c4373c
        fmul st(0), st(1) // 00c43740
        fstp dword ptr [esp + 0cch] // 00c43742
        fld dword ptr [esp + 024h] // 00c43749
        fmul st(0), st(1) // 00c4374d
        fstp dword ptr [esp + 0d0h] // 00c4374f
        fmul dword ptr [esp + 010h] // 00c43756
        fstp dword ptr [esp + 0d4h] // 00c4375a
        fld dword ptr [esp + 0cch] // 00c43761
        fadd dword ptr [edx + 024h] // 00c43768
        fstp dword ptr [edx + 024h] // 00c4376b
        fld dword ptr [edx + 028h] // 00c4376e
        fadd dword ptr [esp + 0d0h] // 00c43771
        fstp dword ptr [edx + 028h] // 00c43778
        fld dword ptr [edx + 02ch] // 00c4377b
        fadd dword ptr [esp + 0d4h] // 00c4377e
        fstp dword ptr [edx + 02ch] // 00c43785
        add ebx, 030h // 00c43788
        add esi, 030h // 00c4378b
        sub dword ptr [esp + 01ch], 1 // 00c4378e
        jne l_00c43270 // 00c43793
        mov edi, dword ptr [esp + 03ch] // 00c43799
    l_00c4379d:
        mov esi, dword ptr [esp + 044h] // 00c4379d
        add edi, 1 // 00c437a1
        add esi, 0c0h // 00c437a4
        cmp edi, dword ptr [ebp + 0eac4h] // 00c437aa
        mov dword ptr [esp + 03ch], edi // 00c437b0
        mov dword ptr [esp + 044h], esi // 00c437b4
        jl l_00c43200 // 00c437b8
        pop esi // 00c437be
        pop ebx // 00c437bf
    l_00c437c0:
        pop edi // 00c437c0
        pop ebp // 00c437c1
        add esp, 0c8h // 00c437c2
        ret 4 // 00c437c8
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void warm_kernel(){
    __asm {
        sub esp, 070h // 00c437d0
        push ebx // 00c437d3
        xor ebx, ebx // 00c437d4
        cmp dword ptr [ecx + 0eac4h], ebx // 00c437d6
        mov dword ptr [esp + 8], ebx // 00c437dc
        jle l_00c43a8d // 00c437e0
        push ebp // 00c437e6
        push esi // 00c437e7
        mov dword ptr [esp + 0ch], ebx // 00c437e8
        push edi // 00c437ec
        _emit 08dh // 00c437ed
        _emit 049h
        _emit 000h
    l_00c437f0:
        mov eax, dword ptr [ecx + 0ea7ch] // 00c437f0
        movsx esi, word ptr [eax + ebx*4] // 00c437f6
        movsx edi, word ptr [eax + ebx*4 + 2] // 00c437fa
        mov edx, dword ptr [ecx + 0ea88h] // 00c437ff
        mov ebp, dword ptr [edx + ebx*4] // 00c43805
        lea eax, [eax + ebx*4] // 00c43808
        mov eax, dword ptr [ecx + 0ea84h] // 00c4380b
        add eax, dword ptr [esp + 010h] // 00c43811
        test ebp, ebp // 00c43815
        jle l_00c43a6f // 00c43817
        lea esi, [esi + esi*2] // 00c4381d
        lea edi, [edi + edi*2] // 00c43820
        shl esi, 4 // 00c43823
        shl edi, 4 // 00c43826
        shl ebx, 4 // 00c43829
        add eax, 014h // 00c4382c
        nop  // 00c4382f
    l_00c43830:
        mov edx, dword ptr [ecx + 0eab4h] // 00c43830
        fld dword ptr [ebx + edx] // 00c43836
        mov edx, dword ptr [ecx + 0eab8h] // 00c43839
        fstp dword ptr [esp + 018h] // 00c4383f
        movss xmm0, dword ptr [ebx + edx] // 00c43843
        fld dword ptr [eax - 014h] // 00c43848
        mov edx, dword ptr [ecx + 0ea6ch] // 00c4384b
        fld dword ptr [esp + 018h] // 00c43851
        add edx, esi // 00c43855
        fld st(0) // 00c43857
        movss dword ptr [esp + 01ch], xmm0 // 00c43859
        fmulp st(2), st(0) // 00c4385f
        fxch st(1) // 00c43861
        fstp dword ptr [esp + 020h] // 00c43863
        fld dword ptr [eax - 010h] // 00c43867
        fmul st(0), st(1) // 00c4386a
        fstp dword ptr [esp + 024h] // 00c4386c
        fld dword ptr [eax - 0ch] // 00c43870
        fmul st(0), st(1) // 00c43873
        fstp dword ptr [esp + 028h] // 00c43875
        fld dword ptr [edx] // 00c43879
        fadd dword ptr [esp + 020h] // 00c4387b
        fstp dword ptr [edx] // 00c4387f
        fld dword ptr [edx + 4] // 00c43881
        fadd dword ptr [esp + 024h] // 00c43884
        fstp dword ptr [edx + 4] // 00c43888
        fld dword ptr [esp + 028h] // 00c4388b
        fadd dword ptr [edx + 8] // 00c4388f
        fstp dword ptr [edx + 8] // 00c43892
        mov edx, dword ptr [ecx + 0ea6ch] // 00c43895
        fld dword ptr [eax - 8] // 00c4389b
        lea edx, [esi + edx + 0ch] // 00c4389e
        fmul st(0), st(1) // 00c438a2
        fstp dword ptr [esp + 02ch] // 00c438a4
        fld dword ptr [eax - 4] // 00c438a8
        fmul st(0), st(1) // 00c438ab
        fstp dword ptr [esp + 030h] // 00c438ad
        fld dword ptr [eax] // 00c438b1
        fmul st(0), st(1) // 00c438b3
        fstp dword ptr [esp + 034h] // 00c438b5
        fld dword ptr [edx] // 00c438b9
        fadd dword ptr [esp + 02ch] // 00c438bb
        fstp dword ptr [edx] // 00c438bf
        fld dword ptr [esp + 030h] // 00c438c1
        fadd dword ptr [edx + 4] // 00c438c5
        fstp dword ptr [edx + 4] // 00c438c8
        fld dword ptr [edx + 8] // 00c438cb
        fadd dword ptr [esp + 034h] // 00c438ce
        fstp dword ptr [edx + 8] // 00c438d2
        mov edx, dword ptr [ecx + 0ea6ch] // 00c438d5
        fld dword ptr [eax + 4] // 00c438db
        add edx, edi // 00c438de
        fmul st(0), st(1) // 00c438e0
        fstp dword ptr [esp + 038h] // 00c438e2
        fld dword ptr [eax + 8] // 00c438e6
        fmul st(0), st(1) // 00c438e9
        fstp dword ptr [esp + 03ch] // 00c438eb
        fld dword ptr [eax + 0ch] // 00c438ef
        fmul st(0), st(1) // 00c438f2
        fstp dword ptr [esp + 040h] // 00c438f4
        fld dword ptr [edx] // 00c438f8
        fadd dword ptr [esp + 038h] // 00c438fa
        fstp dword ptr [edx] // 00c438fe
        fld dword ptr [esp + 03ch] // 00c43900
        fadd dword ptr [edx + 4] // 00c43904
        fstp dword ptr [edx + 4] // 00c43907
        fld dword ptr [edx + 8] // 00c4390a
        fadd dword ptr [esp + 040h] // 00c4390d
        fstp dword ptr [edx + 8] // 00c43911
        mov edx, dword ptr [ecx + 0ea6ch] // 00c43914
        fld dword ptr [eax + 010h] // 00c4391a
        lea edx, [edi + edx + 0ch] // 00c4391d
        fmul st(0), st(1) // 00c43921
        fstp dword ptr [esp + 044h] // 00c43923
        fld dword ptr [eax + 014h] // 00c43927
        fmul st(0), st(1) // 00c4392a
        fstp dword ptr [esp + 048h] // 00c4392c
        fmul dword ptr [eax + 018h] // 00c43930
        fstp dword ptr [esp + 04ch] // 00c43933
        fld dword ptr [edx] // 00c43937
        fadd dword ptr [esp + 044h] // 00c43939
        fstp dword ptr [edx] // 00c4393d
        fld dword ptr [edx + 4] // 00c4393f
        fadd dword ptr [esp + 048h] // 00c43942
        fstp dword ptr [edx + 4] // 00c43946
        fld dword ptr [edx + 8] // 00c43949
        fadd dword ptr [esp + 04ch] // 00c4394c
        fstp dword ptr [edx + 8] // 00c43950
        mov edx, dword ptr [ecx + 0ea6ch] // 00c43953
        fld dword ptr [eax - 014h] // 00c43959
        lea edx, [esi + edx + 018h] // 00c4395c
        fld dword ptr [esp + 01ch] // 00c43960
        fld st(0) // 00c43964
        fmulp st(2), st(0) // 00c43966
        fxch st(1) // 00c43968
        fstp dword ptr [esp + 050h] // 00c4396a
        fld dword ptr [eax - 010h] // 00c4396e
        fmul st(0), st(1) // 00c43971
        fstp dword ptr [esp + 054h] // 00c43973
        fld dword ptr [eax - 0ch] // 00c43977
        fmul st(0), st(1) // 00c4397a
        fstp dword ptr [esp + 058h] // 00c4397c
        fld dword ptr [esp + 050h] // 00c43980
        fadd dword ptr [edx] // 00c43984
        fstp dword ptr [edx] // 00c43986
        fld dword ptr [edx + 4] // 00c43988
        fadd dword ptr [esp + 054h] // 00c4398b
        fstp dword ptr [edx + 4] // 00c4398f
        fld dword ptr [edx + 8] // 00c43992
        fadd dword ptr [esp + 058h] // 00c43995
        fstp dword ptr [edx + 8] // 00c43999
        mov edx, dword ptr [ecx + 0ea6ch] // 00c4399c
        fld dword ptr [eax - 8] // 00c439a2
        lea edx, [esi + edx + 024h] // 00c439a5
        fmul st(0), st(1) // 00c439a9
        fstp dword ptr [esp + 05ch] // 00c439ab
        fld dword ptr [eax - 4] // 00c439af
        fmul st(0), st(1) // 00c439b2
        fstp dword ptr [esp + 060h] // 00c439b4
        fld dword ptr [eax] // 00c439b8
        fmul st(0), st(1) // 00c439ba
        fstp dword ptr [esp + 064h] // 00c439bc
        fld dword ptr [edx] // 00c439c0
        fadd dword ptr [esp + 05ch] // 00c439c2
        fstp dword ptr [edx] // 00c439c6
        fld dword ptr [edx + 4] // 00c439c8
        fadd dword ptr [esp + 060h] // 00c439cb
        fstp dword ptr [edx + 4] // 00c439cf
        fld dword ptr [edx + 8] // 00c439d2
        fadd dword ptr [esp + 064h] // 00c439d5
        fstp dword ptr [edx + 8] // 00c439d9
        mov edx, dword ptr [ecx + 0ea6ch] // 00c439dc
        fld dword ptr [eax + 4] // 00c439e2
        lea edx, [edi + edx + 018h] // 00c439e5
        fmul st(0), st(1) // 00c439e9
        fstp dword ptr [esp + 068h] // 00c439eb
        fld dword ptr [eax + 8] // 00c439ef
        fmul st(0), st(1) // 00c439f2
        fstp dword ptr [esp + 06ch] // 00c439f4
        fld dword ptr [eax + 0ch] // 00c439f8
        fmul st(0), st(1) // 00c439fb
        fstp dword ptr [esp + 070h] // 00c439fd
        fld dword ptr [edx] // 00c43a01
        fadd dword ptr [esp + 068h] // 00c43a03
        fstp dword ptr [edx] // 00c43a07
        fld dword ptr [edx + 4] // 00c43a09
        fadd dword ptr [esp + 06ch] // 00c43a0c
        fstp dword ptr [edx + 4] // 00c43a10
        fld dword ptr [edx + 8] // 00c43a13
        fadd dword ptr [esp + 070h] // 00c43a16
        fstp dword ptr [edx + 8] // 00c43a1a
        mov edx, dword ptr [ecx + 0ea6ch] // 00c43a1d
        fld dword ptr [eax + 010h] // 00c43a23
        lea edx, [edi + edx + 024h] // 00c43a26
        fmul st(0), st(1) // 00c43a2a
        fstp dword ptr [esp + 074h] // 00c43a2c
        fld dword ptr [eax + 014h] // 00c43a30
        fmul st(0), st(1) // 00c43a33
        fstp dword ptr [esp + 078h] // 00c43a35
        fmul dword ptr [eax + 018h] // 00c43a39
        fstp dword ptr [esp + 07ch] // 00c43a3c
        fld dword ptr [edx] // 00c43a40
        fadd dword ptr [esp + 074h] // 00c43a42
        add ebx, 4 // 00c43a46
        add eax, 030h // 00c43a49
        sub ebp, 1 // 00c43a4c
        fstp dword ptr [edx] // 00c43a4f
        fld dword ptr [esp + 078h] // 00c43a51
        fadd dword ptr [edx + 4] // 00c43a55
        fstp dword ptr [edx + 4] // 00c43a58
        fld dword ptr [edx + 8] // 00c43a5b
        fadd dword ptr [esp + 07ch] // 00c43a5e
        fstp dword ptr [edx + 8] // 00c43a62
        jne l_00c43830 // 00c43a65
        mov ebx, dword ptr [esp + 014h] // 00c43a6b
    l_00c43a6f:
        add dword ptr [esp + 010h], 0c0h // 00c43a6f
        add ebx, 1 // 00c43a77
        cmp ebx, dword ptr [ecx + 0eac4h] // 00c43a7a
        mov dword ptr [esp + 014h], ebx // 00c43a80
        jl l_00c437f0 // 00c43a84
        pop edi // 00c43a8a
        pop esi // 00c43a8b
        pop ebp // 00c43a8c
    l_00c43a8d:
        pop ebx // 00c43a8d
        add esp, 070h // 00c43a8e
        ret  // 00c43a91
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void build_rows_kernel(){
    __asm {
        sub esp, 019ch // 00c4f140
        mov eax, dword ptr [esp + 01a0h] // 00c4f146
        mov ecx, dword ptr [eax + 0ea68h] // 00c4f14d
        push ebx // 00c4f153
        push ebp // 00c4f154
        mov dword ptr [eax + 4], 1 // 00c4f155
        mov edx, dword ptr [ecx + 4] // 00c4f15c
        xor ecx, ecx // 00c4f15f
        cmp edx, ecx // 00c4f161
        push esi // 00c4f163
        push edi // 00c4f164
        mov dword ptr [esp + 0e0h], edx // 00c4f165
        mov dword ptr [eax + 0eac4h], edx // 00c4f16c
        mov dword ptr [esp + 0a8h], ecx // 00c4f172
        jle l_00c50303 // 00c4f179
        xorps xmm1, xmm1 // 00c4f17f
        movss xmm3, dword ptr constant_00d7a208 // 00c4f182
        mov dword ptr [esp + 06ch], ecx // 00c4f18a
        mov edi, edi // 00c4f18e
    l_00c4f190:
        mov edx, dword ptr [eax + 0ea68h] // 00c4f190
        mov edx, dword ptr [edx] // 00c4f196
        mov esi, dword ptr [edx + ecx*4] // 00c4f198
        mov edx, dword ptr [esi + 0cch] // 00c4f19b
        test byte ptr [edx + 050h], 1 // 00c4f1a1
        mov edi, dword ptr [eax] // 00c4f1a5
        mov edi, dword ptr [edi + 02ch] // 00c4f1a7
        mov ebp, dword ptr [edx + 4] // 00c4f1aa
        mov dword ptr [esp + 060h], esi // 00c4f1ad
        mov esi, dword ptr [esi + 0d0h] // 00c4f1b1
        mov ebx, dword ptr [esi + 4] // 00c4f1b7
        mov dword ptr [esp + 064h], edx // 00c4f1ba
        mov dword ptr [esp + 08ch], esi // 00c4f1be
        mov dword ptr [esp + 024h], edi // 00c4f1c5
        je l_00c4f1db // 00c4f1c9
        mov dword ptr [edx + 05ch], 0 // 00c4f1cb
        mov edi, dword ptr [eax + 4] // 00c4f1d2
        mov dword ptr [eax + edi*4 + 8], edx // 00c4f1d5
        jmp l_00c4f1f4 // 00c4f1d9
    l_00c4f1db:
        cmp dword ptr [edx + 058h], edi // 00c4f1db
        je l_00c4f1f8 // 00c4f1de
        mov dword ptr [edx + 058h], edi // 00c4f1e0
        mov edi, dword ptr [eax + 4] // 00c4f1e3
        mov dword ptr [edx + 05ch], edi // 00c4f1e6
        mov edi, dword ptr [eax + 4] // 00c4f1e9
        mov dword ptr [eax + edi*4 + 8], edx // 00c4f1ec
        add dword ptr [eax + 4], 1 // 00c4f1f0
    l_00c4f1f4:
        mov edi, dword ptr [esp + 024h] // 00c4f1f4
    l_00c4f1f8:
        test byte ptr [esi + 050h], 1 // 00c4f1f8
        je l_00c4f20e // 00c4f1fc
        mov dword ptr [esi + 05ch], 0 // 00c4f1fe
        mov edi, dword ptr [eax + 4] // 00c4f205
        mov dword ptr [eax + edi*4 + 8], esi // 00c4f208
        jmp l_00c4f227 // 00c4f20c
    l_00c4f20e:
        cmp dword ptr [esi + 058h], edi // 00c4f20e
        je l_00c4f227 // 00c4f211
        mov dword ptr [esi + 058h], edi // 00c4f213
        mov edi, dword ptr [eax + 4] // 00c4f216
        mov dword ptr [esi + 05ch], edi // 00c4f219
        mov edi, dword ptr [eax + 4] // 00c4f21c
        mov dword ptr [eax + edi*4 + 8], esi // 00c4f21f
        add dword ptr [eax + 4], 1 // 00c4f223
    l_00c4f227:
        mov edx, dword ptr [edx + 05ch] // 00c4f227
        mov esi, dword ptr [esi + 05ch] // 00c4f22a
        mov edi, dword ptr [eax + 0ea7ch] // 00c4f22d
        mov word ptr [edi + ecx*4], dx // 00c4f233
        mov edi, dword ptr [eax + 0ea7ch] // 00c4f237
        mov word ptr [edi + ecx*4 + 2], si // 00c4f23d
        mov edi, dword ptr [eax + 0ea98h] // 00c4f242
        mov word ptr [edi + ecx*4], dx // 00c4f248
        mov edx, dword ptr [eax + 0ea98h] // 00c4f24c
        mov word ptr [edx + ecx*4 + 2], si // 00c4f252
        mov esi, dword ptr [esp + 06ch] // 00c4f257
        mov edi, dword ptr [eax + 0ea84h] // 00c4f25b
        mov edx, dword ptr [esp + 060h] // 00c4f261
        add edi, esi // 00c4f265
        mov dword ptr [esp + 038h], edi // 00c4f267
        mov edi, dword ptr [eax + 0ea9ch] // 00c4f26b
        add edi, esi // 00c4f271
        add edx, 8 // 00c4f273
        mov dword ptr [esp + 024h], edi // 00c4f276
        mov edi, dword ptr [eax + 0eaa0h] // 00c4f27a
        add edi, esi // 00c4f280
        mov dword ptr [esp + 030h], edx // 00c4f282
        mov edx, dword ptr [eax + 0ea80h] // 00c4f286
        add edx, esi // 00c4f28c
        mov esi, dword ptr [esp + 060h] // 00c4f28e
        mov esi, dword ptr [esi + 0c8h] // 00c4f292
        mov dword ptr [esp + 02ch], edi // 00c4f298
        mov edi, dword ptr [eax + 0ea88h] // 00c4f29c
        mov dword ptr [edi + ecx*4], esi // 00c4f2a2
        mov edi, dword ptr [eax + 0eaa4h] // 00c4f2a5
        mov dword ptr [edi + ecx*4], esi // 00c4f2ab
        mov edi, dword ptr [esp + 060h] // 00c4f2ae
        fld dword ptr [edi] // 00c4f2b2
        mov dword ptr [esp + 028h], esi // 00c4f2b4
        cmp dword ptr [esp + 028h], 0 // 00c4f2b8
        mov esi, dword ptr [eax + 0eab0h] // 00c4f2bd
        fstp dword ptr [esi + ecx*4] // 00c4f2c3
        jle l_00c502e4 // 00c4f2c6
        mov eax, dword ptr [esp + 030h] // 00c4f2cc
        add eax, 014h // 00c4f2d0
        mov dword ptr [esp + 034h], eax // 00c4f2d3
        mov eax, dword ptr [esp + 038h] // 00c4f2d7
        shl ecx, 4 // 00c4f2db
        mov esi, dword ptr [esp + 024h] // 00c4f2de
        mov dword ptr [esp + 048h], ecx // 00c4f2e2
        mov ecx, dword ptr [esp + 02ch] // 00c4f2e6
        add ecx, 028h // 00c4f2ea
        mov dword ptr [esp + 0ach], ecx // 00c4f2ed
        lea ecx, [eax + 014h] // 00c4f2f4
        sub eax, edx // 00c4f2f7
        mov dword ptr [esp + 0b0h], eax // 00c4f2f9
        mov eax, dword ptr [esp + 024h] // 00c4f300
        sub eax, edx // 00c4f304
        mov dword ptr [esp + 05ch], eax // 00c4f306
        mov eax, dword ptr [esp + 02ch] // 00c4f30a
        sub eax, edx // 00c4f30e
        mov dword ptr [esp + 094h], eax // 00c4f310
        mov eax, dword ptr [esp + 024h] // 00c4f317
        lea edi, [edx + 010h] // 00c4f31b
        mov edx, dword ptr [esp + 038h] // 00c4f31e
        sub eax, edx // 00c4f322
        mov dword ptr [esp + 038h], eax // 00c4f324
        mov eax, dword ptr [esp + 02ch] // 00c4f328
        sub eax, edx // 00c4f32c
        mov edx, dword ptr [esp + 028h] // 00c4f32e
        mov dword ptr [esp + 0a4h], eax // 00c4f332
        mov eax, dword ptr [esp + 02ch] // 00c4f339
        add esi, 01ch // 00c4f33d
        sub eax, dword ptr [esp + 024h] // 00c4f340
        mov dword ptr [esp + 058h], ecx // 00c4f344
        mov dword ptr [esp + 0c0h], eax // 00c4f348
        mov dword ptr [esp + 090h], edx // 00c4f34f
        jmp l_00c4f364 // 00c4f356
        jmp l_00c4f360 // 00c4f358
        _emit 08dh // 00c4f35a
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c4f360:
        mov ecx, dword ptr [esp + 058h] // 00c4f360
    l_00c4f364:
        mov edx, dword ptr [esp + 034h] // 00c4f364
        mov eax, dword ptr [esp + 064h] // 00c4f368
        fld dword ptr [edx - 4] // 00c4f36c
        fstp dword ptr [esp + 014h] // 00c4f36f
        fld dword ptr [edx - 8] // 00c4f373
        fstp dword ptr [esp + 028h] // 00c4f376
        fld dword ptr [edx] // 00c4f37a
        fstp dword ptr [esp + 024h] // 00c4f37c
        fld dword ptr [eax + 014h] // 00c4f380
        fld dword ptr [esp + 014h] // 00c4f383
        fld st(0) // 00c4f387
        fmulp st(2), st(0) // 00c4f389
        fld dword ptr [eax + 8] // 00c4f38b
        fld dword ptr [esp + 028h] // 00c4f38e
        fld st(0) // 00c4f392
        fmulp st(2), st(0) // 00c4f394
        fxch st(3) // 00c4f396
        faddp st(1), st(0) // 00c4f398
        fld dword ptr [eax + 020h] // 00c4f39a
        fld dword ptr [esp + 024h] // 00c4f39d
        fld st(0) // 00c4f3a1
        fmulp st(2), st(0) // 00c4f3a3
        fxch st(2) // 00c4f3a5
        faddp st(1), st(0) // 00c4f3a7
        fadd dword ptr [eax + 02ch] // 00c4f3a9
        fstp dword ptr [esp + 0190h] // 00c4f3ac
        fld st(2) // 00c4f3b3
        fmul dword ptr [eax + 0ch] // 00c4f3b5
        fld dword ptr [eax + 018h] // 00c4f3b8
        fmul st(0), st(3) // 00c4f3bb
        faddp st(1), st(0) // 00c4f3bd
        fld dword ptr [eax + 024h] // 00c4f3bf
        fmul st(0), st(2) // 00c4f3c2
        faddp st(1), st(0) // 00c4f3c4
        fadd dword ptr [eax + 030h] // 00c4f3c6
        fstp dword ptr [esp + 0194h] // 00c4f3c9
        fld dword ptr [eax + 01ch] // 00c4f3d0
        fmulp st(2), st(0) // 00c4f3d3
        fld dword ptr [eax + 010h] // 00c4f3d5
        fmulp st(3), st(0) // 00c4f3d8
        fxch st(1) // 00c4f3da
        faddp st(2), st(0) // 00c4f3dc
        fmul dword ptr [eax + 028h] // 00c4f3de
        faddp st(1), st(0) // 00c4f3e1
        fadd dword ptr [eax + 034h] // 00c4f3e3
        mov eax, dword ptr [esp + 08ch] // 00c4f3e6
        fstp dword ptr [esp + 0198h] // 00c4f3ed
        fld dword ptr [edx + 8] // 00c4f3f4
        fstp dword ptr [esp + 028h] // 00c4f3f7
        fld dword ptr [edx + 4] // 00c4f3fb
        fstp dword ptr [esp + 014h] // 00c4f3fe
        fld dword ptr [edx + 0ch] // 00c4f402
        fstp dword ptr [esp + 024h] // 00c4f405
        fld dword ptr [esp + 014h] // 00c4f409
        fld st(0) // 00c4f40d
        fmul dword ptr [eax + 8] // 00c4f40f
        fld dword ptr [eax + 014h] // 00c4f412
        fld dword ptr [esp + 028h] // 00c4f415
        fld st(0) // 00c4f419
        fmulp st(2), st(0) // 00c4f41b
        fxch st(2) // 00c4f41d
        faddp st(1), st(0) // 00c4f41f
        fld dword ptr [esp + 024h] // 00c4f421
        fld st(0) // 00c4f425
        fmul dword ptr [eax + 020h] // 00c4f427
        faddp st(2), st(0) // 00c4f42a
        fld dword ptr [eax + 02ch] // 00c4f42c
        faddp st(2), st(0) // 00c4f42f
        fxch st(1) // 00c4f431
        fstp dword ptr [esp + 0f0h] // 00c4f433
        fld dword ptr [eax + 0ch] // 00c4f43a
        fmul st(0), st(3) // 00c4f43d
        fld st(2) // 00c4f43f
        fmul dword ptr [eax + 018h] // 00c4f441
        faddp st(1), st(0) // 00c4f444
        fld dword ptr [eax + 024h] // 00c4f446
        fmul st(0), st(2) // 00c4f449
        faddp st(1), st(0) // 00c4f44b
        fadd dword ptr [eax + 030h] // 00c4f44d
        fstp dword ptr [esp + 0f4h] // 00c4f450
        fld dword ptr [eax + 010h] // 00c4f457
        fmulp st(3), st(0) // 00c4f45a
        fld dword ptr [eax + 01ch] // 00c4f45c
        fmulp st(2), st(0) // 00c4f45f
        fxch st(2) // 00c4f461
        faddp st(1), st(0) // 00c4f463
        fld dword ptr [eax + 028h] // 00c4f465
        fmulp st(2), st(0) // 00c4f468
        faddp st(1), st(0) // 00c4f46a
        fadd dword ptr [eax + 034h] // 00c4f46c
        mov eax, dword ptr [esp + 01b0h] // 00c4f46f
        fstp dword ptr [esp + 0f8h] // 00c4f476
        fld dword ptr [edx + 010h] // 00c4f47d
        mov edx, dword ptr [eax] // 00c4f480
        fmul dword ptr [edx + 020h] // 00c4f482
        mov eax, dword ptr [eax + 0eab4h] // 00c4f485
        mov edx, dword ptr [esp + 048h] // 00c4f48b
        fstp dword ptr [esp + 014h] // 00c4f48f
        fld dword ptr [esp + 014h] // 00c4f493
        fstp dword ptr [edx + eax] // 00c4f497
        mov eax, dword ptr [esp + 01b0h] // 00c4f49a
        mov edx, dword ptr [eax + 0eab8h] // 00c4f4a1
        mov eax, dword ptr [esp + 034h] // 00c4f4a7
        fld dword ptr [eax + 014h] // 00c4f4ab
        mov eax, dword ptr [esp + 048h] // 00c4f4ae
        fstp dword ptr [eax + edx] // 00c4f4b2
        mov eax, dword ptr [esp + 01b0h] // 00c4f4b5
        mov edx, dword ptr [esp + 034h] // 00c4f4bc
        movss xmm0, dword ptr [edx + 018h] // 00c4f4c0
        comiss xmm1, xmm0 // 00c4f4c5
        mov eax, dword ptr [eax] // 00c4f4c8
        movss xmm2, dword ptr [eax + 028h] // 00c4f4ca
        movss dword ptr [esp + 0d0h], xmm0 // 00c4f4cf
        movss dword ptr [esp + 0138h], xmm2 // 00c4f4d8
        jbe l_00c4f4eb // 00c4f4e1
        movss dword ptr [esp + 068h], xmm1 // 00c4f4e3
        jmp l_00c4f50d // 00c4f4e9
    l_00c4f4eb:
        fld dword ptr [esp + 0138h] // 00c4f4eb
        fld dword ptr [esp + 0d0h] // 00c4f4f2
        fcomip st(0), st(1) // 00c4f4f9
        fstp st(0) // 00c4f4fb
        jbe l_00c4f507 // 00c4f4fd
        movss dword ptr [esp + 068h], xmm2 // 00c4f4ff
        jmp l_00c4f50d // 00c4f505
    l_00c4f507:
        movss dword ptr [esp + 068h], xmm0 // 00c4f507
    l_00c4f50d:
        mov eax, dword ptr [esp + 064h] // 00c4f50d
        fld dword ptr [esp + 0190h] // 00c4f511
        fsub dword ptr [eax + 02ch] // 00c4f518
        movaps xmm0, xmm3 // 00c4f51b
        fstp dword ptr [esp + 070h] // 00c4f51e
        fld dword ptr [esp + 0194h] // 00c4f522
        fsub dword ptr [eax + 030h] // 00c4f529
        fstp dword ptr [esp + 074h] // 00c4f52c
        fld dword ptr [esp + 0198h] // 00c4f530
        fsub dword ptr [eax + 034h] // 00c4f537
        mov eax, dword ptr [esp + 08ch] // 00c4f53a
        fstp dword ptr [esp + 078h] // 00c4f541
        fld dword ptr [esp + 0f0h] // 00c4f545
        fsub dword ptr [eax + 02ch] // 00c4f54c
        fstp dword ptr [esp + 098h] // 00c4f54f
        fld dword ptr [esp + 0f4h] // 00c4f556
        fsub dword ptr [eax + 030h] // 00c4f55d
        fstp dword ptr [esp + 09ch] // 00c4f560
        fld dword ptr [esp + 0f8h] // 00c4f567
        fsub dword ptr [eax + 034h] // 00c4f56e
        mov eax, dword ptr [esp + 030h] // 00c4f571
        subss xmm0, dword ptr [eax] // 00c4f575
        movss dword ptr [esp + 0148h], xmm0 // 00c4f579
        fstp dword ptr [esp + 0a0h] // 00c4f582
        mov eax, dword ptr [esp + 0148h] // 00c4f589
        fld dword ptr [esp + 070h] // 00c4f590
        movaps xmm0, xmm3 // 00c4f594
        fld st(0) // 00c4f597
        subss xmm0, dword ptr [edx - 010h] // 00c4f599
        fchs  // 00c4f59e
        subss xmm3, dword ptr [edx - 0ch] // 00c4f5a0
        fstp dword ptr [esp + 0108h] // 00c4f5a5
        mov dword ptr [edi - 010h], eax // 00c4f5ac
        fld dword ptr [esp + 074h] // 00c4f5af
        movss dword ptr [esp + 014ch], xmm0 // 00c4f5b3
        fld st(0) // 00c4f5bc
        mov eax, dword ptr [esp + 014ch] // 00c4f5be
        fchs  // 00c4f5c5
        mov dword ptr [edi - 0ch], eax // 00c4f5c7
        fstp dword ptr [esp + 010ch] // 00c4f5ca
        movss dword ptr [esp + 0150h], xmm3 // 00c4f5d1
        fld dword ptr [esp + 078h] // 00c4f5da
        mov eax, dword ptr [esp + 0150h] // 00c4f5de
        fchs  // 00c4f5e5
        mov dword ptr [edi - 8], eax // 00c4f5e7
        fstp dword ptr [esp + 0110h] // 00c4f5ea
        mov eax, dword ptr [esp + 030h] // 00c4f5f1
        fld dword ptr [esp + 010ch] // 00c4f5f5
        fld st(0) // 00c4f5fc
        fmul dword ptr [edx - 0ch] // 00c4f5fe
        fld dword ptr [esp + 0110h] // 00c4f601
        fld st(0) // 00c4f608
        fmul dword ptr [edx - 010h] // 00c4f60a
        fsubp st(2), st(0) // 00c4f60d
        fxch st(1) // 00c4f60f
        fstp dword ptr [edi - 4] // 00c4f611
        fmul dword ptr [eax] // 00c4f614
        fld dword ptr [esp + 0108h] // 00c4f616
        fld st(0) // 00c4f61d
        fmul dword ptr [edx - 0ch] // 00c4f61f
        fsubp st(2), st(0) // 00c4f622
        fxch st(1) // 00c4f624
        fstp dword ptr [edi] // 00c4f626
        fmul dword ptr [edx - 010h] // 00c4f628
        fld dword ptr [eax] // 00c4f62b
        fmulp st(2), st(0) // 00c4f62d
        fsubrp st(1), st(0) // 00c4f62f
        fstp dword ptr [edi + 4] // 00c4f631
        mov eax, dword ptr [eax] // 00c4f634
        fld dword ptr [esp + 09ch] // 00c4f636
        mov dword ptr [edi + 8], eax // 00c4f63d
        mov eax, dword ptr [esp + 030h] // 00c4f640
        fld st(0) // 00c4f644
        mov eax, dword ptr [eax + 4] // 00c4f646
        mov dword ptr [edi + 0ch], eax // 00c4f649
        mov eax, dword ptr [esp + 030h] // 00c4f64c
        mov eax, dword ptr [eax + 8] // 00c4f650
        mov dword ptr [edi + 010h], eax // 00c4f653
        fmul dword ptr [edx - 0ch] // 00c4f656
        fld dword ptr [esp + 0a0h] // 00c4f659
        fld st(0) // 00c4f660
        fmul dword ptr [edx - 010h] // 00c4f662
        mov eax, dword ptr [esp + 030h] // 00c4f665
        fsubp st(2), st(0) // 00c4f669
        fxch st(1) // 00c4f66b
        fstp dword ptr [edi + 014h] // 00c4f66d
        fld st(0) // 00c4f670
        fmul dword ptr [eax] // 00c4f672
        fld dword ptr [esp + 098h] // 00c4f674
        fld st(0) // 00c4f67b
        fmul dword ptr [edx - 0ch] // 00c4f67d
        fsubp st(2), st(0) // 00c4f680
        fxch st(1) // 00c4f682
        fstp dword ptr [edi + 018h] // 00c4f684
        fld dword ptr [edx - 010h] // 00c4f687
        fmul st(0), st(1) // 00c4f68a
        fld st(3) // 00c4f68c
        fmul dword ptr [eax] // 00c4f68e
        fsubp st(1), st(0) // 00c4f690
        fstp dword ptr [edi + 01ch] // 00c4f692
        fld dword ptr [ebp + 050h] // 00c4f695
        fstp dword ptr [esp + 014h] // 00c4f698
        fld dword ptr [edi - 010h] // 00c4f69c
        fld dword ptr [esp + 014h] // 00c4f69f
        fld st(0) // 00c4f6a3
        fmulp st(2), st(0) // 00c4f6a5
        fxch st(1) // 00c4f6a7
        fstp dword ptr [esp + 0178h] // 00c4f6a9
        mov eax, dword ptr [esp + 0178h] // 00c4f6b0
        fld dword ptr [edi - 0ch] // 00c4f6b7
        fmul st(0), st(1) // 00c4f6ba
        fstp dword ptr [esp + 017ch] // 00c4f6bc
        fmul dword ptr [edi - 8] // 00c4f6c3
        mov dword ptr [ecx - 014h], eax // 00c4f6c6
        mov eax, dword ptr [esp + 017ch] // 00c4f6c9
        mov dword ptr [ecx - 010h], eax // 00c4f6d0
        fstp dword ptr [esp + 0180h] // 00c4f6d3
        mov eax, dword ptr [esp + 0180h] // 00c4f6da
        mov dword ptr [ecx - 0ch], eax // 00c4f6e1
        fld dword ptr [ebp + 06ch] // 00c4f6e4
        fmul dword ptr [edi] // 00c4f6e7
        mov eax, dword ptr [esp + 0b0h] // 00c4f6e9
        fld dword ptr [ebp + 060h] // 00c4f6f0
        fmul dword ptr [edi - 4] // 00c4f6f3
        faddp st(1), st(0) // 00c4f6f6
        fld dword ptr [ebp + 078h] // 00c4f6f8
        fmul dword ptr [edi + 4] // 00c4f6fb
        faddp st(1), st(0) // 00c4f6fe
        fstp dword ptr [ecx - 8] // 00c4f700
        fld dword ptr [ebp + 064h] // 00c4f703
        fmul dword ptr [edi - 4] // 00c4f706
        fld dword ptr [ebp + 070h] // 00c4f709
        fmul dword ptr [edi] // 00c4f70c
        faddp st(1), st(0) // 00c4f70e
        fld dword ptr [ebp + 07ch] // 00c4f710
        fmul dword ptr [edi + 4] // 00c4f713
        faddp st(1), st(0) // 00c4f716
        fstp dword ptr [eax + edi] // 00c4f718
        fld dword ptr [ebp + 068h] // 00c4f71b
        fmul dword ptr [edi - 4] // 00c4f71e
        fld dword ptr [ebp + 074h] // 00c4f721
        fmul dword ptr [edi] // 00c4f724
        faddp st(1), st(0) // 00c4f726
        fld dword ptr [ebp + 080h] // 00c4f728
        fmul dword ptr [edi + 4] // 00c4f72e
        faddp st(1), st(0) // 00c4f731
        fstp dword ptr [ecx] // 00c4f733
        fld dword ptr [ebx + 050h] // 00c4f735
        fstp dword ptr [esp + 014h] // 00c4f738
        fld dword ptr [edi + 8] // 00c4f73c
        fld dword ptr [esp + 014h] // 00c4f73f
        fld st(0) // 00c4f743
        fmulp st(2), st(0) // 00c4f745
        fxch st(1) // 00c4f747
        fstp dword ptr [esp + 0120h] // 00c4f749
        mov eax, dword ptr [esp + 0120h] // 00c4f750
        fld st(0) // 00c4f757
        fmul dword ptr [edi + 0ch] // 00c4f759
        fstp dword ptr [esp + 0124h] // 00c4f75c
        fmul dword ptr [edi + 010h] // 00c4f763
        mov dword ptr [ecx + 4], eax // 00c4f766
        fstp dword ptr [esp + 0128h] // 00c4f769
        mov eax, dword ptr [esp + 0124h] // 00c4f770
        mov dword ptr [ecx + 8], eax // 00c4f777
        mov eax, dword ptr [esp + 0128h] // 00c4f77a
        mov dword ptr [ecx + 0ch], eax // 00c4f781
        fld dword ptr [ebx + 06ch] // 00c4f784
        fmul dword ptr [edi + 018h] // 00c4f787
        fld dword ptr [ebx + 060h] // 00c4f78a
        fmul dword ptr [edi + 014h] // 00c4f78d
        faddp st(1), st(0) // 00c4f790
        fld dword ptr [ebx + 078h] // 00c4f792
        fmul dword ptr [edi + 01ch] // 00c4f795
        faddp st(1), st(0) // 00c4f798
        fstp dword ptr [ecx + 010h] // 00c4f79a
        fld dword ptr [ebx + 064h] // 00c4f79d
        fmul dword ptr [edi + 014h] // 00c4f7a0
        fld dword ptr [ebx + 070h] // 00c4f7a3
        fmul dword ptr [edi + 018h] // 00c4f7a6
        faddp st(1), st(0) // 00c4f7a9
        fld dword ptr [ebx + 07ch] // 00c4f7ab
        fmul dword ptr [edi + 01ch] // 00c4f7ae
        faddp st(1), st(0) // 00c4f7b1
        fstp dword ptr [ecx + 014h] // 00c4f7b3
        fld dword ptr [ebx + 068h] // 00c4f7b6
        fmul dword ptr [edi + 014h] // 00c4f7b9
        fld dword ptr [ebx + 074h] // 00c4f7bc
        fmul dword ptr [edi + 018h] // 00c4f7bf
        faddp st(1), st(0) // 00c4f7c2
        fld dword ptr [ebx + 080h] // 00c4f7c4
        fmul dword ptr [edi + 01ch] // 00c4f7ca
        faddp st(1), st(0) // 00c4f7cd
        fstp dword ptr [ecx + 018h] // 00c4f7cf
        fld dword ptr [ebp + 010h] // 00c4f7d2
        fstp dword ptr [esp + 02ch] // 00c4f7d5
        fld dword ptr [ebp + 014h] // 00c4f7d9
        fstp dword ptr [esp + 028h] // 00c4f7dc
        fld dword ptr [esp + 078h] // 00c4f7e0
        fmul dword ptr [esp + 02ch] // 00c4f7e4
        fld st(4) // 00c4f7e8
        fmul dword ptr [esp + 028h] // 00c4f7ea
        fsubp st(1), st(0) // 00c4f7ee
        fstp dword ptr [esp + 0e4h] // 00c4f7f0
        fld dword ptr [ebp + 0ch] // 00c4f7f7
        fstp dword ptr [esp + 024h] // 00c4f7fa
        fld st(4) // 00c4f7fe
        fmul dword ptr [esp + 028h] // 00c4f800
        fld dword ptr [esp + 078h] // 00c4f804
        fmul dword ptr [esp + 024h] // 00c4f808
        fsubp st(1), st(0) // 00c4f80c
        fstp dword ptr [esp + 0e8h] // 00c4f80e
        fld dword ptr [esp + 024h] // 00c4f815
        fmulp st(4), st(0) // 00c4f819
        fld dword ptr [esp + 02ch] // 00c4f81b
        fmulp st(5), st(0) // 00c4f81f
        fxch st(3) // 00c4f821
        fsubrp st(4), st(0) // 00c4f823
        fxch st(3) // 00c4f825
        fstp dword ptr [esp + 0ech] // 00c4f827
        fld dword ptr [ebx + 010h] // 00c4f82e
        fstp dword ptr [esp + 014h] // 00c4f831
        fld dword ptr [ebx + 014h] // 00c4f835
        fstp dword ptr [esp + 028h] // 00c4f838
        fld st(2) // 00c4f83c
        fld dword ptr [esp + 014h] // 00c4f83e
        fld st(0) // 00c4f842
        fmulp st(2), st(0) // 00c4f844
        fld st(2) // 00c4f846
        fld dword ptr [esp + 028h] // 00c4f848
        fld st(0) // 00c4f84c
        fmulp st(2), st(0) // 00c4f84e
        fxch st(3) // 00c4f850
        fsubrp st(1), st(0) // 00c4f852
        fstp dword ptr [esp + 0160h] // 00c4f854
        fld dword ptr [ebx + 0ch] // 00c4f85b
        fstp dword ptr [esp + 014h] // 00c4f85e
        fld st(3) // 00c4f862
        fmulp st(2), st(0) // 00c4f864
        fld dword ptr [esp + 014h] // 00c4f866
        fld st(0) // 00c4f86a
        fmulp st(6), st(0) // 00c4f86c
        fxch st(2) // 00c4f86e
        fsubrp st(5), st(0) // 00c4f870
        fxch st(4) // 00c4f872
        mov eax, dword ptr [esp + 01b0h] // 00c4f874
        fstp dword ptr [esp + 0164h] // 00c4f87b
        mov eax, dword ptr [eax] // 00c4f882
        fmulp st(1), st(0) // 00c4f884
        fxch st(1) // 00c4f886
        fmulp st(2), st(0) // 00c4f888
        fsubrp st(1), st(0) // 00c4f88a
        fstp dword ptr [esp + 0168h] // 00c4f88c
        fld dword ptr [ebx] // 00c4f893
        fstp dword ptr [esp + 014h] // 00c4f895
        fld dword ptr [esp + 0160h] // 00c4f899
        fld dword ptr [esp + 014h] // 00c4f8a0
        fld st(0) // 00c4f8a4
        faddp st(2), st(0) // 00c4f8a6
        fxch st(1) // 00c4f8a8
        fstp dword ptr [esp + 0c4h] // 00c4f8aa
        fld dword ptr [ebx + 4] // 00c4f8b1
        fstp dword ptr [esp + 014h] // 00c4f8b4
        fld dword ptr [esp + 0164h] // 00c4f8b8
        fld dword ptr [esp + 014h] // 00c4f8bf
        fld st(0) // 00c4f8c3
        faddp st(2), st(0) // 00c4f8c5
        fxch st(1) // 00c4f8c7
        fstp dword ptr [esp + 0c8h] // 00c4f8c9
        fld dword ptr [ebx + 8] // 00c4f8d0
        fstp dword ptr [esp + 014h] // 00c4f8d3
        fld dword ptr [esp + 0168h] // 00c4f8d7
        fld dword ptr [esp + 014h] // 00c4f8de
        fld st(0) // 00c4f8e2
        faddp st(2), st(0) // 00c4f8e4
        fxch st(1) // 00c4f8e6
        fstp dword ptr [esp + 0cch] // 00c4f8e8
        fld dword ptr [ebp] // 00c4f8ef
        fstp dword ptr [esp + 014h] // 00c4f8f2
        fld dword ptr [esp + 0c4h] // 00c4f8f6
        fld dword ptr [esp + 014h] // 00c4f8fd
        fld st(0) // 00c4f901
        fsubp st(2), st(0) // 00c4f903
        fxch st(1) // 00c4f905
        fstp dword ptr [esp + 0d4h] // 00c4f907
        fld dword ptr [ebp + 4] // 00c4f90e
        fstp dword ptr [esp + 024h] // 00c4f911
        fld dword ptr [esp + 0c8h] // 00c4f915
        fsub dword ptr [esp + 024h] // 00c4f91c
        fstp dword ptr [esp + 0d8h] // 00c4f920
        fld dword ptr [ebp + 8] // 00c4f927
        fstp dword ptr [esp + 02ch] // 00c4f92a
        fld dword ptr [esp + 0cch] // 00c4f92e
        fsub dword ptr [esp + 02ch] // 00c4f935
        fstp dword ptr [esp + 0dch] // 00c4f939
        fld dword ptr [esp + 0d4h] // 00c4f940
        fsub dword ptr [esp + 0e4h] // 00c4f947
        fstp dword ptr [esp + 07ch] // 00c4f94e
        fld dword ptr [esp + 0d8h] // 00c4f952
        fsub dword ptr [esp + 0e8h] // 00c4f959
        fstp dword ptr [esp + 080h] // 00c4f960
        fld dword ptr [esp + 0dch] // 00c4f967
        fsub dword ptr [esp + 0ech] // 00c4f96e
        fstp dword ptr [esp + 084h] // 00c4f975
        fld dword ptr [eax + 018h] // 00c4f97c
        mov eax, dword ptr [esp + 030h] // 00c4f97f
        fmul dword ptr [esp + 068h] // 00c4f983
        fdiv dword ptr [esp + 01b4h] // 00c4f987
        fstp dword ptr [esp + 018h] // 00c4f98e
        fld dword ptr [esp + 080h] // 00c4f992
        fmul dword ptr [edx - 010h] // 00c4f999
        fld dword ptr [esp + 07ch] // 00c4f99c
        fmul dword ptr [eax] // 00c4f9a0
        faddp st(1), st(0) // 00c4f9a2
        fld dword ptr [esp + 084h] // 00c4f9a4
        fmul dword ptr [edx - 0ch] // 00c4f9ab
        mov edx, dword ptr [esp + 060h] // 00c4f9ae
        faddp st(1), st(0) // 00c4f9b2
        fstp dword ptr [esp + 014h] // 00c4f9b4
        fld dword ptr [esp + 014h] // 00c4f9b8
        fmul dword ptr [edx + 4] // 00c4f9bc
        fadd qword ptr constant_00d7a270 // 00c4f9bf
        fstp dword ptr [esp + 028h] // 00c4f9c5
        fld dword ptr [esp + 028h] // 00c4f9c9
        fldz  // 00c4f9cd
        fcomip st(0), st(1) // 00c4f9cf
        fstp st(0) // 00c4f9d1
        jbe l_00c4f9e6 // 00c4f9d3
        movss xmm0, dword ptr [esp + 028h] // 00c4f9d5
        movss dword ptr [esp + 088h], xmm0 // 00c4f9db
        jmp l_00c4f9ef // 00c4f9e4
    l_00c4f9e6:
        movss dword ptr [esp + 088h], xmm1 // 00c4f9e6
    l_00c4f9ef:
        fld dword ptr [esp + 088h] // 00c4f9ef
        mov eax, dword ptr [esp + 01b0h] // 00c4f9f6
        fchs  // 00c4f9fd
        mov edx, dword ptr [eax + 0ea90h] // 00c4f9ff
        fld dword ptr [ebp + 010h] // 00c4fa05
        mov eax, dword ptr [esp + 048h] // 00c4fa08
        fmul dword ptr [edi] // 00c4fa0c
        movss xmm0, dword ptr [esp + 018h] // 00c4fa0e
        fld dword ptr [ebp + 0ch] // 00c4fa14
        fmul dword ptr [edi - 4] // 00c4fa17
        faddp st(1), st(0) // 00c4fa1a
        fld dword ptr [ebp + 014h] // 00c4fa1c
        fmul dword ptr [edi + 4] // 00c4fa1f
        faddp st(1), st(0) // 00c4fa22
        fstp dword ptr [esp + 014h] // 00c4fa24
        fld dword ptr [esp + 014h] // 00c4fa28
        fld dword ptr [edi - 0ch] // 00c4fa2c
        fmul dword ptr [esp + 024h] // 00c4fa2f
        fld dword ptr [edi - 010h] // 00c4fa33
        fmulp st(4), st(0) // 00c4fa36
        faddp st(3), st(0) // 00c4fa38
        fld dword ptr [edi - 8] // 00c4fa3a
        fmul dword ptr [esp + 02ch] // 00c4fa3d
        faddp st(3), st(0) // 00c4fa41
        fxch st(2) // 00c4fa43
        fstp dword ptr [esp + 014h] // 00c4fa45
        fld dword ptr [esp + 014h] // 00c4fa49
        faddp st(2), st(0) // 00c4fa4d
        fld dword ptr [edi + 8] // 00c4fa4f
        fmulp st(5), st(0) // 00c4fa52
        fld dword ptr [edi + 0ch] // 00c4fa54
        fmulp st(4), st(0) // 00c4fa57
        fxch st(4) // 00c4fa59
        faddp st(3), st(0) // 00c4fa5b
        fld dword ptr [edi + 010h] // 00c4fa5d
        fmulp st(2), st(0) // 00c4fa60
        fxch st(2) // 00c4fa62
        faddp st(1), st(0) // 00c4fa64
        fstp dword ptr [esp + 014h] // 00c4fa66
        fadd dword ptr [esp + 014h] // 00c4fa6a
        fld dword ptr [ebx + 010h] // 00c4fa6e
        fmul dword ptr [edi + 018h] // 00c4fa71
        fld dword ptr [ebx + 0ch] // 00c4fa74
        fmul dword ptr [edi + 014h] // 00c4fa77
        faddp st(1), st(0) // 00c4fa7a
        fld dword ptr [ebx + 014h] // 00c4fa7c
        fmul dword ptr [edi + 01ch] // 00c4fa7f
        faddp st(1), st(0) // 00c4fa82
        fstp dword ptr [esp + 014h] // 00c4fa84
        fadd dword ptr [esp + 014h] // 00c4fa88
        fsubp st(1), st(0) // 00c4fa8c
        fstp dword ptr [esp + 014h] // 00c4fa8e
        fld dword ptr [esp + 014h] // 00c4fa92
        fstp dword ptr [eax + edx] // 00c4fa96
        mov edx, dword ptr [esp + 01b0h] // 00c4fa99
        mov edx, dword ptr [edx + 0ea94h] // 00c4faa0
        movss dword ptr [eax + edx], xmm0 // 00c4faa6
        fld dword ptr [ecx - 010h] // 00c4faab
        fmul dword ptr [edi - 0ch] // 00c4faae
        mov edx, dword ptr [esp + 0b0h] // 00c4fab1
        fld dword ptr [edi - 010h] // 00c4fab8
        fmul dword ptr [ecx - 014h] // 00c4fabb
        faddp st(1), st(0) // 00c4fabe
        fld dword ptr [ecx - 0ch] // 00c4fac0
        fmul dword ptr [edi - 8] // 00c4fac3
        faddp st(1), st(0) // 00c4fac6
        fstp dword ptr [esp + 018h] // 00c4fac8
        fld dword ptr [esp + 018h] // 00c4facc
        fld dword ptr [edi - 4] // 00c4fad0
        fmul dword ptr [ecx - 8] // 00c4fad3
        fld dword ptr [edi] // 00c4fad6
        fmul dword ptr [edx + edi] // 00c4fad8
        faddp st(1), st(0) // 00c4fadb
        fld dword ptr [edi + 4] // 00c4fadd
        fmul dword ptr [ecx] // 00c4fae0
        faddp st(1), st(0) // 00c4fae2
        fstp dword ptr [esp + 018h] // 00c4fae4
        fadd dword ptr [esp + 018h] // 00c4fae8
        fld dword ptr [ecx + 8] // 00c4faec
        fmul dword ptr [edi + 0ch] // 00c4faef
        fld dword ptr [edi + 8] // 00c4faf2
        fmul dword ptr [ecx + 4] // 00c4faf5
        faddp st(1), st(0) // 00c4faf8
        fld dword ptr [ecx + 0ch] // 00c4fafa
        fmul dword ptr [edi + 010h] // 00c4fafd
        faddp st(1), st(0) // 00c4fb00
        fstp dword ptr [esp + 018h] // 00c4fb02
        fadd dword ptr [esp + 018h] // 00c4fb06
        fld dword ptr [ecx + 014h] // 00c4fb0a
        fmul dword ptr [edi + 018h] // 00c4fb0d
        fld dword ptr [edi + 014h] // 00c4fb10
        fmul dword ptr [ecx + 010h] // 00c4fb13
        faddp st(1), st(0) // 00c4fb16
        fld dword ptr [ecx + 018h] // 00c4fb18
        mov ecx, dword ptr [esp + 01b0h] // 00c4fb1b
        fmul dword ptr [edi + 01ch] // 00c4fb22
        mov edx, dword ptr [ecx + 0ea8ch] // 00c4fb25
        faddp st(1), st(0) // 00c4fb2b
        fstp dword ptr [esp + 018h] // 00c4fb2d
        fadd dword ptr [esp + 018h] // 00c4fb31
        fld1  // 00c4fb35
        fdivrp st(1), st(0) // 00c4fb37
        fstp dword ptr [esp + 018h] // 00c4fb39
        fld dword ptr [esp + 018h] // 00c4fb3d
        fstp dword ptr [eax + edx] // 00c4fb41
        mov ecx, dword ptr [ecx + 0eabch] // 00c4fb44
        mov edx, dword ptr [esp + 030h] // 00c4fb4a
        movss dword ptr [eax + ecx], xmm1 // 00c4fb4e
        fld dword ptr [edx] // 00c4fb53
        fstp dword ptr [esp + 014h] // 00c4fb55
        push ecx // 00c4fb59
        fld dword ptr [esp + 018h] // 00c4fb5a
        fstp dword ptr [esp] // 00c4fb5e
        call abs_kernel // 00c4fb61
        mov eax, dword ptr [esp + 034h] // 00c4fb66
        fstp dword ptr [esp + 02ch] // 00c4fb6a
        fld dword ptr [eax - 010h] // 00c4fb6e
        push ecx // 00c4fb71
        fstp dword ptr [esp] // 00c4fb72
        mov dword ptr [esp + 028h], 0 // 00c4fb75
        call abs_kernel // 00c4fb7d
        fstp dword ptr [esp + 028h] // 00c4fb82
        fld dword ptr [esp + 02ch] // 00c4fb86
        fld dword ptr [esp + 028h] // 00c4fb8a
        fcomip st(0), st(1) // 00c4fb8e
        fstp st(0) // 00c4fb90
        jbe l_00c4fba8 // 00c4fb92
        movss xmm0, dword ptr [esp + 028h] // 00c4fb94
        movss dword ptr [esp + 02ch], xmm0 // 00c4fb9a
        mov dword ptr [esp + 024h], 1 // 00c4fba0
    l_00c4fba8:
        fld dword ptr [esp + 02ch] // 00c4fba8
        push ecx // 00c4fbac
        mov ecx, dword ptr [esp + 038h] // 00c4fbad
        fstp qword ptr [esp + 01ch] // 00c4fbb1
        fld dword ptr [ecx - 0ch] // 00c4fbb5
        fstp dword ptr [esp] // 00c4fbb8
        call abs_kernel // 00c4fbbb
        fld qword ptr [esp + 018h] // 00c4fbc0
        fxch st(1) // 00c4fbc4
        fcomip st(0), st(1) // 00c4fbc6
        fstp st(0) // 00c4fbc8
        jbe l_00c4fbd4 // 00c4fbca
        mov dword ptr [esp + 024h], 2 // 00c4fbcc
    l_00c4fbd4:
        mov ecx, dword ptr [esp + 024h] // 00c4fbd4
        mov edx, dword ptr [esp + 030h] // 00c4fbd8
        movss xmm0, dword ptr constant_00d7a208 // 00c4fbdc
        mov eax, 1 // 00c4fbe4
        shl eax, cl // 00c4fbe9
        mov ecx, dword ptr [edx] // 00c4fbeb
        mov dword ptr [esp + 04ch], ecx // 00c4fbed
        mov ecx, dword ptr [edx + 4] // 00c4fbf1
        mov edx, dword ptr [edx + 8] // 00c4fbf4
        mov dword ptr [esp + 054h], edx // 00c4fbf7
        mov dword ptr [esp + 050h], ecx // 00c4fbfb
        and eax, 3 // 00c4fbff
        mov ecx, dword ptr [esp + eax*4 + 04ch] // 00c4fc02
        lea edx, [esp + eax*4 + 04ch] // 00c4fc06
        mov dword ptr [esp + 018h], eax // 00c4fc0a
        mov eax, dword ptr [esp + 024h] // 00c4fc0e
        xor dword ptr [esp + eax*4 + 04ch], ecx // 00c4fc12
        mov ecx, dword ptr [esp + eax*4 + 04ch] // 00c4fc16
        xor dword ptr [edx], ecx // 00c4fc1a
        mov edx, dword ptr [edx] // 00c4fc1c
        xor dword ptr [esp + eax*4 + 04ch], edx // 00c4fc1e
        mov ecx, dword ptr [esp + 018h] // 00c4fc22
        subss xmm0, dword ptr [esp + eax*4 + 04ch] // 00c4fc26
        lea eax, [esp + eax*4 + 04ch] // 00c4fc2c
        mov edx, 1 // 00c4fc30
        shl edx, cl // 00c4fc35
        movss dword ptr [eax], xmm0 // 00c4fc37
        xorps xmm0, xmm0 // 00c4fc3b
        push ecx // 00c4fc3e
        and edx, 3 // 00c4fc3f
        movss dword ptr [esp + edx*4 + 050h], xmm0 // 00c4fc42
        fld dword ptr [esp + 054h] // 00c4fc48
        fld dword ptr [esp + 050h] // 00c4fc4c
        fld dword ptr [esp + 058h] // 00c4fc50
        fld st(1) // 00c4fc54
        fmulp st(2), st(0) // 00c4fc56
        fld st(2) // 00c4fc58
        fmulp st(3), st(0) // 00c4fc5a
        fxch st(1) // 00c4fc5c
        faddp st(2), st(0) // 00c4fc5e
        fmul st(0), st(0) // 00c4fc60
        faddp st(1), st(0) // 00c4fc62
        fstp dword ptr [esp + 01ch] // 00c4fc64
        fld dword ptr [esp + 01ch] // 00c4fc68
        fstp dword ptr [esp] // 00c4fc6c
        push dword ptr [esp+444] // Borrowed context; original frame offsets retained.
        call sqrt_kernel // 00c4fc6f
        fstp dword ptr [esp + 018h] // 00c4fc74
        mov eax, dword ptr [esp + 034h] // 00c4fc78
        fld dword ptr [esp + 04ch] // 00c4fc7c
        fld dword ptr [esp + 018h] // 00c4fc80
        fld st(0) // 00c4fc84
        fdivp st(2), st(0) // 00c4fc86
        fxch st(1) // 00c4fc88
        fstp dword ptr [esp + 04ch] // 00c4fc8a
        fld dword ptr [esp + 050h] // 00c4fc8e
        fdiv st(0), st(1) // 00c4fc92
        fstp dword ptr [esp + 050h] // 00c4fc94
        fdivr dword ptr [esp + 054h] // 00c4fc98
        fstp dword ptr [esp + 054h] // 00c4fc9c
        fld dword ptr [eax - 0ch] // 00c4fca0
        fstp dword ptr [esp + 018h] // 00c4fca3
        fld dword ptr [eax - 010h] // 00c4fca7
        fstp dword ptr [esp + 028h] // 00c4fcaa
        fld dword ptr [esp + 050h] // 00c4fcae
        fld st(0) // 00c4fcb2
        fld dword ptr [esp + 018h] // 00c4fcb4
        fld st(0) // 00c4fcb8
        fmulp st(2), st(0) // 00c4fcba
        fld dword ptr [esp + 054h] // 00c4fcbc
        fld st(0) // 00c4fcc0
        fld dword ptr [esp + 028h] // 00c4fcc2
        fld st(0) // 00c4fcc6
        fmulp st(2), st(0) // 00c4fcc8
        fxch st(4) // 00c4fcca
        fsubrp st(1), st(0) // 00c4fccc
        fstp dword ptr [esp + 0b4h] // 00c4fcce
        fld st(0) // 00c4fcd5
        fld dword ptr [esp + 014h] // 00c4fcd7
        fld st(0) // 00c4fcdb
        fmulp st(2), st(0) // 00c4fcdd
        fld dword ptr [esp + 04ch] // 00c4fcdf
        fld st(0) // 00c4fce3
        fmulp st(5), st(0) // 00c4fce5
        fxch st(2) // 00c4fce7
        fsubrp st(4), st(0) // 00c4fce9
        fxch st(3) // 00c4fceb
        fstp dword ptr [esp + 0b8h] // 00c4fced
        fld st(0) // 00c4fcf4
        fmulp st(4), st(0) // 00c4fcf6
        fld st(4) // 00c4fcf8
        fmulp st(3), st(0) // 00c4fcfa
        fxch st(3) // 00c4fcfc
        fsubrp st(2), st(0) // 00c4fcfe
        fxch st(1) // 00c4fd00
        fstp dword ptr [esp + 0bch] // 00c4fd02
        fld dword ptr [esp + 0b8h] // 00c4fd09
        fld dword ptr [esp + 080h] // 00c4fd10
        fld st(0) // 00c4fd17
        fmulp st(2), st(0) // 00c4fd19
        fld dword ptr [esp + 0b4h] // 00c4fd1b
        fld dword ptr [esp + 07ch] // 00c4fd22
        fld st(0) // 00c4fd26
        fmulp st(2), st(0) // 00c4fd28
        fxch st(3) // 00c4fd2a
        faddp st(1), st(0) // 00c4fd2c
        fld dword ptr [esp + 0bch] // 00c4fd2e
        fmul dword ptr [esp + 084h] // 00c4fd35
        faddp st(1), st(0) // 00c4fd3c
        fstp dword ptr [esp + 018h] // 00c4fd3e
        fld dword ptr [esp + 0b4h] // 00c4fd42
        fld dword ptr [esp + 018h] // 00c4fd49
        fld st(0) // 00c4fd4d
        fmulp st(2), st(0) // 00c4fd4f
        fxch st(1) // 00c4fd51
        fstp dword ptr [esp + 0fch] // 00c4fd53
        fld dword ptr [esp + 0b8h] // 00c4fd5a
        fmul st(0), st(1) // 00c4fd61
        fstp dword ptr [esp + 0100h] // 00c4fd63
        fmul dword ptr [esp + 0bch] // 00c4fd6a
        fstp dword ptr [esp + 0104h] // 00c4fd71
        fmul st(0), st(4) // 00c4fd78
        fld st(3) // 00c4fd7a
        fmulp st(2), st(0) // 00c4fd7c
        faddp st(1), st(0) // 00c4fd7e
        fld st(1) // 00c4fd80
        fmul dword ptr [esp + 084h] // 00c4fd82
        faddp st(1), st(0) // 00c4fd89
        fstp dword ptr [esp + 018h] // 00c4fd8b
        fld dword ptr [esp + 018h] // 00c4fd8f
        fld st(0) // 00c4fd93
        fmulp st(3), st(0) // 00c4fd95
        fxch st(2) // 00c4fd97
        fstp dword ptr [esp + 0114h] // 00c4fd99
        fld st(1) // 00c4fda0
        fmulp st(3), st(0) // 00c4fda2
        fxch st(2) // 00c4fda4
        fstp dword ptr [esp + 0118h] // 00c4fda6
        fmulp st(1), st(0) // 00c4fdad
        fstp dword ptr [esp + 011ch] // 00c4fdaf
        fld dword ptr [esp + 0114h] // 00c4fdb6
        fadd dword ptr [esp + 0fch] // 00c4fdbd
        fstp dword ptr [esp + 03ch] // 00c4fdc4
        fld dword ptr [esp + 0100h] // 00c4fdc8
        fadd dword ptr [esp + 0118h] // 00c4fdcf
        fstp dword ptr [esp + 040h] // 00c4fdd6
        fld dword ptr [esp + 0104h] // 00c4fdda
        fadd dword ptr [esp + 011ch] // 00c4fde1
        fstp dword ptr [esp + 044h] // 00c4fde8
        fld dword ptr [esp + 03ch] // 00c4fdec
        fld dword ptr [esp + 040h] // 00c4fdf0
        fld dword ptr [esp + 044h] // 00c4fdf4
        fld st(1) // 00c4fdf8
        fmulp st(2), st(0) // 00c4fdfa
        fld st(2) // 00c4fdfc
        fmulp st(3), st(0) // 00c4fdfe
        fxch st(1) // 00c4fe00
        faddp st(2), st(0) // 00c4fe02
        fmul st(0), st(0) // 00c4fe04
        faddp st(1), st(0) // 00c4fe06
        fstp dword ptr [esp + 018h] // 00c4fe08
        fld dword ptr constant_00d7a288 // 00c4fe0c
        fld dword ptr [esp + 018h] // 00c4fe12
        fcomi st(0), st(1) // 00c4fe16
        fstp st(1) // 00c4fe18
        jbe l_00c4fe4f // 00c4fe1a
        push ecx // 00c4fe1c
        fstp dword ptr [esp] // 00c4fe1d
        push dword ptr [esp+444] // Borrowed context; original frame offsets retained.
        call sqrt_kernel // 00c4fe20
        fstp dword ptr [esp + 018h] // 00c4fe25
        fld dword ptr [esp + 03ch] // 00c4fe29
        fld dword ptr [esp + 018h] // 00c4fe2d
        fld st(0) // 00c4fe31
        fdivp st(2), st(0) // 00c4fe33
        fxch st(1) // 00c4fe35
        fstp dword ptr [esp + 03ch] // 00c4fe37
        fld dword ptr [esp + 040h] // 00c4fe3b
        fdiv st(0), st(1) // 00c4fe3f
        fstp dword ptr [esp + 040h] // 00c4fe41
        fdivr dword ptr [esp + 044h] // 00c4fe45
        fstp dword ptr [esp + 044h] // 00c4fe49
        jmp l_00c4fe69 // 00c4fe4d
    l_00c4fe4f:
        mov eax, dword ptr [esp + 04ch] // 00c4fe4f
        fstp st(0) // 00c4fe53
        mov ecx, dword ptr [esp + 050h] // 00c4fe55
        mov edx, dword ptr [esp + 054h] // 00c4fe59
        mov dword ptr [esp + 03ch], eax // 00c4fe5d
        mov dword ptr [esp + 040h], ecx // 00c4fe61
        mov dword ptr [esp + 044h], edx // 00c4fe65
    l_00c4fe69:
        fld dword ptr [esp + 074h] // 00c4fe69
        movss xmm3, dword ptr constant_00d7a208 // 00c4fe6d
        fld st(0) // 00c4fe75
        movaps xmm0, xmm3 // 00c4fe77
        fld dword ptr [esp + 044h] // 00c4fe7a
        subss xmm0, dword ptr [esp + 03ch] // 00c4fe7e
        fld st(0) // 00c4fe84
        movss dword ptr [esp + 012ch], xmm0 // 00c4fe86
        fmulp st(2), st(0) // 00c4fe8f
        mov eax, dword ptr [esp + 012ch] // 00c4fe91
        fld dword ptr [esp + 078h] // 00c4fe98
        movaps xmm0, xmm3 // 00c4fe9c
        fld st(0) // 00c4fe9f
        subss xmm0, dword ptr [esp + 040h] // 00c4fea1
        fld dword ptr [esp + 040h] // 00c4fea7
        movss dword ptr [esp + 0130h], xmm0 // 00c4feab
        fld st(0) // 00c4feb4
        mov ecx, dword ptr [esp + 0130h] // 00c4feb6
        fmulp st(2), st(0) // 00c4febd
        movaps xmm0, xmm3 // 00c4febf
        fxch st(4) // 00c4fec2
        subss xmm0, dword ptr [esp + 044h] // 00c4fec4
        movss dword ptr [esp + 0134h], xmm0 // 00c4feca
        fsubrp st(1), st(0) // 00c4fed3
        mov edx, dword ptr [esp + 0134h] // 00c4fed5
        mov dword ptr [esi - 01ch], eax // 00c4fedc
        movaps xmm0, xmm3 // 00c4fedf
        fstp dword ptr [esp + 013ch] // 00c4fee2
        mov dword ptr [esi - 018h], ecx // 00c4fee9
        fld dword ptr [esp + 03ch] // 00c4feec
        subss xmm0, dword ptr [esp + 013ch] // 00c4fef0
        fld st(0) // 00c4fef9
        movss dword ptr [esp + 0154h], xmm0 // 00c4fefb
        fmulp st(2), st(0) // 00c4ff04
        mov eax, dword ptr [esp + 0154h] // 00c4ff06
        fld dword ptr [esp + 070h] // 00c4ff0d
        mov dword ptr [esi - 014h], edx // 00c4ff11
        fld st(0) // 00c4ff14
        mov dword ptr [esi - 010h], eax // 00c4ff16
        fmul st(0), st(4) // 00c4ff19
        mov eax, dword ptr [esp + 03ch] // 00c4ff1b
        mov dword ptr [esi - 4], eax // 00c4ff1f
        fsubp st(3), st(0) // 00c4ff22
        fxch st(2) // 00c4ff24
        fstp dword ptr [esp + 0140h] // 00c4ff26
        fld st(3) // 00c4ff2d
        fmulp st(2), st(0) // 00c4ff2f
        fld st(0) // 00c4ff31
        fmulp st(5), st(0) // 00c4ff33
        fxch st(1) // 00c4ff35
        fsubrp st(4), st(0) // 00c4ff37
        fxch st(3) // 00c4ff39
        fstp dword ptr [esp + 0144h] // 00c4ff3b
        fld dword ptr [esp + 0140h] // 00c4ff42
        fchs  // 00c4ff49
        fstp dword ptr [esp + 0158h] // 00c4ff4b
        mov ecx, dword ptr [esp + 0158h] // 00c4ff52
        fld dword ptr [esp + 0144h] // 00c4ff59
        mov dword ptr [esi - 0ch], ecx // 00c4ff60
        fchs  // 00c4ff63
        mov ecx, dword ptr [esp + 040h] // 00c4ff65
        fstp dword ptr [esp + 015ch] // 00c4ff69
        mov edx, dword ptr [esp + 015ch] // 00c4ff70
        fld dword ptr [esp + 09ch] // 00c4ff77
        mov dword ptr [esi - 8], edx // 00c4ff7e
        fld st(0) // 00c4ff81
        mov edx, dword ptr [esp + 044h] // 00c4ff83
        fmul st(0), st(2) // 00c4ff87
        mov dword ptr [esi], ecx // 00c4ff89
        fld dword ptr [esp + 0a0h] // 00c4ff8b
        mov dword ptr [esi + 4], edx // 00c4ff92
        fld st(0) // 00c4ff95
        fmul st(0), st(5) // 00c4ff97
        fsubp st(2), st(0) // 00c4ff99
        fxch st(1) // 00c4ff9b
        fstp dword ptr [esp + 016ch] // 00c4ff9d
        fmul st(0), st(4) // 00c4ffa4
        fld dword ptr [esp + 098h] // 00c4ffa6
        fld st(0) // 00c4ffad
        fmulp st(4), st(0) // 00c4ffaf
        fxch st(1) // 00c4ffb1
        fsubrp st(3), st(0) // 00c4ffb3
        mov eax, dword ptr [esp + 016ch] // 00c4ffb5
        mov dword ptr [esi + 8], eax // 00c4ffbc
        fxch st(2) // 00c4ffbf
        mov eax, dword ptr [esp + 0ach] // 00c4ffc1
        fstp dword ptr [esp + 0170h] // 00c4ffc8
        mov ecx, dword ptr [esp + 0170h] // 00c4ffcf
        mov dword ptr [esi + 0ch], ecx // 00c4ffd6
        fxch st(1) // 00c4ffd9
        fmulp st(2), st(0) // 00c4ffdb
        fmulp st(2), st(0) // 00c4ffdd
        fsubrp st(1), st(0) // 00c4ffdf
        fstp dword ptr [esp + 0174h] // 00c4ffe1
        mov edx, dword ptr [esp + 0174h] // 00c4ffe8
        mov dword ptr [esi + 010h], edx // 00c4ffef
        fld dword ptr [ebp + 050h] // 00c4fff2
        fstp dword ptr [esp + 018h] // 00c4fff5
        fld dword ptr [esp + 018h] // 00c4fff9
        fld st(0) // 00c4fffd
        fmul dword ptr [esi - 01ch] // 00c4ffff
        fstp dword ptr [esp + 0184h] // 00c50002
        mov ecx, dword ptr [esp + 0184h] // 00c50009
        fld st(0) // 00c50010
        fmul dword ptr [esi - 018h] // 00c50012
        fstp dword ptr [esp + 0188h] // 00c50015
        mov edx, dword ptr [esp + 0188h] // 00c5001c
        fmul dword ptr [esi - 014h] // 00c50023
        mov dword ptr [eax - 028h], ecx // 00c50026
        mov dword ptr [eax - 024h], edx // 00c50029
        mov edx, dword ptr [esp + 05ch] // 00c5002c
        fstp dword ptr [esp + 018ch] // 00c50030
        mov ecx, dword ptr [esp + 018ch] // 00c50037
        mov dword ptr [eax - 020h], ecx // 00c5003e
        fld dword ptr [ebp + 06ch] // 00c50041
        fmul dword ptr [edi + edx] // 00c50044
        mov ecx, dword ptr [esp + 058h] // 00c50047
        fld dword ptr [esi - 010h] // 00c5004b
        mov edx, dword ptr [esp + 038h] // 00c5004e
        fmul dword ptr [ebp + 060h] // 00c50052
        faddp st(1), st(0) // 00c50055
        fld dword ptr [ebp + 078h] // 00c50057
        fmul dword ptr [ecx + edx] // 00c5005a
        mov edx, dword ptr [esp + 05ch] // 00c5005d
        faddp st(1), st(0) // 00c50061
        fstp dword ptr [eax - 01ch] // 00c50063
        fld dword ptr [ebp + 070h] // 00c50066
        fmul dword ptr [edi + edx] // 00c50069
        mov edx, dword ptr [esp + 038h] // 00c5006c
        fld dword ptr [ebp + 064h] // 00c50070
        fmul dword ptr [esi - 010h] // 00c50073
        faddp st(1), st(0) // 00c50076
        fld dword ptr [ebp + 07ch] // 00c50078
        fmul dword ptr [ecx + edx] // 00c5007b
        mov edx, dword ptr [esp + 094h] // 00c5007e
        faddp st(1), st(0) // 00c50085
        fstp dword ptr [edi + edx] // 00c50087
        mov edx, dword ptr [esp + 05ch] // 00c5008a
        fld dword ptr [ebp + 074h] // 00c5008e
        fmul dword ptr [edi + edx] // 00c50091
        mov edx, dword ptr [esp + 038h] // 00c50094
        fld dword ptr [ebp + 068h] // 00c50098
        fmul dword ptr [esi - 010h] // 00c5009b
        faddp st(1), st(0) // 00c5009e
        fld dword ptr [ebp + 080h] // 00c500a0
        fmul dword ptr [ecx + edx] // 00c500a6
        mov edx, dword ptr [esp + 0a4h] // 00c500a9
        faddp st(1), st(0) // 00c500b0
        fstp dword ptr [ecx + edx] // 00c500b2
        fld dword ptr [ebx + 050h] // 00c500b5
        fstp dword ptr [esp + 018h] // 00c500b8
        fld dword ptr [esp + 018h] // 00c500bc
        fld st(0) // 00c500c0
        fmul dword ptr [esi - 4] // 00c500c2
        fstp dword ptr [esp + 019ch] // 00c500c5
        mov edx, dword ptr [esp + 019ch] // 00c500cc
        fld st(0) // 00c500d3
        fmul dword ptr [esi] // 00c500d5
        fstp dword ptr [esp + 01a0h] // 00c500d7
        fmul dword ptr [esi + 4] // 00c500de
        mov dword ptr [eax - 010h], edx // 00c500e1
        fstp dword ptr [esp + 01a4h] // 00c500e4
        mov edx, dword ptr [esp + 01a0h] // 00c500eb
        mov dword ptr [eax - 0ch], edx // 00c500f2
        mov edx, dword ptr [esp + 01a4h] // 00c500f5
        mov dword ptr [eax - 8], edx // 00c500fc
        fld dword ptr [ebx + 06ch] // 00c500ff
        fmul dword ptr [esi + 0ch] // 00c50102
        mov edx, dword ptr [esp + 05ch] // 00c50105
        fld dword ptr [esi + 8] // 00c50109
        fmul dword ptr [ebx + 060h] // 00c5010c
        faddp st(1), st(0) // 00c5010f
        fld dword ptr [ebx + 078h] // 00c50111
        fmul dword ptr [esi + 010h] // 00c50114
        faddp st(1), st(0) // 00c50117
        fstp dword ptr [eax - 4] // 00c50119
        fld dword ptr [ebx + 070h] // 00c5011c
        fmul dword ptr [esi + 0ch] // 00c5011f
        fld dword ptr [ebx + 064h] // 00c50122
        fmul dword ptr [esi + 8] // 00c50125
        faddp st(1), st(0) // 00c50128
        fld dword ptr [ebx + 07ch] // 00c5012a
        fmul dword ptr [esi + 010h] // 00c5012d
        faddp st(1), st(0) // 00c50130
        fstp dword ptr [eax] // 00c50132
        fld dword ptr [ebx + 074h] // 00c50134
        fmul dword ptr [esi + 0ch] // 00c50137
        fld dword ptr [ebx + 068h] // 00c5013a
        fmul dword ptr [esi + 8] // 00c5013d
        faddp st(1), st(0) // 00c50140
        fld dword ptr [ebx + 080h] // 00c50142
        fmul dword ptr [esi + 010h] // 00c50148
        faddp st(1), st(0) // 00c5014b
        fstp dword ptr [eax + 4] // 00c5014d
        fld dword ptr [ebp + 010h] // 00c50150
        fmul dword ptr [edi + edx] // 00c50153
        mov edx, dword ptr [esp + 038h] // 00c50156
        fld dword ptr [esi - 010h] // 00c5015a
        fmul dword ptr [ebp + 0ch] // 00c5015d
        faddp st(1), st(0) // 00c50160
        fld dword ptr [ebp + 014h] // 00c50162
        fmul dword ptr [ecx + edx] // 00c50165
        mov ecx, dword ptr [esp + 01b0h] // 00c50168
        mov edx, dword ptr [ecx + 0eaach] // 00c5016f
        mov ecx, dword ptr [esp + 048h] // 00c50175
        faddp st(1), st(0) // 00c50179
        fstp dword ptr [esp + 018h] // 00c5017b
        fld dword ptr [esp + 018h] // 00c5017f
        fld dword ptr [esi - 01ch] // 00c50183
        fmul dword ptr [ebp] // 00c50186
        fld dword ptr [esi - 018h] // 00c50189
        fmul dword ptr [ebp + 4] // 00c5018c
        faddp st(1), st(0) // 00c5018f
        fld dword ptr [ebp + 8] // 00c50191
        fmul dword ptr [esi - 014h] // 00c50194
        faddp st(1), st(0) // 00c50197
        fstp dword ptr [esp + 018h] // 00c50199
        fadd dword ptr [esp + 018h] // 00c5019d
        fld dword ptr [ebx + 4] // 00c501a1
        fmul dword ptr [esi] // 00c501a4
        fld dword ptr [esi - 4] // 00c501a6
        fmul dword ptr [ebx] // 00c501a9
        faddp st(1), st(0) // 00c501ab
        fld dword ptr [ebx + 8] // 00c501ad
        fmul dword ptr [esi + 4] // 00c501b0
        faddp st(1), st(0) // 00c501b3
        fstp dword ptr [esp + 018h] // 00c501b5
        fadd dword ptr [esp + 018h] // 00c501b9
        fld dword ptr [ebx + 010h] // 00c501bd
        fmul dword ptr [esi + 0ch] // 00c501c0
        fld dword ptr [esi + 8] // 00c501c3
        fmul dword ptr [ebx + 0ch] // 00c501c6
        faddp st(1), st(0) // 00c501c9
        fld dword ptr [ebx + 014h] // 00c501cb
        fmul dword ptr [esi + 010h] // 00c501ce
        faddp st(1), st(0) // 00c501d1
        fstp dword ptr [esp + 018h] // 00c501d3
        fadd dword ptr [esp + 018h] // 00c501d7
        fchs  // 00c501db
        fstp dword ptr [esp + 018h] // 00c501dd
        fld dword ptr [esp + 018h] // 00c501e1
        fstp dword ptr [ecx + edx] // 00c501e5
        fld dword ptr [eax - 024h] // 00c501e8
        fmul dword ptr [esi - 018h] // 00c501eb
        mov edx, dword ptr [esp + 05ch] // 00c501ee
        fld dword ptr [eax - 028h] // 00c501f2
        mov ecx, dword ptr [esp + 094h] // 00c501f5
        fmul dword ptr [esi - 01ch] // 00c501fc
        xorps xmm1, xmm1 // 00c501ff
        faddp st(1), st(0) // 00c50202
        fld dword ptr [eax - 020h] // 00c50204
        fmul dword ptr [esi - 014h] // 00c50207
        faddp st(1), st(0) // 00c5020a
        fstp dword ptr [esp + 018h] // 00c5020c
        fld dword ptr [esp + 018h] // 00c50210
        fld dword ptr [edi + edx] // 00c50214
        mov edx, dword ptr [esp + 038h] // 00c50217
        fmul dword ptr [edi + ecx] // 00c5021b
        mov ecx, dword ptr [esp + 058h] // 00c5021e
        fld dword ptr [esi - 010h] // 00c50222
        fmul dword ptr [eax - 01ch] // 00c50225
        faddp st(1), st(0) // 00c50228
        fld dword ptr [ecx + edx] // 00c5022a
        mov edx, dword ptr [esp + 0a4h] // 00c5022d
        fmul dword ptr [ecx + edx] // 00c50234
        mov ecx, dword ptr [esp + 0c0h] // 00c50237
        mov edx, dword ptr [esp + 01b0h] // 00c5023e
        mov edx, dword ptr [edx + 0eaa8h] // 00c50245
        faddp st(1), st(0) // 00c5024b
        fstp dword ptr [esp + 018h] // 00c5024d
        fadd dword ptr [esp + 018h] // 00c50251
        fld dword ptr [esi + ecx] // 00c50255
        mov ecx, dword ptr [esp + 048h] // 00c50258
        fmul dword ptr [esi] // 00c5025c
        add ecx, 4 // 00c5025e
        fld dword ptr [eax - 010h] // 00c50261
        mov dword ptr [esp + 048h], ecx // 00c50264
        fmul dword ptr [esi - 4] // 00c50268
        faddp st(1), st(0) // 00c5026b
        fld dword ptr [eax - 8] // 00c5026d
        fmul dword ptr [esi + 4] // 00c50270
        faddp st(1), st(0) // 00c50273
        fstp dword ptr [esp + 018h] // 00c50275
        fadd dword ptr [esp + 018h] // 00c50279
        fld dword ptr [esi + 0ch] // 00c5027d
        fmul dword ptr [eax] // 00c50280
        fld dword ptr [esi + 8] // 00c50282
        fmul dword ptr [eax - 4] // 00c50285
        faddp st(1), st(0) // 00c50288
        fld dword ptr [eax + 4] // 00c5028a
        fmul dword ptr [esi + 010h] // 00c5028d
        faddp st(1), st(0) // 00c50290
        fstp dword ptr [esp + 018h] // 00c50292
        fadd dword ptr [esp + 018h] // 00c50296
        fld1  // 00c5029a
        fdivrp st(1), st(0) // 00c5029c
        fstp dword ptr [esp + 018h] // 00c5029e
        fld dword ptr [esp + 018h] // 00c502a2
        fstp dword ptr [ecx + edx - 4] // 00c502a6
        mov ecx, 030h // 00c502aa
        add dword ptr [esp + 058h], ecx // 00c502af
        add dword ptr [esp + 030h], ecx // 00c502b3
        add dword ptr [esp + 034h], ecx // 00c502b7
        add eax, ecx // 00c502bb
        add edi, ecx // 00c502bd
        add esi, ecx // 00c502bf
        sub dword ptr [esp + 090h], 1 // 00c502c1
        mov dword ptr [esp + 0ach], eax // 00c502c9
        jne l_00c4f360 // 00c502d0
        mov eax, dword ptr [esp + 01b0h] // 00c502d6
        mov ecx, dword ptr [esp + 0a8h] // 00c502dd
    l_00c502e4:
        add dword ptr [esp + 06ch], 0c0h // 00c502e4
        add ecx, 1 // 00c502ec
        cmp ecx, dword ptr [esp + 0e0h] // 00c502ef
        mov dword ptr [esp + 0a8h], ecx // 00c502f6
        jl l_00c4f190 // 00c502fd
    l_00c50303:
        pop edi // 00c50303
        pop esi // 00c50304
        pop ebp // 00c50305
        pop ebx // 00c50306
        add esp, 019ch // 00c50307
        ret 12 // 00c5030d
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void solve_kernel(){
    __asm {
        push ecx // 00c5c710
        push ebx // 00c5c711
        push ebp // 00c5c712
        push esi // 00c5c713
        mov ebx, eax // 00c5c714
        mov eax,dword ptr [esp+20] // 00c5c716: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
        cmp dword ptr [eax + 074h], 0 // 00c5c71b
        push edi // 00c5c71f
        lea edi, [eax + 074h] // 00c5c720
        jne l_00c5c73b // 00c5c723
        mov esi, dword ptr [eax + 4] // 00c5c725
        push 01ah // 00c5c728
        push offset name_00d79f04 // 00c5c72a
        push dword ptr [esp+32] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5c72f
        mov dword ptr [edi], eax // 00c5c734
        mov eax,dword ptr [esp+24] // 00c5c736: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
    l_00c5c73b:
        mov esi, dword ptr [edi] // 00c5c73b
        mov ecx, dword ptr [eax + 4] // 00c5c73d
        mov dword ptr [esi], ecx // 00c5c740
        mov dword ptr [eax + 4], esi // 00c5c742
        push dword ptr [esp+24] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c745 // 00c5c745
        mov ebp, edx // 00c5c747
        mov edx, dword ptr [ebx] // 00c5c749
        mov edi, eax // 00c5c74b
        mov eax, dword ptr [edx + 038h] // 00c5c74d
        test eax, eax // 00c5c750
        jle l_00c5c76a // 00c5c752
        mov dword ptr [esp + 010h], eax // 00c5c754
    l_00c5c758:
        push ebx // 00c5c758
        call normal_kernel // 00c5c759
        call friction_kernel // 00c5c75e
        sub dword ptr [esp + 010h], 1 // 00c5c763
        jne l_00c5c758 // 00c5c768
    l_00c5c76a:
        mov edx, ebx // 00c5c76a
        call writeback_kernel // 00c5c76c
        push ebx // 00c5c771
        call store_kernel // 00c5c772
        push dword ptr [esp+24] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c777 // 00c5c777
        sub eax, edi // 00c5c779
        sbb edx, ebp // 00c5c77b
        add dword ptr [esi + 038h], eax // 00c5c77d
        mov dword ptr [esi + 030h], eax // 00c5c780
        mov eax,dword ptr [esp+24] // 00c5c783: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
        adc dword ptr [esi + 03ch], edx // 00c5c788
        mov dword ptr [esi + 034h], edx // 00c5c78b
        add dword ptr [esi + 040h], 1 // 00c5c78e
        mov ecx, dword ptr [eax + 4] // 00c5c792
        mov edx, dword ptr [ecx] // 00c5c795
        pop edi // 00c5c797
        pop esi // 00c5c798
        pop ebp // 00c5c799
        mov dword ptr [eax + 4], edx // 00c5c79a
        pop ebx // 00c5c79d
        pop ecx // 00c5c79e
        ret 4 // 00c5c79f
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void prestep_kernel(){
    __asm {
        mov eax, dword ptr fs:[0] // 00c5c7a0
        push -1 // 00c5c7a6
        push 0 // 00c5c7a8
        push eax // 00c5c7ad
        // 00c5c7ae: native FS registration omitted; normal-return contract.
        mov eax,dword ptr [esp+24] // 00c5c7b5: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
        sub esp, 01ch // 00c5c7ba
        cmp dword ptr [eax + 070h], 0 // 00c5c7bd
        push ebx // 00c5c7c1
        lea ebx, [eax + 070h] // 00c5c7c2
        push ebp // 00c5c7c5
        push esi // 00c5c7c6
        jne l_00c5c7df // 00c5c7c7
        mov esi, dword ptr [eax + 4] // 00c5c7c9
        push 019h // 00c5c7cc
        push offset name_00d79ef4 // 00c5c7ce
        push dword ptr [esp+72] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5c7d3
        mov dword ptr [ebx], eax // 00c5c7d8
        mov eax,dword ptr [esp+64] // 00c5c7da: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
    l_00c5c7df:
        mov ebp, dword ptr [ebx] // 00c5c7df
        mov ecx, dword ptr [eax + 4] // 00c5c7e1
        mov dword ptr [ebp], ecx // 00c5c7e4
        mov dword ptr [esp + 01ch], ebp // 00c5c7e7
        mov dword ptr [eax + 4], ebp // 00c5c7eb
        push dword ptr [esp+64] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c7ee // 00c5c7ee
        mov ebx, eax // 00c5c7f0
        mov dword ptr [esp + 010h], edx // 00c5c7f2
        mov dword ptr [esp + 0ch], ebx // 00c5c7f6
        mov edx, dword ptr [esp + 038h] // 00c5c7fa
        mov dword ptr [esp + 030h], 0 // 00c5c7fe
        mov esi, edi // 00c5c806
        mov dword ptr [edi + 0ea68h], edx // 00c5c808
        push dword ptr [esp+64] // Borrowed context; original frame offsets retained.
        call rows_size_kernel // 00c5c80e
        fld dword ptr [esp + 03ch] // 00c5c813
        push ecx // 00c5c817
        fstp dword ptr [esp] // 00c5c818
        push edi // 00c5c81b
        push dword ptr [esp+72] // Borrowed context; original frame offsets retained.
        call build_rows_shim // 00c5c81c
        mov esi, dword ptr [edi + 4] // 00c5c821
        mov dword ptr [edi + 0eac0h], esi // 00c5c824
        cmp esi, dword ptr [edi + 0ea70h] // 00c5c82a
        jle l_00c5c86c // 00c5c830
        mov eax, dword ptr [edi + 0ea6ch] // 00c5c832
        test eax, eax // 00c5c838
        je l_00c5c845 // 00c5c83a
        push eax // 00c5c83c
        push dword ptr [esp+68] // Borrowed context; original frame offsets retained.
        call bridge_00c5c83d // 00c5c83d
        add esp, 4 // 00c5c842
    l_00c5c845:
        xor ecx, ecx // 00c5c845
        mov eax, esi // 00c5c847
        mov edx, 030h // 00c5c849
        mul edx // 00c5c84e
        seto cl // 00c5c850
        neg ecx // 00c5c853
        or ecx, eax // 00c5c855
        push ecx // 00c5c857
        push dword ptr [esp+68] // Borrowed context; original frame offsets retained.
        call bridge_00c5c858 // 00c5c858
        add esp, 4 // 00c5c85d
        mov dword ptr [edi + 0ea6ch], eax // 00c5c860
        mov dword ptr [edi + 0ea70h], esi // 00c5c866
    l_00c5c86c:
        mov eax, dword ptr [edi + 0ea70h] // 00c5c86c
        xor edx, edx // 00c5c872
        test eax, eax // 00c5c874
        jle l_00c5c8f3 // 00c5c876
        xorps xmm0, xmm0 // 00c5c878
        xor ecx, ecx // 00c5c87b
        _emit 08dh // 00c5c87d
        _emit 049h
        _emit 000h
    l_00c5c880:
        mov eax, dword ptr [edi + 0ea6ch] // 00c5c880
        movss dword ptr [eax + ecx + 8], xmm0 // 00c5c886
        movss dword ptr [eax + ecx + 4], xmm0 // 00c5c88c
        movss dword ptr [eax + ecx], xmm0 // 00c5c892
        add eax, ecx // 00c5c897
        mov eax, dword ptr [edi + 0ea6ch] // 00c5c899
        movss dword ptr [eax + ecx + 014h], xmm0 // 00c5c89f
        movss dword ptr [eax + ecx + 010h], xmm0 // 00c5c8a5
        movss dword ptr [eax + ecx + 0ch], xmm0 // 00c5c8ab
        lea eax, [eax + ecx + 0ch] // 00c5c8b1
        mov eax, dword ptr [edi + 0ea6ch] // 00c5c8b5
        lea eax, [eax + ecx + 024h] // 00c5c8bb
        movss dword ptr [eax + 8], xmm0 // 00c5c8bf
        movss dword ptr [eax + 4], xmm0 // 00c5c8c4
        movss dword ptr [eax], xmm0 // 00c5c8c9
        mov eax, dword ptr [edi + 0ea6ch] // 00c5c8cd
        lea eax, [eax + ecx + 018h] // 00c5c8d3
        add edx, 1 // 00c5c8d7
        movss dword ptr [eax + 8], xmm0 // 00c5c8da
        movss dword ptr [eax + 4], xmm0 // 00c5c8df
        movss dword ptr [eax], xmm0 // 00c5c8e4
        add ecx, 030h // 00c5c8e8
        cmp edx, dword ptr [edi + 0ea70h] // 00c5c8eb
        jl l_00c5c880 // 00c5c8f1
    l_00c5c8f3:
        mov ecx, edi // 00c5c8f3
        call warm_kernel // 00c5c8f5
        push dword ptr [esp+64] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c8fa // 00c5c8fa
        sub eax, ebx // 00c5c8fc
        sbb edx, dword ptr [esp + 010h] // 00c5c8fe
        add dword ptr [ebp + 038h], eax // 00c5c902
        mov dword ptr [ebp + 030h], eax // 00c5c905
        mov eax,dword ptr [esp+64] // 00c5c908: borrowed context.
        mov eax,dword ptr [eax+10h] // Actual profile publication cell.
        mov eax,dword ptr [eax] // Preserve native load and flags.
        adc dword ptr [ebp + 03ch], edx // 00c5c90d
        mov dword ptr [ebp + 034h], edx // 00c5c910
        add dword ptr [ebp + 040h], 1 // 00c5c913
        mov ecx, dword ptr [eax + 4] // 00c5c917
        mov edx, dword ptr [ecx] // 00c5c91a
        mov ecx, dword ptr [esp + 028h] // 00c5c91c
        pop esi // 00c5c920
        pop ebp // 00c5c921
        mov dword ptr [eax + 4], edx // 00c5c922
        // 00c5c925: native FS registration omitted; normal-return contract.
        pop ebx // 00c5c92c
        add esp, 028h // 00c5c92d
        ret 12 // 00c5c930
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
std::uint64_t NativeDynSolverMode1Calls::read_timestamp(U) noexcept {_ReadWriteBarrier();const auto t=__rdtsc();_ReadWriteBarrier();return t;}
void execute_native_dyn_solver_mode1_00403850(void* task,const DynProfileScopeContext& p,NativeDynSolverMode1Calls& calls,const CameraAxesCrtAccess& crt){
    Context context{crt,&p.memory,&calls,p.profile_slot_0109e9f8};Context* c=&context;
    __asm {mov edx,c}
    __asm {mov ecx,task}
    __asm {call task_kernel}
}
NativeDynSolverMode1Runtime::NativeDynSolverMode1Runtime(const DynProfileScopeContext& p,NativeDynSolverMode1Calls& c,const CameraAxesCrtAccess& crt) noexcept:methods_{reinterpret_cast<std::uintptr_t>(&run)},profile_(p),calls_(&c),crt_(crt){static_assert(std::is_standard_layout_v<NativeDynSolverMode1Runtime>);}
void __fastcall NativeDynSolverMode1Runtime::run(void* task,void*){auto* self=*static_cast<NativeDynSolverMode1Runtime**>(task);execute_native_dyn_solver_mode1_00403850(task,self->profile_,*self->calls_,self->crt_);}
} // namespace bsp
