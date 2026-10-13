#pragma once

// cc13-sweep6D: marker instances and the marker manager's GUI-highlight edits
// (docs/CC13_BAND_006D_006F.md). Reconstructed and build-tested only; nothing
// here is wired into the game host, fixture-tested or game-validated.
//
// The world tick's marker walk (update_markers_006dc1a0, world_entities.hpp)
// reaches an instance only through host.marker_update; the functions below are
// what that call lands in (006D61F0 -> 006D7FC0). The highlight edits fill the
// MarkerManagerState vectors the same walk reads.

#include "bsp/marker_classes.hpp"
#include "bsp/world_entities.hpp"

#include <cstdint>

namespace bsp {

// Native 20h object, vtable 00CF8F68: slot 0 scalar deleting destructor
// 006D7F80 (body 006D74D0), slot 1 tick 006D61F0, slot 2 pose 006D7FC0.
struct MarkerInstanceState {
    void* owner_04{};                                   // entity, or null for a position marker
    float position_08[3]{};                             // used when owner_04 is null
    const MarkerClassDescriptor* descriptor_14{};       // class from MarkerClasses
    void* scene_instance_18{};                          // created from descriptor_14->resource_14
    float phase_1c{};                                   // random start phase unless synchronized
};

// Everything the instance code reaches outside its own 20h block.
struct MarkerInstanceHost {
    virtual ~MarkerInstanceHost() = default;
    // descriptor->resource_14 vtable+8h, __thiscall(0, 1.0f); returns the scene instance.
    virtual void* create_scene_instance(void* resource, std::int32_t zero, float one) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, ECX = instance+0Ch, arg [[00E188A8]+19ECh].
    virtual void register_scene_instance(void* scene_instance) = 0;
    // 00BD2F10 with ECX=1: generator stream 1, not the gameplay stream 0.
    virtual float random_uniform_stream1(float lo, float hi) = 0;
    // [[00E188A8]+64Ch], the mission clock the pose phase is built from.
    virtual float game_time_64c() = 0;
    // owner+C8h byte; when clear the pose calls BSP_EntityPose_RefreshWorld first.
    virtual bool owner_world_pose_valid(void* owner) = 0;
    virtual void refresh_owner_world_pose(void* owner) = 0;
    // owner+FCh..+104h, the world position after the refresh.
    virtual void owner_world_position(void* owner, float out[3]) = 0;
    // [scene_instance+0Ch] vtable+34h with the 4x4 pose (BSP_Transform_SetWorldMatrix slot).
    virtual void set_scene_instance_world_matrix(void* scene_instance, const float matrix[16]) = 0;
    // scene_instance vtable+4h, __thiscall(1).
    virtual void release_scene_instance(void* scene_instance) = 0;
};

// 006D8750, __thiscall(this, descriptor, owner, x, y, z), RET 14h.
void construct_marker_instance_006d8750(MarkerInstanceState& instance,
                                        const MarkerClassDescriptor& descriptor, void* owner,
                                        float x, float y, float z, MarkerInstanceHost& host);
// 006D74D0, ECX = instance, RET. Body of the slot 0 destructor.
void destruct_marker_instance_006d74d0(MarkerInstanceState& instance, MarkerInstanceHost& host);
// 006D61F0, slot 1, __thiscall(float delta), RET 4. Delta is not used.
void tick_marker_instance_006d61f0(MarkerInstanceState& instance, float delta,
                                   MarkerInstanceHost& host);
// 006D7FC0, slot 2, ECX = instance, RET.
void update_marker_instance_pose_006d7fc0(MarkerInstanceState& instance, MarkerInstanceHost& host);
// 006D7FC0 without the host: the 4x4 world matrix for a given position and
// phase (identity rotation for modes 0 and 4). Row-major, translation in 12..14.
void marker_instance_pose_matrix(std::int32_t animation_mode, const float position[3], float phase,
                                 float out[16]) noexcept;

// 006D6170, (const float a[3], const float b[3]), RET 8, AL result. The
// positionMarkers map key order: x, then y, then z; any unordered compare is false.
bool marker_vec3_less_006d6170(const float a[3], const float b[3]) noexcept;

// What the highlight removal does to an element besides erasing it.
struct MarkerHighlightHost {
    virtual ~MarkerHighlightHost() = default;
    // element+94h = (float)(0.28 * 0.0).
    virtual void set_highlight_level(void* element, float level) = 0;
    // 006DBAC0 with ECX = element and the walk-4 arguments (1, 0, 1.0f).
    virtual void apply_highlight_pulse_006dbac0(void* element, int mode, int flag, float value) = 0;
    // element vtable+54h / +50h, four floats.
    virtual MarkerColor get_color(void* element) = 0;
    virtual void set_color(void* element, const MarkerColor& color) = 0;
};

// 006DAE20, ECX = manager; (element, recursive, tinted), RET 0Ch.
void add_gui_highlight_006dae20(MarkerManagerState& markers, float& pulse_epoch_00e19970,
                                float game_time_64c, void* element, bool recursive, bool tinted);
// 006DC5A0, ECX = manager; (element), RET 4.
void remove_gui_highlight_006dc5a0(MarkerManagerState& markers, void* element,
                                   MarkerHighlightHost& host);

}  // namespace bsp
