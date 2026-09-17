#pragma once
#include "bsp/native_render_texture_surface_owner.hpp"
#include "bsp/native_post_effect_owner_20h.hpp"

namespace bsp {
struct NativeRenderEffectLifetimeContext {
    // Existing canonical post-effect20 companions, sharing each actual+04.
    // Resolve only at zero; no temporary owner, replacement count or fallback.
    NativeRenderActualOwners& actual_post_effects;
    NativeRenderTextureSurfaceOwnerContext& texture_holders;
    NativeTextureSurfaceReferenceIncrement const volatile& actual_decrement_00ce2220;
    const volatile std::uint32_t* actual_post_effect_profile_00d61ec0;
    const volatile std::uint32_t* actual_holder_profile_00d61eb8;
};

// Full B0F5E0..B0F673. ECX actual effect-pass owner, RET. StampD5E140;
// capture+08 then actual decrement import once; release current+08,+0C and
// clear each after its captured call. Actual D61EC0 post-effect20 references
// retire through their existing canonical companion; D61EB8 holders use full
// B4E410. Base-only cleanup stampsCEB130; no member-release retry.
void destroy_native_render_effect_base_00b0f5e0(void*,NativeRenderEffectLifetimeContext&);

// Four complete30-byte scalar wrappers over the common base body. Each takes
// original ECX owner and stack flags, returns original address in EAX/RET4,
// and frees only after successful destruction when flags&1. Full post-free
// ADD ESP4 tails are retained. New context parameter changes the source ABI.
void* delete_native_depth_downscale_pass_00b10120(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);
void* delete_native_particle_blend_pass_00b10140(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);
void* delete_native_downscale4x4_pass_00b10160(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);
void* delete_native_downscale2x2_pass_00b10180(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);

// Complete42-byte B54E70, original ECX/EAX/RET. Actual43Ch allocation is
// proved by callerB10FB3; only profile/count/+0C/+18/+1C/+20/+24/+08 are
// initialized, in native order. Dimensions, parameter arrays and tail retain
// their preimages for the later B54F90 initializer (not implemented here).
void* __fastcall construct_native_bloom_owner_00b54e70(void*) noexcept;

// Full B54EA0..B54F69: profileD62150; release holders+18/+1C/+20, then
// post-effects+24/+08, using a fresh decrement IAT read for EACH operation.
// Clear after returns, disarm derived cleanup, invoke full common base. On
// an escaping C++ exception the native cleanup projection calls that same
// full base; it can revisit+08. No preclear or synthetic rollback is added.
void destroy_native_bloom_owner_00b54ea0(void*,NativeRenderEffectLifetimeContext&);
// Full30-byte B54F70 scalar wrapper, original ECX/stack flags/EAX/RET4.
void* delete_native_bloom_owner_00b54f70(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);

// All domains/companions must be live and canonical. Current D61EC0/BD30E0/
// B4E430 and D61EB8/BD30E0/B4E410 dispatches are the admitted concrete domain.
// The context owns no native storage. Source validation failures are contract
// errors. Native FH3/SEH, arbitrary profiles and application/gameplay behavior
// are separate from these new source interfaces and C++ cleanup projections.
} // namespace bsp
