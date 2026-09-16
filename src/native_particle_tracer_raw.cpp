#include "bsp/native_particle_tracer_raw.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Tracer parsing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
template<class T> T& field(void* p, Word offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
const char* text(const void* h) noexcept { return *static_cast<char* const volatile*>(h); }
void return_captured(char* captured, NativeStringRawPoolContext& strings) {
    if (captured) release_native_pooled_text_bytes_00aee1e0(captured, strings);
}
void clear_inline(NativePooledTextStorage& h, NativeStringRawPoolContext& strings) {
    return_captured(h.data, strings);
    h.data = nullptr;
}
void parse_scalar(const char* input, float* output) {
    using Atof = double (__cdecl*)(const char*);
    Atof const convert = &std::atof;
    __asm { push input
        call convert
        add esp, 4
        mov eax, output
        fstp dword ptr[eax] }
}
void load_spill(const float* input, float* output) {
    __asm { mov eax, input
        fld dword ptr[eax]
        mov eax, output
        fstp dword ptr[eax] }
}
void store_first(void* builder, void* destination) {
    using First = float (__cdecl*)(const void*);
    First const first = &first_native_particle_parameter_value_00afc1b0;
    __asm { push builder
        call first
        add esp, 4
        mov eax, destination
        fstp dword ptr[eax] }
}
void store_percentage(void* parameter, const float* scalar,
    const NativeParticleTypeParameterRawContext& context) {
    auto* scale_cell = &context.percentage_scale_00d7a358;
    __asm { mov eax, scalar
        fld dword ptr[eax]
        mov ecx, scale_cell
        mov ecx, [ecx]
        fmul qword ptr[ecx]
        mov eax, parameter
        fstp dword ptr[eax] }
}
void unwind(NativeParticleTracerRawAcquired& a, NativeParticleTracerRawContext& c) noexcept {
    static constexpr int previous[]{-1,0,0,2,2,2,2,2,7,8};
    try {
        while (a.unwind_state >= 0) {
            const int state = a.unwind_state;
            a.unwind_state = previous[state];
            if (state == 7) {
                destroy_native_particle_parameter_builder_00af4110(&a.builder,c.builders);
                continue;
            }
            NativePooledTextStorage* const headers[]{&a.line,&a.common_suffix,&a.name,
                &a.head,&a.tail,&a.texture,&a.scalar_suffix,nullptr,&a.outer_suffix,&a.inner_suffix};
            destroy_native_pooled_text_00aee2a0(headers[state],c.builders.strings);
        }
    } catch (...) { std::terminate(); }
}
void append_texture(void* definition, void* item) {
    const Word capacity = field<volatile Word>(definition,0xa0);
    if (field<volatile Word>(definition,0x9c) == capacity) {
        const Word next = capacity * 2u + 2u;
        if (next > field<volatile Word>(definition,0xa0)) {
            field<Word>(definition,0xa0) = next; // published before allocation
            const Word bytes = next > 0x3fffffffu ? 0xffffffffu : next * 4u;
            auto* output = static_cast<Word*>(singleton_lifetime_allocate(
                {SingletonAllocationKind::object,bytes,bytes}));
            if (field<void* volatile>(definition,0x98)) {
                for (Word i=0;i<field<volatile Word>(definition,0x9c);++i)
                    output[i] = field<Word>(field<void* volatile>(definition,0x98),i*4u);
                singleton_lifetime_free(field<void* volatile>(definition,0x98));
            }
            field<void*>(definition,0x98) = output;
        }
    }
    const Word index = field<volatile Word>(definition,0x9c);
    field<void*>(field<void* volatile>(definition,0x98),index*4u) = item;
    ++field<volatile Word>(definition,0x9c);
}
struct FrameUnwind {
    Word header[2];
    NativeStringRawPoolContext& strings;
    bool armed = false;
    ~FrameUnwind() noexcept {
        if (armed) destroy_native_string_header_0041dd20(header,strings);
    }
    void release() {
        armed = false; // B0AB0E/B0AB55 disarm before normal return.
        destroy_native_string_header_0041dd20(header,strings);
    }
};
struct Parameter { const char* name; Word offset; bool runtime; Word site; };
constexpr Parameter parameters[]{
    {"MaxSegmentNum",0x80,false,0xb0b18a},{"MinSegmentLength",0x84,false,0xb0b1b4},
    {"SegmentLifeTime",0x88,false,0xb0b1de},{"BulletRadius",0x8c,true,0xb0b208},
    {"BulletLifeTime",0x90,false,0xb0b23e},{"BulletFadeBegin",0x94,true,0xb0b268},
    {"BulletTailLength",0xac,false,0xb0b29e},{"OuterColor_R",0xb4,true,0xb0b2c8},
    {"OuterColor_G",0xb8,true,0xb0b2fe},{"OuterColor_B",0xbc,true,0xb0b334},
    {"Glow_R",0xc0,true,0xb0b36a},{"Glow_G",0xc4,true,0xb0b3a0},
    {"Glow_B",0xc8,true,0xb0b3d6},{"Tail_R",0xcc,true,0xb0b40c},
    {"Tail_G",0xd0,true,0xb0b442},{"Tail_B",0xd4,true,0xb0b478},
    {"Width",0xd8,true,0xb0b4ae},{"TileLength",0xe4,true,0xb0b4e4},
    {"WidthSpeed",0xdc,true,0xb0b50b},{"WidthSpeedEnd",0xe0,true,0xb0b53e}
};
} // namespace

void load_native_tracer_particle_textures_00b0a920(void* definition,
    const char* filename, NativeParticleTextureNamesRawContext& names, const char* empty) {
    field<Word>(definition,0x9c) = 0;
    const auto count = count_native_particle_texture_frames_00af3a20(filename,names);
    if (count == 0) {
        void* item = find_native_particle_atlas_item_00aefb20(
            names.actual_atlas_manager_00f8c26c,filename,names.strings,names.null_pattern_00e17bf0);
        if (item) append_texture(definition,item);
        return;
    }
    for (std::int32_t index=0;index<count;++index) {
        FrameUnwind frame{{},names.strings};
        construct_native_particle_texture_frame_name_00af3b50(frame.header,filename,index,names);
        frame.armed = true;
        const char* current = reinterpret_cast<const char*>(frame.header[1]);
        if (!current) current = empty;
        void* item = find_native_particle_atlas_item_00aefb20(
            names.actual_atlas_manager_00f8c26c,current,names.strings,names.null_pattern_00e17bf0);
        if (item) append_texture(definition,item);
        frame.release();
        if (!item) return;
    }
}

bool load_native_tracer_particle_definition_00b0ad50(void* definition, void* buffer,
    NativeParticleTracerRawContext& c, NativeParticleTracerRawAcquired& a) {
    using Phase = NativeParticleTracerRawAcquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Tracer parser cannot replay");
    if (&c.builders.strings != &c.properties.strings || &c.builders.strings != &c.names.strings)
        throw std::invalid_argument("Tracer providers require the same raw string pool context");
    a.phase = Phase::running;
    auto& strings = c.builders.strings;
    auto token = [&](const void* line, NativePooledTextStorage& out, int index, Word site) {
        a.native_site=site;return get_native_pooled_text_token_00aee3c0(line,&out,index,strings);
    };
    auto suffix = [&](const void* line, NativePooledTextStorage& out, int index, Word site) {
        a.native_site=site;return get_native_pooled_text_suffix_00af44c0(line,&out,index,strings);
    };
    auto clear = [&](NativePooledTextStorage& h, int state, Word site, bool inlined=false) {
        a.unwind_state=state;a.native_site=site;
        if (inlined) clear_inline(h,strings);else destroy_native_pooled_text_00aee2a0(&h,strings);
    };
    auto read_line = [&](Word site) {
        a.native_site=site;return read_native_text_buffer_line_00af5740(buffer,&a.line,strings,c.text_scratch_00f8c2c8);
    };
    try {
        field<unsigned char>(definition,0x64)=0;
        a.line.data=nullptr;a.unwind_state=0;
        while (read_line(0xb0ad87)) if (_stricmp(text(&a.line),"{")==0) break;
        bool have_line=read_line(0xb0adad);
        while (have_line) {
            if (_stricmp(text(&a.line),"}")==0) break;
            if (_stricmp(text(&a.line),"")!=0) {
                const void* keyword=token(&a.line,a.keyword,0,0xb0adfe);
                const bool param=_stricmp(text(keyword),"Param")==0;
                clear(a.keyword,0,0xb0ae3e,true);
                if (param) {
                    void* common=suffix(&a.line,a.common_suffix,1,0xb0ae5a);
                    a.unwind_state=1;a.native_site=0xb0ae67;
                    a.property_children.emplace_back();
                    const bool handled=load_native_particle_type_property_00b015c0(
                        definition,common,c.properties,a.property_children.back());
                    clear(a.common_suffix,0,0xb0ae99,true);
                    if (!handled) {
                        token(&a.line,a.name,1,0xb0aeb5);a.unwind_state=2;
                        if (_stricmp(text(&a.name),"ShowBullet")==0) {
                            const void* value=token(&a.line,a.boolean_value,2,0xb0aee0);
                            a.native_site=0xb0aee8;
                            const long number=std::atol(text(value));
                            field<unsigned char>(definition,0xb0)=number>0?1:0;
                            clear(a.boolean_value,2,0xb0af21,true);
                            clear(a.name,0,0xb0aff0,true);
                        } else if (_stricmp(text(&a.name),"BulletHeadTexture")==0) {
                            const void* value=token(&a.line,a.head,2,0xb0af73);
                            a.unwind_state=3;a.native_site=0xb0af86;
                            void* item=find_native_particle_atlas_item_00aefb20(c.names.actual_atlas_manager_00f8c26c,text(value),strings,c.names.null_pattern_00e17bf0);
                            field<void*>(definition,0xa4)=item;
                            clear(a.head,2,0xb0afbc,true);clear(a.name,0,0xb0aff0,true);
                        } else if (_stricmp(text(&a.name),"BulletTailTexture")==0) {
                            const void* value=token(&a.line,a.tail,2,0xb0b01f);
                            a.unwind_state=4;a.native_site=0xb0b032;
                            void* item=find_native_particle_atlas_item_00aefb20(c.names.actual_atlas_manager_00f8c26c,text(value),strings,c.names.null_pattern_00e17bf0);
                            field<void*>(definition,0xa8)=item;
                            clear(a.tail,2,0xb0b046);clear(a.name,0,0xb0b567);
                        } else if (_stricmp(text(&a.name),"TracerTexture")==0) {
                            const void* value=token(&a.line,a.texture,2,0xb0b071);
                            a.unwind_state=5;a.native_site=0xb0b080;
                            load_native_tracer_particle_textures_00b0a920(definition,text(value),c.names,c.empty_frame_name_00f8d390);
                            clear(a.texture,2,0xb0b08e);clear(a.name,0,0xb0b567);
                        } else {
                            void* initial=suffix(&a.line,a.scalar_suffix,2,0xb0b0a1);
                            a.unwind_state=6;
                            const void* value=token(initial,a.scalar_token,0,0xb0b0b3);
                            a.native_site=0xb0b0bb;parse_scalar(text(value),&a.scalar);
                            clear(a.scalar_token,6,0xb0b0cb);clear(a.scalar_suffix,2,0xb0b0d9);
                            a.native_site=0xb0b0e2;construct_native_particle_parameter_builder_00afbed0(&a.builder,c.builders);
                            a.unwind_state=7;a.native_site=0xb0b0fc;
                            initialize_native_particle_parameter_endpoints_00afc360(&a.builder,0.0f,0.0f,c.builders);
                            void* outer=suffix(&a.line,a.outer_suffix,2,0xb0b10c);a.unwind_state=8;
                            void* inner=suffix(outer,a.inner_suffix,1,0xb0b11f);a.unwind_state=9;
                            a.native_site=0xb0b12e;(void)parse_native_particle_parameter_00afc470(&a.builder,inner,c.builders);
                            clear(a.inner_suffix,8,0xb0b13c);clear(a.outer_suffix,7,0xb0b14a);
                            float argument;load_spill(&a.scalar,&argument);a.native_site=0xb0b163;
                            if (!load_native_particle_type_parameter_00b00980(definition,&a.name,&a.builder,argument,c.parameters)) {
                                for (const auto& p:parameters) if (_stricmp(text(&a.name),p.name)==0) {
                                    a.native_site=p.site;
                                    if (!p.runtime) store_first(&a.builder,static_cast<unsigned char*>(definition)+p.offset);
                                    else {
                                        void* runtime=convert_native_particle_parameter_00afbf60(&a.builder,c.parameters.parameters);
                                        if (p.offset!=0xe4) store_percentage(runtime,&a.scalar,c.parameters);
                                        field<void*>(definition,p.offset)=runtime;
                                    }
                                    break;
                                }
                            }
                            a.native_site=0xb0b559;destroy_native_particle_parameter_builder_00af4110(&a.builder,c.builders);
                            clear(a.name,0,0xb0b567);
                        }
                    }
                }
            }
            have_line=read_line(0xb0b575);
        }
        a.unwind_state=-1;a.native_site=0xb0b5b1;
        return_captured(a.line.data,strings); // final native header retains stale pointer
        a.phase=Phase::complete;return true;
    } catch (...) { a.phase=Phase::failed;unwind(a,c);throw; }
}
} // namespace bsp
