#pragma once
#include <cstdint>

namespace bsp {
class SceneAttachmentRuntime;
struct SceneResource;
struct NativeTracelineRenderAccess;

// Borrow the SAME current profile, model/scene associations, physical mapping
// and entry camera services already used by collection/geometry/rendering.
// No owner map, copied entry, queue, stream or generator callback is created.
struct NativeInstanceGroupUploadAccess {
    NativeTracelineRenderAccess& render;
    SceneAttachmentRuntime& scenes;
    const volatile float& half_00ce3800;
    const volatile float& unsigned_bias_00ce3978;
};

// Complete B1E990..B1EB7D normal body, ECX actual44h command, RET; this new
// C++ interface adds explicit access. Walk captured raw slots and reload live
// ends/counts/models/generators at the original call boundaries. Category one
// uses full actual-slot B1DCE0 and raw B51AB0; map/unmap use actual B49980 /
// B49A80. Actual B51A20 entries and B51CB0 batches preserve original identities.
//
// Generated Model v50 must currently be B6ED80; generator v8 must currently
// be B556F0 or B55780, as published by B44FD0/B450D0. Other current targets
// are outside this concrete dispatch domain, never successful no-op adapters.
// Producer-created valid live storage and nonthrowing canonical services are
// required. No native lock/count guards, rollback or unmap-on-exception are
// added. Source service ABI, hardware SEH and gameplay remain unestablished.
void upload_native_instance_groups_00b1e990(void* actual_command,
    NativeInstanceGroupUploadAccess&);

// Complete B556F0..B55777 and B55780..B55B16, original generator ECX unused,
// two original stack words (actual28h entry, actual output), RET8. Generic
// writes48 bytes; building writes144 bytes. Preserve native world refresh,
// sequential x87 copies, raw ordered point-light reads and diffuse getter.
// Building EDX adds ADDRESS of actual float2^32; original unsigned conversion
// and signed clamp / unsigned light-presence branches are preserved.
void __fastcall write_native_generic_instance_00b556f0(void* actual_generator,
    void* unused_edx, const void* actual_entry, void* actual_output);
void __fastcall write_native_building_instance_00b55780(void* actual_generator,
    const volatile float* actual_unsigned_bias_00ce3978,
    const void* actual_entry, void* actual_output);

// Complete raw leaves. B6DC50: EAX=ECX+164, RET. B72110: EAX=[ECX+1C],
// RET, no stack argument. B85590: store original DWORD to section+1C, RET4.
void* __fastcall native_node_point_light_array_00b6dc50(void*) noexcept;
SceneResource* __fastcall native_scene_lighting_owner_00b72110(const void*) noexcept;
void __fastcall set_native_section_instance_count_00b85590(void*, void* unused_edx,
    std::uint32_t count) noexcept;

namespace detail {
// Implementation adapter for parent span B1EACC..B1EB1B, NOT another native
// function. Keeps the outgoing argument/FP schedule in one compiled body.
// Actual group ECX, bindings EDX; stack category, captured mesh, section,
// command. No entry copy or lifetime operation; full B51A20 is called.
struct UploadOutputAccess {
    NativeTracelineRenderAccess* render;
    const volatile float* half;
};
void __fastcall initialize_upload_output(void* actual_group,
    const UploadOutputAccess*, std::uint32_t category, void* actual_mesh,
    void* actual_section, void* actual_command);
} // namespace detail
} // namespace bsp
