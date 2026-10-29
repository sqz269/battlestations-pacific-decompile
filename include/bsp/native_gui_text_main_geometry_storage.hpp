#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_gui_widget_position.hpp"
#include "bsp/native_model_storage_dispatch.hpp"
#include "bsp/native_model_storage_geometry.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text main geometry requires MSVC Win32 and x86 inline assembly.
#endif

namespace bsp {
// Pure borrows of ONE completed raw93/canonical122 Text's actual domain.
// primary is a DIFFERENT independently retained481 record from internal188.
// internal aliases the exact nested85 record, including all lifetime/release
// cells, not copies of their current values. The existing481 optional reference
// must have been constructed over those exact context/frame/acquisition cells.
struct NativeGuiTextMainGeometryStorageContext {
    NativeModelStorageDispatchContext& primary;
    NativeModelStorageDispatchContext& internal188;
    NativeModelStorageGeometryContext& association; // SAME primary.storage
    GuiNativeGeometryOwners& geometry;
    // Constructed over the SAME retained actual readonly-data service as122.
    // Its private/transitive table provenance remains an explicit caller proof.
    const NativeNodeMappedRawTransformProfiles& transforms;
    const volatile float& one_00d7a24c;
    const volatile float& negative_zero_00d7a208;
    const volatile float& y_scale_00e12fc4;
    const volatile double& half_00d7a280;
};

enum class NativeGuiTextMainGeometryStoragePhase : std::uint8_t {
    fresh, entered, complete, failed
};
struct NativeGuiTextMainGeometryStorageAcquired {
    using Phase = NativeGuiTextMainGeometryStoragePhase;
    enum class Boundary { none, model_capture, mesh, association, creator_release,
        creator_zero, position };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t active_call_site{}, escaped_call_site{};
    void* destination{};
    void* captured_primary{};
    void* captured_mesh{}; // diagnostic after a terminal, never a rediscovery key
    bool primary_captured{}, mesh_entered{}, mesh_returned{}, arguments_prepared{};
    GuiNativeMeshAcquired mesh;
    NativeModelStorageGeometryAcquired association;
    bool association_entered{}, association_returned{};
    NativeTextureSurfaceReferenceIncrement creator_decrement{};
    long creator_result{};
    bool creator_release_entered{}, creator_decremented{}, creator_release_returned{};
    std::uint32_t creator_zero_profile{}, creator_zero_target{};
    bool creator_profile_captured{}, creator_target_captured{};
    bool creator_zero_entered{}, creator_zero_returned{};
    bool position_prepared{}, position_entered{}, position_returned{};
    struct Transform {
        void* receiver{};
        const void* matrix{};
        std::uint32_t captured_target{}, admitted_profile{};
        bool entered{}, body_entered{}, body_returned{}, failed{};
    } transform;
    std::exception_ptr failure;
    NativeGuiTextMainGeometryStorageAcquired() = default;
    NativeGuiTextMainGeometryStorageAcquired(const NativeGuiTextMainGeometryStorageAcquired&) = delete;
    NativeGuiTextMainGeometryStorageAcquired& operator=(const NativeGuiTextMainGeometryStorageAcquired&) = delete;
};

// Preinstall this complete nonmoving graph before entry. Constructor only
// takes operand/scratch addresses and references; it reads no native operand.
// arguments: unused0, geometry, scalar178, scalar17C. Their actual DWORD
// lifetimes, zero-XYZ backing and initialized transform/bounds preimages are
// caller-owned, mutually disjoint and retained through every failure.
class NativeGuiTextMainGeometryStorageFrame final : private NativeGuiWidgetTransformDispatch {
public:
    NativeGuiTextMainGeometryStorageFrame(NativeGuiTextMainGeometryStorageContext&,
        volatile std::uint32_t (&arguments)[4], volatile std::uint32_t (&position)[3],
        NativeGuiWidgetTransformScratch&, GuiWidgetBounds&) noexcept;
    ~NativeGuiTextMainGeometryStorageFrame() noexcept override;
    NativeGuiTextMainGeometryStorageFrame(const NativeGuiTextMainGeometryStorageFrame&) = delete;
    NativeGuiTextMainGeometryStorageFrame& operator=(const NativeGuiTextMainGeometryStorageFrame&) = delete;
    NativeGuiTextMainGeometryStoragePhase phase() const noexcept { return phase_; }
    // Pure identity/private-phase check; no native credit or payload access.
    bool matches(const NativeGuiTextMainGeometryStorageContext&,
        const NativeGuiTextMainGeometryStorageAcquired&,
        NativeGuiTextMainGeometryStoragePhase) const noexcept;
    // Pure constructor-backing identity, including before acquisition binds.
    // It grants no completion, native liveness, ownership or dispatch permission.
    bool matches_backing(const NativeGuiTextMainGeometryStorageContext&,
        const volatile std::uint32_t (&arguments)[4],
        const volatile std::uint32_t (&position)[3],
        const NativeGuiWidgetTransformScratch&, const GuiWidgetBounds&) const noexcept;
private:
    friend void construct_native_gui_text_main_geometry_storage_00ab7700(
        NativeGuiTextRawModelReference&, NativeGuiTextMainGeometryStorageContext&,
        NativeGuiTextMainGeometryStorageFrame&, NativeGuiTextMainGeometryStorageAcquired&);
    void preflight(NativeGuiTextRawModelReference&, NativeGuiTextMainGeometryStorageAcquired&);
    void call_virtual38(std::uint32_t captured_target, void* current_receiver,
        const void* actual_matrix) override;
    NativeGuiTextMainGeometryStorageContext& context_;
    volatile std::uint32_t (&arguments_)[4];
    volatile std::uint32_t (&position_)[3];
    NativeGuiWidgetTransformScratch& transform_scratch_;
    GuiWidgetBounds& bounds_scratch_;
    NativeModelStorageGeometryFrame association_;
    NativeGuiWidgetTransformBindings transform_bindings_;
    NativeGuiTextMainGeometryStorageAcquired* acquired_{};
    NativeGuiTextMainGeometryStoragePhase phase_{NativeGuiTextMainGeometryStoragePhase::fresh};
};

// Complete successful-allocation normal AB7700..AB77B1, native ECX Text/no
// stack args/RET; this new C++ API is NOT Original register/FH3/SEH/FP ABI.
// Capture primary4C BEFORE mesh construction. One x87 D7A260 load spills to
// BOTH live B75170 argument cells; that setter has a separate late sentinel
// epoch. Release the captured creator with CURRENT CE2220, genuine current0
// at observed zero, then AA7DC0 zeroXYZ using raw AA7220/B6DB10/AA70E0.
//
// No Model creation/publication, Text credit, count copy, Font, content, scene
// router or retirement admission is added. Source entry requires a canonical
// raw122 Text and live independent primary/internal188 records. Caller keeps
// their WHOLE actual/native provider graph, opaque backing, hierarchy, imports,
// current tables and all argument/scratch/record storage stable and disjoint.
// All reached native memory operations and mesh allocation succeed; native
// null-mesh/fault/SEH and unmasked FP traps are outside this Source domain.
// Callbacks may replace current4C with an explicitly admitted canonical Model
// and mutate argument cells, but must not retire Text/Models during remaining
// native reads. Context aliases and completed Model identities remain fixed.
// Genuine raw transform hierarchy/profile contracts still apply; if bounds'
// byte+74 is nonzero its current Model/geometry/section0 must be accessible.
//
// Mesh constructor-only unwind is delegated to the existing genuine provider.
// Later failures retain every reached effect/credit/record; no catch-release,
// rollback, retry, dependency drain or dead-payload rediscovery. Frame destruction
// terminates for entered/failed state. Current unsupported targets reached after
// effects are failed obligations, not harmless pre-effect rejections. The
// canonical zero terminal keeps its existing noexcept/failure disposition.
void construct_native_gui_text_main_geometry_storage_00ab7700(
    NativeGuiTextRawModelReference&, NativeGuiTextMainGeometryStorageContext&,
    NativeGuiTextMainGeometryStorageFrame&, NativeGuiTextMainGeometryStorageAcquired&);
} // namespace bsp
