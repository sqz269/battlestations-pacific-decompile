#include "bsp/native_renderer_debug_records24.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/camera_transform.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_string.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p)+offset);
}
void* at(void* p,U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
void store(void* p,U offset,U value) noexcept {
    *reinterpret_cast<volatile U*>(at(p,offset))=value;
}
std::int32_t signed_bits(U bits) noexcept {
    std::int32_t value;std::memcpy(&value,&bits,4);return value;
}
void require(bool condition,const char* why) {
    if(!condition)throw std::invalid_argument(why);
}
void literal(NativeString& header,NativeStringRawPoolContext& pool,
    const char* text,U length) {
    // The native constructor/inlined constructor zeros BOTH words first.
    store(&header,0,0);store(&header,4,0);
    resize_native_string_header_0041dd40(&header,pool,length,true);
    if(void* data=read<void*>(&header,4))std::memcpy(data,text,read<U>(&header)+1u);
}
void* make_model(void* renderer_to_publish,NativeRendererGeneratedModelContext& c,
    NativeRendererGeneratedModelAcquired& acquired,void** retained_model=nullptr) {
    auto& pool=c.model_and_geometry.models.nodes.require_raw_name_pool();
    require(&pool.actual_manager_publication_01090aa0==&c.actual_manager_01090aa0,
        "debug records require the same actual AA0 string manager");
    NativeString effect,layout,name;
    int stage=-1;
    try {
        // Cold uses 41E870; record uses its inlined 41DD40/BF7680 sequence.
        // The exact known lengths and current post-resize header reads agree.
        literal(effect,pool,"debugshader.mshd",0x10);stage=0;
        literal(layout,pool,"pf43cc.mvfm",0x0b);stage=1;
        literal(name,pool,"DebugSpheres",0x0c);stage=2;
        void* model=create_native_renderer_generated_model_00b4c700(
            name,layout,effect,3,0,0,0,c,acquired);
        if(retained_model)*retained_model=model;
        if(renderer_to_publish)store(renderer_to_publish,0x19e4,reinterpret_cast<U>(model));
        stage=1;destroy_native_string_header_0041dd20(&name,pool);
        stage=0;destroy_native_string_header_0041dd20(&layout,pool);
        stage=-1;destroy_native_string_header_0041dd20(&effect,pool);
        return model;
    } catch(...) {
        // No model cleanup occurs in B2BB90's native FH3 map. A throwing
        // normal string cleanup has already disarmed itself before entry.
        try {
            if(stage>=2)destroy_native_string_header_0041dd20(&name,pool);
            if(stage>=1)destroy_native_string_header_0041dd20(&layout,pool);
            if(stage>=0)destroy_native_string_header_0041dd20(&effect,pool);
        } catch(...) {std::terminate();}
        throw;
    }
}
void* geometry(void* model) noexcept {
    return gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(at(model,0x174)),0);
}
void* stream(void* model) noexcept {
    return native_mesh_vertex_stream_00b73260(*static_cast<NativeMeshStorage*>(geometry(model)),0);
}
void* section(void* model) noexcept {return gui_geometry_element_00b732c0(geometry(model),0);}
void current_stream_slot(void* actual,const NativeRendererDebugRecords24Context& c,U slot,U expected) {
    const auto* profile=c.generated.model_and_geometry.streams.actual_logical_profile_00d61d6c;
    require(read<U>(actual)==0x00d61d6c&&profile&&profile[slot/4]==expected,
        "debug records require the current actual logical vertex slot");
}
NativeModelReference& model_reference(void* actual,NativeRendererGeneratedModelContext& c) {
    auto* reference=dynamic_cast<NativeModelReference*>(&c.model_and_geometry.geometry.actual_owners().resolve_actual(actual));
    require(reference && &reference->model_owner().storage.node==actual,
        "debug records require the canonical actual model reference");
    return *reference;
}
void local_identity(void* renderer,NativeRendererDebugRecords24Context& c) {
    const U one=*reinterpret_cast<const volatile U*>(c.render_entry.one_00d7a24c);
    void* actual=read<void*>(renderer,0x19e4);
    auto& owner=model_reference(actual,c.generated).model_owner();
    const auto* table=owner.environment.vtable_00d62de8;
    require(read<U>(actual)==0x00d62de8&&table&&table[0x38/4]==0x00b6db10,
        "debug cached model requires current local-matrix virtual38");
    auto& transform=owner.node.transform;
    require(transform.raw_node_key()==reinterpret_cast<U>(actual)&&
        &transform.local==&owner.storage.node.local_b0&&
        &transform.valid_flags==&owner.storage.node.valid_flags_5c,
        "debug cached model transform must borrow actual native storage");
    CameraMatrix matrix;
    for(U i=0;i<16;++i)store(matrix.data(),i*4,i%5==0?one:0u);
    set_raw_local_matrix_00b6db10(actual,c.render_entry.services,matrix.data());
}
}

void __fastcall draw_native_renderer_debug_records24_00b2bb90(void* renderer,
    NativeRendererDebugRecords24Context* context,NativeRendererDebugRecords24Frame* frame) {
    if(read<U>(renderer,0x1d10)==0)return;
    if(read<U>(renderer,0x19e4)==0) {
        require(context&&frame,"cold debug model requires actual contexts and persistent acquisitions");
        (void)make_model(renderer,context->generated,frame->cached_model);
        // String returns may change +19E4: reload its CURRENT actual model.
        local_identity(renderer,*context);
    }
    U ordinal=0,offset=0;
    if(read<std::int32_t>(renderer,0x1d10)>0)do {
        void* record=at(read<void*>(renderer,0x1d0c),offset);
        U sphere[4];sphere[0]=read<U>(record);
        void* camera=read<void*>(record,0x14);
        const U camera_mode=read<U>(camera,0x198);
        sphere[1]=read<U>(record,4);sphere[2]=read<U>(record,8);sphere[3]=read<U>(record,0xc);
        if(camera_mode==0) {
            require(context&&frame&&frame->used_record_frames<frame->record_frame_count,
                "active debug record requires a fresh caller-owned frame");
            auto* acquired=frame->records[frame->used_record_frames++];
            require(acquired&&!acquired->started,
                "debug record dependency frames must be fresh and persistent");
            acquired->started=true;
            auto& c=*context;
            require(c.render_entry.mapping==&c.mapping,
                "debug entry and stream contexts must share actual mapping");
            void* model=make_model(nullptr,c.generated,acquired->generated,&acquired->unconsumed_model);
            // Selector is reloaded AFTER all generated-model/string callbacks.
            U selector=read<U>(at(read<void*>(renderer,0x1d0c),offset),0x10);
            void* current_stream=stream(model);current_stream_slot(current_stream,c,0x10,0x00b49980);
            void* output=lock_native_logical_vertex_stream_00b49980(current_stream,c.mapping,0x26,0,0);
            const U color=selector==0x555u?0xff0000ffu:0xffff0000u;
            write_native_debug_sphere_vertices_00b2bf60(output,sphere,color,&c.actual_angle_step_00cec730);
            current_stream=stream(model);current_stream_slot(current_stream,c,0x14,0x00b49a80);
            unlock_native_logical_vertex_stream_00b49a80(current_stream,c.mapping);
            store(section(model),0x18,0x25);store(section(model),0x10,0x26);
            gather_native_system_constants_00b46a70(nullptr,camera,c.gather,acquired->gather);
            void* cache=*c.render_entry.entry_cache_0108fe88;
            const long count=c.pass.actual_increment_00ce221c(reinterpret_cast<volatile long*>(at(cache,8)));
            float visibility,depth_override;
            __asm {
                fldz
                fstp depth_override
                fld1
                fstp visibility
            }
            void* entry=at(read<void*>(cache,4),(static_cast<U>(count)-1u)*0x28u);
            void* captured_geometry=geometry(model);
            void* captured_section=section(model);
            float leading;
            __asm {
                fldz
                fstp leading
            }
            initialize_native_render_entry_00b51a20(entry,&c.render_entry,leading,
                captured_section,captured_geometry,model,camera,visibility,depth_override,0);
            void* material=read<void*>(section(model),0x20);
            void* effect=read<void*>(material,0x7c);
            void* pass=read<void*>(effect,0x9c);
            execute_native_material_pass_current08(pass,entry,c.pass,acquired->pass);
            if(model) {
                auto& reference=model_reference(model,c.generated);
                acquired->unconsumed_model=nullptr; // Native normal call is consumed once.
                unlink_and_release_render_model_00b6dfa0(reference);
            }
            acquired->complete=true;
        }
        offset+=0x18u;++ordinal;
    }while(signed_bits(ordinal)<read<std::int32_t>(renderer,0x1d10));
    void* header=at(renderer,0x1d0c);
    if(read<std::int32_t>(header,8)<0)reserve_native_renderer_records24_00b229d0(header,0,0);
    while(read<std::int32_t>(header,4)>0)store(header,4,read<U>(header,4)-1u);
    store(header,4,0);
}

} // namespace bsp
