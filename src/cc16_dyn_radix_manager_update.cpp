#include "bsp/cc16_dyn_radix_manager_update.hpp"

#include "bsp/cc13_dyn1.hpp"
#include "bsp/native_crt_memset.hpp"

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered instruction schedule requires MSVC Win32.
#endif

// Genuine stack-changing compiler support, not a reconstructed vendor body.
extern "C" void __cdecl _alloca_probe();

namespace bsp::cc16_dyn_radix_manager_update {
namespace {
using U32 = std::uint32_t;
enum class ProfileAppendQualification : U32 { normal_only, qualified };
struct Context {
    const cc15_dyn_live::DynLiveCrt* crt;
    const volatile U32* feature_word;
    const volatile double* half;
    void* const* profile_slot;
    const DynProfileScopeContext* profile;
    RadixManagerProfileNames names;
    ProfileAppendQualification profile_append_qualification;
    DynProfilePartialNodeDelete delete_partial_00bf65ac;
};
static_assert(sizeof(void*) == 4);
static_assert(std::is_standard_layout_v<Context> && sizeof(Context) == 0x30);
static_assert(offsetof(Context, crt) == 0 && offsetof(Context, feature_word) == 4
              && offsetof(Context, half) == 8 && offsetof(Context, profile_slot) == 0xc
              && offsetof(Context, profile) == 0x10 && offsetof(Context, names) == 0x14);
static_assert(offsetof(Context, profile_append_qualification) == 0x28
              && offsetof(Context, delete_partial_00bf65ac) == 0x2c);
static_assert(sizeof(RadixManagerProfileNames) == 0x14);
static_assert(sizeof(DynProfileScopeStorage) == 0x14);

// S is ESP after the original four register saves. The pushed Context occupies
// S+10B0, outside every native scratch/scope slot. These slots are never
// registered with FS. Native state transitions remain literal in the kernel.
const Context& context_from(unsigned char* frame) noexcept {
    return **reinterpret_cast<const Context* const*>(frame + 0x10b0);
}
void unwind_profiles(unsigned char* frame) {
    const auto& context = context_from(frame);
    auto& state = *reinterpret_cast<volatile U32*>(frame + 0x10bc);
    // E04478 map: {prev=-1,CC84DA}, {0,CC84E5}, {1,CC84F0}, {2,CC84FB}.
    // Each funclet passes exactly this native scope to the real C38040 body.
    while (state != 0xffffffffu) {
        const U32 active = state;
        U32 offset;
        switch (active) {
        case 3: offset = 0x98; break; // Pair
        case 2: offset = 0x50; break; // PossiblePair
        case 1: offset = 0x68; break; // Object
        default: offset = 0x80; break; // Sweep (only actual state0)
        }
        state = active - 1u;
        cc13_dyn1::profile_scope_leave_00c38040(frame + offset, *context.profile);
    }
}

// Catch before unwinding the unregistered kernel: the original scope storage
// is still alive here. Only C++ exceptions from the genuine calls are handled.
// No Sort cleanup, manager rollback, allocation rollback or SEH emulation.
void* __cdecl new_bridge(unsigned char* arguments, U32 bytes) {
    auto* frame = arguments + 4;
    const auto& context = context_from(frame);
    try {
        return context.crt->operator_new_00bf55be(bytes);
    } catch (...) {
        unwind_profiles(frame);
        throw;
    }
}
void __cdecl free_bridge(unsigned char* arguments, void* allocation) {
    auto* frame = arguments + 4;
    const auto& context = context_from(frame);
    try {
        context.crt->free_00bf6989(allocation);
    } catch (...) {
        unwind_profiles(frame);
        throw;
    }
}
DynProfileNodeStorage* __cdecl append_bridge(unsigned char* arguments,
    DynProfileNodeStorage* parent, const char* name, U32 id) {
    auto* frame = arguments + 8;
    const auto& context = context_from(frame);
    try {
        if(context.profile_append_qualification==ProfileAppendQualification::qualified)
            return dyn_profile_node_append_child_00c50390(*parent,name,id,
                context.profile->memory,context.delete_partial_00bf65ac);
        return dyn_profile_node_append_child_00c50390(*parent, name, id,
                                                     context.profile->memory);
    } catch (...) {
        unwind_profiles(frame);
        throw;
    }
}
void* __cdecl memset_bridge(unsigned char* arguments, void* destination,
                            int fill, U32 bytes) {
    auto* frame = arguments + 12;
    const auto& context = context_from(frame);
    try {
        return fill_native_crt_bytes_00bf79f0(destination, fill, bytes,
                                             *context.feature_word);
    } catch (...) {
        unwind_profiles(frame);
        throw;
    }
}

// Inserted PUSH ESP passes the original argument-stack address without using
// any scratch register or modifying flags. Each shim consumes just that extra
// word, plus original callee-cleaned arguments for C50390. Native caller ADDs
// and return EAX remain at their original instruction epochs.
__declspec(naked) void new_shim() {
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call new_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void free_shim() {
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call free_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void append_shim() {
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
__declspec(naked) void memset_shim() {
    __asm {
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call memset_bridge
        add esp,10h
        ret 4
    }
}

// Native ECX manager; the private ABI adds a Context stack word, RET4.
// Every original instruction is address-marked. Adaptations are explicit:
// borrowed cells/names, ten call-origin pushes/bridges, and unregistered FH
// reservation slots. Keep all real frame/probe, integer/SSE/x87 operations,
// store/reload epochs, alias behavior and native internal branch destinations.
__declspec(naked) void __fastcall kernel(void*, void*, const Context*) {
    __asm {
        push ebp // 00c5a8b0
        mov ebp,esp // 00c5a8b1
        and esp,0fffffff8h // 00c5a8b3
        push -01h // 00c5a8b6
        push 0 // Reserved handler slot; no original FH registration. // 00c5a8b8
        mov eax,fs:[00h] // 00c5a8bd
        push eax // 00c5a8c3
        nop // No copied FS registration/restoration. // 00c5a8c4
        push dword ptr [ebp+8] // Borrowed Context in original saved-ECX slot. // 00c5a8cb
        mov eax,010a0h // 00c5a8cc
        call _alloca_probe // 00c5a8d1
        push ebx // 00c5a8d6
        push ebp // 00c5a8d7
        push esi // 00c5a8d8
        push edi // 00c5a8d9
        mov edi,ecx // 00c5a8da
        mov ebx,dword ptr [edi + 08h] // 00c5a8dc
        xor eax,eax // 00c5a8df
        cmp ebx,04h // 00c5a8e1
        mov dword ptr [esp + 024h],ebx // 00c5a8e4
        jl l_00c5a9ad // 00c5a8e8
        lea esi,[ebx + -03h] // 00c5a8ee
    l_00c5a8f1:
        mov ecx,dword ptr [edi + 04h] // 00c5a8f1
        mov ebp,dword ptr [ecx + eax*08h + 04h] // 00c5a8f4
        mov edx,dword ptr [edi + 028h] // 00c5a8f8
        fld dword ptr [ebp + edx*04h + 04h] // 00c5a8fb
        lea ecx,[ecx + eax*08h] // 00c5a8ff
        fstp dword ptr [ecx] // 00c5a902
        mov edx,dword ptr [ecx] // 00c5a904
        mov ebp,edx // 00c5a906
        shr ebp,01fh // 00c5a908
        not ebp // 00c5a90b
        add ebp,01h // 00c5a90d
        or ebp,080000000h // 00c5a910
        xor ebp,edx // 00c5a916
        mov dword ptr [ecx],ebp // 00c5a918
        mov ecx,dword ptr [edi + 04h] // 00c5a91a
        mov ebp,dword ptr [ecx + eax*08h + 0ch] // 00c5a91d
        mov edx,dword ptr [edi + 028h] // 00c5a921
        fld dword ptr [ebp + edx*04h + 04h] // 00c5a924
        lea ecx,[ecx + eax*08h] // 00c5a928
        fstp dword ptr [ecx + 08h] // 00c5a92b
        mov edx,dword ptr [ecx + 08h] // 00c5a92e
        mov ebp,edx // 00c5a931
        shr ebp,01fh // 00c5a933
        not ebp // 00c5a936
        add ebp,01h // 00c5a938
        or ebp,080000000h // 00c5a93b
        xor ebp,edx // 00c5a941
        mov dword ptr [ecx + 08h],ebp // 00c5a943
        mov ecx,dword ptr [edi + 04h] // 00c5a946
        mov ebp,dword ptr [ecx + eax*08h + 014h] // 00c5a949
        mov edx,dword ptr [edi + 028h] // 00c5a94d
        fld dword ptr [ebp + edx*04h + 04h] // 00c5a950
        add eax,04h // 00c5a954
        fstp dword ptr [ecx + eax*08h + -010h] // 00c5a957
        mov edx,dword ptr [ecx + eax*08h + -010h] // 00c5a95b
        mov ebp,edx // 00c5a95f
        shr ebp,01fh // 00c5a961
        not ebp // 00c5a964
        add ebp,01h // 00c5a966
        or ebp,080000000h // 00c5a969
        xor ebp,edx // 00c5a96f
        mov dword ptr [ecx + eax*08h + -010h],ebp // 00c5a971
        mov ecx,dword ptr [edi + 04h] // 00c5a975
        mov ebp,dword ptr [ecx + eax*08h + -04h] // 00c5a978
        mov edx,dword ptr [edi + 028h] // 00c5a97c
        movss xmm0,dword ptr [ebp + edx*04h + 04h] // 00c5a97f
        movss dword ptr [ecx + eax*08h + -08h],xmm0 // 00c5a985
        mov edx,dword ptr [ecx + eax*08h + -08h] // 00c5a98b
        mov ebp,edx // 00c5a98f
        shr ebp,01fh // 00c5a991
        not ebp // 00c5a994
        add ebp,01h // 00c5a996
        or ebp,080000000h // 00c5a999
        xor ebp,edx // 00c5a99f
        cmp eax,esi // 00c5a9a1
        mov dword ptr [ecx + eax*08h + -08h],ebp // 00c5a9a3
        jl l_00c5a8f1 // 00c5a9a7
    l_00c5a9ad:
        cmp eax,ebx // 00c5a9ad
        jge l_00c5a9e5 // 00c5a9af
    l_00c5a9b1:
        mov ecx,dword ptr [edi + 04h] // 00c5a9b1
        mov esi,dword ptr [ecx + eax*08h + 04h] // 00c5a9b4
        mov edx,dword ptr [edi + 028h] // 00c5a9b8
        movss xmm0,dword ptr [esi + edx*04h + 04h] // 00c5a9bb
        movss dword ptr [ecx + eax*08h],xmm0 // 00c5a9c1
        mov edx,dword ptr [ecx + eax*08h] // 00c5a9c6
        mov esi,edx // 00c5a9c9
        shr esi,01fh // 00c5a9cb
        not esi // 00c5a9ce
        add esi,01h // 00c5a9d0
        or esi,080000000h // 00c5a9d3
        xor esi,edx // 00c5a9d9
        mov dword ptr [ecx + eax*08h],esi // 00c5a9db
        add eax,01h // 00c5a9de
        cmp eax,ebx // 00c5a9e1
        jl l_00c5a9b1 // 00c5a9e3
    l_00c5a9e5:
        cmp ebx,dword ptr [edi + 014h] // 00c5a9e5
        jbe l_00c5aa3e // 00c5a9e8
        cmp ebx,dword ptr [edi + 018h] // 00c5a9ea
        jbe l_00c5aa3e // 00c5a9ed
        lea eax,[ebx*08h + 00h] // 00c5a9ef
        push eax // 00c5a9f6
        mov dword ptr [edi + 018h],ebx // 00c5a9f7
        push esp // Original argument-stack pointer; no register/flags clobber.
        call new_shim // 00c5a9fa
        mov esi,eax // 00c5a9ff
        xor eax,eax // 00c5aa01
        add esp,04h // 00c5aa03
        cmp dword ptr [edi + 014h],eax // 00c5aa06
        jbe l_00c5aa2b // 00c5aa09
        mov ecx,esi // 00c5aa0b
    l_00c5aa0d:
        test ecx,ecx // 00c5aa0d
        jz l_00c5aa20 // 00c5aa0f
        mov edx,dword ptr [edi + 010h] // 00c5aa11
        mov ebp,dword ptr [edx + eax*08h] // 00c5aa14
        mov dword ptr [ecx],ebp // 00c5aa17
        mov edx,dword ptr [edx + eax*08h + 04h] // 00c5aa19
        mov dword ptr [ecx + 04h],edx // 00c5aa1d
    l_00c5aa20:
        add eax,01h // 00c5aa20
        add ecx,08h // 00c5aa23
        cmp eax,dword ptr [edi + 014h] // 00c5aa26
        jc l_00c5aa0d // 00c5aa29
    l_00c5aa2b:
        mov eax,dword ptr [edi + 010h] // 00c5aa2b
        test eax,eax // 00c5aa2e
        jz l_00c5aa3b // 00c5aa30
        push eax // 00c5aa32
        push esp // Original argument-stack pointer; no register/flags clobber.
        call free_shim // 00c5aa33
        add esp,04h // 00c5aa38
    l_00c5aa3b:
        mov dword ptr [edi + 010h],esi // 00c5aa3b
    l_00c5aa3e:
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5aa3e
        lea ebp,[eax + 030h] // 00c5aa43
        mov dword ptr [edi + 014h],ebx // 00c5aa46
        cmp dword ptr [ebp],00h // 00c5aa49
        jnz l_00c5aa66 // 00c5aa4d
        mov esi,dword ptr [eax + 04h] // 00c5aa4f
        push 09h // 00c5aa52
        mov edx,dword ptr [esp+010b4h] // Original id argument is already pushed.
        mov edx,dword ptr [edx+014h] // Actual native name pointer.
        push edx // 00c5aa54
        push esp // Original argument-stack pointer; no register/flags clobber.
        call append_shim // 00c5aa59
        mov dword ptr [ebp],eax // 00c5aa5e
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5aa61
    l_00c5aa66:
        mov ebp,dword ptr [ebp] // 00c5aa66
        mov ecx,dword ptr [eax + 04h] // 00c5aa69
        mov dword ptr [ebp],ecx // 00c5aa6c
        mov dword ptr [eax + 04h],ebp // 00c5aa6f
        rdtsc // 00c5aa72
        mov esi,dword ptr [edi + 04h] // 00c5aa74
        push 01000h // 00c5aa77
        mov dword ptr [esp + 054h],eax // 00c5aa7c
        mov dword ptr [esp + 058h],edx // 00c5aa80
        mov edx,dword ptr [edi + 010h] // 00c5aa84
        lea eax,[esp + 0b4h] // 00c5aa87
        push 00h // 00c5aa8e
        push eax // 00c5aa90
        mov dword ptr [esp + 024h],edx // 00c5aa91
        push esp // Original argument-stack pointer; no register/flags clobber.
        call memset_shim // 00c5aa95
        add esp,0ch // 00c5aa9a
        xor ecx,ecx // 00c5aa9d
        test ebx,ebx // 00c5aa9f
        jle l_00c5ab0e // 00c5aaa1
    l_00c5aaa3:
        mov eax,dword ptr [esi + ecx*08h] // 00c5aaa3
        mov edx,eax // 00c5aaa6
        and edx,0ffh // 00c5aaa8
        shl edx,04h // 00c5aaae
        add dword ptr [esp + edx*01h + 0b0h],01h // 00c5aab1
        lea edx,[esp + edx*01h + 0b0h] // 00c5aab9
        movzx edx,ah // 00c5aac0
        shl edx,04h // 00c5aac3
        add dword ptr [esp + edx*01h + 0b4h],01h // 00c5aac6
        lea edx,[esp + edx*01h + 0b4h] // 00c5aace
        mov edx,eax // 00c5aad5
        shr edx,010h // 00c5aad7
        and edx,0ffh // 00c5aada
        shr eax,018h // 00c5aae0
        shl edx,04h // 00c5aae3
        add dword ptr [esp + edx*01h + 0b8h],01h // 00c5aae6
        shl eax,04h // 00c5aaee
        add dword ptr [esp + eax*01h + 0bch],01h // 00c5aaf1
        lea edx,[esp + edx*01h + 0b8h] // 00c5aaf9
        lea eax,[esp + eax*01h + 0bch] // 00c5ab00
        add ecx,01h // 00c5ab07
        cmp ecx,ebx // 00c5ab0a
        jl l_00c5aaa3 // 00c5ab0c
    l_00c5ab0e:
        xor ecx,ecx // 00c5ab0e
        xor ebx,ebx // 00c5ab10
        mov dword ptr [esp + 020h],ecx // 00c5ab12
        mov dword ptr [esp + 028h],ecx // 00c5ab16
        lea eax,[esp + 0b4h] // 00c5ab1a
        mov dword ptr [esp + 014h],0100h // 00c5ab21
        jmp l_00c5ab34 // 00c5ab29
    l_00c5ab30:
        mov ebx,dword ptr [esp + 01ch] // 00c5ab30
    l_00c5ab34:
        mov edx,dword ptr [eax + -04h] // 00c5ab34
        mov dword ptr [eax + -04h],ecx // 00c5ab37
        add ecx,edx // 00c5ab3a
        mov edx,dword ptr [eax] // 00c5ab3c
        mov dword ptr [eax],ebx // 00c5ab3e
        add ebx,edx // 00c5ab40
        mov edx,dword ptr [eax + 04h] // 00c5ab42
        mov dword ptr [esp + 01ch],ebx // 00c5ab45
        mov ebx,dword ptr [esp + 020h] // 00c5ab49
        mov dword ptr [eax + 04h],ebx // 00c5ab4d
        add ebx,edx // 00c5ab50
        mov edx,dword ptr [eax + 08h] // 00c5ab52
        mov dword ptr [esp + 020h],ebx // 00c5ab55
        mov ebx,dword ptr [esp + 028h] // 00c5ab59
        mov dword ptr [eax + 08h],ebx // 00c5ab5d
        add ebx,edx // 00c5ab60
        add eax,010h // 00c5ab62
        sub dword ptr [esp + 014h],01h // 00c5ab65
        mov dword ptr [esp + 028h],ebx // 00c5ab6a
        jnz l_00c5ab30 // 00c5ab6e
        mov ebx,dword ptr [esp + 024h] // 00c5ab70
        test ebx,ebx // 00c5ab74
        jle l_00c5abb4 // 00c5ab76
        mov eax,esi // 00c5ab78
        mov dword ptr [esp + 014h],ebx // 00c5ab7a
        mov edi,edi // 00c5ab7e
    l_00c5ab80:
        movzx ecx,byte ptr [eax] // 00c5ab80
        shl ecx,04h // 00c5ab83
        lea edx,[esp + ecx*01h + 0b0h] // 00c5ab86
        mov ecx,dword ptr [edx] // 00c5ab8d
        lea ebx,[ecx + 01h] // 00c5ab8f
        mov dword ptr [edx],ebx // 00c5ab92
        mov ebx,dword ptr [eax] // 00c5ab94
        mov edx,dword ptr [esp + 018h] // 00c5ab96
        mov dword ptr [edx + ecx*08h],ebx // 00c5ab9a
        mov ebx,dword ptr [eax + 04h] // 00c5ab9d
        add eax,08h // 00c5aba0
        sub dword ptr [esp + 014h],01h // 00c5aba3
        mov dword ptr [edx + ecx*08h + 04h],ebx // 00c5aba8
        jnz l_00c5ab80 // 00c5abac
        mov ebx,dword ptr [esp + 024h] // 00c5abae
        jmp l_00c5abb8 // 00c5abb2
    l_00c5abb4:
        mov edx,dword ptr [esp + 018h] // 00c5abb4
    l_00c5abb8:
        test ebx,ebx // 00c5abb8
        jle l_00c5ac80 // 00c5abba
        mov eax,edx // 00c5abc0
        mov dword ptr [esp + 014h],ebx // 00c5abc2
        jmp l_00c5abd0 // 00c5abc6
    l_00c5abd0:
        movzx ecx,byte ptr [eax + 01h] // 00c5abd0
        shl ecx,04h // 00c5abd4
        lea edx,[esp + ecx*01h + 0b4h] // 00c5abd7
        mov ecx,dword ptr [edx] // 00c5abde
        lea ebx,[ecx + 01h] // 00c5abe0
        mov dword ptr [edx],ebx // 00c5abe3
        mov edx,dword ptr [eax] // 00c5abe5
        mov dword ptr [esi + ecx*08h],edx // 00c5abe7
        mov edx,dword ptr [eax + 04h] // 00c5abea
        add eax,08h // 00c5abed
        sub dword ptr [esp + 014h],01h // 00c5abf0
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c5abf5
        jnz l_00c5abd0 // 00c5abf9
        mov ebx,dword ptr [esp + 024h] // 00c5abfb
        test ebx,ebx // 00c5abff
        mov edx,dword ptr [esp + 018h] // 00c5ac01
        jle l_00c5ac80 // 00c5ac05
        mov ecx,esi // 00c5ac07
        mov dword ptr [esp + 014h],ebx // 00c5ac09
    l_00c5ac0d:
        mov eax,dword ptr [ecx] // 00c5ac0d
        shr eax,010h // 00c5ac0f
        and eax,0ffh // 00c5ac12
        shl eax,04h // 00c5ac17
        lea edx,[esp + eax*01h + 0b8h] // 00c5ac1a
        mov eax,dword ptr [edx] // 00c5ac21
        lea ebx,[eax + 01h] // 00c5ac23
        mov dword ptr [edx],ebx // 00c5ac26
        mov ebx,dword ptr [ecx] // 00c5ac28
        mov edx,dword ptr [esp + 018h] // 00c5ac2a
        mov dword ptr [edx + eax*08h],ebx // 00c5ac2e
        mov ebx,dword ptr [ecx + 04h] // 00c5ac31
        add ecx,08h // 00c5ac34
        sub dword ptr [esp + 014h],01h // 00c5ac37
        mov dword ptr [edx + eax*08h + 04h],ebx // 00c5ac3c
        jnz l_00c5ac0d // 00c5ac40
        mov ebx,dword ptr [esp + 024h] // 00c5ac42
        test ebx,ebx // 00c5ac46
        jle l_00c5ac80 // 00c5ac48
        mov eax,edx // 00c5ac4a
        mov dword ptr [esp + 014h],ebx // 00c5ac4c
    l_00c5ac50:
        mov ecx,dword ptr [eax] // 00c5ac50
        shr ecx,018h // 00c5ac52
        shl ecx,04h // 00c5ac55
        lea edx,[esp + ecx*01h + 0bch] // 00c5ac58
        mov ecx,dword ptr [edx] // 00c5ac5f
        lea ebx,[ecx + 01h] // 00c5ac61
        mov dword ptr [edx],ebx // 00c5ac64
        mov edx,dword ptr [eax] // 00c5ac66
        mov dword ptr [esi + ecx*08h],edx // 00c5ac68
        mov edx,dword ptr [eax + 04h] // 00c5ac6b
        add eax,08h // 00c5ac6e
        sub dword ptr [esp + 014h],01h // 00c5ac71
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c5ac76
        jnz l_00c5ac50 // 00c5ac7a
        mov ebx,dword ptr [esp + 024h] // 00c5ac7c
    l_00c5ac80:
        rdtsc // 00c5ac80
        sub eax,dword ptr [esp + 050h] // 00c5ac82
        sbb edx,dword ptr [esp + 054h] // 00c5ac86
        add dword ptr [ebp + 038h],eax // 00c5ac8a
        mov dword ptr [ebp + 030h],eax // 00c5ac8d
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5ac90
        adc dword ptr [ebp + 03ch],edx // 00c5ac95
        mov dword ptr [ebp + 034h],edx // 00c5ac98
        add dword ptr [ebp + 040h],01h // 00c5ac9b
        mov ecx,dword ptr [eax + 04h] // 00c5ac9f
        mov esi,dword ptr [ecx] // 00c5aca2
        lea ebp,[eax + 034h] // 00c5aca4
        mov dword ptr [eax + 04h],esi // 00c5aca7
        cmp dword ptr [ebp],00h // 00c5acaa
        jnz l_00c5acc4 // 00c5acae
        push 0ah // 00c5acb0
        mov edx,dword ptr [esp+010b4h] // Original id argument is already pushed.
        mov edx,dword ptr [edx+018h] // Actual native name pointer.
        push edx // 00c5acb2
        push esp // Original argument-stack pointer; no register/flags clobber.
        call append_shim // 00c5acb7
        mov dword ptr [ebp],eax // 00c5acbc
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5acbf
    l_00c5acc4:
        mov ebp,dword ptr [ebp] // 00c5acc4
        mov edx,dword ptr [eax + 04h] // 00c5acc7
        mov dword ptr [ebp],edx // 00c5acca
        mov dword ptr [esp + 090h],ebp // 00c5accd
        mov dword ptr [eax + 04h],ebp // 00c5acd4
        rdtsc // 00c5acd7
        mov dword ptr [esp + 084h],edx // 00c5acd9
        mov dword ptr [esp + 080h],eax // 00c5ace0
        xor ebp,ebp // 00c5ace7
        mov dword ptr [esp + 010bch],ebp // 00c5ace9
        cmp dword ptr [edi + 020h],ebp // 00c5acf0
        jz l_00c5acf8 // 00c5acf3
        mov dword ptr [edi + 020h],ebp // 00c5acf5
    l_00c5acf8:
        cmp ebx,ebp // 00c5acf8
        xorps xmm0,xmm0 // 00c5acfa
        movss dword ptr [esp + 044h],xmm0 // 00c5acfd
        movss dword ptr [esp + 048h],xmm0 // 00c5ad03
        movss dword ptr [esp + 04ch],xmm0 // 00c5ad09
        movss dword ptr [esp + 038h],xmm0 // 00c5ad0f
        movss dword ptr [esp + 03ch],xmm0 // 00c5ad15
        movss dword ptr [esp + 040h],xmm0 // 00c5ad1b
        mov dword ptr [esp + 028h],ebp // 00c5ad21
        jle l_00c5b0c4 // 00c5ad25
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5ad2b
    l_00c5ad30:
        cmp dword ptr [eax + 038h],00h // 00c5ad30
        lea ebx,[eax + 038h] // 00c5ad34
        jnz l_00c5ad4f // 00c5ad37
        mov esi,dword ptr [eax + 04h] // 00c5ad39
        push 0bh // 00c5ad3c
        mov edx,dword ptr [esp+010b4h] // Original id argument is already pushed.
        mov edx,dword ptr [edx+01ch] // Actual native name pointer.
        push edx // 00c5ad3e
        push esp // Original argument-stack pointer; no register/flags clobber.
        call append_shim // 00c5ad43
        mov dword ptr [ebx],eax // 00c5ad48
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5ad4a
    l_00c5ad4f:
        mov ecx,dword ptr [ebx] // 00c5ad4f
        mov edx,dword ptr [eax + 04h] // 00c5ad51
        mov dword ptr [ecx],edx // 00c5ad54
        mov dword ptr [esp + 078h],ecx // 00c5ad56
        mov dword ptr [eax + 04h],ecx // 00c5ad5a
        rdtsc // 00c5ad5d
        mov dword ptr [esp + 06ch],edx // 00c5ad5f
        mov dword ptr [esp + 068h],eax // 00c5ad63
        mov byte ptr [esp + 010bch],01h // 00c5ad67
        mov eax,dword ptr [edi + 04h] // 00c5ad6f
        mov eax,dword ptr [eax + ebp*08h + 04h] // 00c5ad72
        mov word ptr [eax + 020h],bp // 00c5ad76
        mov eax,dword ptr [edi + 04h] // 00c5ad7a
        mov ebx,dword ptr [eax + ebp*08h + 04h] // 00c5ad7d
        fld dword ptr [ebx + 04h] // 00c5ad81
        add ebx,04h // 00c5ad84
        fadd dword ptr [ebx + 0ch] // 00c5ad87
        add ebp,01h // 00c5ad8a
        cmp ebp,dword ptr [esp + 024h] // 00c5ad8d
        mov dword ptr [esp + 018h],ebp // 00c5ad91
        fstp dword ptr [esp + 01ch] // 00c5ad95
        fld dword ptr [ebx + 010h] // 00c5ad99
        fadd dword ptr [ebx + 04h] // 00c5ad9c
        fstp dword ptr [esp + 014h] // 00c5ad9f
        fld dword ptr [ebx + 014h] // 00c5ada3
        fadd dword ptr [ebx + 08h] // 00c5ada6
        fstp dword ptr [esp + 020h] // 00c5ada9
        fld dword ptr [esp + 01ch] // 00c5adad
        mov edx,dword ptr [esp+010b0h] // EDX is dead here; flags remain live.
        mov edx,dword ptr [edx+8] // Actual 00D7A280 cell.
        fld qword ptr [edx] // 00c5adb1
        fmul st(1),st(0) // Native DC C9; do not reverse this destination. // 00c5adb7
        fxch // 00c5adb9
        fstp dword ptr [esp + 01ch] // 00c5adbb
        fld dword ptr [esp + 014h] // 00c5adbf
        fmul st(0),st(1) // 00c5adc3
        fstp dword ptr [esp + 014h] // 00c5adc5
        fmul dword ptr [esp + 020h] // 00c5adc9
        fstp dword ptr [esp + 020h] // 00c5adcd
        fld dword ptr [esp + 01ch] // 00c5add1
        fld st(0) // 00c5add5
        fadd dword ptr [esp + 044h] // 00c5add7
        fstp dword ptr [esp + 044h] // 00c5addb
        fld dword ptr [esp + 014h] // 00c5addf
        fld st(0) // 00c5ade3
        fadd dword ptr [esp + 048h] // 00c5ade5
        fstp dword ptr [esp + 048h] // 00c5ade9
        fld dword ptr [esp + 020h] // 00c5aded
        mov dword ptr [esp + 020h],ebp // 00c5adf1
        fld st(0) // 00c5adf5
        fadd dword ptr [esp + 04ch] // 00c5adf7
        fstp dword ptr [esp + 04ch] // 00c5adfb
        fld st(2) // 00c5adff
        fmulp st(3),st(0) // 00c5ae01
        fld dword ptr [esp + 038h] // 00c5ae03
        faddp st(3),st(0) // 00c5ae07
        fxch st(2) // 00c5ae09
        fstp dword ptr [esp + 038h] // 00c5ae0b
        fmul st(0),st(0) // 00c5ae0f
        fadd dword ptr [esp + 03ch] // 00c5ae11
        fstp dword ptr [esp + 03ch] // 00c5ae15
        fmul st(0),st(0) // 00c5ae19
        fadd dword ptr [esp + 040h] // 00c5ae1b
        fstp dword ptr [esp + 040h] // 00c5ae1f
        jge l_00c5b083 // 00c5ae23
        mov edx,dword ptr [esp+010b0h] // Borrowed Context.
        mov edx,dword ptr [edx+0ch] // Actual publication cell.
        mov edx,dword ptr [edx] // 00c5ae29
        nop // 00c5ae2f
    l_00c5ae30:
        mov esi,dword ptr [esp + 018h] // 00c5ae30
        mov eax,dword ptr [edi + 04h] // 00c5ae34
        mov eax,dword ptr [eax + esi*08h + 04h] // 00c5ae37
        mov esi,dword ptr [edi + 028h] // 00c5ae3b
        fld dword ptr [ebx + esi*04h + 0ch] // 00c5ae3e
        add eax,04h // 00c5ae42
        fld dword ptr [eax + esi*04h] // 00c5ae45
        mov dword ptr [esp + 01ch],eax // 00c5ae48
        fcomip st(0),st(1) // 00c5ae4c
        fstp st(0) // 00c5ae4e
        ja l_00c5b083 // 00c5ae50
        cmp dword ptr [edx + 03ch],00h // 00c5ae56
        lea ebp,[edx + 03ch] // 00c5ae5a
        jnz l_00c5ae77 // 00c5ae5d
        mov esi,dword ptr [edx + 04h] // 00c5ae5f
        push 0ch // 00c5ae62
        mov edx,dword ptr [esp+010b4h] // Original id argument is already pushed.
        mov edx,dword ptr [edx+020h] // Actual native name pointer.
        push edx // 00c5ae64
        push esp // Original argument-stack pointer; no register/flags clobber.
        call append_shim // 00c5ae69
        mov edx,dword ptr [esp+010b0h] // Borrowed Context.
        mov edx,dword ptr [edx+0ch] // Actual publication cell.
        mov edx,dword ptr [edx] // 00c5ae6e
        mov dword ptr [ebp],eax // 00c5ae74
    l_00c5ae77:
        mov ebp,dword ptr [ebp] // 00c5ae77
        mov ecx,dword ptr [edx + 04h] // 00c5ae7a
        mov dword ptr [ebp],ecx // 00c5ae7d
        mov dword ptr [esp + 060h],ebp // 00c5ae80
        mov dword ptr [edx + 04h],ebp // 00c5ae84
        rdtsc // 00c5ae87
        mov ecx,eax // 00c5ae89
        mov dword ptr [esp + 054h],edx // 00c5ae8b
        mov dword ptr [esp + 050h],ecx // 00c5ae8f
        mov edx,dword ptr [esp + 01ch] // 00c5ae93
        mov byte ptr [esp + 010bch],02h // 00c5ae97
        fld dword ptr [ebx + 0ch] // 00c5ae9f
        fld dword ptr [edx] // 00c5aea2
        fcomip st(0),st(1) // 00c5aea4
        fstp st(0) // 00c5aea6
        ja l_00c5b03c // 00c5aea8
        fld dword ptr [ebx + 010h] // 00c5aeae
        fld dword ptr [edx + 04h] // 00c5aeb1
        fcomip st(0),st(1) // 00c5aeb4
        fstp st(0) // 00c5aeb6
        ja l_00c5b03c // 00c5aeb8
        fld dword ptr [ebx + 014h] // 00c5aebe
        fld dword ptr [edx + 08h] // 00c5aec1
        fcomip st(0),st(1) // 00c5aec4
        fstp st(0) // 00c5aec6
        ja l_00c5b03c // 00c5aec8
        fld dword ptr [edx + 0ch] // 00c5aece
        fld dword ptr [ebx] // 00c5aed1
        fcomip st(0),st(1) // 00c5aed3
        fstp st(0) // 00c5aed5
        ja l_00c5b03c // 00c5aed7
        fld dword ptr [edx + 010h] // 00c5aedd
        fld dword ptr [ebx + 04h] // 00c5aee0
        fcomip st(0),st(1) // 00c5aee3
        fstp st(0) // 00c5aee5
        ja l_00c5b03c // 00c5aee7
        fld dword ptr [edx + 014h] // 00c5aeed
        fld dword ptr [ebx + 08h] // 00c5aef0
        fcomip st(0),st(1) // 00c5aef3
        fstp st(0) // 00c5aef5
        ja l_00c5b03c // 00c5aef7
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5aefd
        cmp dword ptr [eax + 040h],00h // 00c5af02
        lea ecx,[eax + 040h] // 00c5af06
        mov dword ptr [esp + 014h],ecx // 00c5af09
        jnz l_00c5af2b // 00c5af0d
        mov esi,dword ptr [eax + 04h] // 00c5af0f
        push 0dh // 00c5af12
        mov edx,dword ptr [esp+010b4h] // Original id argument is already pushed.
        mov edx,dword ptr [edx+024h] // Actual native name pointer.
        push edx // 00c5af14
        push esp // Original argument-stack pointer; no register/flags clobber.
        call append_shim // 00c5af19
        mov edx,dword ptr [esp + 014h] // 00c5af1e
        mov dword ptr [edx],eax // 00c5af22
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5af24
        mov ecx,edx // 00c5af29
    l_00c5af2b:
        mov esi,dword ptr [ecx] // 00c5af2b
        mov ecx,dword ptr [eax + 04h] // 00c5af2d
        mov dword ptr [esi],ecx // 00c5af30
        mov dword ptr [esp + 0a8h],esi // 00c5af32
        mov dword ptr [eax + 04h],esi // 00c5af39
        rdtsc // 00c5af3c
        mov dword ptr [esp + 09ch],edx // 00c5af3e
        mov dword ptr [esp + 098h],eax // 00c5af45
        mov edx,dword ptr [esp + 018h] // 00c5af4c
        mov ecx,dword ptr [esp + 028h] // 00c5af50
        mov byte ptr [esp + 010bch],03h // 00c5af54
        mov eax,dword ptr [edi + 04h] // 00c5af5c
        mov edx,dword ptr [eax + edx*08h + 04h] // 00c5af5f
        mov eax,dword ptr [eax + ecx*08h + 04h] // 00c5af63
        cmp eax,edx // 00c5af67
        jnc l_00c5af75 // 00c5af69
        mov dword ptr [esp + 02ch],eax // 00c5af6b
        mov dword ptr [esp + 030h],edx // 00c5af6f
        jmp l_00c5af7d // 00c5af73
    l_00c5af75:
        mov dword ptr [esp + 030h],eax // 00c5af75
        mov dword ptr [esp + 02ch],edx // 00c5af79
    l_00c5af7d:
        mov eax,dword ptr [edi + 024h] // 00c5af7d
        cmp dword ptr [edi + 020h],eax // 00c5af80
        jnz l_00c5afed // 00c5af83
        lea eax,[eax + eax*01h + 02h] // 00c5af85
        lea edx,[eax*08h + 00h] // 00c5af89
        push edx // 00c5af90
        mov dword ptr [edi + 024h],eax // 00c5af91
        push esp // Original argument-stack pointer; no register/flags clobber.
        call new_shim // 00c5af94
        mov dword ptr [esp + 018h],eax // 00c5af99
        xor eax,eax // 00c5af9d
        add esp,04h // 00c5af9f
        cmp dword ptr [edi + 020h],eax // 00c5afa2
        jbe l_00c5afd6 // 00c5afa5
        mov ecx,dword ptr [esp + 014h] // 00c5afa7
        jmp l_00c5afb0 // 00c5afab
    l_00c5afb0:
        test ecx,ecx // 00c5afb0
        jz l_00c5afcb // 00c5afb2
        mov edx,dword ptr [edi + 01ch] // 00c5afb4
        mov dword ptr [esp + 01ch],edx // 00c5afb7
        mov edx,dword ptr [edx + eax*08h] // 00c5afbb
        mov dword ptr [ecx],edx // 00c5afbe
        mov edx,dword ptr [esp + 01ch] // 00c5afc0
        mov edx,dword ptr [edx + eax*08h + 04h] // 00c5afc4
        mov dword ptr [ecx + 04h],edx // 00c5afc8
    l_00c5afcb:
        add eax,01h // 00c5afcb
        add ecx,08h // 00c5afce
        cmp eax,dword ptr [edi + 020h] // 00c5afd1
        jc l_00c5afb0 // 00c5afd4
    l_00c5afd6:
        mov eax,dword ptr [edi + 01ch] // 00c5afd6
        test eax,eax // 00c5afd9
        jz l_00c5afe6 // 00c5afdb
        push eax // 00c5afdd
        push esp // Original argument-stack pointer; no register/flags clobber.
        call free_shim // 00c5afde
        add esp,04h // 00c5afe3
    l_00c5afe6:
        mov eax,dword ptr [esp + 014h] // 00c5afe6
        mov dword ptr [edi + 01ch],eax // 00c5afea
    l_00c5afed:
        mov ecx,dword ptr [edi + 020h] // 00c5afed
        mov edx,dword ptr [edi + 01ch] // 00c5aff0
        lea eax,[edx + ecx*08h] // 00c5aff3
        test eax,eax // 00c5aff6
        jz l_00c5b007 // 00c5aff8
        mov ecx,dword ptr [esp + 02ch] // 00c5affa
        mov edx,dword ptr [esp + 030h] // 00c5affe
        mov dword ptr [eax],ecx // 00c5b002
        mov dword ptr [eax + 04h],edx // 00c5b004
    l_00c5b007:
        add dword ptr [edi + 020h],01h // 00c5b007
        rdtsc // 00c5b00b
        sub eax,dword ptr [esp + 098h] // 00c5b00d
        sbb edx,dword ptr [esp + 09ch] // 00c5b014
        add dword ptr [esi + 038h],eax // 00c5b01b
        mov dword ptr [esi + 030h],eax // 00c5b01e
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5b021
        adc dword ptr [esi + 03ch],edx // 00c5b026
        mov dword ptr [esi + 034h],edx // 00c5b029
        add dword ptr [esi + 040h],01h // 00c5b02c
        mov ecx,dword ptr [eax + 04h] // 00c5b030
        mov edx,dword ptr [ecx] // 00c5b033
        mov ecx,dword ptr [esp + 050h] // 00c5b035
        mov dword ptr [eax + 04h],edx // 00c5b039
    l_00c5b03c:
        mov byte ptr [esp + 010bch],01h // 00c5b03c
        rdtsc // 00c5b044
        sub eax,ecx // 00c5b046
        sbb edx,dword ptr [esp + 054h] // 00c5b048
        add dword ptr [ebp + 038h],eax // 00c5b04c
        mov dword ptr [ebp + 030h],eax // 00c5b04f
        mov dword ptr [ebp + 034h],edx // 00c5b052
        adc dword ptr [ebp + 03ch],edx // 00c5b055
        add dword ptr [ebp + 040h],01h // 00c5b058
        mov edx,dword ptr [esp+010b0h] // Borrowed Context.
        mov edx,dword ptr [edx+0ch] // Actual publication cell.
        mov edx,dword ptr [edx] // 00c5b05c
        mov eax,dword ptr [edx + 04h] // 00c5b062
        mov ecx,dword ptr [eax] // 00c5b065
        mov eax,dword ptr [esp + 018h] // 00c5b067
        add eax,01h // 00c5b06b
        cmp eax,dword ptr [esp + 024h] // 00c5b06e
        mov dword ptr [edx + 04h],ecx // 00c5b072
        mov ecx,dword ptr [esp + 078h] // 00c5b075
        mov dword ptr [esp + 018h],eax // 00c5b079
        jl l_00c5ae30 // 00c5b07d
    l_00c5b083:
        mov byte ptr [esp + 010bch],00h // 00c5b083
        rdtsc // 00c5b08b
        sub eax,dword ptr [esp + 068h] // 00c5b08d
        mov ebp,dword ptr [esp + 020h] // 00c5b091
        sbb edx,dword ptr [esp + 06ch] // 00c5b095
        add dword ptr [ecx + 038h],eax // 00c5b099
        mov dword ptr [ecx + 030h],eax // 00c5b09c
        mov eax,dword ptr [esp+010b0h] // Borrowed Context.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5b09f
        adc dword ptr [ecx + 03ch],edx // 00c5b0a4
        mov dword ptr [ecx + 034h],edx // 00c5b0a7
        add dword ptr [ecx + 040h],01h // 00c5b0aa
        cmp ebp,dword ptr [esp + 024h] // 00c5b0ae
        mov edx,dword ptr [eax + 04h] // 00c5b0b2
        mov ecx,dword ptr [edx] // 00c5b0b5
        mov dword ptr [eax + 04h],ecx // 00c5b0b7
        mov dword ptr [esp + 028h],ebp // 00c5b0ba
        jl l_00c5ad30 // 00c5b0be
    l_00c5b0c4:
        fld dword ptr [esp + 044h] // 00c5b0c4
        mov dword ptr [edi + 028h],00h // 00c5b0c8
        fld st(0) // 00c5b0cf
        fmulp st(1),st(0) // 00c5b0d1
        fsubr dword ptr [esp + 038h] // 00c5b0d3
        fstp dword ptr [esp + 02ch] // 00c5b0d7
        fld dword ptr [esp + 048h] // 00c5b0db
        fld st(0) // 00c5b0df
        fmulp st(1),st(0) // 00c5b0e1
        fsubr dword ptr [esp + 03ch] // 00c5b0e3
        fstp dword ptr [esp + 030h] // 00c5b0e7
        fld dword ptr [esp + 04ch] // 00c5b0eb
        fld st(0) // 00c5b0ef
        fmulp st(1),st(0) // 00c5b0f1
        fsubr dword ptr [esp + 040h] // 00c5b0f3
        fstp dword ptr [esp + 034h] // 00c5b0f7
        fld dword ptr [esp + 02ch] // 00c5b0fb
        fld dword ptr [esp + 030h] // 00c5b0ff
        fcomip st(0),st(1) // 00c5b103
        fstp st(0) // 00c5b105
        jbe l_00c5b110 // 00c5b107
        mov dword ptr [edi + 028h],01h // 00c5b109
    l_00c5b110:
        fld dword ptr [esp + 034h] // 00c5b110
        mov edx,dword ptr [edi + 028h] // 00c5b114
        fld dword ptr [esp + edx*04h + 02ch] // 00c5b117
        fxch // 00c5b11b
        fcomip st(0),st(1) // 00c5b11d
        fstp st(0) // 00c5b11f
        jbe l_00c5b12a // 00c5b121
        mov dword ptr [edi + 028h],02h // 00c5b123
    l_00c5b12a:
        rdtsc // 00c5b12a
        sub eax,dword ptr [esp + 080h] // 00c5b12c
        pop edi // 00c5b133
        sbb edx,dword ptr [esp + 080h] // 00c5b134
        mov ecx,eax // 00c5b13b
        mov eax,dword ptr [esp + 08ch] // 00c5b13d
        add dword ptr [eax + 038h],ecx // 00c5b144
        mov dword ptr [eax + 030h],ecx // 00c5b147
        mov dword ptr [eax + 034h],edx // 00c5b14a
        adc dword ptr [eax + 03ch],edx // 00c5b14d
        add dword ptr [eax + 040h],01h // 00c5b150
        mov eax,dword ptr [esp+010ach] // Borrowed Context after footer POP EDI.
        mov eax,dword ptr [eax+0ch] // Actual publication cell.
        mov eax,dword ptr [eax] // 00c5b154
        mov ecx,dword ptr [eax + 04h] // 00c5b159
        mov edx,dword ptr [ecx] // 00c5b15c
        mov ecx,dword ptr [esp + 010b0h] // 00c5b15e
        mov dword ptr [eax + 04h],edx // 00c5b165
        nop // No copied FS registration/restoration. // 00c5b168
        pop esi // 00c5b16f
        pop ebp // 00c5b170
        pop ebx // 00c5b171
        mov esp,ebp // 00c5b172
        pop ebp // 00c5b174
        ret 4 // Remove the private Context argument. // 00c5b175
    }
}
} // namespace

void update_radix_manager_00c5a8b0(void* manager,
    const cc15_dyn_live::DynLiveCrt& crt,
    const DynProfileScopeContext& profile,
    const volatile U32& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280,
    const RadixManagerProfileNames& actual_names) {
    const Context context{&crt, &actual_feature_word_0109eea4, &actual_half_00d7a280,
        profile.profile_slot_0109e9f8,&profile,actual_names,
        ProfileAppendQualification::normal_only,nullptr};
    kernel(manager, nullptr, &context);
}

void update_radix_manager_00c5a8b0(void* manager,
    const cc15_dyn_live::DynLiveCrt& crt,
    const DynProfileScopeContext& profile,
    const volatile U32& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280,
    const RadixManagerProfileNames& actual_names,
    DynProfilePartialNodeDelete delete_partial_00bf65ac) {
    if(!delete_partial_00bf65ac)
        throw std::invalid_argument("Dyn radix update requires a matching raw-node partial deleter");
    const Context context{&crt,&actual_feature_word_0109eea4,&actual_half_00d7a280,
        profile.profile_slot_0109e9f8,&profile,actual_names,
        ProfileAppendQualification::qualified,delete_partial_00bf65ac};
    kernel(manager, nullptr, &context);
}

} // namespace bsp::cc16_dyn_radix_manager_update
