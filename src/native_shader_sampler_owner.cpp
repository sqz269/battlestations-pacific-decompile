#include "bsp/native_shader_sampler_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require_fresh(void* raw){
    if(!raw || reinterpret_cast<std::uintptr_t>(raw)%4)throw std::invalid_argument("native shader storage requires aligned fresh memory");
}
struct StringCleanup {
    NativeString& name;NativeStringStorage& strings;
    ~StringCleanup(){destroy_native_string_header_0041dd20(&name,strings);}
};
} // namespace
NativeShaderStateListStorage* initialize_native_shader_state_list_00b621a0(void* raw){
    require_fresh(raw);auto* list=::new(raw) NativeShaderStateListStorage;
    list->data_00=nullptr;list->count_04=0;list->capacity_08=0;return list;
}
void reserve_native_shader_state_pairs_00b40cf0(NativeShaderStateListStorage& rows,std::int32_t request){
    if(request<10)request=10;if(request<=rows.capacity_08)return;
    const auto bytes=static_cast<std::uint32_t>(request)*8u;
    auto* const data=static_cast<std::uint32_t*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    auto* output=data;
    for(std::int32_t i=0;i<rows.count_04;++i,output+=2){
        if(output){const auto* const input=static_cast<const std::uint32_t*>(rows.data_00)+i*2;
            output[0]=input[0];output[1]=input[1];}
    }
    singleton_lifetime_free(rows.data_00);rows.data_00=data;rows.capacity_08=request;
}
void destroy_native_shader_state_list_00b56de0(NativeShaderStateListStorage*& holder,NativeShaderStateListPool& pool){
    auto* const rows=holder;if(!rows)return;
    if(rows->capacity_08<0)reserve_native_shader_state_pairs_00b40cf0(*rows,0);
    while(rows->count_04>0)--rows->count_04;
    rows->count_04=0;singleton_lifetime_free(rows->data_00);
    pool.return_slot_00b62280(rows);holder=nullptr;
}
NativeShaderSamplerStorage* initialize_native_shader_sampler_00b57b50_fragment(void* raw){
    require_fresh(raw);auto* owner=::new(raw) NativeShaderSamplerStorage;
    owner->vtable_00=0x00d621f4;owner->vertex_sampler_0c=0;owner->texture_source_14=0;
    owner->index_20=0;owner->sampler_states_24=nullptr;owner->texture_stage_states_28=nullptr;return owner;
}
void destroy_native_shader_sampler_00b56ea0(NativeShaderSamplerStorage& owner,NativeShaderStateListPool& pool,NativeStringStorage& strings){
    owner.vtable_00=0x00d621f4;
    const StringCleanup name{owner.name_04,strings};const StringCleanup source{owner.source_name_18,strings};
    destroy_native_shader_state_list_00b56de0(owner.sampler_states_24,pool);
    destroy_native_shader_state_list_00b56de0(owner.texture_stage_states_28,pool);
}
NativeShaderSamplerStorage* delete_native_shader_sampler_00b56fc0(
    NativeShaderSamplerStorage* owner,NativeShaderStateListPool& pool,NativeStringStorage& strings,std::uint32_t flags){
    destroy_native_shader_sampler_00b56ea0(*owner,pool,strings);if(flags&1)singleton_lifetime_free(owner);return owner;
}
NativeShaderSamplerCallableBinding::NativeShaderSamplerCallableBinding(
    NativeShaderSamplerStorage& owner,NativeShaderStateListPool& pool,NativeStringStorage& strings)
    :table_{reinterpret_cast<std::uintptr_t>(&invoke),reinterpret_cast<std::uintptr_t>(this)},storage_(&owner),pool_(pool),strings_(strings){
    if(owner.vtable_00!=0x00d621f4)throw std::invalid_argument("sampler binding requires native D621F4 profile");
    owner.vtable_00=reinterpret_cast<std::uintptr_t>(table_.data());
}
NativeShaderSamplerCallableBinding::~NativeShaderSamplerCallableBinding(){if(storage_)std::terminate();}
void NativeShaderSamplerCallableBinding::detach() noexcept {
    if(!storage_)return;if(storage_->vtable_00!=reinterpret_cast<std::uintptr_t>(table_.data()))std::terminate();
    storage_->vtable_00=0x00d621f4;storage_=nullptr;
}
void* __fastcall NativeShaderSamplerCallableBinding::invoke(void* raw,void*,std::uint32_t flags){
    auto* const owner=static_cast<NativeShaderSamplerStorage*>(raw);
    const auto* const table=reinterpret_cast<const std::uintptr_t*>(owner->vtable_00);
    auto& self=*reinterpret_cast<NativeShaderSamplerCallableBinding*>(table[1]);
    if(self.storage_!=owner || table!=self.table_.data())std::terminate();
    self.detach();return delete_native_shader_sampler_00b56fc0(owner,self.pool_,self.strings_,flags);
}
NativeShaderSamplerClassBinding::NativeShaderSamplerClassBinding(NativeShaderStateListPool& pool,NativeStringStorage& strings) noexcept
    :table_{reinterpret_cast<std::uintptr_t>(&invoke),reinterpret_cast<std::uintptr_t>(this)},pool_(pool),strings_(strings){}
void NativeShaderSamplerClassBinding::bind(NativeShaderSamplerStorage& owner){
    if(owner.vtable_00!=0x00d621f4)throw std::invalid_argument("sampler class binding requires native D621F4 profile");
    owner.vtable_00=reinterpret_cast<std::uintptr_t>(table_.data());
}
void NativeShaderSamplerClassBinding::detach(NativeShaderSamplerStorage& owner) noexcept {
    if(owner.vtable_00!=reinterpret_cast<std::uintptr_t>(table_.data()))std::terminate();owner.vtable_00=0x00d621f4;
}
void* __fastcall NativeShaderSamplerClassBinding::invoke(void* raw,void*,std::uint32_t flags){
    auto* const owner=static_cast<NativeShaderSamplerStorage*>(raw);
    const auto* const table=reinterpret_cast<const std::uintptr_t*>(owner->vtable_00);
    auto& self=*reinterpret_cast<NativeShaderSamplerClassBinding*>(table[1]);self.detach(*owner);
    return delete_native_shader_sampler_00b56fc0(owner,self.pool_,self.strings_,flags);
}
} // namespace bsp
