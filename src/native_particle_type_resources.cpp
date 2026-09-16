#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bsp {
static_assert(sizeof(void*)==4,"Native particle resources require Win32");
namespace {
void* at(void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
const void* at(const void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
template<class T> T load(const void* p,std::uint32_t n=0) noexcept {
    T v;std::memcpy(&v,at(p,n),sizeof v);return v;
}
template<class T> void store(void* p,std::uint32_t n,T v) noexcept {
    std::memcpy(at(p,n),&v,sizeof v);
}
struct String {
    NativeString value;
    NativeStringStorage& storage;
    explicit String(NativeStringStorage& s):storage(s){}
    ~String(){destroy_native_string_header_0041dd20(&value,storage);}
    String(const String&)=delete;
    String& operator=(const String&)=delete;
};
void set_shader(void* definition,const char* name,NativeStringStorage& strings) {
    bool additive;
    {
        String expected(strings);
        resize_native_string_header_0041dd40(&expected.value,strings,8,true);
        if(expected.value.data())std::memcpy(expected.value.data(),"Additive",expected.value.length()+1u);
        String input(strings);
        construct_native_string_cstring_0041e870(&input.value,name,strings);
        additive=equal_native_string_headers_00435c40(&input.value,&expected.value);
    }
    store<std::uint32_t>(definition,0x7c,additive?1u:0u);
}
struct RawShaderName { std::uint32_t length; char* data; };
static_assert(sizeof(RawShaderName)==8);
void return_shader_name(char* data,std::uint32_t length,NativeStringRawPoolContext& strings) {
    if(!data) return;
    auto* pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,length+1u,
        strings.actual_small_returns_disabled_01090aa4);
}
void set_shader(void* definition,const char* name,NativeStringRawPoolContext& strings) {
    RawShaderName expected{};
    resize_native_string_header_0041dd40(&expected,strings,8,true);
    char* const captured_expected=expected.data;
    if(captured_expected) std::memmove(captured_expected,"Additive",expected.length+1u);
    struct ExpectedUnwind {
        RawShaderName& expected;
        NativeStringRawPoolContext& strings;
        bool armed=true;
        ~ExpectedUnwind() noexcept {
            if(armed) destroy_native_string_header_0041dd20(&expected,strings);
        }
    } unwind{expected,strings}; // state0 begins only before constructing input.
    RawShaderName input;
    void* const constructed=construct_native_string_header_0041e870(&input,strings,name);
    const bool additive=equal_native_string_headers_00435c40(constructed,&expected);
    // Input has no caller unwind state. If its normal release throws, only
    // the CURRENT expected header is destroyed by state0.
    return_shader_name(input.data,input.length,strings);
    unwind.armed=false; // native state=-1 before the second getter.
    return_shader_name(captured_expected,expected.length,strings);
    store<std::uint32_t>(definition,0x7c,additive?1u:0u);
}
void append_model(void* definition,void* resource,NativeParticleTypeBaseBindings& b) {
    void* descriptor=at(definition,0x8c);
    const auto capacity=load<std::uint32_t>(descriptor,8);
    if(load<std::uint32_t>(descriptor,4)==capacity){
        auto wanted=static_cast<std::int32_t>(capacity*2u);
        if(wanted<=1)wanted=1;
        reserve_native_object_particle_models_00af8350(descriptor,wanted,b);
    }
    void* cell=at(load<void*>(descriptor),load<std::uint32_t>(descriptor,4)*4u);
    if(cell)store(cell,0,resource);
    store(descriptor,4,load<std::uint32_t>(descriptor,4)+1u);
}
void* fetch(const void* name,NativeParticleTypeResourceBindings& b) {
    // Test factory before singleton call; reread it afterwards only on the
    // explicit-factory branch, preserving callback-induced publication changes.
    if(*b.factory_00f8d31c){
        void* manager=b.resource_manager_004c1400(b.context);
        return b.load_and_cache_00b80720(b.context,manager,name,*b.factory_00f8d31c);
    }
    void* manager=b.resource_manager_004c1400(b.context);
    return load_native_resource_with_default_factory_00b80d70(manager,name,b);
}
} // namespace
void set_native_sprite_particle_shader_00b089e0(void* p,const char* s,NativeStringStorage& b){set_shader(p,s,b);}
void set_native_axial_particle_shader_00b06210(void* p,const char* s,NativeStringStorage& b){set_shader(p,s,b);}
void set_native_floating_particle_shader_00b07c80(void* p,const char* s,NativeStringStorage& b){set_shader(p,s,b);}
void set_native_sprite_particle_shader_00b089e0(void* p,const char* s,NativeStringRawPoolContext& b){set_shader(p,s,b);}
void set_native_axial_particle_shader_00b06210(void* p,const char* s,NativeStringRawPoolContext& b){set_shader(p,s,b);}
void set_native_floating_particle_shader_00b07c80(void* p,const char* s,NativeStringRawPoolContext& b){set_shader(p,s,b);}
void native_object_particle_shader_noop_00af80e0(void*,const char*) noexcept {}
void native_tracer_particle_shader_noop_00b0a040(void*,const char*) noexcept {}
bool dispatch_known_native_particle_shader(void* definition,std::uint32_t target,
    const char* name,NativeStringStorage& strings) {
    switch(target){
    case 0xb089e0:set_native_sprite_particle_shader_00b089e0(definition,name,strings);return true;
    case 0xb06210:set_native_axial_particle_shader_00b06210(definition,name,strings);return true;
    case 0xb07c80:set_native_floating_particle_shader_00b07c80(definition,name,strings);return true;
    case 0xaf80e0:native_object_particle_shader_noop_00af80e0(definition,name);return true;
    case 0xb0a040:native_tracer_particle_shader_noop_00b0a040(definition,name);return true;
    default:return false;
    }
}
bool dispatch_known_native_particle_shader(void* definition,std::uint32_t target,
    const char* name,NativeStringRawPoolContext& strings) {
    switch(target){
    case 0xb089e0:set_native_sprite_particle_shader_00b089e0(definition,name,strings);return true;
    case 0xb06210:set_native_axial_particle_shader_00b06210(definition,name,strings);return true;
    case 0xb07c80:set_native_floating_particle_shader_00b07c80(definition,name,strings);return true;
    case 0xaf80e0:native_object_particle_shader_noop_00af80e0(definition,name);return true;
    case 0xb0a040:native_tracer_particle_shader_noop_00b0a040(definition,name);return true;
    default:return false;
    }
}
namespace {
struct HostModelStorage {
    NativeParticleTypeBaseBindings& bindings;
    void* allocate(std::uint32_t bytes) { return bindings.allocate_array_00bf55be(bytes); }
    void free(void* p) { bindings.owners.free_array_00bf6989(p); }
    void terminal(void* p,std::uint32_t target) {
        bindings.owners.member_virtual00(bindings.owners.context,p,target);
    }
};
struct RawModelStorage {
    void* allocate(std::uint32_t bytes) {
        return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    }
    void free(void* p) noexcept { singleton_lifetime_free(p); }
    void terminal(void* p,std::uint32_t target) {
        using Terminal=void (__thiscall*)(void*);
        reinterpret_cast<Terminal>(target)(p);
    }
};
template<class Storage>
void reserve_models(void* descriptor,std::int32_t wanted,Storage storage) {
    if(wanted<1)wanted=1;
    if(load<std::int32_t>(descriptor,8)>=wanted)return;
    void* fresh=storage.allocate(static_cast<std::uint32_t>(wanted)*4u);
    for(std::int32_t i=0;i<load<std::int32_t>(descriptor,4);++i){
        void* cell=at(fresh,static_cast<std::uint32_t>(i)*4u);
        if(cell)store(cell,0,load<std::uint32_t>(load<void*>(descriptor),static_cast<std::uint32_t>(i)*4u));
    }
    storage.free(load<void*>(descriptor));
    store(descriptor,0,fresh);
    store(descriptor,8,wanted);
}
template<class Storage>
void clear_models(void* definition,Storage storage) {
    while(load<std::uint32_t>(definition,0x90)!=0){
        const auto count=load<std::uint32_t>(definition,0x90);
        void* cell=at(load<void*>(definition,0x8c),count*4u-4u);
        void* resource=load<void*>(cell);
        if(resource){
            if(InterlockedDecrement(static_cast<volatile LONG*>(at(resource,4)))==0){
                const auto target=load<std::uint32_t>(load<void*>(resource));
                storage.terminal(resource,target);
            }
            store<void*>(cell,0,nullptr);
        }
        const auto current=load<std::uint32_t>(definition,0x90);
        if(current)store(definition,0x90,current-1u);
    }
}
} // namespace
void reserve_native_object_particle_models_00af8350(void* descriptor,std::int32_t wanted,
    NativeParticleTypeBaseBindings& bindings) {
    reserve_models(descriptor,wanted,HostModelStorage{bindings});
}
void reserve_native_object_particle_models_00af8350(void* descriptor,std::int32_t wanted) {
    reserve_models(descriptor,wanted,RawModelStorage{});
}
void clear_native_object_particle_models_00af8940(void* definition,NativeParticleTypeBaseBindings& bindings) {
    clear_models(definition,HostModelStorage{bindings});
}
void clear_native_object_particle_models_00af8940(void* definition) {
    clear_models(definition,RawModelStorage{});
}
void* load_native_resource_with_default_factory_00b80d70(void* manager,const void* name,
    NativeParticleTypeResourceBindings& b) {
    return b.load_and_cache_00b80720(b.context,manager,name,load<void*>(manager,4));
}
void load_native_object_particle_models_00af9660(void* definition,const char* filename,
    NativeParticleTypeResourceBindings& b) {
    auto& strings=b.base.owners.strings;
    clear_native_object_particle_models_00af8940(definition,b.base);
    String stem(strings);
    construct_native_string_cstring_0041e870(&stem.value,filename,strings);
    (void)b.resolve_existing_name_00bdf4c0(b.context,*b.actual_vfs_0109ceec,&stem.value);
    std::int32_t dot;
    {
        String delimiter(strings);
        resize_native_string_header_0041dd40(&delimiter.value,strings,1,true);
        if(delimiter.value.data())std::memcpy(delimiter.value.data(),".",delimiter.value.length()+1u);
        dot=reverse_find_native_string_header_00467cf0(&stem.value,&delimiter.value,0x7fffffffu);
    }
    String extension(strings);
    if(dot!=-1){
        {
            String value(strings);
            construct_native_string_substring_00469840(&stem.value,&value.value,
                static_cast<std::uint32_t>(dot),0x7fffffffu,strings);
            copy_native_string_header_00be0a30_fragment(&extension.value,strings,&value.value);
        }
        {
            String value(strings);
            construct_native_string_substring_00469840(&stem.value,&value.value,0,
                static_cast<std::uint32_t>(dot),strings);
            copy_native_string_header_00be0a30_fragment(&stem.value,strings,&value.value);
        }
    }
    auto index=static_cast<std::int32_t>(stem.value.length()-1u);
    std::int32_t digits=0;
    while(index>0){
        const auto ch=static_cast<signed char>(stem.value.data()[index]);
        if(ch<'0'||ch>'9')break;
        --index;++digits;
    }
    if(digits){
        ++index;
        const char* data=stem.value.data()?stem.value.data():b.empty_stem_00f8d320;
        auto number=static_cast<std::uint32_t>(std::atol(data+index));
        char format[1024];std::sprintf(format,"%%0%dd",digits);
        String prefix(strings);
        construct_native_string_substring_00469840(&stem.value,&prefix.value,0,
            static_cast<std::uint32_t>(index),strings);
        for(;;){
            char buffer[1024];buffer[0]=0;
            std::sprintf(buffer,format,static_cast<std::int32_t>(number));++number;
            String name(strings);
            {
                String suffix(strings), head(strings);
                construct_native_string_cstring_0041e870(&suffix.value,buffer,strings);
                concatenate_native_string_headers_004261a0(&prefix.value,&head.value,&suffix.value,strings);
                concatenate_native_string_headers_004261a0(&head.value,&name.value,&extension.value,strings);
            }
            if(!b.resolve_existing_name_00bdf4c0(b.context,*b.actual_vfs_0109ceec,&name.value))break;
            append_model(definition,fetch(&name.value,b),b.base);
        }
    }else{
        String name(strings);
        concatenate_native_string_headers_004261a0(&stem.value,&name.value,&extension.value,strings);
        if(b.resolve_existing_name_00bdf4c0(b.context,*b.actual_vfs_0109ceec,&name.value))
            append_model(definition,fetch(&name.value,b),b.base);
    }
}
} // namespace bsp
