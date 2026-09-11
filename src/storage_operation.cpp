#include "bsp/storage_operation.hpp"

#include <optional>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
struct TemporaryMessage {
    NativeString value;
    NativeStringStorage& storage;
    ~TemporaryMessage() { value.release_to(storage); }
};

void advance(StorageOperationState& state, StorageManagerOperation& manager,
    StorageOperationHost& host) {
    begin_storage_update_0057bda0(state);
    update_storage_00bd3580(manager, host);
    end_storage_update_0057bdc0(state);
}

void select_prompt_kind(std::optional<PromptKind>& previous,
    std::uint8_t flag_d, std::uint8_t flag_e, std::uint8_t flag_f) {
    if (flag_d != 0) {
        if (flag_e != 0 && flag_f == 0) previous = PromptKind::YesNo;
    } else if (flag_e == 0) {
        previous = flag_f != 0 ? PromptKind::Accept : PromptKind::Busy;
    }
    // Native EBP retains the preceding kind for all other flag combinations.
    // Its initial value comes from an uninitialized stack word. The inspected
    // PC producer emits only the three combinations above; no fallback is known.
    if (!previous) {
        throw std::logic_error("006adb50: first prompt flags leave native kind indeterminate");
    }
}
} // namespace

bool storage_state_is_zero_or_one_006ad120(const StorageManagerOperation& manager) noexcept {
    return manager.state_08 == 0 || manager.state_08 == 1;
}

bool storage_prompt_pending_006ad150(const StorageManagerOperation& manager) noexcept {
    return manager.prompt_pending_0c != 0 && manager.prompt_observed_10 == 0;
}

void begin_storage_update_0057bda0(StorageOperationState& state) noexcept {
    if (state.update_guard_00e194b8 == 0) state.update_guard_00e194b8 = 1;
}

void end_storage_update_0057bdc0(StorageOperationState& state) noexcept {
    if (state.update_guard_00e194b8 != 0) state.update_guard_00e194b8 = 0;
}

void update_storage_00bd3580(StorageManagerOperation& manager, StorageOperationHost& host) {
    host.storage_update_virtual_44(manager);
}

void set_storage_response_00bd3550(StorageManagerOperation& manager,
    std::int32_t response) noexcept {
    manager.response_1c = response;
    manager.prompt_pending_0c = 0;
    manager.prompt_observed_10 = 0;
}

void copy_storage_prompt_00bd41a0(StorageManagerOperation& manager,
    NativeStringStorage& storage, NativeString& destination,
    std::uint8_t& flag_d, std::uint8_t& flag_e, std::uint8_t& flag_f) {
    destination.copy_from_00be0a30_fragment(storage, manager.prompt_message_14);
    flag_d = manager.prompt_flag_0d;
    flag_e = manager.prompt_flag_0e;
    flag_f = manager.prompt_flag_0f;
}

void run_storage_operation_006adb50(StorageOperationState& state,
    StorageOperationHost& host, StorageContinuation continuation) {
    // EDI captures the manager before the first update. Replacing the singleton
    // during a host call does not redirect this invocation's manager accesses.
    auto& manager = *state.manager_0109cecc;
    if (!state.continuation_00e198f8) advance(state, manager, host);
    // 006adb99 executes once. The backward branch targets006adba0, not this store.
    state.continuation_00e198f8 = std::move(continuation);
    std::optional<PromptKind> kind;
    for (;;) {
        if (storage_prompt_pending_006ad150(manager)) {
            manager.prompt_observed_10 = 1;
            auto& prompts = host.prompt_host();
            auto& strings = prompts.strings();
            TemporaryMessage message{{}, strings};
            std::uint8_t flag_d, flag_e, flag_f;
            copy_storage_prompt_00bd41a0(manager, strings, message.value,
                flag_d, flag_e, flag_f);
            select_prompt_kind(kind, flag_d, flag_e, flag_f);
            NativeString empty_title;
            raise_prompt_00531b00(host.prompt_screen_00425d10(), prompts, 0,
                message.value, *kind, 0x006ad3c0, 0, empty_title,
                0.0f, 0, NativeString{}, true);
            if (*kind != PromptKind::Busy) return;
            host.try_begin_render_frame_004c6c30();
            host.render_game_004ca440();
            host.finish_render_frame_004ca1f0();
            set_storage_response_00bd3550(manager, 3);
        }
        if (!storage_state_is_zero_or_one_006ad120(manager) && manager.state_08 != 2) {
            advance(state, manager, host);
        }
        if (storage_state_is_zero_or_one_006ad120(manager) || manager.state_08 == 2) {
            auto completed = std::move(state.continuation_00e198f8);
            state.continuation_00e198f8 = {};
            if (completed) {
                completed();
                dismiss_prompt_00532a20(host.prompt_screen_00425d10(),
                    host.prompt_host(), 0);
            }
            return;
        }
    }
}

void respond_storage_prompt_006ad3c0(StorageOperationState& state,
    StorageOperationHost& host, std::int32_t result) {
    const auto response = result == 1 ? 1 : (result == 0 ? 2 : 3);
    set_storage_response_00bd3550(*state.manager_0109cecc, response);
    auto continuation = state.continuation_00e198f8;
    run_storage_operation_006adb50(state, host, std::move(continuation));
}

} // namespace bsp
