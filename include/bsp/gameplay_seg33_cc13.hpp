#pragma once

// Reference models from segment 33 (006DECA0-006FA320): the bomb platforms, single
// (MBombPlatform, class id 25h, vtable 00CF96A8) and multiple (MMultipleBombPlatform, 26h,
// 00CF9918), and their in-flight device reload (cc13-gpr33, docs/CC13_GPR_33.md).
//
// Status: reconstructed, build-tested. NOT wired into any host, not fixture-tested, not
// ABI-compatible: the natives are __thiscall methods on the platform record that reach the
// world through virtual slots and session messages; here the record is a plain struct and every
// virtual or message is an input or a returned event. Names are hypotheses, not recovered
// symbols. Offsets are on the platform record (the tick element is platform+310h).
//
// x87: the image runs with 53-bit precision; an FSTP to a float slot is a float cast. Every
// floating compare below was read in the listing (COMISS/FCOMIP and the branch after it); a NaN
// takes the "false" side in each case, which the C++ comparisons reproduce.
//
// ADOPTION NOTE (cc9). The table in docs/CC13_GPR_33.md "Adoption" is the full version.
//  - Replaces: nothing is replaced. The host's single-rack tick
//    GameUnitsHost run_rack_tick_006e56f0 (src/game_hosts_units.cpp:26935) skips the reload slice
//    006E57A4-006E57D2 (owner vtable[1E4h] -> rack vtable[204h]); rack_tick_reload_slice_006e57a4
//    and rack_advance_reload_006e41c0 / rack_rearm_006e3410 are that slice. The multi rack has no
//    host model at all: count_remaining_rounds_007c1db0 (src/game_hosts_units.cpp:4060) keeps a
//    substitute for it, and the MultiBombPlatform rocket racks (orx_rack_ammo, :579) hold a bare
//    ammo count. MultiRackState and its functions are that model.
//  - Inputs: the rack's authored Ammo and ReloadTime (the bag Equipment entry, 0095A880 ->
//    vtable[1BCh]/[1C0h]; the host already reads them for rack_authored_per_rack), RepeatTime
//    (class+E0h, rack_repeat_per_rack), the count of attached rounds (children answering
//    IsKindOf(2Ah)), dt, the device-reload flag [00E17BF2] (lua_device_reload_enabled_00e17bf2,
//    src/game_hosts_lua.cpp:6045), the reload multiplier [00E08178] (SetDeviceReloadTimeMul
//    008C14B0, not hosted; 1.0 by default) and the session mode [00E188A8]+1FE4h (0 here).
//  - RNG: none. No function modelled here draws from 00BD2F10.
//  - OFF-switch prediction: with the slice bound behind a switch, a rack that has spent its
//    rounds counts ReloadTime / mul seconds down while [00E17BF2] is set and then refills to its
//    authored Ammo (Rearm). In this installation's base campaigns the flag is set by
//    scripts/missions/ijn/JM/jm01, jm02, jm04, jm06, jm08, jm09, jm12, jm13, jm14 and
//    usn/usn_19_coralus (SetDeviceReloadEnabled(true)); jm01 also sets the multiplier to 3 and
//    jm03 to 0 (the countdown then never moves). So on JM08, a bomber that has dropped could drop
//    again ReloadTime seconds later (30 s when the Equipment entry gave none; 1 s floor). USN01,
//    USN04 and USN13 never set the flag and should not move.

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace bsp::gameplay_seg33_cc13 {

// ---------------------------------------------------------------------------------------------
// Constants, each with the address its bytes were read from.
// ---------------------------------------------------------------------------------------------
inline constexpr float kRackReloadTimeDefault = 30.0f;     // 00CE38C8, stored by 006E3C00
inline constexpr float kRackReloadTimeFloor = 1.0f;        // 00D7A24C, 006E3A20
inline constexpr float kRackRepeatIdle = -1.0f;            // 00D7A260, 006E5787 / 006E5A2A
inline constexpr float kBombPlatformRepeatTimeDefault = 0.6f;  // 00CE3D30, 006E0242
inline constexpr int kMultiRackSlots = 0x28;               // the 40-entry arrays of 00CF9918
inline constexpr float kMultiRackSlotIdle = -99.0f;        // 00CF5BFC
inline constexpr float kMultiRackFirstLoadDelay = 0.5f;    // 00CE3800, 006E5D51
inline constexpr float kMultiRackLoadDelay = 1.2f;         // 00CE3814, 006E5D70
inline constexpr float kMultiRackLoadParked = 999.0f;      // 00CF4888, 006E59A0
inline constexpr double kMultiRackLoadTimerCeiling = 100.0;  // 00D7A220 (a double), 006E594A
inline constexpr int kMultiRackRealRoundCap = 3;           // 006E5F64 CMP EAX,3 / JG
inline constexpr std::uint8_t kMsgGunFire = 0xAD;          // the fire message 006E3940
inline constexpr std::uint8_t kMsgBombDrop = 0xB1;
inline constexpr std::uint8_t kMsgPlatformReload = 0xB2;   // 006E3A60
inline constexpr std::uint8_t kMsgRackLoad = 0xB3;         // built inline, vtable 00CF9650

// The session mode at [00E188A8]+1FE4h: 0 and 1 run the authority arm (they route the message),
// 2 is a network client. Single player is 0.
enum class SessionMode : int { Local = 0, Host = 1, Client = 2 };

// ---------------------------------------------------------------------------------------------
// Single rack, MBombPlatform (constructor 006E3C00).
// ---------------------------------------------------------------------------------------------
struct SingleRackState {
    float field_480{kRackRepeatIdle};          // +480h, -1.0 at construction, read elsewhere
    int ammo_484{1};                           // `ammo` (serializer 006E4A60)
    int org_ammo_488{1};                       // `orgAmmo`
    float reload_time_48c{kRackReloadTimeDefault};
    float reload_left_490{kRackReloadTimeDefault};
    float to_repeat_494{0.0f};                 // `toRepeatTime`
    bool drop_bombs_498{false};                // `dropBombs`
    bool target_point_set_4f8{false};          // +4F8h, set only by 006E3F90
};

// 006E3C00, __thiscall(rack, cls), the rack fields after BSP_Gun_Construct.
SingleRackState rack_construct_006e3c00() noexcept;
// 006E3530, slot +1BCh, RET 4: ammo = orgAmmo = n.
void rack_set_ammo_006e3530(SingleRackState& rack, int n) noexcept;
// 006E3A20, slot +1C0h, RET 4: ReloadTime below 1.0 becomes 1.0; both +48Ch and +490h.
void rack_set_reload_time_006e3a20(SingleRackState& rack, float reload_time) noexcept;
// 006E3500: vtable[21Ch](2Ah) + ammo. attached_rounds is that vtable answer (006E34D0, the
// children answering IsKindOf(2Ah)).
int rack_round_count_006e3500(const SingleRackState& rack, int attached_rounds) noexcept;
// 006E3570, slot +20Ch.
bool rack_is_empty_006e3570(const SingleRackState& rack, int attached_rounds) noexcept;
// 006E4130, slot +214h: 0 with no orgAmmo, else rounds / orgAmmo (FILD/FIDIV, one float round).
float rack_ammo_fraction_006e4130(const SingleRackState& rack, int attached_rounds) noexcept;
// 006E4170, slot +1F8h: RepeatTime (class+E0h) x max(rounds - 1, 0) (FIMUL, one float round).
float rack_remaining_drop_time_006e4170(const SingleRackState& rack, int attached_rounds,
                                        float repeat_time) noexcept;
// 006E3550, slot +1F0h, RET 4: toRepeatTime += delay (float), dropBombs = 1.
void rack_fire_006e3550(SingleRackState& rack, float delay) noexcept;
// 006E3CD0, slot +1FCh: dropBombs.
bool rack_is_busy_006e3cd0(const SingleRackState& rack) noexcept;

// What 006E41C0 does when the countdown expires.
enum class RackReloadEvent {
    None,
    RouteReloadMessage,   // modes 0/1: B2h (006E3A60, argument -1) routed; +490h = +48Ch
    ClientZeroed,         // mode 2: +490h = 0
};
// 006E41C0, slot +204h, RET 4. dt is scaled by [00E08178] and rounded to float first; the
// countdown runs only while the rack is empty (vtable[20Ch] = 006E3570).
RackReloadEvent rack_advance_reload_006e41c0(SingleRackState& rack, int attached_rounds,
                                             float dt, float reload_mul,
                                             SessionMode mode) noexcept;
// 006E3410, slot +200h (the B2h arm of 006E4290): +490h = +48Ch; with a round still hanging
// (vtable[210h](2Ah, 0) = 006E3FE0 with the flag clear) ammo = orgAmmo - 1 and nothing else;
// otherwise ammo = orgAmmo and the tail jump to vtable[224h]. Returns true when that attach
// (006E3E70) follows; the caller then applies rack_attach_next_round_006e3e70.
bool rack_rearm_006e3410(SingleRackState& rack, bool round_attached) noexcept;
// 006E3E70, slot +224h: the ammo bookkeeping only. With the rack's +5Dh clear, ammo -= 1 whether
// or not the round's create succeeded (the create, the owner bind and the pose are the host's).
void rack_attach_next_round_006e3e70(SingleRackState& rack, bool rack_flag_5d) noexcept;

// 006E56F0 (ESI = rack+310h), the part between the live-owner gate (006E5718-006E5748, the
// caller's) and the fire window. Counts toRepeatTime down while above -1.0 (006E577F-006E579E),
// then runs the reload when the owner's vtable[1E4h] answers (006E57A4-006E57D2; for the plane
// that slot is 006D1F40, the byte [00E17BF2], docs/SQUADRON_ORDNANCE_STATE.md; other owner
// classes were not read).
RackReloadEvent rack_tick_reload_slice_006e57a4(SingleRackState& rack, int attached_rounds,
                                                float dt, bool device_reload_enabled,
                                                float reload_mul, SessionMode mode) noexcept;
// 006E57D4-006E57FA: the fire window opens with dropBombs, dt > 0 and toRepeatTime < 0.
bool rack_fire_window_open_006e57d4(const SingleRackState& rack, float dt) noexcept;
// 006E5816-006E58E1, the bookkeeping of the window. can_fire is vtable[1D0h](1) (006E3460);
// owner_is_plane is owner vtable[5Ch](0Fh); gate is 007CC8E0 for a plane or owner vtable[1E8h]
// otherwise; flag_3b8 is the rack byte +3B8h. Returns true when the fire message ADh is routed
// (toRepeatTime then = RepeatTime).
bool rack_fire_window_step_006e5816(SingleRackState& rack, bool can_fire, bool owner_is_plane,
                                    bool gate, bool flag_3b8, float repeat_time) noexcept;

// ---------------------------------------------------------------------------------------------
// Multiple rack, MMultipleBombPlatform (constructor 006E42D0). The "display" rounds at +514h
// are entities the rack builds on its platform nodes (006E5AB0); a load message turns one into
// a real round (006E5D70), which is what the drop takes.
// ---------------------------------------------------------------------------------------------
struct MultiRackState {
    SingleRackState base;
    std::array<bool, kMultiRackSlots> display_round_514{};  // +514h, entity pointer non-null
    std::array<float, kMultiRackSlots> slot_timer_5b4{};    // +5B4h, -99 idle
    std::array<bool, kMultiRackSlots> slot_loaded_654{};    // +654h
    int display_rounds_67c{0};                              // +67Ch
    float load_timer_680{0.0f};                             // +680h
    int ammo_684{0};                                        // +684h
};

// 006E42D0: 006E3C00, then +680h = 0, +67Ch = 0, every slot timer -99, every flag 0.
MultiRackState multi_rack_construct_006e42d0() noexcept;
// 006E3770, slot +1BCh: +484h = +488h = +684h = n.
void multi_rack_set_ammo_006e3770(MultiRackState& rack, int n) noexcept;
// 006E35C0, slot +20Ch (the single rack's IsEmpty slot): any slot timer >= 0.
bool multi_rack_any_slot_reloading_006e35c0(const MultiRackState& rack) noexcept;
// 006E35F0, slot +208h: the smallest positive slot timer, 0 when none is positive.
float multi_rack_min_reload_left_006e35f0(const MultiRackState& rack) noexcept;
// 006E3730, slot +21Ch: real rounds, plus the display rounds when there are any and either a
// real round exists or vtable[210h](kind, 0) answers (has_round_of_kind, 006E45E0).
int multi_rack_count_rounds_006e3730(const MultiRackState& rack, int real_rounds,
                                     bool has_round_of_kind) noexcept;
// 006E45E0, slot +210h: 006E3FE0, else the bullet class of the first display round (through
// its +188h and 00472520) asked vtable[8](kind). first_display_class_matches is that answer.
bool multi_rack_has_round_006e45e0(const MultiRackState& rack, bool base_has_round,
                                   bool first_display_class_matches) noexcept;
// 006E3700, slot +214h: (float)vtable[21Ch](2Ah) / orgAmmo, no zero guard (x87 masked: +inf or
// NaN with orgAmmo 0).
float multi_rack_ammo_fraction_006e3700(const MultiRackState& rack, int round_count) noexcept;
// 006E3790: kills the display round in `slot`, or with slot < 0 in the first occupied slot.
// Returns the slot killed, or -1. Runs only with +67Ch > 0.
int multi_rack_kill_display_round_006e3790(MultiRackState& rack, int slot) noexcept;
// 006E37F0, slot +1F4h: flag[slot] = 0, timer[slot] = ReloadTime; with slot < 0 the first slot
// whose timer is negative. Returns the slot written, or nullopt when slot < 0 found none (the
// image then writes index `slot` itself, before the arrays: not modelled).
std::optional<int> multi_rack_reset_slot_timer_006e37f0(MultiRackState& rack, int slot) noexcept;
// 006E3840 / 006E3870 (the observer at rack+10h, slots +4h/+8h of 00CF9900): a display round
// died; its slot is cleared and +67Ch decremented. `slot` is the entry that held it.
void multi_rack_display_round_died_006e3840(MultiRackState& rack, int slot) noexcept;
// 006E4410, slot +200h (the B2h arm of 006E4290): the slots below orgAmmo with the flag clear
// and no display round, each of which gets a B3h message (0077C2A0 with arguments 2, 0).
std::vector<int> multi_rack_request_loads_006e4410(const MultiRackState& rack) noexcept;
// 006E44C0, slot +204h: each slot timer >= 0 counts down by dt x [00E08178]; on expiry the
// timer goes to -99 and +684h += 1; modes 0/1 then route B2h for that slot and return (the
// returned slot), mode 2 sets the slot's flag and goes on.
std::optional<int> multi_rack_advance_reload_006e44c0(MultiRackState& rack, float dt,
                                                      float reload_mul,
                                                      SessionMode mode) noexcept;
// 006E4690 (mode 2 every tick, and the B1h arm): kill display rounds while
// max(+684h - real, 0) < +67Ch, then mark loaded the empty slots whose timer is not positive until
// +67Ch + real slots are counted. Returns the killed slots in order. The image spins (reads
// +510h, index -1) if +67Ch > 0 with no occupied slot; the model stops instead.
std::vector<int> multi_rack_trim_006e4690(MultiRackState& rack, int real_rounds) noexcept;
// 006E5AB0, slot +224h: with +5Dh clear, over the class's platforms (class+188h) while ammo
// (+484h) > 0, every empty slot gets a display round; `create` is the entity create (00473BC0
// vtable[8]) and may fail. Then +680h = 0.5.
template <typename Create>
void multi_rack_fill_display_rounds_006e5ab0(MultiRackState& rack, bool rack_flag_5d,
                                             int platform_count, Create&& create) {
    if (rack_flag_5d) return;                                   // 006E5AB9
    for (int i = 0; i < platform_count; ++i) {                  // 006E5D40
        if (rack.base.ammo_484 <= 0) break;                     // 006E5BB4
        if (rack.display_round_514[static_cast<std::size_t>(i)]) continue;  // 006E5BC1
        if (!create(i)) continue;                               // 006E5C49
        rack.display_round_514[static_cast<std::size_t>(i)] = true;
        rack.base.ammo_484 -= 1;                                // 006E5D22
        rack.display_rounds_67c += 1;                           // 006E5D29
    }
    rack.load_timer_680 = kMultiRackFirstLoadDelay;             // 006E5D51
}
// 006E5D70, the B2h/B3h arm of 006E5F10: with +5Dh..+60h clear, kills the slot's display round,
// +680h = 1.2, flag[slot] = 1, timer[slot] = -99. Returns true when modes 0/1 also spawn the
// real round first (the host's create, with +150h = slot).
bool multi_rack_generate_real_round_006e5d70(MultiRackState& rack, int slot, bool torn_down,
                                             SessionMode mode) noexcept;
// 006E5900 (ESI = rack+310h) before the base tick 006E56F0: in modes 0/1, the load timer below
// 100 counts down; on expiry the first occupied slot gets a B3h (returned), or with none +67Ch = 0
// and the timer parks at 999. Mode 2 runs the trim instead (`trimmed`). Then toRepeatTime = -1.
struct MultiRackPreTick {
    std::optional<int> load_slot;
    std::vector<int> trimmed;
    bool clear_drop_after_tick{false};  // dt > 0 with dropBombs set: cleared after 006E56F0
};
MultiRackPreTick multi_rack_pre_tick_006e5900(MultiRackState& rack, float dt, SessionMode mode,
                                              int real_rounds) noexcept;
// 006E5F10, slot +164h, the message arms the multi rack adds before 006E4290: ADh with display
// rounds and at most three real rounds sets +680h = -1 (load at the next tick); B1h decrements
// +684h and trims (with display rounds), then resets the timer of the dropped round's slot.
void multi_rack_on_fire_message_006e5f54(MultiRackState& rack, int real_rounds) noexcept;
std::vector<int> multi_rack_on_drop_message_006e5f86(MultiRackState& rack, int real_rounds,
                                                     int dropped_slot) noexcept;

// ---------------------------------------------------------------------------------------------
// Projectile record leaves.
// ---------------------------------------------------------------------------------------------
struct Seg33Vec3 {
    float x{0.0f}, y{0.0f}, z{0.0f};
};
// 00CE3820 (a double, 1e-10): the squared-speed floor of the speed getters. Some headers carry
// 1.0e-11 for this address (bsp/ai_planners.hpp, bsp/ai_planner_tails.hpp); the bytes
// BB BD D7 D9 DF 7C DB 3D are 1e-10.
inline constexpr double kProjectileSpeedFloorSquared = 1.0e-10;
// 006E7CD0 (bullet, velocity +178h, slot +38h) and 006E2890 (bomb family, velocity +318h): the
// squared length (x*x + y*y) + z*z in double, FSTP float, FCOMI against the floor / JBE, then
// sqrt FSTP float; 0 at or below the floor (and for NaN).
float projectile_speed_006e7cd0(const Seg33Vec3& velocity) noexcept;
// 006E2220, __thiscall(record, t), RET 4: +A4h.. = +448h.. + float(t x velocity +318h..), each
// product and sum FSTP float. The pose invalidation of the child nodes is not modelled.
Seg33Vec3 projectile_extrapolate_position_006e2220(const Seg33Vec3& base,
                                                   const Seg33Vec3& velocity, float t) noexcept;

// ---------------------------------------------------------------------------------------------
// Command classes (cc13-gpr33b): vtable slot +10h and the menu sort of 006F9110.
// ---------------------------------------------------------------------------------------------
// Slot +10h of the 26 command-class vtables (bsp::kEntityOrderCommandClasses in
// bsp/entity_orders.hpp carries slots +4h/+8h/+0Ch). Every body is XOR EAX,EAX / RET except
// closetoship's 006F8E80 (MOV EAX,1 / RET); stop, retreat and returntobase keep the base body
// 006F7F50. Indexed by ordinal - 1 (registration order).
inline constexpr int kCommandClassSlot10[26] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // settarget .. dogfight
    0, 0, 0, 0, 0, 0, 0, 0, 0,              // follow .. land
    1,                                      // closetoship (23)
    0, 0, 0,                                // Leave, disband, tutorial
};
// 006F7B30, __fastcall(a /*ECX*/, b /*EDX*/): a->vtable[10h]() - b->vtable[10h]().
int command_class_compare_006f7b30(int slot10_a, int slot10_b) noexcept;
// 006F7C20, __thiscall(list, comparator), RET 4, called by 006F9110 with 006F7B30 on the
// collected menu list. For each cursor from the head: scan the later nodes, replacing the best
// (starting at the cursor) when compare(best, scan) == -1 (006F7C4A CMP EAX,-1 / CMOVE). When the
// best is not the cursor, the best is moved to just after the cursor and the cursor to the tail;
// the next cursor is the best's successor. Stops when the cursor is the tail. Returns the
// resulting order as indices into `keys` (the slot +10h values of the collected list).
std::vector<std::size_t> command_menu_sort_006f7c20(const std::vector<int>& keys);

// ---------------------------------------------------------------------------------------------
// Catapult launch timeline (MCatapult, class 28h, vtable 00CFAAB8), cc13-gpr33b.
// 006ED1C0 is slot +8h of the tick-element table 00CFAA70 (ESI = catapult+310h). Offsets below
// are on the catapult record. The field names +480h..+490h are the debug dump's (006EC140,
// bsp/air_operations.hpp CatapultInstanceOffsets); the rest are hypotheses.
// ADOPTION: no host launches from a catapult (GetLastCatapulted answers -1,
// src/game_hosts_lua.cpp:6197). Inputs: the global tick [00F876B0], the launch heading 006ED000
// (gun-arc geometry, not modelled), the class's +88h/+8Ch/+DCh/+E4h, the reload timer
// [catapult+414h]. RNG: none. OFF prediction: a ship that launches a floatplane would hold it 1 s,
// turn to the launch heading until 2 s, raise to the launch pitch and, from `turn_time`, ramp the
// launch speed 10 + 70 (t - turn_time) m/s, releasing the plane at turn_time + 0.5 s (about 45 m/s
// along its forward axis), then return to rest after turn_time + 1 s.
// ---------------------------------------------------------------------------------------------
inline constexpr float kCatapultSecondsPerTick = 0.05f;     // 00D0DE84
inline constexpr double kCatapultTurnSlack = 1.0;           // 00D7A210
inline constexpr float kCatapultTurnClampLo = 1.0f;         // 00D7A24C
inline constexpr float kCatapultTurnClampHi = 4.0f;         // 00CE3D34
inline constexpr double kCatapultTurnExtra = 1.5;           // 00CE3D78
inline constexpr float kCatapultHoldEnd = 1.0f;             // 00D7A24C, 006ED426
inline constexpr float kCatapultRotateEnd = 2.0f;           // 00CE3958, 006ED462
inline constexpr double kCatapultReleaseAfterTurn = 0.5;    // 00D7A280, 006ED4B0
inline constexpr double kCatapultSpeedGain = 70.0;          // 00CFACE0, 006ED50E
inline constexpr double kCatapultSpeedBase = 10.0;          // 00CE3DC0, 006ED514
inline constexpr float kCatapultSpeedAfterRelease = -4.0f;  // 00CF1430, 006ED5FE

struct CatapultLaunchState {
    int ammo_480{0};                 // "ammo"
    bool start_launch_488{false};    // "startLaunch"
    bool added_launched_489{false};  // VEHICLE_ADD_LAUNCHED sent
    int launch_tick_490{-1};         // "launchTick", -1 idle
    bool has_plane_4a8{false};       // the launched plane (observer pair at +494h, entity +4A8h)
    float yaw_4ac{0.0f};             // current angles (+4ACh, +4B0h)
    float pitch_4b0{0.0f};
    float yaw_target_4c4{0.0f};      // targets the turn step 006EBD90 follows
    float pitch_target_4c8{0.0f};
    float field_4cc{0.0f};           // set to class+E4h while holding; the end test reads it
    float launch_speed_4d4{0.0f};
};
struct CatapultClassView {
    float yaw_rate_88{0.0f};
    float pitch_rate_8c{0.0f};
    float launch_pitch_dc{0.0f};
    float rest_e4{0.0f};
};
// 006ED1C0-006ED261, the head after BSP_Gun_FixedStepTick: with startLaunch and dt > 0,
// can_fire (vtable[1D0h](1), 006EBD30) routes the fire message ADh (returns true); otherwise a
// disabled catapult (+3B8h) or ammo below 1 clears startLaunch.
bool catapult_fire_head_006ed1c0(CatapultLaunchState& s, float dt, bool can_fire,
                                 bool disabled_3b8) noexcept;
// 006ED2C1-006ED3B6: the turn time, clamp(max(|launch_pitch|/pitch_rate + 1, |yaw_delta|/yaw_rate),
// 1, 4) + 1.5; a non-positive rate gives 0 for its term.
float catapult_turn_time_006ed2c1(const CatapultClassView& cls, float yaw_delta) noexcept;
struct CatapultTickEvents {
    bool route_add_launched{false};  // VEHICLE_ADD_LAUNCHED (006EB9C0) routed (7, 0)
    bool takeoff{false};             // plane velocity set along its forward axis, PLANE_TAKEOFF(1)
    float takeoff_speed{0.0f};       // the +4D4h value used for it
    bool ended{false};               // launchTick back to -1
    float elapsed{0.0f};
    float turn_time{0.0f};
};
// 006ED261-006ED675, with launchTick >= 0. launch_heading is 006ED000's answer; the wrapped
// subtraction and addition are 00438B10 / 00438AA0 (bsp/unit_rudder.hpp).
CatapultTickEvents catapult_launch_timeline_006ed261(CatapultLaunchState& s,
                                                     const CatapultClassView& cls,
                                                     int global_tick, float launch_heading,
                                                     float reload_timer_414, bool plane_has_body,
                                                     SessionMode mode) noexcept;

// 006F5960 (command building): IMUL of the int capture range +7A0h by itself (32-bit wrap), FILD.
float command_building_capture_range_squared_006f5960(std::int32_t capture_range_7a0) noexcept;

// ---------------------------------------------------------------------------------------------
// Class fields read by 006E01C0 (BombPlatform, after the gun reader 007327B0) and 006E02C0
// (MultiBombPlatform, after 006E01C0).
// ---------------------------------------------------------------------------------------------
struct BombPlatformClassFields {
    float launch_speed_dc{0.0f};   // "LaunchSpeed" (GetNumber, no default)
    float repeat_time_e0{kBombPlatformRepeatTimeDefault};  // "RepeatTime"
    bool hide_ammo_e4{false};      // "HideAmmo", false when nil
    std::vector<int> platforms_e8; // "Platforms"[i].platform; count at +188h, no bound check
};

}  // namespace bsp::gameplay_seg33_cc13
