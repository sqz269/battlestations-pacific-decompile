#include "bsp/native_particle_definition_loading.hpp"
#include "bsp/native_particle_type_factory.hpp"
#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
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

namespace {
struct RawEmitterString { std::uint32_t length; char* data; };
static_assert(sizeof(RawEmitterString)==8);
char* raw_text_pointer(const void* header) noexcept {
    return *static_cast<char* const volatile*>(header);
}
char* raw_string_pointer(const RawEmitterString& text) noexcept {
    return *reinterpret_cast<char* const volatile*>(&text.data);
}
std::uint32_t raw_string_length(const RawEmitterString& text) noexcept {
    return *static_cast<const volatile std::uint32_t*>(&text.length);
}
void raw_return_string(char* captured, std::uint32_t size,
    NativeStringRawPoolContext& strings) {
    auto* pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,captured,size,
        strings.actual_small_returns_disabled_01090aa4);
}
struct RawEnumUnwind {
    NativeStringRawPoolContext& strings;
    void* header=nullptr;
    ~RawEnumUnwind() noexcept {
        // DF2F10/DF2F44: state0 ->-1 first literal; state1 ->-1 second
        // literal. Neither constructed input string has unwind ownership.
        if(header) destroy_native_string_header_0041dd20(header,strings);
    }
};
void set_raw_emission_type(void* definition,const char* value,std::uint32_t field,
    NativeStringRawPoolContext& strings) {
    RawEmitterString first_expected{0,nullptr}, first_actual;
    RawEnumUnwind unwind{strings};
    resize_native_string_header_0041dd40(&first_expected,strings,6,true);
    char* const first_bytes=raw_string_pointer(first_expected);
    if(first_bytes) std::memmove(first_bytes,"PerSec",raw_string_length(first_expected)+1u);
    const char* const captured_input=value;
    unwind.header=&first_expected;
    void* actual=construct_native_string_header_0041e870(&first_actual,strings,captured_input);
    const bool per_second=equal_native_string_headers_00435c40(actual,&first_expected);
    destroy_native_string_header_0041dd20(&first_actual,strings);
    unwind.header=nullptr; // BEFORE the captured first buffer's current length.
    if(first_bytes) raw_return_string(first_bytes,raw_string_length(first_expected)+1u,strings);
    if(per_second) { store<std::uint32_t>(definition,field,0); return; }

    RawEmitterString second_expected, second_actual;
    void* expected=construct_native_string_header_0041e870(&second_expected,strings,"PerMeter");
    unwind.header=&second_expected;
    actual=construct_native_string_header_0041e870(&second_actual,strings,captured_input);
    const bool per_meter=equal_native_string_headers_00435c40(actual,expected);
    destroy_native_string_header_0041dd20(&second_actual,strings);
    char* const second_bytes=raw_string_pointer(second_expected);
    unwind.header=nullptr;
    if(second_bytes) raw_return_string(second_bytes,raw_string_length(second_expected)+1u,strings);
    if(per_meter) store<std::uint32_t>(definition,field,1);
}
struct RawFlagUnwind {
    NativePooledTextStorage& token;
    NativePooledTextStorage& copied;
    NativeStringRawPoolContext& strings;
    int state=-1;
    ~RawFlagUnwind() noexcept {
        // DF2F9C: 0->-1 token,1->0 copy,2->-1 copy;
        // 3->-1 token,4->3 copy,5->-1 copy. States2/5 discard token
        // ownership BEFORE its normal return. Keyword/numeric tokens unowned.
        if(state==1 || state==2 || state==4 || state==5) {
            destroy_native_pooled_text_00aee2a0(&copied,strings);
            if(state==1) state=0;
            else if(state==4) state=3;
            else state=-1;
        }
        if(state==0 || state==3) destroy_native_pooled_text_00aee2a0(&token,strings);
    }
};
bool raw_flag_keyword(const void* line,NativePooledTextStorage& token,
    const char* keyword,bool clear,NativeStringRawPoolContext& strings) {
    const void* result=get_native_pooled_text_token_00aee3c0(line,&token,0,strings);
    const bool match=_stricmp(raw_text_pointer(result),keyword)==0;
    if(clear) destroy_native_pooled_text_00aee2a0(&token,strings);
    else if(char* captured=raw_text_pointer(&token))
        release_native_pooled_text_bytes_00aee1e0(captured,strings);
    return match;
}
bool raw_enum_flag(void* definition,const void* line,bool emit,
    RawFlagUnwind& unwind) {
    auto& strings=unwind.strings;
    const void* token=get_native_pooled_text_token_00aee3c0(line,&unwind.token,1,strings);
    const char* const input=raw_text_pointer(token);
    unwind.state=emit?3:0;
    construct_native_pooled_text_00af5660(&unwind.copied,input,strings);
    char* const token_bytes=raw_text_pointer(&unwind.token);
    unwind.state=emit?5:2;
    if(token_bytes) release_native_pooled_text_bytes_00aee1e0(token_bytes,strings);
    char* const copied_bytes=raw_text_pointer(&unwind.copied);
    unwind.token.data=nullptr;
    if(emit) set_native_particle_emit_emission_type_00afa4e0(definition,copied_bytes,strings);
    else set_native_particle_part_emission_type_00afa370(definition,copied_bytes,strings);
    unwind.state=-1;
    if(copied_bytes) release_native_pooled_text_bytes_00aee1e0(copied_bytes,strings);
    return true;
}
void raw_store_first_value(void* destination,const void* builder) {
    using First=float (__cdecl*)(const void*);
    First const first=&first_native_particle_parameter_value_00afc1b0;
    __asm {
        push builder
        call first
        add esp,4
        mov ecx,destination
        fstp dword ptr[ecx]
    }
}
void publish_raw_curve(void* definition,std::uint32_t field,void* builder,
    const float* scalar,NativeParticleParameterRuntimeRawContext& runtime,
    const volatile double* scale) {
    void* const curve=convert_native_particle_parameter_00afbf60(builder,runtime);
    // Retain the pointer, but load its current volatile double only here.
    // Original stack32 scalar -> FLD32, FMUL64, direct FSTP curve+0.
    __asm {
        mov eax,scalar
        fld dword ptr[eax]
        mov ecx,scale
        fmul qword ptr[ecx]
        mov eax,curve
        fstp dword ptr[eax]
    }
    store(definition,field,curve);
}
} // namespace

void set_native_particle_part_emission_type_00afa370(void* p,const char* value,
    NativeStringRawPoolContext& strings) { set_raw_emission_type(p,value,0x74,strings); }
void set_native_particle_emit_emission_type_00afa4e0(void* p,const char* value,
    NativeStringRawPoolContext& strings) { set_raw_emission_type(p,value,0x78,strings); }

bool load_native_particle_definition_flag_00afa650(void* p,const void* line,
    NativeStringRawPoolContext& strings) {
    NativePooledTextStorage token,copied;
    RawFlagUnwind unwind{token,copied,strings};
    if(raw_flag_keyword(line,token,"PartEmissionType",false,strings))
        return raw_enum_flag(p,line,false,unwind);
    if(raw_flag_keyword(line,token,"EmitEmissionType",false,strings))
        return raw_enum_flag(p,line,true,unwind);
    if(raw_flag_keyword(line,token,"Looping",false,strings)) {
        const void* value=get_native_pooled_text_token_00aee3c0(line,&token,1,strings);
        const bool enabled=std::atol(raw_text_pointer(value))>0;
        store<std::uint8_t>(p,0x15,static_cast<std::uint8_t>(enabled));
        if(enabled) store<std::uint8_t>(load<void*>(p,0x10),0x66,1);
        destroy_native_pooled_text_00aee2a0(&token,strings);
        return true;
    }
    if(raw_flag_keyword(line,token,"FollowDirection",true,strings)) {
        const void* value=get_native_pooled_text_token_00aee3c0(line,&token,1,strings);
        store<std::uint8_t>(p,0x1d,static_cast<std::uint8_t>(std::atol(raw_text_pointer(value))>0));
        destroy_native_pooled_text_00aee2a0(&token,strings);
        return true;
    }
    if(raw_flag_keyword(line,token,"Stops",true,strings)) {
        const void* value=get_native_pooled_text_token_00aee3c0(line,&copied,1,strings);
        store<std::uint8_t>(p,0x1c,static_cast<std::uint8_t>(std::atol(raw_text_pointer(value))>0));
        destroy_native_pooled_text_00aee2a0(&copied,strings);
        return true;
    }
    return false;
}

bool load_native_particle_base_parameter_00af9d00(void* p,const void* name,
    void* builder,float scalar,NativeParticleParameterRuntimeRawContext& runtime,
    const volatile double* scale) {
    if(_stricmp(raw_text_pointer(name),"BornRatio")==0) {
        raw_store_first_value(at(p,0x18),builder);
        return true;
    }
    if(_stricmp(raw_text_pointer(name),"Lifetime")==0) { publish_raw_curve(p,0x20,builder,&scalar,runtime,scale); return true; }
    if(_stricmp(raw_text_pointer(name),"ParticleEmission")==0) { publish_raw_curve(p,0x24,builder,&scalar,runtime,scale); return true; }
    if(_stricmp(raw_text_pointer(name),"EmitterEmission")==0) { publish_raw_curve(p,0x28,builder,&scalar,runtime,scale); return true; }
    if(_stricmp(raw_text_pointer(name),"Speed")==0) { publish_raw_curve(p,0x2c,builder,&scalar,runtime,scale); return true; }
    if(equal_native_pooled_text_00aedf80(name,"VerticalSpeed")) { publish_raw_curve(p,0x30,builder,&scalar,runtime,scale); return true; }
    if(equal_native_pooled_text_00aedf80(name,"InheritedSpeed")) { publish_raw_curve(p,0x34,builder,&scalar,runtime,scale); return true; }
    if(equal_native_pooled_text_00aedf80(name,"WindSensitivity")) { publish_raw_curve(p,0x38,builder,&scalar,runtime,scale); return true; }
    return false;
}
} // namespace bsp
