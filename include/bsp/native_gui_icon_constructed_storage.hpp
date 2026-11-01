#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_gui_widget_position.hpp"
#include "bsp/native_model_storage_geometry.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <exception>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Existing SAME canonical domains, prepared and retained before entry. The
// geometry supplier uses the actual B73B60/B73D70 pool/constructor and records
// real creator/companion custody; no new mesh owner or callback is introduced.
struct NativeGuiIconConstructedStorageContext {
    NativeRenderActualOwnerRegistry& owners;
    GuiNativeGeometryOwners& geometry;
    const game::GameNativeReadOnlyData& data;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const volatile std::uint32_t& unchanged_00d7a260;
    NativeGuiWidgetTransformBindings& transform;
    const volatile double& half_00d7a280;
};

enum class NativeGuiIconConstructedStoragePhase { fresh, entered, complete, failed };
struct NativeGuiIconConstructedStorageAcquired {
    NativeGuiIconConstructedStoragePhase phase{NativeGuiIconConstructedStoragePhase::fresh};
    void* actual_icon{};
    void* captured_model{};
    void* captured_mesh{};
    std::uint32_t active_call_site{}, escaped_call_site{};
    std::uint8_t observed_74{};
    bool model_captured{}, mesh_entered{}, mesh_returned{}, arguments_staged{};
    bool association_entered{}, association_returned{};
    bool decrement_entered{}, decremented{}, creator_zero_entered{}, creator_zero_returned{};
    bool position_staged{}, position_entered{}, position_returned{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long creator_result{};
    std::uint32_t creator_profile{}, creator_target{};
    GuiNativeMeshAcquired mesh;
    NativeModelStorageGeometryAcquired association;
    std::exception_ptr failure;
};

// Borrow ordinary, genuinely addressable caller DWORD arrays, not volatile
// backing cast through other helpers. No initializer writes any home. Their
// preimages, transform/bounds scratch and all C++ subobjects must be real/live.
// Four homes are [unused0, geometry, scalar178, scalar17C]; the SAME existing
// Model frame reads them at its real reached epochs. The three position homes
// are zeroed only at the final Native stage, including the skipped-mesh arm.
class NativeGuiIconConstructedStorageFrame final {
public:
    NativeGuiIconConstructedStorageFrame(NativeGuiIconConstructedStorageContext& c,
        std::uint32_t (&association_homes)[4], std::uint32_t (&position_homes)[3],
        NativeGuiWidgetTransformScratch& transform, GuiWidgetBounds& bounds) noexcept
        : context_(c), arguments_(association_homes), position_(position_homes),
          transform_(transform), bounds_(bounds), association_(association_homes[0],
              association_homes[1], association_homes[2], association_homes[3]) {}
    ~NativeGuiIconConstructedStorageFrame() = default;
    NativeGuiIconConstructedStorageFrame(const NativeGuiIconConstructedStorageFrame&) = delete;
    NativeGuiIconConstructedStorageFrame& operator=(const NativeGuiIconConstructedStorageFrame&) = delete;
    NativeGuiIconConstructedStorageFrame(NativeGuiIconConstructedStorageFrame&&) = delete;
    NativeGuiIconConstructedStorageFrame& operator=(NativeGuiIconConstructedStorageFrame&&) = delete;
private:
    friend void construct_native_gui_icon_geometry_storage_00ab2540(void*,
        NativeGuiIconConstructedStorageFrame&, NativeGuiIconConstructedStorageContext&,
        NativeGuiIconConstructedStorageAcquired&);
    NativeGuiIconConstructedStorageContext& context_;
    std::uint32_t (&arguments_)[4];
    std::uint32_t (&position_)[3];
    NativeGuiWidgetTransformScratch& transform_;
    GuiWidgetBounds& bounds_;
    NativeModelStorageGeometryFrame association_;
    NativeGuiIconConstructedStoragePhase phase_{NativeGuiIconConstructedStoragePhase::fresh};
};

// Whole AB2540 NORMAL payload/call schedule, not its private FH3 action. Test
// actual+74; when zero capture CURRENT+4C BEFORE allocation and preserve it
// across callbacks. Spill one live D7A260 via FLD/FST/FSTP into the two actual
// scalar homes, then geometry/unused homes; call the SAME canonical completed
// S1 NativeModelReference's checked raw B75170 entry, not a typed owner cast or
// the incompatible NativeModelStorageReference entry. Reload CE2220 AFTER that
// whole call; decrement captured mesh+4, and only on observed zero capture its
// CURRENT profile/slot0 and dispatch its existing genuine canonical terminal.
// Always stage zero XYZ and perform whole AA7DC0, even if +74 skipped creation.
//
// Caller proves genuine live raw Icon/Model/storage/backing, SAME authentic RO
// constants/table/import/registry/mesh/transforms and provider graph. All frame,
// acquisition, homes and Source controls/spills are disjoint from native data,
// provider-write and active argument/return/provider extents. Distinct nested
// calls need distinct retained frames/acquisitions and operand/scratch backing.
// Mesh normal-null follows the actual association/decrement schedule; no null,
// count, mode or bounds guard/default mesh is supplied. Original fault behavior
// and arbitrary foreign current targets are not an admitted C++ storage domain.
//
// The existing combined creator adds real host admission after B73D70. Its
// private allocation-save/EH-state epoch cannot reproduce the Original caller
// stack; those CB7A08 operands/actions are held, never reseeded or replayed.
// UNKNOWN escapes retain actual partial/complete/remaining graph and incomplete
// allocation/companion/zero obligation. Catch records only and rethrows; no
// cleanup/refund/retry/rollback, Native Origin, ownership or disposition token.
// Keep this frame through failure; its existing Model frame destructor and
// inherited noexcept constructor/terminal suppliers keep their Source policies.
// No new NoThrow/terminal policy, Native/vendor/helper/CG/duplicate credit or
// Original register/stack/FLAGS/FH3/SEH/unmasked-FP/runtime/activation claim.
void construct_native_gui_icon_geometry_storage_00ab2540(void* actual_icon,
    NativeGuiIconConstructedStorageFrame&, NativeGuiIconConstructedStorageContext&,
    NativeGuiIconConstructedStorageAcquired&);
} // namespace bsp
