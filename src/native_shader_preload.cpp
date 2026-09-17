#include "bsp/native_shader_preload.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <list>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
using I=std::int32_t;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
template<class T> void put(void* p,U offset,T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<char*>(p)+offset)=value;
}
I signed_bits(U value) noexcept {I result;std::memcpy(&result,&value,4);return result;}
constexpr char preload_script[]="shaderfx/shaderpreload.lua";
static_assert(sizeof(preload_script)==0x1b);
}

void reserve_native_application_pointer_vector_00735ec0(
    NativeApplicationPointerVectorStorage& rows,I requested,
    const NativeApplicationPointerVectorAllocation& allocation) {
    if(requested<1)requested=1;
    if(read<I>(&rows,8)>=requested)return;
    void* const fresh=allocation.allocate_00bf55be(static_cast<U>(requested)*4u);
    auto cursor=reinterpret_cast<std::uintptr_t>(fresh);
    U index=0;
    while(signed_bits(index)<read<I>(&rows,4)) {
        if(cursor) {
            const auto source=read<std::uintptr_t>(&rows);
            put<U>(reinterpret_cast<void*>(cursor),0,
                read<U>(reinterpret_cast<const void*>(source+index*4u)));
        }
        ++index;cursor+=4u;
    }
    allocation.free_00bf6989(read<void*>(&rows));
    put<void*>(&rows,0,fresh);put<I>(&rows,8,requested);
}

struct NativeShaderPreloadOperation::Impl {
    Impl() {} // Do not zero native Lua owners/objects or their unwritten bytes.
    struct MaterialStep {
        NativeMaterialEffectCacheAcquired acquisition;
        void* renderer{};
        void* result{};
        bool returned{},appended{};
    };
    NativeLuaStateStorage lua;
    NativeLuaObjectStorage globals,table,key,value;
    NativeString name;
    NativeShaderBinaryCacheOperation cache;
    std::list<MaterialStep> materials;
    NativeShaderPreloadContext* context{};
    void* application{};
    NativeShaderPreloadPhase phase{NativeShaderPreloadPhase::fresh};
    U site{},appended{};
    bool cache_live{},lua_live{},globals_live{},table_live{},key_live{},value_live{},name_live{};

    void make_name(const char* text,U length,U resize_site,U copy_site) {
        put<U>(&name,0,0);put<char*>(&name,4,nullptr);name_live=true;
        site=resize_site;resize_native_string_header_0041dd40(&name,context->strings,length,true);
        char* const data=read<char*>(&name,4);
        if(data){site=copy_site;std::memmove(data,text,read<U>(&name)+1u);}
    }
    void release_name(U release_site) {
        char* const data=read<char*>(&name,4);
        if(data){const U bytes=read<U>(&name)+1u;site=release_site;context->strings.release(data,bytes);}
        name_live=false; // Preserve the native dead header until its next construction.
    }
};
NativeShaderPreloadOperation::NativeShaderPreloadOperation():impl_(std::make_unique<Impl>()) {}
NativeShaderPreloadOperation::~NativeShaderPreloadOperation(){if(retains_native_state())std::terminate();}
NativeShaderPreloadPhase NativeShaderPreloadOperation::phase() const noexcept{return impl_->phase;}
U NativeShaderPreloadOperation::active_call_site() const noexcept{return impl_->site;}
U NativeShaderPreloadOperation::appended_entries() const noexcept{return impl_->appended;}
bool NativeShaderPreloadOperation::retains_native_state() const noexcept {
    const auto& a=*impl_;
    return a.phase==NativeShaderPreloadPhase::failed||a.cache_live||a.lua_live||a.globals_live
        ||a.table_live||a.key_live||a.value_live||a.name_live;
}
NativeLuaStateStorage* NativeShaderPreloadOperation::retained_lua_state() noexcept {
    return impl_->lua_live?&impl_->lua:nullptr;
}
void* NativeShaderPreloadOperation::unpublished_material_result() const noexcept {
    const auto& entries=impl_->materials;
    return !entries.empty()&&entries.back().returned&&!entries.back().appended?entries.back().result:nullptr;
}
void preload_native_application_shaders_0073bf80(void* application,
    NativeShaderPreloadContext& c,NativeShaderPreloadOperation& operation) {
    auto& a=*operation.impl_;
    if(!application||a.phase!=NativeShaderPreloadPhase::fresh)
        throw std::logic_error("shader preload requires actual application storage and a fresh retained frame");
    if(&c.strings!=&c.materials.strings||&c.strings!=&c.materials.effects.construction.strings
        ||c.bootstrap.do_file_00b69e00!=c.files.do_file_00b69e00
        ||&c.files.manager_0109ceec!=&c.cache.actual_vfs_0109ceec
        ||&c.files.manager_0109ceec!=&c.materials.effects.current_vfs_0109ceec
        ||&c.actual_renderer_00f8d394!=&c.materials.effects.construction.current_renderer_00f8d394)
        throw std::logic_error("shader preload requires shared string, Lua and VFS application services");
    a.application=application;a.context=&c;a.phase=NativeShaderPreloadPhase::cache;
    try {
        a.site=0x0073bfa6;create_native_shader_binary_cache_00b3a600(c.cache,a.cache);a.cache_live=true;
        a.phase=NativeShaderPreloadPhase::bootstrap;
        a.site=0x0073bfaf;construct_native_lua_state_00b66bd0(&a.lua);a.lua_live=true;
        a.site=0x0073bfc3;open_native_lua_state_00b6a020(a.lua,1,c.strings,c.bootstrap);
        a.phase=NativeShaderPreloadPhase::script;
        a.make_name(preload_script,0x1a,0x0073bfd8,0x0073bff3);
        a.site=0x0073c00d;run_native_lua_file_00b69d40(a.lua,a.name,0,c.strings,c.files);
        a.release_name(0x0073c033);
        a.phase=NativeShaderPreloadPhase::globals;
        a.site=0x0073c041;native_lua_globals_00b67980(a.lua,&a.globals);a.globals_live=true;
        a.site=0x0073c05a;native_lua_get_by_name_00b67800(a.globals,&a.table,"FileNames");a.table_live=true;
        a.site=0x0073c06b;destroy_native_lua_object_00b67700(a.globals);a.globals_live=false;
        a.site=0x0073c074;construct_native_lua_object_00b65f50(&a.key);a.key_live=true;
        a.site=0x0073c085;construct_native_lua_object_00b65f50(&a.value);a.value_live=true;
        // C097/C1B6 call4254B0, a single RET. Neither literal is dereferenced.
        a.phase=NativeShaderPreloadPhase::iteration;
        a.site=0x0073c0ad;native_lua_iterate_first_00b67080(a.table,a.key,a.value);
        a.site=0x0073c0bb;
        while(!native_lua_is_unbound_00b66420(a.value)) {
            a.site=0x0073c0d4;const char* const text=native_lua_string_00b662b0(a.value);
            // Native scans this result without a null/type fallback. Convertible
            // nonnull values and readable NUL-terminated extents are required.
            const U length=static_cast<U>(std::strlen(text));
            a.make_name(text,length,0x0073c0f8,0x0073c10f);
            a.phase=NativeShaderPreloadPhase::material;
            a.materials.emplace_back();auto& step=a.materials.back();
            step.renderer=c.actual_renderer_00f8d394;a.site=0x0073c12f;
            if(read<U>(step.renderer)!=0x00d5f0a8||!c.actual_renderer_profile_00d5f0a8
                ||c.actual_renderer_profile_00d5f0a8[0x48/4]!=0x00b318b0)
                throw std::logic_error("shader preload requires the current native renderer+48 material loader");
            step.result=load_native_renderer_material_effect_00b318b0(
                step.renderer,&a.name,c.materials,&step.acquisition);step.returned=true;
            a.phase=NativeShaderPreloadPhase::append;
            auto& rows=*reinterpret_cast<NativeApplicationPointerVectorStorage*>(static_cast<char*>(application)+8);
            const I capacity=read<I>(&rows,8);
            if(read<I>(&rows,4)==capacity) {
                const I doubled=signed_bits(static_cast<U>(capacity)*2u);
                const NativeApplicationPointerVectorAllocation allocation{c.cache.allocate_array_00bf55be,c.cache.free_array_00bf6989};
                a.site=0x0073c14a;reserve_native_application_pointer_vector_00735ec0(rows,doubled>1?doubled:1,allocation);
            }
            const U count=read<U>(&rows,4);const auto data=read<std::uintptr_t>(&rows);
            void* const output=reinterpret_cast<void*>(data+count*4u);
            if(output)put<void*>(output,0,step.result);
            put<U>(&rows,4,read<U>(&rows,4)+1u);step.appended=true;++a.appended;
            a.release_name(0x0073c183);
            a.phase=NativeShaderPreloadPhase::iteration;
            a.site=0x0073c196;native_lua_iterate_next_00b67190(a.table,a.key,a.value);
            a.site=0x0073c1a4;
        }
        a.phase=NativeShaderPreloadPhase::cache_release;
        a.site=0x0073c1be;release_native_shader_binary_cache_00b3b140(c.cache);a.cache_live=false;
        a.phase=NativeShaderPreloadPhase::cleanup;
        a.site=0x0073c1cf;destroy_native_lua_object_00b67700(a.value);a.value_live=false;
        a.site=0x0073c1e0;destroy_native_lua_object_00b67700(a.key);a.key_live=false;
        a.site=0x0073c1f0;destroy_native_lua_object_00b67700(a.table);a.table_live=false;
        a.site=0x0073c204;close_native_lua_state_00b669a0(a.lua);a.lua_live=false;
        a.phase=NativeShaderPreloadPhase::complete;
    }catch(...){a.phase=NativeShaderPreloadPhase::failed;throw;}
}
} // namespace bsp
