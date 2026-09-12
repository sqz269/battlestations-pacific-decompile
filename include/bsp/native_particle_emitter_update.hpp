#pragma once
#include "bsp/native_particle_emitter_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_point_light_links.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Reuse the SAME physical point-light backlinks and canonical node lifetimes
// for B04F00. The remaining inherited definition-current1C capture and actual
//72B740 getter stay required abstract operations; no substitute owner or lock.
class NativeParticleEmitterActualCleanupBindings : public NativeParticleEmitterCleanupBindings {
public:
    NativeParticleEmitterActualCleanupBindings(NativePointLightLinksRuntime&,
        GeneratedModelLifetimeRuntime&) noexcept;
    void call_00b7c160(void* actual_light) final;
    void call_00b6dfa0(void* actual_node) final;
private:
    NativePointLightLinksRuntime& links_;
    GeneratedModelLifetimeRuntime& nodes_;
};

// Borrow the existing actual cleanup/retained-owner/CRT services. The current
// derived container table has two DWORDs; its native identity is D5DF24 and
// current deleting slot04 must be B057A0. An unsupported current profile is an
// error, never a no-op terminal. Every reached pointer needs actual backing.
struct NativeParticleEmitterUpdateBindings {
    NativeParticleEmitterCleanupBindings* cleanup;
    NativeRenderActualOwners* retained_owners;
    const CameraAxesCrtAccess* crt;
    const volatile std::uint32_t* vtable_00d5df24;
};

// Complete AFF640..AFF689. ECX actual28h emitter, stack float delta, RET4/EAX
// active count; EDX adds borrowed bindings. Calls real B05110, publishes20,
// reloads CURRENT container10 on zero, calls current04(1), clears10 AFTER its
// callback and reloads20. Null container bypasses float loading and writes20=0.
std::int32_t __fastcall update_native_particle_emitter_00aff640(
    void* actual_emitter, const NativeParticleEmitterUpdateBindings*, float delta);
// Complete B05110..B05206. ECX actual30h container, stack delta, RET4/EAX live
// signed count1C; EDX adds bindings.6Ch state clocks, type3 AF1EB0, expiry via
// actual B04F00(0), row rotation/retry, and current model1F0/1F4/1F8 increments.
std::int32_t __fastcall update_native_particle_emitter_container_00b05110(
    NativeParticleEmitterContainer*, const NativeParticleEmitterUpdateBindings*, float delta);

// Complete AF1EB0..AF22AC. ECX actual generated-model-derived submodel, stack
// delta, RET4; EDX adds same bindings for original ST0 CRT sqrt. Retains frame
// clocks, definition animation fields,14h point-record walk, x87 spills,
// signed remainder and real geometry/model/element bounds writes.
void __fastcall update_native_particle_submodel_00af1eb0(
    void* actual_model, const NativeParticleEmitterUpdateBindings*, float delta);
// Complete B04990..B04A6B. ECX actual row header, stack(first,last), RET8.
// Rotate first row identity/value through inclusive last, keeping EACH row's
// padding WORD2. Intermediate values use x87, final saved value uses MOVSS.
void __fastcall rotate_native_particle_emitter_row_00b04990(
    NativeParticleEmitterRows*, void* unused_edx, std::int32_t first, std::int32_t last);
// Complete B72650..B72680: ECX geometry, stack four-float bounds, RET4.
// Publish dirty80=1 then four forward x87 stores84..90; preserve alias order.
void __fastcall set_native_particle_geometry_bounds_00b72650(
    void* actual_geometry, void* unused_edx, const float* bounds);

// Complete B04FC0..B0506E, incorrectly named vector deleting dtor in Ghidra:
// ECX actual container, RET, no flags. Publish D5DF20; signed live-count walk
// B04F00(state,1), then free states and cookie rows. Retain native count1C.
void destroy_native_particle_emitter_base_00b04fc0(
    NativeParticleEmitterContainer&, NativeParticleEmitterCleanupBindings&);
// Complete B05730..B0579F: derived profile, release current2C through SAME
// actual+04/current0 owner binding, clear2C after callback, full base destructor.
// Base executes on C++ unwind; a cleanup exception terminates.
void destroy_native_particle_emitter_container_00b05730(
    NativeParticleEmitterContainer&, const NativeParticleEmitterUpdateBindings&);
// Complete B057A0..B057BD: ECX owner, stack flags, RET4/EAX original address.
// Ordinary actual CRT free iff flags&1 after successful derived destruction.
NativeParticleEmitterContainer* delete_native_particle_emitter_container_00b057a0(
    NativeParticleEmitterContainer*, std::uint32_t flags,
    const NativeParticleEmitterUpdateBindings&);
// Reconstructed C++ interfaces, not original vtables/FH3/SEH ABI or game proof.
} // namespace bsp
