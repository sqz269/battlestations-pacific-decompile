#include "bsp/native_particle_type_construction.hpp"
#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + n);
}
template<class T> volatile T& field(const void* p, Word n = 0) noexcept {
    return *static_cast<volatile T*>(at(p, n));
}
void copy_record(void* destination, const void* source) noexcept {
    __asm {
        mov edi, destination
        mov esi, source
        mov ecx, 7
        rep movsd
    }
}
struct ConstructionUnwind {
    void* owner;
    NativeStringRawPoolContext& strings;
    int state = 0;
    ~ConstructionUnwind() noexcept {
        // DF34BC/DF34A4: 2->1 CBB463, 1->0 CBB458, 0->-1 CBB450.
        // A secondary exception during the original unwind terminates.
        if (state >= 2) destroy_native_particle_type_records_00b00f70(at(owner, 0x68));
        if (state >= 1) destroy_native_string_header_0041dd20(at(owner, 8), strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(owner);
    }
};
} // namespace

void* construct_native_particle_type_base_00b01150(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    field<Word>(p) = 0x00ceb130;
    field<Word>(p, 4) = 1;
    field<Word>(p) = 0x00d5ddc0;
    ConstructionUnwind unwind{p, a.strings};
    void* const destination = at(p, 8);
    field<Word>(destination) = 0;
    field<Word>(destination, 4) = 0;
    void* const descriptor = at(p, 0x68);
    field<Word>(descriptor) = 0;
    field<Word>(descriptor, 4) = 0;
    field<Word>(descriptor, 8) = 0;
    unwind.state = 2;
    if (destination != name) {
        resize_native_string_header_0041dd40(destination, a.strings, field<Word>(name), true);
        if (field<Word>(name) != 0) {
            const Word bytes = field<Word>(destination);
            const void* const source = field<void*>(name, 4);
            void* const target = field<void*>(destination, 4);
            if (bytes) std::memmove(target, source, bytes);
        }
    }
    // MOVSS/XORPS only: preserve raw bits, including signaling NaNs and -0.
    // There is no x87 arithmetic or extra float rounding in these bodies.
    const Word one = *a.one_00d7a24c;
    const Word scalar = *a.scalar_00ce3804;
    field<Word>(p, 0x24) = one;
    field<Word>(p, 0x14) = value;
    field<Word>(p, 0x58) = 0;
    Word record[7];
    record[0] = 0;
    record[1] = 0;
    const Word record_scalar = *a.record_scalar_00ce6650;
    field<void*>(p, 0x18) = parent;
    field<std::uint8_t>(p, 0x78) = 1;
    field<Word>(p, 0x1c) = 0;
    field<std::uint8_t>(p, 0x29) = 0;
    field<std::uint8_t>(p, 0x28) = 0;
    field<Word>(p, 0x2c) = 0;
    field<Word>(p, 0x30) = 0;
    field<Word>(p, 0x48) = 0;
    field<Word>(p, 0x34) = 0;
    field<Word>(p, 0x38) = 0;
    field<Word>(p, 0x3c) = 0;
    field<Word>(p, 0x40) = 0;
    field<Word>(p, 0x44) = 0;
    field<Word>(p, 0x74) = 0;
    field<std::uint8_t>(p, 0x4c) = 0;
    field<Word>(p, 0x50) = 0;
    field<Word>(p, 0x54) = 0;
    field<Word>(p, 0x5c) = 0;
    field<std::uint8_t>(p, 0x60) = 0;
    field<std::uint8_t>(p, 0x61) = 0;
    field<std::uint8_t>(p, 0x62) = 1;
    field<std::uint8_t>(p, 0x63) = 0;
    field<std::uint8_t>(p, 0x65) = 0;
    field<Word>(p, 0x20) = scalar;
    record[2] = record_scalar;
    record[3] = record_scalar;
    float inputs[4];
    std::memcpy(inputs, record, sizeof inputs);
    std::uint16_t halves[4];
    a.half_import.convert(halves, inputs, 4);
    std::memcpy(record + 4, halves, sizeof halves);
    record[6] = a.initial_record_stack_word18;
    const Word capacity = field<Word>(descriptor, 8);
    if (field<Word>(descriptor, 4) == capacity) {
        Word growth = capacity + capacity;
        if (static_cast<std::int32_t>(growth) <= 1) growth = 1;
        reserve_native_particle_type_records_00b00c20(descriptor, static_cast<std::int32_t>(growth));
    }
    const Word count = field<Word>(descriptor, 4);
    void* const target = at(field<void*>(descriptor), count * 0x1cu);
    if (target) copy_record(target, record);
    field<Word>(descriptor, 4) = field<Word>(descriptor, 4) + 1u;
    unwind.state = -1;
    return p;
}

void* construct_native_sprite_particle_base_00b08830(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a);
    field<Word>(p, 0x10) = 0;
    field<Word>(p, 0x80) = 0;
    field<Word>(p, 0x84) = 0;
    field<Word>(p, 0x88) = 0;
    field<Word>(p) = 0x00d5dff4;
    return p;
}
void* construct_native_axial_particle_base_00b058e0(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a);
    field<Word>(p, 0x10) = 0;
    field<Word>(p, 0xa0) = 0;
    field<std::uint8_t>(p, 0x80) = 0;
    field<Word>(p, 0x8c) = 0;
    field<Word>(p, 0x90) = 0;
    field<Word>(p) = 0x00d5df30;
    field<Word>(p, 0x84) = 0;
    field<Word>(p, 0x88) = 0;
    return p;
}
void* construct_native_floating_particle_base_00b076f0(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a);
    field<Word>(p, 0x80) = 0;
    field<Word>(p, 0x84) = 0;
    field<Word>(p, 0x88) = 0;
    field<Word>(p) = 0x00d5dfb0;
    field<Word>(p, 0x10) = 1;
    return p;
}
void* construct_native_floating_particle_definition_00b00770(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_floating_particle_base_00b076f0(p, name, value, parent, a);
    field<Word>(p) = 0x00d5dcec;
    return p;
}
void* construct_native_object_particle_definition_00af89e0(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a);
    field<Word>(p) = 0x00d5db00;
    field<Word>(p, 0x8c) = 0;
    field<Word>(p, 0x90) = 0;
    field<Word>(p, 0x94) = 0;
    field<Word>(p, 0x80) = 0;
    field<Word>(p, 0x84) = 0;
    field<Word>(p, 0x10) = 2;
    return p;
}
void* construct_native_tracer_particle_definition_00b0a0b0(void* p, const void* name,
    Word value, void* parent, NativeParticleTypeConstructionContext& a) {
    construct_native_particle_type_base_00b01150(p, name, value, parent, a);
    field<Word>(p) = 0x00d5e048;
    field<Word>(p, 0x98) = 0;
    field<Word>(p, 0x9c) = 0;
    field<Word>(p, 0xa0) = 0;
    field<Word>(p, 0x10) = 3;
    return p;
}
} // namespace bsp
