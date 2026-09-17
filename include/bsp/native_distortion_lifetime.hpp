#pragma once
#include "bsp/native_render_effect_lifetime.hpp"
#include "bsp/gui_camera_store_owner.hpp"

namespace bsp {
// Actual0Ch header at distortion owner+254; eight-byte opaque records.
// The array helpers neither retain nor release either record DWORD.
struct NativeDistortionRecordArray {
    void* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(NativeDistortionRecordArray)==12);
void reserve_native_distortion_records_00b4ed70(NativeDistortionRecordArray&,std::int32_t);
void resize_native_distortion_records_00b4ee20(NativeDistortionRecordArray&,std::int32_t);
// Resize0 then free current pointer; preserve the stale pointer/capacity.
void destroy_native_distortion_records_00b4f0a0(NativeDistortionRecordArray&);

struct NativeDistortionOwnerConstants {
    const volatile std::uint32_t& bits_00cf4848;
    const volatile std::uint32_t& bits_00ce3958;
    const volatile std::uint32_t& bits_00d1f3c4;
};
struct NativeDistortionLifetimeContext {
    NativeRenderEffectLifetimeContext& effects;
    NativeNodeDestructionRuntime& nodes;
    NativeFrameTargetOwnerContext& frames;
    const volatile std::uint32_t* frame_profile_00d5e600;
    const volatile std::uint32_t* scene_profile_00d62d48;
    // SAME actual+3C scene's existing concrete companion, consulted only at
    // zero. No fabricated scene/count or new map. It can self-delete; external
    // binding owners must not reuse a dangling publication after terminal exit.
    NativeGuiSceneOwner* const volatile& scene_owner_3c;
};
// Complete B4F0C0[129], original ECX/EAX/RET, actual26Ch caller allocation.
// Preserve individual constant-read order and all unwritten bytes, including
// +34 camera/+38 frame/+3C scene. Construction alone does NOT establish safe
// values for those three cleanup fields; the initializer/caller must do so.
void* construct_native_distortion_owner_00b4f0c0(void*,const NativeDistortionOwnerConstants&);
// Complete B4EE70[286], original ECX/RET. Two holders10/14 use fresh import
// reads; capture18 then decrement import once for18/1C/20/38, unlink current
// camera34, release3C/240 with that captured target. Clear only after return.
void clear_native_distortion_resources_00b4ee70(void*,NativeDistortionLifetimeContext&);
// Full repaired B4F150[112]: clear resources, resize254 records0, free backing,
// disarm and full B0F5E0. State1 unwind disposes records then base, state0 base.
void destroy_native_distortion_owner_00b4f150(void*,NativeDistortionLifetimeContext&);
// Full B4F540[30]: destroy, free iff flags bit0, return original identity.
void* delete_native_distortion_owner_00b4f540(void*,std::uint32_t,NativeDistortionLifetimeContext&);
// New source interfaces and canonical supported domains; native entry ABI,
// arbitrary profiles, allocation-failure/corrupt extents and FH3/SEH are not
// established. This module does not implement B4F560 initialization/rendering.
} // namespace bsp
