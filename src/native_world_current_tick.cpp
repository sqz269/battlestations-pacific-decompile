#include "bsp/native_world_current_tick.hpp"
#include "bsp/native_particle_axial_loading.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_subtree_invalidation.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

namespace bsp {
namespace {

// This is our own local pointer argument block, not an overlay of Native
// World, entity, record, global or reference-member storage. Public wrappers
// obtain these addresses through the actual C++ reference members.
struct TickBindings {
    const volatile float* clock;
    const volatile float* negative_zero;
    const volatile float* one;
    const SingletonLifetimeCallbacks* validation;
};
static_assert(sizeof(void*) == 4 && sizeof(float) == 4);
static_assert(std::is_standard_layout_v<TickBindings>);
static_assert(sizeof(TickBindings) == 0x10);
static_assert(offsetof(TickBindings, clock) == 0);
static_assert(offsetof(TickBindings, negative_zero) == 4);
static_assert(offsetof(TickBindings, one) == 8);
static_assert(offsetof(TickBindings, validation) == 0x0c);
static_assert(sizeof(CameraMatrix) == 0x40 && alignof(CameraMatrix) == 4);
static_assert(std::is_trivially_default_constructible_v<CameraMatrix>);
static_assert(std::is_trivially_destructible_v<CameraMatrix>);

// The caller supplies a genuine live service. Do not turn returning Native
// validation sites into an unconditional throw, null no-op or recovery path.
__declspec(noinline) void __cdecl invoke_tick_invalid_parameter(
    const SingletonLifetimeCallbacks* validation) {
    validation->invalid_parameter(validation->context);
}

// The existing current-operand Z overload requires a real CameraMatrix and
// returns void. Begin a trivial object in this private aligned stack scratch
// without allocating or initializing its float values, then return the known
// destination. Its only ordered16-DWORD result copy remains in the main body.
__declspec(noinline) void* __fastcall build_tick_rotation_z(
    void* destination, const float* angle,
    const volatile float* negative_zero, const volatile float* one) {
    auto* const matrix = ::new (destination) CameraMatrix;
    build_gui_rotation_z_00b64780(*matrix, *angle, negative_zero, one);
    return destination;
}

// Complete Native00904600 schedule with explicit operand/service bindings.
// ECX=actual World, EDX=our local bindings, one unused DWORD stack argument.
// One Source-only PUSH retains bindings above the original2E8 scratch frame;
// after the four original register saves it is at ESP+2F8. Every original
// scratch offset is unchanged. All temporary extra arguments are removed by
// their callee or explicitly before returning to that frame baseline.
// Numeric Native comments identify all323 original instruction sites.
__declspec(naked) void __fastcall run_world_matrix_body(
    void*, const TickBindings*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [ecx + 0x4b4] // 00904600
        push edx // Source-only bindings spill, outside Native scratch.
        sub esp, 0x2e8 // 00904606
        push ebx // 0090460C
        push ebp // 0090460D
        push esi // 0090460E
        mov esi, dword ptr [eax] // 0090460F
        push edi // 00904611
        lea edi, [ecx + 0x4b0] // 00904612
    matrix_00904618:
        cmp edi, edi // 00904618
        mov ebx, dword ptr [edi + 4] // 0090461A
        je matrix_00904624 // 0090461D
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 0090461F
        add esp, 4
    matrix_00904624:
        cmp esi, ebx // 00904624
        je matrix_00904bd6 // 00904626
        cmp esi, dword ptr [edi + 4] // 0090462C
        mov eax, dword ptr [esp + 0x2f8]
        mov eax, dword ptr [eax + 0x0]
        movss xmm0, dword ptr [eax] // 0090462F
        movss dword ptr [esp + 0x1c], xmm0 // 00904637
        jne matrix_00904644 // 0090463D
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 0090463F
        add esp, 4
    matrix_00904644:
        cmp esi, dword ptr [edi + 4] // 00904644
        fld dword ptr [esp + 0x1c] // 00904647
        fsub dword ptr [esi + 0x68] // 0090464B
        fstp dword ptr [esp + 0x18] // 0090464E
        jne matrix_00904659 // 00904652
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904654
        add esp, 4
    matrix_00904659:
        mov ecx, dword ptr [esi + 8] // 00904659
        cmp byte ptr [ecx + 0x5c], 0 // 0090465C
        je matrix_00904b2c // 00904660
        cmp esi, dword ptr [edi + 4] // 00904666
        jne matrix_00904670 // 00904669
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 0090466B
        add esp, 4
    matrix_00904670:
        fld dword ptr [esp + 0x18] // 00904670
        fdiv dword ptr [esi + 0x24] // 00904674
        fstp dword ptr [esp + 0x10] // 00904677
        fld dword ptr [esp + 0x10] // 0090467B
        fldz // 0090467F
        fcomip st(0), st(1) // 00904681
        fstp st(0) // 00904683
        jbe matrix_00904b67 // 00904685
        xorps xmm0, xmm0 // 0090468B
    matrix_0090468e:
        movss dword ptr [esp + 0x14], xmm0 // 0090468E
    matrix_00904694:
        cmp esi, dword ptr [edi + 4] // 00904694
        jne matrix_0090469e // 00904697
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904699
        add esp, 4
    matrix_0090469e:
        cmp esi, dword ptr [edi + 4] // 0090469E
        mov ebx, dword ptr [esi + 8] // 009046A1
        jne matrix_009046b5 // 009046A4
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 009046A6
        add esp, 4
        cmp esi, dword ptr [edi + 4] // 009046AB
        jne matrix_009046b5 // 009046AE
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 009046B0
        add esp, 4
    matrix_009046b5:
        cmp esi, dword ptr [edi + 4] // 009046B5
        fld dword ptr [esi + 0xc] // 009046B8
        fld dword ptr [esp + 0x14] // 009046BB
        xorps xmm0, xmm0 // 009046BF
        mov eax, dword ptr [esp + 0x2f8]
        mov eax, dword ptr [eax + 0x8]
        movss xmm1, dword ptr [eax] // 009046C2
        fld st(0) // 009046CA
        fmulp st(2), st(0) // 009046CC
        movss dword ptr [esp + 0xbc], xmm0 // 009046CE
        fxch st(1) // 009046D7
        movss dword ptr [esp + 0xc0], xmm0 // 009046D9
        movss dword ptr [esp + 0xc4], xmm0 // 009046E2
        fstp dword ptr [esp + 0x2c] // 009046EB
        movss dword ptr [esp + 0xc8], xmm0 // 009046EF
        fld dword ptr [esi + 0x10] // 009046F8
        movss dword ptr [esp + 0xd0], xmm0 // 009046FB
        fmul st(0), st(1) // 00904704
        movss dword ptr [esp + 0xd4], xmm0 // 00904706
        movss dword ptr [esp + 0xd8], xmm0 // 0090470F
        movss dword ptr [esp + 0xdc], xmm0 // 00904718
        fstp dword ptr [esp + 0x30] // 00904721
        movss dword ptr [esp + 0xe4], xmm0 // 00904725
        fld dword ptr [esi + 0x14] // 0090472E
        movss xmm0, dword ptr [esp + 0x2c] // 00904731
        fmul st(0), st(1) // 00904737
        movss dword ptr [esp + 0xe8], xmm0 // 00904739
        movss xmm0, dword ptr [esp + 0x30] // 00904742
        movss dword ptr [esp + 0xec], xmm0 // 00904748
        fstp dword ptr [esp + 0x34] // 00904751
        movss dword ptr [esp + 0xb8], xmm1 // 00904755
        movss xmm0, dword ptr [esp + 0x34] // 0090475E
        movss dword ptr [esp + 0xcc], xmm1 // 00904764
        movss dword ptr [esp + 0xe0], xmm1 // 0090476D
        movss dword ptr [esp + 0xf0], xmm0 // 00904776
        movss dword ptr [esp + 0xf4], xmm1 // 0090477F
        jne matrix_00904795 // 00904788
        fstp st(0) // 0090478A
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 0090478C
        add esp, 4
        fld dword ptr [esp + 0x14] // 00904791
    matrix_00904795:
        fmul dword ptr [esi + 0x18] // 00904795
        lea edx, [esp + 0x28] // 00904798
        lea ecx, [esp + 0x278] // 0090479C
        fstp dword ptr [esp + 0x10] // 009047A3
        fld dword ptr [esp + 0x10] // 009047A7
        fchs // 009047AB
        fstp dword ptr [esp + 0x28] // 009047AD
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 8] // Current one address.
        push dword ptr [eax + 4] // Current negative-zero address.
        call build_native_particle_rotation_x_00b64640 // 009047B1
        cmp esi, dword ptr [edi + 4] // 009047B6
        movss xmm0, dword ptr [eax] // 009047B9
        movss dword ptr [esp + 0x38], xmm0 // 009047BD
        movss xmm0, dword ptr [eax + 4] // 009047C3
        movss dword ptr [esp + 0x3c], xmm0 // 009047C8
        movss xmm0, dword ptr [eax + 8] // 009047CE
        movss dword ptr [esp + 0x40], xmm0 // 009047D3
        movss xmm0, dword ptr [eax + 0xc] // 009047D9
        movss dword ptr [esp + 0x44], xmm0 // 009047DE
        movss xmm0, dword ptr [eax + 0x10] // 009047E4
        movss dword ptr [esp + 0x48], xmm0 // 009047E9
        movss xmm0, dword ptr [eax + 0x14] // 009047EF
        movss dword ptr [esp + 0x4c], xmm0 // 009047F4
        movss xmm0, dword ptr [eax + 0x18] // 009047FA
        movss dword ptr [esp + 0x50], xmm0 // 009047FF
        movss xmm0, dword ptr [eax + 0x1c] // 00904805
        movss dword ptr [esp + 0x54], xmm0 // 0090480A
        movss xmm0, dword ptr [eax + 0x20] // 00904810
        movss dword ptr [esp + 0x58], xmm0 // 00904815
        movss xmm0, dword ptr [eax + 0x24] // 0090481B
        movss dword ptr [esp + 0x5c], xmm0 // 00904820
        movss xmm0, dword ptr [eax + 0x28] // 00904826
        movss dword ptr [esp + 0x60], xmm0 // 0090482B
        movss xmm0, dword ptr [eax + 0x2c] // 00904831
        movss dword ptr [esp + 0x64], xmm0 // 00904836
        movss xmm0, dword ptr [eax + 0x30] // 0090483C
        movss dword ptr [esp + 0x68], xmm0 // 00904841
        movss xmm0, dword ptr [eax + 0x34] // 00904847
        movss dword ptr [esp + 0x6c], xmm0 // 0090484C
        movss xmm0, dword ptr [eax + 0x38] // 00904852
        movss dword ptr [esp + 0x70], xmm0 // 00904857
        movss xmm0, dword ptr [eax + 0x3c] // 0090485D
        movss dword ptr [esp + 0x74], xmm0 // 00904862
        jne matrix_0090486f // 00904868
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 0090486A
        add esp, 4
    matrix_0090486f:
        fld dword ptr [esi + 0x1c] // 0090486F
        lea edx, [esp + 0x20] // 00904872
        fmul dword ptr [esp + 0x14] // 00904876
        lea ecx, [esp + 0x138] // 0090487A
        fstp dword ptr [esp + 0x10] // 00904881
        fld dword ptr [esp + 0x10] // 00904885
        fchs // 00904889
        fstp dword ptr [esp + 0x20] // 0090488B
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 8] // Current one address.
        push dword ptr [eax + 4] // Current negative-zero address.
        call build_native_particle_rotation_y_00b646e0 // 0090488F
        cmp esi, dword ptr [edi + 4] // 00904894
        movss xmm0, dword ptr [eax] // 00904897
        movss dword ptr [esp + 0x78], xmm0 // 0090489B
        movss xmm0, dword ptr [eax + 4] // 009048A1
        movss dword ptr [esp + 0x7c], xmm0 // 009048A6
        movss xmm0, dword ptr [eax + 8] // 009048AC
        movss dword ptr [esp + 0x80], xmm0 // 009048B1
        movss xmm0, dword ptr [eax + 0xc] // 009048BA
        movss dword ptr [esp + 0x84], xmm0 // 009048BF
        movss xmm0, dword ptr [eax + 0x10] // 009048C8
        movss dword ptr [esp + 0x88], xmm0 // 009048CD
        movss xmm0, dword ptr [eax + 0x14] // 009048D6
        movss dword ptr [esp + 0x8c], xmm0 // 009048DB
        movss xmm0, dword ptr [eax + 0x18] // 009048E4
        movss dword ptr [esp + 0x90], xmm0 // 009048E9
        movss xmm0, dword ptr [eax + 0x1c] // 009048F2
        movss dword ptr [esp + 0x94], xmm0 // 009048F7
        movss xmm0, dword ptr [eax + 0x20] // 00904900
        movss dword ptr [esp + 0x98], xmm0 // 00904905
        movss xmm0, dword ptr [eax + 0x24] // 0090490E
        movss dword ptr [esp + 0x9c], xmm0 // 00904913
        movss xmm0, dword ptr [eax + 0x28] // 0090491C
        movss dword ptr [esp + 0xa0], xmm0 // 00904921
        movss xmm0, dword ptr [eax + 0x2c] // 0090492A
        movss dword ptr [esp + 0xa4], xmm0 // 0090492F
        movss xmm0, dword ptr [eax + 0x30] // 00904938
        movss dword ptr [esp + 0xa8], xmm0 // 0090493D
        movss xmm0, dword ptr [eax + 0x34] // 00904946
        movss dword ptr [esp + 0xac], xmm0 // 0090494B
        movss xmm0, dword ptr [eax + 0x38] // 00904954
        movss dword ptr [esp + 0xb0], xmm0 // 00904959
        movss xmm0, dword ptr [eax + 0x3c] // 00904962
        movss dword ptr [esp + 0xb4], xmm0 // 00904967
        jne matrix_00904977 // 00904970
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904972
        add esp, 4
    matrix_00904977:
        fld dword ptr [esi + 0x20] // 00904977
        lea edx, [esp + 0x24] // 0090497A
        fmul dword ptr [esp + 0x14] // 0090497E
        lea ecx, [esp + 0x1f8] // 00904982
        fstp dword ptr [esp + 0x10] // 00904989
        fld dword ptr [esp + 0x10] // 0090498D
        fchs // 00904991
        fstp dword ptr [esp + 0x24] // 00904993
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 8] // Current one address.
        push dword ptr [eax + 4] // Current negative-zero address.
        call build_tick_rotation_z // 00904997
        movss xmm0, dword ptr [eax] // 0090499C
        movss dword ptr [esp + 0xf8], xmm0 // 009049A0
        movss xmm0, dword ptr [eax + 4] // 009049A9
        movss dword ptr [esp + 0xfc], xmm0 // 009049AE
        movss xmm0, dword ptr [eax + 8] // 009049B7
        movss dword ptr [esp + 0x100], xmm0 // 009049BC
        movss xmm0, dword ptr [eax + 0xc] // 009049C5
        movss dword ptr [esp + 0x104], xmm0 // 009049CA
        movss xmm0, dword ptr [eax + 0x10] // 009049D3
        movss dword ptr [esp + 0x108], xmm0 // 009049D8
        movss xmm0, dword ptr [eax + 0x14] // 009049E1
        movss dword ptr [esp + 0x10c], xmm0 // 009049E6
        movss xmm0, dword ptr [eax + 0x18] // 009049EF
        movss dword ptr [esp + 0x110], xmm0 // 009049F4
        movss xmm0, dword ptr [eax + 0x1c] // 009049FD
        movss dword ptr [esp + 0x114], xmm0 // 00904A02
        movss xmm0, dword ptr [eax + 0x20] // 00904A0B
        movss dword ptr [esp + 0x118], xmm0 // 00904A10
        movss xmm0, dword ptr [eax + 0x24] // 00904A19
        mov ebp, dword ptr [ebx] // 00904A1E
        movss dword ptr [esp + 0x11c], xmm0 // 00904A20
        movss xmm0, dword ptr [eax + 0x28] // 00904A29
        movss dword ptr [esp + 0x120], xmm0 // 00904A2E
        movss xmm0, dword ptr [eax + 0x2c] // 00904A37
        movss dword ptr [esp + 0x124], xmm0 // 00904A3C
        movss xmm0, dword ptr [eax + 0x30] // 00904A45
        movss dword ptr [esp + 0x128], xmm0 // 00904A4A
        movss xmm0, dword ptr [eax + 0x34] // 00904A53
        movss dword ptr [esp + 0x12c], xmm0 // 00904A58
        movss xmm0, dword ptr [eax + 0x38] // 00904A61
        movss dword ptr [esp + 0x130], xmm0 // 00904A66
        movss xmm0, dword ptr [eax + 0x3c] // 00904A6F
        lea edx, [esi + 0x28] // 00904A74
        push edx // 00904A77
        lea eax, [esp + 0x17c] // 00904A78
        push eax // 00904A7F
        lea ecx, [esp + 0xc0] // 00904A80
        push ecx // 00904A87
        lea edx, [esp + 0x1c4] // 00904A88
        push edx // 00904A8F
        lea eax, [esp + 0x48] // 00904A90
        push eax // 00904A94
        lea ecx, [esp + 0x24c] // 00904A95
        push ecx // 00904A9C
        lea edx, [esp + 0x90] // 00904A9D
        push edx // 00904AA4
        lea eax, [esp + 0x2d4] // 00904AA5
        push eax // 00904AAC
        lea ecx, [esp + 0x118] // 00904AAD
        movss dword ptr [esp + 0x154], xmm0 // 00904AB4
        add ebp, 0x88 // 00904ABD
        call multiply_native_camera_matrices_00413920 // 00904AC3
        mov ecx, eax // 00904AC8
        call multiply_native_camera_matrices_00413920 // 00904ACA
        mov ecx, eax // 00904ACF
        call multiply_native_camera_matrices_00413920 // 00904AD1
        mov ecx, eax // 00904AD6
        call multiply_native_camera_matrices_00413920 // 00904AD8
        mov edx, dword ptr [ebp] // 00904ADD
        push eax // 00904AE0
        mov ecx, ebx // 00904AE1
        call edx // 00904AE3
        cmp esi, dword ptr [edi + 4] // 00904AE5
        jne matrix_00904aef // 00904AE8
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904AEA
        add esp, 4
    matrix_00904aef:
        mov eax, dword ptr [esi + 8] // 00904AEF
        mov ebp, dword ptr [eax + 0x48] // 00904AF2
        test ebp, ebp // 00904AF5
        mov byte ptr [eax + 0xc8], 0 // 00904AF7
        mov byte ptr [eax + 0x10c], 0 // 00904AFE
        je matrix_00904b15 // 00904B05
    matrix_00904b07:
        mov ecx, ebp // 00904B07
        call invalidate_native_subtree_pose_0042ed50 // 00904B09
        mov ebp, dword ptr [ebp + 0x44] // 00904B0E
        test ebp, ebp // 00904B11
        jne matrix_00904b07 // 00904B13
    matrix_00904b15:
        cmp esi, dword ptr [edi + 4] // 00904B15
        jne matrix_00904b1f // 00904B18
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B1A
        add esp, 4
    matrix_00904b1f:
        mov ecx, dword ptr [esi + 8] // 00904B1F
        mov eax, dword ptr [ecx] // 00904B22
        mov edx, dword ptr [eax + 0xd8] // 00904B24
        call edx // 00904B2A
    matrix_00904b2c:
        cmp esi, dword ptr [edi + 4] // 00904B2C
        jne matrix_00904b36 // 00904B2F
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B31
        add esp, 4
    matrix_00904b36:
        mov eax, dword ptr [esi + 8] // 00904B36
        cmp byte ptr [eax + 0x5c], 0 // 00904B39
        je matrix_00904b89 // 00904B3D
        cmp esi, dword ptr [edi + 4] // 00904B3F
        jne matrix_00904b49 // 00904B42
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B44
        add esp, 4
    matrix_00904b49:
        fld dword ptr [esi + 0x24] // 00904B49
        fld dword ptr [esp + 0x18] // 00904B4C
        fcomip st(0), st(1) // 00904B50
        fstp st(0) // 00904B52
        ja matrix_00904b89 // 00904B54
        cmp esi, dword ptr [edi + 4] // 00904B56
        jne matrix_00904b60 // 00904B59
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B5B
        add esp, 4
    matrix_00904b60:
        mov esi, dword ptr [esi] // 00904B60
        jmp matrix_00904618 // 00904B62
    matrix_00904b67:
        movss xmm0, dword ptr [esp + 0x10] // 00904B67
        mov eax, dword ptr [esp + 0x2f8]
        mov eax, dword ptr [eax + 0x8]
        movss xmm1, dword ptr [eax] // 00904B6D
        comiss xmm0, xmm1 // 00904B75
        jbe matrix_0090468e // 00904B78
        movss dword ptr [esp + 0x14], xmm1 // 00904B7E
        jmp matrix_00904694 // 00904B84
    matrix_00904b89:
        cmp esi, dword ptr [edi + 4] // 00904B89
        mov ebp, esi // 00904B8C
        jne matrix_00904b95 // 00904B8E
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B90
        add esp, 4
    matrix_00904b95:
        test edi, edi // 00904B95
        mov esi, dword ptr [esi] // 00904B97
        jne matrix_00904ba0 // 00904B99
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904B9B
        add esp, 4
    matrix_00904ba0:
        cmp ebp, dword ptr [edi + 4] // 00904BA0
        jne matrix_00904bb3 // 00904BA3
        mov eax, dword ptr [esp + 0x2f8]
        push dword ptr [eax + 0x0c]
        call invoke_tick_invalid_parameter // 00904BA5
        add esp, 4
        cmp ebp, dword ptr [edi + 4] // 00904BAA
        je matrix_00904618 // 00904BAD
    matrix_00904bb3:
        mov ecx, dword ptr [ebp + 4] // 00904BB3
        mov edx, dword ptr [ebp] // 00904BB6
        mov dword ptr [ecx], edx // 00904BB9
        mov eax, dword ptr [ebp] // 00904BBB
        mov ecx, dword ptr [ebp + 4] // 00904BBE
        push ebp // 00904BC1
        mov dword ptr [eax + 4], ecx // 00904BC2
        call singleton_lifetime_free // 00904BC5
        add esp, 4 // 00904BCA
        add dword ptr [edi + 8], -1 // 00904BCD
        jmp matrix_00904618 // 00904BD1
    matrix_00904bd6:
        pop edi // 00904BD6
        pop esi // 00904BD7
        pop ebp // 00904BD8
        pop ebx // 00904BD9
        add esp, 0x2e8 // 00904BDA
        add esp, 4 // Discard Source-only bindings spill.
        ret 4 // 00904BE0
    }
}

// Complete Native00904BF0 walk. EBX additionally retains bindings; Native
// delta therefore lives at ESP+10 instead of ESP+0C after our saves.
// Incoming uint32 bits avoid an extra C++ floating evaluation before
// the explicit Native FLD/FSTP argument paths.
__declspec(naked) void __fastcall run_world_entity_body(
    void*, const TickBindings*, std::uint32_t) {
    __asm {
        push ebx
        mov ebx, edx
        push esi // 00904BF0
        push edi // 00904BF1
        mov edi, ecx // 00904BF2
        mov eax, dword ptr [edi + 4] // 00904BF4
        mov esi, dword ptr [eax] // 00904BF7
        test esi, esi // 00904BF9
        je entities_00904c21 // 00904BFB
        lea ecx, [ecx] // 00904BFD
    entities_00904c00:
        cmp byte ptr [esi + 0x5c], 0 // 00904C00
        je entities_00904c1a // 00904C04
        mov edx, dword ptr [esi] // 00904C06
        fld dword ptr [esp + 0x10] // 00904C08
        mov eax, dword ptr [edx + 0xdc] // 00904C0C
        push ecx // 00904C12
        mov ecx, esi // 00904C13
        fstp dword ptr [esp] // 00904C15
        call eax // 00904C18
    entities_00904c1a:
        mov esi, dword ptr [esi + 0x38] // 00904C1A
        test esi, esi // 00904C1D
        jne entities_00904c00 // 00904C1F
    entities_00904c21:
        fld dword ptr [esp + 0x10] // 00904C21
        push ecx // 00904C25
        mov ecx, edi // 00904C26
        fstp dword ptr [esp] // 00904C28
        mov edx, ebx // Same borrowed bindings for the concrete tail.
        call run_world_matrix_body // 00904C2B
        pop edi // 00904C30
        pop esi // 00904C31
        pop ebx
        ret 4 // 00904C32
    }
}

} // namespace

void update_native_world_entities_00904bf0(
    void* actual_world, float delta, const NativeWorldCurrentTickContext& context) {
    const TickBindings bindings{&context.mission_clock_00f876a4,
        &context.negative_zero_00d7a208, &context.one_00d7a24c, &context.validation};
    std::uint32_t delta_bits;
    std::memcpy(&delta_bits, &delta, sizeof(delta_bits));
    run_world_entity_body(actual_world, &bindings, delta_bits);
}

void run_native_world_matrix_pass_00904600(
    void* actual_world, float unused_delta, const NativeWorldCurrentTickContext& context) {
    const TickBindings bindings{&context.mission_clock_00f876a4,
        &context.negative_zero_00d7a208, &context.one_00d7a24c, &context.validation};
    std::uint32_t delta_bits;
    std::memcpy(&delta_bits, &unused_delta, sizeof(delta_bits));
    run_world_matrix_body(actual_world, &bindings, delta_bits);
}

} // namespace bsp
