#pragma once

namespace bsp::game {
class GameNativeLocaleManagerProcess;
class GameNativeStringProcess;
class GameNativeReadOnlyData;
class GameNativeLocaleGlobalsInputFrame;

// Address-only immutable borrow of the actual canonical owner, SAME permanent
// raw-string process and retained verified read-only data mapping. Caller keeps
// the entire provider/control graph live and disjoint through every callback.
class GameNativeLocaleGlobalsInputContext final {
public:
    GameNativeLocaleGlobalsInputContext(GameNativeLocaleManagerProcess&,
        GameNativeStringProcess&, const GameNativeReadOnlyData&) noexcept;
    GameNativeLocaleGlobalsInputContext(const GameNativeLocaleGlobalsInputContext&) = delete;
    GameNativeLocaleGlobalsInputContext& operator=(const GameNativeLocaleGlobalsInputContext&) = delete;
    GameNativeLocaleGlobalsInputContext(GameNativeLocaleGlobalsInputContext&&) = delete;
    GameNativeLocaleGlobalsInputContext& operator=(GameNativeLocaleGlobalsInputContext&&) = delete;
private:
    friend void register_native_locale_globals_input_0073e0ba(
        const GameNativeLocaleGlobalsInputContext&, GameNativeLocaleGlobalsInputFrame&);
    GameNativeLocaleManagerProcess& owner_;
    GameNativeStringProcess& strings_;
    const GameNativeReadOnlyData& data_;
};

struct GameNativeLocaleGlobalsInputProgress {
    bool attempted{}, admitted{}, literal_returned{};
    bool length_zeroed{}, data_zeroed{}, resize_entered{}, resize_returned{};
    bool copy_entered{}, copy_returned{}, copy_skipped{}, state26_armed{};
    bool registration_entered{}, registration_returned{}, normal_data_captured{};
    bool normal_disarmed{}, getter_entered{}, getter_returned{};
    bool return_entered{}, return_returned{}, selected_entered{}, selected_returned{};
    bool normal_returned{}, failed{};
};

// Caller-retained nonmoving raw8 and private one-shot history. Constructor,
// status and default destruction are own-metadata only; no native RAII cleanup.
class GameNativeLocaleGlobalsInputFrame final {
public:
    explicit GameNativeLocaleGlobalsInputFrame(
        const GameNativeLocaleGlobalsInputContext&) noexcept;
    ~GameNativeLocaleGlobalsInputFrame() noexcept = default;
    GameNativeLocaleGlobalsInputFrame(const GameNativeLocaleGlobalsInputFrame&) = delete;
    GameNativeLocaleGlobalsInputFrame& operator=(const GameNativeLocaleGlobalsInputFrame&) = delete;
    GameNativeLocaleGlobalsInputFrame(GameNativeLocaleGlobalsInputFrame&&) = delete;
    GameNativeLocaleGlobalsInputFrame& operator=(GameNativeLocaleGlobalsInputFrame&&) = delete;
    GameNativeLocaleGlobalsInputProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept { return phase_ == Phase::returned && normal_result_; }
private:
    friend void register_native_locale_globals_input_0073e0ba(
        const GameNativeLocaleGlobalsInputContext&, GameNativeLocaleGlobalsInputFrame&);
    enum class Phase { fresh, active, returned, failed };
    enum class Cleanup { unarmed, armed_26, consumed };
    const GameNativeLocaleGlobalsInputContext& context_;
    alignas(4) unsigned char header_[8]; // not initialized until the actual two stores
    const void* literal_{}; // identity only after the admitted synchronous operation
    Phase phase_{Phase::fresh};
    Cleanup cleanup_{Cleanup::unarmed};
    bool normal_result_{};
    GameNativeLocaleGlobalsInputProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void complete();
};

// Only73E0BA..129 plus selected hex-state0x26. Real raw resize(7,preserve1),
// CURRENT length+1 copy from actual CF7774, borrowed owner registration, then
// EARLY-data normal return or CURRENT-header selected cleanup. No startup,
// reload, loaded permission, Native register/FH3/hardware or runtime grant.
void register_native_locale_globals_input_0073e0ba(
    const GameNativeLocaleGlobalsInputContext&, GameNativeLocaleGlobalsInputFrame&);
} // namespace bsp::game
