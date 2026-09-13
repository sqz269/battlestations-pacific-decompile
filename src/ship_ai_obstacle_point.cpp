#include "bsp/ship_ai_obstacle_point.hpp"
#include <cstddef>
#include <type_traits>

namespace bsp {
namespace {
static_assert(std::is_standard_layout_v<ShipAiObstacleNode>);
// Map the native readers onto the existing semantic record, never reinterpret
// that record as the original 90h allocation. Historical field names are kept.
enum : std::size_t {
    off_near_box_x = offsetof(ShipAiObstacleNode, near_box_x), // native+20
    off_near_box_z = offsetof(ShipAiObstacleNode, near_box_z), // native+24
    off_axis_beam_x = offsetof(ShipAiObstacleNode, axis_beam_x), // native+28
    off_axis_beam_z = offsetof(ShipAiObstacleNode, axis_beam_z), // native+2C
    off_axis_forward_x = offsetof(ShipAiObstacleNode, axis_forward_x), // native+30
    off_axis_forward_z = offsetof(ShipAiObstacleNode, axis_forward_z), // native+34
    off_near_half_beam = offsetof(ShipAiObstacleNode, near_half_beam), // native+38
    off_near_half_length = offsetof(ShipAiObstacleNode, near_half_length), // native+3C
    off_avoid_box_x = offsetof(ShipAiObstacleNode, avoid_box_x), // native+44
    off_avoid_box_z = offsetof(ShipAiObstacleNode, avoid_box_z), // native+48
    off_avoid_half_beam = offsetof(ShipAiObstacleNode, avoid_half_beam), // native+5C
    off_avoid_half_length = offsetof(ShipAiObstacleNode, avoid_half_length), // native+60
    off_no_pose_68 = offsetof(ShipAiObstacleNode, no_pose_68), // native+68
    off_no_arc_69 = offsetof(ShipAiObstacleNode, no_arc_69), // native+69
};

// The original node is ECX and point is the sole stack argument. Source ECX is
// the semantic node and EDX is point. Push point before the unchanged 8-byte
// native frame; argument-pointer/dot spills move from ESP+C to ESP+8. Delta
// spills stay at ESP+0/+4. All FP operations are unchanged. EDX is scratch.
__declspec(naked) bool __fastcall near_kernel(const ShipAiObstacleNode*, const float*) noexcept {
    __asm {
        push edx // source point is in EDX; reserve its spill slot before native frame
        SUB ESP,0x8 // 009d80c0
        CMP byte ptr [ECX + off_no_pose_68],0x0 // 009d80c3
        JNZ L_009d814f // 009d80c7
        MOV EAX,dword ptr [ESP + 0x8] // 009d80cd
        FLD dword ptr [EAX] // 009d80d1
        FSUB dword ptr [ECX + off_near_box_x] // 009d80d3
        FSTP dword ptr [ESP] // 009d80d6
        FLD dword ptr [EAX + 0x4] // 009d80d9
        FSUB dword ptr [ECX + off_near_box_z] // 009d80dc
        FSTP dword ptr [ESP + 0x4] // 009d80df
        FLD dword ptr [ECX + off_axis_beam_z] // 009d80e3
        FLD dword ptr [ESP + 0x4] // 009d80e6
        FLD st(0) // 009d80ea
        FMULP st(2),st(0) // 009d80ec
        FLD dword ptr [ECX + off_axis_beam_x] // 009d80ee
        FLD dword ptr [ESP] // 009d80f1
        FLD st(0) // 009d80f4
        FMULP st(2),st(0) // 009d80f6
        FXCH st(3) // 009d80f8
        FADDP st(1),st(0) // 009d80fa
        FSTP dword ptr [ESP + 0x8] // 009d80fc
        MOV EAX,dword ptr [ESP + 0x8] // 009d8100
        AND EAX,0x7fffffff // 009d8104
        MOV dword ptr [ESP + 0x8],EAX // 009d8109
        FLD dword ptr [ESP + 0x8] // 009d810d
        FLD dword ptr [ECX + off_near_half_beam] // 009d8111
        FCOMIP st(0),st(1) // 009d8114
        FSTP st(0) // 009d8116
        JC L_009d814b // 009d8118
        FMUL dword ptr [ECX + off_axis_forward_z] // 009d811a
        FLD dword ptr [ECX + off_axis_forward_x] // 009d811d
        FMULP st(2),st(0) // 009d8120
        FADDP st(1),st(0) // 009d8122
        FSTP dword ptr [ESP + 0x8] // 009d8124
        MOV EDX,dword ptr [ESP + 0x8] // 009d8128
        AND EDX,0x7fffffff // 009d812c
        MOV dword ptr [ESP + 0x8],EDX // 009d8132
        FLD dword ptr [ESP + 0x8] // 009d8136
        FLD dword ptr [ECX + off_near_half_length] // 009d813a
        FCOMIP st(0),st(1) // 009d813d
        FSTP st(0) // 009d813f
        JC L_009d814f // 009d8141
        MOV AL,0x1 // 009d8143
        ADD ESP,0x8 // 009d8145
        pop edx // release the source point/spill slot
        RET // source fastcall has no stack argument // 009d8148
L_009d814b:
        FSTP st(0) // 009d814b
        FSTP st(0) // 009d814d
L_009d814f:
        XOR AL,AL // 009d814f
        ADD ESP,0x8 // 009d8151
        pop edx // release the source point/spill slot
        RET // source fastcall has no stack argument // 009d8154
    }
}

__declspec(naked) bool __fastcall avoid_kernel(const ShipAiObstacleNode*, const float*) noexcept {
    __asm {
        push edx // source point is in EDX; reserve its spill slot before native frame
        SUB ESP,0x8 // 009d8160
        CMP byte ptr [ECX + off_no_pose_68],0x0 // 009d8163
        JNZ L_009d81f9 // 009d8167
        CMP byte ptr [ECX + off_no_arc_69],0x0 // 009d816d
        JNZ L_009d81f9 // 009d8171
        MOV EAX,dword ptr [ESP + 0x8] // 009d8177
        FLD dword ptr [EAX] // 009d817b
        FSUB dword ptr [ECX + off_avoid_box_x] // 009d817d
        FSTP dword ptr [ESP] // 009d8180
        FLD dword ptr [EAX + 0x4] // 009d8183
        FSUB dword ptr [ECX + off_avoid_box_z] // 009d8186
        FSTP dword ptr [ESP + 0x4] // 009d8189
        FLD dword ptr [ECX + off_axis_beam_z] // 009d818d
        FLD dword ptr [ESP + 0x4] // 009d8190
        FLD st(0) // 009d8194
        FMULP st(2),st(0) // 009d8196
        FLD dword ptr [ECX + off_axis_beam_x] // 009d8198
        FLD dword ptr [ESP] // 009d819b
        FLD st(0) // 009d819e
        FMULP st(2),st(0) // 009d81a0
        FXCH st(3) // 009d81a2
        FADDP st(1),st(0) // 009d81a4
        FSTP dword ptr [ESP + 0x8] // 009d81a6
        MOV EAX,dword ptr [ESP + 0x8] // 009d81aa
        AND EAX,0x7fffffff // 009d81ae
        MOV dword ptr [ESP + 0x8],EAX // 009d81b3
        FLD dword ptr [ESP + 0x8] // 009d81b7
        FLD dword ptr [ECX + off_avoid_half_beam] // 009d81bb
        FCOMIP st(0),st(1) // 009d81be
        FSTP st(0) // 009d81c0
        JC L_009d81f5 // 009d81c2
        FMUL dword ptr [ECX + off_axis_forward_z] // 009d81c4
        FLD dword ptr [ECX + off_axis_forward_x] // 009d81c7
        FMULP st(2),st(0) // 009d81ca
        FADDP st(1),st(0) // 009d81cc
        FSTP dword ptr [ESP + 0x8] // 009d81ce
        MOV EDX,dword ptr [ESP + 0x8] // 009d81d2
        AND EDX,0x7fffffff // 009d81d6
        MOV dword ptr [ESP + 0x8],EDX // 009d81dc
        FLD dword ptr [ESP + 0x8] // 009d81e0
        FLD dword ptr [ECX + off_avoid_half_length] // 009d81e4
        FCOMIP st(0),st(1) // 009d81e7
        FSTP st(0) // 009d81e9
        JC L_009d81f9 // 009d81eb
        MOV AL,0x1 // 009d81ed
        ADD ESP,0x8 // 009d81ef
        pop edx // release the source point/spill slot
        RET // source fastcall has no stack argument // 009d81f2
L_009d81f5:
        FSTP st(0) // 009d81f5
        FSTP st(0) // 009d81f7
L_009d81f9:
        XOR AL,AL // 009d81f9
        ADD ESP,0x8 // 009d81fb
        pop edx // release the source point/spill slot
        RET // source fastcall has no stack argument // 009d81fe
    }
}
} // namespace

bool ship_ai_obstacle_point_in_near_box_009d80c0(
    const ShipAiObstacleNode& node, const std::array<float, 2>& point) noexcept {
    return near_kernel(&node, point.data());
}
bool ship_ai_obstacle_point_in_avoid_box_009d8160(
    const ShipAiObstacleNode& node, const std::array<float, 2>& point) noexcept {
    return avoid_kernel(&node, point.data());
}
} // namespace bsp
