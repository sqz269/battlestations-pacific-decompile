#include "bsp/native_particle_definition_loading.hpp"
#include "bsp/native_particle_type_factory.hpp"
#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4, "Native particle definition loading requires Win32");
namespace {
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
const void* at(const void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
template<class T> T load(const void* p, std::uint32_t n = 0) noexcept {
    T v; std::memcpy(&v, at(p,n), sizeof v); return v;
}
template<class T> void store(void* p, std::uint32_t n, T v) noexcept {
    std::memcpy(at(p,n), &v, sizeof v);
}

// The native input has AL semantics; _stricmp is intentionally unguarded.
bool text_is(const void* text, const char* value) {
    return _stricmp(load<const char*>(text),value)==0;
}
class Pooled {
public:
    NativePooledTextStorage header{nullptr};
    explicit Pooled(NativeStringStorage& storage) noexcept : storage_(storage) {}
    Pooled(const void* line, std::int32_t index, NativeStringStorage& storage)
        : storage_(storage) {
        get_native_pooled_text_token_00aee3c0(line,&header,index,storage_);
    }
    Pooled(const void* line, std::int32_t index,
        NativeParticleDefinitionLoadingBindings& b) : storage_(b.owners.strings) {
        get_native_pooled_text_suffix_00af44c0(line,&header,index,storage_);
    }
    ~Pooled() { destroy_native_pooled_text_00aee2a0(&header,storage_); }
    Pooled(const Pooled&) = delete;
    Pooled& operator=(const Pooled&) = delete;
private:
    NativeStringStorage& storage_;
};

bool token_is(const void* line, const char* value, NativeStringStorage& storage) {
    Pooled token(line,0,storage);
    return text_is(&token.header,value); // temp release precedes caller branch
}

class String {
public:
    NativeString header;
    String(const char* text, NativeStringStorage& storage, bool capture = false)
        : storage_(storage), captured_(capture) {
        resize_native_string_header_0041dd40(&header,storage_,
            static_cast<std::uint32_t>(std::strlen(text)),true);
        if (header.data()) std::memmove(header.data(),text,header.length()+1u);
        captured_pointer_=header.data();
    }
    ~String() {
        if (captured_) {
            if (captured_pointer_) storage_.release(captured_pointer_,header.length()+1u);
        } else destroy_native_string_header_0041dd20(&header,storage_);
    }
    String(const String&) = delete;
    String& operator=(const String&) = delete;
private:
    NativeStringStorage& storage_;
    bool captured_;
    char* captured_pointer_{};
};

void set_emission_type(void* definition, const char* value, std::uint32_t field,
    NativeParticleDefinitionLoadingBindings& b) {
    bool matched;
    {
        // AFA370/AFA4E0 first literal uses an inlined constructor and captures
        // its data before constructing the input; its length is reloaded later.
        String expected("PerSec",b.owners.strings,true);
        String actual(value,b.owners.strings);
        matched=equal_native_string_headers_00435c40(&actual.header,&expected.header);
    }
    if (matched) { store<std::uint32_t>(definition,field,0); return; }
    {
        String expected("PerMeter",b.owners.strings);
        String actual(value,b.owners.strings);
        matched=equal_native_string_headers_00435c40(&actual.header,&expected.header);
    }
    if (matched) store<std::uint32_t>(definition,field,1);
}

// Preserve the original FLD float / FMUL double / FSTP float sequence. The
// double is a live application operand (its actual bits are 3f847ae140000000).
float percentage(float scalar, const volatile double* scale) noexcept {
    float result;
    __asm {
        fld scalar
        mov eax,scale
        fmul qword ptr [eax]
        fstp result
    }
    return result;
}
void publish_curve(void* definition, std::uint32_t field, void* builder,
    float scalar, NativeParticleDefinitionLoadingBindings& b) {
    void* curve=convert_native_particle_parameter_00afbf60(builder,b.parameters);
    store(curve,0,percentage(scalar,b.percentage_scale_00d7a358));
    store(definition,field,curve); // no replacement release or null fallback
}
float add_native(float first,float second) noexcept {
    float result;
    __asm {
        fld first
        fadd second
        fstp result
    }
    return result;
}
} // namespace

bool load_native_particle_base_parameter_00af9d00(void* p,const void* name,
    void* builder,float scalar,NativeParticleDefinitionLoadingBindings& b) {
    if (text_is(name,"BornRatio")) {
        store(p,0x18,first_native_particle_parameter_value_00afc1b0(builder));
        return true;
    }
    if (text_is(name,"Lifetime")) { publish_curve(p,0x20,builder,scalar,b); return true; }
    if (text_is(name,"ParticleEmission")) { publish_curve(p,0x24,builder,scalar,b); return true; }
    if (text_is(name,"EmitterEmission")) { publish_curve(p,0x28,builder,scalar,b); return true; }
    if (text_is(name,"Speed")) { publish_curve(p,0x2c,builder,scalar,b); return true; }
    if (equal_native_pooled_text_00aedf80(name,"VerticalSpeed")) { publish_curve(p,0x30,builder,scalar,b); return true; }
    if (equal_native_pooled_text_00aedf80(name,"InheritedSpeed")) { publish_curve(p,0x34,builder,scalar,b); return true; }
    if (equal_native_pooled_text_00aedf80(name,"WindSensitivity")) { publish_curve(p,0x38,builder,scalar,b); return true; }
    return false;
}

void publish_native_particle_emitter_member_00af9f00(void* parent,void* child) noexcept {
    if (!child) return;
    store(parent,0x50,add_native(load<float>(child,0x18),load<float>(parent,0x50)));
    const auto count=load<std::uint32_t>(parent,0x4c);
    store(parent,0x3c+count*4u,child);
    store(parent,0x4c,load<std::uint32_t>(parent,0x4c)+1u);
}
void* publish_native_particle_particle_member_00af9f20(void* parent,void* child) noexcept {
    store(parent,0x6c,add_native(load<float>(child,0x24),load<float>(parent,0x6c)));
    const auto count=load<std::uint32_t>(parent,0x68);
    store(parent,0x54+count*4u,child);
    store(parent,0x68,load<std::uint32_t>(parent,0x68)+1u);
    return load<void*>(parent,0x50+load<std::uint32_t>(parent,0x68)*4u);
}
void set_native_particle_part_emission_type_00afa370(void* p,const char* value,
    NativeParticleDefinitionLoadingBindings& b) { set_emission_type(p,value,0x74,b); }
void set_native_particle_emit_emission_type_00afa4e0(void* p,const char* value,
    NativeParticleDefinitionLoadingBindings& b) { set_emission_type(p,value,0x78,b); }

bool load_native_particle_definition_flag_00afa650(void* p,const void* line,
    NativeParticleDefinitionLoadingBindings& b) {
    auto& strings=b.owners.strings;
    if (token_is(line,"PartEmissionType",strings)) {
        Pooled copied(strings);
        {
            Pooled token(line,1,strings);
            construct_native_pooled_text_00af5660(&copied.header,token.header.data,strings);
        }
        set_native_particle_part_emission_type_00afa370(p,copied.header.data,b);
        return true;
    }
    if (token_is(line,"EmitEmissionType",strings)) {
        Pooled copied(strings);
        {
            Pooled token(line,1,strings);
            construct_native_pooled_text_00af5660(&copied.header,token.header.data,strings);
        }
        set_native_particle_emit_emission_type_00afa4e0(p,copied.header.data,b);
        return true;
    }
    if (token_is(line,"Looping",strings)) {
        Pooled token(line,1,strings);
        const bool enabled=std::atol(token.header.data)>0;
        store<std::uint8_t>(p,0x15,static_cast<std::uint8_t>(enabled));
        if (enabled) store<std::uint8_t>(load<void*>(p,0x10),0x66,1);
        return true;
    }
    if (token_is(line,"FollowDirection",strings)) {
        Pooled token(line,1,strings);
        store<std::uint8_t>(p,0x1d,static_cast<std::uint8_t>(std::atol(token.header.data)>0));
        return true;
    }
    if (token_is(line,"Stops",strings)) {
        Pooled token(line,1,strings);
        store<std::uint8_t>(p,0x1c,static_cast<std::uint8_t>(std::atol(token.header.data)>0));
        return true;
    }
    return false;
}

namespace {
class Builder {
public:
    NativeParticleParameterBuilderStorage header; // native kind+C starts indeterminate
    explicit Builder(NativeParticleParameterLoadingBindings& b) : bindings_(b) {
        construct_native_particle_parameter_builder_00afbed0(&header,bindings_);
    }
    ~Builder() { destroy_native_particle_parameter_builder_00af4110(&header,bindings_); }
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;
private:
    NativeParticleParameterLoadingBindings& bindings_;
};
struct Property { const char* name; std::uint32_t field; };
constexpr Property cone_properties[]={{"InnerEmitSpeed",0x80},{"OuterEmitSpeed",0x84},
    {"MaxAngle",0x88},{"InnerDistance",0x8c},{"OuterDistance",0x90}};
constexpr Property sphere_properties[]={{"EmittedSpeed",0x80},{"InnerRadius",0x84},
    {"OuterRadius",0x88}};
constexpr Property smartarea_properties[]={{"EmittedSpeed",0x80},{"Radius",0x84},
    {"RadiusSpeed",0x88},{"RadiusAngle",0x8c}};

bool load_parameter_line(void* definition,const void* line,const Property* properties,
    std::size_t count,NativeParticleDefinitionLoadingBindings& b) {
    {
        Pooled suffix(line,1,b);
        if (load_native_particle_definition_flag_00afa650(definition,&suffix.header,b))
            return true;
    }
    Pooled name(line,1,b.owners.strings);
    float scalar;
    {
        Pooled suffix(line,2,b);
        Pooled value(&suffix.header,0,b.owners.strings);
        scalar=static_cast<float>(std::atof(value.header.data)); // native FSTP first
    }
    Builder builder(b.parameters);
    initialize_native_particle_parameter_endpoints_00afc360(&builder.header,0.0f,0.0f,b.parameters);
    {
        Pooled suffix(line,2,b);
        Pooled curve_text(&suffix.header,1,b);
        // Native ignores AL, retaining partial parser mutations on failure.
        (void)parse_native_particle_parameter_00afc470(&builder.header,&curve_text.header,b.parameters);
    }
    if (load_native_particle_base_parameter_00af9d00(definition,&name.header,
        &builder.header,scalar,b)) return true;
    for (std::size_t i=0;i<count;++i) {
        if (text_is(&name.header,properties[i].name)) {
            publish_curve(definition,properties[i].field,&builder.header,scalar,b);
            return true;
        }
    }
    return false;
}

void load_child_lines(void* definition,const void* line,void* text,
    NativeParticleDefinitionLoadingBindings& b) {
    if (token_is(line,"Emitter",b.owners.strings)) {
        void* child;
        {
            Pooled name_token(line,1,b.owners.strings);
            String name(name_token.header.data,b.owners.strings);
            Pooled kind_token(line,2,b.owners.strings);
            String kind(kind_token.header.data,b.owners.strings);
            child=create_native_particle_definition_00af9fb0(&kind.header,&name.header,
                load<std::uint32_t>(definition,0x10),reinterpret_cast<std::uintptr_t>(definition),
                text,b.owners);
        } // both NativeString/token pairs die BEFORE member publication
        publish_native_particle_emitter_member_00af9f00(definition,child);
    }
    // Independent re-extraction after emitter parsing/publication is native.
    if (token_is(line,"Particle",b.owners.strings)) {
        void* child;
        {
            Pooled name_token(line,1,b.owners.strings);
            String name(name_token.header.data,b.owners.strings);
            Pooled kind_token(line,2,b.owners.strings);
            String kind(kind_token.header.data,b.owners.strings);
            if (!b.particle_types || &b.particle_types->base.owners != &b.owners)
                throw std::logic_error("particle definition requires its actual particle type domain");
            child=create_native_particle_type_definition_00b00ce0(
                &kind.header,&name.header,definition,text,*b.particle_types);
        }
        (void)publish_native_particle_particle_member_00af9f20(definition,child);
    }
}

bool load_definition(void* definition,void* text,const Property* properties,
    std::size_t count,bool cone_unknown_fallthrough,NativeParticleDefinitionLoadingBindings& b) {
    Pooled line(b.owners.strings);
    while (read_native_text_buffer_line_00af5740(text,&line.header,b.owners.strings,
        b.text_scratch_00f8c2c8)) {
        if (text_is(&line.header,"{")) break;
    }
    // Even a failed opening-brace scan performs this additional read.
    bool present=read_native_text_buffer_line_00af5740(text,&line.header,b.owners.strings,
        b.text_scratch_00f8c2c8);
    while (present && !text_is(&line.header,"}")) {
        if (!text_is(&line.header,"")) {
            bool check_children=true;
            if (token_is(&line.header,"Param",b.owners.strings)) {
                const bool handled=load_parameter_line(definition,&line.header,properties,count,b);
                check_children=cone_unknown_fallthrough && !handled;
            }
            if (check_children) load_child_lines(definition,&line.header,text,b);
        }
        present=read_native_text_buffer_line_00af5740(text,&line.header,b.owners.strings,
            b.text_scratch_00f8c2c8);
    }
    return true;
}
} // namespace

bool load_native_particle_cone_definition_00b03ec0(void* p,void* text,
    NativeParticleDefinitionLoadingBindings& b) {
    return load_definition(p,text,cone_properties,5,true,b);
}
bool load_native_particle_sphere_definition_00b02fd0(void* p,void* text,
    NativeParticleDefinitionLoadingBindings& b) {
    return load_definition(p,text,sphere_properties,3,false,b);
}
bool load_native_particle_smartarea_definition_00b02210(void* p,void* text,
    NativeParticleDefinitionLoadingBindings& b) {
    return load_definition(p,text,smartarea_properties,4,false,b);
}
} // namespace bsp
