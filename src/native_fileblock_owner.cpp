#include "bsp/native_fileblock_owner.hpp"
#include "bsp/native_fileblock_identifier.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_fileblock_scope.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileBlock ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U=std::uint32_t;
void* at(const void* p,U n=0) noexcept { return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U value) noexcept { *static_cast<volatile U*>(at(p,n))=value; }
void* pointer(const void* p,U n=0) noexcept { return reinterpret_cast<void*>(word(p,n)); }
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void return_captured(void* data,U bytes,NativeStringRawPoolContext& raw,U& active,U getter_site,U return_site) {
    active=getter_site;
    auto* const pool=native_string_pool_get_or_create_00419cc0(raw.actual_published_01090aa8,raw.actual_manager_publication_01090aa0);
    active=return_site;
    return_native_string_pool_00bd1510(pool,data,bytes,raw.actual_small_returns_disabled_01090aa4);
}
}

struct NativeFileBlockOwnerAcquired::Impl {
    NativeFileBlockOwnerPhase phase=NativeFileBlockOwnerPhase::fresh;
    U active=0,failure=0;
    std::int32_t native_state=-1;
    void* owner=nullptr;
    const void* input=nullptr;
    void* manager=nullptr;
    void* name_data=nullptr;
    U name_bytes=0;
    NativeVfsFileBlockScopeAcquired scope;
    void begin(void* p) {
        if(phase!=NativeFileBlockOwnerPhase::fresh) throw std::logic_error("Native FileBlock invocation cannot replay");
        owner=p;
    }
    void complete() noexcept { active=0;phase=NativeFileBlockOwnerPhase::complete; }
    void failed() noexcept { failure=active;phase=NativeFileBlockOwnerPhase::failed; }
    void destroy_body(NativeFileBlockOwnerContext& context) {
        put(owner,0,0x00d68494);
        manager=context.actual_vfs_publication_0109ceec;
        native_state=1;phase=NativeFileBlockOwnerPhase::leaving;active=0x00bdcb66;
        leave_native_vfs_fileblock_00bdc9b0(manager,at(owner,0x14),context.scopes,scope);
        name_data=pointer(owner,0x18); // BDCB6B before state1 -> state0.
        native_state=0;
        if(name_data) {
            name_bytes=word(owner,0x14)+1u;phase=NativeFileBlockOwnerPhase::returning_name;
            return_captured(name_data,name_bytes,context.raw_pool,active,0x00bdcb80,0x00bdcb87);
        }
        native_state=-1;phase=NativeFileBlockOwnerPhase::destroying_base;active=0x00bdcb96;
        destroy_native_ref_counted_base_00bd30f0(owner);
    }
};
NativeFileBlockOwnerAcquired::NativeFileBlockOwnerAcquired():impl_(std::make_unique<Impl>()) {}
NativeFileBlockOwnerAcquired::~NativeFileBlockOwnerAcquired() {
    if(impl_->phase!=NativeFileBlockOwnerPhase::fresh && impl_->phase!=NativeFileBlockOwnerPhase::complete) std::terminate();
}
NativeFileBlockOwnerPhase NativeFileBlockOwnerAcquired::phase() const noexcept { return impl_->phase; }
U NativeFileBlockOwnerAcquired::active_call_site() const noexcept { return impl_->active; }
U NativeFileBlockOwnerAcquired::failure_site() const noexcept { return impl_->failure; }
std::int32_t NativeFileBlockOwnerAcquired::native_state_at_last_call() const noexcept { return impl_->native_state; }
const NativeVfsFileBlockScopeAcquired& NativeFileBlockOwnerAcquired::scope_invocation() const noexcept { return impl_->scope; }

void* construct_native_fileblock_00be0a30(void* owner,const void* name,U gate,
    NativeFileBlockOwnerContext& context,NativeFileBlockOwnerAcquired& acquired) {
    auto& f=*acquired.impl_;f.begin(owner);f.input=name;
    try {
        put(owner,0,0x00ceb130);put(owner,4,1);put(owner,0,0x00d68494);
        auto* const output=at(owner,0x14);const bool identical=output==name;
        put(owner,8,0);put(owner,0x0c,0);put(owner,0x10,0);f.native_state=0;
        put(output,0,0);put(output,4,0);
        f.phase=NativeFileBlockOwnerPhase::copying_name;
        if(!identical) {
            const auto length=word(name);f.active=0x00be0a8a;
            resize_native_string_header_0041dd40(output,context.strings,length,true);
            if(word(name)!=0) {
                const auto count=word(output);
                auto* const destination=pointer(output,4); // native captures destination before source
                const auto* const source=pointer(name,4);f.active=0x00be0a9f;
                // Native BF7680 also implements the backward-overlap path.
                if(count!=0) std::memmove(destination,source,count);
            }
        }
        f.native_state=1;f.phase=NativeFileBlockOwnerPhase::identifying;f.active=0x00be0aae;
        prepare_native_fileblock_identifier_00bdf950(owner,context.strings);
        f.manager=context.actual_vfs_publication_0109ceec;
        f.phase=NativeFileBlockOwnerPhase::entering;f.active=0x00be0abf;
        enter_native_vfs_fileblock_00be0980(f.manager,output,static_cast<std::uint8_t>(gate),context.scopes,f.scope);
        f.complete();return owner;
    } catch(...) { f.failed();throw; }
}
void destroy_native_fileblock_00bdcb30(void* owner,NativeFileBlockOwnerContext& context,NativeFileBlockOwnerAcquired& acquired) {
    auto& f=*acquired.impl_;f.begin(owner);
    try { f.destroy_body(context);f.complete(); } catch(...) { f.failed();throw; }
}
void* delete_native_fileblock_00bdebe0(void* owner,U flags,NativeFileBlockOwnerContext& context,NativeFileBlockOwnerAcquired& acquired) {
    auto& f=*acquired.impl_;f.begin(owner);
    try {
        f.destroy_body(context);
        if((flags&1u)!=0) { f.phase=NativeFileBlockOwnerPhase::freeing_owner;f.active=0x00bdebf0;singleton_lifetime_free(owner); }
        f.complete();return owner;
    } catch(...) { f.failed();throw; }
}

struct NativeLoadingJobFileBlockAcquired::Impl {
    NativeLoadingJobFileBlockPhase phase=NativeLoadingJobFileBlockPhase::fresh;
    U active=0,failure=0;
    void* job=nullptr;
    const void* input=nullptr;
    void* allocation=nullptr;
    void* result=nullptr;
    U temporary[2];
    bool temporary_armed=false;
    std::int32_t native_state=-1;
    void* name_data=nullptr;
    U name_bytes=0;
    NativeFileBlockOwnerAcquired nested;
    void complete() noexcept { active=0;phase=NativeLoadingJobFileBlockPhase::complete; }
};
NativeLoadingJobFileBlockAcquired::NativeLoadingJobFileBlockAcquired():impl_(std::make_unique<Impl>()) {}
NativeLoadingJobFileBlockAcquired::~NativeLoadingJobFileBlockAcquired() {
    if(impl_->phase!=NativeLoadingJobFileBlockPhase::fresh && impl_->phase!=NativeLoadingJobFileBlockPhase::complete) std::terminate();
}
NativeLoadingJobFileBlockPhase NativeLoadingJobFileBlockAcquired::phase() const noexcept { return impl_->phase; }
U NativeLoadingJobFileBlockAcquired::active_call_site() const noexcept { return impl_->active; }
U NativeLoadingJobFileBlockAcquired::failure_site() const noexcept { return impl_->failure; }
const void* NativeLoadingJobFileBlockAcquired::allocated_owner() const noexcept { return impl_->allocation; }
const void* NativeLoadingJobFileBlockAcquired::temporary_name_header() const noexcept { return impl_->temporary; }
const NativeFileBlockOwnerAcquired& NativeLoadingJobFileBlockAcquired::owner_invocation() const noexcept { return impl_->nested; }

void ensure_native_loading_job_fileblock_00504790(void* job,NativeFileBlockOwnerContext& context,NativeLoadingJobFileBlockAcquired& acquired) {
    auto& f=*acquired.impl_;
    if(f.phase!=NativeLoadingJobFileBlockPhase::fresh) throw std::logic_error("Native loading job FileBlock invocation cannot replay");
    f.job=job;
    try {
        if(word(job,0x20)!=0 || word(job,0x14)==0) { f.complete();return; }
        f.input=at(job,0x14);f.phase=NativeLoadingJobFileBlockPhase::allocating;f.active=0x005047c7;
        f.allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x1c,0x1c});
        f.native_state=0;
        if(f.allocation) {
            const auto count=word(f.input)-18u;
            f.phase=NativeLoadingJobFileBlockPhase::substring;f.active=0x005047ee;
            auto* const name=construct_native_string_substring_00469840(f.input,f.temporary,13,count,context.strings);
            f.temporary_armed=true;f.native_state=1;
            f.phase=NativeLoadingJobFileBlockPhase::constructing;f.active=0x00504805;
            f.result=construct_native_fileblock_00be0a30(f.allocation,name,1,context,f.nested);
        }
        f.phase=NativeLoadingJobFileBlockPhase::publishing;put(job,0x20,bits(f.result));f.native_state=-1;
        if(f.temporary_armed) {
            f.name_data=pointer(f.temporary,4);
            if(f.name_data) {
                f.name_bytes=word(f.temporary)+1u;f.phase=NativeLoadingJobFileBlockPhase::returning_name;
                return_captured(f.name_data,f.name_bytes,context.raw_pool,f.active,0x0050482e,0x00504835);
            }
        }
        f.complete();
    } catch(...) { f.failure=f.active;f.phase=NativeLoadingJobFileBlockPhase::failed;throw; }
}
} // namespace bsp
