#include "bsp/native_particle_layer_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Layer lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    Word value;
    std::memcpy(&value, at(p, offset), sizeof value);
    return value;
}
void write(void* p, Word offset, Word value) noexcept {
    std::memcpy(at(p, offset), &value, sizeof value);
}
struct LayerUnwind {
    void* layer;
    NativeStringRawPoolContext& strings;
    int state;
    ~LayerUnwind() noexcept {
        // Constructor map DF2FCC; destructor map DF3008. No retry or lower
        // cleanup follows a second exception during genuine C++ unwinding.
        if (state >= 2) destroy_native_string_header_0041dd20(at(layer, 0x14), strings);
        if (state >= 1) destroy_native_string_header_0041dd20(at(layer, 8), strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(layer);
    }
};
void return_captured_string(void* header, Word data, NativeStringRawPoolContext& strings) {
    if (!data) return;
    const Word bytes = read(header) + 1u;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, reinterpret_cast<void*>(data), bytes,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

void* construct_native_particle_layer_00afab90(void* layer, const void* name,
    Word value_10, NativeStringRawPoolContext& strings) {
    write(layer, 0, 0x00ceb130);
    write(layer, 4, 1);
    void* const destination = at(layer, 8);
    write(layer, 0, 0x00d5dc38);
    LayerUnwind cleanup{layer, strings, 0};
    write(destination, 0, 0);
    write(destination, 4, 0);
    write(layer, 0x14, 0);
    write(layer, 0x18, 0);
    const bool identical = destination == name;
    cleanup.state = 2;
    *static_cast<unsigned char*>(at(layer, 0x2c)) = 1;
    if (!identical) {
        const Word requested = read(name);
        resize_native_string_header_0041dd40(destination, strings, requested, true);
        if (read(name) != 0) {
            const Word count = read(destination);
            const Word source_data = read(name, 4);
            const Word destination_data = read(destination, 4);
            // BF7680 includes a backward-overlap branch. Zero-byte copy omitted
            // consistently with the existing raw native-string providers.
            if (count) std::memmove(reinterpret_cast<void*>(destination_data),
                reinterpret_cast<const void*>(source_data), count);
        }
    }
    write(layer, 0x10, value_10);
    write(layer, 0x30, 0);
    write(layer, 0x34, 0);
    write(layer, 0x38, 0x459c4000); // D1AF84, binary32 5000.0; MOVSS, no x87.
    write(layer, 0x3c, 0);
    cleanup.state = -1;
    return layer;
}

void destroy_native_particle_layer_00afac50(void* layer, NativeStringRawPoolContext& strings) {
    write(layer, 0, 0x00d5dc38);
    const Word material_data = read(layer, 0x18);
    LayerUnwind cleanup{layer, strings, 1};
    return_captured_string(at(layer, 0x14), material_data, strings);
    const Word name_data = read(layer, 0x0c);
    cleanup.state = 0;
    return_captured_string(at(layer, 8), name_data, strings);
    cleanup.state = -1;
    destroy_native_ref_counted_base_00bd30f0(layer);
}

void* scalar_delete_native_particle_layer_00aface0(void* layer, Word flags,
    NativeStringRawPoolContext& strings) {
    destroy_native_particle_layer_00afac50(layer, strings);
    if (flags & 1u) singleton_lifetime_free(layer);
    return layer;
}
} // namespace bsp
