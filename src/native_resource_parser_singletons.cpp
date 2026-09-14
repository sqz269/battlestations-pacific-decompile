#include "bsp/native_resource_parser_singletons.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource parser singletons require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
} // namespace

namespace {
void* get_parser(NativeResourceParserSingletonContext& context, std::uint32_t initial_profile,
    std::uint32_t primary_profile, std::uint32_t secondary_profile) {
    void* const initial = context.actual_parser_publication;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(word(first_manager, 0x10));
    alignas(4) std::uint32_t guard[2]{0x00ce37fc,
        reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    // Native state0 is armed only after Enter and the depth increment.
    try {
        if (!context.actual_parser_publication) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            if (allocation) {
                word(allocation, 4) = initial_profile;
                word(allocation) = primary_profile;
                word(allocation, 4) = secondary_profile;
            }
            context.actual_parser_publication = allocation;
            void* const current = context.actual_parser_publication;
            void* const captured_secondary = current ? at(current, 4) : nullptr;
            void* const second_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            register_native_singleton_object_00bd0c30(
                second_manager, nullptr, captured_secondary);
        }
        if (section) {
            word(section, 0x18) = word(section, 0x18) - 1u;
            LeaveCriticalSection(section);
        }
    } catch (...) {
        // The published allocation is deliberately retained on registration
        // failure. Original EH has only the captured guard cleanup here.
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_parser_publication;
}

void* delete_parser(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    void* const secondary = primary ? at(primary, 4) : nullptr;
    context.actual_parser_publication = nullptr;
    word(secondary) = 0x00ce3818;
    word(primary) = 0x00cfd7d0u;
    if ((flags & 1u) != 0) singleton_lifetime_free(primary);
    return primary;
}

} // namespace

void* get_native_group_params_parser_00b7e100(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d6306cu, 0x00d63088u, 0x00d63084u);
}
void* delete_native_group_params_parser_00b7dd00(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_group_params_parser_secondary_00b7dae0(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_group_params_parser_00b8f8e0(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d63550u), strings);
}

void* get_native_mesh_parser_00b7e1d0(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d63070u, 0x00d63098u, 0x00d63094u);
}
void* delete_native_mesh_parser_00b7dd40(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_mesh_parser_secondary_00b7db40(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_mesh_parser_00b7eb60(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00ce5fd0u), strings);
}

void* get_native_skined_mesh_parser_00b7e2a0(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d63074u, 0x00d630a8u, 0x00d630a4u);
}
void* delete_native_skined_mesh_parser_00b7dd80(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_skined_mesh_parser_secondary_00b7dba0(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_skined_mesh_parser_00b94170(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d637f8u), strings);
}

void* get_native_matrix_indexed_mesh_parser_00b7e370(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d63078u, 0x00d630b8u, 0x00d630b4u);
}
void* delete_native_matrix_indexed_mesh_parser_00b7ddc0(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_matrix_indexed_mesh_parser_secondary_00b7dc00(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_matrix_indexed_mesh_parser_00b941a0(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d63804u), strings);
}

void* get_native_camera_parser_00b7e460(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d6307cu, 0x00d630c8u, 0x00d630c4u);
}
void* delete_native_camera_parser_00b7de00(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_camera_parser_secondary_00b7dc60(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_camera_parser_00b8b210(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d633d4u), strings);
}

void* get_native_skined_mesh_animation_parser_00b7e530(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00d63080u, 0x00d630d8u, 0x00d630d4u);
}
void* delete_native_skined_mesh_animation_parser_00b7de40(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_skined_mesh_animation_parser_secondary_00b7dcc0(void* secondary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_skined_mesh_animation_parser_00b92a20(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d636ecu), strings);
}

void* NativeDefaultResourceParserNameCalls::type_name(std::uintptr_t target,
    void* parser, void* output) {
    switch (target) {
    case 0x00b8f8e0u: return name_native_group_params_parser_00b8f8e0(parser, output, strings_);
    case 0x00b7eb60u: return name_native_mesh_parser_00b7eb60(parser, output, strings_);
    case 0x00b94170u: return name_native_skined_mesh_parser_00b94170(parser, output, strings_);
    case 0x00b941a0u: return name_native_matrix_indexed_mesh_parser_00b941a0(parser, output, strings_);
    case 0x00b8b210u: return name_native_camera_parser_00b8b210(parser, output, strings_);
    case 0x00b92a20u: return name_native_skined_mesh_animation_parser_00b92a20(parser, output, strings_);
    default: throw std::logic_error("Unbound native parser name target");
    }
}
} // namespace bsp
