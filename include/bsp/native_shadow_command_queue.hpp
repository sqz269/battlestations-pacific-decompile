#pragma once

#include "bsp/native_frame_job_execution.hpp"
#include <cstdint>

namespace bsp {
struct NativeInstanceGroupUploadAccess;

// Complete B1EB80..B1EBD5[86]. Original ECX actual queue, public command DWORD
// then flags DWORD, RET8. EDX adds the borrowed SAME upload/render access.
// Queue+14 is the existing raw pointer-array header: data/count/capacity at
// +14/+18/+1C. Keep equality-only growth, current data/count, ORIGINAL public
// command read before the computed-null destination test, current count RMW,
// then the late ORIGINAL public flag byte. A nonzero flag traverses/uploads
// the captured command; zero does not dereference access or command.
void __fastcall append_native_shadow_command_00b1eb80(void* actual_queue,
    NativeInstanceGroupUploadAccess* access, void* actual_command,
    std::uint32_t flags);

// Complete A8AE50..A8AE75[38]. Original ECX job is unused; one public command
// DWORD and RET4. Capture command once, get context and PUSH it BEFORE getting
// scene, traverse with the same access.render, then upload captured command.
void __fastcall execute_native_shadow_frame_job_00a8ae50(void* unused_actual_job,
    NativeInstanceGroupUploadAccess* access, void* actual_command);

// Complete literal4-byte MOV/RET bodies. ECX actual command, EAX current field.
// No public stack arguments, validation, retained reference, or host owner.
void* __fastcall native_shadow_command_scene_00b1bf20(const void* actual_command) noexcept;
void* __fastcall native_shadow_command_context_00b1bf30(const void* actual_command) noexcept;

// Source-only composition of the existing application job-dispatch chain.
// Borrows the actual live D5B570 table, SAME upload/render access, and actual
// remaining dispatcher. Fixed table backing, current contents. D5B570 with a
// changed slot0 fails; other profiles go to the real remaining dispatcher.
// The scheduler C++ call supplies a new public argument slot; it does not
// preserve the original scheduler frame's argument aliases. No owned job,
// additional registry/queue, retained command, or successful default route.
class NativeShadowFrameJobDispatch final : public NativeFrameJobDispatch {
public:
    NativeShadowFrameJobDispatch(NativeInstanceGroupUploadAccess&,
        const volatile std::uint32_t* actual_table_00d5b570,
        NativeFrameJobDispatch& remaining);
    void execute_current_00(void* actual_job_owner, std::uint32_t argument) override;
private:
    NativeInstanceGroupUploadAccess& access_;
    const volatile std::uint32_t* const table_;
    NativeFrameJobDispatch& remaining_;
};

// Actual header/backing must satisfy B1C6C0's established valid-storage/CRT
// domain. Traversal/upload require live actual command/scene data, canonical
// EL table binding and the existing nonthrowing finite upload services. Flags0
// may append a null command. No header/count/null-command guard, ownership,
// rollback or cleanup is added. Original argument positions are preserved in
// the two naked entries, but private access storage and provider C++ adapters
// do not establish original binary ABI, arbitrary stack aliases, EH/SEH or
// exceptions across naked frames. Parent/job-domain/game closure is unproved.
} // namespace bsp
