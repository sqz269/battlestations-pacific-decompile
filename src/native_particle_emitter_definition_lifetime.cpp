#include "bsp/native_particle_emitter_definition_lifetime.hpp"
#include "bsp/native_particle_emitter_derived_lifetimes.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_particle_three_lifetimes.hpp"
#include "bsp/native_particle_object_tracer_lifetime.hpp"
#include "bsp/native_particle_layer_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emitter lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}
bool has_known_terminal(Word profile) noexcept {
    switch (profile) {
    case 0x00d5ddc0: case 0x00d5db00: case 0x00d5df30: case 0x00d5dcc0:
    case 0x00d5dfb0: case 0x00d5dcec: case 0x00d5dff4: case 0x00d5dd18:
    case 0x00d5e048: case 0x00d5dc38: case 0x00d5dbc4:
    case 0x00d5debc: case 0x00d5de88: case 0x00d5de48: return true;
    default: return false;
    }
}
void invoke_current_scalar(void* child, NativeParticleTypeLifetimeContext& context) {
    // The captured slot0 is BD30E0. Its actual body reloads the table, pushes
    // flags1, and calls slot4. Preserve that second current-table read.
    const Word profile = field<Word>(child);
    switch (profile) {
    case 0x00d5ddc0: delete_native_particle_type_base_00b01130(child, 1, context); return;
    case 0x00d5db00: delete_native_object_particle_type_00af8bb0(child, 1, context); return;
    case 0x00d5df30: delete_native_axial_particle_base_00b05ce0(child, 1, context); return;
    case 0x00d5dcc0: delete_native_axial_particle_definition_00b008c0(child, 1, context); return;
    case 0x00d5dfb0: delete_native_floating_particle_base_00b07c60(child, 1, context); return;
    case 0x00d5dcec: delete_native_floating_particle_definition_00b008e0(child, 1, context); return;
    case 0x00d5dff4: delete_native_sprite_particle_base_00b089c0(child, 1, context); return;
    case 0x00d5dd18: delete_native_sprite_particle_definition_00b00900(child, 1, context); return;
    case 0x00d5e048: delete_native_tracer_particle_type_00b0a820(child, 1, context); return;
    case 0x00d5dc38: scalar_delete_native_particle_layer_00aface0(child, 1, context.strings); return;
    case 0x00d5dbc4: delete_native_particle_definition_00afa350(child, 1, context); return;
    case 0x00d5debc: delete_native_particle_cone_definition_00b03b40(child, 1, context); return;
    case 0x00d5de88: delete_native_particle_sphere_definition_00b02fb0(child, 1, context); return;
    case 0x00d5de48: delete_native_particle_smartarea_definition_00b01ea0(child, 1, context); return;
    default:
        using Scalar = void* (__thiscall*)(void*, Word);
        field<Scalar>(reinterpret_cast<void*>(profile), 4)(child, 1);
    }
}
void release_rows(void* owner, Word first, Word count, NativeParticleTypeLifetimeContext& context) {
    Word index = 0;
    if (field<Signed>(owner, count) <= 0) return;
    void* cursor = at(owner, first);
    do {
        void* const child = field<void*>(cursor);
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(child, 4))) == 0) {
            const Word profile = field<Word>(child);
            if (has_known_terminal(profile)) invoke_current_scalar(child, context);
            else {
                using Terminal = void (__thiscall*)(void*);
                const auto terminal = field<Terminal>(reinterpret_cast<void*>(profile));
                terminal(child);
            }
        }
        ++index;
        cursor = at(cursor, 4);
    } while (static_cast<Signed>(index) < field<Signed>(owner, count));
}
struct EmitterUnwind {
    void* owner;
    NativeParticleTypeLifetimeContext& context;
    int state = 1;
    ~EmitterUnwind() noexcept {
        // DF2EA8: state1 ->0 CBAFE8(name); state0 ->-1 CBAFE0(base).
        if (state >= 1) destroy_native_string_header_0041dd20(at(owner, 8), context.strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(owner);
    }
};
} // namespace

void destroy_native_particle_definition_00afa100(void* owner, NativeParticleTypeLifetimeContext& context) {
    field<Word>(owner) = 0x00d5dbc4;
    void* captured = field<void*>(owner, 0x24);
    EmitterUnwind unwind{owner, context};
    for (Word offset : {0x24u, 0x28u, 0x2cu, 0x20u, 0x30u, 0x34u, 0x38u}) {
        if (offset != 0x24) captured = field<void*>(owner, offset);
        if (captured) {
            destroy_native_particle_parameter_00affdf0(captured);
            return_native_particle_parameter_00b00090(captured, context.parameter_pool_00f8d344);
            field<Word>(owner, offset) = 0;
        }
    }
    release_rows(owner, 0x3c, 0x4c, context);
    release_rows(owner, 0x54, 0x68, context);
    void* const name = field<void*>(owner, 0x0c);
    unwind.state = 0;
    if (name) {
        const Word bytes = field<Word>(owner, 8) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.strings.actual_published_01090aa8, context.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, name, bytes,
            context.strings.actual_small_returns_disabled_01090aa4);
    }
    unwind.state = -1;
    destroy_native_ref_counted_base_00bd30f0(owner);
}
void* delete_native_particle_definition_00afa350(void* owner, Word flags, NativeParticleTypeLifetimeContext& context) {
    destroy_native_particle_definition_00afa100(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
