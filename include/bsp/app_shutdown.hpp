#pragma once
#include <cstdint>

// Application shutdown order recovered from 00737f30 (ledger name
// BSP_Application_Shutdown; the stubbed trace at 0073832d names it MitApp::Deinit).
// Evidence and call sites are in docs/APP_SHUTDOWN.md and
// reports/app_shutdown_audit.json.
//
// None of the subsystems torn down by that routine are reconstructed, so this
// module models the order and the guard structure only: every effect is routed
// through ApplicationShutdownHost. No global or type of the original binary is
// invented here.

namespace bsp {

// Singleton instance pointers destroyed by 00737f30, named by their address in
// the original image. Only the address is proven; the owning subsystem of each
// is a hypothesis and is deliberately not encoded in the name.
enum class ShutdownSingleton : std::uint32_t {
    global_00f8abe8 = 0x00f8abe8u,
    global_00f871b4 = 0x00f871b4u,
    global_00f8753c = 0x00f8753cu,
    global_0109cecc = 0x0109ceccu,
    global_00f8bc4c = 0x00f8bc4cu,
    global_010909b0 = 0x010909b0u,
    global_00f8aefc = 0x00f8aefcu,
    global_00f8c218 = 0x00f8c218u,
    global_00e1aea0 = 0x00e1aea0u,
    global_00f8c210 = 0x00f8c210u,
    global_00f8c280 = 0x00f8c280u,
    global_00f8d39c = 0x00f8d39cu,
    global_00f8bbf0 = 0x00f8bbf0u,
    global_00f8c274 = 0x00f8c274u,
    global_00f8bbf4 = 0x00f8bbf4u,
    global_00f8c26c = 0x00f8c26cu,
    global_00f8bc5c = 0x00f8bc5cu,
    global_00f8bf44 = 0x00f8bf44u,
    global_0109cf14 = 0x0109cf14u,
    global_00f8d420 = 0x00f8d420u,
    global_01090490 = 0x01090490u,
    global_00f8bbf8 = 0x00f8bbf8u,
    global_00e1aed4 = 0x00e1aed4u,
};

// One recovered action of the teardown sequence.
enum class ShutdownAction : std::uint32_t {
    // 00737f4c: 004c12b0 lazily creates and registers the GUI manager, then
    // 00737f53 passes it to 00aa0f70. The manager may be created here.
    create_gui_manager,
    reset_gui_screens,
    // 00737f63 and 00737f70: virtual slots +0x0c then +0x14 on DAT_00f8d394.
    device_virtual_call,
    // 00737f80: ascending walk of the system array, InterlockedDecrement on
    // element+4, destroy through vtable slot 0 with no arguments on zero, then
    // the slot is cleared.
    release_system_array,
    // 00737fb9 then 00737fc5.
    game_on_destroy,
    destroy_game,
    // 00737fda: only when the network client global is set.
    stop_network_threads,
    // vtable slot 0 with argument 1 on the global, which is left as it is; the
    // singleton base destructor clears it (see docs/APP_SHUTDOWN.md).
    release_singleton,
    // The 00736xxx template: unregister from the lifetime manager, destroy, and
    // clear the global, all under the manager lock.
    destroy_registered_singleton,
    // 00737ff1.
    xlive_uninitialize,
    // 00738008 and 0073800f; the hook body is a single RET in this build.
    file_store_service_hook,
    // 00738119 onwards: four cFileStore::RemoveFile calls.
    remove_datatable_file,
    // 00738314: 00bbc5d0 finalizes and frees DAT_01090900.
    free_global_block,
    // 00738328: the static destructor stub 00bb4db0.
    run_static_destructor,
    // 00738332: stubbed trace of "After MitApp::Deinit".
    trace_message,
    // 00738337: 00888750, a single RET.
    empty_hook,
};

struct ShutdownStep {
    ShutdownAction action;
    // Singleton address for the two singleton actions, vtable byte offset for
    // device_virtual_call, datatable index for remove_datatable_file, 0 otherwise.
    std::uint32_t detail;
    // Call site inside 00737f30.
    std::uint32_t site;
};

struct ShutdownDatatable {
    const char* path;
    // Length passed to BSP_NativeString_Resize, excluding the terminator.
    std::uint32_t length;
};

// Removed from the file store in this order at 00738119-007382c7.
constexpr ShutdownDatatable kShutdownDatatables[] = {
    { "scripts/datatables/inputs.lua", 0x1du },
    { "scripts/datatables/keyboardsetup.lua", 0x24u },
    { "scripts/datatables/controllerinputnames.lua", 0x2bu },
    { "scripts/datatables/controlpresets.lua", 0x25u },
};

// The complete recovered order of 00737f30.
constexpr ShutdownStep kApplicationShutdownSteps[] = {
    { ShutdownAction::create_gui_manager, 0u, 0x00737f4cu },
    { ShutdownAction::reset_gui_screens, 0u, 0x00737f53u },
    { ShutdownAction::device_virtual_call, 0x0cu, 0x00737f63u },
    { ShutdownAction::device_virtual_call, 0x14u, 0x00737f70u },
    { ShutdownAction::release_system_array, 0u, 0x00737f80u },
    { ShutdownAction::game_on_destroy, 0u, 0x00737fb9u },
    { ShutdownAction::destroy_game, 0u, 0x00737fc5u },
    { ShutdownAction::stop_network_threads, 0u, 0x00737fdau },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8abe8), 0x00737fe9u },
    { ShutdownAction::xlive_uninitialize, 0u, 0x00737ff1u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f871b4), 0x00738000u },
    { ShutdownAction::file_store_service_hook, 0u, 0x00738008u },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8753c), 0x00738014u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_0109cecc), 0x00738019u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bc4c), 0x0073802bu },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_010909b0), 0x0073803du },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8aefc), 0x0073804fu },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8c218), 0x00738061u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00e1aea0), 0x00738073u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8c210), 0x00738085u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8c280), 0x00738097u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8d39c), 0x007380a9u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bbf0), 0x007380bbu },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8c274), 0x007380cdu },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bbf4), 0x007380dfu },
    { ShutdownAction::remove_datatable_file, 0u, 0x00738128u },
    { ShutdownAction::remove_datatable_file, 1u, 0x007381a1u },
    { ShutdownAction::remove_datatable_file, 2u, 0x00738217u },
    { ShutdownAction::remove_datatable_file, 3u, 0x0073828du },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8c26c), 0x007382d2u },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bc5c), 0x007382dau },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bc4c), 0x007382dfu },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bf44), 0x0073830au },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_0109cf14), 0x0073830fu },
    { ShutdownAction::free_global_block, 0u, 0x00738314u },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8d420), 0x00738319u },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_01090490), 0x0073831eu },
    { ShutdownAction::destroy_registered_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00f8bbf8), 0x00738323u },
    { ShutdownAction::run_static_destructor, 0u, 0x00738328u },
    { ShutdownAction::trace_message, 0u, 0x00738332u },
    { ShutdownAction::empty_hook, 0u, 0x00738337u },
    { ShutdownAction::release_singleton,
        static_cast<std::uint32_t>(ShutdownSingleton::global_00e1aed4), 0x0073834cu },
};

// The message 00737f30 passes in ECX to the stubbed trace at 004c9c90; the
// literal lives at 00cfeaec.
constexpr const char kShutdownTraceMessage[] = "After MitApp::Deinit";

// Fields of the application object that 00737f30 reads or writes.
struct ApplicationShutdownState {
    // this+0x08: array of reference-counted system pointers.
    void** systems;
    // this+0x0c: element count of that array.
    std::uint32_t system_count;
    // this+0x14: the GGame instance, cleared once destroyed.
    void* game;
};

// Everything 00737f30 reaches that is not reconstructed yet. The comments give
// the original address so an implementation can be checked against it.
class ApplicationShutdownHost {
public:
    virtual ~ApplicationShutdownHost() {}

    // 004c12b0: double-checked lazy create of DAT_00f8bc5c, returned in EAX.
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    // 00aa0f70 with the manager in ECX.
    virtual void gui_reset_screens_00aa0f70(void* gui_manager) = 0;
    // DAT_00f8d394 virtual call at the given byte offset, no arguments.
    virtual void device_virtual_call_00f8d394(std::uint32_t vtable_offset) = 0;

    // InterlockedDecrement on system+4; returns the new value.
    virtual std::int32_t release_system_reference(void* system) = 0;
    // Virtual slot 0 with no arguments.
    virtual void destroy_system(void* system) = 0;

    // 004dc5c0, then the game object's scalar deleting destructor.
    virtual void game_on_destroy_004dc5c0(void* game) = 0;
    virtual void delete_game(void* game) = 0;

    // DAT_00f8abdc, then 00a3b6e0 with that client in ECX.
    virtual void* network_client_00f8abdc() = 0;
    virtual void network_stop_threads_00a3b6e0(void* client) = 0;

    // Virtual slot 0 with argument 1; the global is not cleared here.
    virtual void release_singleton(ShutdownSingleton which) = 0;
    // Unregister from the lifetime manager, destroy, clear the global.
    virtual void destroy_registered_singleton(ShutdownSingleton which) = 0;

    // 00a4d476.
    virtual void xlive_uninitialize() = 0;
    // 0051f460 then 00be8350, whose body is empty in this build.
    virtual void file_store_service_hook_00be8350() = 0;
    // 004fc150, 00be80b0, then cFileStore::RemoveFile at 00be7130.
    virtual void remove_datatable_file_00be7130(const ShutdownDatatable& file) = 0;
    // 00bbc5d0.
    virtual void free_global_block_00bbc5d0() = 0;
    // 00bb4db0.
    virtual void run_static_destructor_00bb4db0() = 0;
    // 004c9c90, a single RET that receives the message in ECX.
    virtual void trace_004c9c90(const char* message) = 0;
    // 00888750, a single RET.
    virtual void empty_hook_00888750() = 0;
};

// Drives the recovered order of 00737f30 over the injected host. Mirrors the
// original null guards: a null system slot, game pointer or network client
// skips its step, and cleared fields are written back to state.
void shutdown_application(ApplicationShutdownHost& host, ApplicationShutdownState& state);

// Number of recovered steps; kept as a function so callers need no array size
// arithmetic of their own.
std::uint32_t shutdown_step_count();

} // namespace bsp
