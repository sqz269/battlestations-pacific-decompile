#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Borrowed application state for AF3A20/AF3B50. The atlas field is the actual
// F8C26C publication cell, not a captured manager: AF3A20 reloads it for every
// numbered-name probe. The fallback bytes are the original directly addressed
// one-byte empty string and null-pattern storage.
struct NativeParticleTextureNamesRawContext {
    NativeStringRawPoolContext& strings;
    void* volatile& actual_atlas_manager_00f8c26c;
    const char* empty_stem_00f8c2c1;
    const char* null_pattern_00e17bf0;
};

// Raw eight-byte length/data headers over the application's actual string pool.
// These overloads retain the older NativeStringStorage interfaces unchanged.
void resize_native_particle_string_fill_0043bbf0(void* actual_header,
    std::uint32_t length, std::int8_t fill, NativeStringRawPoolContext&);
void replace_native_particle_string_substrings_004cad40(void* actual_header,
    const void* actual_search_header, const void* actual_replacement_header,
    std::uint32_t count, NativeStringRawPoolContext&);
void* construct_native_particle_texture_stem_00af37d0(void* actual_output_header,
    const char* filename, NativeStringRawPoolContext&);
void* construct_native_particle_texture_extension_00af38b0(void* actual_output_header,
    const char* filename, NativeStringRawPoolContext&);
void* construct_native_particle_texture_prefix_00af3960(void* actual_output_header,
    const char* stem, NativeStringRawPoolContext&);

// AEFB20 receives one actual manager in ECX and returns a borrowed actual 30h
// item. Manager +4 is its current item-pointer array, +8 its current signed
// count, and item +0C is the actual eight-byte name header.
void* find_native_particle_atlas_item_00aefb20(void* actual_manager,
    const char* filename, NativeStringRawPoolContext&,
    const char* null_pattern_00e17bf0);

// AF3A20 ECX filename, RET/EAX signed count. AF3B50 ECX output, EDX filename,
// stack signed index, RET4/EAX output. These are new C++ source interfaces;
// they do not claim original register ABI or FH3 identity.
std::int32_t count_native_particle_texture_frames_00af3a20(const char* filename,
    NativeParticleTextureNamesRawContext&);
void* construct_native_particle_texture_frame_name_00af3b50(
    void* actual_output_header, const char* filename, std::int32_t index,
    NativeParticleTextureNamesRawContext&);
} // namespace bsp
