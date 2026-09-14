#pragma once
#include "bsp/native_mesh_pool.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// These pools use the same actual 38h allocator/lock/table layout as the mesh
// pool. Bind the very storage passed to declaration and buffer factories.
using NativeGraphicsPoolStorage = NativeMeshPoolStorage;
enum class NativeGraphicsPoolKind { vertex_declaration, hardware_layout,
    physical_buffer, logical_vertex, logical_index };
struct NativeGraphicsPoolProfile {
    std::uint32_t constructor, destructor, trim, vtable;
    std::uint32_t slot_bytes, slab_index_offset, free_count_offset;
};
const NativeGraphicsPoolProfile& native_graphics_pool_profile(NativeGraphicsPoolKind);

// New C++ interface for the five recovered ECX/RET lifecycles. Physical index
// and vertex pools share a profile but have distinct storage and companions.
// Host destruction does not perform native shutdown. The companion and shared
// list domain must outlive native use, including new-handler traversal.
class NativeGraphicsPoolLifetime {
public:
    NativeGraphicsPoolLifetime(AllocatorListDomain&, NativeGraphicsPoolStorage&,
        NativeGraphicsPoolKind);
    ~NativeGraphicsPoolLifetime() = default;
    NativeGraphicsPoolLifetime(const NativeGraphicsPoolLifetime&) = delete;
    NativeGraphicsPoolLifetime& operator=(const NativeGraphicsPoolLifetime&) = delete;
    NativeGraphicsPoolStorage& initialize(); // returns native this
    void trim_empty_slabs(); // virtual zero; no lock entry or payload destruction
    void destroy();          // explicit, single native shutdown
    NativeGraphicsPoolStorage& storage() noexcept { return storage_; }
    NativeGraphicsPoolKind kind() const noexcept { return kind_; }
private:
    AllocatorListDomain& list_;
    NativeGraphicsPoolStorage& storage_;
    NativeGraphicsPoolKind kind_;
    const NativeGraphicsPoolProfile& profile_;
    static void invoke_trim(void*);
    void free_table() noexcept;
    void destroy_critical_section() noexcept;
};

enum class NativeGraphicsPoolGlobal { vertex_declarations_0108fd38,
    hardware_layouts_0108fd70, physical_indices_0108fda8,
    physical_vertices_0108fde0, logical_vertices_0108fe18,
    logical_indices_0108fe50 };
using NativeGraphicsPoolAtexit = int (*)(void (*)());
// Bind actual globals once; there is no fallback pool or automatic retry.
void bind_static_native_graphics_pool(NativeGraphicsPoolGlobal, NativeGraphicsPoolLifetime&);
int initialize_static_native_vertex_declaration_pool_00cd7be0(NativeGraphicsPoolAtexit = &std::atexit);
int initialize_static_native_hardware_layout_pool_00cd7c00(NativeGraphicsPoolAtexit = &std::atexit);
int initialize_static_native_physical_index_pool_00cd7c20(NativeGraphicsPoolAtexit = &std::atexit);
int initialize_static_native_physical_vertex_pool_00cd7c40(NativeGraphicsPoolAtexit = &std::atexit);
int initialize_static_native_logical_vertex_pool_00cd7c60(NativeGraphicsPoolAtexit = &std::atexit);
int initialize_static_native_logical_index_pool_00cd7c80(NativeGraphicsPoolAtexit = &std::atexit);
void destroy_static_native_vertex_declaration_pool_00ce0ce0();
void destroy_static_native_hardware_layout_pool_00ce0cf0();
void destroy_static_native_physical_index_pool_00ce0d00();
void destroy_static_native_physical_vertex_pool_00ce0d10();
void destroy_static_native_logical_vertex_pool_00ce0d20();
void destroy_static_native_logical_index_pool_00ce0d30();

} // namespace bsp
