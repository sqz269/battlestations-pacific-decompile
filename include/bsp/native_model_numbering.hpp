#pragma once
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_vertex_position_read.hpp"
#include "bsp/model_type_bootstrap.hpp"

namespace bsp {
struct NativeNodeStorage;

// Borrow existing owner, string, mapping, declaration and atlas domains.
// Numeric native vtables are identifiers; never invoke them as host code.
struct NativeModelNumberingServices {
    NativeStringStorage& strings;
    GuiTextNativeRendererServices& renderer;
    NativeMeshSectionLayoutServices& layouts;
    const NativeD3dx9Float16Import& half_import;
    void* const volatile& renderer_00f8d394;
    void* const volatile& atlas_00f8c26c;
    const volatile ModelTypeDescriptor& model_type_01090034;
    const char* null_atlas_pattern_00e17bf0;
    const char* null_number_name_00e19b4f;
    void* type_context;
    // Required real current virtual+0C dispatch for the same raw node/profile.
    // The native profile identity is captured before the live type-id read.
    bool (*node_virtual0c)(void*, NativeNodeStorage&, std::uint32_t profile,
        std::uint32_t requested_type);
};

enum class NativeModelNumberingPhase { fresh, running, complete, failed };
// Diagnostic acquisition state; no destructor, rollback or replacement owner.
// Failed operations cannot be replayed. A completed one may serve the next
// section while traversing the same hierarchy. Effects survive exceptions.
struct NativeModelNumberingOperation {
    NativeModelNumberingPhase phase{NativeModelNumberingPhase::fresh};
    std::uint32_t native_site{};
    NativeMeshSectionStorage* section{};
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
};

// Borrowed inputs for the caller's complete vertex loop, after both mappings
// and the initial texture publication. No renderer or owner is synthesized.
struct NativeNumberingVertexServices {
    NativeStringStorage& strings;
    const NativeD3dx9Float16Import& half_import;
    void* const volatile& atlas_00f8c26c;
    const char* null_atlas_pattern_00e17bf0;
    const char* null_number_name_00e19b4f;
};
// 7116E6..71180D plus its 711869..711A1F digit/atlas branches. Number is
// already clamped to 0..999 by 711510; count reloads actual section+10.
// Source/destination maps are the actual borrowed mappings returned above.
void rebuild_native_numbering_vertices_00711714_fragment(void* actual_source,
    const void* actual_source_mapping, void* actual_destination_mapping,
    NativeMeshSectionStorage&, std::uint32_t clamped_number,
    NativeNumberingVertexServices&, std::uint32_t& diagnostic_native_site);

// 711510: ECX mesh, EDX draw section, stack unsigned number, RET4.
// Complete normal sequence for initialized reader formats, actual D61D6C
// streams and the existing concrete renderer/layout services. New C++ ABI;
// original FH3, unmasked FP and invalid-storage delivery remain unproved.
void set_native_section_numbering_00711510(NativeMeshStorage&,
    NativeMeshSectionStorage&, std::uint32_t, NativeModelNumberingServices&,
    NativeModelNumberingOperation&);
// 711A20: ECX node, EDX number, RET. Reload section counts, geometry and sibling
// links after native calls. Set visibility only after a matching section.
void set_native_node_numbering_00711a20(NativeNodeStorage&, std::uint32_t,
    NativeModelNumberingServices&, NativeModelNumberingOperation&);
// 711BE0: ECX model, stack number, RET4. Reads model+160 then holder+0C.
void set_native_model_numbering_00711be0(void* actual_model, std::uint32_t,
    NativeModelNumberingServices&, NativeModelNumberingOperation&);
} // namespace bsp
