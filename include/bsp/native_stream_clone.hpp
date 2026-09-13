#pragma once
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_logical_index_owner.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"

namespace bsp {
class GuiNativeGeometryOwners;
struct NativeLogicalIndexCreationContext;

// Source bookkeeping only. A call_in_progress state after an exception says
// the nested native call may have made effects; it is not permission to retry.
enum class NativeStreamCloneMapPhase { not_called, call_in_progress, returned,
    unlock_in_progress, unlocked };
enum class NativeStreamClonePhase { empty, factory, registration, source_map,
    destination_map, copy, source_unlock, destination_unlock, complete,
    mesh_publication, consumed };
struct NativeStreamCloneAcquired {
    void* creator{}; // ONE actual +04 reference, published before renderer append.
    RenderCommandReference* companion{}; // Same count; possibly not yet registered.
    bool canonical_registration{};
    bool vertex{};
    void* source{}; // Borrowed and live until all entered mappings are resolved.
    void* source_mapping{};
    void* destination_mapping{};
    NativeStreamCloneMapPhase source_map{NativeStreamCloneMapPhase::not_called};
    NativeStreamCloneMapPhase destination_map{NativeStreamCloneMapPhase::not_called};
    NativeStreamClonePhase phase{NativeStreamClonePhase::empty};
    std::uint32_t native_site{};
};

// SAME current renderer/global synchronization, physical services and canonical
// raw owners. Profile arrays contain immutable native code tokens, not callable
// pointers. Actual raw factories/mapping/terminal implementations are composed.
// Vertex/index logical views span at least10/9 DWORDs, renderer views25,
// physical mapping views9. All contexts and registrations outlive every stream.
struct NativeStreamCloneServices {
    GuiNativeGeometryOwners& geometry;
    NativeLogicalVertexOwnerContext& vertices;
    NativeLogicalIndexCreationContext& indices;
    NativeLogicalBufferMappingContext& mapping;
};

// Complete B729A0..B72A65 / B72A70..B72B19 normal callers. Native ECX source,
// EAX distinct creator, RET (no stack arguments). Destination gets the live
// source format/declaration, flags and count. Both locks receive readonly=0.
// Counts/format/declaration are reloaded at their native call sites; DWORD byte
// products wrap. Copy uses the CRT memcpy contract, then source/destination
// unlock in that order. Original source/destination storage and valid mapped
// extents must survive callbacks. No typed Logical* overlay or byte-vector copy.
// Acquired must be empty/consumed. Every entered factory/map/unmap and partial
// creator is recorded. Exceptions preserve effects and are NOT resumable. An
// interrupted factory with creator!=null has completed construction but may
// have incomplete renderer registration, and is not canonically registered.
void* clone_native_index_stream_00b729a0(void* actual_source,
    NativeStreamCloneServices&, NativeStreamCloneAcquired&);
void* clone_native_vertex_stream_00b72a70(void* actual_source,
    NativeStreamCloneServices&, NativeStreamCloneAcquired&);

// New MSVC Win32 interfaces, not binary entry/SEH replacements. Required real
// device recreation remains a reached provider boundary in the actual factories.
} // namespace bsp
