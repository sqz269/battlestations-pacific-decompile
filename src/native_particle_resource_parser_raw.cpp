#include "bsp/native_particle_resource_parser_raw.hpp"
#include "bsp/native_particle_emitter_factory_raw.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_text_helpers.hpp"
#include "bsp/native_particle_layer_lifetime.hpp"
#include "bsp/native_particle_layer_reader.hpp"
#include "bsp/native_particle_bounds_reader.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource parser requires MSVC Win32 x87.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Acquired = NativeParticleResourceParserRawAcquired;
using Context = NativeParticleResourceParserRawContext;
static_assert(sizeof(void*) == 4);
U word(const void* p) noexcept { return *static_cast<const volatile U*>(p); }
char* text(const void* p) noexcept { return reinterpret_cast<char*>(word(p)); }
void* pointer(U w) noexcept { return reinterpret_cast<void*>(w); }
void* at(void* p, U offset) noexcept { return pointer(reinterpret_cast<U>(p) + offset); }
void store(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p,offset)) = value; }
void return_captured(char* p, NativeStringRawPoolContext& strings) {
    if (p) release_native_pooled_text_bytes_00aee1e0(p, strings);
}
void build_string(U* out, const char* captured, NativeStringRawPoolContext& strings) {
    out[0] = 0; out[1] = 0;
    resize_native_string_header_0041dd40(out, strings,
        static_cast<U>(std::strlen(captured)), true);
    char* const destination = reinterpret_cast<char*>(word(out + 1));
    if (destination) std::memmove(destination, captured, word(out) + 1u);
}
__declspec(naked) void __cdecl atof_store(const char*, void*) {
    __asm {
        push dword ptr [esp+4]
        call atof
        add esp, 4
        mov eax, dword ptr [esp+8]
        fstp dword ptr [eax]
        ret
    }
}
__declspec(naked) void __cdecl discard_atof(const char*) {
    __asm {
        push dword ptr [esp+4]
        call atof
        fstp st(0)
        add esp, 4
        ret
    }
}
__declspec(naked) void __cdecl copy_float(const void*, void*) {
    __asm {
        mov ecx, dword ptr [esp+4]
        fld dword ptr [ecx]
        mov eax, dword ptr [esp+8]
        fstp dword ptr [eax]
        ret
    }
}
__declspec(naked) void __cdecl first_store(const void*, void*, void*) {
    __asm {
        push dword ptr [esp+4]
        call first_native_particle_parameter_value_00afc1b0
        add esp, 4
        mov ecx, dword ptr [esp+8]
        fstp dword ptr [ecx]
        fld dword ptr [ecx]
        mov eax, dword ptr [esp+12]
        fstp dword ptr [eax]
        ret
    }
}
// Keep AFC1B0's hardware ST0 live until BF7420 observes the actual feature
// cell. There is no intermediate binary32/double store or C++ integer cast.
__declspec(naked) std::int32_t __cdecl first_integer(const void*, const volatile U*) {
    __asm {
        push dword ptr [esp+4]
        call first_native_particle_parameter_value_00afc1b0
        add esp, 4
        mov ecx, dword ptr [esp+8]
        call native_crt_truncate_st0_00bf7420
        ret
    }
}
} // namespace

struct NativeParticleResourceParserRawAcquired::Impl {
    explicit Impl(std::int32_t kind) { builder.kind_0c = kind; }
    // EBP-8Ch..-20h, then the native EBP-1Ch builder. Keep these addresses
    // alive before the recursively retained emitter/parser frames.
    U locals[28];
    NativeParticleParameterBuilderStorage builder;
    std::optional<NativeParticleEmitterFactoryRawAcquired> emitter;
    U* slot(unsigned i) noexcept { return locals+i; }
    void unwind(Acquired& a, Context& c) noexcept {
        static constexpr int previous[]{-1,0,1,1,3,4,0,6,7,8,0,10,11,0,13};
        static constexpr unsigned slots[]{0,1,18,28,20,19,7,22,6,24,15,21,26,21,26};
        try {
            while (a.unwind_state >= 0) {
                const int state=a.unwind_state;
                a.unwind_state=previous[state];
                if (state==3) destroy_native_particle_parameter_builder_00af4110(&builder,c.builder);
                else if (state==10) singleton_lifetime_free(pointer(*slot(15)));
                else if (state==11 || state==13) {
                    if (*slot(2)&1u) {
                        *slot(2)&=~1u;
                        destroy_native_pooled_text_00aee2a0(slot(21),c.builder.strings);
                    }
                } else if (state==12 || state==14) {
                    if (*slot(2)&2u) {
                        *slot(2)&=~2u;
                        destroy_native_string_header_0041dd20(slot(26),c.builder.strings);
                    }
                } else if (state==7 || state==9)
                    destroy_native_string_header_0041dd20(slot(slots[state]),c.builder.strings);
                else destroy_native_pooled_text_00aee2a0(slot(slots[state]),c.builder.strings);
            }
        } catch (...) { std::terminate(); }
    }
};
NativeParticleResourceParserRawAcquired::NativeParticleResourceParserRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleResourceParserRawAcquired::~NativeParticleResourceParserRawAcquired() = default;
NativeParticleEmitterFactoryRawAcquired* NativeParticleResourceParserRawAcquired::emitter_child() noexcept {
    return impl_->emitter ? &*impl_->emitter : nullptr;
}

bool parse_native_particle_resource_00af4ba0(void* resource, void* buffer, Context& c, Acquired& a) {
    using Phase=Acquired::Phase;
    if (a.phase!=Phase::fresh) throw std::logic_error("Particle resource parser cannot replay");
    a.phase=Phase::running;
    auto& f=*a.impl_;
    auto& strings=c.builder.strings;
    auto slot=[&](unsigned i) { return f.slot(i); };
    auto give=[&](unsigned i) { return_captured(text(slot(i)),strings); *slot(i)=0; };
    auto clear=[&](unsigned i) { destroy_native_pooled_text_00aee2a0(slot(i),strings); };
    auto token=[&](unsigned i,int n,U site) {
        a.native_site=site;
        return get_native_pooled_text_token_00aee3c0(slot(0),slot(i),n,strings);
    };
    auto line_read=[&](U site) {
        a.native_site=site;
        return read_native_text_buffer_line_00af5740(buffer,slot(0),strings,c.actual_text_scratch_00f8c2c8);
    };
    auto finish=[&](bool result) {
        char* captured=text(slot(0)); a.unwind_state=-1;
        return_captured(captured,strings);
        a.phase=Phase::complete;
        return result;
    };
    *slot(2)=0;
    try {
        a.native_site=0x00af4bcfu;
        rewind_native_text_buffer_00af55f0(buffer);
        *slot(0)=0; a.unwind_state=0;
        if (!line_read(0x00af4be6u)) return finish(false);
        const bool system=_stricmp(text(token(8,0,0x00af4c37u)),"ParticleSystem")==0;
        return_captured(text(slot(8)),strings);
        if (!system) return finish(false);
        while (line_read(0x00af4ccdu)) {
            if (_stricmp(text(slot(0)),"{")==0) break;
        }
        bool available=line_read(0x00af4cf3u);
        while (available && _stricmp(text(slot(0)),"}")!=0) {
            if (_stricmp(text(slot(0)),"")!=0) {
                const bool parameter=_stricmp(text(token(3,0,0x00af4d3eu)),"Param")==0;
                give(3);
                if (parameter) {
                    token(1,1,0x00af4d9au); a.unwind_state=1;
                    if (_stricmp(text(slot(1)),"LocalSpace")==0) {
                        void* t=token(4,2,0x00af4dccu);
                        a.native_site=0x00af4dd4u;
                        *static_cast<volatile unsigned char*>(at(resource,0x64))=std::atol(text(t))>0;
                        give(4);
                        char* captured=text(slot(1)); a.unwind_state=0;
                        return_captured(captured,strings); *slot(1)=0;
                    } else {
                        struct Direct { const char* name; U offset; unsigned slot; U token_site; U parse_site; bool number; };
                        static constexpr Direct direct[]{
                            {"FakeLocalSpace",0x65,9,0x00af4e70,0x00af4e78,false},
                            {"UnderWater",0x70,10,0x00af4ec8,0x00af4ed0,false},
                            {"ColorBurn",0x74,11,0x00af4f20,0x00af4f27,true},
                            {"PerPixelNormal",0x78,12,0x00af4f6f,0x00af4f77,false},
                            {"SoftParticle",0x79,13,0x00af4fcb,0x00af4fd3,false},
                            {"HasReflection",0x7a,14,0x00af5023,0x00af502b,false},
                            {"MaxParticleSize",0x60,16,0x00af507b,0x00af5082,true}};
                        const Direct* selected=nullptr;
                        for (const auto& d:direct) if (_stricmp(text(slot(1)),d.name)==0) { selected=&d; break; }
                        if (selected) {
                            void* t=token(selected->slot,2,selected->token_site);
                            a.native_site=selected->parse_site;
                            if (selected->number) {
                                if (selected->offset==0x60) { atof_store(text(t),slot(15)); copy_float(slot(15),at(resource,0x60)); }
                                else atof_store(text(t),at(resource,selected->offset));
                            } else *static_cast<volatile unsigned char*>(at(resource,selected->offset))=std::atol(text(t))>0;
                            clear(selected->slot);
                            if (selected->offset==0x78) *static_cast<volatile unsigned char*>(at(resource,0x78))=0;
                            a.unwind_state=0; clear(1);
                        } else {
                            a.native_site=0x00af50b6u;
                            void* suffix=get_native_pooled_text_suffix_00af44c0(slot(0),slot(18),2,strings);
                            a.unwind_state=2; a.native_site=0x00af50cbu;
                            void* number=get_native_pooled_text_token_00aee3c0(suffix,slot(17),0,strings);
                            a.native_site=0x00af50d2u; discard_atof(text(number)); clear(17);
                            a.unwind_state=1; clear(18);
                            a.native_site=0x00af50fau;
                            construct_native_particle_parameter_builder_00afbed0(&f.builder,c.builder);
                            a.unwind_state=3; a.native_site=0x00af511au;
                            initialize_native_particle_parameter_endpoints_00afc360(&f.builder,0.0f,0.0f,c.builder);
                            a.native_site=0x00af512au;
                            suffix=get_native_pooled_text_suffix_00af44c0(slot(0),slot(20),2,strings);
                            a.unwind_state=4; a.native_site=0x00af5140u;
                            void* curve=get_native_pooled_text_suffix_00af44c0(suffix,slot(19),1,strings);
                            a.unwind_state=5; a.native_site=0x00af5155u;
                            (void)parse_native_particle_parameter_00afc470(&f.builder,curve,c.builder);
                            a.unwind_state=4; clear(19); a.unwind_state=3; clear(20);
                            if (_stricmp(text(slot(1)),"MaxEmitters")==0) {
                                a.native_site=0x00af519eu;
                                store(resource,0x58,static_cast<U>(first_integer(&f.builder,c.actual_feature_word_0109eea4)));
                            } else if (_stricmp(text(slot(1)),"FrameRate")==0) {
                                a.native_site=0x00af51c8u;
                                store(resource,0x5c,static_cast<U>(first_native_particle_parameter_integer_00afc1c0(&f.builder)));
                            } else if (_stricmp(text(slot(1)),"LODInnerDistance")==0) {
                                a.native_site=0x00af51efu; first_store(&f.builder,slot(15),at(resource,0x68));
                            } else if (_stricmp(text(slot(1)),"LODOuterDistance")==0) {
                                a.native_site=0x00af521eu; first_store(&f.builder,slot(15),at(resource,0x6c));
                            }
                            a.native_site=0x00af5235u;
                            destroy_native_particle_parameter_builder_00af4110(&f.builder,c.builder);
                            a.unwind_state=0; clear(1);
                        }
                    }
                } else {
                    const bool emitter=_stricmp(text(token(5,0,0x00af525au)),"Emitter")==0;
                    give(5);
                    if (emitter) {
                        void* t=token(7,1,0x00af52b6u);
                        const char* captured=text(t); a.unwind_state=6; a.native_site=0x00af52cau;
                        build_string(slot(22),captured,strings); a.unwind_state=7;
                        a.native_site=0x00af52dbu;
                        const int count=count_native_particle_token_fields_00af3e90(slot(0));
                        t=token(6,count-1,0x00af52edu);
                        captured=text(t); a.unwind_state=8; a.native_site=0x00af5301u;
                        build_string(slot(24),captured,strings); a.unwind_state=9;
                        f.emitter.emplace(c.child_builder_kind); a.native_site=0x00af5320u;
                        void* child=create_native_particle_definition_00af9fb0(slot(24),slot(22),
                            reinterpret_cast<U>(resource),0,buffer,c.emitters,*f.emitter);
                        a.unwind_state=8; destroy_native_string_header_0041dd20(slot(24),strings);
                        char* kind=text(slot(6)); a.unwind_state=7; return_captured(kind,strings); *slot(6)=0;
                        a.unwind_state=6; destroy_native_string_header_0041dd20(slot(22),strings);
                        char* name=text(slot(7)); a.unwind_state=0; return_captured(name,strings); *slot(7)=0;
                        if (child) {
                            const U count_now=word(at(resource,0x30));
                            store(resource,0x10+4u*count_now,reinterpret_cast<U>(child));
                            store(resource,0x30,word(at(resource,0x30))+1u);
                        }
                    } else {
                        const bool layer=_stricmp(text(token(8,0,0x00af5401u)),"Layer")==0;
                        give(8);
                        if (layer) {
                            a.native_site=0x00af5454u;
                            void* child=singleton_lifetime_allocate({SingletonAllocationKind::object,0x40,0x40});
                            *slot(15)=reinterpret_cast<U>(child); a.unwind_state=10;
                            if (child) {
                                void* t=token(21,1,0x00af5479u);
                                const char* captured=text(t); *slot(2)|=1u; a.unwind_state=11;
                                a.native_site=0x00af5492u; build_string(slot(26),captured,strings);
                                *slot(2)|=2u; a.unwind_state=12; a.native_site=0x00af54abu;
                                child=construct_native_particle_layer_00afab90(child,slot(26),reinterpret_cast<U>(resource),strings);
                            }
                            U mask=*slot(2); a.unwind_state=13;
                            if (mask&2u) {
                                mask&=~2u; *slot(2)=mask;
                                destroy_native_string_header_0041dd20(slot(26),strings);
                            }
                            a.unwind_state=0;
                            if (mask&1u) { *slot(2)=mask&~1u; clear(21); }
                            a.native_site=0x00af5516u;
                            (void)read_native_particle_layer_00afad00(child,buffer,c.builder,c.actual_text_scratch_00f8c2c8);
                            if (child) {
                                const U count_now=word(at(resource,0x54));
                                store(resource,0x34+4u*count_now,reinterpret_cast<U>(child));
                                store(resource,0x54,word(at(resource,0x54))+1u);
                            }
                        }
                    }
                }
            }
            available=line_read(0x00af5536u);
        }
        a.native_site=0x00af554fu;
        read_native_particle_bounds_00af4700(resource,buffer,strings,c.actual_text_scratch_00f8c2c8);
        a.native_site=0x00af5556u;
        prepare_native_particle_variant_00af40e0(resource,strings);
        return finish(true);
    } catch (...) {
        a.native_state_at_failure=a.unwind_state;
        a.phase=Phase::failed;
        f.unwind(a,c);
        throw;
    }
}
} // namespace bsp
