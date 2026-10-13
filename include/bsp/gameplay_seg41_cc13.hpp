#pragma once

// Segment 41 (00735F30-00758EB0) reference models: the LandConvoy's damage
// pooling, the MLandFort / MCommandBuilding destruction and death-smoke rules,
// the MLandingShip's ramp anchor and small overrides, and the three Lua mission
// triggers' per-tick checks. Packet cc13-gpr41; docs/CC13_GPR_41.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw)
// and the Ghidra pseudocode, not native layouts or ABI-compatible replacements.
// Field names are hypotheses; the "_NNN" suffix is the native byte offset. Every
// engine service the image calls is a host callback, so nothing here invents a
// global or stubs a call. Build-tested only; not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_41.md section 2):
// - Replaces:
//   * convoy damage pooling (0074CD30, 00742330, 00742940): the hit path's
//     single AddDamage at src/game_hosts_gunnery.cpp:9797 (add_damage), which
//     applies 0095DA00 to every victim; MLandVehicle's vtable[1ACh] is 0074CD30.
//     The host keeps the convoy only as a name (game_hosts_units.cpp:11436).
//   * the land fort destruction rule 00747400 and death handler 00747170: the
//     generic kill at src/game_hosts_gunnery.cpp:11318 (kill_unit), which kills
//     every unit at health 0; MLandFort's vtable[1A8h] is 00747400.
//   * the death smoke 007465E0 and its frame arm 00745F1E: nothing; cc9's
//     src/game_hosts_ship_ai.cpp:12559 says "The +744h arm (00745F1E..) has no
//     producer here".
//   * the fort's command filter 00743F50 / 00744B30: the generic follow test at
//     src/game_hosts_script_orders.cpp:3046 (entity_may_follow_target_00779d50),
//     asked for every follower; MLandFort and MCommandBuilding answer through
//     vtable[16Ch] = 00744B30 instead.
//   * the landing ship anchor 0074A420 / 0074AC30: the ramp latch in
//     include/bsp/building_pads.hpp:204 sets only ramp_down_1188; the pose hold
//     has no host.
//   * the triggers 00750000 / 00750230 / 007504F0: nothing; mission_lua_host.cpp
//     :516-518 lists the three natives unbound.
// - Inputs: see each section; the host supplies poses, unit flags (+5Ch alive
//   gate, +5Dh dead, +5Eh, +60h), party (+54h), the class rows (+13Ch/+160h
//   effect definitions, +164h SmokeFireChanceMul, +168h/+16Ch fire duration
//   min/max), and the fixed-step clock [00F876A4].
// - RNG: one draw, through LandFortHost::uniform on STREAM 0 in image order:
//   007465E0's U(class+168h, class+16Ch) at 0074672B / 0074679D (ECX = 0).
//   It is gameplay (it sets the +748h timer the frame update consumes). The
//   master AddDamage(1e10) in 00747170 reaches the master's own vtable[1ACh]
//   (007470B0 for a land fort: a stream-1 roll and possibly stream-0 fire
//   draws; cc9 hosts that at game_hosts_ship_ai.cpp:12510); that is the
//   host's dispatch, not drawn here.
// - OFF-switch prediction: each model off changes nothing. On: see the doc's
//   table; the largest are (a) convoy members with a positive convoy +364h stop
//   taking their own hits (the lead member takes them), (b) a land fort section
//   with a master hands the master 1e10 damage on death, and a fort whose
//   +48h/+44h chain holds a live element does not die at health 0, (c) dead
//   forts with a death-smoke definition draw one stream-0 value each.

#include <cstdint>
#include <vector>

namespace bsp {
namespace gameplay_seg41_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// 1. LandConvoy damage pooling (class 1Ah, convoy vtable 00CEA570; member class
//    MLandVehicle 19h, vtable 00CFFDE0). Member +738h is the convoy
//    (land_and_structures.hpp kLandGroupOwnerOffset).
// ---------------------------------------------------------------------------
struct ConvoyState {
    float field_364{0.0f};            // > 0: hits pool on the lead member (meaning not named)
    float speed_368{0.0f};            // 00741140; Lua "Speed" (007416F0)
    std::int32_t field_344{0};        // 00741160 (Lua native 008959F0)
    bool reentry_390{false};          // set around the lead member's AddDamage
    std::vector<std::uint32_t> members_398;  // +398h..+39Ch, 0 = empty slot
    std::int32_t initial_size_3b8{0}; // Lua "InitialSize"
    float collected_damage_3c0{0.0f}; // Lua "CollectedDamage"
    float last_hit_3c4{0.0f};         // clock of the last hit; Lua "LastHit" = clock - this
};

class ConvoyHost {
public:
    virtual ~ConvoyHost() = default;
    // member->vtable[1ACh](amount); for an MLandVehicle that is 0074CD30, so a
    // host that binds both routes it back to add_damage_parent_aware_0074cd30.
    virtual void member_add_damage(std::uint32_t member, float amount) = 0;
    virtual void base_add_damage_0095da00(std::uint32_t unit, float amount) = 0;
    virtual float member_health_370(std::uint32_t member) = 0;      // raw +370h
    virtual float member_get_health_00923be0(std::uint32_t member) = 0;
    virtual void kill_members_00742210(int reason) = 0;             // reason 0
    virtual void base_set_invincible_0042ed80(bool on) = 0;
    virtual void member_set_invincible(std::uint32_t member, bool on) = 0;  // vtable[0F4h]
    virtual float fixed_step_clock() = 0;                            // [00F876A4]
    // 00BF6713: the checked-iterator failure the image raises on an empty roster.
    virtual void invalid_iterator_00bf6713() = 0;
};

void set_speed_00741140(ConvoyState& c, float speed);
void set_field_344_00741160(ConvoyState& c, std::int32_t value);
// MLandVehicle vtable[1ACh]. `convoy` is member+738h (nullptr outside a convoy).
void add_damage_parent_aware_0074cd30(ConvoyState* convoy, std::uint32_t unit, float amount,
                                      ConvoyHost& host);
void on_member_damaged_00742330(ConvoyState& c, float amount, ConvoyHost& host);
// Convoy vtable[110h].
float health_00742940(const ConvoyState& c, ConvoyHost& host);
// Convoy vtable[0F4h].
void set_invincible_00741f20(const ConvoyState& c, bool on, ConvoyHost& host);

// ---------------------------------------------------------------------------
// 2. MLandFort (class 1Bh, vtable 00CFF3F8) and MCommandBuilding (1Ch, 00CFB028,
//    which shares 00747170, 00743F50 and 00744B30).
// ---------------------------------------------------------------------------

// vtable[168h]: true for "settarget", "clearorders" and "cleartarget"
// (00CF8B0C, 00CF8AF4, 00CF8B00), compared case-insensitively (00438E10).
bool supports_command_00743f50(const char* command);

struct FollowTargetView {
    bool present{false};
    bool kind_05{false};       // IsKindOf(5), vtable[5Ch]
    bool kind_18{false};       // PlaneSquadronGen
    bool kind_1a{false};       // LandConvoy
    bool kind_08{false};       // MSubmarine
    bool dead_5d{false};
    std::int32_t party_54{0};
};

class FortCommandHost {
public:
    virtual ~FortCommandHost() = default;
    // 00467170 then the command object's vtable[8h]; called only without a target.
    virtual bool command_object_slot8(const char* command) = 0;
    virtual std::int32_t party_relative_to_00803510(std::int32_t self_party,
                                                    std::int32_t target_party) = 0;
    virtual bool target_above_depth_charge_depth_00852820() = 0;
};

// vtable[16Ch].
bool may_follow_target_00744b30(const char* command, std::int32_t self_party_54,
                                const FollowTargetView& target, FortCommandHost& host);

struct SetMasterPlan {
    bool adopt_master_party{false};      // this->vtable[2Ch](master+54h, master+58h, &0)
    float master_max_health_36c{0.0f};   // master+36Ch = this+36Ch
    float master_health_370{0.0f};       // 00877B90(master, this+370h)
};
// 00744A20 (callers 006D5220 airfield, 00849F70 shipyard). Stores +738h/+73Ch
// first; with a null master nothing else happens (`apply` is false).
struct SetMasterResult {
    bool apply{false};
    SetMasterPlan plan;
};
SetMasterResult set_master_00744a20(bool master_present, std::int32_t self_party_54,
                                    std::int32_t master_party_54, float self_max_health_36c,
                                    float self_health_370);

// vtable[1A8h], 00747400. Evaluated when the unit's health reaches zero.
enum class FortDieOutcome {
    already_dead,       // +5Dh set
    inferior_failure,   // +71Ch set: vtable[194h]("InferiorFailure", 0); no death
    not_eligible,       // +50h clear
    chain_alive,        // an element of the +48h/+44h chain has +5Dh clear
    die,                // vtable[1B4h](0072AB10 last level) then vtable[70h](1)
};
FortDieOutcome die_00747400(bool dead_5d, bool inferior_71c, bool field_50,
                            const std::vector<bool>& chain_dead_5d);

struct FortTimers {
    bool fire_740{false};      // burning effect (00746320, cc9's host)
    bool smoke_744{false};     // death smoke effect (007465E0)
    float time_748{0.0f};      // shared remaining time
    float clock_750{0.0f};     // secondary-explosion clock
};

struct FortClassRow {
    bool death_smoke_definition_160{false};
    float smoke_fire_chance_mul_164{0.0f};
    float fire_duration_min_168{0.0f};
    float fire_duration_max_16c{0.0f};
};

class LandFortHost {
public:
    virtual ~LandFortHost() = default;
    // 00BD2F10(stream, lo, hi). Stream 0 here, gameplay, in image order.
    virtual float uniform(int stream, float lo, float hi) = 0;
    virtual void stop_fire_effect_740() = 0;      // 00867B10, +9 = 1, release
    // 008689C0 with class+160h at the 00744C40 point, parented to +4A4h; true
    // when the effect handle came back non-null.
    virtual bool create_death_smoke_effect() = 0;
    virtual void send_effect_message_7(float duration) = 0;  // 007448B0(7, d), 0077C2A0(m, 4, 0)
    virtual void stop_smoke_effect_744() = 0;     // 00867B10, +9 = 1, 00484620(0)
};

// 007465E0 StartDeathFire.
void start_death_fire_007465e0(FortTimers& t, const FortClassRow& cls, LandFortHost& host);
// 00745F1E..00745F9D, the +744h arm of the frame update 00745BE0. It runs
// after the +740h arm every frame (that arm falls through at 00745F16).
void smoke_frame_00745f1e(FortTimers& t, float dt, LandFortHost& host);

class FortDestroyHost {
public:
    virtual ~FortDestroyHost() = default;
    virtual int last_damage_state_index_0072ab10(std::uint32_t unit) = 0;
    virtual void set_damaged_gfx_level(std::uint32_t unit, int level) = 0;  // vtable[1B4h]
    virtual void rebuild_part_render_list() = 0;   // +360h: +19Ch set, list +198h into +D0h/+F8h
    virtual void master_add_damage(float amount) = 0;  // master->vtable[1ACh]
    virtual bool master_die_gate() = 0;            // master +5Ch set, +5Dh/+60h/+5Eh clear
    virtual void master_die() = 0;                 // master->vtable[1A8h]
    // 008689C0 with class+13Ch at class+1Ch[0]+0Ch, parented to +4A4h.
    virtual void create_death_effect() = 0;
    virtual void start_death_fire() = 0;           // 007465E0
    virtual void base_on_destroyed_00959450() = 0;
};

struct FortDestroyInput {
    std::uint32_t self{0};
    std::int32_t state_70{0};
    float clock{0.0f};
    bool part_list_19c{false};
    bool master_738{false};
    float smoke_fire_chance_mul_164{0.0f};
    std::vector<std::uint32_t> chain_48;   // +48h, then each element's +44h
};
// vtable[7Ch], 00747170.
void on_destroyed_00747170(const FortDestroyInput& in, FortDestroyHost& host);

// ---------------------------------------------------------------------------
// 3. MLandingShip (vtable 00CFFA30; tick element at unit+310h, table 00CFF9EC).
// ---------------------------------------------------------------------------
struct LandingShipAnchor {
    bool ramp_down_1188{false};
    bool anchored_1189{false};
    Vec3 ramp_node_11b4;              // first +1198h record's node world position
    float pose_11c0[16]{};            // pose +74h captured at the lowering
};
// 0074A420 LowerRamp. `node_world` is the first +1198h record's node +120h
// (refreshed through 00B6DB70 when its +5Ch bit 2 is clear).
void lower_ramp_0074a420(LandingShipAnchor& a, const Vec3& node_world, const float pose_74[16]);

class LandingShipPoseHost {
public:
    virtual ~LandingShipPoseHost() = default;
    virtual void run_controller_update_00811ab0(float dt) = 0;
    virtual void set_pose_74(const float m[16]) = 0;  // 004134F0 into unit+74h
    virtual void clear_pose_valid_flags() = 0;        // unit+C8h = 0, unit+10Ch = 0
    virtual void invalidate_child_chain() = 0;        // 0042ED50 over +48h/+44h
};
// Tick slot +4h, 0074AC30 (ECX = unit+310h).
void place_step_pose_0074ac30(const LandingShipAnchor& a, float dt, LandingShipPoseHost& host);

// vtable[24Ch]: +1214h when `alternate`, else +1220h.
Vec3 get_ramp_point_0074bca0(const Vec3& point_1214, const Vec3& point_1220, bool alternate);
// vtable[244h]: class Length (+A0h) < 80.0 and speed (vtable[38h]) > 3.0.
bool small_and_moving_0074bbd0(float class_length_a0, float speed);
// vtable[240h]: false while on a pad (+1200h), else the base 008128E0(0).
bool slot240_00749810(bool on_pad_1200, bool base_008128e0);

// ---------------------------------------------------------------------------
// 4. Lua mission triggers (base 00D00554). Each check is the trigger's
//    vtable[0Ch](dt); a hit calls 0074FDD0 (the Lua callback), whose own
//    repeat/once handling is not modelled.
// ---------------------------------------------------------------------------
// 0074FEE0 / 007500E0: the stored radius is r * r (FMUL ST0,ST0, FSTP float).
float trigger_radius_squared(float radius);

// 00750000 (vtable 00D00580). `watched` is (+4)->+28h after the pose refresh;
// `target` is the descriptor +44h resolved by 006F7DD0. Planar: x and z.
bool proximity_check_00750000(const Vec3& watched, const Vec3& target, float radius_sq_5c);

struct LineOfSightTrigger {
    float range_sq_48{0.0f};
    float timer_4c{0.0f};      // 0 at construction
};
class LineOfSightHost {
public:
    virtual ~LineOfSightHost() = default;
    // 0084B6B0 from `from` to `to`, ignoring `ignore`; true with the first hit
    // entity in `hit`.
    virtual bool cast_0084b6b0(const Vec3& from, const Vec3& to, std::uint32_t ignore,
                               std::uint32_t& hit) = 0;
};
// 00750230 (vtable 00D005AC). `watched_ignore` is the watched entity, or its
// +3D0h when it IsKindOf(18h) (a squadron's lead). Returns true when it fires.
bool line_of_sight_check_00750230(LineOfSightTrigger& t, float dt, const Vec3& watched,
                                  const Vec3& target, std::uint32_t watched_ignore,
                                  std::uint32_t target_44, LineOfSightHost& host);

enum class SquadOrdnanceCheck { target_gone, fire, wait };
// 007504F0 (vtable 00D005C0): +30h null -> vtable[10h]; else fire when
// 007ED790(squadron, 0) answers false.
SquadOrdnanceCheck squad_ordnance_check_007504f0(bool squadron_present_30,
                                                 bool any_can_drop_007ed790);

}  // namespace gameplay_seg41_cc13
}  // namespace bsp
