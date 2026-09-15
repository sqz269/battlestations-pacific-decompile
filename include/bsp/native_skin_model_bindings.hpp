#pragma once
#include "bsp/native_resource_instance_postprocess.hpp"
#include <cstdint>

namespace bsp {
// The node slot-zero lifetime protocol is identical to the existing borrowed
// animator protocol: resolve the same current table and perform its complete
// target, including ownership. These calls receive actual nodes, not animators.
using NativeSkinNodeLifetime = NativeResourceAnimatorLifetime;

// Actual 0Ch array header, 60h records: node0, translation4, angles10, scalar1C,
// matrix20. The model owns the header at184/188/18C. This layout is distinct
// from the ordinary model's four-byte bone-pointer array at the same offset.
// B90EA0/B91460: ECXheader, signed stack capacity/count, RET4. Source adds a
// borrowed lifetime context. Reserve copies WITHOUT retaining the node, then
// releases old records in ascending order and frees current old backing before
// publishing data/capacity. Resize leaves new scalar1C untouched.
void reserve_native_skin_bindings_00b90ea0(void* header, std::int32_t capacity,
    NativeSkinNodeLifetime&);
void resize_native_skin_bindings_00b91460(void* header, std::int32_t count,
    NativeSkinNodeLifetime&);
// B91590 tail-adapts ECXmodel+184 to B91460, preserving its stack argument.
void resize_native_skin_model_bindings_00b91590(void* model, std::int32_t count,
    NativeSkinNodeLifetime&);

// B90C30: ECXmodel, five stack arguments, RET14h. Publish/retain the incoming
// node before releasing old; reacquire current backing after release. Source
// takes scalar bits to preserve the native MOVSS transport exactly.
void bind_native_skin_node_00b90c30(void* model, std::uint32_t index, void* node,
    const void* translation, const void* angles, std::uint32_t scalar_bits,
    NativeSkinNodeLifetime&);

// Complete24-byte ECXanimator/stackfloat3/RET4 setters. Unused EDX preserves
// native stack placement. Three ordered x87 copies preserve alias/status effects.
void __fastcall set_native_animator_translation_00b75d80(void*, void*, const void*) noexcept;
void __fastcall set_native_animator_angles_00b76570(void*, void*, const void*) noexcept;

// B64DB0: ECXdestination, EDXtranslation, stackangles/unused-scale, EAXdest,
// RET8. The second stack pointer is NOT read. Native FSIN/FCOS float spills,
// SSE negative-zero subtraction, two matrix multiplies and general inversion
// are preserved; this is not a conventional host Euler/scale replacement.
void* __fastcall build_native_skin_inverse_pose_00b64db0(void* destination,
    const void* translation, const void* angles, const void* unused_scale);

// B90CC0: ECXmodel, stackparent-node/parent-index, RET8. Walk child34 and
// sibling3C links; first matching record only; recurse only through matched
// children. B91000: ECXmodel/RET, first update present-node animators, then
// initialize direct-child matrices and recurse. No missing-animator/cycle guard.
void __fastcall build_native_skin_descendant_matrices_00b90cc0(void* model,
    void* unused_edx, void* parent_node, std::uint32_t parent_index);
void __fastcall finalize_native_skin_bindings_00b91000(void* model);
} // namespace bsp
