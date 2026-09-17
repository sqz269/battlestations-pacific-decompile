#pragma once
#include "bsp/native_render_pass_initializers.hpp"
#include <array>

namespace bsp {
struct NativeBloomTextureContext {
    NativeRenderTextureSurfaceOwnerContext& holders;
    const volatile std::uint32_t* texture_profile_00d61948;
    const volatile float& unsigned_bias_00ce3978;
    const volatile double& half_00d7a280;
};
struct NativeBloomInitializationArguments {
    const void* input_holder;
    std::uint32_t width,height,format,parameter_438_bits;
};
static_assert(sizeof(NativeBloomInitializationArguments)==0x14);
struct NativeBloomTextureAcquired {
    std::array<void*,3> raw{},returned{};
    std::array<bool,3> published{};
    std::array<NativeRenderTextureSurfaceOwnerAcquired,3> holders;
    NativeRenderTextureSurfaceOwnerArguments holder_arguments{};
    std::uint32_t current{};
    float saved_output_x;
    bool entered{},completed{},raw_free_started{};
};
struct NativeBloomInitializationContext {
    NativeBloomTextureContext textures;
    NativePostEffect20ConstructionContext& post_effects;
    NativeMaterialParameterAccess& parameters;
    const char* blur_name_00d62180;
    const char* bloom_name_00d62164;
    const char* sample_offsets_name_00d5e40c;
    const char* sample_weights_name_00d61fac;
    const char* texture_offset_name_00d62170;
};
struct NativeBloomInitializationAcquired {
    void* receiver{};
    NativeBloomTextureAcquired textures;
    std::array<void*,2> raw_posts{},returned_posts{};
    std::array<bool,2> posts_published{},post_raw_free_started{},completed_post_preserved{};
    NativeString* common_name_header{};
    NativeString* last_name_header{};
    const void* last_return_header{};
    std::uint32_t name_returns_started{},name_mask{};
    int native_state{-1};
    bool completed{};
};

// Prepare both existing B4E470 companion blocks before entering the native
// projection; they remain alive through published children and failed attempts.
// No second count/domain, automatic published-child cleanup or rollback exists.
class NativeBloomInitializationBlock final {
public:
    enum class Phase {idle,preparing,prepared,executing,settled};
    NativeBloomInitializationBlock() noexcept=default;
    ~NativeBloomInitializationBlock() noexcept;
    NativeBloomInitializationBlock(const NativeBloomInitializationBlock&)=delete;
    NativeBloomInitializationBlock& operator=(const NativeBloomInitializationBlock&)=delete;
    void prepare(NativeBloomInitializationContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept {return phase_;}
    const NativeBloomInitializationAcquired& acquired() const noexcept {return acquired_;}
    NativePostEffect20ConstructionBlock& blur() noexcept {return posts_[0];}
    NativePostEffect20ConstructionBlock& bloom() noexcept {return posts_[1];}
    // Releases nothing. Caller disposes every survivor and residual name,
    // ends parameter borrows and resets BOTH child blocks before this call.
    void reset_after_host_quiescence() noexcept;
private:
    friend void initialize_native_bloom_00b54f90(void*,std::size_t,
        NativeBloomInitializationArguments,NativeBloomInitializationBlock&);
    void settle() noexcept;
    void build_name(NativeString&,const char*,std::uint32_t);
    void return_name(NativeString&);
    void create_post(std::uint32_t,std::uint32_t,const char*,std::uint32_t,int,int);
    void register_parameter(std::uint32_t,std::uint32_t,const char*,std::uint32_t,int,bool=false);
    void unwind();
    NativeBloomInitializationContext* context_{};
    Phase phase_{Phase::idle};
    NativeBloomInitializationAcquired acquired_;
    NativePostEffect20ConstructionBlock posts_[2];
    NativeString common_name_,last_name_;
};

// Complete1459-byte B54F90. Original ECX existing43Ch bloom, five stackwords,
// RET14h; source adds persistent host state. Store exact parameter+438 bits;
// obtain current input texture dimensions; create three mode0 holders; derive
// half-texel pairs+428/+430 with original x87 ordering and an intervening height
// getter; create blur and bloom20h children; register six borrowed float4 arrays
// and bind only blur slot0 to current input texture. Arrays+28..427 remain
// preimages for later draw/update code. Preserve13-state cleanup and completed
// child host-binding failures. Blur's+430 registration spans four DWORDs through
// +43F despite the native43Ch allocation; this initializer never reads that last
// word. Preserve the original descriptor, leaving its later consumer/allocator
// padding contract unproved. No full initializer/native FH3/gameplay claim.
void initialize_native_bloom_00b54f90(void*,std::size_t,
    NativeBloomInitializationArguments,NativeBloomInitializationBlock&);
} // namespace bsp
