#include "bsp/native_avoid_zone_construct.hpp"
#include "bsp/native_scene_path_point.hpp"
#include "bsp/native_entity_path_access.hpp"
#include "bsp/native_avoid_bounds_include.hpp"
#include "bsp/native_avoid_clip_cleanup.hpp"
#include "bsp/native_avoid_zone_clip.hpp"
#include "bsp/native_avoid_zone_winding.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This complete raw constructor requires MSVC Win32.
#endif

namespace bsp {
namespace {
// Exact immutable Native payloads: CE3880 and D7A260.
alignas(8) constexpr std::uint64_t kConstructorSquaredDistance = 0x4039000000000000ULL;
constexpr std::uint32_t kConstructorNegativeOne = 0xbf800000U;

struct ConstructorWorkingSet {
    alignas(4) unsigned char algorithm[0x70];
    volatile std::int32_t phase;
    std::uint32_t* actual_arguments;
    void* const volatile* world_pointer_cell;
    std::uint32_t virtual_ebp;
};
static_assert(offsetof(ConstructorWorkingSet, phase) == 0x70);
static_assert(offsetof(ConstructorWorkingSet, actual_arguments) == 0x74);
static_assert(offsetof(ConstructorWorkingSet, world_pointer_cell) == 0x78);
static_assert(offsetof(ConstructorWorkingSet, virtual_ebp) == 0x7c);
static_assert(sizeof(ConstructorWorkingSet) == 0x80);

__declspec(noinline) void* __cdecl allocate_construct_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

// Original locals are rebased from entryESP-7Ch; Native EBP's algorithm value
// is held in virtual_ebp with flag-preserving GP sequences. No x87 form or
// spill is changed. Actual public argument cells are borrowed, not copied.
__declspec(naked) void* __fastcall run_constructor_kernel(void*, ConstructorWorkingSet*) {
    __asm {
        push ebp
        mov ebp, edx
    // 0041CCD0 PUSH -0x1 [native_eh_or_frame_removed]
    // 0041CCD2 PUSH 0xc5e198 [native_eh_or_frame_removed]
    // 0041CCD7 MOV EAX,FS:[0x0] [native_eh_or_frame_removed]
    // 0041CCDD PUSH EAX [native_eh_or_frame_removed]
    // 0041CCDE MOV dword ptr FS:[0x0],ESP [native_eh_or_frame_removed]
    // 0041CCE5 SUB ESP,0x60 [native_eh_or_frame_removed]
    // 0041CCE8 PUSH EBX [literal]
        _emit 0x53
    // 0041CCE9 PUSH ESI [literal]
        _emit 0x56
    // 0041CCEA MOV ESI,ECX [literal]
        _emit 0x8b
        _emit 0xf1
    // 0041CCEC PUSH EDI [literal]
        _emit 0x57
    // 0041CCED XOR EDI,EDI [literal]
        _emit 0x33
        _emit 0xff
    // 0041CCEF MOV dword ptr [ESI],EDI [literal]
        _emit 0x89
        _emit 0x3e
    // 0041CCF1 MOV dword ptr [ESP + 0x10],ESI [algorithm_cell_rebase]
        _emit 0x89
        _emit 0xb5
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CCF5 MOV dword ptr [ESI + 0x4],EDI [literal]
        _emit 0x89
        _emit 0x7e
        _emit 0x04
    // 0041CCF8 MOV dword ptr [ESI + 0x8],EDI [literal]
        _emit 0x89
        _emit 0x7e
        _emit 0x08
    // 0041CCFB MOV EAX,dword ptr [ESP + 0x80] [actual_public_argument_cell]
        mov eax, dword ptr [ebp + 074h]
        mov eax, dword ptr [eax + 4]
    // 0041CD02 MOV EBX,dword ptr [ESP + 0x7c] [actual_public_argument_cell]
        mov ebx, dword ptr [ebp + 074h]
        mov ebx, dword ptr [ebx]
    // 0041CD06 MOV dword ptr [ESI + 0xc],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x0c
    // 0041CD09 MOV dword ptr [ESI + 0x10],EDI [literal]
        _emit 0x89
        _emit 0x7e
        _emit 0x10
    // 0041CD0C MOV EAX,dword ptr [EBX + 0x8] [literal]
        _emit 0x8b
        _emit 0x43
        _emit 0x08
    // 0041CD0F CMP EAX,EDI [literal]
        _emit 0x3b
        _emit 0xc7
    // 0041CD11 MOV dword ptr [ESP + 0x74],EDI [current_source_phase]
        _emit 0x89
        _emit 0xbd
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD15 JZ 0x0041cfee [branch]
        je ctor_0041cfee
    // 0041CD1B MOV ECX,dword ptr [EBX + 0xc] [literal]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
    // 0041CD1E SUB ECX,EAX [literal]
        _emit 0x2b
        _emit 0xc8
    // 0041CD20 SAR ECX,0x2 [literal]
        _emit 0xc1
        _emit 0xf9
        _emit 0x02
    // 0041CD23 JZ 0x0041cfee [branch]
        je ctor_0041cfee
    // 0041CD29 PUSH EBP [native_eh_or_frame_removed]
    // 0041CD2A PUSH EDI [literal]
        _emit 0x57
    // 0041CD2B LEA EDX,[ESP + 0x34] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x95
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD2F PUSH EDX [literal]
        _emit 0x52
    // 0041CD30 MOV ECX,EBX [literal]
        _emit 0x8b
        _emit 0xcb
    // 0041CD32 CALL 0x007af800 [genuine_provider]
        call transform_native_scene_path_point_007af800
    // 0041CD37 MOVSS XMM0,dword ptr [ESP + 0x30] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD3D MOVSS XMM1,dword ptr [ESP + 0x38] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x8d
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD43 MOVSS dword ptr [ESI + 0x14],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x46
        _emit 0x14
    // 0041CD48 MOVSS dword ptr [ESI + 0x18],XMM1 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4e
        _emit 0x18
    // 0041CD4D PUSH 0x24 [literal]
        _emit 0x6a
        _emit 0x24
    // 0041CD4F MOVSS dword ptr [ESP + 0x1c],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD55 MOVSS dword ptr [ESP + 0x20],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD5B MOVSS dword ptr [ESP + 0x64],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x60
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD61 MOVSS dword ptr [ESP + 0x68],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x64
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD67 MOVSS dword ptr [ESP + 0x6c],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD6D MOVSS dword ptr [ESP + 0x70],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x6c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD73 MOVSS dword ptr [ESI + 0x1c],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x46
        _emit 0x1c
    // 0041CD78 MOVSS dword ptr [ESI + 0x20],XMM1 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4e
        _emit 0x20
    // 0041CD7D CALL 0x00bf681b [genuine_provider]
        call allocate_construct_bytes
    // 0041CD82 XORPS XMM0,XMM0 [literal]
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
    // 0041CD85 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CD88 CMP EAX,EDI [literal]
        _emit 0x3b
        _emit 0xc7
    // 0041CD8A JZ 0x0041cdd1 [branch]
        je ctor_0041cdd1
    // 0041CD8C MOVSS XMM1,dword ptr [ESP + 0x18] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x8d
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD92 MOVSS dword ptr [EAX],XMM1 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x08
    // 0041CD96 MOVSS XMM1,dword ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x8d
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CD9C MOVSS dword ptr [EAX + 0x4],XMM1 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x48
        _emit 0x04
    // 0041CDA1 MOVSS dword ptr [EAX + 0x14],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x14
    // 0041CDA6 MOVSS dword ptr [EAX + 0x10],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x10
    // 0041CDAB MOVSS dword ptr [EAX + 0x8],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
    // 0041CDB0 FLD float ptr [EAX + 0x14] [literal]
        _emit 0xd9
        _emit 0x40
        _emit 0x14
    // 0041CDB3 FSTP float ptr [EAX + 0xc] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x0c
    // 0041CDB6 MOVSS dword ptr [EAX + 0x18],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x18
    // 0041CDBB MOVSS dword ptr [EAX + 0x1c],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x1c
    // 0041CDC0 MOVSS XMM0,dword ptr [0x00d7a260] [source_literal_binding]
        movss xmm0, dword ptr kConstructorNegativeOne
    // 0041CDC8 MOVSS dword ptr [EAX + 0x20],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x20
    // 0041CDCD MOV EBP,EAX [virtual_native_ebp]
        mov dword ptr [ebp + 07Ch], eax
    // 0041CDCF JMP 0x0041cdd3 [branch]
        jmp ctor_0041cdd3
    // 0041CDD1 XOR EBP,EBP [virtual_native_ebp]
    ctor_0041cdd1:
        push eax
        xor eax, eax
        mov dword ptr [ebp + 07Ch], eax
        pop eax
    // 0041CDD3 MOV EAX,dword ptr [ESI + 0x8] [literal]
    ctor_0041cdd3:
        _emit 0x8b
        _emit 0x46
        _emit 0x08
    // 0041CDD6 MOV ECX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CDD9 CMP ECX,EAX [literal]
        _emit 0x3b
        _emit 0xc8
    // 0041CDDB JNZ 0x0041ce33 [branch]
        jne ctor_0041ce33
    // 0041CDDD LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041CDE1 CMP EAX,dword ptr [ESI + 0x8] [literal]
        _emit 0x3b
        _emit 0x46
        _emit 0x08
    // 0041CDE4 JBE 0x0041ce33 [branch]
        jbe ctor_0041ce33
    // 0041CDE6 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041CDE8 MOV dword ptr [ESI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x08
    // 0041CDEB MOV EDX,0x4 [literal]
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CDF0 MUL EDX [literal]
        _emit 0xf7
        _emit 0xe2
    // 0041CDF2 SETO CL [literal]
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
    // 0041CDF5 NEG ECX [literal]
        _emit 0xf7
        _emit 0xd9
    // 0041CDF7 OR ECX,EAX [literal]
        _emit 0x0b
        _emit 0xc8
    // 0041CDF9 PUSH ECX [literal]
        _emit 0x51
    // 0041CDFA CALL 0x00bf55be [genuine_provider]
        call allocate_construct_bytes
    // 0041CDFF ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CE02 CMP dword ptr [ESI],0x0 [literal]
        _emit 0x83
        _emit 0x3e
        _emit 0x00
    // 0041CE05 MOV EDI,EAX [literal]
        _emit 0x8b
        _emit 0xf8
    // 0041CE07 JZ 0x0041ce2f [branch]
        je ctor_0041ce2f
    // 0041CE09 MOV ECX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CE0C XOR EAX,EAX [literal]
        _emit 0x33
        _emit 0xc0
    // 0041CE0E TEST ECX,ECX [literal]
        _emit 0x85
        _emit 0xc9
    // 0041CE10 JBE 0x0041ce24 [branch]
        jbe ctor_0041ce24
    // 0041CE12 MOV EDX,dword ptr [ESI] [literal]
    ctor_0041ce12:
        _emit 0x8b
        _emit 0x16
    // 0041CE14 MOV ECX,dword ptr [EDX + EAX*0x4] [literal]
        _emit 0x8b
        _emit 0x0c
        _emit 0x82
    // 0041CE17 MOV dword ptr [EDI + EAX*0x4],ECX [literal]
        _emit 0x89
        _emit 0x0c
        _emit 0x87
    // 0041CE1A MOV EDX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
    // 0041CE1D ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041CE20 CMP EAX,EDX [literal]
        _emit 0x3b
        _emit 0xc2
    // 0041CE22 JC 0x0041ce12 [branch]
        jb ctor_0041ce12
    // 0041CE24 MOV EAX,dword ptr [ESI] [literal]
    ctor_0041ce24:
        _emit 0x8b
        _emit 0x06
    // 0041CE26 PUSH EAX [literal]
        _emit 0x50
    // 0041CE27 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041CE2C ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CE2F MOV dword ptr [ESI],EDI [literal]
    ctor_0041ce2f:
        _emit 0x89
        _emit 0x3e
    // 0041CE31 XOR EDI,EDI [literal]
        _emit 0x33
        _emit 0xff
    // 0041CE33 MOV ECX,dword ptr [ESI + 0x4] [literal]
    ctor_0041ce33:
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CE36 MOV EDX,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x16
    // 0041CE38 MOV dword ptr [EDX + ECX*0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 07Ch]
        mov dword ptr [edx + ecx*4], eax
        pop eax
    // 0041CE3B MOV ECX,0x1 [literal]
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CE40 ADD dword ptr [ESI + 0x4],ECX [literal]
        _emit 0x01
        _emit 0x4e
        _emit 0x04
    // 0041CE43 MOV EAX,dword ptr [EBX + 0x8] [literal]
        _emit 0x8b
        _emit 0x43
        _emit 0x08
    // 0041CE46 CMP EAX,EDI [literal]
        _emit 0x3b
        _emit 0xc7
    // 0041CE48 JNZ 0x0041ce53 [branch]
        jne ctor_0041ce53
    // 0041CE4A MOV dword ptr [ESP + 0x84],EDI [actual_public_argument_cell]
        push eax
        mov eax, dword ptr [ebp + 074h]
        mov dword ptr [eax + 4], edi
        pop eax
    // 0041CE51 JMP 0x0041ce62 [branch]
        jmp ctor_0041ce62
    // 0041CE53 MOV EBX,dword ptr [EBX + 0xc] [literal]
    ctor_0041ce53:
        _emit 0x8b
        _emit 0x5b
        _emit 0x0c
    // 0041CE56 SUB EBX,EAX [literal]
        _emit 0x2b
        _emit 0xd8
    // 0041CE58 SAR EBX,0x2 [literal]
        _emit 0xc1
        _emit 0xfb
        _emit 0x02
    // 0041CE5B MOV dword ptr [ESP + 0x84],EBX [actual_public_argument_cell]
        push eax
        mov eax, dword ptr [ebp + 074h]
        mov dword ptr [eax + 4], ebx
        pop eax
    // 0041CE62 CMP dword ptr [ESP + 0x84],ECX [actual_public_argument_cell]
    ctor_0041ce62:
        push eax
        mov eax, dword ptr [ebp + 074h]
        cmp dword ptr [eax + 4], ecx
        pop eax
    // 0041CE69 MOV EBX,ECX [literal]
        _emit 0x8b
        _emit 0xd9
    // 0041CE6B JLE 0x0041cfe6 [branch]
        jle ctor_0041cfe6
    // 0041CE71 MOV ECX,dword ptr [ESP + 0x80] [actual_public_argument_cell]
    ctor_0041ce71:
        mov ecx, dword ptr [ebp + 074h]
        mov ecx, dword ptr [ecx]
    // 0041CE78 PUSH EBX [literal]
        _emit 0x53
    // 0041CE79 LEA EAX,[ESP + 0x58] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x85
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CE7D PUSH EAX [literal]
        _emit 0x50
    // 0041CE7E CALL 0x007af800 [genuine_provider]
        call transform_native_scene_path_point_007af800
    // 0041CE83 MOVSS XMM0,dword ptr [EAX] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
    // 0041CE87 MOVSS dword ptr [ESP + 0x30],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CE8D FLD float ptr [ESP + 0x30] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CE91 FSUB float ptr [ESP + 0x18] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0xa5
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CE95 MOVSS XMM1,dword ptr [EAX + 0x4] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x04
    // 0041CE9A MOVSS dword ptr [ESP + 0x34],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEA0 MOVSS XMM1,dword ptr [EAX + 0x8] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x08
    // 0041CEA5 FSTP float ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEA9 MOVSS dword ptr [ESP + 0x38],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEAF FLD float ptr [ESP + 0x38] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEB3 MOVSS dword ptr [ESP + 0x20],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEB9 FSUB float ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0xa5
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEBD MOVSS dword ptr [ESP + 0x24],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEC3 FSTP float ptr [ESP + 0x2c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEC7 FLD float ptr [ESP + 0x2c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CECB FLD float ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CECF FMUL ST0 [literal]
        _emit 0xdc
        _emit 0xc8
    // 0041CED1 FLD ST1 [literal]
        _emit 0xd9
        _emit 0xc1
    // 0041CED3 FMULP ST2 [literal]
        _emit 0xde
        _emit 0xca
    // 0041CED5 FADDP [literal]
        _emit 0xde
        _emit 0xc1
    // 0041CED7 FSTP float ptr [ESP + 0x10] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEDB FLD double ptr [0x00ce3880] [source_literal_binding]
        fld qword ptr kConstructorSquaredDistance
    // 0041CEE1 FLD float ptr [ESP + 0x10] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CEE5 FCOMIP ST0,ST1 [literal]
        _emit 0xdf
        _emit 0xf1
    // 0041CEE7 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041CEE9 JBE 0x0041cfd6 [branch]
        jbe ctor_0041cfd6
    // 0041CEEF PUSH 0x24 [literal]
        _emit 0x6a
        _emit 0x24
    // 0041CEF1 CALL 0x00bf681b [genuine_provider]
        call allocate_construct_bytes
    // 0041CEF6 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CEF9 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041CEFB JZ 0x0041cf45 [branch]
        je ctor_0041cf45
    // 0041CEFD MOVSS XMM0,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CF03 MOVSS dword ptr [EAX],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
    // 0041CF07 MOVSS XMM0,dword ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CF0D MOVSS dword ptr [EAX + 0x4],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
    // 0041CF12 XORPS XMM0,XMM0 [literal]
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
    // 0041CF15 MOVSS dword ptr [EAX + 0x14],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x14
    // 0041CF1A MOVSS dword ptr [EAX + 0x10],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x10
    // 0041CF1F MOVSS dword ptr [EAX + 0x8],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
    // 0041CF24 FLD float ptr [EAX + 0x14] [literal]
        _emit 0xd9
        _emit 0x40
        _emit 0x14
    // 0041CF27 FSTP float ptr [EAX + 0xc] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x0c
    // 0041CF2A MOVSS dword ptr [EAX + 0x18],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x18
    // 0041CF2F MOVSS dword ptr [EAX + 0x1c],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x1c
    // 0041CF34 MOVSS XMM0,dword ptr [0x00d7a260] [source_literal_binding]
        movss xmm0, dword ptr kConstructorNegativeOne
    // 0041CF3C MOVSS dword ptr [EAX + 0x20],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x20
    // 0041CF41 MOV EBP,EAX [virtual_native_ebp]
        mov dword ptr [ebp + 07Ch], eax
    // 0041CF43 JMP 0x0041cf47 [branch]
        jmp ctor_0041cf47
    // 0041CF45 XOR EBP,EBP [virtual_native_ebp]
    ctor_0041cf45:
        push eax
        xor eax, eax
        mov dword ptr [ebp + 07Ch], eax
        pop eax
    // 0041CF47 MOV EAX,dword ptr [ESI + 0x8] [literal]
    ctor_0041cf47:
        _emit 0x8b
        _emit 0x46
        _emit 0x08
    // 0041CF4A MOV ECX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CF4D CMP ECX,EAX [literal]
        _emit 0x3b
        _emit 0xc8
    // 0041CF4F JNZ 0x0041cfa5 [branch]
        jne ctor_0041cfa5
    // 0041CF51 LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041CF55 CMP EAX,dword ptr [ESI + 0x8] [literal]
        _emit 0x3b
        _emit 0x46
        _emit 0x08
    // 0041CF58 JBE 0x0041cfa5 [branch]
        jbe ctor_0041cfa5
    // 0041CF5A XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041CF5C MOV dword ptr [ESI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x08
    // 0041CF5F MOV EDX,0x4 [literal]
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CF64 MUL EDX [literal]
        _emit 0xf7
        _emit 0xe2
    // 0041CF66 SETO CL [literal]
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
    // 0041CF69 NEG ECX [literal]
        _emit 0xf7
        _emit 0xd9
    // 0041CF6B OR ECX,EAX [literal]
        _emit 0x0b
        _emit 0xc8
    // 0041CF6D PUSH ECX [literal]
        _emit 0x51
    // 0041CF6E CALL 0x00bf55be [genuine_provider]
        call allocate_construct_bytes
    // 0041CF73 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CF76 CMP dword ptr [ESI],0x0 [literal]
        _emit 0x83
        _emit 0x3e
        _emit 0x00
    // 0041CF79 MOV EDI,EAX [literal]
        _emit 0x8b
        _emit 0xf8
    // 0041CF7B JZ 0x0041cfa3 [branch]
        je ctor_0041cfa3
    // 0041CF7D MOV ECX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CF80 XOR EAX,EAX [literal]
        _emit 0x33
        _emit 0xc0
    // 0041CF82 TEST ECX,ECX [literal]
        _emit 0x85
        _emit 0xc9
    // 0041CF84 JBE 0x0041cf98 [branch]
        jbe ctor_0041cf98
    // 0041CF86 MOV EDX,dword ptr [ESI] [literal]
    ctor_0041cf86:
        _emit 0x8b
        _emit 0x16
    // 0041CF88 MOV ECX,dword ptr [EDX + EAX*0x4] [literal]
        _emit 0x8b
        _emit 0x0c
        _emit 0x82
    // 0041CF8B MOV dword ptr [EDI + EAX*0x4],ECX [literal]
        _emit 0x89
        _emit 0x0c
        _emit 0x87
    // 0041CF8E MOV EDX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
    // 0041CF91 ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041CF94 CMP EAX,EDX [literal]
        _emit 0x3b
        _emit 0xc2
    // 0041CF96 JC 0x0041cf86 [branch]
        jb ctor_0041cf86
    // 0041CF98 MOV EAX,dword ptr [ESI] [literal]
    ctor_0041cf98:
        _emit 0x8b
        _emit 0x06
    // 0041CF9A PUSH EAX [literal]
        _emit 0x50
    // 0041CF9B CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041CFA0 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041CFA3 MOV dword ptr [ESI],EDI [literal]
    ctor_0041cfa3:
        _emit 0x89
        _emit 0x3e
    // 0041CFA5 MOV ECX,dword ptr [ESI + 0x4] [literal]
    ctor_0041cfa5:
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
    // 0041CFA8 MOV EDX,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x16
    // 0041CFAA MOV dword ptr [EDX + ECX*0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 07Ch]
        mov dword ptr [edx + ecx*4], eax
        pop eax
    // 0041CFAD ADD dword ptr [ESI + 0x4],0x1 [literal]
        _emit 0x83
        _emit 0x46
        _emit 0x04
        _emit 0x01
    // 0041CFB1 LEA EAX,[ESP + 0x20] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x85
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CFB5 PUSH EAX [literal]
        _emit 0x50
    // 0041CFB6 LEA ECX,[ESI + 0x14] [literal]
        _emit 0x8d
        _emit 0x4e
        _emit 0x14
    // 0041CFB9 CALL 0x00415010 [genuine_provider]
        call include_native_avoid_bounds_point_00415010
    // 0041CFBE MOVSS XMM0,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CFC4 MOVSS dword ptr [ESP + 0x18],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CFCA MOVSS XMM0,dword ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CFD0 MOVSS dword ptr [ESP + 0x1c],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041CFD6 ADD EBX,0x1 [literal]
    ctor_0041cfd6:
        _emit 0x83
        _emit 0xc3
        _emit 0x01
    // 0041CFD9 CMP EBX,dword ptr [ESP + 0x84] [actual_public_argument_cell]
        push eax
        mov eax, dword ptr [ebp + 074h]
        cmp ebx, dword ptr [eax + 4]
        pop eax
    // 0041CFE0 JL 0x0041ce71 [branch]
        jl ctor_0041ce71
    // 0041CFE6 MOV EBX,dword ptr [ESP + 0x80] [actual_public_argument_cell]
    ctor_0041cfe6:
        mov ebx, dword ptr [ebp + 074h]
        mov ebx, dword ptr [ebx]
    // 0041CFED POP EBP [native_eh_or_frame_removed]
    // 0041CFEE MOV EAX,[0x00e188a8] [actual_late_world_cell]
    ctor_0041cfee:
        mov eax, dword ptr [ebp + 078h]
        mov eax, dword ptr [eax]
    // 0041CFF3 MOVSS XMM0,dword ptr [EAX + 0x711c] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x1c
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041CFFB MOVSS dword ptr [ESP + 0x44],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D001 MOVSS XMM0,dword ptr [EAX + 0x7120] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x20
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041D009 MOVSS dword ptr [ESP + 0x48],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D00F MOVSS XMM0,dword ptr [EAX + 0x7128] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x28
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041D017 MOVSS dword ptr [ESP + 0x38],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D01D MOVSS XMM0,dword ptr [EAX + 0x712c] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x2c
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041D025 MOVSS dword ptr [ESP + 0x3c],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D02B MOVSS XMM0,dword ptr [EAX + 0x7124] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x24
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041D033 LEA ECX,[ESP + 0x38] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x8d
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D037 PUSH ECX [literal]
        _emit 0x51
    // 0041D038 LEA EDX,[ESP + 0x48] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x95
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D03C MOVSS dword ptr [ESP + 0x44],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D042 MOVSS XMM0,dword ptr [EAX + 0x7130] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x80
        _emit 0x30
        _emit 0x71
        _emit 0x00
        _emit 0x00
    // 0041D04A PUSH EDX [literal]
        _emit 0x52
    // 0041D04B MOV ECX,ESI [literal]
        _emit 0x8b
        _emit 0xce
    // 0041D04D MOVSS dword ptr [ESP + 0x54],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D053 CALL 0x0041a540 [genuine_provider]
        call clip_native_avoid_zone_world_bounds_0041a540
    // 0041D058 MOV EAX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
    // 0041D05B TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041D05D JBE 0x0041d09c [branch]
        jbe ctor_0041d09c
    // 0041D05F PUSH 0x1 [literal]
        _emit 0x6a
        _emit 0x01
    // 0041D061 MOV ECX,ESI [literal]
        _emit 0x8b
        _emit 0xce
    // 0041D063 CALL 0x0041a200 [genuine_provider]
        call derive_native_avoid_zone_0041a200
    // 0041D068 MOV ECX,dword ptr [EBX + 0x14] [literal]
        _emit 0x8b
        _emit 0x4b
        _emit 0x14
    // 0041D06B PUSH 0x1 [literal]
        _emit 0x6a
        _emit 0x01
    // 0041D06D CALL 0x00923810 [genuine_provider]
        call get_native_entity_ancestor_00923810
    // 0041D072 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041D074 JZ 0x0041d09c [branch]
        je ctor_0041d09c
    // 0041D076 MOV ECX,dword ptr [EBX + 0x14] [literal]
        _emit 0x8b
        _emit 0x4b
        _emit 0x14
    // 0041D079 PUSH 0x1 [literal]
        _emit 0x6a
        _emit 0x01
    // 0041D07B CALL 0x00923810 [genuine_provider]
        call get_native_entity_ancestor_00923810
    // 0041D080 MOV EDX,dword ptr [EAX] [literal]
        _emit 0x8b
        _emit 0x10
    // 0041D082 MOV ECX,EAX [literal]
        _emit 0x8b
        _emit 0xc8
    // 0041D084 MOV EAX,dword ptr [EDX + 0x5c] [literal]
        _emit 0x8b
        _emit 0x42
        _emit 0x5c
    // 0041D087 PUSH 0x44 [literal]
        _emit 0x6a
        _emit 0x44
    // 0041D089 CALL EAX [literal_virtual_call]
        _emit 0xff
        _emit 0xd0
    // 0041D08B TEST AL,AL [literal]
        _emit 0x84
        _emit 0xc0
    // 0041D08D JZ 0x0041d09c [branch]
        je ctor_0041d09c
    // 0041D08F MOV ECX,dword ptr [EBX + 0x14] [literal]
        _emit 0x8b
        _emit 0x4b
        _emit 0x14
    // 0041D092 PUSH 0x1 [literal]
        _emit 0x6a
        _emit 0x01
    // 0041D094 CALL 0x00923810 [genuine_provider]
        call get_native_entity_ancestor_00923810
    // 0041D099 MOV dword ptr [ESI + 0x10],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x10
    // 0041D09C MOV ECX,dword ptr [ESP + 0x6c] [native_eh_or_frame_removed]
    ctor_0041d09c:
    // 0041D0A0 POP EDI [literal]
        _emit 0x5f
    // 0041D0A1 MOV EAX,ESI [literal]
        _emit 0x8b
        _emit 0xc6
    // 0041D0A3 POP ESI [literal]
        _emit 0x5e
    // 0041D0A4 POP EBX [literal]
        _emit 0x5b
    // 0041D0A5 MOV dword ptr FS:[0x0],ECX [native_eh_or_frame_removed]
    // 0041D0AC ADD ESP,0x6c [native_eh_or_frame_removed]
    // 0041D0AF RET 0x8 [private_kernel_return]
        pop ebp
        ret
    }
}

class CurrentConstructorArrayGuard {
public:
    explicit CurrentConstructorArrayGuard(ConstructorWorkingSet& working) noexcept : working_(working) {}
    ~CurrentConstructorArrayGuard() noexcept {
        if (working_.phase >= 0) {
            // Native C5E190 reloads CURRENT saved receiver; do not use the
            // entry argument or a separately cached owner for this cleanup.
            void* current_receiver;
            std::memcpy(&current_receiver, working_.algorithm + 0x14, sizeof(current_receiver));
            free_and_clear_native_avoid_pointer_00412dc0(current_receiver);
        }
    }
    CurrentConstructorArrayGuard(const CurrentConstructorArrayGuard&) = delete;
    CurrentConstructorArrayGuard& operator=(const CurrentConstructorArrayGuard&) = delete;
private:
    ConstructorWorkingSet& working_;
};

__declspec(noinline) void* __cdecl construct_current_source_frame(
    void* actual_zone, void* const volatile* actual_world_pointer_cell,
    std::uint32_t* actual_argument_cells) {
    ConstructorWorkingSet working;
    working.phase = -1;
    working.actual_arguments = actual_argument_cells;
    working.world_pointer_cell = actual_world_pointer_cell;
    CurrentConstructorArrayGuard guard(working);
    // A fixed genuine entry, not an injectable callback. Non-noexcept volatile
    // binding retains current C++ cleanup around the naked complete kernel.
    using Kernel = void* (__fastcall*)(void*, ConstructorWorkingSet*);
    Kernel volatile kernel = run_constructor_kernel;
    void* result = kernel(actual_zone, &working);
    working.phase = -1; // no normal cleanup; Native only removes its EH frame
    return result;
}
} // namespace

__declspec(naked) void* __fastcall construct_native_avoid_zone_0041ccd0(
    void*, void* const volatile*, void*, std::uint32_t) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call construct_current_source_frame
        add esp, 0ch
        ret 8
    }
}
} // namespace bsp
