#pragma once
#include "bsp/native_render_effect_lifetime.hpp"
#include "bsp/native_render_pass_initializers.hpp"
#include <array>

namespace bsp {
// Full B50D40[136], original ECX/EAX/RET. Capture CE6650 bits before any
// store; initialize only the native 250h owner's named fields. Arrays10..20F
// and padding24D..24F retain their preimages. The source adds a constant view.
void* construct_native_luminance_owner_00b50d40(void*,const volatile std::uint32_t&);
// Full B50DD0[526]: four holders228..234 use fresh decrement-import reads;
// then capture210 and the import once, release210/238/23C/214..224/248/244.
// Clear each nonnull slot after return; normal and armed unwind use full
// B0F5E0. Canonical post/holder/texture/surface domains only; no rollback.
void destroy_native_luminance_owner_00b50dd0(void*,NativeRenderEffectLifetimeContext&);
// Full B50FE0[30], repaired post-free tail; free iff flags bit0, return owner.
void* delete_native_luminance_owner_00b50fe0(void*,std::uint32_t,NativeRenderEffectLifetimeContext&);

struct NativeLuminanceInitializationContext {
    NativePostEffect20ConstructionContext& post_effects;
    NativeRenderTextureSurfaceOwnerContext& holders;
    NativeMaterialParameterAccess& parameters;
    // D5E1D0,D62050,D6203C,D6203C,D62024,D62008, in creation order.
    std::array<const char*,6> effect_names;
    const char* sample_offsets_name_00d5e40c;
    const char* adaptation_name_00d61ff4;
};
struct NativeLuminanceInitializationAcquired {
    void* receiver{};
    std::array<void*,6> raw_holders{},returned_holders{},raw_posts{},returned_posts{};
    std::array<bool,6> holders_published{},posts_published{},holder_free_started{},post_free_started{},completed_posts_preserved{};
    std::array<NativeRenderTextureSurfaceOwnerAcquired,6> holders;
    std::uint32_t current_holder{},current_post{},name_mask{},name_returns_started{};
    NativeString* common_name_header{};
    NativeString* adaptation_name_header{};
    NativeString* last_return_header{};
    NativeRuntimeTextureCreationArguments readback_arguments{1,1,1,114,2};
    NativeRuntimeTextureCreationAcquired readback_texture;
    NativeTextureSurfaceGetterAcquired readback_surface;
    int native_state{-1};
    bool completed{};
};
class NativeLuminanceInitializationBlock final {
public:
    enum class Phase { idle,preparing,prepared,executing,settled };
    NativeLuminanceInitializationBlock() noexcept=default;
    ~NativeLuminanceInitializationBlock() noexcept;
    NativeLuminanceInitializationBlock(const NativeLuminanceInitializationBlock&)=delete;
    NativeLuminanceInitializationBlock& operator=(const NativeLuminanceInitializationBlock&)=delete;
    void prepare(NativeLuminanceInitializationContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept {return phase_;}
    const NativeLuminanceInitializationAcquired& acquired() const noexcept {return acquired_;}
    NativePostEffect20ConstructionBlock& post_effect(std::size_t index) {return posts_.at(index);}
    // No releases: external disposition/quiescence and all six post blocks
    // reset to idle are required before resetting this persistent host block.
    void reset_after_host_quiescence() noexcept;
private:
    friend void initialize_native_luminance_00b51090(void*,std::size_t,const void*,NativeLuminanceInitializationBlock&);
    void create_post(std::uint32_t,int);
    void build_name(NativeString&,const char*,std::uint32_t);
    void return_name(NativeString&);
    void bind_sample(std::uint32_t,const void*,std::uint32_t,int);
    void unwind();
    void settle() noexcept;
    NativeLuminanceInitializationContext* context_{};
    Phase phase_{Phase::idle};
    NativeLuminanceInitializationAcquired acquired_;
    std::array<NativePostEffect20ConstructionBlock,6> posts_;
    NativeString common_name_,adaptation_name_;
};
// Complete B51090[2109], original ECX owner, stack input holder, RET4.
// Six R32F holders(1/4/16/64/1/1), six post20s, four shared16-float4
// registrations at+10, scalar at240, and actual system-memory1x1 texture
// plus retained surface244/248. Offsets/weights and adaptation value untouched.
// New source interface; full runtime/native FH3/SEH remain separate evidence.
void initialize_native_luminance_00b51090(void*,std::size_t,const void*,NativeLuminanceInitializationBlock&);
} // namespace bsp
