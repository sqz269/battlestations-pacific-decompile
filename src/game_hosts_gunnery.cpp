// bsp_game.exe milestone 2t: the gun chain.
//
// See include/bsp/game_hosts_gunnery.hpp for the address list and for what this
// process does not hold. Nothing here reconstructs native code: every step is a
// call of a bsp:: rule or host already on main, or a recorded gap.

#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/gun_aim_terms.hpp"
#include "bsp/gun_fire_points.hpp"
#include "bsp/geom_mesh_resource.hpp"
#include "bsp/gun_mount_positions.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/structured_hierarchy.hpp"

#include <array>
#include <limits>
#include <map>
#include <memory>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "bsp/ai_tuning_globals.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/gun_gravity_arc.hpp"
#include "bsp/bullet_engagement_range.hpp"
#include "bsp/gun_heading_snap.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/session_participant_pools.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/gun_bot_remainder.hpp"
#include "bsp/gun_bot_ticks.hpp"
#include "bsp/gun_dispersion.hpp"
#include "bsp/hit_narrowphase.hpp"
#include "bsp/spatial_index.hpp"
#include "bsp/kill_credit.hpp"
#include "bsp/attack_target_classify.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/recon_sensor_pass.hpp"
#include "bsp/sensor_table_data.hpp"
#include "bsp/ship_hit_record.hpp"
#include "bsp/submarine_model.hpp"
#include "bsp/unit_kind_query.hpp"
#include "bsp/unit_damage.hpp"
#include "bsp/unit_hit_path.hpp"
#include "bsp/unit_weapons.hpp"

namespace bsp::game {
namespace {

// Packets cc9_aa_targeting and cc9_rng_streams: the three gun-side acceptance
// terms of 00729BC0's bot slots and 00865773's minimum-air-range skip, which the
// host used to answer with a range test alone. docs/AA_TARGETING.md and
// docs/RANDOM_STREAMS.md. Each is its own switch and was measured as its own
// pair with BSP_GUNNERY_RNG_STREAMS=1:
//  * kind 5/6 minimum air range: LANDED. The pair moved exactly the eight
//    predicted Yorktown-class01 FLAK rows (gun rows 199-206) and nothing else.
//  * AA gunner armour test: LANDED. No aircraft in USN04 has the armour to
//    trigger it, and its pair was identical, as predicted.
//  * fire window: OFF. It needs the per-gun node frame the host does not build;
//    bound here in the hull frame, which is a substitution.
constexpr bool kAaMinRangeBound = true;     // 005459E0 / 00729B90
constexpr bool kAaArmourBound = true;       // 008FBE00's armour test
constexpr bool kAaFireWindowBound = false;  // 0085A9A0 (hull-frame substitution)

// Packet cc9_gun_ballistics. docs/GUN_BALLISTICS.md.
//  * kGunGravityArcBound: the gravity arc 00955630 and its 006DF8BF pre-estimate
//    belong to the ArtilleryGunnerBot tick 006DF520 alone (sub-types 2, 3, 4, 9,
//    and 6 against a non-plane). The AAGunnerBot 00902920 (sub-type 1) and the
//    AAFlakBot 009030C0 (sub-type 5, and 6 against a plane) aim at the lead point
//    with no gravity term at all (docs/AA_VERTICAL_WINDOW.md section 2).
//  * kBulletNoGravityBound: a round flies without gravity when its class sets
//    NoGravity (classDesc+20h); the host had gravity on for every round.
//  * kGunAimErrorBound: the ArtilleryGunnerBot's aim-error envelope 006DEFF0 /
//    006DF5A0 / 006DFB0B, on the row units.skill_level() selects.
constexpr bool kGunGravityArcBound = true;
constexpr bool kBulletNoGravityBound = true;
constexpr bool kGunAimErrorBound = true;
// Packet cc9_aa_lead, docs/AA_LEAD.md.
//  * kPlaneGunfireHooked: a plane's forward guns (PLANEGUN, category 0) take their
//    trigger from the latched gunFire the plane's fixed step hands each enabled
//    gun's SetTriggerHeld (007CE9A0-007CE9F4), published as
//    units.plane_gun_trigger_bc9. The weapon-group enable byte
//    ([[unit+538h]+94h][gun+38Ch]+0Ch) is taken as on: labelled.
constexpr bool kPlaneGunfireHooked = true;
//  * kGunInterceptBound: the AA gunner's and flak bot's lead point is 00901C20's
//    closed-form intercept, not the target velocity times distance / V0.
constexpr bool kGunInterceptBound = true;
//  * kDualPurposeSecondAmmoBound: a sub-type 6 gun meets a plane with its SECOND
//    ammunition record (00729BC0 / 00729B90 variant 1, +74h + 1*48h, the
//    device's Bullet[2]) and so through its AAFlakBot. The host had loaded only
//    Bullet[1] and read the pair of entries as two barrels.
constexpr bool kDualPurposeSecondAmmoBound = true;
//  * kAaGunnerErrorBound: the AAGunnerBot's commanded vertical, when negative, is
//    halved before 0085ABA0 (00902F62..00902F76, [00D7A280] = 0.5). Only this
//    step of 00902920's error model is bound; the swinging error pair
//    (00902B38..00902EF7) needs the unread vtable[100h] sample and 00BD2F90 split.
constexpr bool kAaGunnerErrorBound = true;
//  * kAaTargetWorldVelocityBound: the AA bots lead a plane with its world
//    velocity (vtable[34h] = 007BBB70, unit+AC8h), not its body axis times
//    0092D730's forward speed. Packet cc9_aa_lethality_audit.
constexpr bool kAaTargetWorldVelocityBound = true;
//  * kFlakProximityBurstBound: a Flak-type round runs 0070C370's proximity fuse
//    after the base tick's direct-strike sweep: it locks the nearest plane,
//    torpedo boat or landing ship within min(300 m, L/2 + 2 * BlastRange) of
//    its step's midpoint, takes 00901C20's intercept of it, and bursts that far
//    along its track (0070C210, the radial blast). OFF: direct strikes only.
//    Packet cc9_flak_proximity_burst.
constexpr bool kFlakProximityBurstBound = true;
//  * kGunBarrelCountBound: gun+448h, the barrel count, is 0072AB80 on the
//    muzzle list 007325A0 builds from the device model (the row's `Mesh`,
//    [class+50h]): ("fire", 0)'s whole Points list, else one first point per
//    consecutive ("fire", k) Aux item from k = 1, floored to 1 (0072E71A).
//    OFF: the number of `Bullet` records in the device row. A model that is
//    absent or unreadable keeps the record count and is counted in the
//    summary. Packet cc9_gun_barrel_count, docs/GUN_BARREL_COUNT.md.
constexpr bool kGunBarrelCountBound = true;
//  * kShipPlatformAttachmentBound: a ship gun fires from its own mount, the
//    platform frame's origin p0 of the ship model's ("slot", platform key)
//    group (0095F500's slot pass into platform+4Ch; 0072DD20 hands that frame
//    to the device model's root node at 0072E9A1-0072E9AA), carried to world
//    by the ship pose every tick. OFF: the unit origin raised by the class
//    `Height` for every gun. Labelled: model +x = starboard, +y = up, +z =
//    bow (from the port/starboard firing arcs); the device model's own node
//    chain (base, barrel, yaw and elevation) and the per-barrel muzzle offsets
//    are not applied; planes are not covered. docs/SHIP_PLATFORM_ATTACHMENT.md.
constexpr bool kShipPlatformAttachmentBound = true;
//  * kAaLineOfFireBound: an AA gun (weapon kinds 1, 5, 6; 00729560 installs the
//    predicate at gun+42Ch) refuses a target when 0072CDD0 answers blocked:
//    the segment from the gun (+5 m) to the target (+5 m, at least y = 5)
//    first meets another unit - neither the firer nor the target (0098B130
//    excludes both) - on the firer's own side (hit+54h == owner+54h). The
//    answer is cached per target for the gun's life (0072F6E0; the cached
//    record's expiry field has no reader found). Labelled: units are the
//    unit model BoundingBox, oriented by its pose (the image tests the unit AABB then 0085CDB0);
//    the static-geometry half (spatial query, flags 44h) is not modelled,
//    since nothing static stands between ships at sea. OFF: always clear.
//    docs/SHIP_PLATFORM_ATTACHMENT.md.
constexpr bool kAaLineOfFireBound = true;
//  * kAabb0085cdb0Bound: packet cc9_aabb_0085cdb0 (docs/AABB_0085CDB0.md).
//    0098B130 tests each unit's spatial-index WORLD AABB (0098A750: the
//    oriented box's axis-aligned hull) with 0085CDB0, nearest hit first;
//    00929B80 runs 0085CDB0 on the shape's local box with the segment
//    carried into its frame. OFF: the oriented-box slab tests.
constexpr bool kAabb0085cdb0Bound = true;
//  * kArtilleryAimPointBound: the ArtilleryGunnerBot (006DF520, sub-types 2, 3,
//    4, 9, and 6 against a non-plane) aims at a point ON its ship target, not
//    at the target origin raised by the class Height. Step 4: when bot+B4h
//    runs out it is reset to the skill row's TargetPointRefreshTime (row+14h,
//    006DF71B) and target->vtable[100h] = 00816650 (the ship vtables) draws a
//    body-frame point with the row's SectionTargetChance and section weights
//    (row+18h..+24h, 006DF72B-006DF743) over a box of 0.6 (00CE3D30) about the
//    origin (00F87574, zero). Step 5 carries it to world by the target's matrix
//    (00414D10 with target+CCh). Labelled: the ship's engine-room, magazine
//    and fuel-tank section points are not loaded, so the section path is
//    unavailable and every draw takes the hull box; bot+90h's stepped offset
//    toward bot+84h is not modelled; a target change re-draws at once. OFF:
//    the target origin raised by Height. docs/SURFACE_GUNNERY_REFERENCE.md.
constexpr bool kArtilleryAimPointBound = true;
//  * kTorpedoFriendlyCrossingBound: packet cc9_torpedo_launch_gate. 008FFF20's
//    friendly-crossing gate 0090058A..009007F6: a torpedo launch is held when
//    another own-side ship within 2000 m would be within 0.3 * run + 200 m of
//    the crossing of its heading line with the 1000 m run line when the torpedo
//    gets there. SUBSTITUTIONS: the lead point lacks 00951FC0's alternating
//    +6D4h spread offset (the host does not model it); [gun+3CCh]'s row-2 axis
//    is the hull forward turned by the gun's current horizontal angle; the
//    own-party list is every live same-side unit (a sunk ship leaves it).
//    docs/TORPEDO_LAUNCH_GATE.md.
constexpr bool kTorpedoFriendlyCrossingBound = true;
//  * kMuzzleOffsetsBound: packet cc9_muzzle_offsets. A ship gun's shot starts
//    at 00730762's TransformAffinePoint(class+98h[barrel], [gun+3CCh] world):
//    the device model's "barrel"/"base"/first node, posed by 00859550 from the
//    gun's current angles, under the platform frame and the ship pose. Only
//    the spawn point moves; the aim keeps the mount (gun+FCh). SUBSTITUTIONS:
//    0071AD50's name lookup is an exact name match on the parsed Hierarchy;
//    hierarchy item 0 is taken as [model+0Ch]; Item matrices are taken as
//    parent-relative; the host's horizontal angle is negated to the image's
//    convention; recoil (0085A270's barrel loop) and the [gun+3Ch] slot-94h
//    override are not applied; planes and Mesh-less devices keep the mount.
//    docs/MUZZLE_OFFSETS.md.
constexpr bool kMuzzleOffsetsBound = true;
//  * kGunHorzImageSignBound: packet cc9_gun_horz_sign. A gun's horizontal angle
//    takes the image's sign: 008FDAF0 and 0085A9A0 (0085AB17) store
//    -0.0 - 00521370's atan2(x, z), so a positive angle turns towards model -x
//    (port), and 00859550's RotY(-horz) turns the barrel the same way. The
//    authored Windows and RestAngles are loaded unnegated, as the image loads
//    them. OFF: the host's earlier +atan2(right, forward), which mirrors every
//    authored window. docs/GUN_HORZ_SIGN.md.
constexpr bool kGunHorzImageSignBound = true;
// +1 in the host's earlier convention, -1 in the image's: the factor on the
// starboard component wherever a horizontal angle meets a direction.
constexpr float kGunHorzSign = kGunHorzImageSignBound ? -1.0f : 1.0f;
//  * kBulletThrowBound: packet cc9_bullet_throw. 00730160's dispersion cone:
//    the fire record's `Throw` times the seat bot's BulletThrowMul
//    (0073031D..00730498, the role gates through 00521E70), then
//    theta = U(0, 2pi), radius = tan(magnitude) * U(0, 1) (00730540..0073058F)
//    added across the barrel (00730654..0073075E); torpedo sub-type 0Ah takes
//    the deterministic fan (007304A0) instead. Own RNG purpose (bullet_throw).
//    SUBSTITUTIONS: unit+63Ch is 1.0 (0095DD79 for every unit whose role 4 is
//    not held locally; the idle player's steady fraction is not modelled);
//    00470440(7) is 1.0 (no modifier records); a role slot other than 8 is
//    taken as player-held (00927F10 not modelled); the cone's two axes are any
//    orthonormal pair across the shot direction (the draw is rotation-uniform);
//    the fan rotates about the hull up axis (0085C3F0 read only at its entry).
//    docs/BULLET_THROW.md.
constexpr bool kBulletThrowBound = true;
//  * kPartySlotAiHeldBound: packet cc9_usn02_sameside_torpedoes. 00521E70's
//    AI test on a role slot other than 8 reads the party slot record through
//    00927F10 (the ship AI host's SessionParticipantPools) instead of taking
//    it as player-held. Used by the bullet-throw gates and the player gun seat.
constexpr bool kPartySlotAiHeldBound = true;
//  * kTorpedoGyroHeadingBound: packet cc9_usn02_sameside_torpedoes. 008FFF20's
//    launch command 007311B0 carries `heading` = the world heading of the
//    friendly gate's own run line (atan2 at 00900513, stored 00900526) plus the
//    TorpedoBot AngleErr jitter (00900830..00900876); 00856637 copies it to the
//    torpedo's record+46Ch and 00857061 turns the swimming torpedo toward it at
//    HeadingTurn degrees per second. The snapped bot+60h only trains the tube.
//    Without this the host's torpedo ran straight down the snapped tube
//    heading, up to pi/4 off the line the gate tested. SUBSTITUTIONS: the turn
//    runs only while swimming; `swimdepth` (00852410) is not modelled; the
//    rotation sense is toward the commanded heading.
constexpr bool kTorpedoGyroHeadingBound = true;
// TorpedoBot AngleErrMin / AngleErrMax, degrees, this installation's
// robots.lua by skill index (008FD640 +0Ch / +10h).
//  * kReconTeamListsBound: packet cc9_recon_team_lists. The gunnery sweep's
//    contact list 008053C0(unit+54h)+0DE8h is the recon slot's published enemy
//    triple as 008073C0 leaves it: scanned classes (00806480) in bucket order,
//    identified (level 2) targets only, blips going to `unknown` instead. OFF:
//    the earlier stand-in, every other-side unit with any level above none, in
//    unit order. SUBSTITUTIONS: the squadron and convoy aggregates (classes 18h
//    and 1Ah, 00805490 / 00805680) are not built. docs/RECON_TEAM_LISTS.md.
constexpr bool kReconTeamListsBound = true;
constexpr float kTorpedoAngleErr[6][2] = {
    {10.0f, 20.0f}, {0.0f, 10.0f}, {0.0f, 0.5f}, {0.0f, 6.0f}, {0.0f, 3.0f}, {0.0f, 0.5f}};
// This installation's shipglobals.lua:74 authors TurnOffAAGunThrow = false
// (0083B5E0 reads it into settings+760h at 00841B68).
constexpr bool kTurnOffAaGunThrow = false;
// This installation's robots.lua (2025-06-01), by skill index 0 Stun,
// 1 SPNormal, 2 SPVeteran, 3 MPNormal, 4 MPVeteran, 5 Elite. The getters:
// AAGunnerBot 008FB4C0 (+18h), TailGunnerBot 008FB4E0 (+1Ch), AAFlakBot
// 008FB500 (+28h), TorpedoBot 008FB550 (+1Ch), DepthChargeBot 008FB570 (+10h),
// ArtilleryGunnerBot 006DEE00 (the SubDirector's +30h), PilotBot 00999610
// (AimBulletThrowMul, +250h).
enum ThrowSeatBot : int { kThrowAaGunner, kThrowTailGunner, kThrowAaFlak, kThrowTorpedo,
    kThrowDepthCharge, kThrowArtillery, kThrowPilot, kThrowSeatCount };
constexpr float kBulletThrowMul[kThrowSeatCount][6] = {
    {1.0f, 1.0f, 0.0f, 0.5f, 0.25f, 0.0f},   // AAGunnerBot
    {1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f},    // TailGunnerBot
    {1.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f},    // AAFlakBot
    {1.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.0f},    // TorpedoBot
    {1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f},    // DepthChargeBot
    {1.0f, 0.9f, 0.1f, 0.85f, 0.5f, 0.1f},   // ArtillerySubDirectorBot
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},    // PilotBot AimBulletThrowMul
};
//  * kShipSectionPointsBound: the artillery draw 00816650 can pick the target's
//    engine room (kind 5), fuel tank (6) or magazine (8). 0081F980 fills
//    unit+A88h / +A78h / +A68h from the ship model's GeomMesh elements of those
//    kinds (00820566 / 008205D7 / 00820648; the last element of a kind wins),
//    each point being 00723030's centre of the element's root box. The draw
//    takes them with the gunner's skill row: SectionTargetChance and the three
//    weights (robots.lua ArtilleryGunnerBot). Labelled: the element box is
//    taken as its triangles' bounding box in model space; no section is ever
//    destroyed (0093A570's list stays empty). OFF: the hull box every time.
constexpr bool kShipSectionPointsBound = true;
//  * kShellHullHitTestBound: a round's segment against a SHIP is tested against
//    the triangles of the ship model's GeomMesh elements (00724510 ->
//    00723E90 -> 00723D60 -> 00723AA0: closest hit over the elements' triangle
//    lists, in the geometry node's space), with the model's mesh bounds as the
//    broad phase. Labelled: every GeomMesh triangle is tested in model space
//    (the node transforms are identity on the hulls read), with no per-element
//    AABB tree (007238E0 unread). OFF: the class hull box.
constexpr bool kShellHullHitTestBound = true;
//  * kKillCreditDamageGateBound: 0077CE60 writes the attribution block (the
//    +2C4h attacker the kill credit 0091BDA0 names) only for a live victim and
//    only when the hit's damage, 00470510 for a hull segment or 00470740 when
//    record+34h is -1, is above 0.0 (0077CE8E-0077CEBE, [00D7A218]). The host
//    wrote it on every hit, so a zero-damage hit could take the credit. OFF:
//    every hit. Packet cc9_kill_credit, docs/KILL_CREDIT.md.
constexpr bool kKillCreditDamageGateBound = true;

// 00901C20 BSP_GunBot_InterceptSolution, the time-of-flight half, as a pure rule.
// rel = target position - shooter position; vel = target velocity - shooter
// velocity (a ship target's vertical component zeroed first); speed = V0.
// Constants: 2.0f speed floor (00CE3958), 1.0 distance floor, 0.01f
// near-radial threshold (00D7A238), 1.5 (00CE3D78, double) speed-ratio gate,
// 4.0 (00D7A328, double), the [0, 12] clamp (00CEB4B8). No gravity, no loop.
float intercept_time_00901c20(const float rel[3], const float vel[3], float speed,
                              float distance) {
    if (distance <= 1.0f) return 0.0f;
    const float a = (vel[0] * rel[0] + vel[1] * rel[1] + vel[2] * rel[2]) / distance;
    const float vv = vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2];
    float t = 0.0f;
    if (0.01f <= vv - a * a) {
        if (static_cast<double>(vv) * 1.5 < static_cast<double>(speed) * speed) {
            const float qa = vv - speed * speed;
            const float qb = (distance + distance) * a;
            const float disc = static_cast<float>(static_cast<double>(qb) * qb
                - static_cast<double>(qa) * 4.0 * distance * distance);
            float t1 = 0.0f;
            float t2 = 0.0f;
            if (0.0f <= disc) {
                const float root = std::sqrt(disc);
                t1 = (root - qb) / (qa + qa);
                t2 = (-qb - root) / (qa + qa);
            }
            t = t1;
            if (t1 < 0.0f || (0.0f <= t2 && t2 < t1)) t = t2;
            if (t < 0.0f) t = 0.0f;
        }
    } else {
        t = distance / (speed - a);
        if (t < 0.0f) t = 0.0f;
    }
    return std::min(std::max(t, 0.0f), 12.0f);
}

// This installation's scripts/datatables/robots.lua (modified 2025-06-01),
// Robots["ArtilleryGunnerBot"], indexed as 00901610 reads the levels and as
// luamw_init.lua's SKILL_* number them (docs/GAME_DIFFICULTY.md): Stun 0,
// SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5. MaxAngleError is
// DEG(x) in the file.
constexpr float kDegToRad = 0.01745329251994329577f;
constexpr bsp::ArtilleryGunnerAimErrorLevel kArtilleryGunnerLevels[6] = {
    {10.0f * kDegToRad, 1.0f},  // 0 Stun
    {5.0f * kDegToRad, 1.0f},   // 1 SPNormal
    {0.0f * kDegToRad, 2.0f},   // 2 SPVeteran
    {4.0f * kDegToRad, 1.5f},   // 3 MPNormal
    {2.0f * kDegToRad, 1.8f},   // 4 MPVeteran
    {0.0f * kDegToRad, 2.0f},   // 5 Elite
};

constexpr int kMaxPlatformScan = 64;   // the `Platforms` keys this process scans
constexpr int kMaxWindowScan = 8;      // the `Windows` entries per platform
constexpr float kAngleScale = 1000000.0f;
constexpr float kMilliScale = 1000.0f;
constexpr float kHalfPi = 1.57079637050628662109375f;
constexpr float kQuarterPi = 0.785398185253143310546875f;  // 00CEB5A8
constexpr float kGravity = 9.8100004196166992187500f;      // 00CF9058

// 00470470's reset leaves +34h at -1, which is the hull segment a direct
// segment hit keeps: all three known shapes write -1 (docs/HIT_NARROWPHASE.md).
constexpr int kDirectHitHullSegment = -1;
// FLD double ptr [00D7A270] at 0084BE63: the burst centre is backed off from
// the impact point along the impact direction by this much. The value reads
// 0.05 (tools/pe_const_read.py d:00d7a270), and the direction it scales is the
// **unit** direction 0084BF00 stores at buffer+10h, so this is 5 cm out of the
// struck surface and not a time step.
constexpr float kBlastCentreBackOff = 0.05f;

float dot3(const float a[3], const float b[3]) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float length3(const float v[3]) noexcept {
    return std::sqrt(dot3(v, v));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameGunneryHost::Impl {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in, GameMissionLuaHost& lua_in)
        : log(log_in), units(units_in), lua(lua_in) {}

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }

    GameHostLog& log;
    GameUnitsHost& units;
    GameMissionLuaHost& lua;
    GameShipAiHost* ship_ai{nullptr};

    // One entry per created unit, index aligned with GameUnitsHost.
    struct UnitState {
        GameGunneryUnitRow row;
        // The gunnery pass object at unit+6DCh, as its live fields.
        bool attached{false};
        bool enabled{false};             // pass+58h
        float throttle{0.0f};            // pass+6Ch
        bool sweep_suppressed{false};    // pass+59h
        bsp::UnitGunneryCategoryState category{};
        int bridge_countdown{0};         // adapter+8h
        bool allow_fire_cache{false};    // adapter+0Ch
        // The visibility cache at pass+68h: one entry per candidate, 0Ch bytes.
        struct VisibilityEntry {
            std::size_t target{0};
            bool visible{false};
            float ttl{0.0f};
        };
        std::vector<VisibilityEntry> visibility;
        // Per class descriptor, read once out of the authored row.
        float hull_length{0.0f};
        float hull_width{0.0f};
        float hull_height{0.0f};
        float armour{0.0f};
        float max_health{0.0f};
        float health{0.0f};
        // The twelve category records at unit+394h and the ranges at unit+430h.
        std::array<std::vector<std::size_t>, bsp::kUnitGunneryCategoryCount> category_guns{};
        std::array<float, bsp::kUnitGunneryCategoryCount> category_ranges{};
        float artillery_max_range{0.0f};  // unit+490h, 00956E43
        float any_weapon_max_range{0.0f}; // unit+494h, 00956E59
        bool dead{false};
        // The kill attribution block at victim+2C4h..+2E8h, as 0077CE60 leaves it.
        bsp::KillAttributionFields attribution{};
        std::size_t last_attacker{0};   // victim+2C4h, one based
        // The per-step scratch the pass fills.
        std::vector<bsp::GunneryCandidate> candidates;
        std::vector<int> order;
        std::vector<std::size_t> candidate_units;
        std::size_t fire_target{0};      // one based
        std::size_t command_target{0};   // one based
    };

    std::vector<UnitState> unit_state;
    std::vector<GameGunRow> guns;
    std::vector<GameDeviceClassRow> devices;
    std::vector<GameBulletClassRow> bullets;
    std::vector<GameProjectileRow> shots;
    // Packet cc8_torpedo_closest_approach: one row per swimming round, kept
    // after the round is erased.
    std::vector<GameTorpedoApproachRow> torpedo_approaches;
    // Packet cc8_dive_glide: one row per released bomb, kept after the erase.
    std::vector<GameBombImpactRow> bomb_impacts;
    // Packet cc8_torpedo_aim_census: the ordered-target half of one record.
    void fill_ordered_fields(GameTorpedoApproachRow& rec,
                             const GameProjectileRow& row) const {
        rec.ordered_name = unit_name_or_index(row.ordered_target);
        rec.ordered_min_distance = row.ordered_min_distance;
        rec.ordered_min_time = row.ordered_min_time;
        rec.crossing_angle = row.crossing_angle;
        rec.drop_owner_heading = row.drop_owner_heading;
        rec.drop_target_heading = row.drop_target_heading;
        rec.drop_crossing_angle = row.drop_crossing_angle;
        rec.drop_ordered_range = row.drop_ordered_range;
        if (row.ordered_target != 0 && row.ordered_min_distance >= 0.0f) {
            const float dx = row.target_pos_at_min[0] - row.target_pos_release[0];
            const float dz = row.target_pos_at_min[2] - row.target_pos_release[2];
            rec.target_travel = std::sqrt(dx * dx + dz * dz);
        }
    }
    std::string unit_name_or_index(std::size_t one_based) const {
        if (one_based == 0) return std::string("-");
        const std::size_t i = one_based - 1;
        if (i < unit_state.size() && !unit_state[i].row.name.empty()) {
            return unit_state[i].row.name;
        }
        return std::string("unit#") + std::to_string(one_based);
    }
    GameGunnerySummary summary{};

    // 00E19BF8, built by 00727BD0 from the authored lists at 00E092C8.
    std::vector<int> rank_table;
    float think_time{bsp::kInstalledWeaponDirectorThinkTime};
    float clock_seconds{0.0f};
    // Packet cc9_aa_targeting: observation only, gated by BSP_AA_TRACE_UNIT and
    // BSP_AA_TRACE_TARGET (see aa_trace_unit()/aa_trace_target() below). Unset,
    // nothing is recorded or printed.
    struct AaTargetStats {
        unsigned long long scored{0};
        unsigned long long accepted{0};
        unsigned long long range_rejects{0};
        unsigned long long gun_range_rejects{0};
        unsigned long long assigns{0};
        float min_dist{-1.0f};
        float min_reject_dist{-1.0f};
        float reject_range_at_min{0.0f};
        float alt_at_min{0.0f};
        std::string ship_at_min;
        int category_at_min{-1};
        // The aim-and-fire side, per gun-tick with this target held.
        unsigned long long targeted_ticks{0};
        unsigned long long angle_refusals{0};
        unsigned long long shots{0};
        float refused_vert_max{-1000.0f};
        float accepted_vert_max{-1000.0f};
        float shot_range_min{-1.0f};
    };
    std::map<std::size_t, AaTargetStats> aa_target_stats;
    static bool aa_trace_matches(const std::string& name) {
        const std::string& list = aa_trace_unit_name();
        if (list.empty()) return false;
        std::size_t start = 0;
        while (start <= list.size()) {
            const std::size_t comma = list.find(',', start);
            const std::string item = list.substr(start,
                comma == std::string::npos ? std::string::npos : comma - start);
            if (item == name) return true;
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
        return false;
    }
    static const std::string& aa_trace_unit_name() {
        static const std::string v = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            std::string out;
            if (_dupenv_s(&text, &bytes, "BSP_AA_TRACE_UNIT") == 0 && text != nullptr)
                out = text;
            std::free(text);
            return out;
        }();
        return v;
    }
    static const std::string& aa_trace_target_prefix() {
        static const std::string v = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            std::string out;
            if (_dupenv_s(&text, &bytes, "BSP_AA_TRACE_TARGET") == 0 && text != nullptr)
                out = text;
            std::free(text);
            return out;
        }();
        return v;
    }
    // "MinRange", classDesc+58h (0070C030), per bullet class; 00729B90 reads it.
    std::map<int, float> bullet_min_range;
    unsigned long long aa_window_rejects{0};
    unsigned long long aa_armour_rejects{0};
    unsigned long long aa_min_range_skips{0};
    // Per gun index: evaluations where 00865773's minimum-air-range skip applies.
    std::map<std::size_t, unsigned long long> aa_min_range_skips_by_gun;
    std::vector<std::string> aa_cand_lines;
    bool aa_changed{false};
    unsigned long long step_index{0};
    // 00BD2F10's stream. One sequence for the whole run so a rerun repeats.
    std::uint32_t rng{0x9E3779B9u};

    float random_range_00bd2f10(float low, float high) {
        rng = rng * 1664525u + 1013904223u;
        const float unit = static_cast<float>((rng >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }

    // Packet cc9_rng_streams: the gunnery draws, each named by its consumer.
    // The image draws all of them from 00BD2F10 on stream ECX=1, the one the
    // pilot bots, gun bots and projectiles share (docs/RANDOM_STREAMS.md), and
    // the default here keeps that shape: one shared generator, in the same
    // order as before this packet, so a default run is unchanged.
    //
    // BSP_GUNNERY_RNG_STREAMS=1 is a MEASUREMENT SUBSTITUTION, never on in a
    // reference run: each (consumer, a, b) key gets its own deterministic
    // generator seeded from the run seed and the key, so a pair that changes
    // one unit's behaviour leaves every other key's draws where they were.
    enum class Draw : std::uint32_t {
        visibility_ttl = 1,   // 00864D90, key (shooter unit, target unit)
        fire_stagger = 2,     // 0072D2C0 via the fire request, key (gun, 0)
        hit_effect = 3,       // ship-hit fire/flood chance, key (victim unit, 0)
        hull_damage = 4,      // 00470510's base, key (gun, victim unit)
        blast_damage = 5,     // 0084BAD0's blast, key (gun, 0)
        aim_error = 6,        // 006DEFF0's three draws and 006DF5C6's period, key (gun, 0)
        ship_ai_torpedo = 7,  // 009F0AD0 and the brain timer draws, key (unit, 0)
        death_mode = 8,       // 007CA914, the plane death-mode choice, key (unit, 0)
        death_delay = 9,      // 007BBFA0's ExplosionExplosionDelay (stream 0), key (unit, 0)
        aim_wander = 10,      // 009FA620 / 009FA7E0, the dogfight aim distortion, key (unit, 0)
        aim_point = 11,       // 00816650's hull-box draws for the artillery bot, key (gun, 0)
        bullet_throw = 12,    // 00730557 / 00730575, the throw cone, key (gun, 0)
        torpedo_gyro = 13,    // 00900830 / 0090083E, the launch heading jitter, key (gun, 0|1)
    };
    unsigned long long next_projectile_serial{0};
    static bool rng_streams_enabled() {
        static const bool on = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            bool value = false;
            if (_dupenv_s(&text, &bytes, "BSP_GUNNERY_RNG_STREAMS") == 0 && text != nullptr)
                value = text[0] == '1';
            std::free(text);
            return value;
        }();
        return on;
    }
    std::map<std::uint64_t, std::uint32_t> rng_by_key;
    // Packet cc9_gun_ballistics: 006DF1F0 leaves bot+74h = 0 and bot+78h = 1, so
    // the first tick rerolls.
    struct AimErrorState {
        bsp::GunAimAngles previous{};
        bsp::GunAimAngles target{};
        float period{1.0f};
        float countdown{0.0f};
    };
    std::map<std::size_t, AimErrorState> aim_error_by_gun;
    // Packet cc9_surface_gunnery_reference: 006DF520 step 4's per-bot point.
    struct ArtilleryAimPoint {
        float timer_b4{-1.0f};
        std::size_t target{static_cast<std::size_t>(-1)};
        std::array<float, 3> body{};   // bot+A8h..+B0h
    };
    std::map<std::size_t, ArtilleryAimPoint> artillery_aim_by_gun;
    unsigned long long artillery_aim_points{0};
    unsigned long long shell_mesh_hits{0};
    unsigned long long summary_zero_damage_attributions_skipped{0};   // packet cc9_kill_credit
    // 006DF6D7-006DF7B5 then 006DF7BB-006DF7E7: the body point on the target,
    // refreshed every TargetPointRefreshTime, carried to world by its pose.
    // robots.lua ArtilleryGunnerBot, by skill 0 Stun .. 5 Elite:
    // {SectionTargetChance, EngineRoomWeight, MagazineWeight, FueltankWeight}
    static constexpr float kArtillerySectionRows[6][4] = {
        {0.0f, 1.0f, 0.1f, 0.1f},   // Stun
        {0.2f, 1.0f, 0.1f, 0.1f},   // SPNormal
        {1.0f, 0.5f, 1.0f, 1.0f},   // SPVeteran
        {0.5f, 1.0f, 0.8f, 0.8f},   // MPNormal
        {0.8f, 1.0f, 1.0f, 1.0f},   // MPVeteran
        {1.0f, 0.5f, 1.0f, 1.0f},   // Elite
    };
    unsigned long long artillery_section_points{0};
    void artillery_aim_point(std::size_t gun_index, std::size_t target, float dt,
                             float out[3], std::size_t owner = static_cast<std::size_t>(-1)) {
        ArtilleryAimPoint& st = artillery_aim_by_gun[gun_index];
        if (st.target != target) { st.target = target; st.timer_b4 = -1.0f; }
        st.timer_b4 -= dt;
        if (st.timer_b4 <= 0.0f) {
            st.timer_b4 = 5.0f;   // TargetPointRefreshTime, 5 in every row of this installation
            const UnitState& t = unit_state[target];
            bsp::LeadAimHullExtents hull;
            hull.length = t.hull_length;
            hull.width = t.hull_width;
            hull.height = t.hull_height;
            bsp::ShipLeadRandomDraws draws;
            bsp::ShipLeadSections sections{};
            float row[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            bool sections_live = false;
            if (kShipSectionPointsBound && owner != static_cast<std::size_t>(-1)) {
                int level = units.skill_level(owner);
                if (level < 0 || level > 5) level = 1;
                for (int k = 0; k < 4; ++k) row[k] = kArtillerySectionRows[level][k];
                const ShipModelSlots& model = ship_model_slots(t.row.type_id);
                sections.engine_room.present = model.section_present[0];
                sections.engine_room.point = model.section_point[0];
                sections.magazine.present = model.section_present[1];
                sections.magazine.point = model.section_point[1];
                sections.fuel_tank.present = model.section_present[2];
                sections.fuel_tank.point = model.section_point[2];
                sections_live = true;
            }
            // 0081667C: the roll is drawn only when the chance is positive
            // (00816659 skips the draw otherwise).
            if (row[0] > 0.0f) {
                draws.section_roll = draw(Draw::aim_point, gun_index, 0, 0.0f, 1.0f);
            }
            float total = 0.0f;
            if (sections_live && row[0] > draws.section_roll) {
                total = (sections.engine_room.present ? row[1] : 0.0f)
                    + (sections.magazine.present ? row[2] : 0.0f)
                    + (sections.fuel_tank.present ? row[3] : 0.0f);
            }
            if (total > 1e-4f) {
                draws.pick = draw(Draw::aim_point, gun_index, 0, 0.0f, total - 1e-4f);  // 00816796
                ++artillery_section_points;
            } else {
                draws.box_x = draw(Draw::aim_point, gun_index, 0, -0.6f, 0.6f);     // 00816883
                draws.box_y = draw(Draw::aim_point, gun_index, 0, 0.0f, 0.6f);      // 008168A0
                draws.box_z = draw(Draw::aim_point, gun_index, 0, -0.6f, 0.6f);     // 008168D5
            }
            st.body = bsp::ship_lead_point_00816650(sections, hull, {0.6f, 0.6f, 0.6f},
                {0.0f, 0.0f, 0.0f}, row[0], row[1], row[2], row[3], draws, true, true, true);
            ++artillery_aim_points;
            done("Ship::lead_point_00816650", 0x00816650u);
        }
        float r[3], u[3], f[3], o[3];
        unit_pose(target, r, u, f, o);
        for (int i = 0; i < 3; ++i) {
            out[i] = o[i] + r[i] * st.body[0] + u[i] * st.body[1] + f[i] * st.body[2];
        }
    }
    // 0072C6A0's slots, and 00729BC0's dispatch on the projectile kind of the
    // ammunition in use. A sub-type 6 gun answers a plane with its SECOND
    // ammunition entry (00729BC0's variant 1, +74h+7Ch), a Flak round that the
    // AAFlakBot drives. This host loads only the first entry, an Artillery round
    // (V0 300, gravity on), so a host sub-type 6 shot at anything is an
    // ArtilleryGunnerBot shot. Until the second entry is loaded, the AA aim law
    // applies to sub-types 1 and 5 alone. docs/GUN_BALLISTICS.md.
    bool aa_bot_aims(int category, std::size_t) const {
        return category == 1 || category == 5;
    }
    bool artillery_bot_aims(int category, std::size_t) const {
        return category == 2 || category == 3 || category == 4 || category == 6
            || category == 9;
    }
    // 006DF59F..006DF651: count down, reroll 006DEFF0 with a fresh U(3, 8)
    // period on expiry, and interpolate from the old pair to the new one.
    bsp::GunAimAngles aim_error_tick(std::size_t gun_index, std::size_t owner, float dt) {
        AimErrorState& st = aim_error_by_gun[gun_index];
        st.countdown -= dt;
        if (st.countdown < 0.0f) {
            int level = units.skill_level(owner);
            if (level < 0 || level > 5) level = 1;   // the host's default, SPNormal
            const bsp::ArtilleryGunnerAimErrorLevel& row = kArtilleryGunnerLevels[level];
            st.period = draw(Draw::aim_error, gun_index, 0, 3.0f, 8.0f);   // 006DF5C6
            st.countdown = st.period;
            const float t = draw(Draw::aim_error, gun_index, 0, 0.0f, 1.0f);  // 006DEFF3
            const float weight = t != 0.0f
                ? static_cast<float>(std::exp2(static_cast<double>(row.power)
                    * std::log2(static_cast<double>(t))))
                : 0.0f;
            const float span = draw(Draw::aim_error, gun_index, 0, 0.0f,
                row.max_angle_error);                                          // 006DF0B8
            const float radius = span * weight;
            const float roll = draw(Draw::aim_error, gun_index, 0, 0.0f,
                6.28318548202514648438f);                                      // 006DF0DC
            st.previous = st.target;
            st.target.horz = std::sin(roll) * radius;
            st.target.vert = std::cos(roll) * radius;
            ++aim_error_rerolls;
            done("GunBot::reroll_aim_error_envelope_006deff0", 0x006deff0u);
        }
        bsp::ArtilleryGunnerBotErrorEnvelope env;
        env.start_horz = st.target.horz;
        env.end_horz = st.previous.horz;
        env.start_vert = st.target.vert;
        env.end_vert = st.previous.vert;
        return bsp::gun_bot_muzzle_error_006df520(env, st.period, st.countdown);
    }
    unsigned long long aa_direct_aims{0};
    unsigned long long artillery_arc_aims{0};
    unsigned long long aim_error_rerolls{0};
    unsigned long long no_gravity_shots{0};
    unsigned long long plane_trigger_ticks{0};
    // Packet cc9_player_gun_seat.
    unsigned long long seat_messages{0};
    unsigned long long seat_handovers{0};
    unsigned long long seat_returns{0};
    unsigned long long seat_held_ticks{0};
    unsigned long long seat_trigger_ticks{0};
    void apply_gun_aim_message(std::size_t unit, const GunAimMessage79& message);
    // ShipGlobals.AAGunnerErrorModifier.CalcTargetPosTimeAddFix / AddMul, the
    // gameplay settings +758h / +75Ch 00901C20 adds to its time of flight
    // (docs/GAMEPLAY_SETTINGS.md; loader defaults 0, installed 0.05 / 0.1).
    float aa_time_add_fix{0.0f};
    float aa_time_add_mul{0.0f};
    bool aa_time_logged{false};
    static bool env_flag(const char* name) {
        char* text = nullptr;
        std::size_t bytes = 0;
        bool value = false;
        if (_dupenv_s(&text, &bytes, name) == 0 && text != nullptr) value = text[0] == '1';
        std::free(text);
        return value;
    }
    static bool aa_time_bias_off() {
        static const bool on = env_flag("BSP_AA_TIME_BIAS_OFF");
        return on;
    }
    // MEASUREMENT option, never in a reference run: keep the shooter's own
    // velocity in the intercept (the host's rounds, like 006E8430's, do not
    // inherit it), to separate the relative-motion term from the time law.
    static bool aa_keep_shooter_velocity() {
        static const bool on = env_flag("BSP_AA_NO_SHOOTER_VEL");
        return on;
    }
    unsigned long long intercept_solves{0};
    // Packet cc9_aa_lead: Bullet[2] of a sub-type 6 gun, by gun index.
    struct SecondAmmo {
        int bullet_class{-1};
        float muzzle_speed{0.0f};
        float max_range{0.0f};
        float min_range{0.0f};
    };
    std::map<std::size_t, SecondAmmo> second_ammo_by_gun;
    unsigned long long dp_air_rounds{0};
    unsigned long long aa_negative_halvings{0};
    unsigned long long water_depth_kills{0};   // packet cc9_water_surface_law
    unsigned long long flak_locks{0};           // packet cc9_flak_proximity_burst
    unsigned long long flak_bursts{0};
    // Packet cc9_gun_barrel_count: the device model's muzzle list, one load
    // per device class (the image does it once per weapon class, 007325A0).
    struct DeviceFirePoints {
        bool loaded{false};          // the model opened and parsed
        std::string mesh;
        std::size_t fire_items{0};
        bsp::GunFireMuzzleList list; // class+98h..A0h
        int records{-1};             // the OFF count, Bullet records
        std::size_t guns{0};         // guns built on this device
        // Packet cc9_muzzle_offsets: the model's Hierarchy and the two names
        // 0072E9E2/0072EA15 look up; -1 when absent.
        std::vector<bsp::HierarchyItem> nodes;
        int barrel_node{-1};
        int base_node{-1};
    };
    std::map<int, DeviceFirePoints> fire_points_by_device;
    unsigned long long barrel_guns_from_model{0};
    unsigned long long barrel_guns_changed{0};
    unsigned long long barrel_guns_fallback{0};
    // DIAGNOSTIC, read-only, packet cc9_e2_zero_ordnance: with BSP_DEATH_TABLE
    // set, every death prints one "death row" line (damaging hits by category,
    // the killer and ranges). No gameplay term reads these fields.
    struct DeathTableRow {
        float first_damage{-1.0f};
        int hits[12]{};
        float damage[12]{};
        int last_category{-1};
        std::size_t last_gun{0};
        bool last_blast{false};
    };
    std::map<std::size_t, DeathTableRow> death_table;
    static bool death_table_enabled() {
        static const bool on = [] {
            char* text = nullptr;
            std::size_t length = 0;
            const bool set = _dupenv_s(&text, &length, "BSP_DEATH_TABLE") == 0
                && text != nullptr && text[0] != '\0' && text[0] != '0';
            std::free(text);
            return set;
        }();
        return on;
    }
    DeviceFirePoints& device_fire_points(int device);
    // Packet cc9_ship_platform_attachment: the ship model's Aux items, one load
    // per vehicle class.
    struct ShipModelSlots {
        bool loaded{false};
        std::string mesh;
        std::vector<bsp::GunFirePointItem> items;
        long long logged_unit{-1};   // the one unit whose mounts are logged
        bool has_box{false};         // the model's BoundingBox, model frame
        std::array<float, 6> box{};
        // Packet cc9_hull_sections: the GeomMesh triangles and the three
        // section points, engine room (5), magazine (8), fuel tank (6).
        bool has_mesh{false};
        std::vector<std::array<float, 3>> tris;   // three vertices per triangle
        std::array<float, 6> mesh_box{};
        bool section_present[3]{false, false, false};
        std::array<float, 3> section_point[3]{};
    };
    std::map<int, ShipModelSlots> ship_slots_by_class;
    // 0072F6E0's per-gun cache: (gun, target) -> clear.
    std::map<std::pair<std::size_t, std::size_t>, bool> line_of_fire_cache;
    unsigned long long line_of_fire_queries{0};
    unsigned long long line_of_fire_blocked{0};
    unsigned long long line_of_fire_refusals{0};
    unsigned long long line_of_fire_aabb_tests{0};
    unsigned long long narrowphase_box_0085cdb0{0};
    // 0072CDD0 with 0098B130: true when the raised segment's nearest unit hit
    // (firer and target excluded) is on the firer's side.
    bool line_of_fire_blocked_0072cdd0(std::size_t owner, std::size_t target,
        const float muzzle[3]) {
        float target_pos[3];
        {
            float r[3], u[3], f[3];
            unit_pose(target, r, u, f, target_pos);
        }
        const float from[3] = {muzzle[0], muzzle[1] + 5.0f, muzzle[2]};      // 00D7A370
        const float to[3] = {target_pos[0], std::max(5.0f, target_pos[1] + 5.0f),  // 00CE3850
            target_pos[2]};
        const float dir[3] = {to[0] - from[0], to[1] - from[1], to[2] - from[2]};
        float best = 2.0f;
        std::size_t best_unit = static_cast<std::size_t>(-1);
        for (std::size_t u = 0; u < unit_state.size(); ++u) {
            if (u == owner || u == target || unit_state[u].dead) continue;
            const UnitState& st = unit_state[u];
            // The unit's box: its model's BoundingBox (masts and superstructure
            // included) when the class model reads, else the class hull box.
            float ctr[3] = {0.0f, 0.0f, 0.0f};
            float ext[3] = {st.hull_width * 0.5f, st.hull_height * 0.5f,
                st.hull_length * 0.5f};
            if (st.row.type_id >= 0) {
                const ShipModelSlots& model = ship_model_slots(st.row.type_id);
                if (model.has_box) {
                    for (int a = 0; a < 3; ++a) {
                        ctr[a] = (model.box[a] + model.box[a + 3]) * 0.5f;
                        ext[a] = (model.box[a + 3] - model.box[a]) * 0.5f;
                    }
                }
            }
            if (ext[0] <= 0.0f || ext[2] <= 0.0f) continue;
            float r[3], up[3], f[3], o[3];
            unit_pose(u, r, up, f, o);
            if constexpr (kAabb0085cdb0Bound) {
                // 0098B130: the node's world AABB (0098A750) then 0085CDB0.
                const bsp::CameraMatrix m{r[0], r[1], r[2], 0.0f, up[0], up[1], up[2], 0.0f,
                    f[0], f[1], f[2], 0.0f, o[0], o[1], o[2], 1.0f};
                bsp::SpatialLocalBox local;
                local.centre = {ctr[0], ctr[1], ctr[2]};
                local.extent = {ext[0], ext[1], ext[2]};
                const bsp::HitQueryBounds world = bsp::spatial_world_bounds_0098a750(local, m);
                bsp::HitQueryPoint hit_point;
                float entry = 0.0f;
                if (bsp::segment_box_hit_0085cdb0(world, {from[0], from[1], from[2]},
                        {to[0], to[1], to[2]}, hit_point, &entry)
                    && entry < best) {
                    best = entry;
                    best_unit = u;
                }
                ++line_of_fire_aabb_tests;
                continue;
            }
            // Segment in the unit's frame, then a slab test on the oriented box.
            const float rel[3] = {from[0] - o[0], from[1] - o[1], from[2] - o[2]};
            const float* axes[3] = {r, up, f};
            float t0 = 0.0f, t1 = 1.0f;
            bool hit = true;
            for (int a = 0; a < 3 && hit; ++a) {
                const float p = rel[0] * axes[a][0] + rel[1] * axes[a][1] + rel[2] * axes[a][2]
                    - ctr[a];
                const float d = dir[0] * axes[a][0] + dir[1] * axes[a][1] + dir[2] * axes[a][2];
                if (std::fabs(d) < 1e-9f) {
                    if (p < -ext[a] || p > ext[a]) hit = false;
                    continue;
                }
                float ta = (-ext[a] - p) / d, tb = (ext[a] - p) / d;
                if (ta > tb) std::swap(ta, tb);
                t0 = std::max(t0, ta);
                t1 = std::min(t1, tb);
                if (t0 > t1) hit = false;
            }
            if (hit && t0 < best) { best = t0; best_unit = u; }
        }
        return best_unit != static_cast<std::size_t>(-1)
            && unit_state[best_unit].row.side == unit_state[owner].row.side;
    }
    unsigned long long mounts_from_model{0};
    unsigned long long mounts_missing{0};
    ShipModelSlots& ship_model_slots(int type_id);
    // The ship's model mesh for the shell hit test, when bound and loaded.
    const ShipModelSlots* ship_mesh_of(std::size_t index) {
        if (!kShellHullHitTestBound || index >= unit_state.size()) return nullptr;
        if (!units.unit_is_kind_of(index, bsp::kUnitGunneryKindShipBase)) return nullptr;
        const int type_id = unit_state[index].row.type_id;
        if (type_id < 0) return nullptr;
        const ShipModelSlots& model = ship_model_slots(type_id);
        return model.has_mesh ? &model : nullptr;
    }
    // The gun's firing point: its mount carried to world by the ship pose when
    // bound and known, otherwise the unit origin raised by the class Height.
    void gun_muzzle_point(const GameGunRow& gun, const UnitState& state,
        const float right[3], const float up[3], const float forward[3],
        const float origin[3], float out[3]) const {
        if (kShipPlatformAttachmentBound && gun.mount_known) {
            for (int i = 0; i < 3; ++i) {
                out[i] = origin[i] + right[i] * gun.mount_local[0]
                    + up[i] * gun.mount_local[1] + forward[i] * gun.mount_local[2];
            }
            return;
        }
        out[0] = origin[0];
        out[1] = origin[1] + state.hull_height;
        out[2] = origin[2];
    }
    // Packet cc9_muzzle_offsets (docs/MUZZLE_OFFSETS.md): 00730160's shot origin,
    // TransformAffinePoint(class+98h[barrel], [gun+3CCh] world) with the node pose
    // 00859550 writes. False when the gun has no mount, no loaded offsets or no
    // usable node chain; the caller then keeps the mount point, which is the
    // image's own empty-list fallback (00730899, the root translation).
    bool gun_barrel_muzzle_world_00730762(const GameGunRow& gun, int barrel,
        const float right[3], const float up[3], const float forward[3],
        const float origin[3], float out[3]) {
        if (!kMuzzleOffsetsBound || !kShipPlatformAttachmentBound || !gun.mount_known) {
            return false;
        }
        const auto found = fire_points_by_device.find(gun.device_class);
        if (found == fire_points_by_device.end()) return false;
        const DeviceFirePoints& fp = found->second;
        if (!fp.loaded || fp.list.offsets.empty() || fp.nodes.empty()) return false;
        const std::size_t count = fp.nodes.size();
        const bool have_barrel = fp.barrel_node > 0;
        // 0072EE65..0072EE8F: barrel, then base, then the model's first node.
        const int pick = have_barrel ? fp.barrel_node : (fp.base_node > 0 ? fp.base_node : 0);
        // The root's local is the platform frame (0072E9A1), a translation.
        bsp::CameraMatrix root_local{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, gun.mount_local[0], gun.mount_local[1],
            gun.mount_local[2], 1.0f};
        bsp::CameraMatrix barrel_local{};
        if (have_barrel) {
            if (!fp.nodes[static_cast<std::size_t>(fp.barrel_node)].matrix) return false;
            barrel_local = *fp.nodes[static_cast<std::size_t>(fp.barrel_node)].matrix;
        }
        // CONVENTION BRIDGE: the host's horizontal angle is the negative of the
        // image's gun+480h (docs/MUZZLE_OFFSETS.md section 2), so the image pose
        // is built from -horz to point the barrel where the host fires.
        bsp::turning_gun_apply_angles_00859550(-kGunHorzSign * gun.angles.horz, gun.angles.vert,
            have_barrel, root_local, barrel_local);
        const bsp::CameraMatrix ship{right[0], right[1], right[2], 0.0f,
            up[0], up[1], up[2], 0.0f, forward[0], forward[1], forward[2], 0.0f,
            origin[0], origin[1], origin[2], 1.0f};
        // 00B6DB70: world = local * parent world, walked from the picked node up
        // to the root (item 0), whose parent is the ship.
        std::vector<int> chain;
        int at = pick;
        while (at != 0) {
            if (chain.size() > count) return false;
            chain.push_back(at);
            const std::uint32_t parent = fp.nodes[static_cast<std::size_t>(at)].parent;
            if (parent >= count) return false;
            at = static_cast<int>(parent);
        }
        bsp::CameraMatrix world{};
        bsp::multiply_camera_matrices_00413920(world, root_local, ship);
        for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
            const int node = *it;
            bsp::CameraMatrix local{};
            if (have_barrel && node == fp.barrel_node) {
                local = barrel_local;
            } else if (fp.nodes[static_cast<std::size_t>(node)].matrix) {
                local = *fp.nodes[static_cast<std::size_t>(node)].matrix;
            } else {
                return false;
            }
            bsp::CameraMatrix next{};
            bsp::multiply_camera_matrices_00413920(next, local, world);
            world = next;
        }
        bsp::GunMuzzleMount mount;
        mount.node_world = world;
        mount.muzzle_offsets = fp.list.offsets;
        const int n = static_cast<int>(fp.list.offsets.size());
        mount.barrel_index = ((barrel % n) + n) % n;
        const std::array<float, 3> p = bsp::gun_muzzle_world_position_00730762(mount);
        for (int i = 0; i < 3; ++i) out[i] = p[i];
        last_muzzle_node_world = world;     // DIAGNOSTIC for BSP_MUZZLE_TRACE
        last_muzzle_pick = pick;
        return true;
    }
    bsp::CameraMatrix last_muzzle_node_world{};
    int last_muzzle_pick{-1};
    // DIAGNOSTIC: BSP_MUZZLE_TRACE=<unit-name prefix> prints every placed shot of
    // that unit with the mount, the muzzle, the node facing and the target bearing.
    static const std::string& muzzle_trace_prefix() {
        static const std::string v = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            std::string out;
            if (_dupenv_s(&text, &bytes, "BSP_MUZZLE_TRACE") == 0 && text != nullptr)
                out = text;
            std::free(text);
            return out;
        }();
        return v;
    }
    // Set by run_projectiles around apply_hit / apply_impact_blast so a round's
    // own class (the second ammunition) prices its damage; -1 means the gun's.
    int round_bullet_class{-1};
    const SecondAmmo* dp_air_ammo(std::size_t gun_index, int category,
                                  std::size_t target) const {
        if (!kDualPurposeSecondAmmoBound || category != 6) return nullptr;
        if (!units.unit_is_kind_of(target, bsp::kUnitGunneryKindPlaneBase)) return nullptr;
        const auto it = second_ammo_by_gun.find(gun_index);
        return it != second_ammo_by_gun.end() && it->second.bullet_class >= 0
            && it->second.muzzle_speed > 0.0f ? &it->second : nullptr;
    }
    unsigned long long plane_gun_rounds{0};
    // Per gun index: hits landed and damage dealt, for the gunrow lines.
    std::map<std::size_t, std::pair<unsigned long long, double>> hits_by_gun;
    float draw(Draw purpose, std::size_t a, std::size_t b, float low, float high) {
        if (!rng_streams_enabled()) return random_range_00bd2f10(low, high);
        const std::uint64_t key = (static_cast<std::uint64_t>(purpose) << 56)
            ^ (static_cast<std::uint64_t>(a & 0xFFFFFFu) << 28)
            ^ static_cast<std::uint64_t>(b & 0xFFFFFFFu);
        auto it = rng_by_key.find(key);
        if (it == rng_by_key.end()) {
            // splitmix64 of the run seed and the key: a fixed, key-local start.
            std::uint64_t z = key + 0x9E3779B97F4A7C15ull * (0x9E3779B9ull + 1u);
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
            z ^= z >> 31;
            it = rng_by_key.emplace(key, static_cast<std::uint32_t>(z)).first;
        }
        std::uint32_t& state = it->second;
        state = state * 1664525u + 1013904223u;
        const float unit = static_cast<float>((state >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }

    const GameDeviceClassRow* device(int id) const {
        for (const GameDeviceClassRow& row : devices) {
            if (row.id == id) return &row;
        }
        return nullptr;
    }
    const GameBulletClassRow* bullet(int id) const {
        for (const GameBulletClassRow& row : bullets) {
            if (row.id == id) return &row;
        }
        return nullptr;
    }

    // --- authored data, through the live interpreter ------------------------
    void flatten_class_tables(const std::vector<int>& class_ids);
    int flat(int class_id, const char* key, int fallback) {
        return lua.read_vehicle_class_integer(class_id, "BSPGun", key, fallback);
    }
    float flat_scaled(int class_id, const char* key, float scale, float fallback) {
        const int raw = flat(class_id, key, 0x7FFFFFFF);
        if (raw == 0x7FFFFFFF) return fallback;
        return static_cast<float>(raw) / scale;
    }

    // Packet cc8_gunnery_host: both take the first unit index to build, so a
    // later spawn batch registers only the units it added. `guns` is appended
    // to and never cleared, so re-running either over an already-built unit
    // would duplicate its guns and re-seed its throttle, health and category
    // state. See GameGunneryHost::register_new_units_00864bd0.
    void build_guns(std::size_t first_unit);
    void build_rank_table();
    void attach_passes(std::size_t first_unit);
    // How many units of the units host this object has already registered.
    // The units host only ever appends, so [0, built_units) is settled.
    std::size_t built_units{0};
    void refresh_build_summary();
    // Packet cc9_gunnery_host: already-built units whose stored ordnance mask a
    // later batch found different from the recomputed aggregate, which only a
    // torpedo drop's kind 2Bh clear can cause. Counted in both modes of
    // restore_all_ordnance_enabled() below; re-stored only when it is on.
    unsigned long long ordnance_rearms{0};

    void run_gunnery_pass(std::size_t index, float dt);
    void refresh_command_targets();
    // 0071EBF0 per unit, one based; 0 when the unit has no current command
    // target. Rebuilt only when the command row count or the unit count
    // changes, which is at load and on a new order, not every tick.
    std::vector<std::size_t> command_target_by_unit;
    std::size_t command_rows_resolved{static_cast<std::size_t>(-1)};
    // The second half of the cache key: a row flipping current without the
    // vector growing has to re-resolve too. See refresh_command_targets.
    std::size_t command_rows_current{static_cast<std::size_t>(-1)};
    std::size_t command_targets_resolved{0};
    void run_gun_aim_and_fire(float dt);
    void run_projectiles(float dt);
    // Publishes the rows 00A08460 reads into the process-wide table the AI
    // coordinator holds. docs/AI_TARGET_WEIGHT_TERMS.md term 2.
    void publish_ai_weapon_facts();
    void apply_hit(std::size_t shooter, std::size_t gun_row, std::size_t victim,
        const float point[3], const float direction[3],
        const bsp::HitRecord* blast_record = nullptr);
    // 0084BC60 step 7 (0084BE28..0084BEE3) and the gather behind 0084BAD0: the
    // radial burst every class with a Blast table makes on impact, and the one
    // that carries a torpedo's warhead. docs/TORPEDO_WARHEAD.md.
    void apply_impact_blast(std::size_t shooter, std::size_t gun_row,
        const float point[3], const float direction[3]);
    void kill_unit(std::size_t victim);

    // --- rule (c), the sensor pass ------------------------------------------
    // docs/RECON_SENSOR_PASS_BINDING.md. 008073C0 runs once per tick over every
    // side, not once per firing unit, so the state and its per-class sensor
    // records live on the host and the step runs in fixed_step.

    // One 008082A0 record per distinct `ReconClass` id seen, plus the 56
    // addressable rows 008048A0 indexes with sensor_list_index_008085aa. The
    // record is held by pointer so that adding a class never moves the entry
    // arrays a live GunneryReconSensorRow points at.
    struct ReconClassRecord {
        bsp::SensorClassTable table;
        std::array<std::vector<bsp::ReconSensorEntry>, bsp::kSensorListCount> entries;
        std::array<bsp::GunneryReconSensorRow, bsp::kSensorListCount> rows;
    };
    std::map<int, std::unique_ptr<ReconClassRecord>> recon_classes;
    // Per unit, resolved once from the authored row: the class's `ReconClass`
    // record and the SQUARE of its `ReconModifier`, which is what class+B8h
    // holds (009623CF FMUL ST0,ST0 before the 009623D9 store).
    std::vector<const ReconClassRecord*> unit_recon_record;
    std::vector<float> unit_recon_modifier_sq;
    bsp::ReconSensorPassState recon_pass;
    // [00F874B8], the refresh countdown, and slot+2Ch, the last-pass stamp the
    // pass subtracts from the clock to get its own dt. Starting the countdown
    // at zero makes the first tick run a pass, as a freshly zeroed native
    // timer does.
    float recon_refresh_timer{0.0f};
    float recon_last_pass_seconds{0.0f};
    unsigned long long recon_classes_missing{0};   // `ReconClass` absent from the row
    unsigned long long recon_modifier_absent{0};   // `ReconModifier` absent from the row

    // The published level per observing side, summed over every tick and every
    // (side, target) pair the pass covered. It sums to the state's own flat
    // detected_* counters; the split is what attributes a contact drop to a
    // side rather than to the run as a whole.
    struct ReconSideCensus {
        int side{0};
        unsigned long long none_level{0};
        unsigned long long blip{0};
        unsigned long long identified{0};
    };
    std::vector<ReconSideCensus> recon_side_census;
    // Packet cc9_recon_team_lists: each recon slot's five published triples
    // after the last 008073C0 rebuild, as unit indices in native order:
    // 0 own (+DD8h), 1 enemy (+DE4h), 2 neutral (+DF0h), 3 unknown (+DFCh),
    // 4 their union (+E08h). docs/RECON_TEAM_LISTS.md.
    struct ReconSlotTriples {
        int side{0};
        std::array<std::vector<std::size_t>, 5> lists{};
    };
    std::vector<ReconSlotTriples> recon_triples;
    unsigned long long recon_triple_builds{0};
    unsigned long long recon_triple_sum[5]{};
    const ReconSlotTriples* recon_triples_for(int side) const {
        for (const ReconSlotTriples& t : recon_triples) {
            if (t.side == side) return &t;
        }
        return nullptr;
    }
    void publish_recon_triples_008073c0();

    const ReconClassRecord* recon_record_for_class(int recon_class_id);
    void resolve_recon_inputs();
    void step_recon_sensor_pass_008073c0(float dt);

    void unit_pose(std::size_t index, float right[3], float up[3], float forward[3],
        float origin[3]) const {
        if (!units.unit_pose(index, right, up, forward, origin)) {
            for (int i = 0; i < 3; ++i) right[i] = up[i] = forward[i] = origin[i] = 0.0f;
            right[0] = 1.0f;
            up[1] = 1.0f;
            forward[2] = 1.0f;
        }
    }

    void unit_velocity(std::size_t index, float out[3]) const {
        float right[3], up[3], forward[3], origin[3];
        unit_pose(index, right, up, forward, origin);
        const GameUnitRow* row = units.unit_row(index);
        const float speed = row != nullptr ? row->forward_speed : 0.0f;
        for (int i = 0; i < 3; ++i) out[i] = forward[i] * speed;
    }

    // The aim point 00864D90 builds: the target's position raised by the class
    // Height at [target+538h]+0A8h.
    void unit_aim_point(std::size_t index, float out[3]) const {
        float right[3], up[3], forward[3], origin[3];
        unit_pose(index, right, up, forward, origin);
        for (int i = 0; i < 3; ++i) out[i] = origin[i];
        if (index < unit_state.size()) out[1] += unit_state[index].hull_height;
    }

    // Packet cc9_torpedo_launch_gate: 008FFF20's friendly walk, 0090058A..009007FC.
    // True when some own-side ship blocks the launch (the image's JA 0090096D).
    // Packet cc9_bullet_throw: 0073031D..00730498, the cone half-angle this shot
    // draws from. `seat_out` names the bot whose multiplier applied, or -1.
    // 00521E70's test on a role slot: 8 (unassigned) or a party slot whose
    // record answers AI at +9 (00927F10, [[00E188A8]+18CCh+slot*4]+9). With
    // kPartySlotAiHeldBound off, or the record unavailable, a non-8 slot is
    // taken as player-held (the earlier substitution).
    bool slot_ai_held_00927f10(std::int32_t slot) const {
        if (slot == 8) return true;
        if constexpr (kPartySlotAiHeldBound) {
            const bsp::SessionParticipantPools* pools =
                ship_ai != nullptr ? ship_ai->session_participants() : nullptr;
            std::uint8_t ai = 0;
            if (pools != nullptr && pools->try_ai_held_00927f10(slot, ai)) return ai != 0;
        }
        return false;
    }
    bool role_ai_held_00521e70(std::size_t unit, int role) const {
        std::int32_t slot = 8;
        if (!units.unit_current_role_slot(unit, role, slot)) return true;
        return slot_ai_held_00927f10(slot);
    }
    float bullet_throw_magnitude_0073031d(const GameGunRow& gun, std::size_t owner,
        int& seat_out) const {
        seat_out = -1;
        float magnitude = gun.throw_amount;                           // 007302DF
        const int f = gun.category;                                   // gunclass+80h
        if ((f == 1 || f == 5 || f == 6) && kTurnOffAaGunThrow) {     // 007302EA..00730317
            magnitude = 0.0f;
        }
        if (gun.bullet_class < 0) return magnitude;                   // 00730323..0073032F
        int level = units.skill_level(owner);                         // bot+34h, 008FBCAC
        if (level < 0 || level > 5) level = 1;
        int seat = -1;
        const bool plane = units.unit_is_kind_of(owner, bsp::kUnitGunneryKindPlaneBase);
        if (plane && f == 0) {                                        // 00730348..0073038D
            if (role_ai_held_00521e70(owner, 1)) seat = kThrowPilot;  // [unit+0DF4h]
        } else {
            const int sub = gun.bullet_sub_type;                      // [[gun+3F8h]+34h]+8h
            if (sub == 0x0A) {                                        // gun+39Ch, role 5
                if (f == 7 && role_ai_held_00521e70(owner, 5)) seat = kThrowTorpedo;
            } else if (sub == 0x0B) {                                 // role 7, +3A4h else +3A0h
                if (role_ai_held_00521e70(owner, 7)) {
                    if (f == 9) seat = kThrowArtillery;
                    else if (f == 8) seat = kThrowDepthCharge;
                }
            } else if (sub == 0x10) {                                 // gun+394h, role 3
                if ((f == 5 || f == 6) && role_ai_held_00521e70(owner, 3)) seat = kThrowAaFlak;
            } else if (sub >= 4 && sub <= 7) {                        // gun+398h, role 4
                if ((f == 2 || f == 3 || f == 4 || f == 6)
                    && role_ai_held_00521e70(owner, 4)) seat = kThrowArtillery;
            } else if (f == 1 && role_ai_held_00521e70(owner, 2)      // gun+390h, 00730471
                       && role_ai_held_00521e70(owner, 3)) {          // 00730482
                seat = plane ? kThrowTailGunner : kThrowAaGunner;     // 00922E90(gun, 0Fh)
            }
        }
        if (seat >= 0) {
            magnitude *= kBulletThrowMul[seat][level];                // 00730498 FMUL
            seat_out = seat;
        }
        return magnitude;
    }
    unsigned long long torpedo_gyro_launches{0};
    unsigned long long torpedo_gyro_turn_steps{0};
    double torpedo_gyro_offset_sum_deg{0.0};
    unsigned long long throw_cone_shots{0};
    unsigned long long throw_fan_shots{0};
    unsigned long long throw_zero_shots{0};
    double throw_angle_sum_deg{0.0};
    double throw_magnitude_sum_deg{0.0};
    unsigned long long throw_by_seat[kThrowSeatCount + 1]{};
    int torpedo_friendly_hold_logs{0};
    // Packet cc9_muzzle_offsets counters.
    unsigned long long muzzle_offset_shots{0};
    unsigned long long muzzle_offset_fallbacks{0};
    double muzzle_shift_sum{0.0};
    double muzzle_shift_max{0.0};
    // DIAGNOSTIC, no gameplay effect: at each torpedo launch, the closest
    // own-side ship to holding it (the smallest miss - threshold).
    bool torpedo_friendly_diag{false};
    bool torpedo_friendly_hold_008fff20(const GameGunRow& gun, std::size_t owner_unit,
        const float gun_position[3], const float right[3], const float forward[3],
        const std::array<float, 2>& lead_xz, float snap_radians) {
        int in_range = 0;
        int crossed = 0;
        float best_margin = 1.0e30f;
        std::size_t best = owner_unit;
        bsp::TorpedoFriendlyCrossing best_c{};
        const float h = gun.angles.horz;
        const float hs = kGunHorzSign * std::sin(h);
        const std::array<float, 2> axis{{forward[0] * std::cos(h) + right[0] * hs,
            forward[2] * std::cos(h) + right[2] * hs}};
        const std::array<float, 3> gun_at{{gun_position[0], gun_position[1], gun_position[2]}};
        const std::array<float, 2> run_end = bsp::torpedo_run_end_008fff20(
            {{gun_at[0], gun_at[2]}}, lead_xz, axis, snap_radians);
        // 00900236: [[gun+3F8h]+34h]+0E4h, WaterTravelSpeed.
        const float water_speed = gun.water_travel_speed > 0.0f
            ? gun.water_travel_speed : gun.muzzle_speed;
        const int own_side = units.unit_side_0054(owner_unit);
        for (std::size_t i = 0; i < unit_state.size(); ++i) {
            // 0090058F: [008053C0([owner+54h]) + 0DDCh], the own-party list.
            if (i == owner_unit || unit_state[i].dead) continue;   // 009005BA
            if (units.unit_side_0054(i) != own_side) continue;
            if (!units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) continue; // 009005AD
            float r[3], u[3], f[3], o[3], v[3];
            unit_pose(i, r, u, f, o);
            unit_velocity(i, v);
            const bsp::TorpedoFriendlyCrossing c = bsp::torpedo_friendly_crossing_008fff20(
                gun_at, run_end, water_speed, {{o[0], o[1], o[2]}}, {{f[0], f[2]}},
                {{v[0], v[2]}});
            if (torpedo_friendly_diag) {
                if (c.in_range) ++in_range;
                if (c.crossed) {
                    ++crossed;
                    if (c.miss - c.threshold < best_margin) {
                        best_margin = c.miss - c.threshold;
                        best = i;
                        best_c = c;
                    }
                }
                continue;
            }
            if (!c.blocks) continue;
            if (torpedo_friendly_hold_logs < 60) {
                ++torpedo_friendly_hold_logs;
                log.notef("gunnery: torpedo friendly hold t=%.2f shooter=%s friendly=%s "
                    "run=%.1f miss=%.1f threshold=%.1f (008FFF20 009007F6)",
                    static_cast<double>(clock_seconds),
                    unit_state[owner_unit].row.name.c_str(), unit_state[i].row.name.c_str(),
                    static_cast<double>(c.run_distance), static_cast<double>(c.miss),
                    static_cast<double>(c.threshold));
            }
            return true;
        }
        if (torpedo_friendly_diag) {
            log.notef("gunnery: torpedo launch t=%.2f shooter=%s gun=%s friendly_in_2km=%d "
                "crossed=%d closest=%s run=%.1f miss=%.1f threshold=%.1f",
                static_cast<double>(clock_seconds), unit_state[owner_unit].row.name.c_str(),
                gun.target_name.c_str(), in_range, crossed,
                best == owner_unit ? "-" : unit_state[best].row.name.c_str(),
                static_cast<double>(best_c.run_distance), static_cast<double>(best_c.miss),
                static_cast<double>(best_c.threshold));
            // Every same-side ship within 8 km: along / across the run line from
            // the gun (x, z), its distance, and its speed across the run.
            const float rx = run_end[0] - gun_at[0], rz = run_end[1] - gun_at[2];
            const float rl = std::sqrt(rx * rx + rz * rz);
            std::string near_list;
            for (std::size_t i = 0; rl > 0.0f && i < unit_state.size(); ++i) {
                if (i == owner_unit || unit_state[i].dead) continue;
                if (units.unit_side_0054(i) != own_side) continue;
                if (!units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) continue;
                float r[3], u[3], f[3], o[3], v[3];
                unit_pose(i, r, u, f, o);
                unit_velocity(i, v);
                const float dx = o[0] - gun_at[0], dz = o[2] - gun_at[2];
                const float d = std::sqrt(dx * dx + dz * dz);
                if (d > 8000.0f) continue;
                const float along = (dx * rx + dz * rz) / rl;
                const float across = (dx * rz - dz * rx) / rl;
                const float v_across = (v[0] * rz - v[2] * rx) / rl;
                char buf[128];
                std::snprintf(buf, sizeof buf, " %s(along=%.0f across=%.0f d=%.0f vx=%.1f)",
                    unit_state[i].row.name.c_str(), static_cast<double>(along),
                    static_cast<double>(across), static_cast<double>(d),
                    static_cast<double>(v_across));
                near_list += buf;
            }
            log.notef("gunnery: torpedo launch friends t=%.2f shooter=%s plat=%d water_speed=%.1f "
                "run_dir=(%.3f %.3f)%s", static_cast<double>(clock_seconds),
                unit_state[owner_unit].row.name.c_str(), gun.platform_key,
                static_cast<double>(water_speed), static_cast<double>(rl > 0 ? rx / rl : 0.0f),
                static_cast<double>(rl > 0 ? rz / rl : 0.0f), near_list.c_str());
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
// The authored tables, flattened through the live Lua state
// ---------------------------------------------------------------------------

// Packet cc9_gun_barrel_count. The image loads the device row's `Mesh` into the
// weapon class's +50h and 007325A0 reads its "fire" Points items into class+98h.
// This process opens the same path through the mounted VFS once per device and
// keeps 007325A0's list; the barrel count is 0072AB80 on it.
// Packet cc9_ship_platform_attachment. The vehicle class's `Mesh` is the ship
// model whose ("slot", key) groups 0095F500 turns into platform frames. Read
// once per class through the mounted VFS, as the device models are.
GameGunneryHost::Impl::ShipModelSlots&
GameGunneryHost::Impl::ship_model_slots(int type_id) {
    auto found = ship_slots_by_class.find(type_id);
    if (found != ship_slots_by_class.end()) return found->second;
    ShipModelSlots& entry = ship_slots_by_class[type_id];
    entry.mesh = lua.read_vehicle_class_string(type_id, "Mesh");
    std::string error;
    std::vector<std::uint8_t> bytes;
    if (entry.mesh.empty()) {
        error = "no Mesh string";
    } else if (!lua.read_resource_file(entry.mesh, bytes)) {
        error = "model did not open";
    } else if (bsp::read_mmod_aux_point_items_0071b3e0(bytes, entry.items, error)) {
        entry.loaded = true;
    }
    if (!bytes.empty()) entry.has_box = bsp::read_mmod_bounding_box(bytes, entry.box);
    if (!bytes.empty()) {
        std::vector<bsp::GeomMeshResourcePayload> meshes;
        std::string mesh_error;
        bsp::read_mmod_geom_meshes(bytes, meshes, mesh_error);
        float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
        for (const bsp::GeomMeshResourcePayload& mesh : meshes) {
            const auto vert = [&](std::uint16_t i, std::array<float, 3>& out) {
                if (i >= mesh.vertices.size()) return false;
                out = mesh.vertices[i];
                return true;
            };
            for (const bsp::GeomMeshTriangle& t : mesh.triangles) {
                std::array<float, 3> a, b, c;
                if (!vert(t.v0, a) || !vert(t.v1, b) || !vert(t.v2, c)) continue;
                entry.tris.push_back(a);
                entry.tris.push_back(b);
                entry.tris.push_back(c);
                for (const auto& v : {a, b, c}) {
                    for (int k = 0; k < 3; ++k) {
                        lo[k] = std::min(lo[k], v[k]);
                        hi[k] = std::max(hi[k], v[k]);
                    }
                }
            }
            // 0081F980: engine room 5 -> slot 0, magazine 8 -> slot 1, fuel 6 -> slot 2.
            for (const bsp::GeomMeshElement& el : mesh.elements) {
                const int slot = el.kind == 5 ? 0 : el.kind == 8 ? 1 : el.kind == 6 ? 2 : -1;
                if (slot < 0) continue;
                float elo[3] = {1e30f, 1e30f, 1e30f}, ehi[3] = {-1e30f, -1e30f, -1e30f};
                bool any = false;
                for (std::uint16_t ord : el.triangle_ordinals) {
                    if (ord >= mesh.triangles.size()) continue;
                    const bsp::GeomMeshTriangle& t = mesh.triangles[ord];
                    for (std::uint16_t vi : {t.v0, t.v1, t.v2}) {
                        if (vi >= mesh.vertices.size()) continue;
                        any = true;
                        for (int k = 0; k < 3; ++k) {
                            elo[k] = std::min(elo[k], mesh.vertices[vi][k]);
                            ehi[k] = std::max(ehi[k], mesh.vertices[vi][k]);
                        }
                    }
                }
                if (!any) continue;
                entry.section_present[slot] = true;   // the last element of a kind wins
                for (int k = 0; k < 3; ++k) {
                    entry.section_point[slot][k] = (elo[k] + ehi[k]) * 0.5f;   // 00723030
                }
            }
        }
        if (!entry.tris.empty()) {
            entry.has_mesh = true;
            for (int k = 0; k < 3; ++k) { entry.mesh_box[k] = lo[k]; entry.mesh_box[k + 3] = hi[k]; }
        }
        log.notef("gunnery: vehicle class %d geom meshes=%zu triangles=%zu sections engine=%d "
            "magazine=%d fuel=%d%s%s (0081F980 / 00723030)", type_id, meshes.size(),
            entry.tris.size() / 3, entry.section_present[0] ? 1 : 0,
            entry.section_present[1] ? 1 : 0, entry.section_present[2] ? 1 : 0,
            mesh_error.empty() ? "" : " error: ", mesh_error.c_str());
    }
    std::size_t slots = 0;
    for (const bsp::GunFirePointItem& item : entry.items) {
        if (item.name == "slot") ++slots;
    }
    log.notef("gunnery: vehicle class %d mesh=%s slot groups=%zu%s%s (0095F500)", type_id,
        entry.mesh.empty() ? "-" : entry.mesh.c_str(), slots, entry.loaded ? "" : " not read: ",
        entry.loaded ? "" : error.c_str());
    return entry;
}

GameGunneryHost::Impl::DeviceFirePoints&
GameGunneryHost::Impl::device_fire_points(int device) {
    auto found = fire_points_by_device.find(device);
    if (found != fire_points_by_device.end()) return found->second;
    DeviceFirePoints& entry = fire_points_by_device[device];
    entry.mesh = lua.read_device_class_string(device, "Mesh");
    std::string error;
    if (entry.mesh.empty()) {
        error = "no Mesh string";
    } else {
        std::vector<std::uint8_t> bytes;
        std::vector<bsp::GunFirePointItem> items;
        if (!lua.read_resource_file(entry.mesh, bytes)) {
            error = "model did not open";
        } else if (bsp::read_mmod_aux_point_items_0071b3e0(bytes, items, error)) {
            entry.loaded = true;
            for (const bsp::GunFirePointItem& item : items) {
                if (item.name == "fire") ++entry.fire_items;
            }
            entry.list = bsp::gun_fire_muzzle_offsets_007325a0(items);
            std::string hierarchy_error;
            if (bsp::read_mmod_hierarchy_items(bytes, entry.nodes, hierarchy_error)) {
                // 0071AD50: the node paired with the Note whose text is the name.
                std::vector<bsp::MmodNoteItem> notes;
                std::vector<std::string> tags;
                std::string note_error;
                const bool notes_read = bsp::read_mmod_resource_notes(bytes, notes, tags,
                    note_error);
                auto node_with_note = [&](const char* name) {
                    for (const bsp::MmodNoteItem& note : notes) {
                        if (note.text != name) continue;
                        for (std::size_t i = 0; i < entry.nodes.size(); ++i) {
                            for (std::uint32_t r : entry.nodes[i].resources) {
                                if (r == note.resource_position) return static_cast<int>(i);
                            }
                        }
                    }
                    return -1;
                };
                entry.barrel_node = node_with_note("barrel");   // 0072EA15, 00CF70C4
                entry.base_node = node_with_note("base");       // 0072E9E2, 00CFACD8
                std::string chain;
                chain += notes_read ? " notes:" : (" notes-error:" + note_error);
                for (const bsp::MmodNoteItem& note : notes) {
                    chain += " '" + note.text + "'@" + std::to_string(note.resource_position);
                }
                chain += " tags:";
                for (const std::string& tag : tags) chain += " " + tag;
                const int pick = entry.barrel_node >= 0 ? entry.barrel_node : entry.base_node;
                for (int at = pick, guard = 0; at >= 0 && guard < 16; ++guard) {
                    const bsp::HierarchyItem& n = entry.nodes[static_cast<std::size_t>(at)];
                    char buf[160];
                    if (n.matrix) {
                        std::snprintf(buf, sizeof buf, " %d:%s(p=%d t=%.2f,%.2f,%.2f r2=%.2f,%.2f,%.2f)",
                            at, n.name.c_str(), static_cast<int>(n.parent),
                            static_cast<double>((*n.matrix)[12]), static_cast<double>((*n.matrix)[13]),
                            static_cast<double>((*n.matrix)[14]), static_cast<double>((*n.matrix)[8]),
                            static_cast<double>((*n.matrix)[9]), static_cast<double>((*n.matrix)[10]));
                    } else {
                        std::snprintf(buf, sizeof buf, " %d:%s(p=%d no matrix)", at, n.name.c_str(),
                            static_cast<int>(n.parent));
                    }
                    chain += buf;
                    if (at == 0 || n.parent >= entry.nodes.size()) break;
                    at = static_cast<int>(n.parent);
                }
                chain += " names:";
                for (const bsp::HierarchyItem& n : entry.nodes) {
                    chain += " '";
                    for (char ch : n.name) chain += (ch == '\0') ? '~' : ch;
                    chain += "'/" + std::to_string(static_cast<int>(n.parent));
                    for (std::uint32_t r : n.resources) chain += "," + std::to_string(r);
                }
                chain += " aux:";
                for (const bsp::GunFirePointItem& it : items) {
                    chain += " '" + it.name + "'#" + std::to_string(static_cast<int>(it.index))
                        + "@" + std::to_string(it.resource_position);
                }
                std::string offsets;
                for (const auto& o : entry.list.offsets) {
                    char buf[64];
                    std::snprintf(buf, sizeof buf, " (%.2f,%.2f,%.2f)", static_cast<double>(o[0]),
                        static_cast<double>(o[1]), static_cast<double>(o[2]));
                    offsets += buf;
                }
                log.notef("gunnery: device %d mesh=%s hierarchy nodes=%zu barrel=%d base=%d "
                    "chain:%s offsets:%s (0072EE65/00859550)", device, entry.mesh.c_str(),
                    entry.nodes.size(), entry.barrel_node, entry.base_node, chain.c_str(),
                    offsets.c_str());
            } else {
                entry.nodes.clear();
                log.notef("gunnery: device %d mesh=%s hierarchy not read: %s", device,
                    entry.mesh.c_str(), hierarchy_error.c_str());
            }
        }
    }
    if (entry.loaded) {
        log.notef("gunnery: device %d mesh=%s aux fire items=%zu index0=%d muzzles=%zu "
            "barrels=%d%s (007325A0/0072AB80)", device, entry.mesh.c_str(), entry.fire_items,
            entry.list.whole_index0 ? 1 : 0, entry.list.offsets.size(),
            bsp::gun_muzzle_count_0072ab80(entry.list.offsets.size()),
            entry.list.empty_item_stopped ? " stopped-at-empty-item" : "");
    } else {
        log.notef("gunnery: device %d mesh=%s barrel count not read from the model: %s",
            device, entry.mesh.empty() ? "-" : entry.mesh.c_str(), error.c_str());
    }
    return entry;
}

void GameGunneryHost::Impl::flatten_class_tables(const std::vector<int>& class_ids) {
    // The twelve `Function` spellings 007327B0 compares against, emitted from
    // the reconstruction so the literal list stays on the C++ side and the
    // chunk only performs the compare.
    std::string chunk;
    chunk.reserve(6144);
    chunk += "local F = {";
    for (int i = 0; i < bsp::kUnitGunneryCategoryCount; ++i) {
        const char* name = bsp::gunnery_category_function_name(i);
        chunk += "\"";
        chunk += (name != nullptr ? name : "");
        chunk += "\",";
    }
    chunk += "}\nlocal ids = {";
    for (std::size_t i = 0; i < class_ids.size(); ++i) {
        char number[16];
        std::snprintf(number, sizeof(number), "%d,", class_ids[i]);
        chunk += number;
    }
    chunk +=
        "}\n"
        "local function cat(s)\n"
        "  if type(s) ~= 'string' then return -1 end\n"
        "  local u = string.upper(s)\n"
        "  for i = 1, table.getn(F) do if F[i] == u then return i - 1 end end\n"
        "  return -1\n"
        "end\n"
        "local function num(v, s)\n"
        "  if type(v) ~= 'number' then return nil end\n"
        "  return math.floor(v * s + 0.5)\n"
        "end\n"
        "local think = 2\n"
        "if type(Globals) == 'table' and type(Globals.WeaponSystems) == 'table'\n"
        "   and type(Globals.WeaponSystems.WeaponDirectorThinkTime) == 'number' then\n"
        "  think = Globals.WeaponSystems.WeaponDirectorThinkTime\n"
        "end\n"
        "local aafix, aamul = 0, 0\n"
        "if type(ShipGlobals) == 'table' and type(ShipGlobals.AAGunnerErrorModifier) == 'table' then\n"
        "  local m = ShipGlobals.AAGunnerErrorModifier\n"
        "  if type(m.CalcTargetPosTimeAddFix) == 'number' then aafix = m.CalcTargetPosTimeAddFix end\n"
        "  if type(m.CalcTargetPosTimeAddMul) == 'number' then aamul = m.CalcTargetPosTimeAddMul end\n"
        "end\n"
        "for _, id in ipairs(ids) do\n"
        "  local row = type(VehicleClass) == 'table' and VehicleClass[id] or nil\n"
        "  if type(row) == 'table' then\n"
        "    local f = {}\n"
        "    f.think = num(think, 1000) or 2000\n"
        "    f.aafix = num(aafix, 100000) or 0\n"
        "    f.aamul = num(aamul, 100000) or 0\n"
        "    f.hp = num(row.HP, 1000) or 0\n"
        "    f.armour = num(row.Armour, 1000) or 0\n"
        "    f.length = num(row.Length, 1000) or 0\n"
        "    f.width = num(row.Width, 1000) or 0\n"
        "    f.height = num(row.Height, 1000) or 0\n"
        "    local n = 0\n"
        "    local plats = row.Platforms\n"
        "    if type(plats) == 'table' then\n"
        "      for k = 1, 64 do\n"
        "        local p = plats[k]\n"
        "        if type(p) == 'table' and type(p.Gun) == 'table'\n"
        "           and type(p.Gun[1]) == 'number' then\n"
        "          local dev = type(DeviceClass) == 'table' and DeviceClass[p.Gun[1]] or nil\n"
        "          if type(dev) == 'table' then\n"
        "            n = n + 1\n"
        "            local q = 'p' .. n .. '_'\n"
        "            f[q .. 'key'] = k\n"
        "            f[q .. 'dev'] = p.Gun[1]\n"
        "            f[q .. 'cat'] = cat(dev.Function)\n"
        "            f[q .. 'hrs'] = num(dev.HorzRotSpeed, 1000) or 0\n"
        "            f[q .. 'vrs'] = num(dev.VertRotSpeed, 1000) or 0\n"
        "            local bn = 0\n"
        "            local b1 = nil\n"
        "            if type(dev.Bullet) == 'table' then\n"
        "              for bi = 1, 16 do\n"
        "                if type(dev.Bullet[bi]) == 'table' then\n"
        "                  bn = bn + 1\n"
        "                  if b1 == nil then b1 = dev.Bullet[bi] end\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            f[q .. 'barrels'] = bn\n"
        "            if type(b1) == 'table' then\n"
        "              f[q .. 'bullet'] = num(b1.Bullet, 1) or -1\n"
        // LATENT GUARD. 007313E0 reads ReloadTime as a PAIR into +28h/+2Ch and
        // 00BD2F10 draws between them per shot; when a row authors a scalar the
        // reader writes it to both ends, which is every row in this installation.
        // But num() returns nil for a table, so a paired ReloadTime would leave
        // `reload` at 0 here and the gun would fire every fixed step. Nothing
        // authors a pair today, so this changes no current behaviour.
        // docs/GUN_SHOT_CADENCE.md divergence 9.
        "              local rt1 = b1.ReloadTime\n"
        "              if type(rt1) == 'table' then rt1 = rt1[1] end\n"
        "              f[q .. 'reload'] = num(rt1, 1000) or 0\n"
        "              f[q .. 'bdelay'] = num(b1.BarrelDelayTime, 1000) or 0\n"
        "              f[q .. 'throw'] = num(b1.Throw, 1000000) or 0\n"
        // bulletclasses.lua publishes the arcade or realistic table under the
        // global `Bullets`; `BulletClass` is accepted as well so a differently
        // named installation still reads.
        "              local BT = type(Bullets) == 'table' and Bullets\n"
        "                 or (type(BulletClass) == 'table' and BulletClass or nil)\n"
        "              local bc = BT and BT[b1.Bullet] or nil\n"
        "              if type(bc) == 'table' then\n"
        "                f[q .. 'v0'] = num(bc.V0, 1000) or 0\n"
        "                f[q .. 'range'] = num(bc.Range, 1000) or 0\n"
        "                f[q .. 'dmin'] = num(bc.DamageMin, 1000) or 0\n"
        "                f[q .. 'dmax'] = num(bc.DamageMax, 1000) or 0\n"
        "                f[q .. 'wdmg'] = num(bc.WaterDamage, 1000) or 0\n"
        "                f[q .. 'fdmg'] = num(bc.FireDamage, 1000) or 0\n"
        "                f[q .. 'fchance'] = num(bc.FireChance, 1000) or 0\n"
        "                f[q .. 'mass'] = num(bc.Mass, 1000) or 0\n"
        "                f[q .. 'nograv'] = (bc.NoGravity == true) and 1000 or 0\n"
        "                if type(bc.Blast) == 'table' then\n"
        "                  f[q .. 'bdmin'] = num(bc.Blast.BlastDamageMin, 1000) or 0\n"
        "                  f[q .. 'bdmax'] = num(bc.Blast.BlastDamageMax, 1000) or 0\n"
        "                  f[q .. 'brange'] = num(bc.Blast.BlastRange, 1000) or 0\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            local b2 = nil\n"
        "            if type(dev.Bullet) == 'table' and type(dev.Bullet[2]) == 'table' then\n"
        "              b2 = dev.Bullet[2]\n"
        "            end\n"
        "            if type(b2) == 'table' then\n"
        "              f[q .. 'bullet2'] = num(b2.Bullet, 1) or -1\n"
        "              local BT2 = type(Bullets) == 'table' and Bullets\n"
        "                 or (type(BulletClass) == 'table' and BulletClass or nil)\n"
        "              local bc2 = BT2 and BT2[b2.Bullet] or nil\n"
        "              if type(bc2) == 'table' then\n"
        "                f[q .. '2v0'] = num(bc2.V0, 1000) or 0\n"
        "                f[q .. '2dmin'] = num(bc2.DamageMin, 1000) or 0\n"
        "                f[q .. '2dmax'] = num(bc2.DamageMax, 1000) or 0\n"
        "                f[q .. '2nograv'] = (bc2.NoGravity == true) and 1000 or 0\n"
        "                if type(bc2.Blast) == 'table' then\n"
        "                  f[q .. '2bdmin'] = num(bc2.Blast.BlastDamageMin, 1000) or 0\n"
        "                  f[q .. '2bdmax'] = num(bc2.Blast.BlastDamageMax, 1000) or 0\n"
        "                  f[q .. '2brange'] = num(bc2.Blast.BlastRange, 1000) or 0\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            local wn = 0\n"
        "            if type(p.Windows) == 'table' then\n"
        "              for wi = 1, 8 do\n"
        "                local w = p.Windows[wi]\n"
        "                if type(w) == 'table' then\n"
        "                  wn = wn + 1\n"
        "                  local r = q .. 'w' .. wn .. '_'\n"
        "                  f[r .. 'nf'] = (w.Nofire and 1) or 0\n"
        "                  f[r .. 'minh'] = num(w.MinHorzAngle, 1000000) or 0\n"
        "                  f[r .. 'maxh'] = num(w.MaxHorzAngle, 1000000) or 0\n"
        "                  f[r .. 'minv'] = num(w.MinVertAngle, 1000000) or 0\n"
        "                  f[r .. 'maxv'] = num(w.MaxVertAngle, 1000000) or 0\n"
        "                end\n"
        "              end\n"
        "            end\n"
        "            f[q .. 'wn'] = wn\n"
        "            if type(p.RestAngles) == 'table' then\n"
        "              f[q .. 'rh'] = num(p.RestAngles[1], 1000000) or 0\n"
        "              f[q .. 'rv'] = num(p.RestAngles[2], 1000000) or 0\n"
        "            end\n"
        "          end\n"
        "        end\n"
        "      end\n"
        "    end\n"
        "    f.n = n\n"
        "    row.BSPGun = f\n"
        "  end\n"
        "end\n";

    const int loaded = lua.luaL_loadbuffer(chunk.c_str(),
        static_cast<int>(chunk.size()), "bsp_gunnery_flatten");
    if (loaded != 0) {
        log.note("gunnery: the authored-table flatten chunk did not compile; no gun exists");
        lua.lua_settop(-2);
        return;
    }
    const int called = lua.lua_pcall(0, 0, 0);
    if (called != 0) {
        log.note("gunnery: the authored-table flatten chunk failed; no gun exists");
        lua.lua_settop(-2);
        return;
    }
    done("Gunnery::vehicle_class_platforms_009610f6", 0x009610f6u);
    done("Gunnery::device_class_function_007327b0", 0x007327b0u);
}

// ---------------------------------------------------------------------------
// 00727BD0 and the per-unit gun lists
// ---------------------------------------------------------------------------

void GameGunneryHost::Impl::build_rank_table() {
    rank_table.assign(static_cast<std::size_t>(bsp::kUnitGunneryCategoryCount)
        * static_cast<std::size_t>(bsp::kUnitGunneryClassIdCount), 0);
    bsp::build_rank_table_00727bd0(bsp::kGunneryPreferenceLists, rank_table.data());
    done("Gunnery::build_target_rank_table_00727bd0", 0x00727bd0u);
}

namespace {

// _dupenv_s rather than getenv, which is a /W4 /WX error under MSVC; the same
// form game_hosts_ai.cpp's ai_weight_model_enabled uses. Off unless set to 1.
bool restore_all_ordnance_enabled() {
    static const bool enabled = [] {
        char* text = nullptr;
        std::size_t bytes = 0;
        if (_dupenv_s(&text, &bytes, "BSP_GUNNERY_RESTORE_ALL_ORDNANCE") != 0) return false;
        const bool on = text != nullptr && text[0] == '1';
        std::free(text);
        return on;
    }();
    return enabled;
}

}  // namespace

void GameGunneryHost::Impl::build_guns(std::size_t first_unit) {
    const std::size_t count = units.count();
    if (first_unit >= count) return;
    // resize, not assign: the units host appends, so the states already built
    // keep their health, timers and category state and the new units come in
    // default-constructed behind them.
    unit_state.resize(count);

    // The distinct class ids the scene created, for one flatten pass. Only the
    // new units': flatten writes `VehicleClass[id].BSPGun` into the Lua state,
    // which outlives this call, so a class an earlier batch flattened is
    // already there and re-flattening it would rewrite the same table.
    std::vector<int> class_ids;
    for (std::size_t i = first_unit; i < count; ++i) {
        const GameUnitRow* row = units.unit_row(i);
        if (row == nullptr || row->type_id < 0) continue;
        if (std::find(class_ids.begin(), class_ids.end(), row->type_id) == class_ids.end()) {
            class_ids.push_back(row->type_id);
        }
    }
    flatten_class_tables(class_ids);

    bool think_read = false;
    for (std::size_t i = first_unit; i < count; ++i) {
        UnitState& state = unit_state[i];
        const GameUnitRow* row = units.unit_row(i);
        state.row.unit_index = i;
        state.row.name = row != nullptr ? row->name : std::string();
        state.row.side = units.unit_side_0054(i);
        state.row.type_id = row != nullptr ? row->type_id : -1;
        state.category = bsp::unit_gunnery_initial_category_state_00864580();
        if (row == nullptr || row->type_id < 0) continue;
        const int type_id = row->type_id;

        if (!think_read) {
            const float value = flat_scaled(type_id, "think", kMilliScale, -1.0f);
            if (value > 0.0f) {
                think_time = value;
                think_read = true;
                aa_time_add_fix = flat_scaled(type_id, "aafix", 100000.0f, 0.0f);
                aa_time_add_mul = flat_scaled(type_id, "aamul", 100000.0f, 0.0f);
                if (!aa_time_logged) log.notef("gunnery: AAGunnerErrorModifier CalcTargetPosTimeAddFix=%.3f "
                    "AddMul=%.3f (settings +758h/+75Ch, 00901C20)",
                    static_cast<double>(aa_time_add_fix), static_cast<double>(aa_time_add_mul));
                aa_time_logged = true;
                done("Gunnery::weapon_director_think_time_0087e16b", 0x0087e16bu);
            }
        }
        state.max_health = flat_scaled(type_id, "hp", kMilliScale, 0.0f);
        state.health = state.max_health;
        state.armour = flat_scaled(type_id, "armour", kMilliScale, 0.0f);
        state.hull_length = flat_scaled(type_id, "length", kMilliScale, 0.0f);
        state.hull_width = flat_scaled(type_id, "width", kMilliScale, 0.0f);
        state.hull_height = flat_scaled(type_id, "height", kMilliScale, 0.0f);
        state.row.health = state.health;
        state.row.max_health = state.max_health;

        const int platforms = flat(type_id, "n", 0);
        for (int p = 1; p <= platforms && p <= kMaxPlatformScan; ++p) {
            char key[32];
            auto make = [&key, p](const char* leaf) {
                std::snprintf(key, sizeof(key), "p%d_%s", p, leaf);
                return key;
            };
            GameGunRow gun;
            gun.unit_index = i;
            gun.unit_name = state.row.name;
            gun.platform_key = flat(type_id, make("key"), 0);
            gun.device_class = flat(type_id, make("dev"), -1);
            gun.category = flat(type_id, make("cat"), -1);
            if (gun.category < 0 || gun.category >= bsp::kUnitGunneryCategoryCount) continue;
            const char* function = bsp::gunnery_category_function_name(gun.category);
            gun.function = function != nullptr ? function : "";
            char label[32];
            std::snprintf(label, sizeof(label), "Platform %d", gun.platform_key);
            gun.platform_name = label;
            gun.speeds.horz = flat_scaled(type_id, make("hrs"), kMilliScale, 0.0f);
            gun.speeds.vert = flat_scaled(type_id, make("vrs"), kMilliScale, 0.0f);
            gun.barrel_num = std::max(1, flat(type_id, make("barrels"), 1));
            if (kGunBarrelCountBound && gun.device_class >= 0) {
                // 0072E71A: gun+448h = 0072AB80(class) on 007325A0's list.
                DeviceFirePoints& fp = device_fire_points(gun.device_class);
                fp.records = gun.barrel_num;
                ++fp.guns;
                if (fp.loaded) {
                    const int image = bsp::gun_muzzle_count_0072ab80(fp.list.offsets.size());
                    if (image != gun.barrel_num) ++barrel_guns_changed;
                    gun.barrel_num = image;
                    ++barrel_guns_from_model;
                    done("GunClass::load_fire_node_muzzle_offsets_007325a0", 0x007325a0u);
                } else {
                    ++barrel_guns_fallback;
                }
            }
            gun.bullet_class = flat(type_id, make("bullet"), -1);
            gun.reload_time = flat_scaled(type_id, make("reload"), kMilliScale, 0.0f);
            gun.barrel_delay_time = flat_scaled(type_id, make("bdelay"), kMilliScale, 0.0f);
            gun.throw_amount = flat_scaled(type_id, make("throw"), kAngleScale, 0.0f);
            gun.muzzle_speed = flat_scaled(type_id, make("v0"), kMilliScale, 0.0f);
            gun.max_range = flat_scaled(type_id, make("range"), kMilliScale, 0.0f);
            // 00731020 answers with descriptor+60h, NOT the authored Lua `Range`:
            // 006E8770 puts `Range` at +68h and never writes +60h. The finalise
            // hook 006E9890 derives +60h from the class sub-type - artillery
            // (4..7) keeps `Range`, the gun group (1,2,3,10h) takes FlyTime * V0,
            // 0Bh takes 240 and everything else 3000 - and MTorpedo overrides it
            // at 00855A90.
            //
            // Only 32 of the 119 bullet classes in this installation author a
            // `Range` at all. The other 87 were reading 0, which collapsed their
            // category to category_engagement_range_00956d63's 10.0f seed and made
            // 00863990 refuse every candidate: on IJN01 that silenced 82 PLANEGUN,
            // 295 AAMACHINEGUN, 60 FLAK, 12 TORPEDO and 10 DEPTHCHARGE guns.
            // docs/BULLET_ENGAGEMENT_RANGE.md.
            //
            // Unauthored fields are left at the struct's defaults, which are the
            // constructor's and the reader's, so filling only what the row
            // authored reproduces native state. `FlyTime` defaults to FLT_MAX.
            if (gun.bullet_class >= 0) {
                bsp::WeaponClassFinaliseInput fin;
                const std::string bullet_type =
                    lua.read_bullet_class_string(gun.bullet_class, "Type");
                fin.sub_type = bsp::weapon_class_sub_type_for_lua_type(bullet_type);
                // docs/ORDNANCE_KIND_IDENTITY.md: the Type string names the
                // projectile descriptor whose vtable[8] the 007ED7E0 family
                // queries, and its answer set is the entity class-id space.
                gun.ordnance = bsp::ordnance_kinds_for_bullet_type(bullet_type.c_str());
                fin.range = lua.read_bullet_class_number(
                    gun.bullet_class, "Range", 0.0f);
                fin.muzzle_speed = lua.read_bullet_class_number(
                    gun.bullet_class, "V0", 0.0f);
                fin.fly_time = lua.read_bullet_class_number(
                    gun.bullet_class, "FlyTime", bsp::kWeaponClassFlyTimeDefault);
                fin.damage_min = lua.read_bullet_class_number(
                    gun.bullet_class, "DamageMin", 0.0f);
                fin.blast_damage_min = lua.read_bullet_class_number(
                    gun.bullet_class, "Blast.BlastDamageMin", 0.0f);
                fin.name = lua.read_bullet_class_string(gun.bullet_class, "Name");
                fin.water_travel_speed = lua.read_bullet_class_number(
                    gun.bullet_class, "WaterTravelSpeed", 0.0f);
                fin.max_fall = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxFall", 0.0f);
                fin.flak_min_range = lua.read_bullet_class_number(
                    gun.bullet_class, "MinRange", 0.0f);
                bullet_min_range[gun.bullet_class] = fin.flak_min_range;
                const bsp::WeaponClassFinaliseResult finalised =
                    bsp::weapon_class_derive_engagement_range(fin);
                // The hook's other product, which this call already computed and
                // used to discard: +8h after the 006E9968 rewrite. 009FE270
                // switches on it for the AI target-weight accuracy.
                gun.bullet_sub_type = finalised.sub_type;
                if (finalised.engagement_range > 0.0f) {
                    gun.max_range = finalised.engagement_range;
                    ++summary.bullet_ranges_derived;
                }
                if (finalised.swim_speed > 0.0f) {
                    gun.water_travel_speed = fin.water_travel_speed;
                    gun.swim_speed = finalised.swim_speed;
                    ++summary.torpedo_ranges_derived;
                }
            }
            gun.rest_horz = flat_scaled(type_id, make("rh"), kAngleScale, 0.0f);
            gun.rest_vert = flat_scaled(type_id, make("rv"), kAngleScale, 0.0f);

            // Packet cc9_aa_lead: Bullet[2], read for every gun, used only by a
            // sub-type 6 gun against a plane (dp_air_ammo).
            {
                const int b2 = flat(type_id, make("bullet2"), -1);
                if (b2 >= 0) {
                    SecondAmmo second;
                    second.bullet_class = b2;
                    second.muzzle_speed = flat_scaled(type_id, make("2v0"), kMilliScale, 0.0f);
                    bsp::WeaponClassFinaliseInput fin2;
                    const std::string type2 = lua.read_bullet_class_string(b2, "Type");
                    fin2.sub_type = bsp::weapon_class_sub_type_for_lua_type(type2);
                    fin2.range = lua.read_bullet_class_number(b2, "Range", 0.0f);
                    fin2.muzzle_speed = lua.read_bullet_class_number(b2, "V0", 0.0f);
                    fin2.fly_time = lua.read_bullet_class_number(b2, "FlyTime",
                        bsp::kWeaponClassFlyTimeDefault);
                    fin2.damage_min = lua.read_bullet_class_number(b2, "DamageMin", 0.0f);
                    fin2.blast_damage_min = lua.read_bullet_class_number(b2,
                        "Blast.BlastDamageMin", 0.0f);
                    fin2.name = lua.read_bullet_class_string(b2, "Name");
                    fin2.flak_min_range = lua.read_bullet_class_number(b2, "MinRange", 0.0f);
                    const bsp::WeaponClassFinaliseResult r2 =
                        bsp::weapon_class_derive_engagement_range(fin2);
                    second.max_range = r2.engagement_range > 0.0f ? r2.engagement_range
                                                                  : fin2.range;
                    second.min_range = fin2.flak_min_range;
                    bullet_min_range[b2] = fin2.flak_min_range;
                    second_ammo_by_gun[guns.size()] = second;
                    if (bullet(b2) == nullptr) {
                        GameBulletClassRow r;
                        r.id = b2;
                        r.found = true;
                        r.type = type2;
                        r.muzzle_speed = second.muzzle_speed;
                        r.range = second.max_range;
                        r.damage_min = flat_scaled(type_id, make("2dmin"), kMilliScale, 0.0f);
                        r.damage_max = flat_scaled(type_id, make("2dmax"), kMilliScale, 0.0f);
                        r.blast_damage_min = flat_scaled(type_id, make("2bdmin"), kMilliScale, 0.0f);
                        r.blast_damage_max = flat_scaled(type_id, make("2bdmax"), kMilliScale, 0.0f);
                        r.blast_range = flat_scaled(type_id, make("2brange"), kMilliScale, 0.0f);
                        r.no_gravity = flat_scaled(type_id, make("2nograv"), kMilliScale, 0.0f) > 0.5f;
                        bullets.push_back(r);
                        log.notef("gunnery: bullet class %d Type=%s V0=%.1f range=%.0f "
                            "NoGravity=%d (second ammunition of category %d)", r.id,
                            r.type.c_str(), static_cast<double>(r.muzzle_speed),
                            static_cast<double>(r.range), r.no_gravity ? 1 : 0, gun.category);
                    }
                }
            }
            if (gun.bullet_class >= 0 && bullet(gun.bullet_class) == nullptr) {
                GameBulletClassRow b;
                b.id = gun.bullet_class;
                b.found = true;
                b.muzzle_speed = gun.muzzle_speed;
                b.range = gun.max_range;
                b.water_travel_speed = gun.water_travel_speed;
                b.swim_speed = gun.swim_speed;
                // 008568E0's two water-entry limits, read from the same Bullets
                // row the range derivation above reads. docs/TORPEDO_TICK.md.
                b.max_water_hit_vel = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxWaterHitVel", 0.0f);
                b.max_fall = lua.read_bullet_class_number(
                    gun.bullet_class, "MaxFall", 0.0f);
                b.heading_turn = lua.read_bullet_class_number(
                    gun.bullet_class, "HeadingTurn", 0.0f);
                b.damage_min = flat_scaled(type_id, make("dmin"), kMilliScale, 0.0f);
                b.damage_max = flat_scaled(type_id, make("dmax"), kMilliScale, 0.0f);
                b.water_damage = flat_scaled(type_id, make("wdmg"), kMilliScale, 0.0f);
                b.fire_damage = flat_scaled(type_id, make("fdmg"), kMilliScale, 0.0f);
                b.fire_chance = flat_scaled(type_id, make("fchance"), kMilliScale, 0.0f);
                b.mass = flat_scaled(type_id, make("mass"), kMilliScale, 0.0f);
                // Packet cc9_gun_ballistics: the row's Type string (the same one
                // the finaliser above maps to a sub-type) and NoGravity.
                b.type = lua.read_bullet_class_string(gun.bullet_class, "Type");
                b.no_gravity = flat_scaled(type_id, make("nograv"), kMilliScale, 0.0f) > 0.5f;
                log.notef("gunnery: bullet class %d \"%s\" Type=%s V0=%.1f range=%.0f "
                    "NoGravity=%d (first used by category %d)", b.id, b.name.c_str(),
                    b.type.c_str(), static_cast<double>(b.muzzle_speed),
                    static_cast<double>(b.range), b.no_gravity ? 1 : 0, gun.category);
                b.blast_damage_min = flat_scaled(type_id, make("bdmin"), kMilliScale, 0.0f);
                b.blast_damage_max = flat_scaled(type_id, make("bdmax"), kMilliScale, 0.0f);
                b.blast_range = flat_scaled(type_id, make("brange"), kMilliScale, 0.0f);
                bullets.push_back(b);
            }
            if (gun.device_class >= 0 && device(gun.device_class) == nullptr) {
                GameDeviceClassRow d;
                d.id = gun.device_class;
                d.found = true;
                d.function = gun.function;
                d.category = gun.category;
                d.horz_rot_speed = gun.speeds.horz;
                d.vert_rot_speed = gun.speeds.vert;
                d.bullet_class = gun.bullet_class;
                d.reload_time = gun.reload_time;
                d.barrel_delay_time = gun.barrel_delay_time;
                d.barrel_num = gun.barrel_num;
                devices.push_back(d);
            }

            // 007F5A10 treats the window list as a partition of the circle and
            // dereferences its search result with no null guard, so the list
            // must already hold a covering window before the first authored
            // insert. docs/GUN_PLATFORM_ARC.md records that seed as "somewhere
            // this packet did not find". Its flags are settled by the authored
            // data: `Turret A` of class 265 authors [-145,0] and [0,145] and
            // `Turret D` authors [30,180] and [-180,-30], each leaving the
            // sector its own superstructure blocks uncovered, so an uncovered
            // heading must refuse both tests and the seed carries flags 0.
            const int windows = flat(type_id, make("wn"), 0);
            bsp::GunFiringArc seed;
            seed.flags = 0;
            seed.min_horz = -bsp::kGunArcBoundLimit;
            seed.max_horz = bsp::kGunArcBoundLimit;
            seed.min_vert = -kHalfPi;
            seed.max_vert = kHalfPi;
            if (windows <= 0) {
                // A platform that authors no `Windows` has nothing to carve the
                // seed with, so the seed itself is what the gun traverses and
                // fires through. This is the executable's reading of an absent
                // key, not a recovered default.
                seed.flags = static_cast<std::uint8_t>(bsp::kGunArcFlagTraverse
                    | bsp::kGunArcFlagFire);
            }
            gun.arcs.push_back(seed);
            bool traverses = false;
            for (int w = 1; w <= windows && w <= kMaxWindowScan; ++w) {
                char wkey[40];
                auto wmake = [&wkey, p, w](const char* leaf) {
                    std::snprintf(wkey, sizeof(wkey), "p%d_w%d_%s", p, w, leaf);
                    return wkey;
                };
                bsp::GunFiringArc arc;
                const bool nofire = flat(type_id, wmake("nf"), 0) != 0;
                arc.flags = bsp::kGunArcFlagTraverse;
                if (!nofire) arc.flags = static_cast<std::uint8_t>(
                    arc.flags | bsp::kGunArcFlagFire);
                arc.min_horz = flat_scaled(type_id, wmake("minh"), kAngleScale, 0.0f);
                arc.max_horz = flat_scaled(type_id, wmake("maxh"), kAngleScale, 0.0f);
                arc.min_vert = flat_scaled(type_id, wmake("minv"), kAngleScale, 0.0f);
                arc.max_vert = flat_scaled(type_id, wmake("maxv"), kAngleScale, 0.0f);
                if (arc.min_horz != 0.0f || arc.max_horz != 0.0f ||
                    arc.min_vert != 0.0f || arc.max_vert != 0.0f) {
                    traverses = true;
                }
                bsp::gun_add_authored_arc_007f6b10(gun.arcs, arc);
            }
            // The guard below drops a gun whose authored windows produced no arc
            // beyond the seed, on the reading that the arc data failed. That is
            // right for a weapon that traverses and wrong for one that does not.
            // A BOMBPLATFORM authors exactly one window with all four bounds at
            // zero - measured: every one of the 81 guns this used to drop in
            // USN01 was category 0Ah, each with `arcs=1 windows=1` and a window
            // of minh=maxh=minv=maxv=0 - and 007F6B10 correctly declines to add
            // a zero-span arc, so the seed alone IS the complete answer for it.
            // Distinguishing on the authored data rather than on the category
            // keeps this a statement about what the data says.
            if (gun.arcs.size() <= 1 && windows > 0 && traverses) continue;
            done("Gunnery::add_authored_arc_007f6b10", 0x007f6b10u);

            gun.angles.horz = gun.rest_horz;
            gun.angles.vert = gun.rest_vert;
            gun.angles.target_horz = gun.rest_horz;
            gun.angles.target_vert = gun.rest_vert;
            gun.fire.barrel_timers.assign(static_cast<std::size_t>(gun.barrel_num), 0.0f);
            if (kShipPlatformAttachmentBound
                && units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) {
                ShipModelSlots& ship = ship_model_slots(type_id);
                bsp::GunPlatformSlotFrame frame;
                if (ship.loaded && bsp::gun_platform_slot_frame_0095f500(ship.items,
                        gun.platform_key, frame)) {
                    gun.mount_known = true;
                    gun.mount_local[0] = frame.origin[0];
                    gun.mount_local[1] = frame.origin[1];
                    gun.mount_local[2] = frame.origin[2];
                    ++mounts_from_model;
                    if (ship.logged_unit < 0 || ship.logged_unit == static_cast<long long>(i)) {
                        ship.logged_unit = static_cast<long long>(i);
                        log.notef("gunnery: mount %s platform %d (%s) cat=%d local=(%.2f %.2f %.2f) "
                            "forward=(%.2f %.2f %.2f) (0095F500 slot %d)", state.row.name.c_str(),
                            gun.platform_key, gun.platform_name.c_str(), gun.category,
                            static_cast<double>(frame.origin[0]), static_cast<double>(frame.origin[1]),
                            static_cast<double>(frame.origin[2]), static_cast<double>(frame.forward[0]),
                            static_cast<double>(frame.forward[1]), static_cast<double>(frame.forward[2]),
                            gun.platform_key);
                        // Packet cc9_gun_horz_sign: which side the authored fire
                        // windows face in the image's sign (positive = model -x, port).
                        std::string spans;
                        bool any = false, pos = true, neg = true;
                        for (std::size_t a = 0; a < gun.arcs.size(); ++a) {   // 007F5A10 sorts by angle
                            const auto& arc = gun.arcs[a];
                            if ((arc.flags & bsp::kGunArcFlagFire) == 0) continue;
                            any = true;
                            if (arc.min_horz < 0.0f) pos = false;
                            if (arc.max_horz > 0.0f) neg = false;
                            char buf[48];
                            std::snprintf(buf, sizeof buf, " [%.0f..%.0f]",
                                static_cast<double>(arc.min_horz * 57.2957795f),
                                static_cast<double>(arc.max_horz * 57.2957795f));
                            spans += buf;
                        }
                        const char* image_side = !any ? "none" : pos ? "port" : neg ? "starboard" : "both";
                        const char* mount_side = frame.origin[0] < -1.0f ? "port"
                            : frame.origin[0] > 1.0f ? "starboard" : "centre";
                        log.notef("gunnery: horz side %s platform %d cat=%d mount=%s image_faces=%s "
                            "host_faces=%s windows%s (008FDAF0/0085AB17)", state.row.name.c_str(),
                            gun.platform_key, gun.category, mount_side, image_side,
                            std::strcmp(image_side, "port") == 0 ? "starboard"
                                : std::strcmp(image_side, "starboard") == 0 ? "port" : image_side,
                            spans.c_str());
                    }
                    done("VehicleClass::bind_slot_frames_0095f500", 0x0095f500u);
                } else {
                    ++mounts_missing;
                }
            }
            guns.push_back(gun);
        }
    }

    // The ordnance inventory 007EEC50's AttackFeasibilityInputs need, handed to
    // the units host because the script-order host owns those inputs and cannot
    // see the guns. Aggregated over the unit's guns exactly as the 007ED7E0
    // family aggregates over the weapon controller's slots - the union of the
    // answer sets, since the family asks "does any slot carry kind N".
    // docs/ORDNANCE_KIND_IDENTITY.md. Stored at load, not at report time,
    // because the mission script issues its orders during luaStageInit.
    // Packet cc9_gunnery_host: stored for the NEW units only. This used to
    // re-store every unit from 0 on the reasoning that an earlier unit's
    // aggregate cannot change because `guns` only gains rows for new units. The
    // aggregate cannot, but the stored mask can: release_ordnance_drop clears a
    // torpedo bomber's kind 2Bh bit when it drops (packet cc8_torpedo_breakoff),
    // and the next spawn batch put the bit back, re-arming every spent bomber -
    // the fourth instance of a per-batch pass re-seeding units already in play,
    // after the health, the directors and the coordinator. USN04 at 9000
    // mission frames drops 12 torpedoes and then creates 12 more batches.
    // BSP_GUNNERY_RESTORE_ALL_ORDNANCE=1 restores the old store for a
    // same-binary pair; docs/GUNNERY_HOST_LIFETIME.md section 8.
    {
        std::vector<std::uint64_t> masks(count, 0u);
        for (const GameGunRow& gun : guns) {
            if (gun.unit_index < masks.size()) masks[gun.unit_index] |= gun.ordnance.mask;
        }
        unsigned long long rearms = 0;
        for (std::size_t i = 0; i < first_unit && i < count; ++i) {
            if (units.unit_ordnance(i) != masks[i]) ++rearms;
        }
        ordnance_rearms += rearms;
        const bool restore_all = restore_all_ordnance_enabled();
        if (rearms != 0) {
            log.notef("gunnery: spawn batch at unit %zu finds %llu already-built unit(s) whose "
                "ordnance mask a drop cleared; %s", first_unit, rearms,
                restore_all ? "RE-STORED (BSP_GUNNERY_RESTORE_ALL_ORDNANCE=1, the pre-fix "
                              "behaviour)" : "left cleared");
        }
        for (std::size_t i = restore_all ? 0 : first_unit; i < count; ++i) {
            units.store_unit_ordnance(i, masks[i]);
        }
    }

    // 00956C20: the twelve category lists at unit+394h, the all-guns list at
    // unit+424h and the ranges at unit+430h, over the device list this process
    // built. Run through the reconstruction's own sequence. New units only: an
    // already-built unit's gun set did not change, and the binding below
    // rebuilds every gun as `dead=false operational=true`, so re-running it
    // over an existing unit would put a gun this run has destroyed back into
    // its category lists.
    for (std::size_t i = first_unit; i < count; ++i) {
        UnitState& state = unit_state[i];
        std::vector<std::size_t> owned;
        for (std::size_t g = 0; g < guns.size(); ++g) {
            if (guns[g].unit_index == i) owned.push_back(g);
        }
        if (owned.empty()) continue;

        struct RebuildBinding final : bsp::UnitWeaponCategoryIndexHost {
            RebuildBinding(Impl& owner_in, UnitState& state_in,
                const std::vector<std::size_t>& owned_in)
                : owner(owner_in), state(state_in), owned(owned_in) {}
            void clear_torpedo_flag() override { torpedo = false; }
            void clear_list_00955eb0(int record) override {
                if (record < 0) {
                    all.clear();
                } else if (record < bsp::kUnitGunneryCategoryCount) {
                    state.category_guns[static_cast<std::size_t>(record)].clear();
                }
            }
            int device_count() override { return static_cast<int>(owned.size()); }
            bsp::GunneryRebuildDevice device_at(int index) override {
                bsp::GunneryRebuildDevice out;
                if (index < 0 || static_cast<std::size_t>(index) >= owned.size()) return out;
                const std::size_t slot = owned[static_cast<std::size_t>(index)];
                const GameGunRow& gun = owner.guns[slot];
                out.device = reinterpret_cast<void*>(slot + 1);
                out.dead = false;
                out.is_gun = true;
                out.operational = true;   // 00729F10 over an undamaged gun
                out.weapon_function = gun.category;
                out.max_range = gun.max_range;  // 00731020
                out.flak_alternate_range = gun.max_range;
                const GameBulletClassRow* b = owner.bullet(gun.bullet_class);
                out.has_blast = b != nullptr && b->blast_range > 0.0f;
                out.blast_inner = b != nullptr ? b->blast_range : 0.0f;
                out.blast_outer = b != nullptr ? b->blast_range : 0.0f;
                return out;
            }
            void append_to_category(int category, void* gun) override {
                if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return;
                state.category_guns[static_cast<std::size_t>(category)].push_back(
                    reinterpret_cast<std::size_t>(gun) - 1);
            }
            void append_to_all_guns(void* gun) override {
                all.push_back(reinterpret_cast<std::size_t>(gun) - 1);
            }
            void set_torpedo_flag() override { torpedo = true; }
            void store_category_range(int category, float range) override {
                if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return;
                state.category_ranges[static_cast<std::size_t>(category)] = range;
            }
            void store_category_blast_sum(int, float) override {}
            void store_artillery_max_range(float range) override {
                // 00956E43, unit+490h. Packet cc8_ship_ai_firepower_inputs.
                state.artillery_max_range = range;
            }
            void store_any_weapon_max_range(float range) override {
                // 00956E59, unit+494h, the maximum over every category.
                state.any_weapon_max_range = range;
            }

            Impl& owner;
            UnitState& state;
            const std::vector<std::size_t>& owned;
            std::vector<std::size_t> all;
            bool torpedo{false};
        };

        RebuildBinding binding(*this, state, owned);
        bsp::rebuild_weapon_category_index_00956c20(binding);
        done("Gunnery::rebuild_weapon_category_index_00956c20", 0x00956c20u);
        state.row.guns = owned.size();
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            const std::size_t slot = static_cast<std::size_t>(c);
            state.row.category_guns[slot]
                = static_cast<int>(state.category_guns[slot].size());
            state.row.category_ranges[slot] = state.category_ranges[slot];
        }
        state.row.artillery_max_range = state.artillery_max_range;
        state.row.any_weapon_max_range = state.any_weapon_max_range;
    }
}

void GameGunneryHost::Impl::attach_passes(std::size_t first_unit) {
    // New units only: the body below re-primes the throttle accumulator, the
    // category state, the bridge countdown and the fire cache, which is the
    // load-time seed and not something an already-ticking unit may be given a
    // second time.
    for (std::size_t i = first_unit; i < unit_state.size(); ++i) {
        UnitState& state = unit_state[i];
        if (state.row.guns == 0) continue;
        // 00864BD0: the base attach onto the unit's tick element unit+310h, the
        // visibility cache, the two policy objects, the twelve masks and enable
        // bytes, then the director bridge 00863A80 which runs 008624C0 forced.
        state.attached = true;
        state.enabled = true;
        bsp::GunneryThrottle throttle;
        bsp::gunnery_throttle_prime_00864c1d(throttle, think_time);
        state.throttle = throttle.accumulator;
        state.category = bsp::unit_gunnery_initial_category_state_00864580();
        state.bridge_countdown = 0;
        state.allow_fire_cache = false;
        state.row.pass_attached = true;
        state.row.pass_enabled = true;
        ++summary.passes_attached;
        done("Gunnery::pass_attach_00864bd0", 0x00864bd0u);
        done("Gunnery::pass_construct_00864580", 0x00864580u);
        done("Gunnery::install_policies_008636a0", 0x008636a0u);
        done("Gunnery::create_director_bridge_00863a80", 0x00863a80u);
    }
}

// ---------------------------------------------------------------------------
// 00864FE0: the two-second think
// ---------------------------------------------------------------------------

namespace {

// Packet cc9_aa_targeting instrumentation. _dupenv_s rather than getenv (a /W4
// /WX error under MSVC), read once.
std::string aa_env(const char* name) {
    char* text = nullptr;
    std::size_t bytes = 0;
    std::string out;
    if (_dupenv_s(&text, &bytes, name) == 0 && text != nullptr) out = text;
    std::free(text);
    return out;
}
const std::string& aa_trace_unit() {
    static const std::string v = aa_env("BSP_AA_TRACE_UNIT");
    return v;
}
const std::string& aa_trace_target() {
    static const std::string v = aa_env("BSP_AA_TRACE_TARGET");
    return v;
}
bool aa_target_matches(const std::string& name) {
    const std::string& prefix = aa_trace_target();
    return !prefix.empty() && name.compare(0, prefix.size(), prefix) == 0;
}

// The bsp::UnitGunneryPassHost binding for one unit, for one call.
class GunneryPassBinding final : public bsp::UnitGunneryPassHost {
public:
    GunneryPassBinding(GameGunneryHost::Impl& owner, std::size_t unit_index)
        : owner_(owner), unit_(unit_index), state_(owner.unit_state[unit_index]) {}

    bool unit_present() override { return true; }
    bool unit_is_dead() override { return state_.dead; }
    float throttle_threshold_00432650() override { return owner_.think_time; }
    bool unit_allows_sweep() override {
        // unit+61h. Milestone 2s: no writer outside the constructor.
        return owner_.units.unit_flag_0061(unit_);
    }

    void age_visibility_cache_00862c30(float elapsed) override {
        std::vector<GameGunneryHost::Impl::UnitState::VisibilityEntry> kept;
        kept.reserve(state_.visibility.size());
        for (auto& entry : state_.visibility) {
            float ttl = entry.ttl;
            if (bsp::visibility_entry_survives_00862c30(ttl, elapsed)) {
                entry.ttl = ttl;
                kept.push_back(entry);
            }
        }
        state_.visibility.swap(kept);
        owner_.done("Gunnery::age_visibility_cache_00862c30", 0x00862c30u);
    }

    bool has_director_bridge() override { return true; }

    void apply_director_stance_008624c0() override {
        // 008363E0 defaults director+3Ch and +3Dh to 1 for a unit of neither
        // class 0Bh nor class 9, and 007202FD sets all four of +220h..+223h to
        // 1 (docs/DIRECTOR_UPDATE_ARMS.md). No producer in this process moves
        // them, so the stance is the defaulted one.
        bsp::DirectorGunneryStance stance;
        const bool plane = owner_.units.unit_is_kind_of(unit_,
            bsp::kUnitGunneryKindPlaneBase);
        state_.category = bsp::apply_director_stance_008624c0(state_.category, stance,
            plane, false, state_.bridge_countdown, state_.allow_fire_cache);
        ++owner_.summary.bridge_applies;
        owner_.done("Gunnery::director_bridge_apply_008624c0", 0x008624c0u);
    }

    void update_target_records_00862cd0() override {
        // The this+B0h per-target engagement list. 00862CD0 and 00864880 are
        // contract: unread (docs/UNIT_GUNNERY_PASS.md section 10), so the list
        // is not built and nothing reads one back.
        owner_.record("Gunnery::update_target_records_00862cd0", 0x00862cd0u);
    }

    bool category_record_present(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        return !state_.category_guns[static_cast<std::size_t>(category)].empty();
    }
    bool category_enabled(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        const std::size_t slot = static_cast<std::size_t>(category);
        return state_.category.enabled[slot] && state_.category.mask[slot] != 0;
    }
    bool torpedo_category_enabled() override { return state_.category.torpedo_enable; }
    bool torpedo_may_take_fire_target() override { return true; }  // pass+7Dh, ctor 1
    bool category_gate_slot4(int) override {
        // 008636A0 installs the default gate 00D0D31C on a unit that is not
        // class 8; its vtable[4h] is 00861BE0, `mov al,1`.
        return true;
    }

    bool sweep_suppressed() override { return state_.sweep_suppressed; }

    int recon_contact_count_008053c0() override {
        // [recon+DE8h], the side's published enemy contact list.
        contacts_.clear();
        const int own_side = owner_.units.unit_side_0054(unit_);
        if constexpr (kReconTeamListsBound) {
            // Packet cc9_recon_team_lists: the published enemy triple itself.
            const auto* t = owner_.recon_triples_for(own_side);
            if (t != nullptr) {
                for (const std::size_t i : t->lists[1]) {
                    ++owner_.summary.contact_considered;
                    if (i == unit_) continue;
                    if (i < owner_.unit_state.size() && owner_.unit_state[i].dead) {
                        ++owner_.summary.contact_reject_dead;
                        continue;
                    }
                    const bool ship_base =
                        owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase);
                    const bool plane_base =
                        owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindPlaneBase);
                    if (!ship_base && !plane_base) {
                        ++owner_.summary.contact_reject_kind;
                        continue;
                    }
                    if (plane_base) ++owner_.summary.contact_admit_plane;
                    else ++owner_.summary.contact_admit_ship;
                    contacts_.push_back(i);
                }
            }
            ++owner_.summary.recon_sweeps;
            owner_.done("Gunnery::recon_slot_contacts_008053c0", 0x008053c0u);
            return static_cast<int>(contacts_.size());
        }
        const std::size_t count = owner_.units.count();
        for (std::size_t i = 0; i < count; ++i) {
            if (i == unit_) continue;
            ++owner_.summary.contact_considered;
            if (owner_.units.unit_side_0054(i) == own_side) {
                ++owner_.summary.contact_reject_side;
                continue;
            }
            // docs/RECON_SLOT_LISTS.md rule (a): the class must be one the scan
            // visits. rule (b): +5Ch set, +5Dh / +5Eh / +60h clear.
            if (!owner_.units.unit_alive_and_visible(i)) {
                ++owner_.summary.contact_reject_visible;
                continue;
            }
            if (i < owner_.unit_state.size() && owner_.unit_state[i].dead) {
                ++owner_.summary.contact_reject_dead;
                continue;
            }
            // Rule (a) is "a class the scan visits", and the native scan
            // 00806480 visits seven PLANE leaf ids under IsKindOf(02h) as well
            // as the ship bases. Admitting only ship bases is why nothing ever
            // shot at an aircraft: on IJN01 the seven A7M fighters took zero
            // hits and zero damage while 295 AAMACHINEGUN and 60 FLAK guns sat
            // idle. docs/PLANE_UNIT_TICK.md.
            const bool ship_base =
                owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase);
            const bool plane_base =
                owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindPlaneBase);
            if (!ship_base && !plane_base) {
                ++owner_.summary.contact_reject_kind;
                continue;
            }
            // docs/RECON_SLOT_LISTS.md rule (c): the level the sensor pass
            // published for (own side, this target). `none` means the entry
            // drained out of every published list, so the sweep must not see
            // it. A side the pass never covered keeps
            // kReconDetectionUnknownLevel, the permissive answer the tree used
            // before rule (c) ran.
            const bsp::ReconDetectionLevel level = owner_.recon_pass.level(own_side, i);
            if (level == bsp::ReconDetectionLevel::none) {
                ++owner_.summary.contact_reject_recon_level;
                continue;
            }
            if (plane_base) ++owner_.summary.contact_admit_plane;
            else ++owner_.summary.contact_admit_ship;
            contacts_.push_back(i);
        }
        ++owner_.summary.recon_sweeps;
        owner_.record("Gunnery::recon_slot_contacts_008053c0", 0x008053c0u);
        return static_cast<int>(contacts_.size());
    }
    void* recon_contact(int index) override {
        if (index < 0 || static_cast<std::size_t>(index) >= contacts_.size()) return nullptr;
        return handle(contacts_[static_cast<std::size_t>(index)]);
    }

    bool score_candidate_00863990(int category, void* target, float& distance) override {
        // Packet cc8_torpedo_gun_assignment: the per-reason funnel for the
        // torpedo category only. Every `return false` below gets its own
        // counter, so the run says which guard refuses and not merely that one
        // did. The counters are instrumentation; no native address produces them.
        const bool torpedo_cat = category == bsp::kUnitGunneryTorpedoCategory;
        if (torpedo_cat) ++owner_.summary.torpedo_cat_score_calls;
        const std::size_t other = unit_of(target);
        if (other >= owner_.units.count()) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_unknown;
            return false;
        }
        // 008633D0 then 00862820, then the range gate and the plane penalty.
        const bool is_plane = owner_.units.unit_is_kind_of(other,
            bsp::kUnitGunneryKindPlaneBase);
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return false;
        const std::size_t slot = static_cast<std::size_t>(category);
        bsp::GunneryTargetLiveness liveness;
        liveness.registered = owner_.units.unit_active(other);
        liveness.dead = owner_.unit_state[other].dead;
        if (!bsp::target_is_engageable_00862820(liveness)) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_liveness;
            return false;
        }
        if (owner_.units.unit_class_id(other) < 0) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_class;
            return false;
        }
        if (bsp::gunnery_rank(owner_.rank_table.data(), category,
                owner_.units.unit_class_id(other)) == 0) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_rank;
            return false;
        }
        if (!bsp::category_mask_admits_target_008633d0(state_.category.mask[slot],
                is_plane)) {
            if (torpedo_cat) ++owner_.summary.torpedo_cat_reject_mask;
            return false;
        }
        float mine[3], theirs[3];
        owner_.unit_aim_point(unit_, mine);
        owner_.unit_aim_point(other, theirs);
        const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
            theirs[2] - mine[2]};
        bsp::GunneryScoreInputs in;
        in.distance = length3(delta);
        in.category_range = state_.category_ranges[slot];
        in.target_is_plane = is_plane;
        in.target_lacks_follow_target = false;
        const bsp::GunneryScoreResult out = bsp::score_candidate_00863990(in);
        distance = out.distance;
        if (!aa_trace_unit().empty() && state_.row.name == aa_trace_unit()) {
            char line[256];
            std::snprintf(line, sizeof line, "    cand cat=%d %-28s rank=%d dist=%.0f "
                "range=%.0f alt=%.0f %s", category,
                owner_.unit_state[other].row.name.c_str(),
                bsp::gunnery_rank(owner_.rank_table.data(), category,
                    owner_.units.unit_class_id(other)),
                static_cast<double>(in.distance), static_cast<double>(in.category_range),
                static_cast<double>(theirs[1]), out.accepted ? "accepted" : "out_of_range");
            owner_.aa_cand_lines.emplace_back(line);
        }
        if (aa_target_matches(owner_.unit_state[other].row.name)) {
            GameGunneryHost::Impl::AaTargetStats& st = owner_.aa_target_stats[other];
            ++st.scored;
            if (out.accepted) ++st.accepted;
            else ++st.range_rejects;
            if (st.min_dist < 0.0f || in.distance < st.min_dist) {
                st.min_dist = in.distance;
                st.alt_at_min = theirs[1] - mine[1];
                st.ship_at_min = state_.row.name;
                st.category_at_min = category;
            }
            if (!out.accepted && (st.min_reject_dist < 0.0f || in.distance < st.min_reject_dist)) {
                st.min_reject_dist = in.distance;
                st.reject_range_at_min = in.category_range;
            }
        }
        owner_.done("Gunnery::score_candidate_00863990", 0x00863990u);
        if (out.distance > 0.0f
            && (state_.row.nearest_enemy <= 0.0f || out.distance < state_.row.nearest_enemy)) {
            state_.row.nearest_enemy = out.distance;
        }
        if (in.category_range > state_.row.best_range) {
            state_.row.best_range = in.category_range;
        }
        if (out.accepted) ++accepted_;
        else ++rejected_;
        if (torpedo_cat) {
            // Everything before this point passed, so the only guard left inside
            // 00863990 is the range test against the category range at
            // owner+category*4+430h. Recording the two values on the first few
            // rejections is what turns "the range refused" into a number.
            if (out.accepted) {
                ++owner_.summary.torpedo_cat_score_accepted;
            } else {
                ++owner_.summary.torpedo_cat_reject_range;
                if (owner_.summary.torpedo_cat_reject_range <= 3) {
                    owner_.log.notef("gunnery: torpedo category refused a target at "
                        "%.0f m against a category range of %.0f m (00863990)",
                        static_cast<double>(in.distance),
                        static_cast<double>(in.category_range));
                }
            }
        }
        return out.accepted;
    }

    bool unit_ai_suppresses_00862440(void* target) override {
        const std::size_t other = unit_of(target);
        // entity+1D4h, the script-set untouchable flag. No binding in this
        // mission sets it, so the proxy answers clear.
        const bool untouchable = false;
        owner_.done("Gunnery::untouchable_gate_00862440", 0x00862440u);
        return bsp::entity_suppresses_gunnery_00862440(other < owner_.units.count(),
            untouchable);
    }

    bool visible_00864d90(void* target) override {
        const std::size_t other = unit_of(target);
        for (const auto& entry : state_.visibility) {
            if (entry.target == other) return entry.visible;
        }
        // 00864680, the line-of-sight test itself, is contract: unread. Over
        // open water with no terrain in this process the answer is yes.
        owner_.record("Gunnery::line_of_sight_00864680", 0x00864680u);
        GameGunneryHost::Impl::UnitState::VisibilityEntry entry;
        entry.target = other;
        entry.visible = true;
        entry.ttl = owner_.draw(GameGunneryHost::Impl::Draw::visibility_ttl, unit_, other,
            0.0f,
            bsp::kInstalledLosVisibleTimeOut);
        state_.visibility.push_back(entry);
        owner_.done("Gunnery::visibility_cache_append_00864d90", 0x00864d90u);
        return true;
    }

    int target_rank(int category, void* target) override {
        const std::size_t other = unit_of(target);
        if (other >= owner_.units.count()) return 0;
        return bsp::gunnery_rank(owner_.rank_table.data(), category,
            owner_.units.unit_class_id(other));
    }

    void* director_fire_target_slot4() override {
        return state_.fire_target != 0 ? handle(state_.fire_target - 1) : nullptr;
    }
    void* director_command_target_0071ebf0() override {
        return state_.command_target != 0 ? handle(state_.command_target - 1) : nullptr;
    }

    std::array<float, 3> unit_world_position_00427eb0() override {
        float point[3];
        owner_.unit_aim_point(unit_, point);
        return {point[0], point[1], point[2]};
    }
    int sub_entities_slot0fc(void* target) override {
        // target->vtable[0FCh]. The base implementation 00432480 is what 92 of
        // the 94 entity-hierarchy vtables carry in this slot - every ship, gun
        // platform and projectile - and it appends the entity itself, once and
        // unconditionally: PUSH ECX / MOV [ESP],ECX / MOV ECX,[ESP+8] /
        // PUSH EAX / CALL 004323D0 BSP_PointerVector_PushBack / RET 4.
        // The two overrides are MAirfield 006D4DD0 (intact hangars) and the
        // plane squadron 007F44E0 (live planes); neither is a unit this
        // process creates, so a target here always takes the base.
        // docs/SHIP_SUB_ENTITY_LIST.md.
        owner_.record("Gunnery::target_sub_entities_slot0fc", 0x008654acu);
        sub_entity_ = target;
        // Provenance for the diagnostic counters: this method is reached only from
        // step 8.7's two arms, so anything it hands back came from the director's
        // targets rather than from the recon sweep.
        if (target != nullptr) arm_entities_.insert(target);
        return target != nullptr ? 1 : 0;
    }
    void* sub_entity(int index) override {
        return index == 0 ? sub_entity_ : nullptr;
    }
    std::array<float, 3> entity_world_position(void* entity) override {
        const std::size_t other = unit_of(entity);
        float point[3] = {0.0f, 0.0f, 0.0f};
        if (other < owner_.units.count()) owner_.unit_aim_point(other, point);
        return {point[0], point[1], point[2]};
    }
    float vector_length_0042b2f0(const std::array<float, 3>& v) override {
        const float raw[3] = {v[0], v[1], v[2]};
        return length3(raw);
    }

    int category_gun_count(int category) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return 0;
        const int guns = static_cast<int>(
            state_.category_guns[static_cast<std::size_t>(category)].size());
        if (guns > 0) ++owner_.summary.assignment_passes;
        return guns;
    }
    void* category_gun(int category, int index) override {
        if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return nullptr;
        const auto& list = state_.category_guns[static_cast<std::size_t>(category)];
        if (index < 0 || static_cast<std::size_t>(index) >= list.size()) return nullptr;
        return reinterpret_cast<void*>(list[static_cast<std::size_t>(index)] + 1);
    }
    bsp::GunneryGunInputs gun_inputs(void* gun, void* target) override {
        bsp::GunneryGunInputs in;
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        if (slot >= owner_.guns.size()) return in;
        const GameGunRow& row = owner_.guns[slot];
        in.weapon_sub_type = row.category;
        in.is_torpedo_class_launcher = row.category == bsp::kUnitGunneryTorpedoCategory;
        in.minimum_air_range = 0.0f;
        // 00729BC0: the projectile-kind dispatch onto one of the six bot slots,
        // then that slot's vtable[1Ch]. The slot exists when the category maps
        // to one, and the range gate is the ammunition record's own +60h.
        const std::size_t other = unit_of(target);
        bool in_range = false;
        if (other < owner_.units.count()) {
            float mine[3], theirs[3];
            owner_.unit_aim_point(unit_, mine);
            owner_.unit_aim_point(other, theirs);
            const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
                theirs[2] - mine[2]};
            const GameGunneryHost::Impl::SecondAmmo* dp = owner_.dp_air_ammo(slot,
                row.category, other);
            in_range = length3(delta) <= (dp != nullptr ? dp->max_range : row.max_range);
        }
        in.slot_accepts_target = in_range;
        // Evaluated on every call so a run with the switch off still counts what
        // the bound terms would refuse; applied only when that term's own switch is on.
        {
            // Evaluated on every call; each term is applied only when its own
            // switch is on, and counted either way.
            bsp::GunneryGunInputs observed = in;
            bind_aa_acceptance(row, other, observed);
            if constexpr (kAaMinRangeBound) {
                in.is_torpedo_class_launcher = observed.is_torpedo_class_launcher;
                in.minimum_air_range = observed.minimum_air_range;
            }
            if constexpr (kAaFireWindowBound || kAaArmourBound) {
                if (!observed.slot_accepts_target
                    && ((kAaFireWindowBound && last_refusal_ == AaRefusal::window)
                        || (kAaArmourBound && last_refusal_ == AaRefusal::armour)
                        || (kAaLineOfFireBound && last_refusal_ == AaRefusal::line_of_fire))) {
                    in.slot_accepts_target = false;
                }
            }
        }
        if (!in_range && other < owner_.unit_state.size()
            && aa_target_matches(owner_.unit_state[other].row.name)) {
            ++owner_.aa_target_stats[other].gun_range_rejects;
        }
        ++owner_.summary.gun_evaluations;
        if (!in_range) ++owner_.summary.gun_slot_rejects;
        owner_.done("Gunnery::bot_slot_accepts_target_00729bc0", 0x00729bc0u);
        return in;
    }
    // Packet cc9_aa_targeting, each term behind its own switch above. docs/AA_TARGETING.md.
    //
    // 005459E0 / 00729B90 (00865773's skip): the weapon kind [[gun+3F4h]+80h] is
    // the device row's Function id, and the test is kind 5 or 6 - FLAK and
    // LIGHTARTILLERYFLAK - against the ammunition's MinRange (+58h). The host
    // keyed the flag on category 7 (TORPEDO) with a zero minimum. Kind 6 reads
    // its SECOND ammunition entry (+74h+7Ch) against a plane, which this host
    // does not load, so kind 6 keeps a zero minimum: unbound, labelled.
    //
    // 00729BC0 -> the bot slot's vtable[1Ch], for the two AA bots only:
    //  * AAGunnerBot 008FBE00 (category 1, slot +390h): a target answering
    //    IsKindOf(4) is refused when max(DamageMax +B0h, BlastDamageMax +B8h) of
    //    the round is <= the target class's Armour (+4Ch); then 0085A9A0.
    //  * AAFlakBot 008FBFC0 (category 5, slot +394h): 0085A9A0 alone.
    //  0085A9A0 turns the gun-to-target direction into the gun frame and answers
    //  BSP_GunPlatform_AnglesInFireWindow 007F60A0 at (-0.0 - yaw, pitch), the
    //  same negated convention 008FDAF0 produces for the aim. SUBSTITUTION: the
    //  host has no per-gun node frame, so this uses the hull frame and the
    //  shared muzzle point the host's aim code already uses. Both bots then ask
    //  the gun's vtable[1D4h] = 0072F6E0, which answers 1 while gun+42Ch is zero
    //  and otherwise caches a line-of-fire predicate whose installer is unread;
    //  it stays true here.
    enum class AaRefusal { none, window, armour, line_of_fire };
    AaRefusal last_refusal_{AaRefusal::none};
    void bind_aa_acceptance(const GameGunRow& row, std::size_t other,
                            bsp::GunneryGunInputs& in) {
        last_refusal_ = AaRefusal::none;
        if (other >= owner_.units.count()) return;
        const bool plane = owner_.units.unit_is_kind_of(other,
            bsp::kUnitGunneryKindPlaneBase);
        const int kind = row.category;
        in.is_torpedo_class_launcher = kind == 5 || kind == 6;       // 005459E0
        in.minimum_air_range = 0.0f;
        if (kind == 5) {                                              // 00729B90
            const auto it = owner_.bullet_min_range.find(row.bullet_class);
            if (it != owner_.bullet_min_range.end()) in.minimum_air_range = it->second;
        }
        if (kind == 6) {       // 00729BA9: kind 6 reads the SECOND record's +58h
            const GameGunneryHost::Impl::SecondAmmo* dp = owner_.dp_air_ammo(
                static_cast<std::size_t>(&row - owner_.guns.data()), kind, other);
            if (dp != nullptr) in.minimum_air_range = dp->min_range;
        }
        float right[3], up[3], forward[3], origin[3];
        owner_.unit_pose(unit_, right, up, forward, origin);
        float tr[3], tu[3], tf[3], target_pos[3];
        owner_.unit_pose(other, tr, tu, tf, target_pos);
        float muzzle[3];
        owner_.gun_muzzle_point(row, state_, right, up, forward, origin, muzzle);
        const float d[3] = {target_pos[0] - muzzle[0], target_pos[1] - muzzle[1],
            target_pos[2] - muzzle[2]};
        const float len = length3(d);
        if (plane && in.is_torpedo_class_launcher && in.minimum_air_range > len) {
            ++owner_.aa_min_range_skips;
            ++owner_.aa_min_range_skips_by_gun[static_cast<std::size_t>(&row - owner_.guns.data())];
        }
        if (!in.slot_accepts_target) return;
        // 008FBE00 / 008FBFC0 end with vtable[1D4h] = 00730A20 -> 0072F6E0, the
        // line-of-fire decision 00729560 installs for kinds 1, 5 and 6.
        auto line_of_fire_refuses = [&]() -> bool {
            if constexpr (!kAaLineOfFireBound) return false;
            if (kind != 1 && kind != 5 && kind != 6) return false;
            const std::size_t g = static_cast<std::size_t>(&row - owner_.guns.data());
            const auto key = std::make_pair(g, other);
            auto cached = owner_.line_of_fire_cache.find(key);
            bool clear;
            if (cached != owner_.line_of_fire_cache.end()) {
                clear = cached->second;
            } else {
                ++owner_.line_of_fire_queries;
                clear = !owner_.line_of_fire_blocked_0072cdd0(unit_, other, muzzle);
                if (!clear) ++owner_.line_of_fire_blocked;
                owner_.line_of_fire_cache.emplace(key, clear);
                owner_.done("Gun::target_shot_decision_0072f6e0", 0x0072f6e0u);
            }
            if (clear) return false;
            in.slot_accepts_target = false;
            last_refusal_ = AaRefusal::line_of_fire;
            ++owner_.line_of_fire_refusals;
            return true;
        };
        if (kind == 6) { line_of_fire_refuses(); return; }
        if (kind != 1 && kind != 5) return;
        if (kind == 1 && owner_.units.unit_is_kind_of(other, 4)) {      // 008FBE17
            const GameBulletClassRow* b = owner_.bullet(row.bullet_class);
            const float best = b != nullptr ? std::max(b->damage_max, b->blast_damage_max)
                                            : 0.0f;
            if (b != nullptr && best <= owner_.unit_state[other].armour) {
                in.slot_accepts_target = false;
                last_refusal_ = AaRefusal::armour;
                ++owner_.aa_armour_rejects;
                return;
            }
        }
        if (len <= 0.0f || row.arcs.empty()) return;
        const float u[3] = {d[0] / len, d[1] / len, d[2] / len};
        const float horz = kGunHorzSign * std::atan2(dot3(u, right), dot3(u, forward));  // 0085AB17
        const float vert = std::asin(std::max(-1.0f, std::min(1.0f, dot3(u, up))));
        const bsp::GunPlatformArcs arcs{row.arcs.data(), row.arcs.size()};
        if (!bsp::gun_fire_allowed_007f60a0(arcs, horz, vert)) {        // 0085A9A0
            in.slot_accepts_target = false;
            last_refusal_ = AaRefusal::window;
            ++owner_.aa_window_rejects;
            return;
        }
        line_of_fire_refuses();
    }
    bool target_is_plane(void* target) override {
        const std::size_t other = unit_of(target);
        return other < owner_.units.count()
            && owner_.units.unit_is_kind_of(other, bsp::kUnitGunneryKindPlaneBase);
    }
    void set_bot_fire_target_00727f10(void* gun, void* target, bool is_fire_target) override {
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        const std::size_t other = unit_of(target);
        if (slot >= owner_.guns.size()) return;
        GameGunRow& row = owner_.guns[slot];
        if (!aa_trace_unit().empty() && state_.row.name == aa_trace_unit()
            && row.target_unit != other + 1) {
            float mine[3], theirs[3];
            owner_.unit_aim_point(unit_, mine);
            owner_.unit_aim_point(other, theirs);
            const float d[3] = {theirs[0] - mine[0], theirs[1] - mine[1], theirs[2] - mine[2]};
            owner_.log.notef("  aa trace t=%.2f %s gun=%zu cat=%d %s max_range=%.0f: %s -> %s "
                "dist=%.0f alt=%.0f fire_target=%d",
                static_cast<double>(owner_.clock_seconds), state_.row.name.c_str(), slot,
                row.category, row.function.c_str(), static_cast<double>(row.max_range),
                row.target_name.empty() ? "none" : row.target_name.c_str(),
                other < owner_.unit_state.size() ? owner_.unit_state[other].row.name.c_str() : "?",
                static_cast<double>(length3(d)), static_cast<double>(d[1]),
                is_fire_target ? 1 : 0);
            owner_.aa_changed = true;
        }
        if (other < owner_.unit_state.size() && aa_target_matches(owner_.unit_state[other].row.name)) {
            ++owner_.aa_target_stats[other].assigns;
        }
        row.target_unit = other + 1;
        row.target_name = other < owner_.unit_state.size()
            ? owner_.unit_state[other].row.name : std::string();
        row.target_is_fire_target = is_fire_target;
        ++row.assigns;
        ++state_.row.assigns;
        ++owner_.summary.assigns;
        {
            const bool from_arm = arm_entities_.count(target) != 0;
            float fraction = 0.0f;
            if (other < owner_.units.count() && row.max_range > 0.0f) {
                float mine[3], theirs[3];
                owner_.unit_aim_point(unit_, mine);
                owner_.unit_aim_point(other, theirs);
                const float delta[3] = {theirs[0] - mine[0], theirs[1] - mine[1],
                    theirs[2] - mine[2]};
                fraction = length3(delta) / row.max_range;
            }
            if (from_arm) {
                ++owner_.summary.assigns_from_arm;
                owner_.summary.arm_reach_fraction_sum += fraction;
                if (fraction > 0.5f) ++owner_.summary.arm_assigns_beyond_half;
            } else {
                ++owner_.summary.assigns_from_recon;
                owner_.summary.recon_reach_fraction_sum += fraction;
                if (fraction > 0.5f) ++owner_.summary.recon_assigns_beyond_half;
            }
        }
        owner_.done("Gunnery::set_bot_fire_target_00727f10", 0x00727f10u);
    }
    void add_gun_to_target_record_00864ca0(void*, void*) override {
        owner_.record("Gunnery::add_gun_to_target_record_00864ca0", 0x00864ca0u);
    }
    void clear_bot_fire_target_00728000(void* gun) override {
        const std::size_t slot = reinterpret_cast<std::size_t>(gun) - 1;
        if (slot >= owner_.guns.size()) return;
        GameGunRow& row = owner_.guns[slot];
        if (!aa_trace_unit().empty() && state_.row.name == aa_trace_unit()
            && row.target_unit != 0) {
            owner_.log.notef("  aa trace t=%.2f %s gun=%zu cat=%d %s max_range=%.0f: %s -> none",
                static_cast<double>(owner_.clock_seconds), state_.row.name.c_str(), slot,
                row.category, row.function.c_str(), static_cast<double>(row.max_range),
                row.target_name.c_str());
            owner_.aa_changed = true;
        }
        ++row.clears;
        ++state_.row.clears;
        ++owner_.summary.clears;
        row.target_unit = 0;
        row.target_name.clear();
        row.target_is_fire_target = false;
        owner_.done("Gunnery::clear_bot_fire_target_00728000", 0x00728000u);
    }

    std::size_t candidates_seen() const noexcept { return accepted_; }
    std::size_t candidates_rejected() const noexcept { return rejected_; }

private:
    void* handle(std::size_t unit_index) const {
        return reinterpret_cast<void*>(unit_index + 1);
    }
    std::size_t unit_of(void* pointer) const {
        return pointer != nullptr ? reinterpret_cast<std::size_t>(pointer) - 1
                                  : owner_.units.count();
    }

    GameGunneryHost::Impl& owner_;
    std::size_t unit_;
    GameGunneryHost::Impl::UnitState& state_;
    std::vector<std::size_t> contacts_;
    std::size_t accepted_{0};
    std::size_t rejected_{0};
    void* sub_entity_{nullptr};   // the one entry 00432480 appends: the target itself
    std::set<void*> arm_entities_;   // entities handed out by step 8.7 this pass
};

}  // namespace

// The per-unit answer to 0071EBF0, resolved once rather than per unit per tick.
// A first cut did the name match inside run_gunnery_pass and cost O(commands x
// units) every unit every tick - 77 units against 83 current commands - which
// took a 40-second mission past ten minutes. The semantics are unchanged; only
// the placement is.
// 0071EBF0's rule, which is NOT "the last current row wins".
//
// With [this+30h] == 1 the image scans the unit's own slot array at +54h - ten
// entries of 0x1Ch, the scan stopping at the first NULL (0071EC10-0071EC1D) -
// then walks BACKWARD from the last occupied entry (0071EC1F `LEA ESI,[EAX-1]`,
// stepping at 0071EC4D/0071EC50) and takes the first whose `vtable[+0Ch]`
// answers 1 or 2 (0071EC43 `CMP EAX,1` and 0071EC48 `CMP EAX,2`, both to
// 0071ECC6). A command of any other category is stepped OVER, not taken. When
// none answers, 0071EC57 falls to the lazily-built static at 00E19BB4, which is
// a neutral record and never another unit. ([this+30h] == 2 returns this+18Ch
// instead; this host has no such mode.)
//
// The host's rows carry both fields the rule needs: `slot_index` is the entry
// 0071E6C0 pushed and `category` is what vtable[+0Ch] answers. Taking rows in
// vector order and ignoring the category is what let a `moveto` row (category
// 3) appearing mid-mission take the dive bomber's target off its `divebomb` row
// (category 2) and re-point it at a unit whose position equalled the aircraft's,
// which showed up as approach+BCh = exactly 0.0 m in local/usn04_aim.log.
// docs/DIVE_BOMB_TASK.md, "The aim run".
//
// Resolving the accepted row's token is a SECOND step on purpose. The image
// returns the accepted slot's target field whatever it holds, so an accepting
// row whose token names nothing leaves the unit with no target - it does not
// fall through to an older row.
void GameGunneryHost::Impl::refresh_command_targets() {
    const std::vector<GameCommandRow>& command_rows = units.commands().rows();
    // The cache key is the row count AND how many of them are current, so a row
    // going current or stale without the vector growing still re-resolves. Both
    // are O(commands), not the O(commands x units) the name match costs.
    std::size_t current_rows = 0;
    for (const GameCommandRow& command : command_rows) {
        if (command.current) ++current_rows;
    }
    if (command_rows_resolved == command_rows.size()
        && command_rows_current == current_rows
        && command_target_by_unit.size() == units.count()) {
        return;
    }
    command_rows_resolved = command_rows.size();
    command_rows_current = current_rows;
    command_target_by_unit.assign(units.count(), 0);
    std::map<std::string, std::size_t> by_name;
    for (std::size_t i = 0; i < units.count(); ++i) {
        const GameUnitRow* row = units.unit_row(i);
        if (row != nullptr && !row->name.empty()) by_name.emplace(row->name, i + 1);
    }
    // Step one, the backward walk: per unit, the accepting row that sits in the
    // highest slot. A row the host never pushed carries slot_index -1; those
    // are ordered behind every pushed row and among themselves by vector
    // position, which is the best standing this host has for them.
    std::vector<const GameCommandRow*> accepted(units.count(), nullptr);
    std::vector<long long> accepted_rank(units.count(), -1);
    long long position = 0;
    for (const GameCommandRow& command : command_rows) {
        ++position;
        if (!command.current) continue;
        if (command.unit_index >= accepted.size()) continue;
        // 0071EC43 and 0071EC48: only these two categories answer.
        if (command.category != 1 && command.category != 2) continue;
        const long long rank = command.slot_index >= 0
            ? (static_cast<long long>(command.slot_index) << 32) + position
            : position;
        if (rank <= accepted_rank[command.unit_index]) continue;
        accepted_rank[command.unit_index] = rank;
        accepted[command.unit_index] = &command;
    }
    // Step two: resolve only that row's token.
    for (std::size_t i = 0; i < accepted.size(); ++i) {
        if (accepted[i] == nullptr || accepted[i]->target_token.empty()) continue;
        const std::map<std::string, std::size_t>::const_iterator found =
            by_name.find(accepted[i]->target_token);
        if (found != by_name.end()) command_target_by_unit[i] = found->second;
        // Packet cc8_torpedo_aim_census. 0071EBF0's rule picks a row per unit
        // and resolves its token BY NAME; "command_targets units_with=N" said
        // how many resolved and never which, so the ship a bomber is actually
        // attacking was unnamed in every run this stream has taken.
        log.notef("  command target 0071EBF0: unit=%s token=\"%s\" -> %s",
            (i < unit_state.size() ? unit_state[i].row.name.c_str() : "?"),
            accepted[i]->target_token.c_str(),
            found != by_name.end()
                ? (found->second != 0 && found->second - 1 < unit_state.size()
                       ? unit_state[found->second - 1].row.name.c_str()
                       : "(index out of range)")
                : "(no unit of that name)");
    }
    command_targets_resolved = 0;
    for (std::size_t i = 0; i < command_target_by_unit.size(); ++i) {
        if (command_target_by_unit[i] != 0) ++command_targets_resolved;
        // The plane's control path needs the same answer, and taking it from
        // here rather than resolving names again is what keeps the two from
        // drifting apart.
        units.store_unit_command_target(i, command_target_by_unit[i]);
    }
}

// ---------------------------------------------------------------------------
// docs/RECON_SLOT_LISTS.md rule (c), the sensor pass. 008073C0 over 00806840.
// ---------------------------------------------------------------------------

// scripts/datatables/autoload/reconclasses.lua takes `RealisticTable` only when
// the global `GameMode` is 1 and falls through to `ArcadeTable` otherwise. This
// installation's root gamemode.lua sets `GameMode = 0` (and a nil GameMode takes
// the same arm), so arcade is the image's table here too.
// docs/CLASSTABLE_SELECTION.md.
inline constexpr bsp::ReconTableVariant kReconVariant = bsp::ReconTableVariant::arcade;

const GameGunneryHost::Impl::ReconClassRecord*
GameGunneryHost::Impl::recon_record_for_class(int recon_class_id) {
    auto found = recon_classes.find(recon_class_id);
    if (found != recon_classes.end()) return found->second.get();
    auto made = std::make_unique<ReconClassRecord>();
    // 008082A0 on the authored rows. An id outside 1..12 gives the empty record
    // the loader leaves behind, which 008048A0 then reads as "no entries".
    made->table = bsp::build_sensor_class_table_008082a0(kReconVariant, recon_class_id);
    for (std::size_t i = 0; i < bsp::kSensorListCount; ++i) {
        const std::vector<bsp::SensorTableEntry>& src = made->table.lists[i];
        std::vector<bsp::ReconSensorEntry>& dst = made->entries[i];
        dst.reserve(src.size());
        for (const bsp::SensorTableEntry& row : src) {
            // Field for field onto the 1Ch entry 008048A0 walks. The loader has
            // already squared Dist (008085F1), halved Gain (00808626) and
            // turned MaxLevel into the cap (0080866B), so nothing is re-derived
            // here: this is the record layout change only.
            bsp::ReconSensorEntry entry;
            entry.authored_distance = row.dist;             // +0h
            entry.max_normalized_distance_sq = row.dist_sq; // +4h
            entry.gain_per_second = row.gain_per_second;    // +8h
            entry.cap = row.max_value;                      // +0Ch
            entry.mask_bit = row.raw_type;                  // +10h
            entry.max_bearing_error = row.half_angle;       // +14h
            entry.bearing_limited = row.bearing_limited;    // +18h
            dst.push_back(entry);
        }
        made->rows[i].entries = dst.empty() ? nullptr : dst.data();
        made->rows[i].count = dst.size();
    }
    const ReconClassRecord* raw = made.get();
    recon_classes.emplace(recon_class_id, std::move(made));
    return raw;
}

void GameGunneryHost::Impl::resolve_recon_inputs() {
    const std::size_t count = units.count();
    if (unit_recon_record.size() == count) return;
    unit_recon_record.assign(count, nullptr);
    unit_recon_modifier_sq.assign(count, bsp::kGunneryReconModifierSqDefault);
    for (std::size_t i = 0; i < count; ++i) {
        // The `VehicleClass` table index, which is GameUnitRow::type_id and NOT
        // GameUnitsHost::unit_class_id: the latter is the entity class id the
        // IsKindOf chain walks (06h ship base, 0Fh plane base), and indexing
        // the authored table with it resolves every unit to one wrong row.
        // build_guns takes type_id for the same reason.
        const GameUnitRow* row = units.unit_row(i);
        const int class_id = row != nullptr ? row->type_id : -1;
        if (class_id < 0) continue;
        // `ReconClass` is the integer field 00962... reads next to
        // `ReconModifier`; the row selects which ReconClass[] record the unit's
        // class+B4h table pointer ends up at. 637 of the installed rows carry
        // it, across the twelve ids the shipped table defines.
        const int recon_class = lua.read_vehicle_class_integer(class_id, "ReconClass",
            nullptr, -1);
        if (recon_class < 0) {
            ++recon_classes_missing;
        } else {
            unit_recon_record[i] = recon_record_for_class(recon_class);
        }
        // 009623A9 with the 009623AE FLD1 default, squared at 009623CF.
        const float sentinel = -1.0f;
        const float modifier = lua.read_vehicle_class_number(class_id, "ReconModifier",
            sentinel);
        if (modifier == sentinel) {
            ++recon_modifier_absent;
            unit_recon_modifier_sq[i] = bsp::kGunneryReconModifierSqDefault;
        } else {
            unit_recon_modifier_sq[i] = modifier * modifier;
        }
    }
}

namespace {

// Everything 008048A0 and 00806840 read, answered out of the gunnery host's own
// unit state. One method per native read; nothing is defaulted silently.
class GunneryReconSensorPassHost final : public bsp::ReconSensorPassHost {
public:
    explicit GunneryReconSensorPassHost(GameGunneryHost::Impl& owner) : owner_(owner) {}

    std::size_t unit_count() override { return owner_.units.count(); }

    bool unit_present(std::size_t index) override {
        // Rule (b), the same 0043F080 gate bytes the contact sweep uses, plus
        // the world-registry membership the scan walks.
        if (!owner_.units.unit_alive_and_visible(index)) return false;
        if (!owner_.units.unit_active(index)) return false;
        if (index < owner_.unit_state.size() && owner_.unit_state[index].dead) return false;
        return true;
    }

    int unit_side(std::size_t index) override { return owner_.units.unit_side_0054(index); }

    bool unit_is_observer_class(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQueryUnit);
    }
    bool unit_is_unit_base(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQueryUnit);
    }
    bool unit_is_submarine(std::size_t index) override {
        return owner_.units.unit_is_kind_of(index, bsp::kUnitKindQuerySubmarine);
    }
    bool unit_is_surface_target(std::size_t index) override {
        // 00922DC0's thunk into 00922C80, filled from the same facts the ship
        // AI's target_is_surface_00922dc0 fills. The set branch 008DDF90 is not
        // built in this process, so its tail is taken, as there.
        bsp::EntityTargetFacts tf;
        tf.present = true;
        tf.not_engageable = owner_.units.unit_flag_005d(index);
        tf.is_plane = owner_.units.unit_is_kind_of(index, 0x0f);
        tf.is_plane_squadron = owner_.units.unit_is_kind_of(index, 0x18);
        tf.is_ship_family = owner_.units.unit_is_kind_of(index, 0x06);
        tf.is_submarine = owner_.units.unit_is_kind_of(index, 0x08);
        tf.is_airfield = owner_.units.unit_is_kind_of(index, 0x45);
        tf.is_shipyard = owner_.units.unit_is_kind_of(index, 0x46);
        tf.is_command_building = owner_.units.unit_is_kind_of(index, 0x1c);
        tf.is_dummy_target = owner_.units.unit_is_kind_of(index, 0x35);
        tf.is_land_fort = owner_.units.unit_is_kind_of(index, 0x1b);
        float px = 0.0f, py = 0.0f, pz = 0.0f;
        owner_.units.unit_position_00fc(index, px, py, pz);
        tf.world_y = py;
        const bsp::SurfaceTargetAnswer answer = bsp::entity_is_surface_target_00922c80(tf);
        if (answer == bsp::SurfaceTargetAnswer::kUnreadSetBranch) {
            return bsp::entity_surface_target_tail_00922c80(tf, true);
        }
        return answer == bsp::SurfaceTargetAnswer::kYes;
    }

    bsp::SensorCategory unit_sensor_category(std::size_t index) override {
        // unit->[+1E4h]->vtable[1](). The installed getters are constants, so
        // the answer is the unit's kind: 0074E190 air for the plane base,
        // 006DFD20 surface for the ship base and its three land siblings
        // (0074DD50, 006D1D30, 006F57B0), 004F1740 unclassified otherwise.
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitKindQuerySubmarine)) {
            // 00852B90's three states depend on the periscope and on depth
            // bands this process does not read; the periscope half is
            // reconstructed and answers periscope_in for a stowed periscope,
            // which is the state a submarine that nothing has raised is in.
            owner_.record("Recon::submarine_sensor_state_00852b90", 0x00852b90u);
            return bsp::submarine_periscope_sensor_state(false);
        }
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitGunneryKindPlaneBase)) {
            return bsp::kSensorCategoryPlane_0074e190;
        }
        if (owner_.units.unit_is_kind_of(index, bsp::kUnitGunneryKindShipBase) ||
            owner_.units.unit_is_kind_of(index, 0x45) ||
            owner_.units.unit_is_kind_of(index, 0x1b)) {
            return bsp::kSensorCategoryShip_006dfd20;
        }
        return bsp::kSensorCategoryDefault_004f1740;
    }

    void unit_world_xz(std::size_t index, float& x, float& z) override {
        float y = 0.0f;
        owner_.units.unit_position_00fc(index, x, y, z);
    }

    float unit_heading(std::size_t index) override {
        return owner_.units.unit_heading_radians(index);
    }

    float unit_recon_modifier_sq(std::size_t index) override {
        if (index >= owner_.unit_recon_modifier_sq.size()) {
            return bsp::kGunneryReconModifierSqDefault;
        }
        return owner_.unit_recon_modifier_sq[index];
    }

    const bsp::GunneryReconSensorRow* unit_sensor_rows(std::size_t index,
        std::size_t& count) override {
        count = 0;
        if (index >= owner_.unit_recon_record.size()) return nullptr;
        const GameGunneryHost::Impl::ReconClassRecord* record =
            owner_.unit_recon_record[index];
        if (record == nullptr) return nullptr;  // 008048C1's early false
        count = bsp::kSensorListCount;
        return record->rows.data();
    }

    float unit_environment_factor(std::size_t) override {
        // 008E6430(0Ch, observer) is gated on [00E0C978] and [[00F88C30]+118h].
        // This process builds no gameplay-modifier list, which is the same
        // empty-list 1.0f the hit path already takes at 008E6430.
        return bsp::kReconSensorDefaultFactor;
    }

    bool unit_detection_forced(std::size_t) override { return false; }
    bsp::ReconDetectionLevel unit_forced_level(std::size_t) override {
        return bsp::ReconDetectionLevel::none;
    }

    float simplified_recon_multiplier() override {
        // [game+21C4h]+74h, 1.0f from 00444D20 unless a mission script called
        // 008B24A0. Neither USN01 nor USN02 does.
        return 1.0f;
    }
    float simplified_sonar_multiplier() override { return 1.0f; }

    int network_role() override {
        // [00E188A8]+1FE4h. This process runs the single originating session,
        // so the pass is never the kGunneryReconNonOriginatingRole skip.
        return 0;
    }

private:
    GameGunneryHost::Impl& owner_;
};

}  // namespace

void GameGunneryHost::Impl::step_recon_sensor_pass_008073c0(float frame_dt) {
    // 008079B0 BSP_Recon_ServicePeriodicRefresh's countdown at [00F874B8]:
    // subtract the frame delta, return while it is still positive, otherwise
    // reload by adding 3.0 (00D7A2B0), clamped at zero, and run the pass.
    // Stepping 008073C0 every frame instead would publish `none` for every
    // target, because 00805BE0 zeroes each record's value before the pass and
    // one 20 Hz frame of the shipped gain cannot reach the blip threshold.
    recon_refresh_timer -= frame_dt;
    if (recon_refresh_timer > 0.0f) return;
    recon_refresh_timer += bsp::kReconSensorPassRefreshPeriod;
    if (recon_refresh_timer < 0.0f) recon_refresh_timer = 0.0f;
    // 008073C1/008073CC/008073D6: dt is the measured gap since this slot's
    // previous rebuild, [00F876A4] minus slot+2Ch, not the frame delta.
    const float dt = clock_seconds - recon_last_pass_seconds;
    recon_last_pass_seconds = clock_seconds;
    resolve_recon_inputs();
    GunneryReconSensorPassHost host(*this);
    bsp::recon_sensor_pass_step_008073c0(recon_pass, dt, host);
    // The published answer per observing side, tallied over the pairs the pass
    // covered this tick.
    const std::size_t count = units.count();
    for (std::size_t observer = 0; observer < count; ++observer) {
        const int side = units.unit_side_0054(observer);
        if (!recon_pass.side_covered(side)) continue;
        bool seen = false;
        for (const ReconSideCensus& row : recon_side_census) {
            if (row.side == side) { seen = true; break; }
        }
        if (seen) continue;
        ReconSideCensus row;
        row.side = side;
        recon_side_census.push_back(row);
    }
    for (ReconSideCensus& row : recon_side_census) {
        for (std::size_t target = 0; target < count; ++target) {
            if (units.unit_side_0054(target) == row.side) continue;
            if (!host.unit_present(target)) continue;
            switch (recon_pass.level(row.side, target)) {
                case bsp::ReconDetectionLevel::none: ++row.none_level; break;
                case bsp::ReconDetectionLevel::blip: ++row.blip; break;
                case bsp::ReconDetectionLevel::identified: ++row.identified; break;
            }
        }
    }
    publish_recon_triples_008073c0();
    done("Recon::sensor_pass", 0x008073c0u);
    done("Recon::evaluate_sensors", 0x008048a0u);
}

void GameGunneryHost::Impl::publish_recon_triples_008073c0() {
    // 008073C0 steps 5..13 for every slot this process observes. The buckets
    // are walked in class-id order (the 61h-bucket loops at 00807529,
    // 00807615, 008076F9); inside a bucket the records follow this pass's scan
    // order, because 008065B0 appends new records and splices carried ones to
    // the tail. The scan (0080749C..00807527) visits only the 22 ids of
    // 00806480, each through the world registry's per-class list, behind the
    // four gate bytes and IsKindOf(2).
    std::array<int, 22> ids = bsp::kReconScannedClassIds;
    std::sort(ids.begin(), ids.end());
    std::vector<int> sides;
    for (const ReconSideCensus& row : recon_side_census) sides.push_back(row.side);
    const std::size_t count = units.count();
    for (std::size_t u = 0; u < count; ++u) {
        const int s = units.unit_side_0054(u);
        if (std::find(sides.begin(), sides.end(), s) == sides.end()) sides.push_back(s);
    }
    GunneryReconSensorPassHost host(*this);
    recon_triples.clear();
    for (const int side : sides) {
        ReconSlotTriples t;
        t.side = side;
        std::vector<std::size_t> unknown_enemy, unknown_neutral;
        for (const int id : ids) {
            const std::size_t n = units.world_list_size(id);
            for (std::size_t pos = 0; pos < n; ++pos) {
                const std::size_t u = units.world_list_entry(id, pos);
                if (u >= count) continue;
                if (!host.unit_present(u)) continue;                        // 008074D5..008074EE
                if (!units.unit_is_kind_of(u, bsp::kReconScanRequiredClassId)) continue;  // 008074F2
                const bsp::ReconRelation rel = bsp::recon_relation_for_008065ff(side,
                    units.unit_side_0054(u));
                if (rel == bsp::ReconRelation::own) {                       // step 6
                    t.lists[0].push_back(u);
                    continue;
                }
                // Steps 9 and 10: the drain keeps level 2 in its relation's
                // triple, copies level 1 into `unknown`, drops level 0.
                const bsp::ReconDetectionLevel level = recon_pass.level(side, u);
                const bool enemy = rel == bsp::ReconRelation::enemy;
                if (level == bsp::ReconDetectionLevel::identified) {
                    t.lists[enemy ? 1 : 2].push_back(u);
                } else if (level == bsp::ReconDetectionLevel::blip) {
                    (enemy ? unknown_enemy : unknown_neutral).push_back(u);
                }
            }
        }
        t.lists[3] = unknown_enemy;                                         // 00807644
        t.lists[3].insert(t.lists[3].end(), unknown_neutral.begin(), unknown_neutral.end());
        for (int k = 0; k < 4; ++k) {                                       // step 13
            t.lists[4].insert(t.lists[4].end(), t.lists[k].begin(), t.lists[k].end());
        }
        for (int k = 0; k < 5; ++k) recon_triple_sum[k] += t.lists[k].size();
        recon_triples.push_back(std::move(t));
    }
    ++recon_triple_builds;
    done("Recon::publish_triples_008073c0", 0x00807529u);
}

bool GameGunneryHost::recon_triple_units(int side, int triple,
    std::vector<std::size_t>& out) const {
    out.clear();
    if (triple < 0 || triple > 4) return false;
    const Impl::ReconSlotTriples* t = impl_->recon_triples_for(side);
    if (t == nullptr) return false;
    out = t->lists[static_cast<std::size_t>(triple)];
    return true;
}

void GameGunneryHost::Impl::run_gunnery_pass(std::size_t index, float dt) {
    UnitState& state = unit_state[index];
    if (!state.attached) return;

    // The director's two targets, as this process holds them: 00863640 reads
    // director+238h, which the automatic target think 009F5DA0 wrote through
    // 00835860, and 0071EBF0 takes the newest queued command's target.
    state.fire_target = 0;
    state.command_target = 0;
    // 0071EBF0 answers with the newest QUEUED COMMAND's target. Until now this
    // host had no queued commands to answer with, so the field stayed 0 and
    // step 8.7's first arm was always skipped. The script-order path now issues
    // real commands that reach a weapon director, so the answer exists: take the
    // unit's current command row and resolve its target by name, exactly as the
    // fire-target arm below resolves its own.
    refresh_command_targets();
    if (index < command_target_by_unit.size()) {
        state.command_target = command_target_by_unit[index];
    }
    if (ship_ai != nullptr) {
        const std::vector<GameShipAiRow>& rows = ship_ai->rows();
        if (index < rows.size()) {
            const GameShipAiRow& row = rows[index];
            for (std::size_t i = 0; i < units.count(); ++i) {
                const GameUnitRow* candidate = units.unit_row(i);
                if (candidate == nullptr) continue;
                if (!row.fire_target.empty() && candidate->name == row.fire_target) {
                    state.fire_target = i + 1;
                }
                // NOT row.brain_target_name. That field is brain+0B20h, the
                // navigation goal vector's target, which sits beside the goal
                // position brain+0B2Ch..0B34h and names whatever the ship is
                // steering toward - frequently the ship itself. 0071EBF0 answers
                // with the newest QUEUED COMMAND's target, an order's target,
                // which is a different thing entirely.
                //
                // Using the goal target here made every gun engage its own hull:
                // step 8.7 appends the target itself (00432480), and neither the
                // native 00863990 nor this host applies a party or self test -
                // verified in the listing at 00863990..00863A73, which gates only
                // on the category mask 008633D0, an owner vtable[5Ch](5) test, the
                // per-category range at owner+category*4+430h and a plane penalty.
                // The native never meets the case because a queued command's
                // target is an enemy. See docs/GAME_EXECUTABLE.md.
                //
                // No command-target producer is wired in this process, and this
                // run queues no orders, so the faithful answer is "no command
                // target" - which is what the native would return here.
            }
        }
    }
    state.row.fire_target = state.fire_target != 0
        ? unit_state[state.fire_target - 1].row.name : std::string();
    state.row.command_target = state.command_target != 0
        ? unit_state[state.command_target - 1].row.name : std::string();
    done("Gunnery::director_fire_target_00863640", 0x00863640u);
    done("Gunnery::director_newest_command_target_0071ebf0", 0x0071ebf0u);

    // Packet cc8_torpedo_gun_assignment. The torpedo category is cut out of the
    // recon sweep by the image itself at 008651F5 (`CMP ESI,7 / JE 00865442`),
    // so on a unit that carries a torpedo-category gun the only way a candidate
    // can appear is step 8.7's two director targets. Counting how often either
    // exists is the first question, before any guard inside 00863990 matters.
    if (!state.category_guns[
            static_cast<std::size_t>(bsp::kUnitGunneryTorpedoCategory)].empty()) {
        ++summary.torpedo_cat_pass_ticks;
        if (state.command_target != 0) ++summary.torpedo_cat_with_command_target;
        if (state.fire_target != 0) ++summary.torpedo_cat_with_fire_target;
    }

    aa_cand_lines.clear();
    aa_changed = false;
    GunneryPassBinding binding(*this, index);
    const float before = state.throttle;
    bool enabled = state.enabled;
    bsp::unit_gunnery_pass_tick_00864fe0(binding, dt, state.throttle, enabled);
    if (aa_changed) {
        log.notef("  aa trace t=%.2f %s think candidates (%zu scored):",
            static_cast<double>(clock_seconds), state.row.name.c_str(), aa_cand_lines.size());
        std::size_t shown = 0;
        for (const std::string& line : aa_cand_lines) {
            if (++shown > 80) break;
            log.notef("%s", line.c_str());
        }
    }
    state.enabled = enabled;
    state.row.pass_enabled = enabled;
    ++state.row.ticks;
    ++summary.pass_ticks;
    if (state.throttle < before) {
        ++state.row.think_bodies;
        ++summary.pass_bodies;
        state.row.candidates += binding.candidates_seen();
        summary.candidates += binding.candidates_seen();
        summary.candidates_rejected += binding.candidates_rejected();
    }
    done("Gunnery::pass_tick_00864fe0", 0x00864fe0u);
}

// ---------------------------------------------------------------------------
// The aim ticks, the trigger latch and the 0ADh fire message
// ---------------------------------------------------------------------------

namespace {

class FireRequestBinding final : public bsp::GunFireRequestHost {
public:
    FireRequestBinding(GameGunneryHost::Impl& owner, std::size_t gun_slot)
        : owner_(owner), gun_(gun_slot) {}

    bool has_owning_unit() const override { return true; }          // gun+3F0h
    bool unit_suppressed() const override {                         // unit+5Dh
        const std::size_t unit = owner_.guns[gun_].unit_index;
        return unit < owner_.unit_state.size() && owner_.unit_state[unit].dead;
    }
    bool gun_suppressed() const override { return false; }          // gun+5Dh
    void release_fire_target_ref_006952a0() override {
        owner_.record("Gun::release_fire_target_ref_006952a0", 0x006952a0u);
    }
    bool is_rapid_fixed_slave_006e3d50(int) override { return false; }
    void send_fixed_slave_fire_message_0077c7b0(bool) override {
        owner_.record("Gun::fixed_slave_fire_message_0077c7b0", 0x0077c7b0u);
    }
    float random_stagger_00bd2f10(float lo, float hi) override {
        owner_.done("Gun::fire_stagger_00bd2f10", 0x00bd2f10u);
        return owner_.draw(GameGunneryHost::Impl::Draw::fire_stagger, gun_, 0, lo, hi);
    }
    void stop_firing_0072b4c0() override {
        owner_.record("Gun::stop_firing_0072b4c0", 0x0072b4c0u);
    }
    void release_effect_ref() override {}
    // 0072D18C, the first thing the gun does each step: age the list at
    // gun+120h and drop the records whose countdown has gone strictly negative.
    // docs/GUN_BASE_TICK.md. The list has no producer in this reconstruction,
    // so the sweep is a no-op today; the counter says so out loud rather than
    // letting an always-zero look like a working path.
    void base_tick_0072ad40(float dt) override {
        GameGunRow& row = owner_.guns[gun_];
        owner_.summary.gun_pending_timers_expired +=
            bsp::gun_age_pending_timers_0072ad40(row.pending_timers, dt);
        owner_.summary.gun_pending_timers_live += row.pending_timers.size();
        owner_.done("Gun::base_tick_0072ad40", 0x0072ad40u);
    }
    void set_barrel_reload_timer_0072cf00(int index, float value) override {
        GameGunRow& row = owner_.guns[gun_];
        if (index < 0 || static_cast<std::size_t>(index) >= row.fire.barrel_timers.size()) {
            return;
        }
        const bsp::BarrelReloadWrite write = bsp::barrel_reload_write_0072cf00(value,
            1.0e9f, 1.0f, false);
        row.fire.barrel_timers[static_cast<std::size_t>(index)] = write.timer;
        owner_.done("Gun::set_barrel_reload_timer_0072cf00", 0x0072cf00u);
    }
    bool unit_fire_blocked() const override { return false; }   // unit+720h
    bool gun_disabled() const override { return false; }        // gun+3B8h
    void send_fire_message_0ad(std::uint16_t) override {
        ++owner_.guns[gun_].fire_messages;
        ++owner_.summary.fire_messages;
        sent_ = true;
        owner_.done("Gun::fire_message_0ad_0072d290", 0x0072d290u);
    }

    bool sent() const noexcept { return sent_; }

private:
    GameGunneryHost::Impl& owner_;
    std::size_t gun_;
    bool sent_{false};
};

}  // namespace

void GameGunneryHost::Impl::run_gun_aim_and_fire(float dt) {
    for (std::size_t g = 0; g < guns.size(); ++g) {
        GameGunRow& gun = guns[g];
        const std::size_t owner_unit = gun.unit_index;
        if (owner_unit >= unit_state.size()) continue;
        UnitState& state = unit_state[owner_unit];
        if (state.dead) continue;

        // Which of the five aim bots this gun's weapon sub-type selects.
        const bsp::GunBotSlotAssignment slots
            = bsp::gun_bot_slots_for_subtype_0072c6a0(gun.category, true, false);
        (void)slots;

        float want_horz = gun.rest_horz;
        float want_vert = gun.rest_vert;
        bool have_target = false;
        std::size_t target = 0;
        if (gun.target_unit != 0 && gun.target_unit - 1 < unit_state.size()) {
            target = gun.target_unit - 1;
            if (unit_state[target].dead || !units.unit_alive_and_visible(target)) {
                // 008FFA20's target-validity test: a dead target is dropped.
                gun.target_unit = 0;
                gun.target_name.clear();
                done("GunBot::target_still_valid_008ffa20", 0x008ffa20u);
            } else {
                have_target = true;
            }
        }
        // Packet cc9_player_gun_seat: the side gate 008FFA99 (and 00902999,
        // 00903136, 0090003B) runs a bot only while [gun+1ACh] is 8 or an AI
        // slot. A gun the player's seat holds gets no bot target, angle or
        // trigger; the angles and trigger come from message 79h instead.
        const bool player_seat = kPlayerGunSeatBound && !slot_ai_held_00927f10(gun.seat_1ac);
        if (player_seat) {
            have_target = false;
            ++seat_held_ticks;
        }

        float right[3], up[3], forward[3], origin[3];
        unit_pose(owner_unit, right, up, forward, origin);
        // PLACEHOLDER with no native counterpart, and it is the reason the recovered
        // ballistic arc 00955630 currently buys nothing. docs/GUN_MOUNT_POSITIONS.md:
        // the native muzzle origin is
        //   TransformAffinePoint(class->muzzleOffsets[gun+44Ch], gun[+3CCh]->worldMatrix)
        // at 007307A0/007307D3, where gun+3CCh is the model node named "barrel" or
        // "base" and the offsets come from the model's "fire" node group. `Height`
        // (class+0A8h) is the HULL height - ship-motion draft and a hit-slab half
        // extent - and nothing on the firing path reads it. Raising one shared origin
        // by it gives a whole battery the same muzzle point and makes
        // h = aim.y - muzzle.y near zero, which is exactly the case in which the arc
        // degenerates to the asin(g*R/v^2)/2 pre-estimate it replaced
        // (tools/gun_arc_pre_estimate_compare.py). Real per-gun origins need the model
        // node transforms, which this process does not yet build.
        float muzzle[3];
        gun_muzzle_point(gun, state, right, up, forward, origin, muzzle);

        bool arc_solved = true;
        // Packet cc9_torpedo_launch_gate: 008FFF20's lead point (x, z) and
        // |bot+60h - raw heading| for the friendly-crossing gate below.
        std::array<float, 2> torpedo_lead_xz{{0.0f, 0.0f}};
        float torpedo_snap_radians = 0.0f;
        if (have_target) {
            float theirs[3];
            if (kArtilleryAimPointBound && artillery_bot_aims(gun.category, target)
                && units.unit_is_kind_of(target, bsp::kUnitGunneryKindShipBase)) {
                artillery_aim_point(g, target, dt, theirs, owner_unit);
            } else {
                unit_aim_point(target, theirs);
            }
            float velocity[3];
            unit_velocity(target, velocity);
            const std::array<float, 3> shooter{muzzle[0], muzzle[1], muzzle[2]};
            const std::array<float, 3> at{theirs[0], theirs[1], theirs[2]};
            const std::array<float, 3> v{velocity[0], velocity[1], velocity[2]};
            std::array<float, 3> lead = at;
            float pitch = 0.0f;

            if (gun.category == bsp::kUnitGunneryTorpedoCategory) {
                // The torpedo bot runs its own intercept solver and its round
                // does not fall, so no gravity term is added.
                // 0090022B loads [[gun+3F8h]+34h]+0E4h, WaterTravelSpeed, for
                // the torpedo bot's solver, while the AA flak bot's call at
                // 009031CF takes +50h (V0) - the choice is deliberate and the
                // host had been passing V0. For the Mark 15 that is 13 against
                // 51.444, and at 13 m/s the quadratic's leading coefficient
                // turns positive and no aspect has a solution against an 18.78
                // m/s destroyer, so the solver answered nothing on every launch.
                // docs/TORPEDO_LAUNCH_ACCURACY.md.
                const float solver_speed = gun.water_travel_speed > 0.0f
                    ? gun.water_travel_speed : gun.muzzle_speed;
                if (solver_speed > 0.0f
                    && bsp::torpedo_intercept_point_008fbb00(shooter, at,
                        solver_speed, v, lead)) {
                    done("GunBot::intercept_point_008fbb00", 0x008fbb00u);
                } else {
                    lead = at;
                }
            } else if (kGunGravityArcBound && gun.muzzle_speed > 0.0f
                       && (aa_bot_aims(gun.category, target)
                           || dp_air_ammo(g, gun.category, target) != nullptr)) {
                const SecondAmmo* dp_round = dp_air_ammo(g, gun.category, target);
                const float aa_v0 = dp_round != nullptr ? dp_round->muzzle_speed
                                                        : gun.muzzle_speed;
                // 00902920 / 009030C0: 00901C20's lead point, no gravity term, no
                // arc. SUBSTITUTION for 00901C20's closed-form intercept: the lead
                // is the target's velocity times distance / V0, which is what the
                // host's lead below reduces to at zero pitch.
                // 00901C55..00901C74 reads the target's pose position +FCh..+104h,
                // not the Height-raised point 00864D90 tests visibility from, which
                // is what `at` holds. Aiming a straight round at that raised point
                // puts it Height/2 over the target's hit box.
                float tr[3], tu[3], tf[3], origin_of_target[3];
                unit_pose(target, tr, tu, tf, origin_of_target);
                const float span[3] = {origin_of_target[0] - muzzle[0],
                    origin_of_target[1] - muzzle[1], origin_of_target[2] - muzzle[2]};
                const float flight = length3(span) / aa_v0;
                // 00901C20 leads with the target's own velocity. unit_velocity reads
                // the row's forward_speed, which is zero for a plane (the same
                // reason release_ordnance_drop reads 0092D730), so a plane target
                // is led from its body axis and 0092D730's speed.
                float target_velocity[3] = {v[0], v[1], v[2]};
                if (units.unit_is_kind_of(target, bsp::kUnitGunneryKindPlaneBase)) {
                    // Packet cc9_aa_lethality_audit: vtable[34h] for a plane is
                    // 007BBB70, a copy of unit+AC8h, the world linear velocity.
                    float world_v[3];
                    if (kAaTargetWorldVelocityBound
                        && units.unit_linear_velocity(target, world_v)) {
                        for (int i = 0; i < 3; ++i) target_velocity[i] = world_v[i];
                    } else {
                        const float speed = units.unit_forward_speed_0092d730(target);
                        for (int i = 0; i < 3; ++i) target_velocity[i] = tf[i] * speed;
                    }
                }
                if (kGunInterceptBound && aa_v0 >= 2.0f) {
                    // 00901C20: relative motion (the shooter's own velocity is
                    // subtracted; a ship target, IsKindOf(6), has its vertical
                    // zeroed), the closed-form time, the two settings biases,
                    // then aim = target + t * V. SUBSTITUTION: the 0085E4D0
                    // turn-rate averaging for a turning plane needs the angular
                    // velocity at +AF8h, which this host does not carry.
                    float shooter_velocity[3];
                    unit_velocity(owner_unit, shooter_velocity);
                    float rel_v[3];
                    for (int i = 0; i < 3; ++i) {
                        rel_v[i] = target_velocity[i]
                            - (aa_keep_shooter_velocity() ? 0.0f : shooter_velocity[i]);
                    }
                    if (units.unit_is_kind_of(target, bsp::kUnitGunneryKindShipBase)) {
                        rel_v[1] = 0.0f;
                    }
                    const float dist = length3(span);
                    float t = intercept_time_00901c20(span, rel_v, aa_v0, dist);
                    // BSP_AA_TIME_BIAS_OFF=1 is a MEASUREMENT option (never in a
                    // reference run) that drops the two settings biases so a pair
                    // can separate them from the quadratic.
                    if (!aa_time_bias_off()) {
                        t = aa_time_add_fix + t + (dist / 1000.0f) * aa_time_add_mul;
                    }
                    for (int i = 0; i < 3; ++i) lead[i] = origin_of_target[i] + rel_v[i] * t;
                    ++intercept_solves;
                    done("GunBot::intercept_solution_00901c20", 0x00901c20u);
                } else {
                    for (int i = 0; i < 3; ++i) {
                        lead[i] = origin_of_target[i] + target_velocity[i] * flight;
                    }
                }
                pitch = 0.0f;
                ++aa_direct_aims;
                record("GunBot::intercept_solution_00901c20", 0x00901c20u);
            } else if (gun.muzzle_speed > 0.0f) {
                ++artillery_arc_aims;
                // 006DF520 steps 5, 6 and 7, the muzzle bot's own gravity
                // pre-estimate (docs/GUN_BOT_TICKS.md section 6.3):
                //   s     = distance * 9.81 / v^2              006DF8BF
                //   pitch = min(asin(s) * 0.5, pi/4)           00CEB5A8 clamps
                //   the aim point is pushed out along the target velocity by
                //   distance / (v * cos(pitch) * [muzzle+5Ch])
                // and step 7 refuses the shot when s > 1. The refinement
                // 00955630 makes on top of that is a contract here, so the
                // pre-estimate is the whole of the elevation this run uses.
                const float span[3] = {at[0] - muzzle[0], at[1] - muzzle[1],
                    at[2] - muzzle[2]};
                const float distance = length3(span);
                const float speed_squared = gun.muzzle_speed * gun.muzzle_speed;
                const float s = distance * kGravity / speed_squared;
                arc_solved = s <= 1.0f;
                pitch = std::min(std::asin(std::min(s, 1.0f)) * 0.5f, kQuarterPi);
                // [muzzle+5Ch] has no recovered producer; the divisor is taken
                // as one, which makes the push-out the plain time of flight.
                const float cosine = std::cos(pitch);
                const float flight = cosine > 0.0f ? distance / (gun.muzzle_speed * cosine)
                                                   : 0.0f;
                for (int i = 0; i < 3; ++i) lead[i] = at[i] + v[i] * flight;
                // Step 8, 006DFAD4: the recovered solve 00955630 replaces the
                // pre-estimate for the FINAL elevation. The pre-estimate above
                // still sets the time-of-flight push-out, which is what step 6
                // uses it for. docs/GUN_GRAVITY_ARC.md:
                //   k = g*R^2 / 2v^2,  D = R^2 - 4k(k + h),
                //   tan(pitch) = (R - sqrt(D)) / 2k          the low flat root
                // R is the HORIZONTAL distance and h the height difference, not
                // the slant range the pre-estimate used.
                bsp::GunGravityArcQuery arc_query;
                arc_query.aim_point = {lead[0], lead[1], lead[2]};
                arc_query.muzzle_position = {muzzle[0], muzzle[1], muzzle[2]};
                arc_query.muzzle_speed = gun.muzzle_speed;
                // mount_frame == nullptr is 0085B8DE's deliberate world-frame
                // path: the pair comes back without the local-frame round trip.
                arc_query.mount_frame = nullptr;
                const bsp::GunGravityArcSolution arc =
                    bsp::solve_gun_gravity_arc_00955630(arc_query);
                arc_solved = arc_solved && arc.solved;
                // angles.vert is *outPitch, the elevation above horizontal, and
                // the 009557F8 negate applies to *outYaw only. want_vert below
                // is asin(direct line) + pitch, so what belongs in `pitch` is
                // the superelevation above the direct line, not the total.
                const float led[3] = {lead[0] - muzzle[0], lead[1] - muzzle[1],
                    lead[2] - muzzle[2]};
                const float led_horizontal =
                    std::sqrt(led[0] * led[0] + led[2] * led[2]);
                const float direct_line = std::atan2(led[1], led_horizontal);
                pitch = arc.angles.vert - direct_line;
                // 00955630 reconstructed (src/gun_gravity_arc.cpp), taken on the
                // mount == NULL path 0085B8DE uses: the host builds no gun node
                // frame, so the local-frame round trip is skipped. Labelled.
                done("GunBot::ballistic_arc_00955630", 0x00955630u);
                done("GunBot::gravity_pre_estimate_006df8bf", 0x006df8bfu);
            }

            const float delta[3] = {lead[0] - muzzle[0], lead[1] - muzzle[1],
                lead[2] - muzzle[2]};
            const float distance = length3(delta);
            if (distance > 0.0f) {
                const float unit_delta[3] = {delta[0] / distance, delta[1] / distance,
                    delta[2] / distance};
                // 008FDAF0: the world direction as a hull-relative angle pair.
                want_horz = kGunHorzSign * std::atan2(dot3(unit_delta, right),
                    dot3(unit_delta, forward));   // 008FDAF0 negates when bound
                const float vertical = std::max(-1.0f,
                    std::min(1.0f, dot3(unit_delta, up)));
                want_vert = std::asin(vertical) + pitch;
                if (gun.category == bsp::kUnitGunneryTorpedoCategory) {
                    // 008FFF20 step 8 at 00900380 does NOT refuse a heading that
                    // falls outside a firing window: 0085AB50 -> 007F6190 snaps
                    // it up to pi/4 onto the nearest window edge and fires along
                    // that edge, abandoning the shot only when no window's
                    // horizontal bounds hold the heading at all or the snap
                    // exceeds the limit. The host had been handing the raw
                    // heading to gun_set_target_angles_0085aba0, which refuses
                    // outright, so a torpedo mount almost never opened a launch
                    // window. docs/TORPEDO_LAUNCH_ACCURACY.md.
                    const bsp::GunPlatformArcs snap_arcs{gun.arcs.data(),
                        gun.arcs.size()};
                    const float snapped = bsp::gun_snap_heading_to_fire_window_007f6190(
                        snap_arcs, want_horz, kQuarterPi);
                    if (bsp::gun_heading_snap_failed(snapped)) {
                        have_target = false;          // 00900392..009003A2
                    } else {
                        // 0090043D..0090044A: 00438B10(bot+60h, raw heading).
                        torpedo_snap_radians = std::fabs(
                            bsp::wrapped_angle_subtract_00438b10(snapped, want_horz));
                        torpedo_lead_xz = {{lead[0], lead[2]}};
                        want_horz = snapped;
                        ++summary.torpedo_heading_snaps;
                    }
                    done("GunBot::snap_heading_to_fire_window_007f6190", 0x007f6190u);
                    // 009003DD commands a hard 0.0f vertical for the torpedo bot.
                    // Every torpedo platform in this installation's
                    // vehicleclasses.lua authors its one window with
                    // MinVertAngle == MaxVertAngle == 0, so any computed
                    // depression is refused outright by
                    // gun_set_target_angles_0085aba0 and the mount never opens a
                    // launch window. docs/TORPEDO_LAUNCH_ACCURACY.md.
                    want_vert = 0.0f;
                }

                done("GunBot::angles_from_world_direction_008fdaf0", 0x008fdaf0u);
                if (kAaGunnerErrorBound && gun.category == 1 && want_vert < 0.0f) {
                    want_vert *= 0.5f;    // 00902F76 FMUL [00D7A280]
                    ++aa_negative_halvings;
                }
                if (kGunAimErrorBound && !aa_bot_aims(gun.category, target)
                    && dp_air_ammo(g, gun.category, target) == nullptr
                    && artillery_bot_aims(gun.category, target)) {
                    const bsp::GunAimAngles error = aim_error_tick(g, owner_unit, dt);
                    bsp::GunAimAngles solved;
                    solved.horz = want_horz;
                    solved.vert = want_vert;
                    const bsp::GunAimAngles aimed =
                        bsp::gun_bot_aim_error_apply_006dfb0b(solved, error);
                    want_horz = aimed.horz;
                    want_vert = aimed.vert;
                }
            }
            if (!arc_solved) {
                ++gun.arc_unsolved;
                ++summary.arc_unsolved;
                have_target = false;   // 006DFA60's gate: no shot is armed
            }
        }

        if (player_seat) {                      // 00959E01's 0085ABA0 request
            want_horz = gun.seat_horz;
            want_vert = gun.seat_vert;
        }
        const bsp::GunPlatformArcs arcs{gun.arcs.data(), gun.arcs.size()};
        float accepted_mark = 0.0f;
        const bool accepted = bsp::gun_set_target_angles_0085aba0(gun.angles, arcs,
            gun.speeds, want_horz, want_vert, accepted_mark);
        if (accepted) {
            ++gun.angle_sets;
            ++summary.angle_sets;
        } else {
            ++gun.angle_refusals;
            ++summary.angle_refusals;
            // angle_sets + angle_refusals is exactly guns * mission_ticks - every
            // gun, every tick, with no target gate - so the refusal count carries
            // no information about targets and must never be read as one. Verified
            // on IJN01: 230442 + 72558 = 303000 = 606 * 500 to the digit.
            // docs/AA_VERTICAL_WINDOW.md. These split out the ticks where the gun
            // actually held a target.
            if (have_target) ++summary.angle_refusals_targeted;
        }
        done("GunBot::set_target_angles_0085aba0", 0x0085aba0u);
        if (have_target && !aa_trace_target_prefix().empty()
            && unit_state[target].row.name.compare(0, aa_trace_target_prefix().size(),
                   aa_trace_target_prefix()) == 0) {
            AaTargetStats& st = aa_target_stats[target];
            ++st.targeted_ticks;
            const float vert_deg = want_vert * 57.2957795f;
            if (accepted) {
                st.accepted_vert_max = std::max(st.accepted_vert_max, vert_deg);
            } else {
                ++st.angle_refusals;
                st.refused_vert_max = std::max(st.refused_vert_max, vert_deg);
            }
        }

        const bsp::GunArcRouteOutcome route = bsp::gun_arc_route_deltas_007f6530(arcs,
            gun.angles.horz, gun.angles.vert, gun.angles.target_horz,
            gun.angles.target_vert);
        done("Gun::arc_route_deltas_007f6530", 0x007f6530u);
        if (bsp::gun_step_aim_0085ad80(gun.angles, gun.speeds, route.deltas, dt, false)) {
            ++gun.aim_steps;
            ++summary.aim_steps;
        }
        done("Gun::step_aim_0085ad80", 0x0085ad80u);

        // 006DF520 step 12 arms the trigger through 006DEE40 against
        // *00CF9054 = 0.1 degree, not the stepper's 0.01-degree dead band that
        // gun_aim_settled_0085ae4a carries. Using the latter as a fire gate made
        // the host ten times stricter per axis than the native.
        // docs/GUN_SHOT_CADENCE.md divergence 2. This is a faithfulness fix and
        // is NOT expected to raise the shot count materially: the packet measures
        // it at 13% of targeted refusals, and the count is held down by the
        // authored 17.5 s reload, not by the settle test.
        // Only the CONSTANT differs from gun_aim_settled_0085ae4a: the wrapped
        // difference and the strict comparison are kept, because a plain
        // subtraction changes the semantics across the +/-pi wrap. An earlier
        // revision of this line dropped the wrap and was wrong for that reason.
        const bool settled =
            std::fabs(bsp::wrapped_angle_subtract_00438b10(
                gun.angles.target_horz, gun.angles.horz)) < bsp::kGunFireSettleBand &&
            std::fabs(bsp::wrapped_angle_subtract_00438b10(
                gun.angles.target_vert, gun.angles.vert)) < bsp::kGunFireSettleBand;
        const bool may_fire_here = bsp::gun_fire_allowed_007f60a0(arcs, gun.angles.horz,
            gun.angles.vert);
        done("Gun::fire_window_007f60a0", 0x007f60a0u);
        if (have_target && settled && !may_fire_here) {
            ++gun.arc_blocks;
            ++summary.arc_blocks;
        }
        // unit+634h, the scripted per-group fire inhibit. No mission-script
        // action in this mission writes it, so every bit is clear.
        const bool inhibited = false;
        // Which conjunct of want_fire fails on a tick that had a target. A
        // refusal here is not automatically a defect: a beam mount cannot train
        // astern, so check the commanded bearing against the platform's windows
        // before reading a non-zero want_fire_no_accept as one.
        // DEFINITIONAL, not independent evidence: `accepted` is the return of the
        // same call that increments angle_refusals, so on a targeted tick this is
        // the same event as angle_refusals_targeted and the two always match. Kept
        // only so the decomposition below reads completely; it is the counter to
        // drop first. docs/AA_VERTICAL_WINDOW.md.
        // Packet cc8_torpedo_release_spawn. The torpedo gate census: the same
        // conjuncts as `want_fire` below, restricted to guns whose round can
        // swim. Read as a funnel - the first counter that collapses names the
        // gate that keeps torpedoes out of the water.
        const bool torpedo_gun = gun.swim_speed > 0.0f;
        if (torpedo_gun) {
            ++summary.torpedo_gun_ticks;
            if (have_target) {
                ++summary.torpedo_gun_targeted;
                if (accepted) {
                    ++summary.torpedo_gun_accepted;
                    if (settled) {
                        ++summary.torpedo_gun_settled;
                        if (may_fire_here) ++summary.torpedo_gun_window;
                    }
                }
            }
        }
        if (have_target && !accepted) ++summary.want_fire_no_accept;
        if (have_target && accepted && !settled) ++summary.want_fire_no_settle;
        if (have_target && accepted && settled && !may_fire_here) {
            ++summary.want_fire_no_window;
        }
        bool want_fire = have_target && accepted && settled && may_fire_here
            && !inhibited;
        if (kTorpedoFriendlyCrossingBound && want_fire
            && gun.category == bsp::kUnitGunneryTorpedoCategory) {
            // 008FFF20 0090058A..009007F6, after the settle test and vtable[1D0h].
            ++summary.torpedo_friendly_scans;
            if (torpedo_friendly_hold_008fff20(gun, owner_unit, muzzle, right, forward,
                    torpedo_lead_xz, torpedo_snap_radians)) {
                want_fire = false;                 // 0090096D -> vtable[1E8h](0)
                ++summary.torpedo_friendly_holds;
            }
            done("TorpedoBot::friendly_crossing_scan_008fff20", 0x0090058au);
        }
        const bool plane_gun = gun.category == 0
            && units.unit_is_kind_of(owner_unit, bsp::kUnitGunneryKindPlaneBase);
        if constexpr (kPlaneGunfireHooked) {
            if (plane_gun) {
                // 007CE9F4: gun->vtable[1E8h](gunFire) - the trigger is the pilot's,
                // not a bot's; no target, window or settle gate stands in front of it.
                want_fire = units.plane_gun_trigger_bc9(owner_unit);
                if (want_fire) ++plane_trigger_ticks;
            }
        }
        if (player_seat) {                      // 00959DEB / 00959F50 / 00959F68
            want_fire = gun.seat_trigger;
            if (want_fire) ++seat_trigger_ticks;
        }

        FireRequestBinding fire_host(*this, g);
        const bool before = gun.fire.fire_requested;
        const bool latched = bsp::gun_set_fire_request_0072d2c0(gun.fire, fire_host,
            want_fire);
        done("Gun::set_fire_request_0072d2c0", 0x0072d2c0u);
        if (latched && !before) {
            ++gun.trigger_rises;
            ++summary.trigger_rises;
        }

        const bool sent = bsp::gun_fixed_step_tick_0072d130(gun.fire, fire_host, dt);
        done("Gun::fixed_step_tick_0072d130", 0x0072d130u);
        if (torpedo_gun && sent) ++summary.torpedo_gun_sent;
        if (!sent) continue;

        // 0072D860, the 0ADh arm: gun->vtable[1DCh] FireIfReady 00727E30, which
        // asks CanFire and fires on a yes.
        ++summary.fire_if_ready;
        bsp::GunFireGateInputs gate;
        gate.fire_params_armed = true;
        gate.disabled = false;
        gate.damage_counter = 0;
        gate.barrel_delay_time = gun.fire.barrel_delay_time;
        gate.secondary_delay = gun.fire.fire_stagger;
        gate.unit_cooldown_applies = false;
        gate.weapon_type_id = gun.category;
        gate.muzzle_world_y = muzzle[1];
        gate.muzzle_submerged = false;
        gate.muzzle_blocked = false;
        gate.barrel_count = gun.barrel_num;
        gate.reload_timers = gun.fire.barrel_timers.data();
        const bool can_fire = bsp::gun_can_fire_turning_0085a830(inhibited, arcs,
            gun.angles, gate, true);
        done("Gun::can_fire_0085a830", 0x0085a830u);
        if (!can_fire) {
            ++gun.can_fire_refusals;
            ++summary.can_fire_refusals;
            continue;
        }

        // 00730160 BSP_Gun_Fire: the next ready barrel, one projectile, then
        // the two timers.
        const int barrel = bsp::next_ready_barrel_007298d0(gun.fire.barrel_timers.data(),
            gun.barrel_num, gun.next_fire_barrel);
        gun.next_fire_barrel = (barrel + 1) % std::max(1, gun.barrel_num);
        if (barrel >= 0 && static_cast<std::size_t>(barrel) < gun.fire.barrel_timers.size()) {
            gun.fire.barrel_timers[static_cast<std::size_t>(barrel)] = gun.reload_time;
        }
        gun.fire.barrel_delay_time = gun.barrel_delay_time;
        if (plane_gun) ++plane_gun_rounds;
        if (aa_trace_matches(state.row.name)) {
            float aim[3] = {0.0f, 0.0f, 0.0f};
            float mz[3];
            if (have_target) unit_aim_point(target, aim);
            unit_aim_point(owner_unit, mz);
            const float sp[3] = {aim[0] - mz[0], aim[1] - mz[1], aim[2] - mz[2]};
            log.notef("  aa shot t=%.2f %s gun=%zu cat=%d class=%d v0=%.0f barrels=%d "
                "-> %s range=%.0f vert=%.1f",
                static_cast<double>(clock_seconds), state.row.name.c_str(), g, gun.category,
                (have_target && dp_air_ammo(g, gun.category, target) != nullptr)
                    ? dp_air_ammo(g, gun.category, target)->bullet_class : gun.bullet_class,
                static_cast<double>((have_target && dp_air_ammo(g, gun.category, target) != nullptr)
                    ? dp_air_ammo(g, gun.category, target)->muzzle_speed : gun.muzzle_speed),
                gun.barrel_num,
                have_target ? unit_state[target].row.name.c_str() : "none",
                have_target ? static_cast<double>(length3(sp)) : -1.0,
                static_cast<double>(gun.angles.vert * 57.2957795f));
        }
        if (have_target && aa_target_stats.count(target) != 0) {
            AaTargetStats& st = aa_target_stats[target];
            ++st.shots;
            float aim[3], mz[3];
            unit_aim_point(target, aim);
            unit_aim_point(owner_unit, mz);
            const float sp[3] = {aim[0] - mz[0], aim[1] - mz[1], aim[2] - mz[2]};
            const float r = length3(sp);
            if (st.shot_range_min < 0.0f || r < st.shot_range_min) st.shot_range_min = r;
        }
        ++gun.shots;
        ++state.row.shots;
        ++summary.shots;
        if (torpedo_gun) ++summary.torpedo_gun_shots;
        if (kTorpedoFriendlyCrossingBound && gun.category == bsp::kUnitGunneryTorpedoCategory) {
            torpedo_friendly_diag = true;     // DIAGNOSTIC line only
            torpedo_friendly_hold_008fff20(gun, owner_unit, muzzle, right, forward,
                torpedo_lead_xz, torpedo_snap_radians);
            torpedo_friendly_diag = false;
        }
        if (gun.first_shot_seconds < 0.0f) gun.first_shot_seconds = clock_seconds;
        if (summary.first_shot_seconds < 0.0f) {
            summary.first_shot_seconds = clock_seconds;
            float aim[3] = {0.0f, 0.0f, 0.0f};
            if (target < unit_state.size()) unit_aim_point(target, aim);
            const float span[3] = {aim[0] - muzzle[0], aim[1] - muzzle[1],
                aim[2] - muzzle[2]};
            log.notef("gunnery: first shot at t=%.2f s, %s platform %d (%s, bullet %d) at "
                "%s, range %.0f m, horz %.1f deg, vert %.1f deg",
                static_cast<double>(clock_seconds), gun.unit_name.c_str(),
                gun.platform_key, gun.function.c_str(), gun.bullet_class,
                gun.target_name.c_str(), static_cast<double>(length3(span)),
                static_cast<double>(gun.angles.horz * 57.2957795f),
                static_cast<double>(gun.angles.vert * 57.2957795f));
        }
        done("Gun::fire_00730160", 0x00730160u);
        done("Gun::next_ready_barrel_007298d0", 0x007298d0u);

        // 0072BF10 then the factory 006E8430: the muzzle direction with the
        // class `Throw` spread, and the launch velocity.
        const float horz = gun.angles.horz;
        const float vert = gun.angles.vert;
        const float flat_component = std::cos(vert);
        float direction[3];
        for (int i = 0; i < 3; ++i) {
            direction[i] = forward[i] * (flat_component * std::cos(horz))
                + right[i] * (flat_component * kGunHorzSign * std::sin(horz))
                + up[i] * std::sin(vert);
        }
        if (gun.shots == 1) {
            // DIAGNOSTIC, both sides: the throw terms this gun fires with.
            int seat = -1;
            const float m = bullet_throw_magnitude_0073031d(gun, owner_unit, seat);
            log.notef("gunnery: throw first shot %s plat=%d cat=%d sub=%d throw_deg=%.3f "
                "level=%d seat=%d magnitude_deg=%.3f (0073031D)", state.row.name.c_str(),
                gun.platform_key, gun.category, gun.bullet_sub_type,
                static_cast<double>(gun.throw_amount * 57.2957795f),
                units.skill_level(owner_unit), seat, static_cast<double>(m * 57.2957795f));
        }
        if constexpr (kBulletThrowBound) {
            // Packet cc9_bullet_throw: 0073031D..0073075E.
            int seat = -1;
            const float magnitude = bullet_throw_magnitude_0073031d(gun, owner_unit, seat);
            ++throw_by_seat[seat >= 0 ? seat : kThrowSeatCount];
            if (!(magnitude > 0.0f)) {
                ++throw_zero_shots;                                   // 00730525 JBE
            } else if (gun.bullet_sub_type == 0x0A) {
                // 007304A0..0073051A: the deterministic torpedo fan, about the
                // hull up axis here (0085C3F0 rotates about row 1).
                const float offset = bsp::gun_torpedo_fan_offset_007304a0(magnitude,
                    std::max(1, gun.barrel_num), std::max(0, barrel));
                const float c = std::cos(offset), s = std::sin(offset);
                float side[3] = {up[1] * direction[2] - up[2] * direction[1],
                    up[2] * direction[0] - up[0] * direction[2],
                    up[0] * direction[1] - up[1] * direction[0]};
                for (int i = 0; i < 3; ++i) direction[i] = direction[i] * c + side[i] * s;
                ++throw_fan_shots;
            } else {
                // 00730540..0073058F: theta = U(0, 2pi), radius = tan(m) * U(0, 1).
                const float theta = draw(Draw::bullet_throw, g, 0, 0.0f, 6.28318548f);
                const float u = draw(Draw::bullet_throw, g, 0, 0.0f, 1.0f);
                const float radius = static_cast<float>(std::tan(magnitude) * u);
                // 007305D6 (sub-types 4..7) times unit+63Ch = 1.0; 0073062C times
                // 00470440(7) = 1.0. Then 00730654..0073075E, not renormalised.
                float a[3] = {up[1] * direction[2] - up[2] * direction[1],
                    up[2] * direction[0] - up[0] * direction[2],
                    up[0] * direction[1] - up[1] * direction[0]};
                float la = length3(a);
                if (la < 1e-6f) { a[0] = right[0]; a[1] = right[1]; a[2] = right[2]; la = 1.0f; }
                for (int i = 0; i < 3; ++i) a[i] /= la;
                const float b[3] = {direction[1] * a[2] - direction[2] * a[1],
                    direction[2] * a[0] - direction[0] * a[2],
                    direction[0] * a[1] - direction[1] * a[0]};
                const float ct = std::cos(theta), st = std::sin(theta);
                for (int i = 0; i < 3; ++i) {
                    direction[i] += radius * (ct * b[i] + st * a[i]);
                }
                ++throw_cone_shots;
                throw_angle_sum_deg += std::atan(radius) * 57.29577951308232;
                throw_magnitude_sum_deg += magnitude * 57.29577951308232;
            }
            done("Gun::throw_cone_00730540", 0x00730540u);
        }
        GameProjectileRow shot;
        shot.gun_row = g;
        shot.owner_unit = owner_unit + 1;
        shot.owner_side = units.unit_side_0054(owner_unit);
        shot.bullet_class = gun.bullet_class;
        float launch_speed = gun.muzzle_speed;
        if (have_target) {
            if (const SecondAmmo* dp = dp_air_ammo(g, gun.category, target)) {
                shot.bullet_class = dp->bullet_class;
                launch_speed = dp->muzzle_speed;
                ++dp_air_rounds;
            }
        }
        shot.alive = true;
        // Packet cc9_muzzle_offsets: 00730762's per-barrel muzzle point.
        float spawn[3] = {muzzle[0], muzzle[1], muzzle[2]};
        if constexpr (kMuzzleOffsetsBound) {
            float at_muzzle[3];
            if (gun_barrel_muzzle_world_00730762(gun, barrel, right, up, forward, origin,
                    at_muzzle)) {
                const float shift[3] = {at_muzzle[0] - muzzle[0], at_muzzle[1] - muzzle[1],
                    at_muzzle[2] - muzzle[2]};
                const double moved = static_cast<double>(length3(shift));
                ++muzzle_offset_shots;
                muzzle_shift_sum += moved;
                muzzle_shift_max = std::max(muzzle_shift_max, moved);
                for (int i = 0; i < 3; ++i) spawn[i] = at_muzzle[i];
                done("Gun::muzzle_world_position_00730762", 0x00730762u);
                if (!muzzle_trace_prefix().empty()
                    && state.row.name.compare(0, muzzle_trace_prefix().size(),
                           muzzle_trace_prefix()) == 0) {
                    const bsp::CameraMatrix& w = last_muzzle_node_world;
                    const float fx = w[8], fz = w[10];     // node row 2, the facing
                    const float fl = std::sqrt(fx * fx + fz * fz);
                    const float along = fl > 0.0f ? (shift[0] * fx + shift[2] * fz) / fl : 0.0f;
                    const float across = fl > 0.0f ? (shift[0] * fz - shift[2] * fx) / fl : 0.0f;
                    float tp[3] = {0.0f, 0.0f, 0.0f};
                    if (have_target) unit_aim_point(target, tp);
                    const auto fp_it = fire_points_by_device.find(gun.device_class);
                    const char* node_name = "?";
                    if (fp_it != fire_points_by_device.end() && last_muzzle_pick >= 0
                        && static_cast<std::size_t>(last_muzzle_pick) < fp_it->second.nodes.size()) {
                        node_name = fp_it->second.nodes[static_cast<std::size_t>(last_muzzle_pick)].name.c_str();
                    }
                    log.notef("gunnery: muzzle trace t=%.2f %s plat=%d cat=%d barrel=%d node=%d(%s) "
                        "mount=(%.1f %.1f %.1f) muzzle=(%.1f %.1f %.1f) facing_deg=%.1f "
                        "shot_deg=%.1f target=%s target_deg=%.1f shift_along=%.2f shift_across=%.2f "
                        "shift_up=%.2f",
                        static_cast<double>(clock_seconds), state.row.name.c_str(), gun.platform_key,
                        gun.category, barrel, last_muzzle_pick, node_name,
                        static_cast<double>(muzzle[0]), static_cast<double>(muzzle[1]),
                        static_cast<double>(muzzle[2]), static_cast<double>(at_muzzle[0]),
                        static_cast<double>(at_muzzle[1]), static_cast<double>(at_muzzle[2]),
                        static_cast<double>(std::atan2(fx, fz) * 57.2957795f),
                        static_cast<double>(std::atan2(direction[0], direction[2]) * 57.2957795f),
                        have_target ? unit_state[target].row.name.c_str() : "none",
                        static_cast<double>(std::atan2(tp[0] - muzzle[0], tp[2] - muzzle[2]) * 57.2957795f),
                        static_cast<double>(along), static_cast<double>(across),
                        static_cast<double>(shift[1]));
                }
            } else {
                ++muzzle_offset_fallbacks;
            }
        }
        for (int i = 0; i < 3; ++i) shot.position[i] = spawn[i];
        const bsp::TickPoint3 launch_direction{direction[0], direction[1], direction[2]};
        const bsp::TickPoint3 velocity = bsp::projectile_launch_velocity_006e8430(
            launch_speed, launch_direction, 0);
        shot.flight.velocity = velocity;
        shot.flight.snapshot_current = bsp::TickPoint3{spawn[0], spawn[1], spawn[2]};
        shot.flight.local_position = shot.flight.snapshot_current;
        shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
        {
            const GameBulletClassRow* round_class = bullet(shot.bullet_class);
            shot.flight.class_disables_gravity = kBulletNoGravityBound
                && round_class != nullptr && round_class->no_gravity;   // classDesc+20h
            if (shot.flight.class_disables_gravity) ++no_gravity_shots;
        }
        if (kTorpedoGyroHeadingBound && gun.category == bsp::kUnitGunneryTorpedoCategory
            && torpedo_gun && (torpedo_lead_xz[0] != 0.0f || torpedo_lead_xz[1] != 0.0f)) {
            // 008FFF20 00900476..00900526: the run line's world heading from the
            // gun, then 00900830..00900876 the AngleErr jitter; 007311B0 carries
            // it and 00856637 installs it as record+46Ch.
            const float h = gun.angles.horz;
            const float hs = kGunHorzSign * std::sin(h);
            const std::array<float, 2> axis{{forward[0] * std::cos(h) + right[0] * hs,
                forward[2] * std::cos(h) + right[2] * hs}};
            const std::array<float, 2> run_end = bsp::torpedo_run_end_008fff20(
                {{muzzle[0], muzzle[2]}}, torpedo_lead_xz, axis, torpedo_snap_radians);
            const float dx = run_end[0] - muzzle[0], dz = run_end[1] - muzzle[2];
            if (dx != 0.0f || dz != 0.0f) {
                int level = units.skill_level(owner_unit);
                if (level < 0 || level > 5) level = 1;
                const float magnitude = draw(Draw::torpedo_gyro, g, 0,
                    kTorpedoAngleErr[level][0], kTorpedoAngleErr[level][1]);
                // 0090083E 00BD2FC0 & 1 -> +1 or -1 (0090084A..0090084F).
                const float sign = draw(Draw::torpedo_gyro, g, 1, 0.0f, 1.0f) < 0.5f
                    ? -1.0f : 1.0f;
                shot.commanded_heading = static_cast<float>(std::atan2(dx, dz)
                    + sign * magnitude * 3.14159265358979 / 180.0);
                const float vyaw = std::atan2(shot.flight.velocity.x, shot.flight.velocity.z);
                torpedo_gyro_offset_sum_deg += std::fabs(bsp::wrapped_angle_subtract_00438b10(
                    shot.commanded_heading, vyaw)) * 57.2957795;
                ++torpedo_gyro_launches;
            }
        }
        shots.push_back(shot);
        ++summary.projectiles;
        done("Projectile::launch_velocity_006e8430", 0x006e8430u);
        done("Projectile::spawn_0072bf10", 0x0072bf10u);
    }
}

// ---------------------------------------------------------------------------
// Projectile flight, the sweep and the impact
// ---------------------------------------------------------------------------

namespace {

class SegmentBinding final : public bsp::SegmentQueryHost {
public:
    SegmentBinding(GameGunneryHost::Impl& owner, std::size_t exclude)
        : owner_(owner), exclude_(exclude) {}

    float grid_cell_size() override { return 0.0f; }  // no spatial grid here
    const void* cell_first_node(int, int) override { return nullptr; }
    const void* cell_next_node(const void*) override { return nullptr; }
    const void* cell_node_entity(const void*) override { return nullptr; }
    int loose_entity_count() override { return static_cast<int>(owner_.units.count()); }
    const void* loose_entity(int slot) override {
        return reinterpret_cast<const void*>(static_cast<std::size_t>(slot) + 1);
    }
    const void* entity_owner(const void* entity) override { return entity; }
    bool entity_is_kind(const void*, int) override { return true; }
    bsp::HitQueryBounds entity_bounds(const void* entity) override {
        bsp::HitQueryBounds bounds;
        const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
        float centre[3], half[3];
        if (!box_of(index, centre, half)) return bounds;
        bounds.min.x = centre[0] - half[0];
        bounds.min.y = centre[1] - half[1];
        bounds.min.z = centre[2] - half[2];
        bounds.max.x = centre[0] + half[0];
        bounds.max.y = centre[1] + half[1];
        bounds.max.z = centre[2] + half[2];
        return bounds;
    }
    int shape_count(const void* entity) override {
        const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
        if (index == exclude_) return 0;
        if (index >= owner_.unit_state.size()) return 0;
        if (owner_.unit_state[index].dead) return 0;
        if (!owner_.units.unit_alive_and_visible(index)) return 0;
        return 1;
    }
    bool shape_trace_segment(const void* entity, int, const bsp::HitQueryPoint& from,
        const bsp::HitQueryPoint& to, bsp::HitRecordFill& record) override;
    int child_count(const void*) override { return 0; }
    const void* child_entity(const void*, int) override { return nullptr; }

    std::size_t hit_unit{0};   // one based

private:
    bool box_of(std::size_t index, float centre[3], float half[3]) const;

    GameGunneryHost::Impl& owner_;
    std::size_t exclude_;
};

bool SegmentBinding::box_of(std::size_t index, float centre[3], float half[3]) const {
    if (index >= owner_.unit_state.size()) return false;
    const GameGunneryHost::Impl::UnitState& state = owner_.unit_state[index];
    float right[3], up[3], forward[3], origin[3];
    owner_.unit_pose(index, right, up, forward, origin);
    if (const auto* model = owner_.ship_mesh_of(index)) {
        // Packet cc9_hull_sections: the mesh bounds, posed, as the broad phase.
        float c[3], e[3];
        for (int k = 0; k < 3; ++k) {
            c[k] = (model->mesh_box[k] + model->mesh_box[k + 3]) * 0.5f;
            e[k] = (model->mesh_box[k + 3] - model->mesh_box[k]) * 0.5f;
        }
        for (int i = 0; i < 3; ++i) {
            centre[i] = origin[i] + right[i] * c[0] + up[i] * c[1] + forward[i] * c[2];
            half[i] = std::fabs(right[i]) * e[0] + std::fabs(up[i]) * e[1]
                + std::fabs(forward[i]) * e[2];
        }
        return true;
    }
    const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
        state.hull_length * 0.5f};
    if (extents[0] <= 0.0f || extents[2] <= 0.0f) return false;
    for (int i = 0; i < 3; ++i) {
        centre[i] = origin[i];
        half[i] = std::fabs(right[i]) * extents[0] + std::fabs(up[i]) * extents[1]
            + std::fabs(forward[i]) * extents[2];
    }
    return true;
}

bool SegmentBinding::shape_trace_segment(const void* entity, int,
    const bsp::HitQueryPoint& from, const bsp::HitQueryPoint& to,
    bsp::HitRecordFill& record) {
    const std::size_t index = reinterpret_cast<std::size_t>(entity) - 1;
    float centre[3], half[3];
    if (!box_of(index, centre, half)) return false;
    // The hull box in its own frame: a slab test along the three pose rows.
    float right[3], up[3], forward[3], origin[3];
    owner_.unit_pose(index, right, up, forward, origin);
    if (const auto* model = owner_.ship_mesh_of(index)) {
        // 00723E90 / 00723D60 / 00723AA0: the segment in the ship's frame
        // against every GeomMesh triangle; the closest hit wins.
        const float* ax[3] = {right, up, forward};
        const float wf[3] = {from.x, from.y, from.z};
        const float wt[3] = {to.x, to.y, to.z};
        float o[3], d[3];
        for (int i = 0; i < 3; ++i) {
            o[i] = (wf[0] - origin[0]) * ax[i][0] + (wf[1] - origin[1]) * ax[i][1]
                + (wf[2] - origin[2]) * ax[i][2];
            d[i] = (wt[0] - wf[0]) * ax[i][0] + (wt[1] - wf[1]) * ax[i][1]
                + (wt[2] - wf[2]) * ax[i][2];
        }
        float best = 2.0f;
        const auto& tri = model->tris;
        for (std::size_t n = 0; n + 2 < tri.size(); n += 3) {
            const auto& a = tri[n];
            const auto& b = tri[n + 1];
            const auto& c = tri[n + 2];
            const float e1[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
            const float e2[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
            const float p[3] = {d[1] * e2[2] - d[2] * e2[1], d[2] * e2[0] - d[0] * e2[2],
                d[0] * e2[1] - d[1] * e2[0]};
            const float det = e1[0] * p[0] + e1[1] * p[1] + e1[2] * p[2];
            if (std::fabs(det) < 1e-12f) continue;
            const float inv = 1.0f / det;
            const float s0[3] = {o[0] - a[0], o[1] - a[1], o[2] - a[2]};
            const float u = (s0[0] * p[0] + s0[1] * p[1] + s0[2] * p[2]) * inv;
            if (u < 0.0f || u > 1.0f) continue;
            const float q[3] = {s0[1] * e1[2] - s0[2] * e1[1], s0[2] * e1[0] - s0[0] * e1[2],
                s0[0] * e1[1] - s0[1] * e1[0]};
            const float v = (d[0] * q[0] + d[1] * q[1] + d[2] * q[2]) * inv;
            if (v < 0.0f || u + v > 1.0f) continue;
            const float t = (e2[0] * q[0] + e2[1] * q[1] + e2[2] * q[2]) * inv;
            if (t >= 0.0f && t <= 1.0f && t < best) best = t;
        }
        if (best > 1.0f) return false;
        bsp::HitQueryPoint point;
        point.x = wf[0] + (wt[0] - wf[0]) * best;
        point.y = wf[1] + (wt[1] - wf[1]) * best;
        point.z = wf[2] + (wt[2] - wf[2]) * best;
        bsp::shape_hit_fill_0087fec0(record, point, entity);
        record.shape_kind = 0x0A;
        record.hull_segment = kDirectHitHullSegment;
        bsp::hit_record_set_entity_00470370(record, entity);
        hit_unit = index + 1;
        ++owner_.shell_mesh_hits;
        return true;
    }
    const GameGunneryHost::Impl::UnitState& state = owner_.unit_state[index];
    const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
        state.hull_length * 0.5f};
    const float world_from[3] = {from.x, from.y, from.z};
    const float world_to[3] = {to.x, to.y, to.z};
    const float rel[3] = {world_from[0] - origin[0], world_from[1] - origin[1],
        world_from[2] - origin[2]};
    const float span[3] = {world_to[0] - world_from[0], world_to[1] - world_from[1],
        world_to[2] - world_from[2]};
    const float* axes[3] = {right, up, forward};
    float local_origin[3];
    float local_span[3];
    for (int i = 0; i < 3; ++i) {
        local_origin[i] = dot3(rel, axes[i]);
        local_span[i] = dot3(span, axes[i]);
    }
    if constexpr (kAabb0085cdb0Bound) {
        // 00929B80: both endpoints into the shape's frame, 0085CDB0 on the
        // local box, the point back to world (here along the world segment).
        bsp::HitQueryBounds local_box;
        local_box.min = {-extents[0], -extents[1], -extents[2]};
        local_box.max = {extents[0], extents[1], extents[2]};
        bsp::HitQueryPoint local_hit;
        float entry = 0.0f;
        if (!bsp::segment_box_hit_0085cdb0(local_box,
                {local_origin[0], local_origin[1], local_origin[2]},
                {local_origin[0] + local_span[0], local_origin[1] + local_span[1],
                 local_origin[2] + local_span[2]}, local_hit, &entry)) {
            return false;
        }
        ++owner_.narrowphase_box_0085cdb0;
        bsp::HitQueryPoint point;
        point.x = world_from[0] + span[0] * entry;
        point.y = world_from[1] + span[1] * entry;
        point.z = world_from[2] + span[2] * entry;
        bsp::shape_hit_fill_0087fec0(record, point, entity);
        record.shape_kind = 0x0A;
        record.hull_segment = kDirectHitHullSegment;
        bsp::hit_record_set_entity_00470370(record, entity);
        hit_unit = index + 1;
        return true;
    }
    float enter = 0.0f;
    float leave = 1.0f;
    for (int i = 0; i < 3; ++i) {
        const float low = -extents[i];
        const float high = extents[i];
        if (std::fabs(local_span[i]) < 1.0e-6f) {
            if (local_origin[i] < low || local_origin[i] > high) return false;
            continue;
        }
        float t0 = (low - local_origin[i]) / local_span[i];
        float t1 = (high - local_origin[i]) / local_span[i];
        if (t0 > t1) std::swap(t0, t1);
        enter = std::max(enter, t0);
        leave = std::min(leave, t1);
        if (enter > leave) return false;
    }
    bsp::HitQueryPoint point;
    point.x = world_from[0] + span[0] * enter;
    point.y = world_from[1] + span[1] * enter;
    point.z = world_from[2] + span[2] * enter;
    bsp::shape_hit_fill_0087fec0(record, point, entity);
    record.shape_kind = 0x0A;
    record.hull_segment = kDirectHitHullSegment;
    bsp::hit_record_set_entity_00470370(record, entity);
    hit_unit = index + 1;
    return true;
}

}  // namespace

void GameGunneryHost::Impl::run_projectiles(float dt) {
    for (GameProjectileRow& shot : shots) {
        if (!shot.alive) continue;
        if (shot.serial == 0) shot.serial = ++next_projectile_serial;
        if (shot.swimming) shot.swim_seconds += dt;   // 0085748A, record+488h += dt
        if (kTorpedoGyroHeadingBound && shot.swimming && shot.commanded_heading != 10000.0f) {
            // 00857061: e = -SubtractWrapped(yaw, record+46Ch), clamped to
            // +/- HeadingTurn (deg/s, classDesc+0E8h) * pi/180, times dt.
            const GameBulletClassRow* round = bullet(shot.bullet_class);
            const float turn_rate = round != nullptr ? round->heading_turn : 0.0f;
            const float vx = shot.flight.velocity.x, vz = shot.flight.velocity.z;
            const float speed = std::sqrt(vx * vx + vz * vz);
            if (turn_rate > 0.0f && speed > 0.0f) {
                const float yaw = std::atan2(vx, vz);
                const float e = bsp::wrapped_angle_subtract_00438b10(shot.commanded_heading, yaw);
                const float rate = static_cast<float>(turn_rate * 3.14159265358979 / 180.0);
                const float step = std::max(-rate, std::min(rate, e)) * dt;
                if (step != 0.0f) {
                    const float ny = yaw + step;
                    shot.flight.velocity.x = speed * std::sin(ny);
                    shot.flight.velocity.z = speed * std::cos(ny);
                    ++torpedo_gyro_turn_steps;
                }
            }
        }
        const float from[3] = {shot.position[0], shot.position[1], shot.position[2]};
        shot.flight = bsp::projectile_flight_step(shot.flight, dt);
        shot.position[0] = shot.flight.local_position.x;
        shot.position[1] = shot.flight.local_position.y;
        shot.position[2] = shot.flight.local_position.z;
        shot.flight.snapshot_current = shot.flight.local_position;
        shot.life += dt;
        ++summary.projectile_steps;
        done("Projectile::flight_step_006e7670", 0x006e7670u);

        // TRACE, packet cc8_torpedo_swim item 1. Additive logging only: follow
        // every dropped torpedo from its drop line's id until it leaves the
        // list, and name the exit that took it. Nothing here changes a value.
        const bool trace = shot.torpedo_trace_id != 0;
        if (trace) {
            log.notef("  torpedo trace %llu t=%.2f life=%.2f from_y=%.2f "
                "pos=(%.1f,%.2f,%.1f) vel=(%.1f,%.2f,%.1f) swimming=%d",
                shot.torpedo_trace_id, static_cast<double>(clock_seconds),
                static_cast<double>(shot.life), static_cast<double>(from[1]),
                static_cast<double>(shot.position[0]),
                static_cast<double>(shot.position[1]),
                static_cast<double>(shot.position[2]),
                static_cast<double>(shot.flight.velocity.x),
                static_cast<double>(shot.flight.velocity.y),
                static_cast<double>(shot.flight.velocity.z),
                shot.swimming ? 1 : 0);
        }

        // Packet cc8_torpedo_closest_approach. A swimming round carries no
        // target - GameProjectileRow has an owner and no victim, and the swim
        // keeps its launch heading - so the closest approach is measured
        // against every unit of another side rather than against an assumed
        // target. Horizontal only: the round is levelled onto the surface
        // plane at 009D0CE0's swim and a ship's y is its waterline.
        if (shot.swimming) {
            const std::size_t unit_count = units.count();
            for (std::size_t i = 0; i < unit_count; ++i) {
                if (i + 1 == shot.owner_unit) continue;
                if (units.unit_side_0054(i) == shot.owner_side) continue;
                float ux = 0.0f, uy = 0.0f, uz = 0.0f;
                units.unit_position_00fc(i, ux, uy, uz);
                const float dx = shot.position[0] - ux;
                const float dz = shot.position[2] - uz;
                const float d = std::sqrt(dx * dx + dz * dz);
                if (shot.min_enemy_distance < 0.0f || d < shot.min_enemy_distance) {
                    shot.min_enemy_distance = d;
                    shot.min_enemy_time = shot.life;
                    shot.min_enemy_unit = i + 1;
                }
                // Packet cc8_torpedo_aim_census: the same minimum against the
                // ORDERED target, kept separately because the nearest unit and
                // the aimed-at unit need not be the same ship.
                if (shot.ordered_target == i + 1 &&
                    (shot.ordered_min_distance < 0.0f ||
                     d < shot.ordered_min_distance)) {
                    shot.ordered_min_distance = d;
                    shot.ordered_min_time = shot.life;
                    shot.target_pos_at_min[0] = ux;
                    shot.target_pos_at_min[1] = uy;
                    shot.target_pos_at_min[2] = uz;
                    // The round's track against the target's course. Both are
                    // the game's compass convention (0 = +Z, pi/2 = +X), so the
                    // difference is taken with the same wrap the planner uses.
                    const float track = std::atan2(shot.flight.velocity.x,
                                                   shot.flight.velocity.z);
                    const float course = units.unit_heading_radians(i);
                    shot.crossing_angle =
                        std::fabs(bsp::wrapped_angle_subtract_00438b10(track, course));
                }
            }
        }

        const bsp::TickPoint3 a{from[0], from[1], from[2]};
        const bsp::TickPoint3 b{shot.position[0], shot.position[1], shot.position[2]};
        if (!bsp::projectile_segment_is_sweepable(a, b)) {
            if (trace) {
                log.notef("  torpedo trace %llu skip=not_sweepable",
                    shot.torpedo_trace_id);
            }
            continue;
        }
        ++summary.sweeps;

        // 0084BF00 step 2: the entity sweep. The static trace is the water
        // surface, which 0078CF20 answers at height zero for open sea.
        SegmentBinding query(*this, shot.owner_unit - 1);
        bsp::SegmentQueryArgs args;
        args.from = bsp::HitQueryPoint{from[0], from[1], from[2]};
        args.to = bsp::HitQueryPoint{shot.position[0], shot.position[1], shot.position[2]};
        args.exclude_entity = reinterpret_cast<const void*>(shot.owner_unit);
        bsp::HitRecordFill record;
        bsp::hit_record_reset_00470470(record);
        const bool hit = bsp::query_segment_0098add0(query, args, record);
        done("Projectile::sweep_entities_0098add0", 0x0098add0u);

        if (hit && query.hit_unit != 0) {
            const float point[3] = {record.position.x, record.position.y,
                record.position.z};
            const float direction[3] = {shot.flight.velocity.x, shot.flight.velocity.y,
                shot.flight.velocity.z};
            ++summary.impacts_entity;
            if (trace) {
                log.notef("  torpedo trace %llu exit=entity_impact hit=%s "
                    "at=(%.1f,%.2f,%.1f) life=%.2f",
                    shot.torpedo_trace_id,
                    unit_name_or_index(query.hit_unit).c_str(),
                    static_cast<double>(point[0]), static_cast<double>(point[1]),
                    static_cast<double>(point[2]),
                    static_cast<double>(shot.life));
            }
            done("Projectile::on_impact_0084bc60", 0x0084bc60u);
            round_bullet_class = shot.bullet_class;
            apply_hit(shot.owner_unit - 1, shot.gun_row, query.hit_unit - 1, point,
                direction);
            // 0084BC60 step 7: the impact also spawns the burst that carries a
            // torpedo's warhead. Without it a torpedo does its DamageMin draw
            // and nothing else, which a carrier's Armour cancels exactly.
            //
            // The image's gate is the class descriptor's +6Ch and nothing else,
            // so this is NOT torpedo-only: every class with a Blast table
            // bursts on this step, and this loop carries bombs and shells too.
            // apply_impact_blast returns without doing anything when the row
            // has no Blast, which is that same gate.
            apply_impact_blast(shot.owner_unit - 1, shot.gun_row, point, direction);
            round_bullet_class = -1;
            shot.alive = false;
            continue;
        }

        // The water crossing 0078D1B0 solves and the ocean sampler 0078CF20
        // behind it: open sea is height zero.
        // Packet cc9_flak_proximity_burst. 0070C370 after its first call
        // 006E6490 (the base tick and its direct-strike sweep above):
        //   0070C3B1: FlyTime (+54h) spent -> 0070C210(proj, 0), the silent
        //             expiry the host's life bound already models;
        //   0070C3E2: armed once flight time > FlakFuseTime (+D8h; no reader
        //             writes it, so 0 from 006E8320) and the scaled step > 0;
        //   0070C41C: the step segment from the previous snapshot, its length L
        //             and unit direction; M the segment midpoint (0070C4A8);
        //   unlocked (byte +288h clear): walk the world entity list
        //             (008053C0 +DE8h) for IsKindOf(5) and one of 0Fh (plane),
        //             0Eh (MTorpedoBoat) or 0Ch (MLandingShip), no side test;
        //             d2 = |pos - M|^2 must be below min(90000 (00CFD508),
        //             (0.5 L + 2 BlastRange (+70h))^2) and below the best so
        //             far; each such entity is locked (+288h = 1, +294h), and
        //             00901C20 from the round's position with V0 (+50h) and a
        //             zero shooter velocity (00F87574) gives the aim point A;
        //             remaining = dot(A - prev, dir) + [+290h], floored at 0;
        //   locked:   L < remaining -> remaining -= L; else the round moves to
        //             prev + dir * remaining and 0070C210(proj, 1) bursts it:
        //             the radial blast at proj+FCh with radius BlastRange and
        //             U(BlastDamageMin, BlastDamageMax) (docs/EXPLOSION_RADIAL_DAMAGE.md).
        // SUBSTITUTIONS, labelled: the distance error [+290h] (the AAFlakBot's
        // DistErr, 008FDBE0 -> bot+60h) is 0, which is exact at the SPVeteran row
        // USN04 sets; entities are this host's units (dead ones skipped, as the
        // image's list drops a destroyed entity); the unlocked passing rule at
        // 0070C7B6-0070C806 (10% per tick beyond 50 m) is not modelled, since it
        // can only act on the tick that also locks.
        if constexpr (kFlakProximityBurstBound) {
            const GameBulletClassRow* const fc = bullet(shot.bullet_class);
            if (fc != nullptr && fc->type == "Flak" && fc->blast_range > 0.0f
                && shot.life > 0.0f) {
                const float seg[3] = {shot.position[0] - from[0],
                    shot.position[1] - from[1], shot.position[2] - from[2]};
                const float seg_len = length3(seg);
                if (seg_len > 0.0f) {
                    const float dir[3] = {seg[0] / seg_len, seg[1] / seg_len,
                        seg[2] / seg_len};
                    if (!shot.flak_locked) {
                        const float mid[3] = {(shot.position[0] + from[0]) * 0.5f,
                            (shot.position[1] + from[1]) * 0.5f,
                            (shot.position[2] + from[2]) * 0.5f};
                        const float reach = seg_len * 0.5f + fc->blast_range * 2.0f;
                        const float limit = std::min(90000.0f, reach * reach);
                        float best = std::numeric_limits<float>::max();
                        for (std::size_t i = 0; i < unit_state.size(); ++i) {
                            if (unit_state[i].dead) continue;
                            if (!units.unit_is_kind_of(i, bsp::kUnitGunneryKindPlaneBase)
                                && !units.unit_is_kind_of(i, 0x0E)
                                && !units.unit_is_kind_of(i, 0x0C)) continue;
                            float r[3], u[3], f[3], o[3];
                            unit_pose(i, r, u, f, o);
                            const float dx = o[0] - mid[0], dy = o[1] - mid[1],
                                dz = o[2] - mid[2];
                            const float d2 = dx * dx + dy * dy + dz * dz;
                            if (!(limit > d2) || !(best > d2)) continue;
                            best = d2;
                            shot.flak_locked = true;
                            shot.flak_target = i + 1;
                            float tv[3] = {0.0f, 0.0f, 0.0f};
                            if (!units.unit_linear_velocity(i, tv)) {
                                tv[0] = tv[1] = tv[2] = 0.0f;
                            }
                            if (units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) {
                                tv[1] = 0.0f;
                            }
                            const float rel[3] = {o[0] - shot.position[0],
                                o[1] - shot.position[1], o[2] - shot.position[2]};
                            const float dist = length3(rel);
                            float t = 0.0f;
                            if (fc->muzzle_speed >= 2.0f) {
                                t = intercept_time_00901c20(rel, tv, fc->muzzle_speed, dist);
                                t = aa_time_add_fix + t + (dist / 1000.0f) * aa_time_add_mul;
                            }
                            const float aim[3] = {o[0] + tv[0] * t, o[1] + tv[1] * t,
                                o[2] + tv[2] * t};
                            float rem = (aim[0] - from[0]) * dir[0]
                                + (aim[1] - from[1]) * dir[1] + (aim[2] - from[2]) * dir[2];
                            if (rem < 0.0f) rem = 0.0f;   // 0070C6D8-0070C6E3
                            shot.flak_remaining = rem;
                        }
                        if (shot.flak_locked) {
                            ++flak_locks;
                            done("FlakProjectile::proximity_lock_0070c661", 0x0070c661u);
                        }
                    }
                    if (shot.flak_locked) {
                        if (seg_len < shot.flak_remaining) {
                            shot.flak_remaining -= seg_len;   // 0070C7AD
                        } else {
                            const float burst[3] = {from[0] + dir[0] * shot.flak_remaining,
                                from[1] + dir[1] * shot.flak_remaining,
                                from[2] + dir[2] * shot.flak_remaining};
                            for (int i = 0; i < 3; ++i) shot.position[i] = burst[i];
                            const float no_dir[3] = {0.0f, 0.0f, 0.0f};
                            ++flak_bursts;
                            round_bullet_class = shot.bullet_class;
                            apply_impact_blast(shot.owner_unit - 1, shot.gun_row, burst, no_dir);
                            round_bullet_class = -1;
                            done("FlakProjectile::detonate_0070c210", 0x0070c210u);
                            shot.alive = false;
                            continue;
                        }
                    }
                }
            }
        }

        if (shot.position[1] <= 0.0f && from[1] > 0.0f) {
            ++summary.water_crossings;
            // A torpedo does not die at the surface: it enters its swim. The
            // round's record carries a swim speed at +470h, written at
            // 0085786D..00857875 as WaterTravelSpeed * the double at 00D0C5E0
            // (0.5999994277954102), and 00855A90 derives the engagement range as
            // that speed times FlyTime - so the authored range only makes sense
            // if the round actually travels at it underwater. Before this, every
            // torpedo was killed on the frame it touched the sea and 12 of 16
            // launches ended as `water`. docs/TORPEDO_LAUNCH_ACCURACY.md.
            const GameBulletClassRow* const entry = bullet(shot.bullet_class);
            const float swim = entry != nullptr ? entry->swim_speed : 0.0f;
            // 008568E0's first limit: the round breaks up if it hits the sea
            // faster than `MaxWaterHitVel` (classDesc+0DCh). This is why a
            // torpedo bomber has to release low and slow. The second limit,
            // `shot[+0Ch] < -classDesc[+0ECh]`, is the MaxFall-derived fall
            // speed against the round's own depth field and is NOT applied
            // here: this host has no such field on the shot.
            // docs/TORPEDO_TICK.md, docs/TORPEDO_RELEASE_SPAWN.md.
            const float entry_velocity[3] = {shot.flight.velocity.x,
                shot.flight.velocity.y, shot.flight.velocity.z};
            const float entry_speed = length3(entry_velocity);
            const float hit_limit = entry != nullptr ? entry->max_water_hit_vel : 0.0f;
            if (trace) {
                log.notef("  torpedo trace %llu water_crossing from_y=%.3f to_y=%.3f "
                    "life=%.2f swim=%.1f hit_limit=%.1f entry_speed=%.1f swimming=%d",
                    shot.torpedo_trace_id, static_cast<double>(from[1]),
                    static_cast<double>(shot.position[1]),
                    static_cast<double>(shot.life), static_cast<double>(swim),
                    static_cast<double>(hit_limit),
                    static_cast<double>(entry_speed), shot.swimming ? 1 : 0);
            }
            if (swim > 0.0f && !shot.swimming && hit_limit > 0.0f
                && entry_speed > hit_limit) {
                ++summary.water_entry_breakups;
                if (summary.water_entry_breakups <= 4) {
                    log.notef("gunnery: water entry broke the round up at %.1f m/s, "
                        "MaxWaterHitVel %.1f m/s (008568E0)",
                        static_cast<double>(entry_speed),
                        static_cast<double>(hit_limit));
                }
                ++summary.impacts_static;
                shot.alive = false;
                continue;
            }
            if (swim > 0.0f && !shot.swimming) {
                shot.swimming = true;
                // Level the round onto the surface plane at the swim speed,
                // keeping the heading the launch gave it. Gravity is turned off
                // through the flight state's own classDesc[+20h] flag rather
                // than by stepping the round outside the recovered
                // projectile_flight_step, so the swim still runs through the
                // reconstructed rule.
                shot.flight.class_disables_gravity = true;
                const float vx = shot.flight.velocity.x;
                const float vz = shot.flight.velocity.z;
                const float horizontal = std::sqrt(vx * vx + vz * vz);
                if (horizontal > 0.0f) {
                    shot.flight.velocity.x = vx / horizontal * swim;
                    shot.flight.velocity.z = vz / horizontal * swim;
                }
                shot.flight.velocity.y = 0.0f;
                shot.position[1] = 0.0f;
                shot.flight.local_position.y = 0.0f;
                shot.flight.snapshot_current.y = 0.0f;
                ++summary.torpedo_swims_started;
                if (trace) {
                    log.notef("  torpedo trace %llu swim_started life=%.2f "
                        "pos=(%.1f,%.2f,%.1f)", shot.torpedo_trace_id,
                        static_cast<double>(shot.life),
                        static_cast<double>(shot.position[0]),
                        static_cast<double>(shot.position[1]),
                        static_cast<double>(shot.position[2]));
                }
                continue;
            }
            ++summary.impacts_static;
            if (trace) {
                log.notef("  torpedo trace %llu exit=water_static life=%.2f",
                    shot.torpedo_trace_id, static_cast<double>(shot.life));
            }
            done("Projectile::water_crossing_0078d1b0", 0x0078d1b0u);
            shot.alive = false;
            continue;
        }
        const GameBulletClassRow* row = bullet(shot.bullet_class);
        const float range = row != nullptr ? row->range : 0.0f;
        // Once swimming, the round travels at the swim speed, which is the
        // speed 00855A90's range was derived against (swim * FlyTime), so the
        // life bound only lands on FlyTime if the same speed is used here.
        const float cruise = shot.swimming && row != nullptr && row->swim_speed > 0.0f
            ? row->swim_speed
            : (row != nullptr && row->muzzle_speed > 0.0f ? row->muzzle_speed : 1.0f);
        const float speed = cruise;
        if (range > 0.0f && shot.life * speed > range) {
            ++summary.expired;
            if (trace) {
                log.notef("  torpedo trace %llu exit=expired life=%.2f speed=%.1f "
                    "range=%.1f travelled=%.1f", shot.torpedo_trace_id,
                    static_cast<double>(shot.life), static_cast<double>(speed),
                    static_cast<double>(range),
                    static_cast<double>(shot.life * speed));
            }
            shot.alive = false;
        }
    }
    // Packet cc8_torpedo_closest_approach: keep a dying swimming round's closest
    // approach before the row is erased, because nothing else outlives it.
    for (const GameProjectileRow& row : shots) {
        if (row.alive || !row.swimming) continue;
        GameTorpedoApproachRow rec;
        rec.owner_name = unit_name_or_index(row.owner_unit);
        rec.nearest_name = unit_name_or_index(row.min_enemy_unit);
        rec.min_distance = row.min_enemy_distance;
        rec.min_time = row.min_enemy_time;
        rec.life_at_end = row.life;
        fill_ordered_fields(rec, row);
        torpedo_approaches.push_back(rec);
    }
    // Packet cc8_dive_glide, the same shape one hunk up and for the same
    // reason: a bomb's impact has to be taken before the row is erased.
    for (const GameProjectileRow& row : shots) {
        if (row.alive || !row.is_bomb) continue;
        GameBombImpactRow rec;
        rec.owner_name = unit_name_or_index(row.owner_unit);
        for (int i = 0; i < 3; ++i) {
            rec.predicted[i] = row.predicted_impact[i];
            rec.actual[i] = row.position[i];
            rec.target_release[i] = row.target_pos_release[i];
        }
        const float pdx = rec.actual[0] - rec.predicted[0];
        const float pdz = rec.actual[2] - rec.predicted[2];
        rec.predicted_error = std::sqrt(pdx * pdx + pdz * pdz);
        if (row.ordered_target != 0) {
            const float tdx = rec.actual[0] - rec.target_release[0];
            const float tdz = rec.actual[2] - rec.target_release[2];
            rec.target_error = std::sqrt(tdx * tdx + tdz * tdz);
        }
        // Packet cc8_dive_aim item 2. Everything above is sampled at the DROP;
        // these are sampled HERE, as the round dies, which is the only tick at
        // which "where the target actually is when the bomb arrives" exists.
        rec.release_fall_time = row.release_fall_time;
        rec.target_speed_release = row.target_speed_release;
        rec.target_heading_release = row.target_heading_release;
        if (row.ordered_target != 0) {
            rec.target_name = unit_name_or_index(row.ordered_target - 1);
            float ix = 0.0f, iy = 0.0f, iz = 0.0f;
            units.unit_position_00fc(row.ordered_target - 1, ix, iy, iz);
            rec.target_pos_impact[0] = ix;
            rec.target_pos_impact[1] = iy;
            rec.target_pos_impact[2] = iz;
            rec.target_heading_impact =
                units.unit_heading_radians(row.ordered_target - 1);
            const float mx = rec.actual[0] - ix;
            const float mz = rec.actual[2] - iz;
            rec.impact_error = std::sqrt(mx * mx + mz * mz);
            // Resolve the miss in the target's own frame. The hull heading is
            // the same pose convention the rest of this file uses, so its
            // forward axis is (sin h, cos h) and its starboard axis (cos h,
            // -sin h) - the convention 009C7C7C's `pi/2 - atan2(dz, dx)` fixes.
            const float sh = std::sin(rec.target_heading_impact);
            const float ch = std::cos(rec.target_heading_impact);
            rec.miss_along = mx * sh + mz * ch;
            rec.miss_across = mx * ch - mz * sh;
        }
        rec.life = row.life;
        rec.died_above_water = row.position[1] > 0.0f;
        bomb_impacts.push_back(rec);
    }
    shots.erase(std::remove_if(shots.begin(), shots.end(),
        [](const GameProjectileRow& row) { return !row.alive; }), shots.end());
}

// ---------------------------------------------------------------------------
// The hit record, the damage and the kill credit
// ---------------------------------------------------------------------------

namespace {

class ShipHitBinding final : public bsp::ShipHitRecordHost {
public:
    ShipHitBinding(GameGunneryHost::Impl& owner, std::size_t victim, std::size_t shooter,
        const GameBulletClassRow* weapon, const float direction[3])
        : owner_(owner), victim_(victim), shooter_(shooter), weapon_(weapon) {
        for (int i = 0; i < 3; ++i) direction_[i] = direction[i];
    }

    bool hull_is_submarine() override { return false; }
    float hull_armour() override { return owner_.unit_state[victim_].armour; }
    float class_armour_virtual() override { return owner_.unit_state[victim_].armour; }
    float class_mass() override {
        // [this+538h]+B0h, the `Mass` key. 00826FD0 compares it against 100 and
        // 500 to pick which of the three part-damage arms runs.
        return owner_.units.unit_hull_mass_00b0(victim_);
    }
    void clear_hit_accumulator() override {}

    float hull_damage(float armour) override {
        return bsp::hull_damage_00470510(hit_, armour);
    }
    float part_damage(float armour, int part_hit_index) override {
        return bsp::part_damage_004705c0(hit_, armour, part_hit_index);
    }

    int session_mode() override { return 0; }
    bool unit_is_local_players() override { return false; }
    unsigned campaign_difficulty_index() override { return 0; }
    float difficulty_multiplier(unsigned) override {
        owner_.record("ShipHit::difficulty_multiplier_008270ad", 0x008270adu);
        return 1.0f;
    }

    void apply_part_damage(int, const float[3], float damage) override {
        applied_ += damage;
        ++owner_.summary.part_damages;
        owner_.done("ShipHit::part_health_0092d1f0", 0x0092d1f0u);
    }
    void impact_direction(float out[3]) override {
        for (int i = 0; i < 3; ++i) out[i] = direction_[i];
    }

    void roll_axis(float out[3]) override { out[0] = 0.0f; out[1] = 1.0f; out[2] = 0.0f; }
    float settings_roll_torque_scale() override { return 0.0f; }
    float settings_roll_mass_root() override { return 0.0f; }
    void route_add_hull_torque(const bsp::ShipRollTorque&) override {
        owner_.record("ShipHit::add_hull_torque_00827312", 0x00827312u);
    }

    float weapon_water_damage() override {
        return weapon_ != nullptr ? weapon_->water_damage : 0.0f;
    }
    float weapon_fire_damage() override {
        return weapon_ != nullptr ? weapon_->fire_damage : 0.0f;
    }
    float weapon_fire_chance() override {
        return weapon_ != nullptr ? weapon_->fire_chance : 0.0f;
    }
    float random_unit_float() override {
        return owner_.draw(GameGunneryHost::Impl::Draw::hit_effect, victim_, 0, 0.0f, 1.0f);
    }
    void record_flood_rate(float rate) override {
        if (rate > 0.0f) {
            ++owner_.unit_state[victim_].row.floods_started;
            ++owner_.summary.flood_messages_9e;
        }
    }
    void record_fire_rate(float rate) override {
        if (rate > 0.0f) {
            ++owner_.unit_state[victim_].row.fires_started;
            ++owner_.summary.fire_messages_9e;
        }
    }
    void route_set_damage_channel(bsp::ShipDamageChannel, float, bool) override {
        owner_.done("ShipHit::damage_channel_message_9e_0080fa50", 0x0080fa50u);
    }

    void roll_component_failure(float) override {
        owner_.record("ShipHit::component_failure_0093bed0", 0x0093bed0u);
    }

    bool is_local_players_unit() override { return false; }
    bool shooter_world_position(float out[3]) override {
        float right[3], up[3], forward[3], origin[3];
        owner_.unit_pose(shooter_, right, up, forward, origin);
        for (int i = 0; i < 3; ++i) out[i] = origin[i];
        return true;
    }
    void push_damage_direction(const float[3]) override {}
    void route_hull_impact_effect(int, const float[3]) override {}
    void route_part_impact_effect(int, const float[3]) override {}

    // One AddDamage, the `0.0 < damage` test at 008778C6 / 00877A2E and the
    // 00879070 -> 00877B90 write both passes share.
    bool add_damage(float damage) {
        if (damage <= 0.0f) return false;
        applied_ += damage;
        bsp::UnitHealth health;
        health.current_health = owner_.unit_state[victim_].health;
        health.max_health = owner_.unit_state[victim_].max_health;
        bsp::UnitDamageGates gates;
        const bsp::UnitDamageOutcome outcome = bsp::apply_damage_00879070(health, gates,
            damage);
        owner_.done("ShipHit::apply_damage_00879070", 0x00879070u);
        if (outcome.refused) return false;
        const bsp::UnitHealthWrite write = bsp::set_health_00877b90(health,
            outcome.new_health, bsp::UnitSessionMode::campaign, 0, false);
        owner_.done("ShipHit::set_health_00877b90", 0x00877b90u);
        if (write.wrote) owner_.unit_state[victim_].health = write.stored_health;
        return true;
    }

    bool apply_base_hit_record() override {
        // 008777D0, the base hit record: the hull pass and the part pass. The
        // hull damage reaches the unit's own health through 00879070.
        //
        // The image gates the hull pass on `hit+34h != -1` (OR EDI,0FFFFFFFFh at
        // 008777DA, CMP [EBX+34h],EDI and JZ 008778D8 at 008777DD/E2). This host
        // does NOT reproduce that gate, deliberately: every hull segment index
        // the reconstruction has read is -1, so the gate as read would zero all
        // gunnery damage. docs/EXPLOSION_RADIAL_DAMAGE.md marks the producer of
        // a non-negative +34h as a labelled gap. Because the hull pass here is
        // ungated, a record that is meant to carry its damage in the part pass
        // must leave +14h at zero; apply_torpedo_blast does.
        const float armour = owner_.unit_state[victim_].armour;
        const float damage = bsp::hull_damage_00470510(hit_, armour);
        owner_.done("ShipHit::base_hit_record_008777d0", 0x008777d0u);
        bool applied_any = add_damage(damage);
        if (applied_any) ++owner_.summary.hull_damages;

        // The part pass, 008778D8..00877A43: one 004705C0 per entry of the
        // array at +3Ch, each floored at zero inside the formula and added
        // through the same 00879070 the hull pass uses (AddDamage at 00877A37).
        // A blast record carries its damage here, in +28h.
        for (int i = 0; i < hit_.part_hit_count; ++i) {
            const float part = bsp::part_damage_004705c0(hit_, armour, i);
            owner_.done("ShipHit::part_damage_004705c0", 0x004705c0u);
            if (add_damage(part)) applied_any = true;
        }
        return applied_any;
    }

    void set_hit(const bsp::HitRecord& hit) noexcept { hit_ = hit; }
    float applied() const noexcept { return applied_; }

private:
    GameGunneryHost::Impl& owner_;
    std::size_t victim_;
    std::size_t shooter_;
    const GameBulletClassRow* weapon_;
    float direction_[3]{};
    bsp::HitRecord hit_{};
    float applied_{0.0f};
};

}  // namespace

void GameGunneryHost::Impl::apply_hit(std::size_t shooter, std::size_t gun_row,
    std::size_t victim, const float point[3], const float direction[3],
    const bsp::HitRecord* blast_record) {
    if (victim >= unit_state.size() || shooter >= unit_state.size()) return;
    UnitState& target = unit_state[victim];
    if (target.dead) return;
    const GameGunRow& gun = guns[gun_row];
    const int priced_class = round_bullet_class >= 0 ? round_bullet_class : gun.bullet_class;
    if (gun.category == 7 && victim != shooter
        && unit_state[shooter].row.side == target.row.side) {
        // DIAGNOSTIC, packet cc9_usn02_sameside_torpedoes: a torpedo on its own side.
        float sp[3], vp[3], r[3], u[3], f[3];
        unit_pose(shooter, r, u, f, sp);
        unit_pose(victim, r, u, f, vp);
        log.notef("gunnery: friendly torpedo hit t=%.2f shooter=%s plat=%d victim=%s "
            "point=(%.0f %.0f) shooter_pos=(%.0f %.0f) victim_pos=(%.0f %.0f) %s",
            static_cast<double>(clock_seconds), unit_state[shooter].row.name.c_str(),
            gun.platform_key, target.row.name.c_str(), static_cast<double>(point[0]),
            static_cast<double>(point[2]), static_cast<double>(sp[0]), static_cast<double>(sp[2]),
            static_cast<double>(vp[0]), static_cast<double>(vp[2]),
            blast_record != nullptr ? "blast" : "direct");
    }
    if (aa_trace_matches(unit_state[shooter].row.name)) {
        log.notef("  aa hit t=%.2f %s gun=%zu cat=%d on %s %s health_before=%.1f",
            static_cast<double>(clock_seconds), unit_state[shooter].row.name.c_str(), gun_row,
            gun.category, target.row.name.c_str(), blast_record != nullptr ? "blast" : "direct",
            static_cast<double>(target.health));
    }
    const GameBulletClassRow* weapon = bullet(priced_class);

    // 00926E80 queues the record, 009239A0 dispatches it: the entity gates, the
    // OnHit delivery and the attribution listener.
    ++summary.queued_hits;
    done("Projectile::queue_hit_00926e80", 0x00926e80u);

    // 00470350 HitRecord::SetShot: record+14h = shot->vtable[54h](), which is
    // 006E7C60, a uniform draw between the weapon class's DamageMin (+ACh) and
    // DamageMax (+B0h) through 00BD2F10. A direct segment hit gets no part
    // damage: +28h stays zero (docs/HIT_NARROWPHASE.md).
    bsp::HitRecord hit;
    const float low = weapon != nullptr ? weapon->damage_min : 0.0f;
    const float high = weapon != nullptr ? weapon->damage_max : 0.0f;
    hit.hull_damage_base = draw(Draw::hull_damage, gun_row, victim, low, high);
    hit.part_damage_base = 0.0f;
    hit.falloff_range = 0.0f;
    hit.ignore_falloff = false;
    hit.armour_selector = 0.0f;
    hit.hull_segment = kDirectHitHullSegment;
    hit.weapon_scale = 1.0f;   // shot->vtable[58h] = 006E7C90, gun+43Ch, default 1.0f
    hit.owner_modifier = 1.0f; // 008E6430 over an empty modifier list
    hit.part_hits = nullptr;
    hit.part_hit_count = 0;
    done("Projectile::hit_record_set_shot_00470350", 0x00470350u);
    done("Projectile::shot_damage_base_006e7c60", 0x006e7c60u);

    // A blast record replaces the direct-hit one whole: 0084BAD0 builds its own
    // record per gathered entity (+28h the burst damage, +24h the radius, +2Ch
    // the ignore-falloff byte) and the direct DamageMin/DamageMax draw plays no
    // part in it.
    if (blast_record != nullptr) hit = *blast_record;

    bsp::ShipHitRecordView view;
    view.segment_kind = 0x0A;
    for (int i = 0; i < 3; ++i) view.impact_point[i] = point[i];
    view.shot_present = true;
    view.shot_is_depth_charge = false;
    view.shot_is_torpedo = gun.category == bsp::kUnitGunneryTorpedoCategory;
    view.weapon_present = weapon != nullptr;

    ShipHitBinding binding(*this, victim, shooter, weapon, direction);
    binding.set_hit(hit);
    const float before = target.health;
    bsp::apply_ship_hit_record_00826f10(binding, hit, view);
    ++summary.ship_hit_records;
    ++summary.dispatched_hits;
    done("ShipHit::apply_hit_record_00826f10", 0x00826f10u);
    done("Projectile::dispatch_queued_hit_009239a0", 0x009239a0u);

    const float applied = before - target.health;
    if (death_table_enabled() && applied > 0.0f && gun.category >= 0 && gun.category < 12) {
        DeathTableRow& dt = death_table[victim];
        if (dt.first_damage < 0.0f) dt.first_damage = clock_seconds;
        ++dt.hits[gun.category];
        dt.damage[gun.category] += applied;
        dt.last_category = gun.category;
        dt.last_gun = gun_row;
        dt.last_blast = blast_record != nullptr;
    }
    if (aa_trace_matches(unit_state[shooter].row.name)) {
        log.notef("  aa hit applied t=%.2f gun=%zu class=%d base=%.1f applied=%.1f health=%.1f "
            "armour=%.1f", static_cast<double>(clock_seconds), gun_row, priced_class,
            static_cast<double>(hit.hull_damage_base + hit.part_damage_base),
            static_cast<double>(applied), static_cast<double>(target.health),
            static_cast<double>(target.armour));
    }
    target.row.hits_taken += 1;
    target.row.damage_taken += applied;
    target.row.health = target.health;
    unit_state[shooter].row.hits_dealt += 1;
    unit_state[shooter].row.damage_dealt += applied;
    {
        auto& per_gun = hits_by_gun[gun_row];
        ++per_gun.first;
        per_gun.second += applied;
    }
    summary.damage_total += applied;
    if (summary.first_hit_seconds < 0.0f) summary.first_hit_seconds = clock_seconds;

    // 0077CE60, step 7 of 009239A0: the attribution block on the victim.
    const bool attribute = !kKillCreditDamageGateBound
        || (hit.hull_damage_base + hit.part_damage_base) > 0.0f;   // 0077CEB7 COMISS / JBE
    if (!attribute) ++summary_zero_damage_attributions_skipped;
    bsp::KillAttributionSource source;
    source.ordnance_kind = gun.category;
    source.ordnance_category = bsp::kill_credit_ordnance_category_00779a00(gun.category);
    source.owner_side = units.unit_side_0054(shooter);
    source.origin_slot = units.unit_side_0054(shooter);
    source.owner_id = static_cast<int>(shooter) + 1;
    source.has_owner_unit = true;
    bsp::KillAttributionAttacker attacker;
    attacker.unit_class = units.unit_class_id(shooter);
    attacker.owning_slot = units.unit_side_0054(shooter);
    attacker.type_id = unit_state[shooter].row.type_id;
    attacker.is_ship = true;
    bsp::KillAttributionKamikaze kamikaze;
    if (attribute) {
        target.attribution = bsp::kill_attribution_0077ce60(target.attribution, true, source,
            attacker, kamikaze, true);
        target.last_attacker = shooter + 1;
    }
    ++summary.attributions;
    done("Hit::attribution_0077ce60", 0x0077ce60u);

    bsp::UnitHealth health;
    health.current_health = target.health;
    health.max_health = target.max_health;
    if (bsp::unit_is_dead(health)) kill_unit(victim);
}

// 0084BC60 step 7, read from the listing at 0084BE25..0084BEE3.
//
//   MOV EAX,[ESI+8]            ; the projectile's weapon class descriptor
//   CMP byte ptr [EAX+6Ch],0   ; the "has blast" flag
//   JZ  0084BEE8               ; no Blast table, no burst
//   FLD  float ptr [EAX+0B8h]  ; BlastDamageMax
//   FLD  float ptr [EAX+0B4h]  ; BlastDamageMin
//   CALL 00BD2F10              ; a uniform draw between them
//   FSTP float ptr [ESP+84h]   ; -> the &damage the 0084BAD0 contract takes
//   LEA  EDX,[EAX+70h]         ; -> &radius, BlastRange, one float, not a draw
//   ... hitPos - direction * [00D7A270]   ; the burst centre, 00D7A270 = 0.05
//   CALL 0084BAD0
//
// So a torpedo's warhead is its Blast sub-table, not its DamageMin/DamageMax.
// For bullet class 69 (the Kate's "17.7 Type 91 Mod3 airplane torpedo") this
// installation's arcade table gives BlastDamageMin = BlastDamageMax = 1200 and
// BlastRange = 50, against a DamageMin/DamageMax of 50 -- and the Lexington's
// Armour is 50, so the contact damage is exactly (50 - 50) = 0 and the whole
// warhead is in the burst. docs/TORPEDO_WARHEAD.md.
//
// 0084BAD0 gathers one record per collision node inside the sphere (00904470
// -> 0098C630) and queues each with +28h = damage and +24h = radius. Two
// stand-ins here, both labelled: the gather is over this host's hull boxes
// rather than the image's shape tree, and the per-record distance that feeds
// 004705C0's falloff has no read producer in the image at all -- the array at
// +3Ch is docs/EXPLOSION_RADIAL_DAMAGE.md's labelled gap. The falloff
// arithmetic is 004705C0's; the distance handed to it is this host's.
void GameGunneryHost::Impl::apply_impact_blast(std::size_t shooter,
    std::size_t gun_row, const float point[3], const float direction[3]) {
    if (gun_row >= guns.size() || shooter >= unit_state.size()) return;
    const GameGunRow& gun = guns[gun_row];
    const GameBulletClassRow* weapon = bullet(round_bullet_class >= 0 ? round_bullet_class : gun.bullet_class);
    if (weapon == nullptr || weapon->blast_range <= 0.0f) return;

    const float damage = draw(Draw::blast_damage, gun_row, 0, weapon->blast_damage_min,
        weapon->blast_damage_max);
    // The image backs the burst centre off along buffer+10h, which 0084BF00
    // writes as the **unit** direction of the swept segment (delta scaled by
    // the reciprocal of BSP_Vector3f_Length; docs/PROJECTILE_IMPACT.md's field
    // table calls it "the segment direction, normalised"). The caller here
    // hands over the shot's velocity, so it must be normalised first: at a
    // shell's 700 m/s the raw vector would put the burst 35 m back down the
    // flight path instead of 5 cm out of the surface.
    float centre[3] = {point[0], point[1], point[2]};
    const float speed = std::sqrt(direction[0] * direction[0]
        + direction[1] * direction[1] + direction[2] * direction[2]);
    if (speed > 0.0f) {
        for (int i = 0; i < 3; ++i) {
            centre[i] -= (direction[i] / speed) * kBlastCentreBackOff;
        }
    }

    for (std::size_t i = 0; i < unit_state.size(); ++i) {
        if (i == shooter) continue;  // 0084BBF9 skips the burst's own source
        UnitState& state = unit_state[i];
        if (state.dead) continue;

        // The distance from the burst centre to the unit's hull box, in the
        // hull's own frame: the same slab SegmentBinding::shape_trace_segment
        // sweeps, so a round that struck the hull bursts at distance zero.
        float right[3], up[3], forward[3], origin[3];
        unit_pose(i, right, up, forward, origin);
        const float extents[3] = {state.hull_width * 0.5f, state.hull_height * 0.5f,
            state.hull_length * 0.5f};
        if (extents[0] <= 0.0f || extents[2] <= 0.0f) continue;
        const float rel[3] = {centre[0] - origin[0], centre[1] - origin[1],
            centre[2] - origin[2]};
        const float* axes[3] = {right, up, forward};
        float outside = 0.0f;
        for (int a = 0; a < 3; ++a) {
            const float excess = std::fabs(dot3(rel, axes[a])) - extents[a];
            if (excess > 0.0f) outside += excess * excess;
        }
        const float distance = std::sqrt(outside);
        if (distance > weapon->blast_range) continue;

        bsp::HitPartEntry entry;
        entry.kind = 0;
        entry.part_index = 0;
        entry.distance = distance;
        bsp::HitRecord blast;
        blast.hull_damage_base = 0.0f;  // the burst's damage is +28h, see above
        blast.part_damage_base = damage;
        blast.falloff_range = weapon->blast_range;
        blast.ignore_falloff = false;
        blast.armour_selector = 0.0f;
        blast.hull_segment = kDirectHitHullSegment;
        blast.weapon_scale = 1.0f;
        blast.owner_modifier = 1.0f;
        blast.part_hits = &entry;
        blast.part_hit_count = 1;

        const float before = state.health;
        apply_hit(shooter, gun_row, i, point, direction, &blast);
        log.notef("  impact blast bullet=%d on %s dist=%.1f base=%.1f range=%.1f "
            "armour=%.1f took=%.1f health=%.1f",
            gun.bullet_class,
            unit_name_or_index(i + 1).c_str(), static_cast<double>(distance),
            static_cast<double>(damage),
            static_cast<double>(weapon->blast_range),
            static_cast<double>(state.armour),
            static_cast<double>(before - unit_state[i].health),
            static_cast<double>(unit_state[i].health));
    }
    done("Projectile::blast_radial_damage_0084bad0", 0x0084bad0u);
}

void GameGunneryHost::Impl::kill_unit(std::size_t victim) {
    UnitState& target = unit_state[victim];
    if (target.dead) return;
    target.dead = true;
    target.row.sunk = true;
    target.row.sunk_seconds = clock_seconds;
    target.enabled = false;
    target.row.pass_enabled = false;
    ++summary.deaths;
    if (death_table_enabled()) {
        const DeathTableRow dt = death_table.count(victim) != 0 ? death_table[victim]
                                                                : DeathTableRow{};
        float at[3];
        unit_aim_point(victim, at);
        std::string killer = "-";
        float killer_range = -1.0f;
        if (target.last_attacker != 0) {
            float kp[3];
            unit_aim_point(target.last_attacker - 1, kp);
            killer = unit_state[target.last_attacker - 1].row.name;
            killer_range = std::sqrt((at[0] - kp[0]) * (at[0] - kp[0])
                + (at[1] - kp[1]) * (at[1] - kp[1]) + (at[2] - kp[2]) * (at[2] - kp[2]));
        }
        std::string nearest = "-";
        float nearest_range = -1.0f;
        for (std::size_t u = 0; u < unit_state.size(); ++u) {
            if (u == victim || unit_state[u].dead || unit_state[u].row.side == target.row.side)
                continue;
            if (!units.unit_is_kind_of(u, bsp::kUnitGunneryKindShipBase)) continue;
            float up[3];
            unit_aim_point(u, up);
            const float dx = at[0] - up[0], dz = at[2] - up[2];
            const float d = std::sqrt(dx * dx + dz * dz);
            if (nearest_range < 0.0f || d < nearest_range) { nearest_range = d; nearest = unit_state[u].row.name; }
        }
        log.notef("death row: victim=%s t=%.2f alt=%.0f first_damage=%.2f killer=%s "
            "killer_gun=%zu killer_cat=%d killer_blast=%d killer_range=%.0f nearest_ship=%s "
            "nearest_horizontal=%.0f hits c0=%d c1=%d c5=%d c6=%d dmg c0=%.0f c1=%.0f c5=%.0f "
            "c6=%.0f", target.row.name.c_str(), static_cast<double>(clock_seconds),
            static_cast<double>(at[1]), static_cast<double>(dt.first_damage), killer.c_str(),
            dt.last_gun, dt.last_category, dt.last_blast ? 1 : 0,
            static_cast<double>(killer_range), nearest.c_str(),
            static_cast<double>(nearest_range), dt.hits[0], dt.hits[1], dt.hits[5], dt.hits[6],
            static_cast<double>(dt.damage[0]), static_cast<double>(dt.damage[1]),
            static_cast<double>(dt.damage[5]), static_cast<double>(dt.damage[6]));
    }
    done("Death::entity_kill_00926d90", 0x00926d90u);
    record("Death::unit_sink_008110f0", 0x008110f0u);

    if (target.last_attacker != 0) {
        target.row.killed_by = unit_state[target.last_attacker - 1].row.name;
        ++unit_state[target.last_attacker - 1].row.kill_credits;
    }

    struct KillBinding final : bsp::KillCreditHost {
        explicit KillBinding(Impl& owner_in) : owner(owner_in) {}
        int session_mode() override { return 0; }
        bool skip_friendly_losses() override { return false; }
        int local_player_side() override { return -1; }
        bool root_entity_already_scored() override { return false; }
        void add_loss(int, bool, const std::string&) override { ++losses; }
        void add_type_kill(int, std::size_t, int) override { ++type_kills; }
        int& kill_tree_leaf(int, std::size_t, const bsp::ScoringKillKey&) override {
            return leaf;
        }
        int add_named_counter(int, const std::string&) override { return ++counter; }
        int award_threshold(const std::string&) override {
            owner.record("KillCredit::award_threshold_0050fc30", 0x0050fc30u);
            return 0;
        }
        void grant_award(int, const std::string&, int) override { ++awards; }
        void update_kill_list(int, std::uint32_t, int, int, bool) override { ++entries; }

        Impl& owner;
        int leaf{0};
        int counter{0};
        int losses{0};
        int type_kills{0};
        int awards{0};
        int entries{0};
    };

    KillBinding binding(*this);
    bsp::UnitKillInputs in;
    in.victim_name = target.row.name;
    in.victim_type_id = target.row.type_id;
    in.victim_owning_slot = target.row.side;
    in.victim_is_root = true;
    in.has_attacker = target.last_attacker != 0;
    in.attacker_is_ship_base = true;
    in.ordnance_kind = target.attribution.ordnance_kind;
    in.sole_attacker = target.attribution.sole_attacker;
    if (target.last_attacker != 0) {
        in.attacker_type_id = unit_state[target.last_attacker - 1].row.type_id;
    }
    in.attribution.attacker_side = target.attribution.attacker_side;
    in.attribution.credited_slot = target.attribution.credited_slot;
    in.attribution.originating_slot = target.attribution.origin_slot;
    in.attribution.attacker_class = target.attribution.attacker_class;
    in.attribution.victim_class = units.unit_class_id(victim);
    in.attribution.victim_side = target.row.side;
    bsp::kill_credit_record_unit_kill_0091bda0(binding, in);
    ++summary.kill_credits;
    done("KillCredit::record_unit_kill_0091bda0", 0x0091bda0u);
}

// ---------------------------------------------------------------------------
// GameGunneryHost
// ---------------------------------------------------------------------------

GameGunneryHost::GameGunneryHost(GameHostLog& log, GameUnitsHost& units,
    GameMissionLuaHost& lua)
    : impl_(std::make_unique<Impl>(log, units, lua)) {}

GameGunneryHost::~GameGunneryHost() = default;

void GameGunneryHost::set_ship_ai(GameShipAiHost* ai) noexcept {
    impl_->ship_ai = ai;
    // Packet cc8_ship_ai_firepower_inputs: the reverse edge. 0095EB40 reads the
    // tables 00956C20 built (unit+394h, +430h, +494h and the category lists),
    // and this host is the only thing in the process that runs 00956C20.
    if (ai != nullptr) ai->bind_gunnery(this);
}

void GameGunneryHost::Impl::refresh_build_summary() {
    summary.guns = guns.size();
    summary.device_rows = devices.size();
    summary.bullet_rows = bullets.size();
    // A count over the current rows, not an accumulator, so it is recomputed
    // rather than added to when a later batch registers.
    summary.units_with_guns = 0;
    for (const UnitState& state : unit_state) {
        if (state.row.guns > 0) ++summary.units_with_guns;
    }
    built_units = units.count();
}

// Packet cc9_player_gun_seat: the group 1/2 arm of 00959C20 (00959C91..
// 00959F6D) for one unit. docs/PLAYER_GUN_SEAT.md.
void GameGunneryHost::Impl::apply_gun_aim_message(std::size_t unit,
                                                  const GunAimMessage79& m) {
    ++seat_messages;
    if (m.group != 1 && m.group != 2) {
        record("PlayerGunSeat::message_other_group", 0x00959c20u);
        return;
    }
    if (unit >= unit_state.size() || unit_state[unit].dead) return;
    // 00954A10's two angles: 00521370 on the forward row (pitch = asin of y,
    // clamped to [-1, 1]; yaw = atan2(x, z)), +30h clamped to +/-1.57
    // (00D1A640/00D1A638), then 004B4D80 rebuilds the direction.
    const float fy = std::max(-1.0f, std::min(1.0f, m.forward[1]));
    float pitch = std::asin(fy);
    const float yaw = std::atan2(m.forward[0], m.forward[2]);
    pitch = std::max(-1.57f, std::min(1.57f, pitch));
    const float dir[3] = {std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                          std::cos(yaw) * std::cos(pitch)};
    // 00957D79..00957DA0: the 1000-unit segment from the camera through the
    // spatial index would aim at what it hits; not modelled, so every gun
    // takes the range-sphere point (labelled).
    record("PlayerGunSeat::segment_query", 0x00957da0u);
    if (m.has_target_36) record("PlayerGunSeat::target_intercept", 0x00957ca6u);
    std::int32_t role2_holder = 8;              // [unit+1B4h]
    if (!units.unit_current_role_slot(unit, 2, role2_holder)) role2_holder = 8;
    float right[3], up[3], forward[3], origin[3];
    unit_pose(unit, right, up, forward, origin);
    UnitState& state = unit_state[unit];
    for (std::size_t g = 0; g < guns.size(); ++g) {
        GameGunRow& gun = guns[g];
        if (gun.unit_index != unit) continue;
        // 00954210(kind 1 or 2): operational (00729F10; unit+720h and gun
        // +3B8h/+5Dh are never set here) and Function 1, 5 or 6.
        if (!(gun.category == 1 || gun.category == 5 || gun.category == 6)) continue;
        float muzzle[3];
        gun_muzzle_point(gun, state, right, up, forward, origin, muzzle);
        // 00957DE2..009580AB: the camera ray's point at the gun's bullet range.
        const float rel[3] = {muzzle[0] - m.camera[0], muzzle[1] - m.camera[1],
                              muzzle[2] - m.camera[2]};
        const float t = dot3(rel, dir);
        const float c[3] = {m.camera[0] + t * dir[0], m.camera[1] + t * dir[1],
                            m.camera[2] + t * dir[2]};
        const float off[3] = {muzzle[0] - c[0], muzzle[1] - c[1], muzzle[2] - c[2]};
        const float r2 = gun.max_range * gun.max_range;
        float s2 = r2 - dot3(off, off);
        const float floor2 = static_cast<float>(r2 / 9.0);      // 00CF0AB8
        if (s2 < floor2) s2 = floor2;
        const float s = std::sqrt(s2);
        float aim[3] = {c[0] + s * dir[0], c[1] + s * dir[1], c[2] + s * dir[2]};
        if (m.camera[1] > 0.0f && aim[1] < 0.0f) {                // 00958060: the sea
            float den = m.camera[1] - aim[1];
            if (std::fabs(den) < 1.0f) den = den < 0.0f ? -1.0f : 1.0f;
            const float k = m.camera[1] / den;
            aim[0] = m.camera[0] + (aim[0] - m.camera[0]) * k;
            aim[1] = m.camera[1] + (aim[1] - m.camera[1]) * k;
            aim[2] = m.camera[2] + (aim[2] - m.camera[2]) * k;
        }
        // 00955830: the direction from the mount as the gun's own angle pair,
        // in this host's hull-relative convention (008FDAF0's).
        const float d[3] = {aim[0] - muzzle[0], aim[1] - muzzle[1], aim[2] - muzzle[2]};
        const float len = length3(d);
        if (len <= 0.0f) continue;
        const float u[3] = {d[0] / len, d[1] / len, d[2] / len};
        const float horz = kGunHorzSign * std::atan2(dot3(u, right), dot3(u, forward));
        const float vert = std::asin(std::max(-1.0f, std::min(1.0f, dot3(u, up))));
        const bsp::GunPlatformArcs arcs{gun.arcs.data(), gun.arcs.size()};
        const bool in_window = bsp::gun_fire_allowed_007f60a0(arcs, horz, vert);   // 00959D72
        const bool seat_ai = slot_ai_held_00927f10(gun.seat_1ac);   // 00521E70(gun, 0)
        if (!in_window) {
            if (!seat_ai) {                                        // 00959D8E: 00729F70
                gun.seat_1ac = 8;
                gun.seat_trigger = false;
                gun.target_unit = 0;
                gun.target_name.clear();
                ++gun.seat_returns;
                ++seat_returns;
            }
            continue;
        }
        if (seat_ai && role2_holder != 8) {                        // 00959DA4..00959DB7
            gun.seat_1ac = role2_holder;
            ++gun.seat_handovers;
            ++seat_handovers;
            // 00959DB9..00959DEB, on the hand-over only: Function 6 calls
            // 0084C500(0) (not read), Function 1 drops its trigger.
            if (gun.category == 6) record("PlayerGunSeat::flak_0084c500", 0x00959dccu);
            if (gun.category == 1) gun.seat_trigger = false;
        }
        gun.seat_horz = horz;                                      // 00959E01
        gun.seat_vert = vert;
        // 00959E46..00959F5C: the trigger. 99h held fires a kind-1 mount, or
        // any mount whose angles are within 3 degrees (00D1A8A0) of the pair.
        const float band = 0.0523598776f;
        const bool within_band = std::fabs(bsp::wrapped_angle_subtract_00438b10(gun.angles.horz, horz))
                < band
            && std::fabs(bsp::wrapped_angle_subtract_00438b10(gun.angles.vert, vert)) < band;
        gun.seat_trigger = m.held_34 && (gun.category == 1 || within_band);
    }
    done("PlayerGunSeat::apply_message_00959c20", 0x00959c20u);
}

void GameGunneryHost::apply_gun_aim_message_00959c20(std::size_t unit_index,
                                                     const GunAimMessage79& message) {
    impl_->apply_gun_aim_message(unit_index, message);
}

void GameGunneryHost::attach_00864bd0() {
    Impl& host = *impl_;
    host.build_rank_table();
    host.build_guns(0);
    host.attach_passes(0);
    host.refresh_build_summary();
    host.log.notef("gunnery: %zu unit(s) carry %zu gun(s) from %zu authored device class "
        "row(s) and %zu bullet class row(s); the weapon director think time is %.3f s "
        "(Globals.WeaponSystems.WeaponDirectorThinkTime, 0087e16b)",
        host.summary.units_with_guns, host.summary.guns, host.summary.device_rows,
        host.summary.bullet_rows, static_cast<double>(host.think_time));
}

void GameGunneryHost::register_new_units_00864bd0() {
    Impl& host = *impl_;
    const std::size_t first = host.built_units;
    const std::size_t count = host.units.count();
    if (count <= first) return;
    const std::size_t guns_before = host.guns.size();
    // The rank table is built from the compiled-in preference lists alone and
    // carries no per-unit state, so it is not rebuilt here.
    host.build_guns(first);
    host.attach_passes(first);
    host.refresh_build_summary();
    host.log.notef("gunnery: spawn batch registered unit(s) %zu..%zu on the existing host, "
        "%zu new gun(s) (%zu total); the summary, hit records and in-flight rounds of the "
        "%zu unit(s) already built are untouched",
        first, count - 1, host.guns.size() - guns_before, host.guns.size(), first);
}

void GameGunneryHost::fixed_step(float step_seconds) {
    Impl& host = *impl_;
    if (host.guns.empty()) return;
    host.clock_seconds += step_seconds;
    ++host.step_index;
    // 008073C0 runs once over every side before the per-unit pass, because it
    // is O(sides * observers * targets); running it inside the contact sweep
    // would repeat the whole pass once per firing unit.
    host.step_recon_sensor_pass_008073c0(step_seconds);
    for (std::size_t i = 0; i < host.unit_state.size(); ++i) {
        host.run_gunnery_pass(i, step_seconds);
    }
    host.run_gun_aim_and_fire(step_seconds);
    host.run_projectiles(step_seconds);
    // The rows are complete for this step here, after every per-unit pass and
    // the aim, fire and projectile passes have run. 00A08460 BSP_Ai_TargetWeight
    // reads the target's hit points and the attacker's barrels, and the AI
    // coordinator holds neither host, so the values are published into the
    // process-wide table it reads. docs/AI_TARGET_WEIGHT_TERMS.md term 2.
    host.publish_ai_weapon_facts();
}

void GameGunneryHost::Impl::publish_ai_weapon_facts() {
    GameAiWeaponFacts& facts = game_ai_weapon_facts();
    facts.reset();
    // target+48h, the hit points 00A08593 reads and 00A09737's epilogue divides
    // the accumulated damage by. The class maximum is what makes that a ratio.
    for (const UnitState& state : unit_state) {
        GameAiWeaponFacts::Unit& row = facts.row_for_write(state.row.unit_index);
        row.hit_points = state.row.max_health;
        // target+4Ch, read at 00A085A8. Nothing in this process produces a
        // capture state, so it stays zero and the model's capture accumulator
        // contributes nothing.
        row.capture_state = 0.0f;
    }
    // 00A095E3 walks the attacker's subsystems at +94h/+98h and their 48h-stride
    // barrel entries at +74h/+78h. This process has one gun row per gun and a
    // barrel count on it, so the barrels are flattened into one list per unit.
    for (const GameGunRow& gun : guns) {
        // A gun whose bullet class never resolved is not a weapon and must not
        // reach 00A08460's barrel walk. The case measured on IJN01 is the
        // CATAPULT: it is gunnery category 0Bh, one of the twelve, so a gun row
        // IS built for it here, but of the 416 device classes in this
        // installation the 20 CATAPULT rows are the only ones that author no
        // `Bullet` block, so gun.bullet_class stays -1 and the sub-type stays 0.
        // The image says the same thing from the other side: the authored
        // preference row for category 0Bh at 00E0A374 is EMPTY, so a catapult
        // targets nothing. Measured before this skip: 21020 barrel lookups on
        // Cruiser and 21020 on BattleShip, the two member classes in IJN01 that
        // mount one.
        //
        // This changes no weight. A sub-type 0 barrel answered accuracy 0 and
        // 00A094F5 already skipped it, so the totals are identical; what goes
        // away is a phantom barrel in the row and in the census.
        if (gun.bullet_sub_type == 0) {
            ++summary.ai_barrels_unresolved_skipped;
            continue;
        }
        GameAiWeaponFacts::Unit& row = facts.row_for_write(gun.unit_index);
        GameAiWeaponFacts::Barrel barrel;
        barrel.reload = gun.reload_time;
        // 0072AB80 BSP_GunClass_MuzzleCount at 00A09501 is the shots argument,
        // and BSP_Gun_SetupFromDescriptor stores its answer to gun+448h at
        // 0072E71A, which is exactly the field carried here as barrel_num. So
        // this one IS available; the earlier reading that it was not came from
        // mistaking the muzzle count for a barrel count.
        barrel.shots = gun.barrel_num > 0 ? gun.barrel_num : 1;
        // 009FE270 at 00A094E6 is not a stored accuracy: it is a lookup by
        // (bullet sub-type, target class group) into the AI mode tuning record,
        // so what the row owes is the selector and the lookup runs per target.
        // docs/AI_TARGET_WEIGHT_TERMS.md.
        barrel.bullet_sub_type = gun.bullet_sub_type;
        {
            // Resolvable is a property of the sub-type alone: every group of a
            // resolvable sub-type answers either an offset or a legitimate
            // zero. Asked with one group here purely to read the flag back.
            bool resolved = false;
            (void)bsp::ai_bullet_type_accuracy_offset_009fe270(
                barrel.bullet_sub_type, bsp::AiAccuracyTargetGroup::BigShip,
                resolved);
            barrel.accuracy_resolved = resolved;
        }
        row.barrels.push_back(barrel);
    }
    // A row is complete when every input 00A08460 reads is published. The
    // reload and the shot count always are; the accuracy is now reachable for
    // every sub-type but Rocket (12h), whose small/big split 009FE4F1 makes
    // through unread target-state predicates. A unit carrying any rocket barrel
    // therefore stays on the stand-in rather than scoring that barrel at zero.
    // Vacuously true for a unit with no guns: the barrels are the ATTACKER's
    // side of 00A08460 and the hit points above are the TARGET's, and the gate
    // asks both rows, so requiring barrels here would refuse every gunless
    // target and the model would never run on the static installations that
    // make up most of IJN01.
    for (GameAiWeaponFacts::Unit& row : facts.units) {
        // What the row itself now owes IS published: the reload, the shot count
        // and the bullet sub-type, for every sub-type but Rocket.
        bool row_inputs_published = true;
        for (const GameAiWeaponFacts::Barrel& barrel : row.barrels) {
            if (!barrel.accuracy_resolved) row_inputs_published = false;
        }
        // ... and the model is still NOT switched on, for a reason measured
        // rather than assumed. With `row_inputs_published` assigned straight to
        // the flag, IJN01 answers a zero weight for 460600 of its 465500
        // candidates - exactly its `fort_targets` count - so `scored` falls to
        // 4900 and `attackmove` 2250 and `settarget` 141 both go to 0 with all
        // 2450 served members taking the fallback moveto. The AI stops
        // attacking altogether. That is a regression against the stand-in and
        // it is 00A08460's own coverage, not this row's: its
        // attacker-is-type-0Fh branch 00A0861F..00A09222 is unprojected and
        // AiWeightModelBinding::entity_is_type and entity_kind are stubs.
        //
        // A first reading blamed the target hit points and was REFUTED by a
        // second run: gating on `hit_points > 0` left complete_rows at 321 and
        // changed no other number, so every row has real health.
        //
        // The collapse that held this at false is now traced and fixed: it was
        // ai_target_weights.cpp handing the barrel walk a null subsystem, so
        // barrel_count answered 0 and the loop never ran. Enabled here, and the
        // run that justifies it is in docs/AI_TARGET_WEIGHT_TERMS.md.
        row.inputs_complete = row_inputs_published;
    }
}

const std::vector<std::size_t>* GameGunneryHost::unit_category_guns(
    std::size_t unit_index, int category) const noexcept {
    if (unit_index >= impl_->unit_state.size()) return nullptr;
    if (category < 0 || category >= bsp::kUnitGunneryCategoryCount) return nullptr;
    return &impl_->unit_state[unit_index].category_guns[static_cast<std::size_t>(category)];
}

const GameBulletClassRow* GameGunneryHost::bullet_class_row(int id) const noexcept {
    return impl_->bullet(id);
}

const std::vector<GameGunRow>& GameGunneryHost::guns() const noexcept {
    return impl_->guns;
}

bool GameGunneryHost::unit_dead(std::size_t unit_index) const noexcept {
    if (unit_index >= impl_->unit_state.size()) return false;
    const Impl::UnitState& state = impl_->unit_state[unit_index];
    return state.dead || state.health <= 0.0f;
}

float GameGunneryHost::death_mode_draw_00bd2f10(int stream, std::size_t unit_index,
    float low, float high) {
    // Stream 1 is the shared generator, exactly as ship_ai_draw uses it; under the
    // measurement option both streams get their own labelled key (unit, 0).
    if (stream == 1) {
        return impl_->draw(Impl::Draw::death_mode, unit_index, 0, low, high);
    }
    if (Impl::rng_streams_enabled()) {
        return impl_->draw(Impl::Draw::death_delay, unit_index, 0, low, high);
    }
    // Stream 0 is a separate generator in the image (ECX = 0). SUBSTITUTION,
    // labelled: its seed and sequence are not reproduced, only its separation.
    static std::uint32_t stream0 = 0x2545F491u;
    stream0 = stream0 * 1664525u + 1013904223u;
    const float unit = static_cast<float>((stream0 >> 8) & 0xFFFFFFu)
        / static_cast<float>(0x1000000u);
    return low + (high - low) * unit;
}

void GameGunneryHost::fighter_aim_distortion_009fa7e0(std::size_t unit_index, float dt,
    bool owner_is_fighter, float out[2]) {
    static std::map<std::size_t, bsp::GunAimWander> wanders;
    auto it = wanders.find(unit_index);
    if (it == wanders.end()) {
        float draws[4];
        draws[0] = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, 0.1f, 1.0f);  // 00D7A2F0
        draws[1] = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, -1.0f, 1.0f);
        draws[2] = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, -1.0f, 1.0f);
        draws[3] = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, -1.0f, 1.0f);
        it = wanders.emplace(unit_index, bsp::GunAimWander{}).first;
        bsp::gun_aim_wander_init_009fa620(it->second, draws);
    }
    const float rx = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, -1.0f, 1.0f);
    const float ry = impl_->draw(Impl::Draw::aim_wander, unit_index, 0, -1.0f, 1.0f);
    bsp::gun_aim_wander_step_009fa7e0(it->second, dt, rx, ry);
    out[0] = it->second.out[0];
    out[1] = it->second.out[1];
    if (owner_is_fighter) {
        float div = 1.8f;   // Pilot/Dogfight/FighterAimMulVersusAI default
        if (impl_->lua.plane_globals_loaded()) {
            div = impl_->lua.plane_globals().pilot_dogfight_fighter_aim_mul_versus_ai;
        }
        if (div != 0.0f) {
            out[0] /= div;   // 009FCD4E FDIVP
            out[1] /= div;   // 009FCD56 FDIVR
        }
    }
}

float GameGunneryHost::min_fixed_gun_muzzle_speed_007c2610(std::size_t unit_index) const noexcept {
    float lowest = std::numeric_limits<float>::max();   // 00D7A248
    for (const GameGunRow& gun : impl_->guns) {
        if (gun.unit_index != unit_index || gun.category != 0) continue;
        // 007C264F-007C2657: keep the running minimum unless V0 exceeds it.
        if (!(gun.muzzle_speed > lowest)) lowest = gun.muzzle_speed;
    }
    return lowest;
}

void GameGunneryHost::kill_unit_00926d90(std::size_t unit_index, int cause) {
    if (unit_index >= impl_->unit_state.size()) return;
    // Cause 1 is the only one a caller passes (007CE3A7); the funnel is the
    // one a gunfire death takes, which is also cause 1 (0077D1A0 -> 00926C80).
    (void)cause;
    ++impl_->water_depth_kills;
    impl_->kill_unit(unit_index);
}

const std::vector<GameGunneryUnitRow>& GameGunneryHost::unit_rows() const noexcept {
    static std::vector<GameGunneryUnitRow> rows;
    rows.clear();
    rows.reserve(impl_->unit_state.size());
    for (const Impl::UnitState& state : impl_->unit_state) rows.push_back(state.row);
    return rows;
}

const GameGunnerySummary& GameGunneryHost::summary() const noexcept {
    return impl_->summary;
}

bool GameGunneryHost::release_ordnance_drop(std::size_t unit_index) {
    Impl& h = *impl_;
    // The unit's torpedo-capable gun rows. `swim_speed > 0` is the same test the
    // water crossing uses to decide that a round swims instead of dying at the
    // surface, so a row selected here is exactly a row whose round can reach the
    // swim model.
    const GameGunRow* chosen = nullptr;
    for (const GameGunRow& gun : h.guns) {
        if (gun.unit_index != unit_index) continue;
        if (gun.swim_speed <= 0.0f) continue;
        chosen = &gun;
        break;
    }
    if (chosen == nullptr) {
        ++h.summary.torpedo_drop_refusals;
        return false;
    }

    float right[3], up[3], forward[3], origin[3];
    h.unit_pose(unit_index, right, up, forward, origin);
    // 0092D730 over the unit's body axis and linear velocity, not the cached
    // GameUnitRow field the Impl's own unit_velocity reads: that field is zero
    // for a plane, which made the first drop a pure free fall.
    const float speed = h.units.unit_forward_speed_0092d730(unit_index);
    const float velocity[3] = {forward[0] * speed, forward[1] * speed,
        forward[2] * speed};

    GameProjectileRow shot;
    shot.gun_row = static_cast<std::size_t>(chosen - h.guns.data());
    shot.owner_unit = unit_index + 1;
    shot.owner_side = h.units.unit_side_0054(unit_index);
    shot.bullet_class = chosen->bullet_class;
    shot.alive = true;
    for (int i = 0; i < 3; ++i) shot.position[i] = origin[i];
    // SUBSTITUTION, labelled: the release geometry. The native mount node and
    // the platform's own release slot are unread, so the round leaves from the
    // plane's origin along its forward axis at the plane's own forward speed.
    // A drop inherits the aircraft's velocity; it is not given a muzzle speed,
    // which is why `projectile_launch_velocity_006e8430` is not used here.
    shot.flight.velocity = bsp::TickPoint3{velocity[0], velocity[1], velocity[2]};
    shot.flight.snapshot_current = bsp::TickPoint3{origin[0], origin[1], origin[2]};
    shot.flight.local_position = shot.flight.snapshot_current;
    shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
    shot.flight.class_disables_gravity = false;
    // Packet cc8_torpedo_aim_census: pin the ordered target and where it was at
    // the drop, before the round is filed. Nothing downstream can recover this:
    // the round carries no victim and the target keeps moving.
    {
        const std::size_t owner = shot.owner_unit - 1;
        if (owner < h.command_target_by_unit.size()) {
            shot.ordered_target = h.command_target_by_unit[owner];
        }
        if (shot.ordered_target != 0) {
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            h.units.unit_position_00fc(shot.ordered_target - 1, tx, ty, tz);
            shot.target_pos_release[0] = tx;
            shot.target_pos_release[1] = ty;
            shot.target_pos_release[2] = tz;
            // Packet cc8_torpedo_retire item 5. The crossing geometry at the
            // DROP, in the same convention as the closest-approach one above:
            // unit_heading_radians is the unit's vtable[50h] heading, the hull
            // POSE row 2, for the aircraft as well as for the ship. For an
            // aircraft it is the same number the torpedo aim census prints as
            // yaw_C6C - that census prints yaw_C6C and hull_1050 side by side
            // and they agree to four decimals - so the aircraft's run-in
            // heading and the target's course are commensurable here, which
            // they would NOT be against the ship-ai step heading.
            shot.drop_owner_heading =
                h.units.unit_heading_radians(shot.owner_unit - 1);
            shot.drop_target_heading =
                h.units.unit_heading_radians(shot.ordered_target - 1);
            shot.drop_crossing_angle =
                std::fabs(bsp::wrapped_angle_subtract_00438b10(
                    shot.drop_owner_heading, shot.drop_target_heading));
            // Packet cc8_torpedo_release item 1: the release range the aspect
            // gate governs. Horizontal and centre to centre, the same
            // convention as every other distance this census prints
            // (:3601), and the same X/Z difference 009D3519 takes.
            float ox = 0.0f, oy = 0.0f, oz = 0.0f;
            h.units.unit_position_00fc(owner, ox, oy, oz);
            const float rdx = tx - ox;
            const float rdz = tz - oz;
            shot.drop_ordered_range = std::sqrt(rdx * rdx + rdz * rdz);
        }
    }
    // TRACE, packet cc8_torpedo_swim item 1: the id on this round's own drop
    // line, carried on the row so the per-tick trace is keyed to it.
    shot.torpedo_trace_id = h.summary.torpedo_drops + 1;
    h.log.notef("  torpedo trace %llu spawn by %s at=(%.1f,%.2f,%.1f) "
        "vel=(%.1f,%.2f,%.1f) bullet=%d", shot.torpedo_trace_id,
        chosen->unit_name.c_str(), static_cast<double>(shot.position[0]),
        static_cast<double>(shot.position[1]),
        static_cast<double>(shot.position[2]),
        static_cast<double>(velocity[0]), static_cast<double>(velocity[1]),
        static_cast<double>(velocity[2]), chosen->bullet_class);
    h.shots.push_back(shot);
    ++h.summary.projectiles;
    ++h.summary.torpedo_drops;
    // Packet cc8_torpedo_breakoff: RESTORED, as half two of the spent-bomber
    // fix. A drop clears the owner's kind 2Bh bit, so approach+132h (== task+52Ah,
    // section 10.2) goes false on the next approach update.
    //
    // This was tried alone in 1e7c0f2f2 and reverted in c5235a9c6. The revert's
    // stated reason -- "009D3F60's entry chooser sends a task whose +52Ah is clear
    // straight to kDone" -- is WITHDRAWN by section 12: 009D3F60 has one call site
    // that cannot run after a drop (10.1), and the route that run took to `done`
    // is unexplained, not diagnosed. What the run did prove is that the clear
    // alone makes things worse, and section 12.4 gives the reason: the image
    // retires a spent bomber through 009D4C10, whose ordnance arm at 009D4C5C
    // (`CMP byte [ESI+52Ah],0` / `JNZ 009D4C1D`) is one of TWO halves. With
    // should_break_off still pinned false the clear had nothing to feed. It is
    // restored here only together with that binding in src/game_hosts_units.cpp.
    //
    // What the image consumes on a drop is still a HYPOTHESIS and stays one:
    // 009D34C5 refreshes approach+132h from 007B93F0(0) = 007B91C0(2Bh, 0), which
    // walks the controller's devices at ctl+974h, takes the one carrying kind 2Bh
    // and asks the ordnance object it returns `vtable[8](0)`. THAT body is unread.
    // The supporting evidence is a sibling predicate at 007B9426 testing a live
    // round count at ordnance+E0h, and the fact that 009D4030's goaway branch
    // reaches `done` only with the byte clear.
    //
    // The smallest faithful model: a drop consumes the unit's torpedo loadout, so
    // the kind bit clears. This host models no per-device round count, so it
    // cannot decrement one -- the count and its producer 006E3500 are unread here,
    // and "one torpedo per aircraft" is the assumption this makes explicit rather
    // than hides. USN01's Mavs release once each, which is consistent with it and
    // does not prove it. docs/TORPEDO_AFTER_THE_DROP.md sections 9, 12 and 13.
    {
        const std::size_t owner = shot.owner_unit - 1;
        const std::uint64_t mask = h.units.unit_ordnance(owner);
        const std::uint64_t torpedo_bit = std::uint64_t(1) << (0x2b - 0x08);
        if ((mask & torpedo_bit) != 0) {
            h.units.store_unit_ordnance(owner, mask & ~torpedo_bit);
            ++h.summary.torpedo_loadout_cleared;
        }
    }
    if (h.summary.torpedo_drops <= 4) {
        h.log.notef("gunnery: torpedo drop %llu by %s at %.0f m, speed %.1f m/s, "
            "bullet %d, swim %.1f m/s",
            h.summary.torpedo_drops, chosen->unit_name.c_str(),
            static_cast<double>(origin[1]), static_cast<double>(speed),
            chosen->bullet_class, static_cast<double>(chosen->swim_speed));
    }
    return true;
}

// Packet cc8_dive_glide, edited under the integrator's hunk arbitration of
// 2026-09-19. The bomb twin of release_ordnance_drop above: the same 0072F830
// spawn and the same substituted release geometry, with the kind 2Ah selection
// the dive bomber needs in place of the torpedo `swim_speed > 0` one. See the
// header for why the predicate was the whole defect.
bool GameGunneryHost::release_bomb_drop(std::size_t unit_index,
                                        const float predicted_impact[3],
                                        float release_fall_time) {
    Impl& h = *impl_;
    const GameGunRow* chosen = nullptr;
    for (const GameGunRow& gun : h.guns) {
        if (gun.unit_index != unit_index) continue;
        if (!bsp::ordnance_has_general_bomb_2ah(gun.ordnance)) continue;
        chosen = &gun;
        break;
    }
    if (chosen == nullptr) {
        ++h.summary.bomb_drop_refusals;
        return false;
    }

    float right[3], up[3], forward[3], origin[3];
    h.unit_pose(unit_index, right, up, forward, origin);
    // Same substitution as the torpedo drop, and for the same reason: the
    // native mount node and the platform's release slot are unread, so the
    // round leaves from the plane's origin along its forward axis at the
    // plane's own forward speed. A bomb inherits the aircraft's velocity - it
    // is not given a muzzle speed - which is exactly the assumption 007BCC80's
    // fall time makes when it advances the aircraft by its own velocity.
    const float speed = h.units.unit_forward_speed_0092d730(unit_index);
    const float velocity[3] = {forward[0] * speed, forward[1] * speed,
        forward[2] * speed};

    GameProjectileRow shot;
    shot.gun_row = static_cast<std::size_t>(chosen - h.guns.data());
    shot.owner_unit = unit_index + 1;
    shot.owner_side = h.units.unit_side_0054(unit_index);
    shot.bullet_class = chosen->bullet_class;
    shot.alive = true;
    shot.is_bomb = true;
    // Packet cc8_dive_aim item 2: the tf in force at THIS tick, not the one the
    // aimdive summary prints after the dive has ended.
    shot.release_fall_time = release_fall_time;
    for (int i = 0; i < 3; ++i) {
        shot.position[i] = origin[i];
        shot.predicted_impact[i] = predicted_impact[i];
    }
    shot.flight.velocity = bsp::TickPoint3{velocity[0], velocity[1], velocity[2]};
    shot.flight.snapshot_current = bsp::TickPoint3{origin[0], origin[1], origin[2]};
    shot.flight.local_position = shot.flight.snapshot_current;
    shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
    shot.flight.class_disables_gravity = false;
    {
        const std::size_t owner = shot.owner_unit - 1;
        if (owner < h.command_target_by_unit.size()) {
            shot.ordered_target = h.command_target_by_unit[owner];
        }
        if (shot.ordered_target != 0) {
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            h.units.unit_position_00fc(shot.ordered_target - 1, tx, ty, tz);
            shot.target_pos_release[0] = tx;
            shot.target_pos_release[1] = ty;
            shot.target_pos_release[2] = tz;
            // Packet cc8_dive_aim item 2. A ship's velocity is its forward
            // speed along its hull axis, so these two ARE the velocity at the
            // drop; recording them rather than a derived vector keeps the
            // census on accessors that are already proven (0092D730 and the
            // vtable[50h] hull heading the torpedo drop above uses).
            shot.target_speed_release =
                h.units.unit_forward_speed_0092d730(shot.ordered_target - 1);
            shot.target_heading_release =
                h.units.unit_heading_radians(shot.ordered_target - 1);
        }
    }
    h.shots.push_back(shot);
    ++h.summary.projectiles;
    ++h.summary.bomb_drops;
    // NOT DONE, deliberately: no kind 2Ah twin of the torpedo drop's 2Bh clear.
    // The torpedo side can clear its bit because one drop is its whole loadout;
    // a dive bomber's is a salvo of up to two per pass out of a stock this host
    // does not model per device (006E3500 is unread, the same hole
    // kDiveBombCarriedRoundsSubstitute names). Clearing 2Ah here would make
    // 009C7AFE's HasGeneralBombOrdnance false after the FIRST bomb and take the
    // aimdive's second release away with it - and the torpedo side's own
    // history is the warning: the bare clear was tried in 1e7c0f2f2 and
    // reverted in c5235a9c6. The unit-side `dive_bomb_rounds_remaining` is what
    // counts the stock down today.
    if (h.summary.bomb_drops <= 6) {
        h.log.notef("gunnery: bomb drop %llu by %s at %.0f m, speed %.1f m/s, "
            "bullet %d | predicted impact %.0f %.0f %.0f",
            h.summary.bomb_drops, chosen->unit_name.c_str(),
            static_cast<double>(origin[1]), static_cast<double>(speed),
            chosen->bullet_class,
            static_cast<double>(predicted_impact[0]),
            static_cast<double>(predicted_impact[1]),
            static_cast<double>(predicted_impact[2]));
    }
    return true;
}

const bsp::ReconSensorPassState& GameGunneryHost::recon_sensor_pass_state() const noexcept {
    return impl_->recon_pass;
}

void GameGunneryHost::log_sample(unsigned long long step_index,
    unsigned long long interval) {
    Impl& host = *impl_;
    if (interval == 0 || step_index % interval != 0) return;
    if (host.guns.empty()) return;
    host.log.notef("  gunnery step %llu t=%.2f assigns=%llu shots=%llu shots_in_flight=%zu "
        "hits=%llu damage=%.1f deaths=%llu", step_index,
        static_cast<double>(host.clock_seconds), host.summary.assigns, host.summary.shots,
        host.shots.size(), host.summary.dispatched_hits,
        static_cast<double>(host.summary.damage_total), host.summary.deaths);
}

void GameGunneryHost::report() {
    Impl& host = *impl_;
    const GameGunnerySummary& s = host.summary;
    host.log.notef("summary mission gunnery units=%zu guns=%zu passes=%zu ticks=%llu "
        "bodies=%llu bridge=%llu sweeps=%llu candidates=%llu rejected=%llu "
        "assignment_passes=%llu gun_evaluations=%llu slot_rejects=%llu assigns=%llu "
        "clears=%llu",
        s.units_with_guns, s.guns, s.passes_attached, s.pass_ticks, s.pass_bodies,
        s.bridge_applies, s.recon_sweeps, s.candidates, s.candidates_rejected,
        s.assignment_passes, s.gun_evaluations, s.gun_slot_rejects, s.assigns,
        s.clears);
    {
        // docs/RECON_SLOT_LISTS.md rule (c). The pass's own counters over the
        // run, then the published level per observing side, then the contact
        // drops rule (c) is responsible for.
        const bsp::ReconSensorPassState& pass = host.recon_pass;
        {
            const double b = host.recon_triple_builds ? static_cast<double>(host.recon_triple_builds) : 1.0;
            host.log.notef("summary mission recon triples builds=%llu mean own=%.1f enemy=%.1f "
                "neutral=%.1f unknown=%.1f union=%.1f bound=%d (008073C0 / 004C3CB0, packet "
                "cc9_recon_team_lists)", host.recon_triple_builds, host.recon_triple_sum[0] / b,
                host.recon_triple_sum[1] / b, host.recon_triple_sum[2] / b,
                host.recon_triple_sum[3] / b, host.recon_triple_sum[4] / b,
                kReconTeamListsBound ? 1 : 0);
            for (const auto& t : host.recon_triples) {
                host.log.notef("  recon triples final side %d own=%zu enemy=%zu neutral=%zu "
                    "unknown=%zu union=%zu", t.side, t.lists[0].size(), t.lists[1].size(),
                    t.lists[2].size(), t.lists[3].size(), t.lists[4].size());
            }
        }
        host.log.notef("summary mission recon sensor_pass passes=%llu observers=%llu "
            "targets=%llu forced=%llu no_table=%llu blip=%llu identified=%llu none=%llu "
            "classes=%zu class_missing=%llu modifier_absent=%llu suppressed=%d "
            "contact_reject_level=%llu",
            pass.passes, pass.observers_admitted, pass.targets_tested,
            pass.targets_skipped_forced, pass.no_sensor_table, pass.detected_blip,
            pass.detected_identified, pass.detected_none, host.recon_classes.size(),
            host.recon_classes_missing, host.recon_modifier_absent,
            pass.suppressed_by_network_role ? 1 : 0, s.contact_reject_recon_level);
        for (const Impl::ReconSideCensus& row : host.recon_side_census) {
            host.log.notef("  recon side %d levels none=%llu blip=%llu identified=%llu",
                row.side, row.none_level, row.blip, row.identified);
        }
        for (const auto& entry : host.recon_classes) {
            // Which ReconClass[] record each id resolved to, and how many of
            // the 56 lists 008082A0 actually filled. An id whose record is
            // empty is the loader's own answer for a class outside 1..12.
            std::size_t filled = 0, rows = 0;
            for (std::size_t i = 0; i < bsp::kSensorListCount; ++i) {
                if (entry.second->rows[i].count != 0) {
                    ++filled;
                    rows += entry.second->rows[i].count;
                }
            }
            host.log.notef("  recon class %d lists=%zu entries=%zu", entry.first,
                filled, rows);
        }
    }
    host.log.notef("summary mission gunnery source arm=%llu recon=%llu "
        "arm_mean_reach=%.3f recon_mean_reach=%.3f arm_beyond_half=%llu "
        "recon_beyond_half=%llu",
        s.assigns_from_arm, s.assigns_from_recon,
        s.assigns_from_arm ? s.arm_reach_fraction_sum / double(s.assigns_from_arm) : 0.0,
        s.assigns_from_recon ? s.recon_reach_fraction_sum / double(s.assigns_from_recon) : 0.0,
        s.arm_assigns_beyond_half, s.recon_assigns_beyond_half);
    {
        // The ordnance inventory 007EEC50's AttackFeasibilityInputs need, per
        // unit, aggregated over that unit's guns exactly as the 007ED7E0 family
        // aggregates over the weapon controller's slots.
        // docs/ORDNANCE_KIND_IDENTITY.md.
        std::size_t torpedo = 0, general_bomb = 0, drop_kamikaze = 0, paratrooper = 0;
        std::map<std::size_t, int> per_unit;
        for (const GameGunRow& gun : host.guns) {
            int& bits = per_unit[gun.unit_index];
            if (bsp::ordnance_has_torpedo_2bh(gun.ordnance)) bits |= 1;
            if (bsp::ordnance_has_general_bomb_2ah(gun.ordnance)) bits |= 2;
            if (bsp::ordnance_has_drop_kamikaze_2fh(gun.ordnance)) bits |= 4;
            if (bsp::ordnance_has_paratrooper_31h(gun.ordnance)) bits |= 8;
        }
        for (const std::pair<const std::size_t, int>& row : per_unit) {
            if (row.second & 1) ++torpedo;
            if (row.second & 2) ++general_bomb;
            if (row.second & 4) ++drop_kamikaze;
            if (row.second & 8) ++paratrooper;
        }
        host.log.notef("summary mission gunnery ordnance units_with torpedo=%zu "
            "general_bomb=%zu drop_kamikaze=%zu paratrooper=%zu (of %zu units with guns)",
            torpedo, general_bomb, drop_kamikaze, paratrooper, per_unit.size());
    }
    host.log.notef("summary mission gunnery command_targets units_with=%zu "
        "(0071EBF0's answer, step 8.7's first arm; 0 means that arm never runs)",
        host.command_targets_resolved);
    host.log.notef("summary mission gunnery torpedo_ranges_derived=%llu "
        "swims_started=%llu snaps=%llu bullet_ranges_derived=%llu "
        "base_tick_timers_live=%llu expired=%llu",
        s.torpedo_ranges_derived, s.torpedo_swims_started,
        s.torpedo_heading_snaps, s.bullet_ranges_derived,
        s.gun_pending_timers_live, s.gun_pending_timers_expired);
    {
        // Packet cc8_torpedo_release_spawn: the funnel that says where a round
        // that could swim actually stops. Counted over gun rows, so a unit with
        // several tubes contributes one entry per tube.
        unsigned long long torpedo_guns = 0;
        for (const GameGunRow& row : host.guns) {
            if (row.swim_speed > 0.0f) ++torpedo_guns;
        }
        host.log.notef("summary mission gunnery torpedo_gate guns=%llu ticks=%llu "
            "targeted=%llu accepted=%llu settled=%llu window=%llu sent=%llu shots=%llu",
            torpedo_guns, s.torpedo_gun_ticks, s.torpedo_gun_targeted,
            s.torpedo_gun_accepted, s.torpedo_gun_settled, s.torpedo_gun_window,
            s.torpedo_gun_sent, s.torpedo_gun_shots);
        host.log.notef("summary mission gunnery torpedo_friendly scans=%llu holds=%llu",
            s.torpedo_friendly_scans, s.torpedo_friendly_holds);
        host.log.notef("summary mission gunnery player seat messages=%llu handovers=%llu "
            "returns=%llu held_ticks=%llu trigger_ticks=%llu bound=%d (00959C20/008FFA99, "
            "packet cc9_player_gun_seat)", host.seat_messages, host.seat_handovers,
            host.seat_returns, host.seat_held_ticks, host.seat_trigger_ticks,
            kPlayerGunSeatBound ? 1 : 0);
        host.log.notef("summary mission gunnery torpedo gyro launches=%llu turn_steps=%llu "
            "mean_launch_offset_deg=%.2f bound=%d (007311B0/00856637/00857061, packet "
            "cc9_usn02_sameside_torpedoes)", host.torpedo_gyro_launches,
            host.torpedo_gyro_turn_steps,
            host.torpedo_gyro_launches ? host.torpedo_gyro_offset_sum_deg / host.torpedo_gyro_launches
                                       : 0.0,
            kTorpedoGyroHeadingBound ? 1 : 0);
        host.log.notef("summary mission gunnery bullet throw cone=%llu fan=%llu zero=%llu "
            "mean_magnitude_deg=%.4f mean_angle_deg=%.4f seats aa=%llu tail=%llu flak=%llu "
            "torpedo=%llu depth=%llu artillery=%llu pilot=%llu none=%llu bound=%d "
            "(0073031D/00730540, packet cc9_bullet_throw)",
            host.throw_cone_shots, host.throw_fan_shots, host.throw_zero_shots,
            host.throw_cone_shots ? host.throw_magnitude_sum_deg / host.throw_cone_shots : 0.0,
            host.throw_cone_shots ? host.throw_angle_sum_deg / host.throw_cone_shots : 0.0,
            host.throw_by_seat[kThrowAaGunner], host.throw_by_seat[kThrowTailGunner],
            host.throw_by_seat[kThrowAaFlak], host.throw_by_seat[kThrowTorpedo],
            host.throw_by_seat[kThrowDepthCharge], host.throw_by_seat[kThrowArtillery],
            host.throw_by_seat[kThrowPilot], host.throw_by_seat[kThrowSeatCount],
            kBulletThrowBound ? 1 : 0);
        host.log.notef("summary mission gunnery muzzle offsets shots=%llu fallbacks=%llu "
            "mean_shift=%.2f max_shift=%.2f bound=%d (00730762/00859550, packet cc9_muzzle_offsets)",
            host.muzzle_offset_shots, host.muzzle_offset_fallbacks,
            host.muzzle_offset_shots != 0
                ? host.muzzle_shift_sum / static_cast<double>(host.muzzle_offset_shots) : 0.0,
            host.muzzle_shift_max, kMuzzleOffsetsBound ? 1 : 0);
        host.log.notef("summary mission gunnery torpedo_drop drops=%llu refusals=%llu "
            "water_entry_breakups=%llu",
            s.torpedo_drops, s.torpedo_drop_refusals, s.water_entry_breakups);
        host.log.notef("summary mission gunnery ballistics aa_direct_aims=%llu "
            "artillery_arc_aims=%llu aim_error_rerolls=%llu no_gravity_shots=%llu "
            "switches arc=%d nograv=%d aimerr=%d (packet cc9_gun_ballistics)",
            host.aa_direct_aims, host.artillery_arc_aims, host.aim_error_rerolls,
            host.no_gravity_shots, kGunGravityArcBound ? 1 : 0,
            kBulletNoGravityBound ? 1 : 0, kGunAimErrorBound ? 1 : 0);
        host.log.notef("summary mission gunnery plane guns trigger_ticks=%llu rounds=%llu "
            "hooked=%d intercept_solves=%llu intercept=%d dp_air_rounds=%llu dp=%d "
            "negative_halvings=%llu halving=%d (packet cc9_aa_lead)",
            host.plane_trigger_ticks, host.plane_gun_rounds, kPlaneGunfireHooked ? 1 : 0,
            host.intercept_solves, kGunInterceptBound ? 1 : 0, host.dp_air_rounds,
            kDualPurposeSecondAmmoBound ? 1 : 0, host.aa_negative_halvings,
            kAaGunnerErrorBound ? 1 : 0);
        host.log.notef("summary mission gunnery water depth kills=%llu (007CE3A7, packet "
            "cc9_water_surface_law)", host.water_depth_kills);
        host.log.notef("summary mission gunnery flak proximity locks=%llu bursts=%llu bound=%d "
            "(0070C370, packet cc9_flak_proximity_burst)", host.flak_locks, host.flak_bursts,
            kFlakProximityBurstBound ? 1 : 0);
        host.log.notef("summary mission gunnery barrel count from model guns=%llu changed=%llu "
            "fallback=%llu devices=%zu bound=%d (007325A0/0072AB80, packet cc9_gun_barrel_count)",
            host.barrel_guns_from_model, host.barrel_guns_changed, host.barrel_guns_fallback,
            host.fire_points_by_device.size(), kGunBarrelCountBound ? 1 : 0);
        host.log.notef("summary mission gunnery artillery aim points drawn=%llu bound=%d "
            "(006DF520 step 4 / 00816650, packet cc9_surface_gunnery_reference)",
            host.artillery_aim_points, kArtilleryAimPointBound ? 1 : 0);
        host.log.notef("summary mission gunnery kill credit zero-damage hits not attributed=%llu bound=%d "
            "(0077CEB7, packet cc9_kill_credit)", host.summary_zero_damage_attributions_skipped,
            kKillCreditDamageGateBound ? 1 : 0);
        host.log.notef("summary mission gunnery ship sections picked=%llu bound=%d; shell mesh hits=%llu "
            "bound=%d (00816650 / 0081F980; 00724510 -> 00723AA0, packet cc9_hull_sections)",
            host.artillery_section_points, kShipSectionPointsBound ? 1 : 0, host.shell_mesh_hits,
            kShellHullHitTestBound ? 1 : 0);
        host.log.notef("summary mission gunnery aa line of fire queries=%llu blocked=%llu "
            "refusals=%llu bound=%d (0072F6E0/0072CDD0/0098B130, packet cc9_ship_platform_attachment)",
            host.line_of_fire_queries, host.line_of_fire_blocked, host.line_of_fire_refusals,
            kAaLineOfFireBound ? 1 : 0);
        host.log.notef("summary mission gunnery aabb 0085cdb0 line_of_fire_tests=%llu "
            "narrowphase_box_hits=%llu bound=%d (0098B130/00929B80, packet cc9_aabb_0085cdb0)",
            host.line_of_fire_aabb_tests, host.narrowphase_box_0085cdb0, kAabb0085cdb0Bound ? 1 : 0);
        host.log.notef("summary mission gunnery ship mounts from model=%llu missing=%llu "
            "classes=%zu bound=%d (0095F500 slot frames, packet cc9_ship_platform_attachment)",
            host.mounts_from_model, host.mounts_missing, host.ship_slots_by_class.size(),
            kShipPlatformAttachmentBound ? 1 : 0);
        for (const auto& [device, fp] : host.fire_points_by_device) {
            host.log.notef("summary mission gunnery barrel device=%d guns=%zu records=%d image=%d "
                "loaded=%d mesh=%s", device, fp.guns, fp.records,
                fp.loaded ? bsp::gun_muzzle_count_0072ab80(fp.list.offsets.size()) : -1,
                fp.loaded ? 1 : 0, fp.mesh.empty() ? "-" : fp.mesh.c_str());
        }
        host.log.notef("summary mission aa acceptance bound=%d window_rejects=%llu "
            "armour_rejects=%llu min_range_skips=%llu (packet cc9_aa_targeting)",
            (kAaMinRangeBound ? 1 : 0) | (kAaFireWindowBound ? 2 : 0) | (kAaArmourBound ? 4 : 0),
            host.aa_window_rejects, host.aa_armour_rejects,
            host.aa_min_range_skips);
        for (const auto& kv : host.aa_target_stats) {
            const GameGunneryHost::Impl::AaTargetStats& st = kv.second;
            host.log.notef("summary mission aa target %-24s scored=%llu accepted=%llu "
                "range_rejects=%llu gun_range_rejects=%llu assigns=%llu min_dist=%.0f "
                "(by %s cat %d, rel_alt %.0f) min_rejected=%.0f against range %.0f | "
                "held_ticks=%llu angle_refusals=%llu shots=%llu refused_vert_max=%.1f "
                "accepted_vert_max=%.1f shot_range_min=%.0f",
                host.unit_state[kv.first].row.name.c_str(), st.scored, st.accepted,
                st.range_rejects, st.gun_range_rejects, st.assigns,
                static_cast<double>(st.min_dist), st.ship_at_min.c_str(), st.category_at_min,
                static_cast<double>(st.alt_at_min), static_cast<double>(st.min_reject_dist),
                static_cast<double>(st.reject_range_at_min), st.targeted_ticks,
                st.angle_refusals, st.shots, static_cast<double>(st.refused_vert_max),
                static_cast<double>(st.accepted_vert_max),
                static_cast<double>(st.shot_range_min));
        }
        host.log.notef("summary mission gunnery torpedo_loadout_cleared=%llu "
            "(drops that cleared the owner's kind 2Bh bit, so approach+132h goes false)",
            s.torpedo_loadout_cleared);
        host.log.notef("summary mission gunnery ordnance_rearms=%llu mode=%s (already-built "
            "units a later spawn batch found with a drop-cleared mask; packet cc9_gunnery_host)",
            host.ordnance_rearms, restore_all_ordnance_enabled() ? "restore_all" : "new_only");
        // TRACE, packet cc8_torpedo_swim item 1: a traced round still in the
        // list at mission end took no exit at all, which is its own answer.
        for (const GameProjectileRow& row : host.shots) {
            if (row.torpedo_trace_id == 0) continue;
            host.log.notef("  torpedo trace %llu STILL IN FLIGHT at mission end "
                "life=%.2f pos=(%.1f,%.2f,%.1f) vel=(%.1f,%.2f,%.1f) swimming=%d",
                row.torpedo_trace_id, static_cast<double>(row.life),
                static_cast<double>(row.position[0]),
                static_cast<double>(row.position[1]),
                static_cast<double>(row.position[2]),
                static_cast<double>(row.flight.velocity.x),
                static_cast<double>(row.flight.velocity.y),
                static_cast<double>(row.flight.velocity.z),
                row.swimming ? 1 : 0);
        }
    // Packet cc8_dive_glide: the bomb drops, and each round's impact against
    // the point approach+D8h/+E0h predicted for it at the release tick. The
    // predicted error is the one that scores the CCIP chain; the target error
    // scores the whole attack. Both are planar and centre to centre.
    {
        host.log.notef("summary mission gunnery bomb_drops=%llu refusals=%llu "
            "(kind 2Ah rows; a refusal means the unit carried no bomb platform)",
            s.bomb_drops, s.bomb_drop_refusals);
        std::vector<GameBombImpactRow> rows = host.bomb_impacts;
        host.log.notef("summary mission gunnery bomb_impacts=%zu", rows.size());
        for (const GameBombImpactRow& r : rows) {
            host.log.notef("  bomb from %-12s impact %.0f %.0f %.0f after %.2f s "
                "| vs predicted 009C7D71 %.0f %.0f %.0f = %.1f m "
                "| vs target at release = %.1f m | died %s",
                r.owner_name.c_str(),
                static_cast<double>(r.actual[0]), static_cast<double>(r.actual[1]),
                static_cast<double>(r.actual[2]), static_cast<double>(r.life),
                static_cast<double>(r.predicted[0]),
                static_cast<double>(r.predicted[1]),
                static_cast<double>(r.predicted[2]),
                static_cast<double>(r.predicted_error),
                static_cast<double>(r.target_error),
                r.died_above_water ? "above water (entity sweep)"
                                   : "at the sea surface");
            // Packet cc8_dive_aim item 2. Every label here names WHEN it was
            // sampled, because the trap this line exists to close is the
            // aimdive census `tf=` column, which is last-sampled and was read
            // as a release value. `tf@release` is 009C7D71's own fall time on
            // the tick the round left; compare it against `after` on the line
            // above, which is the fall this round actually flew.
            host.log.notef("    at release: tf@release=%.2f s | target %-12s "
                "speed=%.1f m/s heading=%.3f rad "
                "| at impact: target moved to %.0f %.0f %.0f heading=%.3f rad "
                "| miss vs target AT IMPACT = %.1f m "
                "(along course %+.1f m, across %+.1f m)",
                static_cast<double>(r.release_fall_time),
                // CORRECTED: this slot printed `owner_name` under a `target`
                // label in the first cut, which is the exact trap the packet
                // rules name - a column's meaning comes from its printing code.
                // local\aim_before.log's rows read `target D3A Val #3.1`, which
                // is the BOMBER. The ordered target's name is carried now.
                r.target_name.empty() ? "-" : r.target_name.c_str(),
                static_cast<double>(r.target_speed_release),
                static_cast<double>(r.target_heading_release),
                static_cast<double>(r.target_pos_impact[0]),
                static_cast<double>(r.target_pos_impact[1]),
                static_cast<double>(r.target_pos_impact[2]),
                static_cast<double>(r.target_heading_impact),
                static_cast<double>(r.impact_error),
                static_cast<double>(r.miss_along),
                static_cast<double>(r.miss_across));
        }
        for (const GameProjectileRow& row : host.shots) {
            if (!row.is_bomb) continue;
            host.log.notef("  bomb from %-12s still in flight at alt %.0f m",
                host.unit_name_or_index(row.owner_unit).c_str(),
                static_cast<double>(row.position[1]));
        }
    }
    // Packet cc8_torpedo_closest_approach: the measurement the torpedo stream
    // has owed since docs/TORPEDO_AFTER_THE_DROP.md section 2. Distances are
    // CENTRE TO CENTRE and horizontal - this host has no oriented hull box - so
    // read each against the target's own Length (Northampton's class row is
    // 180.0 m), not as a miss distance from the plating.
    {
        std::vector<GameTorpedoApproachRow> rows = host.torpedo_approaches;
        for (const GameProjectileRow& row : host.shots) {
            if (!row.swimming) continue;
            GameTorpedoApproachRow rec;
            rec.owner_name = host.unit_name_or_index(row.owner_unit);
            rec.nearest_name = host.unit_name_or_index(row.min_enemy_unit);
            rec.min_distance = row.min_enemy_distance;
            rec.min_time = row.min_enemy_time;
            rec.life_at_end = row.life;
            host.fill_ordered_fields(rec, row);
            rows.push_back(rec);
        }
        host.log.notef("summary mission gunnery torpedo_closest_approach swims=%zu "
            "(centre to centre, horizontal)", rows.size());
        for (const GameTorpedoApproachRow& r : rows) {
            host.log.notef("  torpedo from %-12s nearest %-14s min=%.1f m at t=%.2f s "
                "of %.2f s run | ordered %-14s min=%.1f m at t=%.2f s "
                "target_moved=%.1f m crossing=%.3f rad "
                "| at the drop: own_pose=%.4f target_pose=%.4f "
                "crossing=%.4f rad (%.1f deg) release_range=%.1f m "
                "abs_cos_aspect=%.3f",
                r.owner_name.c_str(), r.nearest_name.c_str(),
                static_cast<double>(r.min_distance), static_cast<double>(r.min_time),
                static_cast<double>(r.life_at_end), r.ordered_name.c_str(),
                static_cast<double>(r.ordered_min_distance),
                static_cast<double>(r.ordered_min_time),
                static_cast<double>(r.target_travel),
                static_cast<double>(r.crossing_angle),
                static_cast<double>(r.drop_owner_heading),
                static_cast<double>(r.drop_target_heading),
                static_cast<double>(r.drop_crossing_angle),
                static_cast<double>(r.drop_crossing_angle) * 180.0 / 3.14159265358979323846,
                static_cast<double>(r.drop_ordered_range),
                // The gate's own x: |cos(aspect)| at 009D1FA3/009D1FBF, printed
                // beside the range so a moved release can be read against the
                // 0.5 knee without recomputing it by hand.
                r.drop_crossing_angle < 0.0f
                    ? -1.0
                    : std::fabs(std::cos(static_cast<double>(r.drop_crossing_angle))));
        }
    }
    {
        // Packet cc8_torpedo_gun_assignment. Which category a swim-capable round
        // is actually mounted in, and whether the units that own a
        // TORPEDO-category gun ever get the director target that is their only
        // candidate source. Both questions are answered per run rather than
        // assumed from the category name.
        std::array<int, bsp::kUnitGunneryCategoryCount> cat_swim{};
        std::vector<std::size_t> torpedo_units;
        for (const GameGunRow& row : host.guns) {
            if (row.category < 0 || row.category >= bsp::kUnitGunneryCategoryCount) continue;
            const std::size_t slot = static_cast<std::size_t>(row.category);
            if (row.swim_speed > 0.0f) ++cat_swim[slot];
            if (row.category == bsp::kUnitGunneryTorpedoCategory
                && std::find(torpedo_units.begin(), torpedo_units.end(), row.unit_index)
                    == torpedo_units.end()) {
                torpedo_units.push_back(row.unit_index);
            }
        }
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            if (cat_swim[static_cast<std::size_t>(c)] == 0) continue;
            const char* name = bsp::gunnery_category_function_name(c);
            host.log.notef("  swim-capable guns in category %2d %-20s %d", c,
                name != nullptr ? name : "?", cat_swim[static_cast<std::size_t>(c)]);
        }
        std::size_t with_command = 0;
        for (const std::size_t u : torpedo_units) {
            if (u < host.command_target_by_unit.size()
                && host.command_target_by_unit[u] != 0) {
                ++with_command;
            }
        }
        host.log.notef("  TORPEDO-category owners=%zu, of which with a command "
            "target=%zu (their only candidate source: 008651F5 cuts category 7 "
            "out of the recon sweep)", torpedo_units.size(), with_command);
    }
        host.log.notef("summary mission gunnery torpedo_candidates pass_ticks=%llu "
            "command_target=%llu fire_target=%llu scored=%llu accepted=%llu "
            "reject unknown=%llu liveness=%llu class=%llu rank=%llu mask=%llu range=%llu",
            s.torpedo_cat_pass_ticks, s.torpedo_cat_with_command_target,
            s.torpedo_cat_with_fire_target, s.torpedo_cat_score_calls,
            s.torpedo_cat_score_accepted, s.torpedo_cat_reject_unknown,
            s.torpedo_cat_reject_liveness, s.torpedo_cat_reject_class,
            s.torpedo_cat_reject_rank, s.torpedo_cat_reject_mask,
            s.torpedo_cat_reject_range);
    }
    host.log.notef("summary mission gunnery contacts considered=%llu side=%llu "
        "invisible=%llu dead=%llu kind=%llu admit_ship=%llu admit_plane=%llu",
        s.contact_considered, s.contact_reject_side, s.contact_reject_visible,
        s.contact_reject_dead, s.contact_reject_kind, s.contact_admit_ship,
        s.contact_admit_plane);
    host.log.notef("summary mission gunnery targeted refusals=%llu no_accept=%llu "
        "no_settle=%llu no_window=%llu", s.angle_refusals_targeted,
        s.want_fire_no_accept, s.want_fire_no_settle, s.want_fire_no_window);
    host.log.notef("summary mission gunnery aim angle_sets=%llu refusals=%llu steps=%llu "
        "arc_blocks=%llu arc_unsolved=%llu trigger_rises=%llu fire_messages=%llu "
        "fire_if_ready=%llu can_fire_refusals=%llu shots=%llu first_shot=%.2f s",
        s.angle_sets, s.angle_refusals, s.aim_steps, s.arc_blocks, s.arc_unsolved,
        s.trigger_rises,
        s.fire_messages, s.fire_if_ready, s.can_fire_refusals, s.shots,
        static_cast<double>(s.first_shot_seconds));
    host.log.notef("summary mission gunnery projectiles created=%llu steps=%llu sweeps=%llu "
        "entity_impacts=%llu water=%llu expired=%llu in_flight=%zu",
        s.projectiles, s.projectile_steps, s.sweeps, s.impacts_entity, s.water_crossings,
        s.expired, host.shots.size());
    host.log.notef("summary mission gunnery damage queued_hits=%llu dispatched=%llu "
        "hit_records=%llu hull=%llu part=%llu fires=%llu floods=%llu attributions=%llu "
        "deaths=%llu kill_credits=%llu total_damage=%.1f first_hit=%.2f s",
        s.queued_hits, s.dispatched_hits, s.ship_hit_records, s.hull_damages,
        s.part_damages, s.fire_messages_9e, s.flood_messages_9e, s.attributions, s.deaths,
        s.kill_credits, static_cast<double>(s.damage_total),
        static_cast<double>(s.first_hit_seconds));

    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_guns{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_assigns{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_shots{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_refusals{};
    std::array<unsigned long long, bsp::kUnitGunneryCategoryCount> cat_blocks{};
    for (const GameGunRow& gun : host.guns) {
        if (gun.category < 0 || gun.category >= bsp::kUnitGunneryCategoryCount) continue;
        const std::size_t slot = static_cast<std::size_t>(gun.category);
        ++cat_guns[slot];
        cat_assigns[slot] += gun.assigns;
        cat_shots[slot] += gun.shots;
        cat_refusals[slot] += gun.angle_refusals;
        cat_blocks[slot] += gun.arc_blocks;
    }
    host.log.note("  cat  Function              guns   assigns     shots  no_window  "
        "arc_blocked");
    for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
        const std::size_t slot = static_cast<std::size_t>(c);
        if (cat_guns[slot] == 0) continue;
        const char* name = bsp::gunnery_category_function_name(c);
        host.log.notef("  %3d  %-20s %5llu %9llu %9llu %10llu %12llu", c,
            name != nullptr ? name : "?", cat_guns[slot], cat_assigns[slot],
            cat_shots[slot], cat_refusals[slot], cat_blocks[slot]);
    }

    std::vector<const GameGunRow*> ordered;
    ordered.reserve(host.guns.size());
    for (const GameGunRow& gun : host.guns) {
        if (gun.shots > 0) ordered.push_back(&gun);
    }
    std::sort(ordered.begin(), ordered.end(),
        [](const GameGunRow* a, const GameGunRow* b) { return a->shots > b->shots; });
    if (!ordered.empty()) {
        host.log.note("  gun                       plat  cat  range  assigns  shots  "
            "first_shot  target");
        std::size_t shown = 0;
        for (const GameGunRow* gun : ordered) {
            if (shown++ >= 20) break;
            host.log.notef("  %-24s %5d %4d %6.0f %8llu %6llu %11.2f  %s",
                gun->unit_name.c_str(), gun->platform_key, gun->category,
                static_cast<double>(gun->max_range), gun->assigns, gun->shots,
                static_cast<double>(gun->first_shot_seconds),
                gun->target_name.empty() ? "-" : gun->target_name.c_str());
        }
    }

    if (Impl::rng_streams_enabled()) {
        // Per-entity rows for BSP_GUNNERY_RNG_STREAMS pairs: every gun, by index.
        host.log.note("summary mission gunnery per-consumer random streams ON "
            "(BSP_GUNNERY_RNG_STREAMS=1, a measurement substitution)");
        for (std::size_t g = 0; g < host.guns.size(); ++g) {
            const GameGunRow& gun = host.guns[g];
            host.log.notef("  gunrow %4zu %-24s plat %3d cat %2d assigns %6llu clears %6llu "
                "shots %6llu rises %6llu refusals %7llu minrange_skips %5llu hits %4llu dealt %8.1f"
                " dev %4d barrels %d",
                g, gun.unit_name.c_str(), gun.platform_key, gun.category, gun.assigns,
                gun.clears, gun.shots, gun.trigger_rises, gun.angle_refusals,
                host.aa_min_range_skips_by_gun.count(g) != 0
                    ? host.aa_min_range_skips_by_gun.at(g) : 0ull,
                host.hits_by_gun.count(g) != 0 ? host.hits_by_gun.at(g).first : 0ull,
                host.hits_by_gun.count(g) != 0 ? host.hits_by_gun.at(g).second : 0.0,
                gun.device_class, gun.barrel_num);
        }
    }
    host.log.note("  unit                 side  guns  cats                 range  nearest"
        "  shots  hits   dealt   taken   health   sunk_at  killed_by");
    for (const Impl::UnitState& state : host.unit_state) {
        if (state.row.guns == 0 && state.row.hits_taken == 0) continue;
        std::string categories;
        for (int c = 0; c < bsp::kUnitGunneryCategoryCount; ++c) {
            if (state.row.category_guns[static_cast<std::size_t>(c)] == 0) continue;
            char text[24];
            std::snprintf(text, sizeof(text), "%d:%d ", c,
                state.row.category_guns[static_cast<std::size_t>(c)]);
            categories += text;
        }
        host.log.notef("  %-20s %4d %5zu  %-18s %6.0f %8.0f %6llu %5llu %7.0f %7.0f "
            "%8.0f %9.2f  %s",
            state.row.name.c_str(), state.row.side, state.row.guns, categories.c_str(),
            static_cast<double>(state.row.best_range),
            static_cast<double>(state.row.nearest_enemy),
            state.row.shots, state.row.hits_taken,
            static_cast<double>(state.row.damage_dealt),
            static_cast<double>(state.row.damage_taken),
            static_cast<double>(state.row.health),
            static_cast<double>(state.row.sunk_seconds),
            state.row.killed_by.empty() ? "-" : state.row.killed_by.c_str());
    }
}

// Packet cc9_ship_torpedo_response.
std::vector<GameGunneryHost::LiveTorpedo> GameGunneryHost::live_torpedoes() const {
    std::vector<LiveTorpedo> out;
    for (const GameProjectileRow& shot : impl_->shots) {
        if (!shot.alive || shot.serial == 0) continue;
        const GameBulletClassRow* row = impl_->bullet(shot.bullet_class);
        if (row == nullptr || !(row->water_travel_speed > 0.0f)) continue;
        LiveTorpedo t;
        t.serial = shot.serial;
        t.owner_unit = shot.owner_unit;
        t.owner_side = shot.owner_side;
        for (int i = 0; i < 3; ++i) t.position[i] = shot.position[i];
        t.velocity[0] = shot.flight.velocity.x;
        t.velocity[1] = shot.flight.velocity.y;
        t.velocity[2] = shot.flight.velocity.z;
        t.swim_seconds = shot.swim_seconds;
        t.swimming = shot.swimming;
        t.water_travel_speed = row->water_travel_speed;
        out.push_back(t);
    }
    return out;
}

float GameGunneryHost::ship_ai_draw(std::size_t unit_index, float low, float high) {
    return impl_->draw(Impl::Draw::ship_ai_torpedo, unit_index, 0, low, high);
}

}  // namespace bsp::game
