#pragma once
// Packet cc13-goaway (docs/CC13_TORP_GOAWAY.md): the torpedo-bomber goaway
// enter's three shared-RNG draws, and the PilotBotConfig load the third one
// needs. Semantic interfaces, not native layouts; names are hypotheses.

#include "bsp/lua_script_runtime.hpp"
#include "bsp/robot_config.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace bsp {
struct MissionLuaHostServices;
}

namespace bsp::cc13_torp_goaway {

// ---------------------------------------------------------------------------
// Switches. All OFF: the host keeps the low ends it has always taken
// (torpedo_goaway::kDistanceJitterLo, torpedo_goaway_tick::kClimbJitterLo) and
// the TorpFlikFlakTime hole (window delay 0, an `unimplemented` record).
//
// 009D0D90 BSP_BotStateTorpedoGoAway_Enter, __fastcall(ECX = state), RET, draws
// on stream 1 (ECX = 1, CALL 00BD2F10) in this order:
//   009D0E2C  UniformFloatRange(1.0 FLD1, 1.15 [00D20CE4])    -> * SafeDist -> +24h
//   009D0E71  UniformFloatRange(50 [00CEB4D4], 100 [00CE3D08]) -> + band   -> +1Ch
//             (only when [approach+132h] != 0, the JZ at 009D0E4A)
//   009D0EA3  UniformFloatRange(row+10h, row+14h)              -> +28h
// where row = approach+14h = &PilotBotConfig.levels[[[unit+DF4h]+34h]]
// (009F9CFF-009F9D22), row+10h/+14h = TorpFlikFlakTime 1/2 (reader 009973B0,
// 00997C68 / 00997CC5). Each draw advances the shared stream, so a sub-switch
// that is ON while an earlier one is OFF still leaves the later draws in the
// image's relative order but one draw short; turn the three on together.
// ---------------------------------------------------------------------------
inline constexpr bool kTorpGoawayEnterDrawsBound = false;      // parent
inline constexpr bool kTorpGoawayDistanceJitterDrawBound = false;  // 009D0E2C
inline constexpr bool kTorpGoawayClimbJitterDrawBound = false;     // 009D0E71
inline constexpr bool kTorpGoawayWindowDelayDrawBound = false;     // 009D0EA3
// 00901610 run once through the mission host's script reader, so row+10h/+14h
// come from this installation's loaded Robots.lua rather than a copy. OFF: no
// load, no script reads, no log lines (the OFF run is unchanged). The window
// delay draw needs it; without a loaded row the draw stays the OFF hole.
inline constexpr bool kPilotBotConfigLoadBound = false;

inline constexpr bool distance_jitter_draw_on() noexcept {
    return kTorpGoawayEnterDrawsBound && kTorpGoawayDistanceJitterDrawBound;
}
inline constexpr bool climb_jitter_draw_on() noexcept {
    return kTorpGoawayEnterDrawsBound && kTorpGoawayClimbJitterDrawBound;
}
inline constexpr bool window_delay_draw_on() noexcept {
    return kTorpGoawayEnterDrawsBound && kTorpGoawayWindowDelayDrawBound
        && kPilotBotConfigLoadBound;
}

// ---------------------------------------------------------------------------
// The script files 00901610 reads, over the mission host's VFS reader
// (00885110: open +18h, size +30h, read +24h, close) and its 00BDEF90
// override list. Missing and empty files are silent skips, as
// LuaScriptFiles requires. Host adaptation: the image's 00901610 runs on its
// own PcStorage owner whose files come from the same VFS singleton.
// ---------------------------------------------------------------------------
class MissionScriptFiles final : public LuaScriptFiles {
public:
    explicit MissionScriptFiles(MissionLuaHostServices& host) noexcept : host_(host) {}
    bool read_file_00b66ca0(const std::string& path, std::uint32_t mode,
        std::vector<char>& bytes) override;
    std::vector<std::string> override_paths_00bdef90(const std::string& path) override;
    std::size_t reads() const noexcept { return reads_; }
    std::size_t misses() const noexcept { return misses_; }

private:
    MissionLuaHostServices& host_;
    std::size_t reads_{0};
    std::size_t misses_{0};
};

struct PilotBotLoadResult {
    bool loaded{false};
    bool already_loaded{false};
    std::string error;
    std::size_t script_reads{0};
    std::size_t script_misses{0};
};

// 00901610 load_robot_config, the whole loader (luaMW_init.lua then Robots.lua
// on a mask-41h owner, nine descriptors, the alias lookups), run once per
// process like its caller 004DC6A0. The PilotBot rows are kept, as the image
// keeps [00F8A30C] for the process; the other descriptors are released.
// Throws nothing; a failure leaves the rows unloaded and says why.
PilotBotLoadResult load_pilot_bot_config_00901610(LuaScriptFiles& files);

// The six PilotBotConfig.levels rows, or null before a successful load.
const std::array<PilotBotParameters, 6>* loaded_pilot_bot_rows() noexcept;

// The row approach+14h points at: levels[index], index = [[unit+DF4h]+34h].
// Out of 0..5 there is no image row (it would read past the array); null.
const PilotBotParameters* pilot_bot_row(int index) noexcept;

}  // namespace bsp::cc13_torp_goaway
