#include "bsp/native_shader_state_definitions.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <Windows.h>

namespace bsp {
namespace {
struct DefinitionLiteral {const char* name;std::uint32_t state,conversion;};
constexpr DefinitionLiteral render_literals[]={
#include "shader_render_state_registry.inc"
};
constexpr DefinitionLiteral sampler_literals[]={
#include "shader_sampler_state_registry.inc"
};
constexpr DefinitionLiteral stage_literals[]={
#include "shader_texture_stage_state_registry.inc"
};
static_assert(std::size(render_literals)==31 && std::size(sampler_literals)==13 && std::size(stage_literals)==18);
NativeShaderStateDefinition* at(NativeShaderStateDefinitionArray& rows,std::int32_t index){
    return reinterpret_cast<NativeShaderStateDefinition*>(
        reinterpret_cast<std::uintptr_t>(rows.data_00)+static_cast<std::uint32_t>(index)*16u);
}
void release_section(CapturedSoundLifetimeSection* section) noexcept {
    if(section)section->~CapturedSoundLifetimeSection();
}
void destroy_members(NativeShaderStateDefinitionsStorage& owner,
    NativeShaderStateDefinitionsStorage* volatile& published,SoundLifetimeAccess lifetime,
    NativeStringStorage& strings,int index){
    auto* rows=index==2?&owner.texture_stage_1c:index==1?&owner.sampler_10:&owner.render_04;
    __try {destroy_native_shader_state_definition_array_00b58300(*rows,strings);}
    __finally {
        if(index)destroy_members(owner,published,lifetime,strings,index-1);
        else destroy_native_shader_state_definition_base_00b566b0(owner,published,lifetime);
    }
}
void register_literal(NativeShaderStateDefinitionArray& rows,const DefinitionLiteral& literal,
    NativeStringStorage& strings){
    NativeString name;
    name.resize_0041dd40(strings,static_cast<std::uint32_t>(std::strlen(literal.name)),true);
    auto* const captured_data=name.data();const auto captured_length=name.length();
    if(captured_data)std::memcpy(captured_data,literal.name,captured_length+1u);
    // Native ctor states4..65: unwind releases the CURRENT temporary header;
    // ordinary completion instead uses pointer AND length captured above.
    __try {register_native_shader_state_definition_00b58200(rows,name,literal.state,literal.conversion,strings);}
    __finally {
        if(AbnormalTermination())destroy_native_string_header_0041dd20(&name,strings);
        else if(captured_data)strings.release(captured_data,captured_length+1u);
    }
}
} // namespace
NativeShaderStateDefinition* copy_native_shader_state_definition_00b57630(
    void* fresh,const NativeShaderStateDefinition& source,NativeStringStorage& strings){
    auto* destination=::new(fresh) NativeShaderStateDefinition;
    copy_native_string_header_00be0a30_fragment(&destination->name_00,strings,&source.name_00);
    destination->state_08=source.state_08;destination->conversion_0c=source.conversion_0c;
    return destination;
}
void reserve_native_shader_state_definitions_00b57800(
    NativeShaderStateDefinitionArray& rows,std::int32_t request,NativeStringStorage& strings){
    if(request<1)request=1;if(request<=rows.capacity_08)return;
    const auto bytes=static_cast<std::uint32_t>(request)*16u;
    auto* data=static_cast<std::byte*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    // Native state0 only invokes the CRT no-op placement delete401130; it
    // does not destroy prior copies or free this allocation if copying fails.
    for(std::int32_t i=0;i<rows.count_04;++i){
        auto* destination=data+static_cast<std::uint32_t>(i)*16u;
        if(destination)copy_native_shader_state_definition_00b57630(destination,*at(rows,i),strings);
    }
    for(std::int32_t i=0;i<rows.count_04;++i)destroy_native_string_header_0041dd20(at(rows,i),strings);
    singleton_lifetime_free(rows.data_00);rows.data_00=data;rows.capacity_08=request;
}
void resize_native_shader_state_definitions_00b57930(
    NativeShaderStateDefinitionArray& rows,std::int32_t request,NativeStringStorage& strings){
    if(rows.capacity_08<request)reserve_native_shader_state_definitions_00b57800(rows,request,strings);
    for(auto i=rows.count_04;i<request;++i)if(auto* entry=at(rows,i))::new(entry) NativeShaderStateDefinition;
    while(rows.count_04>request){--rows.count_04;destroy_native_string_header_0041dd20(at(rows,rows.count_04),strings);}
    rows.count_04=request;
}
void append_native_shader_state_definition_00b58050(
    NativeShaderStateDefinitionArray& rows,const NativeShaderStateDefinition& source,NativeStringStorage& strings){
    if(rows.count_04==rows.capacity_08){
        auto capacity=static_cast<std::int32_t>(static_cast<std::uint32_t>(rows.capacity_08)*2u);
        if(capacity<2)capacity=1;reserve_native_shader_state_definitions_00b57800(rows,capacity,strings);
    }
    if(auto* destination=at(rows,rows.count_04))copy_native_shader_state_definition_00b57630(destination,source,strings);
    ++rows.count_04;
}
void register_native_shader_state_definition_00b58200(
    NativeShaderStateDefinitionArray& rows,const NativeString& name,std::uint32_t state,
    std::uint32_t conversion,NativeStringStorage& strings){
    NativeShaderStateDefinition entry;
    copy_native_string_header_00be0a30_fragment(&entry.name_00,strings,&name);
    auto* const captured_data=entry.name_00.data();
    entry.state_08=state;entry.conversion_0c=conversion;
    __try {append_native_shader_state_definition_00b58050(rows,entry,strings);}
    __finally {
        if(AbnormalTermination())destroy_native_string_header_0041dd20(&entry.name_00,strings);
        else if(captured_data)strings.release(captured_data,entry.name_00.length()+1u);
    }
}
void destroy_native_shader_state_definition_array_00b58300(
    NativeShaderStateDefinitionArray& rows,NativeStringStorage& strings){
    resize_native_shader_state_definitions_00b57930(rows,0,strings);singleton_lifetime_free(rows.data_00);
}
void append_unique_native_shader_state_pair_00b567b0(
    NativeShaderStateListStorage& rows,std::uint32_t state,std::uint32_t payload){
    const auto count=static_cast<std::uint32_t>(rows.count_04);
    auto* cursor=static_cast<const std::uint32_t*>(rows.data_00);
    for(std::uint32_t i=0;i<count;++i,cursor+=2)if(cursor[0]==state)return;
    if(count==static_cast<std::uint32_t>(rows.capacity_08)){
        auto capacity=static_cast<std::int32_t>(static_cast<std::uint32_t>(rows.capacity_08)+5u);
        if(capacity<11)capacity=10;reserve_native_shader_state_pairs_00b40cf0(rows,capacity);
    }
    auto* output=reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uintptr_t>(rows.data_00)+
        static_cast<std::uint32_t>(rows.count_04)*8u);
    if(output){output[0]=state;output[1]=payload;}++rows.count_04;
}
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definition_base_00b56610(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime){
    owner.vtable_00=0x00d621ec;alignas(CapturedSoundLifetimeSection) std::byte section_storage[sizeof(CapturedSoundLifetimeSection)];
    CapturedSoundLifetimeSection* captured=nullptr;
    __try {
        captured=::new(section_storage) CapturedSoundLifetimeSection(lifetime);
        published=&owner;auto manager=lifetime.get_manager_00415350();manager->register_object(published);
    } __finally {release_section(captured);if(AbnormalTermination())owner.vtable_00=0x00ce3818;}
    return &owner;
}
void destroy_native_shader_state_definition_base_00b566b0(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime){
    owner.vtable_00=0x00d621ec;alignas(CapturedSoundLifetimeSection) std::byte section_storage[sizeof(CapturedSoundLifetimeSection)];
    CapturedSoundLifetimeSection* captured=nullptr;
    __try {
        captured=::new(section_storage) CapturedSoundLifetimeSection(lifetime);
        auto manager=lifetime.get_manager_00415350();manager->unregister_object(published);published=nullptr;
    } __finally {release_section(captured);owner.vtable_00=0x00ce3818;}
}
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definition_base_00b56750(
    NativeShaderStateDefinitionsStorage* owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime,std::uint32_t flags){
    destroy_native_shader_state_definition_base_00b566b0(*owner,published,lifetime);
    if(flags&1)singleton_lifetime_free(owner);return owner;
}
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definitions_00b585a0(
    void* fresh,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime,NativeStringStorage& strings){
    auto* owner=::new(fresh) NativeShaderStateDefinitionsStorage;
    construct_native_shader_state_definition_base_00b56610(*owner,published,lifetime);
    owner->vtable_00=0x00d62260;
    owner->render_04={nullptr,0,0};owner->sampler_10={nullptr,0,0};owner->texture_stage_1c={nullptr,0,0};
    __try {
        for(const auto& literal:render_literals)register_literal(owner->render_04,literal,strings);
        for(const auto& literal:sampler_literals)register_literal(owner->sampler_10,literal,strings);
        for(const auto& literal:stage_literals)register_literal(owner->texture_stage_1c,literal,strings);
    } __finally {if(AbnormalTermination())destroy_members(*owner,published,lifetime,strings,2);}
    return owner;
}
void destroy_native_shader_state_definitions_00b58320(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime,NativeStringStorage& strings){
    owner.vtable_00=0x00d62260;destroy_members(owner,published,lifetime,strings,2);
}
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definitions_00b59e50(
    NativeShaderStateDefinitionsStorage* owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SoundLifetimeAccess lifetime,NativeStringStorage& strings,std::uint32_t flags){
    destroy_native_shader_state_definitions_00b58320(*owner,published,lifetime,strings);
    if(flags&1)singleton_lifetime_free(owner);return owner;
}
// Preserve the projected interface while sharing the actual array/literal bodies.
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definition_base_00b56610(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime){
    return construct_native_shader_state_definition_base_00b56610(owner,published,SoundLifetimeAccess(lifetime));
}
void destroy_native_shader_state_definition_base_00b566b0(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime){
    destroy_native_shader_state_definition_base_00b566b0(owner,published,SoundLifetimeAccess(lifetime));
}
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definition_base_00b56750(
    NativeShaderStateDefinitionsStorage* owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime,std::uint32_t flags){
    return delete_native_shader_state_definition_base_00b56750(owner,published,SoundLifetimeAccess(lifetime),flags);
}
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definitions_00b585a0(
    void* fresh,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime,NativeStringStorage& strings){
    return construct_native_shader_state_definitions_00b585a0(fresh,published,SoundLifetimeAccess(lifetime),strings);
}
void destroy_native_shader_state_definitions_00b58320(
    NativeShaderStateDefinitionsStorage& owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime,NativeStringStorage& strings){
    destroy_native_shader_state_definitions_00b58320(owner,published,SoundLifetimeAccess(lifetime),strings);
}
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definitions_00b59e50(
    NativeShaderStateDefinitionsStorage* owner,NativeShaderStateDefinitionsStorage* volatile& published,
    SingletonLifetimeDomain& lifetime,NativeStringStorage& strings,std::uint32_t flags){
    return delete_native_shader_state_definitions_00b59e50(owner,published,SoundLifetimeAccess(lifetime),strings,flags);
}
NativeShaderStateDefinitionsLifetimeBinding::NativeShaderStateDefinitionsLifetimeBinding(
    NativeShaderStateDefinitionsStorage* volatile& published,SingletonLifetimeCallbacks next):published_(published),next_(next){
    if(!next.destroy_registered || !next.invalid_parameter)throw std::invalid_argument("state definitions require other-owner lifetime callbacks");
}
SingletonLifetimeCallbacks NativeShaderStateDefinitionsLifetimeBinding::callbacks() noexcept {
    return {this,&destroy_registered,&invalid_parameter};
}
void NativeShaderStateDefinitionsLifetimeBinding::bind(SingletonLifetimeDomain& lifetime,NativeStringStorage& strings){
    if(lifetime_ || strings_)throw std::logic_error("state definition lifetime already bound");lifetime_=&lifetime;strings_=&strings;
}
void NativeShaderStateDefinitionsLifetimeBinding::destroy_registered(void* context,void* raw,std::uint32_t flags) noexcept {
    auto& self=*static_cast<NativeShaderStateDefinitionsLifetimeBinding*>(context);
    auto* owner=static_cast<NativeShaderStateDefinitionsStorage*>(raw);
    if(owner->vtable_00==0x00d62260 || owner->vtable_00==0x00d621ec){
        if(!self.lifetime_ || !self.strings_)std::terminate();
        if(owner->vtable_00==0x00d62260)delete_native_shader_state_definitions_00b59e50(owner,self.published_,*self.lifetime_,*self.strings_,flags);
        else delete_native_shader_state_definition_base_00b56750(owner,self.published_,*self.lifetime_,flags);
    }else self.next_.destroy_registered(self.next_.context,raw,flags);
}
void NativeShaderStateDefinitionsLifetimeBinding::invalid_parameter(void* context){
    auto& self=*static_cast<NativeShaderStateDefinitionsLifetimeBinding*>(context);self.next_.invalid_parameter(self.next_.context);
}
} // namespace bsp
