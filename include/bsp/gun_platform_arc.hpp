// Gun platform traverse arcs and the gun's fire-request chain.
//
// Packet cc2_gun_platform_arc. Every rule here is a hypothesis reconstructed
// from a read of the native body; the addresses in the comments are the
// evidence. docs/GUN_PLATFORM_ARC.md carries the rule tables.
//
// This header depends on bsp/gun_aiming.hpp for GunFiringArc, GunPlatformArcs,
// GunStepDeltas and the shared angle constants; nothing declared here repeats a
// name that header already owns.
#ifndef BSP_GUN_PLATFORM_ARC_HPP
#define BSP_GUN_PLATFORM_ARC_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/gun_aiming.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Gun instance offsets this packet established. The aiming header already owns
// +358h's neighbours; these are the fields the fire chain reads and writes.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kGunArcOffDestroyedLevel = 0x358; // 00729AA7, 007301A7, 007297B3
inline constexpr std::size_t kGunArcOffBarrelTimers = 0x414;   // 0072D130's countdown array
inline constexpr std::size_t kGunArcOffShotDecisions = 0x424;  // the 00730A20 thunk's this
inline constexpr std::size_t kGunArcOffBarrelIndex = 0x44c;    // 006FE160's ring position
inline constexpr std::size_t kGunArcOffBarrelDelayTime = 0x450; // 0072D130 counts it down
inline constexpr std::size_t kGunArcOffFireRequest = 0x454;    // 0072D2C0 writes, 0072D130 reads
inline constexpr std::size_t kGunArcOffFireTargetRef = 0x46c;  // 0072D23A; +14h of the pair at gun+458h
inline constexpr std::size_t kGunArcOffFireStagger = 0x478;    // 0072D2C0 seeds, 0072D130 spends
inline constexpr std::size_t kGunArcOffLastShotTime = 0x4a0;   // 008598B0's freshness stamp
inline constexpr std::size_t kGunArcOffLastShotPose = 0x4a4;   // float[6] copied by 008598B0
inline constexpr std::size_t kGunArcOffFireEffectFlag = 0x4d4; // 006FDC90, 006FDCD0
inline constexpr std::size_t kGunArcOffFireReleaseGate = 0x4d8; // 006FDCD0 only

// Platform record: the aiming header gives +3Ch and +40h; the insert routine
// adds the capacity word, which is what makes the triple a vector.
inline constexpr std::size_t kGunArcOffArcCapacity = 0x44; // 007F5A10

// The id the 0ADh message carries comes from this offset of the target ref.
inline constexpr std::size_t kGunArcOffTargetNetworkId = 0x174; // 0072D130

// Vtable slots this packet read.
inline constexpr std::size_t kGunVtableSlotShotDecision = 0x1d4;   // 00730A20 -> 0072F6E0
inline constexpr std::size_t kGunVtableSlotSnapshotFields = 0x188; // 004F17F0 / 00859A20
inline constexpr std::size_t kGunVtableSlotMuzzleOrigin = 0x1e0;   // 006E3DC0 / 006FE160
inline constexpr std::size_t kGunVtableSlotRecentShot = 0x1e4;     // 006E3DE0 / 008598B0
inline constexpr std::size_t kGunVtableSlotSetFireRequest = 0x1e8; // 0072D2C0 and overrides

// ---------------------------------------------------------------------------
// Constants the arc routines use that the aiming header does not carry.
// ---------------------------------------------------------------------------
// 00D08BA8 / 00D08BAC: the horizontal bound clamp 007F6B10 applies. Not pi: a
// shade inside it, so an authored 180 degree edge cannot land on the seam.
inline constexpr float kGunArcBoundLimit = 3.1414794921875f;
// 00D7A238 and 00D7A358: a span narrower than this is widened by this much.
inline constexpr float kGunArcMinimumSpan = 0.009999999776482582f;
inline constexpr float kGunArcSpanPadding = 0.009999999776482582f;
// 00D7A348: 008598B0 publishes a shot only this long after it happened.
inline constexpr float kGunArcRecentShotWindow = 0.25f;
// 00CE81A8: the random range 0072D2C0 seeds the fire stagger from.
inline constexpr float kGunArcFireStaggerMax = 0.11999999731779099f;

// ---------------------------------------------------------------------------
// 007F6840: the 007F5FC0 walk that hands back the window instead of a bool.
// Returns the index of the first traverse-enabled window containing the pair,
// or -1. 0085B0C5 uses only whether the answer exists.
// ---------------------------------------------------------------------------
std::ptrdiff_t gun_find_traverse_window_007f6840(const GunPlatformArcs& arcs,
                                                 float horz,
                                                 float vert) noexcept;

// ---------------------------------------------------------------------------
// 007F6530: the arc-aware step deltas.
//
// The shortest-path deltas are written first and are what the caller gets when
// the current horizontal angle sits in no traverse window at all. Otherwise the
// routine walks the window list from the current window in the direction of
// travel. Meeting a window with the traverse bit clear before reaching the one
// that holds the target negates the horizontal delta and reverses the walk, so
// the gun goes the long way round. It then clamps the vertical delta to the
// first window on that path whose elevation bounds exclude the wanted vertical
// angle, which is how a gun ducks under an obstruction while it traverses.
//
// `routed` reports whether the horizontal delta was negated. The native routine
// has no such output; it is here so a caller can assert the branch.
// ---------------------------------------------------------------------------
struct GunArcRouteOutcome {
    GunStepDeltas deltas{};      // the two out parameters at 0085AEB1 and 0085AEB8
    bool routed_around{false};   // 007F669F fired: the blocked-window reversal
    bool vertical_clamped{false}; // 007F6825 fired: an intervening window capped it
    bool current_window_known{false}; // false when the first walk found nothing
};
GunArcRouteOutcome gun_arc_route_deltas_007f6530(const GunPlatformArcs& arcs,
                                                 float horz,
                                                 float vert,
                                                 float target_horz,
                                                 float target_vert) noexcept;

// ---------------------------------------------------------------------------
// 0085B0F0's seam search, the part the aiming doc left unread. A replicated
// angle that no traverse window accepts is retried half a degree either side
// before the whole update is abandoned. Returns false when all three candidates
// miss, in which case 0085B0F0 returns without writing anything.
// ---------------------------------------------------------------------------
bool gun_snap_angle_into_window_0085b0f0(const GunPlatformArcs& arcs,
                                         float angle,
                                         float& accepted) noexcept;

// ---------------------------------------------------------------------------
// The producer. 007F5A10 treats the window list as a partition of the
// horizontal circle: an authored arc carves out its own span and the leftovers
// keep the surrounding window's flags and elevation bounds. The list must
// already hold a window that covers the new one - the native routine
// dereferences the search result without a null guard - so a caller seeds it
// with one full-circle window first.
// ---------------------------------------------------------------------------
void gun_split_insert_arc_007f5a10(std::vector<GunFiringArc>& windows,
                                   const GunFiringArc& arc);

// 007F6B10: normalise one authored arc and insert it. All-zero angles are
// ignored, a degenerate span is padded, both horizontal bounds are pulled
// inside +-pi, and a window that straddles the seam becomes two inserts.
void gun_add_authored_arc_007f6b10(std::vector<GunFiringArc>& windows, GunFiringArc arc);

// ---------------------------------------------------------------------------
// The fire chain's integration boundary. One virtual per native call site in
// 0072D2C0 and 0072D130, in the order the native code makes them. No defaults:
// none of this stands in for unrecovered behaviour.
// ---------------------------------------------------------------------------
struct GunFireRequestHost {
    virtual ~GunFireRequestHost() = default;

    // 0072D2DC: the owning unit, null when the gun is detached.
    virtual bool has_owning_unit() const = 0;
    // 0072D2E6 and 0072D2EC: unit+5Dh and gun+5Dh, either of which forces false.
    virtual bool unit_suppressed() const = 0;
    virtual bool gun_suppressed() const = 0;

    // 0072D311: BSP_Observer_UnregisterPair on the pair at gun+458h, after which
    // its subject field gun+46Ch is nulled at 0072D316.
    virtual void release_fire_target_ref_006952a0() = 0;
    // 0072D32C loads gun->vtable[5Ch]; the indirect call asks it about 23h.
    virtual bool is_rapid_fixed_slave_006e3d50(int class_id) = 0;
    // 0072D37D: the opcode 0AFh message, built at 0072D339 and sent through 0077C7B0.
    virtual void send_fixed_slave_fire_message_0077c7b0(bool want_fire) = 0;
    // 0072D3B3: FUN_00BD2F10(0, 0.12f), the stagger draw stored at 0072D3B8.
    virtual float random_stagger_00bd2f10(float lo, float hi) = 0;
    // 0072D3D3: the stop-firing tail on a falling edge.
    virtual void stop_firing_0072b4c0() = 0;

    // 0072D166 reads the reference at gun+41Ch and calls its own vtable slot 0
    // when the count at gun+420h has just reached zero; 0072D178 nulls it.
    virtual void release_effect_ref() = 0;
    // 0072D18C: FUN_0072AD40(dt), the base tick the gun runs first.
    virtual void base_tick_0072ad40(float dt) = 0;
    // 0072D21B: BSP_Gun_SetBarrelReloadTimer(index, value, 0) at 0072CF00.
    virtual void set_barrel_reload_timer_0072cf00(int index, float value) = 0;
    // 0072D1C1: unit+720h, the first gate on the send.
    virtual bool unit_fire_blocked() const = 0;
    // 0072D1CD: gun+3B8h, the same disable byte the step gate reads.
    virtual bool gun_disabled() const = 0;
    // 0072D290: BSP_Session_RouteMessage with the opcode 0ADh message built at 0072D25B.
    virtual void send_fire_message_0ad(std::uint16_t target_id) = 0;
};

// The gun's mutable fire state, the fields 0072D130 and 0072D2C0 move.
struct GunFireRequestState {
    bool fire_requested{false};  // +454h
    float fire_stagger{0.0f};    // +478h
    float barrel_delay_time{0.0f}; // +450h
    std::vector<float> barrel_timers{}; // +414h, +448h entries
    std::uint16_t target_network_id{0}; // [+46Ch]+174h, zero when there is no ref
    bool has_target_ref{false};
    int effect_ref_count{0};   // +420h, the count 0072D141 decrements
    bool holds_effect_ref{false}; // +41Ch is non-null
};

// 0072D2C0 over the host. Returns the latched value, which is not always the
// value asked for.
bool gun_set_fire_request_0072d2c0(GunFireRequestState& state,
                                   GunFireRequestHost& host,
                                   bool want_fire);

// 0072D130 over the host: the countdowns, then the conditional 0ADh send.
// Returns true when the message went out this step.
bool gun_fixed_step_tick_0072d130(GunFireRequestState& state,
                                  GunFireRequestHost& host,
                                  float dt);

// ---------------------------------------------------------------------------
// 006FE160: the salvo ring that displaces a muzzle origin. `basis_right` and
// `basis_up` are the two rows BSP_Matrix_BuildLookAt produces from the
// direction; the native routine derives them inside, which is a contract here.
// ---------------------------------------------------------------------------
struct GunRingOffsetInputs {
    float origin[3]{};      // the routine's second argument
    float basis_right[3]{}; // the look-at row scaled by cos
    float basis_up[3]{};    // the look-at row scaled by sin
    float radius{0.0f};     // descriptor+D0h
    int barrel_index{0};    // gun+44Ch
    int muzzle_count{1};    // (descriptor+A0h - descriptor+9Ch) / 0Ch
};
void gun_ring_muzzle_origin_006fe160(const GunRingOffsetInputs& in, float out[3]) noexcept;

} // namespace bsp

#endif // BSP_GUN_PLATFORM_ARC_HPP
