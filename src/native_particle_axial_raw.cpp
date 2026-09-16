#include "bsp/native_particle_axial_raw.hpp"
#include "bsp/native_particle_axial_loading.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Axial particle composition requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeParticleAxialRawAcquired;
Word word(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void* at(void* p, Word n) noexcept { return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n); }
void put(void* p, Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
const char* text(const void* p) noexcept { return reinterpret_cast<const char*>(word(p)); }
void return_captured(const char* p, NativeStringRawPoolContext& c) {
    if(p) release_native_pooled_text_bytes_00aee1e0(const_cast<char*>(p),c);
}
void clear_inline(Word* header, NativeStringRawPoolContext& c) {
    return_captured(text(header),c);
    put(header,0);
}
void unwind(Acquired& a, NativeParticleAxialRawContext& c) noexcept {
    static constexpr int previous[]{-1,0,0,2,2,2,5,6};
    static constexpr unsigned slots[]{1,4,0,6,8,11,10,9};
    try {
        while(a.unwind_state>=0) {
            const int state=a.unwind_state;
            a.unwind_state=previous[state];
            auto* header=&a.native_locals_48_10[slots[state]];
            if(state==5) destroy_native_particle_parameter_builder_00af4110(header,c.builder);
            else destroy_native_pooled_text_00aee2a0(header,c.builder.strings);
        }
    } catch(...) { std::terminate(); }
}
void number_to_float(const char* value, Word* destination) {
    __asm {
        push value
        call atof
        mov ecx,destination
        fstp dword ptr [ecx]
        add esp,4
    }
}
bool common_parameter(void* definition, const void* name, void* builder,
    Word* percentage, Word* suffix, NativeParticleTypeParameterRawContext& c) {
    using Function=bool (*)(void*,const void*,void*,float,NativeParticleTypeParameterRawContext&);
    const auto function=static_cast<Function>(&load_native_particle_type_parameter_00b00980);
    auto* context=&c;
    Word result;
    __asm {
        push context
        mov ecx,percentage
        fld dword ptr [ecx]
        push ecx
        fstp dword ptr [esp]
        push builder
        push name
        push definition
        mov ecx,suffix
        mov dword ptr [ecx],0 // B068B6: after argument spill, before call.
        call function
        add esp,20
        mov result,eax
    }
    return static_cast<unsigned char>(result)!=0;
}
void first_to_field(void* builder, Word* spill, void* destination) {
    const auto function=&first_native_particle_parameter_value_00afc1b0;
    __asm {
        push builder
        call function
        add esp,4
        mov ecx,spill
        fstp dword ptr [ecx]
        fld dword ptr [ecx]
        mov eax,destination
        fstp dword ptr [eax]
    }
}
void publish(void* definition, Word offset, void* builder, Word* percentage,
    NativeParticleTypeParameterRawContext& c) {
    void* curve=convert_native_particle_parameter_00afbf60(builder,c.parameters);
    auto* scale_cell=&c.percentage_scale_00d7a358;
    __asm {
        mov ecx,percentage
        fld dword ptr [ecx]
        mov ecx,scale_cell
        mov ecx,dword ptr [ecx]
        fmul qword ptr [ecx]
        mov eax,curve
        fstp dword ptr [eax]
    }
    put(at(definition,offset),reinterpret_cast<Word>(curve));
}
} // namespace

void refresh_native_axial_particle_axis_00b05d00(void* definition,
    NativeParticleAxialAxisRawContext& c) {
    float angles[3];
    auto* scale_cell=&c.angle_scale_00d5daf8;
    auto* base_cell=&c.axis_base_00ce3830;
    __asm {
        mov ecx,definition
        fld dword ptr [ecx+88h]
        mov eax,scale_cell
        mov eax,dword ptr [eax]
        fld qword ptr [eax]
        fmul st(1),st(0)
        xorps xmm0,xmm0
        movss dword ptr angles[8],xmm0
        fxch st(1)
        fstp dword ptr angles[0]
        fmul dword ptr [ecx+84h]
        mov eax,base_cell
        mov eax,dword ptr [eax]
        fsubr qword ptr [eax]
        fstp dword ptr angles[4]
    }
    CameraMatrix x,y,z,zx,combined;
    build_native_particle_rotation_y_00b646e0(y.data(),&angles[0],c.negative_zero_00d7a208,c.one_00d7a24c);
    build_native_particle_rotation_x_00b64640(x.data(),&angles[1],c.negative_zero_00d7a208,c.one_00d7a24c);
    build_gui_rotation_z_00b64780(z,angles[2],c.negative_zero_00d7a208,c.one_00d7a24c);
    multiply_native_camera_matrices_00413920(z.data(),nullptr,zx.data(),x.data());
    multiply_native_camera_matrices_00413920(zx.data(),nullptr,combined.data(),y.data());
    auto* one_cell=&c.one_00d7a24c;
    __asm {
        mov eax,one_cell
        mov eax,dword ptr [eax]
        movss xmm1,dword ptr [eax]
        xorps xmm0,xmm0
        mov ecx,definition
        movss dword ptr [ecx+94h],xmm0
        movss dword ptr [ecx+98h],xmm1
        movss dword ptr [ecx+9ch],xmm0
    }
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

void set_native_axial_particle_alignment_00b062f0(void* definition,const char* value,
    NativeStringRawPoolContext& c) {
    Word header[2];
    construct_native_string_header_0041e870(header,c,value);
    char* const captured=reinterpret_cast<char*>(word(header+1));
    Word alignment;
    if(captured && _stricmp(captured,"Center")==0) alignment=0;
    else if(equal_native_string_header_00425850(header,"Bottom")) alignment=1;
    else if(equal_native_string_header_00425850(header,"Top")) alignment=2;
    else if(equal_native_string_header_00425850(header,"Left")) alignment=3;
    else {
        alignment=equal_native_string_header_00425850(header,"Right") ? 4u : 0u;
        put(at(definition,0xa0),alignment);
        destroy_native_string_header_0041dd20(header,c);
        return;
    }
    put(at(definition,0xa0),alignment);
    if(captured) {
        const Word size=word(header)+1u;
        auto* pool=native_string_pool_get_or_create_00419cc0(
            c.actual_published_01090aa8,c.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool,captured,size,c.actual_small_returns_disabled_01090aa4);
    }
}

bool load_native_axial_particle_definition_00b064a0(void* definition,void* buffer,
    NativeParticleAxialRawContext& c,NativeParticleAxialRawAcquired& a) {
    if(a.phase!=Acquired::Phase::fresh) throw std::logic_error("Axial frame is not fresh");
    a.phase=Acquired::Phase::running;
    auto* h=a.native_locals_48_10;
    auto& strings=c.builder.strings;
    put(h+1,0);
    put(h+14,c.initial_builder_kind_0c);
    a.unwind_state=0;
    try {
        for(;;) {
            a.native_site=0x00b064d2;
            if(!read_native_text_buffer_line_00af5740(buffer,h+1,strings,c.text_scratch_00f8c2c8)) break;
            a.native_site=0x00b064e5;
            if(_stricmp(text(h+1),"{")==0) break;
        }
        a.native_site=0x00b064f8;
        bool next=read_native_text_buffer_line_00af5740(buffer,h+1,strings,c.text_scratch_00f8c2c8);
        while(next) {
            a.native_site=0x00b06510;
            if(_stricmp(text(h+1),"}")==0) break;
            a.native_site=0x00b0652a;
            if(_stricmp(text(h+1),"")!=0) {
                a.native_site=0x00b06544;
                const void* token=get_native_pooled_text_token_00aee3c0(h+1,h+3,0,strings);
                a.native_site=0x00b06551;
                const bool parameter=_stricmp(text(token),"Param")==0;
                a.native_site=0x00b06589;
                clear_inline(h+3,strings);
                if(parameter) {
                    a.native_site=0x00b065a5;
                    const void* suffix=get_native_pooled_text_suffix_00af44c0(h+1,h+4,1,strings);
                    a.unwind_state=1;
                    a.native_site=0x00b065b2;
                    a.property.emplace(); // prior child is completed; failures never reach this again.
                    const bool handled=load_native_particle_type_property_00b015c0(definition,suffix,c.properties,*a.property);
                    const char* captured=text(h+4);
                    a.unwind_state=0;
                    a.native_site=0x00b065e9;
                    return_captured(captured,strings);
                    put(h+4,0);
                    if(!handled) {
                        a.native_site=0x00b06605;
                        get_native_pooled_text_token_00aee3c0(h+1,h,1,strings);
                        a.unwind_state=2;
                        a.native_site=0x00b06619;
                        if(_stricmp(text(h),"FollowDirection")==0) {
                            a.native_site=0x00b06630;
                            token=get_native_pooled_text_token_00aee3c0(h+1,h+5,2,strings);
                            a.native_site=0x00b06638;
                            const long value=std::atol(text(token));
                            captured=text(h+5);
                            *static_cast<volatile unsigned char*>(at(definition,0x80))=static_cast<unsigned char>(value>0);
                            a.native_site=0x00b06671;
                            return_captured(captured,strings);
                            captured=text(h);
                            put(h+5,0);
                            a.unwind_state=0;
                            a.native_site=0x00b0673d;
                            return_captured(captured,strings);
                            put(h,0);
                        } else {
                            a.native_site=0x00b066a8;
                            if(_stricmp(text(h),"AxialScaleType")==0) {
                                a.native_site=0x00b066c3;
                                token=get_native_pooled_text_token_00aee3c0(h+1,h+6,2,strings);
                                a.unwind_state=3;
                                a.native_site=0x00b066d2;
                                set_native_axial_particle_alignment_00b062f0(definition,text(token),strings);
                                captured=text(h+6);
                                a.unwind_state=2;
                                a.native_site=0x00b06709;
                                return_captured(captured,strings);
                                captured=text(h);
                                put(h+6,0);
                                a.unwind_state=0;
                                a.native_site=0x00b0673d;
                                return_captured(captured,strings);
                                put(h,0);
                            } else {
                                a.native_site=0x00b06754;
                                suffix=get_native_pooled_text_suffix_00af44c0(h+1,h+8,2,strings);
                                a.unwind_state=4;
                                a.native_site=0x00b06766;
                                token=get_native_pooled_text_token_00aee3c0(suffix,h+7,0,strings);
                                a.native_site=0x00b0676e;
                                number_to_float(text(token),h+2);
                                a.native_site=0x00b067a0;
                                return_captured(text(h+7),strings);
                                captured=text(h+8);
                                put(h+7,0);
                                a.unwind_state=2;
                                a.native_site=0x00b067d9;
                                return_captured(captured,strings);
                                put(h+8,0);
                                a.native_site=0x00b067e6;
                                construct_native_particle_parameter_builder_00afbed0(h+11,c.builder);
                                a.unwind_state=5;
                                a.native_site=0x00b06800;
                                initialize_native_particle_parameter_endpoints_00afc360(h+11,0.0f,0.0f,c.builder);
                                a.native_site=0x00b06810;
                                suffix=get_native_pooled_text_suffix_00af44c0(h+1,h+10,2,strings);
                                a.unwind_state=6;
                                a.native_site=0x00b06823;
                                suffix=get_native_pooled_text_suffix_00af44c0(suffix,h+9,1,strings);
                                a.unwind_state=7;
                                a.native_site=0x00b06832;
                                (void)parse_native_particle_parameter_00afc470(h+11,suffix,c.builder);
                                captured=text(h+9);
                                a.unwind_state=6;
                                a.native_site=0x00b06869;
                                return_captured(captured,strings);
                                captured=text(h+10);
                                put(h+9,0);
                                a.unwind_state=5;
                                a.native_site=0x00b0689d;
                                return_captured(captured,strings);
                                a.native_site=0x00b068ba;
                                if(common_parameter(definition,h,h+11,h+2,h+10,c.parameters)) {
                                    a.native_site=0x00b068c8;
                                    destroy_native_particle_parameter_key_vector_00af4060(h+11);
                                } else {
                                    a.native_site=0x00b068eb;
                                    if(_stricmp(text(h),"AngleElevation")==0) {
                                        a.native_site=0x00b068fb;
                                        first_to_field(h+11,h+2,at(definition,0x84));
                                        a.native_site=0x00b06910;
                                        refresh_native_axial_particle_axis_00b05d00(definition,c.axis);
                                        a.native_site=0x00b068c8;
                                        destroy_native_particle_parameter_key_vector_00af4060(h+11);
                                    } else {
                                        a.native_site=0x00b06926;
                                        if(_stricmp(text(h),"AngleHeading")==0) {
                                            a.native_site=0x00b06936;
                                            first_to_field(h+11,h+2,at(definition,0x88));
                                            a.native_site=0x00b0694b;
                                            refresh_native_axial_particle_axis_00b05d00(definition,c.axis);
                                        } else {
                                            a.native_site=0x00b0695c;
                                            if(_stricmp(text(h),"Length")==0) {
                                                a.native_site=0x00b0696c;
                                                publish(definition,0x8c,h+11,h+2,c.parameters);
                                            } else {
                                                a.native_site=0x00b0698f;
                                                if(_stricmp(text(h),"Width")==0) {
                                                    a.native_site=0x00b0699f;
                                                    publish(definition,0x90,h+11,h+2,c.parameters);
                                                }
                                            }
                                        }
                                        a.native_site=0x00b069ba;
                                        destroy_native_particle_parameter_builder_00af4110(h+11,c.builder);
                                    }
                                }
                                a.unwind_state=0;
                                a.native_site=0x00b069c8;
                                destroy_native_pooled_text_00aee2a0(h,strings);
                            }
                        }
                    }
                }
            }
            a.native_site=0x00b069d6;
            next=read_native_text_buffer_line_00af5740(buffer,h+1,strings,c.text_scratch_00f8c2c8);
        }
        const char* captured=text(h+1);
        a.unwind_state=-1;
        a.native_site=0x00b06a19;
        return_captured(captured,strings); // Native final inline return does not zero the line header.
        a.phase=Acquired::Phase::complete;
        return true;
    } catch(...) {
        a.phase=Acquired::Phase::failed;
        unwind(a,c);
        throw;
    }
}
} // namespace bsp
