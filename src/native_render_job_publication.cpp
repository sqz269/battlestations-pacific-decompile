#include "bsp/native_render_job_publication.hpp"
#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/native_render_preparation_job_owner.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t profile;
    TrackedCriticalSection* section;
};
static_assert(sizeof(NativeGuard)==8);
static_assert(offsetof(TrackedCriticalSection,depth)==0x18);
static_assert(sizeof(NativeRenderPreparationJobStorage)==8);
static_assert(offsetof(NativeRenderPreparationJobStorage,secondary_04)==4);
std::uint32_t word(const void* p, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(p)+offset);
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(p)+offset)=value;
}
NativeGuard enter(NativeRenderJobPublicationContext& c) {
    void* const manager=get_native_singleton_manager_00415350(c.actual_manager_01090aa0);
    auto* const section=reinterpret_cast<TrackedCriticalSection*>(word(manager,0x10));
    NativeGuard result{0x00ce37fc,section};
    if(section) {
        EnterCriticalSection(&section->native);
        put(section,0x18,word(section,0x18)+1u);
    }
    return result; // cleanup arms only after native entry/depth increment
}
void leave(const NativeGuard& guard) {
    if(guard.section) {
        put(guard.section,0x18,word(guard.section,0x18)-1u);
        LeaveCriticalSection(&guard.section->native);
    }
}
void unwind(NativeGuard& guard) noexcept {
    try { destroy_native_singleton_guard_00411ee0(&guard); }
    catch(...) { std::terminate(); }
}
} // namespace

NativeFrameJobOwnerStorage* get_native_frame_jobs_actual_004c1130(
    NativeRenderJobPublicationContext& c) {
    auto* const first=c.actual_frame_0109cf08;
    if(first) return first;
    NativeGuard guard=enter(c);
    try {
        if(!c.actual_frame_0109cf08) {
            void* const raw=singleton_lifetime_allocate({
                SingletonAllocationKind::object,0x138a8,sizeof(NativeFrameJobOwnerStorage)});
            NativeFrameJobOwnerStorage* created=nullptr;
            try {
                if(raw) {
                    auto* const owner=::new(raw) NativeFrameJobOwnerStorage;
                    try {
                        if(!c.frame_lifetime)
                            throw std::logic_error("Raw frame-job construction requires its actual lifetime bindings");
                        created=construct_native_frame_job_owner_004bfa40(
                            *owner,c.actual_frame_0109cf08,*c.frame_lifetime);
                    } catch(...) {
                        owner->~NativeFrameJobOwnerStorage();
                        throw;
                    }
                }
            } catch(...) { singleton_lifetime_free(raw); throw; }
            c.actual_frame_0109cf08=created;
            void* const manager=get_native_singleton_manager_00415350(c.actual_manager_01090aa0);
            auto* const current=c.actual_frame_0109cf08;
            register_native_singleton_object_00bd0c30(manager,nullptr,current);
        }
        leave(guard); // original state0 remains armed through Leave
    } catch(...) { unwind(guard); throw; }
    return c.actual_frame_0109cf08;
}

NativeRenderPreparationJobStorage* get_native_preparation_job_actual_00b0ffb0(
    NativeRenderJobPublicationContext& c) {
    auto* const first=c.actual_preparation_00f8d444;
    if(first) return first;
    NativeGuard guard=enter(c);
    try {
        if(!c.actual_preparation_00f8d444) {
            void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,8,8});
            NativeRenderPreparationJobStorage* created=nullptr;
            if(raw) {
                created=::new(raw) NativeRenderPreparationJobStorage;
                created->secondary_04.native_vtable_00=0x00d5e154;
                created->native_vtable_00=0x00d5e160;
                created->secondary_04.native_vtable_00=0x00d5e15c;
            }
            c.actual_preparation_00f8d444=created;
            auto* const current=c.actual_preparation_00f8d444;
            auto* const secondary=current?&current->secondary_04:nullptr;
            void* const manager=get_native_singleton_manager_00415350(c.actual_manager_01090aa0);
            register_native_singleton_object_00bd0c30(manager,nullptr,secondary);
        }
        leave(guard);
    } catch(...) { unwind(guard); throw; }
    return c.actual_preparation_00f8d444;
}

void destroy_native_preparation_job_actual_00b0f1d0(
    NativeRenderPreparationJobStorage* owner, NativeRenderJobPublicationContext& c) noexcept {
    c.actual_preparation_00f8d444=nullptr;
    put(owner?&owner->secondary_04:nullptr,0,0x00ce3818);
}
NativeRenderPreparationJobStorage* delete_native_preparation_job_actual_00b0f210(
    NativeRenderPreparationJobStorage* owner, std::uint32_t flags,
    NativeRenderJobPublicationContext& c) noexcept {
    destroy_native_preparation_job_actual_00b0f1d0(owner,c);
    if(flags&1u) singleton_lifetime_free(owner);
    return owner;
}
NativeRenderPreparationJobStorage* delete_native_preparation_job_secondary_actual_00b0f1c0(
    NativeRenderPreparationJobSecondary* secondary, std::uint32_t flags,
    NativeRenderJobPublicationContext& c) noexcept {
    auto* const owner=reinterpret_cast<NativeRenderPreparationJobStorage*>(
        reinterpret_cast<std::uintptr_t>(secondary)-4u);
    return delete_native_preparation_job_actual_00b0f210(owner,flags,c);
}
NativeRenderPreparationJobSecondary* delete_native_preparation_job_base_actual_00b0d930(
    NativeRenderPreparationJobSecondary* base, std::uint32_t flags,
    NativeRenderJobPublicationContext& c) noexcept {
    c.actual_preparation_00f8d444=nullptr;
    put(base,0,0x00ce3818);
    if(flags&1u) singleton_lifetime_free(base);
    return base;
}
} // namespace bsp
