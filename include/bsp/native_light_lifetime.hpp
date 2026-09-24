#pragma once
#include "bsp/native_node_base_destruction.hpp"
#include "bsp/native_render_context.hpp"
#include <cstdint>

namespace bsp {
class DirectionalLightPool;
struct NativeLightLifetimeContext {
    // SAME actual registry/import/type/scene/string domains throughout.
    NativeNodeBaseDestructionContext& node;
    // Required ONLY for reached zero-count shadow174. This binding must admit
    // that owner's actual FAMILY/EXTENT, resolve its current numeric profile,
    // invoke genuine current0 and retire the SAME canonical actual+4 owner.
    // It must not apply node170/root checks to a non-node shadow. No default.
    // Actual508h directional shadows are distinct from non-refcounted2Ch
    // depth targets. No new shadow companion or terminal closure is supplied.
    NativeNodeTreeRetirementDispatch* shadow_zero{};
};
struct NativeLightLifetimeFrame {
    NativeSceneRegistrationGateEraseFrame scene_remove;
    NativeNodeBaseDestructionFrame node;
};
struct NativeLightLifetimeAcquired {
    bool started{}, complete{}, exception_cleanup_started{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{}, completed_scene_releases{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    void* captured_unregister_scene{};
    void* captured_released_scene{};
    void* captured_shadow{};
    bool shadow_decrement_completed{}, shadow_terminal_completed{};
    bool array_cleanup_completed{}, pool_returned{};
    NativeNodeBaseDestructionAcquired node;
};

// Complete23B B7C1C0, ECX actual12B borrowed-pointer array, RET. Valid
// nonnegative count/capacity admits genuine B7BC70 resize0 with no reserve,
// then capture CURRENT begin and shared CRT free. Preserve begin/capacity.
// No SceneResource object is viewed/destroyed and no element credits change.
// Starts the trivial12B descriptor lifetime and restores all preimage bytes
// before typed resize access; raw B7C4C0 itself writes only assembly words.
void destroy_native_light_scene_array_00b7c1c0(void* actual_array) noexcept;

// Complete250B B7C5B0: ECX actual light, RET. Stamp D62F58; capture current
// CE2220 once BEFORE the signed drain, reuse it for every scene and shadow.
// Gate sees first current back; release rereads CURRENT begin/count afterward;
// then decrement CURRENT count if nonzero and test CURRENT signed count.
// Raw scene identities and actual3Ch canonical owners only; no logical owner.
// Caller initializes frame preimages and retains frame/acquisitions on failure.
void destroy_native_light_storage_00b7c5b0(void* actual_light,
    NativeLightLifetimeFrame&, NativeLightLifetimeContext&,
    NativeLightLifetimeAcquired&);

// Complete38B B7C820: stamp D62FB0, call full Light body, then reread CURRENT
// flags lowbyte; bit0 returns the SAME1F0 slot to actual01090154/B7B2F0.
// Real DirectionalLightPool, live+1EC slab metadata, no later payload access.
// Direct scalar permits nonzero count and flags0; it adds no decrement.
void* delete_native_directional_light_storage_00b7c820(void* actual_light,
    const volatile std::uint32_t& flags, DirectionalLightPool&,
    NativeLightLifetimeFrame&, NativeLightLifetimeContext&,
    NativeLightLifetimeAcquired&);

struct NativeDirectionalLightIdentityContext {
    NativeLightLifetimeContext& lifetime;
    NativeRenderActualOwnerRegistry& owners;
    DirectionalLightPool& pool_01090154;
    const volatile std::uint32_t* actual_profile_00d62fb0;
};
// Separate postconstruction canonical metadata over genuine B7C6B0 raw1F0
// pool storage; no DirectionalLightOwner/SceneAttachmentRuntime conversion.
// Raw B7C6B0 -> B7C4C0 -> B6F5A0 materializes NativeNodeStorage and its
// atomic+4. Admission requires that completed, still-live constructor path.
// Binds SAME actual+4/registry, writes no native bytes and adds no credit.
// Duplicate refusal leaves native owner unchanged. This is not native factory
// behavior. Once admitted, all destruction goes through this companion.
class NativeDirectionalLightStorageReference final : public RenderCommandReference {
public:
    NativeDirectionalLightStorageReference(void* actual_light,
        NativeDirectionalLightIdentityContext&, NativeLightLifetimeFrame&,
        NativeLightLifetimeAcquired&);
    ~NativeDirectionalLightStorageReference() override;
    NativeDirectionalLightStorageReference(const NativeDirectionalLightStorageReference&) = delete;
    NativeDirectionalLightStorageReference& operator=(const NativeDirectionalLightStorageReference&) = delete;
    // Observed count0/current0 BD30E0 -> freshly current4 B7C820/flags1.
    void release_zero_references() noexcept override;
    // Explicit direct entry also permits nonzero count and flags0. Metadata
    // retires once after normal return or source unwind; failure retirement
    // does NOT assert completed destruction or pool return. No dead-slot read.
    void* delete_scalar_00b7c820(const volatile std::uint32_t& flags);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeDirectionalLightIdentityContext& context_;
    NativeLightLifetimeFrame& frame_;
    NativeLightLifetimeAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};

// Valid descriptor domain: nonnegative count/capacity and accessible reached
// slots. After B83EC0, its freshly selected back element MUST remain valid;
// count0 there would be an invalid native read, not a successful skipped release.
// After terminal callbacks count0 is supported. Backing uses shared real CRT.
// A decrement returning zero must leave that same actual count at zero for
// canonical dispatch; count-changing return continuations are not admitted.
// Contexts/companions/frames/diagnostics remain stable; external quiescence
// prevents slot reuse until metadata retirement. Preserve unresolved credits,
// backing and failure prefixes for disposition; no rollback or retry is added.
// Null shadow or nonzero remaining shadow count is directly admitted. A zero
// shadow needs the explicitly required genuine provider; no full raw shadow/
// camera terminal graph, FH3/SEH/private ABI/application/AC59A0 claim follows.
} // namespace bsp
