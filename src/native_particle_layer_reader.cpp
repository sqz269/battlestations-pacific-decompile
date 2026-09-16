#include "bsp/native_particle_layer_reader.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Layer reading requires MSVC Win32.
#endif

namespace bsp {
namespace {
char* current_text(const void* header) noexcept {
    return *static_cast<char* const volatile*>(header);
}
template<class T> T& field(void* owner, unsigned offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(owner) + offset);
}
void* current_records(void* builder) noexcept {
    return field<void* volatile>(builder, 0);
}
void clear_vector(void* builder) noexcept {
    field<void* volatile>(builder, 0) = nullptr;
    field<std::int32_t volatile>(builder, 4) = 0;
    field<std::int32_t volatile>(builder, 8) = 0;
}
void discard_atof(const char* text) {
    // AFAEBD/AFAEC2: the conversion is called even though its ST0 is discarded.
    using Atof = double (__cdecl*)(const char*);
    Atof const convert = &std::atof;
    __asm {
        push text
        call convert
        fstp st(0)
        add esp, 4
    }
}
void* store_first_and_capture(void* builder, void* destination) {
    using First = float (__cdecl*)(const void*);
    First const first = &first_native_particle_parameter_value_00afc1b0;
    float spill;
    void* records;
    __asm {
        push builder
        call first
        add esp, 4
        fstp spill
        fld spill
        mov ecx, builder
        mov eax, [ecx]
        mov records, eax
        mov edx, destination
        fstp dword ptr [edx]
    }
    return records;
}
void store_first(void* builder, void* destination) {
    using First = float (__cdecl*)(const void*);
    First const first = &first_native_particle_parameter_value_00afc1b0;
    float spill;
    __asm {
        push builder
        call first
        add esp, 4
        fstp spill
        fld spill
        mov edx, destination
        fstp dword ptr [edx]
    }
}
struct LayerUnwind {
    NativePooledTextStorage& line;
    NativePooledTextStorage& name;
    NativePooledTextStorage& initial_suffix;
    NativeParticleParameterBuilderStorage& builder;
    NativePooledTextStorage& outer_suffix;
    NativePooledTextStorage& inner_suffix;
    NativeParticleParameterBuilderRawContext& context;
    int state = 0;
    ~LayerUnwind() noexcept {
        // DF3060: 5->4 inner, 4->3 outer, 3->1 builder, 2->1 initial
        // suffix, 1->0 name, 0->-1 line. Keyword/numeric tokens are unowned.
        if (state == 5) {
            destroy_native_pooled_text_00aee2a0(&inner_suffix, context.strings);
            state = 4;
        }
        if (state == 4) {
            destroy_native_pooled_text_00aee2a0(&outer_suffix, context.strings);
            state = 3;
        }
        if (state == 3) {
            destroy_native_particle_parameter_builder_00af4110(&builder, context);
            state = 1;
        }
        if (state == 2) {
            destroy_native_pooled_text_00aee2a0(&initial_suffix, context.strings);
            state = 1;
        }
        if (state == 1) {
            destroy_native_pooled_text_00aee2a0(&name, context.strings);
            state = 0;
        }
        if (state == 0) destroy_native_pooled_text_00aee2a0(&line, context.strings);
    }
};
}

bool read_native_particle_layer_00afad00(void* layer, void* buffer,
    NativeParticleParameterBuilderRawContext& context, char* scratch) {
    auto& strings = context.strings;
    NativePooledTextStorage line{nullptr};
    NativePooledTextStorage name, initial_suffix, outer_suffix, inner_suffix;
    NativeParticleParameterBuilderStorage builder; // kind+C remains indeterminate.
    LayerUnwind unwind{line, name, initial_suffix, builder, outer_suffix,
        inner_suffix, context};
    while (read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch)) {
        if (_stricmp(current_text(&line), "{") == 0) break;
    }
    // EOF from the search still performs this body read.
    bool have_line = read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch);
    while (have_line) {
        if (_stricmp(current_text(&line), "}") == 0) break;
        if (_stricmp(current_text(&line), "") != 0) {
            NativePooledTextStorage keyword;
            const void* token = get_native_pooled_text_token_00aee3c0(&line, &keyword, 0, strings);
            const bool parameter = _stricmp(current_text(token), "Param") == 0;
            destroy_native_pooled_text_00aee2a0(&keyword, strings);
            if (parameter) {
                get_native_pooled_text_token_00aee3c0(&line, &name, 1, strings);
                unwind.state = 1;
                if (_stricmp(current_text(&name), "Sort") == 0) {
                    NativePooledTextStorage value;
                    token = get_native_pooled_text_token_00aee3c0(&line, &value, 2, strings);
                    const long integer = std::atol(current_text(token));
                    char* const value_bytes = current_text(&value);
                    field<unsigned char volatile>(layer, 0x24) = integer > 0;
                    if (value_bytes) release_native_pooled_text_bytes_00aee1e0(value_bytes, strings);
                    char* const name_bytes = current_text(&name);
                    value.data = nullptr;
                    unwind.state = 0;
                    if (name_bytes) release_native_pooled_text_bytes_00aee1e0(name_bytes, strings);
                    name.data = nullptr;
                } else {
                    NativePooledTextStorage discarded;
                    const void* suffix = get_native_pooled_text_suffix_00af44c0(&line, &initial_suffix, 2, strings);
                    unwind.state = 2;
                    token = get_native_pooled_text_token_00aee3c0(suffix, &discarded, 0, strings);
                    discard_atof(current_text(token));
                    char* const discarded_bytes = current_text(&discarded);
                    if (discarded_bytes) release_native_pooled_text_bytes_00aee1e0(discarded_bytes, strings);
                    char* const initial_bytes = current_text(&initial_suffix);
                    discarded.data = nullptr;
                    unwind.state = 1;
                    if (initial_bytes) release_native_pooled_text_bytes_00aee1e0(initial_bytes, strings);
                    initial_suffix.data = nullptr;
                    construct_native_particle_parameter_builder_00afbed0(&builder, context);
                    unwind.state = 3;
                    initialize_native_particle_parameter_endpoints_00afc360(&builder, 0.0f, 0.0f, context);
                    suffix = get_native_pooled_text_suffix_00af44c0(&line, &outer_suffix, 2, strings);
                    unwind.state = 4;
                    suffix = get_native_pooled_text_suffix_00af44c0(suffix, &inner_suffix, 1, strings);
                    unwind.state = 5;
                    (void)parse_native_particle_parameter_00afc470(&builder, suffix, context);
                    char* const inner_bytes = current_text(&inner_suffix);
                    unwind.state = 4;
                    if (inner_bytes) release_native_pooled_text_bytes_00aee1e0(inner_bytes, strings);
                    char* const outer_bytes = current_text(&outer_suffix);
                    inner_suffix.data = nullptr;
                    unwind.state = 3;
                    if (outer_bytes) release_native_pooled_text_bytes_00aee1e0(outer_bytes, strings);
                    outer_suffix.data = nullptr;

                    if (_stricmp(current_text(&name), "MaxParticles") == 0) {
                        field<std::int32_t volatile>(layer, 0x20) =
                            first_native_particle_parameter_integer_00afc1c0(&builder);
                        singleton_lifetime_free(current_records(&builder));
                        char* const name_bytes = current_text(&name);
                        clear_vector(&builder);
                        unwind.state = 0;
                        if (name_bytes) release_native_pooled_text_bytes_00aee1e0(name_bytes, strings);
                        name.data = nullptr;
                    } else if (_stricmp(current_text(&name), "RenderPriority") == 0) {
                        const std::int32_t value = first_native_particle_parameter_integer_00afc1c0(&builder);
                        void* const records = current_records(&builder);
                        field<std::int32_t volatile>(layer, 0x28) = value;
                        singleton_lifetime_free(records);
                        char* const name_bytes = current_text(&name);
                        clear_vector(&builder);
                        unwind.state = 0;
                        if (name_bytes) release_native_pooled_text_bytes_00aee1e0(name_bytes, strings);
                        name.data = nullptr;
                    } else {
                        if (_stricmp(current_text(&name), "LODFadeIn") == 0) {
                            singleton_lifetime_free(store_first_and_capture(&builder, &field<float>(layer, 0x30)));
                            clear_vector(&builder);
                        } else if (_stricmp(current_text(&name), "LODFadeInWidth") == 0) {
                            singleton_lifetime_free(store_first_and_capture(&builder, &field<float>(layer, 0x34)));
                            clear_vector(&builder);
                        } else {
                            if (_stricmp(current_text(&name), "LODFadeOut") == 0)
                                store_first(&builder, &field<float>(layer, 0x38));
                            else if (_stricmp(current_text(&name), "LODFadeOutWidth") == 0)
                                store_first(&builder, &field<float>(layer, 0x3c));
                            destroy_native_particle_parameter_builder_00af4110(&builder, context);
                        }
                        unwind.state = 0;
                        destroy_native_pooled_text_00aee2a0(&name, strings);
                    }
                }
            }
        }
        have_line = read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch);
    }
    char* const line_bytes = current_text(&line);
    unwind.state = -1;
    if (line_bytes) release_native_pooled_text_bytes_00aee1e0(line_bytes, strings);
    return true;
}
} // namespace bsp
