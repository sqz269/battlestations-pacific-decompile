#include "bsp/native_loading_queue_retirement.hpp"
#include "bsp/native_fileblock_owner.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_filestore_provider_lifetime.hpp"
#include "bsp/native_filestore_remove.hpp"
#include "bsp/native_loading_queue_work_items.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native loading queue retirement requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p,U n=0) noexcept { return ptr(bits(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(at(p,n))=v; }
std::int32_t signed_word(U v) noexcept { std::int32_t result;std::memcpy(&result,&v,sizeof(result));return result; }
}
struct NativeLoadingQueueRetireAcquired::Impl {
    NativeLoadingQueueRetirePhase phase=NativeLoadingQueueRetirePhase::fresh;
    U active=0,failure=0;
    void* first=nullptr;void* block=nullptr;void* table=nullptr;U target=0;
    void* factory=nullptr;void* provider=nullptr;const void* name=nullptr;
    void* slot=nullptr;void* destroyed=nullptr;
    NativeFileBlockOwnerAcquired nested;
};
NativeLoadingQueueRetireAcquired::NativeLoadingQueueRetireAcquired():impl_(std::make_unique<Impl>()) {}
NativeLoadingQueueRetireAcquired::~NativeLoadingQueueRetireAcquired() {
    if(impl_->phase!=NativeLoadingQueueRetirePhase::fresh && impl_->phase!=NativeLoadingQueueRetirePhase::complete) std::terminate();
}
NativeLoadingQueueRetirePhase NativeLoadingQueueRetireAcquired::phase() const noexcept { return impl_->phase; }
U NativeLoadingQueueRetireAcquired::failure_site() const noexcept { return impl_->failure; }
const void* NativeLoadingQueueRetireAcquired::initial_job() const noexcept { return impl_->first; }
const void* NativeLoadingQueueRetireAcquired::job_selected_for_destruction() const noexcept { return impl_->destroyed; }
const void* NativeLoadingQueueRetireAcquired::captured_front_slot() const noexcept { return impl_->slot; }
const NativeFileBlockOwnerAcquired& NativeLoadingQueueRetireAcquired::fileblock_invocation() const noexcept { return impl_->nested; }
void retire_native_loading_queue_front_00506bf0(void* loader,NativeLoadingQueueRetireContext& context,NativeLoadingQueueRetireAcquired& acquired) {
    auto& f=*acquired.impl_;
    if(f.phase!=NativeLoadingQueueRetirePhase::fresh) throw std::logic_error("Native loading queue retirement cannot replay");
    f.phase=NativeLoadingQueueRetirePhase::reading_front;
    try {
        f.first=ptr(word(ptr(word(loader,0x10))));f.block=ptr(word(f.first,0x20));
        if(f.block) {
            f.table=ptr(word(f.block));f.target=word(f.table,4);
            f.phase=NativeLoadingQueueRetirePhase::deleting_fileblock;f.active=0x00506c07;
            if(f.target!=0x00bdebe0) throw std::runtime_error("Native loading queue FileBlock deleting target is not reconstructed");
            delete_native_fileblock_00bdebe0(f.block,1,context.fileblocks,f.nested);
            put(f.first,0x20,0);
        }
        f.phase=NativeLoadingQueueRetirePhase::getting_factory;f.active=0x00506c11;
        f.factory=get_native_filestore_factory_004fc150(context.factories);
        f.name=at(f.first,0x14);f.phase=NativeLoadingQueueRetirePhase::getting_provider;f.active=0x00506c1c;
        f.provider=get_native_file_store_provider_00be80b0(f.factory,context.providers);
        f.phase=NativeLoadingQueueRetirePhase::removing_file;f.active=0x00506c23;
        remove_native_file_store_file_00be7130(f.provider,f.name,context.providers.strings,context.providers.streams,context.providers.invalid_parameters);
        f.slot=ptr(word(loader,0x10));f.destroyed=ptr(word(f.slot));
        if(f.destroyed) {
            f.phase=NativeLoadingQueueRetirePhase::destroying_job;f.active=0x00506c33;
            destroy_native_loading_job_storage_005051a0(f.destroyed,context.raw_strings);
            f.phase=NativeLoadingQueueRetirePhase::freeing_job;f.active=0x00506c39;
            singleton_lifetime_free(f.destroyed);put(f.slot,0,0);
        }
        f.phase=NativeLoadingQueueRetirePhase::shifting;
        U index=0;
        if(signed_word(word(loader,0x14)-1u)>0) {
            do {
                const auto current_array=word(loader,0x10);auto* const destination=ptr(current_array+index*4u);
                const auto next=word(destination,4);put(destination,0,next);
                const auto limit=word(loader,0x14)-1u;++index;
                if(signed_word(index)>=signed_word(limit)) break;
            } while(true);
        }
        put(loader,0x14,word(loader,0x14)-1u);
        f.active=0;f.phase=NativeLoadingQueueRetirePhase::complete;
    } catch(...) { f.failure=f.active;f.phase=NativeLoadingQueueRetirePhase::failed;throw; }
}
} // namespace bsp
