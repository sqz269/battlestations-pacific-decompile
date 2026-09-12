#pragma once
#include "bsp/platform_cursor.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/xlive_library.hpp"

namespace bsp::game {
class GameSoundCursorCalls {
public:
    virtual ~GameSoundCursorCalls() = default;
    virtual void update_cursor_focus_00becb20(bool loading) = 0;
};
// Load-time platform adapter. This borrows the application's existing cursor,
// input/online publication services, and actual selected XLive DLL. It creates
// no input or online owners and has no substitute pretranslation result.
class GameSoundLoadEvents final : public ResourceLoadEventHost {
public:
    GameSoundLoadEvents(GameSoundCursorCalls&, XLiveLibrary&) noexcept;
    bool pretranslate(MSG&) override;
    void update_cursor_focus_00becb20(bool loading) override;
    std::uint64_t pretranslation_calls() const noexcept { return pretranslation_calls_; }
    std::uint64_t focus_calls() const noexcept { return focus_calls_; }
private:
    GameSoundCursorCalls& cursor_;
    XLiveLibrary& xlive_;
    std::uint64_t pretranslation_calls_{};
    std::uint64_t focus_calls_{};
};
} // namespace bsp::game
