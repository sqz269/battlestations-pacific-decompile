#include "bsp/native_resource_extra_parser_singletons.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource extra parser singletons require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}

void* get_parser(NativeResourceParserSingletonContext& context,
    std::uint32_t initial_profile, std::uint32_t primary_profile,
    std::uint32_t secondary_profile) {
    void* const initial = context.actual_parser_publication;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(word(first_manager, 0x10));
    alignas(4) std::uint32_t guard[2]{0x00ce37fcu,
        reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
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
        // Native state0 owns only the captured lifetime-manager guard. A
        // published allocation remains published when registration throws.
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_parser_publication;
}

void* delete_parser(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    void* const secondary = primary ? at(primary, 4) : nullptr;
    context.actual_parser_publication = nullptr;
    word(secondary) = 0x00ce3818u;
    word(primary) = 0x00cfd7d0u;
    if ((flags & 1u) != 0) singleton_lifetime_free(primary);
    return primary;
}
} // namespace

void* get_native_animation_channels_parser_00736dd0(
    NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00cfea08u, 0x00cfea38u, 0x00cfea34u);
}
void* delete_native_animation_channels_parser_00737150(void* primary,
    std::uint32_t flags, NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_animation_channels_parser_secondary_00735d90(void* secondary,
    std::uint32_t flags, NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_animation_channels_parser_00b8b050(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d633acu), strings);
}

void* get_native_bone_parser_00736ea0(NativeResourceParserSingletonContext& context) {
    return get_parser(context, 0x00cfea0cu, 0x00cfea48u, 0x00cfea44u);
}
void* delete_native_bone_parser_00737190(void* primary, std::uint32_t flags,
    NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(primary, flags, context);
}
void* delete_native_bone_parser_secondary_00735dc0(void* secondary,
    std::uint32_t flags, NativeResourceParserSingletonContext& context) noexcept {
    return delete_parser(at(secondary, 0xfffffffcu), flags, context);
}
void* name_native_bone_parser_00b8b080(void*, void* output,
    NativeStringRawPoolContext& strings) {
    return construct_native_string_cstring_0041e870(output,
        reinterpret_cast<const char*>(0x00d633c0u), strings);
}

void delete_native_resource_extra_registered_owner(std::uintptr_t profile,
    void* secondary, std::uint32_t flags, NativeResourceExtraParserContexts& contexts) {
    switch (profile) {
    case 0x00cfea34u:
        delete_native_animation_channels_parser_secondary_00735d90(
            secondary, flags, contexts.animation_channels);
        return;
    case 0x00cfea44u:
        delete_native_bone_parser_secondary_00735dc0(
            secondary, flags, contexts.bone);
        return;
    default:
        throw std::logic_error("Unbound native resource extra parser profile");
    }
}

void* NativeResourceExtraParserNameCalls::type_name(std::uintptr_t target,
    void* parser, void* output) {
    switch (target) {
    case 0x00b8b050u:
        return name_native_animation_channels_parser_00b8b050(parser, output, strings_);
    case 0x00b8b080u:
        return name_native_bone_parser_00b8b080(parser, output, strings_);
    default:
        return remaining_.type_name(target, parser, output);
    }
}
} // namespace bsp
