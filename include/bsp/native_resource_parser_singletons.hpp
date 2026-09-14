#pragma once
#include "bsp/native_resource_parser_registration.hpp"

namespace bsp {
struct NativeStringRawPoolContext;

// Borrow the application's actual singleton manager publication (01090AA0)
// and the selected parser's publication cell listed below. Bindings are stable;
// their volatile values may change. The parser allocation is exactly eight bytes.
struct NativeResourceParserSingletonContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_parser_publication;
};

// All getters: complete206B, native no input / EAX parser / RET. Capture the
// first manager's section+10, enter/increment, recheck and publish under lock.
// Capture current parser+4 BEFORE the second manager getter and register it.
// Registration failure retains publication/allocation and releases that guard.
// Primary deletion: complete58B, ECX parser, stacked flags, EAX original
// address, RET4. Clear publication, stamp CE3818 secondary and CFD7D0 primary,
// free iff bit0. Secondary entry: complete8B, subtract four then tail-delete.
// No unregister or parser-body destruction is invented.
// Name getter: complete33B, incoming ECX ignored, stacked actual8h output,
// EAX output, RET4. Construct through41E870 using the actual mapped read-only
// literal; it clears the output header internally. The caller owns that name.

// GroupParams: publication 0109033c, primary 00d63088, secondary 00d63084.
void* get_native_group_params_parser_00b7e100(NativeResourceParserSingletonContext&);
void* delete_native_group_params_parser_00b7dd00(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_group_params_parser_secondary_00b7dae0(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_group_params_parser_00b8f8e0(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// Mesh: publication 0109047c, primary 00d63098, secondary 00d63094.
void* get_native_mesh_parser_00b7e1d0(NativeResourceParserSingletonContext&);
void* delete_native_mesh_parser_00b7dd40(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_mesh_parser_secondary_00b7db40(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_mesh_parser_00b7eb60(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// SkinedMesh: publication 01090480, primary 00d630a8, secondary 00d630a4.
void* get_native_skined_mesh_parser_00b7e2a0(NativeResourceParserSingletonContext&);
void* delete_native_skined_mesh_parser_00b7dd80(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_skined_mesh_parser_secondary_00b7dba0(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_skined_mesh_parser_00b94170(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// MatrixIndexedMesh: publication 01090484, primary 00d630b8, secondary 00d630b4.
void* get_native_matrix_indexed_mesh_parser_00b7e370(NativeResourceParserSingletonContext&);
void* delete_native_matrix_indexed_mesh_parser_00b7ddc0(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_matrix_indexed_mesh_parser_secondary_00b7dc00(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_matrix_indexed_mesh_parser_00b941a0(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// Camera: publication 010902a0, primary 00d630c8, secondary 00d630c4.
void* get_native_camera_parser_00b7e460(NativeResourceParserSingletonContext&);
void* delete_native_camera_parser_00b7de00(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_camera_parser_secondary_00b7dc60(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_camera_parser_00b8b210(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// SkinedMeshAnimation: publication 0109043c, primary 00d630d8, secondary 00d630d4.
void* get_native_skined_mesh_animation_parser_00b7e530(NativeResourceParserSingletonContext&);
void* delete_native_skined_mesh_animation_parser_00b7de40(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* delete_native_skined_mesh_animation_parser_secondary_00b7dcc0(void*, std::uint32_t flags, NativeResourceParserSingletonContext&) noexcept;
void* name_native_skined_mesh_animation_parser_00b92a20(void* actual_parser, void* actual_output_header, NativeStringRawPoolContext&);

// Finite dispatch for the six actual slot4 targets above. Dispatch the captured
// target without reading the parser table again. Other targets are an explicit
// composition error, never silently replaced by a default type name.
class NativeDefaultResourceParserNameCalls final : public NativeResourceParserNameCalls {
public:
    explicit NativeDefaultResourceParserNameCalls(NativeStringRawPoolContext& strings) noexcept
        : strings_(strings) {}
    void* type_name(std::uintptr_t captured_target, void* actual_parser,
        void* actual_output_header) override;
private:
    NativeStringRawPoolContext& strings_;
};

// New explicit-service C++ interfaces. Profiles are native identity DWORDs,
// not callable C++ vtables. Requires real Win32 sections, source CRT allocation,
// actual singleton registration/string pool and mapped literal data. Native
// FH3/SEH identity, raw manager bootstrap, parse virtual bodies, executable
// admission and gameplay remain separate evidence boundaries.
} // namespace bsp
