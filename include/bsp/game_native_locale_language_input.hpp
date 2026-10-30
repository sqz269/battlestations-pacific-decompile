#pragma once

#include <cstdint>

namespace bsp { struct NativeStringRawPoolContext; }
namespace bsp::game {
class GameNativeSettingsApplication;
class GameNativeLocaleManagerProcess;
class GameNativeStringProcess;
class GameNativeLocaleLanguageInputFrame;

// Actual [73E06A,73E0BA) only. Bind genuine ready settings, pristine initialized
// canonical locale and SAME permanent string process. Construction binds
// addresses only; caller retains the whole graph and excludes invalidating
// writers/reentry through provider internals outside this private operation.
class GameNativeLocaleLanguageInputContext final {
public:
    GameNativeLocaleLanguageInputContext(GameNativeSettingsApplication&,
        GameNativeLocaleManagerProcess&, GameNativeStringProcess&) noexcept;
    GameNativeLocaleLanguageInputContext(const GameNativeLocaleLanguageInputContext&) = delete;
    GameNativeLocaleLanguageInputContext& operator=(const GameNativeLocaleLanguageInputContext&) = delete;
    GameNativeLocaleLanguageInputContext(GameNativeLocaleLanguageInputContext&&) = delete;
    GameNativeLocaleLanguageInputContext& operator=(GameNativeLocaleLanguageInputContext&&) = delete;
private:
    friend void initialize_native_locale_language_input_0073e06a(
        const GameNativeLocaleLanguageInputContext&, GameNativeLocaleLanguageInputFrame&);
    GameNativeSettingsApplication& settings_;
    GameNativeLocaleManagerProcess& locale_;
    GameNativeStringProcess& strings_;
};

struct GameNativeLocaleLanguageInputProgress {
    bool attempted{}, language_entered{}, language_returned{};
    bool construction_entered{}, construction_returned{}, state_0x25_armed{};
    bool locale_entered{}, locale_returned{}, normal_edge_consumed{}, normal_null_skip{};
    bool pool_getter_entered{}, pool_getter_returned{}, normal_return_entered{}, normal_return_returned{};
    bool selected_edge_consumed{}, selected_destroy_entered{}, selected_destroy_returned{};
    bool normal_completed{}, failed{};
};

// Caller-retained, nonmoving one-shot history. Scratch has no initial value
// until real41E870. Status/destruction inspect own metadata only; neither does
// Native cleanup. Retain Frame and unresolved graphs after any failure.
class GameNativeLocaleLanguageInputFrame final {
public:
    explicit GameNativeLocaleLanguageInputFrame(const GameNativeLocaleLanguageInputContext&) noexcept;
    ~GameNativeLocaleLanguageInputFrame() noexcept = default;
    GameNativeLocaleLanguageInputFrame(const GameNativeLocaleLanguageInputFrame&) = delete;
    GameNativeLocaleLanguageInputFrame& operator=(const GameNativeLocaleLanguageInputFrame&) = delete;
    GameNativeLocaleLanguageInputFrame(GameNativeLocaleLanguageInputFrame&&) = delete;
    GameNativeLocaleLanguageInputFrame& operator=(GameNativeLocaleLanguageInputFrame&&) = delete;
    GameNativeLocaleLanguageInputProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept { return phase_ == Phase::returned && normal_result_; }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::active || phase_ == Phase::failed;
    }
private:
    friend void initialize_native_locale_language_input_0073e06a(
        const GameNativeLocaleLanguageInputContext&, GameNativeLocaleLanguageInputFrame&);
    enum class Phase { fresh, active, returned, failed };
    const GameNativeLocaleLanguageInputContext& context_;
    alignas(std::uint32_t) unsigned char input_[8];
    Phase phase_{Phase::fresh};
    int state_{-1}; // Native selected state0x25 only after real ctor normal return
    bool normal_result_{};
    NativeStringRawPoolContext* raw_{};
    const char* language_{}; // historical identity after constructor reads
    void* constructor_result_{};
    void* captured_data_{};
    std::uint32_t captured_bytes_{};
    GameNativeLocaleLanguageInputProgress progress_{};
    void require_active();
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void unwind_selected() noexcept;
};

// New ordinary Game Source interface, not whole73D410/Native FH3/stack ABI.
// Inner rejected replay/mismatch never consumes another invocation's cleanup.
// Only the actually entered invocation owns state0x25; no globals/reload or
// startup/loaded permission follows from normal completion.
void initialize_native_locale_language_input_0073e06a(
    const GameNativeLocaleLanguageInputContext&, GameNativeLocaleLanguageInputFrame&);
} // namespace bsp::game
