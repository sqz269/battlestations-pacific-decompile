#pragma once

// Segment 74 sound routines reconstructed by cc13-audio1. Names are hypotheses,
// not recovered symbols. Evidence, original ABIs and open questions are in
// docs/CC13_SOUND_STREAMING.md. These are behavioural C++ interfaces over the
// existing projections (SoundClassOwnership, SoundConfigurationState) or over
// raw native objects addressed by offset; none is a drop-in binary replacement.
// FMOD, the VFS and diagnostics stay host interfaces.

#include "bsp/audio_online_startup.hpp"
#include "bsp/input_action_classifier.hpp"
#include "bsp/native_string.hpp"
#include "bsp/sound_class_ownership.hpp"
#include "bsp/sound_configuration.hpp"
#include "bsp/sound_manager_levels.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::cc13_audio {

// ---------------------------------------------------------------------------
// Class ducking, class pause and listener environment (sound manager 00F8BBD8)
// ---------------------------------------------------------------------------

// 00A7AC70, __thiscall(manager, uint32 mask, float level), RET 8.
// Same walk as 00A7ABF0 but on descriptor +14h, the factor GetMixedVolume
// (00A7AAA0) multiplies with descriptor +10h. Bit positions are array ordinals
// (SHL CL, so ordinals >= 32 alias). FUCOMIP equality removes the bit from the
// pending mask; NaN counts as a change, +0/-0 as equal. Any residual bits reach
// 00A7A3F0. Native dereferences every visited slot: all slots must be nonnull.
void set_sound_class_duck_levels_00a7ac70(SoundClassOwnership& classes,
    std::uint32_t mask, float level) noexcept;

// 00A7D120, __thiscall(manager, byte push), RET 4. Nesting counter at
// manager+16Ch. push: increments; only the 0->1 transition applies level 0.5
// (00CE3800). pop: decrements; only reaching 0 applies 1.0. The mask is
// 0xFFFF - bit(GUITestSpeech) - bit(Warnings) - bit(GUIMusic), computed by
// subtraction exactly as native (a missed name is -1, i.e. bit 31; two names on
// the same ordinal subtract twice). Callers: voice slot start/poll/stop and the
// HUD map screen. names resolves the three literals through 00A7ACF0.
void push_pop_sound_speech_duck_00a7d120(SoundClassOwnership& classes,
    std::int32_t& duck_depth_16c, bool push, SoundLevelNameHost& names,
    NativeStringStorage& strings);

// 00A7F890, __thiscall(manager, signed index, byte paused), RET 8.
// Grows the class table (00A7C2C0) when index >= count, compares descriptor
// +0Ch, and only on a difference grows again and stores. Descriptor +0Ch is the
// class pause byte: 00A896A4 and the debug dump OR it into the paused state.
// Requires index >= 0 and a nonnull descriptor at index after growth.
void set_sound_class_paused_00a7f890(SoundClassOwnership& classes,
    std::int32_t index, std::uint8_t paused);

// 00A7B710, __thiscall(manager, const char*), RET 4. Uses the listener table
// at manager+A4: when 00A7AE00 gives a new ordinal, +110h receives the old
// +10Ch, +10Ch the new ordinal, then 00A7AE80 selects by the same name.
// The in-mission interface passes "Cockpit", "Air" or "Underwater".
void set_sound_listener_environment_00a7b710(SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names);

// 00A7D9D0, __thiscall(manager, RefPtr<descriptor> by value), RET 4.
// Out-of-line copy of setup's inlined 00A80EBA insertion: descriptor +08h
// receives the current class count, 00A7C240 appends and retains it, then the
// by-value reference is released (refcount at +4, vslot0 at zero). No native
// caller was found (no rel32 or absolute reference). descriptor must be nonnull
// and carry the reference the by-value parameter owned.
void add_sound_class_00a7d9d0(SoundClassOwnership& classes,
    SoundClassDescriptor* by_value_reference);

// ---------------------------------------------------------------------------
// Dialog / music streams (actual 54h D5B360 objects, raw offsets)
// ---------------------------------------------------------------------------

// 00A85C40, __thiscall(stream, byte paused), RET 4. Stores +08h only when the
// value differs and state +20h is 1 (STARTING) or 2 (PLAY). The stream update
// applies +08h through Channel::setPaused. Caller 00A77760 (pause all streams).
void set_sound_stream_paused_00a85c40(void* stream, std::uint8_t paused) noexcept;

// 00A86710, __thiscall(stream, native-string* name), RET 4. Finds the row by
// name in the dialog table +3Ch (00A865F0; a miss selects row 0), then for every
// row of table+0Ch writes target 1.0f/0.0f and rate |current-target|/3.0 to the
// 12-byte {target,current,rate} records at +48h. The difference is divided as a
// double (x87 at 53-bit precision), rounded to float, and a nonpositive or NaN
// quotient is replaced by -0.0f minus it (so equality yields -0.0f).
void select_sound_stream_row_00a86710(void* stream, const void* actual_name_header) noexcept;

class SoundStreamDebugHost {
public:
    virtual ~SoundStreamDebugHost() = default;
    // 00C2DE18 Channel::getPosition(channel, &ms, FMOD_TIMEUNIT_MS=1).
    virtual FmodResult channel_get_position_ms(void* channel, std::uint32_t* ms) = 0;
    // 00A7A460 with "Out of sounjd memory:" when the result is 2Bh.
    virtual void on_out_of_sound_memory(const char* message) = 0;
    // 004C14C0 then 004B47E0(sink, float x, float y, fmt, ...). In this build
    // 004B47E0 is a trivial body, so retail draws nothing.
    virtual void draw_text(float x, float y, const std::string& text) = 0;
};

// 00A869B0, __thiscall(stream, int x, int y), RET 8, returns y in EAX.
// Only in state 2 (PLAY): "%s st:%s pos:%.2f" with the +30h name (null data
// prints the shared empty string 00F8BBEE), state name from 00E12F00 and
// max(saved +18h, channel ms) / 1000.0 as an unsigned integer. Then one
// "%s(%s) vol:%.2f" line per row (count +4Ch) at x+32: row name, speaker layout
// name from 00E12F10 indexed by table row +10h, and the row's current level.
// Lines advance y by 12. The native ms local is uninitialized when getPosition
// does not write it; this interface passes 0 instead.
std::int32_t draw_sound_stream_debug_00a869b0(void* stream, std::int32_t x,
    std::int32_t y, SoundStreamDebugHost& host);

// ---------------------------------------------------------------------------
// Sample settings apply and .def write-back (sample object, raw offsets)
// ---------------------------------------------------------------------------

class SoundSampleApplyHost {
public:
    virtual ~SoundSampleApplyHost() = default;
    virtual FmodResult sound_set_3d_min_max_distance(void* sound, float min, float max) = 0; // 00C2DE9C
    virtual FmodResult sound_set_variations(void* sound, float frequency, float volume, float pan) = 0; // 00C2DE96
    virtual FmodResult sound_get_mode(void* sound, std::uint32_t* mode) = 0; // 00C2DEA8
    virtual FmodResult sound_set_mode(void* sound, std::uint32_t mode) = 0; // 00C2DE8A
    virtual void on_out_of_sound_memory(const char* message) = 0; // 00A7A460 on 2Bh
    // 0093FD00: assign the parameter vector embedded at descriptor +3Ch.
    virtual void assign_parameters(void* destination_header, const void* source_header) = 0;
    // [0109CEEC] vslot +4 (name header, 35h); null when the file cannot open.
    virtual void* open_definition_file(const void* actual_name_header, std::uint32_t mode) = 0;
    // File vslot +5C(&builder, 0) followed by vslot +4(1).
    virtual void write_and_delete_definition_file(void* file, const std::string& text) = 0;
    // 00BD1D90 / 00BD1BB0 with the freshly constructed 00426500 builder.
    virtual std::string format_float(float value) = 0;
    virtual std::string format_int(std::int32_t value) = 0;
};

// 00A82AE0, __thiscall(destination, source), RET 4, returns destination.
// Copies the 3Ch-byte sample descriptor field by field (dwords 00..10, bytes
// 14..16, dwords 18/1C, bytes 20..22, dwords 24..34, byte 38), then assigns the
// parameter vector at +3Ch through 0093FD00. Padding bytes 17h, 23h, 39h..3Bh
// are not written. Dwords 04..10, 18, 1C and 2C..34 move through FLD/FSTP, so
// a signalling NaN arrives quieted; 00, 24 and 28 move as integers.
void* copy_sound_sample_descriptor_00a82ae0(void* destination, const void* source,
    SoundSampleApplyHost& host);

// The .def text 00A83120 builds, in native append order: "3D"/"2D", Vol,
// Pitch, optional Loop, MinDistance, MaxDistance, "Decompress <int>",
// optional Ambient, optional Linear, FreqRndHz, VolRnd and, with sample +1Eh,
// "LoopPointsMillisec <start> <end>". sample is the whole object (descriptor at +08h).
std::string format_sound_sample_definition_00a83120(const void* sample,
    SoundSampleApplyHost& host);

// 00A83120, __thiscall(sample, descriptor by value [48h bytes]), RET 48h.
// Copies the descriptor to sample+08h, then on resource(+78h)+10h:
// set3DMinMaxDistance(+14h,+18h), setVariations(+38h,+3Ch,0), getMode and
// setMode with Linear (+28h) choosing LINEARROLLOFF 200000h over LOGROLLOFF
// 100000h and Decompress (+1Ch) choosing CREATESAMPLE 100h over
// CREATECOMPRESSEDSAMPLE 200h. Every result 2Bh reports out of memory. When the
// sample name (+50h) contains ".def", the definition text is written back
// through the VFS (mode 35h). The by-value parameter vector is destroyed by the
// callee (0093F950 resize 0, then free); that cleanup belongs to the caller here.
void apply_sound_sample_settings_00a83120(void* sample, const void* descriptor,
    SoundSampleApplyHost& host);

// ---------------------------------------------------------------------------
// Sound instance / event virtuals (base vtable D5ABB8, event vtable D5B480)
// ---------------------------------------------------------------------------

// Base-class defaults, never defined in Ghidra before (referenced only from
// vtables D5ABB8, D5ABF8, D5AD58 and D5B480):
// 00A7BE50 slot +18 handle: XOR EAX,EAX; RET -> null.
void* sound_instance_default_handle_00a7be50() noexcept;
// 00A7BE60 slot +1C audibility: FLD1; RET -> 1.0.
float sound_instance_default_audibility_00a7be60() noexcept;
// 00A7BEA0 slot +28 parameter lookup (out, name), RET 8: writes {0, 0.0f, 0.0f, 0}.
void* sound_instance_default_parameter_00a7bea0(void* out16, const void* name) noexcept;
// 00A7BED0 slot +38 vector query (out), RET 4: writes three 0.0f.
void* sound_instance_default_vector_00a7bed0(void* out12) noexcept;
// 00A7BE70 slot +2C (RET 8) and 00A7BE80 slot +34 (RET 18h) are empty bodies.

class SoundEventHost {
public:
    virtual ~SoundEventHost() = default;
    virtual FmodResult event_get_state(void* event, std::uint32_t* state) = 0; // 00C2DF9E
    virtual FmodResult event_set_pitch(void* event, float pitch, std::int32_t units) = 0; // 00C2DFB6
    virtual FmodResult event_set_volume(void* event, float volume) = 0; // 00C2DFB0
    virtual FmodResult event_set_paused(void* event, std::uint8_t paused) = 0; // 00C2DF98, pushed as a byte
    virtual void advance_fade(void* instance, float dt) = 0; // 00A7A2E0
    virtual void create_and_start(void* instance) = 0; // 00A894E0
    virtual float mixed_volume(void* instance) = 0; // 00A7AAA0
    virtual void update_parameters(void* instance) = 0; // 00A89560
};

// 00A89210 event slot +0C, ECX instance, RET, AL. Finished byte +15h -> true;
// no event +54h -> false; otherwise getState into a zeroed local and return
// whether FMOD_EVENT_STATE_PLAYING (8) is clear. The result is ignored.
bool sound_event_is_completed_00a89210(void* instance, SoundEventHost& host);

// 00A89600 event slot +30, ECX instance, stack float dt + unused dword, RET 8.
// Mirrors channel update 00A7AF10 for FMOD events: skip when finished (+15h);
// advance fade; count the start delay +3Ch down and clear hold +38h once it is
// not above zero (an unordered result keeps the hold); create the event when
// +54h is null; increment tick +18h; when dirty +14h, setPitch(+20h - 1.0, RAW)
// and setVolume(mixed volume) and clear +14h; update parameters; finally
// setPaused(hold | instance paused +1Ch | class paused class(+44h)+0Ch).
void update_sound_event_00a89600(void* instance, float dt, SoundEventHost& host);

// ---------------------------------------------------------------------------
// Directional shadow cascades (same segment; directional shadow owner object)
// ---------------------------------------------------------------------------

// 00A8AC90, __fastcall(ECX float3* in/out, EDX matrix), RET. Transforms
// {x,y,z,1} through 00B62D10, then stores x/w, y/w, z/w with 1/w rounded to
// float first (FLD1/FDIVRP/FSTP) and each product rounded by its FSTP.
// Caller 00A8ECB0.
void project_shadow_point_00a8ac90(float* inout_xyz, const void* actual_matrix_float16);

// 00A8B860, ECX out float2, EDX p0, stack p1, q0, q1, RET 0Ch. Intersection of
// line p0-p1 with line q0-q1: inv = 1/det rounded to float and stored to out[1]
// and out[0] first; the two cross terms are rounded to float; every other
// intermediate stays on the x87 stack (53-bit, reproduced with double, where
// no float-derived product or difference can overflow or go subnormal). The
// y term rereads q0.y, q1.y, p0.y, p1.y and out[1] after out[0] is stored, so
// aliasing out with an input behaves as native. No zero-determinant guard.
// Caller 00A8B930 (unreferenced).
void intersect_lines_2d_00a8b860(float* out, const float* p0, const float* p1,
    const float* q0, const float* q1) noexcept;

class ShadowCascadeSubmitHost {
public:
    virtual ~ShadowCascadeSubmitHost() = default;
    // 00A8BD20, __thiscall(owner, float near, float far, int cascade), RET 0Ch.
    virtual void fit_cascade(void* owner, float near_distance, float far_distance, std::int32_t cascade) = 0;
    virtual void refresh_world_matrix(void* camera) = 0; // 00B6DB70
    virtual void set_extra_clip_plane(void* camera, const float* plane4) = 0; // 00B70410
    virtual void clear_extra_clip_plane(void* camera) = 0; // 00B70470
    // operator_new(44h) then 00B1F1F0(owner +34h, cascade camera, render camera, target).
    virtual void* new_render_command(void* command_owner, void* cascade_camera,
        void* render_camera, void* target) = 0;
    virtual void set_metadata(void* command, const std::uint32_t* metadata3) = 0; // 00B1BF50
    // 00B1D910 with a temporary native string holding the four bytes at 00D5B5D0.
    virtual void set_diagnostic_name(void* command, const char* name) = 0;
    // 004C11F0 queue, then 00B1EB80(queue, command, immediate).
    virtual void append_shadow_command(void* command, std::uint32_t immediate) = 0;
    // 004C1130 pool, (pool+4)->vslot1(00A8E090 shadow job, command).
    virtual void submit_frame_job(void* command) = 0;
    // (pool+4)->vslot2(0) after the loop.
    virtual void flush_frame_jobs() = 0;
};

// 00A8EA20, __fastcall(owner), SEH frame, RET. Sets the four cascade far
// distances +390h..+39Ch to 35, 100, 300, 1000 and +4ECh to 1.5, fits cascades
// [5,35], [35,100], [100,300], [300,1000] through 00A8BD20, then for each of the
// four cascade cameras (+20h + 4i) with render camera +30h (reloaded per pass):
// refresh the render camera's world matrix unless its flags +5Ch bit1 is set;
// when the render camera height +124h is below -5.0 (00CFBC84) give the cascade
// camera the clip plane {0,-1,0,2}, otherwise clear it; build a render command
// for target +4F0h + 4i with metadata {0, 62h, 1Eh} and name "SHBU", queue it
// and submit it as a shadow frame job. A failed allocation leaves the command
// null and native then calls through it. Caller BSP_ShadowMap_UpdateViewFromCamera.
void submit_shadow_cascades_00a8ea20(void* owner, ShadowCascadeSubmitHost& host);

class ShadowSingleViewHost {
public:
    virtual ~ShadowSingleViewHost() = default;
    virtual void retain(void* camera) = 0;  // InterlockedIncrement(camera+4), [00CE221C]
    virtual void release(void* camera) = 0; // InterlockedDecrement(camera+4), vslot0 at zero
    // 00A8AAA0, __thiscall(owner, out float16, viewport), RET 8.
    virtual void build_viewport_matrix(void* owner, float* out16, void* viewport) = 0;
    virtual void set_viewport(void* camera, void* viewport) = 0; // 00B71990
    // 00A8AEB0, __thiscall(owner, camera, a, b), RET 0Ch.
    virtual void fit_single_view(void* owner, void* camera, std::uint32_t a, std::uint32_t b) = 0;
    // 00413920 left*right into destination, then 004134F0 copy into owner+144h.
    virtual void multiply_into_shader_matrix(void* owner, const float* left16, const float* right16) = 0;
    // [00F8C210] 00AD7A30(camera, 1.0f); only the ground-ortho path calls it.
    virtual void prepare_ground_casters(void* camera, float weight) = 0;
    // 004C11F0 queue, 00B1F4D0(owner +34h, camera +20h, target +500h, name by value).
    virtual void queue_named_command(void* command_owner, void* camera, void* target, const char* name) = 0;
    // 004C11F0 queue, 00B1F5E0(owner +34h, camera +20h, target +500h, first argument).
    virtual void queue_command(void* command_owner, void* camera, void* target, std::uint32_t argument) = 0;
    // 004C11F0 queue, 00B1CB50(&metadata3).
    virtual void set_last_command_metadata(const std::uint32_t* metadata3) = 0;
};

// 00A8E860, __thiscall(owner, camera, a, b), SEH frame, RET 0Ch. Caller
// BSP_InterfaceView_DrawGroundOrtho. Only when the settings object at
// [00F8BBF0] has bytes +0Ch and +0Dh set: retain camera into +30h (assign on
// change: store, retain new, release old), build the viewport matrix for
// viewport +504h, set it on the shadow camera +20h, fit the view (00A8AEB0 with
// camera, a, b), shader matrix +144h = (+44h) * viewport matrix, overwrite its
// third column (+14Ch/+15Ch/+16Ch/+17Ch) with +4ACh..+4B8h through FLD/FSTP,
// prepare ground casters for the camera with weight 1.0, queue the SHBU command
// for target +500h with metadata {0, 62h, 1Eh}, then release and clear +30h.
void render_shadow_ground_ortho_view_00a8e860(void* owner, const void* settings_00f8bbf0,
    void* camera, std::uint32_t a, std::uint32_t b, ShadowSingleViewHost& host);

// 00A8E180, __thiscall(owner, argument, camera, a, b), RET 10h. Caller 00B3C960.
// Same as 00A8E860 except that it has no caster preparation and queues
// 00B1F5E0 with its first argument instead of the named SHBU command.
void render_shadow_single_view_00a8e180(void* owner, const void* settings_00f8bbf0,
    std::uint32_t argument, void* camera, std::uint32_t a, std::uint32_t b,
    ShadowSingleViewHost& host);

// ---------------------------------------------------------------------------
// Input action binding queries (same segment; InputActionBinding projection)
// ---------------------------------------------------------------------------

// 00A92050, ECX 30h action, RET, AL. True when any binding (vector +10h,
// count +14h, stride 34h) has byte +14h (force_unit_scale) set. Sibling of
// 00A92090. Callers: HUD unit-order controls and 0060ABD0.
bool any_input_binding_forces_unit_scale_00a92050(const std::vector<InputActionBinding>& bindings) noexcept;

// 00A92910, __thiscall(manager, int first, int second), RET 8, AL. Actions are
// manager+4 + index*30h. True when some binding of the first action and some
// binding of the second both have device_class (+4) and input_code (+10h)
// different from -1 and equal to each other. The first count is read once,
// the second per pass. Caller 005F6EF0 (control binding screen).
bool input_actions_share_binding_00a92910(const std::vector<InputActionBinding>& first,
    const std::vector<InputActionBinding>& second) noexcept;

} // namespace bsp::cc13_audio
