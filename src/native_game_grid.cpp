#include "bsp/native_game_grid.hpp"
#include "bsp/camera_decomposition.hpp"
#include <array>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U bits(const void* p) noexcept {return static_cast<U>(reinterpret_cast<std::uintptr_t>(p));}
void* at(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(bits(p)+offset);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* pointer(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(word(p,offset));}
std::int32_t signed_word(U value) noexcept {std::int32_t result;std::memcpy(&result,&value,4);return result;}
bool below(U value,U limit) noexcept {return signed_word(value)<signed_word(limit);}

// Keep the load/store sequence rather than assigning C++ floats: even the
// copies quiet SNaNs, and a destination may overlap a later source component.
__declspec(naked) void __fastcall copy_float(void*,const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
__declspec(naked) void __fastcall copy_vector(void*,const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fstp dword ptr [ecx+8]
        ret
    }
}
// ECX=three binary32 differences, EDX=left, stack=right. No C++ arithmetic
// intervenes between each x87 subtraction and its native binary32 spill.
__declspec(naked) void __fastcall difference(void*,const void*,const void*) {
    __asm {
        mov eax,dword ptr [esp+4]
        fld dword ptr [edx]
        fsub dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fsub dword ptr [eax+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fsub dword ptr [eax+8]
        fstp dword ptr [ecx+8]
        ret 4
    }
}
// ECX=normal, EDX={row difference xyz, column difference xyz}. Preserve the
// exact 709D99..709DDA stack order and extended products, including NaN operand
// selection; independent C++ expressions can change both rounding and NaNs.
__declspec(naked) void __fastcall cross(void*,const void*) {
    __asm {
        fld dword ptr [edx+20]
        fld st(0)
        fld dword ptr [edx+4]
        fld st(0)
        fmulp st(2),st(0)
        fld dword ptr [edx+16]
        fld st(0)
        fld dword ptr [edx+8]
        fld st(0)
        fmulp st(2),st(0)
        fxch st(4)
        fsubrp st(1),st(0)
        fstp dword ptr [ecx]
        fld dword ptr [edx+12]
        fld st(0)
        fmulp st(4),st(0)
        fld dword ptr [edx]
        fld st(0)
        fmulp st(6),st(0)
        fxch st(4)
        fsubrp st(5),st(0)
        fxch st(4)
        fstp dword ptr [ecx+4]
        fmulp st(2),st(0)
        fmulp st(2),st(0)
        fsubrp st(1),st(0)
        fstp dword ptr [ecx+8]
        ret
    }
}
}

void NativeGameGridCpuCalls::free_00bf6989(void* allocation){std::free(allocation);}

void* copy_native_game_grid_descriptor_00709c90(void* destination,const void* source) {
    put(destination,0,word(source));put(destination,4,word(source,4));
    for(U offset=8;offset!=0x38;offset+=4)copy_float(at(destination,offset),at(source,offset));
    return destination;
}

void calculate_native_game_grid_normals_00709cf0(void* grid) {
    U row=1;
    if(below(row,word(grid,0x40)-1u)) {
        U column_limit=word(grid,0x3c)-1u;
        do {
            U column=1;
            if(below(column,column_limit)) {
                const U next_row=row+1u;
                do {
                    const U columns=word(grid,0x3c);
                    void* const positions=pointer(grid,8);
                    void* const after=at(positions,(next_row*columns+column)*12u);
                    void* const before=at(positions,((row-1u)*columns+column)*12u);
                    const U offset=(row*columns+column)*12u;
                    std::array<float,6> differences;
                    difference(differences.data(),before,after);
                    difference(differences.data()+3,at(positions,offset-12u),at(positions,offset+12u));
                    cross(at(pointer(grid,0x1c),offset),differences.data());
                    // Original reloads BOTH columns and output pointer after
                    // the cross stores, before calling the existing callee.
                    const U current_offset=(word(grid,0x3c)*row+column)*12u;
                    auto* const normal=static_cast<std::array<float,3>*>(at(pointer(grid,0x1c),current_offset));
                    normalize_camera_basis_0042b260(*normal);
                    column_limit=word(grid,0x3c)-1u;
                    ++column;
                } while(below(column,column_limit));
            }
            ++row;
        } while(below(row,word(grid,0x40)-1u));
    }
    U row_offset=12;
    for(row=0;below(row,word(grid,0x40));++row,row_offset+=12) {
        const U offset=word(grid,0x3c)*row*12u;
        void* const first=at(pointer(grid,0x1c),offset);
        copy_vector(first,at(first,12));
        const U end=word(grid,0x3c)*row_offset;
        void* const normals=pointer(grid,0x1c);
        copy_vector(at(normals,end-12u),at(normals,end-24u));
    }
    U byte_offset=0;
    for(U column=0;below(column,word(grid,0x3c));++column,byte_offset+=12) {
        const U next=(word(grid,0x3c)+column)*12u;
        void* const normals=pointer(grid,0x1c);
        copy_vector(at(normals,byte_offset),at(normals,next));
        const U rows=word(grid,0x40),columns=word(grid,0x3c);
        void* const before=at(pointer(grid,0x1c),((rows-2u)*columns+column)*12u);
        const U last=((rows-1u)*columns+column)*12u;
        copy_vector(at(pointer(grid,0x1c),last),before);
    }
}

void release_native_game_grid_cpu_arrays_00709760(void* grid,NativeGameGridCpuCalls& calls) {
    for(U offset=8;offset!=0x20;offset+=4) {
        void* const allocation=pointer(grid,offset);
        if(allocation)calls.free_00bf6989(allocation);
    }
    for(U offset=8;offset!=0x20;offset+=4)put(grid,offset,0);
}
} // namespace bsp
