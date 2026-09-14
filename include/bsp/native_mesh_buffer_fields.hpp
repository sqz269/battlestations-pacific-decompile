#pragma once
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>

namespace bsp {
// Complete BF02F0[37]/BE9A20[27]. Native ECX reader/handle; stacked
// destination,count,budget / destination,count; RET0C/8. Current stream slot24
// overwrites the requested-count slot through its actual-count pointer; debit
// CURRENT budget afterward, wrapping. No zero-fill, EOF, extent or status check.
void read_native_resource_raw_00bf02f0(void* actual_reader, void* destination,
    std::uint32_t count, std::uint32_t* actual_budget, NativeResourceStreamReadContext&);
void read_native_resource_node_raw_00be9a20(void* actual_handle, void* destination,
    std::uint32_t count, NativeResourceStreamReadContext&);

// Complete MOV/RET and RET leaves. Native ECX actual mesh / ignored owner,
// no stack arguments. Getter returns mesh+7C in EAX; post-upload has no result.
std::uint32_t native_mesh_vertex_stream_count_00b72b20(const void* actual_mesh) noexcept;
void native_logical_stream_post_upload_00b49b30(void* actual_owner) noexcept;

struct NativeMeshBufferReadContext {
    NativeResourceStreamReadContext& reads;
    // SAME actual renderer, native raw stream/declaration/pool services and
    // canonical owners used by the target mesh. The declaration cache's string
    // adapter must borrow the reader's raw string publication/lifetime/gate.
    GuiTextNativeRendererServices& graphics;
};
enum class NativeMeshBufferReadPhase {
    empty, header, declaration, factory, declaration_release, map, read,
    skip_tail, unmap, post_upload, mesh_publication, creator_release, name_return, complete
};
struct NativeMeshBufferReadAcquired {
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired stream;
    NativeMeshBufferReadPhase phase{NativeMeshBufferReadPhase::empty};
    std::uint32_t native_site{};
    // Borrowed audit identities, not extra references; may be stale after a
    // native final release. Owned obligations are only in the acquired records.
    void* captured_declaration{};
    void* captured_stream{};
};

// B93AA0[154]/B93E60[302]: incoming ECX unused, stacked mesh/handle, RET8.
// Factory flags1; current captured renderer and logical slots select complete
// source implementations. Actual raw mapped GPU storage receives one read.
// Index calls map/unmap/post-upload, replaces mesh+60 then releases creator.
// Vertex releases the acquired declaration BEFORE its stride read/map, skips
// trailing rope.mvfm bytes, unmaps, appends at CURRENT mesh+7C, drops creator,
// then returns the completed local format name. Both keep native DWORD products.
// No resource rollback or automatic unmap on failure. Vertex EH owns only its
// completed local name. Acquired starts empty and survives exceptions; entered
// operations must not be replayed. A completed creator reference is consumed
// only at the original release site; canonical registrations are not owners.
void read_native_mesh_indices_00b93aa0(void* actual_mesh, void* actual_handle,
    NativeMeshBufferReadContext&, NativeMeshBufferReadAcquired&);
void read_native_mesh_vertex_stream_00b93e60(void* actual_mesh, void* actual_handle,
    NativeMeshBufferReadContext&, NativeMeshBufferReadAcquired&);

// New explicit-context interfaces. The current index profile must span through
// slot2C; vertex through slot14. Numeric native table entries are never executed.
// Original register/stack ABI, native EH/SEH/private aliases and hardware faults
// are not replaced. Actual device recreation remains a required reached provider.
} // namespace bsp
