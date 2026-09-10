#pragma once
#include <array>
#include <cstdint>

// Frame tail of GGame::OnMove (004e4a40): everything between the last simulation
// call and BSP_Game_Render (004ca440). See docs/GAME_RENDER_TAIL.md for the
// evidence behind every offset named here. Nothing in this header is a recovered
// symbol; the names are hypotheses.
namespace bsp {

// ---------------------------------------------------------------------------
// Particle clock, singleton 00f8d420 (getter 004de4b0, update 00b19a10)
// ---------------------------------------------------------------------------

// 004e538e computes DAT_00f876a4 * (double)00ce47a0 on the x87 stack and rounds
// the product to float through a stack slot. 00ce47a0 holds the double 1000.0,
// and 00f876a4 is the global time float written by 00874640, so the argument is
// the global time expressed in milliseconds.
inline constexpr double kParticleClockTimeScale = 1000.0;
float particle_clock_time_004e538e(float global_time) noexcept;

// One tick target of the particle clock. The native records are 2Ch bytes and
// only +28h is read by 00b19a10, which calls the object's virtual +18h with the
// time. The other 28h bytes of each record are not reconstructed.
struct ParticleTimeSink {
    virtual ~ParticleTimeSink() = default;
    virtual void set_time(float time_ms) = 0; // vtable +18h, __thiscall, RET 4
};
inline constexpr std::uint32_t kParticleClockRecordStride = 0x2Cu;
inline constexpr std::uint32_t kParticleClockRecordSinkOffset = 0x28u;

// Allocated by 004de4b0 with 1Ch bytes; the constructor stores two vtables at +0
// and +4 and zeroes +8h..+14h. +18h is written only by 00b19a10 and read by the
// shader system-constant prefix at 00b46cb4 as component y of "Time" (c33).
struct ParticleClockOwnedRecord;
struct ParticleClock {
    float shader_time{}; // +18h
    ParticleTimeSink** sinks{}; // derived from +8h, which points at the records
    std::uint32_t sink_count{}; // +0Ch
    // Opaque native identities/words, not callable host vtables or layout.
    // The lazy constructor writes these and sinks/count, leaving shader_time
    // at its supplied allocation preimage. Aggregate defaults are host choices.
    std::uint32_t base_vtable_00{}, secondary_vtable_04{};
    std::uint32_t native_capacity_10{}, native_extra_14{};
    // Actual owned +8h record array, when supplied by the concrete lifetime
    // adapter. The earlier sinks field is a borrowed projection for callers
    // that do not own native records. The updater prefers this actual array.
    ParticleClockOwnedRecord* owned_records{};
};

// 00b19a10, __thiscall(this, float), RET 4. Captures +8h, stores raw time, then
// walks owned records front to back. After each per-call x87 argument spill and
// virtual call, reloads +8h/+0Ch for the end while advancing its raw cursor.
// A sink reallocating records retains the native invalidation risk. The older
// borrowed sinks projection retains its checked index-based compatibility path.
void set_particle_clock_time_00b19a10(ParticleClock& clock, float time_ms);

// ---------------------------------------------------------------------------
// Foliage group manager, singleton 00f8c274 (00af0450, 00af0c50)
// ---------------------------------------------------------------------------

// 00af0450 is __thiscall(this, float) RET 4 and does nothing but store +2Ch.
// The matching getter 00af0460 is FLD [ECX+2Ch]; RET, and the shader prefix at
// 00b46cdb uses it as component z of "Time" (c33), so the field is a scalar
// shader input and never read by the group walk itself.
inline constexpr std::uint32_t kFoliageManagerShaderTimeOffset = 0x2Cu;

// The float at 00d7a218 is 0.0. Groups whose +ACh equals it exactly are skipped
// by the second and third passes; the native compare is UCOMISS with the parity
// test inverted, so a NaN scale is treated as active.
inline constexpr float kFoliageGroupInactiveLodScale = 0.0f;

// Sentinel returned by the camera cull 00b71530 when the bounds are fully
// outside the frustum; every other value counts as visible.
inline constexpr std::uint32_t kFrustumRejectCode = 0xAAAu;

// Camera pair 00af0c50 receives. The first is the render camera stored at
// game+19FCh; the second comes from the camera director and is consulted only
// for groups whose description sets the secondary-visibility byte.
struct FoliageCameraPair {
    void* render_camera{}; // first stack argument, ECX of the cull and matrix calls
    void* secondary_camera{}; // second stack argument, only used for flagged groups
};

// 004e53de..004e540f: the director lives at *(game+19F0h)+A8h and holds two
// camera slots; the byte at +CCh chooses between them.
inline constexpr std::uint32_t kCameraDirectorSelectorOffset = 0xCCu;
inline constexpr std::uint32_t kCameraDirectorPrimarySlotOffset = 0xC8u;
inline constexpr std::uint32_t kCameraDirectorAlternateSlotOffset = 0xD0u;

// One entry of the manager's group array (+4h base, +8h count, 4-byte elements).
// Only the fields 00af0c50 touches are modelled.
struct FoliageGroup {
    float lod_scale{}; // +ACh; compared against 00d7a218, which holds 0.0
    std::uint32_t quad_count{}; // +1F8h; 0 means the group has not been built yet
    bool description_needs_render_mode{}; // *(+18Ch)+70h == 1 forces the render-mode test
    bool description_allows_secondary{}; // *(+18Ch)+7Ah, gates the second cull
    bool passes_group_test{}; // result of 00af6360(camera), evaluated by the host
    std::uint32_t primary_cull{kFrustumRejectCode}; // 00b71530 against the render camera
    std::uint32_t secondary_cull{kFrustumRejectCode}; // 00b71530 against the second camera
    bool visible{}; // written back as the float 0.0/1.0 argument of 00b6da70
};

// Manager state 00af0c50 reads and writes. +10h is a sub-object reset by
// 00af0900(0) before the walk and is not reconstructed.
struct FoliageGroupManager {
    std::uint32_t frame_counter{}; // +20h, incremented once per call
    void* current_camera{}; // +1Ch, set to the render camera argument
    void* dynamic_buffer_owner{}; // +24h; when null no buffer work happens at all
    void* locked_buffer{}; // +28h, the pointer returned by the allocator
    float shader_time{}; // +2Ch, written by 00af0450 only
};

// Statistics 00af0c50 publishes into the device object reached through
// 00b74640(0,0) then 00b732c0: +10h receives four times the built quad count and
// +18h twice, which is the vertex and triangle count of a quad per instance.
struct FoliageBuildResult {
    std::uint32_t visible_quads{}; // sum of +1F8h over the culled-in groups
    std::uint32_t pending_quads{}; // first-pass sum, sizes the locked buffer
    std::uint32_t buffer_bytes{}; // pending_quads * 4, the allocation request
    std::uint32_t device_vertex_count{}; // device +10h
    std::uint32_t device_triangle_count{}; // device +18h
    bool buffer_locked{}; // the stack flag reusing the second argument slot
};

// Integration boundary for 00af0c50. One method per native call site.
struct FoliageBuildHost {
    virtual ~FoliageBuildHost() = default;
    virtual void reset_group_scratch() = 0; // 00af0900(0), ECX = manager+10h
    virtual bool camera_render_mode_set(void* camera) = 0; // camera+198h != 0
    virtual bool group_passes_test(FoliageGroup& group, void* camera) = 0; // 00af6360
    virtual std::uint32_t cull_group(void* camera, FoliageGroup& group) = 0; // 00b71530
    virtual bool debug_capture_enabled() = 0; // 0051f520 result +4h
    virtual void debug_submit(FoliageGroup& group, void* camera, std::uint32_t cull) = 0;
    virtual void set_group_visible(FoliageGroup& group, float visible) = 0; // 00b6da70
    virtual void* lock_dynamic_buffer(std::uint32_t bytes) = 0; // device +3Ch vtable +10h
    virtual void unlock_dynamic_buffer() = 0; // device +3Ch vtable +14h
    virtual void refresh_camera_world_matrix(void* camera) = 0; // 00b6db70
    virtual void refresh_camera_view_matrices(void* camera) = 0; // 00b6fcb0 then 00b70490
    virtual void request_group_build(FoliageGroup& group) = 0; // 004c1130 sub-object +4h
    virtual void flush_group_builds() = 0; // same sub-object, virtual +8h, argument 1
    virtual void fill_group_quads(FoliageGroup& group, void* camera) = 0; // 00af63a0
    virtual void publish_device_counts(std::uint32_t vertices, std::uint32_t triangles) = 0;
};

// Bit 1 of the camera transform valid_flags (native camera+5Ch). When clear the
// world matrix is refreshed before the view matrices are taken.
inline constexpr std::uint32_t kCameraWorldMatrixValidBit = 0x2u;

// 00af0c50, __thiscall(this, render_camera, secondary_camera), RET 8.
FoliageBuildResult build_foliage_visible_set_00af0c50(FoliageGroupManager& manager,
    FoliageGroup* groups, std::uint32_t group_count, const FoliageCameraPair& cameras,
    std::uint32_t camera_valid_flags, FoliageBuildHost& host);

// ---------------------------------------------------------------------------
// Sound request queue, singleton 00f89b34 (getter 004c1b90, apply 00941140)
// ---------------------------------------------------------------------------

// The instance is 28h bytes. +4h..+14h hold five cue identifiers indexed by the
// request slot, +18h..+1Dh are six request bytes, +20h is the forced slot and
// +24h is the context handle passed to the sound call.
inline constexpr int kSoundRequestSlotCount = 6;
inline constexpr int kSoundRequestNone = 6; // the reset value of +20h
inline constexpr int kSoundRequestSilentSlot = 5; // 00941070 returns without playing
inline constexpr int kSoundRequestFallbackSlot = 0; // slot 1 degrades to this

struct SoundRequestQueue {
    std::array<std::uint8_t, kSoundRequestSlotCount> requested{}; // +18h..+1Dh
    int forced_slot{kSoundRequestNone}; // +20h
};

// Result of the selection in 00941140. The queue is always cleared afterwards.
struct SoundRequestDecision {
    bool apply{}; // whether 00941070 runs at all
    int slot{kSoundRequestNone}; // the argument it receives
};

// 00941140, __fastcall(this), RET. A forced slot wins outright; otherwise the
// highest set request wins, and slot 1 falls back to slot 0 unless one of the
// two leading entries of the lists at 00f8bbf4+70h and +B8h answers true to its
// virtual +2Ch.
SoundRequestDecision apply_sound_request_00941140(SoundRequestQueue& queue,
    bool slot_one_condition);

// ---------------------------------------------------------------------------
// GUI visibility policy (004c6c70)
// ---------------------------------------------------------------------------

// Every input 004c6c70 reads, in the order the native code reads them. Absent
// objects are reported through the *_present flags; a missing object contributes
// nothing, which is how the null tests behave.
struct GuiVisibilityInputs {
    bool loading_screen_active{}; // 00e198b4 +3Ch
    bool menu_screen_active{}; // 00e198ac +3Ch
    bool other_screen_active{}; // 00e198b8 +3Ch
    int game_state{}; // game+5D4h; state 2 alone forces the GUI on
    bool hud_present{}; // 00e198c4
    bool hud_dialog_flag{}; // *(hud+D8h)+5h
    bool base_screen_active{}; // 0068a140
    bool hud_page_a_flag{}; // *(hud+5Ch)+5h
    bool hud_page_b_flag{}; // *(hud+54h)+5h
    bool loading_screen_sub_flag{}; // *(00e198b4+54h)+64h
    bool hud_page_c_flag{}; // *(hud+60h)+5h
    bool hud_page_c_second_flag{}; // *(hud+60h)+70h
    bool hud_page_b_suppress{}; // *(hud+54h)+5F0h; when set it cancels the pair above
    bool cinematic_flag{}; // game+7184h
    bool modal_present{}; // 00e194b4
    bool modal_flag{}; // 00e194b4 +5h
    bool menu_command_flag{}; // 00425d10 result +5h, ORed in last
    bool gui_suppressed{}; // 00f8abe8 +3E8h (decimal 1000)
};

// 004c6c70 always calls 00aa0e00 and calls 00aa0e50 only on some paths, so the
// pointer decision carries its own applies flag.
struct GuiVisibilityDecision {
    bool enabled{}; // argument of BSP_GuiManager_SetEnabled
    bool applies_pointer{}; // whether BSP_GuiManager_SetPointerVisible is reached
    bool pointer_visible{}; // its argument
};

// 004c6c70, __fastcall(game), RET.
GuiVisibilityDecision decide_gui_visibility_004c6c70(const GuiVisibilityInputs& in);

// ---------------------------------------------------------------------------
// The frame tail itself (004e538e..004e54f0)
// ---------------------------------------------------------------------------

// In-mission game state; only this state runs the foliage step. Named the same
// way docs/GAME_SIMULATION_GATE.md names it.
inline constexpr int kInMissionGameState = 0x0D;

// Profiler counter slots the tail closes and opens. The identifiers are the
// dwords held in those globals, not indices computed here.
struct RenderTailCounters {
    std::uint32_t game_counter{}; // 0109db08, ended at 004e54a7
    std::uint32_t render_counter{}; // 0109db14, spans BSP_Game_Render
};

// Per-frame inputs of the tail that the surrounding update already resolved.
struct RenderTailInputs {
    bool simulation_ran{}; // the gate at 004e50b0 was taken
    int game_state{}; // game+5D4h
    float global_time{}; // 00f876a4
    float raw_delta{}; // the unscaled delta the frame was entered with
    float foliage_shader_time{}; // *(game+5FCh)+1054h
    bool menu_interface_present{}; // 00e198ac
};

// Integration boundary for the tail. One method per native call site, in frame
// order. The render queue globals are read-only in this packet, so the queue
// appears only as the two profiled calls that bracket it.
struct RenderTailHost {
    virtual ~RenderTailHost() = default;
    virtual void set_particle_clock_time(float time_ms) = 0; // 004de4b0 then 00b19a10
    virtual void update_without_simulation() = 0; // 004c40f0
    virtual void set_foliage_shader_time(float value) = 0; // 00af0450, ECX = 00f8c274
    virtual FoliageCameraPair select_foliage_cameras() = 0; // 004e53de..004e5401
    virtual void build_foliage_visible_set(const FoliageCameraPair& cameras) = 0; // 00af0c50
    virtual void apply_gui_visibility() = 0; // 004c6c70
    virtual void update_menu_interface(float raw_delta) = 0; // 00685c80
    virtual void update_multiplayer_interface() = 0; // 004d80d0
    virtual bool service_menu_requests() = 0; // 006840f0, returns its out parameter
    virtual void clear_request_latch() = 0; // 00e18cdc = 0
    virtual void tick_peer_session() = 0; // 00776230, ECX = game+1EF0h
    virtual void apply_sound_request() = 0; // 004c1b90 then 00941140
    virtual void update_frontend_screens() = 0; // 004d8620
    virtual void profiler_end_counter(std::uint32_t counter) = 0; // 004c1dd0 then 00be3660
    virtual void profiler_begin_counter(std::uint32_t counter) = 0; // 004c1dd0 then 00be3640
    virtual void game_render() = 0; // 004ca440
    virtual void finish_render_frame() = 0; // 004ca1f0
};

// What the tail did, so a caller can check the path without instrumenting the
// host. requests_serviced counts every 006840f0 call including the first.
struct RenderTailResult {
    bool particle_step_ran{};
    bool foliage_step_ran{};
    int requests_serviced{};
    int drain_iterations{};
    bool render_block_ran{};
};

// 004e538e..004e54f0 of BSP_Game_OnMove. The native code clears the drain flag at
// 004e548a and still tests it at 004e54b5 before the render block; the flag is
// provably zero there, so the block always runs on this path. The dead test is
// kept because it is in the binary.
RenderTailResult run_render_tail(const RenderTailInputs& inputs,
    const RenderTailCounters& counters, RenderTailHost& host);
}
