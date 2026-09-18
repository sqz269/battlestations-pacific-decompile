#pragma once
#include "bsp/native_resource_manager_lifetime.hpp"
namespace bsp {
// Actual five parser publications, all borrowing the SAME raw01090AA0 cell.
// Keep these contexts alive through the shared singleton-manager drain.
struct NativeGameResourceParserContexts {
    NativeResourceParserSingletonContext geom_mesh;
    NativeResourceParserSingletonContext convex_object;
    NativeResourceParserSingletonContext note;
    NativeResourceParserSingletonContext zone_desc;
    NativeResourceParserSingletonContext aux;
};
struct NativeGameResourceParsersContext {
    NativeResourceManagerContext& manager;
    NativeGameResourceParserContexts& parsers;
};
// Getter206B: no native inputs, EAX8h owner, RET. Captured lifetime lock,
// double-check/publication, then register captured secondary+4. Deleter58B:
// ECX primary, stack flags, EAX captured primary, RET4; clear publication,
// stamp base tables, free iff bit0. Secondary8B adjusts -4 then tail-deletes.
// Name33B: incoming ECX ignored, stack fresh8h output, EAX output, RET4.
// Mapped original literals and canonical raw string/lifetime services required.
// geom_mesh: publication00E19BD0, primary00CFD800, secondary00CFD7FC.
void* get_native_geom_mesh_parser_00716fe0(NativeResourceParserSingletonContext&);
void* delete_native_geom_mesh_parser_00716380(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* delete_native_geom_mesh_parser_secondary_007161c0(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* name_native_geom_mesh_parser_007258f0(void*,void*,NativeStringRawPoolContext&);
// convex_object: publication00E19A90, primary00CFD810, secondary00CFD80C.
void* get_native_convex_object_parser_007170b0(NativeResourceParserSingletonContext&);
void* delete_native_convex_object_parser_007163c0(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* delete_native_convex_object_parser_secondary_00716220(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* name_native_convex_object_parser_006fb000(void*,void*,NativeStringRawPoolContext&);
// note: publication00E19B84, primary00CFD820, secondary00CFD81C.
void* get_native_note_parser_00717180(NativeResourceParserSingletonContext&);
void* delete_native_note_parser_00716400(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* delete_native_note_parser_secondary_00716280(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* name_native_note_parser_007186e0(void*,void*,NativeStringRawPoolContext&);
// zone_desc: publication00E19B8C, primary00CFD830, secondary00CFD82C.
void* get_native_zone_desc_parser_00717250(NativeResourceParserSingletonContext&);
void* delete_native_zone_desc_parser_00716440(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* delete_native_zone_desc_parser_secondary_007162e0(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* name_native_zone_desc_parser_007187e0(void*,void*,NativeStringRawPoolContext&);
// aux: publication00E19B88, primary00CFD840, secondary00CFD83C.
void* get_native_aux_parser_00717320(NativeResourceParserSingletonContext&);
void* delete_native_aux_parser_00716480(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* delete_native_aux_parser_secondary_00716340(void*,std::uint32_t,NativeResourceParserSingletonContext&) noexcept;
void* name_native_aux_parser_00718760(void*,void*,NativeStringRawPoolContext&);
// Exact103B normal wrapper: five repeated manager/getter/register triples,
// Aux,ZoneDesc,Note,GeomMesh,ConvexObject. Capture each manager BEFORE its
// parser getter. Ignore registration AL; do not cache manager across triples.
void register_native_game_resource_parsers_00717e80(NativeGameResourceParsersContext&);
void delete_native_game_resource_registered_owner(std::uintptr_t captured_profile,
    void* popped_secondary,std::uint32_t flags,NativeGameResourceParserContexts&);
class NativeGameResourceParserNameCalls final:public NativeResourceParserNameCalls {
public:
    NativeGameResourceParserNameCalls(NativeStringRawPoolContext& strings,NativeResourceParserNameCalls& remaining) noexcept:strings_(strings),remaining_(remaining) {}
    void* type_name(std::uintptr_t captured_target,void* actual_parser,void* actual_output) override;
private:
    NativeStringRawPoolContext& strings_;
    NativeResourceParserNameCalls& remaining_;
};
// Source composition over actual storage. Native tables are identities, not
// callable C++ vtables. Original FH3/SEH/private-stack/binary ABI and parse
// slot8 payload behavior remain separate evidence boundaries.
} // namespace bsp
