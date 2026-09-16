#include "bsp/native_particle_emitter_construction.hpp"

#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emitter construction requires MSVC Win32.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

void* at(const void* owner, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(owner) + offset);
}

template<class T>
T read(const void* owner, Word offset = 0) noexcept {
    T value;
    std::memcpy(&value, at(owner, offset), sizeof value);
    return value;
}

template<class T>
void write(void* owner, Word offset, T value) noexcept {
    std::memcpy(at(owner, offset), &value, sizeof value);
}

struct BaseConstructorUnwind {
    void* owner;
    NativeStringRawPoolContext& strings;
    int state = -1;
    bool active = true;

    ~BaseConstructorUnwind() noexcept {
        if (!active) return;
        // DF2EDC: state1 ->0 CBB008(name+8), state0 ->-1 CBB000(base).
        // A second exception during C++ cleanup terminates, as native FH3 does.
        if (state >= 1)
            destroy_native_string_header_0041dd20(at(owner, 8), strings);
        if (state >= 0)
            destroy_native_ref_counted_base_00bd30f0(owner);
    }
};

void write_profile(void* owner, Word profile) noexcept {
    write(owner, 0, profile);
}

} // namespace

void* construct_native_particle_definition_00afa280(void* owner,
    const void* name, Word word10, Word word70, Word flag14,
    NativeParticleEmitterConstructionContext& context) {
    write_profile(owner, 0x00ceb130);
    write<Word>(owner, 4, 1);
    write_profile(owner, 0x00d5dbc4);

    BaseConstructorUnwind unwind{owner, context.lifetime.strings};
    unwind.state = 0;
    write<Word>(owner, 8, 0);
    write<Word>(owner, 0x0c, 0);
    write<Word>(owner, 0x4c, 0);
    write<Word>(owner, 0x68, 0);
    unwind.state = 1;
    write<std::uint8_t>(owner, 0x7c, 1);

    void* const destination = at(owner, 8);
    if (destination != name) {
        resize_native_string_header_0041dd40(destination,
            context.lifetime.strings, read<Word>(name), true);
        if (read<Word>(name) != 0) {
            const Word bytes = read<Word>(destination);
            void* const target = read<void*>(destination, 4);
            const void* const source = read<const void*>(name, 4);
            if (bytes != 0) std::memmove(target, source, bytes);
        }
    }

    write<std::uint8_t>(owner, 0x14, static_cast<std::uint8_t>(flag14));
    write<Word>(owner, 0x10, word10);
    write<std::uint8_t>(owner, 0x15, 0);
    write<std::uint8_t>(owner, 0x1d, 0);
    write<Word>(owner, 0x24, 0);
    write<Word>(owner, 0x28, 0);
    write<Word>(owner, 0x2c, 0);
    write<Word>(owner, 0x20, 0);
    write<Word>(owner, 0x34, 0);
    write<Word>(owner, 0x70, word70);
    write<std::uint8_t>(owner, 0x1c, 1);
    write<Word>(owner, 0x6c, 0);
    write<Word>(owner, 0x50, 0);
    unwind.active = false;
    return owner;
}

void* construct_native_particle_cone_definition_00b03940(void* owner,
    const void* name, Word word10, Word word70, Word flag14,
    NativeParticleEmitterConstructionContext& context) {
    construct_native_particle_definition_00afa280(
        owner, name, word10, word70, flag14, context);
    write_profile(owner, 0x00d5debc);
    return owner;
}

void* construct_native_particle_sphere_definition_00b02b90(void* owner,
    const void* name, Word word10, Word word70, Word flag14,
    NativeParticleEmitterConstructionContext& context) {
    construct_native_particle_definition_00afa280(
        owner, name, word10, word70, flag14, context);
    write_profile(owner, 0x00d5de88);
    return owner;
}

void* construct_native_particle_smartarea_definition_00b01cb0(void* owner,
    const void* name, Word word10, Word word70, Word flag14,
    NativeParticleEmitterConstructionContext& context) {
    construct_native_particle_definition_00afa280(
        owner, name, word10, word70, flag14, context);
    write_profile(owner, 0x00d5de48);
    return owner;
}

} // namespace bsp
