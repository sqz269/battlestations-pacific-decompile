#include "bsp/native_render_command_execution.hpp"
#include "bsp/native_camera_frame_command.hpp"
#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/native_material_entry_dispatch.hpp"
#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_render_diagnostic_labels.hpp"
#include "bsp/native_render_job_publication.hpp"
#include "bsp/native_render_state_leaves.hpp"
#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+offset);
}
__declspec(noinline) Word word(const void* p, Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void* pointer(const void* p, Word offset=0) noexcept {
    return reinterpret_cast<void*>(word(p,offset));
}
void require(bool valid,const char* message) {
    if(!valid) throw std::logic_error(message);
}
template<class T,class U> bool same_cell(const volatile T& a,const volatile U& b) noexcept {
    return static_cast<const volatile void*>(&a)==static_cast<const volatile void*>(&b);
}
Word renderer_slot(void* renderer,Word offset,NativeRenderCommandExecutionContext& c) {
    require(word(renderer)==0x00d5f0a8 && c.actual_renderer_profile_00d5f0a8,
        "command requires its current D5F0A8 renderer binding");
    return c.actual_renderer_profile_00d5f0a8[offset/4];
}
NativeRenderBatchStorage* batch_at(void* command,Word index) noexcept {
    return static_cast<NativeRenderBatchStorage*>(pointer(command,0x0c+index*4));
}
Word batch_slot(NativeRenderBatchStorage* batch,Word offset,
    NativeRenderBatchPreparationContext& preparation) {
    const volatile Word* profile=nullptr;
    switch(word(batch)) {
    case 0x00d5e5ac: profile=preparation.actual_batch_profile_00d5e5ac; break;
    case 0x00d62064: profile=preparation.actual_base_batch_profile_00d62064; break;
    default: break;
    }
    require(profile!=nullptr,"command requires its current concrete batch profile");
    return profile[offset/4];
}
void require_preparation(NativeRenderCommandExecutionContext& c) {
    require(c.preparation &&
        same_cell(c.preparation->actual_manager_01090aa0,c.actual_strings.actual_manager_publication_01090aa0) &&
        same_cell(c.preparation->actual_queue_constructor.actual_renderer_00f8d394,c.actual_renderer_00f8d394),
        "command preparation must use the same raw manager and renderer cells");
}
NativeFrameJobLifetimeBindings& job_bindings(NativeRenderCommandExecutionContext& c) {
    require_preparation(c);
    require(c.jobs && c.jobs->frame_lifetime && c.preparation_dispatch &&
        same_cell(c.jobs->actual_manager_01090aa0,c.actual_strings.actual_manager_publication_01090aa0) &&
        &c.preparation_dispatch->preparation_context()==c.preparation &&
        &c.jobs->frame_lifetime->execution.job_dispatch()==c.preparation_dispatch,
        "command jobs must use the same actual preparation dispatcher and lifetime domain");
    return *c.jobs->frame_lifetime;
}
void prepare_batches(void* command,NativeRenderCommandExecutionContext& c) {
    const bool parallel=c.actual_synchronization_0108d6dc.mode_00==0 &&
        native_render_batch_count_00b51b20(batch_at(command,0))>50 &&
        native_render_batch_count_00b51b20(batch_at(command,1))>50;
    if(parallel) {
        for(Word index=0;index<2;++index) {
            auto* const initial=batch_at(command,index);
            *static_cast<volatile Word*>(at(initial,8))=index;
            auto& bindings=job_bindings(c);
            auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
            auto* const captured_batch=batch_at(command,index);
            auto* const captured_pool=reinterpret_cast<NativeFrameJobPoolStorage*>(at(owner,4));
            auto* const job=get_native_preparation_job_actual_00b0ffb0(*c.jobs);
            bindings.execution.require_enqueue_virtual_04(*captured_pool);
            enqueue_native_frame_job_00be3020(*captured_pool,job,reinterpret_cast<Word>(captured_batch));
        }
        auto& bindings=job_bindings(c);
        auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
        auto* const pool=reinterpret_cast<NativeFrameJobPoolStorage*>(at(owner,4));
        bindings.execution.require_dispatch_virtual_08(*pool);
        dispatch_native_frame_jobs_00be3150(*pool,1,bindings.scope,bindings.execution);
    } else {
        for(Word index=0;index<2;++index) {
            auto* const batch=batch_at(command,index);
            require_preparation(c);
            require(batch_slot(batch,0x0c,*c.preparation)==0x00b51df0,
                "command requires current B51DF0 serial batch preparation");
            volatile Word argument=index;
            prepare_native_render_batch_actual_00b51df0(*batch,argument,*c.preparation);
        }
    }
}
} // namespace

void execute_native_render_command_00b1d950(void* command,
    NativeRenderCommandExecutionContext& c,NativeRenderCommandExecutionFrame* frame) {
    if(!native_render_can_execute_command_00b20240(c.actual_renderer_00f8d394)) return;
    void* const context=pointer(command,0x28);
    void* const target_renderer=c.actual_renderer_00f8d394;
    const Word target_slot=renderer_slot(target_renderer,0x98,c);
    void* const captured_camera=pointer(context,8);
    auto* const target=static_cast<NativeFrameTargetOwnerStorage*>(pointer(context,0x14));
    require(target_slot==0x00b24e70 && c.targets &&
        &c.targets->actual_synchronization_0108d6dc==&c.actual_synchronization_0108d6dc,
        "command requires genuine frame targets in the same synchronization domain");
    bind_native_renderer_frame_targets_00b24e70(target_renderer,target,*c.targets);
    require(c.camera && c.camera->renderer_00==&c.actual_renderer_00f8d394 &&
        c.camera->renderer_profile_04==c.actual_renderer_profile_00d5f0a8,
        "command camera must use the same renderer cell and profile");
    execute_native_camera_frame_command_00b71360(captured_camera,c.camera);
    void* const scene=pointer(command,4);
    require(c.system && frame && frame->system &&
        c.system->actual_renderer_00f8d394==&c.actual_renderer_00f8d394 &&
        c.system->actual_service_00f8d39c==&c.actual_service_00f8d39c &&
        c.system->actual_manager_01090aa0==&c.actual_strings.actual_manager_publication_01090aa0 &&
        c.system->actual_synchronization==&c.actual_synchronization_0108d6dc,
        "command system constants require their persistent frame and shared actual cells");
    gather_native_system_constants_00b46a70(scene,captured_camera,*c.system,*frame->system);
    void* const service=c.actual_service_00f8d39c;
    if(service) set_native_render_diagnostic_label_00b13030(service,at(command,0x14),c.actual_strings);

    void* const metadata_renderer=c.actual_renderer_00f8d394;
    const Word metadata_slot=renderer_slot(metadata_renderer,0x114,c);
    require(metadata_slot==0x00b20210,"command requires the concrete RET4 metadata override");
    void* const metadata_argument=at(command,0x1c);
    try { // Native B1D9D6 arms after metadata renderer/slot/argument capture.
        native_render_command_hook_no_op_00b20210(metadata_argument);
        void* const active_renderer=c.actual_renderer_00f8d394;
        require(renderer_slot(active_renderer,0x2c,c)==0x00b1fe20,
            "command requires current active-frame getter");
        if(native_render_is_frame_active_00b1fe20(active_renderer)) {
            prepare_batches(command,c);
            for(Word index=0;index<2;++index) {
                auto* const batch=batch_at(command,index);
                require_preparation(c);
                require(batch_slot(batch,8,*c.preparation)==0x00b55550 && c.entries &&
                    same_cell(c.entries->actual_renderer_00f8d394,c.actual_renderer_00f8d394) &&
                    c.entries->actual_renderer_profile_00d5f0a8==c.actual_renderer_profile_00d5f0a8,
                    "command requires the same genuine material-entry batch dispatcher");
                execute_native_render_batch_entries_00b55550(
                    batch,index,captured_camera,*c.entries,frame->batches[index]);
            }
        }
    } catch(...) {
        try { reset_native_render_diagnostic_label_00b13510(c.actual_service_00f8d39c,c.actual_strings); }
        catch(...) { std::terminate(); }
        throw;
    }
    // Native B1DAA2 disarms before reset; a reset failure must not reset again.
    reset_native_render_diagnostic_label_00b13510(c.actual_service_00f8d39c,c.actual_strings);
}
} // namespace bsp
