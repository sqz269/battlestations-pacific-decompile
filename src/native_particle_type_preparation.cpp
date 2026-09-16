#include "bsp/native_particle_type_preparation.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type preparation requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(void* p, Word offset) noexcept { return pointer(address(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    Word value;
    std::memcpy(&value, pointer(address(p) + offset), sizeof value);
    return value;
}
struct NameArgument { Word length; Word data; };
static_assert(sizeof(NameArgument) == 8);

struct ConsumedNameUnwind {
    void* argument;
    NativeStringRawPoolContext& strings;
    bool active = true;
    ~ConsumedNameUnwind() noexcept {
        // CBB890 uses the actual incoming stack header (EBP+4), not the
        // caller's initially captured EDI/EBX values. A second exception
        // during this active unwind must terminate.
        if (active) destroy_native_string_header_0041dd20(argument, strings);
    }
};

void prepare_type(void* type, const char* material, NativeStringRawPoolContext& strings) {
    const Word index = read(type, 0x74);
    void* const parent = pointer(read(type, 0x14));
    void* const layer = pointer(read(parent, index * 4u + 0x34u));
    NameArgument argument;
    construct_native_string_header_0041e870(&argument, strings, material);
    set_native_particle_layer_material_00b075d0(layer, &argument, strings);
}
} // namespace

void set_native_particle_layer_material_00b075d0(void* layer,
    void* argument, NativeStringRawPoolContext& strings) {
    const Word captured_data = read(argument, 4); // EBX is loaded before EDI.
    const Word captured_length = read(argument);
    void* const destination = at(layer, 0x14);
    ConsumedNameUnwind unwind{argument, strings};
    if (destination != argument) {
        resize_native_string_header_0041dd40(destination, strings, captured_length, true);
        if (captured_length != 0) {
            const Word bytes = read(destination);
            void* const data = pointer(read(destination, 4));
            if (bytes) std::memmove(data, pointer(captured_data), bytes);
        }
    }
    unwind.active = false;
    if (captured_data != 0) {
        // Native state=-1 before resolving pool; getter failure must not retry
        // argument destruction. Normal return preserves the consumed header.
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(captured_data),
            captured_length + 1u, strings.actual_small_returns_disabled_01090aa4);
    }
}

void prepare_native_sprite_particle_type_00b0a000(void* type, NativeStringRawPoolContext& strings) {
    prepare_type(type, "ParticleSprite.mvfm", strings);
}
void prepare_native_axial_particle_type_00b07660(void* type, NativeStringRawPoolContext& strings) {
    prepare_type(type, "ParticleAxial.mvfm", strings);
}
void prepare_native_floating_particle_type_00b087b0(void* type, NativeStringRawPoolContext& strings) {
    prepare_type(type, "ParticleFloating.mvfm", strings);
}
void __fastcall prepare_native_object_particle_type_00af8a30(void*) noexcept {}
void __fastcall prepare_native_tracer_particle_type_00b0a0f0(void*) noexcept {}
void __fastcall clamp_native_tracer_particle_type_00b0a100(void*) noexcept {}
} // namespace bsp
