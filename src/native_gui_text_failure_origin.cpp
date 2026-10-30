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
} // namespace bsp
