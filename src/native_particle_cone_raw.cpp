#include "bsp/native_particle_cone_raw.hpp"
#include "bsp/native_particle_definition_loading.hpp"
#include "bsp/native_particle_emitter_factory_raw.hpp"
#include "bsp/native_particle_type_factory_raw.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Cone emitter parser requires MSVC Win32 x87.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Acquired = NativeParticleConeRawAcquired;
using Context = NativeParticleConeRawContext;
static_assert(sizeof(void*) == 4);
U word(const void* p) noexcept { return *static_cast<const volatile U*>(p); }
char* text(const void* p) noexcept { return reinterpret_cast<char*>(word(p)); }
struct Header { U length, data; };
void return_captured(char* p, NativeStringRawPoolContext& strings) {
    if (p) release_native_pooled_text_bytes_00aee1e0(p, strings);
}
void construct_header(Header& h, const char* captured, NativeStringRawPoolContext& strings) {
    h.length = 0; h.data = 0;
    resize_native_string_header_0041dd40(&h, strings, static_cast<U>(std::strlen(captured)), true);
    void* const target = reinterpret_cast<void*>(word(&h.data));
    if (target) std::memmove(target, captured, word(&h.length) + 1u);
}
__declspec(naked) void __cdecl atof_store(const char*, U*) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+8]
        call atof
        mov ecx, dword ptr [ebp+12]
        fstp dword ptr [ecx]
        add esp, 4
        pop ebp
        ret
    }
}
__declspec(naked) void __cdecl copy_float_store(float*, const U*) {
    __asm {
        mov ecx, dword ptr [esp+8]
        fld dword ptr [ecx]
        mov eax, dword ptr [esp+4]
        fstp dword ptr [eax]
        ret
    }
}
__declspec(naked) void __cdecl scale_parameter(void*, const U*, const volatile double*) {
    __asm {
        mov ecx, dword ptr [esp+8]
        fld dword ptr [ecx]
        mov edx, dword ptr [esp+12]
        fmul qword ptr [edx]
        mov eax, dword ptr [esp+4]
        fstp dword ptr [eax]
        ret
    }
}
} // namespace

struct NativeParticleConeRawAcquired::Impl {
    // Actual native frame storage, in declaration order before retained children.
    // Only line and the explicit builder kind are initialized before production.
    U line, name, percentage_bits, command, flag_suffix, percentage_token;
    U percentage_suffix, curve, curve_suffix, emitter_command, emitter_kind_token;
    U emitter_name_token, particle_command, particle_kind_token, particle_name_token;
    Header emitter_kind, emitter_name, particle_kind, particle_name;
    NativeParticleParameterBuilderStorage builder;
    std::optional<NativeParticleEmitterFactoryRawAcquired> emitter;
    std::optional<NativeParticleTypeFactoryRawAcquired> particle;
    explicit Impl(std::int32_t kind) { builder.kind_0c = kind; }
    void unwind(Acquired& a, Context& c) noexcept {
        static constexpr int previous[]{-1,0,0,2,2,4,5,0,7,8,9,0,11,12,13};
        try {
            while (a.unwind_state >= 0) {
                const int state = a.unwind_state;
                a.unwind_state = previous[state];
                if (state == 4) destroy_native_particle_parameter_builder_00af4110(&builder, c.builder);
                else if (state == 8 || state == 10 || state == 12 || state == 14) {
                    Header* const h = state == 8 ? &emitter_name : state == 10 ? &emitter_kind
                        : state == 12 ? &particle_name : &particle_kind;
                    destroy_native_string_header_0041dd20(h, c.builder.strings);
                } else {
                    U* const h = state == 0 ? &line : state == 1 ? &flag_suffix
                        : state == 2 ? &name : state == 3 ? &percentage_suffix
                        : state == 5 ? &curve_suffix : state == 6 ? &curve
                        : state == 7 ? &emitter_name_token : state == 9 ? &emitter_kind_token
                        : state == 11 ? &particle_name_token : &particle_kind_token;
                    destroy_native_pooled_text_00aee2a0(h, c.builder.strings);
                }
            }
        } catch (...) { std::terminate(); }
    }
};
NativeParticleConeRawAcquired::NativeParticleConeRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleConeRawAcquired::~NativeParticleConeRawAcquired() = default;
NativeParticleEmitterFactoryRawAcquired* NativeParticleConeRawAcquired::emitter_child() noexcept {
    return impl_->emitter ? &*impl_->emitter : nullptr;
}
NativeParticleTypeFactoryRawAcquired* NativeParticleConeRawAcquired::particle_child() noexcept {
    return impl_->particle ? &*impl_->particle : nullptr;
}

bool load_native_cone_emitter_definition_00b03ec0(void* definition, void* buffer,
    Context& c, Acquired& a) {
    using Phase = Acquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Cone emitter parser operation cannot replay");
    a.phase = Phase::running;
    auto& f = *a.impl_;
    auto& strings = c.builder.strings;
    const volatile double* const scale = c.percentage_scale_00d7a358;
    f.line = 0; a.unwind_state = 0;
    auto read = [&](U site) {
        a.native_site = site;
        return read_native_text_buffer_line_00af5740(buffer, &f.line, strings, c.actual_text_scratch_00f8c2c8);
    };
    auto token_is = [&](U& header, const char* expected, U site) {
        a.native_site = site;
        void* const h = get_native_pooled_text_token_00aee3c0(&f.line, &header, 0, strings);
        const bool result = _stricmp(text(h), expected) == 0;
        return_captured(text(&header), strings); header = 0;
        return result;
    };
    try {
        while (read(0x00b03efcu)) if (_stricmp(text(&f.line), "{") == 0) break;
        bool have_line = read(0x00b03f22u);
        while (have_line) {
            if (_stricmp(text(&f.line), "}") == 0) break;
            if (_stricmp(text(&f.line), "") != 0) {
                bool consumed = false;
                if (token_is(f.command, "Param", 0x00b03f6eu)) {
                    a.native_site = 0x00b03fcau;
                    void* const suffix = get_native_pooled_text_suffix_00af44c0(&f.line, &f.flag_suffix, 1, strings);
                    a.unwind_state = 1; a.native_site = 0x00b03fdau;
                    consumed = load_native_particle_definition_flag_00afa650(definition, suffix, strings);
                    char* const captured_flag = text(&f.flag_suffix);
                    a.unwind_state = 0;
                    return_captured(captured_flag, strings); f.flag_suffix = 0;
                    if (!consumed) {
                        a.native_site = 0x00b0402bu;
                        get_native_pooled_text_token_00aee3c0(&f.line, &f.name, 1, strings);
                        a.unwind_state = 2; a.native_site = 0x00b04043u;
                        void* const percent = get_native_pooled_text_suffix_00af44c0(&f.line, &f.percentage_suffix, 2, strings);
                        a.unwind_state = 3; a.native_site = 0x00b04058u;
                        void* const scalar = get_native_pooled_text_token_00aee3c0(percent, &f.percentage_token, 0, strings);
                        a.native_site = 0x00b04060u;
                        atof_store(text(scalar), &f.percentage_bits);
                        return_captured(text(&f.percentage_token), strings); f.percentage_token = 0;
                        char* const captured_percent = text(&f.percentage_suffix);
                        a.unwind_state = 2;
                        return_captured(captured_percent, strings); f.percentage_suffix = 0;
                        a.native_site = 0x00b040ddu;
                        construct_native_particle_parameter_builder_00afbed0(&f.builder, c.builder);
                        a.unwind_state = 4; a.native_site = 0x00b040fau;
                        initialize_native_particle_parameter_endpoints_00afc360(&f.builder, 0.0f, 0.0f, c.builder);
                        a.native_site = 0x00b0410au;
                        void* const curves = get_native_pooled_text_suffix_00af44c0(&f.line, &f.curve_suffix, 2, strings);
                        a.unwind_state = 5; a.native_site = 0x00b04120u;
                        void* const curve = get_native_pooled_text_suffix_00af44c0(curves, &f.curve, 1, strings);
                        a.unwind_state = 6; a.native_site = 0x00b04132u;
                        parse_native_particle_parameter_00afc470(&f.builder, curve, c.builder);
                        char* const captured_curve = text(&f.curve);
                        a.unwind_state = 5;
                        return_captured(captured_curve, strings); f.curve = 0;
                        char* const captured_curves = text(&f.curve_suffix);
                        a.unwind_state = 4;
                        return_captured(captured_curves, strings); f.curve_suffix = 0;
                        float scalar_argument;
                        copy_float_store(&scalar_argument, &f.percentage_bits);
                        a.native_site = 0x00b041bdu;
                        const bool common = load_native_particle_base_parameter_00af9d00(
                            definition, &f.name, &f.builder, scalar_argument, c.parameters, scale);
                        U offset = 0;
                        if (!common) {
                            if (_stricmp(text(&f.name), "InnerEmitSpeed") == 0) { offset = 0x80; a.native_site = 0x00b0421du; }
                            else if (_stricmp(text(&f.name), "OuterEmitSpeed") == 0) { offset = 0x84; a.native_site = 0x00b042a3u; }
                            else if (_stricmp(text(&f.name), "MaxAngle") == 0) { offset = 0x88; a.native_site = 0x00b042dbu; }
                            else if (_stricmp(text(&f.name), "InnerDistance") == 0) { offset = 0x8c; a.native_site = 0x00b0433bu; }
                            else if (_stricmp(text(&f.name), "OuterDistance") == 0) { offset = 0x90; a.native_site = 0x00b0436eu; }
                            if (offset) {
                                void* const parameter = convert_native_particle_parameter_00afbf60(&f.builder, c.parameters);
                                scale_parameter(parameter, &f.percentage_bits, scale);
                                *reinterpret_cast<volatile U*>(reinterpret_cast<U>(definition) + offset) = reinterpret_cast<U>(parameter);
                            }
                        }
                        consumed = common || offset != 0;
                        if (common || (offset && offset <= 0x88)) {
                            std::free(f.builder.records_00);
                            // Common/Inner capture the name before clearing the builder.
                            char* const captured_name = (common || offset == 0x80) ? text(&f.name) : nullptr;
                            f.builder.records_00 = nullptr; f.builder.count_04 = 0; f.builder.capacity_08 = 0;
                            a.unwind_state = 0;
                            if (common || offset == 0x80) { return_captured(captured_name, strings); f.name = 0; }
                            else destroy_native_pooled_text_00aee2a0(&f.name, strings);
                        } else {
                            a.native_site = offset ? 0x00b04389u : 0x00b043a4u;
                            destroy_native_particle_parameter_builder_00af4110(&f.builder, c.builder);
                            a.unwind_state = 0;
                            destroy_native_pooled_text_00aee2a0(&f.name, strings);
                        }
                    }
                }
                if (!consumed) {
                    if (token_is(f.emitter_command, "Emitter", 0x00b043c4u)) {
                        a.native_site = 0x00b04425u;
                        void* const name = get_native_pooled_text_token_00aee3c0(&f.line, &f.emitter_name_token, 1, strings);
                        const char* const captured_name = text(name);
                        a.unwind_state = 7; a.native_site = 0x00b04453u;
                        construct_header(f.emitter_name, captured_name, strings);
                        a.unwind_state = 8; a.native_site = 0x00b04485u;
                        void* const kind = get_native_pooled_text_token_00aee3c0(&f.line, &f.emitter_kind_token, 2, strings);
                        const char* const captured_kind = text(kind);
                        a.unwind_state = 9; a.native_site = 0x00b044b3u;
                        construct_header(f.emitter_kind, captured_kind, strings);
                        const U owner_word10 = word(reinterpret_cast<const void*>(reinterpret_cast<U>(definition) + 0x10u));
                        a.unwind_state = 10; a.native_site = 0x00b044f3u;
                        if (!c.emitters) throw std::invalid_argument("Cone emitter parser requires reached emitter factory context");
                        f.emitter.emplace(c.child_builder_kind);
                        void* const child = create_native_particle_definition_00af9fb0(&f.emitter_kind, &f.emitter_name,
                            owner_word10, reinterpret_cast<U>(definition), buffer, *c.emitters, *f.emitter);
                        a.unwind_state = 9;
                        destroy_native_string_header_0041dd20(&f.emitter_kind, strings);
                        char* const kind_text = text(&f.emitter_kind_token);
                        a.unwind_state = 8;
                        return_captured(kind_text, strings); f.emitter_kind_token = 0;
                        a.unwind_state = 7;
                        destroy_native_string_header_0041dd20(&f.emitter_name, strings);
                        char* const name_text = text(&f.emitter_name_token);
                        a.unwind_state = 0;
                        return_captured(name_text, strings); f.emitter_name_token = 0;
                        a.native_site = 0x00b045b9u;
                        publish_native_particle_emitter_member_00af9f00(definition, child);
                    }
                    if (token_is(f.particle_command, "Particle", 0x00b045cau)) {
                        a.native_site = 0x00b04626u;
                        void* const name = get_native_pooled_text_token_00aee3c0(&f.line, &f.particle_name_token, 1, strings);
                        const char* const captured_name = text(name);
                        a.unwind_state = 11; a.native_site = 0x00b04654u;
                        construct_header(f.particle_name, captured_name, strings);
                        a.unwind_state = 12; a.native_site = 0x00b04686u;
                        void* const kind = get_native_pooled_text_token_00aee3c0(&f.line, &f.particle_kind_token, 2, strings);
                        const char* const captured_kind = text(kind);
                        a.unwind_state = 13; a.native_site = 0x00b046b4u;
                        construct_header(f.particle_kind, captured_kind, strings);
                        a.unwind_state = 14; a.native_site = 0x00b046f0u;
                        if (!c.particles) throw std::invalid_argument("Cone emitter parser requires reached particle factory context");
                        f.particle.emplace(c.child_builder_kind);
                        void* const child = create_native_particle_type_definition_00b00ce0(&f.particle_kind, &f.particle_name,
                            definition, buffer, *c.particles, *f.particle);
                        a.unwind_state = 13;
                        destroy_native_string_header_0041dd20(&f.particle_kind, strings);
                        char* const kind_text = text(&f.particle_kind_token);
                        a.unwind_state = 12;
                        return_captured(kind_text, strings); f.particle_kind_token = 0;
                        a.unwind_state = 11;
                        destroy_native_string_header_0041dd20(&f.particle_name, strings);
                        char* const name_text = text(&f.particle_name_token);
                        a.unwind_state = 0;
                        return_captured(name_text, strings); f.particle_name_token = 0;
                        a.native_site = 0x00b047b6u;
                        publish_native_particle_particle_member_00af9f20(definition, child);
                    }
                }
            }
            have_line = read(0x00b047c9u);
        }
        char* const captured_line = text(&f.line);
        a.unwind_state = -1;
        return_captured(captured_line, strings);
        a.phase = Phase::complete;
        return true;
    } catch (...) {
        a.native_state_at_failure = a.unwind_state;
        f.unwind(a, c);
        a.phase = Phase::failed;
        throw;
    }
}
} // namespace bsp
