#include "bsp/native_renderer_debug_records24.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p)+offset);
}
void* at(void* p,U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
void store(void* p,U offset,U value) noexcept {
    *reinterpret_cast<volatile U*>(at(p,offset))=value;
}
void require(bool condition,const char* why) { if(!condition)throw std::invalid_argument(why); }
}

void __fastcall invalidate_raw_descendants_00b6da30(void* actual) {
    void* child=read<void*>(actual,0x34);
    while(child) {
        const U flags=read<U>(child,0x5c);
        if(flags&2u) {
            store(child,0x138,read<U>(child,0x138)&0xffffffcfu);
            const bool has_child=read<void*>(child,0x34)!=nullptr;
            store(child,0x5c,flags&0xfffffff5u);
            if(has_child)invalidate_raw_descendants_00b6da30(child);
        }
        child=read<void*>(child,0x3c);
    }
}
namespace {
void current_bounds_notification(void* actual,NativeTracelineRenderServices* profiles) {
    require(profiles!=nullptr,"current actual A0 virtual3C requires its canonical profile resolver");
    const volatile U* profile=profiles->profile(actual);
    require(profile&&profile[0x3c/4]==0x00b6dbc0,
        "current actual A0 virtual3C target is outside the observed native closure");
    notify_raw_bounds_00b6dbc0(actual,profiles);
}
}
void __fastcall notify_raw_bounds_00b6dbc0(void* actual,NativeTracelineRenderServices* profiles) {
    // The native JMP dispatch is a tail loop. Do not add host recursion here.
    for(;;) {
        store(actual,0x138,read<U>(actual,0x138)&0xffffffc3u);
        actual=read<void*>(actual,0xa0);
        if(!actual)return;
        require(profiles!=nullptr,"current enclosing A0 virtual3C requires its canonical profile resolver");
        const volatile U* profile=profiles->profile(actual);
        require(profile&&profile[0x3c/4]==0x00b6dbc0,
            "current enclosing A0 virtual3C target is outside the observed native closure");
    }
}
void __fastcall set_raw_local_matrix_00b6db10(void* actual,
    NativeTracelineRenderServices* profiles,const void* source) {
    copy_native_camera_matrix_004134f0(at(actual,0xb0),nullptr,source);
    if(read<std::uint8_t>(actual,0x5c)&0xau) {
        void* captured_attachment=read<void*>(actual,0xa0);
        store(actual,0x138,read<U>(actual,0x138)&0xffffffcfu);
        store(actual,0x5c,0);
        if(captured_attachment)current_bounds_notification(captured_attachment,profiles);
        if(read<void*>(actual,0x34))invalidate_raw_descendants_00b6da30(actual);
    }
}

// Exact interior instruction schedule; this is not a separate original function.
__declspec(naked) void __fastcall write_native_debug_sphere_vertices_00b2bf60(
    void*,const void*,std::uint32_t,const volatile double*) {
    __asm {
        push ebp
        sub esp,98h
        mov ebp,[esp+0a4h]
        mov eax,ecx
        mov ecx,[esp+0a0h]
        movss xmm0,dword ptr [edx+0]
        movss dword ptr [esp+0x50],xmm0
        movss xmm0,dword ptr [edx+4]
        movss dword ptr [esp+0x54],xmm0
        movss xmm0,dword ptr [edx+8]
        movss dword ptr [esp+0x58],xmm0
        movss xmm0,dword ptr [edx+12]
        movss dword ptr [esp+0x5c],xmm0
        movss xmm0,dword ptr [esp + 0x58] // 00b2bf60
        xor edx,edx // 00b2bf66
        mov dword ptr [esp + 0x14],edx // 00b2bf68
        lea esp,[esp] // 00b2bf6c
    l_00b2bf70:
        fild dword ptr [esp + 0x14] // 00b2bf70
        fmul qword ptr [ebp] // 00b2bf74
        fstp dword ptr [esp + 0x14] // 00b2bf7a
        fld dword ptr [esp + 0x14] // 00b2bf7e
        fstp dword ptr [esp + 0x1c] // 00b2bf82
        fld dword ptr [esp + 0x1c] // 00b2bf86
        fcos // 00b2bf8a
        fstp dword ptr [esp + 0x4c] // 00b2bf8c
        fld dword ptr [esp + 0x4c] // 00b2bf90
        fstp dword ptr [esp + 0x18] // 00b2bf94
        fld dword ptr [esp + 0x14] // 00b2bf98
        fstp dword ptr [esp + 0x1c] // 00b2bf9c
        fld dword ptr [esp + 0x1c] // 00b2bfa0
        fsin // 00b2bfa4
        fstp dword ptr [esp + 0x70] // 00b2bfa6
        fld dword ptr [esp + 0x70] // 00b2bfaa
        add edx,0x1 // 00b2bfae
        fld dword ptr [esp + 0x5c] // 00b2bfb1
        movss dword ptr [eax + 0x8],xmm0 // 00b2bfb5
        fld st(0) // 00b2bfba
        mov dword ptr [eax + 0xc],ecx // 00b2bfbc
        fmulp st(2) ,st(0) // 00b2bfbf
        add eax,0x10 // 00b2bfc1
        cmp edx,0xd // 00b2bfc4
        fld dword ptr [esp + 0x50] // 00b2bfc7
        mov dword ptr [esp + 0x14],edx // 00b2bfcb
        faddp st(2),st(0) // 00b2bfcf
        fxch // 00b2bfd1
        fstp dword ptr [esp + 0x74] // 00b2bfd3
        fmul dword ptr [esp + 0x18] // 00b2bfd7
        fadd dword ptr [esp + 0x54] // 00b2bfdb
        fstp dword ptr [esp + 0x78] // 00b2bfdf
        fld dword ptr [esp + 0x74] // 00b2bfe3
        fstp dword ptr [eax + -0x10] // 00b2bfe7
        fld dword ptr [esp + 0x78] // 00b2bfea
        fstp dword ptr [eax + -0xc] // 00b2bfee
        jl l_00b2bf70 // 00b2bff1
        movss xmm0,dword ptr [esp + 0x50] // 00b2bff7
        mov edx,0x1 // 00b2bffd
        mov dword ptr [esp + 0x14],edx // 00b2c002
        jmp l_00b2c010 // 00b2c006
    l_00b2c010:
        fild dword ptr [esp + 0x14] // 00b2c010
        fmul qword ptr [ebp] // 00b2c014
        fstp dword ptr [esp + 0x14] // 00b2c01a
        fld dword ptr [esp + 0x14] // 00b2c01e
        fstp dword ptr [esp + 0x18] // 00b2c022
        fld dword ptr [esp + 0x18] // 00b2c026
        fsin // 00b2c02a
        fstp dword ptr [esp + 0x48] // 00b2c02c
        fld dword ptr [esp + 0x48] // 00b2c030
        fstp dword ptr [esp + 0x1c] // 00b2c034
        fld dword ptr [esp + 0x14] // 00b2c038
        fstp dword ptr [esp + 0x18] // 00b2c03c
        fld dword ptr [esp + 0x18] // 00b2c040
        fcos // 00b2c044
        fstp dword ptr [esp + 0x68] // 00b2c046
        fld dword ptr [esp + 0x68] // 00b2c04a
        add edx,0x1 // 00b2c04e
        fld dword ptr [esp + 0x5c] // 00b2c051
        movss dword ptr [eax],xmm0 // 00b2c055
        fld st(0) // 00b2c059
        mov dword ptr [eax + 0xc],ecx // 00b2c05b
        fmulp st(2) ,st(0) // 00b2c05e
        add eax,0x10 // 00b2c060
        cmp edx,0xd // 00b2c063
        fld dword ptr [esp + 0x54] // 00b2c066
        mov dword ptr [esp + 0x14],edx // 00b2c06a
        faddp st(2),st(0) // 00b2c06e
        fxch // 00b2c070
        fstp dword ptr [esp + 0x84] // 00b2c072
        fmul dword ptr [esp + 0x1c] // 00b2c079
        fadd dword ptr [esp + 0x58] // 00b2c07d
        fstp dword ptr [esp + 0x88] // 00b2c081
        fld dword ptr [esp + 0x84] // 00b2c088
        fstp dword ptr [eax + -0xc] // 00b2c08f
        fld dword ptr [esp + 0x88] // 00b2c092
        fstp dword ptr [eax + -0x8] // 00b2c099
        jl l_00b2c010 // 00b2c09c
        movss xmm0,dword ptr [esp + 0x54] // 00b2c0a2
        xor edx,edx // 00b2c0a8
        mov dword ptr [esp + 0x14],edx // 00b2c0aa
        mov edi,edi // 00b2c0ae
    l_00b2c0b0:
        fild dword ptr [esp + 0x14] // 00b2c0b0
        fmul qword ptr [ebp] // 00b2c0b4
        fstp dword ptr [esp + 0x14] // 00b2c0ba
        fld dword ptr [esp + 0x14] // 00b2c0be
        fstp dword ptr [esp + 0x18] // 00b2c0c2
        fld dword ptr [esp + 0x18] // 00b2c0c6
        fsin // 00b2c0ca
        fstp dword ptr [esp + 0x3c] // 00b2c0cc
        fld dword ptr [esp + 0x3c] // 00b2c0d0
        fstp dword ptr [esp + 0x1c] // 00b2c0d4
        fld dword ptr [esp + 0x14] // 00b2c0d8
        fstp dword ptr [esp + 0x18] // 00b2c0dc
        fld dword ptr [esp + 0x18] // 00b2c0e0
        fcos // 00b2c0e4
        fstp dword ptr [esp + 0x6c] // 00b2c0e6
        fld dword ptr [esp + 0x6c] // 00b2c0ea
        add edx,0x1 // 00b2c0ee
        fld dword ptr [esp + 0x5c] // 00b2c0f1
        movss dword ptr [eax + 0x4],xmm0 // 00b2c0f5
        fld st(0) // 00b2c0fa
        mov dword ptr [eax + 0xc],ecx // 00b2c0fc
        fmulp st(2) ,st(0) // 00b2c0ff
        add eax,0x10 // 00b2c101
        cmp edx,0xd // 00b2c104
        fld dword ptr [esp + 0x50] // 00b2c107
        mov dword ptr [esp + 0x14],edx // 00b2c10b
        faddp st(2),st(0) // 00b2c10f
        fxch // 00b2c111
        fstp dword ptr [esp + 0x8c] // 00b2c113
        fmul dword ptr [esp + 0x1c] // 00b2c11a
        fadd dword ptr [esp + 0x58] // 00b2c11e
        fstp dword ptr [esp + 0x94] // 00b2c122
        fld dword ptr [esp + 0x8c] // 00b2c129
        fstp dword ptr [eax + -0x10] // 00b2c130
        fld dword ptr [esp + 0x94] // 00b2c133
        fstp dword ptr [eax + -0x8] // 00b2c13a
        jl l_00b2c0b0 // 00b2c13d
        add esp,98h
        pop ebp
        ret 8
    }
}
} // namespace bsp
