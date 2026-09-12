#pragma once
#include <cstddef>
#include <cstdint>

// Projection of the link between an airfield's hangar list and a plane's `Runway on Path`
// taxi state: the 12-byte hangar record `006D5220` builds, the two mirrored hangar searches
// `006D2780` / `006D2640`, the launch-site slot `006CF420` that turns the chosen path into a
// taxi target, the attach/detach rule of `007B8E80`, and the 4 <-> 5 switch the plane bot
// applies at `009CD7E1`.
//
// docs/AIRFIELD_TAXI.md carries the evidence, the coverage notes and the corrections. Every
// name here is a hypothesis, not a recovered symbol. Nothing in this header is a
// binary-compatible layout; the `kTaxi*` constants are the native offsets.
//
// Contracts named but not reconstructed: the air-operations update `006CDC70` that fires the
// launch (docs/AIR_OPERATIONS.md); the slot release `006C65B0` on landing (same doc); the
// ground law `007DCCF0` and the runway steering band `007DA380` that consume the state this
// switch selects (docs/PLANE_GROUND_OPS.md); the scene path sampler `007AF800`
// (docs/AVOID_ZONE_GEOMETRY.md); the plane-side state writers `007C16F0` / `007C1680`.

namespace bsp {

// ---------------------------------------------------------------------------
// Native offsets. Airfield unit fields (MAirfield), from 006D5220 and 006D3250.
// ---------------------------------------------------------------------------
inline constexpr int kTaxiHangarVectorBase = 0x830;  // Hangar*, heap allocated
inline constexpr int kTaxiHangarVectorSize = 0x834;  // uint32 count
inline constexpr int kTaxiHangarVectorCap = 0x838;   // uint32 capacity
inline constexpr int kTaxiParkSlotArray = 0x83C;     // six inline vec3, 006D3250
inline constexpr int kTaxiParkSlotCount = 0x884;     // the count 006D3250 randomises over
inline constexpr int kTaxiWorldMatrix = 0x0CC;       // the airfield's world transform
inline constexpr int kTaxiInverseWorldMatrix = 0x110;// its cached inverse, latched by +10Ch
inline constexpr int kTaxiInverseLatch = 0x10C;      // byte; 0 means rebuild the inverse

// Launch-site object fields (vtable 00CF89F8), from 006CF190/006CF3E0/006CF5B0/006CF9F0.
inline constexpr int kTaxiSiteObserverNode = 0x04;   // the embedded observer pair
inline constexpr int kTaxiSiteReadyPlaneWatch = 0x18;// the observed target, cleared on placement
inline constexpr int kTaxiSiteSurfaceBase = 0x20;    // the +28h slot thunks by this bias
inline constexpr int kTaxiSiteOccupancyBase = 0x34;  // unit** of the planes on the site
inline constexpr int kTaxiSiteOccupancyCount = 0x38; // its count
inline constexpr int kTaxiSiteOwnerUnit = 0x44;      // the MAirfield unit

// Air-operations block fields this packet adds to docs/AIR_OPERATIONS.md.
inline constexpr int kTaxiBlockLaunchSite = 0x3C;    // the launch-site object
inline constexpr int kTaxiBlockOwnerUnit = 0x7C;     // already documented; repeated for clarity
inline constexpr int kTaxiBlockContactHolder = 0x80; // the holder 007C5F60 hands to 007B8E80

// Plane unit fields.
inline constexpr int kTaxiPlaneContactHolder = 0xBF4;// 007B8E80's only write target
inline constexpr int kTaxiPlaneContactFlag = 0xBF8;  // byte; non-zero while attached
inline constexpr int kTaxiPlaneQueueIndex = 0x9D8;   // int; spaces the plane along the entry path
inline constexpr int kTaxiPlaneLocalX = 0x0A4;       // the translation triple the taxi law reads
inline constexpr int kTaxiPlaneLocalY = 0x0A8;
inline constexpr int kTaxiPlaneLocalZ = 0x0AC;
inline constexpr int kTaxiPlaneWorldPos = 0x0FC;     // 006CF420's fallback target

// Class-description and tuning fields the taxi law shares with the runway steering band.
inline constexpr int kTaxiClassYawSpd = 0x1B0;       // classDesc+1B0h YawSpd
inline constexpr int kTaxiClassQueuePitch = 0x158;   // classDesc+158h, the queue spacing base
inline constexpr int kTaxiTuningFloor = 0x188;       // tuning+188h, the rate floor
inline constexpr int kTaxiTuningYawBlend = 0x2A8;    // tuning+2A8h
inline constexpr int kTaxiTuningYawSpdMul = 0x2B0;   // tuning+2B0h RunwayYawTurnSpdMul

// The hangar-liveness byte quartet 006D2730 and 007C16F0 both test.
inline constexpr int kTaxiEntityPresent = 0x5C;      // byte; must be set
inline constexpr int kTaxiEntityOutOfAction = 0x5D;  // byte; must be clear
inline constexpr int kTaxiEntityFlag5E = 0x5E;       // byte; must be clear
inline constexpr int kTaxiEntityFlag60 = 0x60;       // byte; must be clear

// The hangar-usability float 006D2640 gates on; 006D2780 does not read it.
inline constexpr int kTaxiHangarCondition = 0x370;

// The sentinels the two searches seed their extreme with.
inline constexpr float kTaxiEntryPathSentinel = -9999.0f;  // 00CF8E44
inline constexpr float kTaxiExitPathSentinel = 9999.0f;    // 00CE4C04

// The separation radius 006CF5B0 refuses a spot inside.
inline constexpr float kTaxiSpotSeparation = 30.0f;  // 00CE7630

// The numerator of the required-rate quotient at 009CD787.
inline constexpr float kTaxiRequiredRateNumerator = 1.5f;  // 00CE3D78

// ---------------------------------------------------------------------------
// The record 006D5220 builds. Three pointers, stride 0Ch. The two path fields
// are already path interfaces: 006D5220 runs every authored reference through
// 007AC9D0 BSP_Entity_PathInterfaceForKind before it stores them.
// ---------------------------------------------------------------------------
struct AirfieldHangarRecord {
    const void* object = nullptr;      // +0h  the hangar building entity
    const void* entry_path = nullptr;  // +4h  "EntryPath" / "entryPathID"
    const void* exit_path = nullptr;   // +8h  "ExitPath"  / "exitPathID"
};

// One hangar as the two searches see it: the record plus the two facts they read
// off the object, so the selection rule can be expressed without an entity host.
struct AirfieldHangarCandidate {
    AirfieldHangarRecord record;
    bool object_present = false;  // record.object != nullptr
    float condition = 0.0f;       // object+370h; 006D2640 needs this above zero
    float local_z = 0.0f;         // the object's position in the airfield's local frame
};

// Which of the two searches a caller wants.
enum class HangarPathKind {
    Entry,  // 006D2780: maximum local Z, no condition gate, returns record.entry_path
    Exit,   // 006D2640: minimum local Z, condition > 0, returns record.exit_path
};

// The outcome of a search.
struct HangarPathPick {
    bool found = false;
    std::size_t index = 0;          // the winning record's index in the vector
    const void* path = nullptr;     // entry_path or exit_path per the kind
};

// The four-byte liveness quartet, in the order 006D2730 and 007C16F0 test it.
struct EntityLivenessBytes {
    bool present = false;        // +5Ch must be set
    bool out_of_action = false;  // +5Dh must be clear
    bool flag_60 = false;        // +60h must be clear
    bool flag_5e = false;        // +5Eh must be clear
};

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 006D2730 BSP_AirField_FindUsableHangarObject's per-object test, and the same
// quartet 007C16F0 applies to the plane at 007C1776-007C178C.
bool entity_is_usable_006d2730(const EntityLivenessBytes& bytes);

// 006D2780 and 006D2640. One routine because the two bodies are mirror images:
// the same walk, the same local-Z projection, opposite extremes, opposite
// sentinels, opposite record fields, and a condition gate on the exit search only.
HangarPathPick pick_hangar_path_006d2780_006d2640(const AirfieldHangarCandidate* hangars,
                                                  std::size_t count,
                                                  HangarPathKind kind);

// The point index 006CF420 asks the scene path for: the vector at path+8h..+0Ch
// holds one pointer per point, so the count is the byte span >> 2 and the index
// used is that minus one. A path with no points yields -1, which is what the
// native code would hand the sampler; the caller must treat it as no target.
int taxi_target_point_index_006cf420(std::ptrdiff_t path_points_begin,
                                     std::ptrdiff_t path_points_end);

// 009CD752-009CD7A0: the yaw rate the taxi law needs before it will give up free
// steering. `max_by_ref` mirrors 00415550 BSP_Math_MaxFloatByRef.
float required_yaw_rate_009cd752(float class_yaw_spd,
                                 float tuning_yaw_spd_mul,
                                 float tuning_rate_floor,
                                 float tuning_yaw_blend);

// 009CD7E1-009CD802: the whole 4 <-> 5 decision. `flight_state` is unit+900h and
// `needs_path` is the BL the rate test produced. Returns the state to write, or
// the current state when the native code writes nothing.
enum class TaxiStateAction {
    None,        // CL == BL: leave unit+900h alone
    JoinPath,    // 007C16F0: 4 -> 5
    LeavePath,   // 007C1680: 5 -> 4
};
TaxiStateAction taxi_state_action_009cd7e1(int flight_state, bool needs_path);

// 007B8E80's no-op test, kept separate because the side effects below depend on it.
bool contact_site_changes_007b8e80(const void* current_holder, const void* new_holder);

// ---------------------------------------------------------------------------
// The host. One method per native call site of the sequences below.
// ---------------------------------------------------------------------------
class AirfieldTaxiHost {
public:
    virtual ~AirfieldTaxiHost() = default;

    // --- 007B8E80 BSP_Plane_SetGroundContactSite ---------------------------
    // 006952A0 BSP_Observer_UnregisterPair(ECX = (holder+4)->+7Ch, EDX = plane+10h).
    virtual void unregister_observer_006952a0(const void* owner_unit) = 0;
    // 00694A60 BSP_Observer_RegisterPair, the same pair.
    virtual void register_observer_00694a60(const void* owner_unit) = 0;
    // byte ((holder+4)->+3Ch)+1Ch = 1 at 007B8EB4.
    virtual void mark_site_released_007b8eb4(const void* launch_site) = 0;
    // plane+BF4h = holder at 007B8ECA.
    virtual void store_contact_holder_007b8eca(const void* holder) = 0;
    // holder+4h, then block+7Ch and block+3Ch.
    virtual const void* block_of_holder(const void* holder) const = 0;
    virtual const void* owner_unit_of_block(const void* block) const = 0;
    virtual const void* launch_site_of_block(const void* block) const = 0;

    // --- 009CD540 BSP_PlaneBot_TaxiStep ------------------------------------
    virtual int plane_flight_state() const = 0;            // unit+900h
    virtual const void* plane_contact_holder() const = 0;  // unit+BF4h
    virtual bool owner_out_of_action() const = 0;          // byte owner+5Dh
    // 007B9000 then 007C1680 at 009CD59E / 009CD5A9.
    virtual void run_surface_release_007b9000() = 0;
    virtual void leave_path_007c1680() = 0;
    virtual void join_path_007c16f0() = 0;
    // (task+4)+18h writes at 009CD5B8-009CD5E2.
    virtual void request_neutral_controls_009cd5b8() = 0;
    // site->vtable[2Ch](out, plane) = 006CF420. Returns false when the site had
    // no usable hangar and fell back to the plane's own position.
    virtual bool taxi_target_006cf420(float out_xyz[3]) = 0;
    // the plane's translation triple at +A4h / +ACh.
    virtual float plane_local_x() const = 0;
    virtual float plane_local_z() const = 0;
    // site->vtable[34h](plane, 1, {x, y}) = 006CF5B0, at 009CD98E.
    virtual bool spot_is_clear_006cf5b0(float x, float y) = 0;
    // the final requests at 009CDC48-009CDC7F.
    virtual void write_steer_request_009cdc4b(float yaw) = 0;
    virtual void write_speed_request_009cdc70(float speed) = 0;
    // byte task+18h = 1, the task-failed flag.
    virtual void fail_task_009cd583() = 0;

    // --- 006CF9F0 / 007C5F60, the launch handoff ---------------------------
    virtual const void* site_owner_unit() const = 0;          // site+44h
    virtual bool block_owner_alive() const = 0;               // unit+7A8h, byte +5Dh
    virtual void pose_on_queue_006cf730() = 0;                // block+3Ch ->vtable[40h]
    virtual void attach_contact_holder_007c6242() = 0;        // 007B8E80(plane, *(block+80h))
    virtual void set_flight_state_locked_007c627c() = 0;      // plane+900h = 2, then 007C11E0
    virtual void run_pre_pass_007c5ac0() = 0;                 // 007C5AC0 at 007C6253
    virtual void arm_after_place_007c3c90() = 0;              // 007C3C90(plane, 1)
    virtual void invalidate_subtree_pose_0042ed50() = 0;      // 0042ED50
    virtual void notify_plane_placed_006cfa41() = 0;          // (plane+310h)->vtable[8](0.0f)
    virtual void clear_ready_plane_watch_006cfa4f() = 0;      // 006952A0 then site+18h = 0
    virtual void rebuild_slot_lists_008073c0() = 0;           // 008073C0
};

// ---------------------------------------------------------------------------
// Sequences.
// ---------------------------------------------------------------------------

// 007B8E80 BSP_Plane_SetGroundContactSite, complete.
void set_ground_contact_site_007b8e80(AirfieldTaxiHost& host,
                                      const void* current_holder,
                                      const void* new_holder);

// Why 009CD540 stopped, so a caller can tell the three exits apart.
enum class TaxiStepOutcome {
    Failed,        // 009CD583: not in state 5, or no holder
    DroppedOff,    // 009CD59E: the owner is gone; 007B9000 then 007C1680
    Drove,         // the law ran and wrote both requests
};

// 009CD540 BSP_PlaneBot_TaxiStep, structure only. The yaw and speed magnitudes
// are the host's: the native x87 arithmetic between 009CD6AB and 009CDC48 was
// not reduced, so this routine reproduces the gates, the order of the calls and
// the 4 <-> 5 switch, and passes the host's own magnitudes through.
TaxiStepOutcome taxi_step_009cd540(AirfieldTaxiHost& host, float yaw_request, float speed_request);

// 006CF9F0 BSP_AirOpsSite_PlacePlaneOnSpot together with the part of 007C5F60
// that this packet read. Returns false when the guard at 006CF9F9-006CFA0E
// refused, in which case only the watch is cleared.
bool place_plane_on_spot_006cf9f0(AirfieldTaxiHost& host);

}  // namespace bsp
