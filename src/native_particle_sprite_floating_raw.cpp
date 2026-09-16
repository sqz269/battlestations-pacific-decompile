#include "bsp/native_particle_sprite_floating_raw.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Sprite/Floating particle parsers require MSVC Win32 x87.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Acquired = NativeParticleSpriteFloatingRawAcquired;
using Context = NativeParticleSpriteFloatingRawContext;
static_assert(sizeof(void*) == 4);
U word(const void* p) noexcept { return *static_cast<const volatile U*>(p); }
char* text(const void* p) noexcept { return reinterpret_cast<char*>(word(p)); }
void publish(void* definition, U offset, void* parameter) noexcept {
    *reinterpret_cast<volatile U*>(reinterpret_cast<U>(definition) + offset) = reinterpret_cast<U>(parameter);
}
void return_captured(char* bytes, NativeStringRawPoolContext& strings) {
    if (bytes) release_native_pooled_text_bytes_00aee1e0(bytes, strings);
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
void unwind(Acquired& a, Context& c) noexcept {
    static constexpr int previous[]{-1, 0, 0, 2, 2, 4, 5};
    try {
        while (a.unwind_state >= 0) {
            const int state = a.unwind_state;
            a.unwind_state = previous[state];
            if (state == 4) {
                destroy_native_particle_parameter_builder_00af4110(&a.builder_storage, c.builder);
                continue;
            }
            U* const header = state == 0 ? &a.line : state == 1 ? &a.property_suffix
                : state == 2 ? &a.name : state == 3 ? &a.percentage_suffix
                : state == 5 ? &a.curve_suffix : &a.curve;
            destroy_native_pooled_text_00aee2a0(header, c.properties.strings);
        }
    } catch (...) { std::terminate(); }
}
void finish_builder(Acquired& a, Context& c, bool captured_name) {
    std::free(a.builder_storage.records_00);
    char* const captured = captured_name ? text(&a.name) : nullptr;
    a.builder_storage.records_00 = nullptr;
    a.builder_storage.count_04 = 0;
    a.builder_storage.capacity_08 = 0;
    a.unwind_state = 0;
    if (captured_name) { return_captured(captured, c.properties.strings); a.name = 0; }
    else destroy_native_pooled_text_00aee2a0(&a.name, c.properties.strings);
}
bool load(void* definition, void* buffer, Context& c, Acquired& a, bool sprite) {
    using Phase = Acquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Particle parser operation cannot replay");
    a.phase = Phase::running;
    auto& strings = c.properties.strings;
    if (sprite) *reinterpret_cast<volatile unsigned char*>(reinterpret_cast<U>(definition) + 0x64u) = 0;
    a.line = 0; a.unwind_state = 0;
    auto line_read = [&](U site) {
        a.native_site = site;
        return read_native_text_buffer_line_00af5740(buffer, &a.line, strings, c.actual_text_scratch_00f8c2c8);
    };
    try {
        while (line_read(sprite ? 0x00b08af7u : 0x00b07d92u)) {
            if (_stricmp(text(&a.line), "{") == 0) break;
        }
        bool available = line_read(sprite ? 0x00b08b1du : 0x00b07db8u);
        while (available) {
            if (_stricmp(text(&a.line), "}") == 0) break;
            if (_stricmp(text(&a.line), "") != 0) {
                a.native_site = sprite ? 0x00b08b6eu : 0x00b07e04u;
                void* const key = get_native_pooled_text_token_00aee3c0(&a.line, &a.key_token, 0, strings);
                const bool is_parameter = _stricmp(text(key), "Param") == 0;
                return_captured(text(&a.key_token), strings); a.key_token = 0;
                if (is_parameter) {
                    a.native_site = sprite ? 0x00b08bceu : 0x00b07e65u;
                    void* const suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.property_suffix, 1, strings);
                    a.unwind_state = 1;
                    // A completed child has no outstanding provider obligations.
                    // emplace adds no allocation; failed children never reach it.
                    a.property.emplace();
                    a.native_site = sprite ? 0x00b08bdbu : 0x00b07e72u;
                    const bool handled = load_native_particle_type_property_00b015c0(definition, suffix, c.properties, *a.property);
                    char* const property_text = text(&a.property_suffix);
                    a.unwind_state = 0;
                    return_captured(property_text, strings); a.property_suffix = 0;
                    if (!handled) {
                        a.native_site = sprite ? 0x00b08c2cu : 0x00b07ec5u;
                        get_native_pooled_text_token_00aee3c0(&a.line, &a.name, 1, strings);
                        a.unwind_state = 2;
                        a.native_site = sprite ? 0x00b08c41u : 0x00b07edau;
                        void* const percentage_suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.percentage_suffix, 2, strings);
                        a.unwind_state = 3;
                        a.native_site = sprite ? 0x00b08c53u : 0x00b07eecu;
                        void* const percentage_token = get_native_pooled_text_token_00aee3c0(percentage_suffix, &a.percentage_token, 0, strings);
                        a.native_site = sprite ? 0x00b08c5bu : 0x00b07ef4u;
                        atof_store(text(percentage_token), &a.percentage_bits);
                        // The percentage token never has a native unwind state.
                        return_captured(text(&a.percentage_token), strings); a.percentage_token = 0;
                        char* const percentage_text = text(&a.percentage_suffix);
                        a.unwind_state = 2;
                        return_captured(percentage_text, strings); a.percentage_suffix = 0;
                        a.native_site = sprite ? 0x00b08cceu : 0x00b07f6au;
                        construct_native_particle_parameter_builder_00afbed0(&a.builder_storage, c.builder);
                        a.unwind_state = 4;
                        a.native_site = sprite ? 0x00b08ce8u : 0x00b07f84u;
                        initialize_native_particle_parameter_endpoints_00afc360(&a.builder_storage, 0.0f, 0.0f, c.builder);
                        a.native_site = sprite ? 0x00b08cf8u : 0x00b07f94u;
                        void* const curve_suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.curve_suffix, 2, strings);
                        a.unwind_state = 5;
                        a.native_site = sprite ? 0x00b08d0bu : 0x00b07fa7u;
                        void* const curve = get_native_pooled_text_suffix_00af44c0(curve_suffix, &a.curve, 1, strings);
                        a.unwind_state = 6;
                        a.native_site = sprite ? 0x00b08d1au : 0x00b07fb6u;
                        (void)parse_native_particle_parameter_00afc470(&a.builder_storage, curve, c.builder);
                        char* const curve_text = text(&a.curve);
                        a.unwind_state = 5;
                        return_captured(curve_text, strings); a.curve = 0;
                        char* const suffix_text = text(&a.curve_suffix);
                        a.unwind_state = 4;
                        return_captured(suffix_text, strings);
                        U parameter_argument;
                        copy_float_store(&parameter_argument, &a.percentage_bits);
                        float argument; std::memcpy(&argument, &parameter_argument, sizeof argument);
                        a.curve_suffix = 0;
                        a.native_site = sprite ? 0x00b08d9bu : 0x00b0803au;
                        const bool common = load_native_particle_type_parameter_00b00980(definition, &a.name, &a.builder_storage, argument, c.parameters);
                        bool initial = false;
                        if (!common) {
                            initial = _stricmp(text(&a.name), "InitialRotation") == 0;
                            const bool rotation = !initial && _stricmp(text(&a.name), "RotationSpeed") == 0;
                            const bool size = !initial && !rotation && _stricmp(text(&a.name), "Size") == 0;
                            if (initial || rotation || size) {
                                a.native_site = sprite ? (initial ? 0x00b08df7u : rotation ? 0x00b08e7au : 0x00b08f33u)
                                    : (initial ? 0x00b08097u : rotation ? 0x00b08122u : 0x00b081e2u);
                                void* const parameter = convert_native_particle_parameter_00afbf60(&a.builder_storage, c.parameters.parameters);
                                // Capture CURRENT scale only after runtime conversion.
                                scale_parameter(parameter, &a.percentage_bits, c.parameters.percentage_scale_00d7a358);
                                if (sprite && size) {
                                    a.native_site = 0x00b08f47u;
                                    set_native_sprite_particle_size_00b08870(definition, &c.parameters, parameter);
                                } else publish(definition, initial ? 0x80u : rotation ? 0x84u : 0x88u, parameter);
                            }
                        }
                        a.native_site = sprite ? (common ? 0x00b08da9u : initial ? 0x00b08e13u : 0x00b08e96u)
                            : (common ? 0x00b08048u : initial ? 0x00b080b3u : 0x00b0813eu);
                        finish_builder(a, c, common || initial);
                    }
                }
            }
            available = line_read(sprite ? 0x00b08ebeu : 0x00b08169u);
        }
        char* const line_text = text(&a.line);
        a.unwind_state = -1;
        a.native_site = sprite ? 0x00b08ef2u : 0x00b081a2u;
        return_captured(line_text, strings); // Native final header stays stale.
        a.phase = Phase::complete;
        return true;
    } catch (...) {
        a.phase = Phase::failed;
        unwind(a, c);
        throw;
    }
}
} // namespace

NativeParticleSpriteFloatingRawAcquired::NativeParticleSpriteFloatingRawAcquired(std::int32_t kind) noexcept {
    builder_storage.kind_0c = kind;
}
bool load_native_sprite_particle_definition_00b08ac0(void* definition, void* text_buffer,
    Context& context, Acquired& acquired) { return load(definition, text_buffer, context, acquired, true); }
bool load_native_floating_particle_definition_00b07d60(void* definition, void* text_buffer,
    Context& context, Acquired& acquired) { return load(definition, text_buffer, context, acquired, false); }
} // namespace bsp
