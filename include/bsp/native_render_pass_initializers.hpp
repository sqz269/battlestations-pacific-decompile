#pragma once
#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_render_texture_surface_owner.hpp"
#include "bsp/native_material_parameters.hpp"

namespace bsp {
struct NativeRenderPassInitializationContext {
    NativePostEffect20ConstructionContext& post_effects;
    NativeRenderTextureSurfaceOwnerContext& holders;
    NativeMaterialParameterAccess& parameters;
    const volatile double& half_00d7a280;
    const volatile double& unsigned_bias_00d57da0;
    const char* depth_name_00d620c0;
    const char* particle_name_00d5e41c;
    const char* sample_offsets_name_00d5e40c;
};
struct NativeDepthDownscalePassArguments {
    const void* input_holder;
    std::uint32_t width,height,format;
};
struct NativeParticleBlendPassArguments {
    std::uint32_t unused_first_word;
    void* texture0;
    void* texture1;
    std::uint32_t width,height,format,multisample,mode;
    NativeSurfaceOwnerStorage* external_surface;
};
static_assert(sizeof(NativeDepthDownscalePassArguments)==16);
static_assert(sizeof(NativeParticleBlendPassArguments)==36);

// Diagnostics are persistent identities, not extra ownership. A started return
// or free must never be replayed, even if the corresponding header stays stale.
struct NativeRenderPassInitializationAcquired {
    void* receiver{};
    void* raw_post_effect{};
    void* returned_post_effect{};
    void* raw_holder{};
    void* returned_holder{};
    NativeString* effect_name_header{};
    NativeString* parameter_name_header{};
    NativeRenderTextureSurfaceOwnerArguments holder_arguments{};
    NativeRenderTextureSurfaceOwnerAcquired holder;
    int native_state{-1};
    bool effect_name_constructed{};
    bool effect_name_return_started{};
    bool parameter_name_return_started{};
    bool post_raw_free_started{};
    bool holder_raw_free_started{};
    bool completed_post_preserved_after_host_failure{};
    bool post_published{};
    bool holder_published{};
    bool completed{};
};

// Prepare before entering either initializer. The full B4E470 constructor's
// canonical companions survive the parent, including settled failure states.
// This frame does not release published children or restore old receiver fields.
class NativeRenderPassInitializationBlock final {
public:
    enum class Phase { idle,preparing,prepared,executing,settled };
    NativeRenderPassInitializationBlock() noexcept=default;
    ~NativeRenderPassInitializationBlock() noexcept;
    NativeRenderPassInitializationBlock(const NativeRenderPassInitializationBlock&)=delete;
    NativeRenderPassInitializationBlock& operator=(const NativeRenderPassInitializationBlock&)=delete;
    void prepare(NativeRenderPassInitializationContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept {return phase_;}
    const NativeRenderPassInitializationAcquired& acquired() const noexcept {return acquired_;}
    NativePostEffect20ConstructionBlock& post_effect() noexcept {return post_effect_;}
    // Releases nothing. Caller disposes every native survivor/name allocation,
    // ends all parameter-source borrows and resets post_effect() first.
    void reset_after_host_quiescence() noexcept;
private:
    friend void initialize_native_depth_downscale_pass_00b540b0(void*,std::size_t,
        NativeDepthDownscalePassArguments,NativeRenderPassInitializationBlock&);
    friend void initialize_native_particle_blend_pass_00b542d0(void*,std::size_t,
        NativeParticleBlendPassArguments,NativeRenderPassInitializationBlock&);
    void execute(void*,std::size_t,bool,const NativeDepthDownscalePassArguments&,
        const NativeParticleBlendPassArguments&);
    void set_state(int) noexcept;
    void return_effect_name();
    void return_parameter_name();
    void unwind(bool&);
    void settle() noexcept;
    NativeRenderPassInitializationContext* context_{};
    Phase phase_{Phase::idle};
    NativeRenderPassInitializationAcquired acquired_;
    NativePostEffect20ConstructionBlock post_effect_;
    NativeString effect_name_,parameter_name_;
};

// Exact B4CB20 four-byte borrowed primary-surface getter: ECX holder, EAX, RET.
NativeSurfaceOwnerStorage* __fastcall native_render_holder_primary_00b4cb20(const void*) noexcept;
// Complete B4CB70 eighteen-byte wrapper: ECX post20, stack surface, RET4.
// Reload actual post20+08 frame and invoke full B1FAB0(slot0,surface).
void set_native_post_effect_color0_00b4cb70(void*,NativeSurfaceOwnerStorage*,NativeFrameTargetOwnerContext&);

// Full B540B0[500], ECX existing20h pass; four stack words, RET10h.
// Construct B4E470(count3,null), publish+08, register borrowed inline float4
// half/(2*width),half/(2*height),0,0 using wrapping DWORD doubles and x87;
// bind input holder's borrowed texture to current material slot0; construct
// B4E020(width,height,format,0,0,null), publish+0C, bind its primary to color0.
void initialize_native_depth_downscale_pass_00b540b0(void*,std::size_t,
    NativeDepthDownscalePassArguments,NativeRenderPassInitializationBlock&);
// Full B542D0[523], ECX existing20h pass; nine stack words, RET24h.
// First stack word is unread. Bind texture0/1 BEFORE half/width,half/height
// offsets; forward all six remaining holder arguments including low mode byte.
void initialize_native_particle_blend_pass_00b542d0(void*,std::size_t,
    NativeParticleBlendPassArguments,NativeRenderPassInitializationBlock&);

// All contexts must borrow the same canonical owners, pools, strings, renderer
// and readonly constants. Receiver/profile/count preexist and are not reset.
// New source interfaces and explicit C++ cleanup projection do not establish
// native ABI/FH3/SEH, private-stack aliasing or application/gameplay parity.
} // namespace bsp
