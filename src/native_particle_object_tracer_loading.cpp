#include "bsp/native_particle_object_tracer_loading.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_string.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*)==4,"Actual particle loading requires Win32");
namespace {
void* at(void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
const void* at(const void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
template<class T> T load(const void* p,std::uint32_t n=0) noexcept {
    T v;std::memcpy(&v,at(p,n),sizeof v);return v;
}
template<class T> void store(void* p,std::uint32_t n,T v) noexcept {
    std::memcpy(at(p,n),&v,sizeof v);
}
bool text_is(const void* text,const char* value) {
    return _stricmp(load<const char*>(text),value)==0;
}
class Pooled {
public:
    NativePooledTextStorage header{nullptr};
    explicit Pooled(NativeStringStorage& strings) noexcept : strings_(strings) {}
    Pooled(const void* line,std::int32_t index,NativeStringStorage& strings,
        bool suffix=false) : strings_(strings) {
        if(suffix)get_native_pooled_text_suffix_00af44c0(line,&header,index,strings_);
        else get_native_pooled_text_token_00aee3c0(line,&header,index,strings_);
    }
    ~Pooled(){destroy_native_pooled_text_00aee2a0(&header,strings_);}
    Pooled(const Pooled&)=delete;
    Pooled& operator=(const Pooled&)=delete;
private:
    NativeStringStorage& strings_;
};
class Builder {
public:
    NativeParticleParameterBuilderStorage header;
    Builder(NativeParticleParameterLoadingBindings& bindings,std::int32_t& kind)
        : b_(bindings),kind_(kind) {
        header.kind_0c=kind_;
        construct_native_particle_parameter_builder_00afbed0(&header,b_);
    }
    ~Builder(){
        destroy_native_particle_parameter_builder_00af4110(&header,b_);
        kind_=header.kind_0c;
    }
    Builder(const Builder&)=delete;
    Builder& operator=(const Builder&)=delete;
private:
    NativeParticleParameterLoadingBindings& b_;
    std::int32_t& kind_;
};
bool token_is(const void* line,const char* value,NativeStringStorage& strings) {
    Pooled token(line,0,strings);
    return text_is(&token.header,value);
}
float percentage(float value,const volatile double* scale) noexcept {
    float result;
    __asm {
        fld value
        mov eax,scale
        fmul qword ptr [eax]
        fstp result
    }
    return result;
}
void* make_curve(void* builder,float scalar,NativeParticleTypeLoadingBindings& b) {
    void* curve=convert_native_particle_parameter_00afbf60(builder,b.parameters);
    // The CURRENT double is read after conversion (which can allocate).
    store(curve,0,percentage(scalar,b.percentage_scale_00d7a358));
    return curve;
}
void publish_curve(void* definition,std::uint32_t field,void* builder,
    float scalar,NativeParticleTypeLoadingBindings& b) {
    store(definition,field,make_curve(builder,scalar,b));
}
void append_texture(void* p,void* item,NativeParticleTypePropertyBindings& b) {
    const auto capacity=load<std::uint32_t>(p,0xa0);
    if(load<std::uint32_t>(p,0x9c)==capacity) {
        const auto wanted=capacity*2u+2u;
        if(wanted>capacity) {
            store(p,0xa0,wanted);
            const auto bytes=wanted>0x3fffffffu?0xffffffffu:wanted*4u;
            void* fresh=b.base.allocate_array_00bf55be(bytes);
            if(load<void*>(p,0x98)) {
                for(std::uint32_t i=0;i<load<std::uint32_t>(p,0x9c);++i)
                    store(fresh,i*4u,load<std::uint32_t>(load<void*>(p,0x98),i*4u));
                b.base.owners.free_array_00bf6989(load<void*>(p,0x98));
            }
            store(p,0x98,fresh);
        }
    }
    store(load<void*>(p,0x98),load<std::uint32_t>(p,0x9c)*4u,item);
    store(p,0x9c,load<std::uint32_t>(p,0x9c)+1u);
}
// Both parsers deliberately construct/parse a parameter for unknown fields.
// Their temporary order is observable through the actual shared string pool.
template<class Apply> void parse_parameter(void* definition,const void* line,
    const void* name,NativeParticleTypeLoadingBindings& b,std::int32_t& kind,Apply apply) {
    auto& strings=b.parameters.owners.strings;
    float scalar;
    {
        Pooled rest(line,2,strings,true);
        Pooled token(&rest.header,0,strings);
        scalar=static_cast<float>(std::atof(token.header.data));
    }
    Builder builder(b.parameters,kind);
    initialize_native_particle_parameter_endpoints_00afc360(&builder.header,0.0f,0.0f,b.parameters);
    {
        Pooled rest(line,2,strings,true);
        Pooled curve(&rest.header,1,strings,true);
        (void)parse_native_particle_parameter_00afc470(&builder.header,&curve.header,b.parameters);
    }
    if(!load_native_particle_type_parameter_00b00980(definition,name,&builder.header,scalar,b))
        apply(&builder.header,scalar);
}
template<class Apply> bool parse_definition(void* p,void* text,
    NativeParticleTypeLoadingBindings& b,Apply apply) {
    auto& strings=b.parameters.owners.strings;
    std::int32_t kind=b.initial_builder_kind_0c;
    store<std::uint8_t>(p,0x64,0);
    Pooled line(strings);
    while(read_native_text_buffer_line_00af5740(text,&line.header,strings,b.text_scratch_00f8c2c8))
        if(text_is(&line.header,"{"))break;
    while(read_native_text_buffer_line_00af5740(text,&line.header,strings,b.text_scratch_00f8c2c8)) {
        if(text_is(&line.header,"}"))break;
        if(text_is(&line.header,""))continue;
        if(!token_is(&line.header,"Param",strings))continue;
        bool handled;
        {
            Pooled suffix(&line.header,1,strings,true);
            handled=load_native_particle_type_property_00b015c0(p,&suffix.header,b.properties);
        }
        if(handled)continue;
        Pooled name(&line.header,1,strings);
        apply(&line.header,&name.header,kind);
    }
    return true;
}
} // namespace

void set_native_object_particle_size_00af80f0(void* p,void* parameter) {
    store(p,0x84,parameter);
    if(!parameter){store<std::uint32_t>(p,0x88,0);return;}
    const auto kind=load<std::uint16_t>(parameter,0x0a);
    if(kind==0){store(p,0x88,load<std::uint32_t>(parameter,4));return;}
    const float value=kind==1
        ?evaluate_native_particle_linear_curve_00affa70(parameter,nullptr,0.0f)
        :evaluate_native_particle_cubic_curve_00affae0(parameter,nullptr,0.0f);
    store(p,0x88,value);
}

void load_native_tracer_particle_textures_00b0a920(void* p,const char* filename,
    NativeParticleTypePropertyBindings& b) {
    store<std::uint32_t>(p,0x9c,0);
    const auto count=count_native_particle_texture_frames_00af3a20(filename,b);
    if(count==0) {
        void* item=find_native_particle_atlas_item_00aefb20(b.actual_atlas_manager_00f8c26c,filename,b.base.owners.strings,b.null_pattern_00e17bf0);
        if(item)append_texture(p,item,b);
        return;
    }
    for(std::int32_t i=0;i<count;++i) {
        NativeString frame;
        construct_native_particle_texture_frame_name_00af3b50(&frame,filename,i,b);
        try {
            void* item=find_native_particle_atlas_item_00aefb20(b.actual_atlas_manager_00f8c26c,frame.data()?frame.data():b.empty_frame_name_00f8d390,b.base.owners.strings,b.null_pattern_00e17bf0);
            if(!item){destroy_native_string_header_0041dd20(&frame,b.base.owners.strings);return;}
            append_texture(p,item,b);
        } catch(...) {
            destroy_native_string_header_0041dd20(&frame,b.base.owners.strings);
            throw;
        }
        destroy_native_string_header_0041dd20(&frame,b.base.owners.strings);
    }
}

bool load_native_object_particle_definition_00af8bd0(void* p,void* text,
    NativeParticleTypeLoadingBindings& b) {
    return parse_definition(p,text,b,[&](const void* line,const void* name,std::int32_t& kind) {
        if(text_is(name,"Model")) {
            Pooled model(line,2,b.parameters.owners.strings);
            const auto target=load<std::uint32_t>(load<void*>(p),0x20);
            if(target==0x00af9660 && b.properties.resources){
                if(&b.properties.resources->base!=&b.properties.base)
                    throw std::logic_error("particle model loading requires the same native base domain");
                load_native_object_particle_models_00af9660(p,model.header.data,*b.properties.resources);
            }else b.properties.model_virtual20(b.properties.context,p,target,model.header.data);
            return;
        }
        parse_parameter(p,line,name,b,kind,[&](void* builder,float scalar) {
            if(text_is(name,"RotationSpeed"))publish_curve(p,0x80,builder,scalar,b);
            else if(text_is(name,"Size"))set_native_object_particle_size_00af80f0(p,make_curve(builder,scalar,b));
        });
    });
}

bool load_native_tracer_particle_definition_00b0ad50(void* p,void* text,
    NativeParticleTypeLoadingBindings& b) {
    return parse_definition(p,text,b,[&](const void* line,const void* name,std::int32_t& kind) {
        auto& strings=b.parameters.owners.strings;
        if(text_is(name,"ShowBullet")) {
            Pooled value(line,2,strings);
            store<std::uint8_t>(p,0xb0,std::atol(value.header.data)>0?1:0);
            return;
        }
        if(text_is(name,"BulletHeadTexture")) {
            Pooled value(line,2,strings);
            store(p,0xa4,find_native_particle_atlas_item_00aefb20(b.properties.actual_atlas_manager_00f8c26c,value.header.data,strings,b.properties.null_pattern_00e17bf0));
            return;
        }
        if(text_is(name,"BulletTailTexture")) {
            Pooled value(line,2,strings);
            store(p,0xa8,find_native_particle_atlas_item_00aefb20(b.properties.actual_atlas_manager_00f8c26c,value.header.data,strings,b.properties.null_pattern_00e17bf0));
            return;
        }
        if(text_is(name,"TracerTexture")) {
            Pooled value(line,2,strings);
            load_native_tracer_particle_textures_00b0a920(p,value.header.data,b.properties);
            return;
        }
        parse_parameter(p,line,name,b,kind,[&](void* builder,float scalar) {
            struct Field {const char* name;std::uint32_t offset;bool curve;};
            static const Field fields[]={
                {"MaxSegmentNum",0x80,false},{"MinSegmentLength",0x84,false},
                {"SegmentLifeTime",0x88,false},{"BulletRadius",0x8c,true},
                {"BulletLifeTime",0x90,false},{"BulletFadeBegin",0x94,true},
                {"BulletTailLength",0xac,false},{"OuterColor_R",0xb4,true},
                {"OuterColor_G",0xb8,true},{"OuterColor_B",0xbc,true},
                {"Glow_R",0xc0,true},{"Glow_G",0xc4,true},{"Glow_B",0xc8,true},
                {"Tail_R",0xcc,true},{"Tail_G",0xd0,true},{"Tail_B",0xd4,true},
                {"Width",0xd8,true},{"TileLength",0xe4,true},
                {"WidthSpeed",0xdc,true},{"WidthSpeedEnd",0xe0,true}};
            for(const auto& field:fields)if(text_is(name,field.name)) {
                if(!field.curve)store(p,field.offset,first_native_particle_parameter_value_00afc1b0(builder));
                else if(field.offset==0xe4)store(p,field.offset,convert_native_particle_parameter_00afbf60(builder,b.parameters));
                else publish_curve(p,field.offset,builder,scalar,b);
                return;
            }
        });
    });
}
} // namespace bsp
