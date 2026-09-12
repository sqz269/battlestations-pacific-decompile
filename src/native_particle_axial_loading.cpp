#include "bsp/native_particle_axial_loading.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
static_assert(sizeof(void*) == 4, "Native Axial loading requires Win32");
namespace {
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
template<class T> void store(void* p, std::uint32_t n, T value) noexcept {
    std::memcpy(at(p,n),&value,sizeof value);
}
class Pooled {
public:
    NativePooledTextStorage header{nullptr};
    explicit Pooled(NativeStringStorage& s) : storage_(s) {}
    Pooled(const void* line,std::int32_t index,NativeStringStorage& s,bool suffix=false)
        : storage_(s) {
        if(suffix) get_native_pooled_text_suffix_00af44c0(line,&header,index,s);
        else get_native_pooled_text_token_00aee3c0(line,&header,index,s);
    }
    ~Pooled() { destroy_native_pooled_text_00aee2a0(&header,storage_); }
    Pooled(const Pooled&)=delete;
    Pooled& operator=(const Pooled&)=delete;
private:
    NativeStringStorage& storage_;
};
class Builder {
public:
    NativeParticleParameterBuilderStorage& header;
    Builder(NativeParticleParameterBuilderStorage& storage,NativeParticleParameterLoadingBindings& b)
        : header(storage),bindings_(b) {
        construct_native_particle_parameter_builder_00afbed0(&header,b);
    }
    ~Builder() { destroy_native_particle_parameter_builder_00af4110(&header,bindings_); }
    Builder(const Builder&)=delete;
    Builder& operator=(const Builder&)=delete;
private:
    NativeParticleParameterLoadingBindings& bindings_;
};
bool is(const Pooled& p,const char* text) { return _stricmp(p.header.data,text)==0; }
void publish(void* definition,std::uint32_t offset,void* builder,float percentage,
    NativeParticleTypeLoadingBindings& b) {
    void* curve=convert_native_particle_parameter_00afbf60(builder,b.parameters);
    const volatile double* scale=b.percentage_scale_00d7a358;
    __asm {
        fld percentage
        mov eax,scale
        fmul qword ptr [eax]
        mov eax,curve
        fstp dword ptr [eax]
    }
    store(definition,offset,curve);
}
} // namespace

void* __fastcall build_native_particle_rotation_x_00b64640(void* destination,const float* angle,
    const volatile float* negative_zero,const volatile float* one) {
    float values[3];
    __asm {
        mov ecx,destination
        mov edx,angle
        movss xmm0,dword ptr [edx] // 00b64643
        mov eax,ecx // 00b64647
        movss dword ptr values[0],xmm0 // 00b64649
        fld float ptr values[0] // 00b6464e
        fsin // 00b64651
        fstp float ptr values[4] // 00b64653
        movss xmm0,dword ptr [edx] // 00b64657
        movss dword ptr values[8],xmm0 // 00b6465b
        fld float ptr values[8] // 00b64661
        fcos // 00b64665
        fstp float ptr values[0] // 00b64667
        xorps xmm0,xmm0 // 00b6466a
        movss xmm3,dword ptr values[4] // 00b6466d
        mov ecx,one
        movss xmm1,dword ptr [ecx] // 00b64673
        movss xmm2,dword ptr values[0] // 00b6467b
        mov ecx,negative_zero
        movss xmm4,dword ptr [ecx] // 00b64680
        subss xmm4,xmm3 // 00b64688
        movss dword ptr [eax],xmm1 // 00b6468c
        movss dword ptr [eax + 0x4],xmm0 // 00b64690
        movss dword ptr [eax + 0x8],xmm0 // 00b64695
        movss dword ptr [eax + 0xc],xmm0 // 00b6469a
        movss dword ptr [eax + 0x10],xmm0 // 00b6469f
        movss dword ptr [eax + 0x14],xmm2 // 00b646a4
        movss dword ptr [eax + 0x18],xmm3 // 00b646a9
        movss dword ptr [eax + 0x1c],xmm0 // 00b646ae
        movss dword ptr [eax + 0x20],xmm0 // 00b646b3
        movss dword ptr [eax + 0x24],xmm4 // 00b646b8
        movss dword ptr [eax + 0x28],xmm2 // 00b646bd
        movss dword ptr [eax + 0x2c],xmm0 // 00b646c2
        movss dword ptr [eax + 0x30],xmm0 // 00b646c7
        movss dword ptr [eax + 0x34],xmm0 // 00b646cc
        movss dword ptr [eax + 0x38],xmm0 // 00b646d1
        movss dword ptr [eax + 0x3c],xmm1 // 00b646d6
    }
    return destination;
}

void* __fastcall build_native_particle_rotation_y_00b646e0(void* destination,const float* angle,
    const volatile float* negative_zero,const volatile float* one) {
    float values[3];
    __asm {
        mov ecx,destination
        mov edx,angle
        movss xmm0,dword ptr [edx] // 00b646e3
        mov eax,ecx // 00b646e7
        movss dword ptr values[0],xmm0 // 00b646e9
        fld float ptr values[0] // 00b646ee
        fsin // 00b646f1
        fstp float ptr values[4] // 00b646f3
        movss xmm0,dword ptr [edx] // 00b646f7
        movss dword ptr values[8],xmm0 // 00b646fb
        fld float ptr values[8] // 00b64701
        fcos // 00b64705
        fstp float ptr values[0] // 00b64707
        xorps xmm0,xmm0 // 00b6470a
        movss xmm2,dword ptr values[4] // 00b6470d
        mov ecx,negative_zero
        movss xmm3,dword ptr [ecx] // 00b64713
        movss xmm1,dword ptr values[0] // 00b6471b
        subss xmm3,xmm2 // 00b64720
        movss dword ptr [eax + 0x8],xmm3 // 00b64724
        mov ecx,one
        movss xmm3,dword ptr [ecx] // 00b64729
        movss dword ptr [eax],xmm1 // 00b64731
        movss dword ptr [eax + 0x4],xmm0 // 00b64735
        movss dword ptr [eax + 0xc],xmm0 // 00b6473a
        movss dword ptr [eax + 0x10],xmm0 // 00b6473f
        movss dword ptr [eax + 0x14],xmm3 // 00b64744
        movss dword ptr [eax + 0x18],xmm0 // 00b64749
        movss dword ptr [eax + 0x1c],xmm0 // 00b6474e
        movss dword ptr [eax + 0x20],xmm2 // 00b64753
        movss dword ptr [eax + 0x24],xmm0 // 00b64758
        movss dword ptr [eax + 0x28],xmm1 // 00b6475d
        movss dword ptr [eax + 0x2c],xmm0 // 00b64762
        movss dword ptr [eax + 0x30],xmm0 // 00b64767
        movss dword ptr [eax + 0x34],xmm0 // 00b6476c
        movss dword ptr [eax + 0x38],xmm0 // 00b64771
        movss dword ptr [eax + 0x3c],xmm3 // 00b64776
    }
    return destination;
}

void build_gui_rotation_z_00b64780(CameraMatrix& matrix,const float& value,
    const volatile float* negative_zero,const volatile float* one) {
    void* destination=matrix.data();
    const float* angle=&value;
    float values[3];
    __asm {
        mov ecx,destination
        mov edx,angle
        movss xmm0,dword ptr [edx] // 00b64783
        mov eax,ecx // 00b64787
        movss dword ptr values[0],xmm0 // 00b64789
        fld float ptr values[0] // 00b6478e
        fsin // 00b64791
        fstp float ptr values[4] // 00b64793
        movss xmm0,dword ptr [edx] // 00b64797
        movss dword ptr values[8],xmm0 // 00b6479b
        fld float ptr values[8] // 00b647a1
        fcos // 00b647a5
        fstp float ptr values[0] // 00b647a7
        xorps xmm0,xmm0 // 00b647aa
        movss xmm1,dword ptr values[0] // 00b647ad
        movss xmm2,dword ptr values[4] // 00b647b2
        mov ecx,negative_zero
        movss xmm3,dword ptr [ecx] // 00b647b8
        movss dword ptr [eax],xmm1 // 00b647c0
        movss dword ptr [eax + 0x14],xmm1 // 00b647c4
        mov ecx,one
        movss xmm1,dword ptr [ecx] // 00b647c9
        subss xmm3,xmm2 // 00b647d1
        movss dword ptr [eax + 0x4],xmm2 // 00b647d5
        movss dword ptr [eax + 0x8],xmm0 // 00b647da
        movss dword ptr [eax + 0xc],xmm0 // 00b647df
        movss dword ptr [eax + 0x10],xmm3 // 00b647e4
        movss dword ptr [eax + 0x18],xmm0 // 00b647e9
        movss dword ptr [eax + 0x1c],xmm0 // 00b647ee
        movss dword ptr [eax + 0x20],xmm0 // 00b647f3
        movss dword ptr [eax + 0x24],xmm0 // 00b647f8
        movss dword ptr [eax + 0x28],xmm1 // 00b647fd
        movss dword ptr [eax + 0x2c],xmm0 // 00b64802
        movss dword ptr [eax + 0x30],xmm0 // 00b64807
        movss dword ptr [eax + 0x34],xmm0 // 00b6480c
        movss dword ptr [eax + 0x38],xmm0 // 00b64811
        movss dword ptr [eax + 0x3c],xmm1 // 00b64816
    }
}

void refresh_native_axial_particle_axis_00b05d00(void* definition,
    NativeParticleTypeLoadingBindings& b) {
    float angles[3];
    const volatile double* scale=b.angle_scale_00d5daf8;
    const volatile double* base=b.axis_base_00ce3830;
    __asm {
        mov ecx,definition
        fld dword ptr [ecx+88h]
        mov eax,scale
        fld qword ptr [eax]
        fmul st(1),st(0)
        xorps xmm0,xmm0
        movss dword ptr angles[8],xmm0
        fxch st(1)
        fstp dword ptr angles[0]
        fmul dword ptr [ecx+84h]
        mov eax,base
        fsubr qword ptr [eax]
        fstp dword ptr angles[4]
    }
    CameraMatrix x,y,z,zx,combined;
    build_native_particle_rotation_y_00b646e0(y.data(),&angles[0],b.negative_zero_00d7a208,b.one_00d7a24c);
    build_native_particle_rotation_x_00b64640(x.data(),&angles[1],b.negative_zero_00d7a208,b.one_00d7a24c);
    build_gui_rotation_z_00b64780(z,angles[2],b.negative_zero_00d7a208,b.one_00d7a24c);
    multiply_native_camera_matrices_00413920(z.data(),nullptr,zx.data(),x.data());
    multiply_native_camera_matrices_00413920(zx.data(),nullptr,combined.data(),y.data());
    const volatile float* one=b.one_00d7a24c;
    __asm {
        mov eax,one
        movss xmm1,dword ptr [eax]
        xorps xmm0,xmm0
        mov ecx,definition
        movss dword ptr [ecx+94h],xmm0
        movss dword ptr [ecx+98h],xmm1
        movss dword ptr [ecx+9ch],xmm0
    }
    // The original transform stages all three source values, with disjoint
    // matrix/output scratch. The typed provider has exactly that domain here.
    std::array<float,3> source,result;
    std::memcpy(source.data(),at(definition,0x94),sizeof source);
    transform_point_004142e0(source,combined,result);
    const float* output=result.data();
    __asm {
        mov eax,output
        mov ecx,definition
        fld dword ptr [eax]
        fstp dword ptr [ecx+94h]
        fld dword ptr [eax+4]
        fstp dword ptr [ecx+98h]
        fld dword ptr [eax+8]
        fstp dword ptr [ecx+9ch]
    }
}

void set_native_axial_particle_alignment_00b062f0(void* definition,const char* text,
    NativeStringStorage& strings) {
    NativeString value;
    value.assign_0041e870(strings,text);
    char* const captured=value.data();
    std::uint32_t alignment=0;
    if(captured && _stricmp(captured,"Center")==0) alignment=0;
    else if(equal_native_string_header_00425850(&value,"Bottom")) alignment=1;
    else if(equal_native_string_header_00425850(&value,"Top")) alignment=2;
    else if(equal_native_string_header_00425850(&value,"Left")) alignment=3;
    else {
        alignment=equal_native_string_header_00425850(&value,"Right") ? 4u : 0u;
        store(definition,0xa0,alignment);
        destroy_native_string_header_0041dd20(&value,strings);
        return;
    }
    store(definition,0xa0,alignment);
    if(captured) strings.release(captured,value.length()+1u);
}

bool load_native_axial_particle_definition_00b064a0(void* definition,void* text,
    NativeParticleTypeLoadingBindings& b) {
    auto& strings=b.parameters.owners.strings;
    NativeParticleParameterBuilderStorage builder_storage;
    builder_storage.kind_0c=b.initial_builder_kind_0c;
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
        if(is(name,"FollowDirection")) {
            Pooled value(&line.header,2,strings);
            store<std::uint8_t>(definition,0x80,std::atol(value.header.data)>0);
        } else if(is(name,"AxialScaleType")) {
            Pooled value(&line.header,2,strings);
            set_native_axial_particle_alignment_00b062f0(definition,value.header.data,strings);
        } else {
            float percentage;
            {
                Pooled suffix(&line.header,2,strings,true);
                Pooled value(&suffix.header,0,strings);
                percentage=static_cast<float>(std::atof(value.header.data));
            }
            Builder builder(builder_storage,b.parameters);
            initialize_native_particle_parameter_endpoints_00afc360(&builder.header,0.0f,0.0f,b.parameters);
            {
                Pooled suffix(&line.header,2,strings,true);
                Pooled curve(&suffix.header,1,strings,true);
                (void)parse_native_particle_parameter_00afc470(&builder.header,&curve.header,b.parameters);
            }
            if(load_native_particle_type_parameter_00b00980(definition,&name.header,
                &builder.header,percentage,b)) continue;
            if(is(name,"AngleElevation")) {
                store(definition,0x84,first_native_particle_parameter_value_00afc1b0(&builder.header));
                refresh_native_axial_particle_axis_00b05d00(definition,b);
            } else if(is(name,"AngleHeading")) {
                store(definition,0x88,first_native_particle_parameter_value_00afc1b0(&builder.header));
                refresh_native_axial_particle_axis_00b05d00(definition,b);
            } else if(is(name,"Length")) publish(definition,0x8c,&builder.header,percentage,b);
            else if(is(name,"Width")) publish(definition,0x90,&builder.header,percentage,b);
        }
    }
    return true;
}
} // namespace bsp
