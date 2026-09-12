#pragma once
// A projectile from the muzzle to the hit record: the record the factory fills,
// the per-step flight integration, and the spawn and impact sequences.
//
// Addresses: 0072F830 0072BF10 006E8430 006E7B00 006E6BB0 009555A0 006E6490
//            0070C370 006E65C0 006E65F0 006E7670 006E7760 0084C430 0084BF00
//            0084BC60 0098B370 00926E80 00926700 009239A0.
// Evidence and coverage: docs/PROJECTILE_IMPACT.md,
// reports/projectile_impact.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native routines are __thiscall or __fastcall on raw pointers; these are new
// C++ interfaces over the same arithmetic, not drop-in binary replacements.
#include <cstddef>
#include <cstdint>

#include "bsp/tick_element_overrides.hpp"
#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants the listing loads by address.
// ---------------------------------------------------------------------------

// 00CF9058, a double. Read by 006E65C0 (FMUL QWORD), 006E7670 and 006E8430.
inline constexpr double kProjectileGravity = 9.8103800773621;

// 00D7A280, a double, the half in the s = v*t - 0.5*g*t^2 term at 006E7714.
inline constexpr double kProjectileHalf = 0.5;

// 00D0DE84, the 0.05f the launch bias at 006E85xx multiplies by the gravity.
// The same address is kTickElementFixedStep in bsp/tick_element_overrides.hpp;
// this alias names the role it plays here.
inline constexpr float kProjectileLaunchBiasStep = 0.05f;

// 00D0B9C0, a double: 0084BF00 skips the sweep below this squared length.
inline constexpr double kProjectileMinSweepLengthSquared = 1.0e-7;

// 00D7A2F0, the upper bound of the per-shot random at 006E7B97.
inline constexpr float kProjectileRandomPhaseMax = 0.1f;

// 006E8430 allocates and zeroes exactly this much.
inline constexpr std::size_t kProjectileRecordSize = 0x284;

// ---------------------------------------------------------------------------
// Class ids the listing passes to vtable[5Ch].
// ---------------------------------------------------------------------------

inline constexpr int kProjectileClassId = 0x29;          // 006E7B87, [proj+C4h]
inline constexpr int kProjectileAltShotClassId = 0x2A;   // 009239A0, 0084BC60
inline constexpr int kProjectileScoringTargetKind = 0x44; // 00923A2C
inline constexpr int kProjectileHitListenerKind = 0x02;   // the 0077CE60 gate

// The weapon sub-types at classDesc[+8h] that the flight and impact separate.
inline constexpr int kProjectileSubTypeDecalVariant = 0x10;    // 0084BC60
inline constexpr int kProjectileSubTypeFriendlyExempt = 0x11;  // 0084BF00

// ---------------------------------------------------------------------------
// The projectile record, base object. 006E8430 zeroes 284h bytes first, so
// every field not listed starts at zero. The interface a caller holds is
// base + kProjectileOffShotInterface and the tick element the fixed step drives
// is base + kProjectileOffTickElement.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kProjectileOffChildNodeNext = 0x44;   // 006E7699
inline constexpr std::size_t kProjectileOffChildNodeHead = 0x48;   // 006E767D
inline constexpr std::size_t kProjectileOffOwnerParty = 0x54;      // 0072C025; the owner PARTY index, not an id (docs/PROJECTILE_HELPERS.md)
inline constexpr std::size_t kProjectileOffGunId = 0x58;           // 0072C013
inline constexpr std::size_t kProjectileOffSweepEnabled = 0x5C;    // 006E64EF
inline constexpr std::size_t kProjectileOffBindGate = 0x5D;        // 006E6BB0
inline constexpr std::size_t kProjectileOffLaunchVelocity = 0x94;  // 006E8430
inline constexpr std::size_t kProjectileOffLocalPosition = 0xA4;   // 006E7670
inline constexpr std::size_t kProjectileOffClassId = 0xC4;         // 006E7B87
inline constexpr std::size_t kProjectileOffPoseValid = 0xC8;       // 006E7682
inline constexpr std::size_t kProjectileOffWorldTranslation = 0xFC; // 0084BC60
inline constexpr std::size_t kProjectileOffPoseFlag2 = 0x10C;      // 006E7689
inline constexpr std::size_t kProjectileOffShotInterface = 0x170;  // 006E7B24
inline constexpr std::size_t kProjectileOffClassDesc = 0x174;      // 006E8430
inline constexpr std::size_t kProjectileOffVelocity = 0x178;       // 006E65C0
inline constexpr std::size_t kProjectileOffOwnerSubId = 0x184;     // 009555CB
inline constexpr std::size_t kProjectileOffOwnerEntityId = 0x188;  // 009555D3
inline constexpr std::size_t kProjectileOffTeamId = 0x18C;         // 0072C0FB
inline constexpr std::size_t kProjectileOffPlatformVelocity = 0x1B8; // 0072C068
inline constexpr std::size_t kProjectileOffFlightTime = 0x1C4;     // 006E64BF
inline constexpr std::size_t kProjectileOffSweepExtra = 0x1C8;     // 006E6541
inline constexpr std::size_t kProjectileOffSpawnFlag = 0x1CC;      // 006E8430
inline constexpr std::size_t kProjectileOffSnapshotCurrent = 0x1D0;  // 006E7D50
inline constexpr std::size_t kProjectileOffSnapshotPrevious = 0x1DC; // 006E6569
inline constexpr std::size_t kProjectileOffObserverStamp = 0x204;  // 006E6BB0
inline constexpr std::size_t kProjectileOffOwner = 0x238;          // 006E6BB0
inline constexpr std::size_t kProjectileOffSelfPointer = 0x23C;    // 006E7B91
inline constexpr std::size_t kProjectileOffTickElement = 0x244;    // 006E7B3B
inline constexpr std::size_t kProjectileOffNearCameraFlag = 0x278; // provisional
inline constexpr std::size_t kProjectileOffRandomPhase = 0x27C;    // 006E7B9C
inline constexpr std::size_t kProjectileOffFlakState = 0x294;      // 0070C3C1

// The class descriptor fields the flight and impact read.
inline constexpr std::size_t kProjectileClassOffSubType = 0x08;      // 0095561B
inline constexpr std::size_t kProjectileClassOffNoGravity = 0x20;    // 006E65C6
inline constexpr std::size_t kProjectileClassOffMuzzleSpeed = 0x50;  // 006E8430
inline constexpr std::size_t kProjectileClassOffMaxLife = 0x54;      // 006E658D
inline constexpr std::size_t kProjectileClassOffTimeScale = 0x5C;    // 006E649B
inline constexpr std::size_t kProjectileClassOffExplosion = 0x6C;    // 0084BE28
inline constexpr std::size_t kProjectileClassOffTraceVariant = 0x74; // 0084BF00
inline constexpr std::size_t kProjectileClassOffScoringId = 0xCC;    // 00923A2C
inline constexpr std::size_t kProjectileClassOffFuseTime = 0xD8;     // 0070C3DA

// ---------------------------------------------------------------------------
// The 70h-byte staging buffer 0084BF00 zeroes and 0084BC60 reads. The hit
// record of bsp/unit_hit_path.hpp starts at kProjectileStagingOffHitRecord;
// the offsets in that header are relative to the record, not to this buffer.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kProjectileStagingSize = 0x70;
inline constexpr std::size_t kProjectileStagingOffShot = 0x00;
inline constexpr std::size_t kProjectileStagingOffOwner = 0x04;
inline constexpr std::size_t kProjectileStagingOffClassDesc = 0x08;
inline constexpr std::size_t kProjectileStagingOffDirection = 0x10; // 0084BE1D
inline constexpr std::size_t kProjectileStagingOffHitRecord = 0x1C; // 0084BE18

// Producer-side hit-record fields. The rest of the record is filled by the
// narrowphase under 0098ADD0 and is unread; bsp/unit_hit_path.hpp carries the
// consumer-side offsets for those.
inline constexpr std::size_t kHitRecordOffHitEntity = 0x00;
inline constexpr std::size_t kHitRecordOffHitPosition = 0x08;
// 00926E80 appends the impact direction here, in the queued copy only.
inline constexpr std::size_t kHitRecordOffQueuedDirection = 0x54;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// The motion mode the shot interface's vtable[2Ch] selects. 006E8430 decides it
// once at spawn from the water height at the muzzle; 006E6750 and 006E6490 then
// use it to pick between two position slots and two velocity slots.
enum class ProjectileMotionMode : std::uint8_t {
    // vtable[2Ch] false: 006E7670 for position, 006E65C0 for velocity.
    kBallistic = 0,
    // vtable[2Ch] true: 006E7760 for position, 006E65F0 for velocity.
    kDamped = 1,
};

// The state the four integration slots read and write. Positions are the
// snapshot the previous commit left and the local position the slots update;
// TickPoint3 comes from bsp/tick_element_overrides.hpp.
struct ProjectileFlightState {
    TickPoint3 velocity;          // projectile+178h..180h
    TickPoint3 snapshot_current;  // projectile+1D0h..1D8h
    TickPoint3 local_position;    // projectile+A4h..ACh
    float flight_time = 0.0f;     // projectile+1C4h
    ProjectileMotionMode mode = ProjectileMotionMode::kBallistic;
    bool class_disables_gravity = false; // classDesc[+20h]
};

// 006E7670 (slot 114h) and 006E7760 (slot 118h), position only. The pose flags
// and the child-node walk are effects and stay with the host.
//
//   position = snapshot_current + velocity * scaled_step
//   position.y -= 0.5 * g * scaled_step^2   (ballistic and gravity enabled)
//
// The subtraction is written as (scaled*g)*scaled*0.5 in the listing; it is
// evaluated here in the same order so the float rounding matches.
TickPoint3 projectile_integrate_position_006e7670(const ProjectileFlightState& state,
                                                  float scaled_step);

// 006E65C0 (slot 11Ch) and 006E65F0 (slot 120h), velocity only.
//
//   ballistic: v.y -= scaled_step * g, and only when gravity is enabled
//   damped:    v += (-v) * scaled_step
TickPoint3 projectile_integrate_velocity_006e65c0(const ProjectileFlightState& state,
                                                  float scaled_step);

// One fixed step of both slots, in the order 006E6750 then 006E6490 run them:
// the position slot first, from the snapshot the last commit wrote, then the
// flight time, then the velocity slot. Returns the updated state; the caller
// commits it with projectile_tick_commit_pose_006e7d50.
ProjectileFlightState projectile_flight_step(const ProjectileFlightState& state,
                                             float scaled_step);

// 006E8430's launch velocity. speed is classDesc[+50h] and direction is the
// perturbed muzzle direction 0072F830 built.
//
//   v = speed * direction
//   v.y -= 0.05f * g   when sub_type is 4, 5, 6 or 7
TickPoint3 projectile_launch_velocity_006e8430(float speed, const TickPoint3& direction,
                                               int sub_type);

// True for the four sub-types that take the launch bias (006E856x).
bool projectile_sub_type_takes_launch_bias(int sub_type);

// 006E6490's expiry test and 0070C370's two thresholds.
bool projectile_has_expired(float flight_time, float class_max_life);
bool projectile_fuse_is_armed(float flight_time, float class_fuse_time);

// 0084BF00 step 2: the sweep runs only for a segment longer than this.
bool projectile_segment_is_sweepable(const TickPoint3& from, const TickPoint3& to);

// 0084BC60 step 1: mode 1 becomes 3 for a Landscape hit and 4 for a plane hit
// (docs/PROJECTILE_HELPERS.md corrected the earlier "scoring target" / "unit" reading).
enum class ProjectileImpactMode : std::uint8_t {
    kEntity = 1,
    kStatic = 2,
    kLandscape = 3,
    kPlane = 4,
};
ProjectileImpactMode projectile_refine_impact_mode(ProjectileImpactMode mode,
                                                   bool target_is_landscape_kind,
                                                   bool target_is_plane_kind);

// ---------------------------------------------------------------------------
// Spawn
// ---------------------------------------------------------------------------

// What 0072BF10 hands the factory and what it writes back afterwards.
struct ProjectileSpawnRequest {
    TickPoint3 position;      // 0072F830's first argument
    TickPoint3 direction;     // the per-pellet perturbed direction
    int owner_id = 0;         // [gun[+3F0h]+54h]
    int gun_id = 0;           // [gun+58h]
    int team_id = 0;          // gun[+1ACh], or the owner's +1B0h when it is 8
    int spawn_flag = 0;       // 0072F830 step 6
    bool near_camera = false; // 0072BF10 step 1
    float shot_scale = 1.0f;  // gun[+43Ch], applied only when it is not 1
    bool inherit_platform_velocity = false; // [gun[+3F4h]+95h]
};

// One virtual per native call site of the spawn sequence.
struct ProjectileSpawnHost {
    virtual ~ProjectileSpawnHost() = default;

    // 0072C006: classDesc->vtable[20h], seven stack arguments. Returns the shot
    // interface, projectile+170h.
    virtual void* create_projectile_006e8430(const ProjectileSpawnRequest& request) = 0;
    // 006E8430 step 8: BSP_GameWorld_SampleWaterHeight(pos.x, pos.z), and the
    // shot's vtable[28h](0) at or below it, vtable[24h](0) above it.
    virtual float water_height(float x, float z) = 0;
    virtual void shot_set_medium(void* shot, bool submerged, int phase) = 0;
    // 0072C013 and 0072C025, through the self-pointer.
    virtual void store_ids(void* shot, int gun_id, int owner_id) = 0;
    // 0072C033: gun->vtable[1E4h](shot).
    virtual void gun_on_projectile_created(void* shot) = 0;
    // 0072C043: gun[+474h] = [00F876A4].
    virtual void stamp_fire_time() = 0;
    // 0072C064: owner->vtable[34h]() is the platform velocity, stored at
    // shot+48h..50h and added into the velocity at shot+8h..10h.
    virtual TickPoint3 owner_velocity() = 0;
    virtual void add_platform_velocity(void* shot, const TickPoint3& v) = 0;
    // 0072C0AE: shot->vtable[20h]((float)gun[+43Ch]).
    virtual void shot_set_scale(void* shot, float scale) = 0;
    // 0072C0F0: shot->vtable[34h](&{1,1,1}, &{0,0,0}).
    virtual void shot_set_appearance(void* shot) = 0;
    // 0072C0FB and 0072C14B.
    virtual void shot_set_team(void* shot, int team_id) = 0;
    // 0072C155 -> 009555A0 -> 006E6BB0: bind the owner, then publish the
    // director timestamp at [director + 1D0h + sub_type*4].
    virtual void bind_owner_006e6bb0(void* shot) = 0;
    virtual void stamp_director_fire_time(int sub_type) = 0;
};

// 0072BF10 with its factory call, in order. Returns the shot interface.
void* projectile_spawn_0072bf10(const ProjectileSpawnRequest& request, int sub_type,
                                ProjectileSpawnHost& host);

// ---------------------------------------------------------------------------
// Sweep and impact
// ---------------------------------------------------------------------------

// The staging buffer 0084BF00 fills, as the later steps read it.
struct ProjectileSweepStaging {
    void* shot = nullptr;        // +0h, projectile+170h
    void* owner = nullptr;       // +4h, projectile+238h
    void* class_desc = nullptr;  // +8h, projectile+174h
    TickPoint3 direction;        // +10h, the normalised segment
    void* hit_entity = nullptr;  // +1Ch, the hit record's own +0h
    TickPoint3 hit_position;     // +24h, the hit record's +8h
};

// One virtual per native call site of the sweep and the impact.
struct ProjectileImpactHost {
    virtual ~ProjectileImpactHost() = default;

    // 0084BF00 step 1: BSP_Vector3f_Length on the segment delta. Left as a
    // contract; the reconstruction only needs its result to build the unit
    // direction and never reimplements the square root.
    virtual float segment_length(const TickPoint3& delta) = 0;
    // 0084C13F / 0084C14C: 0084B4B0 when alternate is true, else 0084B380.
    // Writes the hit point and returns the byte at +0Ch of the float[4] result.
    virtual bool trace_static(const TickPoint3& from, const TickPoint3& to, bool alternate,
                              TickPoint3& hit) = 0;
    virtual bool shot_motion_mode_2c() = 0;
    virtual bool class_prefers_alternate_trace() = 0;
    // 0084C1D3: 0098B370 against the spatial index, with the owner's collision
    // mask from vtable[B0h]. Fills the hit record at staging +1Ch.
    virtual bool sweep_entities_0098b370(const TickPoint3& from, const TickPoint3& to,
                                         ProjectileSweepStaging& staging) = 0;
    // 0084BF00 step 6: classDesc[+8h] == 11h and a shared +9D4h parent.
    virtual bool entity_hit_is_friendly_exempt(void* hit_entity) = 0;
    // 0084BF00 step 7: [00E188A8]+1FE4h == 2 with the shot passing (2Ah).
    virtual bool replay_forces_static_path() = 0;
    // 0084C2F2 and 0084C34B, the two per-branch notifications.
    virtual void on_entity_hit_0084afd0(void* hit_entity) = 0;
    virtual void on_static_hit_007bc4e0() = 0;
    // 0084BF00 step 9: shot->vtable[24h] / [28h] with 1, the surface crossing
    // that lets a shell keep flying after it enters or leaves the water.
    virtual void shot_switch_medium(bool submerged) = 0;
    // 0084BCBx: projectile+C8h = 1, +10Ch = 0, +FCh..104h = the hit position.
    virtual void teleport_projectile_to_impact(const TickPoint3& hit_position) = 0;
    // 0084BD44 / 0084BD89: the session hit message, routed with
    // BSP_Session_RouteMessage(msg, 4, 0).
    virtual bool session_should_route_hit() = 0;
    virtual void route_hit_message(ProjectileImpactMode mode,
                                   const TickPoint3& hit_position) = 0;
    // 0084BDEA: 0084B8C0, gated on the byte at shot+45h.
    virtual bool shot_leaves_decal() = 0;
    virtual void place_decal_0084b8c0(const TickPoint3& hit_position) = 0;
    // 0084BE00: BSP_MissionEntity_Kill(1). The projectile is removed here, not
    // on a timer. Its gate is one of 0084BC60's two incoming char flags, which
    // this packet did not resolve, so it stays a host question.
    virtual bool impact_kills_projectile() = 0;
    virtual void kill_projectile() = 0;
    // 0084BE20: 00926E80(record, direction). The hit is deferred, not applied.
    virtual void queue_hit_00926e80(void* hit_record, const TickPoint3& direction) = 0;
    // 0084BEE3: 0084BAD0 at hit_position - direction * [00D7A270].
    virtual bool class_has_explosion() = 0;
    virtual float explosion_back_off() = 0;
    virtual void spawn_explosion_0084bad0(const TickPoint3& origin, bool decal_variant) = 0;
};

// 0084BF00: the segment sweep and the branch into 0084BC60. Returns the mode
// the impact ran with, or kStatic with no impact when nothing was hit; the
// out-parameter says whether an impact happened at all.
ProjectileImpactMode projectile_sweep_segment_0084bf00(const TickPoint3& from,
                                                       const TickPoint3& to,
                                                       ProjectileSweepStaging& staging,
                                                       bool& impacted,
                                                       ProjectileImpactHost& host);

// 0084BC60: everything that happens once the impact point is known.
void projectile_on_impact_0084bc60(ProjectileImpactMode mode,
                                   ProjectileSweepStaging& staging,
                                   ProjectileImpactHost& host);

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

// One virtual per native call site of 009239A0, the producer of the OnHit call
// docs/UNIT_HIT_PATH.md lists as unread.
struct ProjectileHitDispatchHost {
    virtual ~ProjectileHitDispatchHost() = default;

    // 009239Bx: [entity+5Eh] and [entity+5Fh] must both be clear.
    virtual bool entity_accepts_events(void* entity) = 0;
    // 009239D4 / 009239E3 / 009239F6 / 00923A0D: vtable[5Ch] on the shot.
    virtual bool shot_is_kind(void* shot, int class_id) = 0;
    // shot[+174h] for 29h, shot[+314h] for 2Ah.
    virtual void* shot_damage_source(void* shot, bool alternate_kind) = 0;
    virtual int source_scoring_id(void* source) = 0;
    virtual bool entity_is_kind(void* entity, int class_id) = 0;
    // 00923A2C: entity->vtable[24h](scoringId, &position, direction).
    virtual void credit_hit(void* entity, int scoring_id, const TickPoint3& position,
                            const TickPoint3& direction) = 0;
    // 00923A70: entity->vtable[ECh](record) is 007BBCF0
    // BSP_UnitInstance_OnHit. Returns false to pass the hit to the parent.
    virtual bool deliver_hit_007bbcf0(void* entity, void* hit_record) = 0;
    // entity[+3Ch], the hierarchy parent the two walks follow.
    virtual void* entity_parent(void* entity) = 0;
    // 00923Ax: 0077CE60(record) when the entity passes vtable[5Ch](2).
    virtual void notify_hit_listener_0077ce60(void* hit_record) = 0;
};

// What the drain hands 009239A0: the queued record, its entity and the impact
// direction 00926E80 appended at +54h.
struct ProjectileQueuedHit {
    void* hit_entity = nullptr; // record +0h
    void* shot = nullptr;       // record +4h
    TickPoint3 hit_position;    // record +8h
    TickPoint3 direction;       // record +54h in the queued copy
    void* record = nullptr;     // the record itself, passed on to OnHit
};

// 009239A0. Returns the entity that accepted the hit, or nullptr when the
// parent chain ran out.
void* projectile_dispatch_queued_hit_009239a0(const ProjectileQueuedHit& hit,
                                              ProjectileHitDispatchHost& host);

}  // namespace bsp
