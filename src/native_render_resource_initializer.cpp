#include "bsp/native_render_resource_initializer.hpp"

#include <stdexcept>

namespace bsp {

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

} // namespace bsp
