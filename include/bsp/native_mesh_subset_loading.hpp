#pragma once
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_instance_generator_owner.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_buffer_fields.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_resource_root_dispatch.hpp"
#include <memory>
#include <vector>

namespace bsp {
struct NativeMeshSubsetLoadingContext {
    GuiNativeGeometryOwners& geometry;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialEffectCacheContext& effects;
    NativeMeshTextureFieldContext& texture_fields; // SAME reads/string/renderer/owners.
    NativeMeshLightingConstants lighting;
    NativeInstanceGeneratorOwners& generators; // SAME actual layout and geometry domain.
    const volatile std::uint32_t* material_profile_00d5e520;
};
struct NativeMeshSubsetAcquired {
    enum class Phase { empty, section, prefix, name, alias, material,
        material_admission, material_assignment, material_release, streams_clear,
        child, child_fields, child_release, default_stream, layout,
        mesh_publication, generator, section_release, name_return, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    GuiNativeSectionAcquired section;
    NativeMeshSectionStorage* captured_section{}; // Audit identity; no extra retain.
    NativeMaterialFactoryAcquired factory;
    NativeMaterialStorage* captured_material{};
    bool material_creator_consumed{}, section_published{}, section_creator_consumed{};
    NativeString name;
    bool name_completed{}, name_cleanup_armed{}, name_returned{};
    void* child{};
    bool child_cleanup_armed{};
    // Each Texture child has a distinct persistent cache/provider frame. A
    // failed child or generator is not retried/discarded by this parent.
    std::vector<std::unique_ptr<NativeMeshTextureFieldAcquired>> texture_children;
    NativeInstanceGeneratorAcquired generator;
};

// Complete B941D0[750], ECX unused output-pair context; stacked mesh/parent;
// RET8. Actual533FA0, five ordered DWORDs, case-insensitive soldiers.mshd
// alias to soldier.mshd, numeric535320 and canonical admission. Ordered
// Texture/LightingSettings/BoundingSphere/VertexStreamIndex children; unknown
// children skip. Sphere values are discarded straight from x87 ST0.
// Default first stream only when section count==0, actual B865A0 layout,
// append/retain section in mesh BEFORE B85610, then consume creator and name.
// Native EH owns only the completed name and current completed child, never
// material/section creators or mesh publications. Persist acquired on error.
// All services borrow the same actual domains and outlive native resources.
// Cold shader/program dependencies must be implemented by their real owners;
// no successful substitute is provided. New Win32 source, not native SEH/ABI.
void read_native_mesh_subset_00b941d0(void* actual_mesh, void* actual_parent_handle,
    NativeMeshSubsetLoadingContext&, NativeMeshSubsetAcquired&);

struct NativeMeshLoadingContext {
    NativeMeshSubsetLoadingContext& subsets;
    NativeMeshMetadataReadContext& metadata; // SAME actual reader as subsets.
};
struct NativeMeshFieldsAcquired {
    enum class Phase { empty, prefix, child, field, child_release, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    void* child{};
    bool child_cleanup_armed{};
    std::vector<std::unique_ptr<NativeMeshSubsetAcquired>> subsets;
    std::vector<std::unique_ptr<NativeMeshBufferReadAcquired>> buffers;
    std::vector<std::unique_ptr<NativeMeshMetadataReadAcquired>> metadata;
};
struct NativeMeshLoadingAcquired {
    enum class Phase { empty, construction, fields, polling, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{}, poll_iterations{};
    GuiNativeMeshAcquired mesh;
    NativeMeshFieldsAcquired fields;
};
// B72B40[4], ECX actual mesh; EAX current DWORD+58; RET. Volatile read keeps
// each native count sample, including the constructor's otherwise empty loop.
std::uint32_t native_mesh_section_count_00b72b40(const void* actual_mesh) noexcept;
// B944E0[557], ECX actual8h output pair; stacked parent handle; RET4. Prefix
// DWORD publishes pair+4. First four tags compare nullable C strings; later
// five compare the actual counted node+10 header with425850. Ordered native
// children share one actual mesh domain; only current completed child is an
// EH obligation. Distinct child acquisition frames persist through failure.
void read_native_mesh_fields_00b944e0(void* actual_pair, void* actual_parent_handle,
    NativeMeshLoadingContext&, NativeMeshFieldsAcquired&);
// B94710[131], ECX actual8h output pair; stacked parent; EAX current pair[0];
// RET4. Raw-slot-only constructor state, publish pair[0], parse, then sample
// current pair mesh/count in the original unsigned empty loop. No mesh/pair
// rollback, synthetic completion, fixed count snapshot or per-section action.
void* construct_native_mesh_from_node_00b94710(void* actual_pair, void* actual_parent_handle,
    NativeMeshLoadingContext&, NativeMeshLoadingAcquired&);

struct NativeMeshResourceAcquired {
    enum class Phase { empty, mesh, allocation, construction, mesh_release, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    // Persistent native local pair. Zero is host bookkeeping before B94710
    // publishes it, not a claim about the native uninitialized stack preimage.
    std::uint32_t pair[2]{};
    NativeMeshLoadingAcquired loading;
    void* item{};
    void* captured_mesh{};
    bool item_retained_mesh{}, mesh_creator_consumed{}, cleanup_deferred{};
};
// B947A0[169], B94850[175], B94900[175]: ECX parser ignored; stacked
// handle; EAX item/null; RET4. Actual B94710, allocate10h, B868B0/ref1,
// D63738/mesh+8/prefix+C, retain mesh, optional D6375C/D63780 profile,
// consume temporary mesh. FH3 state0 owns the local pair; state1 additionally
// owns raw item storage. Disarm before normal mesh release. Persist acquired
// on error. Pre-publication faults and failed host-only canonical admission
// defer cleanup explicitly; native uninitialized-stack/SEH parity is unproven.
void* parse_native_mesh_item_00b947a0(void* handle, NativeMeshLoadingContext&, NativeMeshResourceAcquired&);
void* parse_native_mesh_item_00b94850(void* handle, NativeMeshLoadingContext&, NativeMeshResourceAcquired&);
void* parse_native_mesh_item_00b94900(void* handle, NativeMeshLoadingContext&, NativeMeshResourceAcquired&);
// B93BA0 captures handle[0], releases nonnull mesh and clears handle[0] only
// after successful release; preserves pair+4. ECX handle, RET.
void release_native_mesh_handle_00b93ba0(void* handle, NativeRenderActualOwners&);
// B93910: stamp D63738, unconditionally release mesh+8, destroy item base
// on normal return or unwind. Does not clear mesh/prefix. ECX item, RET.
void destroy_native_mesh_item_00b93910(void* item, NativeRenderActualOwners&);
// Identical native scalar deleting entries: B93910 then free iff flags&1,
// captured item returned even after free; ECX item, stacked flags, RET4.
void* delete_native_mesh_item_00b93b40(void*, std::uint32_t, NativeRenderActualOwners&);
void* delete_native_mesh_item_00b93b60(void*, std::uint32_t, NativeRenderActualOwners&);
void* delete_native_mesh_item_00b93b80(void*, std::uint32_t, NativeRenderActualOwners&);

// Compose the three numeric parser targets with existing root dispatch.
// Acquisition frames are retained separately from resource-item ownership,
// including failed frames; no destructor, replay or extra retain is implicit.
class NativeMeshResourceCalls final : public NativeResourceDispatchCalls {
public:
    NativeMeshResourceCalls(NativeResourceDispatchCalls&, NativeMeshLoadingContext&);
    void renderer_hook(std::uintptr_t, void*) override;
    void* parse_item(std::uintptr_t, void*, void*) override;
    void append_item(std::uintptr_t, void*, void*) override;
    const std::vector<std::unique_ptr<NativeMeshResourceAcquired>>& acquisitions() const noexcept;
private:
    NativeResourceDispatchCalls& other_;
    NativeMeshLoadingContext& loading_;
    std::vector<std::unique_ptr<NativeMeshResourceAcquired>> acquisitions_;
};
// Existing raw resource-slot release dispatches current slot0 to BD30E0.
// For these three captured profiles, read CURRENT slot4 and run its actual
// scalar deletion with flags1. Mesh+4 uses the SAME canonical geometry domain.
class NativeMeshResourceReferences final : public NativeAdoptedSubstreamDispatch {
public:
    NativeMeshResourceReferences(NativeAdoptedSubstreamDispatch&, NativeRenderActualOwners&);
    std::uint8_t source_is_open(std::uintptr_t, void*) override;
    std::uint32_t source_seek(std::uintptr_t, void*, std::uint32_t, std::uint32_t, std::uint32_t) override;
    void source_read(std::uintptr_t, void*, void*, std::uint32_t, std::uint32_t*) override;
    void source_write(std::uintptr_t, void*, const void*, std::uint32_t, std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t, void*, std::uintptr_t) override;
private:
    NativeAdoptedSubstreamDispatch& other_;
    NativeRenderActualOwners& owners_;
};
} // namespace bsp
