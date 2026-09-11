#pragma once

#include "bsp/frontend_prompts.hpp"

#include <cstdint>
#include <functional>

namespace bsp {

// Typed projections, not native layouts or binary hooks. Addresses, original
// register/stack ABI and the first-prompt flag precondition are documented in
// docs/STORAGE_OPERATION_CONTINUATIONS.md. Descriptive names are hypotheses.
struct StorageManagerOperation {
    std::int32_t state_08{};
    std::uint8_t prompt_pending_0c{};
    std::uint8_t prompt_flag_0d{};
    std::uint8_t prompt_flag_0e{};
    std::uint8_t prompt_flag_0f{};
    std::uint8_t prompt_observed_10{};
    NativeString prompt_message_14; // owned by the manager, not by this driver
    std::int32_t response_1c{};
};

using StorageContinuation = std::function<void()>;
struct StorageOperationState {
    StorageManagerOperation* manager_0109cecc{}; // required, nonnull singleton
    std::uint8_t update_guard_00e194b8{};
    StorageContinuation continuation_00e198f8; // shared across nested calls
};

struct StorageOperationHost {
    virtual ~StorageOperationHost() = default;
    // 00bd3580 tail-jumps to this manager's vtable+44h. Implement the real
    // storage progress boundary; a no-op implementation can spin indefinitely.
    virtual void storage_update_virtual_44(StorageManagerOperation&) = 0;
    virtual FrontEndPromptScreen& prompt_screen_00425d10() = 0;
    virtual FrontEndPromptHost& prompt_host() = 0;
    // The native driver ignores TryBeginRenderFrame's result and executes all
    // three calls. The rendering implementation must supply real frame work.
    virtual void try_begin_render_frame_004c6c30() = 0;
    virtual void render_game_004ca440() = 0;
    virtual void finish_render_frame_004ca1f0() = 0;
};

bool storage_state_is_zero_or_one_006ad120(const StorageManagerOperation&) noexcept;
bool storage_prompt_pending_006ad150(const StorageManagerOperation&) noexcept;
void begin_storage_update_0057bda0(StorageOperationState&) noexcept;
void end_storage_update_0057bdc0(StorageOperationState&) noexcept;
void update_storage_00bd3580(StorageManagerOperation&, StorageOperationHost&);
// ECX=manager, stack response, RET4. Writes +1Ch before clearing +0Ch/+10h.
void set_storage_response_00bd3550(StorageManagerOperation&, std::int32_t response) noexcept;
// ECX=manager; stack destination, flag0d*, flag0e*, flag0f*; RET10h.
// Message copy precedes the three sequential byte reads/stores, including aliases.
void copy_storage_prompt_00bd41a0(StorageManagerOperation&, NativeStringStorage&,
    NativeString& destination, std::uint8_t& flag_0d,
    std::uint8_t& flag_0e, std::uint8_t& flag_0f);

// 006adb50: ECX=native void(*)() continuation, RET. May call it synchronously;
// returns with the shared slot retained when an interactive prompt is raised.
// Completion clears the slot before invoking it, then dismisses prompt slot0
// after the callback, even if that callback started another storage operation.
// FrontEndPromptHost::invoke_callback must route address006ad3c0 to the response
// function below using this same StorageOperationState and StorageOperationHost.
void run_storage_operation_006adb50(StorageOperationState&, StorageOperationHost&,
    StorageContinuation continuation);
// 006ad3c0: ECX=prompt result; 1=>response1, 0=>response2, other=>response3.
// Reads the shared continuation AFTER setting the response, then tail-resumes.
void respond_storage_prompt_006ad3c0(StorageOperationState&, StorageOperationHost&,
    std::int32_t result);

} // namespace bsp
