#include "bsp/native_particle_sphere_raw.hpp"
#include "bsp/native_particle_emitter_factory_raw.hpp"
#include "bsp/native_particle_type_factory_raw.hpp"
#include "bsp/native_particle_definition_loading.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Sphere emitter parser requires MSVC Win32 x87.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Acquired = NativeParticleSphereRawAcquired;
using Context = NativeParticleSphereRawContext;
static_assert(sizeof(void*) == 4);
U word(const void* p) noexcept { return *static_cast<const volatile U*>(p); }
char* text(const void* p) noexcept { return reinterpret_cast<char*>(word(p)); }
void* pointer(U w) noexcept { return reinterpret_cast<void*>(w); }
void return_captured(char* p, NativeStringRawPoolContext& strings) {
    if (p) release_native_pooled_text_bytes_00aee1e0(p, strings);
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
__declspec(naked) void __cdecl copy_float_store(U*, const U*) {
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
// Inline native construction: captured token bytes, zero 8h header, strlen,
// genuine resize, then CURRENT pointer/length and overlap-safe copy.
void build_string(U* out, const char* captured, NativeStringRawPoolContext& strings) {
    out[0] = 0; out[1] = 0;
    resize_native_string_header_0041dd40(out, strings,
        static_cast<U>(std::strlen(captured)), true);
    char* const destination = reinterpret_cast<char*>(word(out + 1));
    if (destination) std::memmove(destination, captured, word(out) + 1u);
}
} // namespace

struct NativeParticleSphereRawAcquired::Impl {
    explicit Impl(std::int32_t kind) { builder.kind_0c = kind; }
    // Native EBP-7C through EBP-20, followed by the EBP-1C builder. In
    // particular saved definition and both actual8h child strings stay live.
    U locals[24];
    NativeParticleParameterBuilderStorage builder;
    std::optional<NativeParticleEmitterFactoryRawAcquired> emitter;
    std::optional<NativeParticleTypeFactoryRawAcquired> particle;
    U* slot(unsigned index) noexcept { return locals + index; }
    void unwind(Acquired& a, Context& c) noexcept {
        static constexpr int previous[]{-1,0,0,2,2,4,5,0,7,8,9,0,11,12,13};
        static constexpr unsigned slots[]{0,5,1,7,24,9,8,12,18,11,16,15,22,14,20};
        try {
            while (a.unwind_state >= 0) {
                const int state = a.unwind_state;
                a.unwind_state = previous[state];
                if (state == 4)
                    destroy_native_particle_parameter_builder_00af4110(&builder, c.builder);
                else if (state == 8 || state == 10 || state == 12 || state == 14)
                    destroy_native_string_header_0041dd20(slot(slots[state]), c.builder.strings);
                else destroy_native_pooled_text_00aee2a0(slot(slots[state]), c.builder.strings);
            }
        } catch (...) { std::terminate(); }
    }
};
NativeParticleSphereRawAcquired::NativeParticleSphereRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleSphereRawAcquired::~NativeParticleSphereRawAcquired() = default;
NativeParticleEmitterFactoryRawAcquired* NativeParticleSphereRawAcquired::emitter_child() noexcept {
    return impl_->emitter ? &*impl_->emitter : nullptr;
}
NativeParticleTypeFactoryRawAcquired* NativeParticleSphereRawAcquired::particle_child() noexcept {
    return impl_->particle ? &*impl_->particle : nullptr;
}

bool load_native_sphere_emitter_definition_00b02fd0(void* definition, void* buffer,
    Context& c, Acquired& a) {
    using Phase = Acquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Sphere emitter parser cannot replay");
    a.phase = Phase::running;
    auto& f = *a.impl_;
    auto& strings = c.builder.strings;
    auto slot = [&](unsigned i) { return f.slot(i); };
    auto give = [&](unsigned i) { return_captured(text(slot(i)), strings); *slot(i) = 0; };
    auto line_read = [&](U site) {
        a.native_site = site;
        return read_native_text_buffer_line_00af5740(buffer, slot(0), strings,
            c.actual_text_scratch_00f8c2c8);
    };
    *slot(3) = reinterpret_cast<U>(definition);
    *slot(0) = 0; a.unwind_state = 0;
    try {
        while (line_read(0x00b0300cu)) {
            if (_stricmp(text(slot(0)), "{") == 0) break;
        }
        bool available = line_read(0x00b03032u);
        while (available) {
            if (_stricmp(text(slot(0)), "}") == 0) break;
            if (_stricmp(text(slot(0)), "") != 0) {
                a.native_site = 0x00b0307eu;
                void* token = get_native_pooled_text_token_00aee3c0(slot(0), slot(4), 0, strings);
                const bool parameter_line = _stricmp(text(token), "Param") == 0;
                give(4);
                if (parameter_line) {
                    a.native_site = 0x00b030dau;
                    void* suffix = get_native_pooled_text_suffix_00af44c0(slot(0), slot(5), 1, strings);
                    a.unwind_state = 1;
                    a.native_site = 0x00b030eau;
                    const bool flag = load_native_particle_definition_flag_00afa650(definition, suffix, strings);
                    char* captured = text(slot(5)); a.unwind_state = 0;
                    return_captured(captured, strings); *slot(5) = 0;
                    if (!flag) {
                        a.native_site = 0x00b0313bu;
                        get_native_pooled_text_token_00aee3c0(slot(0), slot(1), 1, strings);
                        a.unwind_state = 2;
                        a.native_site = 0x00b03153u;
                        suffix = get_native_pooled_text_suffix_00af44c0(slot(0), slot(7), 2, strings);
                        a.unwind_state = 3;
                        a.native_site = 0x00b03168u;
                        token = get_native_pooled_text_token_00aee3c0(suffix, slot(6), 0, strings);
                        a.native_site = 0x00b03170u;
                        atof_store(text(token), slot(2));
                        give(6);
                        captured = text(slot(7)); a.unwind_state = 2;
                        return_captured(captured, strings); *slot(7) = 0;
                        a.native_site = 0x00b031edu;
                        construct_native_particle_parameter_builder_00afbed0(&f.builder, c.builder);
                        a.unwind_state = 4;
                        a.native_site = 0x00b0320au;
                        initialize_native_particle_parameter_endpoints_00afc360(&f.builder, 0.0f, 0.0f, c.builder);
                        a.native_site = 0x00b0321au;
                        suffix = get_native_pooled_text_suffix_00af44c0(slot(0), slot(9), 2, strings);
                        a.unwind_state = 5;
                        a.native_site = 0x00b03230u;
                        void* curve = get_native_pooled_text_suffix_00af44c0(suffix, slot(8), 1, strings);
                        a.unwind_state = 6;
                        a.native_site = 0x00b03242u;
                        (void)parse_native_particle_parameter_00afc470(&f.builder, curve, c.builder);
                        captured = text(slot(8)); a.unwind_state = 5;
                        return_captured(captured, strings);
                        captured = text(slot(9)); *slot(8) = 0; a.unwind_state = 4;
                        return_captured(captured, strings);
                        U argument_bits; copy_float_store(&argument_bits, slot(2));
                        float argument; std::memcpy(&argument, &argument_bits, sizeof argument);
                        *slot(9) = 0;
                        a.native_site = 0x00b032cdu;
                        const bool common = load_native_particle_base_parameter_00af9d00(definition,
                            slot(1), &f.builder, argument, c.parameters, c.percentage_scale_00d7a358);
                        bool speed = false;
                        if (!common) {
                            speed = _stricmp(text(slot(1)), "EmittedSpeed") == 0;
                            const bool inner = !speed && _stricmp(text(slot(1)), "InnerRadius") == 0;
                            const bool outer = !speed && !inner && _stricmp(text(slot(1)), "OuterRadius") == 0;
                            if (speed || inner || outer) {
                                a.native_site = speed ? 0x00b0332du : inner ? 0x00b033b3u : 0x00b03413u;
                                void* parameter = convert_native_particle_parameter_00afbf60(&f.builder, c.parameters);
                                scale_parameter(parameter, slot(2), c.percentage_scale_00d7a358);
                                *reinterpret_cast<volatile U*>(reinterpret_cast<U>(definition)
                                    + (speed ? 0x80u : inner ? 0x84u : 0x88u)) = reinterpret_cast<U>(parameter);
                            }
                        }
                        a.native_site = common ? 0x00b032dbu : speed ? 0x00b03349u : 0x00b033cfu;
                        std::free(f.builder.records_00);
                        // The common/EmittedSpeed arm captures name after free
                        // but before clearing the builder and disarming state.
                        captured = common || speed ? text(slot(1)) : nullptr;
                        f.builder.records_00 = nullptr; f.builder.count_04 = 0; f.builder.capacity_08 = 0;
                        a.unwind_state = 0;
                        if (common || speed) { return_captured(captured, strings); *slot(1) = 0; }
                        else destroy_native_pooled_text_00aee2a0(slot(1), strings);
                    }
                } else {
                    a.native_site = 0x00b0343bu;
                    token = get_native_pooled_text_token_00aee3c0(slot(0), slot(10), 0, strings);
                    const bool emitter_line = _stricmp(text(token), "Emitter") == 0;
                    give(10);
                    if (emitter_line) {
                        a.native_site = 0x00b03497u;
                        token = get_native_pooled_text_token_00aee3c0(slot(0), slot(12), 1, strings);
                        const char* source = text(token); a.unwind_state = 7;
                        a.native_site = 0x00b034c5u;
                        build_string(slot(18), source, strings); a.unwind_state = 8;
                        a.native_site = 0x00b034f7u;
                        token = get_native_pooled_text_token_00aee3c0(slot(0), slot(11), 2, strings);
                        source = text(token); a.unwind_state = 9;
                        a.native_site = 0x00b03525u;
                        build_string(slot(16), source, strings);
                        void* const parent = pointer(*slot(3));
                        const U word10 = word(reinterpret_cast<const void*>(reinterpret_cast<U>(parent) + 0x10u));
                        a.unwind_state = 10; a.native_site = 0x00b03565u;
                        if (!c.emitters) throw std::invalid_argument("Missing concrete nested emitter factory context");
                        f.emitter.emplace(c.child_builder_kind);
                        void* child = create_native_particle_definition_00af9fb0(slot(16), slot(18), word10,
                            reinterpret_cast<U>(parent), buffer, *c.emitters, *f.emitter);
                        a.unwind_state = 9; destroy_native_string_header_0041dd20(slot(16), strings);
                        char* kind_text = text(slot(11)); a.unwind_state = 8;
                        return_captured(kind_text, strings);
                        *slot(11) = 0; a.unwind_state = 7;
                        destroy_native_string_header_0041dd20(slot(18), strings);
                        char* name_text = text(slot(12)); a.unwind_state = 0;
                        return_captured(name_text, strings); *slot(12) = 0;
                        a.native_site = 0x00b0362bu;
                        publish_native_particle_emitter_member_00af9f00(parent, child);
                        definition = parent;
                    }
                    // Deliberately tokenize CURRENT line again after child
                    // loading; native Emitter and Particle checks are separate.
                    a.native_site = 0x00b0363cu;
                    token = get_native_pooled_text_token_00aee3c0(slot(0), slot(13), 0, strings);
                    const bool particle_line = _stricmp(text(token), "Particle") == 0;
                    give(13);
                    if (particle_line) {
                        a.native_site = 0x00b03698u;
                        token = get_native_pooled_text_token_00aee3c0(slot(0), slot(15), 1, strings);
                        const char* source = text(token); a.unwind_state = 11;
                        a.native_site = 0x00b036c6u;
                        build_string(slot(22), source, strings); a.unwind_state = 12;
                        a.native_site = 0x00b036f8u;
                        token = get_native_pooled_text_token_00aee3c0(slot(0), slot(14), 2, strings);
                        source = text(token); a.unwind_state = 13;
                        a.native_site = 0x00b03726u;
                        build_string(slot(20), source, strings);
                        void* const parent = pointer(*slot(3));
                        a.unwind_state = 14; a.native_site = 0x00b03762u;
                        if (!c.particles) throw std::invalid_argument("Missing concrete nested particle factory context");
                        f.particle.emplace(c.child_builder_kind);
                        void* child = create_native_particle_type_definition_00b00ce0(slot(20), slot(22),
                            parent, buffer, *c.particles, *f.particle);
                        a.unwind_state = 13; destroy_native_string_header_0041dd20(slot(20), strings);
                        char* kind_text = text(slot(14)); a.unwind_state = 12;
                        return_captured(kind_text, strings); *slot(14) = 0; a.unwind_state = 11;
                        destroy_native_string_header_0041dd20(slot(22), strings);
                        char* name_text = text(slot(15)); a.unwind_state = 0;
                        return_captured(name_text, strings); *slot(15) = 0;
                        a.native_site = 0x00b03828u;
                        (void)publish_native_particle_particle_member_00af9f20(parent, child);
                        definition = parent;
                    }
                }
            }
            available = line_read(0x00b0383bu);
        }
        char* line_text = text(slot(0)); a.unwind_state = -1;
        a.native_site = 0x00b0387au;
        return_captured(line_text, strings); // Native final line header is stale.
        a.phase = Phase::complete;
        return true;
    } catch (...) {
        a.phase = Phase::failed; a.native_state_at_failure = a.unwind_state;
        f.unwind(a, c);
        throw;
    }
}
} // namespace bsp
