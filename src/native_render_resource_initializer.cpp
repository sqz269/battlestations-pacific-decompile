#include "bsp/native_render_resource_initializer.hpp"
#include "bsp/native_post_effect_parent_lifetime.hpp"

#include <stdexcept>
#include <type_traits>
#include <exception>

namespace bsp {

bool NativeRenderResourceInitialization::requires_process_retention() const noexcept {
    return phase_ == Phase::running || phase_ == Phase::failed ||
        (selected_parent_ && selected_parent_->requires_process_retention());
}

void NativeRenderResourceInitialization::bind_parent_initializers(NativePostEffectParentDomain& parent,
    NativeBrightPassInitializationContext& bright,NativePostEffectDustContext& dust,
    NativeMeshSectionLifetimeSourceScope& source) {
    if(phase_!=Phase::fresh || children_closed_ || selected_parent_ || selected_bright_ ||
       selected_dust_ || selected_source_ || parent_initializers_closed_)
        throw std::logic_error("selected parent initializers require a fresh unbound operation");
    // Concrete compatibility is checked against each actual legacy stage context
    // at its typed bind. All checks here precede nonthrowing pointer commits.
    selected_parent_=&parent;selected_bright_=&bright;selected_dust_=&dust;selected_source_=&source;
}
void NativeRenderResourceInitialization::release_parent_source_borrow() noexcept {
    selected_source_=nullptr; // invocation borrow only; never Source end
}

void NativeRenderResourceInitialization::close_parent_initializers_after_host_quiescence(
    NativePostEffectParentDomain& parent) {
    if(selected_parent_!=&parent)
        throw std::logic_error("selected initializer close requires the same Parent domain");
    if(phase_!=Phase::normal_work_complete && phase_!=Phase::already_initialized_returned)
        throw std::logic_error("partial, running or failed selected initialization remains retained");
    if(selected_source_ || continuation_.bright_source_ || passthrough_dust_.dust_source_ ||
       parent.requires_process_retention())
        throw std::logic_error("selected initializer invocation or Parent failure remains retained");
    if(parent_initializers_closed_)return;
    if((continuation_.bright_parent_ && continuation_.bright_parent_!=&parent) ||
       (passthrough_dust_.dust_parent_ && passthrough_dust_.dust_parent_!=&parent))
        throw std::logic_error("selected initializer Parent identities differ");
    auto& dust=passthrough_dust_.dust;auto& bright=continuation_.bright;
    using DustPhase=NativePostEffectDustBlock::Phase;
    using BrightPhase=NativeBrightPassInitializationBlock::Phase;
    // Non-draining pair preflight. This is not the missing whole-legacy pass.
    if((dust.phase()!=DustPhase::idle && dust.phase()!=DustPhase::prepared && dust.phase()!=DustPhase::settled) ||
       (bright.phase()!=BrightPhase::idle && bright.phase()!=BrightPhase::prepared && bright.phase()!=BrightPhase::settled))
        throw std::logic_error("selected initializer Caller is still partial or running");
    // PRECONDITION: actual owner already completed parent.close_after_host_quiescence()
    // after the real external Source end and all required fences. Source8 typed
    // Caller detach enforces that close, idle inner Block and genuine Source end.
    // Never reset the inner Parent Block or require healthy pending custody==0.
    if(dust.phase()==DustPhase::prepared)dust.cancel_preparation();
    else if(dust.phase()==DustPhase::settled)dust.reset_after_host_quiescence();
    if(bright.phase()==BrightPhase::prepared)bright.cancel_preparation();
    else if(bright.phase()==BrightPhase::settled)bright.reset_after_host_quiescence();
    parent_initializers_closed_=true; // only ordered history, not readiness/fence
}

void NativeRenderResourceInitialization::close_children_after_host_quiescence() noexcept {
    if(selected_parent_ && !parent_initializers_closed_)std::terminate();
    if (children_closed_) return;
    if (phase_ == Phase::fresh || phase_ == Phase::already_initialized_returned) {
        children_closed_ = true;
        return;
    }
    if (phase_ != Phase::normal_work_complete) std::terminate();
    // The existing blocks reject surviving native creators, companions,
    // parameter borrows and viewport records. Prepared-only blocks own no
    // native work and use their existing cancellation path.
    const auto close_post = [](auto& block) noexcept {
        using Block = std::remove_reference_t<decltype(block)>;
        if (block.phase() == Block::Phase::idle) return;
        if (block.phase() == Block::Phase::prepared) block.cancel_preparation();
        else block.reset_after_host_quiescence();
    };
    const auto close_pass = [&](auto& block) noexcept {
        using Block = std::remove_reference_t<decltype(block)>;
        if (block.phase() == Block::Phase::idle) return;
        if (block.phase() == Block::Phase::prepared) block.cancel_preparation();
        else {
            close_post(block.post_effect());
            block.reset_after_host_quiescence();
        }
    };
    const auto close_bloom = [&](NativeBloomInitializationBlock& block) noexcept {
        using Block = NativeBloomInitializationBlock;
        if (block.phase() == Block::Phase::idle) return;
        if (block.phase() == Block::Phase::prepared) block.cancel_preparation();
        else {
            close_post(block.blur()); close_post(block.bloom());
            block.reset_after_host_quiescence();
        }
    };
    // Reverse parent construction order; every native owner is already retired.
    close_post(tail_.clear); close_post(tail_.wave);
    close_post(post664_.post); close_post(post65c_.post); close_post(post658_.post);
    auto& dust = passthrough_dust_.dust;
    if (dust.phase() == NativePostEffectDustBlock::Phase::prepared) dust.cancel_preparation();
    else if (dust.phase() != NativePostEffectDustBlock::Phase::idle) {
        close_post(dust.child()); dust.reset_after_host_quiescence();
    }
    close_post(passthrough_dust_.post);
    auto& distortion = distortion_.distortion;
    if (distortion.phase() == NativeDistortionInitializationBlock::Phase::prepared)
        distortion.cancel_preparation();
    else if (distortion.phase() != NativeDistortionInitializationBlock::Phase::idle) {
        for (std::uint32_t i = 0; i != 3; ++i) close_post(distortion.post(i));
        distortion.reset_after_host_quiescence();
    }
    close_bloom(second_bloom_.bloom);
    close_post(dof_.post);
    for (auto& post : continuation_.posts) close_post(post);
    close_bloom(continuation_.bloom);
    close_pass(continuation_.bright);
    auto& luminance = continuation_.luminance;
    if (luminance.phase() == NativeLuminanceInitializationBlock::Phase::prepared)
        luminance.cancel_preparation();
    else if (luminance.phase() != NativeLuminanceInitializationBlock::Phase::idle) {
        for (std::size_t i = 0; i != 6; ++i) close_post(luminance.post_effect(i));
        luminance.reset_after_host_quiescence();
    }
    for (auto& pass : continuation_.passes) close_pass(pass);
    for (auto& reference : continuation_.pass_references) {
        if (reference && !reference->retired()) std::terminate();
        reference.reset();
    }
    if (second_bloom_.bloom_reference && !second_bloom_.bloom_reference->retired()) std::terminate();
    second_bloom_.bloom_reference.reset();
    if (distortion_.pass_reference && !distortion_.pass_reference->retired()) std::terminate();
    distortion_.pass_reference.reset();
    children_closed_ = true;
}

void initialize_native_render_resources_00b107f0(void* service,
    const volatile NativeRenderResourceInitArguments& arguments,
    NativeRenderResourceInitializerContext& context,
    NativeRenderResourceInitialization& operation) {
    using Phase = NativeRenderResourceInitialization::Phase;
    if (operation.phase_ != Phase::fresh)
        throw std::logic_error("Native render-resource initialization cannot be replayed");

    operation.phase_ = Phase::running;
    auto& continuation = context.continuation;
    try {
        operation.active_stage_ = 0x00b107f0;
        begin_native_render_resource_init_00b107f0_fragment(service, arguments,
            continuation.entry, operation.entry_);
        if (operation.entry_.phase ==
            NativeRenderResourceInitEntryState::Phase::already_initialized_returned) {
            operation.phase_ = Phase::already_initialized_returned;
            return;
        }

        operation.active_stage_ = 0x00b109bc;
        if(operation.selected_parent_)
            continue_native_render_resource_init_00b109bc_fragment(operation.entry_,
                continuation, operation.continuation_, *operation.selected_parent_,
                *operation.selected_bright_, *operation.selected_source_);
        else
            continue_native_render_resource_init_00b109bc_fragment(operation.entry_,
                continuation, operation.continuation_);
        operation.active_stage_ = 0x00b11599;
        continue_native_render_resource_init_00b11599_fragment(operation.continuation_,
            continuation, operation.post_);
        operation.active_stage_ = 0x00b118af;
        continue_native_render_resource_init_00b118af_fragment(operation.post_,
            continuation, context.dof_names, operation.dof_);
        operation.active_stage_ = 0x00b11d7e;
        continue_native_render_resource_init_00b11d7e_fragment(operation.dof_,
            continuation, operation.second_bloom_);
        operation.active_stage_ = 0x00b11e40;
        continue_native_render_resource_init_00b11e40_fragment(operation.second_bloom_,
            continuation, context.distortion, operation.distortion_);
        operation.active_stage_ = 0x00b11ef4;
        if(operation.selected_parent_)
            continue_native_render_resource_init_00b11ef4_fragment(operation.distortion_,
                continuation, context.passthrough_dust, operation.passthrough_dust_,
                *operation.selected_parent_, *operation.selected_dust_, *operation.selected_source_);
        else
            continue_native_render_resource_init_00b11ef4_fragment(operation.distortion_,
                continuation, context.passthrough_dust, operation.passthrough_dust_);
        operation.active_stage_ = 0x00b120b9;
        continue_native_render_resource_init_00b120b9_fragment(operation.passthrough_dust_,
            continuation, context.post658_names, operation.post658_);
        operation.active_stage_ = 0x00b124a1;
        continue_native_render_resource_init_00b124a1_fragment(operation.post658_,
            continuation, context.post65c_names, operation.post65c_);
        operation.active_stage_ = 0x00b1297a;
        continue_native_render_resource_init_00b1297a_fragment(operation.post65c_,
            continuation, context.post664_names, operation.post664_);
        operation.active_stage_ = 0x00b12c9a;
        continue_native_render_resource_init_00b12c9a_fragment(operation.post664_,
            continuation, context.tail_names, operation.tail_);
        operation.phase_ = Phase::normal_work_complete;
    } catch (...) {
        operation.phase_ = Phase::failed;
        throw;
    }
}

void initialize_native_render_resources_00b107f0(void* service,
    const volatile NativeRenderResourceInitArguments& arguments,
    NativeRenderResourceInitializerContext& context,NativeRenderResourceInitialization& operation,
    NativePostEffectParentDomain& parent,NativeBrightPassInitializationContext& bright,
    NativePostEffectDustContext& dust,NativeMeshSectionLifetimeSourceScope& source) {
    operation.bind_parent_initializers(parent,bright,dust,source); // outside cleanup: entered only
    try {
        initialize_native_render_resources_00b107f0(service,arguments,context,operation);
    } catch(...) {
        operation.release_parent_source_borrow(); // only this completed bind
        throw;
    }
    operation.release_parent_source_borrow();
}
} // namespace bsp
