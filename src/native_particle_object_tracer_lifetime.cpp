#include "bsp/native_particle_object_tracer_lifetime.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Object/Tracer lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
std::int32_t count(const void* p, Word offset) noexcept {
    return static_cast<std::int32_t>(word(p, offset));
}
void* pointer(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void release_parameter(void* owner, Word offset, NativeParticleTypeLifetimeContext& context) {
    void* const captured = pointer(owner, offset);
    if (!captured) return;
    destroy_native_particle_parameter_00affdf0(captured);
    return_native_particle_parameter_00b00090(captured, context.parameter_pool_00f8d344);
    word(owner, offset) = 0;
}
struct Unwind {
    void* owner;
    NativeParticleTypeLifetimeContext& context;
    bool object;
    int state = 1;
    ~Unwind() noexcept {
        if (state < 0) return;
        if (state == 1) {
            if (object) destroy_native_object_particle_model_vector_00af89c0(at(owner, 0x8c));
            else destroy_native_particle_tracer_buffer_00869b10(at(owner, 0x98));
        }
        destroy_native_particle_type_base_00b00fb0(owner, context);
    }
};
} // namespace

void resize_native_object_particle_models_00af83b0(void* vector, std::int32_t wanted) {
    if (wanted > count(vector, 8)) reserve_native_object_particle_models_00af8350(vector, wanted);
    for (Word i = word(vector, 4); static_cast<std::int32_t>(i) < wanted; ++i) {
        void* const cell = at(pointer(vector), i * 4u);
        if (cell) word(cell) = 0;
    }
    while (wanted < count(vector, 4)) word(vector, 4) = word(vector, 4) - 1u;
    word(vector, 4) = static_cast<Word>(wanted);
}

void destroy_native_object_particle_model_vector_00af89c0(void* vector) {
    resize_native_object_particle_models_00af83b0(vector, 0);
    singleton_lifetime_free(pointer(vector));
}

void destroy_native_particle_tracer_buffer_00869b10(void* cell) noexcept {
    void* const captured = pointer(cell);
    if (captured) {
        singleton_lifetime_free(captured);
        word(cell) = 0;
    }
}

void destroy_native_object_particle_type_00af8a40(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    word(owner) = 0x00d5db00;
    Unwind unwind{owner, context, true};
    release_parameter(owner, 0x80, context);
    release_parameter(owner, 0x84, context);
    clear_native_object_particle_models_00af8940(owner);
    unwind.state = 0;
    destroy_native_object_particle_model_vector_00af89c0(at(owner, 0x8c));
    unwind.state = -1;
    destroy_native_particle_type_base_00b00fb0(owner, context);
}

void destroy_native_tracer_particle_type_00b0a4e0(void* owner,
    NativeParticleTypeLifetimeContext& context) {
    word(owner) = 0x00d5e048;
    Unwind unwind{owner, context, false};
    constexpr Word slots[] = {0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xc8, 0xcc,
        0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0x8c, 0x94};
    for (const Word offset : slots) release_parameter(owner, offset, context);
    destroy_native_particle_tracer_buffer_00869b10(at(owner, 0x98));
    unwind.state = -1;
    destroy_native_particle_type_base_00b00fb0(owner, context);
}

void* delete_native_object_particle_type_00af8bb0(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_object_particle_type_00af8a40(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_tracer_particle_type_00b0a820(void* owner, Word flags,
    NativeParticleTypeLifetimeContext& context) {
    destroy_native_tracer_particle_type_00b0a4e0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
