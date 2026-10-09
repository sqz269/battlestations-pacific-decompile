#pragma once
#include "bsp/game_native_game_runtime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp { struct NativeStringRawPoolContext; }
namespace bsp::game {
// Retained Source composition of 73E150..73E1D9, not an Original FH3 frame.
// Requires an exclusive fresh runtime with every construction/lifetime service
// composed, the genuine Application+14 and current E1AE78 cells, and the SAME
// canonical raw string domain used by that runtime. These borrowed objects and
// the captured nonnull C string must remain valid through their last use.
// Lifetime free_00bf6989 must match canonical malloc and actually free on a
// successful flags&1 call. A throwing override can leave disposition unknown.
// No automatic cleanup, publication rollback, operation replay or pool owner.
class GameNativeGameApplicationFrame final {
public:
    enum class Phase { fresh, constructing, live, deleting, destroyed, failed,
        retired, diagnostic_retired };
    enum class Stage { none, allocation, name_preparation, game_construction,
        name_return, scalar_delete };
    enum class Allocation { absent, retained, indeterminate, released };
    enum class Name { unprepared, preparing, complete, return_attempted,
        returned, externally_resolved };
    enum class DiagnosticDisposition { no_allocation, already_released };
    struct Status {
        Phase phase;
        Stage active_stage, failed_stage;
        Allocation allocation;
        Name name;
        GameNativeGameRuntime::Phase runtime_phase;
        std::uintptr_t allocation_address, constructor_result_address,
            scalar_result_address; // Historical bits, NOT live ownership.
        const char* captured_name; // Borrow only; never freed by this frame.
        // Caller/helper boundary anchors; child fault progress stays in runtime.
        std::uint32_t native_site, failed_native_site, observed_name_mask,
            scalar_flags;
        std::int32_t parent_cleanup_state;
        bool parent_state_recorded, constructor_entered, constructor_completed,
            application_published, scalar_attempted, scalar_completed;
    };
    GameNativeGameApplicationFrame(GameNativeGameRuntime&,
        void* volatile& actual_application_game_14,
        char* volatile& actual_duplicate_name_00e1ae78,
        NativeStringRawPoolContext& canonical_strings);
    ~GameNativeGameApplicationFrame();
    GameNativeGameApplicationFrame(const GameNativeGameApplicationFrame&)=delete;
    GameNativeGameApplicationFrame& operator=(const GameNativeGameApplicationFrame&)=delete;
    GameNativeGameApplicationFrame(GameNativeGameApplicationFrame&&)=delete;
    GameNativeGameApplicationFrame& operator=(GameNativeGameApplicationFrame&&)=delete;
    NativeGameStorage* construct();
    NativeGameStorage* scalar_delete(std::uint32_t flags);
    // Only proven retained storage: never entered, scalar completed with bit 0
    // clear, or externally resolved graph already acknowledged by the runtime.
    void free_retained_storage();
    // Caller has actually resolved the pending partial/completed name. This
    // acknowledgment performs no header access, return, clear or retry.
    void acknowledge_name_resolution();
    // Caller has resolved graph/name/publication obligations and explicitly
    // proves the original allocation absent or already released. No cleanup.
    void acknowledge_diagnostic_retirement(DiagnosticDisposition);
    Status status() const noexcept;
    NativeGameStorage* retained_storage() const noexcept { return retained_; }
    // Diagnostic bytes may describe a partial or ALREADY RETURNED buffer.
    // Their nonnull value never authorizes repeating raw string cleanup.
    void* name_header_for_diagnostics() noexcept;
private:
    bool active() const noexcept;
    bool name_resolved() const noexcept;
    bool runtime_retired() const noexcept;
    void record_failure() noexcept;
    GameNativeGameRuntime& runtime_;
    void* volatile& application_game_14_;
    char* volatile& duplicate_name_00e1ae78_;
    NativeStringRawPoolContext& strings_;
    alignas(4) std::byte name_header_[8]; // Callee initializes; no preimage claim.
    NativeGameStorage* retained_{};
    Phase phase_{Phase::fresh};
    Stage active_stage_{Stage::none}, failed_stage_{Stage::none};
    Allocation allocation_{Allocation::absent};
    Name name_{Name::unprepared};
    std::uintptr_t allocation_address_{}, constructor_result_address_{},
        scalar_result_address_{};
    const char* captured_name_{};
    std::uint32_t native_site_{}, failed_native_site_{}, observed_name_mask_{},
        scalar_flags_{};
    std::int32_t parent_cleanup_state_{}; // Unobserved until state 27h.
    bool parent_state_recorded_{}, constructor_entered_{}, constructor_completed_{},
        application_published_{}, scalar_attempted_{}, scalar_completed_{};
};
} // namespace bsp::game
