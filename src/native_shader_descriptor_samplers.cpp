#include "bsp/native_shader_descriptor_samplers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Op=NativeDescriptorSamplerOperation;
template<class T> T read(const void* p,std::size_t offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
void begin(Op& a,std::uint32_t function,void* owner) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("descriptor sampler operation is one-shot");
    a.phase=Op::Phase::running;a.function=function;a.owner=owner;
}
std::int32_t doubled(std::int32_t capacity) noexcept {
    const auto bits=static_cast<std::uint32_t>(capacity)*2u;
    std::int32_t value;std::memcpy(&value,&bits,4);return value>1?value:1;
}
}
NativeDescriptorSamplerOperation::~NativeDescriptorSamplerOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeDescriptorSamplerOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
void append_native_material_texture_reference_00b5f100(NativeMaterialPassBaseStorage& pass,
    std::uint32_t index,std::uint8_t stage,volatile std::uint32_t& local,Op& a) {
    begin(a,0x00b5f100,&pass);a.index=index;a.actual_stage_local=&local;
    try {
        *reinterpret_cast<volatile std::uint8_t*>(&local)=stage;
        auto& rows=pass.pairs_24;
        const auto capacity=read<std::int32_t>(&rows,8);
        if(read<std::int32_t>(&rows,4)==capacity) {
            a.native_site=0x00b5f12b;
            reserve_native_material_pass_pairs_00b40c80(rows,doubled(capacity));
        }
        const auto count=read<std::uint32_t>(&rows,4);
        const auto data=read<std::uintptr_t>(&rows);
        auto* row=reinterpret_cast<std::uint32_t*>(data+count*8u);
        if(row) {
            a.stage_word=local;
            row[0]=index;row[1]=a.stage_word;
        }
        *reinterpret_cast<volatile std::uint32_t*>(&rows.count_04)=read<std::uint32_t>(&rows,4)+1u;
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
void append_native_material_pass_binding_00b44cf0(NativeMaterialPassStorage& pass,
    std::uint32_t word,void* retained,const NativeString& name,
    NativeMaterialPassDestructionAccess& access,Op& a) {
    begin(a,0x00b44cf0,&pass);a.value=word;a.source_name=&name;
    a.retained_owner=retained;a.binding_access=&access;
    try {
        a.native_site=0x00b44d0b;
        a.binding=static_cast<NativeMaterialPassBindingStorage*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object,0x10,sizeof(NativeMaterialPassBindingStorage)}));
        if(a.binding) {
            try {
                a.native_site=0x00b44d34;
                initialize_native_material_pass_binding_00b44690(a.binding,word,retained,name,access);
            }catch(...){singleton_lifetime_free(a.binding);a.raw_binding_freed=true;throw;}
        }
        a.index=read<std::uint32_t>(&pass,0x6c);
        if(a.index>=pass.bindings_5c.size())
            throw std::logic_error("native material binding index exceeds four accessible slots");
        pass.bindings_5c[a.index]=a.binding;
        *reinterpret_cast<volatile std::uint32_t*>(&pass.binding_count_6c)=read<std::uint32_t>(&pass,0x6c)+1u;
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
void set_native_material_effect_texture_name_00b18fb0(NativeMaterialEffectBaseStorage& effect,
    std::uint32_t index,const NativeString& name,NativeStringStorage& strings,Op& a) {
    begin(a,0x00b18fb0,&effect);a.index=index;a.source_name=&name;a.strings=&strings;
    try {
        if(index>=effect.names_3c.size())
            throw std::logic_error("native material texture name index exceeds eleven accessible slots");
        const auto current=static_cast<std::uint32_t>(static_cast<std::int32_t>(read<std::int16_t>(&effect,0x94)));
        if(index>=current)
            *reinterpret_cast<volatile std::uint16_t*>(&effect.name_count_94)=static_cast<std::uint16_t>(index+1u);
        a.destination_name=&effect.names_3c[index];
        if(a.destination_name!=&name) {
            a.native_site=0x00b18fde;
            resize_native_string_header_0041dd40(a.destination_name,strings,read<std::uint32_t>(&name),true);
            if(read<std::uint32_t>(&name)!=0) {
                const auto count=read<std::uint32_t>(a.destination_name);
                const auto source=read<const void*>(&name,4);
                auto* destination=read<void*>(a.destination_name,4);
                a.native_site=0x00b18ff3;
                // BF7680 selects reverse copy when destination overlaps source.
                std::memmove(destination,source,count);
            }
        }
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
