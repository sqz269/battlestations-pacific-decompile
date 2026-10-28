#ifndef BSP_CC13_RELEASE_TIMER_HPP
#define BSP_CC13_RELEASE_TIMER_HPP

// Packet cc13-reltimer: the three inputs the host binding of 009FA3A0
// BSP_ReleaseTimer_Tick (src/game_hosts_units.cpp, AimTickBinding::
// accumulate_timer_009fa3a0) stubbed, each behind its own switch, all
// committed OFF. docs/CC13_RELEASE_TIMER.md carries the evidence; the timer
// body itself stays bsp::release_timer_tick_009fa3a0
// (src/torpedo_first_release.cpp).
//
// The timer is the aim state's member at state+18h (009D2019 LEA EBP,[ESI+18h]);
// the aim state is task+318h, built inline by 009D2DA0
// BSP_BotTaskTorpedo_ConstructStates (009D2F11-009D2F51):
//   timer+0h  = [[state+4]+4], the unit           (009D2F28/009D2F31/009D2F41)
//   timer+4h  = [00CE3800] 0.5f, the reseed low   (009D2F09 MOVSS / 009D2F34)
//   timer+8h  = [00D7A24C] 1.0f, the reseed high  (009D2F39 MOVSS / 009D2F44)
//   timer+0Ch = 0, the enable byte                (009D2F49)
//   timer+0Dh = 0, the notify byte                (009D2F4D)
//   timer+10h = 0.0f, the countdown               (009D2F51)
// Nothing else writes +4h/+8h, so the reseed bounds are those two constants.

#include <cstdint>
#include <vector>

namespace bsp::game {
class GameMissionLuaHost;
}

namespace bsp::cc13_release_timer {

// 009FA3D5-009FA3F1: countdown = 00BD2F10(ECX = 1, [timer+4h], [timer+8h]),
// FLD float / FSTP float for both bounds and FSTP float [ESI+10h] for the
// result. Stream 1 is the SHARED generator, so every fire adds one draw to it.
// True: the host's countdown after a fire is that draw (through
// GameGunneryHost::death_mode_draw_00bd2f10(1, ...)), so the next fire waits
// 0.5-1.0 s. False: the countdown is 0.0f after a fire and nothing is drawn,
// as before. The fidelity gain is the countdown VALUE: the image's stream 1
// is per-thread and clock-seeded (docs/CC13_RNG_STREAM_CENSUS.md), so its
// sequence is not reproducible. The switch is separate because the host draw
// shifts the host's shared stream.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 101). Pairs
// local\p24n_off vs local\p24h1 (main c01bcb8fb), both envs: every row moves;
// BSM09 and USN21 still end; USN01 u1i loses its default-env completion
// (RNG-coupled, no end in the ref env on either side).
inline constexpr bool kReleaseTimerReseedDrawBound = true;

// 009FA3C3-009FA3CC: 007BB110 BSP_Unit_CanReleaseOrdnance(ECX = unit). True:
// the host evaluates 007BB110 itself (unit_can_release_007bb110 below) from
// channel C of the unit+DECh block and the bay byte unit+9C3h. False: the
// substitution "carries a torpedo (2Bh) && torpedo_releases < 1", as before.
// The substitution caps the timer at one fire per aircraft; the real test
// does not, so without the reseed a second fire would come on the very next
// tick (countdown 0.0f - dt < 0). Hence the dependency.
//
// The predicate also needs the bay opener (kReleaseBayOpenCc13Bound): 007BB110
// refuses while a loaded bay-door rack's channel C is below 1.0f, and without
// the opener nothing in the host but 007BBBA0 (behind this very test) moves
// it. physics24's pairs showed it: BSM09 timer_blocked=378, fires 0, the
// mission lost; USN21 torpedo drops 5 -> 0.
// cc9-physics24 2026-10-19: ON by mechanism over kReleaseBayOpenCc13Bound
// (OCEAN_WAVE_FIELD 103): with the bay opener, 2 blocks on USN21 default,
// none elsewhere; every win holds.
inline constexpr bool kReleaseTimerPredicate007BB110Bound = true;

// The image's bay opener (docs/CC13_RELEASE_TIMER.md section 10), two pieces:
//  - 0099AFCC-0099B055 in BSP_PilotBot_Tick 0099ACD0: mode = the first task's
//    vtable[+50h] that is not 1 (default 1); flag = (mode == 2), and in mode 1
//    a wingman (squadron [unit+9D4h]+3D0h, the leader, set and not this unit)
//    takes the leader's step byte 9C1h (channel C idle or at 1.0f) instead;
//    007BB150 stores unit+DF0h = flag ? 9C3h : 0. Torpedo task +50h is
//    009D49E0 (2 in aim/done/goaway/attackrun/prepare with approach+130h, the
//    aim-solution byte 009D2021, set; 0 there without it; 1 elsewhere), dive
//    bomb +50h is 009C8290 (2 in aimdive, 0 in its other attack states, 1
//    elsewhere).
//  - 007C6A8D-007C6B2E in the plane tick 007C6820 (plane vtable slot): unless
//    unit+520h, channel C's target +61h = 1 with the value forced to 1.0f when
//    the plane carries drop-kamikaze ordnance (007B91C0(2Fh, 0)), else target =
//    (+DF0h || +C25h || +C20h) ? 9C3h : 0, each change marking +68h and +11h.
// So an AI torpedo bomber's bay starts opening once the aim tick's
// aim-solution byte is set (release distance + 200 m), and the timer's
// 007BB110 passes once the channel reaches 1.0f. True: the host runs both
// pieces once per fixed step before the block step, for planes with an
// installed torpedo or dive-bomb task, and clears approach+130h at the task
// install (009D05AB, 009D0380 run from 009D2DA0). False: channel C moves only
// through 007BBBA0, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 103); gameplay
// identical alone on BSM09, USN01 u1i and USN21 in both envs.
inline constexpr bool kReleaseBayOpenCc13Bound = true;
static_assert(!kReleaseTimerPredicate007BB110Bound || kReleaseTimerReseedDrawBound,
              "the real 007BB110 needs the 009FA3EA reseed, or the timer fires every tick");
static_assert(!kReleaseTimerPredicate007BB110Bound || kReleaseBayOpenCc13Bound,
              "the real 007BB110 needs the bay opener, or a bay-door plane never releases");

// 009FA3F4-009FA405: after a fire, timer+0Dh = 1 when the unit's step-record
// byte [unit + 9C0h + 8*word[00F876B8]] (007CDC70's 007B9140(unit, 0)) is 0.
// True: the host evaluates that byte and stores the notify on the slot. False:
// the notify is never set, as before. The image has NO reader of timer+0Dh
// (docs/CC13_RELEASE_TIMER.md section 2), so this switch changes no behaviour;
// it adds one log line per store.
// ON (cc9-sweep3c, 2026-10-28) without pairs: no reader, so no behaviour moves.
inline constexpr bool kReleaseTimerNotify0DhBound = true;

// 009D2F09 MOVSS XMM1,[00CE3800] (00 00 00 3F), stored at timer+4h.
inline constexpr float kReseedLow_00ce3800 = 0.5f;  // 00CE3800
// 009D2F39 MOVSS XMM1,[00D7A24C] (00 00 80 3F), stored at timer+8h.
inline constexpr float kReseedHigh_00d7a24c = 1.0f;  // 00D7A24C
// 009FA3DF MOV ECX,1.
inline constexpr int kReseedStream = 1;

// 007BB110, char __thiscall(unit), no stack arguments, plain RET:
//   channel = [unit+DECh]+60h
//   007BB119 CMP byte [channel],0 / JZ -> AL = 1      (channel C disabled)
//   007BB11E UCOMISS [channel+4],[00D7A24C] 1.0f; LAHF / TEST AH,44h / JP:
//     equal and ordered -> AL = 1                    (bay fully open)
//     not equal or unordered -> fall through
//   007BB133 AL = ([unit + 9C3h + 8*word[00F876B8]] == 0)
// so it refuses only while the bay channel is enabled, not at exactly 1.0f,
// and the step record says a bay-door rack still holds a round. A NaN value
// takes the fall-through, as the comparison below does.
bool unit_can_release_007bb110(bool channel_c_enabled, float channel_c_value,
                               bool bay_rack_holds_round_9c3) noexcept;

// The authored UseBayDoor of each bomb rack, in the host's rack order
// (GameUnitsHost::Impl::ordnance_racks: the flattened BSPGun rows p1..pn with
// p<i>_cat == BOMBPLATFORM and a device Type of BombPlatform or
// MultiBombPlatform). The image reads it as [[plane+538h]+94h][child+38Ch]+0Dh
// (007CD9A0), the platform record VehicleClass[id].Platforms[k].UseBayDoor
// fills at 0096127A; BSPGun's p<i>_key is that k. An entry is false when the
// platform or the field is absent. SUBSTITUTION, labelled: the image walks the
// plane's device list (unit+48h), and the host's rack order stands for it.
// Cached per class in the Lua host (one mission; packet cc13-baystale) once
// BSPGun exists and the chunk succeeded, so the chunk runs once per class.
std::vector<bool> bomb_rack_bay_doors(bsp::game::GameMissionLuaHost& lua, int type_id);

// 0099B003-0099B055 with 007BB150: the byte BSP_PilotBot_Tick stores at
// unit+DF0h. `mode` is the task's vtable[+50h] answer (1 when no task answers
// otherwise). `has_leader` is true when [unit+9D4h] and its +3D0h are set and
// +3D0h is not this unit (0099B01C-0099B039); `leader_9c1` is then that
// leader's step byte 9C1h. `own_9c3` is this unit's 9C3h (007BB158).
std::uint8_t pilot_step_flag_df0_0099b055(int mode, bool has_leader, bool leader_9c1,
                                          bool own_9c3) noexcept;

// 007CDC70's 9C1h for a unit with a block: channel C disabled, or its value
// exactly 1.0f (UCOMISS ordered equal, 007CDC89-007CDC94).
bool step_byte_9c1_007cdc84(bool channel_c_enabled, float channel_c_value) noexcept;

}  // namespace bsp::cc13_release_timer

namespace bsp {
struct PlaneActuatorBlock;
}

namespace bsp::cc13_release_timer {

// 007C6A8D-007C6B2E, the channel C arm of 007C6820: nothing when unit+520h is
// set; with drop-kamikaze ordnance (007B91C0(2Fh, 0)) the channel is forced
// open (+64h = 1.0f, +61h = 1, +68h = 1) unless it already is (target 1 and
// value exactly 1.0f); otherwise target = (DF0h || C25h || C20h) ? 9C3h : 0.
// A changed target marks the channel moving (+68h); a moving channel raises
// the block's +11h. Returns true when the target or value was written.
bool plane_bay_target_007c6a8d(bsp::PlaneActuatorBlock& block, bool suppress_520,
                               bool drop_kamikaze_2f, bool df0, bool c25, bool c20,
                               bool own_9c3) noexcept;

}  // namespace bsp::cc13_release_timer

#endif  // BSP_CC13_RELEASE_TIMER_HPP
