#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace bsp {
static_assert(sizeof(void*) == 4, "Native particle loading requires Win32");
namespace {
template<class T> void store(void* p, std::uint32_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(p)+offset,&value,sizeof value);
}
template<class T> T load(const void* p, std::uint32_t offset=0) noexcept {
    T value; std::memcpy(&value,static_cast<const char*>(p)+offset,sizeof value); return value;
}
class Pooled {
public:
    NativePooledTextStorage header{nullptr};
    explicit Pooled(NativeStringStorage& strings) : strings_(strings) {}
    Pooled(const void* source,std::int32_t index,NativeStringStorage& strings,bool suffix=false)
        : strings_(strings) {
        if(suffix) get_native_pooled_text_suffix_00af44c0(source,&header,index,strings);
        else get_native_pooled_text_token_00aee3c0(source,&header,index,strings);
    }
    ~Pooled() { destroy_native_pooled_text_00aee2a0(&header,strings_); }
    Pooled(const Pooled&)=delete;
    Pooled& operator=(const Pooled&)=delete;
private:
    NativeStringStorage& strings_;
};
class Builder {
public:
    NativeParticleParameterBuilderStorage& header;
    Builder(NativeParticleParameterBuilderStorage& storage,NativeParticleParameterLoadingBindings& b)
        : header(storage),bindings_(b) { construct_native_particle_parameter_builder_00afbed0(&header,b); }
    ~Builder() { destroy_native_particle_parameter_builder_00af4110(&header,bindings_); }
    Builder(const Builder&)=delete;
    Builder& operator=(const Builder&)=delete;
private:
    NativeParticleParameterLoadingBindings& bindings_;
};
bool is(const Pooled& p,const char* text) { return _stricmp(p.header.data,text)==0; }
template<class Context> void* convert(void* builder,float percentage,Context& b) {
    void* parameter=convert_native_particle_parameter_00afbf60(builder,b.parameters);
    const volatile double* scale=b.percentage_scale_00d7a358;
    __asm {
        fld percentage
        mov eax,scale
        fmul qword ptr [eax]
        mov eax,parameter
        fstp dword ptr [eax]
    }
    return parameter;
}
template<class Context> void publish(void* definition,std::uint32_t offset,void* builder,float percentage,
    Context& b) { store(definition,offset,convert(builder,percentage,b)); }

bool load_sprite_or_floating(void* definition,void* text,NativeParticleTypeLoadingBindings& b,bool sprite) {
    auto& strings=b.parameters.owners.strings;
    // AFBED0 clears only 0/4/8. Keep the same +C residue between lines.
    NativeParticleParameterBuilderStorage storage;
    storage.kind_0c=b.initial_builder_kind_0c;
    if(sprite) store<std::uint8_t>(definition,0x64,0);
    Pooled line(strings);
    while(read_native_text_buffer_line_00af5740(text,&line.header,strings,b.text_scratch_00f8c2c8)) {
        if(is(line,"{")) break;
    }
    while(read_native_text_buffer_line_00af5740(text,&line.header,strings,b.text_scratch_00f8c2c8)) {
        if(is(line,"}")) break;
        if(is(line,"")) continue;
        bool parameter;
        { Pooled token(&line.header,0,strings); parameter=is(token,"Param"); }
        if(!parameter) continue;
        bool handled;
        {
            Pooled suffix(&line.header,1,strings,true);
            handled=load_native_particle_type_property_00b015c0(definition,&suffix.header,b.properties);
        }
        if(handled) continue;
        Pooled name(&line.header,1,strings);
        float percentage;
        {
            Pooled suffix(&line.header,2,strings,true);
            Pooled value(&suffix.header,0,strings);
            percentage=static_cast<float>(std::atof(value.header.data));
        }
        Builder builder(storage,b.parameters);
        initialize_native_particle_parameter_endpoints_00afc360(&builder.header,0.0f,0.0f,b.parameters);
        {
            Pooled suffix(&line.header,2,strings,true);
            Pooled curve(&suffix.header,1,strings,true);
            (void)parse_native_particle_parameter_00afc470(&builder.header,&curve.header,b.parameters);
        }
        if(load_native_particle_type_parameter_00b00980(definition,&name.header,&builder.header,percentage,b)) continue;
        if(is(name,"InitialRotation")) publish(definition,0x80,&builder.header,percentage,b);
        else if(is(name,"RotationSpeed")) publish(definition,0x84,&builder.header,percentage,b);
        else if(is(name,"Size")) {
            void* value=convert(&builder.header,percentage,b);
            if(sprite) set_native_sprite_particle_size_00b08870(definition,value,b);
            else store(definition,0x88,value);
        }
    }
    return true;
}
} // namespace

std::int32_t first_native_particle_parameter_integer_00afc1c0(const void* builder) {
    std::int32_t value;
    __asm {
        mov ecx,builder
        mov eax,dword ptr [ecx]
        cvttss2si eax,dword ptr [eax+4]
        mov value,eax
    }
    return value;
}

namespace {
void publish_first_value(void* definition,std::uint32_t destination_offset,const void* builder) {
    // AFC1B0's FLD32 feeds B00980's single destination FSTP32, including SNaN.
    __asm {
        mov eax,builder
        mov eax,dword ptr [eax]
        mov edx,definition
        add edx,destination_offset
        fld dword ptr [eax+4]
        fstp dword ptr [edx]
    }
}
template<class Context> bool load_parameter_property(void* definition,const void* property,
    void* builder,float percentage,Context& b) {
    if(_stricmp(load<const char*>(property),"BornRatio")==0) {
        publish_first_value(definition,0x24,builder); return true;
    }
    if(_stricmp(load<const char*>(property),"Lifetime")==0) { publish(definition,0x1c,builder,percentage,b); return true; }
    if(_stricmp(load<const char*>(property),"TerminateAfter")==0) {
        publish_first_value(definition,0x20,builder); return true;
    }
    if(_stricmp(load<const char*>(property),"Speed")==0) { publish(definition,0x2c,builder,percentage,b); return true; }
    if(_stricmp(load<const char*>(property),"VerticalSpeed")==0) { publish(definition,0x30,builder,percentage,b); return true; }
    struct Property { const char* name; std::uint32_t offset; };
    static constexpr Property curves[]={{"InheritedSpeed",0x48},{"WindSensitivity",0x34},
        {"Alpha",0x38},{"Color_R",0x3c},{"Color_G",0x40},{"Color_B",0x44},{"AnimPlaySpeed",0x5c}};
    for(const auto& curve:curves) if(equal_native_pooled_text_00aedf80(property,curve.name)) {
        publish(definition,curve.offset,builder,percentage,b); return true;
    }
    if(equal_native_pooled_text_00aedf80(property,"AnimStartFrame")) {
        store(definition,0x50,first_native_particle_parameter_integer_00afc1c0(builder)); return true;
    }
    if(equal_native_pooled_text_00aedf80(property,"AnimEndFrame")) {
        store(definition,0x54,first_native_particle_parameter_integer_00afc1c0(builder)); return true;
    }
    return false;
}

} // namespace
bool load_native_particle_type_parameter_00b00980(void* definition,const void* property,
    void* builder,float percentage,NativeParticleTypeLoadingBindings& b) {
    return load_parameter_property(definition,property,builder,percentage,b);
}
bool load_native_particle_type_parameter_00b00980(void* definition,const void* property,
    void* builder,float percentage,NativeParticleTypeParameterRawContext& b) {
    return load_parameter_property(definition,property,builder,percentage,b);
}

void set_native_sprite_particle_size_00b08870(void* definition,void* parameter,
    NativeParticleTypeLoadingBindings& b) {
    store(definition,0x88,parameter);
    const float maximum=parameter?bound_native_runtime_particle_value_00b001a0(parameter,b):0.0f;
    store(definition,0x8c,maximum);
}
bool load_native_sprite_particle_definition_00b08ac0(void* definition,void* text,
    NativeParticleTypeLoadingBindings& b) { return load_sprite_or_floating(definition,text,b,true); }
bool load_native_floating_particle_definition_00b07d60(void* definition,void* text,
    NativeParticleTypeLoadingBindings& b) { return load_sprite_or_floating(definition,text,b,false); }

namespace {
// Private bridge borrows numeric pointer-member addresses and the CRT domain in EBX.
// The three original kernels never use EBX; original stack and x87 schedules
// below are retained. The public API is deliberately not a binary replacement.
struct BoundAccess {
    const CameraAxesCrtAccess* crt;
    const volatile double* const* limit;
    const volatile double* const* base;
    const volatile double* const* derivative;
    const volatile double* const* discriminant;
};
using RawContext=NativeParticleTypeParameterRawContext;
static_assert(sizeof(BoundAccess)==20 && sizeof(RawContext)==28);
static_assert(offsetof(RawContext,last_time_00d7a220)==12 &&
    offsetof(RawContext,bound_base_00d7a210)==16 &&
    offsetof(RawContext,derivative_scale_00d7a2b0)==20 &&
    offsetof(RawContext,discriminant_scale_00d7a328)==24);
__declspec(naked) float quadratic_roots_kernel() {
    __asm {
        push ecx // 00affe20
        fld dword ptr [esp + 0xc] // 00affe21
        push esi // 00affe25
        fmul st(0), st(0) // 00affe26
        push edi // 00affe28
        fld dword ptr [esp + 0x10] // 00affe29
        mov esi, edx // 00affe2d
        push edx
        mov edx,dword ptr [ebx+16]
        mov edx,dword ptr [edx]
        fmul qword ptr [edx] // 00affe2f
        pop edx
        mov edi, ecx // 00affe35
        fmul dword ptr [esp + 0x18] // 00affe37
        fsubp st(1), st(0) // 00affe3b
        fstp dword ptr [esp + 0x18] // 00affe3d
        fldz  // 00affe41
        fld dword ptr [esp + 0x18] // 00affe43
        fcomi st(0), st(1) // 00affe47
        fstp st(1) // 00affe49
        jb bound_00affe84 // 00affe4b
        mov ecx,dword ptr [ebx] // actual CRT access; roots are already in EDI/ESI
        call native_crt_sqrt_st0_00bf7030 // 00affe4d
        fstp dword ptr [esp + 8] // 00affe52
        fld dword ptr [esp + 8] // 00affe56
        fstp dword ptr [esp + 8] // 00affe5a
        fld dword ptr [esp + 0x10] // 00affe5e
        fadd st(0), st(0) // 00affe62
        fld dword ptr [esp + 8] // 00affe64
        fld st(0) // 00affe68
        fld dword ptr [esp + 0x14] // 00affe6a
        fld st(0) // 00affe6e
        fsubp st(2), st(0) // 00affe70
        fxch st(1) // 00affe72
        fdiv st(0), st(3) // 00affe74
        fstp dword ptr [edi] // 00affe76
        fchs  // 00affe78
        fsubrp st(1), st(0) // 00affe7a
        fdivrp st(1), st(0) // 00affe7c
        fstp dword ptr [esi] // 00affe7e
        fld dword ptr [esp + 0x18] // 00affe80
    bound_00affe84:
        pop edi // 00affe84
        pop esi // 00affe85
        pop ecx // 00affe86
        ret 0xc // 00affe87
    }
}
__declspec(naked) float cubic_bound_kernel() {
    __asm {
        sub esp, 0x18 // 00affe90
        push esi // 00affe93
        mov esi, ecx // 00affe94
        fld dword ptr [esi + 4] // 00affe96
        xorps xmm0, xmm0 // 00affe99
        fsub dword ptr [esi] // 00affe9c
        sub esp, 0xc // 00affe9e
        lea edx, [esp + 0x18] // 00affea1
        lea ecx, [esp + 0x14] // 00affea5
        fstp dword ptr [esp + 0x10] // 00affea9
        movss dword ptr [esp + 0x14], xmm0 // 00affead
        fld dword ptr [esp + 0x10] // 00affeb3
        fstp dword ptr [esp + 0x18] // 00affeb7
        fld dword ptr [esi + 0x14] // 00affebb
        fstp dword ptr [esp + 8] // 00affebe
        fld dword ptr [esi + 0x10] // 00affec2
        fadd st(0), st(0) // 00affec5
        fstp dword ptr [esp + 0x1c] // 00affec7
        fld dword ptr [esp + 0x1c] // 00affecb
        fstp dword ptr [esp + 4] // 00affecf
        fld dword ptr [esi + 0xc] // 00affed3
        push edx
        mov edx,dword ptr [ebx+12]
        mov edx,dword ptr [edx]
        fmul qword ptr [edx] // 00affed6
        pop edx
        fstp dword ptr [esp + 0x1c] // 00affedc
        fld dword ptr [esp + 0x1c] // 00affee0
        fstp dword ptr [esp] // 00affee4
        call quadratic_roots_kernel // 00affee7
        fldz  // 00affeec
        fxch st(1) // 00affeee
        fcomip st(0), st(1) // 00affef0
        fstp st(0) // 00affef2
        fld dword ptr [esp + 4] // 00affef4
        jbe bound_00afff45 // 00affef8
        movss xmm0, dword ptr [esp + 8] // 00affefa
        xorps xmm1, xmm1 // 00afff00
        comiss xmm1, xmm0 // 00afff03
        movss xmm2, dword ptr [esp + 4] // 00afff06
        jbe bound_00afff13 // 00afff0c
        movaps xmm0, xmm1 // 00afff0e
        jmp bound_00afff1e // 00afff11
    bound_00afff13:
        fld dword ptr [esp + 8] // 00afff13
        fcomip st(0), st(1) // 00afff17
        jbe bound_00afff1e // 00afff19
        movaps xmm0, xmm2 // 00afff1b
    bound_00afff1e:
        movss dword ptr [esp + 8], xmm0 // 00afff1e
        movss xmm0, dword ptr [esp + 0xc] // 00afff24
        comiss xmm1, xmm0 // 00afff2a
        ja bound_00afff3f // 00afff2d
        fld dword ptr [esp + 0xc] // 00afff2f
        fcomip st(0), st(1) // 00afff33
        jbe bound_00afff3c // 00afff35
        movaps xmm1, xmm2 // 00afff37
        jmp bound_00afff3f // 00afff3a
    bound_00afff3c:
        movaps xmm1, xmm0 // 00afff3c
    bound_00afff3f:
        movss dword ptr [esp + 0xc], xmm1 // 00afff3f
    bound_00afff45:
        fld dword ptr [esi + 0xc] // 00afff45
        fstp dword ptr [esp + 0x10] // 00afff48
        fld dword ptr [esi + 0x10] // 00afff4c
        fstp dword ptr [esp + 4] // 00afff4f
        fld dword ptr [esi + 0x14] // 00afff53
        fstp dword ptr [esp + 0x14] // 00afff56
        fld dword ptr [esi + 0x18] // 00afff5a
        fstp dword ptr [esp + 0x18] // 00afff5d
        fld dword ptr [esp + 0x10] // 00afff61
        fld st(0) // 00afff65
        fld dword ptr [esp + 8] // 00afff67
        fld st(0) // 00afff6b
        fmulp st(2), st(0) // 00afff6d
        fld dword ptr [esp + 4] // 00afff6f
        fld st(0) // 00afff73
        faddp st(3), st(0) // 00afff75
        fld st(1) // 00afff77
        fmulp st(3), st(0) // 00afff79
        fld dword ptr [esp + 0x14] // 00afff7b
        fld st(0) // 00afff7f
        faddp st(4), st(0) // 00afff81
        fxch st(3) // 00afff83
        fmulp st(2), st(0) // 00afff85
        fld dword ptr [esp + 0x18] // 00afff87
        fld st(0) // 00afff8b
        faddp st(3), st(0) // 00afff8d
        fxch st(2) // 00afff8f
        fstp dword ptr [esp + 8] // 00afff91
        fld st(3) // 00afff95
        fld dword ptr [esp + 0xc] // 00afff97
        fld st(0) // 00afff9b
        fmulp st(2), st(0) // 00afff9d
        fld st(2) // 00afff9f
        faddp st(2), st(0) // 00afffa1
        fld st(0) // 00afffa3
        fmulp st(2), st(0) // 00afffa5
        fld st(4) // 00afffa7
        faddp st(2), st(0) // 00afffa9
        fmulp st(1), st(0) // 00afffab
        fadd st(0), st(2) // 00afffad
        fstp dword ptr [esp + 4] // 00afffaf
        fld st(4) // 00afffb3
        fmulp st(4), st(0) // 00afffb5
        faddp st(3), st(0) // 00afffb7
        fld st(3) // 00afffb9
        fmulp st(3), st(0) // 00afffbb
        fxch st(2) // 00afffbd
        faddp st(1), st(0) // 00afffbf
        fmulp st(2), st(0) // 00afffc1
        faddp st(1), st(0) // 00afffc3
        fstp dword ptr [esp + 0xc] // 00afffc5
        fld dword ptr [esp + 4] // 00afffc9
        fld dword ptr [esp + 8] // 00afffcd
        fcomi st(0), st(1) // 00afffd1
        jb bound_00afffeb // 00afffd3
        fstp st(1) // 00afffd5
        fld dword ptr [esp + 0xc] // 00afffd7
        fxch st(1) // 00afffdb
        fcomip st(0), st(1) // 00afffdd
        fstp st(0) // 00afffdf
        jb bound_00b00001 // 00afffe1
        movss xmm0, dword ptr [esp + 8] // 00afffe3
        jmp bound_00b00007 // 00afffe9
    bound_00afffeb:
        fstp st(0) // 00afffeb
        fld dword ptr [esp + 0xc] // 00afffed
        fxch st(1) // 00affff1
        fcomip st(0), st(1) // 00affff3
        fstp st(0) // 00affff5
        jb bound_00b00001 // 00affff7
        movss xmm0, dword ptr [esp + 4] // 00affff9
        jmp bound_00b00007 // 00afffff
    bound_00b00001:
        movss xmm0, dword ptr [esp + 0xc] // 00b00001
    bound_00b00007:
        fld dword ptr [esi + 0x18] // 00b00007
        movss dword ptr [esp + 0xc], xmm0 // 00b0000a
        fstp dword ptr [esp + 8] // 00b00010
        pop esi // 00b00014
        fld dword ptr [esp + 4] // 00b00015
        fld dword ptr [esp + 8] // 00b00019
        fcomip st(0), st(1) // 00b0001d
        fstp st(0) // 00b0001f
        ja bound_00b00029 // 00b00021
        movss xmm0, dword ptr [esp + 4] // 00b00023
    bound_00b00029:
        movss dword ptr [esp + 8], xmm0 // 00b00029
        fld dword ptr [esp + 8] // 00b0002f
        add esp, 0x18 // 00b00033
        ret  // 00b00036
    }
}
__declspec(naked) float parameter_bound_kernel() {
    __asm {
        sub esp, 0xc // 00b001a0
        push edi // 00b001a3
        mov edi, ecx // 00b001a4
        movzx eax, word ptr [edi + 0xa] // 00b001a6
        sub eax, 0 // 00b001aa
        je bound_00b0029f // 00b001ad
        sub eax, 1 // 00b001b3
        je bound_00b00225 // 00b001b6
        sub eax, 1 // 00b001b8
        je bound_00b001c4 // 00b001bb
        fldz  // 00b001bd
        pop edi // 00b001bf
        add esp, 0xc // 00b001c0
        ret  // 00b001c3
    bound_00b001c4:
        push esi // 00b001c4
        mov esi, dword ptr [edi + 4] // 00b001c5
        movss xmm0, dword ptr [esi + 0xc] // 00b001c8
        movss dword ptr [esp + 8], xmm0 // 00b001cd
    bound_00b001d3:
        mov ecx, esi // 00b001d3
        call cubic_bound_kernel // 00b001d5
        fstp dword ptr [esp + 0xc] // 00b001da
        fld dword ptr [esp + 0xc] // 00b001de
        fld dword ptr [esp + 8] // 00b001e2
        fcomip st(0), st(1) // 00b001e6
        fstp st(0) // 00b001e8
        ja bound_00b001f8 // 00b001ea
        movss xmm0, dword ptr [esp + 0xc] // 00b001ec
        movss dword ptr [esp + 8], xmm0 // 00b001f2
    bound_00b001f8:
        push edx
        mov edx,dword ptr [ebx+4]
        mov edx,dword ptr [edx]
        fld qword ptr [edx] // 00b001f8
        pop edx
        fld dword ptr [esi + 4] // 00b001fe
        fcomip st(0), st(1) // 00b00201
        jae bound_00b0020c // 00b00203
        fstp st(0) // 00b00205
        add esi, 0x1c // 00b00207
        jmp bound_00b001d3 // 00b0020a
    bound_00b0020c:
        fdivr dword ptr [edi] // 00b0020c
        pop esi // 00b0020e
        pop edi // 00b0020f
        push edx
        mov edx,dword ptr [ebx+8]
        mov edx,dword ptr [edx]
        fadd qword ptr [edx] // 00b00210
        pop edx
        fmul dword ptr [esp] // 00b00216
        fstp dword ptr [esp + 4] // 00b00219
        fld dword ptr [esp + 4] // 00b0021d
        add esp, 0xc // 00b00221
        ret  // 00b00224
    bound_00b00225:
        mov ecx, dword ptr [edi + 4] // 00b00225
        push edx
        mov edx,dword ptr [ebx+4]
        mov edx,dword ptr [edx]
        fld qword ptr [edx] // 00b00228
        pop edx
        movss xmm0, dword ptr [ecx + 8] // 00b0022e
        xorps xmm1, xmm1 // 00b00233
        movss dword ptr [esp + 4], xmm0 // 00b00236
    bound_00b0023c:
        movss xmm0, dword ptr [ecx + 0xc] // 00b0023c
        comiss xmm0, xmm1 // 00b00241
        movss dword ptr [esp + 0xc], xmm0 // 00b00244
        jbe bound_00b0025a // 00b0024a
        fld dword ptr [ecx + 4] // 00b0024c
        fsub dword ptr [ecx] // 00b0024f
        fmul dword ptr [esp + 0xc] // 00b00251
        fadd dword ptr [ecx + 8] // 00b00255
        jmp bound_00b0025d // 00b00258
    bound_00b0025a:
        fld dword ptr [ecx + 8] // 00b0025a
    bound_00b0025d:
        fstp dword ptr [esp + 8] // 00b0025d
        fld dword ptr [esp + 8] // 00b00261
        fld dword ptr [esp + 4] // 00b00265
        fcomip st(0), st(1) // 00b00269
        fstp st(0) // 00b0026b
        ja bound_00b0027b // 00b0026d
        movss xmm0, dword ptr [esp + 8] // 00b0026f
        movss dword ptr [esp + 4], xmm0 // 00b00275
    bound_00b0027b:
        fld dword ptr [ecx + 4] // 00b0027b
        fcomip st(0), st(1) // 00b0027e
        jae bound_00b00287 // 00b00280
        add ecx, 0x14 // 00b00282
        jmp bound_00b0023c // 00b00285
    bound_00b00287:
        fdivr dword ptr [edi] // 00b00287
        pop edi // 00b00289
        push edx
        mov edx,dword ptr [ebx+8]
        mov edx,dword ptr [edx]
        fadd qword ptr [edx] // 00b0028a
        pop edx
        fmul dword ptr [esp] // 00b00290
        fstp dword ptr [esp + 8] // 00b00293
        fld dword ptr [esp + 8] // 00b00297
        add esp, 0xc // 00b0029b
        ret  // 00b0029e
    bound_00b0029f:
        fld dword ptr [edi] // 00b0029f
        push edx
        mov edx,dword ptr [ebx+4]
        mov edx,dword ptr [edx]
        fdiv qword ptr [edx] // 00b002a1
        pop edx
        push edx
        mov edx,dword ptr [ebx+8]
        mov edx,dword ptr [edx]
        fadd qword ptr [edx] // 00b002a7
        pop edx
        fmul dword ptr [edi + 4] // 00b002ad
        pop edi // 00b002b0
        fstp dword ptr [esp + 8] // 00b002b1
        fld dword ptr [esp + 8] // 00b002b5
        add esp, 0xc // 00b002b9
        ret  // 00b002bc
    }
}

__declspec(naked) float __fastcall invoke_parameter_bound(const void*,const BoundAccess*) {
    __asm { push ebx
        mov ebx,edx
        call parameter_bound_kernel
        pop ebx
        ret
    }
}
__declspec(naked) float __fastcall invoke_cubic_bound(const void*,const BoundAccess*) {
    __asm { push ebx
        mov ebx,edx
        call cubic_bound_kernel
        pop ebx
        ret
    }
}
__declspec(naked) float __fastcall invoke_quadratic(float*,float*,float,float,float,const BoundAccess*) {
    __asm { lea eax,[esp+4]
        push ebx
        mov ebx,dword ptr [eax+12]
        push dword ptr [eax+8]
        push dword ptr [eax+4]
        push dword ptr [eax]
        call quadratic_roots_kernel
        pop ebx
        ret 16
    }
}

// EAX=context; bind member ADDRESSES, not current pointer values. CRT is a
// reference to the same access object, whose own cells remain live as before.
#define BSP_BOUND_RAW_VIEW() \
    __asm { lea ebx,[eax+24] } \
    __asm { push ebx } \
    __asm { lea ebx,[eax+20] } \
    __asm { push ebx } \
    __asm { lea ebx,[eax+16] } \
    __asm { push ebx } \
    __asm { lea ebx,[eax+12] } \
    __asm { push ebx } \
    __asm { push dword ptr [eax+8] } \
    __asm { mov ebx,esp }

__declspec(naked) float __fastcall raw_parameter_bound_entry(const void*,const RawContext*) {
    __asm { push ebx
        push eax
        mov eax,edx
    }
    BSP_BOUND_RAW_VIEW()
    __asm { call parameter_bound_kernel
        add esp,20
        pop eax
        pop ebx
        ret
    }
}
BoundAccess bounds(NativeParticleTypeLoadingBindings& b) {
    return {&b.parameters.crt,&b.parameters.last_time_00d7a220,&b.bound_base_00d7a210,
        &b.derivative_scale_00d7a2b0,&b.discriminant_scale_00d7a328};
}
} // namespace
float bound_native_runtime_particle_value_00b001a0(const void* parameter,
    NativeParticleTypeLoadingBindings& b) {
    const BoundAccess access=bounds(b);
    return invoke_parameter_bound(parameter,&access);
}
float bound_native_particle_cubic_segment_00affe90(const void* segment,
    NativeParticleTypeLoadingBindings& b) {
    const BoundAccess access=bounds(b);
    return invoke_cubic_bound(segment,&access);
}
float solve_native_particle_quadratic_00affe20(float* first,float* second,float a,float c,float d,
    NativeParticleTypeLoadingBindings& b) {
    const BoundAccess access=bounds(b);
    return invoke_quadratic(first,second,a,c,d,&access);
}
__declspec(naked) float __fastcall bound_native_runtime_particle_value_00b001a0(
    const void*,const RawContext*) {
    __asm { jmp raw_parameter_bound_entry }
}
__declspec(naked) float __fastcall bound_native_particle_cubic_segment_00affe90(
    const void*,const RawContext*) {
    __asm { push ebx
        push eax
        mov eax,edx
    }
    BSP_BOUND_RAW_VIEW()
    __asm { call cubic_bound_kernel
        add esp,20
        pop eax
        pop ebx
        ret
    }
}
__declspec(naked) float __fastcall solve_native_particle_quadratic_00affe20(
    float*,float*,float,float,float,const RawContext*) {
    __asm { push ebx
        push eax
        mov eax,dword ptr [esp+24]
    }
    BSP_BOUND_RAW_VIEW()
    __asm { push dword ptr [esp+40]
        push dword ptr [esp+40]
        push dword ptr [esp+40]
        call quadratic_roots_kernel
        add esp,20
        pop eax
        pop ebx
        ret 16
    }
}
#undef BSP_BOUND_RAW_VIEW
__declspec(naked) void __fastcall set_native_sprite_particle_size_00b08870(
    void*,const RawContext*,void*) {
    __asm { push esi
        mov esi,ecx
        mov ecx,dword ptr [esp+8]
        test ecx,ecx
        mov dword ptr [esi+88h],ecx
        jz raw_size_zero
        call raw_parameter_bound_entry
        fstp dword ptr [esp+8]
        movss xmm0,dword ptr [esp+8]
        movss dword ptr [esi+8ch],xmm0
        pop esi
        ret 4
    raw_size_zero:
        xorps xmm0,xmm0
        movss dword ptr [esi+8ch],xmm0
        pop esi
        ret 4
    }
}
} // namespace bsp
