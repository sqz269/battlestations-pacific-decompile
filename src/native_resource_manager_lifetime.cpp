#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_resource_manager_trees.hpp"
#include "bsp/native_resource_tree_cleanup.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
void* at(void* p, std::uint32_t n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
volatile std::uint32_t& word(void* p, std::uint32_t n = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, n));
}
void* volatile& pointer(void* p, std::uint32_t n = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, n));
}
void finish_head(void* tree, void* allocation) noexcept {
    pointer(tree, 4) = allocation;
    *static_cast<volatile unsigned char*>(at(allocation, 0x19)) = 1;
    void* head = pointer(tree, 4);
    pointer(head, 4) = head;
    head = pointer(tree, 4);
    pointer(head) = head;
    head = pointer(tree, 4);
    pointer(head, 8) = head;
    word(tree, 8) = 0;
}
void unwind_members(void* manager, int state, NativeResourceManagerContext& context) noexcept {
    if (state >= 2) destroy_native_resource_cache_tree_00b80ed0(
        at(manager, 0x14), context.strings, context.invalid_parameters);
    if (state >= 1) destroy_native_resource_parser_tree_00b80e90(
        at(manager, 8), context.strings, context.invalid_parameters);
    destroy_native_resource_manager_base_00b7d2c0(manager, context);
}
void register_parser(void* manager, void* volatile& publication,
    void* (*getter)(NativeResourceParserSingletonContext&), NativeResourceManagerContext& context) {
    NativeResourceParserSingletonContext parser{context.actual_lifetime_manager_01090aa0, publication};
    void* const current = getter(parser);
    register_native_resource_type_parser_00b80a50(manager, current, context.strings,
        context.invalid_parameters, context.names);
}
} // namespace

void* delete_native_default_resource_factory_00b7d290(void* factory, std::uint32_t flags) noexcept {
    const bool release = (flags & 1u) != 0;
    word(factory) = 0x00cfd7dcu;
    if (release) singleton_lifetime_free(factory);
    return factory;
}
void NativeDefaultResourceManagerFactoryCalls::delete_factory(std::uintptr_t target,
    void* factory, std::uint32_t flags) {
    if (target != 0x00b7d290u) throw std::logic_error("Unbound resource manager factory deletion target");
    delete_native_default_resource_factory_00b7d290(factory, flags);
}
void destroy_native_resource_manager_base_00b7d2c0(void* manager,
    NativeResourceManagerContext& context) noexcept {
    context.actual_resource_manager_010901c4 = nullptr;
    word(manager) = 0x00ce3818u;
}

void* construct_native_resource_manager_00b81040(void* manager, NativeResourceManagerContext& context) {
    int state = 0;
    try {
        word(manager) = 0x00d63128u;
        void* const factory = singleton_lifetime_allocate({SingletonAllocationKind::object, 4, 4});
        if (factory) word(factory) = 0x00d63060u;
        pointer(manager, 4) = factory;
        finish_head(at(manager, 8), allocate_native_resource_parser_head_storage_00b7e000());
        state = 1;
        finish_head(at(manager, 0x14), allocate_native_resource_cache_head_storage_00b7e050());
        state = 2;
        register_parser(manager, context.parsers.mesh_0109047c, get_native_mesh_parser_00b7e1d0, context);
        register_parser(manager, context.parsers.skined_mesh_01090480, get_native_skined_mesh_parser_00b7e2a0, context);
        register_parser(manager, context.parsers.skined_mesh_animation_0109043c, get_native_skined_mesh_animation_parser_00b7e530, context);
        register_parser(manager, context.parsers.matrix_indexed_mesh_01090484, get_native_matrix_indexed_mesh_parser_00b7e370, context);
        register_parser(manager, context.parsers.camera_010902a0, get_native_camera_parser_00b7e460, context);
        register_parser(manager, context.parsers.group_params_0109033c, get_native_group_params_parser_00b7e100, context);
        return manager;
    } catch (...) {
        unwind_members(manager, state, context);
        throw;
    }
}

void* get_native_resource_manager_004c1400(NativeResourceManagerContext& context) {
    void* const initial = context.actual_resource_manager_010901c4;
    if (initial) return initial;
    void* const first_manager = get_native_singleton_manager_00415350(context.actual_lifetime_manager_01090aa0);
    auto* const section = static_cast<CRITICAL_SECTION*>(pointer(first_manager, 0x10));
    std::uint32_t guard[2]{0x00ce37fcu, reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    try {
        if (!context.actual_resource_manager_010901c4) {
            void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
            void* result;
            try {
                result = allocation ? construct_native_resource_manager_00b81040(allocation, context) : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            context.actual_resource_manager_010901c4 = result;
            void* const second_manager = get_native_singleton_manager_00415350(context.actual_lifetime_manager_01090aa0);
            // Unlike the parser getters, read this publication AFTER the second lookup.
            void* const current = context.actual_resource_manager_010901c4;
            register_native_singleton_object_00bd0c30(second_manager, nullptr, current);
        }
        if (section) {
            word(section, 0x18) = word(section, 0x18) - 1u;
            LeaveCriticalSection(section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_resource_manager_010901c4;
}

void destroy_native_resource_manager_00b80f10(void* manager, NativeResourceManagerContext& context) {
    word(manager) = 0x00d63128u;
    void* const factory = pointer(manager, 4);
    int state = 2;
    try {
        if (factory) {
            const auto target = word(pointer(factory));
            context.factories.delete_factory(target, factory, 1);
        }
        state = 1;
        destroy_native_resource_cache_tree_00b80ed0(at(manager, 0x14), context.strings, context.invalid_parameters);
        state = 0;
        destroy_native_resource_parser_tree_00b80e90(at(manager, 8), context.strings, context.invalid_parameters);
        destroy_native_resource_manager_base_00b7d2c0(manager, context);
    } catch (...) {
        unwind_members(manager, state, context);
        throw;
    }
}
void* delete_native_resource_manager_00b81150(void* manager, std::uint32_t flags,
    NativeResourceManagerContext& context) {
    destroy_native_resource_manager_00b80f10(manager, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(manager);
    return manager;
}

void delete_native_resource_registered_owner(std::uintptr_t profile, void* owner,
    std::uint32_t flags, NativeResourceManagerContext& context) {
    const auto parser = [&](void* volatile& publication, auto deleter) {
        NativeResourceParserSingletonContext binding{context.actual_lifetime_manager_01090aa0, publication};
        deleter(owner, flags, binding);
    };
    switch (profile) {
    case 0x00d63128u: delete_native_resource_manager_00b81150(owner, flags, context); return;
    case 0x00d63084u: parser(context.parsers.group_params_0109033c, delete_native_group_params_parser_secondary_00b7dae0); return;
    case 0x00d63094u: parser(context.parsers.mesh_0109047c, delete_native_mesh_parser_secondary_00b7db40); return;
    case 0x00d630a4u: parser(context.parsers.skined_mesh_01090480, delete_native_skined_mesh_parser_secondary_00b7dba0); return;
    case 0x00d630b4u: parser(context.parsers.matrix_indexed_mesh_01090484, delete_native_matrix_indexed_mesh_parser_secondary_00b7dc00); return;
    case 0x00d630c4u: parser(context.parsers.camera_010902a0, delete_native_camera_parser_secondary_00b7dc60); return;
    case 0x00d630d4u: parser(context.parsers.skined_mesh_animation_0109043c, delete_native_skined_mesh_animation_parser_secondary_00b7dcc0); return;
    default: throw std::logic_error("Unbound resource singleton profile");
    }
}
} // namespace bsp
