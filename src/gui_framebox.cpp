#include "bsp/gui_framebox.hpp"
#include "bsp/gui_lua_reader.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> void required(const T& service, const char* name) {
    if (!service) throw std::logic_error(name);
}
void read_value(const GuiTable& table,const char* name,GuiLuaFieldType type,void* out,
    const bool& crt_sse2_conversion) {
    const auto* value=table.find(name);
    if (value && value->kind()!=GuiValue::Kind::Nil)
        gui_lua_store_value_00bd63b0(*value,gui_lua_field(type,out),nullptr,crt_sse2_conversion);
}
float rounded(double value) noexcept { return static_cast<float>(value); }
float sub2(float value,float first,float second) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {
        fld value
        fsub first
        fsub second
        fstp result
    }
    return result;
#else
    return rounded((static_cast<double>(value)-first)-second);
#endif
}
float scaled_sub2(float value,float scale,float first,float second) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {
        fld value
        fmul scale
        fsub first
        fsub second
        fstp result
    }
    return result;
#else
    return rounded((static_cast<double>(value)*scale-first)-second);
#endif
}
float uv_border(float border,float texture_extent,float span) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {
        fld border
        fdiv texture_extent
        fmul span
        fstp result
    }
    return result;
#else
    return rounded((static_cast<double>(border)/texture_extent)*span);
#endif
}
float uv_border_y(float border,float texture_extent,float scale,float span) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {
        fld border
        fdiv texture_extent
        fdiv scale
        fmul span
        fstp result
    }
    return result;
#else
    return rounded(((static_cast<double>(border)/texture_extent)/scale)*span);
#endif
}
struct Rect { float x,y,w,h; };
void write_rect(std::array<GuiQuadVertex,54>& out,std::size_t offset,
    const Rect& p,const Rect& uv) {
    const auto right=rounded(static_cast<double>(p.x)+p.w);
    const auto bottom=rounded(static_cast<double>(p.y)+p.h);
    const auto u_right=rounded(static_cast<double>(uv.x)+uv.w);
    const auto v_bottom=rounded(static_cast<double>(uv.y)+uv.h);
    out[offset]={p.x,p.y,0,uv.x,uv.y};
    out[offset+1]={right,p.y,0,u_right,uv.y};
    out[offset+2]={right,bottom,0,u_right,v_bottom};
    out[offset+3]=out[offset];
    out[offset+4]=out[offset+2];
    out[offset+5]={p.x,bottom,0,uv.x,v_bottom};
}
void copy_state_dimensions(GuiLayoutWidget& widget,const GuiFrameBoxState& state) {
    widget.transform.size=state.size;
    widget.transform.pivot_x=state.pivot[0];
    widget.transform.pivot_y=state.pivot[1];
}
}

GuiWidgetSize gui_framebox_native_size_00aceb70(std::uint32_t width,
    std::uint32_t height) noexcept {
    return {rounded(static_cast<double>(width)/960.0),
        rounded(static_cast<double>(height)/720.0)};
}

std::int16_t gui_framebox_add_state_00ad2b30(GuiFrameBoxWidget& frame,
    const GuiWidgetTransform& widget,const std::string& name,const float* pivot,
    const GuiWidgetSize* size,const GuiFrameBoxTextureServices& services) {
    required(services.release,"FrameBox texture release service missing");
    GuiFrameBoxState record;
    record.texture_name=name;
    record.pivot=pivot?std::array<float,2>{pivot[0],pivot[1]}:
        std::array<float,2>{widget.pivot_x,widget.pivot_y};
    record.size=size?*size:widget.size;
    std::array<float,4> uv{0,0,1,1};
    std::array<float,2> extent{record.size.width,record.size.height};
    void* texture=resolve_gui_texture_00aa2660(name,uv,extent,1.0f,services.callbacks);
    if (!texture) throw std::runtime_error("FrameBox texture resolution returned null: "+name);
    record.texture=std::shared_ptr<void>(texture,services.release);
    record.uv={uv[0],uv[1],uv[2],uv[3]};
    record.size={extent[0],extent[1]};
    if (record.size.width==0.0f && record.size.height==0.0f) {
        required(services.callbacks.width,"FrameBox texture width service missing");
        required(services.callbacks.height,"FrameBox texture height service missing");
        record.size=gui_framebox_native_size_00aceb70(services.callbacks.width(texture),
            services.callbacks.height(texture));
    }
    frame.states.push_back(std::move(record));
    const auto bits=static_cast<std::uint16_t>(frame.states.size()-1);
    std::int16_t result;
    std::memcpy(&result,&bits,sizeof result);
    return result;
}

void gui_framebox_read_properties_00ad08e0(GuiFrameBoxWidget& frame,
    const GuiWidgetTransform& widget,const GuiTable& table,
    const GuiFrameBoxTextureServices& services,const bool& crt_sse2_conversion) {
    frame.has_texture=true; frame.shader_name.clear();
    frame.frame_sizes_x={kGuiFrameBoxDefaultX,kGuiFrameBoxDefaultX};
    frame.frame_sizes_y={kGuiFrameBoxDefaultY,kGuiFrameBoxDefaultY};
    read_value(table,"HasTexture",GuiLuaFieldType::Bool,&frame.has_texture,crt_sse2_conversion);
    read_value(table,"ShaderName",GuiLuaFieldType::String,&frame.shader_name,crt_sse2_conversion);
    read_value(table,"FrameSizesX",GuiLuaFieldType::Vec2,frame.frame_sizes_x.data(),crt_sse2_conversion);
    read_value(table,"FrameSizesY",GuiLuaFieldType::Vec2,frame.frame_sizes_y.data(),crt_sse2_conversion);
    const auto* states_value=table.find("States");
    if (!states_value || states_value->kind()==GuiValue::Kind::Nil) return;
    const auto* states=states_value->table();
    if (!states) throw std::runtime_error("FrameBox States scope is not a table");
    for (const auto& value:states->array) {
        if (value.kind()==GuiValue::Kind::Nil) break;
        const auto* state=value.table();
        if (!state) throw std::runtime_error("FrameBox state scope is not a table");
        std::string texture;
        std::array<float,2> size{widget.size.width,widget.size.height};
        std::array<float,2> pivot{widget.pivot_x,widget.pivot_y};
        read_value(*state,"Texture",GuiLuaFieldType::String,&texture,crt_sse2_conversion);
        read_value(*state,"Size",GuiLuaFieldType::Vec2,size.data(),crt_sse2_conversion);
        read_value(*state,"Pivot",GuiLuaFieldType::Vec2,pivot.data(),crt_sse2_conversion);
        const GuiWidgetSize extent{size[0],size[1]};
        gui_framebox_add_state_00ad2b30(frame,widget,texture,pivot.data(),&extent,services);
    }
}

void gui_framebox_read_properties_00ad08e0(GuiFrameBoxWidget& frame,
    const GuiWidgetTransform& widget,GuiLuaReader& reader,
    const GuiFrameBoxTextureServices& services) {
    const auto read=[&](const char* name,GuiLuaFieldType type,void* out,
        GuiLuaVariant fallback) {
        reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
            gui_lua_field(type,out),fallback);
    };
    auto fallback=gui_lua_key_by_index(1);
    read("HasTexture",GuiLuaFieldType::Bool,&frame.has_texture,fallback);
    read("ShaderName",GuiLuaFieldType::String,&frame.shader_name,gui_lua_key_by_name(""));
    const std::array<float,2> x{kGuiFrameBoxDefaultX,kGuiFrameBoxDefaultX};
    const std::array<float,2> y{kGuiFrameBoxDefaultY,kGuiFrameBoxDefaultY};
    fallback.value.pointer=const_cast<float*>(x.data());
    read("FrameSizesX",GuiLuaFieldType::Vec2,frame.frame_sizes_x.data(),fallback);
    fallback.value.pointer=const_cast<float*>(y.data());
    read("FrameSizesY",GuiLuaFieldType::Vec2,frame.frame_sizes_y.data(),fallback);
    const auto states_key=gui_lua_key_by_name("States");
    if (!reader.has_key_00bd5eb0(states_key)) return;
    const auto starting_depth=reader.depth();
    try {
        reader.enter_00bd8e20(states_key);
        for (std::int32_t index=1; reader.has_key_00bd5eb0(gui_lua_key_by_index(index)); ++index) {
            reader.enter_00bd8e20(gui_lua_key_by_index(index));
            std::string texture;
            std::array<float,2> size{widget.size.width,widget.size.height};
            std::array<float,2> pivot{widget.pivot_x,widget.pivot_y};
            read("Texture",GuiLuaFieldType::String,&texture,gui_lua_key_by_name(""));
            const auto size_default=size, pivot_default=pivot;
            fallback.value.pointer=const_cast<float*>(size_default.data());
            read("Size",GuiLuaFieldType::Vec2,size.data(),fallback);
            fallback.value.pointer=const_cast<float*>(pivot_default.data());
            read("Pivot",GuiLuaFieldType::Vec2,pivot.data(),fallback);
            const GuiWidgetSize extent{size[0],size[1]};
            gui_framebox_add_state_00ad2b30(frame,widget,texture,pivot.data(),&extent,services);
            reader.leave_00bd7a20();
            if (index==std::numeric_limits<std::int32_t>::max())
                throw std::overflow_error("FrameBox state index overflow");
        }
        reader.leave_00bd7a20();
    } catch (...) {
        while (reader.depth()>starting_depth) reader.leave_00bd7a20();
        throw;
    }
}

std::array<GuiQuadVertex,54> gui_framebox_write_geometry_00acf9b0(
    const GuiFrameBoxWidget& frame,const GuiWidgetSize& size,const GuiFrameBoxState& state,
    std::uint32_t texture_width,std::uint32_t texture_height,float scale) {
    const auto native=gui_framebox_native_size_00aceb70(texture_width,texture_height);
    const auto u_span=rounded(static_cast<double>(state.uv.right)-state.uv.left);
    const auto v_span=rounded(static_cast<double>(state.uv.bottom)-state.uv.top);
    const auto texture_x=rounded(static_cast<double>(u_span)*native.width);
    const auto texture_y=rounded(static_cast<double>(v_span)*native.height);
    const float left=frame.frame_sizes_x[0],right=frame.frame_sizes_x[1];
    const auto top=rounded(static_cast<double>(frame.frame_sizes_y[0])*scale);
    const auto bottom=rounded(static_cast<double>(frame.frame_sizes_y[1])*scale);
    const auto center_x=sub2(size.width,left,right);
    const auto center_y=scaled_sub2(size.height,scale,top,bottom);
    const auto u_left=uv_border(left,texture_x,u_span);
    const auto u_right=uv_border(right,texture_x,u_span);
    const auto v_top=uv_border_y(top,texture_y,scale,v_span);
    const auto v_bottom=uv_border_y(bottom,texture_y,scale,v_span);
    const auto u_center=sub2(u_span,u_left,u_right);
    const auto v_center=sub2(v_span,v_top,v_bottom);
    const double u_middle=static_cast<double>(state.uv.left)+u_left;
    const double v_middle=static_cast<double>(state.uv.top)+v_top;
    const float px[]{0,left,rounded(static_cast<double>(left)+center_x)};
    const float py[]{0,top,rounded(static_cast<double>(top)+center_y)};
    const float pw[]{left,center_x,right},ph[]{top,center_y,bottom};
    const float ux[]{state.uv.left,rounded(u_middle),rounded(u_middle+u_center)};
    const float vy[]{state.uv.top,rounded(v_middle),rounded(v_middle+v_center)};
    const float uw[]{u_left,u_center,u_right},vh[]{v_top,v_center,v_bottom};
    constexpr std::size_t cells[][2]{{0,0},{2,0},{0,2},{2,2},{1,0},{0,1},{2,1},{1,2},{1,1}};
    std::array<GuiQuadVertex,54> out{};
    for (std::size_t i=0;i<9;++i) {
        const auto x=cells[i][0],y=cells[i][1];
        write_rect(out,i*6,{px[x],py[y],pw[x],ph[y]},{ux[x],vy[y],uw[x],vh[y]});
    }
    return out;
}

const GuiFrameBoxState& gui_framebox_state_at(const GuiFrameBoxWidget& frame,
    std::int16_t index) {
    if (static_cast<std::uint32_t>(static_cast<std::int32_t>(index))>=frame.states.size())
        throw std::range_error("FrameBox native state range check");
    return frame.states[static_cast<std::size_t>(index)];
}
const char* gui_framebox_material_name(const GuiFrameBoxWidget& frame) noexcept {
    return !frame.shader_name.empty()?frame.shader_name.c_str():
        frame.has_texture?"GuiDefault.mshd":"guifade.mshd";
}
GuiFrameBoxWidget gui_framebox_copy_state_00ad27a0(const GuiFrameBoxWidget& source) {
    auto out=source;
    out.shader_name.clear();
    return out;
}
void gui_framebox_constructed74_00acf8f0(GuiLayoutWidget& widget,
    const GuiFrameBoxRuntimeServices& services) {
    if (!widget.transform.bounds_enabled) {
        required(services.geometry.associate_geometry,"FrameBox geometry association missing");
        services.geometry.associate_geometry(std::make_shared<GeneratedInstanceGeometry>(),-1.0f,-1.0f);
    }
    required(services.set_position_00aa7dc0,"FrameBox position/bounds service missing");
    services.set_position_00aa7dc0({0,0,0});
}
void gui_framebox_rebuild_00ad0d80(GuiFrameBoxWidget& frame,GuiLayoutWidget& widget,
    float& overbright,std::int16_t index,const GuiFrameBoxRuntimeServices& services) {
    frame.current_state=index;
    GuiGeometryRebuildRequest request;
    request.vertex_count=54; request.primitive=4; request.range_words={0,54,0,18};
    if (frame.has_texture) {
        const auto& state=gui_framebox_state_at(frame,index);
        required(services.logical_texture,"FrameBox logical texture ownership service missing");
        request.texture=services.logical_texture(state.texture.get());
        if (!request.texture) throw std::runtime_error("FrameBox logical texture is null");
    }
    request.choose_material=[&](bool existing) {
        return GuiGeometryMaterialChoice{gui_framebox_material_name(frame),existing};
    };
    request.prepare_vertices=[&](std::vector<GuiQuadVertex>& vertices,bool writes) {
        if (frame.has_texture) copy_state_dimensions(widget,gui_framebox_state_at(frame,index));
        if (!writes) return false;
        // Native00ACF9B0 dereferences the selected state even when HasTexture
        // is false. A missing state/texture is an error, never a blank fallback.
        const auto& state=gui_framebox_state_at(frame,index);
        if (!state.texture) throw std::runtime_error("FrameBox geometry texture is null");
        required(services.textures.callbacks.width,"FrameBox texture width missing");
        required(services.textures.callbacks.height,"FrameBox texture height missing");
        const auto geometry=gui_framebox_write_geometry_00acf9b0(frame,widget.transform.size,
            state,services.textures.callbacks.width(state.texture.get()),
            services.textures.callbacks.height(state.texture.get()),services.y_scale);
        vertices.assign(geometry.begin(),geometry.end());
        return true;
    };
    rebuild_gui_geometry_00ab3cb0_fragment(widget,overbright,services.geometry,request);
}
void gui_framebox_select_state_00acf070(GuiFrameBoxWidget& frame,GuiLayoutWidget& widget,
    float& overbright,std::int16_t index,const GuiFrameBoxRuntimeServices& services) {
    if (index==frame.current_state) return;
    const auto unsigned_index=static_cast<std::uint32_t>(static_cast<std::int32_t>(index));
    if (unsigned_index<frame.states.size() || !frame.has_texture)
        gui_framebox_rebuild_00ad0d80(frame,widget,overbright,index,services);
}
void gui_framebox_loaded78_00aceb50(GuiFrameBoxWidget& frame,GuiLayoutWidget& widget,
    float& overbright,const GuiFrameBoxRuntimeServices& services) {
    required(services.base_loaded78_00aa7170,"FrameBox base loaded78 service missing");
    services.base_loaded78_00aa7170();
    gui_framebox_select_state_00acf070(frame,widget,overbright,0,services);
}
}
