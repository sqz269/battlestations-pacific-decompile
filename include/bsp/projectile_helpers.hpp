#pragma once
// The flight-and-impact leaves of the projectile chain: the two static water
// traces, the four impact-step helpers, the impact-effect dispatch, the plane
// sweep that shares 0084BF00, and the flak proximity search.
//
// Addresses: 0084B380 0084B4B0 0078D1B0 0084AFD0 007BC4E0 0084B000 0084B650
//            0084B8C0 0084B6F0 007C0910 0070C370 (0070C4A4..0070C795).
// Evidence and coverage: docs/PROJECTILE_HELPERS.md,
// reports/projectile_helpers.json.
//
// The chain above these leaves is bsp/projectile_impact.hpp (0084C430 sweep,
// 0084BF00 trace-and-impact, 0084BC60 impact, 009239A0 dispatch) and
// bsp/blast_damage.hpp (0084BAD0); this header reuses those types and does not
// restate them.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native routines are __thiscall or __fastcall on raw pointers; these are new
// C++ interfaces over the same arithmetic, not drop-in binary replacements.
// Terrain, the scene graph, the renderer, the effect registry and the ballistic
// solver are described as contracts and are not ported.
#include <cstddef>
#include <cstdint>

#include "bsp/projectile_impact.hpp"
#include "bsp/tick_element_overrides.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants the listing loads by address.
// ---------------------------------------------------------------------------

// 00CE6638, a double. 0078D1B0 refuses to bisect a segment whose endpoints
// differ in y by less than this (FCOMIP against the folded absolute value at
// 0078D249).
inline constexpr double kWaterCrossingMinHeightDelta = 0.01;

// 00D7A208, the float -0.0f that 0078D22F..0078D243 subtracts the height delta
// from. It is a compiler-built fabs, not a bias.
inline constexpr float kWaterCrossingNegativeZero = -0.0f;

// The bisection at 0078D282 stops once the working segment is this short, so
// the crossing point is accurate to about one world unit.
inline constexpr float kWaterCrossingTargetLength = 1.0f;

// 00CFD508, read twice by 0070C370 (0070C5A8 and 0070C7B6). The flak's squared
// proximity radius is clamped to it, so the search never reaches past 300
// world units even for a very large blast radius.
inline constexpr float kFlakSearchRadiusSquaredCap = 90000.0f;

// 00CFD50C, 0070C7D7. Below this squared miss distance the flak does not roll
// for a late airburst; 50 world units.
inline constexpr float kFlakAirburstMinMissSquared = 2500.0f;

// 0070C518 doubles classDesc[+70h] before adding half the step length.
inline constexpr float kFlakBlastRadiusSearchMultiple = 2.0f;

// 00CE38B8 over 00CE3D08 at 0070C7E0..0070C806: the per-tick chance that a
// flak round that has passed its closest approach bursts anyway.
inline constexpr float kFlakAirburstRollThreshold = 10.0f;
inline constexpr float kFlakAirburstRollRange = 100.0f;

// 00D7A248, the FLT_MAX 0070C4B1 seeds the nearest-candidate tracker with.
inline constexpr float kFlakNearestSeed = 3.402823466e+38f;

// ---------------------------------------------------------------------------
// The segment list 0084BF00 takes, and the two static traces that walk it.
// ---------------------------------------------------------------------------

// 0084BF00's third argument is not a bare float[6]. 0084C430 reserves 0C4h
// bytes (SUB ESP,0xc4 at 0084C430), writes one segment at +0h..+14h and sets
// the count at +0C0h to 1 (0084C4BD); 007C0910 reserves the same 0C0h block as
// one local and fills several. 0084B380 and 0084B4B0 walk [+0h, count*18h).
inline constexpr std::size_t kTraceSegmentStride = 0x18;
inline constexpr std::size_t kTraceSegmentCapacity = 8;
inline constexpr std::size_t kTraceSegmentListCountOffset = 0xC0;

struct TraceSegment {
    TickPoint3 from;  // +0h..+8h
    TickPoint3 to;    // +0Ch..+14h
};

struct TraceSegmentList {
    TraceSegment segments[kTraceSegmentCapacity];
    int count = 0;  // +0C0h
};

// The float[4] both traces return through the hidden first argument: three
// floats and a flag byte at +0Ch that 0084C15F..0084C16B tests.
struct StaticTraceResult {
    TickPoint3 point;
    bool hit = false;
};

// Which of the two traces 0084C11C..0084C14C selects. The shot's vtable[2Ch]
// is the medium query (0 in air); classDesc[+74h] is the per-class override.
enum class StaticTraceVariant : std::uint8_t {
    // 0084B380: the first segment whose end is at or below the water surface.
    kSurfaceEntry = 0,
    // 0084B4B0: the first segment whose end is strictly above it.
    kSurfaceExit = 1,
};
StaticTraceVariant static_trace_variant_0084c11c(int shot_medium,
                                                 bool class_prefers_entry) noexcept;

// The water height a trace compares against. Both traces and the bisection
// reach it through 0078CF20 on [[00E188A8]+19F0h], whose contract is
// `float __thiscall(world, float x, float z)` (docs/OCEAN_HEIGHT.md,
// src/ocean_height.cpp). It is a host record here because the receiver and its
// +A8h wave field belong to the scene owner.
struct WaterSurfaceSampler {
    virtual ~WaterSurfaceSampler() = default;
    // 0078CF20. The two arguments are the two horizontal axes.
    virtual float water_height_0078cf20(float x, float z) = 0;
};

// 0078D1B0, the crossing point. A bisection, not an interpolation: it halves
// the segment kWaterCrossingTargetLength-wards, each step sampling the surface
// under the midpoint and keeping the half that still straddles it. Returns the
// endpoint that ended up on the submerged side.
//
// Faithful to the shipped code, including the axis defect below: the sampler
// is called with the midpoint's (x, y), not (x, z). The working length comes
// from 0042B2F0 at 0078D277, reused from bsp/gamepad_force_events.hpp rather
// than reimplemented.
TickPoint3 water_crossing_point_0078d1b0(WaterSurfaceSampler& water,
                                         const TickPoint3& from,
                                         const TickPoint3& to) noexcept;

// 0084B380 and 0084B4B0. Both walk the list in order and stop at the first
// segment whose *end point* is on the wanted side of the surface, then hand
// that segment to 0078D1B0. A list that never crosses returns hit = false.
//
// The sampler is called with (end.x, end.y) and the result is compared against
// end.y, exactly as 0084B3F8 and 0084B401 do.
StaticTraceResult static_trace_0084b380(WaterSurfaceSampler& water,
                                        const TraceSegmentList& list) noexcept;
StaticTraceResult static_trace_0084b4b0(WaterSurfaceSampler& water,
                                        const TraceSegmentList& list) noexcept;

// ---------------------------------------------------------------------------
// 0084AFD0: the parent-chain notification of the entity-hit branch.
// ---------------------------------------------------------------------------

// __thiscall(entity, void* payload), RET 4 at 0084AFFA. The payload 0084C2E5
// hands it is an eight-byte pair built on the caller's stack.
struct EntityHitNotice {
    int kind = 1;              // 0084C2E6, always 1 at the one call site
    void* projectile = nullptr; // 0084C2DB, [shot+0CCh]
};

// One virtual per native call site of the notification walk.
struct EntityHitChainHost {
    virtual ~EntityHitChainHost() = default;
    // 0084AFEB: entity->vtable[F8h](payload). Non-zero means "handled".
    virtual bool offer_hit_notice(void* entity, const EntityHitNotice& notice) = 0;
    // 0084AFF1: [entity+3Ch], the parent link.
    virtual void* parent_of(void* entity) = 0;
};

// Walks from the hit entity up the +3Ch chain and stops at the first link that
// accepts. Returns the accepting entity, or null when the chain ran out.
void* notify_entity_hit_chain_0084afd0(EntityHitChainHost& host, void* entity,
                                       const EntityHitNotice& notice) noexcept;

// ---------------------------------------------------------------------------
// 007BC4E0: the plane's persistent crash effect.
// ---------------------------------------------------------------------------

// Offsets on the plane instance that 007BC4E0 reads and writes.
inline constexpr std::size_t kPlaneOffCrashEffect = 0xAA4;      // 007BC4E6
inline constexpr std::size_t kPlaneOffCrashEffectBound = 0xAA8; // 007BC4EF
inline constexpr std::size_t kPlaneOffUnitClassDesc = 0x730;    // 007BC514
// The unit class descriptor's effect slot, not the weapon descriptor's.
inline constexpr std::size_t kUnitClassOffCrashEffect = 0x24;   // 007BC521

struct PlaneCrashEffectHost {
    virtual ~PlaneCrashEffectHost() = default;
    // 007BC4E6 and 007BC4EF: the cache and its bound flag.
    virtual bool crash_effect_already_bound(void* plane) = 0;
    // 007BC4F8: [plane+0C8h], refreshed through 00414DB0 when clear.
    virtual void refresh_world_pose(void* plane) = 0;
    // 007BC512: plane->vtable[34h](&out), the orientation the effect takes.
    virtual const void* plane_effect_direction(void* plane) = 0;
    // 007BC524: 0084B6F0([[plane+730h]+24h], plane+0FCh, direction). The host
    // resolves the descriptor slot; the effect registry below 0084B6F0 is the
    // renderer's and is a contract, see docs/PROJECTILE_HELPERS.md.
    virtual void* spawn_point_effect_0084b6f0(void* plane,
                                              const TickPoint3& position,
                                              const void* direction) = 0;
    // 007BC537: InterlockedIncrement on effect+4h.
    virtual void add_effect_reference(void* effect) = 0;
    // 007BC52B and 007BC53D: store the handle and set the bound flag.
    virtual void store_crash_effect(void* plane, void* effect) = 0;
};

// Runs at most once per plane. Does nothing when the cache or its flag is
// already set, which is why a plane that grazes the ground repeatedly keeps
// one effect rather than accumulating them.
void bind_plane_crash_effect_007bc4e0(PlaneCrashEffectHost& host, void* plane,
                                      const TickPoint3& world_position) noexcept;

// ---------------------------------------------------------------------------
// 0084B000 and 0084B650: the network hit message and its guard.
// ---------------------------------------------------------------------------

// 0075B430's kind argument at 0084B001.
inline constexpr int kProjectileHitMessageKind = 0xBA;
// 0084B025, the vtable the constructor stamps.
inline constexpr std::uint32_t kProjectileHitMessageVTable = 0x00D02D80;

// The message 0084B000 builds, by the offsets it writes. RET 10h at 0084B062,
// so four stack arguments after the ECX receiver.
struct ProjectileHitMessage {
    // +4h, written twice (0084B033 and 0084B05E) with the same 1.
    int routing_field = 1;
    std::uint16_t pad_18 = 0;  // +18h
    std::uint8_t pad_1a = 0;   // +1Ah
    int mode = 0;              // +1Ch, arg1: the refined ProjectileImpactMode
    void* source = nullptr;    // +20h, arg2
    TickPoint3 hit_position;   // +24h..+2Ch, arg3 by pointer
    TickPoint3 direction;      // +30h..+38h, arg4 by pointer
};

ProjectileHitMessage build_hit_message_0084b000(ProjectileImpactMode mode, void* source,
                                                const TickPoint3& hit_position,
                                                const TickPoint3& direction) noexcept;

// 0084B650: __fastcall(obj) with no stack argument (plain RET at 0084B667).
// A guarded down-cast, the same shape MSVC emits for dynamic_cast on a single
// inheritance chain: the object when it answers vtable[5Ch](2Ah), else null.
void* checked_cast_to_shot_owner_0084b650(void* object, bool passes_kind_2a) noexcept;

// ---------------------------------------------------------------------------
// 0084B8C0: the impact-effect dispatch.
// ---------------------------------------------------------------------------

// Weapon-class-descriptor effect slots the dispatch selects between. They are
// fields of the descriptor at staging +8h (projectile+174h), well inside the
// 0D4h the Bullet descriptor allocates.
inline constexpr std::size_t kWeaponClassOffEffectStatic = 0x24;      // 0084B922
inline constexpr std::size_t kWeaponClassOffEffectEntityAir = 0x28;   // 0084B908
inline constexpr std::size_t kWeaponClassOffEffectMedium1 = 0x30;     // 0084B8F5
inline constexpr std::size_t kWeaponClassOffEffectMedium2 = 0x34;     // 0084B8E0
inline constexpr std::size_t kWeaponClassOffEffectLandscape = 0x38;   // 0084B943
inline constexpr std::size_t kWeaponClassOffEffectPlane = 0x3C;       // 0084B95D

// The result of the dispatch: which descriptor field holds the effect
// definition, or none when the pair falls through to the bare RET at 0084B969.
struct ImpactEffectSlot {
    bool valid = false;
    std::size_t class_desc_offset = 0;
};

// 0084B8C0: __fastcall(ECX = mode, EDX = classDesc) with three stack arguments
// (RET 0Ch at 0084B8ED and every other exit). `medium` is the shot's
// vtable[2Ch], the same query that picks the static trace.
ImpactEffectSlot impact_effect_slot_0084b8c0(ProjectileImpactMode mode, int medium) noexcept;

// ---------------------------------------------------------------------------
// 0070C370's proximity search, 0070C4A4..0070C795.
// ---------------------------------------------------------------------------

// The flak's own fields, past the 284h a Bullet uses and inside the 298h a
// MFlakBullet allocates. 0070C370 reaches them through the tick element at
// projectile+244h, so the listing spells them [ESI+40h]..[ESI+50h].
inline constexpr std::size_t kFlakOffNearestSquared = 0x284;   // 0070C7C7
inline constexpr std::size_t kFlakOffTargetLatched = 0x288;    // 0070C661
inline constexpr std::size_t kFlakOffBurstDistance = 0x28C;    // 0070C6D5
inline constexpr std::size_t kFlakOffBurstDistanceBias = 0x290; // 0070C6C6
inline constexpr std::size_t kFlakOffTarget = 0x294;           // 0070C665

// Class ids the candidate filter passes to vtable[5Ch], in listing order.
inline constexpr int kFlakCandidateRequiredKind = 0x05;  // 0070C553
inline constexpr int kFlakCandidatePlaneKind = 0x0F;     // 0070C566
inline constexpr int kFlakCandidateAltKindE = 0x0E;      // 0070C575
inline constexpr int kFlakCandidateAltKindC = 0x0C;      // 0070C586

// 0070C50F..0070C526: the sphere is centred on the step's midpoint and reaches
// half the step plus twice the class blast radius, then is capped.
float flak_search_radius_squared_0070c50f(float step_length, float blast_radius) noexcept;

// 0070C553..0070C58C: kind 5 and then one of 0Fh, 0Eh or 0Ch.
bool flak_candidate_kind_passes(bool is_kind_5, bool is_kind_f, bool is_kind_e,
                                bool is_kind_c) noexcept;

// The mutable state the search carries across candidates and across ticks.
struct FlakProximityState {
    // Per-tick, the nearest squared distance found so far. Seeded FLT_MAX.
    float nearest_squared = kFlakNearestSeed;
    // projectile+288h / +294h / +28Ch / +290h.
    bool target_latched = false;
    void* target = nullptr;
    float burst_distance = 0.0f;
    float burst_distance_bias = 0.0f;
    // projectile+284h, the previous tick's nearest squared distance.
    float previous_nearest_squared = 0.0f;
};

// One virtual per native call site inside 0070C4A4..0070C795.
struct FlakProximityHost {
    virtual ~FlakProximityHost() = default;
    // 0070C52A: BSP_Recon_EnsureSlot([projectile+54h]), then the intrusive
    // list at slot+0DE8h. The list is the party's recon set, not every entity.
    virtual void* first_candidate() = 0;
    virtual void* next_candidate(void* cursor) = 0;
    virtual void* entity_of(void* cursor) = 0;
    // 0070C553..0070C58C, four vtable[5Ch] calls on the candidate.
    virtual bool candidate_is_kind(void* entity, int class_id) = 0;
    // 0070C592: refresh the candidate's pose when [entity+0C8h] is clear.
    virtual TickPoint3 candidate_world_position(void* entity) = 0;
    // 0070C677 and 0070C680. 00427EB0 takes no stack argument (plain RET),
    // so the four dwords pushed before it are 00901C20's; 00901C20 is
    // RET 10h. The pair is the ballistic lead solution, a contract.
    virtual TickPoint3 lead_solution_00901c20(void* entity, float muzzle_speed) = 0;
};

// 0070C4A4..0070C6F1: walk the party recon list, keep the nearest candidate
// inside the capped sphere, and record how far the round still has to fly
// before it is level with that candidate's predicted position.
void flak_proximity_search_0070c4a4(FlakProximityHost& host, FlakProximityState& state,
                                    const TickPoint3& midpoint,
                                    const TickPoint3& previous_position,
                                    const TickPoint3& step_direction,
                                    float search_radius_squared,
                                    float muzzle_speed) noexcept;

// 0070C705..0070C71E: with a target latched, the round bursts on the step that
// covers the remaining distance. 0070C7AD subtracts the step otherwise.
bool flak_should_detonate_0070c716(const FlakProximityState& state,
                                   float step_length) noexcept;

// 0070C7B6..0070C806: with no burst, the round still rolls for a late airburst
// once the miss distance has started growing again and is outside 50 units.
// The roll itself is 00BD2F10(0.0f, 100.0f) < 10.0f, left to the caller.
//
// Not a pure predicate: 0070C7D0 stores this tick's nearest squared distance
// into projectile+284h on the branch where it is still shrinking, so the state
// is updated here exactly as the listing does it.
bool flak_airburst_conditions_0070c7b6(FlakProximityState& state,
                                       float nearest_squared) noexcept;

}  // namespace bsp
