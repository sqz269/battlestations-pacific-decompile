#include "bsp/native_resource_instance_lifecycle.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
template<class T> T& field(void* base, std::uint32_t offset=0) noexcept {
    return *static_cast<T*>(at(base, offset));
}
using Word=volatile std::uint32_t;
using Byte=volatile std::uint8_t;
using Pointer=void* volatile;
void clear_vector(void* object, std::uint32_t offset) noexcept {
    clear_native_singleton_storage_00bd0220(at(object,offset),nullptr);
}
void clear_map(void* map) noexcept {
    // The actual destructor supplies begin/end of this same valid map.
    // Thus B89980 necessarily takes its full-range branch (999F..99FE).
    clear_native_resource_instance_map_nodes_00b89150(map,nullptr,
        field<Pointer>(field<Pointer>(map,4),4));
    void* head=field<Pointer>(map,4);
    field<Pointer>(head,4)=head;
    head=field<Pointer>(map,4);
    field<Word>(map,8)=0;
    field<Pointer>(head)=head;
    head=field<Pointer>(map,4);
    field<Pointer>(head,8)=head;
    // B89DB0 and unwind B89D70 free CURRENT head, then zero head/count.
    singleton_lifetime_free(field<Pointer>(map,4));
    field<Pointer>(map,4)=nullptr;
    field<Word>(map,8)=0;
}
void unwind_base(void* object, int state) noexcept {
    if(state>=3) clear_map(at(object,0x30));
    if(state>=2) clear_vector(object,0x20);
    if(state>=1) clear_vector(object,0x10);
    if(state>=0) destroy_native_ref_counted_base_00bd30f0(object);
}
void unwind_game(void* object, const NativeResourceInstanceLifetimeAccess& access) noexcept {
    for(auto offset : {0x7cu,0x6cu,0x5cu,0x4cu,0x3cu}) clear_vector(object,offset);
    destroy_native_resource_instance_00b89db0(object,&access);
}
class InstanceDeletes final : public NativeRefCountedDeleteCalls {
public:
    explicit InstanceDeletes(NativeResourceInstanceLifetimeAccess& a) noexcept : access(a) {}
    void delete_vslot04(void* owner, std::uint32_t profile, std::uint32_t flags) override {
        if(profile==0x00cfd8e0) delete_native_game_resource_instance_00718d00(owner,&access,flags);
        else if(profile==0x00d63244) delete_native_resource_instance_00b89fc0(owner,&access,flags);
        else std::terminate(); // Outside the two evidenced profile bindings.
    }
private:
    NativeResourceInstanceLifetimeAccess& access;
};
std::uint32_t selected_slot(void*, std::uint32_t profile) noexcept {
    if(profile==0x00cfd8e0 || profile==0x00d63244) return 0x00bd30e0;
    std::terminate();
}
void selected_zero(void* context, std::uint32_t entry, void* owner) {
    if(entry!=0x00bd30e0) std::terminate();
    InstanceDeletes calls(*static_cast<NativeResourceInstanceLifetimeAccess*>(context));
    invoke_native_ref_counted_delete_00bd30e0(owner,calls);
}
} // namespace

void* allocate_native_resource_instance_map_head_00b87930() {
    void* const head=singleton_lifetime_allocate({SingletonAllocationKind::object,0x24,0x24});
    if(head) field<Pointer>(head)=nullptr;
    void* slot=at(head,4);
    if(slot) field<Pointer>(slot)=nullptr;
    slot=at(head,8);
    if(slot) field<Pointer>(slot)=nullptr;
    field<Byte>(head,0x20)=1;
    field<Byte>(head,0x21)=0;
    return head;
}
void* __fastcall construct_native_resource_instance_00b89f20(void* object,void*,void* resource) {
    field<Word>(object)=0x00ceb130;
    field<Word>(object,4)=1;
    field<Word>(object)=0x00d63244;
    field<Pointer>(object,8)=resource;
    for(auto offset : {0x14u,0x18u,0x1cu,0x24u,0x28u,0x2cu}) field<Word>(object,offset)=0;
    try {
        void* head=allocate_native_resource_instance_map_head_00b87930();
        field<Pointer>(object,0x34)=head;
        field<Byte>(head,0x21)=1;
        head=field<Pointer>(object,0x34);field<Pointer>(head,4)=head;
        head=field<Pointer>(object,0x34);field<Pointer>(head)=head;
        head=field<Pointer>(object,0x34);field<Pointer>(head,8)=head;
        field<Word>(object,0x38)=0;
        InterlockedIncrement(static_cast<volatile LONG*>(at(field<Pointer>(object,8),4)));
    } catch(...) {
        unwind_base(object,2);
        throw;
    }
    return object;
}
void* __fastcall construct_native_game_resource_instance_0071acd0(void* object,void*,void* resource) {
    construct_native_resource_instance_00b89f20(object,nullptr,resource);
    field<Word>(object)=0x00cfd8e0;
    for(auto offset : {0x40u,0x44u,0x48u,0x50u,0x54u,0x58u,0x60u,0x64u,0x68u,
        0x70u,0x74u,0x78u,0x80u,0x84u,0x88u}) field<Word>(object,offset)=0;
    return object;
}
void* __fastcall create_native_game_resource_instance_0071aed0(void* resource) {
    void* const allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x8c,0x8c});
    try {
        if(allocation) return construct_native_game_resource_instance_0071acd0(allocation,nullptr,resource);
    } catch(...) {
        singleton_lifetime_free(allocation);
        throw;
    }
    return nullptr;
}
void __fastcall clear_native_resource_instance_map_nodes_00b89150(void* map,void*,void* node) noexcept {
    while(field<Byte>(node,0x21)==0) {
        clear_native_resource_instance_map_nodes_00b89150(map,nullptr,field<Pointer>(node,8));
        void* const data=field<Pointer>(node,0x14);
        void* const left=field<Pointer>(node);
        if(data) singleton_lifetime_free(data);
        field<Word>(node,0x14)=0;
        field<Word>(node,0x18)=0;
        field<Word>(node,0x1c)=0;
        singleton_lifetime_free(node);
        node=left;
    }
}
void __fastcall destroy_native_resource_instance_00b89db0(void* object,
    const NativeResourceInstanceLifetimeAccess* access) {
    field<Word>(object)=0x00d63244;
    int state=3;
    try {
        void* const resource=field<Pointer>(object,8);
        if(resource) {
            if(InterlockedDecrement(static_cast<volatile LONG*>(at(resource,4)))==0) {
                const auto profile=field<Word>(resource);
                const auto entry=access->owners.resource_slot_zero(resource,profile);
                access->owners.dispatch_resource_zero(entry,resource);
            }
            field<Pointer>(object,8)=nullptr;
        }
        void* const root=field<Pointer>(object,0xc);
        if(root) unlink_and_release_render_model_00b6dfa0(access->owners.root_lifetime(root));
        state=2;
        clear_map(at(object,0x30));
        clear_vector(object,0x20);
        clear_vector(object,0x10);
        state=-1;
        destroy_native_ref_counted_base_00bd30f0(object);
    } catch(...) {
        unwind_base(object,state);
        throw;
    }
}
void __fastcall destroy_native_game_resource_instance_00718b30(void* object,
    const NativeResourceInstanceLifetimeAccess* access) {
    field<Word>(object)=0x00cfd8e0;
    bool members_live=true;
    try {
        void* const root=field<Pointer>(object,0xc);
        if(root && access->listener.controlled_listener_00e188dc==root)
            publish_native_controlled_listener_004bca80(nullptr,access->listener,access->renderer);
        for(auto offset : {0x7cu,0x6cu,0x5cu,0x4cu,0x3cu}) clear_vector(object,offset);
        members_live=false;
        destroy_native_resource_instance_00b89db0(object,access);
    } catch(...) {
        if(members_live) unwind_game(object,*access);
        throw;
    }
}
void* __fastcall delete_native_resource_instance_00b89fc0(void* object,
    const NativeResourceInstanceLifetimeAccess* access,std::uint32_t flags) {
    destroy_native_resource_instance_00b89db0(object,access);
    if(flags&1u) singleton_lifetime_free(object);
    return object;
}
void* __fastcall delete_native_game_resource_instance_00718d00(void* object,
    const NativeResourceInstanceLifetimeAccess* access,std::uint32_t flags) {
    destroy_native_game_resource_instance_00718b30(object,access);
    if(flags&1u) singleton_lifetime_free(object);
    return object;
}
NativeUnitPartSelectedSetCallbacks native_resource_instance_selected_callbacks(
    NativeResourceInstanceLifetimeAccess& access) noexcept {
    return {&access,selected_zero,selected_slot};
}
} // namespace bsp
