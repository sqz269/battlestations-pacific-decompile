#pragma once
#include "bsp/native_model_pool.hpp"
#include "bsp/native_node_base_destruction.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_node_construction.hpp"

namespace bsp {
// One real node/string/import/actual-owner domain. Constants borrow CURRENT
// CE4970/CE4ADC/D7A24C and D7A208 cells; no copied defaults or semantic Model
// environment. The initialized pool is the SAME actual01090054 owner.
struct NativeModelStorageContext {
    NativeNodeBaseDestructionContext& node;
    NativeModelPool& pool_01090054;
    const NativeNodeRawConstants& constants;
    const volatile std::uint32_t& negative_zero_00d7a208;
};
struct NativeModelStorageConstructAcquired {
    bool started{}, node_constructor_returned{}, tail_constructed{}, complete{};
    bool metadata_bound{};
    std::uint32_t active_call_site{};
    void* actual_model{};
    NativeModelStorageContext* context{};
};
struct NativeModelStorageMemberAcquired {
    bool started{}, complete{}, decrement_completed{}, terminal_completed{}, field_cleared{};
    std::uint32_t active_call_site{};
    void* captured_owner{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
};
struct NativeModelStorageLifetimeFrame {
    NativeNodeBaseDestructionFrame node{};
    // BD30E0's Source current04 binding seeds THIS persistent argument cell.
    // B75290 rereads its low byte after destruction; callbacks may change it.
    volatile std::uint32_t scalar_flags_argument{};
};
struct NativeModelStorageLifetimeAcquired {
    bool started{}, complete{}, exception_cleanup_started{};
    bool retained_decrement_completed{}, retained_terminal_completed{}, retained_cleared{};
    bool geometry_decrement_completed{}, geometry_terminal_completed{}, geometry_cleared{};
    bool scalar_started{}, scalar_complete{}, flags_read{}, pool_returned{};
    std::uint8_t flags_low_byte{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    void* captured_retained{};
    void* captured_geometry{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    NativeModelStorageMemberAcquired geometry_unwind;
    NativeNodeBaseDestructionAcquired node;
};

// Optional HOST extent completion for a canonical NativeModelOwner transport.
// The raw bodies below do not own that caller's typed node/tail extents. This
// nonthrowing callback ends those extents/metadata only, never native cleanup,
// reference adjustment, allocation or another B6F440. Its backing survives the
// destructor and its explicit failure disposition.
struct NativeModelStorageHostCompletion {
    void* context;
    void (*end_typed_storage)(void*) noexcept;
};

// Complete B75030[132B], ECX actual188h slot/stack name, EAX self, RET4.
// Raw B6F5A0 then CURRENT model constants/tail/pose stores. Establishes only
// the real node prefix and trivial model tail; preserves pool index184.
// Constructor failure leaves physical-slot custody with its caller; B6F5A0
// already consumes its own prefix cleanup, so do not repeat that destruction.
void* construct_native_model_storage_00b75030(void* actual_model,
    const void* actual_name_header, NativeModelStorageContext&,
    NativeModelStorageConstructAcquired&);

// B74F20[43B], native ECX=model+178, RET. Capture CURRENT member+8 geometry,
// then use the FRESH current CE2220 target and captured actual+4/current0.
// Clear the ORIGINAL member+8 only after callback; no containing-model release.
void destroy_native_model_geometry_storage_00b74f20(void* actual_member_178,
    NativeModelStorageContext&, NativeModelStorageMemberAcquired&);

// B750C0[165B], ECX model/no args, RET. Capture174, then sample CE2220 ONCE;
// use that epoch for normal174 AND freshly reloaded180. Clear each field only
// after callback. State1 unwind uses B74F20's FRESH import, then raw B6F440;
// state0 only B6F440; state-1 never repeats base cleanup. C++ projection only.
void destroy_native_model_storage_00b750c0(void* actual_model,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&);
// Same complete body; complete HOST extents after raw node destruction, also
// on an escaping C++ exception once that raw base was entered. No cleanup replay.
void destroy_native_model_storage_00b750c0(void* actual_model,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&, const NativeModelStorageHostCompletion&);

// B75290[32B], ECX model/stack flags, EAX original identity, RET4. The Source
// API borrows an actual initialized flags DWORD: read CURRENT LOW BYTE only
// AFTER B750C0, then bit0 returns the SAME slot via B74750/actual01090054.
// Flags may alias still-accessible model storage; they do not alias context or
// diagnostics. The designated frame flags cell is also supported. No count
// decrement/gate is added; flags0 destroys payload without physical return.
void* delete_native_model_storage_00b75290(void* actual_model,
    const volatile std::uint32_t& actual_flags_argument,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&);
// Completion precedes CURRENT flags LOW BYTE and any physical pool return.
// Existing no-completion callers keep their original raw-storage contract.
void* delete_native_model_storage_00b75290(void* actual_model,
    const volatile std::uint32_t& actual_flags_argument,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&, const NativeModelStorageHostCompletion&);

struct NativeModelStorageIdentityContext {
    NativeModelStorageContext& storage;
    const volatile std::uint32_t* actual_profile_00d62de8;
};
// Separate postconstruction canonical metadata, never another native credit.
// Requires the completed construction record from THIS context, SAME live188h
// pool membership/current184/free-index state and live actual+4 atomic.
// Current D62DE8[0/4] must be BD30E0/B75290. Registry admission may throw before
// metadata_bound is set; the completed raw owner then remains caller-owned.
class NativeModelStorageReference final : public RenderCommandReference {
public:
    NativeModelStorageReference(void* actual_model, NativeModelStorageIdentityContext&,
        NativeModelStorageConstructAcquired&, NativeModelStorageLifetimeFrame&,
        NativeModelStorageLifetimeAcquired&);
    // RESERVED birth: the SAME registry/actual Model identity was reserved
    // before Native construction. Reuses every ordinary admission/count check;
    // metadata_bound is set only after successful existing publish. No new
    // Native credit, insertion/rehash, retain or cleanup. Mutex locking may
    // throw; invalid capabilities/aliases/counts keep publish's terminal contract.
    // Keep the caller-owned lvalue token/registry alive, stable and serialized;
    // token backing is separate from actual Model/companion/reached writes.
    // A C++ escape leaves the completed raw owner, custody and reservation
    // with the caller, metadata_bound still false, for explicit disposition.
    // This path neither cancels the token nor classifies Native exceptions.
    NativeModelStorageReference(void* actual_model, NativeModelStorageIdentityContext&,
        NativeModelStorageConstructAcquired&, NativeModelStorageLifetimeFrame&,
        NativeModelStorageLifetimeAcquired&,
        NativeRenderActualOwnerRegistry::BindingAdmission& caller_reservation);
    ~NativeModelStorageReference() override;
    NativeModelStorageReference(const NativeModelStorageReference&) = delete;
    NativeModelStorageReference& operator=(const NativeModelStorageReference&) = delete;
    // SAME observed actual+4 zero/current0 BD30E0 -> freshly current04
    // B75290/flags1 in the persistent argument cell. Escaping failure terminates.
    void release_zero_references() noexcept override;
    void* delete_scalar_00b75290(const volatile std::uint32_t& actual_flags_argument);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeModelStorageIdentityContext& context_;
    NativeModelStorageLifetimeFrame& frame_;
    NativeModelStorageLifetimeAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};

// Qualified MSVC Win32 Source interfaces, not native private stack/FH3/SEH or
// binary ABI. Contexts, constants, prepared frames, construction/lifetime
// diagnostics and companions remain disjoint/stable through callbacks/failure
// disposition, pool return and metadata retirement. No asynchronous mutation
// or slot reuse before external quiescence. All admitted deletion goes through
// the canonical reference. Metadata retirement does NOT prove cleanup/return
// after failure; preserve backing and unresolved credits, never rollback/retry.
// Nonnull174/180 require the existing genuine NativeRenderActualOwners current0
// contract over those SAME actual counts; no node-extent/scene-null check is
// applied to them. All reached raw node/tree/world/scene/point-light/string
// providers and successful accessible memory operations remain caller-required.
// No NativeModelOwner, NativeNodeBinding, logical SceneResource, child suffix,
// parenting transport, P3a2 publication or GUI/game completion is supplied.
} // namespace bsp
