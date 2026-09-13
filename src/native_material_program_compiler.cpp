#include "bsp/native_material_program_compiler.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition,const char* message) {if(!condition)throw std::logic_error(message);}
template<class T> T current(const void* base,std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(base)+offset);
}
template<class T> void publish(void* base,std::size_t offset,T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(base)+offset)=value;
}
void release_field_owners(NativeShaderDescriptorArray& rows,NativeStringStorage& strings) {
    require(rows.count_04>=0,"builder field destruction requires nonnegative valid extent");
    for(std::uint32_t i=0;i<current<std::uint32_t>(&rows,4);++i) {
        auto** const data=current<void**>(&rows,0);
        require(data,"builder field destruction requires actual pointer storage");
        auto** const slot=data+i;void* const field=current<void*>(slot,0);
        if(field) {
            destroy_native_string_header_0041dd20(field,strings);
            singleton_lifetime_free(field);publish(slot,0,static_cast<void*>(nullptr));
        }
    }
}
void destroy_array(NativeShaderDescriptorArray& rows,std::int32_t minimum,std::uint32_t stride) {
    require(current<std::int32_t>(&rows,4)>=0,"builder array destruction requires nonnegative valid extent");
    if(current<std::int32_t>(&rows,8)<0) {
        require(current<std::int32_t>(&rows,4)<=minimum,"builder reserve would exceed native replacement extent");
        const auto bytes=static_cast<std::size_t>(minimum)*stride;
        auto* const fresh=static_cast<std::byte*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object,bytes,bytes}));
        // No callback occurs inside this copy; valid actual extents are required.
        for(std::int32_t i=0;i<current<std::int32_t>(&rows,4);++i) {
            const auto* const source=current<const std::byte*>(&rows,0);
            std::memcpy(fresh+static_cast<std::size_t>(i)*stride,
                source+static_cast<std::size_t>(i)*stride,stride);
        }
        singleton_lifetime_free(current<void*>(&rows,0));
        publish(&rows,0,static_cast<void*>(fresh));publish(&rows,8,minimum);
    }
    while(current<std::int32_t>(&rows,4)>0)
        publish(&rows,4,current<std::int32_t>(&rows,4)-1);
    publish(&rows,4,std::int32_t{0});
    singleton_lifetime_free(current<void*>(&rows,0));
}
std::int32_t grown_fields(std::int32_t capacity) noexcept {
    const auto bits=static_cast<std::uint32_t>(capacity)+5u;
    std::int32_t signed_bits;std::memcpy(&signed_bits,&bits,4);
    return signed_bits>10?signed_bits:10;
}
NativeShaderFieldStorage* field_at(NativeShaderDescriptorStorage* descriptor,std::uint32_t index) {
    auto* const data=current<NativeShaderFieldStorage**>(descriptor,0xd0);
    return current<NativeShaderFieldStorage*>(data,static_cast<std::size_t>(index)*4u);
}
} // namespace

NativeMaterialProgramBuilderStorage* initialize_native_material_program_builder_00b354d0(void* raw) {
    require(raw&&reinterpret_cast<std::uintptr_t>(raw)%4u==0,"builder requires aligned actual B0h storage");
    auto* const builder=::new(raw) NativeMaterialProgramBuilderStorage;
    builder->vtable_00=0x00d5f314;
    for(std::size_t offset=4;offset<=0x68;offset+=4)publish(builder,offset,std::uint32_t{0});
    builder->byte_6c=0xff;builder->byte_6d=0xff;
    publish(builder,0x9c,std::uint32_t{0});publish(builder,0xa0,std::uint32_t{0});
    return builder;
}
void destroy_native_material_program_builder_00b3a7e0(
    NativeMaterialProgramBuilderStorage& builder,NativeStringStorage& strings) {
    builder.vtable_00=0x00d5f314;
    auto& owners=builder.virtual_owners_40;
    require(owners.count_04>=0,"builder virtual children require nonnegative valid extent");
    for(std::uint32_t i=0;i<current<std::uint32_t>(&owners,4);++i) {
        auto** const data=current<void**>(&owners,0);
        require(data,"builder virtual children require actual pointer storage");
        auto** const slot=data+i;void* const child=current<void*>(slot,0);
        if(child) {
            const auto* const table=current<const std::uintptr_t*>(child,0);
            using Delete=void*(__thiscall*)(void*,std::uint32_t);
            reinterpret_cast<Delete>(table[0])(child,1);
            publish(slot,0,static_cast<void*>(nullptr));
        }
    }
    release_field_owners(builder.fields_04,strings);
    release_field_owners(builder.fields_10,strings);
    release_field_owners(builder.fields_28,strings);
    release_field_owners(builder.fields_1c,strings);
    release_field_owners(builder.fields_34,strings);
    destroy_native_string_header_0041dd20(&builder.name_9c,strings);
    destroy_array(builder.words_60,1,2);destroy_array(builder.words_54,1,2);
    destroy_native_string_header_0041dd20(&builder.source_4c,strings);
    destroy_array(builder.virtual_owners_40,6,4);
    destroy_array(builder.fields_34,10,4);destroy_array(builder.fields_28,10,4);
    destroy_array(builder.fields_1c,10,4);destroy_array(builder.fields_10,10,4);
    destroy_array(builder.fields_04,10,4);
}

void append_native_material_program_vertex_inputs_00b35930(
    NativeMaterialProgramBuilderStorage& builder,NativeStringStorage& strings,
    NativeMaterialProgramVertexInputsAcquired& acquired) {
    require(!acquired.entered,"vertex-input acquisition cannot be replayed");acquired.entered=true;
    for(const auto offset:{0x70u,0x74u}) {
        acquired.descriptor_offset=offset;
        for(std::uint32_t i=0;i<current<std::uint32_t>(
            current<NativeShaderDescriptorStorage*>(&builder,offset),0xd4);++i) {
            acquired.index=i;acquired.native_site=offset==0x70?0x00b35972:0x00b35aa4;
            void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,0x1c,0x1c});
            acquired.unpublished_field=static_cast<NativeShaderFieldStorage*>(raw);
            // Source descriptor is reloaded after allocation. Each scalar input
            // is captured before pooled-name allocation, like native registers.
            auto* const descriptor=current<NativeShaderDescriptorStorage*>(&builder,offset);
            const auto semantic_index=field_at(descriptor,i)->semantic_index_18;
            const auto semantic=field_at(descriptor,i)->semantic_14;
            const auto components=field_at(descriptor,i)->component_count_0c;
            const auto scalar=field_at(descriptor,i)->scalar_type_08;
            const auto* const source=field_at(descriptor,i);
            auto* const field=::new(raw) NativeShaderFieldStorage;acquired.name_constructed=true;
            acquired.native_site=offset==0x70?0x00b359f9:0x00b35b2b;
            copy_native_string_header_00be0a30_fragment(&field->name_00,strings,&source->name_00);
            field->scalar_type_08=scalar;field->semantic_14=semantic;
            field->component_count_0c=components;field->component_mask_10=0;
            field->semantic_index_18=semantic_index;
            auto& rows=builder.fields_04;
            if(current<std::int32_t>(&rows,4)==current<std::int32_t>(&rows,8)) {
                acquired.native_site=offset==0x70?0x00b35a60:0x00b35b92;
                reserve_native_shader_field_pointers_00b34680(rows,grown_fields(current<std::int32_t>(&rows,8)));
            }
            const auto count=current<std::int32_t>(&rows,4);
            auto** const data=current<NativeShaderFieldStorage**>(&rows,0);
            require(count>=0&&count<current<std::int32_t>(&rows,8)&&data,
                "builder vertex append requires an accessible actual pointer slot");
            publish(data,static_cast<std::size_t>(count)*4u,field);
            publish(&rows,4,current<std::uint32_t>(&rows,4)+1u);
            acquired.unpublished_field=nullptr;acquired.name_constructed=false;
        }
    }
    acquired.complete=true;
}

NativeMaterialProgramCompileOperation::NativeMaterialProgramCompileOperation()
    :frame_(std::make_unique<NativeMaterialProgramCompilerFrame>()) {}
NativeMaterialProgramCompileOperation::~NativeMaterialProgramCompileOperation() {
    if(retains_native_state())std::terminate();
}
bool NativeMaterialProgramCompileOperation::complete() const noexcept {
    return frame_->phase==NativeMaterialProgramCompilePhase::complete;
}
bool NativeMaterialProgramCompileOperation::retains_native_state() const noexcept {
    return frame_->builder_live||frame_->local_name_live||frame_->substring_live
        ||frame_->vertex_inputs.unpublished_field;
}
const NativeMaterialProgramCompilerFrame& NativeMaterialProgramCompileOperation::frame() const noexcept {return *frame_;}
NativeMaterialPassStorage* compile_native_material_program_00b3c3a0(
    const NativeMaterialProgramRequest& request,NativeMaterialProgramCompileContext& context,
    NativeMaterialProgramCompileOperation& operation) {
    auto& frame=*operation.frame_;
    require(frame.phase==NativeMaterialProgramCompilePhase::fresh,"program compile requires a fresh retained operation");
    frame.effect=&request.effect_ecx;frame.descriptor=&request.descriptor_edx;
    frame.mode_descriptor=&request.mode_descriptor;frame.original_name=&request.program_name;
    frame.render_target_count=request.render_target_count;frame.generation=request.generation;
    frame.mode=request.mode_flag;frame.descriptor_flag=request.descriptor_flag;frame.policy=request.policy;
    try {
        frame.phase=NativeMaterialProgramCompilePhase::builder;frame.native_site=0x00b3c3c6;
        auto& builder=*initialize_native_material_program_builder_00b354d0(&frame.builder);frame.builder_live=true;
        frame.phase=NativeMaterialProgramCompilePhase::wrapper_name;frame.native_site=0x00b3c3f4;
        copy_native_string_header_00be0a30_fragment(&builder.name_9c,context.strings,frame.original_name);
        builder.descriptor_flag_a9=frame.descriptor_flag;builder.generation_ac=frame.generation;
        builder.render_target_count_a4=frame.render_target_count;builder.mode_a8=frame.mode;
        builder.policy_aa=frame.policy;builder.legacy_generation_98=frame.generation<3?1:0;
        frame.phase=NativeMaterialProgramCompilePhase::pass_prefix;frame.native_site=0x00b3c47b;
        builder.effect_78=frame.effect;builder.descriptor_70=frame.descriptor;builder.mode_descriptor_74=frame.mode_descriptor;
        frame.local_name_live=true;frame.native_site=0x00b3b420;
        copy_native_string_header_00be0a30_fragment(&frame.local_name,context.strings,&builder.name_9c);
        // Native conditional 4254B0(format,name) body is a verified plain RET.
        if(context.load_variants_0108d6f0)frame.native_site=0x00b3b461;
        frame.native_site=0x00b3b475;
        const auto slash=reverse_find_native_string_bytes_004bcb80(builder.name_9c,"/",0x7fffffff);
        if(slash!=0xffffffffu) {
            frame.substring_live=true;frame.native_site=0x00b3b490;
            construct_native_string_substring_00469840(&builder.name_9c,&frame.substring,
                slash+1u,builder.name_9c.length(),context.strings);
            frame.native_site=0x00b3b4ae;
            copy_native_string_header_00be0a30_fragment(&frame.local_name,context.strings,&frame.substring);
            frame.native_site=0x00b3b4f5;
            destroy_native_string_header_0041dd20(&frame.substring,context.strings);frame.substring_live=false;
        }
        if(context.load_variants_0108d6f0||context.source_mode_0108d6f1) {
            frame.phase=NativeMaterialProgramCompilePhase::vertex_inputs;frame.native_site=0x00b3b50e;
            append_native_material_program_vertex_inputs_00b35930(builder,context.strings,frame.vertex_inputs);
            frame.resume=NativeMaterialProgramCompilerResume::source_after_vertex_inputs_00b3b513;
        } else frame.resume=NativeMaterialProgramCompilerResume::cached_before_pass_allocation_00b3b536;
        frame.phase=NativeMaterialProgramCompilePhase::required_tail;
        frame.native_site=static_cast<std::uint32_t>(frame.resume);
        frame.result=context.tail.continue_material_pass_00b3b3c0(frame,frame.resume,frame.tail_child);
        require(!frame.local_name_live&&!frame.substring_live&&!frame.vertex_inputs.unpublished_field,
            "native compiler continuation returned before completing its native local cleanup");
        frame.phase=NativeMaterialProgramCompilePhase::builder_cleanup;frame.native_site=0x00b3c491;
        destroy_native_material_program_builder_00b3a7e0(builder,context.strings);frame.builder_live=false;
        frame.phase=NativeMaterialProgramCompilePhase::complete;return frame.result;
    } catch(...) {frame.phase=NativeMaterialProgramCompilePhase::failed;throw;}
}
} // namespace bsp
