#pragma once
#include <cstdint>

namespace bsp {
struct NativeCameraAxesContext;
struct NativeCameraHistoryContext;
struct NativeCameraFarCornersContext;
struct NativeRendererFrameTargetsContext;
struct NativeCameraFrameCommandContext;
struct NativeSystemConstantGatherContext;
struct NativeSystemConstantGatherFrame;
struct NativeStringRawPoolContext;
struct NativeRenderBatchPreparationContext;
struct NativeRenderJobPublicationContext;
class NativeRenderPreparationJobDispatch;
struct NativeMaterialEntryDispatchContext;
struct NativeMaterialEntryDispatchFrame;
struct NativeRenderSceneSubmissionContext;
struct NativeRenderSceneSubmissionOperation;
struct NativeRenderQueueExecutionContext;
struct NativeRenderQueueExecutionFrame;
struct NativePostEffectExecutionContext;
struct NativePostEffectDrawFrame;
struct NativeRendererTextureBindingContext;

struct NativeMainSceneExecutionPrefixContext {
    void* const volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    void* const volatile& actual_clock_01090ab0;
    const NativeCameraAxesContext& axes;
    const NativeCameraHistoryContext& history;
    const volatile double& direction_scale_00cec9e0;
    const volatile std::uint32_t& near_plane_00ce3800;
};

// Fresh persistent external continuation state, disjoint from actual native
// storage, for the ORIGINAL later restore. Do not reuse a completed state.
// near_bits is the original camera+1D4 word; color_word is the saved +190 word
// with its high byte replaced by FF exactly as B1573D..B15741. The existing
// service/camera mutations survive failure and are never rolled back here.
struct NativeMainSceneExecutionPrefixState {
    std::uint32_t near_bits{};
    std::uint32_t color_word{};
    bool frame_active{};
    bool near_changed{};
    bool reached_texture_stage{};
    bool reached_bloom_stage{};
    std::uint32_t clear_flags_bits{};
    std::uint32_t active_native_site{};
    bool bloom_targets_started{};
    bool clear_flags_captured{};
    bool clear_flags_changed{};
    bool clear_color_changed{};
    bool reached_camera_frame_stage{};
    std::uint32_t render_mode_bits{};
    bool camera_gather_started{};
    bool camera_frame_completed{};
    bool render_mode_captured{};
    bool render_mode_changed{};
    bool system_gather_started{};
    bool reached_label_stage{};
    bool reached_drsh_batch_stage{};
    bool reached_excluded_node_stage{};
    bool reached_post_drsh_stage{};
    bool reached_scne_label_stage{};
    bool reached_scne_target_stage{};
    bool reached_scne_batch_stage{};
    bool reached_scne_capture_stage{};
    bool reached_post_label_stage{};
    bool reached_post_body_stage{};
};

// Complete raw leaves. Native ECX command / camera. B1BF40 uses RET and returns
// current [[command+28]+08]. B6FE60 has stack outputDWORD, RET4, EAX output.
void* __fastcall get_native_scene_command_camera_00b1bf40(const void* actual_command);
void* __fastcall get_native_camera_clear_color_00b6fe60(const void* actual_camera,
    void* unused_edx, void* actual_output_word);

// Genuine B156C0 entry through B1586C, ending BEFORE B1586D texture-holder
// acquisition. Includes current renderer slot2C frame-active gating, near/color
// capture, raw axes/velocity projection, full B0DB00 history, delayed VP copy,
// and current raw camera VP inverse/copy. Returns true only at that boundary;
// false means native frame-inactive exit. This is NOT whole command success.
//
// No renderer, clock, camera, history, owner or callable table is created. The
// raw service/command graph and the SAME actual CRT/constants must be supplied.
// Each B1BF40 call reloads the current camera; no camera snapshot is substituted.
// B17240 system-prefix preparation and the remaining B156C0 texture/target/
// batch/material/post stages are unresolved here. D5E13C must stay UNADMITTED
// in the existing queue execution domain. This API does not install a route.
bool execute_native_main_scene_prefix_00b156c0(void* actual_service,
    void* actual_command, NativeMainSceneExecutionPrefixContext&,
    NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneTextureConstantsContext {
    const volatile std::uint32_t* actual_texture_profile_00d61948;
    const volatile float* unsigned_bias_00ce3978;
    const volatile double* half_00d7a280;
    const volatile std::uint32_t* negative_one_00d7a260;
    const volatile std::uint32_t* one_00d7a24c;
    const NativeCameraFarCornersContext* far_corners;
};

// Continue the SAME actual service/command and persistent prefix state through
// B1586D..B15BD1, ending BEFORE B15BD2 bloom selection. Repeated genuine B4CB10
// holder getters and captured D61948 current slots3C/40 use B3CE50/B3CE60.
// Retains unsigned x87 conversion, half/full texel stores, all nine XY offsets,
// and current-camera full B0EB10 far rays. No owner allocation or ref changes.
// Only the established valid profile/raw-domain contract is admitted; throwing
// across the naked arithmetic helper is not supported. Never whole-command
// success: D5E13C remains unadmitted and all later renderer stages are required.
void continue_native_main_scene_texture_constants_00b1586d(void* actual_service,
    void* actual_command, const NativeMainSceneTextureConstantsContext&,
    NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneBloomTargetContext {
    const NativeMainSceneTextureConstantsContext& textures;
    const volatile double& adaptation_scale_00ce3cb8;
    const volatile std::uint32_t& negative_half_00ce69d0;
    const volatile std::uint32_t& half_float_00ce3800;
    NativeRendererFrameTargetsContext& targets;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
};

// Continue SAME actual service/command/state through B15BD2..B15E77. Execute
// the real bloom enabled/disabled branches and shared screen constants, save
// current camera+188 clear flags, set7/white through current cameras, assign
// the three current holder-primary surfaces to current service+1C8 groups,
// then resolve current renderer slot98 to full B24E70 over the SAME group and
// surface lifetime/synchronization/settings domain. No default targets, owner
// replacement, extra reference credit, cleanup or rollback. Real owner/callee
// failures preserve prior mutations and active_native_site; never replay a
// started stage. The ownership suffix uses ordinary C++ exception frames.
// Stops BEFORE B15E78 camera-frame execution. D5E13C remains UNADMITTED.
void continue_native_main_scene_bloom_targets_00b15bd2(void* actual_service,
    void* actual_command, const NativeMainSceneBloomTargetContext&,
    NativeMainSceneExecutionPrefixState&);

// Complete B6FDF0..B6FE0B: ECX actual camera, original stacked DWORD mode,
// EAX same camera, RET4. EDX is unused. Store full mode+198 and then the x86
// masked-shift mask+19C; no CameraFrameState projection or owner operation.
void* __fastcall set_native_camera_render_mode_00b6fdf0(void* actual_camera,
    void* unused_edx, std::uint32_t mode);

struct NativeMainSceneCameraGatherContext {
    const NativeMainSceneExecutionPrefixContext& prefix;
    const NativeMainSceneBloomTargetContext& bloom_targets;
    const NativeCameraFrameCommandContext& camera;
    const NativeSystemConstantGatherContext& system;
    NativeSystemConstantGatherFrame& system_frame;
};

// Continue SAME command/state through B15E78..B15EC3: real B71360, a fresh
// camera read to save mode+198, a fresh camera to set mode6, then a camera read
// for scene+A4 and another for full B46A70. Provider contexts borrow the SAME
// renderer publication/profile, synchronization, CRT, axes, timer and texture
// cells as the preceding stages. The service receiver is unused in this clause;
// B46A70 independently reloads its supplied original service publication.
//
// Each call requires a fresh persistent gather frame with the original local
// prefix preimage. Keep it and this continuation alive on failure; no replay,
// retirement, cleanup or restoration occurs here. Source checks occur at their
// reached call boundary, retaining prior camera/renderer mutations. Existing
// provider exception, private-stack-alias and FP limitations still apply.
// Stops BEFORE B15EC4 label allocation/FH3 ownership. D5E13C stays UNADMITTED.
void continue_native_main_scene_camera_gather_00b15e78(void* actual_service,
    void* actual_command, const NativeMainSceneCameraGatherContext&,
    NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneDrshLabelContext {
    const NativeSystemConstantGatherContext& system;
    NativeStringRawPoolContext& strings;
    const void* actual_drsh_bytes_00d5e500;
};

// One fresh persistent external frame, disjoint from actual native owners.
// The caller overwrites both temporary header words before its first resize;
// no unknown native local bytes are synthesized. Other fields preserve the
// native EBX data capture and exact FH3 ownership transitions. No destructor
// performs cleanup. Retain this frame and the same context at later boundaries.
struct NativeMainSceneDrshLabelFrame {
    std::uint32_t temporary_header[2]{};
    void* captured_data{};
    void* captured_service{};
    void* destination_header{};
    std::int32_t native_cleanup_state{-1};
    std::uint32_t active_native_site{};
    std::uint32_t cleanup_native_site{};
    bool started{};
    bool completed{};
    bool failed{};
    bool temporary_return_started{};
    bool temporary_return_completed{};
    bool cleanup_started{};
    bool cleanup_completed{};
};

// B15EC4..B15F62 only. Initialize the original local8B header, resize4 through
// the SAME actual raw string pool, capture EBX data, copy current length+1 from
// borrowed actual D5E500 bytes, then read current F8D39C and arm native state0.
// Retain native destination alias and current-length checks; after assignment
// transition to state2 BEFORE normal captured-data/current-length pool return.
// State0 unwinds CURRENT temporary header; state2 resets CURRENT service label.
// On success state2 remains armed across the B15F63 batch-selection boundary.
// Neither a fixed C++ label literal nor a projected string pool is substituted.
// Source catch cleanup follows the proven map, without rolling back camera or
// destination mutations. Later shader/batch code and D5E13C remain unadmitted.
void continue_native_main_scene_drsh_label_00b15ec4(
    const NativeMainSceneDrshLabelContext&, NativeMainSceneDrshLabelFrame&,
    NativeMainSceneExecutionPrefixState&);

// Explicit source lifecycle adapter for CBC460/CBC46B, not a native FH3 entry.
// Consumes only reached state0 or2, disarming before its real cleanup. State-1
// does nothing. Keep failed frames/callee state after cleanup; no reset/retry.
// A second C++ exception while unwinding terminates, as elsewhere in this domain.
void unwind_native_main_scene_drsh_label(const NativeMainSceneDrshLabelContext&,
    NativeMainSceneDrshLabelFrame&) noexcept;

// The label frame/context from B15EC4 survives this entire clause. All reached
// providers borrow the SAME renderer/manager/synchronization and job domain.
// Optional providers are checked only when their native call is reached.
struct NativeMainSceneBatchContext {
    const NativeMainSceneDrshLabelContext& label;
    NativeRenderBatchPreparationContext* preparation{};
    NativeRenderJobPublicationContext* jobs{};
    NativeRenderPreparationJobDispatch* preparation_dispatch{};
    NativeMaterialEntryDispatchContext* entries{};
};
struct NativeMainSceneBatchFrame {
    NativeMaterialEntryDispatchFrame* entries[2]{};
    bool started{};
    bool completed{};
    bool failed{};
    bool parallel{};
    std::uint32_t active_native_site{};
    std::uint32_t serial_preparations_completed{};
    std::uint32_t submissions_completed{};
    bool dispatch_completed{};
    std::uint32_t executions_completed{};
};

// Complete B15F63..B16067, stopping BEFORE the optional B10500 node clause.
// Current raw synchronization byte and signed counts >50 select real job
// submission/dispatch, followed by batch0 then batch1 execution. Serial path
// is prepare0/execute0/prepare1/execute1. Preserve each current batch reload,
// captured profile before B1BF40, and profile slot+08 read AFTER that getter.
// Parallel +08 mode writes precede each frame getter; captured batch/pool
// survive the preparation-job getter. No callback substitutes raw providers.
// State2 remains armed on success; C++ failure consumes it through the SAME
// explicit label unwind action. No scope/queued-job/native mutation rollback,
// native FH3 dispatch, later normal B16097 disarm, or D5E13C admission here.
void continue_native_main_scene_batches_00b15f63(void* actual_command,
    NativeMainSceneBatchContext&, NativeMainSceneDrshLabelFrame&,
    NativeMainSceneBatchFrame&, NativeMainSceneExecutionPrefixState&);

// The same retained label context and canonical queue/command/owner/string
// domains. Optional children are required only at their reached native calls.
struct NativeMainSceneExcludedNodeContext {
    const NativeMainSceneDrshLabelContext& label;
    NativeRenderBatchPreparationContext* queue{};
    NativeRenderSceneSubmissionContext* submission{};
    NativeRenderQueueExecutionContext* execution{};
};
struct NativeMainSceneExcludedNodeFrame {
    NativeRenderSceneSubmissionOperation* submission{};
    NativeRenderQueueExecutionFrame* execution{};
    std::uint32_t saved_clear_flags{};
    std::uint32_t active_native_site{};
    bool started{};
    bool entered{};
    bool visibility_raised{};
    bool flags_cleared{};
    bool submitted{};
    bool executed{};
    bool flags_restored{};
    bool visibility_cleared{};
    bool completed{};
    bool failed{};
};

// Full B10500..B105AF [176], native ECX service, command/targets, RET8.
// Service+22C gates all child access. Capture current camera flags after FLD1,
// set CURRENT service+1C0 node visibility1, clear current-camera flags, capture
// current-camera scene before queue getter, capture current node before the
// next camera getter, submit through actual B1F5E0, get CURRENT queue again,
// execute/pop its last command through raw B1E6D0, restore saved flags on the
// current camera, then FLDZ and CURRENT node visibility0. No local EH cleanup:
// failed submission/execution leaves earlier visibility/flags/queue mutations.
void execute_native_excluded_node_00b10500(void* actual_service,
    void* actual_command, void* actual_targets, NativeMainSceneExcludedNodeContext&,
    NativeMainSceneExcludedNodeFrame&);

struct NativeMainSceneExcludedRestoreFrame {
    NativeMainSceneExcludedNodeFrame excluded;
    std::uint32_t active_native_site{};
    bool started{};
    bool excluded_called{};
    bool render_mode_restored{};
    bool label_reset_completed{};
    bool clear_flags_restored{};
    bool near_restored{};
    bool completed{};
    bool failed{};
};

// B16068..B160CF. Test CURRENT service+22D before calling full B10500 with
// current service+1C8 targets. Keep state2 armed through that call and render
// mode restore. Disarm at B16097 BEFORE normal current-service B160A2 reset;
// a throwing reset must never reset again. Then restore saved clear flags and
// near plane on separately reloaded cameras, retaining native FLD/FSTP32 near
// conversion. Stop BEFORE service+60 virtual10 at B160D0. Source failure only
// consumes still-armed state2; it never restores earlier native mutations.
void continue_native_main_scene_excluded_restore_00b16068(void* actual_service,
    void* actual_command, NativeMainSceneExcludedNodeContext&,
    NativeMainSceneDrshLabelFrame&, const NativeMainSceneBatchFrame&,
    NativeMainSceneExcludedRestoreFrame&, NativeMainSceneExecutionPrefixState&);

struct NativeMainScenePostGatherContext {
    NativePostEffectExecutionContext& post;
    const NativeSystemConstantGatherContext& system;
    // B107F0 publishes this actual depth-downscale profile at service+60;
    // its current +10 must select full raw B4DFB0 -> B4DCC0.
    const volatile std::uint32_t* depth_profile_00d5e164;
};
struct NativeMainScenePostGatherFrame {
    NativePostEffectDrawFrame& post;
    NativeSystemConstantGatherFrame& system;
    std::uint32_t active_native_site{};
    bool started{};
    bool depth_draw_completed{};
    bool system_gather_completed{};
    bool color_restored{};
    bool completed{};
    bool failed{};
};

// B160D0..B16109: current service+60/current+10 depth draw, fresh camera scene
// capture before another fresh camera and actual B46A70, then saved color
// capture before a third fresh camera and B6FE50. Incoming DRSH state is -1;
// no local cleanup, retry, unmap or earlier-state rollback. Fresh persistent
// post/material/system frames share the same physical renderer/service domain.
// Stops BEFORE B1610A SCNE string construction; no whole D5E13C admission.
void continue_native_main_scene_post_gather_00b160d0(void* actual_service,
    void* actual_command, NativeMainScenePostGatherContext&,
    const NativeMainSceneDrshLabelFrame&, const NativeMainSceneExcludedRestoreFrame&,
    NativeMainScenePostGatherFrame&, NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneScneLabelContext {
    // Preserve the same actual global-service and raw StringManager cells
    // already used by DRSH; the SCNE bytes are borrowed from actual D5E4F8.
    const NativeMainSceneDrshLabelContext& domains;
    const void* actual_scne_bytes_00d5e4f8;
};
struct NativeMainSceneScneLabelFrame {
    std::uint32_t temporary_header[2]{};
    void* captured_data{};
    void* captured_service{};
    void* destination_header{};
    std::int32_t native_cleanup_state{-1};
    std::uint32_t active_native_site{};
    std::uint32_t cleanup_native_site{};
    bool started{};
    bool completed{};
    bool failed{};
    bool temporary_return_started{};
    bool temporary_return_completed{};
    bool cleanup_started{};
    bool cleanup_completed{};
};

// B1610A..B161AC only: initialize the temporary8B header, resize4, capture
// data, copy current length+1 from borrowed SCNE bytes, capture current global
// service, THEN arm state3. Preserve destination alias/current-length gates.
// State5 is installed BEFORE normal captured-data/current-length pool return;
// successful completion leaves5 armed for the following target-selection body.
// State3 cleanup destroys CURRENT temporary header; state5 resets CURRENT
// global service label. No native FH3 entry, automatic destruction or rollback.
void continue_native_main_scene_scne_label_00b1610a(
    const NativeMainSceneScneLabelContext&, NativeMainSceneScneLabelFrame&,
    NativeMainSceneExecutionPrefixState&);
// Explicit source adapters for CBC473/CBC47E. Both reached map entries go to
// -1, installed before real cleanup. A failed cleanup during unwind terminates.
// State4 -> (3,CBC47E) exists in the map but is not installed by this clause.
void unwind_native_main_scene_scne_label(const NativeMainSceneScneLabelContext&,
    NativeMainSceneScneLabelFrame&) noexcept;

struct NativeMainSceneScneTargetContext {
    NativeRendererFrameTargetsContext& targets;
    const NativeCameraFrameCommandContext& camera;
    const NativeMainSceneScneLabelContext& label;
};
struct NativeMainSceneScneTargetFrame {
    std::uint32_t active_native_site{};
    bool started{};
    bool color_installed{};
    bool targets_bound{};
    bool camera_executed{};
    bool completed{};
    bool failed{};
};

// B161AD..B161F4: current fields of the original ESI service select +48/+44
// by byte+22D; borrow primary+C, install color0 in current+1CC, then capture
// current renderer/profile BEFORE reloading current+1CC and slot98. Execute
// B71360 on a fresh command camera. All descendants share the same actual
// renderer, profile, synchronization and CRT domain used by SCNE's system.
// State5 stays armed through success. Failure consumes it once using CURRENT
// global service, preserving target/camera mutations; never rollback/replay.
// Stops BEFORE B161F5 batch selection. New interface is not native caller ABI.
void continue_native_main_scene_scne_targets_00b161ad(void* actual_service,
    void* actual_command, const NativeMainSceneScneTargetContext&,
    NativeMainSceneScneLabelFrame&, NativeMainSceneScneTargetFrame&,
    NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneScneBatchContext {
    const NativeMainSceneScneLabelContext& label;
    // Reuse the established actual preparation/publication/dispatch domain;
    // providers.label must be this SAME label.domains object, not a copy.
    NativeMainSceneBatchContext& providers;
};

// B161F5..B162FD. Same actual signed-count/synchronization gates, concrete
// job publication/worker wait and batch operations as the DRSH clause, with
// this clause's own call sites and state5. Current mode stores precede each
// frame getter; current batch/pool capture precedes the preparation getter.
// Capture batch/profile before each fresh camera; read captured slot8 after
// that getter. B55550 independently reloads its actual renderer and ignores
// its camera/index arguments. Keep the current-job scope/slot failure effects.
// Success reaches B162FE with5 armed; failure disarms before CURRENT global
// service reset. No retry, native FH3 dispatch or whole-scene admission.
void continue_native_main_scene_scne_batches_00b161f5(void* actual_command,
    NativeMainSceneScneBatchContext&, NativeMainSceneScneLabelFrame&,
    NativeMainSceneBatchFrame&, NativeMainSceneExecutionPrefixState&);

struct NativeSurfaceCopyContext {
    // Current original-token D619A0 profile, through slot20. Its dimension
    // slots select the concrete raw B3CD10/B3CD20 leaves, never callbacks.
    const volatile std::uint32_t* actual_surface_profile_00d619a0;
};

// Full B3CDA0, native ECX destination/stack source/RET4. GetDevice on current
// destination+2C, then separately read its current width/height virtuals.
// Reload destination+2C BEFORE source+2C; call the CAPTURED device's current
// StretchRect+88 with null source RECT, destination {0,0,width,height}, filter2.
// Ignore HRESULTs and release that captured device through its current slot8.
// There is no local EH cleanup, wrapper retain/release or fallback device.
void copy_native_surface_00b3cda0(void* actual_destination,
    void* actual_source, const NativeSurfaceCopyContext&);

// Full19B B4CAF0: byte+14 gates every child access; on nonzero capture source
// +0C, THEN destination+10 and call the raw child above. No holder virtual.
void capture_native_render_holder_00b4caf0(void* actual_holder,
    const NativeSurfaceCopyContext&);

struct NativeMainSceneScneCaptureContext {
    const NativeMainSceneScneLabelContext& label;
    NativeRendererTextureBindingContext& textures;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const NativeSurfaceCopyContext& surface_copy;
};
struct NativeMainSceneScneCaptureFrame {
    std::uint32_t active_native_site{};
    std::uint32_t samplers_completed{};
    bool started{};
    bool capture_completed{};
    bool label_reset_completed{};
    bool completed{};
    bool failed{};
};

// B162FE..B1634D. Original service byte+22D chooses current holder+48/+44;
// raw capture precedes eight current global-renderer/current virtual130 null
// texture binds. State5 stays armed through every bind, then becomes -1 BEFORE
// normal B13510 reset on CURRENT global service. A failed normal reset never
// retries cleanup. Earlier failure consumes state5 once, without rollback.
// Stops at B1634E BEFORE POST label construction; not native FH3/game ABI.
void continue_native_main_scene_scne_capture_00b162fe(void* actual_service,
    const NativeMainSceneScneCaptureContext&, NativeMainSceneScneLabelFrame&,
    NativeMainSceneScneCaptureFrame&, NativeMainSceneExecutionPrefixState&);

struct NativeMainScenePostLabelContext {
    const NativeMainSceneDrshLabelContext& domains;
    const void* actual_post_bytes_00d5e4f0;
};
struct NativeMainScenePostLabelFrame {
    std::uint32_t temporary_header[2]{};
    void* captured_data{};
    void* captured_service{};
    void* destination_header{};
    std::int32_t native_cleanup_state{-1};
    std::uint32_t active_native_site{};
    std::uint32_t cleanup_native_site{};
    bool started{};
    bool completed{};
    bool failed{};
    bool temporary_return_started{};
    bool temporary_return_completed{};
    bool cleanup_started{};
    bool cleanup_completed{};
    bool service_flag_cleared{};
};

// B1634E..B163F9 only. Clear ORIGINAL service byte680 BEFORE zeroing/resizing
// the temporary; copy current length+1 from borrowed POST bytes. Capture the
// CURRENT global service, THEN arm6, preserving alias/current-length gates.
// Arm8 BEFORE normal captured-data/current-length pool return; reach B163FA
// with8 armed, before its post-rendering gate. Unarmed failure preserves the
// cleared flag/zeroed temporary; no rollback, retries or POST-effect execution.
void continue_native_main_scene_post_label_00b1634e(void* actual_service,
    const NativeMainScenePostLabelContext&, NativeMainScenePostLabelFrame&,
    NativeMainSceneExecutionPrefixState&);
// Explicit source CBC486/CBC491 actions:6 destroys CURRENT temporary,8 resets
// CURRENT global service. Both disarm to-1 before cleanup; a second cleanup
// exception terminates. Map7->(6,CBC491) exists but this clause never installs7.
void unwind_native_main_scene_post_label(const NativeMainScenePostLabelContext&,
    NativeMainScenePostLabelFrame&) noexcept;
} // namespace bsp
