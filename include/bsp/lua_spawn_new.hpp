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
//   009483D0                   __thiscall(record, frame): creates every member,
//                              appends each created entity to the vector at
//                              record+CCh, and at its tail (009487Bx) stores
//                              `*(record+C0h) = 1`.
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

// 0094C4E4..0094C500: `if (DAT_00F876A4 < *(float*)(globalConfig + 2DCh)
//                          + *(float*)(manager + 0Ch)) return;`
// so one request is attempted per interval and manager+0Ch is the stamp of the
// last attempt (0094C59D MOVSS [EDI+0Ch],XMM0).
inline constexpr std::size_t kSpawnManagerLastAttemptOffset = 0xC;
inline constexpr std::size_t kGlobalConfigSpawnAttemptDelayOffset = 0x2DC;
// The image default, 0087F7E7 `FLD float ptr [00CE74F8]`, bytes cd cc 4c 3f.
// It is a float32 at that address, loaded by a float FLD, not a double.
inline constexpr float kSpawnAttemptDelayDefault = 0.8f;
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
inline constexpr std::size_t kSpawnRequestFulfilledOffset = 0xC0;   // 009487B9
inline constexpr std::size_t kSpawnRequestPartyIndexOffset = 0x80;  // 0094C7D9
inline constexpr std::size_t kSpawnRequestCompletionFnOffset = 0xC4;  // 0094C7D2
inline constexpr std::size_t kSpawnRequestCompletionCtxOffset = 0xC8; // 0094C7CA

// The created entities. 0094C6BF/0094C74B `LEA EDI,[ESI + 0xCC]` then the
// checked begin/end pair 00645C40/00645C70, walked with `ADD EDI,4`: a
// std::vector<Entity*> whose proxy is +CCh and whose three pointers are
// +D0h/+D4h/+D8h, which is what lua_binding_spawn.hpp records as "zeroed".
// 009483D0 appends to it at 00948779 (`*piVar = entity`) as each member is made.
inline constexpr std::size_t kSpawnRequestCreatedVectorOffset = 0xCC;

// Per-member formation offsets, a std::vector<float[3]> at +90h/+94h: the count
// check at 00949356 divides `(+94h - +90h)` by 0xC, and 009483D0 reads element
// `i` for member `i` at 00948440.
inline constexpr std::size_t kSpawnRequestMemberOffsetsBeginOffset = 0x90;
inline constexpr std::size_t kSpawnRequestMemberOffsetsEndOffset = 0x94;

// One 10h-byte group-member element, read by 00949300 and 009483D0:
//   +0h  the vehicle class object; slot 18h of its vtable is a kind test and
//        slot 28h constructs the instance (00948462, 0094847E)
//   +8h  a char* name, defaulted to the empty NativeString data at 00F89B40
//        (00948488) and memcpy'd into the created entity's +154h/+158h string
//   +Ch  one dword read into the property-bag argument (0094845B)
inline constexpr std::size_t kSpawnMemberClassOffset = 0x0;
inline constexpr std::size_t kSpawnMemberNameOffset = 0x8;
inline constexpr std::size_t kSpawnMemberWordOffset = 0xC;

// 009483D0's two arms. `vtable+18h(6)` answers whether the class is of kind 6;
// when it is NOT, the routine takes `operator_new(0x414)` and
// BSP_PlaneSquadronTickableEntity_Construct (00948467..00948477), and when it
// IS it calls the class's own `vtable+28h(0)`. So kind 6 is the surface arm and
// everything else spawns as a plane squadron.
inline constexpr int kSpawnMemberKindSurface = 6;
inline constexpr std::size_t kPlaneSquadronTickableEntityBytes = 0x414;

// The serial the created entity carries: 0094845x `*(entity + 28Ch) = *(record + 7Ch)`.
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

    // 0094C4E4: `now >= last_attempt + interval`.
    bool attempt_due(float now, float interval) const noexcept;
    void stamp_attempt(float now) noexcept { last_attempt_ = now; }
    float last_attempt() const noexcept { return last_attempt_; }

    // 0094C508..0094C56B. With fewer than two records the drain takes the head
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
};

// The first candidate frame for `member` of `request`. The angle is taken
// across `angleRange` and the distance across `distRange`, both by member
// index, so a four-member group fans out rather than stacking. CONTRACT, not a
// reading: 0094A140's own sampling was not decoded.
SpawnNewFrame spawn_member_frame_0094a140(const SpawnNewRequest& request,
                                          std::size_t member) noexcept;

}  // namespace bsp
