#pragma once
#include "bsp/native_render_zero_frame.hpp"

#include "bsp/render_command_queue.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

namespace bsp {

// Actual 18h-byte Win32 storage. All owner fields contain raw native identities,
// never the address of a host RenderCommandReference companion.
struct NativeRenderContextStorage {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
    void* camera_08;
    void* second_owner_0c;
    void* borrowed_command_10;
    void* target_14;
};
static_assert(sizeof(NativeRenderContextStorage) == 0x18);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);
static_assert(offsetof(NativeRenderContextStorage, references_04) == 4);
static_assert(offsetof(NativeRenderContextStorage, camera_08) == 8);
static_assert(offsetof(NativeRenderContextStorage, second_owner_0c) == 0x0c);
static_assert(offsetof(NativeRenderContextStorage, borrowed_command_10) == 0x10);
static_assert(offsetof(NativeRenderContextStorage, target_14) == 0x14);

// Required canonical owner lookup, called ONLY after actual +04 reaches zero.
// Lookup must have no ownership/lifetime side effects. Its result borrows that
// exact atomic, and its nonthrowing terminal method dispatches the owner's
// CURRENT native virtual0 profile to the real destruction/pool-return path.
// Missing identities/profiles are errors, never a no-op fallback. The owner,
// atomic and canonical companion must survive until their terminal callback.
class NativeRenderActualOwners {
public:
    virtual ~NativeRenderActualOwners() = default;
    virtual RenderCommandReference& resolve_actual(void* raw_identity) = 0;
};

// Application-owned canonical metadata for actual render-resource companions.
// Binding borrows both objects, validates that the companion uses raw+04, and
// performs no native write, retain or release. One live identity has exactly
// one companion; replacement is never implicit. All companions must unbind and
// pending admissions must cancel before this registry is destroyed. The
// registry outlives every active token. It serializes its metadata only
// and holds no lock across terminal dispatch; callers still provide the native
// resource/lifetime synchronization that keeps a resolved companion alive.
class NativeRenderActualOwnerRegistry final : public NativeRenderActualOwners {
public:
    // Internal typed-owner metadata node. Naming the type grants no permission;
    // only genuine Mesh/Model owning continuations below can construct/mutate it.
    class ExactRecord final {
    public:
        ~ExactRecord() = default;
        ExactRecord(const ExactRecord&) = delete;
        ExactRecord& operator=(const ExactRecord&) = delete;
        ExactRecord(ExactRecord&&) = delete;
        ExactRecord& operator=(ExactRecord&&) = delete;
    private:
        friend class NativeRenderActualOwnerRegistry;
        friend class NativeMeshCanonicalRecord;
        friend class NativeMeshCanonicalAccess;
        friend class NativeMeshCanonicalDomain;
        friend class NativeMeshCanonicalBorrow;
        friend class NativePostEffectModelFrame;
        friend class NativePostEffectParentFrame;
    friend class NativeMaterialFrame;
    friend class NativeMeshSectionLifetimeFrame;
        enum class Availability : std::uint8_t { unavailable, live };
        ExactRecord(NativeRenderActualOwnerRegistry& registry,
            const void* actual_owning_record_identity) noexcept
            : registry_(&registry), owning_identity_(actual_owning_record_identity) {}
        NativeRenderActualOwnerRegistry* const registry_;
        const void* const owning_identity_;
        void* actual_key_{};
        RenderCommandReference* canonical_{};
        std::atomic<Availability> availability_{Availability::unavailable};
        std::atomic<bool> poisoned_{false};
        std::atomic<bool> normal_native_dead_{false};
        std::atomic<bool> unborn_abandoned_{false};
        // Genuine selected ctor-exception C++ prefix end, not Native death/free.
        std::atomic<bool> partial_cpp_ended_{false};
        std::atomic<std::uint32_t> outer_calls_{0}, borrows_{0};
        ExactRecord* previous_{};
        ExactRecord* next_{};
        ExactRecord* bucket_previous_{};
        ExactRecord* bucket_next_{};
        bool retained_member_{};
        bool retired_{};
    };
    // Owns one unavailable entry in this same registry. Its mapped-value address
    // survives rehash; no iterator or Native storage is retained or inspected.
    // Move/cancel/publish share the caller's serialized ownership contract.
    class BindingAdmission final {
    public:
        BindingAdmission() noexcept = default;
        BindingAdmission(const BindingAdmission&) = delete;
        BindingAdmission& operator=(const BindingAdmission&) = delete;
        BindingAdmission(BindingAdmission&&) noexcept;
        BindingAdmission& operator=(BindingAdmission&&) noexcept;
        ~BindingAdmission() noexcept;
        void cancel() noexcept;
        explicit operator bool() const noexcept { return registry_ != nullptr; }
    private:
        friend class NativeRenderActualOwnerRegistry;
        BindingAdmission(NativeRenderActualOwnerRegistry&, void*,
            RenderCommandReference**) noexcept;
        NativeRenderActualOwnerRegistry* registry_{};
        void* identity_{};
        RenderCommandReference** entry_{};
    };

    NativeRenderActualOwnerRegistry();
    ~NativeRenderActualOwnerRegistry() override;
    NativeRenderActualOwnerRegistry(const NativeRenderActualOwnerRegistry&) = delete;
    NativeRenderActualOwnerRegistry& operator=(const NativeRenderActualOwnerRegistry&) = delete;
    NativeRenderActualOwnerRegistry(NativeRenderActualOwnerRegistry&&) = delete;
    NativeRenderActualOwnerRegistry& operator=(NativeRenderActualOwnerRegistry&&) = delete;

    void bind(void* raw_identity, RenderCommandReference&);
    // Reserve an aligned opaque key before Native construction, without reading
    // raw+04. May allocate/throw; any existing key is rejected, even if prepared.
    [[nodiscard]] BindingAdmission prepare_binding(void* raw_identity);
    // Requires this registry's exact still-null entry, then the SAME live +04
    // alias checks as bind. After successful locking/validation, only fill that
    // entry and consume the token: no insertion, rehash, retain or callback.
    // Invalid capabilities/aliases/counts terminate; mutex failure can throw.
    void publish(BindingAdmission&&, RenderCommandReference&);
    // Exact-match retirement for nonthrowing native terminal callbacks. A
    // missing/mismatched binding is a lifetime contract violation.
    void unbind(void* raw_identity, RenderCommandReference&) noexcept;
    RenderCommandReference* find(void* raw_identity);
    RenderCommandReference& resolve_actual(void* raw_identity) override;
    // Includes unavailable prepared entries; find/resolve cannot use them.
    std::size_t size() const;
    bool empty() const;

    // Function-pointer adapters for GuiNativeGeometryRegistration and every
    // other owner family that shares that same registration shape.
    static void bind_callback(void*, void* raw_identity, RenderCommandReference&);
    static void unbind_callback(void*, void* raw_identity,
        RenderCommandReference&) noexcept;
    static RenderCommandReference* find_callback(void*, void* raw_identity);

private:
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalAccess;
    friend class NativePostEffectModelFrame;
    friend class NativePostEffectParentFrame;
    friend class NativeMaterialFrame;
    friend class NativeMeshSectionLifetimeFrame;
    // Cold attachment counts even an unkeyed allocating/unknown record. Nodes
    // are already allocated; no allocator/callback runs under this mutex.
    std::mutex& exact_metadata_mutex() noexcept;
    void attach_exact_record(ExactRecord&);
    void attach_exact_record_locked(ExactRecord&) noexcept;
    // Captured key is supplied only at the genuine allocation continuation.
    void prepare_exact_record(ExactRecord&, void* captured_actual_key);
    // NEW private release publication: entry established before birth, genuine
    // private ctor continuation and reserved canonical established beforehand.
    // No mutex, allocation, validation, Native count/profile read or callback.
    void publish_exact_record(ExactRecord&, RenderCommandReference&) noexcept;
    // Poison prevents lookup/replacement without an independent genuine end fact.
    // Normal death, never-born abandonment and selected partial C++ end differ;
    // none by itself invents a physical-return receipt.
    void poison_exact_record(ExactRecord&) noexcept;
    void deactivate_exact_record_from_native_end(ExactRecord&) noexcept;
    // Authentic family flags0: Material110h/Section60h carrier and physical debt stay live.
    // Availability only: no poison, Native/partial/unborn end or failure fact.
    void withhold_exact_record_for_live_storage_debt(ExactRecord&) noexcept;
    // Only the selected ctor-exception owner, after actual trivial prefix end
    // and last prefix use, before real raw return. Separately live pool backing:
    // Material+110 / Section+60, or actual Parent+4 atomic end before its
    // SAME reached initializer raw free. Only the private family issuer proves
    // birth/escape/last-use/end; no whole Native return/publication/borrow.
    // Retains node/caller/debt; grants no normal death, refund or physical result.
    void deactivate_exact_record_from_partial_cpp_end(ExactRecord&) noexcept;
    // Genuine selected raw-slot unwind only (Mesh state3 / Model state4):
    // captured allocation, BEFORE ANY Owner/Node/payload C++ birth or native
    // ctor entry, no canonical/pub, last PAYLOAD borrow ended. The separate
    // BC / Model184 pool word remains live for the unchanged real return.
    void abandon_exact_unborn_record(ExactRecord&) noexcept;
    // Parent initializer action0 only: SAME captured allocation, before +4/
    // Owner/canonical/publication birth. Unlike Mesh/Model's seam above, a
    // genuine caught constructor attempt may have entered but not born +4.
    // Retains failed membership/debt; no Native death/free/close/refund fact.
    void abandon_exact_parent_record_before_cpp_birth(ExactRecord&) noexcept;
    // Retire THAT retained record; never erase a replacement or accept a
    // missing arbitrary key. Native physical/flags0 debt stays with the owner.
    void retire_exact_record(ExactRecord&);
    // Owner has proved actual full disposition, no outer calls/borrows/links,
    // and all Source fences. Unlink under mutex; destruction is outside it.
    // empty()/size() are membership observations, never teardown authority.
    void close_exact_record_at_quiescence(ExactRecord&);
    // Authentic owning continuation only: no allocator entry, or actual normal
    // NULL allocation; never an uncertain entered allocation or partial birth.
    // No key/publication/borrow/outer call remains; unlinks without death facts.
    void cancel_exact_record_without_payload(ExactRecord&);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Host helper shared by actual native owners. Nonnull, aligned, live atomic at
// raw+04 is a precondition. Decrement captured raw+04 first; resolve/validate the
// companion only on zero. No use of raw storage/companion after its callback.
void release_native_render_actual_owner(NativeRenderActualOwners&, void* raw_identity);

// Retained metadata for one SAME actual +04 release. The result is derived
// from the atomic's observed prior value, never a later count read. A failed
// zero dispatch leaves its entered phase and the already-zero obligation.
// Observation storage is disjoint from the raw owner and its companion and
// survives any terminal callback; it must not be reused for another release.
struct NativeRenderActualOwnerReleaseObservation {
    void* identity{};
    std::int32_t previous_count{};
    std::uint32_t result_bits{};
    bool entered{}, decremented{}, zero_dispatch_entered{}, zero_dispatch_returned{}, returned{};
    // Separate C++ outer-call custody; public diagnostics never issue an action.
    // Keep through real postcall/health/catch; no reset as Native HOME reuse.
    NativeRenderObservedZeroFrame cpp_zero_frame;
};
void release_native_render_actual_owner_observed(NativeRenderActualOwners&, void* raw_identity,
    NativeRenderActualOwnerReleaseObservation&);

// Shared post-zero dispatch ONLY: caller has already decremented the actual
// live +04 atomic once and observed zero. Resolve the SAME canonical companion,
// validate its borrowed +04 identity, then invoke its nonthrowing current-profile
// terminal. Never decrement, initialize, retain, retry or touch the owner after
// terminal return. Lookup/identity errors leave the already-zero obligation
// with the caller. Native lifetime synchronization is still required.
void dispatch_native_render_actual_owner_zero(NativeRenderActualOwners&, void* raw_identity);

// Placement-only fragment B1EDD3..B1EDF2 within B1EDC0: publish CEB130, set +04
// to1, publish D5E5C4, then clear +08/+0C/+10/+14 in order. Valid storage required.
// No allocation, command+28 publication, retained assignment or batch creation.
NativeRenderContextStorage* initialize_native_render_context_00b1edc0_fragment(void*) noexcept;

// Original ECX=context, RET. Set D5E5C4, release and then clear +08,+0C,+14;
// reload each next identity after the preceding callback. Preserve +04/+10.
// Native state0 unwind and normal completion both publish base CEB130. Lookup
// exceptions propagate after that cleanup; unvisited fields remain untouched.
void destroy_native_render_context_00b1d120(
    NativeRenderContextStorage&, NativeRenderActualOwners&);
// Original ECX=context, stack flags, EAX=original address even after free, RET4.
// Free via the shared actual allocator iff flags&1, only after successful dtor.
NativeRenderContextStorage* delete_native_render_context_00b1d570(
    NativeRenderContextStorage*, NativeRenderActualOwners&, std::uint32_t flags);

class NativeRenderContextReference;
struct NativeRenderContextCompanionDisposal {
    void* context;
    // After native destruction/free. Remove caller-owned lookup binding and/or
    // dispose this companion; no access to it or its native storage follows.
    void (*retire)(void*, NativeRenderContextReference&) noexcept;
};

// Stable host companion for the SAME +04, usable by RenderCommandReference
// retain/release helpers. Construction neither initializes nor retains it.
// One canonical companion per live context is a caller precondition. Native
// storage and lookup/profile dependencies outlive all references. The terminal
// path owns scalar deletion; callers must not directly destroy it once bound.
class NativeRenderContextReference final : public RenderCommandReference {
public:
    NativeRenderContextReference(NativeRenderContextStorage&, NativeRenderActualOwners&,
        const volatile std::uint32_t* vtable_00d5e5c4,
        NativeRenderContextCompanionDisposal);
    ~NativeRenderContextReference() override;
    NativeRenderContextReference(const NativeRenderContextReference&) = delete;
    NativeRenderContextReference& operator=(const NativeRenderContextReference&) = delete;

    NativeRenderContextStorage& storage() noexcept { return storage_; }
    // Requires CURRENT D5E5C4 slots BD30E0/B1D570. Base CEB130 and unverified
    // derived profiles have no fallback. Dependencies must be nonthrowing here.
    void release_zero_references() noexcept override;

private:
    enum class Phase { bound, destroying, retired };
    NativeRenderContextStorage& storage_;
    NativeRenderActualOwners& owners_;
    const volatile std::uint32_t* vtable_00d5e5c4_;
    NativeRenderContextCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};

} // namespace bsp
