#include "bsp/native_resource_graph_builder.hpp"
#include "bsp/gui_group_bounds.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_resource_instance_lifecycle.hpp"
#include "bsp/native_singleton_vector_insert_count.hpp"
#include "bsp/native_tracer_update.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource graph construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p, Word offset=0) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);}
template<class T> T read(const void* p, Word offset=0) noexcept {return *static_cast<const volatile T*>(at(p,offset));}
template<class T> void write(void* p, Word offset, T value) noexcept {*static_cast<volatile T*>(at(p,offset))=value;}
std::int32_t signed_word(Word value) noexcept {std::int32_t r;std::memcpy(&r,&value,4);return r;}
Word count(void* begin,void* end) noexcept {return static_cast<Word>(signed_word(reinterpret_cast<Word>(end)-reinterpret_cast<Word>(begin))>>2);}
void check_index(void* instance,Word index) {
    void* const begin=read<void*>(instance,0x14);
    if(!begin||index>=count(begin,read<void*>(instance,0x18)))_invalid_parameter_noinfo();
}
void* node_at(void* instance,Word index) {
    check_index(instance,index);return read<void*>(read<void*>(instance,0x14),index*4u);
}
Word slot(void* owner,Word offset,NativeResourceGraphContext& context) {
    return context.calls.table(read<Word>(owner))[offset/4u];
}
struct Factory {
    volatile Word profile;
    ~Factory() {profile=0x00d63208u;}
};
struct TemporaryString {
    NativeString value;
    NativeStringStorage& strings;
    bool armed=false;
    explicit TemporaryString(NativeStringStorage& pool):strings(pool){}
    void destroy() noexcept {
        if(armed){armed=false;destroy_native_string_header_0041dd20(&value,strings);}
    }
    ~TemporaryString(){destroy();}
};
void literal(TemporaryString& text,const char* data,Word length) {
    text.value.resize_0041dd40(text.strings,length,true);
    if(text.value.data())std::memcpy(text.value.data(),data,text.value.length()+1u);
    text.armed=true;
}
void build_node(void* resource,void* instance,Word index,void*& root,
    Factory& default_factory,Factory& group_factory,NativeResourceGraphContext& context) {
    void* const record=read<void*>(read<void*>(resource,0x1c),index*4u);
    void* factory=&default_factory;
    for(Word j=0;signed_word(j)<read<std::int32_t>(record,0x50);++j) {
        const Word item_index=read<Word>(read<void*>(record,0x4c),j*4u);
        void* const item=read<void*>(read<void*>(resource,0x10),item_index*4u);
        void* const selected=context.calls.item_factory(slot(item,0x1c,context),item);
        if(selected)factory=selected;
    }
    TemporaryString suffix(context.strings),number(context.strings),prefix(context.strings),joined(context.strings),complete(context.strings),name(context.strings);
    const void* selected_name=at(record,4);
    if(!read<Word>(selected_name)) {
        literal(suffix,">",1);
        native_string_from_int_004260b0(number.value,signed_word(index),context.strings);number.armed=true;
        literal(prefix,"<node ",6);
        concatenate_native_string_headers_004261a0(&prefix.value,&joined.value,&number.value,context.strings);joined.armed=true;
        selected_name=concatenate_native_string_headers_004261a0(&joined.value,&complete.value,&suffix.value,context.strings);complete.armed=true;
    }
    copy_native_string_header_00be0a30_fragment(&name.value,context.strings,selected_name);
    name.armed=true;
    complete.destroy();joined.destroy();prefix.destroy();number.destroy();suffix.destroy();
    if(read<std::uint8_t>(record,0x58)&1u)factory=&group_factory;
    check_index(instance,index);
    // Capture the destination CELL before factory dispatch, which can change
    // the current vector. Publish to that captured cell, then validate afresh.
    void* const cell=at(read<void*>(instance,0x14),index*4u);
    void* const created=context.calls.create_node(slot(factory,4,context),factory,&name.value);
    write(cell,0,created);
    void* const node=node_at(instance,index);
    context.calls.set_matrix(slot(node,0x38,context),node,at(record,0xc));
    if(index==0)root=node;
    else {
        const auto parent_index=read<std::int32_t>(record);
        void* const parent=parent_index<0?root:node_at(instance,static_cast<Word>(parent_index));
        auto& child=context.calls.node_binding(node).transform;
        CameraTransform* const parent_transform=parent?&context.calls.node_binding(parent).transform:nullptr;
        set_native_node_parent_00b6e680(context.parenting,child,parent_transform);
    }
    const auto model_table=context.calls.table(read<Word>(node));
    const Word model_token=native_graph_model_type_00b74310(context.model_type_01090034);
    if(context.calls.is_type(model_table[3],node,model_token)) {
        set_native_generated_model_bounds_00b74390(node,static_cast<const float*>(at(record,0x5c)));
        for(Word j=0;j<6;++j) {
            void* const destination=at(node,0x18+j*4u);const void* const source=at(record,0x6c+j*4u);
            __asm {mov eax, source}
            __asm {mov ecx, destination}
            __asm {fld dword ptr [eax]}
            __asm {fstp dword ptr [ecx]}
        }
    } else {
        const auto group_table=context.calls.table(read<Word>(node));
        const Word group_token=native_graph_group_type_00b8e600(context.group_type_0109032c);
        if(context.calls.is_type(group_table[3],node,group_token)) {
            auto& group=context.calls.group_owner(node);
            if(read<std::uint8_t>(record,0x58)&2u)
                enable_native_graph_group_auto_bounds_00b8f0f0(context.group_spheres,group);
            else {
                set_native_graph_group_box_00b8e6f0(node,nullptr,at(record,0x6c));
                set_native_gui_group_bounds_00b8e6c0(group,at(record,0x5c));
            }
        }
    }
}
} // namespace

void* __fastcall build_native_resource_graph_00b891a0(void* resource,NativeResourceGraphContext& context,Word creation_word,Word) {
    const auto create=slot(resource,0x10,context);
    void* const instance=create==0x0071aed0u?create_native_game_resource_instance_0071aed0(resource):context.calls.create_instance(create,resource);
    resize_native_resource_node_vector_00b88b60(at(instance,0x10),nullptr,read<Word>(resource,0x20),0);
    Factory plain{0x00d63210u};Factory model{0x00d63218u};Factory group{0x00d63220u};
    void* root=nullptr;
    for(Word i=0;signed_word(i)<read<std::int32_t>(resource,0x20);++i)
        build_node(resource,instance,i,root,model,group,context);
    write(instance,0xc,root);
    for(Word i=0;signed_word(i)<read<std::int32_t>(resource,0x20);++i) {
        void* const record=read<void*>(read<void*>(resource,0x1c),i*4u);
        void* const node=node_at(instance,i);
        for(Word j=0;signed_word(j)<read<std::int32_t>(record,0x50);++j) {
            const Word item_index=read<Word>(read<void*>(record,0x4c),j*4u);
            void* const item=read<void*>(read<void*>(resource,0x10),item_index*4u);
            context.calls.attach_item(slot(item,0x18,context),item,instance,record,node,creation_word);
        }
    }
    postprocess_native_resource_instance_00b79bc0(instance,context.postprocess);
    bind_native_resource_mesh_bones_00b87e80(instance,context.bones);
    return instance;
}
void* __fastcall build_native_game_resource_graph_007137f0(void* resource,NativeResourceGraphContext& context,Word creation_word,Word float_word) {
    __asm {fld dword ptr float_word}
    __asm {fstp dword ptr float_word}
    return build_native_resource_graph_00b891a0(resource,context,creation_word,float_word);
}
void __fastcall resize_native_resource_node_vector_00b88b60(void* vector,void*,Word requested,Word value) {
    void* const begin=read<void*>(vector,4);
    const auto initial=begin?count(begin,read<void*>(vector,8)):0u;
    if(initial<requested) {
        const auto current=begin?count(begin,read<void*>(vector,8)):0u;
        void* const end=read<void*>(vector,8);
        if(reinterpret_cast<Word>(begin)>reinterpret_cast<Word>(end))_invalid_parameter_noinfo();
        insert_count_native_singleton_slots_00bd0700(vector,nullptr,vector,end,requested-current,&value);
    } else if(begin) {
        void* const old_end=read<void*>(vector,8);
        if(requested>=count(begin,old_end))return;
        if(reinterpret_cast<Word>(begin)>reinterpret_cast<Word>(old_end))_invalid_parameter_noinfo();
        void* const current_begin=read<void*>(vector,4);
        if(reinterpret_cast<Word>(current_begin)>read<Word>(vector,8))_invalid_parameter_noinfo();
        void* const first=at(current_begin,requested*4u);
        if(reinterpret_cast<Word>(first)>read<Word>(vector,8)||reinterpret_cast<Word>(first)<read<Word>(vector,4))_invalid_parameter_noinfo();
        void* output[2];erase_native_graph_node_range_004fcbe0(vector,nullptr,output,vector,first,vector,old_end);
    }
}
void* __fastcall erase_native_graph_node_range_004fcbe0(void* vector,void*,void* output,void* first_owner,void* first,void* last_owner,void* last) {
    if(!first_owner||first_owner!=last_owner)_invalid_parameter_noinfo();
    if(first!=last) {
        const auto remaining=signed_word(count(last,read<void*>(vector,8)));
        const Word bytes=static_cast<Word>(remaining)*4u;
        void* const end=at(first,bytes);
        if(remaining>0)(void)memmove_s(first,bytes,last,bytes);
        write(vector,8,end);
    }
    write(output,4,first);write(output,0,first_owner);return output;
}
Word native_graph_model_type_00b74310(const volatile Word& current) noexcept {return current;}
Word native_graph_group_type_00b8e600(const volatile Word& current) noexcept {return current;}
__declspec(naked) void __fastcall set_native_graph_group_box_00b8e6f0(void*,void*,const void*) noexcept {
    __asm {
        and dword ptr [ecx+138h],0ffffffcfh
        mov eax,dword ptr [esp+4]
        mov byte ptr [ecx+175h],0
        fld dword ptr [eax]
        fstp dword ptr [ecx+18h]
        fld dword ptr [eax+4]
        fstp dword ptr [ecx+1ch]
        fld dword ptr [eax+8]
        fstp dword ptr [ecx+20h]
        fld dword ptr [eax+0ch]
        fstp dword ptr [ecx+24h]
        fld dword ptr [eax+10h]
        fstp dword ptr [ecx+28h]
        fld dword ptr [eax+14h]
        fstp dword ptr [ecx+2ch]
        ret 4
    }
}
void enable_native_graph_group_auto_bounds_00b8f0f0(NativeGroupWorldSphereRuntime& runtime,NativeGroupOwner& group) {
    write(&group.node.storage,0x175,std::uint8_t{1});
    aggregate_native_group_world_sphere_00b8ebe0(runtime,group);
}
} // namespace bsp
