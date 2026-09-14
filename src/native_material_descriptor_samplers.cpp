#include "bsp/native_material_descriptor_samplers.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
using Op=NativeMaterialDescriptorSamplersOperation;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
void increment(void* p,U offset) noexcept {
    auto* word=reinterpret_cast<volatile U*>(static_cast<char*>(p)+offset);
    *word=read<U>(p,offset)+1u;
}
template<class T> bool unresolved(const std::optional<T>& value) noexcept {
    return value&&(value->phase==T::Phase::running||value->phase==T::Phase::failed);
}
void consume_release_stack(Op& a,NativeMaterialDescriptorSamplerStep& s) {
    const auto& c=*a.context;
    if(a.release_stack_index>=c.release_stack_count||!c.release_stack)
        throw std::logic_error("successful sampler release requires its native B-16 residue");
    const auto& residue=c.release_stack[a.release_stack_index];
    if(residue.sampler_index!=s.sampler_index||residue.released_owner_identity!=s.captured_owner)
        throw std::logic_error("native sampler release stack residue identity/order mismatch");
    c.stack_b_minus_16=residue.word_b_minus_16;
    ++a.release_stack_index;
}
}
NativeMaterialDescriptorSamplersOperation::~NativeMaterialDescriptorSamplersOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeMaterialDescriptorSamplersOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::running)std::terminate();
    for(const auto& s:steps)
        if(s.owner_obligation||unresolved(s.singleton)||unresolved(s.load)||unresolved(s.effect_name)||
            unresolved(s.reference)||unresolved(s.binding))std::terminate();
    phase=Phase::diagnostic_retired;
}
void append_native_material_descriptor_samplers_00b3b280(NativeMaterialProgramBuilderStorage& builder,
    NativeMaterialPassStorage& pass,NativeShaderDescriptorStorage& descriptor,
    NativeMaterialDescriptorSamplersContext& c,Op& a) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("descriptor sampler append is one-shot");
    a.phase=Op::Phase::running;a.builder=&builder;a.pass=&pass;a.descriptor=&descriptor;a.context=&c;
    try {
        U index=0;
        if(read<U>(&descriptor,0xc8)!=0)do {
            auto* const entries=read<NativeShaderSamplerStorage**>(&descriptor,0xc4);
            auto* const sampler=read<NativeShaderSamplerStorage*>(entries,index*4u);
            a.steps.emplace_back();auto& s=a.steps.back();s.sampler_index=index;s.sampler=sampler;
            a.sampler_index=index;
            const U source=read<U>(sampler,0x14);
            bool append_reference=false;
            U texture_index{};
            if(source==0) {texture_index=read<U>(sampler,0x20);append_reference=true;}
            else if(source==1) {
                s.singleton.emplace();a.native_site=s.native_site=0x00b3b2ba;
                s.loader=get_native_sampler_loader_singleton_004de4b0(
                    c.actual_manager_01090aa0,c.actual_loader_00f8d420,*s.singleton);
                s.load.emplace();a.native_site=s.native_site=0x00b3b2c5;
                s.captured_owner=load_native_sampler_with_default_options_00b1b4d0(
                    s.loader,&sampler->source_name_18,c.loader,*s.load);
                // At B-16 the wrapper's PUSH CBC760 remains after its RET4.
                c.stack_b_minus_16=0x00cbc760;
                if(s.captured_owner) {
                    s.owner_obligation=true;
                    const U word=read<U>(&builder,0x8c);
                    s.binding.emplace();a.native_site=s.native_site=0x00b3b2df;
                    append_native_material_pass_binding_00b44cf0(pass,word,s.captured_owner,
                        sampler->source_name_18,c.pass_lifetime,*s.binding);
                    // Three pushed arguments put B44CF0's return PC at B-16.
                    c.stack_b_minus_16=0x00b3b2e4;
                    *reinterpret_cast<volatile std::uint8_t*>(&pass.byte_80)=1;
                    a.native_site=s.native_site=0x00b3b2ef;s.release_entered=true;
                    release_native_render_actual_owner(c.pass_lifetime.retained_owners,s.captured_owner);
                    s.release_returned=true;s.owner_obligation=false;
                    consume_release_stack(a,s);
                    increment(&builder,0x8c);
                }
            } else if(source==3) {
                const U effect_index=read<U>(sampler,0x20);
                auto* const effect=read<NativeMaterialEffectStorage*>(&builder,0x78);
                s.effect_name.emplace();a.native_site=s.native_site=0x00b3b313;
                // B18FC9 saves caller ESI at B-16; no later callee write reaches
                // above that saved register. Its upper bytes are not padding.
                c.stack_b_minus_16=reinterpret_cast<U>(sampler);
                set_native_material_effect_texture_name_00b18fb0(effect->base,effect_index,
                    sampler->source_name_18,c.pass_lifetime.strings,*s.effect_name);
                texture_index=0xffffffffu-read<U>(sampler,0x20);append_reference=true;
            }
            if(append_reference) {
                const auto stage=read<std::uint8_t>(sampler,0xc);
                s.reference.emplace();a.native_site=s.native_site=0x00b3b328;
                append_native_material_texture_reference_00b5f100(pass.base,texture_index,stage,
                    c.stack_b_minus_16,*s.reference);
                increment(&builder,0x8c);
            }
            s.current_states=read<NativeShaderStateListStorage*>(sampler,0x24);
            U state_index=0;
            if(read<U>(s.current_states,4)!=0)do {
                s.state_index=state_index;
                const U slot=read<std::uint8_t>(sampler,0xc)?read<U>(&builder,0x90)+0x10u:read<U>(&builder,0x94);
                const U value=read<U>(read<void*>(s.current_states),state_index*8u+4u);
                const U state=read<U>(read<void*>(s.current_states),state_index*8u);
                a.native_site=s.native_site=0x00b3b369;
                // Native CALL writes B-16 before entering the three-arg setter.
                c.stack_b_minus_16=0x00b3b36e;
                set_native_material_sampler_state_00b5ed60(&pass,slot,state,value);
                s.current_states=read<NativeShaderStateListStorage*>(sampler,0x24);
                ++state_index;
            }while(state_index<read<U>(s.current_states,4));
            increment(&builder,read<std::uint8_t>(sampler,0xc)?0x90u:0x94u);
            ++index;
        }while(index<read<U>(&descriptor,0xc8));
        a.sampler_index=index;a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
