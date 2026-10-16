#include "bsp/native_avoid_zone_clip.hpp"
#include "bsp/avoid_zone_owner.hpp"
#include "bsp/native_avoid_clip_cleanup.hpp"
#include "bsp/ship_ai_lateral_record.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(AvoidZoneNativeStorage) == 0x24);
static_assert(sizeof(ShipAiPathLateralRecord) == 0x24);
static_assert(offsetof(AvoidZoneNativeStorage, capacity) == 8);
static_assert(offsetof(AvoidZoneNativeStorage, min_x) == 0x14);

const double kClipHundred = 100.0;
const float kClipOne = 1.0f;
const float kClipNegativeOne = -1.0f;
const float kClipZero = 0.0f;

// These are the original algorithm's local cells rebased into storage owned by
// the enclosing current C++ frame. No Native FS record or FH3 state is installed.
// Only the Source phase and two actual current-pointer cells drive the guards.
struct ClipWorkingSet {
    alignas(4) unsigned char algorithm[0xc4];
    volatile std::int32_t phase;
    const std::uint32_t* actual_arguments;
    std::uint32_t virtual_ebp;
};
static_assert(offsetof(ClipWorkingSet, phase) == 0xc4);
static_assert(offsetof(ClipWorkingSet, actual_arguments) == 0xc8);
static_assert(offsetof(ClipWorkingSet, virtual_ebp) == 0xcc);
static_assert(sizeof(ClipWorkingSet) == 0xd0);

__declspec(noinline) void* __cdecl allocate_clip_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
}

// EBP is the persistent working-set base. Original EBP arithmetic is lowered
// to virtual_ebp using flag-preserving spill sequences. All other nonframe
// instructions, including ambiguous x87 register forms, use their exact bytes.
// ESP-relative algorithm cells are mechanically rebased without changing their
// opcode, x87 form, operand size or immediate. See the address comments below.
__declspec(naked) void __fastcall run_clip_kernel(void*, ClipWorkingSet*) {
    __asm {
        push ebp
        mov ebp, edx
    // 0041A540 PUSH -0x1 [native_eh_or_frame_removed]
    // 0041A542 PUSH 0xc5e163 [native_eh_or_frame_removed]
    // 0041A547 MOV EAX,FS:[0x0] [native_eh_or_frame_removed]
    // 0041A54D PUSH EAX [native_eh_or_frame_removed]
    // 0041A54E MOV dword ptr FS:[0x0],ESP [native_eh_or_frame_removed]
    // 0041A555 SUB ESP,0xb4 [native_eh_or_frame_removed]
    // 0041A55B MOV EDX,dword ptr [ESP + 0xc8] [actual_public_argument_cell]
        mov edx, dword ptr [ebp + 0C8h]
        mov edx, dword ptr [edx + 4]
    // 0041A562 FLD float ptr [EDX] [literal]
        _emit 0xd9
        _emit 0x02
    // 0041A564 PUSH EDI [literal]
        _emit 0x57
    // 0041A565 FSTP float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A569 MOV EDI,ECX [literal]
        _emit 0x8b
        _emit 0xf9
    // 0041A56B FLD float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A56F MOV dword ptr [ESP + 0xc],EDI [algorithm_cell_rebase]
        _emit 0x89
        _emit 0xbd
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A573 FLD float ptr [EDI + 0x14] [literal]
        _emit 0xd9
        _emit 0x47
        _emit 0x14
    // 0041A576 FCOMIP ST0,ST1 [literal]
        _emit 0xdf
        _emit 0xf1
    // 0041A578 JNC 0x0041ac33 [branch]
        jae clip_0041ac33
    // 0041A57E MOV ECX,dword ptr [ESP + 0xc8] [actual_public_argument_cell]
        mov ecx, dword ptr [ebp + 0C8h]
        mov ecx, dword ptr [ecx]
    // 0041A585 FLD float ptr [ECX] [literal]
        _emit 0xd9
        _emit 0x01
    // 0041A587 FSTP float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A58B FLD float ptr [EDI + 0x1c] [literal]
        _emit 0xd9
        _emit 0x47
        _emit 0x1c
    // 0041A58E FLD float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A592 FCOMI ST0,ST1 [literal]
        _emit 0xdb
        _emit 0xf1
    // 0041A594 FSTP ST1 [literal]
        _emit 0xdd
        _emit 0xd9
    // 0041A596 JNC 0x0041ac31 [branch]
        jae clip_0041ac31
    // 0041A59C FLD float ptr [EDX + 0x8] [literal]
        _emit 0xd9
        _emit 0x42
        _emit 0x08
    // 0041A59F FSTP float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5A3 FLD float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5A7 FLD float ptr [EDI + 0x18] [literal]
        _emit 0xd9
        _emit 0x47
        _emit 0x18
    // 0041A5AA FCOMIP ST0,ST1 [literal]
        _emit 0xdf
        _emit 0xf1
    // 0041A5AC JNC 0x0041ac2f [branch]
        jae clip_0041ac2f
    // 0041A5B2 FLD float ptr [ECX + 0x8] [literal]
        _emit 0xd9
        _emit 0x41
        _emit 0x08
    // 0041A5B5 FSTP float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5B9 FLD float ptr [EDI + 0x20] [literal]
        _emit 0xd9
        _emit 0x47
        _emit 0x20
    // 0041A5BC FLD float ptr [ESP + 0x4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5C0 FCOMI ST0,ST1 [literal]
        _emit 0xdb
        _emit 0xf1
    // 0041A5C2 FSTP ST1 [literal]
        _emit 0xdd
        _emit 0xd9
    // 0041A5C4 JNC 0x0041ac2d [branch]
        jae clip_0041ac2d
    // 0041A5CA FLD double ptr [0x00d7a220] [source_literal_binding]
        fld qword ptr kClipHundred
    // 0041A5D0 XORPS XMM0,XMM0 [literal]
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
    // 0041A5D3 FSUB ST3,ST0 [literal]
        _emit 0xdc
        _emit 0xeb
    // 0041A5D5 MOVSS XMM1,dword ptr [0x00d7a24c] [source_literal_binding]
        movss xmm1, dword ptr kClipOne
    // 0041A5DD MOVSS XMM2,dword ptr [0x00d7a260] [source_literal_binding]
        movss xmm2, dword ptr kClipNegativeOne
    // 0041A5E5 FXCH ST3 [literal]
        _emit 0xd9
        _emit 0xcb
    // 0041A5E7 PUSH EBX [literal]
        _emit 0x53
    // 0041A5E8 FCHS [literal]
        _emit 0xd9
        _emit 0xe0
    // 0041A5EA XOR EBX,EBX [literal]
        _emit 0x33
        _emit 0xdb
    // 0041A5EC FSTP float ptr [ESP + 0x94] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5F3 PUSH ESI [literal]
        _emit 0x56
    // 0041A5F4 FXCH ST3 [literal]
        _emit 0xd9
        _emit 0xcb
    // 0041A5F6 MOVSS dword ptr [ESP + 0x90],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A5FF FADD ST0,ST2 [literal]
        _emit 0xd8
        _emit 0xc2
    // 0041A601 MOVSS dword ptr [ESP + 0x94],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A60A MOVSS dword ptr [ESP + 0x9c],XMM2 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x95
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A613 MOVSS dword ptr [ESP + 0xa0],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A61C FSTP float ptr [ESP + 0xa4] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A623 MOVSS dword ptr [ESP + 0xa8],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A62C FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A62E MOVSS dword ptr [ESP + 0xac],XMM1 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x8d
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A637 FSUB ST0,ST1 [literal]
        _emit 0xd8
        _emit 0xe1
    // 0041A639 MOVSS dword ptr [ESP + 0xb4],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A642 MOVSS dword ptr [ESP + 0xb8],XMM2 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x95
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A64B MOV dword ptr [ESP + 0x28],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A64F FCHS [literal]
        _emit 0xd9
        _emit 0xe0
    // 0041A651 MOV dword ptr [ESP + 0x2c],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A655 FSTP float ptr [ESP + 0xb0] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A65C MOV dword ptr [ESP + 0x30],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A660 FADDP [literal]
        _emit 0xde
        _emit 0xc1
    // 0041A662 FSTP float ptr [ESP + 0xbc] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A669 MOV ESI,dword ptr [EDI + 0x4] [literal]
        _emit 0x8b
        _emit 0x77
        _emit 0x04
    // 0041A66C CMP ESI,EBX [literal]
        _emit 0x3b
        _emit 0xf3
    // 0041A66E MOV dword ptr [ESP + 0xc8],EBX [current_source_phase]
        _emit 0x89
        _emit 0x9d
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A675 JBE 0x0041a690 [branch]
        jbe clip_0041a690
    // 0041A677 LEA EAX,[ESI*0x8 + 0x0] [literal]
        _emit 0x8d
        _emit 0x04
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A67E PUSH EAX [literal]
        _emit 0x50
    // 0041A67F MOV dword ptr [ESP + 0x34],ESI [algorithm_cell_rebase]
        _emit 0x89
        _emit 0xb5
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A683 CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041A688 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A68B MOV dword ptr [ESP + 0x28],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A68F NOP [literal]
        _emit 0x90
    // 0041A690 MOV dword ptr [ESP + 0x2c],ESI [algorithm_cell_rebase]
    clip_0041a690:
        _emit 0x89
        _emit 0xb5
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A694 MOV dword ptr [ESP + 0x50],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A698 MOV dword ptr [ESP + 0x54],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x58
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A69C MOV dword ptr [ESP + 0x58],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x5c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6A0 MOV EAX,dword ptr [EDI + 0x4] [literal]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
    // 0041A6A3 CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041A6A5 MOV byte ptr [ESP + 0xc8],0x1 [current_source_phase]
        _emit 0xc6
        _emit 0x85
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
    // 0041A6AD JBE 0x0041a6c7 [branch]
        jbe clip_0041a6c7
    // 0041A6AF LEA ECX,[EAX*0x8 + 0x0] [literal]
        _emit 0x8d
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6B6 PUSH ECX [literal]
        _emit 0x51
    // 0041A6B7 MOV dword ptr [ESP + 0x5c],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x5c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6BB CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041A6C0 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A6C3 MOV dword ptr [ESP + 0x50],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6C7 MOV EDX,dword ptr [EDI + 0x4] [literal]
    clip_0041a6c7:
        _emit 0x8b
        _emit 0x57
        _emit 0x04
    // 0041A6CA XOR EAX,EAX [literal]
        _emit 0x33
        _emit 0xc0
    // 0041A6CC TEST EDX,EDX [literal]
        _emit 0x85
        _emit 0xd2
    // 0041A6CE JBE 0x0041a6ef [branch]
        jbe clip_0041a6ef
    // 0041A6D0 MOV ECX,dword ptr [EDI] [literal]
    clip_0041a6d0:
        _emit 0x8b
        _emit 0x0f
    // 0041A6D2 MOV ECX,dword ptr [ECX + EAX*0x4] [literal]
        _emit 0x8b
        _emit 0x0c
        _emit 0x81
    // 0041A6D5 MOV EDX,dword ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0x95
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6D9 FLD float ptr [ECX] [literal]
        _emit 0xd9
        _emit 0x01
    // 0041A6DB FSTP float ptr [EDX + EAX*0x8] [literal]
        _emit 0xd9
        _emit 0x1c
        _emit 0xc2
    // 0041A6DE ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041A6E1 FLD float ptr [ECX + 0x4] [literal]
        _emit 0xd9
        _emit 0x41
        _emit 0x04
    // 0041A6E4 FSTP float ptr [EDX + EAX*0x8 + -0x4] [literal]
        _emit 0xd9
        _emit 0x5c
        _emit 0xc2
        _emit 0xfc
    // 0041A6E8 MOV EDX,dword ptr [EDI + 0x4] [literal]
        _emit 0x8b
        _emit 0x57
        _emit 0x04
    // 0041A6EB CMP EAX,EDX [literal]
        _emit 0x3b
        _emit 0xc2
    // 0041A6ED JC 0x0041a6d0 [branch]
        jb clip_0041a6d0
    // 0041A6EF PUSH EBP [native_eh_or_frame_removed]
    clip_0041a6ef:
    // 0041A6F0 MOV ECX,EDI [literal]
        _emit 0x8b
        _emit 0xcf
    // 0041A6F2 CALL 0x004167c0 [genuine_provider]
        call clear_native_avoid_corner_records_004167c0
    // 0041A6F7 LEA EDX,[ESP + 0x94] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x95
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A6FE LEA ESI,[ESP + 0x54] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0xb5
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A702 LEA EAX,[ESP + 0x2c] [algorithm_cell_rebase]
        _emit 0x8d
        _emit 0x85
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A706 MOV dword ptr [ESP + 0x4c],EBX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x9d
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A70A MOV dword ptr [ESP + 0x10],EDX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x95
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A70E JMP 0x0041a718 [branch]
        jmp clip_0041a718
    // 0041A710 MOV EAX,dword ptr [ESP + 0x78] [algorithm_cell_rebase]
    clip_0041a710:
        _emit 0x8b
        _emit 0x85
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A714 MOV ESI,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0xb5
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A718 CMP dword ptr [ESI + 0x4],EBX [literal]
    clip_0041a718:
        _emit 0x39
        _emit 0x5e
        _emit 0x04
    // 0041A71B MOV ECX,EAX [literal]
        _emit 0x8b
        _emit 0xc8
    // 0041A71D MOV EAX,dword ptr [ECX + 0x4] [literal]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
    // 0041A720 MOV EBP,dword ptr [ECX] [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ecx]
        mov dword ptr [ebp + 0CCh], eax
        pop eax
    // 0041A722 MOVSS XMM0,dword ptr [EBP + EAX*0x8 + -0x8] [virtual_native_ebp]
        push ecx
        mov ecx, dword ptr [ebp + 0CCh]
        movss xmm0, dword ptr [ecx + eax*8 - 8]
        pop ecx
    // 0041A728 LEA EAX,[EBP + EAX*0x8 + -0x8] [virtual_native_ebp]
        push ecx
        mov ecx, dword ptr [ebp + 0CCh]
        lea eax, [ecx + eax*8 - 8]
        pop ecx
    // 0041A72C MOVSS dword ptr [ESP + 0x40],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A732 MOVSS XMM0,dword ptr [EAX + 0x4] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x04
    // 0041A737 MOV dword ptr [ESP + 0x20],ECX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x8d
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A73B MOV dword ptr [ESP + 0x78],ESI [algorithm_cell_rebase]
        _emit 0x89
        _emit 0xb5
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A73F MOVSS dword ptr [ESP + 0x44],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A745 JZ 0x0041a74a [branch]
        je clip_0041a74a
    // 0041A747 MOV dword ptr [ESI + 0x4],EBX [literal]
        _emit 0x89
        _emit 0x5e
        _emit 0x04
    // 0041A74A XOR EAX,EAX [literal]
    clip_0041a74a:
        _emit 0x33
        _emit 0xc0
    // 0041A74C CMP dword ptr [ECX + 0x4],EBX [literal]
        _emit 0x39
        _emit 0x59
        _emit 0x04
    // 0041A74F MOV dword ptr [ESP + 0x50],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A753 JBE 0x0041aab9 [branch]
        jbe clip_0041aab9
    // 0041A759 MOVSS XMM0,dword ptr [EDX + 0x4] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
    // 0041A75E MOVSS dword ptr [ESP + 0x38],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A764 MOVSS XMM0,dword ptr [EDX] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x02
    // 0041A768 MOVSS dword ptr [ESP + 0x3c],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A76E MOVSS XMM0,dword ptr [EDX + 0x8] [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
    // 0041A773 FLD float ptr [ESP + 0x3c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A777 FLD float ptr [ESP + 0x38] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A77B MOVSS dword ptr [ESP + 0x48],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A781 FLD float ptr [ESP + 0x48] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A785 JMP 0x0041a79e [branch]
        jmp clip_0041a79e
    // 0041A787..0041A78F: original unreachable padding
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A790 FLD float ptr [ESP + 0x48] [algorithm_cell_rebase]
    clip_0041a790:
        _emit 0xd9
        _emit 0x85
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A794 FLD float ptr [ESP + 0x38] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A798 FLD float ptr [ESP + 0x3c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A79C FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A79E MOV EDX,dword ptr [ECX] [literal]
    clip_0041a79e:
        _emit 0x8b
        _emit 0x11
    // 0041A7A0 FLD float ptr [EDX + EAX*0x8] [literal]
        _emit 0xd9
        _emit 0x04
        _emit 0xc2
    // 0041A7A3 LEA EAX,[EDX + EAX*0x8] [literal]
        _emit 0x8d
        _emit 0x04
        _emit 0xc2
    // 0041A7A6 FSTP float ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7AA FLD float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x40
        _emit 0x04
    // 0041A7AD FSTP float ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7B1 FLD float ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7B5 FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A7B7 FMUL ST3 [literal]
        _emit 0xd8
        _emit 0xcb
    // 0041A7B9 FLD float ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7BD FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A7BF FMUL ST6 [literal]
        _emit 0xd8
        _emit 0xce
    // 0041A7C1 FADDP ST2,ST0 [literal]
        _emit 0xde
        _emit 0xc2
    // 0041A7C3 FLD ST3 [literal]
        _emit 0xd9
        _emit 0xc3
    // 0041A7C5 FADDP ST2,ST0 [literal]
        _emit 0xde
        _emit 0xc2
    // 0041A7C7 FXCH [literal]
        _emit 0xd9
        _emit 0xc9
    // 0041A7C9 FSTP float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7CD FLD float ptr [ESP + 0x44] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7D1 FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A7D3 FMULP ST5 [literal]
        _emit 0xde
        _emit 0xcd
    // 0041A7D5 FLD float ptr [ESP + 0x40] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7D9 FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A7DB FMULP ST7 [literal]
        _emit 0xde
        _emit 0xcf
    // 0041A7DD FXCH ST5 [literal]
        _emit 0xd9
        _emit 0xcd
    // 0041A7DF FADDP ST6,ST0 [literal]
        _emit 0xde
        _emit 0xc6
    // 0041A7E1 FXCH ST5 [literal]
        _emit 0xd9
        _emit 0xcd
    // 0041A7E3 FADDP ST3,ST0 [literal]
        _emit 0xde
        _emit 0xc3
    // 0041A7E5 FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A7E7 FSTP float ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7EB FLDZ [literal]
        _emit 0xd9
        _emit 0xee
    // 0041A7ED FLD float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A7F1 FCOMI ST0,ST1 [literal]
        _emit 0xdb
        _emit 0xf1
    // 0041A7F3 FSTP ST1 [literal]
        _emit 0xdd
        _emit 0xd9
    // 0041A7F5 JBE 0x0041a984 [branch]
        jbe clip_0041a984
    // 0041A7FB XORPS XMM0,XMM0 [literal]
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
    // 0041A7FE COMISS XMM0,dword ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0x0f
        _emit 0x2f
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A803 JBE 0x0041a8ed [branch]
        jbe clip_0041a8ed
    // 0041A809 FLD ST3 [literal]
        _emit 0xd9
        _emit 0xc3
    // 0041A80B MOV EAX,dword ptr [ESI + 0x8] [literal]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
    // 0041A80E FSUBP ST3,ST0 [literal]
        _emit 0xde
        _emit 0xeb
    // 0041A810 MOV EBP,dword ptr [ESI + 0x4] [virtual_native_ebp]
        push eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp + 0CCh], eax
        pop eax
    // 0041A813 CMP EBP,EAX [virtual_native_ebp]
        cmp dword ptr [ebp + 0CCh], eax
    // 0041A815 FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A817 FSTP float ptr [ESP + 0x60] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x60
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A81B FSUB ST0,ST3 [literal]
        _emit 0xd8
        _emit 0xe3
    // 0041A81D FSTP float ptr [ESP + 0x64] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x64
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A821 FLD float ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A825 FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A827 FCHS [literal]
        _emit 0xd9
        _emit 0xe0
    // 0041A829 FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A82B FSUBRP [literal]
        _emit 0xde
        _emit 0xe1
    // 0041A82D FDIVP [literal]
        _emit 0xde
        _emit 0xf9
    // 0041A82F FSTP float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A833 FLD float ptr [ESP + 0x60] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x60
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A837 FLD float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A83B FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A83D FMULP ST2 [literal]
        _emit 0xde
        _emit 0xca
    // 0041A83F FXCH [literal]
        _emit 0xd9
        _emit 0xc9
    // 0041A841 FSTP float ptr [ESP + 0x84] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A848 FMUL float ptr [ESP + 0x64] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x8d
        _emit 0x64
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A84C FSTP float ptr [ESP + 0x88] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A853 FADD float ptr [ESP + 0x84] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x85
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A85A FSTP float ptr [ESP + 0x70] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A85E FADD float ptr [ESP + 0x88] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x85
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A865 FSTP float ptr [ESP + 0x74] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x74
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A869 JNZ 0x0041a8c7 [branch]
        jne clip_0041a8c7
    // 0041A86B LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041A86F MOV dword ptr [ESI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x08
    // 0041A872 ADD EAX,EAX [literal]
        _emit 0x03
        _emit 0xc0
    // 0041A874 ADD EAX,EAX [literal]
        _emit 0x03
        _emit 0xc0
    // 0041A876 ADD EAX,EAX [literal]
        _emit 0x03
        _emit 0xc0
    // 0041A878 PUSH EAX [literal]
        _emit 0x50
    // 0041A879 CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041A87E ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A881 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041A883 TEST EBP,EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        test eax, eax
        pop eax
    // 0041A885 MOV EBX,EAX [literal]
        _emit 0x8b
        _emit 0xd8
    // 0041A887 JBE 0x0041a8b2 [branch]
        jbe clip_0041a8b2
    // 0041A889 MOV EDI,EBP [virtual_native_ebp]
        mov edi, dword ptr [ebp + 0CCh]
    // 0041A88B JMP 0x0041a890 [branch]
        jmp clip_0041a890
    // 0041A88D..0041A88F: original unreachable padding
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    // 0041A890 TEST EAX,EAX [literal]
    clip_0041a890:
        _emit 0x85
        _emit 0xc0
    // 0041A892 JZ 0x0041a8a4 [branch]
        je clip_0041a8a4
    // 0041A894 MOV EDX,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x16
    // 0041A896 FLD float ptr [EDX + ECX*0x8] [literal]
        _emit 0xd9
        _emit 0x04
        _emit 0xca
    // 0041A899 LEA EDX,[EDX + ECX*0x8] [literal]
        _emit 0x8d
        _emit 0x14
        _emit 0xca
    // 0041A89C FSTP float ptr [EAX] [literal]
        _emit 0xd9
        _emit 0x18
    // 0041A89E FLD float ptr [EDX + 0x4] [literal]
        _emit 0xd9
        _emit 0x42
        _emit 0x04
    // 0041A8A1 FSTP float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
    // 0041A8A4 ADD ECX,0x1 [literal]
    clip_0041a8a4:
        _emit 0x83
        _emit 0xc1
        _emit 0x01
    // 0041A8A7 ADD EAX,0x8 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x08
    // 0041A8AA CMP ECX,EDI [literal]
        _emit 0x3b
        _emit 0xcf
    // 0041A8AC JC 0x0041a890 [branch]
        jb clip_0041a890
    // 0041A8AE MOV EDI,dword ptr [ESP + 0x18] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0xbd
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A8B2 MOV EAX,dword ptr [ESI] [literal]
    clip_0041a8b2:
        _emit 0x8b
        _emit 0x06
    // 0041A8B4 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041A8B6 JZ 0x0041a8c1 [branch]
        je clip_0041a8c1
    // 0041A8B8 PUSH EAX [literal]
        _emit 0x50
    // 0041A8B9 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041A8BE ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A8C1 MOV ECX,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
    clip_0041a8c1:
        _emit 0x8b
        _emit 0x8d
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A8C5 MOV dword ptr [ESI],EBX [literal]
        _emit 0x89
        _emit 0x1e
    // 0041A8C7 MOV EAX,dword ptr [ESI] [literal]
    clip_0041a8c7:
        _emit 0x8b
        _emit 0x06
    // 0041A8C9 LEA EAX,[EAX + EBP*0x8] [virtual_native_ebp]
        push ecx
        mov ecx, dword ptr [ebp + 0CCh]
        lea eax, [eax + ecx*8]
        pop ecx
    // 0041A8CC TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041A8CE JZ 0x0041a8e5 [branch]
        je clip_0041a8e5
    // 0041A8D0 MOVSS XMM0,dword ptr [ESP + 0x70] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A8D6 MOVSS dword ptr [EAX],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
    // 0041A8DA MOVSS XMM0,dword ptr [ESP + 0x74] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x74
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A8E0 MOVSS dword ptr [EAX + 0x4],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
    // 0041A8E5 ADD EBP,0x1 [virtual_native_ebp]
    clip_0041a8e5:
        add dword ptr [ebp + 0CCh], 1
    // 0041A8E8 MOV dword ptr [ESI + 0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        mov dword ptr [esi + 4], eax
        pop eax
    // 0041A8EB JMP 0x0041a8f7 [branch]
        jmp clip_0041a8f7
    // 0041A8ED FSTP ST4 [literal]
    clip_0041a8ed:
        _emit 0xdd
        _emit 0xdc
    // 0041A8EF FSTP ST2 [literal]
        _emit 0xdd
        _emit 0xda
    // 0041A8F1 FSTP ST1 [literal]
        _emit 0xdd
        _emit 0xd9
    // 0041A8F3 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041A8F5 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041A8F7 MOV EAX,dword ptr [ESI + 0x8] [literal]
    clip_0041a8f7:
        _emit 0x8b
        _emit 0x46
        _emit 0x08
    // 0041A8FA MOV EBP,dword ptr [ESI + 0x4] [virtual_native_ebp]
        push eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp + 0CCh], eax
        pop eax
    // 0041A8FD CMP EBP,EAX [virtual_native_ebp]
        cmp dword ptr [ebp + 0CCh], eax
    // 0041A8FF JNZ 0x0041a959 [branch]
        jne clip_0041a959
    // 0041A901 LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041A905 LEA ECX,[EAX*0x8 + 0x0] [literal]
        _emit 0x8d
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A90C PUSH ECX [literal]
        _emit 0x51
    // 0041A90D MOV dword ptr [ESI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x08
    // 0041A910 CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041A915 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A918 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041A91A TEST EBP,EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        test eax, eax
        pop eax
    // 0041A91C MOV EBX,EAX [literal]
        _emit 0x8b
        _emit 0xd8
    // 0041A91E JBE 0x0041a944 [branch]
        jbe clip_0041a944
    // 0041A920 MOV EDI,EBP [virtual_native_ebp]
        mov edi, dword ptr [ebp + 0CCh]
    // 0041A922 TEST EAX,EAX [literal]
    clip_0041a922:
        _emit 0x85
        _emit 0xc0
    // 0041A924 JZ 0x0041a936 [branch]
        je clip_0041a936
    // 0041A926 MOV EDX,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x16
    // 0041A928 FLD float ptr [EDX + ECX*0x8] [literal]
        _emit 0xd9
        _emit 0x04
        _emit 0xca
    // 0041A92B LEA EDX,[EDX + ECX*0x8] [literal]
        _emit 0x8d
        _emit 0x14
        _emit 0xca
    // 0041A92E FSTP float ptr [EAX] [literal]
        _emit 0xd9
        _emit 0x18
    // 0041A930 FLD float ptr [EDX + 0x4] [literal]
        _emit 0xd9
        _emit 0x42
        _emit 0x04
    // 0041A933 FSTP float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
    // 0041A936 ADD ECX,0x1 [literal]
    clip_0041a936:
        _emit 0x83
        _emit 0xc1
        _emit 0x01
    // 0041A939 ADD EAX,0x8 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x08
    // 0041A93C CMP ECX,EDI [literal]
        _emit 0x3b
        _emit 0xcf
    // 0041A93E JC 0x0041a922 [branch]
        jb clip_0041a922
    // 0041A940 MOV EDI,dword ptr [ESP + 0x18] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0xbd
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A944 MOV EAX,dword ptr [ESI] [literal]
    clip_0041a944:
        _emit 0x8b
        _emit 0x06
    // 0041A946 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041A948 JZ 0x0041a953 [branch]
        je clip_0041a953
    // 0041A94A PUSH EAX [literal]
        _emit 0x50
    // 0041A94B CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041A950 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041A953 MOV ECX,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
    clip_0041a953:
        _emit 0x8b
        _emit 0x8d
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A957 MOV dword ptr [ESI],EBX [literal]
        _emit 0x89
        _emit 0x1e
    // 0041A959 MOV EAX,dword ptr [ESI] [literal]
    clip_0041a959:
        _emit 0x8b
        _emit 0x06
    // 0041A95B LEA EAX,[EAX + EBP*0x8] [virtual_native_ebp]
        push ecx
        mov ecx, dword ptr [ebp + 0CCh]
        lea eax, [eax + ecx*8]
        pop ecx
    // 0041A95E XOR EBX,EBX [literal]
        _emit 0x33
        _emit 0xdb
    // 0041A960 CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041A962 JZ 0x0041a979 [branch]
        je clip_0041a979
    // 0041A964 MOVSS XMM0,dword ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A96A MOVSS dword ptr [EAX],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
    // 0041A96E MOVSS XMM0,dword ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A974 MOVSS dword ptr [EAX + 0x4],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
    // 0041A979 ADD EBP,0x1 [virtual_native_ebp]
    clip_0041a979:
        add dword ptr [ebp + 0CCh], 1
    // 0041A97C MOV dword ptr [ESI + 0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        mov dword ptr [esi + 4], eax
        pop eax
    // 0041A97F JMP 0x0041aa89 [branch]
        jmp clip_0041aa89
    // 0041A984 MOVSS XMM0,dword ptr [ESP + 0x1c] [algorithm_cell_rebase]
    clip_0041a984:
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A98A COMISS XMM0,dword ptr [0x00d7a218] [source_literal_binding]
        comiss xmm0, dword ptr kClipZero
    // 0041A991 JBE 0x0041aa7f [branch]
        jbe clip_0041aa7f
    // 0041A997 FLD ST3 [literal]
        _emit 0xd9
        _emit 0xc3
    // 0041A999 MOV EAX,dword ptr [ESI + 0x8] [literal]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
    // 0041A99C FSUBP ST3,ST0 [literal]
        _emit 0xde
        _emit 0xeb
    // 0041A99E MOV EBP,dword ptr [ESI + 0x4] [virtual_native_ebp]
        push eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp + 0CCh], eax
        pop eax
    // 0041A9A1 CMP EBP,EAX [virtual_native_ebp]
        cmp dword ptr [ebp + 0CCh], eax
    // 0041A9A3 FXCH ST2 [literal]
        _emit 0xd9
        _emit 0xca
    // 0041A9A5 FSTP float ptr [ESP + 0x8c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9AC FSUB ST0,ST3 [literal]
        _emit 0xd8
        _emit 0xe3
    // 0041A9AE FSTP float ptr [ESP + 0x90] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9B5 FLD float ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9B9 FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A9BB FSUBRP ST2,ST0 [literal]
        _emit 0xde
        _emit 0xe2
    // 0041A9BD FDIVRP [literal]
        _emit 0xde
        _emit 0xf1
    // 0041A9BF FSTP float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9C3 FLD float ptr [ESP + 0x8c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9CA FLD float ptr [ESP + 0x14] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x85
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9CE FLD ST0 [literal]
        _emit 0xd9
        _emit 0xc0
    // 0041A9D0 FMULP ST2 [literal]
        _emit 0xde
        _emit 0xca
    // 0041A9D2 FXCH [literal]
        _emit 0xd9
        _emit 0xc9
    // 0041A9D4 FSTP float ptr [ESP + 0x7c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9D8 FMUL float ptr [ESP + 0x90] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x8d
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9DF FSTP float ptr [ESP + 0x80] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9E6 FADD float ptr [ESP + 0x7c] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x85
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9EA FSTP float ptr [ESP + 0x68] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9EE FADD float ptr [ESP + 0x80] [algorithm_cell_rebase]
        _emit 0xd8
        _emit 0x85
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9F5 FSTP float ptr [ESP + 0x6c] [algorithm_cell_rebase]
        _emit 0xd9
        _emit 0x9d
        _emit 0x6c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041A9F9 JNZ 0x0041aa57 [branch]
        jne clip_0041aa57
    // 0041A9FB LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041A9FF LEA ECX,[EAX*0x8 + 0x0] [literal]
        _emit 0x8d
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA06 PUSH ECX [literal]
        _emit 0x51
    // 0041AA07 MOV dword ptr [ESI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x08
    // 0041AA0A CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041AA0F ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AA12 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041AA14 TEST EBP,EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        test eax, eax
        pop eax
    // 0041AA16 MOV EBX,EAX [literal]
        _emit 0x8b
        _emit 0xd8
    // 0041AA18 JBE 0x0041aa42 [branch]
        jbe clip_0041aa42
    // 0041AA1A MOV EDI,EBP [virtual_native_ebp]
        mov edi, dword ptr [ebp + 0CCh]
    // 0041AA1C LEA ESP,[ESP] [literal]
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    // 0041AA20 TEST EAX,EAX [literal]
    clip_0041aa20:
        _emit 0x85
        _emit 0xc0
    // 0041AA22 JZ 0x0041aa34 [branch]
        je clip_0041aa34
    // 0041AA24 MOV EDX,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x16
    // 0041AA26 FLD float ptr [EDX + ECX*0x8] [literal]
        _emit 0xd9
        _emit 0x04
        _emit 0xca
    // 0041AA29 LEA EDX,[EDX + ECX*0x8] [literal]
        _emit 0x8d
        _emit 0x14
        _emit 0xca
    // 0041AA2C FSTP float ptr [EAX] [literal]
        _emit 0xd9
        _emit 0x18
    // 0041AA2E FLD float ptr [EDX + 0x4] [literal]
        _emit 0xd9
        _emit 0x42
        _emit 0x04
    // 0041AA31 FSTP float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
    // 0041AA34 ADD ECX,0x1 [literal]
    clip_0041aa34:
        _emit 0x83
        _emit 0xc1
        _emit 0x01
    // 0041AA37 ADD EAX,0x8 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x08
    // 0041AA3A CMP ECX,EDI [literal]
        _emit 0x3b
        _emit 0xcf
    // 0041AA3C JC 0x0041aa20 [branch]
        jb clip_0041aa20
    // 0041AA3E MOV EDI,dword ptr [ESP + 0x18] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0xbd
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA42 MOV EAX,dword ptr [ESI] [literal]
    clip_0041aa42:
        _emit 0x8b
        _emit 0x06
    // 0041AA44 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041AA46 JZ 0x0041aa51 [branch]
        je clip_0041aa51
    // 0041AA48 PUSH EAX [literal]
        _emit 0x50
    // 0041AA49 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041AA4E ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AA51 MOV ECX,dword ptr [ESP + 0x20] [algorithm_cell_rebase]
    clip_0041aa51:
        _emit 0x8b
        _emit 0x8d
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA55 MOV dword ptr [ESI],EBX [literal]
        _emit 0x89
        _emit 0x1e
    // 0041AA57 MOV EAX,dword ptr [ESI] [literal]
    clip_0041aa57:
        _emit 0x8b
        _emit 0x06
    // 0041AA59 LEA EAX,[EAX + EBP*0x8] [virtual_native_ebp]
        push ecx
        mov ecx, dword ptr [ebp + 0CCh]
        lea eax, [eax + ecx*8]
        pop ecx
    // 0041AA5C XOR EBX,EBX [literal]
        _emit 0x33
        _emit 0xdb
    // 0041AA5E CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041AA60 JZ 0x0041aa77 [branch]
        je clip_0041aa77
    // 0041AA62 MOVSS XMM0,dword ptr [ESP + 0x68] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA68 MOVSS dword ptr [EAX],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
    // 0041AA6C MOVSS XMM0,dword ptr [ESP + 0x6c] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x6c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA72 MOVSS dword ptr [EAX + 0x4],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
    // 0041AA77 ADD EBP,0x1 [virtual_native_ebp]
    clip_0041aa77:
        add dword ptr [ebp + 0CCh], 1
    // 0041AA7A MOV dword ptr [ESI + 0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        mov dword ptr [esi + 4], eax
        pop eax
    // 0041AA7D JMP 0x0041aa89 [branch]
        jmp clip_0041aa89
    // 0041AA7F FSTP ST4 [literal]
    clip_0041aa7f:
        _emit 0xdd
        _emit 0xdc
    // 0041AA81 FSTP ST2 [literal]
        _emit 0xdd
        _emit 0xda
    // 0041AA83 FSTP ST1 [literal]
        _emit 0xdd
        _emit 0xd9
    // 0041AA85 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041AA87 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041AA89 MOV EAX,dword ptr [ESP + 0x50] [algorithm_cell_rebase]
    clip_0041aa89:
        _emit 0x8b
        _emit 0x85
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA8D MOVSS XMM0,dword ptr [ESP + 0x24] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA93 ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041AA96 CMP EAX,dword ptr [ECX + 0x4] [literal]
        _emit 0x3b
        _emit 0x41
        _emit 0x04
    // 0041AA99 MOVSS dword ptr [ESP + 0x40],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AA9F MOVSS XMM0,dword ptr [ESP + 0x28] [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x85
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAA5 MOVSS dword ptr [ESP + 0x44],XMM0 [algorithm_cell_rebase]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x85
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAAB MOV dword ptr [ESP + 0x50],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAAF JC 0x0041a790 [branch]
        jb clip_0041a790
    // 0041AAB5 MOV EDX,dword ptr [ESP + 0x10] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0x95
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAB9 CMP dword ptr [ESI + 0x4],0x3 [literal]
    clip_0041aab9:
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x03
    // 0041AABD JC 0x0041ab3b [branch]
        jb clip_0041ab3b
    // 0041AABF MOV EAX,dword ptr [ESP + 0x4c] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0x85
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAC3 ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041AAC6 ADD EDX,0xc [literal]
        _emit 0x83
        _emit 0xc2
        _emit 0x0c
    // 0041AAC9 CMP EAX,0x4 [literal]
        _emit 0x83
        _emit 0xf8
        _emit 0x04
    // 0041AACC MOV dword ptr [ESP + 0x4c],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAD0 MOV dword ptr [ESP + 0x10],EDX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x95
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAD4 JL 0x0041a710 [branch]
        jl clip_0041a710
    // 0041AADA MOV EAX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
    // 0041AADD CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041AADF JBE 0x0041ac11 [branch]
        jbe clip_0041ac11
    // 0041AAE5 MOV ESI,dword ptr [ESI] [literal]
        _emit 0x8b
        _emit 0x36
    // 0041AAE7 MOV dword ptr [ESP + 0x10],EAX [algorithm_cell_rebase]
        _emit 0x89
        _emit 0x85
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AAEB JMP 0x0041aaf0 [branch]
        jmp clip_0041aaf0
    // 0041AAED..0041AAEF: original unreachable padding
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    // 0041AAF0 PUSH 0x24 [literal]
    clip_0041aaf0:
        _emit 0x6a
        _emit 0x24
    // 0041AAF2 CALL 0x00bf681b [genuine_provider]
        call allocate_clip_bytes
    // 0041AAF7 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AAFA TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041AAFC JZ 0x0041ab78 [branch]
        je clip_0041ab78
    // 0041AAFE XORPS XMM0,XMM0 [literal]
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
    // 0041AB01 MOVSS dword ptr [EAX + 0x14],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x14
    // 0041AB06 MOVSS dword ptr [EAX + 0x10],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x10
    // 0041AB0B MOVSS dword ptr [EAX + 0x8],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
    // 0041AB10 FLD float ptr [EAX + 0x14] [literal]
        _emit 0xd9
        _emit 0x40
        _emit 0x14
    // 0041AB13 FSTP float ptr [EAX + 0xc] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x0c
    // 0041AB16 MOVSS dword ptr [EAX],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
    // 0041AB1A FLD float ptr [EAX + 0xc] [literal]
        _emit 0xd9
        _emit 0x40
        _emit 0x0c
    // 0041AB1D MOVSS dword ptr [EAX + 0x18],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x18
    // 0041AB22 MOVSS dword ptr [EAX + 0x1c],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x1c
    // 0041AB27 FSTP float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
    // 0041AB2A MOVSS XMM0,dword ptr [0x00d7a260] [source_literal_binding]
        movss xmm0, dword ptr kClipNegativeOne
    // 0041AB32 MOVSS dword ptr [EAX + 0x20],XMM0 [literal]
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x20
    // 0041AB37 MOV EBP,EAX [virtual_native_ebp]
        mov dword ptr [ebp + 0CCh], eax
    // 0041AB39 JMP 0x0041ab7a [branch]
        jmp clip_0041ab7a
    // 0041AB3B MOV EAX,dword ptr [ESP + 0x54] [algorithm_cell_rebase]
    clip_0041ab3b:
        _emit 0x8b
        _emit 0x85
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AB3F CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041AB41 JZ 0x0041ab4c [branch]
        je clip_0041ab4c
    // 0041AB43 PUSH EAX [literal]
        _emit 0x50
    // 0041AB44 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041AB49 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AB4C MOV EAX,dword ptr [ESP + 0x2c] [algorithm_cell_rebase]
    clip_0041ab4c:
        _emit 0x8b
        _emit 0x85
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AB50 CMP EAX,EBX [literal]
        _emit 0x3b
        _emit 0xc3
    // 0041AB52 JZ 0x0041ab5d [branch]
    clip_0041ab52:
        je clip_0041ab5d
    // 0041AB54 PUSH EAX [literal]
        _emit 0x50
    // 0041AB55 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041AB5A ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AB5D POP EBP [native_eh_or_frame_removed]
    clip_0041ab5d:
    // 0041AB5E POP ESI [literal]
        _emit 0x5e
    // 0041AB5F POP EBX [literal]
        _emit 0x5b
    // 0041AB60 POP EDI [literal]
        _emit 0x5f
    // 0041AB61 MOV ECX,dword ptr [ESP + 0xb4] [native_eh_or_frame_removed]
    // 0041AB68 MOV dword ptr FS:[0x0],ECX [native_eh_or_frame_removed]
    // 0041AB6F ADD ESP,0xc0 [native_eh_or_frame_removed]
    // 0041AB75 RET 0x8 [private_kernel_return]
        pop ebp
        ret
    // 0041AB78 XOR EBP,EBP [virtual_native_ebp]
    clip_0041ab78:
        push eax
        xor eax, eax
        mov dword ptr [ebp + 0CCh], eax
        pop eax
    // 0041AB7A MOV EAX,dword ptr [EDI + 0x8] [literal]
    clip_0041ab7a:
        _emit 0x8b
        _emit 0x47
        _emit 0x08
    // 0041AB7D MOV ECX,dword ptr [EDI + 0x4] [literal]
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
    // 0041AB80 CMP ECX,EAX [literal]
        _emit 0x3b
        _emit 0xc8
    // 0041AB82 JNZ 0x0041abe1 [branch]
        jne clip_0041abe1
    // 0041AB84 MOV EAX,dword ptr [EDI + 0x8] [literal]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
    // 0041AB87 LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041AB8B CMP EAX,dword ptr [EDI + 0x8] [literal]
        _emit 0x3b
        _emit 0x47
        _emit 0x08
    // 0041AB8E JBE 0x0041abe1 [branch]
        jbe clip_0041abe1
    // 0041AB90 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041AB92 MOV dword ptr [EDI + 0x8],EAX [literal]
        _emit 0x89
        _emit 0x47
        _emit 0x08
    // 0041AB95 MOV EDX,0x4 [literal]
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AB9A MUL EDX [literal]
        _emit 0xf7
        _emit 0xe2
    // 0041AB9C SETO CL [literal]
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
    // 0041AB9F NEG ECX [literal]
        _emit 0xf7
        _emit 0xd9
    // 0041ABA1 OR ECX,EAX [literal]
        _emit 0x0b
        _emit 0xc8
    // 0041ABA3 PUSH ECX [literal]
        _emit 0x51
    // 0041ABA4 CALL 0x00bf55be [genuine_provider]
        call allocate_clip_bytes
    // 0041ABA9 MOV EBX,dword ptr [ESP + 0x1c] [algorithm_cell_rebase]
        _emit 0x8b
        _emit 0x9d
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041ABAD ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041ABB0 CMP dword ptr [EBX],0x0 [literal]
        _emit 0x83
        _emit 0x3b
        _emit 0x00
    // 0041ABB3 MOV EDI,EAX [literal]
        _emit 0x8b
        _emit 0xf8
    // 0041ABB5 JZ 0x0041abdd [branch]
        je clip_0041abdd
    // 0041ABB7 MOV ECX,dword ptr [EBX + 0x4] [literal]
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
    // 0041ABBA XOR EAX,EAX [literal]
        _emit 0x33
        _emit 0xc0
    // 0041ABBC TEST ECX,ECX [literal]
        _emit 0x85
        _emit 0xc9
    // 0041ABBE JBE 0x0041abd2 [branch]
        jbe clip_0041abd2
    // 0041ABC0 MOV EDX,dword ptr [EBX] [literal]
    clip_0041abc0:
        _emit 0x8b
        _emit 0x13
    // 0041ABC2 MOV ECX,dword ptr [EDX + EAX*0x4] [literal]
        _emit 0x8b
        _emit 0x0c
        _emit 0x82
    // 0041ABC5 MOV dword ptr [EDI + EAX*0x4],ECX [literal]
        _emit 0x89
        _emit 0x0c
        _emit 0x87
    // 0041ABC8 MOV EDX,dword ptr [EBX + 0x4] [literal]
        _emit 0x8b
        _emit 0x53
        _emit 0x04
    // 0041ABCB ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041ABCE CMP EAX,EDX [literal]
        _emit 0x3b
        _emit 0xc2
    // 0041ABD0 JC 0x0041abc0 [branch]
        jb clip_0041abc0
    // 0041ABD2 MOV EAX,dword ptr [EBX] [literal]
    clip_0041abd2:
        _emit 0x8b
        _emit 0x03
    // 0041ABD4 PUSH EAX [literal]
        _emit 0x50
    // 0041ABD5 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041ABDA ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041ABDD MOV dword ptr [EBX],EDI [literal]
    clip_0041abdd:
        _emit 0x89
        _emit 0x3b
    // 0041ABDF MOV EDI,EBX [literal]
        _emit 0x8b
        _emit 0xfb
    // 0041ABE1 MOV ECX,dword ptr [EDI + 0x4] [literal]
    clip_0041abe1:
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
    // 0041ABE4 MOV EDX,dword ptr [EDI] [literal]
        _emit 0x8b
        _emit 0x17
    // 0041ABE6 MOV dword ptr [EDX + ECX*0x4],EBP [virtual_native_ebp]
        push eax
        mov eax, dword ptr [ebp + 0CCh]
        mov dword ptr [edx + ecx*4], eax
        pop eax
    // 0041ABE9 MOV ECX,0x1 [literal]
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041ABEE ADD dword ptr [EDI + 0x4],ECX [literal]
        _emit 0x01
        _emit 0x4f
        _emit 0x04
    // 0041ABF1 MOV EAX,dword ptr [EDI + 0x4] [literal]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
    // 0041ABF4 FLD float ptr [ESI] [literal]
        _emit 0xd9
        _emit 0x06
    // 0041ABF6 MOV EDX,dword ptr [EDI] [literal]
        _emit 0x8b
        _emit 0x17
    // 0041ABF8 MOV EAX,dword ptr [EDX + EAX*0x4 + -0x4] [literal]
        _emit 0x8b
        _emit 0x44
        _emit 0x82
        _emit 0xfc
    // 0041ABFC FSTP float ptr [EAX] [literal]
        _emit 0xd9
        _emit 0x18
    // 0041ABFE ADD ESI,0x8 [literal]
        _emit 0x83
        _emit 0xc6
        _emit 0x08
    // 0041AC01 SUB dword ptr [ESP + 0x10],ECX [algorithm_cell_rebase]
        _emit 0x29
        _emit 0x8d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AC05 FLD float ptr [ESI + -0x4] [literal]
        _emit 0xd9
        _emit 0x46
        _emit 0xfc
    // 0041AC08 FSTP float ptr [EAX + 0x4] [literal]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
    // 0041AC0B JNZ 0x0041aaf0 [branch]
        jne clip_0041aaf0
    // 0041AC11 MOV EAX,dword ptr [ESP + 0x54] [algorithm_cell_rebase]
    clip_0041ac11:
        _emit 0x8b
        _emit 0x85
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AC15 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041AC17 JZ 0x0041ac22 [branch]
        je clip_0041ac22
    // 0041AC19 PUSH EAX [literal]
        _emit 0x50
    // 0041AC1A CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041AC1F ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041AC22 MOV EAX,dword ptr [ESP + 0x2c] [algorithm_cell_rebase]
    clip_0041ac22:
        _emit 0x8b
        _emit 0x85
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041AC26 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041AC28 JMP 0x0041ab52 [branch]
        jmp clip_0041ab52
    // 0041AC2D FSTP ST2 [literal]
    clip_0041ac2d:
        _emit 0xdd
        _emit 0xda
    // 0041AC2F FSTP ST1 [literal]
    clip_0041ac2f:
        _emit 0xdd
        _emit 0xd9
    // 0041AC31 FSTP ST0 [literal]
    clip_0041ac31:
        _emit 0xdd
        _emit 0xd8
    // 0041AC33 MOV ECX,EDI [literal]
    clip_0041ac33:
        _emit 0x8b
        _emit 0xcf
    // 0041AC35 FSTP ST0 [literal]
        _emit 0xdd
        _emit 0xd8
    // 0041AC37 CALL 0x004167c0 [genuine_provider]
        call clear_native_avoid_corner_records_004167c0
    // 0041AC3C MOV ECX,dword ptr [ESP + 0xb8] [native_eh_or_frame_removed]
    // 0041AC43 POP EDI [literal]
        _emit 0x5f
    // 0041AC44 MOV dword ptr FS:[0x0],ECX [native_eh_or_frame_removed]
    // 0041AC4B ADD ESP,0xc0 [native_eh_or_frame_removed]
    // 0041AC51 RET 0x8 [private_kernel_return]
        pop ebp
        ret
    }
}

class CurrentPointerCellGuard {
public:
    CurrentPointerCellGuard(ClipWorkingSet& working, std::int32_t first_phase,
                            std::size_t cell_offset) noexcept
        : working_(working), first_phase_(first_phase), cell_offset_(cell_offset) {}

    ~CurrentPointerCellGuard() noexcept {
        if (working_.phase >= first_phase_) {
            free_native_current_pointer_cell_00412ef0(
                working_.algorithm + cell_offset_);
        }
    }

    CurrentPointerCellGuard(const CurrentPointerCellGuard&) = delete;
    CurrentPointerCellGuard& operator=(const CurrentPointerCellGuard&) = delete;

private:
    ClipWorkingSet& working_;
    const std::int32_t first_phase_;
    const std::size_t cell_offset_;
};

__declspec(noinline) void __cdecl clip_current_source_frames(
    void* actual_zone, const std::uint32_t* actual_argument_cells) {
    ClipWorkingSet working;
    working.phase = -1;
    working.actual_arguments = actual_argument_cells;
    CurrentPointerCellGuard first(working, 0, 0x2c);
    CurrentPointerCellGuard second(working, 1, 0x54);

    // Fixed actual entry, not an injectable provider. The non-noexcept volatile
    // binding retains genuine current MSVC cleanup regions around the naked
    // kernel. Its working-set cells outlive that kernel's unwound call frame.
    using Kernel = void (__fastcall*)(void*, ClipWorkingSet*);
    Kernel volatile kernel = run_clip_kernel;
    kernel(actual_zone, &working);

    // Native leaves both phases active through BOTH normal frees (second,
    // first). Dismiss only after normal kernel completion, including rejection.
    working.phase = -1;
}

} // namespace

__declspec(naked) void __fastcall clip_native_avoid_zone_world_bounds_0041a540(
    void*, void*, const float*, const float*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push ecx
        call clip_current_source_frames
        add esp, 8
        ret 8
    }
}

} // namespace bsp
