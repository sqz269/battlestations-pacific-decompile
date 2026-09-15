#pragma once
#include "bsp/native_animation_registry.hpp"
#include "bsp/native_postprocess_pose.hpp"
#include "bsp/native_postprocess_storage.hpp"
#include "bsp/native_resource_instance_postprocess.hpp"

namespace bsp {
// Table resolution is bookkeeping only, as in NativeResourceAnimatorLifetime.
// Each other operation must dispatch the complete actual native virtual target.
// The context neither owns native objects nor supplies replacement containers.
class NativeResourcePostprocessCalls : public NativeResourceAnimatorLifetime {
public:
    virtual std::uint8_t is_type(std::uint32_t target, void* owner,
        std::uint32_t current_token) = 0;
    virtual void finalize_animator(std::uint32_t target, void* animator) = 0;
};
struct NativeResourcePostprocessContext {
    NativeAnimationRegistryContext& registry;
    NativeResourcePostprocessCalls& calls;
    const CameraAxesCrtAccess& axes_crt;
    const volatile std::uint32_t& compact_type_0109042c;
    const volatile std::uint32_t& track_type_01090268;
    const volatile std::uint32_t& skin_item_type_01090278;
    const volatile std::uint32_t& camera_type_01090288;
    const volatile std::uint32_t& skin_model_type_01090344;
};

// Full physical B79BC0..B7A9DC: original ECX instance, RET. Source EDX is a
// borrowed context over the SAME current tables, token cells, pooled strings,
// CRT validation/FP services and native allocation domain. This is a new C++
// entry point, not original FH3 metadata or a drop-in binary replacement.
// Assumes readable native owners, valid extents and fully implemented virtual
// providers. It preserves native missing-name index -1 and camera self-release;
// callers must not use invalid native graphs as a safety-checked host API.
void __fastcall postprocess_native_resource_instance_00b79bc0(
    void* instance, NativeResourcePostprocessContext&);

// B8F920: original MOV EAX,[01090344]/RET. Source borrows that current cell.
std::uint32_t native_skin_model_type_00b8f920(const volatile std::uint32_t&) noexcept;
// Original ECX item/EAX interior header or vector/RET; scalar returns in ST0.
const void* __fastcall native_skin_item_name_00b8a180(const void*) noexcept;
const void* __fastcall native_skin_item_translation_00b8a190(const void*) noexcept;
const void* __fastcall native_skin_item_angles_00b8a1a0(const void*) noexcept;
float __fastcall native_skin_item_scalar_00b8a1b0(const void*) noexcept;
// Native cleanup leaves stale backing/capacity after resize(0)/free. Pair
// resize B76B50 reuses B40E00 on the established valid-extent domain.
void __fastcall destroy_native_postprocess_node_tracks_00b78f60(
    NativePostprocessNodeTracksArray&);
void __fastcall destroy_native_postprocess_pairs_00b77cd0(NativePostprocessPairArray&);
} // namespace bsp
