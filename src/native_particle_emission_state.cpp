#include "bsp/native_particle_emission_state.hpp"
#include "bsp/native_particle_type_state_dispatch.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include <cstring>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
void* offset(void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
template<class T> T load(const void* p,std::uint32_t n=0) noexcept {
    T v;std::memcpy(&v,reinterpret_cast<const std::byte*>(p)+n,sizeof v);return v;
}
struct NativePopulationGuard {std::uint32_t profile;TrackedCriticalSection* section;};
static_assert(sizeof(NativePopulationGuard)==8);
volatile std::uint32_t& depth(TrackedCriticalSection* p) noexcept {
    return *static_cast<volatile std::uint32_t*>(offset(p,0x18));
}
void enter(TrackedCriticalSection* p) {
    if (p) {EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(p));auto& n=depth(p);n=n+1u;}
}
void leave(TrackedCriticalSection* p) noexcept {
    if (p) {auto& n=depth(p);n=n-1u;LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(p));}
}
}
void clear_native_particle_population_lock_base_00729420(
    NativeParticlePopulationLockStorage* p,
    NativeParticlePopulationLockStorage* volatile& publication) noexcept {
    publication=nullptr;
    p->native_profile_00=0x00ce3818u;
}
NativeParticlePopulationLockStorage* construct_native_particle_population_lock_0072a4f0(
    void* raw,NativeParticlePopulationLockStorage* volatile& publication) {
    auto* p=static_cast<NativeParticlePopulationLockStorage*>(raw);
    p->native_profile_00=0x00cfdeb4u;
    try {p->section_04=create_native_tracked_critical_section_00bd1860();}
    catch (...) {clear_native_particle_population_lock_base_00729420(p,publication);throw;}
    return p;
}
NativeParticlePopulationLockStorage* get_native_particle_population_lock_0072b740(
    void* volatile& manager_publication,
    NativeParticlePopulationLockStorage* volatile& publication) {
    auto* const initial=publication;
    if (initial) return initial;
    void* first_manager=get_native_singleton_manager_00415350(manager_publication);
    auto* const section=load<TrackedCriticalSection*>(first_manager,0x10);
    NativePopulationGuard guard{0x00ce37fcu,section};
    enter(section);
    try {
        if (!publication) {
            void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,8,8});
            NativeParticlePopulationLockStorage* result=nullptr;
            try {if (raw) result=construct_native_particle_population_lock_0072a4f0(raw,publication);}
            catch (...) {singleton_lifetime_free(raw);throw;}
            publication=result;
            void* current_manager=get_native_singleton_manager_00415350(manager_publication);
            auto* current=publication;
            register_native_singleton_object_00bd0c30(current_manager,nullptr,current);
        }
        leave(section);
    } catch (...) {destroy_native_singleton_guard_00411ee0(&guard);throw;}
    return publication;
}
NativeParticlePopulationLockStorage* delete_native_particle_population_lock_0072cc90(
    NativeParticlePopulationLockStorage* p,std::uint32_t flags,
    NativeParticlePopulationLockStorage* volatile& publication) {
    p->native_profile_00=0x00cfdeb4u;
    release_native_tracked_critical_section_0041cc80(&p->section_04);
    clear_native_particle_population_lock_base_00729420(p,publication);
    if (flags&1u) singleton_lifetime_free(p);
    return p;
}

void __fastcall initialize_native_particle_emission_state_00b0ca40(void* state,
    const NativeParticleEmissionStateAccess* access,void* definition,float,
    void* emitter,const float* position,const float* direction,const void* record) {
    __asm {
        mov esi,state
        mov eax,emitter
        mov ecx,definition
        xorps xmm0,xmm0
        mov dword ptr [esi+68h],eax
        mov dword ptr [esi+64h],ecx
        mov dword ptr [esi+60h],0
        movss dword ptr [esi+40h],xmm0
        mov eax,direction
        mov edi,position
        fld dword ptr [edi]
        fstp dword ptr [esi]
        fld dword ptr [edi+4]
        fstp dword ptr [esi+4]
        fld dword ptr [edi+8]
        fstp dword ptr [esi+8]
        fld dword ptr [esi]
        fstp dword ptr [esi+0ch]
        fld dword ptr [esi+4]
        fstp dword ptr [esi+10h]
        fld dword ptr [esi+8]
        fstp dword ptr [esi+14h]
        fld dword ptr [eax]
        fstp dword ptr [esi+18h]
        fld dword ptr [eax+4]
        fstp dword ptr [esi+1ch]
        fld dword ptr [eax+8]
        mov eax,record
        fstp dword ptr [esi+20h]
        movss xmm0,dword ptr [eax+24h]
        movss xmm1,dword ptr [eax+28h]
        movss xmm2,dword ptr [eax+2ch]
        movss dword ptr [esi+24h],xmm0
        movss dword ptr [esi+28h],xmm1
        movss dword ptr [esi+2ch],xmm2
    }
    void* current_definition=load<void*>(state,0x64);
    const void* table=load<const void*>(current_definition);
    const auto target=load<std::uint32_t>(table,0x18);
    if (!access->type_states || !dispatch_known_native_particle_type_state(
            *access->type_states,*access,current_definition,target,state,record)) {
        if (!access->particle_virtual18) throw std::logic_error("particle state requires actual captured particle virtual18");
        access->particle_virtual18(access->context,current_definition,target,state,record);
    }
    if (!load<void*>(state,0x60)) return;
    void* current_emitter=load<void*>(state,0x68);
    current_definition=load<void*>(state,0x64);
    void* model=load<void*>(current_emitter,8);
    auto& population=access->population;
    auto& attachment=population.nodes.scenes.resolve_key(reinterpret_cast<std::uint32_t>(model));
    initialize_native_particle_point_light_volume_00b0ca40_fragment(
        population.nodes.point_lights,*static_cast<void* volatile*>(offset(state,0x60)),
        attachment.transform,*static_cast<volatile std::uint8_t*>(offset(model,0x1b0)),
        position,*static_cast<volatile std::uint32_t*>(offset(current_definition,0x8c)),
        access->minimum_radius_00d7a238);
    auto* lock=get_native_particle_population_lock_0072b740(
        access->actual_manager_01090aa0,access->actual_lock_0108ff50);
    auto* const section=lock->section_04;
    NativePopulationGuard guard{0x00ce37fcu,section};
    enter(section);
    try {
        current_emitter=load<void*>(state,0x68);
        model=load<void*>(current_emitter,8);
        auto* roots=load<RenderNodeRootList*>(model,0xa4);
        void* actual_light=load<void*>(state,0x60);
        if (!access->resolve_light) throw std::logic_error("particle state requires canonical PointLight owner resolution");
        auto& light=access->resolve_light(access->context,actual_light);
        if (&light.node.storage!=actual_light || &light.environment.nodes!=&population.nodes ||
            &population.nodes.point_lights.light(actual_light)!=&light.backlinks)
            throw std::logic_error("particle state resolved a different PointLight owner/domain");
        // Native B7B090 returns for every nonzero backlink count before
        // reading the captured root pointer. Do not form a null reference on
        // that valid linked-light path. The population call rechecks the same
        // physical count while this same critical section remains held.
        if (light.light.backlinks_1e0.count==0u)
            populate_native_point_light_if_unlinked_00b7b090(population,light,*roots);
        leave(section);
    } catch (...) {destroy_native_singleton_guard_00411ee0(&guard);throw;}
}
} // namespace bsp
