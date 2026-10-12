#include "bsp/marker_manager_cc13.hpp"

#include "bsp/world_entity_update.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

namespace bsp {
namespace {

// 00CF8F74, float 4242.0: upper bound of the random start phase.
constexpr float kMarkerPhaseRange = 4242.0f;
// 00D7A208, float -0.0: modes 1 and 2 build their angle as (-0.0f - phase).
constexpr float kNegativeZero = -0.0f;
// 00D7A24C, float 1.0: identity diagonal and the tinted alpha reset.
constexpr float kOne = 1.0f;
// 00CE3880, qword 25.0: mode 4 bob amplitude (FMUL qword at 006D817B).
constexpr double kMarkerBobAmplitude = 25.0;
// 00CF8FC8, qword 0.28 (kMarkerHighlightPulseScale): 006DC5F2 stores 0.28 * 0.0.
constexpr double kHighlightResetScale = static_cast<double>(0.28f);

// 00B64780 is reached through 0042B4D0, which takes the same (-0.0f - phase)
// angle in its own body and copies the result into ECX.
void rotation_z_0042b4d0(float out[16], float phase) noexcept {
    matrix_interpolator_rotation_z_00b64780(out, kNegativeZero - phase);
}

// Sets keyed by element pointer: the native containers are std::set-shaped
// trees (node+0Ch is the element) iterated in pointer order, so insertion keeps
// the vector sorted by std::less and refuses duplicates (insert-unique 006DA5B0).
void insert_unique(std::vector<MarkerEntry>& set, void* element) {
    const auto less = [](const MarkerEntry& entry, void* key) {
        return std::less<void*>{}(entry.target, key);
    };
    auto where = std::lower_bound(set.begin(), set.end(), element, less);
    if (where != set.end() && where->target == element) {
        return;
    }
    set.insert(where, MarkerEntry{element});
}

bool erase_if_present(std::vector<MarkerEntry>& set, void* element) {
    auto where = std::find_if(set.begin(), set.end(),
                              [element](const MarkerEntry& entry) { return entry.target == element; });
    if (where == set.end()) {
        return false;
    }
    set.erase(where);
    return true;
}

}  // namespace

void construct_marker_instance_006d8750(MarkerInstanceState& instance,
                                        const MarkerClassDescriptor& descriptor, void* owner,
                                        float x, float y, float z, MarkerInstanceHost& host) {
    // 006D875F..006D8787: owner, position, descriptor (vtable 00CF8F68 at +0).
    instance.owner_04 = owner;
    instance.position_08[0] = x;
    instance.position_08[1] = y;
    instance.position_08[2] = z;
    instance.descriptor_14 = &descriptor;
    // 006D878A..006D879A: resource vtable+8h(0, 1.0f).
    instance.scene_instance_18 = host.create_scene_instance(descriptor.resource_14, 0, 1.0f);
    // 006D879D..006D87AD.
    host.register_scene_instance(instance.scene_instance_18);
    // 006D87B1..006D87EC: synchronized classes start in phase; the others draw
    // from stream 1 (ECX=1 at 006D87DD), so the gameplay stream 0 is not touched.
    if (descriptor.synchronized_1c != 0) {
        instance.phase_1c = 0.0f;
    } else {
        instance.phase_1c = host.random_uniform_stream1(0.0f, kMarkerPhaseRange);
    }
    update_marker_instance_pose_006d7fc0(instance, host);
}

void destruct_marker_instance_006d74d0(MarkerInstanceState& instance, MarkerInstanceHost& host) {
    // 006D74D6..006D74EC: vslot 1(1) on the scene instance, then clear the cell.
    if (instance.scene_instance_18 != nullptr) {
        host.release_scene_instance(instance.scene_instance_18);
        instance.scene_instance_18 = nullptr;
    }
}

void tick_marker_instance_006d61f0(MarkerInstanceState& instance, float /*delta*/,
                                   MarkerInstanceHost& host) {
    // 006D61F0: mov eax,[ecx]; call [eax+8]; ret 4. The delta is dropped.
    update_marker_instance_pose_006d7fc0(instance, host);
}

void marker_instance_pose_matrix(std::int32_t animation_mode, const float position[3], float phase,
                                 float out[16]) noexcept {
    // 006D8018..006D8087: identity with the position in the translation row.
    std::memset(out, 0, sizeof(float) * 16);
    out[0] = out[5] = out[10] = out[15] = kOne;
    float bob_y = position[1];

    float rotation[16];
    if (animation_mode == 1) {
        // 006D80A5..006D80DC: SUBSS -0.0f - phase, 00B64640, two 004134F0 copies.
        matrix_interpolator_rotation_x_00b64640(rotation, kNegativeZero - phase);
        std::memcpy(out, rotation, sizeof(rotation));
    } else if (animation_mode == 2) {
        // 006D80EB..006D8122: the same with 00B646E0.
        matrix_interpolator_rotation_y_00b646e0(rotation, kNegativeZero - phase);
        std::memcpy(out, rotation, sizeof(rotation));
    } else if (animation_mode == 3) {
        // 006D812E..006D8147: 0042B4D0(phase), one copy.
        rotation_z_0042b4d0(rotation, phase);
        std::memcpy(out, rotation, sizeof(rotation));
    } else if (animation_mode == 4) {
        // 006D8153..006D8185: FSIN of the float phase, stored to float, sign
        // masked (AND 7FFFFFFFh), FMUL qword 25.0, FADD float y, FSTP float.
        const float s = static_cast<float>(std::sin(static_cast<double>(phase)));
        const float magnitude = std::fabs(s);
        bob_y = static_cast<float>(static_cast<double>(magnitude) * kMarkerBobAmplitude
                                   + static_cast<double>(bob_y));
    }
    // 006D8189..006D81AA: the translation row is written after the rotation copy.
    out[12] = position[0];
    out[13] = bob_y;
    out[14] = position[2];
}

void update_marker_instance_pose_006d7fc0(MarkerInstanceState& instance, MarkerInstanceHost& host) {
    // 006D7FCD..006D7FD0: classes without animation are never posed here.
    const std::int32_t mode = instance.descriptor_14->animation_18;
    if (mode == 0) {
        return;
    }
    // 006D7FD8..006D8000: an owner's world position, refreshed first when its
    // pose byte (+C8h) is clear; otherwise the stored position.
    float position[3];
    if (instance.owner_04 != nullptr) {
        if (!host.owner_world_pose_valid(instance.owner_04)) {
            host.refresh_owner_world_pose(instance.owner_04);
        }
        host.owner_world_position(instance.owner_04, position);
    } else {
        position[0] = instance.position_08[0];
        position[1] = instance.position_08[1];
        position[2] = instance.position_08[2];
    }
    // 006D808D..006D809F: FLD t; FADD ST0,ST0; FADD float phase; FSTP float.
    const double t = static_cast<double>(host.game_time_64c());
    const float phase = static_cast<float>(t + t + static_cast<double>(instance.phase_1c));

    float matrix[16];
    marker_instance_pose_matrix(mode, position, phase, matrix);
    // 006D818F..006D81BD: [scene_instance+0Ch] vtable+34h(&matrix).
    host.set_scene_instance_world_matrix(instance.scene_instance_18, matrix);
}

bool marker_vec3_less_006d6170(const float a[3], const float b[3]) noexcept {
    // 006D6170..006D61DC. Each axis: FCOMI (b > a -> true), then FUCOMIP for
    // equality (PF set on unordered -> false). z ends with FCOMIP b > a.
    if (b[0] > a[0]) {
        return true;
    }
    if (!(b[0] == a[0])) {
        return false;
    }
    if (b[1] > a[1]) {
        return true;
    }
    if (!(b[1] == a[1])) {
        return false;
    }
    return b[2] > a[2];
}

void add_gui_highlight_006dae20(MarkerManagerState& markers, float& pulse_epoch_00e19970,
                                float game_time_64c, void* element, bool recursive, bool tinted) {
    // 006DAE23..006DAE6F: recursive -> +38h; else tinted -> +44h; else +2Ch.
    if (recursive) {
        insert_unique(markers.recursive_gui_highlights, element);
    } else if (tinted) {
        insert_unique(markers.tinted_highlights, element);
    } else {
        insert_unique(markers.gui_highlights, element);
    }
    // 006DAE3C..006DAE49 / 006DAE74..006DAE82: every call restarts the pulse.
    // This is the only writer of 00E19970 found in the image.
    pulse_epoch_00e19970 = game_time_64c;
}

void remove_gui_highlight_006dc5a0(MarkerManagerState& markers, void* element,
                                   MarkerHighlightHost& host) {
    // +2Ch: the level is cleared before the erase.
    const auto in_plain = std::find_if(markers.gui_highlights.begin(), markers.gui_highlights.end(),
                                       [element](const MarkerEntry& e) { return e.target == element; });
    if (in_plain != markers.gui_highlights.end()) {
        host.set_highlight_level(element, static_cast<float>(kHighlightResetScale * 0.0));
        markers.gui_highlights.erase(in_plain);
    }
    // +38h: erase, then 006DBAC0(1, 0, 1.0f) on the element.
    if (erase_if_present(markers.recursive_gui_highlights, element)) {
        host.apply_highlight_pulse_006dbac0(element, 1, 0, 1.0f);
    }
    // +44h, 006DC69D..006DC6D5: erase, then color alpha (fourth float) = 1.0.
    if (erase_if_present(markers.tinted_highlights, element)) {
        MarkerColor color = host.get_color(element);
        color.rgba[3] = kOne;
        host.set_color(element, color);
    }
}

}  // namespace bsp
