#include "bsp/native_instance_collection.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/native_render_group_storage.hpp"
#include "bsp/native_render_batch_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native instance collection requires MSVC Win32 x87/SSE.
#endif

namespace bsp {
namespace {
static_assert(offsetof(NativeInstanceCollectionAccess,crt)==0);
static_assert(offsetof(NativeInstanceCollectionAccess,one_00d7a24c)==4);
static_assert(offsetof(NativeInstanceCollectionAccess,distance_width_00ce4d70)==8);
static_assert(offsetof(NativeInstanceCollectionAccess,cutoff_00ceb690)==12);
static_assert(offsetof(NativeInstanceCollectionAccess,default_threshold_00ce77dc)==16);
static_assert(sizeof(NativeString)==8);
template<class T> volatile T& cell(void* p,std::uint32_t offset=0) {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}
template<class T> const volatile T& cell(const void* p,std::uint32_t offset=0) {
    return *reinterpret_cast<const volatile T*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}
void* plus(void* p,std::uint32_t n) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result; std::memcpy(&result,&bits,4); return result;
}
// Capture-data/current-length order matches each inline native string cleanup.
struct TemporaryName {
    void* header;
    ActualNativeStringPoolStorage& strings;
    bool armed=true;
    ~TemporaryName() { if(armed) destroy_native_string_header_0041dd20(header,strings); }
    void finish() noexcept {
        armed=false;
        destroy_native_string_header_0041dd20(header,strings);
    }
};

// B1DFF0 visibility prefix only, called by the full source routine below.
// Dummy frame words preserve the reviewed native local offsets; this helper
// installs no native EH registration and calls only nonthrowing FP/world code.
__declspec(naked) bool __fastcall visible_after_native_fade(void*,
    const NativeInstanceCollectionAccess*,void*) {
    __asm {
        push 0
        push 0
        push 0
        sub esp,0x30
        mov dword ptr [esp+0x14],edx
        mov eax,dword ptr [ecx + 0x28] // 00b1e008
        mov edx,dword ptr [esp + 0x40] // 00b1e00b
        mov edx,dword ptr [edx + 0x4] // 00b1e00f
        push ebx // 00b1e012
        push ebp // 00b1e013
        push esi // 00b1e014
        push edi // 00b1e015
        mov edi,dword ptr [eax + 0x8] // 00b1e016
        mov eax,dword ptr [edx + 0x20] // 00b1e019
        mov ebx,dword ptr [eax + 0x7c] // 00b1e01c
        mov esi,dword ptr [ebx + 0xc4] // 00b1e01f
        cmp byte ptr [esi + 0x16],0x0 // 00b1e025
        mov dword ptr [esp + 0x1c],ecx // 00b1e029
        mov dword ptr [esp + 0x18],edx // 00b1e02d
        jz visible_accept // 00b1e031
        test byte ptr [edi + 0x5c],0x2 // 00b1e037
        jnz visible_00b1e044 // 00b1e03b
        mov ecx,edi // 00b1e03d
        call refresh_native_camera_world_00b6db70 // 00b1e03f
    visible_00b1e044:
        movss xmm0,dword ptr [edi + 0x120] // 00b1e044
        mov eax,dword ptr [esp + 0x50] // 00b1e04c
        mov ebp,dword ptr [eax + 0xc] // 00b1e050
        test byte ptr [ebp + 0x5c],0x2 // 00b1e053
        movss dword ptr [esp + 0x28],xmm0 // 00b1e057
        movss xmm0,dword ptr [edi + 0x124] // 00b1e05d
        movss dword ptr [esp + 0x2c],xmm0 // 00b1e065
        movss xmm0,dword ptr [edi + 0x128] // 00b1e06b
        movss dword ptr [esp + 0x30],xmm0 // 00b1e073
        jnz visible_00b1e082 // 00b1e079
        mov ecx,ebp // 00b1e07b
        call refresh_native_camera_world_00b6db70 // 00b1e07d
    visible_00b1e082:
        fld dword ptr [ebp + 0x120] // 00b1e082
        lea ecx,[esp + 0x34] // 00b1e088
        fstp dword ptr [esp + 0x34] // 00b1e08c
        fld dword ptr [ebp + 0x124] // 00b1e090
        fstp dword ptr [esp + 0x38] // 00b1e096
        fld dword ptr [ebp + 0x128] // 00b1e09a
        fstp dword ptr [esp + 0x3c] // 00b1e0a0
        fld dword ptr [esp + 0x34] // 00b1e0a4
        fsub dword ptr [esp + 0x28] // 00b1e0a8
        fstp dword ptr [esp + 0x20] // 00b1e0ac
        fld dword ptr [esp + 0x38] // 00b1e0b0
        fsub dword ptr [esp + 0x2c] // 00b1e0b4
        fstp dword ptr [esp + 0x14] // 00b1e0b8
        fld dword ptr [esp + 0x3c] // 00b1e0bc
        fsub dword ptr [esp + 0x30] // 00b1e0c0
        fstp dword ptr [esp + 0x10] // 00b1e0c4
        fld dword ptr [esp + 0x20] // 00b1e0c8
        fstp dword ptr [esp + 0x34] // 00b1e0cc
        fld dword ptr [esp + 0x14] // 00b1e0d0
        fstp dword ptr [esp + 0x38] // 00b1e0d4
        fld dword ptr [esp + 0x10] // 00b1e0d8
        fstp dword ptr [esp + 0x3c] // 00b1e0dc
        mov edx,dword ptr [esp+0x24]
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b1e0e0
        mov ebp,dword ptr [edi + 0x198] // 00b1e0e5
        fstp dword ptr [esp + 0x14] // 00b1e0eb
        mov eax,dword ptr [esp+0x24]
        mov eax,dword ptr [eax+4]
        movss xmm0,dword ptr [eax] // 00b1e0ef
        mov edx,0x1 // 00b1e0f7
        mov ecx,ebp // 00b1e0fc
        shl edx,cl // 00b1e0fe
        movss dword ptr [esp + 0x10],xmm0 // 00b1e100
        test dl,0x17 // 00b1e106
        jz visible_00b1e157 // 00b1e109
        fld dword ptr [edi + 0x178] // 00b1e10b
        fstp dword ptr [esp + 0x20] // 00b1e111
        fld dword ptr [esp + 0x20] // 00b1e115
        fsub dword ptr [esp + 0x14] // 00b1e119
        mov eax,dword ptr [esp+0x24]
        mov eax,dword ptr [eax+8]
        fdiv qword ptr [eax] // 00b1e11d
        fstp dword ptr [esp + 0x14] // 00b1e123
        fld dword ptr [esp + 0x14] // 00b1e127
        fldz // 00b1e12b
        fcomip st(0),st(1) // 00b1e12d
        fstp st(0) // 00b1e12f
        jbe visible_00b1e13e // 00b1e131
        xorps xmm0,xmm0 // 00b1e133
        movss dword ptr [esp + 0x10],xmm0 // 00b1e136
        jmp visible_00b1e157 // 00b1e13c
    visible_00b1e13e:
        movss xmm1,dword ptr [esp + 0x14] // 00b1e13e
        comiss xmm1,xmm0 // 00b1e144
        jbe visible_00b1e151 // 00b1e147
        movss dword ptr [esp + 0x10],xmm0 // 00b1e149
        jmp visible_00b1e157 // 00b1e14f
    visible_00b1e151:
        movss dword ptr [esp + 0x10],xmm1 // 00b1e151
    visible_00b1e157:
        fld dword ptr [esi + 0x18] // 00b1e157
        mov esi,dword ptr [esp + 0x50] // 00b1e15a
        mov ecx,dword ptr [esi + 0x8] // 00b1e15e
        fstp dword ptr [esp + 0x20] // 00b1e161
        fld dword ptr [esi] // 00b1e165
        sub esp,0x8 // 00b1e167
        fstp dword ptr [esp + 0x1c] // 00b1e16a
        fld dword ptr [esi + 0x18] // 00b1e16e
        fstp dword ptr [esp + 0x30] // 00b1e171
        fld dword ptr [esp + 0x28] // 00b1e175
        fstp dword ptr [esp + 0x4] // 00b1e179
        fld dword ptr [esp + 0x1c] // 00b1e17d
        fstp dword ptr [esp] // 00b1e181
        mov edx,dword ptr [esp+0x2c]
        call native_stream_threshold_fade_00b73770 // 00b1e184
        fmul dword ptr [esp + 0x28] // 00b1e189
        fmul dword ptr [esp + 0x10] // 00b1e18d
        fstp dword ptr [esp + 0x28] // 00b1e191
        fld dword ptr [esp + 0x28] // 00b1e195
        fst dword ptr [esi + 0x18] // 00b1e199
        mov eax,dword ptr [esp+0x24]
        mov eax,dword ptr [eax+12]
        fld qword ptr [eax] // 00b1e19c
        fcomip st(0),st(1) // 00b1e1a2
        fstp st(0) // 00b1e1a4
        ja visible_reject // 00b1e1a6
        mov ecx,dword ptr [esp + 0x1c] // 00b1e1ac
        mov edx,dword ptr [esp + 0x18] // 00b1e1b0
    visible_accept:
        mov al,1
        jmp visible_return
    visible_reject:
        xor eax,eax
    visible_return:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp,0x3c
        ret 4
    }
}

// B1E1C7..B1E205: FISTP signed64 under saved CW|0C00; compare LOW DWORD
// unsigned against1. This intentionally admits low0/1 of large int64 values.
__declspec(naked) std::uint32_t __fastcall collection_category(const void*) {
    __asm {
        sub esp,0x18
        fld dword ptr [ecx+0x18]
        fnstcw word ptr [esp]
        fstp dword ptr [esp+4]
        movzx eax,word ptr [esp]
        fld dword ptr [esp+4]
        or eax,0xc00
        mov dword ptr [esp+8],eax
        fldcw word ptr [esp+8]
        fistp qword ptr [esp+0xc]
        mov eax,dword ptr [esp+0xc]
        cmp eax,1
        fldcw word ptr [esp]
        jbe category_done
        mov eax,1
    category_done:
        add esp,0x18
        ret
    }
}
__declspec(naked) bool __fastcall is_faded(const void*,const volatile float*) {
    __asm {
        movss xmm0,dword ptr [edx]
        comiss xmm0,dword ptr [ecx+0x18]
        seta al
        ret
    }
}
NativeRenderGroupStorage* indexed_group(NativeRenderPointerArrayStorage& array,
    std::uint32_t id) {
    if(signed_bits(id)>=cell<std::int32_t>(&array,4))
        resize_native_render_group_pointers_00b1c7c0(array,signed_bits(id+1u));
    return cell<NativeRenderGroupStorage*>(plus(cell<void*>(&array),id*4u));
}
void append_borrowed(NativeRenderPointerArrayStorage& array,void* entry,bool group) {
    const auto capacity=cell<std::uint32_t>(&array,8);
    if(cell<std::uint32_t>(&array,4)==capacity) {
        auto doubled=signed_bits(capacity*2u);
        if(doubled<=1) doubled=1;
        if(group) reserve_native_render_group_pointers_00b1c660(array,doubled);
        else reserve_native_instance_entry_pointers_00b1c500(array,doubled);
    }
    const auto count=cell<std::uint32_t>(&array,4);
    void* destination=plus(cell<void*>(&array),count*4u);
    if(destination) cell<void*>(destination)=entry;
    cell<std::uint32_t>(&array,4)+=1u;
}
void initialize_colors(NativeInstanceCollectionAccess& a) {
    if((*a.color_guard_00f8d4b0&1u)!=0) return;
    std::uint32_t one;
    // MOVSS bit copy; no float arithmetic or conversion is introduced.
    std::memcpy(&one,const_cast<const float*>(a.one_00d7a24c),4);
    *a.color_guard_00f8d4b0|=1u;
    constexpr unsigned enabled[24]={1,0,0,1,0,1,0,1,0,0,1,1,1,1,0,1,1,0,1,1,0,1,1,1};
    for(unsigned i=0;i!=24;++i) a.colors_00f8d450[i]=enabled[i]?one:0;
}
} // namespace

const void* __fastcall native_effect_name_00b172d0(const void* effect) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(effect)+0xb8u);
}
std::uint32_t __fastcall native_effect_batch_index_00b17300(const void* effect) noexcept {
    return cell<std::uint32_t>(effect,0xac);
}

__declspec(naked) float __fastcall native_stream_threshold_fade_00b73770(
    const void*,const NativeInstanceCollectionAccess*,float,float) {
    __asm {
        push ecx
        mov eax,dword ptr [ecx+0x50]
        test eax,eax
        jz fade_default
        shl eax,4
        movss xmm0,dword ptr [eax+ecx+4]
        jmp fade_begin
    fade_default:
        mov eax,dword ptr [edx+16]
        movss xmm0,dword ptr [eax]
    fade_begin:
        movss dword ptr [esp],xmm0
        fld dword ptr [esp]
        fld st(0)
        fmul dword ptr [esp+0xc]
        fstp dword ptr [esp+0xc]
        fld dword ptr [esp+8]
        fld dword ptr [esp+0xc]
        fld st(0)
        fsubp st(3),st(0)
        fxch st(1)
        fsubrp st(2),st(0)
        fld1
        fld st(0)
        fdivrp st(2),st(0)
        fxch st(2)
        fmulp st(1),st(0)
        fsubp st(1),st(0)
        fstp dword ptr [esp+0xc]
        fld dword ptr [esp+0xc]
        fldz
        fcomip st(0),st(1)
        fstp st(0)
        jbe fade_above_zero
        xorps xmm0,xmm0
    fade_store:
        movss dword ptr [esp+0xc],xmm0
        fld dword ptr [esp+0xc]
        pop ecx
        ret 8
    fade_above_zero:
        movss xmm0,dword ptr [esp+0xc]
        mov eax,dword ptr [edx+4]
        movss xmm1,dword ptr [eax]
        comiss xmm0,xmm1
        jbe fade_store
        movss dword ptr [esp+0xc],xmm1
        fld dword ptr [esp+0xc]
        pop ecx
        ret 8
    }
}

void append_native_render_batch_entry_00b51cb0(NativeRenderBatchStorage& batch,void* entry) {
    const auto capacity=cell<std::uint32_t>(&batch,0x14);
    if(cell<std::uint32_t>(&batch,0x10)==capacity) {
        auto doubled=signed_bits(capacity*2u);
        if(doubled<=0x100) doubled=0x100;
        reserve_native_render_batch_entries_00b51b50(batch,doubled);
    }
    const auto count=cell<std::uint32_t>(&batch,0x10);
    void* destination=plus(cell<void*>(&batch,0xc),count*4u);
    if(destination) cell<void*>(destination)=entry;
    cell<std::uint32_t>(&batch,0x10)+=1u;
}

void* prefix_native_string_header_0043c130(void* output,const char* prefix,
    const void* right,ActualNativeStringPoolStorage& strings) {
    NativeString temporary;
    construct_native_string_cstring_0041e870(&temporary,prefix,strings);
    TemporaryName cleanup{&temporary,strings};
    concatenate_native_string_headers_004261a0(&temporary,output,right,strings);
    cleanup.finish();
    return output;
}

void collect_native_instance_entry_00b1dff0(void* command,void* entry,
    NativeInstanceCollectionAccess& a) {
    void* const section=cell<void*>(entry,4);
    void* const effect=cell<void*>(cell<void*>(section,0x20),0x7c);
    if(!visible_after_native_fade(command,&a,entry)) return;
    void* const binding=cell<void*>(section,0x5c);
    if(!binding) {
        void* batch;
        if(is_faded(entry,a.one_00d7a24c)) batch=cell<void*>(command,0x10);
        else batch=cell<void*>(command,0xc+4u*native_effect_batch_index_00b17300(effect));
        append_native_render_batch_entry_00b51cb0(*static_cast<NativeRenderBatchStorage*>(batch),entry);
        return;
    }
    const auto category=collection_category(entry);
    auto& indexed=*static_cast<NativeRenderPointerArrayStorage*>(plus(command,0x2c));
    NativeRenderGroupStorage* group=indexed_group(indexed,cell<std::uint32_t>(binding,8));
    if(!group) {
        void* const allocation=a.allocate_00bf681b(0x4c);
        NativeRenderGroupStorage* const constructed=allocation?construct_native_render_group_00b1d6f0(allocation):nullptr;
        const auto publish_id=cell<std::uint32_t>(binding,8);
        if(signed_bits(publish_id)>=cell<std::int32_t>(&indexed,4))
            resize_native_render_group_pointers_00b1c7c0(indexed,signed_bits(publish_id+1u));
        cell<void*>(plus(cell<void*>(&indexed),publish_id*4u))=constructed;
        group=indexed_group(indexed,cell<std::uint32_t>(binding,8));
        assign_native_instance_binding_00b1ca50(group->binding_00,&binding,*a.owners);
        {
            NativeString separator,first,second;
            construct_native_string_cstring_0041e870(&separator,a.separator_00d21d00,*a.strings);
            TemporaryName cleanup0{&separator,*a.strings};
            const void* const current_effect=cell<void*>(cell<void*>(section,0x20),0x7c);
            concatenate_native_string_headers_004261a0(native_effect_name_00b172d0(current_effect),&first,&separator,*a.strings);
            TemporaryName cleanup1{&first,*a.strings};
            const auto& model_name=native_node_name_00b6d800(*static_cast<NativeNodeStorage*>(cell<void*>(entry,0xc)));
            concatenate_native_string_headers_004261a0(&first,&second,&model_name,*a.strings);
            TemporaryName cleanup2{&second,*a.strings};
            assign_native_string_header_00425f40(plus(group,4),&second,*a.strings);
            cleanup2.finish(); cleanup1.finish(); cleanup0.finish();
        }
        initialize_colors(a);
        for(unsigned i=0;i!=2;++i) {
            cell<std::uint32_t>(group,0xc+i*4u)=0;
            void* const cache_header=plus(*a.entry_cache_0108fe88,4);
            const auto n=static_cast<std::uint32_t>(InterlockedIncrement(static_cast<volatile LONG*>(plus(cache_header,4))));
            cell<void*>(group,0x14+i*4u)=plus(cell<void*>(cache_header),n*0x28u-0x28u);
            const auto& model_name=native_node_name_00b6d800(*static_cast<NativeNodeStorage*>(cell<void*>(entry,0xc)));
            NativeString generated_name;
            prefix_native_string_header_0043c130(&generated_name,a.instance_prefix_00d5e5e4,&model_name,*a.strings);
            TemporaryName cleanup{&generated_name,*a.strings};
            *a.acquired_geometry={};
            cell<void*>(group,0x1c+i*4u)=create_native_instance_geometry_00b4c8d0(generated_name,
                cell<void*>(binding,0xc),*static_cast<NativeMeshStorage*>(cell<void*>(entry,8)),
                *static_cast<NativeMeshSectionStorage*>(section),*a.geometry,*a.acquired_geometry);
            cleanup.finish();
            void* const generated=cell<void*>(group,0x1c+i*4u);
            const auto color_id=cell<std::uint32_t>(binding,8);
            void* const mesh=gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(plus(generated,0x174)),0);
            void* const generated_section=gui_geometry_element_00b732c0(mesh,0);
            auto& material=*static_cast<NativeMaterialStorage*>(cell<void*>(generated_section,0x20));
            void* const color=native_material_diffuse_00b179f0(material,0);
            const volatile auto* const source=a.colors_00f8d450+(color_id%5u)*4u;
            for(unsigned j=0;j!=4;++j) cell<std::uint32_t>(color,j*4u)=source[j];
            reserve_native_instance_entry_pointers_00b1c500(group->source_entries_24[i],8);
        }
        cell<std::uint32_t>(group,category?0xc:0x10)=1;
        append_borrowed(*static_cast<NativeRenderPointerArrayStorage*>(plus(command,0x38)),group,true);
    } else {
        group=indexed_group(indexed,cell<std::uint32_t>(binding,8));
        cell<std::uint32_t>(group,category?0xc:0x10)+=1u;
        group=indexed_group(indexed,cell<std::uint32_t>(binding,8));
    }
    append_borrowed(group->source_entries_24[category?0:1],entry,false);
}
void NativeInstanceCollectingRenderServices::collect_entry_00b1dff0(void* command,void* entry) {
    collect_native_instance_entry_00b1dff0(command,entry,collection_);
}
} // namespace bsp
