#include "bsp/native_shader_descriptor_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void destroy_array(NativeShaderDescriptorArray& rows,std::int32_t minimum,std::uint32_t stride) {
    // B458A0 inline reserve/resize/free; unwind B41F40/B34BF0/B34C30
    // reaches the same operations through B40CF0/B34620/B34680.
    if(rows.count_04<0)throw std::logic_error("descriptor array requires nonnegative valid extent");
    if(rows.capacity_08<0) {
        auto* const data=static_cast<std::uint32_t*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::pointer_slots,static_cast<std::size_t>(minimum)*stride,
             static_cast<std::size_t>(minimum)*stride}));
        auto* output=data;
        for(std::int32_t i=0;i<rows.count_04;++i,output+=stride/4) {
            if(output) {
                const auto* const input=static_cast<const std::uint32_t*>(rows.data_00)+i*(stride/4);
                output[0]=input[0];if(stride==8)output[1]=input[1];
            }
        }
        singleton_lifetime_free(rows.data_00);
        rows.data_00=data;rows.capacity_08=minimum;
    }
    while(rows.count_04>0)--rows.count_04;
    rows.count_04=0;singleton_lifetime_free(rows.data_00);
}
struct Members {
    NativeShaderDescriptorStorage& owner;NativeStringStorage& strings;int state=12;
    void finish() {
        while(state>=0) {
            switch(state--) {
            case 12:destroy_native_string_header_0041dd20(&owner.name_100,strings);break;
            case 11:destroy_native_string_header_0041dd20(&owner.name_f8,strings);break;
            case 10:destroy_native_string_header_0041dd20(&owner.name_f0,strings);break;
            case 9:destroy_native_string_header_0041dd20(&owner.name_e8,strings);break;
            case 8:destroy_array(owner.string_owners_dc,10,4);break;
            case 7:destroy_array(owner.string_owners_d0,10,4);break;
            case 6:destroy_array(owner.virtual_owners_c4,6,4);break;
            case 5:destroy_array(owner.pairs_b8,10,8);break;
            case 4:for(int i=13;i>=0;--i)destroy_native_string_header_0041dd20(&owner.mode_names_48[i],strings);break;
            case 3:destroy_native_string_header_0041dd20(&owner.name_3c,strings);break;
            case 2:destroy_native_string_header_0041dd20(&owner.name_34,strings);break;
            case 1:destroy_native_string_header_0041dd20(&owner.name_28,strings);break;
            case 0:destroy_native_string_header_0041dd20(&owner.name_0c,strings);break;
            }
        }
    }
    ~Members() noexcept(false){finish();}
};
void destroy_string_owners(NativeShaderDescriptorArray& rows,NativeStringStorage& strings) {
    for(std::uint32_t i=0;i<static_cast<std::uint32_t>(rows.count_04);++i) {
        auto** const entry=static_cast<void**>(rows.data_00)+i;
        void* const owner=*entry;
        if(owner){destroy_native_string_header_0041dd20(owner,strings);singleton_lifetime_free(owner);*entry=nullptr;}
    }
}
} // namespace
NativeShaderDescriptorStorage* initialize_native_shader_descriptor_00b43700(void* fresh) {
    if(!fresh || reinterpret_cast<std::uintptr_t>(fresh)%alignof(NativeShaderDescriptorStorage))
        throw std::invalid_argument("descriptor requires aligned fresh110h storage");
    // Default-initialization starts actual NativeString lifetimes with zero
    // headers. Trivial scalar/opaque members retain their incoming bytes.
    auto* const owner=::new(fresh) NativeShaderDescriptorStorage;
    owner->vtable_00=0x00d61a44;
    owner->pairs_b8={nullptr,0,0};owner->virtual_owners_c4={nullptr,0,0};
    owner->string_owners_d0={nullptr,0,0};owner->string_owners_dc={nullptr,0,0};
    return owner;
}
void destroy_native_shader_descriptor_00b458a0(NativeShaderDescriptorStorage& owner,NativeStringStorage& strings) {
    owner.vtable_00=0x00d61a44;Members members{owner,strings};
    auto& rows=owner.virtual_owners_c4;
    for(std::uint32_t i=0;i<static_cast<std::uint32_t>(rows.count_04);++i) {
        auto** const entry=static_cast<void**>(rows.data_00)+i;
        void* const child=*entry;
        if(child) {
            const auto* const table=*static_cast<const std::uintptr_t* const*>(child);
            using Delete=void* (__thiscall*)(void*,std::uint32_t);
            reinterpret_cast<Delete>(table[0])(child,1);*entry=nullptr;
        }
    }
    destroy_string_owners(owner.string_owners_d0,strings);
    destroy_string_owners(owner.string_owners_dc,strings);
    members.finish();
}
NativeShaderDescriptorStorage* delete_native_shader_descriptor_00b46930(
    NativeShaderDescriptorStorage* owner,NativeStringStorage& strings,std::uint32_t flags) {
    destroy_native_shader_descriptor_00b458a0(*owner,strings);
    if(flags&1)singleton_lifetime_free(owner);return owner;
}
NativeShaderDescriptorCallableBinding::NativeShaderDescriptorCallableBinding(
    NativeShaderDescriptorStorage& owner,NativeStringStorage& strings)
    :table_{reinterpret_cast<std::uintptr_t>(&invoke),reinterpret_cast<std::uintptr_t>(this)},storage_(&owner),strings_(strings) {
    if(owner.vtable_00!=0x00d61a44)throw std::invalid_argument("descriptor binding requires native D61A44 profile");
    owner.vtable_00=reinterpret_cast<std::uintptr_t>(table_.data());
}
NativeShaderDescriptorCallableBinding::~NativeShaderDescriptorCallableBinding(){if(storage_)std::terminate();}
void NativeShaderDescriptorCallableBinding::detach() noexcept {
    if(!storage_)return;
    if(storage_->vtable_00!=reinterpret_cast<std::uintptr_t>(table_.data()))std::terminate();
    storage_->vtable_00=0x00d61a44;storage_=nullptr;
}
void* __fastcall NativeShaderDescriptorCallableBinding::invoke(void* raw,void*,std::uint32_t flags) {
    auto* const owner=static_cast<NativeShaderDescriptorStorage*>(raw);
    const auto* const table=reinterpret_cast<const std::uintptr_t*>(owner->vtable_00);
    auto& self=*reinterpret_cast<NativeShaderDescriptorCallableBinding*>(table[1]);
    if(self.storage_!=owner || table!=self.table_.data())std::terminate();
    self.detach();return delete_native_shader_descriptor_00b46930(owner,self.strings_,flags);
}
} // namespace bsp
