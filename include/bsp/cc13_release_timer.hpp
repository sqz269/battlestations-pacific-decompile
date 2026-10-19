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
// as before.
inline constexpr bool kReleaseTimerReseedDrawBound = false;

// 009FA3C3-009FA3CC: 007BB110 BSP_Unit_CanReleaseOrdnance(ECX = unit). True:
// the host evaluates 007BB110 itself (unit_can_release_007bb110 below) from
// channel C of the unit+DECh block and the bay byte unit+9C3h. False: the
// substitution "carries a torpedo (2Bh) && torpedo_releases < 1", as before.
// The substitution caps the timer at one fire per aircraft; the real test
// does not, so without the reseed a second fire would come on the very next
// tick (countdown 0.0f - dt < 0). Hence the dependency.
inline constexpr bool kReleaseTimerPredicate007BB110Bound = false;
static_assert(!kReleaseTimerPredicate007BB110Bound || kReleaseTimerReseedDrawBound,
              "the real 007BB110 needs the 009FA3EA reseed, or the timer fires every tick");

// 009FA3F4-009FA405: after a fire, timer+0Dh = 1 when the unit's step-record
// byte [unit + 9C0h + 8*word[00F876B8]] (007CDC70's 007B9140(unit, 0)) is 0.
// True: the host evaluates that byte and stores the notify on the slot. False:
// the notify is never set, as before. The image has NO reader of timer+0Dh
// (docs/CC13_RELEASE_TIMER.md section 2), so this switch changes no behaviour;
// it adds one log line per store.
inline constexpr bool kReleaseTimerNotify0DhBound = false;

// 009D2F09 MOVSS XMM1,[00CE3800] (00 00 00 3F), stored at timer+4h.
inline constexpr float kReseedLow_00ce3800 = 0.5f;
// 009D2F39 MOVSS XMM1,[00D7A24C] (00 00 80 3F), stored at timer+8h.
inline constexpr float kReseedHigh_00d7a24c = 1.0f;
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
std::vector<bool> bomb_rack_bay_doors(bsp::game::GameMissionLuaHost& lua, int type_id);

}  // namespace bsp::cc13_release_timer

#endif  // BSP_CC13_RELEASE_TIMER_HPP
