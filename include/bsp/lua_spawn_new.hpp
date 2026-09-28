#pragma once
// Packet cc8_spawn_new_route. The LIVE side of `SpawnNew`: the request the Lua
// table becomes, the manager's queue at *(00F89B3C), and the one-per-interval
// drain that GGame::OnMove runs.
//
// include/bsp/lua_binding_spawn.hpp already carries the record layout and the
// pure binding contract from packet cc_lua_core; this header does not repeat a
// constant that lives there. What is new here is the consumer, which that
// packet recorded as open ("Nothing in this packet reads the queue back").
//
// The route, from the listing:
//
//   0094C480 SpawnNew          PUSH ECX / MOV ECX,[00F89B3C] / CALL 00949750
//   00949750                   parses one Lua table, allocates DCh bytes and
//                              links the record onto manager+4h. Creates nothing.
//   004E534F  (BSP_Game_OnMove, OnMove step 20 "World tick")
//             CALL 0094C8F0    = `CALL 0094C490; RET 4`. The scaled delta is
//                              pushed and 0094C490 never reads it: 0094C490 is
//                              __fastcall(ECX = manager) and its own clock is
//                              the world time at DAT_00F876A4.
//   0094C490                   the drain, below.
//   0094A140                   __thiscall(record): builds a candidate frame and
//                              retries it by rotation until 00949300 accepts one.
//   00949300                   __thiscall(record, frame, block): per member,
//                              composes the member frame and asks 00941D30
//                              whether that placement is legal; when EVERY
//                              member passes it calls 009483D0 once.
//   009483D0                   __thiscall(record, frame) (`RET 4` at 009487C3):
//                              creates every member, appends each created entity
//                              to the vector at record+CCh, and at its tail
//                              stores `*(record+C0h) = 1` (009487AD
//                              `MOV byte ptr [EBX + 0xc0],0x1`, EBX = the record).
//
// So +C0h is the FULFILLED flag, not "always zero": 00948CC0 clears it at
// construction and 009483D0 sets it. 0094C490 reads it back at 0094C5A7 and,
// when it is still clear, jumps to 0094C802 and pushes the record back onto the
// queue through 009478B0 (the same 00943C00 node allocate-and-link the enqueue
// uses, ECX = the manager). An unsatisfiable request is therefore RETRIED every
// interval, never dropped, and never answered.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The drain's clock
// ---------------------------------------------------------------------------

// 0094C4D1..0094C4F1: ST0 = `*(float*)(globalConfig + 2DCh) + *(float*)(manager
// + 0Ch)` over ST1 = `DAT_00F876A4`, then FCOMIP and `JA` return. So the drain
// proceeds only when `now >= interval + lastAttempt`: one request is attempted
// per interval, and manager+0Ch is the stamp of the last attempt
// (0094C59D MOVSS [EDI+0Ch],XMM0).
inline constexpr std::size_t kSpawnManagerLastAttemptOffset = 0xC;
inline constexpr std::size_t kGlobalConfigSpawnAttemptDelayOffset = 0x2DC;
// The image default, 0087F7E7 `FLD float ptr [00CE74F8]`, bytes cd cc 4c 3f.
// It is a float32 at that address, loaded by a float FLD, not a double.
inline constexpr float kSpawnAttemptDelayDefault = 0.8f; // 00CE74F8
// `Globals["SpawnAttemptDelay"]` (the literal at 00D0E1F8) overrides it:
// 0087F7D1 PUSH 0xd0e1f8 ... 0087F800 FSTP dword ptr [ESI + 0x2dc].
// This installation's scripts/datatables/globals.lua line 201 sets 0.5.
inline constexpr const char* kSpawnAttemptDelayGlobalsKey = "SpawnAttemptDelay";

// ---------------------------------------------------------------------------
// Record offsets this packet establishes or corrects
// ---------------------------------------------------------------------------

// CORRECTION to include/bsp/lua_binding_spawn.hpp, settled by the consumer.
//
// `kSpawnRequestFlagByteOffset = 0xC0` was recorded as "always 0": it is the
// fulfilled flag (009487B9 `MOV byte ptr [ESI + 0xC0],1`, read at 0094C5A7).
//
// `kSpawnRequestPartyOffset = 0xC4` / `kSpawnRequestPlayerOffset = 0xC8` name
// the wrong field. The party is +80h, because 0094C532/0094C542 use it as the
// index into the party table at `game+18CCh + party*4` and 0094C7D9 passes it
// in ECX to the completion callback. +C4h is that callback's function pointer
// and +C8h its context word; 0094C777 `CMP dword ptr [ESI + 0xC4],0` skips the
// whole completion walk when it is null, which a raw party index could not do.
inline constexpr std::size_t kSpawnRequestFulfilledOffset = 0xC0;   // 009487AD
inline constexpr std::size_t kSpawnRequestPartyIndexOffset = 0x80;  // 0094C7D9
inline constexpr std::size_t kSpawnRequestCompletionFnOffset = 0xC4;  // 0094C7D2
inline constexpr std::size_t kSpawnRequestCompletionCtxOffset = 0xC8; // 0094C7CA

// The two ranges on the record, settled by the consumer rather than by tracing
// the binding's staged floats (which did not close - see the doc's "Open").
// 0094A1Cx reads record+68h and record+6Ch as floats and forms
// `(record+6Ch + record+68h) * 0.5` with `0094A30A FLD double ptr [00D7A280]`,
// whose eight bytes are `00 00 00 00 00 00 e0 3f` = 0.5 - a double, by the load
// instruction - and `record+6Ch - record+68h`, then feeds the result to
// BSP_Matrix_BuildRotationY. A midpoint and a half-width turned into a yaw are
// an ANGLE range, so +68h is `angleRange[1]` and +6Ch is `angleRange[2]`.
// record+70h and +74h are compared low-against-high in the same block and +70h
// then divides a lateral offset to give an angle, which is a DISTANCE, so +70h
// is the clamped `distRange[1]` and +74h is `distRange[2]`. record+78h is
// compared as an INT against an entity's +54h, the party field, so it is not a
// float at all - which corrects lua_binding_spawn.hpp's "+74h, +78h two floats".
inline constexpr std::size_t kSpawnRequestAngleLowOffset = 0x68;
inline constexpr std::size_t kSpawnRequestAngleHighOffset = 0x6C;
inline constexpr std::size_t kSpawnRequestDistLowOffset = 0x70;
inline constexpr std::size_t kSpawnRequestDistHighOffset = 0x74;

// The created entities. 0094C6BF/0094C74B `LEA EDI,[ESI + 0xCC]` then the
// checked begin/end pair 00645C40/00645C70, walked with `ADD EDI,4`: a
// std::vector<Entity*> whose proxy is +CCh and whose three pointers are
// +D0h/+D4h/+D8h, which is what lua_binding_spawn.hpp records as "zeroed".
// 009483D0 appends to it as each member is made: 00948742 `MOV dword ptr
// [EAX],ESI` / `ADD EAX,4` / `MOV [EDI+8],EAX` with EDI = record+CCh is the
// in-place arm, and 00948766 `CALL 00647A10` the reallocating one.
inline constexpr std::size_t kSpawnRequestCreatedVectorOffset = 0xCC;

// Per-member formation offsets, a std::vector<float[3]> at +90h/+94h: the count
// check at 00949356 divides `(+94h - +90h)` by 0xC, and 009483D0 reads element
// `i` for member `i` at 00948440.
inline constexpr std::size_t kSpawnRequestMemberOffsetsBeginOffset = 0x90;
inline constexpr std::size_t kSpawnRequestMemberOffsetsEndOffset = 0x94;

// One 10h-byte group-member element, read by 00949300 and 009483D0:
//   +0h  the vehicle class object; slot 18h of its vtable is a kind test
//        (00948510 / 00948519) and slot 28h constructs the instance
//        (00948522 / 00948529)
//   +8h  a char* name, defaulted to the empty NativeString data at 00F89B40,
//        memcpy'd into the created entity's +154h/+158h string
//   +Ch  one dword read into the property-bag argument
inline constexpr std::size_t kSpawnMemberClassOffset = 0x0;
inline constexpr std::size_t kSpawnMemberNameOffset = 0x8;
inline constexpr std::size_t kSpawnMemberWordOffset = 0xC;

// 009483D0's two arms, with the branch sense taken from the bytes:
// `0094851B TEST AL,AL` / `0094851D JZ 0x00948533`. AL == 0 (NOT kind 6) jumps
// to `PUSH 0x414`, the plane-squadron allocation whose constructor is
// `00948563 CALL 007F2C60 BSP_PlaneSquadronTickableEntity_Construct`; AL != 0
// (kind 6) falls through to the class's own `vtable+28h(0)` at 00948529. So
// kind 6 is the surface arm and everything else spawns as a plane squadron.
inline constexpr int kSpawnMemberKindSurface = 6;
inline constexpr std::size_t kPlaneSquadronTickableEntityBytes = 0x414;

// The serial the created entity carries: 009485A4 `MOV dword ptr [ESI + 0x28c],EAX`
// with ESI the created entity and EAX loaded from record+7Ch.
inline constexpr std::size_t kSpawnedEntitySerialOffset = 0x28C;

// CORRECTION to include/bsp/lua_binding_spawn.hpp's range comment, three ways.
//   - The defaults 200.0f/2500.0f (00D19908/00D1990C) belong to `distRange`,
//     not `angleRange`: they are loaded at 00949D69 and 00949D77, AFTER the
//     `distRange` key push at 00949D50 and inside its absence arm.
//   - Both ranges are read at Lua indices 1 and 2 (00949CDA `PUSH 0x1`,
//     00949D19 `PUSH 0x2`, 00949D9E, 00949DD4), not "slot 0 then slot 1".
//   - `angleRange` has no absence test and no default at all; only `distRange`
//     does (00949D95 CALL 00B65FB0 / 00949D9C JNZ keeps the defaults), and only
//     its FIRST element is clamped: 00949E0A loads 10.0f from 00CE38B8,
//     00949E12 COMISS against it and 00949E21 JA selects the constant, i.e.
//     `distLow = max(10.0f, distRange[1])`.
inline constexpr float kSpawnNewDistRangeDefaultLow = 200.0f;   // 00D19908
inline constexpr float kSpawnNewDistRangeDefaultHigh = 2500.0f; // 00D1990C
inline constexpr float kSpawnNewDistRangeLowMinimum = 10.0f;    // 00CE38B8

// ---------------------------------------------------------------------------
// The live request
// ---------------------------------------------------------------------------

// One `groupMembers` entry. The six keys are the ones every call site in this
// installation's mission scripts writes; `Type` is a raw vehicle-class index
// (usn_19_coralus.lua lines 104-108 assign 150/158/159/162 directly).
struct SpawnNewGroupMember {
    std::int32_t type_class_id{0};
    std::string name;
    std::int32_t crew{0};
    std::int32_t race{0};
    std::int32_t wing_count{0};
    std::int32_t equipment{0};
    // The member class's `Length` (+A0h) and `Width` (+A4h), which 00948CC0
    // reads through the element's class object. Filled by the host when it can
    // resolve the class; without them 00948CC0's offsets take both as 0.
    bool has_class_extents{false};
    float class_length_a0{0.0f};
    float class_width_a4{0.0f};
};

// `excludeRadiusOverride`. The record keeps one float of this block at +ACh,
// clamped to zero-or-greater and used as a radius (00948F1x). The five keys are
// read here because the scripts write all five, but this process tests none of
// them: see the deviation note on drain_spawn_request_queue_0094c490.
struct SpawnNewExcludeRadius {
    bool present{false};
    float own_horizontal{0.0f};
    float enemy_horizontal{0.0f};
    float own_vertical{0.0f};
    float enemy_vertical{0.0f};
    float formation_horizontal{0.0f};
};

struct SpawnNewRequest {
    std::int32_t party{-1};      // `party`, record+80h
    bool player{false};          // `player`
    std::string callback;        // `callback`, record+84h/+88h
    std::string id;              // `id`, record+B8h/+BCh
    std::uint32_t serial{0};     // record+7Ch, from the counter at 00E0CF74
    std::vector<SpawnNewGroupMember> members;  // record+4h/+8h

    bool has_ref_pos{false};
    float ref_pos[3]{0.0f, 0.0f, 0.0f};  // `area.refPos`
    // Packet cc9_spawn_new_shipyard (kSpawnNewEntityRefPosBound). `refPos` was an
    // entity table (00949B60 008889C0 answered yes): the record keeps the entity
    // (008F8530 stores it at ref+14h) and 008F8680 hands out that entity's own
    // world matrix at +CCh whenever the frame is asked for. `ref_entity_id` is
    // this process's entity number for it; `ref_frame` is the pose the drain
    // resolved (rows 0..2 the basis, row 3 the translation), used while
    // `ref_frame_valid`.
    std::int32_t ref_entity_id{0};
    bool ref_frame_valid{false};
    float ref_frame[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    bool has_angle_range{false};
    float angle_low{0.0f};       // `area.angleRange[1]`, radians
    float angle_high{0.0f};      // `area.angleRange[2]`, radians
    float dist_low{kSpawnNewDistRangeDefaultLow};
    float dist_high{kSpawnNewDistRangeDefaultHigh};
    bool has_look_at{false};
    float look_at[3]{0.0f, 0.0f, 0.0f};  // `area.lookAt`
    SpawnNewExcludeRadius exclude{};

    // Filled by the drain, mirroring record+CCh and record+C0h.
    std::vector<std::uint32_t> created;
    bool fulfilled{false};
    unsigned attempts{0};
};

// 00949F2B..00949F47, the process-wide counter at 00E0CF74: it hands out the
// current value and post-increments, and a value above 16000 restarts at 1, so
// 16001 is never issued and two missions in one session do not restart it.
std::uint32_t next_spawn_request_serial_00949f2b() noexcept;
void reset_spawn_request_serial_for_tests() noexcept;

// The manager at *(00F89B3C): a 10h-byte object whose fields are the list
// sentinel (+4h), the element count (+8h) and the last-attempt stamp (+0Ch).
// Modelling the list as a vector keeps the two operations the drain needs -
// erase at a chosen position and push at the back - and nothing in the route
// depends on node identity.
class SpawnRequestQueue {
public:
    void enqueue_00949530(SpawnNewRequest request);
    // 009478B0: the record goes back at the END of the list, which is why a
    // request that cannot be placed yields to the ones behind it.
    void requeue_009478b0(SpawnNewRequest request);
    std::size_t size() const noexcept { return requests_.size(); }
    bool empty() const noexcept { return requests_.empty(); }

    // 0094C4D1..0094C4F1: `now >= last_attempt + interval`.
    bool attempt_due(float now, float interval) const noexcept;
    void stamp_attempt(float now) noexcept { last_attempt_ = now; }
    float last_attempt() const noexcept { return last_attempt_; }

    // 0094C4FA..0094C56B. With one record or none the drain takes the head
    // without looking at it. With two or more it walks the list for the first
    // record whose party is ACTIVE - `game+18CCh + party*4` with `+8h != 0` and
    // `+9h == 0` - and falls back to the head when the walk runs off the end.
    // `party_active` is indexed by party; an index outside it is not active,
    // which is the `CMP EAX,EBX / JL` guard at 0094C538 for a negative party.
    std::size_t select_0094c508(const std::vector<bool>& party_active) const noexcept;

    SpawnNewRequest erase_009439b0(std::size_t index);
    const SpawnNewRequest& at(std::size_t index) const { return requests_[index]; }
    const std::vector<SpawnNewRequest>& requests() const noexcept { return requests_; }
    // 00945850 / 00945A20, the other two thunks. Kept here because they are the
    // same list and the same id field at +B8h.
    bool id_is_requested_00945850(const std::string& id) const noexcept;
    std::size_t remove_id_00945a20(const std::string& id);
    void clear() noexcept;

private:
    std::vector<SpawnNewRequest> requests_;
    float last_attempt_{0.0f};
};

// One per process, as the singleton is.
SpawnRequestQueue& spawn_request_queue();

// The id compare both scans make: equal length, then __stricmp (00BF7FBF).
bool spawn_request_id_matches(const std::string& record_id, const std::string& id) noexcept;

// The drain needs the mission Lua host - it creates through that host's
// script-orders host and answers through that host's `thisTable` - but the
// frame step that runs it is the world walk, which cannot reach that host.
// 0094C8F0 has the same shape: BSP_Game_OnMove reaches the manager through a
// process global rather than through anything it owns. So the host registers
// itself here when it is attached and the world walk calls the free function.
class SpawnQueueDrain {
public:
    virtual ~SpawnQueueDrain() = default;
    virtual void run_spawn_queue_0094c490(float step_seconds) = 0;
};
void set_spawn_queue_drain(SpawnQueueDrain* drain) noexcept;
// 004E534F's call, reduced to the part 0094C490 actually uses. Does nothing
// when no host is attached, which is the null-manager arm at 0094C4B8.
void run_spawn_queue_step_0094c8f0(float scaled_delta);

// The frame 0094A140 hands 00949300, reduced to what this process can justify:
// a position and a heading. The native composes a full 4x4 and rotates it
// through a bounded retry; only the first candidate is reproduced here, because
// the retry exists to dodge an occupancy test this process does not run.
struct SpawnNewFrame {
    float position[3]{0.0f, 0.0f, 0.0f};
    float yaw{0.0f};  // radians
    // kSpawnNewPlacementBound: no candidate of 0094A140's search passed
    // 00941D30, so 00949300 creates nothing and the record stays queued.
    bool refused{false};
};

// The first candidate frame for `member` of `request`. The angle is taken
// across `angleRange` and the distance across `distRange`, both by member
// index, so a four-member group fans out rather than stacking. CONTRACT, not a
// reading: 0094A140's own sampling was not decoded.
SpawnNewFrame spawn_member_frame_0094a140(const SpawnNewRequest& request,
                                          std::size_t member) noexcept;
// The fan-out contract itself, which the function above runs unless the
// member-offset binding applies.
SpawnNewFrame spawn_member_frame_0094a140_contract(const SpawnNewRequest& request,
                                                   std::size_t member) noexcept;

// Packet cc9_plane_follow_pitch_flip (docs/SCENE_CONTENTS_HOSTS.md section 22);
// ON by the USN13, USN04, USN01 and USN02 pairs (22.5).
// With the switch on, a request whose `excludeRadiusOverride.formationHorizontal`
// is positive places its members the way the image does: ONE group frame (the
// first candidate of 0094A140, mid-angle, low distance, facing `lookAt`) and
// per-member offsets from the record constructor 00948CC0, which 00949300
// composes with that frame (BSP_Matrix_Multiply4x4 inside its member loop) and
// 009483D0 reads back at 00948440. Off, the fan-out contract above runs.
inline constexpr bool kSpawnNewMemberOffsetsBound = true;

// 00948CC0's member-offset loop (00948E56-009492C0), in the record frame
// (x right, z forward). Members go in rows of three, row r holding members
// 3r (A, centre), 3r+1 (B) and 3r+2 (C):
//   lateral L = max(fH, (A.A4 + B.A4) * 0.5, (A.A4 + C.A4) * 0.5) * 2.5 + 5
//   row gap G = 5 + max(fH, half sums of +A0h with the previous row) * 1.5
//   z_r = z_(r-1) - G (row 0 at z = 0)
//   full row: A (0, 0, z), B (-L, 0, z), C (+L, 0, z); a row of one: A only;
//   a row of two (B, no C): (-L/2, 0, z), (+L/2, 0, z).
// fH is record+ACh = `formationHorizontal`, clamped at 0 (00948EA7; the key
// order is 009481A0's: own/enemy horizontal, own/enemy vertical, formation
// horizontal at block+10h, strings 00D19968..00D19928). Constants: 0.5 double
// [00D7A280], 2.5 double [00CE3DE0], 5.0 double [00D7A370], 1.5 double
// [00CE3D78].
// SUBSTITUTION, labelled: the class +A0h/+A4h extents are not carried into
// the request, so they are taken as 0 here; the result is the image's exactly
// while fH is at least every half sum, which holds for aircraft classes under
// the 100 and 500 the reference missions author. Returns false (no offset)
// when fH <= 0, where the class extents would decide.
bool spawn_member_offset_00948cc0(const SpawnNewRequest& request, std::size_t member,
                                  float offset[3]) noexcept;

// ---------------------------------------------------------------------------
// Packet cc9_spawn_new_placement (docs/SCENE_CONTENTS_HOSTS.md section 23).
// With the switch on, the group frame is 0094A140's own and the members pass
// through 00941D30 with 0094A140's retry. Off, section 22's frame runs.
// ---------------------------------------------------------------------------
inline constexpr bool kSpawnNewPlacementBound = true;   // 23.5 and 23.6

// ---------------------------------------------------------------------------
// Packet cc9_spawn_new_shipyard (docs/LUA_BINDING_MISSION.md, "SpawnNew with an
// entity refPos and a surface group"). Two image rules the drain lacked, which
// together are why JM05's shipyard requests never placed:
//  - `refPos` as an entity: 00949B60 CALL 008889C0 (a table whose `Ptr` is an
//    entity, [Ptr]->vtable+5Ch(1)) takes 00949B70 CALL 00888AA0 and 00949B7D
//    CALL 008F8530, which keeps the entity at ref+14h; 008F8680 then returns
//    entity+CCh (after 00414DB0 when the +C8h clean byte is clear), the
//    entity's world matrix, as the reference frame 0094A140 rotates. Off, the
//    host reads `refPos` only as an {x,y,z} table and an entity refPos leaves
//    the record without one, so the drain requeues it forever.
//  - the surface arm of 009483D0: a member whose class answers
//    vtable+18h(6) (the eight ship leaves, 00963B70 Destroyer .. 00963F10
//    MotherShip, all compare 6) is made by the class's own vtable+28h(0)
//    (00948529), not as a plane squadron, and a surface member after the
//    first asks 0077C8D0 to join the first member's formation (009486DA,
//    0094870E). Off, every member is made as PlaneSquadronGen.
// ---------------------------------------------------------------------------
inline constexpr bool kSpawnNewEntityRefPosBound = true;   // ON by the JM05 pair (LUA_BINDING_MISSION)

// A row-major 4x4: rows 0/1/2 right/up/forward, row 3 the translation, which
// is the order BSP_Matrix_Multiply4x4 (00413920) and 00B646E0 use.
struct SpawnGroupFrame {
    float m[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

// The reference frame record+10h hands 0094A140 through 008F8680:
//  - a position-table `refPos` is the identity basis at that point
//    (00949BC7..00949C5F build it, 008F84D0 stores it at object+18h);
//  - `lookAt` then writes lookAt - refPos into row 2, WITH its vertical part
//    (00949E82..00949EE0), and orthonormalises through 0085DC80: row 1 is made
//    orthogonal to row 2 and row 0 = normalize(row1 x row2);
//  - 0094A140 normalises rows 0, 1 and 2 again (0094A17A, 0094A1F9, 0094A278).
// SUBSTITUTION, labelled: an entity `refPos` would hand that entity's world
// matrix (008F8680's +14h arm); the request keeps only a position, so its
// basis is taken as the identity. No reference-mission call site passes one.
SpawnGroupFrame spawn_reference_frame_0094a140(const SpawnNewRequest& request) noexcept;

// One candidate of 0094A140's retry: RotY(a) * T(0, 0, d) * RotY(-a) * R in
// row-vector order (the three 00413920 calls at 0094A6A0..0094A6AE, left
// operand in ECX). Its basis is R's; its origin is
// R.origin + d * (-sin a * R.row0 + cos a * R.row2), with 00B646E0's
// rows (cos, 0, -sin) / (0, 1, 0) / (sin, 0, cos).
SpawnGroupFrame spawn_candidate_frame_0094a140(const SpawnGroupFrame& reference,
                                               float angle, float distance) noexcept;

// 00949300's member frame, T(offset) * candidate (00949380..009493E4, left
// operand the offset): the translation is origin + offset . basis.
void spawn_member_position_00949300(const SpawnGroupFrame& frame, const float offset[3],
                                    float out[3]) noexcept;

// One entity 00941D30's walk visits ([[00E188A8]+19CCh]+58h): its party
// (entity+54h) and world translation (entity+FCh..+104h).
struct SpawnPlacementEntity {
    std::int32_t party{-1};
    float position[3]{0.0f, 0.0f, 0.0f};
};

// What 00941D30 needs from the world. The host registers one; with none
// registered every candidate is legal (labelled at the solver).
class SpawnPlacementWorld {
public:
    virtual ~SpawnPlacementWorld() = default;
    // 0071C4F0 BSP_Game_PointOutsideMapBounds.
    virtual bool point_outside_map_0071c4f0(const float position[3]) const = 0;
    virtual void placement_entities(std::vector<SpawnPlacementEntity>& out) const = 0;
};
void set_spawn_placement_world(const SpawnPlacementWorld* world) noexcept;

// 00941D30, __fastcall(ECX = record+78h party, EDX = the member's translation
// row, stack: aircraft flag (vtable+18h(0Fh) at 00949420), class, frame,
// record+9Ch block), RET 10h. Refuses outside the map; for a non-aircraft
// member runs terrain/depth probes (not modelled: every member here is created
// as a plane squadron, docs/LUA_SPAWN_NEW_HOST.md section 8 deviation 1); then
// for each entity with dy = p.y - e.y: same party and |dy| <= ownVertical and
// |p - e|^2 < ownHorizontal^2 refuses; other party and |dy| <= enemyVertical
// and |p - e|^2 < enemyHorizontal^2 refuses. The squares are 009481A0's
// block+14h/+18h.
bool spawn_member_placement_legal_00941d30(const float position[3], std::int32_t party,
                                           const SpawnNewExcludeRadius& exclude,
                                           const std::vector<SpawnPlacementEntity>& entities,
                                           bool outside_map) noexcept;

// 0094A140's search. d runs from record+70h (distLow) while d <= record+74h
// in steps of 250 (double [00CF8850]); for each d the arc runs from 0 while
// arc <= d * halfwidth, step 250, where halfwidth = |angleHigh - angleLow| / 2
// and mid = (angleLow + angleHigh) / 2 ([00D7A280] 0.5). Each arc tries
// a = mid + arc/d, then (arc > 0 only, 0094B26x) a = mid - arc/d, and stops at
// the first candidate whose every member passes 00941D30 (00949300's
// all-or-nothing). The aircraft distance loop at 0094A6C3..0094ABxx runs only
// when game+1FE4h != 0, which is 0 in single player
// (docs/CONSTRUCT_WORLD.md), so it is not reproduced. A kind-6 first member
// would zero the frame's y (0094B24x); every member here is a plane.
struct SpawnPlacementResult {
    bool accepted{false};
    SpawnGroupFrame frame{};
    float angle{0.0f};
    float distance{0.0f};
    int candidates{0};
    // Evidence for the log: the entities 00941D30 walked and, for the accepted
    // frame, the nearest of them to any member (3-D, metres; -1 when none).
    int entities{0};
    float nearest{-1.0f};
};
SpawnPlacementResult solve_spawn_placement_0094a140(const SpawnNewRequest& request) noexcept;
// The last request's solved placement (for the host's log), or null.
const SpawnPlacementResult* last_spawn_placement_0094a140() noexcept;

}  // namespace bsp
