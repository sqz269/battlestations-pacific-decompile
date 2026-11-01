#include "bsp/native_render_observed_zero.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_post_effect_model_lifetime.hpp"
#include "bsp/native_post_effect_parent_lifetime.hpp"
#include "bsp/native_material_lifetime.hpp"
#include "bsp/native_mesh_section_lifetime.hpp"
#include <exception>
#include <new>
#include <utility>
#include <stdexcept>
namespace bsp {
NativeRenderObservedZeroFrame::~NativeRenderObservedZeroFrame() noexcept {
    // The owner keeps this object through its complete source uses/catches.
    // Unwind records failure before dropping any C++ pin; Native work is never
    // replayed. Destruction during an active call violates that owner lifetime.
    (void)NativeRenderObservedZeroAccess::end_frame(*this,
        head_ && std::uncaught_exceptions() > uncaught_on_attach_);
}
NativeRenderObservedZeroFrame::NativeRenderObservedZeroFrame(NativeRenderObservedZeroFrame&& other) noexcept {
    if (other.active_invocations_.load(std::memory_order_acquire) != 0u) {
        NativeRenderObservedZeroAccess::fail_frame(other);
        return; // invalid moving caller does not detach an active chain
    }
    head_ = std::exchange(other.head_, nullptr);
    uncaught_on_attach_ = other.uncaught_on_attach_;
    for (auto* n=head_; n; n=n->next_) n->frame_=this;
}
NativeRenderObservedZeroFrame& NativeRenderObservedZeroFrame::operator=(NativeRenderObservedZeroFrame&& other) noexcept {
    if (this==&other) return *this;
    if (other.active_invocations_.load(std::memory_order_acquire) != 0u ||
        !NativeRenderObservedZeroAccess::end_frame(*this,
            head_ && std::uncaught_exceptions() > uncaught_on_attach_)) {
        NativeRenderObservedZeroAccess::fail_frame(other);
        return *this;
    }
    head_=std::exchange(other.head_,nullptr);
    uncaught_on_attach_=other.uncaught_on_attach_;
    for(auto* n=head_;n;n=n->next_) n->frame_=this;
    return *this;
}
void NativeRenderObservedZeroAccess::attach_hold(NativeRenderObservedZeroFrame& frame,
    NativeRenderObservedZeroHold& hold) noexcept {
    if (!frame.head_) frame.uncaught_on_attach_=std::uncaught_exceptions();
    hold.frame_=&frame;
    hold.next_=frame.head_;
    frame.head_=&hold;
}
void NativeRenderObservedZeroAccess::fail_frame(NativeRenderObservedZeroFrame& frame) noexcept {
    for(auto* n=frame.head_;n;n=n->next_) {
        if(n->mesh_) NativeMeshCanonicalAccess::fail_observed_outer(*n->mesh_);
        if(n->model_) n->model_->record_source_failure();
        if(n->parent_) n->parent_->record_source_failure();
        if(n->material_) n->material_->record_source_failure();
        if(n->section_) n->section_->record_source_failure();
    }
}
bool NativeRenderObservedZeroAccess::end_frame(NativeRenderObservedZeroFrame& frame,
    bool source_failed) noexcept {
    if(source_failed) fail_frame(frame);
    if(frame.active_invocations_.load(std::memory_order_acquire)!=0u) {
        fail_frame(frame);
        return false; // keep linked nodes and receiver metadata intact
    }
    while(auto* n=frame.head_) {
        frame.head_=n->next_;
        n->next_=nullptr;
        n->frame_=nullptr;
        if(n->mesh_) NativeMeshCanonicalAccess::release_observed_outer(*n->mesh_);
        if(n->model_) n->model_->release_observed_zero_hold(false);
        if(n->parent_) n->parent_->release_observed_zero_hold(false);
        if(n->material_) n->material_->release_observed_zero_hold(false);
        if(n->section_) n->section_->release_observed_zero_hold(false);
    }
    return true;
}
NativeRenderObservedZeroAccess::CppCountUse::CppCountUse(RenderCommandReference& reference) {
    if (reference.ordinary_mesh_) {
        mesh_.emplace(static_cast<NativeMeshReference&>(reference));
    } else if (auto* frame = reference.ordinary_model_) {
        frame->begin_cpp_count_use(static_cast<NativeModelReference&>(reference));
        model_ = frame;
    } else if (auto* parent_frame = reference.ordinary_parent_) {
        parent_frame->begin_cpp_count_use(reference);
        parent_ = parent_frame;
    } else if (auto* material_frame = reference.ordinary_material_) {
        material_frame->begin_cpp_count_use(reference);
        material_ = material_frame;
    } else if (auto* section_frame = reference.ordinary_section_) {
        section_frame->begin_cpp_count_use(reference);
        section_ = section_frame;
    }
}
NativeRenderObservedZeroAccess::CppCountUse::~CppCountUse() noexcept {
    if (model_) model_->end_cpp_count_use(); // external metadata only
    if (parent_) parent_->end_cpp_count_use(); // external metadata only
    if (material_) material_->end_cpp_count_use(); // external metadata only
    if (section_) section_->end_cpp_count_use(); // external metadata only
}
void NativeRenderObservedZeroAccess::release_ordinary(RenderCommandReference& reference) {
    const CppCountUse use(reference);
    NativeRenderObservedZeroFrame outer;
    const Invocation invocation(outer);
    if (reference.reference_count.fetch_sub(1, std::memory_order_seq_cst) == 1) {
        const auto actual = reinterpret_cast<std::uintptr_t>(&reference.reference_count) - 4u;
        invoke(reference, reinterpret_cast<void*>(actual), outer);
    }
}
void NativeRenderObservedZeroAccess::invoke(RenderCommandReference& reference, void* actual,
    NativeRenderObservedZeroFrame& outer) {
    const Invocation invocation(outer);
    try {
    if (reference.ordinary_mesh_) {
        auto& mesh = static_cast<NativeMeshReference&>(reference);
        NativeMeshCanonicalAccess::hold_observed_zero(mesh, outer);
        NativeMeshCanonicalAccess::arm_observed_zero(mesh);
        mesh.release_zero_references();
        return;
    } else if (auto* frame = reference.ordinary_model_) {
        auto& model=static_cast<NativeModelReference&>(reference);
        frame->hold_observed_zero(outer,model);
        frame->invoke_observed_zero(actual, model);
        return;
    } else if (auto* parent_frame = reference.ordinary_parent_) {
        parent_frame->hold_observed_zero(outer, reference);
        parent_frame->invoke_observed_zero(actual, reference);
        return;
    } else if (auto* material_frame = reference.ordinary_material_) {
        material_frame->hold_observed_zero(outer, reference);
        material_frame->invoke_observed_zero(actual, reference);
        return;
    } else if (auto* section_frame = reference.ordinary_section_) {
        section_frame->hold_observed_zero(outer, reference);
        section_frame->invoke_observed_zero(actual, reference);
        return;
    }
    reference.release_zero_references(); // exact legacy virtual policy
    } catch (...) { fail_frame(outer); throw; }
}
void NativeRenderObservedZeroAccess::dispatch(NativeRenderActualOwners& owners, void* actual_key,
    NativeRenderObservedZeroFrame& outer) {
    const Invocation invocation(outer);
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(actual_key) + 4));
    auto& reference = owners.resolve_actual(actual_key); // SAME single lookup
    if (&reference.reference_count != actual)
        throw std::invalid_argument("native owner companion must borrow the same actual +04 atomic");
    invoke(reference, actual_key, outer); // no second decrement/count sample
}
} // namespace bsp
