#include "bsp/native_game_resource_parsers.hpp"

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
#error Native game resource parser singletons require MSVC Win32.
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


void* get_native_geom_mesh_parser_00716fe0(NativeResourceParserSingletonContext& c) {return get_parser(c,0x00cfd7e4u,0x00cfd800u,0x00cfd7fcu);}
void* delete_native_geom_mesh_parser_00716380(void* owner,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(owner,flags,c);}
void* delete_native_geom_mesh_parser_secondary_007161c0(void* secondary,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(at(secondary,0xfffffffcu),flags,c);}
void* name_native_geom_mesh_parser_007258f0(void*,void* output,NativeStringRawPoolContext& strings) {return construct_native_string_cstring_0041e870(output,reinterpret_cast<const char*>(0x00cfdbccu),strings);}

void* get_native_convex_object_parser_007170b0(NativeResourceParserSingletonContext& c) {return get_parser(c,0x00cfd7e8u,0x00cfd810u,0x00cfd80cu);}
void* delete_native_convex_object_parser_007163c0(void* owner,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(owner,flags,c);}
void* delete_native_convex_object_parser_secondary_00716220(void* secondary,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(at(secondary,0xfffffffcu),flags,c);}
void* name_native_convex_object_parser_006fb000(void*,void* output,NativeStringRawPoolContext& strings) {return construct_native_string_cstring_0041e870(output,reinterpret_cast<const char*>(0x00cfb6d8u),strings);}

void* get_native_note_parser_00717180(NativeResourceParserSingletonContext& c) {return get_parser(c,0x00cfd7ecu,0x00cfd820u,0x00cfd81cu);}
void* delete_native_note_parser_00716400(void* owner,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(owner,flags,c);}
void* delete_native_note_parser_secondary_00716280(void* secondary,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(at(secondary,0xfffffffcu),flags,c);}
void* name_native_note_parser_007186e0(void*,void* output,NativeStringRawPoolContext& strings) {return construct_native_string_cstring_0041e870(output,reinterpret_cast<const char*>(0x00cfd8b4u),strings);}

void* get_native_zone_desc_parser_00717250(NativeResourceParserSingletonContext& c) {return get_parser(c,0x00cfd7f0u,0x00cfd830u,0x00cfd82cu);}
void* delete_native_zone_desc_parser_00716440(void* owner,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(owner,flags,c);}
void* delete_native_zone_desc_parser_secondary_007162e0(void* secondary,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(at(secondary,0xfffffffcu),flags,c);}
void* name_native_zone_desc_parser_007187e0(void*,void* output,NativeStringRawPoolContext& strings) {return construct_native_string_cstring_0041e870(output,reinterpret_cast<const char*>(0x00cfd8c0u),strings);}

void* get_native_aux_parser_00717320(NativeResourceParserSingletonContext& c) {return get_parser(c,0x00cfd7f4u,0x00cfd840u,0x00cfd83cu);}
void* delete_native_aux_parser_00716480(void* owner,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(owner,flags,c);}
void* delete_native_aux_parser_secondary_00716340(void* secondary,std::uint32_t flags,NativeResourceParserSingletonContext& c) noexcept {return delete_parser(at(secondary,0xfffffffcu),flags,c);}
void* name_native_aux_parser_00718760(void*,void* output,NativeStringRawPoolContext& strings) {return construct_native_string_cstring_0041e870(output,reinterpret_cast<const char*>(0x00cfd8bcu),strings);}

void register_native_game_resource_parsers_00717e80(NativeGameResourceParsersContext& c) {
    auto& m=c.manager;
    {
        void* const manager=get_native_resource_manager_004c1400(m);
        void* const parser=get_native_aux_parser_00717320(c.parsers.aux);
        (void)register_native_resource_type_parser_00b80a50(manager,parser,m.strings,m.invalid_parameters,m.names);
    }
    {
        void* const manager=get_native_resource_manager_004c1400(m);
        void* const parser=get_native_zone_desc_parser_00717250(c.parsers.zone_desc);
        (void)register_native_resource_type_parser_00b80a50(manager,parser,m.strings,m.invalid_parameters,m.names);
    }
    {
        void* const manager=get_native_resource_manager_004c1400(m);
        void* const parser=get_native_note_parser_00717180(c.parsers.note);
        (void)register_native_resource_type_parser_00b80a50(manager,parser,m.strings,m.invalid_parameters,m.names);
    }
    {
        void* const manager=get_native_resource_manager_004c1400(m);
        void* const parser=get_native_geom_mesh_parser_00716fe0(c.parsers.geom_mesh);
        (void)register_native_resource_type_parser_00b80a50(manager,parser,m.strings,m.invalid_parameters,m.names);
    }
    {
        void* const manager=get_native_resource_manager_004c1400(m);
        void* const parser=get_native_convex_object_parser_007170b0(c.parsers.convex_object);
        (void)register_native_resource_type_parser_00b80a50(manager,parser,m.strings,m.invalid_parameters,m.names);
    }
}
void delete_native_game_resource_registered_owner(std::uintptr_t profile,void* secondary,std::uint32_t flags,NativeGameResourceParserContexts& c) {
    switch(profile) {
    case 0x00cfd7fcu:delete_native_geom_mesh_parser_secondary_007161c0(secondary,flags,c.geom_mesh);return;
    case 0x00cfd80cu:delete_native_convex_object_parser_secondary_00716220(secondary,flags,c.convex_object);return;
    case 0x00cfd81cu:delete_native_note_parser_secondary_00716280(secondary,flags,c.note);return;
    case 0x00cfd82cu:delete_native_zone_desc_parser_secondary_007162e0(secondary,flags,c.zone_desc);return;
    case 0x00cfd83cu:delete_native_aux_parser_secondary_00716340(secondary,flags,c.aux);return;
    default:throw std::logic_error("Unbound native game resource parser profile");
    }
}
void* NativeGameResourceParserNameCalls::type_name(std::uintptr_t target,void* parser,void* output) {
    switch(target) {
    case 0x007258f0u:return name_native_geom_mesh_parser_007258f0(parser,output,strings_);
    case 0x006fb000u:return name_native_convex_object_parser_006fb000(parser,output,strings_);
    case 0x007186e0u:return name_native_note_parser_007186e0(parser,output,strings_);
    case 0x007187e0u:return name_native_zone_desc_parser_007187e0(parser,output,strings_);
    case 0x00718760u:return name_native_aux_parser_00718760(parser,output,strings_);
    default:return remaining_.type_name(target,parser,output);
    }
}
} // namespace bsp
