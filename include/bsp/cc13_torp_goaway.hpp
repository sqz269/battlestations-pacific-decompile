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
inline constexpr bool kTorpGoawayEnterDrawsBound = true;      // parent
inline constexpr bool kTorpGoawayDistanceJitterDrawBound = true;  // 009D0E2C
inline constexpr bool kTorpGoawayClimbJitterDrawBound = true;     // 009D0E71
inline constexpr bool kTorpGoawayWindowDelayDrawBound = true;     // 009D0EA3
// 00901610 run once through the mission host's script reader, so row+10h/+14h
// come from this installation's loaded Robots.lua rather than a copy. OFF: no
// load, no script reads, no log lines (the OFF run is unchanged). The window
// delay draw needs it; without a loaded row the draw stays the OFF hole.
inline constexpr bool kPilotBotConfigLoadBound = true;

// 009D0F10 BSP_BotStateTorpedoGoAway_Tick, RET 4: the re-seed branch
// (009D0FB2-009D1025, delay +28h < 0 and the aircraft above +20h) draws twice
// more on stream 1, in this order:
//   009D0FD7  UniformFloatRange(3.0 [00CE3854], 6.0 [00CE6630]) -> +34h
//   009D1000  UniformFloatRange([row+10h], [row+14h]) + (+34h)  -> +28h
// (ECX = 1 at 009D0FC9 / 009D0FF1; row = [[state+4]+14h], the same PilotBot row).
// Same parent as the enter's three (kTorpGoawayEnterDrawsBound).
inline constexpr bool kTorpGoawayTickJitterDrawBound = true;     // 009D0FD7
inline constexpr bool kTorpGoawayTickDelayDrawBound = true;      // 009D1000

inline constexpr bool tick_jitter_draw_on() noexcept {
    return kTorpGoawayEnterDrawsBound && kTorpGoawayTickJitterDrawBound;
}
inline constexpr bool tick_delay_draw_on() noexcept {
    return kTorpGoawayEnterDrawsBound && kTorpGoawayTickDelayDrawBound && kPilotBotConfigLoadBound;
}

// ---------------------------------------------------------------------------
// The dive-bomb goaway (docs/CC13_TORP_GOAWAY.md part C). Same stream, same
// shape as the torpedo's; the host pinned all three at their low ends.
//   009C49FC  UniformFloatRange(1.0 FLD1, 1.25 [00CF29A8]) * +20h -> +20h
//             (enter 009C4950, ECX = 1 at 009C49F2, unconditional)
//   009C4D13  UniformFloatRange(3.0 [00CE3854], 6.0 [00CE6630])   -> +2Ch
//   009C4D33  UniformFloatRange(12.0 [00CEB4B8], 15.0 [00CE5380]) + +2Ch -> +24h
//             (both inside 009C4CF1's 0 > +24h guard; ECX = EBX = 1 from
//             009C4A5C, the only EBX write on that path)
// ---------------------------------------------------------------------------
inline constexpr bool kDiveGoawayDrawsCc13Bound = true;           // parent
inline constexpr bool kDiveGoawayStandoffDrawBound = true;        // 009C49FC
inline constexpr bool kDiveGoawayWindowDrawBound = true;          // 009C4D13
inline constexpr bool kDiveGoawayCountdownDrawBound = true;       // 009C4D33

inline constexpr bool dive_standoff_draw_on() noexcept {
    return kDiveGoawayDrawsCc13Bound && kDiveGoawayStandoffDrawBound;
}
inline constexpr bool dive_window_draw_on() noexcept {
    return kDiveGoawayDrawsCc13Bound && kDiveGoawayWindowDrawBound;
}
inline constexpr bool dive_countdown_draw_on() noexcept {
    return kDiveGoawayDrawsCc13Bound && kDiveGoawayCountdownDrawBound;
}

// 007B5BE0 BSP_UnitInstance_GetLargestExtent on the goaway target, when it is
// non-null and answers vtable[5Ch](5) (kind 5 = any unit). Despite the name it
// is max(max(+448h, +444h), +434h) = the longest category 6 / 5 / 1 ammunition
// range (00956D63 into unit+430h + cat*4; LIGHTARTILLERYFLAK / FLAK /
// AAMACHINEGUN), so the standoff grows to clear the target's AA envelope.
// One switch per enter (OFF: no target term; the host still logs the term):
//   dive    009C499C-009C49E0  [approach+48h] vs 2 * approach+B4h, JBE (equal -> term)
//   torpedo 009D0DC9-009D0E0F  [approach+CCh] vs SafeDist tuning+438h, JA
//   level   009B56B6-009B5702  [approach+48h] vs SafeDist tuning+450h, JA, then
//                              min(.., 3500) (qword 00D046A0 / float 00D04698)
// The rocket enter 007B5F10 (007B5F5B-007B5FAD: [approach+70h] vs tuning+664h
// Strike/AttackDist * 0.8 qword 00CE3D40, JA) has no switch: the host runs no
// rocket task (RocketTaskHost has no implementer), so there is nothing to bind.
inline constexpr bool kDiveGoawayTargetExtentCc13Bound = true;
inline constexpr bool kTorpGoawayTargetExtentCc13Bound = true;
inline constexpr bool kLevelGoawayTargetExtentCc13Bound = true;

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
