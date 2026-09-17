#include "bsp/native_downscale_pass_initializers.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native downscale initialization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
Word word(const void* p,Word byte_offset=0) noexcept {
    Word value;
    __asm { mov eax,p }
    __asm { add eax,byte_offset }
    __asm { mov eax,[eax] }
    __asm { mov value,eax }
    return value;
}
void* pointer(Word value) noexcept {return reinterpret_cast<void*>(value);}
void put(void* p,Word byte_offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p)+byte_offset)=value;
}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
NativeMaterialStorage& current_material(void* owner) {
    void* const post=pointer(word(owner,8));
    require(post!=nullptr,"downscale pass requires its current actual post-effect");
    void* const material=native_post_effect_material_00b4cba0(post);
    require(material!=nullptr,"downscale pass requires its current actual material");
    return *static_cast<NativeMaterialStorage*>(material);
}
} // namespace

namespace detail {
// Full inline arithmetic from B5462B..B54671. The single configuration
// capture and width/height read order differ from the unsigned 2x2 helper.
void store_native_downscale4x4_inverse_size(void* owner,
    void* const volatile& configuration) noexcept {
    void* const captured=configuration;
    Word width,height;
    float x,y;
    __asm {
        mov eax,captured
        mov edx,[eax+24h]
        mov width,edx
        fild dword ptr width
        fld1
        mov ecx,[eax+28h]
        fld st(0)
        mov height,ecx
        fdivrp st(2),st(0)
        xorps xmm0,xmm0
        mov edx,owner
        movss dword ptr [edx+218h],xmm0
        movss dword ptr [edx+21ch],xmm0
        fxch st(1)
        fstp dword ptr x
        fidiv dword ptr height
        fstp dword ptr y
        fld dword ptr x
        fstp dword ptr [edx+210h]
        fld dword ptr y
        fstp dword ptr [edx+214h]
    }
}
} // namespace detail

void write_native_downscale2x2_offsets_00b4cd30(Word width,Word height,void* output,
    const NativeDownscale2x2OffsetConstants& constants) noexcept {
    const volatile float* const bias=&constants.unsigned_bias_00ce3978;
    const volatile double* const one_and_half=&constants.one_and_half_00ce3d78;
    const volatile double* const negative_half=&constants.negative_half_00cec9e0;
    float inverse_x,inverse_y,second_x;
    Word row;
    __asm {
        mov eax,width
        fild dword ptr width
        test eax,eax
        jge width_positive
        mov edx,bias
        fadd dword ptr [edx]
    width_positive:
        fld1
        mov ecx,height
        test ecx,ecx
        fld st(0)
        fdivrp st(2),st(0)
        fxch st(1)
        fstp dword ptr inverse_x
        fild dword ptr height
        jge height_positive
        mov edx,bias
        fadd dword ptr [edx]
    height_positive:
        fdivp st(1),st(0)
        mov eax,output
        xor ecx,ecx
        mov row,ecx
        add eax,10h
        fstp dword ptr inverse_y
        fldz
        mov edx,one_and_half
        fld qword ptr [edx]
        fsub st(1),st(0)
        fld dword ptr inverse_x
        fld st(0)
        fmulp st(3),st(0)
        fxch st(2)
        fstp dword ptr inverse_x
        movss xmm0,dword ptr inverse_x
        fxch st(1)
        mov edx,negative_half
        fmul qword ptr [edx]
        fstp dword ptr second_x
        fld dword ptr inverse_y
        movss xmm1,dword ptr second_x
    rows:
        fild dword ptr row
        add ecx,1
        movss dword ptr [eax-10h],xmm0
        movss dword ptr [eax],xmm1
        fsub st(0),st(2)
        add eax,20h
        cmp ecx,2
        fmul st(0),st(1)
        fstp dword ptr row
        fld dword ptr row
        mov row,ecx
        fst dword ptr [eax-2ch]
        fstp dword ptr [eax-1ch]
        jl rows
        fstp st(1)
        fstp st(0)
    }
}

void NativeRenderPassInitializationBlock::execute_downscale(void* owner,std::size_t bytes,
    bool four,const NativeDepthDownscalePassArguments& args,
    const NativeDownscalePassInitializationContext& downscale) {
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=(four?0x220u:0x90u) &&
        phase_==Phase::prepared && context_==&downscale.common &&
        downscale.downscale4x4_name_00d620e8 && downscale.downscale2x2_name_00d620fc &&
        downscale.inverse_size_name_00d620d8,
        "downscale pass requires its existing owner extent and matching prepared domains");
    auto& c=*context_;auto& a=acquired_;a.receiver=owner;phase_=Phase::executing;
    bool name_owned=false;
    try {
        a.raw_post_effect=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        set_state(0);
        if(a.raw_post_effect) {
            construct_native_string_cstring_0041e870(&effect_name_,four?
                downscale.downscale4x4_name_00d620e8:downscale.downscale2x2_name_00d620fc,
                c.post_effects.raw_strings);
            a.effect_name_constructed=true;name_owned=true;set_state(1);
            a.returned_post_effect=construct_native_post_effect20_00b4e470(a.raw_post_effect,
                0x20,effect_name_,3,nullptr,post_effect_);
        }
        put(owner,8,reinterpret_cast<Word>(a.returned_post_effect));a.post_published=true;
        set_state(-1);if(name_owned)return_effect_name();

        put(&parameter_name_,0,0);put(&parameter_name_,4,0);
        resize_native_string_header_0041dd40(&parameter_name_,c.post_effects.raw_strings,four?15u:14u,true);
        if(void* const data=pointer(word(&parameter_name_,4)))
            std::memcpy(data,four?downscale.inverse_size_name_00d620d8:c.sample_offsets_name_00d5e40c,
                word(&parameter_name_)+1u);
        void* const values=pointer(reinterpret_cast<Word>(owner)+(four?0x210u:0x10u));
        set_state(3);
        register_native_material_parameter_00b17e10(current_material(owner),&parameter_name_,
            values,four?4u:16u,0,c.parameters);
        set_state(-1);return_parameter_name();
        void* const texture=native_shadow_holder_texture_00b4cb10(args.input_holder);
        set_native_material_texture_unchecked_00b189f0(&current_material(owner),0,texture,
            c.post_effects.destruction.actual_owners);
        if(four)detail::store_native_downscale4x4_inverse_size(owner,downscale.actual_configuration_0109cf04);
        else {
            void* const configuration=downscale.actual_configuration_0109cf04;
            const Word height=word(configuration,0x28),width=word(configuration,0x24);
            write_native_downscale2x2_offsets_00b4cd30(width,height,values,downscale.offsets);
        }
        a.raw_holder=singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18});
        set_state(4);
        if(a.raw_holder) {
            a.holder_arguments={args.width,args.height,args.format,0,0,nullptr};
            a.returned_holder=construct_native_render_texture_surface_owner_00b4e020(
                a.raw_holder,a.holder_arguments,c.holders,a.holder);
        }
        set_state(-1);put(owner,0xc,reinterpret_cast<Word>(a.returned_holder));a.holder_published=true;
        require(a.returned_holder!=nullptr,"downscale primary getter requires a returned holder");
        auto* const surface=native_render_holder_primary_00b4cb20(a.returned_holder);
        void* const post=pointer(word(owner,8));
        require(post!=nullptr,"downscale color binding requires its current post-effect");
        set_native_post_effect_color0_00b4cb70(post,surface,c.post_effects.destruction.frame_targets);
        a.completed=true;settle();
    } catch(...) {
        try {unwind(name_owned);}catch(...){settle();throw;}
        settle();throw;
    }
}
void initialize_native_downscale4x4_pass_00b544f0(void* owner,std::size_t bytes,
    NativeDepthDownscalePassArguments args,const NativeDownscalePassInitializationContext& context,
    NativeRenderPassInitializationBlock& block) {
    block.execute_downscale(owner,bytes,true,args,context);
}
void initialize_native_downscale2x2_pass_00b546f0(void* owner,std::size_t bytes,
    NativeDepthDownscalePassArguments args,const NativeDownscalePassInitializationContext& context,
    NativeRenderPassInitializationBlock& block) {
    block.execute_downscale(owner,bytes,false,args,context);
}
} // namespace bsp
