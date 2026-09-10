#pragma once

#include "bsp/render_command_queue.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>

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

// Host helper shared by actual native owners. Nonnull, aligned, live atomic at
// raw+04 is a precondition. Decrement captured raw+04 first; resolve/validate the
// companion only on zero. No use of raw storage/companion after its callback.
void release_native_render_actual_owner(NativeRenderActualOwners&, void* raw_identity);

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
