#include "bsp/native_render_service_texture_vectors.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture vector storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native vector pointer words require Win32");

// BF55BE is the tail to BF681B. Both sizes are the SAME already-wrapped native
// DWORD, with the existing real malloc/new-handler loop and matching free.
__declspec(noinline) void* __cdecl allocate_vector_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void raw_00b51fc0();
void raw_00b52040();
void raw_00b52170();
void raw_00b521e0();

__declspec(naked) void raw_00b51fc0() {
    __asm {
        push ecx // 00b51fc0
        push ebx // 00b51fc1
        push esi // 00b51fc2
        mov esi,dword ptr [esp + 010h] // 00b51fc3
        cmp esi,01h // 00b51fc7
        mov ebx,ecx // 00b51fca
        jge L_00b51fda // 00b51fcc
        mov dword ptr [esp + 010h],01h // 00b51fce
        mov esi,dword ptr [esp + 010h] // 00b51fd6
    L_00b51fda:
        cmp dword ptr [ebx + 08h],esi // 00b51fda
        jge L_00b52039 // 00b51fdd
        lea eax,[esi + esi*08h] // 00b51fdf
        push ebp // 00b51fe2
        add eax,eax // 00b51fe3
        add eax,eax // 00b51fe5
        push edi // 00b51fe7
        push eax // 00b51fe8
        call allocate_vector_bytes // 00b51fe9
        xor ebp,ebp // 00b51fee
        add esp,04h // 00b51ff0
        cmp dword ptr [ebx + 04h],ebp // 00b51ff3
        mov edi,eax // 00b51ff6
        mov dword ptr [esp + 010h],edi // 00b51ff8
        jle L_00b52027 // 00b51ffc
        xor edx,edx // 00b51ffe
    L_00b52000:
        test eax,eax // 00b52000
        jz L_00b52015 // 00b52002
        mov esi,dword ptr [ebx] // 00b52004
        add esi,edx // 00b52006
        mov ecx,09h // 00b52008
        mov edi,eax // 00b5200d
        rep movsd // 00b5200f
        mov esi,dword ptr [esp + 018h] // 00b52011
    L_00b52015:
        add ebp,01h // 00b52015
        add edx,024h // 00b52018
        add eax,024h // 00b5201b
        cmp ebp,dword ptr [ebx + 04h] // 00b5201e
        jl L_00b52000 // 00b52021
        mov edi,dword ptr [esp + 010h] // 00b52023
    L_00b52027:
        mov ecx,dword ptr [ebx] // 00b52027
        push ecx // 00b52029
        call singleton_lifetime_free // 00b5202a
        add esp,04h // 00b5202f
        mov dword ptr [ebx],edi // 00b52032
        pop edi // 00b52034
        mov dword ptr [ebx + 08h],esi // 00b52035
        pop ebp // 00b52038
    L_00b52039:
        pop esi // 00b52039
        pop ebx // 00b5203a
        pop ecx // 00b5203b
        ret 04h // 00b5203c
    }
}

__declspec(naked) void raw_00b52040() {
    __asm {
        push ebx // 00b52040
        mov ebx,dword ptr [esp + 08h] // 00b52041
        cmp ebx,01h // 00b52045
        push esi // 00b52048
        mov esi,ecx // 00b52049
        jge L_00b52052 // 00b5204b
        mov ebx,01h // 00b5204d
    L_00b52052:
        cmp dword ptr [esi + 08h],ebx // 00b52052
        jge L_00b520bd // 00b52055
        lea eax,[ebx + ebx*02h] // 00b52057
        push ebp // 00b5205a
        add eax,eax // 00b5205b
        add eax,eax // 00b5205d
        push edi // 00b5205f
        push eax // 00b52060
        call allocate_vector_bytes // 00b52061
        xor edi,edi // 00b52066
        add esp,04h // 00b52068
        cmp dword ptr [esi + 04h],edi // 00b5206b
        mov ebp,eax // 00b5206e
        mov dword ptr [esp + 014h],ebp // 00b52070
        jle L_00b520ab // 00b52074
        xor edx,edx // 00b52076
        mov ecx,ebp // 00b52078
        lea ebx,[ebx] // 00b5207a
    L_00b52080:
        test ecx,ecx // 00b52080
        jz L_00b52099 // 00b52082
        mov eax,dword ptr [esi] // 00b52084
        mov ebp,dword ptr [eax + edx*01h] // 00b52086
        add eax,edx // 00b52089
        mov dword ptr [ecx],ebp // 00b5208b
        mov ebp,dword ptr [eax + 04h] // 00b5208d
        mov dword ptr [ecx + 04h],ebp // 00b52090
        mov eax,dword ptr [eax + 08h] // 00b52093
        mov dword ptr [ecx + 08h],eax // 00b52096
    L_00b52099:
        add edi,01h // 00b52099
        add edx,0ch // 00b5209c
        add ecx,0ch // 00b5209f
        cmp edi,dword ptr [esi + 04h] // 00b520a2
        jl L_00b52080 // 00b520a5
        mov ebp,dword ptr [esp + 014h] // 00b520a7
    L_00b520ab:
        mov ecx,dword ptr [esi] // 00b520ab
        push ecx // 00b520ad
        call singleton_lifetime_free // 00b520ae
        add esp,04h // 00b520b3
        pop edi // 00b520b6
        mov dword ptr [esi],ebp // 00b520b7
        mov dword ptr [esi + 08h],ebx // 00b520b9
        pop ebp // 00b520bc
    L_00b520bd:
        pop esi // 00b520bd
        pop ebx // 00b520be
        ret 04h // 00b520bf
    }
}

__declspec(naked) void raw_00b52170() {
    __asm {
        push ebx // 00b52170
        mov ebx,dword ptr [esp + 08h] // 00b52171
        push esi // 00b52175
        mov esi,ecx // 00b52176
        cmp ebx,dword ptr [esi + 08h] // 00b52178
        jle L_00b52183 // 00b5217b
        push ebx // 00b5217d
        call raw_00b51fc0 // 00b5217e
    L_00b52183:
        mov eax,dword ptr [esi + 04h] // 00b52183
        cmp eax,ebx // 00b52186
        jge L_00b521c1 // 00b52188
        lea edx,[eax + eax*08h] // 00b5218a
        push edi // 00b5218d
        add edx,edx // 00b5218e
        mov edi,ebx // 00b52190
        add edx,edx // 00b52192
        sub edi,eax // 00b52194
    L_00b52196:
        mov eax,dword ptr [esi] // 00b52196
        add eax,edx // 00b52198
        jz L_00b521b8 // 00b5219a
        xor ecx,ecx // 00b5219c
        mov dword ptr [eax],ecx // 00b5219e
        mov dword ptr [eax + 04h],ecx // 00b521a0
        mov dword ptr [eax + 08h],ecx // 00b521a3
        mov dword ptr [eax + 0ch],ecx // 00b521a6
        mov dword ptr [eax + 010h],ecx // 00b521a9
        mov dword ptr [eax + 014h],ecx // 00b521ac
        mov dword ptr [eax + 018h],ecx // 00b521af
        mov dword ptr [eax + 01ch],ecx // 00b521b2
        mov dword ptr [eax + 020h],ecx // 00b521b5
    L_00b521b8:
        add edx,024h // 00b521b8
        sub edi,01h // 00b521bb
        jnz L_00b52196 // 00b521be
        pop edi // 00b521c0
    L_00b521c1:
        cmp ebx,dword ptr [esi + 04h] // 00b521c1
        jge L_00b521d8 // 00b521c4
        or eax,0ffffffffh // 00b521c6
        lea esp,[esp] // 00b521c9
    L_00b521d0:
        add dword ptr [esi + 04h],eax // 00b521d0
        cmp ebx,dword ptr [esi + 04h] // 00b521d3
        jl L_00b521d0 // 00b521d6
    L_00b521d8:
        mov dword ptr [esi + 04h],ebx // 00b521d8
        pop esi // 00b521db
        pop ebx // 00b521dc
        ret 04h // 00b521dd
    }
}

__declspec(naked) void raw_00b521e0() {
    __asm {
        push ebx // 00b521e0
        mov ebx,dword ptr [esp + 08h] // 00b521e1
        push esi // 00b521e5
        mov esi,ecx // 00b521e6
        cmp ebx,dword ptr [esi + 08h] // 00b521e8
        jle L_00b521f3 // 00b521eb
        push ebx // 00b521ed
        call raw_00b52040 // 00b521ee
    L_00b521f3:
        mov eax,dword ptr [esi + 04h] // 00b521f3
        cmp eax,ebx // 00b521f6
        jge L_00b5221f // 00b521f8
        lea edx,[eax + eax*02h] // 00b521fa
        push edi // 00b521fd
        add edx,edx // 00b521fe
        mov edi,ebx // 00b52200
        add edx,edx // 00b52202
        sub edi,eax // 00b52204
    L_00b52206:
        mov eax,dword ptr [esi] // 00b52206
        add eax,edx // 00b52208
        jz L_00b52216 // 00b5220a
        xor ecx,ecx // 00b5220c
        mov dword ptr [eax],ecx // 00b5220e
        mov dword ptr [eax + 04h],ecx // 00b52210
        mov dword ptr [eax + 08h],ecx // 00b52213
    L_00b52216:
        add edx,0ch // 00b52216
        sub edi,01h // 00b52219
        jnz L_00b52206 // 00b5221c
        pop edi // 00b5221e
    L_00b5221f:
        cmp ebx,dword ptr [esi + 04h] // 00b5221f
        jge L_00b5222f // 00b52222
        or eax,0ffffffffh // 00b52224
    L_00b52227:
        add dword ptr [esi + 04h],eax // 00b52227
        cmp ebx,dword ptr [esi + 04h] // 00b5222a
        jl L_00b52227 // 00b5222d
    L_00b5222f:
        mov dword ptr [esi + 04h],ebx // 00b5222f
        pop esi // 00b52232
        pop ebx // 00b52233
        ret 04h // 00b52234
    }
}

} // namespace

__declspec(naked) void __fastcall destroy_native_texture_vector24_00b523c0(void*) {
    __asm {
        push esi // 00b523c0
        push 00h // 00b523c1
        mov esi,ecx // 00b523c3
        call raw_00b52170 // 00b523c5
        mov eax,dword ptr [esi] // 00b523ca
        push eax // 00b523cc
        call singleton_lifetime_free // 00b523cd
        add esp,04h // 00b523d2
        pop esi // 00b523d5
        ret // 00b523d6
    }
}

__declspec(naked) void __fastcall destroy_native_texture_vector12_00b523e0(void*) {
    __asm {
        push esi // 00b523e0
        push 00h // 00b523e1
        mov esi,ecx // 00b523e3
        call raw_00b521e0 // 00b523e5
        mov eax,dword ptr [esi] // 00b523ea
        push eax // 00b523ec
        call singleton_lifetime_free // 00b523ed
        add esp,04h // 00b523f2
        pop esi // 00b523f5
        ret // 00b523f6
    }
}

// New C++ EDX argument becomes the native writable stack argument slot.
__declspec(naked) void __fastcall reserve_native_texture_vector24_00b51fc0(void*, std::int32_t) {
    __asm {
        pop eax
        push edx
        push eax
        jmp raw_00b51fc0
    }
}

// New C++ EDX argument becomes the native writable stack argument slot.
__declspec(naked) void __fastcall reserve_native_texture_vector12_00b52040(void*, std::int32_t) {
    __asm {
        pop eax
        push edx
        push eax
        jmp raw_00b52040
    }
}

// New C++ EDX argument becomes the native writable stack argument slot.
__declspec(naked) void __fastcall resize_native_texture_vector24_00b52170(void*, std::int32_t) {
    __asm {
        pop eax
        push edx
        push eax
        jmp raw_00b52170
    }
}

// New C++ EDX argument becomes the native writable stack argument slot.
__declspec(naked) void __fastcall resize_native_texture_vector12_00b521e0(void*, std::int32_t) {
    __asm {
        pop eax
        push edx
        push eax
        jmp raw_00b521e0
    }
}

} // namespace bsp
