#include "bsp/native_cockpit_helper_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void require_profile(const NativeCockpitHelperOwner& owner) {
    if (owner.storage.profile_00 != 0x00d61854u || !owner.table ||
        owner.table[0] != 0x00bd30e0u || owner.table[1] != 0x00b3c6c0u)
        throw std::logic_error("unsupported current cockpit helper terminal profile");
}
void release_node(NativeCockpitHelperOwner& owner, std::uint32_t key) {
    auto* binding = owner.nodes.attachments.find_actual_node(key);
    if (!binding) throw std::logic_error("cockpit member has no canonical actual node binding");
    unlink_and_release_render_model_00b6dfa0(*binding);
    // Terminal callbacks may have destroyed the member and its companion.
}
void finish_base(NativeCockpitHelperOwner& owner) noexcept {
    owner.storage.profile_00 = 0x00ceb130u; // BD30F0, also CBECC0's tail target
    owner.phase = NativeCockpitHelperOwner::Phase::dead;
}
}
NativeCockpitHelperOwner::NativeCockpitHelperOwner(NativeCockpitHelperStorage& actual,
    NativeNodeDestructionRuntime& runtime, const volatile std::uint32_t* actual_table)
    : storage(actual), nodes(runtime), table(actual_table) {
    require_profile(*this);
    if (storage.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("cockpit helper binding requires constructed live storage");
}
void destroy_native_cockpit_helper_00b3c5c0(NativeCockpitHelperOwner& owner) {
    if (owner.phase != NativeCockpitHelperOwner::Phase::live)
        throw std::logic_error("cockpit helper destruction cannot be retried");
    owner.phase = NativeCockpitHelperOwner::Phase::destroying;
    owner.storage.profile_00 = 0x00d61854u;
    try { // FH3 state0 is armed before the first member operation.
        if (const auto camera = owner.storage.camera_0c) {
            release_node(owner, camera);
            owner.storage.camera_0c = 0;
        }
        if (const auto attached = owner.storage.attached_08) {
            release_node(owner, attached);
            owner.storage.attached_08 = 0;
        }
        if (const auto retained = owner.storage.retained_18) {
            owner.nodes.release_retained_owner(reinterpret_cast<void*>(static_cast<std::uintptr_t>(retained)));
            owner.storage.retained_18 = 0;
        }
    } catch (...) {
        finish_base(owner); // no member retry, no later member cleanup, no free
        throw;
    }
    finish_base(owner); // state -1 before normal BD30F0
}
void* delete_native_cockpit_helper_00b3c6c0(NativeCockpitHelperOwner& owner, std::uint32_t flags) {
    void* const allocation = &owner.storage;
    destroy_native_cockpit_helper_00b3c5c0(owner);
    if (flags & 1u) singleton_lifetime_free(allocation);
    return allocation;
}
NativeCockpitHelperReference::NativeCockpitHelperReference(NativeCockpitHelperOwner& owner,
    NativeCockpitHelperCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.references_04), owner_(owner), disposal_(disposal) {
    if (owner.phase != NativeCockpitHelperOwner::Phase::live || owner.reference_bound_ ||
        !disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("cockpit helper requires unique live companion and retirement");
    require_profile(owner);
    owner.reference_bound_ = true;
}
NativeCockpitHelperReference::~NativeCockpitHelperReference() {
    if (!retired_) std::terminate();
}
void NativeCockpitHelperReference::release_zero_references() noexcept {
    if (retired_ || owner_.phase != NativeCockpitHelperOwner::Phase::live) std::terminate();
    require_profile(owner_); // current BD30E0 -> current deleting slot, flag1
    const auto disposal = disposal_;
    delete_native_cockpit_helper_00b3c6c0(owner_, 1);
    owner_.reference_bound_ = false; // companion only, actual allocation is dead
    retired_ = true;
    disposal.retire(disposal.context, *this);
    // The callback may dispose both host companions. No access follows it.
}
}
