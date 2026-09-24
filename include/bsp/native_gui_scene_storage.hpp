#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>

namespace bsp {
class NativeWeakOwnerDomain;
struct NativeStringRawPoolContext;
struct NativeNodeTreeRetirementContext;
struct NativeNodeTreeRetirementAcquired;

struct NativeGuiSceneStorageContext {
    NativeWeakOwnerDomain& weak;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwnerRegistry& owners;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const volatile std::uint32_t& actual_one_00d7a24c;
    // Required only on a reached nonempty root list. Must borrow SAME owners.
    // Its genuine current18/0 bindings retain their admitted terminal domain.
    NativeNodeTreeRetirementContext* tree{};
};
struct NativeGuiSceneConstructFrame {
    volatile std::uint32_t name_argument;
};
struct NativeGuiSceneConstructAcquired {
    bool started{}, complete{}, weak_constructed{}, name_initialized{};
    bool name_cleanup_completed{}, weak_cleanup_completed{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
};
struct NativeGuiSceneDestroyFrame {
    // Prepared persistent fresh diagnostics, one per reached B6DFA0. No native
    // local scratch is needed by B6DFA0. used starts0; no allocation is added.
    NativeNodeTreeRetirementAcquired* root_calls{};
    std::uint32_t capacity{}, used{};
};
struct NativeGuiSceneLifetimeAcquired {
    bool started{}, complete{}, decrement_completed{}, terminal_completed{};
    bool lighting_cleared{}, name_return_completed{}, weak_destroyed{}, freed{};
    bool name_unwind_completed{}, weak_unwind_completed{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{}, completed_root_calls{};
    void* captured_lighting{};
    void* last_root_identity{}; // numeric diagnostic only after retirement
    void* captured_name{};
    std::uint32_t captured_name_size{};
};

// B724E0[150], ECX actual24h, stacked name pointer, EAX self, RET4. Weak base
// first; capture CURRENT name argument only after name-header zero stores.
// Captured source header survives resize; reread current length/data words.
// Final live-one copy is MOVSS. No destination allocation or host admission.
void* construct_native_gui_scene_storage_00b724e0(void* actual_scene,
    NativeGuiSceneConstructFrame&, NativeGuiSceneStorageContext&,
    NativeGuiSceneConstructAcquired&);
// B72430[166]: captured resource/current decrement/current0, clear1C AFTER
// callback, current root loop, captured name data/size across actual pool getter,
// weak base. Name cleanup consumed before getter; weak consumed before call.
void destroy_native_gui_scene_storage_00b72430(void* actual_scene,
    NativeGuiSceneDestroyFrame&, NativeGuiSceneStorageContext&,
    NativeGuiSceneLifetimeAcquired&);
// B72580[30]: destructor then CURRENT flags low-byte bit0, genuine shared CRT
// free via singleton_lifetime_free, numeric identity return; no later raw read.
// Explicit body imposes no count-zero check. flags0 still destroys the payload.
void* delete_native_gui_scene_storage_00b72580(void* actual_scene,
    const volatile std::uint32_t& flags, NativeGuiSceneDestroyFrame&,
    NativeGuiSceneStorageContext&, NativeGuiSceneLifetimeAcquired&);

struct NativeGuiSceneIdentityContext {
    NativeGuiSceneStorageContext& storage;
    const volatile std::uint32_t* scene_profile_00d62d48;
};
// Separate postconstruction canonical metadata; B724E0 has no admission call.
// Borrow SAME aligned live+4/count in storage.owners without native stores or
// extra credits. Duplicate/failed admission leaves native storage unchanged.
// Companion/context/frame/acquired remain address-stable through retirement;
// external synchronization excludes slot reuse until unbinding completes.
class NativeGuiSceneReference final : public RenderCommandReference {
public:
    NativeGuiSceneReference(void* actual_scene, NativeGuiSceneIdentityContext&,
        NativeGuiSceneDestroyFrame&, NativeGuiSceneLifetimeAcquired&);
    ~NativeGuiSceneReference() override;
    NativeGuiSceneReference(const NativeGuiSceneReference&) = delete;
    NativeGuiSceneReference& operator=(const NativeGuiSceneReference&) = delete;
    // Count already zero/current0 BD30E0; fresh current4 B72580 with flags1.
    // No second decrement. Exceptions retain the existing noexcept/terminate
    // boundary after metadata retirement, not successful native completion.
    void release_zero_references() noexcept override;
    // Separately selected direct scalar entry, including nonzero count/flags0.
    // Once entered, normal return OR source unwind retires metadata once. On
    // exception, Acquired/raw backing/unresolved credits remain for explicit
    // disposition; retirement does NOT assert destruction/free completion.
    void* delete_scalar_00b72580(const volatile std::uint32_t& flags);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiSceneIdentityContext& context_;
    NativeGuiSceneDestroyFrame& frame_;
    NativeGuiSceneLifetimeAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};

// Actual24h fields: profile00/count04/weak08/root0C/name10,14/scalar18/
// resource1C/word20. Constructor establishes an actual24h storage
// lifetime (including atomic count04) and restores all preimage bytes before
// weak925490; it introduces no owning string or logical scene object. Later
// lifetime/admission calls require this constructed storage. Caller supplies
// initialized current cells and fresh disjoint diagnostics. Constructor
// failure never frees destination. After admission all destruction/deletion
// must go through that companion; direct raw deletion would strand metadata.
// Acquisition capacity exhaustion is a HOST diagnostic boundary with prior
// mutations retained. Current tree targets require genuine admitted terminal
// providers: legacy camera/light destruction still excludes raw3Ch-attached
// nodes. No logical owner conversion, fabricated callback, ownership rollback,
// full terminal graph, native private-stack/FH3/SEH/ABI or AC59A0 admission.
} // namespace bsp
