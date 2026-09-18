#include "bsp/native_dyn_sap_processing.hpp"
#include <cstring>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SAP processing requires MSVC Win32 x87 assembly.
#endif
extern "C" void __cdecl _alloca_probe();
extern "C" void __cdecl _alloca_probe_16();
namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U address(const void* p) noexcept{return reinterpret_cast<U>(p);}
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(address(p)+n);}
volatile U& word(const void* p,U n=0) noexcept{return *static_cast<volatile U*>(at(p,n));}
void* ptr(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(word(p,n));}
void ptr(void* p,U n,void* q) noexcept{word(p,n)=address(q);}
struct Context {const AvoidZoneDynHullMemory& memory;NativeDynSapPairCalls& calls;NativeDynSapLifetimeProgress& progress;};
void* __cdecl memset_bridge(void* p,int value,U bytes){return std::memset(p,value,bytes);}
void* __cdecl allocate_bridge(U site,Context* c,U size){c->progress.native_site=site;return (site==0xc4054b)?c->calls.sap_allocate_00bf55be(size,c->memory):c->calls.sap_malloc_00bf9f1a(size,c->memory);}
void __cdecl free_bridge(U site,Context* c,void* p){c->progress.native_site=site;c->progress.cursor=p;if(site==0xc4057f)c->calls.sap_free_00bf6989(p,c->memory);else c->calls.sap_free_00bf9dc8(p,c->memory);}
void __cdecl create_bridge(void* manager,Context* c,void* first,void* second){create_native_dyn_sap_pair_00c3ffe0(manager,first,second,c->memory,c->calls,c->progress);}
void __cdecl erase_bridge(void* manager,Context* c,void* first,void* second){erase_native_dyn_sap_pair_00c4bcd0(manager,first,second,c->memory,c->calls,c->progress);}
void __fastcall decision_kernel(void*,void*,void*,U,U,Context*);
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void create_shim(){
    __asm {
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push edi
        call create_bridge
        add esp,10h
        ret 0ch
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void erase_shim(){
    __asm {
        push ebx
        push eax
        push dword ptr [esp+0ch]
        push dword ptr [esp+14h]
        call erase_bridge
        add esp,10h
        ret 8
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void decision_shim(){
    __asm {
        push dword ptr [esp+4]
        push ecx
        push dword ptr [esp+14h]
        push dword ptr [esp+14h]
        mov ecx,eax
        call decision_kernel
        ret 0ch
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c40193(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c40193h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c401c9(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c401c9h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c4050f(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c4050fh
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c4054b(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c4054bh
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c4057f(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c4057fh
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c40688(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c40688h
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Extra context word is consumed here; original stack arguments retain their offsets.
__declspec(naked) void bridge_00c406a1(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c406a1h
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Recovered instruction schedule. Context is an extra final argument; native
// operand order, float spills and ordinary storage loads/stores are retained.
__declspec(naked) void __fastcall insert_kernel(void*,U,void*) noexcept {
    __asm {
        mov eax,ecx
        mov ecx,edx
        push ebx // 00c36e20
        push esi // 00c36e21
        mov esi, eax // 00c36e22
        mov eax, dword ptr [esi + ecx*4 + 020h] // 00c36e24
        fld dword ptr [esi + ecx*4 + 4] // 00c36e28
        fstp dword ptr [eax] // 00c36e2c
        mov edx, dword ptr [esi + ecx*4 + 02ch] // 00c36e2e
        fld dword ptr [esi + ecx*4 + 010h] // 00c36e32
        mov eax, ecx // 00c36e36
        imul eax, eax, 034h // 00c36e38
        fstp dword ptr [edx] // 00c36e3b
        mov edx, dword ptr [esp + 0ch] // 00c36e3d
        lea eax, [eax + edx + 4] // 00c36e41
        mov edx, dword ptr [esi + ecx*4 + 020h] // 00c36e45
        movss xmm0, dword ptr [edx] // 00c36e49
        push edi // 00c36e4d
        mov edi, dword ptr [edx + 8] // 00c36e4e
        lea ebx, [eax + 010h] // 00c36e51
        cmp edi, ebx // 00c36e54
        movss dword ptr [esp + 010h], xmm0 // 00c36e56
        je l_00c36e71 // 00c36e5c
        fld dword ptr [esp + 010h] // 00c36e5e
    l_00c36e62:
        fld dword ptr [edi] // 00c36e62
        fcomip st(0), st(1) // 00c36e64
        jbe l_00c36e6f // 00c36e66
        mov edi, dword ptr [edi + 8] // 00c36e68
        cmp edi, ebx // 00c36e6b
        jne l_00c36e62 // 00c36e6d
    l_00c36e6f:
        fstp st(0) // 00c36e6f
    l_00c36e71:
        mov edi, dword ptr [edi + 0ch] // 00c36e71
        cmp edx, edi // 00c36e74
        je l_00c36ea3 // 00c36e76
        cmp dword ptr [edi + 8], edx // 00c36e78
        je l_00c36ea3 // 00c36e7b
        mov eax, dword ptr [edx + 8] // 00c36e7d
        push ebp // 00c36e80
        mov ebp, dword ptr [edx + 0ch] // 00c36e81
        mov dword ptr [eax + 0ch], ebp // 00c36e84
        mov eax, dword ptr [edx + 0ch] // 00c36e87
        mov ebp, dword ptr [edx + 8] // 00c36e8a
        mov dword ptr [eax + 8], ebp // 00c36e8d
        mov dword ptr [edx + 0ch], edi // 00c36e90
        mov eax, dword ptr [edi + 8] // 00c36e93
        mov dword ptr [edx + 8], eax // 00c36e96
        mov eax, dword ptr [edi + 8] // 00c36e99
        mov dword ptr [eax + 0ch], edx // 00c36e9c
        mov dword ptr [edi + 8], edx // 00c36e9f
        pop ebp // 00c36ea2
    l_00c36ea3:
        mov ecx, dword ptr [esi + ecx*4 + 02ch] // 00c36ea3
        mov edx, dword ptr [ecx + 8] // 00c36ea7
        cmp edx, ebx // 00c36eaa
        movss xmm0, dword ptr [ecx] // 00c36eac
        movss dword ptr [esp + 010h], xmm0 // 00c36eb0
        je l_00c36ecb // 00c36eb6
        fld dword ptr [esp + 010h] // 00c36eb8
    l_00c36ebc:
        fld dword ptr [edx] // 00c36ebc
        fcomip st(0), st(1) // 00c36ebe
        jbe l_00c36ec9 // 00c36ec0
        mov edx, dword ptr [edx + 8] // 00c36ec2
        cmp edx, ebx // 00c36ec5
        jne l_00c36ebc // 00c36ec7
    l_00c36ec9:
        fstp st(0) // 00c36ec9
    l_00c36ecb:
        mov edx, dword ptr [edx + 0ch] // 00c36ecb
        cmp ecx, edx // 00c36ece
        je l_00c36efb // 00c36ed0
        cmp dword ptr [edx + 8], ecx // 00c36ed2
        je l_00c36efb // 00c36ed5
        mov eax, dword ptr [ecx + 8] // 00c36ed7
        mov esi, dword ptr [ecx + 0ch] // 00c36eda
        mov dword ptr [eax + 0ch], esi // 00c36edd
        mov eax, dword ptr [ecx + 0ch] // 00c36ee0
        mov esi, dword ptr [ecx + 8] // 00c36ee3
        mov dword ptr [eax + 8], esi // 00c36ee6
        mov dword ptr [ecx + 0ch], edx // 00c36ee9
        mov eax, dword ptr [edx + 8] // 00c36eec
        mov dword ptr [ecx + 8], eax // 00c36eef
        mov eax, dword ptr [edx + 8] // 00c36ef2
        mov dword ptr [eax + 0ch], ecx // 00c36ef5
        mov dword ptr [edx + 8], ecx // 00c36ef8
    l_00c36efb:
        pop edi // 00c36efb
        pop esi // 00c36efc
        pop ebx // 00c36efd
        ret 4 // 00c36efe
    }
}
// Recovered instruction schedule. Context is an extra final argument; native
// operand order, float spills and ordinary storage loads/stores are retained.
__declspec(naked) void __fastcall decision_kernel(void*,void*,void*,U,U,Context*) {
    __asm {
        mov eax,ecx
        mov ecx,dword ptr [esp+0ch]
        push ebx // 00c4bd60
        push ebp // 00c4bd61
        mov ebp, dword ptr [esp + 0ch] // 00c4bd62
        cmp byte ptr [ebp + 01ch], 0 // 00c4bd66
        push edi // 00c4bd6a
        mov ebx, eax // 00c4bd6b
        mov edi, edx // 00c4bd6d
        je l_00c4bd7b // 00c4bd6f
        cmp byte ptr [ebx + 01ch], 0 // 00c4bd71
        jne l_00c4be00 // 00c4bd75
    l_00c4bd7b:
        mov edx, 1 // 00c4bd7b
        shl edx, cl // 00c4bd80
        push esi // 00c4bd82
        mov esi, 1 // 00c4bd83
        and edx, 3 // 00c4bd88
        mov eax, dword ptr [ebp + edx*4 + 020h] // 00c4bd8b
        fld dword ptr [eax] // 00c4bd8f
        mov ecx, edx // 00c4bd91
        shl esi, cl // 00c4bd93
        mov ecx, dword ptr [ebx + edx*4 + 02ch] // 00c4bd95
        fld dword ptr [ecx] // 00c4bd99
        fxch st(1) // 00c4bd9b
        and esi, 3 // 00c4bd9d
        fcomip st(0), st(1) // 00c4bda0
        fstp st(0) // 00c4bda2
        ja l_00c4bdff // 00c4bda4
        mov eax, dword ptr [ebx + edx*4 + 020h] // 00c4bda6
        fld dword ptr [eax] // 00c4bdaa
        mov ecx, dword ptr [ebp + edx*4 + 02ch] // 00c4bdac
        fld dword ptr [ecx] // 00c4bdb0
        fxch st(1) // 00c4bdb2
        fcomip st(0), st(1) // 00c4bdb4
        fstp st(0) // 00c4bdb6
        ja l_00c4bdff // 00c4bdb8
        mov edx, dword ptr [ebp + esi*4 + 020h] // 00c4bdba
        fld dword ptr [edx] // 00c4bdbe
        mov eax, dword ptr [ebx + esi*4 + 02ch] // 00c4bdc0
        fld dword ptr [eax] // 00c4bdc4
        fxch st(1) // 00c4bdc6
        fcomip st(0), st(1) // 00c4bdc8
        fstp st(0) // 00c4bdca
        ja l_00c4bdff // 00c4bdcc
        mov ecx, dword ptr [ebx + esi*4 + 020h] // 00c4bdce
        fld dword ptr [ecx] // 00c4bdd2
        mov edx, dword ptr [ebp + esi*4 + 02ch] // 00c4bdd4
        fld dword ptr [edx] // 00c4bdd8
        fxch st(1) // 00c4bdda
        fcomip st(0), st(1) // 00c4bddc
        fstp st(0) // 00c4bdde
        ja l_00c4bdff // 00c4bde0
        cmp byte ptr [esp + 018h], 0 // 00c4bde2
        jne l_00c4bdf8 // 00c4bde7
        push edi // 00c4bde9
        mov eax, ebp // 00c4bdea
        push dword ptr [esp+36] // Explicit borrowed context.
        call erase_shim // 00c4bdec
        pop esi // 00c4bdf1
        pop edi // 00c4bdf2
        pop ebp // 00c4bdf3
        pop ebx // 00c4bdf4
        ret 16 // 00c4bdf5
    l_00c4bdf8:
        push ebx // 00c4bdf8
        push ebp // 00c4bdf9
        push dword ptr [esp+40] // Explicit borrowed context.
        call create_shim // 00c4bdfa
    l_00c4bdff:
        pop esi // 00c4bdff
    l_00c4be00:
        pop edi // 00c4be00
        pop ebp // 00c4be01
        pop ebx // 00c4be02
        ret 16 // 00c4be03
    }
}
// Recovered instruction schedule. Context is an extra final argument; native
// operand order, float spills and ordinary storage loads/stores are retained.
__declspec(naked) void __stdcall move_kernel(void*,void*,U,Context*) {
    __asm {
        sub esp, 014h // 00c4be10
        mov eax, dword ptr [esp + 01ch] // 00c4be13
        push ebx // 00c4be17
        push ebp // 00c4be18
        mov ebp, dword ptr [esp + 028h] // 00c4be19
        mov ecx, dword ptr [eax + ebp*4 + 020h] // 00c4be1d
        fld dword ptr [eax + ebp*4 + 4] // 00c4be21
        fstp dword ptr [ecx] // 00c4be25
        mov edx, dword ptr [eax + ebp*4 + 02ch] // 00c4be27
        fld dword ptr [eax + ebp*4 + 010h] // 00c4be2b
        mov ecx, ebp // 00c4be2f
        imul ecx, ecx, 034h // 00c4be31
        fstp dword ptr [edx] // 00c4be34
        mov edx, dword ptr [esp + 020h] // 00c4be36
        push esi // 00c4be3a
        mov esi, dword ptr [eax + ebp*4 + 020h] // 00c4be3b
        mov ebx, dword ptr [esi + 4] // 00c4be3f
        movss xmm0, dword ptr [esi] // 00c4be42
        lea eax, [esi + 0ch] // 00c4be46
        lea edx, [ecx + edx + 4] // 00c4be49
        push edi // 00c4be4d
        mov edi, dword ptr [eax] // 00c4be4e
        mov dword ptr [esp + 020h], eax // 00c4be50
        lea eax, [edx + 020h] // 00c4be54
        and ebx, 0fffffffeh // 00c4be57
        cmp edi, eax // 00c4be5a
        mov ecx, esi // 00c4be5c
        mov dword ptr [esp + 018h], edx // 00c4be5e
        movss dword ptr [esp + 01ch], xmm0 // 00c4be62
        mov dword ptr [esp + 014h], ecx // 00c4be68
        je l_00c4bee0 // 00c4be6c
        mov edi, edi // 00c4be6e
    l_00c4be70:
        fld dword ptr [edi] // 00c4be70
        fld dword ptr [esp + 01ch] // 00c4be72
        fcomip st(0), st(1) // 00c4be76
        fstp st(0) // 00c4be78
        jbe l_00c4bea9 // 00c4be7a
        mov eax, dword ptr [edi + 4] // 00c4be7c
        test al, 1 // 00c4be7f
        jne l_00c4be9c // 00c4be81
        mov edx, dword ptr [esp + 028h] // 00c4be83
        push 0 // 00c4be87
        and eax, 0fffffffeh // 00c4be89
        push ebx // 00c4be8c
        mov ecx, ebp // 00c4be8d
        push dword ptr [esp+60] // Explicit borrowed context.
        call decision_shim // 00c4be8f
        mov ecx, dword ptr [esp + 014h] // 00c4be94
        mov edx, dword ptr [esp + 018h] // 00c4be98
    l_00c4be9c:
        mov edi, dword ptr [edi + 0ch] // 00c4be9c
        mov esi, dword ptr [esi + 0ch] // 00c4be9f
        lea eax, [edx + 020h] // 00c4bea2
        cmp edi, eax // 00c4bea5
        jne l_00c4be70 // 00c4bea7
    l_00c4bea9:
        cmp esi, ecx // 00c4bea9
        je l_00c4bee0 // 00c4beab
        cmp dword ptr [esi + 0ch], ecx // 00c4bead
        je l_00c4bffc // 00c4beb0
        mov edx, dword ptr [ecx + 8] // 00c4beb6
        mov eax, dword ptr [esp + 020h] // 00c4beb9
        mov edi, dword ptr [eax] // 00c4bebd
        mov dword ptr [edx + 0ch], edi // 00c4bebf
        mov edx, dword ptr [eax] // 00c4bec2
        mov edi, dword ptr [ecx + 8] // 00c4bec4
        mov dword ptr [edx + 8], edi // 00c4bec7
        mov edx, dword ptr [esi + 0ch] // 00c4beca
        mov dword ptr [eax], edx // 00c4becd
        mov dword ptr [ecx + 8], esi // 00c4becf
        mov eax, dword ptr [esi + 0ch] // 00c4bed2
        mov dword ptr [eax + 8], ecx // 00c4bed5
        mov dword ptr [esi + 0ch], ecx // 00c4bed8
        jmp l_00c4bffc // 00c4bedb
    l_00c4bee0:
        mov esi, dword ptr [ecx + 8] // 00c4bee0
        add edx, 010h // 00c4bee3
        cmp esi, edx // 00c4bee6
        mov edi, ecx // 00c4bee8
        mov dword ptr [esp + 030h], edi // 00c4beea
        mov dword ptr [esp + 010h], esi // 00c4beee
        je l_00c4bffc // 00c4bef2
        jmp l_00c4bf04 // 00c4bef8
        lea ebx, [ebx] // 00c4befa
    l_00c4bf00:
        mov esi, dword ptr [esp + 010h] // 00c4bf00
    l_00c4bf04:
        fld dword ptr [esp + 01ch] // 00c4bf04
        fld dword ptr [esi] // 00c4bf08
        fcomip st(0), st(1) // 00c4bf0a
        fstp st(0) // 00c4bf0c
        jbe l_00c4bfce // 00c4bf0e
        mov eax, dword ptr [esi + 4] // 00c4bf14
        mov edi, eax // 00c4bf17
        and edi, 0fffffffeh // 00c4bf19
        test al, 1 // 00c4bf1c
        jne l_00c4bfa7 // 00c4bf1e
        cmp byte ptr [ebx + 01ch], 0 // 00c4bf24
        je l_00c4bf30 // 00c4bf28
        cmp byte ptr [edi + 01ch], 0 // 00c4bf2a
        jne l_00c4bfa7 // 00c4bf2e
    l_00c4bf30:
        mov ecx, ebp // 00c4bf30
        mov edx, 1 // 00c4bf32
        shl edx, cl // 00c4bf37
        mov esi, 1 // 00c4bf39
        and edx, 3 // 00c4bf3e
        mov eax, dword ptr [edi + edx*4 + 02ch] // 00c4bf41
        mov ecx, edx // 00c4bf45
        shl esi, cl // 00c4bf47
        mov ecx, dword ptr [ebx + edx*4 + 020h] // 00c4bf49
        fld dword ptr [ecx] // 00c4bf4d
        fld dword ptr [eax] // 00c4bf4f
        fxch st(1) // 00c4bf51
        and esi, 3 // 00c4bf53
        fcomip st(0), st(1) // 00c4bf56
        fstp st(0) // 00c4bf58
        ja l_00c4bfa3 // 00c4bf5a
        mov ecx, dword ptr [edi + edx*4 + 020h] // 00c4bf5c
        fld dword ptr [ecx] // 00c4bf60
        mov edx, dword ptr [ebx + edx*4 + 02ch] // 00c4bf62
        fld dword ptr [edx] // 00c4bf66
        fxch st(1) // 00c4bf68
        fcomip st(0), st(1) // 00c4bf6a
        fstp st(0) // 00c4bf6c
        ja l_00c4bfa3 // 00c4bf6e
        mov eax, dword ptr [ebx + esi*4 + 020h] // 00c4bf70
        fld dword ptr [eax] // 00c4bf74
        mov ecx, dword ptr [edi + esi*4 + 02ch] // 00c4bf76
        fld dword ptr [ecx] // 00c4bf7a
        fxch st(1) // 00c4bf7c
        fcomip st(0), st(1) // 00c4bf7e
        fstp st(0) // 00c4bf80
        ja l_00c4bfa3 // 00c4bf82
        mov edx, dword ptr [edi + esi*4 + 020h] // 00c4bf84
        fld dword ptr [edx] // 00c4bf88
        mov eax, dword ptr [ebx + esi*4 + 02ch] // 00c4bf8a
        fld dword ptr [eax] // 00c4bf8e
        fxch st(1) // 00c4bf90
        fcomip st(0), st(1) // 00c4bf92
        fstp st(0) // 00c4bf94
        ja l_00c4bfa3 // 00c4bf96
        push edi // 00c4bf98
        mov edi, dword ptr [esp + 02ch] // 00c4bf99
        push ebx // 00c4bf9d
        push dword ptr [esp+60] // Explicit borrowed context.
        call create_shim // 00c4bf9e
    l_00c4bfa3:
        mov esi, dword ptr [esp + 010h] // 00c4bfa3
    l_00c4bfa7:
        mov ecx, dword ptr [esp + 030h] // 00c4bfa7
        mov eax, dword ptr [esp + 018h] // 00c4bfab
        mov esi, dword ptr [esi + 8] // 00c4bfaf
        mov edx, dword ptr [ecx + 8] // 00c4bfb2
        mov ecx, dword ptr [esp + 014h] // 00c4bfb5
        add eax, 010h // 00c4bfb9
        cmp esi, eax // 00c4bfbc
        mov dword ptr [esp + 010h], esi // 00c4bfbe
        mov dword ptr [esp + 030h], edx // 00c4bfc2
        mov edi, edx // 00c4bfc6
        jne l_00c4bf00 // 00c4bfc8
    l_00c4bfce:
        cmp edi, ecx // 00c4bfce
        je l_00c4bffc // 00c4bfd0
        cmp dword ptr [edi + 8], ecx // 00c4bfd2
        je l_00c4bffc // 00c4bfd5
        mov edx, dword ptr [ecx + 8] // 00c4bfd7
        mov eax, dword ptr [esp + 020h] // 00c4bfda
        mov esi, dword ptr [eax] // 00c4bfde
        mov dword ptr [edx + 0ch], esi // 00c4bfe0
        mov edx, dword ptr [eax] // 00c4bfe3
        mov esi, dword ptr [ecx + 8] // 00c4bfe5
        mov dword ptr [edx + 8], esi // 00c4bfe8
        mov dword ptr [eax], edi // 00c4bfeb
        mov eax, dword ptr [edi + 8] // 00c4bfed
        mov dword ptr [ecx + 8], eax // 00c4bff0
        mov edx, dword ptr [edi + 8] // 00c4bff3
        mov dword ptr [edx + 0ch], ecx // 00c4bff6
        mov dword ptr [edi + 8], ecx // 00c4bff9
    l_00c4bffc:
        mov eax, dword ptr [esp + 02ch] // 00c4bffc
        mov ecx, dword ptr [eax + ebp*4 + 02ch] // 00c4c000
        mov eax, dword ptr [esp + 018h] // 00c4c004
        mov esi, dword ptr [ecx + 4] // 00c4c008
        mov edx, dword ptr [ecx + 0ch] // 00c4c00b
        movss xmm0, dword ptr [ecx] // 00c4c00e
        add eax, 020h // 00c4c012
        and esi, 0fffffffeh // 00c4c015
        cmp edx, eax // 00c4c018
        mov ebx, ecx // 00c4c01a
        mov dword ptr [esp + 014h], ecx // 00c4c01c
        movss dword ptr [esp + 01ch], xmm0 // 00c4c020
        mov dword ptr [esp + 030h], ebx // 00c4c026
        mov dword ptr [esp + 010h], edx // 00c4c02a
        jne l_00c4c059 // 00c4c02e
    l_00c4c030:
        mov eax, dword ptr [esp + 018h] // 00c4c030
        mov edx, dword ptr [ecx + 8] // 00c4c034
        add eax, 010h // 00c4c037
        cmp edx, eax // 00c4c03a
        mov ebx, ecx // 00c4c03c
        mov dword ptr [esp + 030h], ebx // 00c4c03e
        mov dword ptr [esp + 010h], edx // 00c4c042
        mov dword ptr [esp + 02ch], eax // 00c4c046
        je l_00c4c261 // 00c4c04a
        jmp l_00c4c168 // 00c4c050
    l_00c4c055:
        mov edx, dword ptr [esp + 010h] // 00c4c055
    l_00c4c059:
        fld dword ptr [edx] // 00c4c059
        fld dword ptr [esp + 01ch] // 00c4c05b
        fcomip st(0), st(1) // 00c4c05f
        fstp st(0) // 00c4c061
        jbe l_00c4c125 // 00c4c063
        mov eax, dword ptr [edx + 4] // 00c4c069
        mov edi, eax // 00c4c06c
        and edi, 0fffffffeh // 00c4c06e
        test al, 1 // 00c4c071
        je l_00c4c104 // 00c4c073
        cmp byte ptr [esi + 01ch], 0 // 00c4c079
        je l_00c4c085 // 00c4c07d
        cmp byte ptr [edi + 01ch], 0 // 00c4c07f
        jne l_00c4c104 // 00c4c083
    l_00c4c085:
        mov ecx, ebp // 00c4c085
        mov edx, 1 // 00c4c087
        shl edx, cl // 00c4c08c
        mov ebx, 1 // 00c4c08e
        and edx, 3 // 00c4c093
        mov eax, dword ptr [edi + edx*4 + 02ch] // 00c4c096
        mov ecx, edx // 00c4c09a
        shl ebx, cl // 00c4c09c
        mov ecx, dword ptr [esi + edx*4 + 020h] // 00c4c09e
        fld dword ptr [ecx] // 00c4c0a2
        fld dword ptr [eax] // 00c4c0a4
        fxch st(1) // 00c4c0a6
        and ebx, 3 // 00c4c0a8
        fcomip st(0), st(1) // 00c4c0ab
        fstp st(0) // 00c4c0ad
        ja l_00c4c0f8 // 00c4c0af
        mov ecx, dword ptr [edi + edx*4 + 020h] // 00c4c0b1
        fld dword ptr [ecx] // 00c4c0b5
        mov edx, dword ptr [esi + edx*4 + 02ch] // 00c4c0b7
        fld dword ptr [edx] // 00c4c0bb
        fxch st(1) // 00c4c0bd
        fcomip st(0), st(1) // 00c4c0bf
        fstp st(0) // 00c4c0c1
        ja l_00c4c0f8 // 00c4c0c3
        mov eax, dword ptr [esi + ebx*4 + 020h] // 00c4c0c5
        fld dword ptr [eax] // 00c4c0c9
        mov ecx, dword ptr [edi + ebx*4 + 02ch] // 00c4c0cb
        fld dword ptr [ecx] // 00c4c0cf
        fxch st(1) // 00c4c0d1
        fcomip st(0), st(1) // 00c4c0d3
        fstp st(0) // 00c4c0d5
        ja l_00c4c0f8 // 00c4c0d7
        mov edx, dword ptr [edi + ebx*4 + 020h] // 00c4c0d9
        fld dword ptr [edx] // 00c4c0dd
        mov eax, dword ptr [esi + ebx*4 + 02ch] // 00c4c0df
        fld dword ptr [eax] // 00c4c0e3
        fxch st(1) // 00c4c0e5
        fcomip st(0), st(1) // 00c4c0e7
        fstp st(0) // 00c4c0e9
        ja l_00c4c0f8 // 00c4c0eb
        push edi // 00c4c0ed
        mov edi, dword ptr [esp + 02ch] // 00c4c0ee
        push esi // 00c4c0f2
        push dword ptr [esp+60] // Explicit borrowed context.
        call create_shim // 00c4c0f3
    l_00c4c0f8:
        mov ecx, dword ptr [esp + 014h] // 00c4c0f8
        mov ebx, dword ptr [esp + 030h] // 00c4c0fc
        mov edx, dword ptr [esp + 010h] // 00c4c100
    l_00c4c104:
        mov eax, dword ptr [ebx + 0ch] // 00c4c104
        mov edx, dword ptr [edx + 0ch] // 00c4c107
        mov dword ptr [esp + 030h], eax // 00c4c10a
        mov eax, dword ptr [esp + 018h] // 00c4c10e
        mov ebx, dword ptr [esp + 030h] // 00c4c112
        add eax, 020h // 00c4c116
        cmp edx, eax // 00c4c119
        mov dword ptr [esp + 010h], edx // 00c4c11b
        jne l_00c4c055 // 00c4c11f
    l_00c4c125:
        cmp ecx, ebx // 00c4c125
        je l_00c4c030 // 00c4c127
        cmp dword ptr [ebx + 0ch], ecx // 00c4c12d
        je l_00c4c261 // 00c4c130
        mov edx, dword ptr [ecx + 8] // 00c4c136
        mov eax, dword ptr [ecx + 0ch] // 00c4c139
        mov dword ptr [edx + 0ch], eax // 00c4c13c
        mov edx, dword ptr [ecx + 0ch] // 00c4c13f
        mov eax, dword ptr [ecx + 8] // 00c4c142
        mov dword ptr [edx + 8], eax // 00c4c145
        mov edx, dword ptr [ebx + 0ch] // 00c4c148
        mov dword ptr [ecx + 8], ebx // 00c4c14b
        pop edi // 00c4c14e
        mov dword ptr [ecx + 0ch], edx // 00c4c14f
        mov eax, dword ptr [ebx + 0ch] // 00c4c152
        pop esi // 00c4c155
        mov dword ptr [eax + 8], ecx // 00c4c156
        pop ebp // 00c4c159
        mov dword ptr [ebx + 0ch], ecx // 00c4c15a
        pop ebx // 00c4c15d
        add esp, 014h // 00c4c15e
        ret 16 // 00c4c161
    l_00c4c164:
        mov edx, dword ptr [esp + 010h] // 00c4c164
    l_00c4c168:
        fld dword ptr [esp + 01ch] // 00c4c168
        fld dword ptr [edx] // 00c4c16c
        fcomip st(0), st(1) // 00c4c16e
        fstp st(0) // 00c4c170
        jbe l_00c4c234 // 00c4c172
        mov eax, dword ptr [edx + 4] // 00c4c178
        mov edi, eax // 00c4c17b
        and edi, 0fffffffeh // 00c4c17d
        test al, 1 // 00c4c180
        je l_00c4c21a // 00c4c182
        cmp byte ptr [esi + 01ch], 0 // 00c4c188
        je l_00c4c198 // 00c4c18c
        cmp byte ptr [edi + 01ch], 0 // 00c4c18e
        jne l_00c4c21a // 00c4c192
    l_00c4c198:
        mov ecx, ebp // 00c4c198
        mov edx, 1 // 00c4c19a
        shl edx, cl // 00c4c19f
        mov ebx, 1 // 00c4c1a1
        and edx, 3 // 00c4c1a6
        mov eax, dword ptr [edi + edx*4 + 02ch] // 00c4c1a9
        mov ecx, edx // 00c4c1ad
        shl ebx, cl // 00c4c1af
        mov ecx, dword ptr [esi + edx*4 + 020h] // 00c4c1b1
        fld dword ptr [ecx] // 00c4c1b5
        fld dword ptr [eax] // 00c4c1b7
        fxch st(1) // 00c4c1b9
        and ebx, 3 // 00c4c1bb
        fcomip st(0), st(1) // 00c4c1be
        fstp st(0) // 00c4c1c0
        ja l_00c4c20e // 00c4c1c2
        mov ecx, dword ptr [edi + edx*4 + 020h] // 00c4c1c4
        fld dword ptr [ecx] // 00c4c1c8
        mov edx, dword ptr [esi + edx*4 + 02ch] // 00c4c1ca
        fld dword ptr [edx] // 00c4c1ce
        fxch st(1) // 00c4c1d0
        fcomip st(0), st(1) // 00c4c1d2
        fstp st(0) // 00c4c1d4
        ja l_00c4c20e // 00c4c1d6
        mov eax, dword ptr [esi + ebx*4 + 020h] // 00c4c1d8
        fld dword ptr [eax] // 00c4c1dc
        mov ecx, dword ptr [edi + ebx*4 + 02ch] // 00c4c1de
        fld dword ptr [ecx] // 00c4c1e2
        fxch st(1) // 00c4c1e4
        fcomip st(0), st(1) // 00c4c1e6
        fstp st(0) // 00c4c1e8
        ja l_00c4c20e // 00c4c1ea
        mov edx, dword ptr [edi + ebx*4 + 020h] // 00c4c1ec
        fld dword ptr [edx] // 00c4c1f0
        mov eax, dword ptr [esi + ebx*4 + 02ch] // 00c4c1f2
        fld dword ptr [eax] // 00c4c1f6
        fxch st(1) // 00c4c1f8
        fcomip st(0), st(1) // 00c4c1fa
        fstp st(0) // 00c4c1fc
        ja l_00c4c20e // 00c4c1fe
        mov ecx, dword ptr [esp + 028h] // 00c4c200
        push ecx // 00c4c204
        mov ebx, edi // 00c4c205
        mov eax, esi // 00c4c207
        push dword ptr [esp+56] // Explicit borrowed context.
        call erase_shim // 00c4c209
    l_00c4c20e:
        mov ecx, dword ptr [esp + 014h] // 00c4c20e
        mov ebx, dword ptr [esp + 030h] // 00c4c212
        mov edx, dword ptr [esp + 010h] // 00c4c216
    l_00c4c21a:
        mov edx, dword ptr [edx + 8] // 00c4c21a
        cmp edx, dword ptr [esp + 02ch] // 00c4c21d
        mov eax, dword ptr [ebx + 8] // 00c4c221
        mov dword ptr [esp + 010h], edx // 00c4c224
        mov dword ptr [esp + 030h], eax // 00c4c228
        mov ebx, eax // 00c4c22c
        jne l_00c4c164 // 00c4c22e
    l_00c4c234:
        cmp ebx, ecx // 00c4c234
        je l_00c4c261 // 00c4c236
        cmp dword ptr [ebx + 8], ecx // 00c4c238
        je l_00c4c261 // 00c4c23b
        mov edx, dword ptr [ecx + 8] // 00c4c23d
        mov eax, dword ptr [ecx + 0ch] // 00c4c240
        mov dword ptr [edx + 0ch], eax // 00c4c243
        mov edx, dword ptr [ecx + 0ch] // 00c4c246
        mov eax, dword ptr [ecx + 8] // 00c4c249
        mov dword ptr [edx + 8], eax // 00c4c24c
        mov dword ptr [ecx + 0ch], ebx // 00c4c24f
        mov edx, dword ptr [ebx + 8] // 00c4c252
        mov dword ptr [ecx + 8], edx // 00c4c255
        mov eax, dword ptr [ebx + 8] // 00c4c258
        mov dword ptr [eax + 0ch], ecx // 00c4c25b
        mov dword ptr [ebx + 8], ecx // 00c4c25e
    l_00c4c261:
        pop edi // 00c4c261
        pop esi // 00c4c262
        pop ebp // 00c4c263
        pop ebx // 00c4c264
        add esp, 014h // 00c4c265
        ret 16 // 00c4c268
    }
}
// Recovered instruction schedule. Context is an extra final argument; native
// operand order, float spills and ordinary storage loads/stores are retained.
__declspec(naked) void __fastcall batch_kernel(void*,void*,Context*) {
    __asm {
        push ebp // 00c40140
        mov ebp, esp // 00c40141
        mov eax, 0104ch // 00c40143
        call _alloca_probe // 00c40148
        push ebx // 00c4014d
        push esi // 00c4014e
        mov esi, ecx // 00c4014f
        push edi // 00c40151
        mov dword ptr [ebp - 028h], esi // 00c40152
        rdtsc  // 00c40155
        mov eax, dword ptr [esi + 0240h] // 00c40157
        xor ebx, ebx // 00c4015d
        cmp eax, ebx // 00c4015f
        mov dword ptr [ebp - 048h], edx // 00c40161
        mov dword ptr [ebp - 0ch], eax // 00c40164
        je l_00c407f0 // 00c40167
        mov edi, eax // 00c4016d
        shl edi, 4 // 00c4016f
        add edi, 8 // 00c40172
        cmp edi, 0400h // 00c40175
        ja l_00c40192 // 00c4017b
        mov eax, edi // 00c4017d
        call _alloca_probe_16 // 00c4017f
        mov eax, esp // 00c40184
        cmp eax, ebx // 00c40186
        je l_00c401a8 // 00c40188
        mov dword ptr [eax], 0cccch // 00c4018a
        jmp l_00c401a5 // 00c40190
    l_00c40192:
        push edi // 00c40192
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c40193 // 00c40193
        add esp, 4 // 00c40198
        cmp eax, ebx // 00c4019b
        je l_00c401a8 // 00c4019d
        mov dword ptr [eax], 0ddddh // 00c4019f
    l_00c401a5:
        add eax, 8 // 00c401a5
    l_00c401a8:
        cmp edi, 0400h // 00c401a8
        mov dword ptr [ebp - 8], eax // 00c401ae
        ja l_00c401c8 // 00c401b1
        mov eax, edi // 00c401b3
        call _alloca_probe_16 // 00c401b5
        mov eax, esp // 00c401ba
        cmp eax, ebx // 00c401bc
        je l_00c401de // 00c401be
        mov dword ptr [eax], 0cccch // 00c401c0
        jmp l_00c401db // 00c401c6
    l_00c401c8:
        push edi // 00c401c8
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c401c9 // 00c401c9
        add esp, 4 // 00c401ce
        cmp eax, ebx // 00c401d1
        je l_00c401de // 00c401d3
        mov dword ptr [eax], 0ddddh // 00c401d5
    l_00c401db:
        add eax, 8 // 00c401db
    l_00c401de:
        mov dword ptr [ebp - 4], eax // 00c401de
        mov eax, dword ptr [esi + 01e4h] // 00c401e1
        lea ecx, [esi + 01e8h] // 00c401e7
        cmp eax, ecx // 00c401ed
        je l_00c401fb // 00c401ef
    l_00c401f1:
        mov dword ptr [eax + 040h], ebx // 00c401f1
        mov eax, dword ptr [eax + 04ch] // 00c401f4
        cmp eax, ecx // 00c401f7
        jne l_00c401f1 // 00c401f9
    l_00c401fb:
        mov eax, dword ptr [esi + 0130h] // 00c401fb
        lea ecx, [esi + 0134h] // 00c40201
        cmp eax, ecx // 00c40207
        je l_00c4021a // 00c40209
        jmp l_00c40210 // 00c4020b
        lea ecx, [ecx] // 00c4020d
    l_00c40210:
        mov dword ptr [eax + 040h], ebx // 00c40210
        mov eax, dword ptr [eax + 04ch] // 00c40213
        cmp eax, ecx // 00c40216
        jne l_00c40210 // 00c40218
    l_00c4021a:
        lea eax, [esi + 024h] // 00c4021a
        mov dword ptr [ebp - 01ch], 020h // 00c4021d
        mov dword ptr [ebp - 018h], eax // 00c40224
        jmp l_00c40233 // 00c40227
        lea esp, [esp] // 00c40229
    l_00c40230:
        mov esi, dword ptr [ebp - 028h] // 00c40230
    l_00c40233:
        mov edi, dword ptr [ebp - 0ch] // 00c40233
        xor ecx, ecx // 00c40236
        test edi, edi // 00c40238
        jle l_00c4029c // 00c4023a
        mov eax, dword ptr [ebp - 8] // 00c4023c
        add eax, 8 // 00c4023f
    l_00c40242:
        mov edx, dword ptr [esi + 023ch] // 00c40242
        mov edx, dword ptr [edx + ecx*4] // 00c40248
        mov byte ptr [edx + 038h], 1 // 00c4024b
        mov edx, dword ptr [esi + 023ch] // 00c4024f
        mov edx, dword ptr [edx + ecx*4] // 00c40255
        lea ebx, [eax - 8] // 00c40258
        add edx, 4 // 00c4025b
        test ebx, ebx // 00c4025e
        je l_00c40271 // 00c40260
        mov ebx, dword ptr [esi + 023ch] // 00c40262
        mov ebx, dword ptr [ebx + ecx*4] // 00c40268
        or ebx, 1 // 00c4026b
        mov dword ptr [eax - 4], ebx // 00c4026e
    l_00c40271:
        test eax, eax // 00c40271
        je l_00c40281 // 00c40273
        mov ebx, dword ptr [esi + 023ch] // 00c40275
        mov ebx, dword ptr [ebx + ecx*4] // 00c4027b
        mov dword ptr [eax + 4], ebx // 00c4027e
    l_00c40281:
        mov ebx, dword ptr [ebp - 01ch] // 00c40281
        fld dword ptr [ebx + edx - 020h] // 00c40284
        add ecx, 1 // 00c40288
        fstp dword ptr [eax - 8] // 00c4028b
        add eax, 010h // 00c4028e
        cmp ecx, edi // 00c40291
        fld dword ptr [ebx + edx - 014h] // 00c40293
        fstp dword ptr [eax - 010h] // 00c40297
        jl l_00c40242 // 00c4029a
    l_00c4029c:
        push 01000h // 00c4029c
        lea eax, [ebp - 0104ch] // 00c402a1
        push 0 // 00c402a7
        push eax // 00c402a9
        call memset_bridge // 00c402aa
        lea esi, [edi + edi] // 00c402af
        add esp, 0ch // 00c402b2
        xor ecx, ecx // 00c402b5
        test esi, esi // 00c402b7
        jle l_00c4034d // 00c402b9
        nop  // 00c402bf
    l_00c402c0:
        mov edx, dword ptr [ebp - 8] // 00c402c0
        movss xmm0, dword ptr [edx + ecx*8] // 00c402c3
        movss dword ptr [ebp - 02ch], xmm0 // 00c402c8
        mov edx, dword ptr [ebp - 02ch] // 00c402cd
        mov eax, edx // 00c402d0
        shr eax, 01fh // 00c402d2
        not eax // 00c402d5
        add eax, 1 // 00c402d7
        or eax, 080000000h // 00c402da
        xor eax, edx // 00c402df
        mov edx, eax // 00c402e1
        and edx, 0ffh // 00c402e3
        shl edx, 4 // 00c402e9
        add dword ptr [ebp + edx - 0104ch], 1 // 00c402ec
        lea edx, [ebp + edx - 0104ch] // 00c402f4
        movzx edx, ah // 00c402fb
        shl edx, 4 // 00c402fe
        add dword ptr [ebp + edx - 01048h], 1 // 00c40301
        lea edx, [ebp + edx - 01048h] // 00c40309
        mov edx, eax // 00c40310
        shr edx, 010h // 00c40312
        and edx, 0ffh // 00c40315
        shr eax, 018h // 00c4031b
        shl edx, 4 // 00c4031e
        add dword ptr [ebp + edx - 01044h], 1 // 00c40321
        shl eax, 4 // 00c40329
        add dword ptr [ebp + eax - 01040h], 1 // 00c4032c
        lea edx, [ebp + edx - 01044h] // 00c40334
        lea eax, [ebp + eax - 01040h] // 00c4033b
        add ecx, 1 // 00c40342
        cmp ecx, esi // 00c40345
        jl l_00c402c0 // 00c40347
    l_00c4034d:
        xor ecx, ecx // 00c4034d
        xor edx, edx // 00c4034f
        xor esi, esi // 00c40351
        xor edi, edi // 00c40353
        lea eax, [ebp - 01048h] // 00c40355
        mov dword ptr [ebp - 010h], 0100h // 00c4035b
    l_00c40362:
        mov ebx, dword ptr [eax - 4] // 00c40362
        mov dword ptr [eax - 4], ecx // 00c40365
        add ecx, ebx // 00c40368
        mov ebx, dword ptr [eax] // 00c4036a
        mov dword ptr [eax], edx // 00c4036c
        add edx, ebx // 00c4036e
        mov ebx, dword ptr [eax + 4] // 00c40370
        mov dword ptr [eax + 4], esi // 00c40373
        add esi, ebx // 00c40376
        mov ebx, dword ptr [eax + 8] // 00c40378
        mov dword ptr [eax + 8], edi // 00c4037b
        add edi, ebx // 00c4037e
        add eax, 010h // 00c40380
        sub dword ptr [ebp - 010h], 1 // 00c40383
        mov dword ptr [ebp - 014h], ebx // 00c40387
        jne l_00c40362 // 00c4038a
        mov esi, dword ptr [ebp - 0ch] // 00c4038c
        mov edi, dword ptr [ebp - 4] // 00c4038f
        add esi, esi // 00c40392
        test esi, esi // 00c40394
        jle l_00c404da // 00c40396
        mov ecx, dword ptr [ebp - 8] // 00c4039c
    l_00c4039f:
        movss xmm0, dword ptr [ecx] // 00c4039f
        movss dword ptr [ebp - 030h], xmm0 // 00c403a3
        mov edx, dword ptr [ebp - 030h] // 00c403a8
        mov eax, edx // 00c403ab
        shr eax, 01fh // 00c403ad
        not eax // 00c403b0
        add eax, 1 // 00c403b2
        xor eax, edx // 00c403b5
        and eax, 0ffh // 00c403b7
        shl eax, 4 // 00c403bc
        lea edx, [ebp + eax - 0104ch] // 00c403bf
        mov eax, dword ptr [edx] // 00c403c6
        lea ebx, [eax + 1] // 00c403c8
        mov dword ptr [edx], ebx // 00c403cb
        mov edx, dword ptr [ecx] // 00c403cd
        mov dword ptr [edi + eax*8], edx // 00c403cf
        mov edx, dword ptr [ecx + 4] // 00c403d2
        add ecx, 8 // 00c403d5
        sub esi, 1 // 00c403d8
        mov dword ptr [edi + eax*8 + 4], edx // 00c403db
        jne l_00c4039f // 00c403df
        mov esi, dword ptr [ebp - 0ch] // 00c403e1
        mov eax, edi // 00c403e4
        add esi, esi // 00c403e6
        jmp l_00c403f0 // 00c403e8
        lea ebx, [ebx] // 00c403ea
    l_00c403f0:
        movss xmm0, dword ptr [eax] // 00c403f0
        movss dword ptr [ebp - 034h], xmm0 // 00c403f4
        mov ecx, dword ptr [ebp - 034h] // 00c403f9
        mov edx, ecx // 00c403fc
        shr edx, 01fh // 00c403fe
        not edx // 00c40401
        add edx, 1 // 00c40403
        xor edx, ecx // 00c40406
        movzx ecx, dh // 00c40408
        shl ecx, 4 // 00c4040b
        lea edx, [ebp + ecx - 01048h] // 00c4040e
        mov ecx, dword ptr [edx] // 00c40415
        lea ebx, [ecx + 1] // 00c40417
        mov dword ptr [edx], ebx // 00c4041a
        mov ebx, dword ptr [eax] // 00c4041c
        mov edx, dword ptr [ebp - 8] // 00c4041e
        mov dword ptr [edx + ecx*8], ebx // 00c40421
        mov ebx, dword ptr [eax + 4] // 00c40424
        add eax, 8 // 00c40427
        sub esi, 1 // 00c4042a
        mov dword ptr [edx + ecx*8 + 4], ebx // 00c4042d
        jne l_00c403f0 // 00c40431
        mov esi, dword ptr [ebp - 0ch] // 00c40433
        mov ecx, edx // 00c40436
        add esi, esi // 00c40438
        lea ebx, [ebx] // 00c4043a
    l_00c40440:
        movss xmm0, dword ptr [ecx] // 00c40440
        movss dword ptr [ebp - 03ch], xmm0 // 00c40444
        mov edx, dword ptr [ebp - 03ch] // 00c40449
        mov eax, edx // 00c4044c
        shr eax, 01fh // 00c4044e
        not eax // 00c40451
        add eax, 1 // 00c40453
        xor eax, edx // 00c40456
        shr eax, 010h // 00c40458
        and eax, 0ffh // 00c4045b
        shl eax, 4 // 00c40460
        lea edx, [ebp + eax - 01044h] // 00c40463
        mov eax, dword ptr [edx] // 00c4046a
        lea ebx, [eax + 1] // 00c4046c
        mov dword ptr [edx], ebx // 00c4046f
        mov edx, dword ptr [ecx] // 00c40471
        mov dword ptr [edi + eax*8], edx // 00c40473
        mov edx, dword ptr [ecx + 4] // 00c40476
        add ecx, 8 // 00c40479
        sub esi, 1 // 00c4047c
        mov dword ptr [edi + eax*8 + 4], edx // 00c4047f
        jne l_00c40440 // 00c40483
        mov esi, dword ptr [ebp - 0ch] // 00c40485
        mov ecx, edi // 00c40488
        add esi, esi // 00c4048a
        lea esp, [esp] // 00c4048c
    l_00c40490:
        movss xmm0, dword ptr [ecx] // 00c40490
        movss dword ptr [ebp - 038h], xmm0 // 00c40494
        mov edx, dword ptr [ebp - 038h] // 00c40499
        mov eax, edx // 00c4049c
        shr eax, 01fh // 00c4049e
        not eax // 00c404a1
        add eax, 1 // 00c404a3
        or eax, 080ffffffh // 00c404a6
        xor eax, edx // 00c404ab
        shr eax, 018h // 00c404ad
        shl eax, 4 // 00c404b0
        lea edx, [ebp + eax - 01040h] // 00c404b3
        mov eax, dword ptr [edx] // 00c404ba
        lea ebx, [eax + 1] // 00c404bc
        mov dword ptr [edx], ebx // 00c404bf
        mov ebx, dword ptr [ecx] // 00c404c1
        mov edx, dword ptr [ebp - 8] // 00c404c3
        mov dword ptr [edx + eax*8], ebx // 00c404c6
        mov ebx, dword ptr [ecx + 4] // 00c404c9
        add ecx, 8 // 00c404cc
        sub esi, 1 // 00c404cf
        mov dword ptr [edx + eax*8 + 4], ebx // 00c404d2
        jne l_00c40490 // 00c404d6
        jmp l_00c404dd // 00c404d8
    l_00c404da:
        mov edx, dword ptr [ebp - 8] // 00c404da
    l_00c404dd:
        mov eax, dword ptr [ebp - 0ch] // 00c404dd
        mov esi, dword ptr [ebp - 018h] // 00c404e0
        add eax, eax // 00c404e3
        test eax, eax // 00c404e5
        mov dword ptr [ebp - 014h], esi // 00c404e7
        jle l_00c4065f // 00c404ea
        lea ecx, [edx + eax*8 - 8] // 00c404f0
        mov dword ptr [ebp - 010h], ecx // 00c404f4
        mov dword ptr [ebp - 018h], eax // 00c404f7
        lea ebx, [ebx] // 00c404fa
    l_00c40500:
        cmp dword ptr [esi - 014h], 0 // 00c40500
        jne l_00c4059d // 00c40504
        push 03e80h // 00c4050a
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c4050f // 00c4050f
        mov ebx, eax // 00c40514
        add esp, 4 // 00c40516
        lea eax, [ebx + 0ch] // 00c40519
        mov ecx, 03e7h // 00c4051c
    l_00c40521:
        lea edx, [eax + 4] // 00c40521
        mov dword ptr [eax], edx // 00c40524
        add eax, 010h // 00c40526
        sub ecx, 1 // 00c40529
        jne l_00c40521 // 00c4052c
        mov dword ptr [ebx + 03e7ch], ecx // 00c4052e
        mov eax, dword ptr [esi - 018h] // 00c40534
        cmp dword ptr [esi - 01ch], eax // 00c40537
        mov dword ptr [esi - 014h], ebx // 00c4053a
        jne l_00c4058a // 00c4053d
        lea eax, [eax + eax + 2] // 00c4053f
        mov dword ptr [esi - 018h], eax // 00c40543
        add eax, eax // 00c40546
        add eax, eax // 00c40548
        push eax // 00c4054a
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c4054b // 00c4054b
        mov edi, eax // 00c40550
        xor eax, eax // 00c40552
        add esp, 4 // 00c40554
        cmp dword ptr [esi - 01ch], eax // 00c40557
        jbe l_00c40577 // 00c4055a
        mov ecx, edi // 00c4055c
        mov edi, edi // 00c4055e
    l_00c40560:
        test ecx, ecx // 00c40560
        je l_00c4056c // 00c40562
        mov edx, dword ptr [esi - 020h] // 00c40564
        mov edx, dword ptr [edx + eax*4] // 00c40567
        mov dword ptr [ecx], edx // 00c4056a
    l_00c4056c:
        add eax, 1 // 00c4056c
        add ecx, 4 // 00c4056f
        cmp eax, dword ptr [esi - 01ch] // 00c40572
        jb l_00c40560 // 00c40575
    l_00c40577:
        mov eax, dword ptr [esi - 020h] // 00c40577
        test eax, eax // 00c4057a
        je l_00c40587 // 00c4057c
        push eax // 00c4057e
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c4057f // 00c4057f
        add esp, 4 // 00c40584
    l_00c40587:
        mov dword ptr [esi - 020h], edi // 00c40587
    l_00c4058a:
        mov eax, dword ptr [esi - 01ch] // 00c4058a
        mov ecx, dword ptr [esi - 020h] // 00c4058d
        lea eax, [ecx + eax*4] // 00c40590
        test eax, eax // 00c40593
        je l_00c40599 // 00c40595
        mov dword ptr [eax], ebx // 00c40597
    l_00c40599:
        add dword ptr [esi - 01ch], 1 // 00c40599
    l_00c4059d:
        mov ecx, dword ptr [esi - 014h] // 00c4059d
        mov edx, dword ptr [ecx + 0ch] // 00c405a0
        add dword ptr [esi + 010h], 1 // 00c405a3
        mov dword ptr [esi - 014h], edx // 00c405a7
        mov eax, esi // 00c405aa
        mov dword ptr [ecx + 0ch], eax // 00c405ac
        mov edx, dword ptr [esi + 8] // 00c405af
        mov dword ptr [ecx + 8], edx // 00c405b2
        mov eax, dword ptr [esi + 8] // 00c405b5
        mov dword ptr [eax + 0ch], ecx // 00c405b8
        mov eax, dword ptr [ebp - 010h] // 00c405bb
        mov dword ptr [esi + 8], ecx // 00c405be
        mov edx, dword ptr [eax] // 00c405c1
        mov dword ptr [ecx], edx // 00c405c3
        mov eax, dword ptr [eax + 4] // 00c405c5
        test al, 1 // 00c405c8
        mov edx, dword ptr [ebp - 01ch] // 00c405ca
        mov dword ptr [ecx + 4], eax // 00c405cd
        je l_00c405da // 00c405d0
        and eax, 0fffffffeh // 00c405d2
        mov dword ptr [eax + edx], ecx // 00c405d5
        jmp l_00c405e1 // 00c405d8
    l_00c405da:
        and eax, 0fffffffeh // 00c405da
        mov dword ptr [eax + edx + 0ch], ecx // 00c405dd
    l_00c405e1:
        cmp dword ptr [ebp - 014h], esi // 00c405e1
        je l_00c405eb // 00c405e4
        lea eax, [ebp - 014h] // 00c405e6
        jmp l_00c405f1 // 00c405e9
    l_00c405eb:
        mov dword ptr [ebp - 020h], ecx // 00c405eb
        lea eax, [ebp - 020h] // 00c405ee
    l_00c405f1:
        mov edx, dword ptr [eax] // 00c405f1
        mov edi, dword ptr [edx + 8] // 00c405f3
        lea ebx, [esi - 010h] // 00c405f6
        cmp edi, ebx // 00c405f9
        je l_00c4061b // 00c405fb
        movss xmm0, dword ptr [ecx] // 00c405fd
        movss dword ptr [ebp - 024h], xmm0 // 00c40601
        fld dword ptr [ebp - 024h] // 00c40606
    l_00c40609:
        fld dword ptr [edi] // 00c40609
        fcomip st(0), st(1) // 00c4060b
        jbe l_00c40619 // 00c4060d
        mov edi, dword ptr [edi + 8] // 00c4060f
        cmp edi, ebx // 00c40612
        mov edx, dword ptr [edx + 8] // 00c40614
        jne l_00c40609 // 00c40617
    l_00c40619:
        fstp st(0) // 00c40619
    l_00c4061b:
        cmp edx, ecx // 00c4061b
        je l_00c40648 // 00c4061d
        cmp dword ptr [edx + 8], ecx // 00c4061f
        je l_00c40648 // 00c40622
        mov eax, dword ptr [ecx + 8] // 00c40624
        mov edi, dword ptr [ecx + 0ch] // 00c40627
        mov dword ptr [eax + 0ch], edi // 00c4062a
        mov eax, dword ptr [ecx + 0ch] // 00c4062d
        mov edi, dword ptr [ecx + 8] // 00c40630
        mov dword ptr [eax + 8], edi // 00c40633
        mov dword ptr [ecx + 0ch], edx // 00c40636
        mov eax, dword ptr [edx + 8] // 00c40639
        mov dword ptr [ecx + 8], eax // 00c4063c
        mov eax, dword ptr [edx + 8] // 00c4063f
        mov dword ptr [eax + 0ch], ecx // 00c40642
        mov dword ptr [edx + 8], ecx // 00c40645
    l_00c40648:
        sub dword ptr [ebp - 010h], 8 // 00c40648
        sub dword ptr [ebp - 018h], 1 // 00c4064c
        mov dword ptr [ebp - 014h], ecx // 00c40650
        jne l_00c40500 // 00c40653
        mov edi, dword ptr [ebp - 4] // 00c40659
        mov edx, dword ptr [ebp - 8] // 00c4065c
    l_00c4065f:
        mov eax, dword ptr [ebp - 01ch] // 00c4065f
        add eax, 4 // 00c40662
        add esi, 034h // 00c40665
        cmp eax, 02ch // 00c40668
        mov dword ptr [ebp - 018h], esi // 00c4066b
        mov dword ptr [ebp - 01ch], eax // 00c4066e
        jl l_00c40230 // 00c40671
        test edx, edx // 00c40677
        je l_00c40690 // 00c40679
        cmp dword ptr [edx - 8], 0ddddh // 00c4067b
        lea eax, [edx - 8] // 00c40682
        jne l_00c40690 // 00c40685
        push eax // 00c40687
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c40688 // 00c40688
        add esp, 4 // 00c4068d
    l_00c40690:
        test edi, edi // 00c40690
        je l_00c406a9 // 00c40692
        cmp dword ptr [edi - 8], 0ddddh // 00c40694
        lea eax, [edi - 8] // 00c4069b
        jne l_00c406a9 // 00c4069e
        push eax // 00c406a0
        push dword ptr [ebp+8] // Explicit borrowed context.
        call bridge_00c406a1 // 00c406a1
        add esp, 4 // 00c406a6
    l_00c406a9:
        mov edi, dword ptr [ebp - 028h] // 00c406a9
        xor edx, edx // 00c406ac
        mov dword ptr [edi + 0240h], edx // 00c406ae
        cmp dword ptr [edi + 0d0h], edx // 00c406b4
        je l_00c40705 // 00c406ba
        mov eax, dword ptr [edi + 0bch] // 00c406bc
        lea ecx, [edi + 0c0h] // 00c406c2
        cmp dword ptr [eax + 0ch], ecx // 00c406c8
        je l_00c406d8 // 00c406cb
        lea ecx, [ecx] // 00c406cd
    l_00c406d0:
        mov eax, dword ptr [eax + 0ch] // 00c406d0
        cmp dword ptr [eax + 0ch], ecx // 00c406d3
        jne l_00c406d0 // 00c406d6
    l_00c406d8:
        mov esi, dword ptr [edi + 0ach] // 00c406d8
        mov dword ptr [eax + 0ch], esi // 00c406de
        mov eax, dword ptr [edi + 0bch] // 00c406e1
        mov dword ptr [edi + 0bch], ecx // 00c406e7
        lea ecx, [edi + 0b0h] // 00c406ed
        mov dword ptr [edi + 0ach], eax // 00c406f3
        mov dword ptr [edi + 0c8h], ecx // 00c406f9
        mov dword ptr [edi + 0d0h], edx // 00c406ff
    l_00c40705:
        mov eax, dword ptr [edi + 020h] // 00c40705
        lea ecx, [edi + 024h] // 00c40708
        cmp eax, ecx // 00c4070b
        mov dword ptr [ebp - 4], eax // 00c4070d
        je l_00c407f0 // 00c40710
        jmp l_00c40720 // 00c40716
        lea esp, [esp] // 00c40718
        nop  // 00c4071f
    l_00c40720:
        mov eax, dword ptr [eax + 4] // 00c40720
        test al, 1 // 00c40723
        je l_00c407dc // 00c40725
        mov edx, dword ptr [ebp - 4] // 00c4072b
        mov ebx, dword ptr [edx + 0ch] // 00c4072e
        and eax, 0fffffffeh // 00c40731
        add eax, 4 // 00c40734
        mov esi, eax // 00c40737
        lea eax, [edi + 024h] // 00c40739
        cmp ebx, eax // 00c4073c
        je l_00c407dc // 00c4073e
    l_00c40744:
        mov ecx, dword ptr [ebx + 4] // 00c40744
        test cl, 1 // 00c40747
        je l_00c407c8 // 00c4074a
        and ecx, 0fffffffeh // 00c4074c
        fld dword ptr [ecx + 4] // 00c4074f
        fstp dword ptr [ebp - 024h] // 00c40752
        fld dword ptr [esi + 0ch] // 00c40755
        fstp dword ptr [ebp - 020h] // 00c40758
        fld dword ptr [ebp - 020h] // 00c4075b
        fld dword ptr [ebp - 024h] // 00c4075e
        fcomi st(0), st(1) // 00c40761
        ja l_00c407d8 // 00c40763
        mov eax, dword ptr [ebp - 4] // 00c40765
        mov edx, dword ptr [eax + 4] // 00c40768
        and edx, 0fffffffeh // 00c4076b
        cmp byte ptr [edx + 01ch], 0 // 00c4076e
        je l_00c4077a // 00c40772
        cmp byte ptr [ecx + 01ch], 0 // 00c40774
        jne l_00c407c4 // 00c40778
    l_00c4077a:
        fcomip st(0), st(1) // 00c4077a
        fstp st(0) // 00c4077c
        ja l_00c407c8 // 00c4077e
        fld dword ptr [esi + 010h] // 00c40780
        fld dword ptr [ecx + 8] // 00c40783
        fcomip st(0), st(1) // 00c40786
        fstp st(0) // 00c40788
        ja l_00c407c8 // 00c4078a
        fld dword ptr [esi + 014h] // 00c4078c
        fld dword ptr [ecx + 0ch] // 00c4078f
        fcomip st(0), st(1) // 00c40792
        fstp st(0) // 00c40794
        ja l_00c407c8 // 00c40796
        fld dword ptr [ecx + 010h] // 00c40798
        fld dword ptr [esi] // 00c4079b
        fcomip st(0), st(1) // 00c4079d
        fstp st(0) // 00c4079f
        ja l_00c407c8 // 00c407a1
        fld dword ptr [ecx + 014h] // 00c407a3
        fld dword ptr [esi + 4] // 00c407a6
        fcomip st(0), st(1) // 00c407a9
        fstp st(0) // 00c407ab
        ja l_00c407c8 // 00c407ad
        fld dword ptr [ecx + 018h] // 00c407af
        fld dword ptr [esi + 8] // 00c407b2
        fcomip st(0), st(1) // 00c407b5
        fstp st(0) // 00c407b7
        ja l_00c407c8 // 00c407b9
        push ecx // 00c407bb
        push edx // 00c407bc
        push dword ptr [ebp+8] // Explicit borrowed context.
        call create_shim // 00c407bd
        jmp l_00c407c8 // 00c407c2
    l_00c407c4:
        fstp st(0) // 00c407c4
        fstp st(0) // 00c407c6
    l_00c407c8:
        mov ebx, dword ptr [ebx + 0ch] // 00c407c8
        lea eax, [edi + 024h] // 00c407cb
        cmp ebx, eax // 00c407ce
        jne l_00c40744 // 00c407d0
        jmp l_00c407dc // 00c407d6
    l_00c407d8:
        fstp st(0) // 00c407d8
        fstp st(0) // 00c407da
    l_00c407dc:
        mov ecx, dword ptr [ebp - 4] // 00c407dc
        mov eax, dword ptr [ecx + 0ch] // 00c407df
        lea ecx, [edi + 024h] // 00c407e2
        cmp eax, ecx // 00c407e5
        mov dword ptr [ebp - 4], eax // 00c407e7
        jne l_00c40720 // 00c407ea
    l_00c407f0:
        lea esp, [ebp - 01058h] // 00c407f0
        pop edi // 00c407f6
        pop esi // 00c407f7
        pop ebx // 00c407f8
        mov esp, ebp // 00c407f9
        pop ebp // 00c407fb
        ret 4 // 00c407fc
    }
}
} // namespace
void allocate_native_dyn_sap_endpoints_00c36c30(void* manager,void* proxy,U axis,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.proxy=proxy;void* pool=at(manager,4+axis*0x34);
    const auto take=[&](U malloc_site,U allocate_site,U free_site){
        if(!ptr(pool,12)){
            o.native_site=malloc_site;void* page=c.sap_malloc_00bf9f1a(16000,m);
            for(U i=0;i<999;++i)ptr(page,i*16+12,at(page,(i+1)*16));word(page,0x3e7c)=0;
            const U capacity=word(pool,8);const bool grow=word(pool,4)==capacity;ptr(pool,12,page);
            if(grow){const U new_capacity=capacity*2+2;word(pool,8)=new_capacity;o.native_site=allocate_site;void* grown=c.sap_allocate_00bf55be(new_capacity*4,m);
                U index=0;void* output=grown;while(index<word(pool,4)){if(output)word(output)=word(at(ptr(pool),index*4));++index;output=at(output,4);}
                if(void* old=ptr(pool)){o.native_site=free_site;o.cursor=old;c.sap_free_00bf6989(old,m);}ptr(pool,0,grown);
            }
            void* output=at(ptr(pool),word(pool,4)*4);if(output)ptr(output,0,page);word(pool,4)=word(pool,4)+1;
        }
        void* endpoint=ptr(pool,12);void* next=ptr(endpoint,12);word(pool,0x30)=word(pool,0x30)+1;ptr(pool,12,next);ptr(endpoint,12,at(pool,0x20));ptr(endpoint,8,ptr(pool,0x28));ptr(ptr(pool,0x28),12,endpoint);ptr(pool,0x28,endpoint);return endpoint;
    };
    void* lower=take(0xc36c51,0xc36c8d,0xc36cbd);void* upper=take(0xc36d14,0xc36d4f,0xc36d7f);
    word(lower,4)=address(proxy)|1;ptr(upper,4,proxy);ptr(proxy,0x20+axis*4,lower);ptr(proxy,0x2c+axis*4,upper);
}
void insert_native_dyn_sap_endpoints_00c36e20(void* manager,void* proxy,U axis) noexcept {insert_kernel(proxy,axis,manager);}
void decide_native_dyn_sap_pair_00c4bd60(void* manager,void* first,void* peer,U axis,U add,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){Context context{m,c,o};decision_kernel(peer,manager,first,add,axis,&context);}
void move_native_dyn_sap_endpoints_00c4be10(void* manager,void* proxy,U axis,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){Context context{m,c,o};move_kernel(manager,proxy,axis,&context);}
void insert_native_dyn_sap_pending_00c4c2a0(void* manager,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    U index=0;while(index<word(manager,0x240)){void* proxy=ptr(at(ptr(manager,0x23c),index*4));
        for(U axis=0;axis<3;++axis)allocate_native_dyn_sap_endpoints_00c36c30(manager,proxy,axis,m,c,o);
        insert_native_dyn_sap_endpoints_00c36e20(manager,proxy,0);insert_native_dyn_sap_endpoints_00c36e20(manager,proxy,1);move_native_dyn_sap_endpoints_00c4be10(manager,proxy,2,m,c,o);
        ++index;*static_cast<volatile unsigned char*>(at(proxy,0x38))=1;
    }word(manager,0x240)=0;
}
void update_native_dyn_sap_proxy_00c4c270(void* manager,void* proxy,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){if(*static_cast<volatile unsigned char*>(at(proxy,0x38)))for(U axis=0;axis<3;++axis)move_native_dyn_sap_endpoints_00c4be10(manager,proxy,axis,m,c,o);}
void batch_native_dyn_sap_pending_00c40140(void* manager,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){Context context{m,c,o};o.manager=manager;batch_kernel(manager,nullptr,&context);}
void process_native_dyn_sap_00c4c320(void* manager,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    if(word(manager,0x240)>50){batch_native_dyn_sap_pending_00c40140(manager,m,c,o);return;}
    insert_native_dyn_sap_pending_00c4c2a0(manager,m,c,o);void* proxy=ptr(manager,0x130);void* sentinel=at(manager,0x134);
    while(proxy!=sentinel){for(U axis=0;axis<3;++axis)move_native_dyn_sap_endpoints_00c4be10(manager,proxy,axis,m,c,o);proxy=ptr(proxy,0x4c);}
}
static_assert(std::is_standard_layout_v<NativeDynSapRuntime>);
NativeDynSapRuntime::NativeDynSapRuntime(const AvoidZoneDynHullMemory& memory,NativeDynSapPairCalls& calls,NativeDynSapLifetimeProgress& progress) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&create),reinterpret_cast<std::uintptr_t>(&remove),reinterpret_cast<std::uintptr_t>(&update),reinterpret_cast<std::uintptr_t>(&process),reinterpret_cast<std::uintptr_t>(&count),reinterpret_cast<std::uintptr_t>(&first),reinterpret_cast<std::uintptr_t>(&next),reinterpret_cast<std::uintptr_t>(&scalar)},memory_(memory),calls_(&calls),progress_(&progress) {}
NativeDynSapRuntime& NativeDynSapRuntime::owner(void* manager) noexcept {return *static_cast<NativeDynSapRuntime*>(ptr(manager));}
void* __fastcall NativeDynSapRuntime::create(void* manager,void*,DynBodyStorage* body,const DynAabb* bounds,U is_static){auto& r=owner(manager);return dyn_sap_create_proxy_00c54aa0(manager,*body,*bounds,static_cast<unsigned char>(is_static)!=0,r.memory_);}
void __fastcall NativeDynSapRuntime::remove(void* manager,void*,void* proxy){auto& r=owner(manager);remove_native_dyn_sap_proxy_00c4c380(manager,proxy,r.memory_,*r.calls_,*r.progress_);}
void __fastcall NativeDynSapRuntime::update(void* manager,void*,void* proxy){auto& r=owner(manager);update_native_dyn_sap_proxy_00c4c270(manager,proxy,r.memory_,*r.calls_,*r.progress_);}
void __fastcall NativeDynSapRuntime::process(void* manager,void*){auto& r=owner(manager);process_native_dyn_sap_00c4c320(manager,r.memory_,*r.calls_,*r.progress_);}
U __fastcall NativeDynSapRuntime::count(void* manager,void*){return count_native_dyn_sap_pairs_00c32b30(manager);}
void* __fastcall NativeDynSapRuntime::first(void* manager,void*){return first_native_dyn_sap_pair_00c32b10(manager);}
void* __fastcall NativeDynSapRuntime::next(void* manager,void*,void* pair){return next_native_dyn_sap_pair_00c32af0(manager,pair);}
void* __fastcall NativeDynSapRuntime::scalar(void* manager,void*,U flags){auto& r=owner(manager);return delete_native_dyn_sap_manager_004043d0(manager,flags,r.memory_,*r.calls_,*r.progress_);}
} // namespace bsp
