#include "bsp/native_dyn_convex_pool.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Dyn convex pool requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t profile=0x00d7a1ec;
NativeDynConvexPool* bound_pool{};
static_assert(sizeof(DynConvexShapePoolStorage)==0x38);
static_assert(offsetof(DynConvexShapePoolStorage,previous)==4);
static_assert(offsetof(DynConvexShapePoolStorage,next)==8);
static_assert(offsetof(DynConvexShapePoolStorage,critical_section)==0xc);
static_assert(offsetof(DynConvexShapePoolStorage,lock_depth)==0x24);
static_assert(offsetof(DynConvexShapePoolStorage,pages)==0x28);
static_assert(offsetof(DynConvexShapePoolStorage,page_count)==0x2c);
static_assert(offsetof(DynConvexShapePoolStorage,page_capacity)==0x30);
static_assert(offsetof(DynConvexShapePoolStorage,first_free_page)==0x34);
std::uint16_t free_count(void* page){return *reinterpret_cast<std::uint16_t*>(static_cast<unsigned char*>(page)+0x10d00);}
NativeDynConvexPool& actual_pool(){if(!bound_pool)throw std::logic_error("Dyn convex pool requires actual process binding");return *bound_pool;}
}
AllocatorListElement& NativeDynConvexPool::element() noexcept {
    return *reinterpret_cast<AllocatorListElement*>(&storage_);
}
NativeDynConvexPool::NativeDynConvexPool(AllocatorListDomain& list,
    DynConvexShapePoolStorage& storage,const AvoidZoneDynHullMemory& memory)
    :list_(list),storage_(storage),memory_(memory) {
    if(!memory_.allocate||!memory_.release)throw std::invalid_argument("Dyn convex pool requires matching allocation services");
    list_.bind_virtual0(element(),{profile,0x00407e30,this,&invoke_trim});
}
DynConvexShapePoolStorage& NativeDynConvexPool::initialize_00407c70(){
    auto& p=storage_;
    // Same fresh-storage constructor as dyn_convex_pool_construct_00407c70,
    // publishing directly to the shared domain before the allocation callback.
    list_.prepend_element_links(element());p.vtable=reinterpret_cast<const void*>(profile);
    InitializeCriticalSection(&p.critical_section);
    p.lock_depth=0;p.pages=nullptr;p.page_count=0;
    p.first_free_page=0xffffffffu;p.page_capacity=32;
    p.pages=static_cast<void**>(memory_.allocate(memory_.context,128));
    return p;
}
void NativeDynConvexPool::invoke_trim(void* p){static_cast<NativeDynConvexPool*>(p)->trim_empty_pages_00407e30();}
void NativeDynConvexPool::trim_empty_pages_00407e30(){
    auto& p=storage_;
    for(std::uint32_t i=0;i<p.page_count;++i){
        if(free_count(p.pages[i])!=128)continue;
        memory_.release(memory_.context,p.pages[i]); // 407E52
        // Reload both table and count after free; retain the stale tail word.
        p.pages[i]=p.pages[p.page_count-1];--p.page_count;
        if(i<p.page_count){
            auto* index=static_cast<unsigned char*>(p.pages[i])+0x214;
            for(std::uint32_t n=128;n;--n,index+=0x218)
                *reinterpret_cast<std::uint32_t*>(index)=i;
        }
        --i; // Unsigned wrap retains the native SUB/ADD loop schedule at zero.
    }
    p.first_free_page=0xffffffffu;
    for(std::uint32_t i=0;i<p.page_count;++i)
        if(free_count(p.pages[i])!=0){p.first_free_page=i;break;}
}
void NativeDynConvexPool::destroy_00407d70(){
    auto& p=storage_;p.vtable=reinterpret_cast<const void*>(profile);
    for(std::uint32_t i=0;i<p.page_count;++i)memory_.release(memory_.context,p.pages[i]);
    if(p.pages)memory_.release(memory_.context,p.pages);
    while(static_cast<std::int32_t>(p.lock_depth)>0){--p.lock_depth;LeaveCriticalSection(&p.critical_section);}
    DeleteCriticalSection(&p.critical_section);
    list_.unlink_base_element_00403970(element());
    list_.unbind_virtual0(element());
}
void bind_static_native_dyn_convex_pool_0109ecf0(NativeDynConvexPool& p){
    if(bound_pool&&bound_pool!=&p)throw std::logic_error("Dyn convex pool cannot be rebound");
    bound_pool=&p;
}
int initialize_static_native_dyn_convex_pool_00cc89c0(int (*registration)(void (*)())){
    if(!registration)throw std::invalid_argument("Dyn convex pool requires CRT atexit");
    actual_pool().initialize_00407c70();return registration(&destroy_static_native_dyn_convex_pool_00cd9240);
}
void destroy_static_native_dyn_convex_pool_00cd9240(){actual_pool().destroy_00407d70();}
} // namespace bsp
