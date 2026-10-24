#pragma once

#include "bsp/native_model_owner.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/model_type_bootstrap.hpp"

namespace bsp {

// Same 38h allocator layout, DISTINCT canonical owner 01090390. A genuine
// 190h MatrixIndexedModel payload precedes this pool's slab-index DWORD.
using NativeMatrixIndexedModelPoolStorage = NativeModelPoolStorage;
class NativeMatrixIndexedModelPool final {
public:
    static constexpr std::uint32_t native_vtable = 0x00d63560;
    static constexpr std::uint32_t native_virtual0 = 0x00b90a20;
    static constexpr std::size_t slot_bytes = 0x194;
    static constexpr std::size_t slot_slab_index_offset = 0x190;
    static constexpr std::size_t slab_bytes = 0x32c4;
    static constexpr std::uint32_t slots_per_slab = 32;
    NativeMatrixIndexedModelPool(AllocatorListDomain&, NativeMatrixIndexedModelPoolStorage&);
    ~NativeMatrixIndexedModelPool() = default;
    NativeMatrixIndexedModelPool(const NativeMatrixIndexedModelPool&) = delete;
    NativeMatrixIndexedModelPool& operator=(const NativeMatrixIndexedModelPool&) = delete;
    void initialize_00b90940();
    void* allocate_raw_slot_00b90ac0();
    void return_raw_slot_00b90150(void*);
    void trim_empty_slabs_00b90a20();
    void destroy_00b90090();
    NativeMatrixIndexedModelPoolStorage& storage() noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void*) noexcept;
private:
    AllocatorListDomain& allocator_list_;
    NativeMatrixIndexedModelPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b8fc70() noexcept;
};

using NativeMatrixIndexedModelPoolAtexit = int (*)(void (*)());
void bind_static_native_matrix_indexed_model_pool_01090390(NativeMatrixIndexedModelPool&);
int initialize_static_native_matrix_indexed_model_pool_00cd8550(
    NativeMatrixIndexedModelPoolAtexit = &std::atexit);
void destroy_static_native_matrix_indexed_model_pool_00ce0f20();
void* allocate_native_matrix_indexed_model_slot_00b90c20();
void return_native_matrix_indexed_model_slot_00b90480(void*);

// Explicit CD8570 startup on retained canonical Source cells, using the SAME
// application's counter and ModelTypeBootstrap storage. No fixed numeric IDs
// or Original CRT ordering claim. A failed initialization keeps its guard set.
void initialize_native_matrix_indexed_model_type_00cd8570(
    TypeIdCounterLifetime&, ModelTypeBootstrap&);
// Four current ID cells (own, model, node, root), never a copied token palette.
const volatile std::uint32_t* native_matrix_indexed_model_type_ids_010903c8() noexcept;

struct NativeMatrixIndexedModelEnvironment {
    NativeModelEnvironment& base;
    NativeMatrixIndexedModelPool& pool_01090390;
    const volatile std::uint32_t* vtable_00d63590; // actual current 23-DWORD profile
    const volatile std::uint32_t* type_ids_010903c8; // four live canonical IDs
};

// One actual 194h slot; base borrows the SAME bytes/node/scene association.
// B91220 calls B75030 exactly, then stamps D63590 and initializes the actual
// pointer array at 184/188/18C. No 188h model allocation or final reference is
// substituted. The trailing slab index190 survives all payload operations.
class NativeMatrixIndexedModelOwner final {
public:
    NativeMatrixIndexedModelOwner(void*, std::size_t, NativeMatrixIndexedModelEnvironment&);
    ~NativeMatrixIndexedModelOwner();
    NativeMatrixIndexedModelOwner(const NativeMatrixIndexedModelOwner&) = delete;
    NativeMatrixIndexedModelOwner& operator=(const NativeMatrixIndexedModelOwner&) = delete;
    NativeModelOwner base;
    NativeMatrixIndexedModelEnvironment& environment;
    NativeRenderPointerArrayStorage& bones;
    enum class Phase { prepared, live, destroying, dead };
    Phase phase{Phase::prepared};
};

// Original ECX model, stacked actual name header, EAX model, RET4. The base
// constructor receives the same header and shared raw-node constants.
void* construct_native_matrix_indexed_model_00b91220(NativeMatrixIndexedModelOwner&,
    const void* actual_name_header, const NativeNodeRawConstants&);
// B91280: resize bone header to0 (no node retains/releases), free CURRENT data,
// then B750C0. Base cleanup also runs if array cleanup throws; never retry it.
void destroy_native_matrix_indexed_model_00b91280(NativeMatrixIndexedModelOwner&);
// Original ECX model, stacked flags, EAX original slot, RET4. Return to the
// actual194h pool only when flags&1, after full payload destruction.
void* delete_native_matrix_indexed_model_00b915a0(NativeMatrixIndexedModelOwner&, std::uint32_t flags);

class NativeMatrixIndexedModelReference;
struct NativeMatrixIndexedModelCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeMatrixIndexedModelReference&) noexcept;
};
class NativeMatrixIndexedModelReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeMatrixIndexedModelReference(NativeMatrixIndexedModelOwner&,
        NativeMatrixIndexedModelCompanionDisposal);
    ~NativeMatrixIndexedModelReference() override;
    NativeMatrixIndexedModelReference(const NativeMatrixIndexedModelReference&) = delete;
    NativeMatrixIndexedModelReference& operator=(const NativeMatrixIndexedModelReference&) = delete;
    NativeMatrixIndexedModelOwner& model_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.base.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.base.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeMatrixIndexedModelOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeMatrixIndexedModelCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};

namespace game {
// Explicit one-attempt startup, with the genuine CRT callback and SAME
// E188B4 application allocator domain. Finish all models before CE0F20.
int initialize_matrix_indexed_model_pool_once_00cd8550();
NativeMatrixIndexedModelPool& matrix_indexed_model_pool_01090390();
}

}  // namespace bsp
