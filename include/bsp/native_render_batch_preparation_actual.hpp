#pragma once

#include "bsp/native_frame_job_execution.hpp"
#include "bsp/native_render_batch_lifetime.hpp"
#include "bsp/native_render_queue_constructor.hpp"

namespace bsp {

struct NativeRenderBatchPreparationContext {
    void* volatile& actual_manager_01090aa0;
    // This constructor's F8D440 reference is also the getter's publication cell.
    NativeRenderQueueConstructorContext& actual_queue_constructor;
    const volatile std::uint32_t* actual_batch_profile_00d5e5ac;
    const volatile std::uint32_t* actual_base_batch_profile_00d62064;
};

// Complete 004C11F0..004C12AC. Original cdecl, no arguments, EAX queue, RET.
// Raw415350 manager, captured manager+10 section/depth, actual34h allocation,
// full B1F280, publish, fresh manager then current publication registration,
// captured unlock, fresh publication return. Construction failure frees its
// saved allocation after constructor cleanup; registration failure retains it.
// Native C++ unwind cleanup terminates on escaping C++ cleanup exceptions.
NativeRenderCommandQueueStorage* get_native_render_command_queue_004c11f0(
    NativeRenderBatchPreparationContext&);

// Complete B51DF0..B51F0B. Original ECX actual18h batch, one DWORD slot,
// RET4, no semantic result. Borrow that writable slot: B1CB30 overwrites it
// even when disabled. Capture its original index before the unconditional
// queue getter; use that captured index for keys/comparator selection.
// Index0 uses the full raw x87 key fragment and unsigned-key comparator;
// nonzero uses material/depth. Both use full raw pointer-slot introsort.
void prepare_native_render_batch_actual_00b51df0(NativeRenderBatchStorage&,
    volatile std::uint32_t& actual_argument_slot, NativeRenderBatchPreparationContext&);

// Complete B1BF70..B1BF81. Original job ECX ignored, stack raw batch pointer,
// tail dispatch/RET4. Read slot's batch, capture profile, read batch+08 mode,
// read captured current profile+0C, overwrite slot with mode, then prepare.
// Only current D5E5AC/D62064 profiles with B51DF0 at+0C are established.
void execute_native_render_preparation_job_00b1bf70(
    volatile std::uint32_t& actual_argument_slot, NativeRenderBatchPreparationContext&);

// Concrete D5E160:B1BF70 composition for the existing actual scheduler.
// Current owner/profile slot is read on every execution. Other owners require
// the supplied existing dispatcher; changed/unknown preparation slots fail.
class NativeRenderPreparationJobDispatch final : public NativeFrameJobDispatch {
public:
    NativeRenderPreparationJobDispatch(NativeRenderBatchPreparationContext&,
        const volatile std::uint32_t* actual_job_profile_00d5e160,
        NativeFrameJobDispatch& remaining);
    void execute_current_00(void* actual_job_owner, std::uint32_t argument) override;
private:
    NativeRenderBatchPreparationContext& context_;
    const volatile std::uint32_t* job_profile_;
    NativeFrameJobDispatch& remaining_;
};

// Borrow actual storage, same owner/string/allocator/lock domains and CURRENT
// profile tables. No alternate publication/count, callback queue getter, typed
// entry overlay or numeric code-address call. Native spans and lifetime are
// caller preconditions; no index/count/entry guards are added. New C++ context
// ABI; native FH3 frame identity, general SEH, concurrency, full queue execution,
// queue destruction, material/system-constant execution and gameplay unproved.
} // namespace bsp
