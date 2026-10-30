#include "bsp/native_gui_text_failure_origin.hpp"
// Exact installed SDK declarations/types. Root must add the selected SDK
// crt/src/vcruntime include directory for THIS translation unit only.
#ifndef _VCRT_ALLOW_INTERNALS
#define _VCRT_ALLOW_INTERNALS
#define BSP_TEXT_UNDEFINE_VCRT_INTERNALS
#endif
#include <ehdata_forceinclude.h>
#include <trnsctrl.h>
#ifdef BSP_TEXT_UNDEFINE_VCRT_INTERNALS
#undef BSP_TEXT_UNDEFINE_VCRT_INTERNALS
#undef _VCRT_ALLOW_INTERNALS
#endif
static_assert(_MSC_FULL_VER == 195136244, "requalify the active-cause reader for another Source toolchain");

namespace bsp {
NativeGuiTextFailureCause NativeGuiTextFailureOriginRuntime::read_current_catch() const {
    NativeGuiTextFailureCause key;
    // Precondition: same established supported Source-runtime C++ catch/PTD.
    // Never use EH-record address as identity or invent a private layout.
    auto* const record = static_cast<EHExceptionRecord*>(*__current_exception());
    if (record && PER_IS_MSVC_EH(record) && PER_PEXCEPTOBJ(record) && PER_PTHROW(record)) {
        key.runtime_ = this; key.object_ = PER_PEXCEPTOBJ(record); key.descriptor_ = PER_PTHROW(record);
    }
    return key;
}
void NativeGuiTextFailureOrigin::begin(const NativeGuiTextFailureOriginRuntime& r,
    const void* supplier, std::uint32_t entry) noexcept {
    if (phase_ != Phase::fresh || !supplier) { valid_ = false; return; }
    runtime_ = &r; supplier_ = supplier; entry_ = entry; phase_ = Phase::entered;
}
void NativeGuiTextFailureOrigin::complete() noexcept {
    if (phase_ == Phase::entered && verdict_ == Verdict::unset) phase_ = Phase::complete;
    else valid_ = false;
}
NativeGuiTextFailureOrigin::Edge NativeGuiTextFailureOrigin::enter(Edge edge) noexcept {
    const auto previous = active_;
    if (phase_ != Phase::entered || committed()) valid_ = false;
    else { edge.parent_site = previous.site; edge.parent_supplier = previous.supplier; active_ = edge; }
    return previous;
}
void NativeGuiTextFailureOrigin::leave(Edge previous) noexcept {
    if (!committed()) active_ = previous;
}
void NativeGuiTextFailureOrigin::commit(Verdict verdict, NativeGuiTextFailureCause cause) noexcept {
    // Every committed denial is sticky. Never promote/replace it for a later
    // same-object sibling, replacement, caught reentry or secondary exception.
    if (committed()) return;
    cause_ = cause; escaped_ = active_; phase_ = Phase::failed;
    verdict_ = valid_ && cause.valid() && cause.runtime_ == runtime_ ? verdict : Verdict::unclassified;
}
void NativeGuiTextFailureOrigin::retain_current(const NativeGuiTextFailureOriginRuntime& r) {
    commit(Verdict::unclassified, r.read_current_catch());
}
void NativeGuiTextFailureOrigin::secondary(NativeGuiTextFailureCause cause, Edge edge) noexcept {
    if (secondary_committed_) return;
    secondary_cause_ = cause; secondary_edge_ = edge; secondary_committed_ = true;
    phase_ = Phase::failed; // primary receipt preserved; secondary never inherits its permit
}
void NativeGuiTextFailureOrigin::adopt(NativeGuiTextFailureCause cause,
    const NativeGuiTextFailureOrigin& child, const void* supplier, std::uint32_t entry) noexcept {
    if (committed()) return;
    const bool exact = valid_ && cause.valid() && cause.runtime_ == runtime_ &&
        active_.supplier == supplier && active_.stage == Stage::ordinary && child.valid_ &&
        child.phase_ == Phase::failed && !child.secondary_committed_ && child.runtime_ == runtime_ && child.supplier_ == supplier &&
        child.entry_ == entry && child.cause_.same(cause) && child.escaped_.stage == Stage::ordinary;
    if (exact) { forwarded_child_ = &child; forwarded_child_edge_ = child.escaped_; }
    // No Native propagation/minting is supplied. Unknown remains unknown.
    commit(exact && child.verdict_ == Verdict::host ? Verdict::host : Verdict::unclassified, cause);
}
void NativeGuiTextFailureHeaderArm::arm(const NativeGuiTextFailureOrigin& invocation,
    const void* header, std::uint32_t state) noexcept {
    if (phase_ != Phase::fresh) { phase_ = Phase::failed; return; }
    invocation_ = &invocation; header_ = header; state_ = state; phase_ = Phase::armed;
}
void NativeGuiTextFailureHeaderArm::normal_return_entered() noexcept {
    phase_ = phase_ == Phase::armed ? Phase::normal_return : Phase::failed;
}
void NativeGuiTextFailureHeaderArm::returned() noexcept {
    phase_ = phase_ == Phase::normal_return || phase_ == Phase::unwind_return ? Phase::returned : Phase::failed;
}
void NativeGuiTextFailureHeaderArm::return_failed() noexcept { phase_ = Phase::failed; }
bool NativeGuiTextFailureHeaderArm::consume_proven_primary(const NativeGuiTextFailureOrigin& invocation,
    const NativeGuiTextFailureCause& current, const void* actual_header, std::uint32_t state) noexcept {
    const bool allowed = phase_ == Phase::armed && invocation_ == &invocation && header_ && header_ == actual_header && state_ == state &&
        invocation.valid_ && invocation.phase_ == NativeGuiTextFailureOrigin::Phase::failed &&
        invocation.verdict_ == NativeGuiTextFailureOrigin::Verdict::proven_native &&
        !invocation.secondary_committed_ && invocation.native_issuer_ && invocation.cause_.same(current) &&
        invocation.escaped_.stage == NativeGuiTextFailureOrigin::Stage::ordinary;
    if (allowed) phase_ = Phase::unwind_return;
    return allowed; // always withheld in this slice: no genuine Native issuer exists
}
void NativeGuiTextFailureConstructorSlotArm::arm(NativeGuiTextFailureOrigin& invocation,
    const NativeGuiTextFailureOriginRuntime& runtime, volatile std::uint32_t* home,
    void* receiver, const void* provider, NativeGuiTextFailureOrigin::Site edge,
    NativeGuiTextFailureConstructorCall& call) noexcept {
    if (phase_ != Phase::fresh || call.phase_ != NativeGuiTextFailureConstructorCall::Phase::fresh) {
        phase_ = Phase::denied; call.valid_ = false; return;
    }
    invocation_ = &invocation; runtime_ = &runtime; home_ = home;
    receiver_ = receiver; provider_ = provider; caller_edge_ = edge; phase_ = Phase::armed;
    call.invocation_ = &invocation; call.slot_ = this; call.runtime_ = &runtime;
    call.home_ = home; call.receiver_ = receiver; call.provider_ = provider;
    call.caller_edge_ = edge; call.phase_ = NativeGuiTextFailureConstructorCall::Phase::prepared;
}
void NativeGuiTextFailureConstructorSlotArm::normal_disarm(NativeGuiTextFailureConstructorCall& call) noexcept {
    if (phase_ == Phase::armed && call.slot_ == this) {
        phase_ = Phase::disarmed; call.phase_ = NativeGuiTextFailureConstructorCall::Phase::disarmed;
    } else { phase_ = Phase::denied; call.valid_ = false; }
}
bool NativeGuiTextFailureConstructorCall::bind_once(void* receiver,
    NativeGuiTextFailureOrigin& origin, const void* record) noexcept {
    if (!valid_ || phase_ != Phase::calling || bound_ || !record || receiver != receiver_ ||
        !invocation_ || !runtime_ || !slot_ || !home_ || callee_record_ ||
        slot_->phase_ != NativeGuiTextFailureConstructorSlotArm::Phase::armed ||
        slot_->invocation_ != invocation_ || slot_->runtime_ != runtime_ || slot_->home_ != home_ ||
        slot_->receiver_ != receiver_ || slot_->provider_ != provider_ ||
        invocation_->runtime_ != runtime_ || invocation_->active_.supplier != provider_ ||
        invocation_->active_.site.native != caller_edge_.native ||
        invocation_->active_.site.source != caller_edge_.source ||
        invocation_->active_.stage != NativeGuiTextFailureOrigin::Stage::ordinary) {
        valid_ = false; return false; // no reset, replacement or admission policy
    }
    bound_ = true; callee_record_ = record; callee_origin_ = &origin; callee_entry_ = 0x00ab9650u;
    invocation_->enter({caller_edge_, record, NativeGuiTextFailureOrigin::Stage::ordinary});
    return true;
}
void NativeGuiTextFailureConstructorCall::body_returned(
    const NativeGuiTextFailureOrigin& origin, const void* record) noexcept {
    if (!valid_ || !bound_ || phase_ != Phase::calling || native_body_returned_ ||
        callee_origin_ != &origin || callee_record_ != record || origin.runtime_ != runtime_ ||
        origin.supplier_ != record || origin.entry_ != 0x00ab9650u) valid_ = false;
    native_body_returned_ = true; // PRIVATE negative frontier; never completes an Origin
}
void NativeGuiTextFailureConstructorCall::failed() {
    const auto current = runtime_->read_current_catch(); // before any diagnostic exception_ptr
    if (!failure_committed_) { escaped_cause_ = current; failure_committed_ = true; }
    if (bound_ && callee_origin_)
        invocation_->adopt(escaped_cause_, *callee_origin_, callee_record_, 0x00ab9650u);
    else invocation_->commit(NativeGuiTextFailureOrigin::Verdict::unclassified, escaped_cause_);
    phase_ = Phase::failed;
}
bool NativeGuiTextFailureConstructorSlotArm::consume_proven_primary(
    const NativeGuiTextFailureOrigin& invocation, const NativeGuiTextFailureOriginRuntime& runtime,
    const NativeGuiTextFailureConstructorCall& call, volatile std::uint32_t* actual_home) {
    const auto current = runtime.read_current_catch();
    if (phase_ != Phase::armed) return false; // preserve the first denial/consumption history
    escaped_cause_ = current;
    const bool allowed = phase_ == Phase::armed && invocation_ == &invocation && runtime_ == &runtime &&
        home_ && home_ == actual_home && call.home_ == home_ && call.slot_ == this &&
        call.invocation_ == &invocation && call.runtime_ == runtime_ && call.receiver_ == receiver_ &&
        call.provider_ == provider_ && call.valid_ && call.bound_ && !call.native_body_returned_ &&
        call.phase_ == NativeGuiTextFailureConstructorCall::Phase::failed &&
        call.callee_record_ && call.callee_origin_ && call.callee_entry_ == 0x00ab9650u &&
        call.failure_committed_ && call.callee_origin_->valid_ &&
        call.callee_origin_->runtime_ == runtime_ && call.callee_origin_->supplier_ == call.callee_record_ &&
        call.callee_origin_->entry_ == 0x00ab9650u &&
        call.callee_origin_->phase_ == NativeGuiTextFailureOrigin::Phase::failed &&
        !call.callee_origin_->secondary_committed_ && call.callee_origin_->cause_.same(escaped_cause_) &&
        caller_edge_.native == 0x00ab9d4fu && call.caller_edge_.native == caller_edge_.native &&
        call.caller_edge_.source == caller_edge_.source && invocation.entry_ == 0x00ab98f0u &&
        invocation.valid_ && invocation.runtime_ == runtime_ &&
        invocation.phase_ == NativeGuiTextFailureOrigin::Phase::failed &&
        invocation.verdict_ == NativeGuiTextFailureOrigin::Verdict::proven_native &&
        invocation.native_issuer_ && !invocation.secondary_committed_ &&
        invocation.cause_.same(escaped_cause_) && call.escaped_cause_.same(escaped_cause_) &&
        invocation.forwarded_child_ == call.callee_origin_ &&
        invocation.escaped_.supplier == call.callee_record_ &&
        invocation.escaped_.site.native == caller_edge_.native &&
        invocation.escaped_.site.source == caller_edge_.source &&
        invocation.escaped_.parent_supplier == provider_ &&
        invocation.escaped_.parent_site.native == caller_edge_.native &&
        invocation.escaped_.parent_site.source == caller_edge_.source &&
        invocation.escaped_.stage == NativeGuiTextFailureOrigin::Stage::ordinary &&
        invocation.forwarded_child_edge_.stage == NativeGuiTextFailureOrigin::Stage::ordinary;
    phase_ = allowed ? Phase::consumed : Phase::denied;
    return allowed; // withheld: Origin has NO Native issuer, including this route
}
} // namespace bsp
