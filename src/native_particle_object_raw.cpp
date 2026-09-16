#include "bsp/native_particle_object_raw.hpp"
#include "bsp/native_particle_object_tracer_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Object particle parser requires MSVC Win32 x87.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Acquired = NativeParticleObjectRawAcquired;
using Context = NativeParticleObjectRawContext;
static_assert(sizeof(void*) == 4);
U word(const void* p) noexcept { return *static_cast<const volatile U*>(p); }
char* text(const void* p) noexcept { return reinterpret_cast<char*>(word(p)); }
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
    static constexpr int previous[]{-1, 0, 0, 2, 2, 2, 5, 6};
    try {
        while (a.unwind_state >= 0) {
            const int state = a.unwind_state;
            a.unwind_state = previous[state];
            if (state == 5) {
                destroy_native_particle_parameter_builder_00af4110(&a.builder_storage, c.builder);
                continue;
            }
            U* const header = state == 0 ? &a.line : state == 1 ? &a.property_suffix
                : state == 2 ? &a.name : state == 3 ? &a.model_name
                : state == 4 ? &a.percentage_suffix : state == 6 ? &a.curve_suffix : &a.curve;
            destroy_native_pooled_text_00aee2a0(header, c.properties.strings);
        }
    } catch (...) { std::terminate(); }
}
void finish_builder(Acquired& a, Context& c, bool common) {
    std::free(a.builder_storage.records_00);
    char* const captured = common ? text(&a.name) : nullptr;
    a.builder_storage.records_00 = nullptr;
    a.builder_storage.count_04 = 0;
    a.builder_storage.capacity_08 = 0;
    a.unwind_state = 0;
    if (common) { return_captured(captured, c.properties.strings); a.name = 0; }
    else destroy_native_pooled_text_00aee2a0(&a.name, c.properties.strings);
}
} // namespace

NativeParticleObjectRawAcquired::NativeParticleObjectRawAcquired(std::int32_t kind) noexcept {
    builder_storage.kind_0c = kind;
}
bool load_native_object_particle_definition_00af8bd0(void* definition, void* buffer,
    Context& c, Acquired& a) {
    using Phase = Acquired::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("Object particle parser operation cannot replay");
    a.phase = Phase::running;
    auto& strings = c.properties.strings;
    *reinterpret_cast<volatile unsigned char*>(reinterpret_cast<U>(definition) + 0x64u) = 0;
    a.line = 0; a.unwind_state = 0;
    auto line_read = [&](U site) {
        a.native_site = site;
        return read_native_text_buffer_line_00af5740(buffer, &a.line, strings, c.actual_text_scratch_00f8c2c8);
    };
    try {
        while (line_read(0x00af8c0bu)) {
            if (_stricmp(text(&a.line), "{") == 0) break;
        }
        bool available = line_read(0x00af8c31u);
        while (available) {
            if (_stricmp(text(&a.line), "}") == 0) break;
            if (_stricmp(text(&a.line), "") != 0) {
                a.native_site = 0x00af8c7eu;
                void* const key = get_native_pooled_text_token_00aee3c0(&a.line, &a.key_token, 0, strings);
                const bool is_parameter = _stricmp(text(key), "Param") == 0;
                return_captured(text(&a.key_token), strings); a.key_token = 0;
                if (is_parameter) {
                    a.native_site = 0x00af8cdau;
                    void* const suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.property_suffix, 1, strings);
                    a.unwind_state = 1;
                    a.property.emplace();
                    a.native_site = 0x00af8ce7u;
                    const bool handled = load_native_particle_type_property_00b015c0(definition, suffix, c.properties, *a.property);
                    char* const property_text = text(&a.property_suffix);
                    a.unwind_state = 0;
                    return_captured(property_text, strings); a.property_suffix = 0;
                    if (!handled) {
                        a.native_site = 0x00af8d35u;
                        get_native_pooled_text_token_00aee3c0(&a.line, &a.name, 1, strings);
                        a.unwind_state = 2;
                        if (_stricmp(text(&a.name), "Model") == 0) {
                            a.native_site = 0x00af8d60u;
                            void* const filename = get_native_pooled_text_token_00aee3c0(&a.line, &a.model_name, 2, strings);
                            const char* const captured_name = text(filename);
                            const U profile = word(definition);
                            // The native call consumes CURRENT profile+20, not a
                            // cached constructor default or a host model owner.
                            a.native_site = 0x00af8d6bu;
                            if (profile != 0x00d5db00u || !c.actual_object_profile_00d5db00_through20) {
                                a.unwind_state = 3; // Explicit unsupported-profile boundary owns the token.
                                throw std::invalid_argument("Unknown current Object particle model profile");
                            }
                            a.captured_model_target = c.actual_object_profile_00d5db00_through20[0x20 / 4];
                            a.unwind_state = 3;
                            a.native_site = 0x00af8d75u;
                            if (a.captured_model_target != 0x00af9660u)
                                throw std::invalid_argument("Unknown current Object particle model target");
                            a.model.emplace();
                            load_native_object_particle_models_00af9660(definition, captured_name, c.models, *a.model);
                            char* const model_text = text(&a.model_name);
                            a.unwind_state = 2;
                            return_captured(model_text, strings);
                            char* const name_text = text(&a.name);
                            a.model_name = 0;
                            a.unwind_state = 0;
                            return_captured(name_text, strings); a.name = 0;
                        } else {
                            a.native_site = 0x00af8ddfu;
                            void* const percentage_suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.percentage_suffix, 2, strings);
                            a.unwind_state = 4;
                            a.native_site = 0x00af8df1u;
                            void* const percentage_token = get_native_pooled_text_token_00aee3c0(percentage_suffix, &a.percentage_token, 0, strings);
                            a.native_site = 0x00af8df9u;
                            atof_store(text(percentage_token), &a.percentage_bits);
                            // Command and percentage tokens are not armed in FH3.
                            return_captured(text(&a.percentage_token), strings); a.percentage_token = 0;
                            char* const percentage_text = text(&a.percentage_suffix);
                            a.unwind_state = 2;
                            return_captured(percentage_text, strings); a.percentage_suffix = 0;
                            a.native_site = 0x00af8e6cu;
                            construct_native_particle_parameter_builder_00afbed0(&a.builder_storage, c.builder);
                            a.unwind_state = 5;
                            a.native_site = 0x00af8e86u;
                            initialize_native_particle_parameter_endpoints_00afc360(&a.builder_storage, 0.0f, 0.0f, c.builder);
                            a.native_site = 0x00af8e96u;
                            void* const curve_suffix = get_native_pooled_text_suffix_00af44c0(&a.line, &a.curve_suffix, 2, strings);
                            a.unwind_state = 6;
                            a.native_site = 0x00af8ea9u;
                            void* const curve = get_native_pooled_text_suffix_00af44c0(curve_suffix, &a.curve, 1, strings);
                            a.unwind_state = 7;
                            a.native_site = 0x00af8eb8u;
                            (void)parse_native_particle_parameter_00afc470(&a.builder_storage, curve, c.builder);
                            char* const curve_text = text(&a.curve);
                            a.unwind_state = 6;
                            return_captured(curve_text, strings);
                            char* const suffix_text = text(&a.curve_suffix);
                            a.curve = 0;
                            a.unwind_state = 5;
                            return_captured(suffix_text, strings);
                            U parameter_argument;
                            copy_float_store(&parameter_argument, &a.percentage_bits);
                            float argument; std::memcpy(&argument, &parameter_argument, sizeof argument);
                            a.curve_suffix = 0;
                            a.native_site = 0x00af8f3au;
                            const bool common = load_native_particle_type_parameter_00b00980(definition, &a.name, &a.builder_storage, argument, c.parameters);
                            if (!common) {
                                const bool rotation = _stricmp(text(&a.name), "RotationSpeed") == 0;
                                const bool size = !rotation && _stricmp(text(&a.name), "Size") == 0;
                                if (rotation || size) {
                                    a.native_site = rotation ? 0x00af8fb2u : 0x00af9072u;
                                    void* const parameter = convert_native_particle_parameter_00afbf60(&a.builder_storage, c.parameters.parameters);
                                    scale_parameter(parameter, &a.percentage_bits, c.parameters.percentage_scale_00d7a358);
                                    if (rotation) *reinterpret_cast<volatile U*>(reinterpret_cast<U>(definition) + 0x80u) = reinterpret_cast<U>(parameter);
                                    else {
                                        a.native_site = 0x00af9086u;
                                        set_native_object_particle_size_00af80f0(definition, parameter);
                                    }
                                }
                            }
                            a.native_site = common ? 0x00af8f48u : 0x00af8fceu;
                            finish_builder(a, c, common);
                        }
                    }
                }
            }
            available = line_read(0x00af8ff9u);
        }
        char* const line_text = text(&a.line);
        a.unwind_state = -1;
        a.native_site = 0x00af9039u;
        return_captured(line_text, strings); // Native final header remains stale.
        a.phase = Phase::complete;
        return true;
    } catch (...) {
        a.phase = Phase::failed;
        a.native_state_at_failure = a.unwind_state;
        unwind(a, c);
        throw;
    }
}
} // namespace bsp
