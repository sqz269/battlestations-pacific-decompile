#include "bsp/native_spatial_attachment.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_traceline_render.hpp"
#include "bsp/native_unit_part_collision.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeSpatialAttachmentAccess, frame_owner_00e188a8) == 0);
static_assert(offsetof(NativeSpatialAttachmentAccess, half_00d7a280) == 4);
static_assert(offsetof(NativeSpatialAttachmentAccess, cell_width_00ce3d90) == 8);
static_assert(offsetof(NativeSpatialAttachmentAccess, floor_00bf85b0) == 12);
static_assert(offsetof(NativeSpatialAttachmentAccess, truncate_dispatch_0109eea4) == 16);
template<class T> T& field(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(object) + offset);
}
void* offset(void* object, std::size_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + bytes);
}
using Word = volatile std::uint32_t;
using Pointer = void* volatile;
using Byte = volatile std::uint8_t;

void __fastcall set_current_local_bounds(void* node,
    const NativeSpatialAttachmentAccess* access, const void* minimum, const void* maximum) {
    set_native_spatial_local_bounds_0098a920(node, minimum, maximum, *access->half_00d7a280);
}
} // namespace

void __fastcall attach_native_spatial_child_0098b920(void* parent,
    const NativeSpatialAttachmentAccess* access, void* child) {
    const std::uint32_t capacity = field<Word>(parent, 0x104);
    if (field<Word>(parent, 0x100) == capacity) {
        const std::uint32_t doubled = capacity + capacity;
        field<Word>(parent, 0x104) = doubled;
        const std::uint32_t bytes = doubled > 0x3fffffffu ? 0xffffffffu : doubled * 4u;
        void* const replacement = singleton_lifetime_allocate({
            SingletonAllocationKind::pointer_slots, bytes, bytes});
        void* const old_slots = field<Pointer>(parent, 0xfc);
        const std::uint32_t copy_bytes = field<Word>(parent, 0x100) * 4u;
        std::memcpy(replacement, old_slots, copy_bytes);
        singleton_lifetime_free(field<Pointer>(parent, 0xfc));
        field<Pointer>(parent, 0xfc) = replacement;
    }
    const std::uint32_t insertion = field<Word>(parent, 0x100);
    field<Pointer>(field<Pointer>(parent, 0xfc), insertion * 4u) = child;
    field<Word>(child, 0x15c) = field<Word>(parent, 0x100);
    field<Word>(parent, 0x100) = field<Word>(parent, 0x100) + 1u;
    field<Pointer>(child, 0x108) = parent;
    merge_native_spatial_child_bounds_0098b530(parent, access, child);
}

void __fastcall attach_native_spatial_node_0098ba10(void* index,
    const NativeSpatialAttachmentAccess* access, void* node, void* parent,
    const void*, std::uint32_t static_flag_word) {
    void* owner = field<Pointer>(node, 0x4c);
    field<Byte>(node, 8) = static_cast<std::uint8_t>(static_flag_word);
    if (!field<Byte>(owner, 0xc8)) refresh_pose_00414db0(access->poses->resolve_pose(owner));
    copy_native_camera_matrix_004134f0(offset(node, 0x50), nullptr, offset(owner, 0xcc));
    owner = field<Pointer>(node, 0x4c);
    if (!field<Byte>(owner, 0x10c)) {
        refresh_pose_00414db0(access->poses->resolve_pose(owner));
        field<Byte>(owner, 0x10c) = 1;
        auto& derived = access->poses->resolve_derived(owner);
        derive_pose_affine_inverse_00b63d50(derived.derived_110, derived.pose.world_cc);
    }
    copy_native_camera_matrix_004134f0(offset(node, 0x90), nullptr, offset(owner, 0x110));
    if (parent) {
        attach_native_spatial_child_0098b920(parent, access, node);
    } else {
        field<Pointer>(node, 0x108) = nullptr;
        rebuild_native_spatial_world_bounds_0098a750(node, access);
        std::uint32_t minimum[2], maximum[2];
        native_spatial_cell_of_point_0098ad60(minimum, offset(node, 0x13c), access);
        native_spatial_cell_of_point_0098ad60(maximum, offset(node, 0x148), access);
        if (static_cast<std::int32_t>(maximum[0] - minimum[0]) <= 1
            && static_cast<std::int32_t>(maximum[1] - minimum[1]) <= 1) {
            register_native_spatial_cells_0098a310(index, node, minimum, maximum);
            field<Pointer>(node, 0x44) = nullptr;
            field<Pointer>(node, 0x48) = field<Pointer>(index, 0x16014);
            void* const current_root = field<Pointer>(index, 0x16014);
            if (current_root) field<Pointer>(current_root, 0x44) = node;
            field<Pointer>(index, 0x16014) = node;
        } else {
            const std::uint32_t slot = field<Word>(index, 0x80);
            field<Pointer>(index, 8u + slot * 4u) = node;
            field<Word>(index, 0x80) = field<Word>(index, 0x80) + 1u;
        }
    }
    field<Byte>(node, 0x158) = 1;
}

// Complete numerical/intrusive kernels; source assembly, not image bytes.

__declspec(naked) void __fastcall accumulate_native_abs_scaled_00722c20(void*, void*, float, float, float, float) {
    __asm {
        movss xmm0, dword ptr [esp + 4] // 00722c20
        movss dword ptr [esp + 4], xmm0 // 00722c26
        mov eax, dword ptr [esp + 4] // 00722c2c
        movss xmm0, dword ptr [esp + 8] // 00722c30
        and eax, 0x7fffffff // 00722c36
        mov dword ptr [esp + 4], eax // 00722c3b
        fld dword ptr [esp + 4] // 00722c3f
        movss dword ptr [esp + 4], xmm0 // 00722c43
        fld dword ptr [esp + 0x10] // 00722c49
        mov edx, dword ptr [esp + 4] // 00722c4d
        movss xmm0, dword ptr [esp + 0xc] // 00722c51
        fld st(0) // 00722c57
        fmulp st(2),st(0) // 00722c59
        and edx, 0x7fffffff // 00722c5b
        fld dword ptr [ecx] // 00722c61
        mov dword ptr [esp + 4], edx // 00722c63
        faddp st(2),st(0) // 00722c67
        fxch st(1) // 00722c69
        fstp dword ptr [ecx] // 00722c6b
        fld dword ptr [esp + 4] // 00722c6d
        movss dword ptr [esp + 4], xmm0 // 00722c71
        mov eax, dword ptr [esp + 4] // 00722c77
        fmul st(0),st(1) // 00722c7b
        and eax, 0x7fffffff // 00722c7d
        mov dword ptr [esp + 4], eax // 00722c82
        fadd dword ptr [ecx + 4] // 00722c86
        fstp dword ptr [ecx + 4] // 00722c89
        fmul dword ptr [esp + 4] // 00722c8c
        fadd dword ptr [ecx + 8] // 00722c90
        fstp dword ptr [ecx + 8] // 00722c93
        ret 0x10 // 00722c96
    }
}

__declspec(naked) void __fastcall register_native_spatial_cells_0098a310(void*, void*, const void*, const void*) {
    __asm {
        sub esp, 0xc // 0098a310
        push ebx // 0098a313
        push ebp // 0098a314
        mov ebp, dword ptr [esp + 0x1c] // 0098a315
        push esi // 0098a319
        mov esi, dword ptr [esp + 0x1c] // 0098a31a
        mov dword ptr [esp + 0x10], ecx // 0098a31e
        mov ecx, dword ptr [esi] // 0098a322
        mov eax, edx // 0098a324
        xor ebx, ebx // 0098a326
        cmp ecx, dword ptr [ebp] // 0098a328
        mov dword ptr [esp + 0x14], eax // 0098a32b
        mov dword ptr [esp + 0xc], ecx // 0098a32f
        jg L_0098a3ab // 0098a333
        mov edx, ecx // 0098a335
        imul edx, edx, 0x96 // 0098a337
        push edi // 0098a33d
        mov edi, edi // 0098a33e
    L_0098a340:
        mov edi, dword ptr [esi + 4] // 0098a340
        cmp edi, dword ptr [ebp + 4] // 0098a343
        jg L_0098a398 // 0098a346
        mov esi, dword ptr [esp + 0x14] // 0098a348
        lea ecx, [ebx + ebx*2 + 3] // 0098a34c
        lea eax, [eax + ecx*4] // 0098a350
        lea ecx, [edx + edi] // 0098a353
        lea esi, [esi + ecx*4 + 0x84] // 0098a356
        lea ecx, [ecx] // 0098a35d
    L_0098a360:
        mov ecx, dword ptr [esi] // 0098a360
        mov dword ptr [eax + 4], ecx // 0098a362
        mov dword ptr [eax], 0 // 0098a365
        mov ecx, dword ptr [esi] // 0098a36b
        test ecx, ecx // 0098a36d
        je L_0098a373 // 0098a36f
        mov dword ptr [ecx], eax // 0098a371
    L_0098a373:
        mov dword ptr [esi], eax // 0098a373
        mov dword ptr [eax], 0 // 0098a375
        add edi, 1 // 0098a37b
        add ebx, 1 // 0098a37e
        add eax, 0xc // 0098a381
        add esi, 4 // 0098a384
        cmp edi, dword ptr [ebp + 4] // 0098a387
        jle L_0098a360 // 0098a38a
        mov eax, dword ptr [esp + 0x18] // 0098a38c
        mov esi, dword ptr [esp + 0x20] // 0098a390
        mov ecx, dword ptr [esp + 0x10] // 0098a394
    L_0098a398:
        add ecx, 1 // 0098a398
        add edx, 0x96 // 0098a39b
        cmp ecx, dword ptr [ebp] // 0098a3a1
        mov dword ptr [esp + 0x10], ecx // 0098a3a4
        jle L_0098a340 // 0098a3a8
        pop edi // 0098a3aa
    L_0098a3ab:
        mov dword ptr [eax + 0x40], ebx // 0098a3ab
        mov edx, dword ptr [ebp + 4] // 0098a3ae
        shl edx, 8 // 0098a3b1
        add edx, dword ptr [ebp] // 0098a3b4
        shl edx, 8 // 0098a3b7
        add edx, dword ptr [esi + 4] // 0098a3ba
        shl edx, 8 // 0098a3bd
        add edx, dword ptr [esi] // 0098a3c0
        pop esi // 0098a3c2
        pop ebp // 0098a3c3
        mov dword ptr [eax + 0x3c], edx // 0098a3c4
        pop ebx // 0098a3c7
        add esp, 0xc // 0098a3c8
        ret 8 // 0098a3cb
    }
}

__declspec(naked) void __fastcall rebuild_native_spatial_world_bounds_0098a750(void*, const NativeSpatialAttachmentAccess*) {
    __asm {
        mov eax,dword ptr [edx] // explicit borrowed access
        mov eax,dword ptr [eax] // 0098a750
        mov eax, dword ptr [eax + 0x648] // 0098a755
        sub esp, 0x18 // 0098a75b
        push esi // 0098a75e
        mov esi, ecx // 0098a75f
        cmp dword ptr [esi + 0x154], eax // 0098a761
        je L_0098a8d3 // 0098a767
        fld dword ptr [esi + 0x130] // 0098a76d
        xorps xmm0, xmm0 // 0098a773
        push edi // 0098a776
        sub esp, 0x10 // 0098a777
        fstp dword ptr [esp + 0xc] // 0098a77a
        mov dword ptr [esi + 0x154], eax // 0098a77e
        fld dword ptr [esi + 0x58] // 0098a784
        lea edi, [esi + 0x50] // 0098a787
        fstp dword ptr [esp + 8] // 0098a78a
        lea ecx, [esp + 0x18] // 0098a78e
        fld dword ptr [edi + 4] // 0098a792
        movss dword ptr [esp + 0x18], xmm0 // 0098a795
        fstp dword ptr [esp + 4] // 0098a79b
        movss dword ptr [esp + 0x1c], xmm0 // 0098a79f
        fld dword ptr [edi] // 0098a7a5
        movss dword ptr [esp + 0x20], xmm0 // 0098a7a7
        fstp dword ptr [esp] // 0098a7ad
        call accumulate_native_abs_scaled_00722c20 // 0098a7b0
        fld dword ptr [esi + 0x134] // 0098a7b5
        sub esp, 0x10 // 0098a7bb
        fstp dword ptr [esp + 0xc] // 0098a7be
        lea ecx, [esp + 0x18] // 0098a7c2
        fld dword ptr [edi + 0x18] // 0098a7c6
        fstp dword ptr [esp + 8] // 0098a7c9
        fld dword ptr [edi + 0x14] // 0098a7cd
        fstp dword ptr [esp + 4] // 0098a7d0
        fld dword ptr [edi + 0x10] // 0098a7d4
        fstp dword ptr [esp] // 0098a7d7
        call accumulate_native_abs_scaled_00722c20 // 0098a7da
        fld dword ptr [esi + 0x138] // 0098a7df
        sub esp, 0x10 // 0098a7e5
        fstp dword ptr [esp + 0xc] // 0098a7e8
        lea ecx, [esp + 0x18] // 0098a7ec
        fld dword ptr [edi + 0x28] // 0098a7f0
        fstp dword ptr [esp + 8] // 0098a7f3
        fld dword ptr [edi + 0x24] // 0098a7f7
        fstp dword ptr [esp + 4] // 0098a7fa
        fld dword ptr [edi + 0x20] // 0098a7fe
        fstp dword ptr [esp] // 0098a801
        call accumulate_native_abs_scaled_00722c20 // 0098a804
        push edi // 0098a809
        lea ecx, [esp + 0x18] // 0098a80a
        push ecx // 0098a80e
        lea ecx, [esi + 0x124] // 0098a80f
        call transform_native_point_004142e0 // 0098a815
        movss xmm0, dword ptr [esp + 0x14] // 0098a81a
        movss dword ptr [esi + 0x13c], xmm0 // 0098a820
        movss xmm0, dword ptr [esp + 0x18] // 0098a828
        movss dword ptr [esi + 0x140], xmm0 // 0098a82e
        movss xmm0, dword ptr [esp + 0x1c] // 0098a836
        movss dword ptr [esi + 0x144], xmm0 // 0098a83c
        fld dword ptr [esi + 0x13c] // 0098a844
        fstp dword ptr [esi + 0x148] // 0098a84a
        fld dword ptr [esi + 0x140] // 0098a850
        fstp dword ptr [esi + 0x14c] // 0098a856
        fld dword ptr [esi + 0x144] // 0098a85c
        fstp dword ptr [esi + 0x150] // 0098a862
        fld dword ptr [esi + 0x13c] // 0098a868
        fld dword ptr [esp + 8] // 0098a86e
        fld st(0) // 0098a872
        fsubp st(2),st(0) // 0098a874
        fxch st(1) // 0098a876
        fstp dword ptr [esi + 0x13c] // 0098a878
        fld dword ptr [esi + 0x140] // 0098a87e
        fld dword ptr [esp + 0xc] // 0098a884
        fld st(0) // 0098a888
        fsubp st(2),st(0) // 0098a88a
        fxch st(1) // 0098a88c
        fstp dword ptr [esi + 0x140] // 0098a88e
        fld dword ptr [esi + 0x144] // 0098a894
        fld dword ptr [esp + 0x10] // 0098a89a
        fld st(0) // 0098a89e
        fsubp st(2),st(0) // 0098a8a0
        fxch st(1) // 0098a8a2
        fstp dword ptr [esi + 0x144] // 0098a8a4
        fld dword ptr [esi + 0x148] // 0098a8aa
        faddp st(3),st(0) // 0098a8b0
        fxch st(2) // 0098a8b2
        fstp dword ptr [esi + 0x148] // 0098a8b4
        fadd dword ptr [esi + 0x14c] // 0098a8ba
        fstp dword ptr [esi + 0x14c] // 0098a8c0
        fadd dword ptr [esi + 0x150] // 0098a8c6
        pop edi // 0098a8cc
        fstp dword ptr [esi + 0x150] // 0098a8cd
    L_0098a8d3:
        pop esi // 0098a8d3
        add esp, 0x18 // 0098a8d4
        ret  // 0098a8d7
    }
}

__declspec(naked) void __fastcall merge_native_spatial_child_bounds_0098b530(void*, const NativeSpatialAttachmentAccess*, void*) {
    __asm {
        mov eax, dword ptr [esp + 4] // 0098b530
        push edx // preserve borrowed access below original locals
        sub esp, 0x74 // 0098b534
        push ebx // 0098b537
        push ebp // 0098b538
        push esi // 0098b539
        push edi // 0098b53a
        mov edi, ecx // 0098b53b
        lea ecx, [ecx] // 0098b53d
    L_0098b540:
        movss xmm0, dword ptr [eax + 0x10c] // 0098b540
        movss dword ptr [esp + 0x2c], xmm0 // 0098b548
        movss xmm0, dword ptr [eax + 0x110] // 0098b54e
        movss dword ptr [esp + 0x30], xmm0 // 0098b556
        movss xmm0, dword ptr [eax + 0x114] // 0098b55c
        movss dword ptr [esp + 0x34], xmm0 // 0098b564
        movss xmm0, dword ptr [eax + 0x118] // 0098b56a
        movss dword ptr [esp + 0x38], xmm0 // 0098b572
        movss xmm0, dword ptr [eax + 0x11c] // 0098b578
        movss dword ptr [esp + 0x3c], xmm0 // 0098b580
        movss xmm0, dword ptr [eax + 0x120] // 0098b586
        xor esi, esi // 0098b58e
        add eax, 0x50 // 0098b590
        movss dword ptr [esp + 0x40], xmm0 // 0098b593
        lea ebp, [edi + 0x90] // 0098b599
        mov dword ptr [esp + 0x10], eax // 0098b59f
        lea ebx, [edi + 0x10c] // 0098b5a3
        lea esp, [esp] // 0098b5a9
    L_0098b5b0:
        mov eax, esi // 0098b5b0
        and eax, 1 // 0098b5b2
        lea eax, [eax + eax*2] // 0098b5b5
        movss xmm0, dword ptr [esp + eax*4 + 0x2c] // 0098b5b8
        mov eax, esi // 0098b5be
        sar eax, 1 // 0098b5c0
        and eax, 1 // 0098b5c2
        lea ecx, [eax + eax*2] // 0098b5c5
        mov eax, esi // 0098b5c8
        sar eax, 2 // 0098b5ca
        and eax, 1 // 0098b5cd
        movss dword ptr [esp + 0x14], xmm0 // 0098b5d0
        movss xmm0, dword ptr [esp + ecx*4 + 0x30] // 0098b5d6
        mov ecx, dword ptr [esp + 0x10] // 0098b5dc
        lea edx, [eax + eax*2] // 0098b5e0
        push ebp // 0098b5e3
        lea eax, [esp + 0x48] // 0098b5e4
        movss dword ptr [esp + 0x1c], xmm0 // 0098b5e8
        movss xmm0, dword ptr [esp + edx*4 + 0x38] // 0098b5ee
        push eax // 0098b5f4
        movss dword ptr [esp + 0x24], xmm0 // 0098b5f5
        call multiply_native_camera_matrices_00413920 // 0098b5fb
        lea ecx, [esp + 0x44] // 0098b600
        push ecx // 0098b604
        lea edx, [esp + 0x24] // 0098b605
        push edx // 0098b609
        lea ecx, [esp + 0x1c] // 0098b60a
        call transform_native_point_004142e0 // 0098b60e
        fld dword ptr [esp + 0x20] // 0098b613
        fld dword ptr [ebx] // 0098b617
        movss xmm0, dword ptr [esp + 0x20] // 0098b619
        fcomip st(0),st(1) // 0098b61f
        movss xmm1, dword ptr [esp + 0x24] // 0098b621
        movss xmm2, dword ptr [esp + 0x28] // 0098b627
        movss dword ptr [esp + 0x14], xmm0 // 0098b62d
        movss dword ptr [esp + 0x18], xmm1 // 0098b633
        movss dword ptr [esp + 0x1c], xmm2 // 0098b639
        jbe L_0098b649 // 0098b63f
        fstp st(0) // 0098b641
        movss dword ptr [ebx], xmm0 // 0098b643
        jmp L_0098b65f // 0098b647
    L_0098b649:
        fld dword ptr [edi + 0x118] // 0098b649
        fxch st(1) // 0098b64f
        fcomip st(0),st(1) // 0098b651
        fstp st(0) // 0098b653
        jbe L_0098b65f // 0098b655
        movss dword ptr [edi + 0x118], xmm0 // 0098b657
    L_0098b65f:
        fld dword ptr [esp + 0x24] // 0098b65f
        fld dword ptr [edi + 0x110] // 0098b663
        fcomip st(0),st(1) // 0098b669
        jbe L_0098b679 // 0098b66b
        fstp st(0) // 0098b66d
        movss dword ptr [edi + 0x110], xmm1 // 0098b66f
        jmp L_0098b68f // 0098b677
    L_0098b679:
        fld dword ptr [edi + 0x11c] // 0098b679
        fxch st(1) // 0098b67f
        fcomip st(0),st(1) // 0098b681
        fstp st(0) // 0098b683
        jbe L_0098b68f // 0098b685
        movss dword ptr [edi + 0x11c], xmm1 // 0098b687
    L_0098b68f:
        fld dword ptr [esp + 0x28] // 0098b68f
        fld dword ptr [edi + 0x114] // 0098b693
        fcomip st(0),st(1) // 0098b699
        jbe L_0098b6a9 // 0098b69b
        fstp st(0) // 0098b69d
        movss dword ptr [edi + 0x114], xmm2 // 0098b69f
        jmp L_0098b6bf // 0098b6a7
    L_0098b6a9:
        fld dword ptr [edi + 0x120] // 0098b6a9
        fxch st(1) // 0098b6af
        fcomip st(0),st(1) // 0098b6b1
        fstp st(0) // 0098b6b3
        jbe L_0098b6bf // 0098b6b5
        movss dword ptr [edi + 0x120], xmm2 // 0098b6b7
    L_0098b6bf:
        add esi, 1 // 0098b6bf
        cmp esi, 8 // 0098b6c2
        jl L_0098b5b0 // 0098b6c5
        lea eax, [edi + 0x118] // 0098b6cb
        push eax // 0098b6d1
        push ebx // 0098b6d2
        mov ecx, edi // 0098b6d3
        mov edx,dword ptr [esp+8Ch] // explicit borrowed access
        call set_current_local_bounds // 0098b6d5
        mov ecx, dword ptr [edi + 0x108] // 0098b6da
        test ecx, ecx // 0098b6e0
        je L_0098b6ed // 0098b6e2
        mov eax, edi // 0098b6e4
        mov edi, ecx // 0098b6e6
        jmp L_0098b540 // 0098b6e8
    L_0098b6ed:
        pop edi // 0098b6ed
        pop esi // 0098b6ee
        pop ebp // 0098b6ef
        pop ebx // 0098b6f0
        add esp, 0x74 // 0098b6f1
        pop edx // explicit borrowed access
        ret 4 // 0098b6f4
    }
}

__declspec(naked) void __fastcall native_spatial_cell_of_point_0098ad60(void*, const void*, const NativeSpatialAttachmentAccess*) {
    __asm {
        push ecx // 0098ad60
        push esi // 0098ad61
        mov esi, edx // 0098ad62
        fld dword ptr [esi] // 0098ad64
        push edi // 0098ad66
        mov eax,dword ptr [esp+10h] // explicit borrowed access
        mov eax,dword ptr [eax+8] // explicit borrowed access
        fdiv qword ptr [eax] // 0098ad67
        sub esp, 8 // 0098ad6d
        mov edi, ecx // 0098ad70
        fstp dword ptr [esp + 0x10] // 0098ad72
        fld dword ptr [esp + 0x10] // 0098ad76
        fstp qword ptr [esp] // 0098ad7a
        mov eax,dword ptr [esp+18h] // explicit borrowed access
        call dword ptr [eax+0Ch] // 0098ad7d
        fstp dword ptr [esp + 0x10] // 0098ad82
        fld dword ptr [esp + 0x10] // 0098ad86
        mov ecx,dword ptr [esp+18h] // explicit borrowed access
        mov ecx,dword ptr [ecx+10h] // explicit borrowed access
        call native_crt_truncate_st0_00bf7420 // 0098ad8a
        add eax, 0x4b // 0098ad8f
        mov dword ptr [edi], eax // 0098ad92
        fld dword ptr [esi + 8] // 0098ad94
        mov eax,dword ptr [esp+18h] // explicit borrowed access
        mov eax,dword ptr [eax+8] // explicit borrowed access
        fdiv qword ptr [eax] // 0098ad97
        fstp dword ptr [esp + 0x10] // 0098ad9d
        fld dword ptr [esp + 0x10] // 0098ada1
        fstp qword ptr [esp] // 0098ada5
        mov eax,dword ptr [esp+18h] // explicit borrowed access
        call dword ptr [eax+0Ch] // 0098ada8
        fstp dword ptr [esp + 0x10] // 0098adad
        fld dword ptr [esp + 0x10] // 0098adb1
        add esp, 8 // 0098adb5
        mov ecx,dword ptr [esp+10h] // explicit borrowed access
        mov ecx,dword ptr [ecx+10h] // explicit borrowed access
        call native_crt_truncate_st0_00bf7420 // 0098adb8
        add eax, 0x4b // 0098adbd
        mov dword ptr [edi + 4], eax // 0098adc0
        pop edi // 0098adc3
        pop esi // 0098adc4
        pop ecx // 0098adc5
        ret 4 // 0098adc6
    }
}

} // namespace bsp
