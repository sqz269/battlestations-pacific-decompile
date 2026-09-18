#ifndef BSP_TORPEDO_APPROACH_UPDATE_HPP
#define BSP_TORPEDO_APPROACH_UPDATE_HPP

// The torpedo approach object and its per-tick update 009D3420, the routine
// that produces every input of the torpedo state machine in
// docs/TORPEDO_TASK_ARM.md. docs/TORPEDO_APPROACH_UPDATE.md carries the
// evidence; docs/BOT_TASKS.md owns the task object and the pilot control block.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout: the offset constants in the comments are the native
// ones, the structs are not.
//
// The approach object is embedded in the torpedo task at task+3F8h, so every
// approach offset below is a task offset minus 3F8h: approach+8Ch is task+484h,
// approach+90h is task+488h, approach+131h is task+529h, approach+132h is
// task+52Ah, approach+CCh is task+4C4h.
//
// Contracts named but not reconstructed: 009FADA0 (the approach sub-object at
// approach+B4h, shared by all ten approach classes), 007DF360 (the
// target-reachability test), 00903BC0 (the segment-blocked test), 0041BC20 (the
// terrain height sampler), 007BCC80 and 007BCFA0 (the torpedo fall time and run
// speed), 009FA3A0 and 007F0280 and 00427EB0 inside the aim tick.

#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, each read from the address the listing names.
// ---------------------------------------------------------------------------

// 009D3499 FLD double [00D7A220]: the second-altitude ceiling.
inline constexpr float kTorpedoSecondAltitudeCeiling_00d7a220 = 100.0f;
// 009D3542 FLD double [00CE3820]: the squared-range epsilon below which the
// 2D range collapses to zero rather than taking a square root.
inline constexpr double kTorpedoRangeEpsilonSquared_00ce3820 = 1e-10;
// 009D3593 FSUBR double [00CE3830] and 009D35A7 FADD double [00CE3828].
inline constexpr float kHalfPi_00ce3830 = 1.5707963705062866f;
inline constexpr float kTwoPi_00ce3828 = 6.2831854820251465f;
// 009D3605 FMUL double [00CE3DF0]: the in-range latch releases at 1.1 times the
// engage distance it latched at.
inline constexpr float kTorpedoRangeHysteresis_00ce3df0 = 1.1f;
// 009D36AE FLD float [00CE38B8]: ground higher than this under the aircraft
// raises the over-land flag.
inline constexpr float kTorpedoOverLandHeight_00ce38b8 = 10.0f;

// The sector scan, 009D3798-009D3A2A.
inline constexpr int kTorpedoSectorCount = 36;            // 009D3A2A CMP 0x24
inline constexpr float kTorpedoScanStartRadius_00d2143c = 420.0f;
inline constexpr float kTorpedoScanSlopeLimit_00cf0a2c = 0.06f;
inline constexpr float kTorpedoScanSlopeStep_00d1fbbc = 0.025f;
inline constexpr float kTorpedoScanSlopeStepReset_00d21438 = 0.015f;
inline constexpr double kTorpedoScanSlopeStepGrowth_00d7a2f8 = 0.02;
inline constexpr double kTorpedoScanRadiusStep_00d1f3f8 = 120.0;
inline constexpr double kTorpedoScanRadiusShrink_00cf1440 = 80.0;
inline constexpr double kTorpedoScanRadiusFloor_00ce3938 = 50.0;
inline constexpr double kTorpedoScanLosRadius_00d1f4c0 = 450.0;
inline constexpr double kTorpedoScanSlopeBase_00ce3d90 = 400.0;
inline constexpr double kTorpedoScanRangeFactor_00cec160 = 1.2;
inline constexpr double kTorpedoScanSeaLevel_00ce3d88 = 20.0;
inline constexpr float kTorpedoScanSeaFloor_00ce3930 = 20.0f;
inline constexpr double kPi_00ce3d28 = 3.1415927410125732;
inline constexpr double kTorpedoSectorsPerHalfTurn_00cee930 = 18.0;
inline constexpr double kTorpedoSectorsPerHalfTurnNeg_00d21430 = -18.0;
// 009D4AE7 MOVSS [00CF87D0]: the cached plan position 009D4A70 parks so that
// the next 009D3420 tick always replans.
inline constexpr float kTorpedoPlanPositionReset_00cf87d0 = 999999.0f;

// 009D324F FMUL double [00D05AC8]: the engaged predicate 009D3210 admits a
// range up to 2.2 times the engage distance.
inline constexpr float kTorpedoEngageRangeScale_00d05ac8 = 2.2f;

// The speed switch and the engagement estimate, 009D3C93-009D3D6D.
inline constexpr float kTorpedoSpeedSwitchSeconds_00cf3f20 = 15.0f;
inline constexpr float kTorpedoEtaCeiling_00ce7630 = 30.0f;
inline constexpr float kTorpedoEtaCeilingValue_00ce38c8 = 30.0f;

// 009D1500, the time-to-target metric.
inline constexpr float kTorpedoCruiseSpeedFloor_00ce4bc4 = 600.0f;
inline constexpr double kTorpedoCruiseSpeedRate_00d20198 = 600.0;
inline constexpr double kTorpedoCruiseSpeedFirstSecond_00d7a210 = 1.0;

// 009D1360, the torpedo run-time estimate.
inline constexpr float kTorpedoDecelRate_00cf1440 = 80.0f;
inline constexpr float kTorpedoHalf_00d7a280 = 0.5f;

// 009D4AC4, the attack-distance clamp on the +54h cruise profile.
// Pilot/Torpedo/AttackDist, tuning singleton +434h, default 2200 in
// docs/GAME_TUNING_SINGLETON.md; Pilot/Torpedo/CruisingAlt is +430h = 500.
inline constexpr float kPilotTorpedoAttackDistDefault = 2200.0f;
inline constexpr float kPilotTorpedoCruisingAltDefault = 500.0f;
// 009D4AA9 MOVSS [00CE4C04], written into the pilot control block +39Ch: the
// speed ceiling the approach update clamps approach+80h against.
inline constexpr float kPilotTorpedoSpeedCeiling_00ce4c04 = 9999.0f;

// ---------------------------------------------------------------------------
// The approach object. Field names are hypotheses; the trailing hex is the
// native offset from the approach base (task offset minus 3F8h).
// ---------------------------------------------------------------------------
struct TorpedoApproachState {
    // The direction plan, rebuilt on the replan timer. sector_clear[i] is 1
    // when a run-in along sector i is free of terrain; index 36 (+54h) is a
    // wrap copy of index 0 that 009D39FD writes.
    unsigned char sector_clear_30[kTorpedoSectorCount + 1]{};  // +30h..+54h
    int sector_clear_count_58{0};                              // +58h
    float turn_offset_5c{0.0f};                                // +5Ch, radians
    float aim_heading_60{0.0f};                                // +60h, aim tick
    int home_sector_64{0};                                     // +64h
    int forward_gap_68{0};                                     // +68h
    int backward_gap_6c{0};                                    // +6Ch

    // The closing-speed bias the engagement estimate divides by. Read only.
    float closing_speed_bias_70{0.0f};                         // +70h

    // The altitude band the arm hands to the moveto state (009D48AC uses
    // alt_floor_74 + alt_margin_78 for both of the range arguments).
    float alt_floor_74{0.0f};                                  // +74h
    float alt_margin_78{0.0f};                                 // +78h

    // CORRECTED by packet cc8_torpedo_run_profile: these are RELEASE DISTANCES
    // in metres, not speeds. The names are kept because they are load bearing in
    // src/torpedo_aim_tick.cpp and src/game_hosts_units.cpp, and renaming the
    // pair reaches past this packet's lease; the rename is a follow-up.
    //
    // 009F9CFF-009F9D22 in BSP_BotApproach_ConstructSpeedReference sets
    // approach+14h to &PilotBotConfig.levels[idx]:
    //
    //   009f9d08  mov  edx, [eax+0DF4h]      the bot object on the unit
    //   009f9d0e  mov  edx, [edx+34h]        its difficulty level index
    //   009f9d11  imul edx, edx, 248h        sizeof(PilotBotParameters)
    //   009f9d18  mov  esi, [00F8A30C]       the PilotBotConfig
    //   009f9d1e  lea  edx, [edx+esi+0Ch]    offsetof(PilotBotConfig, levels)
    //
    // Both constants are checked against include/bsp/robot_config.hpp, which
    // already carries static_assert(sizeof(PilotBotParameters) == 0x248) and
    // static_assert(offsetof(PilotBotConfig, levels) == 0x0c).
    //
    // 009D0484-009D0497 then reads that row, and src/robot_config.cpp names the
    // three fields from the Lua keys the loader pushes at 00997B7A, 00997BB0 and
    // 00997BE6:
    //
    //   record+0h = TorpReleaseAlt       -> scales approach+78h at 009D046A
    //   record+4h = TorpReleaseDistNear  -> +7Ch, used once elapsed_134 >= 15 s
    //   record+8h = TorpReleaseDistFar   -> +80h, used before that
    //
    // both multiplied by approach+24h, which 009F9D30-009F9D61 sets to
    // max(1.0, desc.MaxSpd / Pilot/Torpedo/ReferenceSpeed). desc+188h is MaxSpd
    // by the Lua key at 007D23A1, and tuning+440h is Pilot/Torpedo/ReferenceSpeed
    // in src/game_tuning_singleton.cpp, default KMH(300). A faster aircraft gets
    // a longer release distance, which is what that ratio is for.
    //
    // The consequence: 009D1500 divides the range by one of these, so it is a
    // RANGE RATIO and not a time to target, which is why the aim tick's clause 2
    // compares it against a steering delta in radians without a unit error.
    // docs/TORPEDO_RUN_PROFILE.md.
    float speed_late_7c{0.0f};                                 // +7Ch
    float speed_early_80{0.0f};                                // +80h

    float scan_radius_seed_88{0.0f};                           // +88h, read only
    // The engage distance. 009D3420 only reads it; 009D4AC4 (the +54h cruise
    // profile) writes it as max(current, AttackDist * speed_ratio).
    float engage_range_8c{0.0f};                               // +8Ch

    float range_90{0.0f};    // +90h, 2D range from the unit to the target point
    float bearing_94{0.0f};  // +94h, compass bearing to the target point

    float run_time_98{0.0f};       // +98h, written by 009D1360
    float run_time_bias_9c{0.0f};  // +9Ch, read by 009D1360
    float fall_lead_a0{0.0f};      // +A0h, written by 009D1360

    int weapon_selector_a4{0};             // +A4h
    unsigned char scan_enabled_a8{0};      // +A8h, read only
    unsigned char over_land_a9{0};         // +A9h
    unsigned char no_clear_sector_aa{0};   // +AAh

    float plan_target_x_ac{kTorpedoPlanPositionReset_00cf87d0};  // +ACh
    float plan_target_z_b0{kTorpedoPlanPositionReset_00cf87d0};  // +B0h

    float eta_f8{0.0f};            // +F8h, clamped to [0, 30]
    float replan_period_128{0.0f}; // +128h
    float replan_timer_12c{0.0f};  // +12Ch
    bool aim_solution_130{false};  // +130h, written by the aim tick 009D2021
    bool in_range_latch_131{false};// +131h == task+529h, attackrun -> aim
    bool has_ordnance_132{false};  // +132h == task+52Ah, aim -> goaway
    float elapsed_134{0.0f};       // +134h, seconds since 009D0380 reset it
};

// The fields of the pilot control block (unit+9D4h, approach+0Ch) that 009D3420
// reads. docs/BOT_TASKS.md owns the block.
struct TorpedoApproachControl {
    float second_altitude_398{0.0f};   // +398h, source of alt_floor_74
    float speed_ceiling_39c{0.0f};     // +39Ch, the clamp on speed_early_80
    unsigned char profile_dirty_3ad{0};// +3ADh, zero clears alt_margin_78
    unsigned char always_engage_369{0};// +369h, with the global at 00E17BF2
    int attack_mode_370{0};            // +370h
    bool has_terrain_34c{false};       // +34Ch != 0, the terrain sampler
    // +3D0h is an ARRAY of the units the block drives, count +3CCh, walked at
    // 007EEF5C with LEA/ADD 4; 009D3D88 loads its FIRST element, the flight
    // leader. The two names below are kept for src/game_hosts_units.cpp, which
    // another agent holds; they mean "the block drives at least one unit" and
    // "this approach's unit IS that leader". See the Corrections section of
    // docs/TORPEDO_RELEASE_ORDERS.md.
    bool has_designated_target_3d0{false};   // ctl->+3CCh > 0
    bool designated_target_is_self{false};   // ctl->+3D0h[0] == approach+4h
};

// ---------------------------------------------------------------------------
// Pure rules. Each one carries the address of the listing it came from.
// ---------------------------------------------------------------------------

// 009D3433: the pilot control block's dirty flag clears the altitude margin.
// 009D3445-009D3485: clamp speed_early_80 to the block's ceiling and carry
// speed_late_7c along at the same ratio. 009D3489-009D34AE: take the block's
// second altitude when it is under 100.
void torpedo_approach_apply_profile_009d3433(TorpedoApproachState& s,
                                             const TorpedoApproachControl& ctl) noexcept;

// 009D3519-009D3563: the 2D range, with the epsilon that collapses it to zero.
float torpedo_approach_range_009d3519(const float unit_xz[2],
                                      const float target_xz[2]) noexcept;

// 009D3586-009D35B1: the compass bearing, pi/2 minus atan2 wrapped into
// [0, 2*pi).
float torpedo_approach_bearing_009d3586(const float unit_xz[2],
                                        const float target_xz[2]) noexcept;

// 009D35D4-009D361E: the in-range latch. It closes below engage_range_8c and
// opens at 1.1 times it, and never opens while always_engage holds.
bool torpedo_in_range_latch_009d361e(bool latched, float range,
                                     float engage_range, bool always_engage) noexcept;

// 009D3C93-009D3D6D: the engagement estimate, clamped to [0, 30] seconds.
float torpedo_engagement_eta_009d3c93(const TorpedoApproachState& s,
                                      float unit_speed) noexcept;

// 009D3C99, 009D1509, 009D4874: the one speed switch all three share.
float torpedo_commanded_speed_009d3c99(float elapsed_134, float speed_late_7c,
                                       float speed_early_80) noexcept;

// The pair the two speed slots are seeded from, and where they come from.
struct TorpedoRunSpeeds {
    float speed_late_7c{0.0f};   // +7Ch
    float speed_early_80{0.0f};  // +80h
};

// 009D0484-009D0497, inside BSP_BotApproachTorpedo_Reset (009D0380-009D066F).
// The seed of both speed slots, and the producer this packet had to find:
//
//   009d046c  mov  eax, [esi+14h]      ; the run profile record
//   009d047d  fld  dword ptr [esi+24h] ; the scale
//   009d0484  fld  dword ptr [eax+4]
//   009d048d  fmulp st(2)
//   009d0491  fstp dword ptr [esi+7Ch] ; +7Ch = record[+4] * scale
//   009d0494  fmul dword ptr [eax+8]
//   009d0497  fstp dword ptr [esi+80h] ; +80h = record[+8] * scale
//
// So both are SPEEDS drawn from the run profile at approach+14h, scaled by
// approach+24h. Neither +14h nor +24h is written anywhere in 009D0380; both
// arrive already set. 009D05ED and 009D0625 then jitter the pair through two
// 00BD2F10 draws, and 009D3445 clamps +80h to the control block's ceiling
// 00CE4C04, which is 9999.0 and therefore never bites.
//
// This matters because 009D1500 divides the range by whichever of the two the
// 15-second switch selects, and the aim tick's clause 2 at 009D22D9 compares
// that time against the steering delta in radians. A speed slot that is too
// large shortens the time and breaks the run off early.
TorpedoRunSpeeds torpedo_seed_run_speeds_009d0484(float profile_late_4,
                                                  float profile_early_8,
                                                  float scale_24) noexcept;

// 009D1500: the time-to-target metric the done/prepare tick and the aim tick
// both read. The first second covers `speed` units, every second after it
// covers 600.
float torpedo_time_to_target_009d1500(const TorpedoApproachState& s) noexcept;

// 009D1360, the committed hook. It writes run_time_98 and fall_lead_a0 from the
// aircraft's altitude and speed: the torpedo falls for fall_time seconds while
// carrying the aircraft's speed, decelerates to run_speed at 80 per second, and
// covers the remainder at run_speed.
struct TorpedoRunTimeInputs {
    float unit_speed{0.0f};   // unit->vtable[38h] at 009D1379
    float fall_time{0.0f};    // 007BCC80(unit altitude) at 009D138F
    float run_speed{0.0f};    // 007BCFA0() at 009D13A1
};
struct TorpedoRunTimeResult {
    float run_time_98{0.0f};
    float fall_lead_a0{0.0f};
};
TorpedoRunTimeResult torpedo_run_time_009d1360(const TorpedoApproachState& s,
                                               const TorpedoRunTimeInputs& in) noexcept;

// 009D4AC4: the +54h cruise profile's attack-distance clamp, the one producer
// of engage_range_8c in the image.
float torpedo_engage_range_009d4ac4(float current_8c, float attack_dist_tuning,
                                    float speed_ratio_41c) noexcept;

// 009D38A6-009D38D8: the bearing at the centre of sector i, wrapped into
// [0, 2*pi).
float torpedo_sector_bearing_009d38a6(int sector) noexcept;

// 009D3BAF-009D3BC8: the sector the unit currently sits in, as seen from the
// target, clamped to the last sector.
int torpedo_home_sector_009d3bc8(float bearing_target_to_unit) noexcept;

// 009D3BCE-009D3C56: walk out from the home sector to the nearest clear one in
// each direction and take the cheaper turn. The forward gap divides by -18 and
// the backward gap by +18, so the result is a signed offset in radians.
struct TorpedoTurnPlan {
    float turn_offset_5c{0.0f};
    int forward_gap_68{0};
    int backward_gap_6c{0};
    bool no_clear_sector_aa{false};
};
TorpedoTurnPlan torpedo_turn_plan_009d3bce(const unsigned char sector_clear[],
                                           int clear_count, int home_sector) noexcept;

// ---------------------------------------------------------------------------
// The host. One method per native call site 009D3420 makes.
// ---------------------------------------------------------------------------
struct TorpedoApproachHost {
    TorpedoApproachHost() = default;
    virtual ~TorpedoApproachHost() = default;
    TorpedoApproachHost(const TorpedoApproachHost&) = delete;
    TorpedoApproachHost& operator=(const TorpedoApproachHost&) = delete;

    // 009D34BB, 009FADA0(approach+B4h, dt): the sub-object every approach class
    // ticks first. contract: unread.
    virtual void tick_approach_subobject_009fada0(float dt) = 0;

    // 009D34C5, 007B93F0(ECX = approach+4h, 0): does the aircraft still carry
    // torpedo ordnance (kind 2Bh)?
    virtual bool unit_has_torpedo_ordnance_007b93f0() = 0;

    // 009D34DE, 009D363B, 009D3DA4: 00414DB0 on an entity whose +C8h is clear,
    // then its world x and z out of +FCh and +104h.
    virtual void unit_world_xz(float out_xz[2]) = 0;
    virtual bool target_world_xz(float out_xz[2]) = 0;

    // 009D3517, 009D36E4, 009D3DC8: approach->vtable[0](out), the target point.
    // False when the approach has no target (approach+CCh == 0).
    virtual bool approach_target_point(float out_point[3]) = 0;

    // 009D36A9 and the scan at 009D3960: 0041BC20(ctl+34Ch, x, z), the terrain
    // height sampler. contract: unread.
    virtual float terrain_height_0041bc20(float x, float z) = 0;

    // 009D39CB: 00903BC0(&target_point, &probe_point), the short-range blocked
    // test the scan uses inside 450 units. contract: unread.
    virtual bool segment_blocked_00903bc0(const float from[3], const float to[3]) = 0;

    // 009D36EE: 007DF360(ECX = unit+C50h, target, target_point). contract:
    // unread. Overwrites over_land_a9 whenever unit+C50h is set.
    virtual bool target_reachable_007df360(const float point[3]) = 0;

    // 009D3D01: unit->vtable[38h](), the aircraft's speed.
    virtual float unit_speed_vtable38() = 0;

    // The pilot control block fields, read fresh each tick.
    virtual TorpedoApproachControl read_control_block() = 0;
};

// The sequence routine. Returns what the tick decided, so a caller can report
// it without reading the state back.
struct TorpedoApproachUpdateResult {
    bool early_out_no_target{false};   // 009D3506 -> 009D3E21
    bool no_ordnance_path{false};      // 009D3624 -> 009D3D72
    bool replanned{false};             // the replan timer expired this tick
    bool scan_ran{false};              // the sector scan actually executed
    float range{0.0f};
    float bearing{0.0f};
    float eta{0.0f};
    bool in_range_latch{false};
    bool has_ordnance{false};
};

// 009D3420-009D3E3F. void __thiscall(TorpedoApproachState*, float dt), RET 4:
// the caller 009D486F pushes dt with FSTP [ESP] after PUSH ECX and the body
// ends ADD ESP,0x8c / RET 4 at 009D3E3C.
TorpedoApproachUpdateResult torpedo_approach_update_009d3420(
    TorpedoApproachHost& host, TorpedoApproachState& state, float dt);

// ---------------------------------------------------------------------------
// 009D15F0, the aim tick. coverage: partial - the heading and throttle
// producer and the six command-block writes are reconstructed; the four
// InterpolateClamped chains that fold the bank, the altitude and the lead into
// the throttle are transcribed only as far as their shape.
// ---------------------------------------------------------------------------
struct TorpedoAimTickInputs {
    float range_90{0.0f};
    float bearing_94{0.0f};
    float turn_offset_5c{0.0f};
    float time_to_target{0.0f};   // 009D160A, 009D1500(approach)
    float unit_heading{0.0f};     // 009D1678, unit->vtable[50h]
    float unit_altitude{0.0f};    // unit+100h
    float ground_height{0.0f};    // 009D16B4, 00903860(unit+FCh)
    float alt_floor_74{0.0f};
    float alt_margin_78{0.0f};
    bool over_land_a9{false};
    bool no_clear_sector_aa{false};
    bool has_ordnance_132{false};
    float commanded_speed{0.0f};  // the 009D3C99 switch on this approach
    float fall_lead_a0{0.0f};
};

// The command block is approach+18h. These are the writes 009D1D02-009D1F6B
// make, with the field offsets the attackrun tick uses.
struct TorpedoAimCommand {
    float commanded_heading_2c0{0.0f};   // 009D1D16
    int heading_mode_2cc{2};             // 009D1D1E, the constant 2
    float commanded_throttle_2c8{0.0f};  // 009D1D2E
    float commanded_altitude_2bc{0.0f};  // 009D1EDD
    int altitude_mode_2d0{1};            // 009D1EE5, the constant 1
    float roll_limit_2e8{0.0f};          // 009D1D02
    float flag_278{1.0f};                // 009D1F4A, the constant [00D7A24C]
    unsigned char flag_27c{1};           // 009D1F55
    float flag_2a8{0.0f};                // 009D1F5C, the constant 0
    unsigned char flag_2ac{1};           // 009D1F64
    int flag_2d8{0};                     // 009D1F6B
    float commanded_altitude_floor{0.0f};// the max() the tick folds into 2BCh
    bool aim_solution_130{false};        // 009D2021
};

// 009D15F0-009D2377, void __thiscall(state, float dt), RET 4 at 009D2377.
// The heading is the bearing to the target plus the sector turn offset the
// approach update chose, so the aim state flies the run-in 009D3420 planned.
TorpedoAimCommand torpedo_aim_tick_009d15f0(const TorpedoAimTickInputs& in) noexcept;

}  // namespace bsp

#endif  // BSP_TORPEDO_APPROACH_UPDATE_HPP
