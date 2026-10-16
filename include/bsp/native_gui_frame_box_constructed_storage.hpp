#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_gui_widget_position.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_texture_surface_getter.hpp"

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Concrete Source glue for AA7220's ALREADY captured target/current receiver.
// Only genuine B6DB10 is admitted; the whole132 mapped body performs its own
// reached notification dispatch. No receiver vptr/38 read, callback/default,
// Group40 generalization, new owner or Native body credit belongs to this glue.
class NativeGuiFrameBoxConstructedTransformDispatch final
    : public NativeGuiWidgetTransformDispatch {
public:
    explicit NativeGuiFrameBoxConstructedTransformDispatch(
        const NativeNodeMappedRawTransformProfiles& profiles) noexcept;
    void call_virtual38(std::uint32_t captured_target,
        void* current_receiver, const void* actual_matrix) override;
    const NativeNodeMappedRawTransformProfiles& profiles() const noexcept { return profiles_; }
private:
    const NativeNodeMappedRawTransformProfiles& profiles_;
};

struct NativeGuiFrameBoxConstructedStorageContext {
    NativeModelEnvironment& models;
    GuiNativeGeometryOwners& geometry;
    NativeRenderActualOwnerRegistry& owners;
    const game::GameNativeReadOnlyData& data; // already initialized/held aliases
    NativeGuiWidgetTransformBindings& position;
    NativeGuiFrameBoxConstructedTransformDispatch& dispatch; // SAME position.dispatch
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};
struct NativeGuiFrameBoxConstructedStorageFrame {
    // Stable initialized preimage, no blanket zero. Native raw allocation and
    // final xyz locals are four distinct words; xyz is written even gate skips.
    volatile std::uint32_t (&locals)[4]; // raw allocation; x/y/z
    float scalar_178, scalar_17c; // one reached x87 FLD/FST/FSTP
    NativeGuiWidgetTransformScratch& transform;
    GuiWidgetBounds& bounds;
};
struct NativeGuiFrameBoxConstructedStorageRelease {
    void* identity{}; // possibly ended storage; diagnostic, never another owner
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    std::uint32_t captured_profile{}, captured_target{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiFrameBoxConstructedStorageAcquired {
    // Public diagnostic metadata owns nothing and is NOT an uneditable token.
    // Never reset/assign/replay a reached invocation, including gate-skipped.
    enum class Boundary { none, provider_call, host_metadata };
    bool started{}, complete{}, failed{}, gate_captured{}, geometry_enabled{};
    bool factory_entered{}, factory_returned{}, geometry_entered{}, geometry_returned{};
    bool position_entered{}, position_returned{};
    bool delegated_constructor_action_consumed{}, raw_mesh_return_entered{}, raw_mesh_returned{};
    volatile std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    Boundary boundary{Boundary::none};
    void* destination{};
    void* captured_model{}; // captured BEFORE mesh allocation callbacks
    NativeModelOwner* captured_model_owner{}; // independent metadata, not ownership
    GuiNativeMeshAcquired mesh;
    NativeMeshStorage* mesh_creator{}; // sole creator; consumed BEFORE decrement
    NativeGuiFrameBoxConstructedStorageRelease mesh_release;
};

// Whole ACF8F0[ACF8F0,ACF9A8),184B/57starts. Native ECX actual FrameBox,
// plain RET, preserved EBX and conditionally ESI/EDI. New Source/context/frame
// interface, not private register/stack/FH3/SEH/IAT or binary ABI identity.
//
// Context-only preflight precedes ANY raw74/4C read. Caller supplies actual120h
// slot including hidden+11C trailer; full independent model/mesh/pool/registry,
// node/parent/child graph and distinct recursive provider backing already held
// BEFORE callbacks. All writable frame/locals/scratch/acquired storage is
// mutually separate and disjoint from the WHOLE transitive provider graph.
// Known-region checks cannot establish opaque extent/provenance/lifetimes.
// The existing geometry domain uses the SAME actual mesh environment/pool,
// profile aliases and registry. Stable pure metadata services cannot reenter.
//
// position binds real current D7A24C/D7A208 aliases and the explicit LIVE mutable
// E12FC4 scalar cell; its producer provenance/lifetime is caller proof, never a
// copied0.75/default/new global. The mapped132 profiles/data owner and this
// concrete bridge remain live together. Profiles must have been constructed
// from this SAME data service;132 exposes no owner-identity getter, so that
// exact held-service binding remains caller proof. Enabled AA70E0 additionally
// requires current model/geometry/element0 valid storage after transform calls.
//
// Current74 gate; capture current4C before B73B60/B73D70; genuine combined
// factory alone consumes constructor state0/B72F70 once. Never repeat it.
// After return one reached FLD D7A260/FST/FSTP, state-1 BEFORE B75170 on that
// captured model. No catch creator release around setter/position failures.
// Null allocation still reaches setter before the later required null-access
// Source boundary; it is never early success. CE2220 is read CURRENT at its
// decrement site. Only zero reads current mesh00/slot0 and calls the genuine
// canonical terminal without decrementing again; that terminal is noexcept.
// Registry/profile errors before terminal retain already-zero diagnostics.
//
// Final three zero floats and genuine whole AA7DC0 execute even gate skips.
// All completed creators/publications, contexts/backing and diagnostics remain
// caller-retained on entered failure, even if another provider ends storage.
// provider_call/native_site do NOT establish Native exception origin. Nested
// constructor cleanup escape terminates; no retry, raw rediscovery, automatic
// destructor/creator rollback, widget pool return or production activation.
void constructed_native_gui_frame_box_storage_00acf8f0(void* actual_widget,
    NativeGuiFrameBoxConstructedStorageFrame&, NativeGuiFrameBoxConstructedStorageContext&,
    NativeGuiFrameBoxConstructedStorageAcquired&);
} // namespace bsp
