#pragma once
#include <memory>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeGuiIconDelayedLoadsStorage;
class NativeGuiIconDelayedLoads;
}

namespace bsp::game {
// Sole typed Source backing for the recovered F8BD98 publication. The
// supported Original PE supplies a loader-zero DWORD; this is not storage at
// that VA, a replacement object layout, or an Original callable publication.
// Like GameNativeGuiTextState, this metadata owner survives process teardown.
// No native getter, registration, cancellation or deletion runs on access.
class GameNativeGuiIconProcess final {
public:
    GameNativeGuiIconProcess(const GameNativeGuiIconProcess&) = delete;
    GameNativeGuiIconProcess& operator=(const GameNativeGuiIconProcess&) = delete;

    enum class DelayedLoadsStatus {
        unprepared,
        idle,
        registered_owner,    // Normal custody: keep providers through native drain.
        incomplete_operation, // In flight, or a nonlocal escape; do not abandon it.
        failed_operation     // Stored entered-operation failure, not normal custody.
    };

    // Lazily construct only the genuine service's C++ metadata. The exact
    // stable raw context (including its 01090AA0 manager publication cell) is
    // borrowed once; another context object is rejected, even with aliases.
    // Successful preparation publishes that borrow and service together.
    // Allocation/admission failure leaves both unprepared and invokes no
    // native operation. Same-context repetition returns the SAME service;
    // it does not reset a publication, operation, failure or registration.
    NativeGuiIconDelayedLoads& prepare_delayed_loads(NativeStringRawPoolContext&);
    // Pure existing-service access; rejects before preparation. This is not
    // permission to replay an interrupted operation or use expired providers.
    NativeGuiIconDelayedLoads& delayed_loads();

    // Current observational views only: no writable publication-cell API,
    // pointee reads, ownership credit or implicit preparation/native call.
    NativeGuiIconDelayedLoadsStorage* publication_00f8bd98() const noexcept;
    DelayedLoadsStatus delayed_loads_status() const noexcept;

    // Before ANY get_00ab5700 (including ordinary Icon destruction), root must
    // bind native_deletion_bindings().gui_icon_delayed_loads to THIS service.
    // Retain the SAME service/cell/raw-context/string/manager graph through
    // full shared native drain and every failure disposition. A registered
    // owner alone is normal retention; incomplete/failed calls require the
    // existing sticky failure/process-exit policy. Serialize every access.
    // Post-drain reuse/new-host startup and delayed queue/file delivery are
    // outside this packet. No native owner is destroyed by process metadata.
private:
    friend GameNativeGuiIconProcess& game_native_gui_icon_process();
    GameNativeGuiIconProcess() noexcept;
    ~GameNativeGuiIconProcess();

    NativeGuiIconDelayedLoadsStorage* volatile publication_00f8bd98_{};
    NativeStringRawPoolContext* borrowed_strings_{};
    std::unique_ptr<NativeGuiIconDelayedLoads> delayed_loads_;
};

GameNativeGuiIconProcess& game_native_gui_icon_process();
} // namespace bsp::game
