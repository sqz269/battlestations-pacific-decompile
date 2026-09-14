#include "bsp/native_renderer_destructor.hpp"
#include "bsp/native_renderer_base_lifetime.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_renderer_control_worker.hpp"
#include "bsp/native_renderer_cache_clear.hpp"
#include "bsp/native_renderer_query_terminal.hpp"
#include "bsp/native_renderer_lua_owner.hpp"
#include "bsp/native_shader_state_definitions.hpp"
#include "bsp/native_system_registry_raw_terminal.hpp"
#include "bsp/native_renderer_parent_member_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/native_renderer_record_array_cleanup.hpp"
#include "bsp/native_renderer_remaining_array_cleanup.hpp"
#include "bsp/native_renderer_record_destroy.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/native_renderer_capability_array_cleanup.hpp"
#include "bsp/native_renderer_capability_nested_arrays.hpp"
#include "bsp/native_vertex_declaration_registry_lifetime.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/native_renderer_cache_cleanup.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>

#undef InterlockedDecrement
extern "C" __declspec(dllimport) LONG WINAPI InterlockedDecrement(volatile LONG*);

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4);
void* pointer(U bits) noexcept { return reinterpret_cast<void*>(bits); }
U address(const void* value) noexcept { return reinterpret_cast<U>(value); }
void* at(void* owner, U offset) noexcept { return pointer(address(owner) + offset); }
U word(const volatile void* base, U byte_offset = 0) noexcept {
    U value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, U byte_offset, U value = 0) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
I signed_bits(U value) noexcept {
    I result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void require(bool value) noexcept { if (!value) std::terminate(); }
void expect(const volatile U* profile, U offset, U target) noexcept {
    require(word(profile, offset) == target);
}

// Each final-zero invocation selects current slot0 before full BD30E0 rereads
// the current owner/profile for slot4. Original numeric tokens never execute.
const volatile U* intrusive_profile(U identity, NativeRendererDestructorContext& c) {
    const auto& material = c.binding_cache.actual_material_states;
    const auto& physical = c.binding_reset.actual_vertex.actual_logical_owner.actual_physical_profiles;
    switch (identity) {
    case 0x00d62ad0: return c.query_terminal.actual_query_profile_00d62ad0;
    case 0x00d619a0: return c.binding_cache.actual_frame_target.actual_surface_profile_00d619a0;
    case 0x00d61a2c: return material.actual_material_states.actual_render_profile_00d61a2c;
    case 0x00d61a34: return material.actual_material_states.actual_sampler_profile_00d61a34;
    case 0x00d61a3c: return material.actual_third_profile_00d61a3c;
    case 0x00d61e10: return physical.private_index_00d61e10;
    case 0x00d61e34: return physical.private_vertex_00d61e34;
    case 0x00d61e58: return physical.pooled_index_00d61e58;
    case 0x00d61e7c: return physical.pooled_vertex_00d61e7c;
    default: std::terminate();
    }
}
void release_zero(void* owner, NativeRendererDestructorContext& c) {
    expect(intrusive_profile(word(owner), c), 0, 0x00bd30e0);
    // BD30E0's explicit null test occurs after the caller's profile access.
    if (!owner) return;
    const U identity = word(owner);
    const volatile U* const table = intrusive_profile(identity, c);
    auto& physical = c.binding_reset.actual_vertex.actual_logical_owner.actual_physical;
    switch (identity) {
    case 0x00d62ad0:
        expect(table, 4, 0x00b5fe40);
        (void)delete_native_renderer_query_00b5fe40(owner, 1, c.query_terminal); return;
    case 0x00d619a0:
        expect(table, 4, 0x00b3f5b0);
        (void)delete_native_surface_00b3f5b0(*static_cast<NativeSurfaceOwnerStorage*>(owner),
            1, c.binding_cache.actual_frame_target.actual_surface_context); return;
    case 0x00d61a2c:
        expect(table, 4, 0x00b422f0);
        (void)delete_native_material_render_states_00b422f0(
            static_cast<NativeMaterialStateOwnerStorage*>(owner), 1); return;
    case 0x00d61a34:
        expect(table, 4, 0x00b42310);
        (void)delete_native_material_sampler_states_00b42310(
            static_cast<NativeMaterialStateOwnerStorage*>(owner), 1); return;
    case 0x00d61a3c:
        expect(table, 4, 0x00b42330);
        (void)delete_native_material_third_states_00b42330(
            static_cast<NativeMaterialStateOwnerStorage*>(owner), 1); return;
    case 0x00d61e10:
        expect(table, 4, 0x00b4bb20);
        (void)delete_native_private_index_buffer_00b4bb20(owner, 1, physical); return;
    case 0x00d61e34:
        expect(table, 4, 0x00b4bb40);
        (void)delete_native_private_vertex_buffer_00b4bb40(owner, 1, physical); return;
    case 0x00d61e58:
        expect(table, 4, 0x00b4c210);
        (void)delete_native_pooled_index_buffer_00b4c210(owner, 1, physical); return;
    case 0x00d61e7c:
        expect(table, 4, 0x00b4c230);
        (void)delete_native_pooled_vertex_buffer_00b4c230(owner, 1, physical); return;
    default: std::terminate();
    }
}
void release_cell(void* cell, NativeRendererDestructorContext& c) {
    void* const captured = pointer(word(cell));
    if (!captured) return;
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(captured, 4))) == 0)
        release_zero(captured, c);
    put(cell, 0); // Captured cell, even if the callback replaced its array.
}
void pop_owners(void* header, NativeRendererDestructorContext& c) {
    while (word(header, 4) != 0) {
        const U count = word(header, 4);
        const U base = word(header);
        release_cell(pointer(base + count * 4u - 4u), c);
        const U current_count = word(header, 4);
        if (current_count != 0) put(header, 4, current_count - 1u);
    }
}
void unlink_model(void* owner, NativeRendererDestructorContext& c) noexcept {
    // Reuse the application's established canonical binding. No new node map,
    // shadow transform or diagnostic GeneratedModelLifetime is admitted here.
    auto* const reference = dynamic_cast<NativeModelReference*>(
        c.actual_nodes.attachments.find_actual_node(address(owner)));
    require(reference != nullptr);
    auto& model = reference->model_owner();
    require(address(&model.storage.node) == address(owner));
    require(&model.environment.nodes == &c.actual_nodes);
    require(c.actual_nodes.uses_raw_name_pool() && model.environment.actual_names != nullptr);
    auto& names = c.actual_nodes.require_raw_name_pool();
    require(&names.actual_published_01090aa8 == &c.string_pool_01090aa8 &&
        &names.actual_small_returns_disabled_01090aa4 == &c.small_returns_disabled_01090aa4 &&
        &names.actual_manager_publication_01090aa0 == &c.singleton_manager_01090aa0);
    unlink_and_release_render_model_00b6dfa0(*reference);
}
using ComReference = ULONG (STDMETHODCALLTYPE*)(void*);
void com_release(void* captured) {
    reinterpret_cast<ComReference>(word(pointer(word(captured)), 8))(captured);
}
void com_pair(void* captured) {
    if (!captured) return;
    reinterpret_cast<ComReference>(word(pointer(word(captured)), 4))(captured);
    com_release(captured); // Reload this SAME receiver's current table after AddRef.
}

// Inline parent reserve-to-one: capacity was captured BEFORE the EH state
// change. No second capacity read, extra resize entry, pointee destruction or
// normal-header cleanup wrapper is inserted. Both strides copy DWORDs only.
void reserve_one(void* header, U stride) {
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, stride, stride});
    U destination = address(replacement);
    U index = 0;
    while (signed_bits(index) < signed_bits(word(header, 4))) {
        if (destination) {
            const U source = word(header) + index * stride;
            put(pointer(destination), 0, word(pointer(source)));
            if (stride == 8) put(pointer(destination), 4, word(pointer(source), 4));
        }
        ++index;
        destination += stride;
    }
    singleton_lifetime_free(pointer(word(header)));
    put(header, 0, address(replacement));
    put(header, 8, 1);
}
void decrement_count(void* header) noexcept {
    while (signed_bits(word(header, 4)) > 0) put(header, 4, word(header, 4) - 1u);
}
void inline_dwords(void* header, I captured_capacity, bool capture_before_zero) {
    if (captured_capacity < 0) reserve_one(header, 4);
    const U count = word(header, 4);
    if (signed_bits(count) < 0) {
        U offset = count * 4u;
        do {
            const U destination = word(header) + offset;
            if (destination) put(pointer(destination), 0);
            offset += 4u;
        } while (signed_bits(offset) < 0); // JS is on the WRAPPED byte offset.
    }
    decrement_count(header);
    if (capture_before_zero) {
        void* const captured = pointer(word(header));
        put(header, 4);
        singleton_lifetime_free(captured);
    } else {
        put(header, 4);
        singleton_lifetime_free(pointer(word(header)));
    }
}
using RecordReserve = void (__fastcall*)(void*, U, I);
void inline_records(void* header, I captured_capacity, RecordReserve reserve) {
    if (captured_capacity < 0) reserve(header, 0, 0);
    decrement_count(header);
    put(header, 4);
    singleton_lifetime_free(pointer(word(header)));
}

void unwind_parent(void* owner, I state, void* captured_nested,
    NativeRendererDestructorContext& c, NativeRendererBaseContext& base,
    NativeRendererWorkerLifetimeContext& worker) noexcept {
    switch (state) {
    case 25:
        destroy_native_capability_dwords_00b29e40(at(captured_nested, 0x44)); state = 18; break;
    case 26:
        destroy_native_effect_record_array_00b317c0(at(captured_nested, 4), c.effect_cleanup.actual_cache);
        state = 8; break;
    case 27:
        destroy_native_declaration_records_00b316a0(at(captured_nested, 4), c.declaration_cleanup.cache);
        state = 6; break;
    case 28: destroy_native_renderer_primary_base_00b33d90(owner); return;
    default: break;
    }
    switch (state) {
    case 24: destroy_native_renderer_worker_00b5e2f0(at(owner, 0x1d2c), &worker); [[fallthrough]];
    case 23: destroy_native_renderer_records40_00b29c60(at(owner, 0x1d18)); [[fallthrough]];
    case 22: destroy_native_renderer_records24_00b29c20(at(owner, 0x1d0c)); [[fallthrough]];
    case 21: destroy_native_renderer_records20_00b29be0(at(owner, 0x1d00)); [[fallthrough]];
    case 20: destroy_native_renderer_records16_00b29ba0(at(owner, 0x1cf4)); [[fallthrough]];
    case 19: destroy_native_renderer_capabilities_thunk_00b2f700(at(owner, 0x1b18)); [[fallthrough]];
    case 18: destroy_native_renderer_array_00b280f0(at(owner, 0x1b0c)); [[fallthrough]];
    case 17: destroy_native_renderer_array_00737bf0(at(owner, 0x1b00)); [[fallthrough]];
    case 16: destroy_native_renderer_array_00b280d0(at(owner, 0x1af4)); [[fallthrough]];
    case 15: destroy_native_renderer_array_00b280b0(at(owner, 0x1ae8)); [[fallthrough]];
    case 14: destroy_native_renderer_third_state_cache_00b28090(at(owner, 0x1adc)); [[fallthrough]];
    case 13: destroy_native_renderer_pixel_shader_registry_00b29b80(at(owner, 0x1ad0)); [[fallthrough]];
    case 12: destroy_native_renderer_vertex_shader_registry_00b29b60(at(owner, 0x1ac4)); [[fallthrough]];
    case 11: destroy_native_renderer_index_pointers_00b29b40(at(owner, 0x1ab8)); [[fallthrough]];
    case 10: destroy_native_renderer_vertex_pointers_00b29b20(at(owner, 0x1aac)); [[fallthrough]];
    case 9: destroy_native_effect_registry_thunk_00b32200(at(owner, 0x1a98), c.effect_cleanup); [[fallthrough]];
    case 8: destroy_native_texture_resource_cache_00b32370(at(owner, 0x1a74), c.texture_cleanup); [[fallthrough]];
    case 7: destroy_native_declaration_registry_00b32030(at(owner, 0x1a60), c.declaration_cleanup); [[fallthrough]];
    case 6: destroy_native_embedded_tracked_section_00402f70(at(owner, 0x19f4)); [[fallthrough]];
    case 5: destroy_native_renderer_index_pointers_00b29b40(at(owner, 0x19c4)); [[fallthrough]];
    case 4: destroy_native_renderer_vertex_pointers_00b29b20(at(owner, 0x19b0)); [[fallthrough]];
    case 3: destroy_native_renderer_query_pointers_00b27f50(at(owner, 0x19a0)); [[fallthrough]];
    case 2: destroy_native_renderer_dword_array_0086ae00(at(owner, 0x28)); [[fallthrough]];
    case 1: destroy_native_renderer_resolution_pairs_008d4e60(at(owner, 0x1c)); [[fallthrough]];
    case 0: destroy_native_renderer_base_00b284e0(owner, base); [[fallthrough]];
    case -1: break;
    default: std::terminate();
    }
}
} // namespace

void __fastcall destroy_native_renderer_00b32920(void* owner, NativeRendererDestructorContext* context) {
    auto& c = *context;
    NativeRendererBaseContext base{c.current_renderer_00f8d394, c.singleton_manager_01090aa0};
    NativeStringRawPoolContext raw_strings{c.string_pool_01090aa8,
        c.small_returns_disabled_01090aa4, c.singleton_manager_01090aa0};
    ActualNativeStringPoolStorage strings(c.string_pool_01090aa8,
        c.small_returns_disabled_01090aa4, c.singleton_manager_01090aa0);
    NativeRendererLuaOwnerContext lua{c.singleton_manager_01090aa0,
        c.lua_owner_00f8d434, strings, c.lua_bootstrap};
    NativeSystemConstantRegistryRawContext system{c.system_constants_0108fe94, raw_strings};
    NativeRendererWorkerLifetimeContext worker{&strings};
    volatile I state = -1;
    void* captured_nested = nullptr;
    try {
        put(owner, 0, 0x00d5f0a8); // B32942
        put(owner, 0xc, 0x00d5f0a4);
        state = 24;
        critical_section_destroy_owned_0041cc80(*static_cast<TrackedCriticalSection**>(at(owner, 0x199c)));
        if (void* const control = pointer(word(owner, 0x1970))) {
            require(word(control) == 0x00d5f1f0);
            expect(c.profiles.control_00d5f1f0, 0, 0x00b33c00);
            (void)delete_native_renderer_control_worker_00b33c00(
                static_cast<NativeRendererControlWorkerStorage*>(control), 1);
            put(owner, 0x1970);
        }
        reset_native_renderer_bindings_00b24bf0(owner, c.binding_reset);
        U index = 0;
        while (index < word(owner, 0x19a4)) {
            const U base_address = word(owner, 0x19a0);
            release_cell(pointer(base_address + index * 4u), c);
            ++index;
        }
        clear_native_renderer_binding_cache_00b241c0(at(owner, 0x34), c.binding_cache);
        for (U offset = 0x197c; offset != 0x198c; offset += 4u)
            release_cell(at(owner, offset), c);
        release_cell(at(owner, 0x198c), c);
        pop_owners(at(owner, 0x1adc), c);
        pop_owners(at(owner, 0x1af4), c);
        pop_owners(at(owner, 0x1ae8), c);
        for (U offset = 0x19e0; offset != 0x19ec; offset += 4u) {
            if (void* const model = pointer(word(owner, offset))) {
                unlink_model(model, c);
                put(owner, offset);
            }
        }
        flush_native_declaration_registry_00b31630(at(owner, 0x1a60), c.declaration_cleanup);
        clear_native_effect_registry_00b31750(at(owner, 0x1a98), c.effect_cleanup);
        clear_native_renderer_resource_cache_00b316c0(at(owner, 0x1a74), c.texture_cleanup);
        if (auto* const definitions = c.state_definitions_0108fe90) {
            const U profile = word(definitions);
            if (profile == 0x00d62260) {
                expect(c.profiles.state_00d62260, 0, 0x00b59e50);
                (void)delete_native_shader_state_definitions_00b59e50(definitions,
                    c.state_definitions_0108fe90, SoundLifetimeAccess(c.singleton_manager_01090aa0), strings, 1);
            } else {
                require(profile == 0x00d621ec);
                expect(c.profiles.state_base_00d621ec, 0, 0x00b56750);
                (void)delete_native_shader_state_definition_base_00b56750(definitions,
                    c.state_definitions_0108fe90, SoundLifetimeAccess(c.singleton_manager_01090aa0), 1);
            }
        }
        if (auto* const constants = static_cast<NativeSystemConstantRegistryStorage*>(c.system_constants_0108fe94)) {
            const U profile = word(constants);
            if (profile == 0x00d62a3c) {
                expect(c.profiles.system_00d62a3c, 0, 0x00b5df70);
                (void)delete_native_system_constant_registry_00b5df70(constants, system, 1);
            } else {
                require(profile == 0x00d626f4);
                expect(c.profiles.system_base_00d626f4, 0, 0x00b5bb20);
                (void)delete_native_system_constant_base_00b5bb20(constants, system, 1);
            }
        }
        if (auto* const lua_owner = static_cast<NativeRendererLuaOwnerStorage*>(c.lua_owner_00f8d434)) {
            const U profile = word(lua_owner);
            if (profile == 0x00d5e5a8) {
                expect(c.profiles.lua_00d5e5a8, 0, 0x00b1bc50);
                (void)scalar_delete_native_renderer_lua_owner_00b1bc50(lua_owner, 1, lua);
            } else {
                require(profile == 0x00d5e5a4);
                expect(c.profiles.lua_base_00d5e5a4, 0, 0x00b1bb70);
                (void)scalar_delete_native_renderer_lua_base_00b1bb70(lua_owner, 1, lua);
            }
        }
        com_pair(pointer(word(owner, 0x1a10))); // B32BF5
        com_pair(pointer(word(owner, 0x1990))); // B32C0F
        com_release(pointer(word(owner, 0x1a10))); // B32C29, no null guard
        com_pair(pointer(word(owner, 0x1a10))); // B32C37
        com_pair(pointer(word(owner, 0x1990))); // B32C51
        com_release(pointer(word(owner, 0x1990))); // B32C6B, no null guard
        com_pair(pointer(word(owner, 0x1990))); // B32C79
        com_pair(pointer(word(owner, 0x1a10))); // B32C93
        release_cell(at(owner, 0x1974), c);
        release_cell(at(owner, 0x1978), c);
        com_pair(pointer(word(owner, 0x1990))); // B32CF9
        com_pair(pointer(word(owner, 0x1a10))); // B32D13
        state = 23;
        destroy_native_renderer_worker_00b5e2f0(at(owner, 0x1d2c), &worker);
        I capacity = signed_bits(word(owner, 0x1d20)); state = 22;
        inline_records(at(owner, 0x1d18), capacity, reserve_native_renderer_records40_00b22a70);
        capacity = signed_bits(word(owner, 0x1d14)); state = 21;
        inline_records(at(owner, 0x1d0c), capacity, reserve_native_renderer_records24_00b229d0);
        capacity = signed_bits(word(owner, 0x1d08)); state = 20;
        inline_records(at(owner, 0x1d00), capacity, reserve_native_renderer_records20_00b22940);
        capacity = signed_bits(word(owner, 0x1cfc)); state = 19;
        inline_records(at(owner, 0x1cf4), capacity, reserve_native_renderer_records16_00b228b0);
        captured_nested = at(owner, 0x1b18); state = 25;
        resize_native_capability_headers_00b2ae20(at(captured_nested, 0x50), 0, 0);
        singleton_lifetime_free(pointer(word(captured_nested, 0x50)));
        capacity = signed_bits(word(captured_nested, 0x4c)); state = 18;
        inline_dwords(at(captured_nested, 0x44), capacity, false);
        capacity = signed_bits(word(owner, 0x1b14)); state = 17;
        inline_dwords(at(owner, 0x1b0c), capacity, true);
        capacity = signed_bits(word(owner, 0x1b08)); state = 16;
        inline_dwords(at(owner, 0x1b00), capacity, true);
        capacity = signed_bits(word(owner, 0x1afc)); state = 15;
        inline_dwords(at(owner, 0x1af4), capacity, true);
        capacity = signed_bits(word(owner, 0x1af0)); state = 14;
        inline_dwords(at(owner, 0x1ae8), capacity, true);
        capacity = signed_bits(word(owner, 0x1ae4)); state = 13;
        inline_dwords(at(owner, 0x1adc), capacity, true);
        capacity = signed_bits(word(owner, 0x1ad8)); state = 12;
        inline_dwords(at(owner, 0x1ad0), capacity, true);
        capacity = signed_bits(word(owner, 0x1acc)); state = 11;
        inline_dwords(at(owner, 0x1ac4), capacity, true);
        capacity = signed_bits(word(owner, 0x1ac0)); state = 10;
        inline_dwords(at(owner, 0x1ab8), capacity, true);
        capacity = signed_bits(word(owner, 0x1ab4)); state = 9;
        inline_dwords(at(owner, 0x1aac), capacity, true);
        captured_nested = at(owner, 0x1a98);
        put(captured_nested, 0, 0x00d5f04c); state = 26;
        clear_native_effect_registry_00b31750(captured_nested, c.effect_cleanup);
        state = 8;
        resize_native_effect_records_00b30410(at(captured_nested, 4), 0, c.effect_cleanup.actual_cache);
        c.effect_cleanup.actual_cache.free_array_00bf6989(pointer(word(captured_nested, 4)));
        state = 7;
        destroy_native_texture_resource_cache_00b32370(at(owner, 0x1a74), c.texture_cleanup);
        captured_nested = at(owner, 0x1a60);
        put(captured_nested, 0, 0x00d5f024); state = 27;
        flush_native_declaration_registry_00b31630(captured_nested, c.declaration_cleanup);
        state = 6;
        resize_native_declaration_records_00b30270(at(captured_nested, 4), 0, c.declaration_cleanup.cache);
        c.declaration_cleanup.cache.free_array_00bf6989(pointer(word(captured_nested, 4)));
        // State6 remains armed through both calls, matching the native inline body.
        destroy_native_embedded_tracked_section_00402f70(at(owner, 0x19f4));
        capacity = signed_bits(word(owner, 0x19cc)); state = 4;
        inline_dwords(at(owner, 0x19c4), capacity, true);
        capacity = signed_bits(word(owner, 0x19b8)); state = 3;
        inline_dwords(at(owner, 0x19b0), capacity, true);
        capacity = signed_bits(word(owner, 0x19a8)); state = 2;
        inline_dwords(at(owner, 0x19a0), capacity, true);
        capacity = signed_bits(word(owner, 0x30)); state = 1;
        inline_dwords(at(owner, 0x28), capacity, true);
        capacity = signed_bits(word(owner, 0x24)); state = 0;
        if (capacity < 0) reserve_one(at(owner, 0x1c), 8);
        decrement_count(at(owner, 0x1c));
        void* const pairs = pointer(word(owner, 0x1c));
        put(owner, 0x20);
        singleton_lifetime_free(pairs);
        put(owner, 0, 0x00d5e628);
        put(owner, 0xc, 0x00d5e76c);
        state = 28;
        destroy_native_renderer_singleton_subobject_00b25fe0(at(owner, 0xc), base);
        state = -1;
        destroy_native_renderer_primary_base_00b33d90(owner);
    } catch (...) {
        unwind_parent(owner, state, captured_nested, c, base, worker);
        throw;
    }
}

void* __fastcall delete_native_renderer_00b339f0(void* owner,
    NativeRendererDestructorContext* context, U flags) {
    destroy_native_renderer_00b32920(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* __fastcall delete_native_renderer_secondary_00b32900(void* secondary,
    NativeRendererDestructorContext* context, U flags) {
    return delete_native_renderer_00b339f0(at(secondary, 0xfffffff4u), context, flags);
}
} // namespace bsp
