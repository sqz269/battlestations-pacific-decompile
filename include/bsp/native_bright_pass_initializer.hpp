#pragma once
#include "bsp/native_render_pass_initializers.hpp"
#include <array>

namespace bsp {
struct NativeBrightPassInitializationContext {
    NativePostEffect20ConstructionContext& post_effects;
    NativeRenderTextureSurfaceOwnerContext& holders;
    NativeMaterialParameterAccess& parameters;
    const char* effect_name_00d62138;
    // Original byte storage, in registration order: D62124, D62110,
    // D5E3F0, D5E3C0, D5E3B0. Preserve the native resize/copy extents.
    std::array<const char*,5> parameter_names;
};
struct NativeBrightPassInitializationAcquired {
    NativeRenderPassInitializationAcquired native;
    NativeString* last_parameter_name_header{};
    std::uint32_t effect_name_mask{};
    std::uint32_t parameter_returns_started{};
    bool last_parameter_return_started{};
};

// Persistent host state for full B54940. The existing B4E470 companions must
// outlive the native child and its borrowed parameters. No published-child
// rollback, old-child release, or writes to parameter values are introduced.
class NativeBrightPassInitializationBlock final {
public:
    enum class Phase { idle,preparing,prepared,executing,settled };
    NativeBrightPassInitializationBlock() noexcept=default;
    ~NativeBrightPassInitializationBlock() noexcept;
    NativeBrightPassInitializationBlock(const NativeBrightPassInitializationBlock&)=delete;
    NativeBrightPassInitializationBlock& operator=(const NativeBrightPassInitializationBlock&)=delete;
    void prepare(NativeBrightPassInitializationContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept {return phase_;}
    const NativeBrightPassInitializationAcquired& acquired() const noexcept {return acquired_;}
    NativePostEffect20ConstructionBlock& post_effect() noexcept {return post_effect_;}
    // Releases nothing. Dispose all survivors, end parameter borrows, and reset
    // post_effect() to idle before resetting this block.
    void reset_after_host_quiescence() noexcept;
private:
    friend void initialize_native_bright_pass_00b54940(void*,std::size_t,
        NativeDepthDownscalePassArguments,NativeBrightPassInitializationBlock&);
    void return_effect_name();
    void return_parameter_name(bool);
    void register_parameter(std::uint32_t);
    void unwind();
    void settle() noexcept;
    NativeBrightPassInitializationContext* context_{};
    Phase phase_{Phase::idle};
    NativeBrightPassInitializationAcquired acquired_;
    NativePostEffect20ConstructionBlock post_effect_;
    NativeString effect_name_,parameter_name_,last_parameter_name_;
};

// Complete 885-byte B54940..B54CB4. ECX existing224h owner; stack input
// holder,width,height,format; RET10h. Full post20(count3,null) -> +8, five
// borrowed scalar registrations at +210/+214/+218/+21C/+220, input texture
// -> material slot0, full holder(width,height,format,0,0,null) -> +C, primary
// -> post frame color0. Scalar bytes and +10..20F remain untouched.
// This source interface has explicit contexts/companion storage; it is not
// a drop-in native entry or a proof of native FH3/SEH behavior.
void initialize_native_bright_pass_00b54940(void*,std::size_t,
    NativeDepthDownscalePassArguments,NativeBrightPassInitializationBlock&);
} // namespace bsp
