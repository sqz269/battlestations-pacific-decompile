#include "bsp/native_shader_descriptor_reader.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
struct NativeShaderDescriptorReadOperation::Impl {
    // User-provided constructor avoids value-initializing the actual Lua
    // storage's unwritten bytes before its recovered producer executes.
    Impl() {}
    NativeLuaStateStorage lua;
    NativeLuaObjectStorage globals,shader,value;
    NativeString temporary;
    NativeShaderDescriptorStorage* descriptor{};
    const void* name{};
    NativeShaderDescriptorReadContext* context{};
    NativeShaderDescriptorReadPhase phase{NativeShaderDescriptorReadPhase::fresh};
    std::uint32_t site{};
    bool lua_live{},globals_live{},shader_live{},value_live{},temporary_live{};

    void lookup(const char* key,std::uint32_t call) {
        site=call;
        native_lua_get_by_name_00b67800(shader,&value,key);
        value_live=true;
    }
    void release_value(std::uint32_t call) {
        site=call;destroy_native_lua_object_00b67700(value);value_live=false;
    }
    void release_temporary() noexcept {
        destroy_native_string_header_0041dd20(&temporary,context->strings);
        temporary_live=false;
        // Reuse only after the actual release callback. Native dead locals have
        // no continuing ownership; a later constructor clears this same header.
        ::new(&temporary) NativeString;
    }
    void integer(std::size_t offset,const char* key,std::int32_t fallback,
        std::uint32_t lookup_site,std::uint32_t read_site,std::uint32_t release_site) {
        lookup(key,lookup_site);site=read_site;
        const auto result=native_lua_integer_or_00b66380(value,fallback,context->crt_sse2_conversion);
        std::memcpy(reinterpret_cast<std::byte*>(descriptor)+offset,&result,4);
        release_value(release_site);
    }
    void boolean(std::size_t offset,const char* key,std::uint8_t fallback,
        std::uint32_t lookup_site,std::uint32_t read_site,std::uint32_t release_site) {
        lookup(key,lookup_site);site=read_site;
        const auto result=native_lua_boolean_or_00b662f0(value,fallback);
        std::memcpy(reinterpret_cast<std::byte*>(descriptor)+offset,&result,1);
        release_value(release_site);
    }
    void string(NativeString& output,const char* key,const char* fallback,
        std::uint32_t lookup_site,std::uint32_t read_site,std::uint32_t copy_site,
        std::uint32_t release_site) {
        lookup(key,lookup_site);site=read_site;temporary_live=true;
        native_lua_string_or_00b685c0(value,&temporary,fallback,context->strings);
        site=copy_site;
        copy_native_string_header_00be0a30_fragment(&output,context->strings,&temporary);
        release_temporary();release_value(release_site);
    }
    void version(NativeString& output,const char* key,const char* fallback,
        std::uint32_t lookup_site,std::uint32_t gate_site,std::uint32_t gate_release,
        std::uint32_t fresh_lookup,std::uint32_t read_site,std::uint32_t resize_site,
        std::uint32_t release_site,std::uint32_t default_site) {
        lookup(key,lookup_site);site=gate_site;
        const bool present=native_lua_is_string_00b660a0(value);
        release_value(gate_release);
        if(present) {
            lookup(key,fresh_lookup);site=read_site;
            const char* const text=native_lua_string_00b662b0(value);
            site=resize_site;
            assign_native_string_cstring_0041e350(&output,text,context->strings);
            release_value(release_site);
        } else {
            site=default_site;
            assign_native_string_cstring_0041e350(&output,fallback,context->strings);
        }
    }
    void field_name(const char* text,std::uint32_t resize_site) {
        site=resize_site;temporary_live=true;
        construct_native_string_cstring_0041e870(&temporary,text,context->strings);
    }
};

NativeShaderDescriptorReadOperation::NativeShaderDescriptorReadOperation():impl_(std::make_unique<Impl>()) {}
NativeShaderDescriptorReadOperation::~NativeShaderDescriptorReadOperation() {
    if(retains_native_state())std::terminate();
}
bool NativeShaderDescriptorReadOperation::complete() const noexcept {
    return impl_->phase==NativeShaderDescriptorReadPhase::complete;
}
bool NativeShaderDescriptorReadOperation::retains_native_state() const noexcept {
    const auto& s=*impl_;
    return s.lua_live||s.globals_live||s.shader_live||s.value_live||s.temporary_live||s.temporary.data();
}
NativeShaderDescriptorReadPhase NativeShaderDescriptorReadOperation::phase() const noexcept {return impl_->phase;}
std::uint32_t NativeShaderDescriptorReadOperation::active_call_site() const noexcept {return impl_->site;}
NativeLuaStateStorage* NativeShaderDescriptorReadOperation::retained_lua_state() noexcept {
    return impl_->lua_live?&impl_->lua:nullptr;
}
NativeShaderDescriptorStorage* NativeShaderDescriptorReadOperation::descriptor() const noexcept {return impl_->descriptor;}
const void* NativeShaderDescriptorReadOperation::original_name_header() const noexcept {return impl_->name;}

void read_native_shader_descriptor_00b43b00(NativeShaderDescriptorStorage& descriptor,
    const void* name,std::uint32_t generation,NativeShaderDescriptorReadContext& context,
    NativeShaderDescriptorReadOperation& operation) {
    auto& s=*operation.impl_;
    if(s.phase!=NativeShaderDescriptorReadPhase::fresh||!name)
        throw std::invalid_argument("descriptor parser requires a fresh retained frame and actual name8h");
    if(&context.sampler_binding.strings()!=&context.strings
        ||context.bootstrap.do_file_00b69e00!=context.files.do_file_00b69e00)
        throw std::invalid_argument("descriptor parser requires the same actual string and Lua application domains");
    s.descriptor=&descriptor;s.name=name;s.context=&context;
    s.phase=NativeShaderDescriptorReadPhase::bootstrap;
    try {
        s.site=0x00b43b25;construct_native_lua_state_00b66bd0(&s.lua);s.lua_live=true;
        s.site=0x00b43b3b;open_native_lua_state_00b6a020(s.lua,1,context.strings,context.bootstrap);
        s.phase=NativeShaderDescriptorReadPhase::script;s.site=0x00b43b4e;
        run_native_lua_file_00b69d40(s.lua,*static_cast<const NativeString*>(name),0,context.strings,context.files);
        s.phase=NativeShaderDescriptorReadPhase::globals;s.site=0x00b43b5c;
        native_lua_globals_00b67980(s.lua,&s.globals);s.globals_live=true;
        s.site=0x00b43b75;native_lua_get_by_name_00b67800(s.globals,&s.shader,"Shader");s.shader_live=true;
        s.site=0x00b43b8a;destroy_native_lua_object_00b67700(s.globals);s.globals_live=false;
        s.phase=NativeShaderDescriptorReadPhase::fields;
        s.integer(0x04,"PipeID",0,0x00b43b9d,0x00b43bae,0x00b43bc1);
        s.integer(0x08,"Priority",0,0x00b43bd4,0x00b43be5,0x00b43bf8);
        s.string(descriptor.name_0c,"VertexFormat","simple.mvfm",0x00b43c0b,0x00b43c24,0x00b43c41,0x00b43c91);
        s.boolean(0x15,"ReceiveShadows",0,0x00b43ca4,0x00b43cb5,0x00b43cc8);
        s.string(descriptor.name_100,"ShadowShader","",0x00b43cdb,0x00b43cf4,0x00b43d15,0x00b43d65);
        descriptor.opaque_14[0]=descriptor.name_100.length()?std::byte{1}:std::byte{0};
        s.boolean(0x16,"FinalLODFadeOut",0,0x00b43d81,0x00b43d92,0x00b43da5);
        s.lookup("FinalLODFadeOutRange",0x00b43db8);
        // D7A238=3C23D70A. Original FLD/push/FSTP and returned ST0/store are
        // float32 at both boundaries; no extended arithmetic is hidden here.
        const std::uint32_t fallback_bits=0x3c23d70a;float fallback;
        std::memcpy(&fallback,&fallback_bits,4);
        s.site=0x00b43dd1;const float range=native_lua_number_or_00b66330(s.value,fallback);
        std::memcpy(descriptor.opaque_14+4,&range,4);s.release_value(0x00b43de4);
        s.integer(0x108,"RTCount",1,0x00b43df7,0x00b43e08,0x00b43e1e);
        s.boolean(0x44,"VisilityFade",1,0x00b43e31,0x00b43e42,0x00b43e55);
        s.boolean(0x1c,"AlphaToCoverage",0,0x00b43e68,0x00b43e79,0x00b43e8c);
        s.boolean(0x1d,"NoBandingFix",0,0x00b43e9f,0x00b43eb0,0x00b43ec3);
        s.boolean(0x1e,"DisableAlphaToCoverage",0,0x00b43ed6,0x00b43ee7,0x00b43efa);
        s.boolean(0x1f,"CompressedVertices",1,0x00b43f0d,0x00b43f1e,0x00b43f31);
        s.integer(0x20,"CompressedElemCount",999,0x00b43f44,0x00b43f58,0x00b43f6b);
        s.string(descriptor.name_28,"InstanceGenerator","",0x00b43f7e,0x00b43f97,0x00b43fb4,0x00b44004);
        s.boolean(0x30,"PixelPositionRegister",0,0x00b44017,0x00b44029,0x00b4403c);
        s.boolean(0x31,"OutputAlpha",1,0x00b4404f,0x00b44060,0x00b44073);
        s.boolean(0x32,"LoResBlend",0,0x00b44086,0x00b44096,0x00b440a9);
        s.boolean(0x45,"WriteDepth",0,0x00b440bc,0x00b440cc,0x00b440df);
        // The assembly uses unsigned generation<3, not a stack return address.
        s.version(descriptor.name_34,"VSVersion",generation<3?"vs_2_a":"vs_3_0",
            0x00b440f2,0x00b44101,0x00b44115,0x00b4412f,0x00b4413e,0x00b44164,0x00b44189,0x00b441aa);
        s.version(descriptor.name_3c,"PSVersion",generation<3?"ps_2_b":"ps_3_0",
            0x00b441bd,0x00b441cc,0x00b441e0,0x00b441fa,0x00b44209,0x00b44234,0x00b44259,0x00b4427a);
        s.phase=NativeShaderDescriptorReadPhase::render_states;
        s.lookup("RenderStates",0x00b4428d);s.site=0x00b4429c;
        const bool render_present=native_lua_is_table_00b661b0(s.value);
        s.release_value(0x00b442b0);
        if(render_present) {
            s.lookup("RenderStates",0x00b442ca);
            auto* definitions=context.definitions_0108fe90;
            if(!definitions)throw std::logic_error("descriptor RenderStates requires actual published0108FE90");
            s.site=0x00b442e9;
            read_native_shader_state_table_00b579b0(descriptor.pairs_b8,definitions->render_04,
                s.value,context.strings,context.crt_sse2_conversion);
            s.release_value(0x00b442f9);
        }
        s.phase=NativeShaderDescriptorReadPhase::combiners;
        s.field_name("Combiners",0x00b4430e);s.site=0x00b44349;
        read_native_shader_combiner_table_00b439c0(descriptor.mode_names_48,s.shader,s.temporary,
            context.strings,context.crt_sse2_conversion,context.combiner_stack);
        s.release_temporary();
        s.phase=NativeShaderDescriptorReadPhase::samplers;s.site=0x00b4437b;
        read_native_shader_samplers_00b41830(descriptor,s.shader,context.definitions_0108fe90,
            context.sampler_binding,context.crt_sse2_conversion);
        s.phase=NativeShaderDescriptorReadPhase::vertex_input;
        s.field_name("VertexInput",0x00b44390);s.site=0x00b443ce;
        append_native_shader_field_table_00b419b0(s.shader,descriptor.string_owners_d0,s.temporary,
            context.strings,context.crt_sse2_conversion,context.field_stack);
        s.release_temporary();
        s.phase=NativeShaderDescriptorReadPhase::interpolators;
        s.field_name("Interpolators",0x00b44409);s.site=0x00b44447;
        append_native_shader_field_table_00b419b0(s.shader,descriptor.string_owners_dc,s.temporary,
            context.strings,context.crt_sse2_conversion,context.field_stack);
        s.release_temporary();
        s.phase=NativeShaderDescriptorReadPhase::source_strings;
        s.string(descriptor.name_e8,"Constants","",0x00b44480,0x00b44499,0x00b444b9,0x00b44509);
        s.string(descriptor.name_f0,"VS","",0x00b4451c,0x00b44535,0x00b44555,0x00b445a5);
        s.string(descriptor.name_f8,"PS","",0x00b445b8,0x00b445d1,0x00b445f1,0x00b44640);
        s.phase=NativeShaderDescriptorReadPhase::cleanup;
        s.site=0x00b44651;destroy_native_lua_object_00b67700(s.shader);s.shader_live=false;
        s.site=0x00b44665;close_native_lua_state_00b669a0(s.lua);s.lua_live=false;
        s.phase=NativeShaderDescriptorReadPhase::complete;
    } catch(...) {
        s.phase=NativeShaderDescriptorReadPhase::failed;
        throw;
    }
}
} // namespace bsp
