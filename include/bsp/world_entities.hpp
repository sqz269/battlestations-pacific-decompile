#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

// World tick of the in-mission frame, phase 20 of BSP_Game_OnMove (004e4a40)
// plus the two calls the same frame makes outside the simulation gate. Every
// call in this packet is driven from the scaled delta at game+21F0h, with ECX
// taken from a game field rather than from a singleton getter.
//
// Native call order, 004e52ba..004e5389:
//   00914ef0(scaled)  ECX = game+21A0h   bot scheduler
//   006dc1a0(scaled)  ECX = game+21D4h   marker and GUI-highlight manager
//   00481640(scaled)  ECX = game+21D0h   entity manager forwarder
//   ... ocean, owned by game_world_ocean_effects ...
//   00740e10(scaled)  ECX = 00e1aea0     decal manager
//   0094c8f0(scaled)  ECX = 00f89b3c     trivial body, not in this packet
//   008eb110()        ECX = 00f88c30     power-up manager
//   004d1100/00867ee0                    owned by game_world_ocean_effects
//   00903670()        ECX = game+19CCh   entity activation flush
//   004d7ea0()        ECX = game         owned by game_frame_control
//
// Outside the gate: 00987590(scaled) at 004e4e67 with ECX = game+21E0h, and
// 004d8cd0() at 004e4feb with ECX = game. docs/GAME_WORLD_ENTITIES.md records
// the evidence, the RET sizes and the uncertainties.
namespace bsp {

// ---------------------------------------------------------------------------
// Gates the world tick sits behind
// ---------------------------------------------------------------------------

// Fields of the game object the world tick reads. Not the native layout: the
// offsets are in the comments and the intervening fields belong to other
// packets. 5D4h/634h/21F0h/64Ch are also carried by GameFrameControlState in
// bsp/game_frame_control.hpp; they are repeated here because this packet's
// routines are exercised on their own, not only through that spine.
struct WorldTickGameFields {
    std::uint32_t state{}; // +5D4h, the world tick needs Dh
    bool cinematic{}; // +634h
    // +635h. 004e50b9: a set override byte lets the simulation gate run even
    // while +634h is set. The effect-list gate at 004e4e31 has no such escape.
    bool cinematic_override{};
    bool simulation_suspended{}; // +7184h, checked by the simulation gate
    float scaled_delta{}; // +21F0h, the argument every call below receives
    float elapsed{}; // +64Ch, the undilated mission clock
    std::uint32_t local_player_mode{}; // +1FE4h
    std::size_t active_local_player_slot{}; // +18ECh, indexes 8 slots
    bool mission_events_present{}; // +21E0h non-null
    bool effect_lists_armed{}; // +1EE7h, the one-shot ahead of the effect lists
};

// 004e4e31 and 004e50b0. The world tick and the effect lists share the
// "not cinematic" test; the simulation gate adds state Dh and 7184h.
bool world_tick_runs(const WorldTickGameFields& game) noexcept;
// 004e4e3e..004e4e5d. The mission-event update also needs a strictly positive
// delta; 00d7a218 holds 0.0f.
bool mission_events_run(const WorldTickGameFields& game) noexcept;

// ---------------------------------------------------------------------------
// 00481640 - entity manager forwarder, __thiscall(this, float), RET 4
// ---------------------------------------------------------------------------

// The 58h-byte object at game+21D0h. Constructor 004a43c0 stores the vtable at
// +0h and then five list heads with their sizes at +10h, +1Ch, +28h, +34h and
// +40h; a sixth ends the object at +58h. The forwarder only touches +8h, which
// the constructor leaves alone and the follow-up initialiser 0049d690 fills in.
struct EntityManagerRef {
    // +8h. Null is not handled by the native body, which loads the vtable
    // unconditionally once the gate passes.
    bool sub_manager_present{};
};

// 00481640 reads the world object through the game singleton at 00e188a8
// rather than through its own ECX, so the gate is a property of game+19CCh and
// not of the entity manager. The byte lives at world+4ACh.
struct EntityWorldGate {
    bool enabled{}; // *(game+19CCh) + 4ACh
};

// ---------------------------------------------------------------------------
// 00914ef0 - bot scheduler, __thiscall(this, float), RET 4
// ---------------------------------------------------------------------------

// 004e3e74 allocates 14B8h bytes, runs constructor 0091d640 and stores the
// result at game+21A0h. 0091d640, 00914ef0, 00911e80, 00912a60 and 00914390
// all live in the segment whose keywords are pilotbot, tailgunnerbot,
// torpedobot and thinktimeleft, which is why this reads as the bot scheduler.
inline constexpr std::size_t kBotSlotCount = 8; // 00914fdf, CMP ESI,8
inline constexpr std::size_t kBotSlotStride = 0x284; // 00914fd9 and 0091503e
inline constexpr std::size_t kBotSlotArrayOffset = 0x1E4; // 00914f88

// Message code stored at stack+184h before the dispatch at 0076a9f0, and the
// id handed to 0075b430 when the retarget timer fires.
inline constexpr std::uint32_t kBotSlotDispatchCode = 0x12; // 00914fb2
inline constexpr int kBotRetargetEventId = 0x15; // 00914f6a

struct BotSchedulerState {
    bool enabled{}; // +1498h, gates both countdowns
    bool slots_active{}; // +14A4h, gates the per-slot work and the accumulator
    float retarget_countdown{}; // +149Ch, reloaded from +14A8h
    float think_countdown{}; // +1494h, reloaded from +14ACh
    float retarget_period{}; // +14A8h
    float think_period{}; // +14ACh
    float active_time{}; // +14A0h, accumulates the scaled delta
    // First dword of each 284h-byte record at +1E4h, the only field 00914ef0
    // reads. Sized to kBotSlotCount.
    std::vector<std::uint32_t> slot_tokens{};
};

// ---------------------------------------------------------------------------
// 006dc1a0 - marker and highlight manager, __thiscall(this, float), RET 4
// ---------------------------------------------------------------------------

// The 50h-byte object at game+21D4h, constructor 006deca0, initialiser
// 006d6200. Its segment keywords are entitymarkers, positionmarkers,
// guihighlights, recursiveguihighlights and markerclasses, which is the order
// the five walks below appear in.
//
// Every container is an MSVC std::list built with _SECURE_SCL on: the base
// holds _Myfirstiter, then _Myhead, then _Mysize, and each node is
// {_Next, _Prev, value}. The offsets below are the _Myfirstiter slots.
inline constexpr std::size_t kMarkerGroupListOffset = 0x14; // walk 1
inline constexpr std::size_t kMarkerSecondGroupListOffset = 0x20; // walk 2
inline constexpr std::size_t kMarkerHighlightListOffset = 0x2C; // walk 3
inline constexpr std::size_t kMarkerScaledListOffset = 0x38; // walk 4
inline constexpr std::size_t kMarkerTintedListOffset = 0x44; // walk 5

// Pulse shared by walks 3 and 5: phase = 2 * (game+64Ch - 00e19970), and the
// magnitude is ABS of its cosine (006dc415 and 006dc538, AND EDX,7FFFFFFFh).
// 00e19970 ships as 0.0f and nothing in this packet writes it.
float marker_pulse_magnitude(float elapsed, float pulse_epoch) noexcept;

// 006dc423, FMUL qword ptr [00cf8fc8]. The cell holds 3FD1EB8520000000h, the
// double promotion of 0.28f, so the promotion is load bearing.
inline constexpr double kMarkerHighlightPulseScale = static_cast<double>(0.28f);
// 006dc552 and 006dc561, both qword: 3FE8000000000000h and 3FD0000000000000h.
inline constexpr double kMarkerAlphaPulseScale = 0.75;
inline constexpr double kMarkerAlphaPulseBias = 0.25;
// 006dc496..006dc49c: FUN_006dbac0(1, 0, 1.0f) on each walk-4 element.
inline constexpr int kMarkerScaleModeArg = 1;
inline constexpr int kMarkerScaleFlagArg = 0;
inline constexpr float kMarkerScaleValueArg = 1.0f;

// Value of a walk-1 or walk-2 node: an inner std::list whose own elements each
// carry the marker object. Walk 1 reads the inner base at value+8h and the
// object at innerValue+0Ch; walk 2 reads value+10h and the same innerValue+0Ch.
struct MarkerGroup {
    // Element pointers of the inner list, in list order. A null element is
    // skipped: 006dc260 and 006dc34f test it before the virtual call.
    std::vector<void*> markers{};
};

// Value of a walk-3, walk-4 or walk-5 node. All three read the object at
// node+0Ch, which is value+4h.
struct MarkerEntry {
    void* target{}; // value+4h
};

// Four floats a marker's virtual +54h returns and its virtual +50h consumes.
struct MarkerColor {
    float rgba[4]{};
};

struct MarkerManagerState {
    std::vector<MarkerGroup> entity_marker_groups{}; // +14h
    std::vector<MarkerGroup> position_marker_groups{}; // +20h
    std::vector<MarkerEntry> gui_highlights{}; // +2Ch
    std::vector<MarkerEntry> recursive_gui_highlights{}; // +38h
    std::vector<MarkerEntry> tinted_highlights{}; // +44h
};

// ---------------------------------------------------------------------------
// 00740e10 - decal manager, __thiscall(this, float), RET 4
// ---------------------------------------------------------------------------

// ECX is the global at 00e1aea0. The body registers the profiler label
// "cDecalManager::Update" once, behind bit 0 of 00e1aeb0, and then walks a
// plain array: begin at this+4h, count at this+8h, stride 4. The loop body is
// empty in the shipped image (00740e91..00740e96 is ADD EAX,4 / CMP / JNZ), so
// the per-decal work was compiled away and the delta argument is never read.
struct DecalManagerState {
    bool profiler_label_registered{}; // bit 0 of 00e1aeb0
    std::vector<std::uint32_t> decals{}; // begin at +4h, count at +8h
};

// Returns the number of elements the native loop steps over, which is the only
// observable effect left in the shipped body.
std::size_t update_decal_manager_00740e10(DecalManagerState& decals, float scaled_delta) noexcept;

// ---------------------------------------------------------------------------
// 008eb110 - power-up manager, __thiscall(this), RET, no argument
// ---------------------------------------------------------------------------

// ECX is the global at 00f88c30. Segment keywords pup_gain, pum1stget and
// uspumicon, and the body builds the literal "pup_ready".
inline constexpr std::size_t kPowerUpChannelCount = 16; // 008eb14e, 16 iterations
inline constexpr std::size_t kPowerUpChannelStride = 0x0C; // piVar8 += 3 dwords
inline constexpr std::size_t kPowerUpChannelArrayOffset = 0x84;
inline constexpr std::size_t kPowerUpPlayerListOffset = 0x14C; // stride 0Ch
// _Myhead of the per-player ready list, read at 008eb290; its _Mysize at +28h
// is the gate. The list base is +20h and the stride is 0Ch, as above.
inline constexpr std::size_t kPowerUpReadyListHeadOffset = 0x24;

// One element of a +84h channel list. The node value starts at node+8h; the
// expiry float the sweep compares is at node+20h, so value+18h.
struct PowerUpTimer {
    void* payload{}; // value, handed to 008e8c30
    float expires_at{}; // value+18h, compared against the clock at 00f876a4
};

// One element of the +14Ch per-player list.
struct PowerUpSlot {
    bool notified{}; // value+8h, latched to 1 once the slot becomes ready
    bool relative_deadline{}; // value+1Ch; selects which expiry is used
    float absolute_expiry{}; // value+18h, used when relative_deadline is false
    // value+10h, the source object; its +80h float is added to the clock when
    // relative_deadline is set, which re-derives the deadline every frame.
    void* source{};
    float source_offset{}; // *(source+80h)
};

struct PowerUpManagerState {
    // 16 channels at +84h. The sweep does not erase: it only calls 008e8c30.
    std::vector<PowerUpTimer> channels[kPowerUpChannelCount]{};
    // The per-player list the ready notification walks, for the slot the game
    // object names at +18ECh.
    std::vector<PowerUpSlot> player_slots{};
    bool player_notify_enabled{}; // *(this+28h + slot*0Ch) non-null
};

// ---------------------------------------------------------------------------
// 00903670 - entity activation flush, __thiscall(this), RET, no argument
// ---------------------------------------------------------------------------

// ECX is game+19CCh, the same world object 00481640 gates on. The list head is
// at this+4h and nodes are chained through node+38h; this is an intrusive
// chain, not a std::list.
struct ActivationNode {
    bool spawned{}; // +5Eh, set by 00922fd0
    bool activated{}; // +6Ch, latched by 00922fd0 so the flush runs once
    std::size_t parent{}; // +3Ch, index into the same vector, or kNoParent
};
inline constexpr std::size_t kNoActivationParent = static_cast<std::size_t>(-1);

// 00903680..009036a5. True when the node is spawned, not yet activated, and
// either has no parent or has a parent that is not itself spawned. 00922fd0
// then walks the node's children and calls its virtual +84h.
bool activation_pending(const std::vector<ActivationNode>& nodes, std::size_t index) noexcept;

// ---------------------------------------------------------------------------
// 00987590 - mission event director, __thiscall(this, float), RET 4
// ---------------------------------------------------------------------------

// ECX is game+21E0h. Segment keywords entitykilled, musicover, repair and
// surrender. The queue is a std::list at +E0h (_Myfirstiter +E0h, _Myhead
// +E4h, _Mysize +E8h) whose value is a pointer to an event object.
inline constexpr float kMissionEventTickPeriod = 4.0f; // 00ce3d34, 40800000h

struct MissionEvent {
    float duration{}; // event+10h
    float start_time{}; // event virtual +1Ch, compared against 00f876a4
    float priority{}; // event virtual +18h
    bool ready{}; // 005b71d0(event+14h)
};

struct MissionEventQueueState {
    float tick_accumulator{}; // +198h
    std::vector<MissionEvent> events{}; // the +E0h list
};

struct MissionEventTickResult {
    bool periodic_fired{}; // the +198h accumulator crossed 4.0f
    // Index of the single event the frame retired, or kNoMissionEvent. The
    // native body returns immediately after the erase, so at most one event is
    // destroyed per frame and the rest of the walk is skipped.
    std::size_t retired{};
    // Index of the highest-priority ready event, handed to 00974070, or
    // kNoMissionEvent when the walk found none.
    std::size_t selected{};
};
inline constexpr std::size_t kNoMissionEvent = static_cast<std::size_t>(-1);

// ---------------------------------------------------------------------------
// 004d8cd0 - action deadline table, __thiscall(this), RET, no argument
// ---------------------------------------------------------------------------

// ECX is the game object. Sixteen blocks, each reading one input action record
// through the input singleton 004bec00 and, when the action is pressed this
// frame, writing game+64Ch + 0.4 into the std::map<int,float> at 00e18a7c
// through operator[] at 004d6900.
//
// The record test is exactly the 004c43c0 edge test documented in
// bsp/game_frame_control.hpp: record+28h set, record+24h greater than zero,
// and either record+20h clear or record+1Ch not greater than zero. Records are
// 30h bytes starting at inputSingleton+4h, which is how the inline offsets
// 0E08h, 0E38h, 0EC8h ... 10A8h divide out to the ids below.
inline constexpr std::size_t kActionDeadlineCount = 16;
inline constexpr int kActionDeadlineIds[kActionDeadlineCount] = {
    0x4A, 0x4B, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x46, 0x47, 0x4C, 0x4D, 0x57, 0x58,
};
// 004d8d0e, FADD qword ptr [00ce65d0]. The cell holds 3FD99999A0000000h, the
// double promotion of 0.4f.
inline constexpr double kActionDeadlineDelay = static_cast<double>(0.4f);

// The map at 00e18a7c. Keys are input action ids, values are absolute deadlines
// on the game+64Ch clock.
using ActionDeadlineTable = std::map<int, float>;

// ---------------------------------------------------------------------------
// Owned state
// ---------------------------------------------------------------------------

struct WorldTickState {
    WorldTickGameFields game{};
    EntityWorldGate world_gate{};
    EntityManagerRef entity_manager{};
    BotSchedulerState bots{};
    MarkerManagerState markers{};
    DecalManagerState decals{};
    PowerUpManagerState power_ups{};
    std::vector<ActivationNode> activation_nodes{};
    MissionEventQueueState mission_events{};
    ActionDeadlineTable action_deadlines{};
    float marker_pulse_epoch{}; // 00e19970
    std::uint64_t frame_counter{}; // game+648h, incremented at 004e4ff5
};

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site the packet reaches, declared in frame order.
// Nothing has a default implementation: none of these stands in for
// unrecovered game behaviour. Method names ending in an address name the
// native routine; the rest describe a small inlined sequence.
struct WorldTickHost {
    virtual ~WorldTickHost() = default;

    // Global mission clock at 00f876a4, read by 008eb110 and 00987590.
    virtual float world_clock() = 0;

    // --- 00987590, outside the simulation gate ---
    virtual void mission_events_pre_pass_00982540() = 0; // 00987598
    virtual void mission_events_periodic_00977990() = 0; // 009875c7
    virtual void mission_events_poll_0096d540() = 0; // 009875cc
    virtual void mission_events_poll_00968550() = 0; // 009875d1
    // Virtual +1Ch of an event, returning its start time (00987656).
    virtual float mission_event_start_time(std::size_t index) = 0;
    // Virtual +18h of an event, returning its priority (009876af, 009876bd).
    virtual float mission_event_priority(std::size_t index) = 0;
    // 005b71d0(event+14h) at 0098769b, the readiness test.
    virtual bool mission_event_ready_005b71d0(std::size_t index) = 0;
    // Virtual +0h with argument 1, the scalar deleting destructor (0098766f).
    virtual void mission_event_destroy(std::size_t index) = 0;
    // 00974070 with the winning event (00987606).
    virtual void mission_event_apply_00974070(std::size_t index) = 0;

    // --- 00914ef0, ECX = game+21A0h ---
    virtual void bot_retarget_begin_0075b430(int event_id) = 0; // 00914f6a
    virtual void bot_slot_prepare_00914390(std::size_t slot) = 0; // 00914f97
    // CG_adjustor_thunk_0076a9f0 on a stack object whose vtable is 00d035b8,
    // carrying the slot token, the code 12h and the slot index (00914fc1).
    virtual void bot_slot_dispatch_0076a9f0(std::uint32_t slot_token,
        std::uint32_t code, std::size_t slot) = 0;
    virtual void bot_think_pass_a_00911e80(std::size_t slot) = 0; // 00915029
    virtual void bot_think_pass_b_00912a60(std::size_t slot) = 0; // 0091502e

    // --- 006dc1a0, ECX = game+21D4h ---
    // Virtual +4h of a marker object, taking the scaled delta (006dc27f and
    // 006dc36e). The same slot 00481640 uses on the entity sub-manager.
    virtual void marker_update(void* marker, float scaled_delta) = 0;
    // Virtual +54h then virtual +50h of a highlight target (006dc54c, 006dc56e).
    virtual MarkerColor marker_get_color(void* target) = 0;
    virtual void marker_set_color(void* target, const MarkerColor& color) = 0;
    // 006dbac0(target, 1, 0, 1.0f) at 006dc49c.
    virtual void marker_set_scale_006dbac0(void* target, int mode, int flag, float value) = 0;
    // *(target+94h) = magnitude * 0.28 at 006dc42d.
    virtual void marker_set_highlight_level(void* target, float level) = 0;

    // --- 00481640, ECX = game+21D0h ---
    // Virtual +4h of the sub-manager at entityManager+8h (00481664).
    virtual void entity_manager_update(float scaled_delta) = 0;

    // --- 008eb110, ECX = 00f88c30 ---
    virtual void power_ups_pre_pass_008eac80() = 0; // 008eb13c
    virtual void power_up_expire_008e8c30(void* payload) = 0; // 008eb17a
    // 009789a0(activeSlot, listNodeValue, &"pup_ready") at 008eb2ac.
    virtual void power_up_notify_ready_009789a0(std::size_t slot, std::size_t entry) = 0;
    virtual void power_ups_post_pass_00613760() = 0; // 008eb2df

    // --- 00903670, ECX = game+19CCh ---
    // 00922fd0(node): marks the node and its children and calls virtual +84h.
    virtual void activate_entity_subtree_00922fd0(std::size_t index) = 0;

    // --- 004d8cd0, ECX = game ---
    // The 004c43c0 edge test against the input singleton at 004bec00.
    virtual bool input_action_pressed(int action) = 0;
};

// ---------------------------------------------------------------------------
// Reconstructed routines
// ---------------------------------------------------------------------------

// 00914ef0. Runs only when the local player mode is 0 or 1 and +1498h is set;
// the +14A0h accumulator is separate and runs whenever the mode is not 2.
void update_bot_scheduler_00914ef0(WorldTickState& state, WorldTickHost& host);

// 006dc1a0. Five list walks in native order.
void update_markers_006dc1a0(WorldTickState& state, WorldTickHost& host);

// 00481640. Reads the world gate at *(game+19CCh)+4ACh, not its own ECX.
void update_entity_manager_00481640(WorldTickState& state, WorldTickHost& host);

// 008eb110. The 16-channel expiry sweep followed by the per-player ready pass.
void update_power_ups_008eb110(WorldTickState& state, WorldTickHost& host);

// 00903670. Flushes every pending activation in list order.
void flush_entity_activations_00903670(WorldTickState& state, WorldTickHost& host);

// 00987590. Retires at most one expired event per frame and, when nothing was
// retired, applies the highest-priority ready event.
MissionEventTickResult update_mission_events_00987590(WorldTickState& state, WorldTickHost& host);

// 004d8cd0. Writes a deadline for every one of the 16 actions pressed this
// frame; actions that are not pressed leave their map entry untouched.
void record_action_deadlines_004d8cd0(WorldTickState& state, WorldTickHost& host);

// Phase 20 of 004e4a40 in native order, plus the two calls the same frame makes
// outside the simulation gate. The ocean, effect and mission-completion calls
// interleaved with these belong to other packets and are deliberately absent;
// the sequence leaves their slots as comments rather than stubbing them.
void run_world_tick(WorldTickState& state, WorldTickHost& host);
}
