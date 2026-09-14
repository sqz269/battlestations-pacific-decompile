#include "bsp/native_spatial_lifecycle.hpp"
#include "bsp/native_spatial_index_publication.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = volatile std::uint32_t;
using Pointer = void* volatile;
using Byte = volatile std::uint8_t;
void* offset(void* object, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + bytes);
}
template<class T> T& field(void* object, std::uint32_t bytes) noexcept {
    return *static_cast<T*>(offset(object, bytes));
}
void* current_index(const NativeSpatialLifecycleAccess* access) {
    return get_native_spatial_index_0042e630(*access->manager_publication_01090aa0,
        *access->index_publication_00f8a0d8);
}
} // namespace

void* __fastcall destroy_native_spatial_index_0042d4b0(void* index,
    void* volatile* publication, std::uint32_t flags) noexcept {
    const bool release = (flags & 1u) != 0;
    *publication = nullptr;
    field<Word>(index, 0) = 0x00ce3818u;
    if (release) singleton_lifetime_free(index);
    return index;
}

void __fastcall detach_native_unit_part_00710b80(void* part,
    const NativeSpatialLifecycleAccess* access) {
    if (!field<Byte>(part, 0x184)) return;
    detach_native_spatial_node_0098a500(current_index(access), nullptr, part);
    field<Byte>(part, 0x184) = 0;
}

void __fastcall refresh_native_spatial_node_0098bc70(void* node,
    const NativeSpatialLifecycleAccess* access) {
    const auto* spatial = access->spatial;
    void* owner = field<Pointer>(node, 0x4c);
    if (!field<Byte>(owner, 0xc8)) refresh_pose_00414db0(spatial->poses->resolve_pose(owner));
    copy_native_camera_matrix_004134f0(offset(node, 0x50), nullptr, offset(owner, 0xcc));
    owner = field<Pointer>(node, 0x4c);
    if (!field<Byte>(owner, 0x10c)) {
        refresh_pose_00414db0(spatial->poses->resolve_pose(owner));
        field<Byte>(owner, 0x10c) = 1;
        auto& derived = spatial->poses->resolve_derived(owner);
        derive_pose_affine_inverse_00b63d50(derived.derived_110, derived.pose.world_cc);
    }
    copy_native_camera_matrix_004134f0(offset(node, 0x90), nullptr, offset(owner, 0x110));
    rebuild_native_spatial_world_bounds_0098a750(node, spatial);
    if (field<Word>(node, 0x40)) {
        std::uint32_t minimum[2], maximum[2];
        native_spatial_cell_of_point_0098ad60(minimum, offset(node, 0x13c), spatial);
        native_spatial_cell_of_point_0098ad60(maximum, offset(node, 0x148), spatial);
        std::uint32_t key = (maximum[1] << 8) + maximum[0];
        key = (key << 8) + minimum[1];
        key = (key << 8) + minimum[0];
        if (key != field<Word>(node, 0x3c)) {
            unregister_native_spatial_cells_0098a3d0(current_index(access), node);
            register_native_spatial_cells_0098a310(current_index(access), node, minimum, maximum);
        }
    }
    void* cursor = field<Pointer>(node, 0xfc);
    void* const end = offset(cursor, field<Word>(node, 0x100) * 4u);
    while (cursor != end) {
        refresh_native_spatial_node_0098bc70(field<Pointer>(cursor, 0), access);
        cursor = offset(cursor, 4u);
    }
}

void __fastcall refresh_native_spatial_roots_0098bdb0(void* index,
    const NativeSpatialLifecycleAccess* access, float) {
    void* node = field<Pointer>(index, 0x16014);
    while (node) {
        if (!field<Byte>(node, 8)) refresh_native_spatial_node_0098bc70(node, access);
        node = field<Pointer>(node, 0x48);
    }
}

// Complete intrusive-link kernels follow below.

__declspec(naked) void __fastcall detach_native_spatial_child_0098a2c0(void*, void*, void*) {
    __asm {
        add dword ptr [ecx + 0x100], -1 // 0098a2c0
        mov edx, dword ptr [esp + 4] // 0098a2c7
        push esi // 0098a2cb
        mov esi, dword ptr [ecx + 0x100] // 0098a2cc
        push edi // 0098a2d2
        mov edi, dword ptr [edx + 0x15c] // 0098a2d3
        cmp edi, esi // 0098a2d9
        jge L_0098a2fe // 0098a2db
        mov eax, dword ptr [ecx + 0xfc] // 0098a2dd
        mov esi, dword ptr [eax + esi*4] // 0098a2e3
        mov dword ptr [eax + edi*4], esi // 0098a2e6
        mov eax, dword ptr [edx + 0x15c] // 0098a2e9
        mov ecx, dword ptr [ecx + 0xfc] // 0098a2ef
        mov ecx, dword ptr [ecx + eax*4] // 0098a2f5
        mov dword ptr [ecx + 0x15c], eax // 0098a2f8
    L_0098a2fe:
        pop edi // 0098a2fe
        mov dword ptr [edx + 0x108], 0 // 0098a2ff
        pop esi // 0098a309
        ret 4 // 0098a30a
    }
}

__declspec(naked) void __fastcall unregister_native_spatial_cells_0098a3d0(void*, void*) {
    __asm {
        sub esp, 0x14 // 0098a3d0
        mov eax, dword ptr [edx + 0x3c] // 0098a3d3
        push ebx // 0098a3d6
        push ebp // 0098a3d7
        push esi // 0098a3d8
        mov esi, eax // 0098a3d9
        push edi // 0098a3db
        mov edi, eax // 0098a3dc
        movzx ebx, ah // 0098a3de
        shr esi, 0x10 // 0098a3e1
        shr eax, 0x18 // 0098a3e4
        xor ebp, ebp // 0098a3e7
        and edi, 0xff // 0098a3e9
        and esi, 0xff // 0098a3ef
        cmp dword ptr [edx + 0x40], ebp // 0098a3f5
        mov dword ptr [esp + 0x10], ecx // 0098a3f8
        mov dword ptr [esp + 0x18], ebx // 0098a3fc
        mov dword ptr [esp + 0x20], eax // 0098a400
        jle L_0098a436 // 0098a404
        lea eax, [edx + 0xc] // 0098a406
        lea esp, [esp] // 0098a409
    L_0098a410:
        mov ecx, dword ptr [eax] // 0098a410
        test ecx, ecx // 0098a412
        je L_0098a41c // 0098a414
        mov ebx, dword ptr [eax + 4] // 0098a416
        mov dword ptr [ecx + 4], ebx // 0098a419
    L_0098a41c:
        mov ecx, dword ptr [eax + 4] // 0098a41c
        test ecx, ecx // 0098a41f
        je L_0098a427 // 0098a421
        mov ebx, dword ptr [eax] // 0098a423
        mov dword ptr [ecx], ebx // 0098a425
    L_0098a427:
        add ebp, 1 // 0098a427
        add eax, 0xc // 0098a42a
        cmp ebp, dword ptr [edx + 0x40] // 0098a42d
        jl L_0098a410 // 0098a430
        mov ebx, dword ptr [esp + 0x18] // 0098a432
    L_0098a436:
        xor ebp, ebp // 0098a436
        cmp edi, esi // 0098a438
        jg L_0098a4b4 // 0098a43a
        mov ecx, dword ptr [esp + 0x10] // 0098a43c
        mov eax, edi // 0098a440
        imul eax, eax, 0x96 // 0098a442
        add eax, ebx // 0098a448
        lea ecx, [ecx + eax*4 + 0x84] // 0098a44a
        sub esi, edi // 0098a451
        mov dword ptr [esp + 0x10], ecx // 0098a453
        add esi, 1 // 0098a457
        jmp L_0098a464 // 0098a45a
        lea esp, [esp] // 0098a45c
    L_0098a460:
        mov ebx, dword ptr [esp + 0x18] // 0098a460
    L_0098a464:
        mov eax, dword ptr [esp + 0x20] // 0098a464
        cmp ebx, eax // 0098a468
        jg L_0098a496 // 0098a46a
        sub eax, ebx // 0098a46c
        lea edi, [ebp + ebp*2 + 3] // 0098a46e
        add eax, 1 // 0098a472
        lea edi, [edx + edi*4] // 0098a475
        add ebp, eax // 0098a478
        lea ebx, [ebx] // 0098a47a
    L_0098a480:
        mov ebx, dword ptr [ecx] // 0098a480
        cmp ebx, edi // 0098a482
        jne L_0098a48b // 0098a484
        mov ebx, dword ptr [ebx + 4] // 0098a486
        mov dword ptr [ecx], ebx // 0098a489
    L_0098a48b:
        add edi, 0xc // 0098a48b
        add ecx, 4 // 0098a48e
        sub eax, 1 // 0098a491
        jne L_0098a480 // 0098a494
    L_0098a496:
        mov ecx, dword ptr [esp + 0x10] // 0098a496
        add ecx, 0x258 // 0098a49a
        sub esi, 1 // 0098a4a0
        mov dword ptr [esp + 0x10], ecx // 0098a4a3
        jne L_0098a460 // 0098a4a7
        pop edi // 0098a4a9
        mov dword ptr [edx + 0x40], esi // 0098a4aa
        pop esi // 0098a4ad
        pop ebp // 0098a4ae
        pop ebx // 0098a4af
        add esp, 0x14 // 0098a4b0
        ret  // 0098a4b3
    L_0098a4b4:
        pop edi // 0098a4b4
        pop esi // 0098a4b5
        mov dword ptr [edx + 0x40], ebp // 0098a4b6
        pop ebp // 0098a4b9
        pop ebx // 0098a4ba
        add esp, 0x14 // 0098a4bb
        ret  // 0098a4be
    }
}

__declspec(naked) void __fastcall remove_native_spatial_loose_node_0098a4c0(void*, void*) {
    __asm {
        push edi // 0098a4c0
        mov edi, dword ptr [ecx + 0x80] // 0098a4c1
        xor eax, eax // 0098a4c7
        test edi, edi // 0098a4c9
        jle L_0098a4f2 // 0098a4cb
        push esi // 0098a4cd
        lea esi, [ecx + 8] // 0098a4ce
    L_0098a4d1:
        cmp dword ptr [esi], edx // 0098a4d1
        je L_0098a4e2 // 0098a4d3
        add eax, 1 // 0098a4d5
        add esi, 4 // 0098a4d8
        cmp eax, edi // 0098a4db
        jl L_0098a4d1 // 0098a4dd
        pop esi // 0098a4df
        pop edi // 0098a4e0
        ret  // 0098a4e1
    L_0098a4e2:
        mov edx, dword ptr [ecx + edi*4 + 4] // 0098a4e2
        mov dword ptr [ecx + eax*4 + 8], edx // 0098a4e6
        add dword ptr [ecx + 0x80], -1 // 0098a4ea
        pop esi // 0098a4f1
    L_0098a4f2:
        pop edi // 0098a4f2
        ret  // 0098a4f3
    }
}

__declspec(naked) void __fastcall detach_native_spatial_node_0098a500(void*, void*, void*) {
    __asm {
        push esi // 0098a500
        mov esi, dword ptr [esp + 8] // 0098a501
        cmp byte ptr [esi + 0x158], 0 // 0098a505
        push edi // 0098a50c
        mov edi, ecx // 0098a50d
        je L_0098a581 // 0098a50f
        mov ecx, dword ptr [esi + 0x108] // 0098a511
        test ecx, ecx // 0098a517
        jne L_0098a574 // 0098a519
        cmp dword ptr [esi + 0x40], 0 // 0098a51b
        mov edx, esi // 0098a51f
        mov ecx, edi // 0098a521
        je L_0098a563 // 0098a523
        call unregister_native_spatial_cells_0098a3d0 // 0098a525
        mov eax, dword ptr [edi + 0x16014] // 0098a52a
        cmp eax, esi // 0098a530
        jne L_0098a53d // 0098a532
        mov eax, dword ptr [eax + 0x48] // 0098a534
        mov dword ptr [edi + 0x16014], eax // 0098a537
    L_0098a53d:
        mov eax, dword ptr [esi + 0x44] // 0098a53d
        test eax, eax // 0098a540
        je L_0098a54a // 0098a542
        mov ecx, dword ptr [esi + 0x48] // 0098a544
        mov dword ptr [eax + 0x48], ecx // 0098a547
    L_0098a54a:
        mov eax, dword ptr [esi + 0x48] // 0098a54a
        test eax, eax // 0098a54d
        je L_0098a57a // 0098a54f
        mov edx, dword ptr [esi + 0x44] // 0098a551
        mov dword ptr [eax + 0x44], edx // 0098a554
        pop edi // 0098a557
        mov byte ptr [esi + 0x158], 0 // 0098a558
        pop esi // 0098a55f
        ret 4 // 0098a560
    L_0098a563:
        call remove_native_spatial_loose_node_0098a4c0 // 0098a563
        pop edi // 0098a568
        mov byte ptr [esi + 0x158], 0 // 0098a569
        pop esi // 0098a570
        ret 4 // 0098a571
    L_0098a574:
        push esi // 0098a574
        call detach_native_spatial_child_0098a2c0 // 0098a575
    L_0098a57a:
        mov byte ptr [esi + 0x158], 0 // 0098a57a
    L_0098a581:
        pop edi // 0098a581
        pop esi // 0098a582
        ret 4 // 0098a583
    }
}

} // namespace bsp
