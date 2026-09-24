#pragma once
#include "bsp/native_render_resources_lifetime.hpp"
#include "bsp/native_runtime_texture_creation.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include "bsp/native_shadow_texture_access.hpp"
#include <memory>

namespace bsp {
// Genuine 2Ch owner. +04/+08 are dimensions, NOT a reference count. The
// constructor never initializes padding0F/21..23. No native C++ vtable overlay.
struct NativeShadowDepthTargetStorage {
    std::uint32_t profile_00, width_04, height_08;
    std::uint8_t enabled_0c, created_0d, policy_0e, preserved_0f;
    void* color_texture_10;
    NativeSurfaceOwnerStorage* color_surface_14;
    void* depth_texture_18;
    NativeSurfaceOwnerStorage* depth_surface_1c;
    std::uint8_t attempted_20, preserved_21[3];
    std::uint32_t color_format_24, depth_format_28;
};
static_assert(sizeof(NativeShadowDepthTargetStorage)==0x2c);
static_assert(offsetof(NativeShadowDepthTargetStorage,policy_0e)==0x0e);
static_assert(offsetof(NativeShadowDepthTargetStorage,color_texture_10)==0x10);
static_assert(offsetof(NativeShadowDepthTargetStorage,attempted_20)==0x20);
static_assert(offsetof(NativeShadowDepthTargetStorage,depth_format_28)==0x28);
struct NativeShadowDepthTargetDimensions { std::uint32_t width, height; };
using NativeShadowTargetSectionCall=void (__stdcall*)(void* actual_section);

// Borrow exact current publication/import cells and original numeric tables.
// Runtime/getter/terminal providers must share the same actual renderer,
// surface/texture pools, strings, support, counters and canonical registry.
// The direct domain admits only genuine completed producer storage with valid
// cleanup fields/pool metadata. Nested direct surface owners remain unbound.
struct NativeShadowDepthTargetBindings {
    void* volatile& manager_01090aa0;
    void* volatile& publication_00f8bbf0;
    void* const volatile& renderer_00f8d394;
    NativeRuntimeTextureCreationContext& textures;
    NativeTextureSurfaceGetterContext& surfaces;
    NativeRenderResourcesLifetimeContext& terminals;
    NativeRenderResourcesDirectTerminalDomain& direct;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    const volatile std::uint32_t* texture_profile_00d61948;
    NativeShadowTargetSectionCall const volatile& enter_00ce2218;
    NativeShadowTargetSectionCall const volatile& leave_00ce2210;
};
enum class NativeShadowDepthTargetCall {
    base_construct, base_destroy, extents_construct, extents_destroy,
    construct, create, destroy, scalar_delete
};
struct NativeShadowDepthTargetOperation final {
    enum class Phase { running, complete, failed };
    Phase phase{Phase::running};
    NativeShadowDepthTargetCall call;
    void* receiver{};
    std::uint32_t native_site{}, exception_state{0xffffffffu};
    std::uint32_t singleton_exception_state{0xffffffffu};
    void* captured_section{};
    bool section_entered{}, section_incremented{}, publication_written{};
    void* registration_argument{};
    void* captured_child{};
    NativeRenderServiceTextureDecrement captured_decrement{};
    NativeShadowTargetSectionCall captured_enter{}, captured_leave{};
    NativeShadowDepthTargetDimensions saved_dimensions;
    NativeRuntimeTextureCreationArguments color_arguments, depth_arguments;
    NativeRuntimeTextureCreationAcquired color_texture, depth_texture;
    NativeTextureSurfaceGetterAcquired color_surface, depth_surface;
    explicit NativeShadowDepthTargetOperation(NativeShadowDepthTargetCall,void*) noexcept;
    ~NativeShadowDepthTargetOperation();
};

// One retained host context, no native owner/count/registry. It attaches each
// address-stable invocation before native work, including calls from singleton
// drain. Failed construction/registration/destruction keeps its actual lock,
// publications and acquired child blocks alive. No retry/rollback/settlement.
// Destroying a context with a running/failed call terminates. Completed records
// are metadata only; retain the context/providers through native retirement.
class NativeShadowDepthTargetContext final {
public:
    explicit NativeShadowDepthTargetContext(NativeShadowDepthTargetBindings);
    ~NativeShadowDepthTargetContext();
    NativeShadowDepthTargetContext(const NativeShadowDepthTargetContext&)=delete;
    NativeShadowDepthTargetContext& operator=(const NativeShadowDepthTargetContext&)=delete;
    const NativeShadowDepthTargetBindings bindings;
    const NativeShadowDepthTargetOperation* operation(std::size_t) const noexcept;
    std::size_t operation_count() const noexcept;
    // Host metadata attachment only, used by the eight native entry adapters.
    NativeShadowDepthTargetOperation& begin(NativeShadowDepthTargetCall,void*);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Complete A8A820/A8A8C0 normal paths, ECX actual owner, RET. Capture first
// 415350 manager+10 section; real enter/+18++, then publish/register or
// unregister CURRENT publication after the second getter. Clear after removal;
// --/leave original section. Destructor receiver need not equal publication.
void* construct_native_shadow_target_singleton_00a8a820(void*,NativeShadowDepthTargetContext&);
void destroy_native_shadow_target_singleton_00a8a8c0(void*,NativeShadowDepthTargetContext&);
// A8A980: base call before width/height argument-cell reads; EAX owner, RET8.
// A8A9D0: stamp D5B558 then tail A8A8C0. No implicit allocation or free.
void* construct_native_shadow_target_extents_00a8a980(void*,
    const volatile NativeShadowDepthTargetDimensions&,NativeShadowDepthTargetContext&);
void destroy_native_shadow_target_extents_00a8a9d0(void*,NativeShadowDepthTargetContext&);
// A8FE30: ECX fresh2Ch, width/height stack, EAX owner, RET8. Full native writes
// and current +104/+F8 probes; real B1FF50 and B21EC0. Padding stays untouched.
NativeShadowDepthTargetStorage* construct_native_shadow_depth_target_00a8fe30(void*,
    const volatile NativeShadowDepthTargetDimensions&,NativeShadowDepthTargetContext&);
// A8FF30: ECX target, raw low-byte enable, RET4. Native attempted20 gate,
// exact extent/format captures; real B2A070 and B3FD80 twice with retained
// acquisitions. No additional credits/blanket registrations/null defaults.
void create_native_shadow_depth_target_00a8ff30(NativeShadowDepthTargetStorage&,
    std::uint8_t enabled,NativeShadowDepthTargetContext&);
// A8FFF0: ECX target, RET. Capture14 then ONE current CE2220 epoch, release
// current14/10/1C/18, actual+04 decrement/current0->fresh4 only at zero, clear
// after callback. Then A8A9D0. A900C0: destroy, shared free iff flags&1, RET4.
void destroy_native_shadow_depth_target_00a8fff0(NativeShadowDepthTargetStorage&,
    NativeShadowDepthTargetContext&);
void* delete_native_shadow_depth_target_00a900c0(void*,std::uint32_t flags,
    NativeShadowDepthTargetContext&);
// Borrow the real fields for existing A8FDD0/A8FD90/A8FDB0 adapters. Resolver
// entries must alias the exact produced runtime textures, never copied fields.
NativeShadowDepthTargetFields view_native_shadow_depth_target(NativeShadowDepthTargetStorage&) noexcept;
// Only final D5B5E8 is admitted by singleton drain. Partial base profiles,
// original EH guard transport/cleanup and binary ABI remain separate contracts.
} // namespace bsp
