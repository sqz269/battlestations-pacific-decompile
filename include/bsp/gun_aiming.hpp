#pragma once
// Gun aiming and automatic fire. docs/GUN_AIMING.md carries the evidence; every
// descriptive name here is a hypothesis, not a recovered symbol. The field names
// horzAngle, vertAngle, tHorzAngle, tVertAngle, horzRotDir, HorzRotSpeed,
// VertRotSpeed and MinHorzAngle..MaxVertAngle are the exception: the image and the
// shipped Lua tables carry them as literal strings.
//
// Scope. This models three native routines as rules and one as a sequence:
//   0085ABA0  accept a target angle pair, or refuse it        (pure)
//   0085AD80  step the current pair toward the target pair    (pure, per axis)
//   0085A830 / 00729A80  the automatic-fire gate              (pure)
//   0085A270  the per-frame update                            (host sequence)
// The node matrices, the recoil spring's own math, effects and sound are contracts:
// 00859550 installs two rotation matrices through node->vtable[38h], and the barrel
// point goes to barrelNode->vtable[2Ch]. Neither is ported here.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_weapons.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Instance offsets of the turning gun (00CFBD20 and its three subclasses).
// Names from 0085A130, the class's own debug dump, and 0085BB50, the Lua reader.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kGunAimOffPlatformIndex = 0x38c;  // 0085AC5F, 0085A849
inline constexpr std::size_t kGunAimOffDisableByte = 0x3b8;    // 0085AD96, 00729A80
inline constexpr std::size_t kGunAimOffTraverseNode = 0x3bc;   // 00859550
inline constexpr std::size_t kGunAimOffElevationNode = 0x3c8;  // 00859550
inline constexpr std::size_t kGunAimOffHorzAngle = 0x480;      // "horzAngle", 0085A160
inline constexpr std::size_t kGunAimOffVertAngle = 0x484;      // "vertAngle", 0085A194
inline constexpr std::size_t kGunAimOffHorzAngleHeld = 0x488;  // 0085AD80 seeds horzAngle from it
inline constexpr std::size_t kGunAimOffVertAngleHeld = 0x48c;  // 0085AD80 seeds vertAngle from it
inline constexpr std::size_t kGunAimOffFireInhibit = 0x490;    // 0085A833
inline constexpr std::size_t kGunAimOffTargetHorzAngle = 0x494; // "tHorzAngle", 0085A1C8
inline constexpr std::size_t kGunAimOffTargetVertAngle = 0x498; // "tVertAngle", 0085A1FC
inline constexpr std::size_t kGunAimOffHorzRotDir = 0x49c;      // "horzRotDir", 0085A22E
inline constexpr std::size_t kGunAimOffAimAcceptedMark = 0x4a0; // 0085ABB5 writes -1.0f
inline constexpr std::size_t kGunAimOffReplicatedAngles = 0x4bc; // float[4], zeroed by 006FDDA0
inline constexpr std::size_t kGunAimOffSnapshotSequence = 0x4cc; // FFFFFFFFh from 006FDDA0
inline constexpr std::size_t kGunAimOffTickElement = 0x310;      // the +310h sub-object 0085AD80 runs on

// gun+3F0h. unit_weapons.hpp already declares this offset as kGunOffAmmoProvider;
// the evidence in docs/GUN_AIMING.md identifies the same pointer as the owning
// unit (0085AC51 -> unit+538h, 00729A80 -> unit+6F8h/+6FCh). No second constant is
// declared for one offset; use kGunOffAmmoProvider and read it as the owner.

// Weapon class descriptor, loaded by 007327B0.
inline constexpr std::size_t kGunAimOffHorzRotSpeed = 0x88; // "HorzRotSpeed", stored at 00732A30
inline constexpr std::size_t kGunAimOffVertRotSpeed = 0x8c; // "VertRotSpeed", stored at 00732A6C

// Unit class descriptor: the platform pointer array and its count.
inline constexpr std::size_t kGunAimOffUnitClassDescriptor = 0x538; // 0085AC58
inline constexpr std::size_t kGunAimOffPlatformArray = 0x94;        // 0085AC65
inline constexpr std::size_t kGunAimOffPlatformCount = 0x98;        // 0085A85C

// Platform record: the firing-arc list and the rest angles.
// docs/VEHICLE_CLASS_FIELDS.md is the producer for the two rest angles.
inline constexpr std::size_t kGunAimOffArcArray = 0x3c;   // 007F5FC0
inline constexpr std::size_t kGunAimOffArcCount = 0x40;   // 007F5FC0
inline constexpr std::size_t kGunAimOffRestVertAngle = 0x90; // "RestAngles[2]", default 0.0f
inline constexpr std::size_t kGunAimOffRestHorzAngle = 0x94; // "RestAngles[1]", default FLT_MAX
inline constexpr std::size_t kGunAimArcRecordStride = 0x14;  // 007F5FC0's pointer step

// Gun vtable 00CFE0A8 slots used by the aiming and firing chain.
inline constexpr std::size_t kGunVtableSlotUpdate = 0x0dc;      // 0072B2D0 / 0085A270
inline constexpr std::size_t kGunVtableSlotCanFire = 0x1d0;     // 00729A80 / 0085A830
inline constexpr std::size_t kGunVtableSlotFireIfReady = 0x1dc; // 00727E30
inline constexpr std::size_t kGunVtableSlotHandleMessage = 0x164; // 0072D830
inline constexpr std::uint32_t kGunTickElementVtableRapidFixedSlave = 0x00cfe504; // slot +4h is 0085AD80

// Class ids. kClassTestGun (20h) is in unit_weapons.hpp.
inline constexpr int kGunClassIdRapidFixedSlave = 0x23; // MRFSGun, the 0085AF76 test
inline constexpr int kGunClassIdRapidTurning = 0x24;    // MRTGun
inline constexpr int kGunClassIdSingleTurning = 0x27;   // MSTGun

// Network opcodes on the byte at message+10h, from 0072D830 and its overrides.
inline constexpr int kGunMessageOpcodeFireIfReady = 0xad; // 0072D860
inline constexpr int kGunMessageOpcodeAimAndFire = 0xb0;  // 0084C7E4, MRT and MST only
inline constexpr int kGunMessageOpcodeFixedSlave = 0xaf;  // 00803484, MRFS only

// ---------------------------------------------------------------------------
// Tuning constants, each with the address of the literal it comes from.
// ---------------------------------------------------------------------------
inline constexpr float kGunAimPi = 3.1415927410125732f;      // 00D7A264 (+), 00CE684C (-)
inline constexpr float kGunAimTwoPi = 6.2831854820251465f;   // 00CE3828
inline constexpr float kGunAimArcEpsilon = 0.008726646192371845f; // 00D08B88, half a degree
inline constexpr float kGunAimDeadBand = 0.0001745329354889691f;  // 00CFAA48, 0.01 degree
// The FIRE gate is a different, looser test and a different constant. 006DF520
// step 12 (006DFB8C, 006DFBB6) arms the trigger when 006DEE40 reports both axes
// inside *00CF9054 = 0.0017453 rad, 0.1 degree - ten times the stepper's dead
// band above. Using kGunAimDeadBand to decide whether a gun may fire makes the
// host ten times stricter per axis than the native. Verified: 00CF9054 reads
// 89 c3 e4 3a = 0.0017453293548896909 = 0.1 deg exactly.
// docs/GUN_SHOT_CADENCE.md divergence 2.
inline constexpr float kGunFireSettleBand = 0.0017453293548896909f;  // 00CF9054, 0.1 degree
inline constexpr float kGunAimSoftApproachSpan = 0.1745329350233078f; // 00CE3990, ten degrees
inline constexpr float kGunAimSoftApproachFloor = 0.5f;      // 00CE3800
inline constexpr float kGunAimSnapThreshold = 0.800000011920929f; // 00CE3D40, used by 0085B0F0
inline constexpr float kGunMuzzleWaterlineY = 1.0f;          // 00D7A24C, 00729A80's height test
inline constexpr float kGunAimAcceptedMarkValue = -1.0f;     // 00D7A260
inline constexpr float kGunRecoilSubStep = 0.008333333767950535f; // 00CEF0B8 / 00CFDF80, 1/120 s

// ---------------------------------------------------------------------------
// One firing arc ("window") of a platform, built by 007F6B10 from the Lua keys.
// ---------------------------------------------------------------------------
struct GunFiringArc {
    std::uint8_t flags{0};  // +0h: bit 0 may traverse here, bit 1 may fire here
    float min_horz{0.0f};   // +4h  "MinHorzAngle"
    float max_horz{0.0f};   // +8h  "MaxHorzAngle"
    float min_vert{0.0f};   // +Ch  "MinVertAngle"
    float max_vert{0.0f};   // +10h "MaxVertAngle"
};
inline constexpr std::uint8_t kGunArcFlagTraverse = 0x01; // 007F5FC0 tests bit 0
inline constexpr std::uint8_t kGunArcFlagFire = 0x02;     // 007F60A0 tests bit 1

// The platform's arc list as the native code sees it: a base pointer and a count.
struct GunPlatformArcs {
    const GunFiringArc* first{nullptr};
    std::size_t count{0};
};

// The turning gun's own angle state, in radians.
struct GunTurningAngles {
    float horz{0.0f};        // +480h "horzAngle"
    float vert{0.0f};        // +484h "vertAngle"
    float target_horz{0.0f}; // +494h "tHorzAngle"
    float target_vert{0.0f}; // +498h "tVertAngle"
};

// The two rates the weapon class descriptor carries. Zero means "this axis does
// not turn", which 0085ABA0 turns into a refusal and 0085AD80 into a snap.
struct GunRotationSpeeds {
    float horz{0.0f}; // descriptor+88h "HorzRotSpeed", radians per second
    float vert{0.0f}; // descriptor+8Ch "VertRotSpeed"
};

// ---------------------------------------------------------------------------
// Angle rules.
// ---------------------------------------------------------------------------
// fmod(a, 2pi) folded into (-pi, +pi]: 0085ABBD..0085AC04 and the three copies of
// the same block in 0085A8B0, 0085B0F0 and 0085ABA0's second argument.
float gun_wrap_angle_0085abbd(float radians) noexcept;

// The clamp 007F5FC0 applies to the horizontal angle before every bound test.
float gun_clamp_horizontal_007f5fc0(float radians) noexcept;

// One window test, both axes, each bound widened by kGunAimArcEpsilon.
bool gun_arc_contains_007f5fc0(const GunFiringArc& arc, float horz, float vert) noexcept;

// 007F5FC0: any arc with bit 0 set that contains the pair.
bool gun_traverse_allowed_007f5fc0(const GunPlatformArcs& arcs, float horz, float vert) noexcept;

// 007F60A0: the same walk against bit 1.
bool gun_fire_allowed_007f60a0(const GunPlatformArcs& arcs, float horz, float vert) noexcept;

// 007F5960: one record, horizontal bounds only, no flag test.
bool gun_arc_contains_horizontal_007f5960(const GunFiringArc& arc, float horz) noexcept;

// 0085ABA0. Wraps both angles, refuses a pair no traverse window contains and
// refuses either axis whose rotation speed is zero. On success it writes the
// target pair and returns true. `accepted_mark` receives kGunAimAcceptedMarkValue
// unconditionally, which is what the native routine does at 0085ABB5 before any test.
bool gun_set_target_angles_0085aba0(GunTurningAngles& angles,
                                    const GunPlatformArcs& arcs,
                                    const GunRotationSpeeds& speeds,
                                    float horz,
                                    float vert,
                                    float& accepted_mark) noexcept;

// The per-axis rule inside 0085AD80, as a pure function.
//   delta = wrap(target - current)
//   step  = clamp(delta, -rate*dt, +rate*dt)
//   step *= soft approach, unless the class is MRFSGun
// `delta` is the value 007F6530 hands back on the unobstructed path; the native
// routine takes it from that platform routine, which is a contract here.
float gun_soft_approach_scale_00419010(float remaining_radians) noexcept;
float gun_step_axis_0085ad80(float delta,
                             float rate,
                             float dt,
                             bool is_rapid_fixed_slave) noexcept;

// The whole step for both axes, with 007F6530's deltas supplied by the caller and
// 007F6840's answer supplied by the caller. Returns true when an angle changed.
struct GunStepDeltas {
    float horz{0.0f}; // 007F6530's out parameter for the traverse axis
    float vert{0.0f}; // 007F6530's out parameter for the elevation axis
};
bool gun_step_aim_0085ad80(GunTurningAngles& angles,
                           const GunRotationSpeeds& speeds,
                           const GunStepDeltas& deltas,
                           float dt,
                           bool is_rapid_fixed_slave) noexcept;

// The dead band that ends the step before any platform call (0085AE4A onwards).
bool gun_aim_settled_0085ae4a(const GunTurningAngles& angles) noexcept;

// ---------------------------------------------------------------------------
// The automatic-fire gate.
// ---------------------------------------------------------------------------
// Everything 00729A80 reads, flattened. The three predicates the native routine
// answers with calls (006D1E50, 00728A90, 005459B0) are inputs here.
struct GunFireGateInputs {
    bool fire_params_armed{false};   // [gun+3F8h]+34h non-zero
    bool disabled{false};            // gun+3B8h
    int damage_counter{0};           // gun+358h, positive blocks
    float barrel_delay_time{0.0f};   // gun+450h "barrelDelayTime"
    float secondary_delay{0.0f};     // gun+478h
    bool unit_cooldown_applies{false}; // 006D1E50's answer
    float unit_fire_cooldown{0.0f};  // unit+6F8h
    float unit_torpedo_cooldown{0.0f}; // unit+6FCh
    int weapon_type_id{0};           // [gun+3F4h]+80h; kWeaponTypeTorpedo is 7
    float muzzle_world_y{0.0f};      // BSP_EntityPose_GetWorldPositionRefreshed's y
    bool muzzle_submerged{false};    // 00728A90's answer
    bool muzzle_blocked{false};      // 005459B0's answer
    int barrel_count{0};             // gun+448h "barrelNum"
    const float* reload_timers{nullptr}; // gun+414h, barrel_count entries
};
bool gun_can_fire_00729a80(const GunFireGateInputs& in, bool check_reload) noexcept;

// 0085A830: the turning gun's own two tests, then the base answer.
bool gun_can_fire_turning_0085a830(bool fire_inhibited,
                                   const GunPlatformArcs& arcs,
                                   const GunTurningAngles& angles,
                                   const GunFireGateInputs& in,
                                   bool check_reload) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary. One virtual per native call site in the per-frame update
// and the fire chain, in the order the native code makes them. Nothing here has a
// default implementation: none of it is a stand-in for unrecovered behaviour.
// ---------------------------------------------------------------------------
struct GunAimHost {
    virtual ~GunAimHost() = default;

    // 0085A27E: the base gun update, which advances the barrel recoil.
    virtual void base_gun_update_0072b2d0(float dt) = 0;

    // 0085A348: the barrel node's refreshed world matrix. Returns its row 2, the
    // forward axis the recoil offset is taken along.
    virtual void barrel_node_forward_00b6db60(int barrel_index, float out_forward[3]) = 0;

    // The platform's per-barrel offset from the descriptor vector at +B8h
    // (begin +BCh, end +C0h, stride 0Ch), bounds-checked at 0085A2FE and 0085A331.
    virtual void barrel_platform_offset(int barrel_index, float out_offset[3]) = 0;

    // 0085A3A3: barrelNode->vtable[2Ch](&point).
    virtual void set_barrel_point_vtable_2ch(int barrel_index, const float point[3]) = 0;

    // 0085A3B9: 00859550 installs the two rotation matrices on gun+3BCh and
    // gun+3C8h through node->vtable[38h]. Described, not ported.
    virtual void apply_angles_to_nodes_00859550(float horz, float vert) = 0;

    // 00727E3D: gun->vtable[1D0h](1).
    virtual bool can_fire_vtable_1d0h(bool check_reload) = 0;

    // The call after 00727E47: gun->vtable[1D8h](mode, throwA, throwB).
    virtual void fire_vtable_1d8h(int mode, float throw_a, float throw_b) = 0;
};

// One barrel as 0085A270 reads it: the recoil slide and the active flag that
// 0072B2D0 wrote this step. docs/UNIT_WEAPON_DEVICES.md names +4h "dist".
struct GunBarrelRecoilView {
    bool active{false}; // barrel+10h, set by 0072B2D0 when speed is non-zero
    float dist{0.0f};   // barrel+4h "dist", the recoil slide, zero or negative
};

// 0085A270, as a sequence over the host. `barrels` has `barrel_count` entries.
void gun_update_0085a270(GunAimHost& host,
                         const GunTurningAngles& angles,
                         const GunBarrelRecoilView* barrels,
                         int barrel_count,
                         float dt);

// 00727E30, as a sequence: CanFire with the reload check on, then Fire(0, 0, 0).
bool gun_fire_if_ready_00727e30(GunAimHost& host);

} // namespace bsp
