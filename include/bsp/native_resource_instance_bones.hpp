#pragma once
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/native_vertex_position_read.hpp"
#include <cstdint>

namespace bsp {
struct NativeLogicalBufferMappingContext;
struct NativeMeshWeightNamesStorage;
struct NativeMeshStorage;

// Resolve the captured native table identity to its SAME live table cells.
// This lookup is pure host bookkeeping, with no native calls or owner changes.
// Unknown virtual targets require complete actual services, never a successful
// no-op, copied owner, synthetic bounds, or replacement stream/declaration.
class NativeResourceInstanceBoneCalls {
public:
    virtual ~NativeResourceInstanceBoneCalls() = default;
    virtual const volatile std::uint32_t* table(std::uint32_t captured_profile) noexcept = 0;
    virtual std::uint8_t node_type(std::uint32_t target, void* node, std::uint32_t token) = 0;
    virtual void node_bounds(std::uint32_t target, void* node, void* output) = 0;
    virtual void* stream_declaration(std::uint32_t target, void* stream) = 0;
    virtual std::uint32_t stream_count(std::uint32_t target, void* stream) = 0;
    virtual void* stream_map(std::uint32_t target, void* stream, std::uint32_t count,
        std::uint32_t offset, std::uint8_t read_only) = 0;
    virtual void stream_unmap(std::uint32_t target, void* stream) = 0;
};

struct NativeResourceInstanceBoneContext {
    const volatile std::uint32_t& mesh_type_01090468;
    const volatile ModelTypeDescriptor& model_type_01090034;
    const volatile std::uint32_t& empty_minimum_00ce4970;
    const volatile std::uint32_t& empty_maximum_00ce4adc;
    NativeLogicalBufferMappingContext& mapping;
    const NativeD3dx9Float16Import& half_import;
    NativeResourceInstanceBoneCalls& calls;
    // Actual current CRT BF7FBF contract/locale; no ASCII-only substitution.
    int (*compare_names_00bf7fbf)(const char*, const char*);
};

struct NativeResourceMeshBindingRange {
    void* instance;
    void* owner;
    void* node;
    std::uint32_t count;
};
static_assert(sizeof(NativeResourceMeshBindingRange) == 0x10);

// B87CE0: ECX output, stacked actual instance, EAX output, RET4. Source EDX
// supplies the SAME current token cell. Captures iterator and pair count;
// the checked vector in that node remains live during later enumeration.
NativeResourceMeshBindingRange* __fastcall get_native_mesh_binding_range_00b87ce0(
    NativeResourceMeshBindingRange*, const volatile std::uint32_t*, void* instance);

// Complete native leaves; unused EDX keeps original stacked arguments/RET.
NativeMeshWeightNamesStorage* __fastcall get_native_mesh_weight_names_00b72730(void*) noexcept;
std::uint32_t read_native_mesh_binding_type_00b931b0(const volatile std::uint32_t&) noexcept;
std::int32_t native_mesh_vertex_stream_count_00b72b20(const NativeMeshStorage&) noexcept;
std::int32_t __fastcall get_native_resource_node_count_00b76510(const void*) noexcept;
void __fastcall set_native_model_bone_node_00b90600(void*, void* unused_edx,
    std::uint32_t index, void* node) noexcept;
void* __fastcall get_native_model_bone_node_00b90620(void*, void* unused_edx,
    std::uint32_t index) noexcept;
// B90E30 adapts model+184 to the existing B1C770 pointer-array specialization.
void resize_native_model_bone_nodes_00b90e30(void* model, std::int32_t count);

// Complete42-byte B6DC20 and191-byte427D10 preserve x87 staging/comparisons,
// SSE copies, forward aliases, EAX output and native RET4. No float defaults.
void* __fastcall read_native_node_local_bounds_00b6dc20(const void*, void* unused_edx,
    void* output) noexcept;
void* __fastcall include_native_aabb3_point_00427d10(void*, void* unused_edx,
    const void* point) noexcept;

// Full764-byte B87E80 caller, ECX instance/RET; EDX adds borrowed context.
// Actual AP instance/map, B91220 bone-bearing model prefix/header, native mesh,
// names and node storage must stay live. Names' data/count are re-read after
// callbacks; only their HEADER is captured. First case-insensitive match wins.
// Newly grown bone cells are zero; missing names preserve existing cells.
// Known stream slots dispatch canonical raw declaration/count/map/unmap bodies;
// known node+4C dispatches the exact bounds getter. Other targets use services.
// Initialized position-reader formats only: its native uninitialized scratch
// path raises a source diagnostic, preserving prior mutations and any mapping.
// No rollback, automatic unmap on failure, retain, or replacement node graph.
void __fastcall bind_native_resource_mesh_bones_00b87e80(void* instance,
    NativeResourceInstanceBoneContext&);
} // namespace bsp
