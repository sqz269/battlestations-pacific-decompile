#include "bsp/native_group_world_sphere.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Group sphere reconstruction requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeGroupSphereNumericAccess) == 8);
static_assert(offsetof(NativeGroupSphereNumericAccess, half_00d7a280) == 4);
namespace {
// Private native ECX/EDX/stack ABI. EBX carries only the added borrowed numeric
// access; its actual CRT member is passed to the existing recovered length.
__declspec(naked) float* __fastcall normalize_kernel(float*, const float*, float*) {
    __asm {
        PUSH ECX // 00b8e980
        PUSH ESI // 00b8e981
        PUSH EDI // 00b8e982
        MOV EDI,EDX // 00b8e983
        MOV ESI,ECX // 00b8e985
        MOV ECX,EDI // 00b8e987
        mov edx, dword ptr [ebx] // added actual CRT access
        CALL camera_vector_length_00419440 // 00b8e989
        FSTP dword ptr [ESP + 0x8] // 00b8e98e
        FLD dword ptr [ESP + 0x8] // 00b8e992
        MOV EAX,dword ptr [ESP + 0x10] // 00b8e996
        FST dword ptr [EAX] // 00b8e99a
        MOV EAX,ESI // 00b8e99c
        FLDZ // 00b8e99e
        FXCH // 00b8e9a0
        FCOMI ST(0),ST(1) // 00b8e9a2
        FSTP ST(1) // 00b8e9a4
        JBE L00b8e9ce // 00b8e9a6
        FSTP dword ptr [ESP + 0x10] // 00b8e9a8
        FLD dword ptr [EDI] // 00b8e9ac
        FLD dword ptr [ESP + 0x10] // 00b8e9ae
        FLD ST(0) // 00b8e9b2
        FDIVP ST(2),ST(0) // 00b8e9b4
        FXCH // 00b8e9b6
        FSTP dword ptr [ESI] // 00b8e9b8
        FLD dword ptr [EDI + 0x4] // 00b8e9ba
        FDIV ST(0),ST(1) // 00b8e9bd
        FSTP dword ptr [ESI + 0x4] // 00b8e9bf
        FDIVR dword ptr [EDI + 0x8] // 00b8e9c2
        POP EDI // 00b8e9c5
        FSTP dword ptr [ESI + 0x8] // 00b8e9c6
        POP ESI // 00b8e9c9
        POP ECX // 00b8e9ca
        RET 0x4 // 00b8e9cb
    L00b8e9ce:
        XORPS XMM0,XMM0 // 00b8e9ce
        FSTP ST(0) // 00b8e9d1
        POP EDI // 00b8e9d3
        MOVSS dword ptr [ESI],XMM0 // 00b8e9d4
        MOVSS dword ptr [ESI + 0x4],XMM0 // 00b8e9d8
        MOVSS dword ptr [ESI + 0x8],XMM0 // 00b8e9dd
        POP ESI // 00b8e9e2
        POP ECX // 00b8e9e3
        RET 0x4 // 00b8e9e4
    }
}
void copy_sphere(const float* source, float* destination) {
    __asm {
        mov eax, source
        mov esi, destination
        fld dword ptr [eax] // B8EC3C: preserve float load/store, not memcpy
        fstp dword ptr [esi]
        fld dword ptr [eax + 4]
        fstp dword ptr [esi + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [esi + 8]
        fld dword ptr [eax + 12]
        fstp dword ptr [esi + 12]
    }
}
NativeNodeStorage& raw_node(SceneNodeAttachment& binding) {
    const auto key = binding.transform.raw_node_key();
    if (!key || key != binding.pointer_key)
        throw std::invalid_argument("Group sphere requires the actual node identity");
    return *reinterpret_cast<NativeNodeStorage*>(key);
}
}

__declspec(naked) float* __fastcall normalize_native_group_sphere_delta_00b8e980(
    float*, const float*, float*, const NativeGroupSphereNumericAccess*) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 12]
        push dword ptr [esp + 8]
        call normalize_kernel
        pop ebx
        ret 8
    }
}
__declspec(naked) float* __fastcall merge_native_group_sphere_00b8e9f0(
    float*, const NativeGroupSphereNumericAccess*, const float*) {
    __asm {
        push ebx // added borrowed numeric context
        mov ebx, edx
        SUB ESP,0x30 // 00b8e9f0
        PUSH ESI // 00b8e9f3
        PUSH EDI // 00b8e9f4
        MOV EDI,dword ptr [ESP + 0x40] // 00b8e9f5
        FLD dword ptr [EDI] // 00b8e9f9
        MOV ESI,ECX // 00b8e9fb
        FSTP dword ptr [ESP + 0x40] // 00b8e9fd
        LEA EAX,[ESP + 0x18] // 00b8ea01
        FLD dword ptr [ESI] // 00b8ea05
        PUSH EAX // 00b8ea07
        FSTP dword ptr [ESP + 0xc] // 00b8ea08
        LEA EDX,[ESP + 0x24] // 00b8ea0c
        FLD dword ptr [ESP + 0x44] // 00b8ea10
        LEA ECX,[ESP + 0x30] // 00b8ea14
        FSUB dword ptr [ESP + 0xc] // 00b8ea18
        FSTP dword ptr [ESP + 0x24] // 00b8ea1c
        FLD dword ptr [ESI + 0x4] // 00b8ea20
        FSTP dword ptr [ESP + 0x10] // 00b8ea23
        FLD dword ptr [EDI + 0x4] // 00b8ea27
        FSUB dword ptr [ESP + 0x10] // 00b8ea2a
        FSTP dword ptr [ESP + 0x28] // 00b8ea2e
        FLD dword ptr [ESI + 0x8] // 00b8ea32
        FSTP dword ptr [ESP + 0x14] // 00b8ea35
        FLD dword ptr [EDI + 0x8] // 00b8ea39
        FSUB dword ptr [ESP + 0x14] // 00b8ea3c
        FSTP dword ptr [ESP + 0x2c] // 00b8ea40
        CALL normalize_kernel // 00b8ea44
        FLD dword ptr [EDI + 0xc] // 00b8ea49
        MOV EAX,ESI // 00b8ea4c
        FSTP dword ptr [ESP + 0x14] // 00b8ea4e
        FLD dword ptr [ESI + 0xc] // 00b8ea52
        FSTP dword ptr [ESP + 0x1c] // 00b8ea55
        FLD dword ptr [ESP + 0x14] // 00b8ea59
        FLD ST(0) // 00b8ea5d
        FLD dword ptr [ESP + 0x18] // 00b8ea5f
        FLD ST(0) // 00b8ea63
        FADDP ST(2),ST(0) // 00b8ea65
        FLD dword ptr [ESP + 0x1c] // 00b8ea67
        FCOMI ST(0),ST(2) // 00b8ea6b
        FSTP ST(2) // 00b8ea6d
        JNC L00b8eb10 // 00b8ea6f
        FADD ST(0),ST(1) // 00b8ea75
        FXCH ST(2) // 00b8ea77
        FCOMI ST(0),ST(2) // 00b8ea79
        JC L00b8eaa7 // 00b8ea7b
        MOVSS XMM0,dword ptr [ESP + 0x40] // 00b8ea7d
        FSTP ST(0) // 00b8ea83
        FSTP ST(0) // 00b8ea85
        MOVSS dword ptr [ESI],XMM0 // 00b8ea87
        FSTP ST(0) // 00b8ea8b
        FLD dword ptr [EDI + 0x4] // 00b8ea8d
        FSTP dword ptr [ESI + 0x4] // 00b8ea90
        FLD dword ptr [EDI + 0x8] // 00b8ea93
        FSTP dword ptr [ESI + 0x8] // 00b8ea96
        FLD dword ptr [EDI + 0xc] // 00b8ea99
        POP EDI // 00b8ea9c
        FSTP dword ptr [ESI + 0xc] // 00b8ea9d
        POP ESI // 00b8eaa0
        ADD ESP,0x30 // 00b8eaa1
        pop ebx // added binding cleanup
        RET 0x4 // 00b8eaa4
    L00b8eaa7:
        FXCH // 00b8eaa7
        POP EDI // 00b8eaa9
        FSTP dword ptr [ESP + 0x18] // 00b8eaaa
        FADDP ST(1),ST(0) // 00b8eaae
        mov edx, dword ptr [ebx + 4] // added live D7A280 address
        FMUL qword ptr [EDX] // 00b8eab0
        FSTP dword ptr [ESP + 0x3c] // 00b8eab6
        FLD dword ptr [ESP + 0x3c] // 00b8eaba
        FST dword ptr [ESI + 0xc] // 00b8eabe
        FSUB dword ptr [ESP + 0x18] // 00b8eac1
        FSTP dword ptr [ESP + 0x3c] // 00b8eac5
        FLD dword ptr [ESP + 0x3c] // 00b8eac9
        FLD ST(0) // 00b8eacd
        FMUL dword ptr [ESP + 0x28] // 00b8eacf
        FSTP dword ptr [ESP + 0x1c] // 00b8ead3
        FLD dword ptr [ESP + 0x2c] // 00b8ead7
        FMUL ST(0),ST(1) // 00b8eadb
        FSTP dword ptr [ESP + 0x20] // 00b8eadd
        FMUL dword ptr [ESP + 0x30] // 00b8eae1
        FSTP dword ptr [ESP + 0x24] // 00b8eae5
        FLD dword ptr [ESP + 0x4] // 00b8eae9
        FADD dword ptr [ESP + 0x1c] // 00b8eaed
        FSTP dword ptr [ESI] // 00b8eaf1
        FLD dword ptr [ESP + 0x8] // 00b8eaf3
        FADD dword ptr [ESP + 0x20] // 00b8eaf7
        FSTP dword ptr [ESI + 0x4] // 00b8eafb
        FLD dword ptr [ESP + 0xc] // 00b8eafe
        FADD dword ptr [ESP + 0x24] // 00b8eb02
        FSTP dword ptr [ESI + 0x8] // 00b8eb06
        POP ESI // 00b8eb09
        ADD ESP,0x30 // 00b8eb0a
        pop ebx // added binding cleanup
        RET 0x4 // 00b8eb0d
    L00b8eb10:
        FSTP ST(2) // 00b8eb10
        POP EDI // 00b8eb12
        FSTP ST(0) // 00b8eb13
        POP ESI // 00b8eb15
        FSTP ST(0) // 00b8eb16
        ADD ESP,0x30 // 00b8eb18
        pop ebx // added binding cleanup
        RET 0x4 // 00b8eb1b
    }
}

void aggregate_native_group_world_sphere_00b8ebe0(
    NativeGroupWorldSphereRuntime& runtime, NativeGroupOwner& owner) {
    if (owner.phase != NativeGroupOwner::Phase::live ||
        &owner.environment.nodes != &runtime.nodes ||
        &runtime.nodes.scenes.resolve(owner.node.transform) != &owner.node.scene_attachment ||
        &raw_node(owner.node.scene_attachment) != &owner.storage.node)
        throw std::logic_error("Group sphere requires the same live Group owner domain");
    auto& raw = owner.storage.node;
    auto& tail = owner.storage.group;
    if (tail.attached_count_17c < 0 || tail.attached_capacity_180 < tail.attached_count_17c ||
        (tail.attached_capacity_180 && !tail.attached_nodes_178))
        throw std::logic_error("Group sphere attachment descriptor has invalid extent");
    auto* sphere = raw.world_sphere_13c.data();
    const auto* empty_radius = &owner.environment.constants.bound_00ce4970;
    __asm {
        mov esi, sphere
        xorps xmm0, xmm0
        movss dword ptr [esi], xmm0
        movss dword ptr [esi + 4], xmm0
        movss dword ptr [esi + 8], xmm0
        mov eax, empty_radius
        movss xmm0, dword ptr [eax]
        movss dword ptr [esi + 12], xmm0
    }
    // Native captures count, then begin and the end derived from those values.
    // Avoid null+0 in C++ while retaining the empty native range's no-load path.
    const auto count = tail.attached_count_17c;
    auto** current = tail.attached_nodes_178;
    auto** const end = count ? current + count : current;
    bool seeded = false;
    for (; current != end; ++current) {
        auto* child = *current; // same captured allocation, current element
        if (!child) throw std::logic_error("Group sphere reached a null attachment");
        auto& binding = runtime.nodes.scenes.resolve(*child);
        if (!seeded && (raw_node(binding).auxiliary_flags_138 & 3u) == 0) continue;
        if (!runtime.sphere_virtual48)
            throw std::logic_error("Group sphere requires current child virtual48");
        const auto* child_sphere = runtime.sphere_virtual48(runtime, binding);
        if (!child_sphere) throw std::logic_error("Group sphere child virtual48 returned null");
        if (seeded) {
            if (!runtime.numeric.crt || !runtime.numeric.half_00d7a280)
                throw std::logic_error("Group sphere merge requires actual CRT and D7A280");
            merge_native_group_sphere_00b8e9f0(sphere, &runtime.numeric, child_sphere);
        } else {
            copy_sphere(child_sphere, sphere);
            seeded = true;
        }
    }
    raw.auxiliary_flags_138 |= 0x30u;
}
} // namespace bsp
