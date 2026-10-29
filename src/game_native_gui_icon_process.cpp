#include "bsp/game_native_gui_icon_process.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Icon process metadata requires MSVC Win32.
#endif

namespace bsp::game {
// Supported PE: .data RVA A08000, virtual size297EDC, raw size10000.
// F8BD98 is section offset183D98, hence the loader's exact00000000 word.
// A typed null initializes this ONE Source cell; it is never eagerly reset.
// Evidence and the installed image hash: docs/CODEX_GUI_ICON_DELAYED_PROCESS.md.
GameNativeGuiIconProcess::GameNativeGuiIconProcess() noexcept = default;
GameNativeGuiIconProcess::~GameNativeGuiIconProcess() = default;

NativeGuiIconDelayedLoads& GameNativeGuiIconProcess::prepare_delayed_loads(
    NativeStringRawPoolContext& strings) {
    if (delayed_loads_) {
        if (borrowed_strings_ != &strings)
            throw std::logic_error("GUI Icon delayed process cannot retarget its canonical raw string/manager context");
        return *delayed_loads_;
    }
    // The genuine constructor admits the null cell and retains references;
    // it performs no native effect. Do not publish a borrow before this
    // potentially throwing metadata construction has completed successfully.
    auto prepared = std::make_unique<NativeGuiIconDelayedLoads>(strings, publication_00f8bd98_);
    borrowed_strings_ = &strings;
    delayed_loads_ = std::move(prepared); // Nonthrowing coherent preparation commit.
    return *delayed_loads_;
}

NativeGuiIconDelayedLoads& GameNativeGuiIconProcess::delayed_loads() {
    if (!delayed_loads_)
        throw std::logic_error("GUI Icon delayed process requires successful metadata preparation");
    return *delayed_loads_;
}

NativeGuiIconDelayedLoadsStorage* GameNativeGuiIconProcess::publication_00f8bd98() const noexcept {
    return publication_00f8bd98_;
}

GameNativeGuiIconProcess::DelayedLoadsStatus
GameNativeGuiIconProcess::delayed_loads_status() const noexcept {
    if (!delayed_loads_) return DelayedLoadsStatus::unprepared;
    if (delayed_loads_->failure()) return DelayedLoadsStatus::failed_operation;
    const auto* const operation = delayed_loads_->last_operation();
    if (operation && operation->phase != NativeGuiIconDelayedLoadsOperation::Phase::complete)
        return DelayedLoadsStatus::incomplete_operation;
    // The low-level service also retains ordinary registered owners. Such
    // custody is not evidence of a failed/active operation or an _Exit demand.
    return delayed_loads_->requires_process_retention()
        ? DelayedLoadsStatus::registered_owner : DelayedLoadsStatus::idle;
}

GameNativeGuiIconProcess& game_native_gui_icon_process() {
    // Retained metadata only, matching the existing GUI Text state pattern.
    // Shared native drain owns registered payload deletion. No C++ atexit
    // callback competes with it or drops a service whose graph is retained.
    static auto* const process = new GameNativeGuiIconProcess;
    return *process;
}
} // namespace bsp::game
