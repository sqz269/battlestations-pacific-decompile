#include "bsp/app_shutdown.hpp"

// Driver for the teardown order recovered from 00737f30. The routine itself is
// straight-line code with null guards; the table in the header keeps the order
// reviewable next to its call sites, and this file only walks it.

namespace bsp {
namespace {

constexpr std::uint32_t kStepCount =
    static_cast<std::uint32_t>(sizeof(kApplicationShutdownSteps) / sizeof(kApplicationShutdownSteps[0]));
constexpr std::uint32_t kDatatableCount =
    static_cast<std::uint32_t>(sizeof(kShutdownDatatables) / sizeof(kShutdownDatatables[0]));

// 00737f30 performs 42 actions between the prologue and the epilogue; a change
// to the table is a change to the recovered order and has to be re-evidenced.
static_assert(kStepCount == 42u, "recovered shutdown step count changed");
static_assert(kDatatableCount == 4u, "00737f30 unloads exactly four datatables");

// 00737f80-00737fb0. Ascending index, decrement the counter at element+4, and
// destroy through vtable slot 0 with no arguments once it reaches zero. The
// slot is cleared whether or not the object was destroyed.
void release_system_array(ApplicationShutdownHost& host, ApplicationShutdownState& state)
{
    if (state.systems == nullptr) {
        return;
    }
    for (std::uint32_t index = 0; index < state.system_count; ++index) {
        void* system = state.systems[index];
        if (system != nullptr) {
            if (host.release_system_reference(system) == 0) {
                host.destroy_system(system);
            }
            state.systems[index] = nullptr;
        }
    }
}

} // namespace

std::uint32_t shutdown_step_count()
{
    return kStepCount;
}

void shutdown_application(ApplicationShutdownHost& host, ApplicationShutdownState& state)
{
    // Held between the first two steps exactly as EAX is at 00737f4c/00737f51.
    void* gui_manager = nullptr;

    for (std::uint32_t index = 0; index < kStepCount; ++index) {
        const ShutdownStep& step = kApplicationShutdownSteps[index];
        switch (step.action) {
        case ShutdownAction::create_gui_manager:
            gui_manager = host.gui_manager_get_or_create_004c12b0();
            break;
        case ShutdownAction::reset_gui_screens:
            // 00aa0f70 dereferences the manager without a null test, so the
            // call is unconditional here too.
            host.gui_reset_screens_00aa0f70(gui_manager);
            break;
        case ShutdownAction::device_virtual_call:
            host.device_virtual_call_00f8d394(step.detail);
            break;
        case ShutdownAction::release_system_array:
            release_system_array(host, state);
            break;
        case ShutdownAction::game_on_destroy:
            host.game_on_destroy_004dc5c0(state.game);
            break;
        case ShutdownAction::destroy_game:
            if (state.game != nullptr) {
                host.delete_game(state.game);
                state.game = nullptr;
            }
            break;
        case ShutdownAction::stop_network_threads: {
            void* client = host.network_client_00f8abdc();
            if (client != nullptr) {
                host.network_stop_threads_00a3b6e0(client);
            }
            break;
        }
        case ShutdownAction::release_singleton:
            host.release_singleton(static_cast<ShutdownSingleton>(step.detail));
            break;
        case ShutdownAction::destroy_registered_singleton:
            host.destroy_registered_singleton(static_cast<ShutdownSingleton>(step.detail));
            break;
        case ShutdownAction::xlive_uninitialize:
            host.xlive_uninitialize();
            break;
        case ShutdownAction::file_store_service_hook:
            host.file_store_service_hook_00be8350();
            break;
        case ShutdownAction::remove_datatable_file:
            if (step.detail < kDatatableCount) {
                host.remove_datatable_file_00be7130(kShutdownDatatables[step.detail]);
            }
            break;
        case ShutdownAction::free_global_block:
            host.free_global_block_00bbc5d0();
            break;
        case ShutdownAction::run_static_destructor:
            host.run_static_destructor_00bb4db0();
            break;
        case ShutdownAction::trace_message:
            host.trace_004c9c90(kShutdownTraceMessage);
            break;
        case ShutdownAction::empty_hook:
            host.empty_hook_00888750();
            break;
        }
    }
}

} // namespace bsp
