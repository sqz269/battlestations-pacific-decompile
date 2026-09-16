#include "bsp/native_particle_smartarea_raw.hpp"

#include "bsp/native_particle_definition_loading.hpp"
#include "bsp/native_particle_emitter_factory_raw.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_type_factory_raw.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SmartArea parsing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
struct String8 { Word length; char* data; };
static_assert(sizeof(String8) == 8);

template<class T> T& field(void* p, Word offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
const char* text(const void* header) noexcept {
    return *static_cast<char* const volatile*>(header);
}
void return_captured(char* captured, NativeStringRawPoolContext& strings) {
    if (captured) release_native_pooled_text_bytes_00aee1e0(captured, strings);
}
void clear_inline(NativePooledTextStorage& header, NativeStringRawPoolContext& strings) {
    return_captured(header.data, strings);
    header.data = nullptr;
}

// Four inlined 0041E870-shaped constructions: initialize before strlen,
// preserve on resize, then use CURRENT pointer and CURRENT length+1 to copy.
void make_string(String8& header, const char* captured, NativeStringRawPoolContext& strings) {
    header.length = 0;
    header.data = nullptr;
    const Word length = static_cast<Word>(std::strlen(captured));
    resize_native_string_header_0041dd40(&header, strings, length, true);
    char* const destination = header.data;
    if (destination) std::memmove(destination, captured, header.length + 1u);
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
void spill_scalar(const float* input, float* output) {
    __asm { mov eax, input
        fld dword ptr[eax]
        mov eax, output
        fstp dword ptr[eax] }
}
void store_percentage(void* parameter, const float* scalar,
    NativeParticleSmartAreaRawContext& context) {
    auto* scale_cell = &context.percentage_scale_00d7a358;
    __asm { mov eax, scalar
        fld dword ptr[eax]
        mov ecx, scale_cell
        mov ecx, [ecx]
        fmul qword ptr[ecx]
        mov eax, parameter
        fstp dword ptr[eax] }
}
void free_builder_inline(NativeParticleParameterBuilderStorage& builder) noexcept {
    singleton_lifetime_free(builder.records_00);
    builder.records_00 = nullptr;
    builder.count_04 = 0;
    builder.capacity_08 = 0;
}
} // namespace

struct NativeParticleSmartAreaRawAcquired::Impl {
    explicit Impl(std::int32_t kind) { builder.kind_0c = kind; }
    NativePooledTextStorage line, parameter_name;
    float scalar;
    NativePooledTextStorage keyword, flag_suffix, scalar_token, scalar_suffix,
        curve_inner, curve_outer, emitter_keyword, emitter_kind_token,
        emitter_name_token, particle_keyword, particle_kind_token, particle_name_token;
    String8 emitter_kind, emitter_name, particle_kind, particle_name;
    NativeParticleParameterBuilderStorage builder;
    // Persistent native headers precede children that borrowed their addresses.
    std::optional<NativeParticleEmitterFactoryRawAcquired> emitter;
    std::optional<NativeParticleTypeFactoryRawAcquired> particle;

    void unwind(int& state, NativeParticleSmartAreaRawContext& c) noexcept {
        static constexpr int previous[]{-1,0,0,2,2,4,5,0,7,8,9,0,11,12,13};
        try {
            while (state >= 0) {
                const int active = state;
                state = previous[active];
                void* header = nullptr;
                switch (active) {
                case 0: header = &line; break;
                case 1: header = &flag_suffix; break;
                case 2: header = &parameter_name; break;
                case 3: header = &scalar_suffix; break;
                case 4:
                    destroy_native_particle_parameter_builder_00af4110(&builder, c.builder);
                    continue;
                case 5: header = &curve_outer; break;
                case 6: header = &curve_inner; break;
                case 7: header = &emitter_name_token; break;
                case 8: destroy_native_string_header_0041dd20(&emitter_name, c.builder.strings); continue;
                case 9: header = &emitter_kind_token; break;
                case 10: destroy_native_string_header_0041dd20(&emitter_kind, c.builder.strings); continue;
                case 11: header = &particle_name_token; break;
                case 12: destroy_native_string_header_0041dd20(&particle_name, c.builder.strings); continue;
                case 13: header = &particle_kind_token; break;
                case 14: destroy_native_string_header_0041dd20(&particle_kind, c.builder.strings); continue;
                }
                destroy_native_pooled_text_00aee2a0(header, c.builder.strings);
            }
        } catch (...) { std::terminate(); }
    }
};

NativeParticleSmartAreaRawAcquired::NativeParticleSmartAreaRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleSmartAreaRawAcquired::~NativeParticleSmartAreaRawAcquired() = default;
NativeParticleEmitterFactoryRawAcquired* NativeParticleSmartAreaRawAcquired::emitter_child() noexcept {
    return impl_->emitter ? &*impl_->emitter : nullptr;
}
NativeParticleTypeFactoryRawAcquired* NativeParticleSmartAreaRawAcquired::particle_child() noexcept {
    return impl_->particle ? &*impl_->particle : nullptr;
}

bool load_native_smartarea_emitter_definition_00b02210(void* definition, void* buffer,
    NativeParticleSmartAreaRawContext& c, NativeParticleSmartAreaRawAcquired& a) {
    using Phase = NativeParticleSmartAreaRawAcquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("SmartArea parser cannot replay");
    a.phase = Phase::running;
    auto& f = *a.impl_;
    auto& strings = c.builder.strings;
    auto token = [&](const void* input, NativePooledTextStorage& output, int index, Word site) {
        a.native_site = site;
        return get_native_pooled_text_token_00aee3c0(input, &output, index, strings);
    };
    auto suffix = [&](const void* input, NativePooledTextStorage& output, int index, Word site) {
        a.native_site = site;
        return get_native_pooled_text_suffix_00af44c0(input, &output, index, strings);
    };
    auto clear = [&](NativePooledTextStorage& header, int state, Word site, bool inlined = true) {
        a.unwind_state = state;
        a.native_site = site;
        if (inlined) clear_inline(header, strings);
        else destroy_native_pooled_text_00aee2a0(&header, strings);
    };
    auto release_string = [&](String8& header, int state, Word site) {
        a.unwind_state = state;
        a.native_site = site;
        destroy_native_string_header_0041dd20(&header, strings);
    };
    auto read_line = [&](Word site) {
        a.native_site = site;
        return read_native_text_buffer_line_00af5740(buffer, &f.line, strings,
            c.actual_text_scratch_00f8c2c8);
    };
    try {
        f.line.data = nullptr;
        a.unwind_state = 0;
        while (read_line(0x00b0224c))
            if (_stricmp(text(&f.line), "{") == 0) break;
        bool have_line = read_line(0x00b02272);
        while (have_line) {
            if (_stricmp(text(&f.line), "}") == 0) break;
            if (_stricmp(text(&f.line), "") != 0) {
                const void* keyword = token(&f.line, f.keyword, 0, 0x00b022be);
                const bool is_parameter = _stricmp(text(keyword), "Param") == 0;
                clear(f.keyword, 0, 0x00b022f7);
                if (is_parameter) {
                    void* flag = suffix(&f.line, f.flag_suffix, 1, 0x00b0231a);
                    a.unwind_state = 1;
                    a.native_site = 0x00b0232a;
                    const bool handled_flag = load_native_particle_definition_flag_00afa650(
                        definition, flag, strings);
                    clear(f.flag_suffix, 0, 0x00b02358);
                    if (!handled_flag) {
                        token(&f.line, f.parameter_name, 1, 0x00b0237b);
                        a.unwind_state = 2;
                        void* initial = suffix(&f.line, f.scalar_suffix, 2, 0x00b02393);
                        a.unwind_state = 3;
                        const void* value = token(initial, f.scalar_token, 0, 0x00b023a8);
                        a.native_site = 0x00b023b0;
                        parse_scalar(text(value), &f.scalar);
                        clear(f.scalar_token, 3, 0x00b023e2);
                        clear(f.scalar_suffix, 2, 0x00b02419);
                        a.native_site = 0x00b0242d;
                        construct_native_particle_parameter_builder_00afbed0(&f.builder, c.builder);
                        a.unwind_state = 4;
                        a.native_site = 0x00b0244a;
                        initialize_native_particle_parameter_endpoints_00afc360(&f.builder, 0.0f, 0.0f, c.builder);
                        void* outer = suffix(&f.line, f.curve_outer, 2, 0x00b0245a);
                        a.unwind_state = 5;
                        void* inner = suffix(outer, f.curve_inner, 1, 0x00b02470);
                        a.unwind_state = 6;
                        a.native_site = 0x00b02482;
                        (void)parse_native_particle_parameter_00afc470(&f.builder, inner, c.builder);
                        clear(f.curve_inner, 5, 0x00b024b2);
                        clear(f.curve_outer, 4, 0x00b024e9);
                        float argument;
                        spill_scalar(&f.scalar, &argument);
                        a.native_site = 0x00b0250d;
                        const bool handled_base = load_native_particle_base_parameter_00af9d00(
                            definition, &f.parameter_name, &f.builder, argument,
                            c.parameters, c.percentage_scale_00d7a358);
                        Word offset = 0;
                        Word conversion_site = 0;
                        if (!handled_base) {
                            if (_stricmp(text(&f.parameter_name), "EmittedSpeed") == 0) {
                                offset = 0x80; conversion_site = 0x00b0256d;
                            } else if (_stricmp(text(&f.parameter_name), "Radius") == 0) {
                                offset = 0x84; conversion_site = 0x00b025f3;
                            } else if (_stricmp(text(&f.parameter_name), "RadiusSpeed") == 0) {
                                offset = 0x88; conversion_site = 0x00b0262b;
                            } else if (_stricmp(text(&f.parameter_name), "RadiusAngle") == 0) {
                                offset = 0x8c; conversion_site = 0x00b0268b;
                            }
                            if (offset) {
                                a.native_site = conversion_site;
                                void* parameter = convert_native_particle_parameter_00afbf60(&f.builder, c.parameters);
                                store_percentage(parameter, &f.scalar, c);
                                field<void*>(definition, offset) = parameter;
                            }
                        }
                        if (handled_base || offset == 0x80 || offset == 0x84 || offset == 0x88) {
                            a.native_site = handled_base ? 0x00b0251b
                                : offset == 0x80 ? 0x00b02589 : 0x00b02647;
                            free_builder_inline(f.builder);
                        } else {
                            a.native_site = 0x00b026a6;
                            destroy_native_particle_parameter_builder_00af4110(&f.builder, c.builder);
                        }
                        if (handled_base || offset == 0x80)
                            clear(f.parameter_name, 0, 0x00b025c4);
                        else
                            clear(f.parameter_name, 0, offset == 0x84 || offset == 0x88
                                ? 0x00b02667 : 0x00b026b7, false);
                    }
                } else {
                    const void* emitter_keyword = token(&f.line, f.emitter_keyword, 0, 0x00b026cb);
                    const bool is_emitter = _stricmp(text(emitter_keyword), "Emitter") == 0;
                    clear(f.emitter_keyword, 0, 0x00b02704);
                    if (is_emitter) {
                        const void* name = token(&f.line, f.emitter_name_token, 1, 0x00b02727);
                        const char* const name_bytes = text(name);
                        a.unwind_state = 7;
                        a.native_site = 0x00b02755;
                        make_string(f.emitter_name, name_bytes, strings);
                        a.unwind_state = 8;
                        const void* kind = token(&f.line, f.emitter_kind_token, 2, 0x00b02787);
                        const char* const kind_bytes = text(kind);
                        a.unwind_state = 9;
                        a.native_site = 0x00b027b5;
                        make_string(f.emitter_kind, kind_bytes, strings);
                        a.native_site = 0x00b027df;
                        const Word word10 = field<volatile Word>(definition, 0x10);
                        a.unwind_state = 10;
                        a.native_site = 0x00b027f5;
                        if (!c.emitters) throw std::invalid_argument("Missing concrete SmartArea emitter factory context");
                        if (f.emitter && f.emitter->phase != NativeParticleEmitterFactoryRawAcquired::Phase::complete)
                            throw std::logic_error("Unfinished SmartArea emitter child cannot be replaced");
                        f.emitter.emplace(c.child_builder_kind);
                        void* child = create_native_particle_definition_00af9fb0(&f.emitter_kind,
                            &f.emitter_name, word10, reinterpret_cast<Word>(definition), buffer,
                            *c.emitters, *f.emitter);
                        release_string(f.emitter_kind, 9, 0x00b02817);
                        clear(f.emitter_kind_token, 8, 0x00b0284a);
                        release_string(f.emitter_name, 7, 0x00b02875);
                        clear(f.emitter_name_token, 0, 0x00b028a8);
                        a.native_site = 0x00b028bb;
                        publish_native_particle_emitter_member_00af9f00(definition, child);
                    }
                    // Native code retokenizes Particle even after Emitter success.
                    const void* particle_keyword = token(&f.line, f.particle_keyword, 0, 0x00b028cc);
                    const bool is_particle = _stricmp(text(particle_keyword), "Particle") == 0;
                    clear(f.particle_keyword, 0, 0x00b02905);
                    if (is_particle) {
                        const void* name = token(&f.line, f.particle_name_token, 1, 0x00b02928);
                        const char* const name_bytes = text(name);
                        a.unwind_state = 11;
                        a.native_site = 0x00b02956;
                        make_string(f.particle_name, name_bytes, strings);
                        a.unwind_state = 12;
                        const void* kind = token(&f.line, f.particle_kind_token, 2, 0x00b02988);
                        const char* const kind_bytes = text(kind);
                        a.unwind_state = 13;
                        a.native_site = 0x00b029b6;
                        make_string(f.particle_kind, kind_bytes, strings);
                        a.unwind_state = 14;
                        a.native_site = 0x00b029f2;
                        if (!c.particles) throw std::invalid_argument("Missing concrete SmartArea particle factory context");
                        if (f.particle && f.particle->phase != NativeParticleTypeFactoryRawAcquired::Phase::complete)
                            throw std::logic_error("Unfinished SmartArea particle child cannot be replaced");
                        f.particle.emplace(c.child_builder_kind);
                        void* child = create_native_particle_type_definition_00b00ce0(&f.particle_kind,
                            &f.particle_name, definition, buffer, *c.particles, *f.particle);
                        release_string(f.particle_kind, 13, 0x00b02a14);
                        clear(f.particle_kind_token, 12, 0x00b02a47);
                        release_string(f.particle_name, 11, 0x00b02a72);
                        clear(f.particle_name_token, 0, 0x00b02aa5);
                        a.native_site = 0x00b02ab8;
                        (void)publish_native_particle_particle_member_00af9f20(definition, child);
                    }
                }
            }
            have_line = read_line(0x00b02acb);
        }
        a.unwind_state = -1;
        a.native_site = 0x00b02b03;
        return_captured(f.line.data, strings); // No native final null store.
        a.native_site = 0x00b02b22;
        a.phase = Phase::complete;
        return true;
    } catch (...) {
        a.native_state_at_failure = a.unwind_state;
        f.unwind(a.unwind_state, c);
        a.phase = Phase::failed;
        throw;
    }
}
} // namespace bsp
