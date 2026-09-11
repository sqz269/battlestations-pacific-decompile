#pragma once

#include "bsp/effect_admission.hpp"
#include "bsp/native_live_effect_manager.hpp"

namespace bsp {

// Complete lifetime of the separate 8h deletion-lock owner at F87654/D0D3D0.
// Reuse the existing physical two-word owner and concrete shared-domain access;
// supply a DISTINCT actual publication cell from the insertion lock at F87650.
// The application's real lifetime callback must dispatch866790 for this owner.
void unwind_effect_deletion_lock_base_00866040(EffectManager&,
    EffectManager* volatile& actual_global_00f87654) noexcept;
EffectManager& construct_effect_deletion_lock_008662b0(EffectManager&,
    EffectManager* volatile& actual_global_00f87654, EffectManagerLifetimeAccess&);
EffectManager* effect_deletion_lock_singleton_00866500(
    EffectManager* volatile& actual_global_00f87654, EffectManagerLifetimeAccess&);
EffectManager* delete_effect_deletion_lock_00866790(EffectManager*, std::uint32_t flags,
    EffectManager* volatile& actual_global_00f87654, EffectManagerLifetimeAccess&) noexcept;

// Complete8665F0: three stack DWORDs (next, previous, source-cell address),
// EAX allocated node, RET0C. ECX is unused. Source is read AFTER allocation and
// link publication, only if the computed payload address is nonnull. No retain.
NativeEffectDeletionNode* create_effect_deletion_node_008665f0(
    NativeEffectDeletionNode* next, NativeEffectDeletionNode* previous,
    const void* source_pointer_cell);

// Complete8675E0: ECX actual0Ch list, stack DWORD increment, RET4. Preserve
// unsigned (3FFFFFFF - captured_count) comparison/wrap. Overflow builds the
// actual legacy string/length-error owner and throws the existing owning host
// NativeAliasListLengthError transport. Native exception ABI is not emulated.
void grow_effect_deletion_list_count_008675e0(
    NativeEffectDeletionListStorage&, std::uint32_t increment);

// Complete868010: ECX actual28h live manager, stack raw effect, RET4. Capture
// separate866500 lock, then current head/previous, allocate node, increment count,
// splice through captured head and new node's current previous. Holds the real
// section across all operations. Failure unlocks; no unlinked-node rollback or
// effect release is present in native state0. No retain, including for null input.
void enqueue_effect_deletion_00868010(NativeLiveEffectManagerStorage&, void* actual_effect,
    EffectManager* volatile& actual_lock_global_00f87654, EffectManagerLifetimeAccess&);

class EffectDeletionDispatch {
public:
    virtual ~EffectDeletionDispatch() = default;
    // REQUIRED current native virtual+04 scalar destructor for this EXACT raw
    // owner, with supplied flags. No reference decrement, virtual0 substitution,
    // alternate owner or successful fallback. Missing bindings must fail.
    virtual void scalar_delete_current_04(void* actual_owner, std::uint32_t flags) = 0;
    // BF6713 may return. The caller continues with its captured node afterward.
    virtual void invalid_parameter_00bf6713() = 0;
};

// Complete8671A0: ECX actual28h manager, RET. No internal lock. While current
// count != 0, dispatch captured head->next payload through current virtual04(1),
// clear captured payload AFTER callback, reload head/front for unlink, free it
// and decrement CURRENT count. Payloads/links may change during callbacks;
// captured storage must remain alive until its native last access.
void flush_effect_deletions_008671a0(NativeLiveEffectManagerStorage&, EffectDeletionDispatch&);

// New C++ interfaces with actual native storage and canonical lock bindings.
// Whole live-manager lifetime and frame-job-policy/virtual0 composition remain
// separate; no native integer vtable address is called as host machine code.
} // namespace bsp
