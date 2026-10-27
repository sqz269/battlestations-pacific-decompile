#include "bsp/actions_seg5_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/unit_motion.hpp"

#include <cmath>
#include <cstring>
#include <string>
#include <string.h>

namespace bsp::cc13_seg5 {

namespace {

// The inlined CRT compare of an action's Enum data pointer (+30h): the
// image tests the pointer and, when it is non-null, calls _stricmp (00BF7FBF)
// and tests for zero.
bool enum_data_equals(Object action, const char* text) {
    const char* data = native_at<const char*>(action, 0x30);
    return data != nullptr && _stricmp(data, text) == 0;
}

Object movie_player(Seg5Host& host) {
    return native_at<Object>(host.game_00e188a8(), 0x38);
}

// The std::vector<Track*> at player+68h/+6Ch. The image re-tests the range
// before each element read and calls the checked-iterator report 00BF6713 on
// failure; that test repeats the loop condition, so it cannot fire.
std::uint32_t track_count(Object player) {
    Object first = native_at<Object>(player, 0x68);
    if (first == nullptr) return 0;
    return static_cast<std::uint32_t>(native_at<Object>(player, 0x6C) - first) >> 2;
}

Object track_at(Object player, std::uint32_t index) {
    return native_at<Object*>(player, 0x68)[index];
}

// The child walk shared by 00465180, 00465210 and 00465450: from entity+48h
// through each child's +44h.
template <class Visit>
void for_each_child(Object entity, Visit visit) {
    for (Object child = native_at<Object>(entity, 0x48); child != nullptr;
         child = native_at<Object>(child, 0x44)) {
        visit(child);
    }
}

// FLD action+20h; FILD child+38Ch; FUCOMIP: the group number (an int) equals
// Num1; otherwise UCOMISS Num1 against 0.0f (00D7A218) - Num1 == 0 selects
// every group. Both comparisons are false on a NaN.
bool gun_group_selected(Object action, Object child) {
    const float num1 = native_at<float>(action, 0x20);
    const std::int32_t group = native_at<std::int32_t>(child, 0x38C);
    return static_cast<double>(group) == static_cast<double>(num1) || num1 == 0.0f;
}

} // namespace

// 0045C690. Body 0045C690..0045C6F7.
Object movie_find_track_by_entity_0045c690(Object player, Object entity) {
    for (std::uint32_t i = 0; i < track_count(player); ++i) {
        if (native_at<Object>(track_at(player, i), 0x11C) == entity) return track_at(player, i);
    }
    return nullptr;
}

// 0045DCA0. Each iteration copies the track name with 0045D840 into a stack
// NativeString {size, data}; the compare is false when the copy's data is
// null; a non-null copy goes back to the sized pool (00419CC0, 00BD1510 with
// size + 1, 1) before the match is tested.
Object movie_find_camera_track_0045dca0(Seg5Host& host, Object player) {
    for (std::uint32_t i = 0; i < track_count(player); ++i) {
        alignas(4) std::uint8_t copy[8];
        movie_track_copy_name_0045d840(host, track_at(player, i), copy);
        const char* data = native_at<const char*>(copy, 4);
        const bool match = data != nullptr && _stricmp(data, "CameraPosition 01") == 0;
        void* storage = native_at<void*>(copy, 4);
        if (storage != nullptr) {
            const std::uint32_t bytes = native_at<std::uint32_t>(copy, 0) + 1;
            Object pool = host.sized_pool_singleton_00419cc0(storage, bytes, 1);
            host.sized_pool_return_block_00bd1510(pool, storage, bytes, 1);
        }
        if (match) return track_at(player, i);
    }
    return nullptr;
}

// 0045D840.
Object movie_track_copy_name_0045d840(Seg5Host& host, Object track, Object out) {
    Object name = track + 0x120;
    native_at<std::uint32_t>(out, 0) = 0;
    native_at<void*>(out, 4) = nullptr;
    if (out == name) return out;
    host.native_string_resize_0041dd40(out, native_at<std::uint32_t>(name, 0), 1);
    if (native_at<std::uint32_t>(name, 0) != 0) {
        host.crt_memcpy_00bf7680(native_at<void*>(out, 4), native_at<void*>(name, 4),
                                 native_at<std::uint32_t>(out, 0));
    }
    return out;
}

// 0045C560.
void movie_update_tracks_0045c560(Seg5Host& host, Object player) {
    for (std::uint32_t i = 0; i < track_count(player); ++i) {
        host.track_update_vcall_0c(track_at(player, i), native_at<float>(player, 0x04));
    }
}

// 004650F0 (sSetTarget). Nothing happens without a resolved ReferPath entity
// (+14h). The command is attackmove-like 00E08F78 when the running entity
// answers IsKindOf(6), else 00E08EF8. The 18h-byte target on the stack is
// 00465080 inlined with the non-null entity and 0.0f: +0 byte 1, +2 word
// entity+174h, +4 entity, +8..+10h the static vector, +14h 0.0f. Then
// 0077D600(ECX running entity, command, &target, 1). RET 4.
void run_set_target_004650f0(Seg5Host& host, Object action, Object entity) {
    Object target_entity = native_at<Object>(action, 0x14);
    if (target_entity == nullptr) return;
    Object command = host.is_kind_vcall_5c(entity, 6) ? host.command_00e08f78() : host.command_00e08ef8();
    std::uint8_t target[0x18]{};
    const float* vec = host.static_vector_00f87574();
    std::memcpy(target + 0x08, &vec[0], 4);
    std::memcpy(target + 0x0C, &vec[1], 4);
    std::memcpy(target + 0x10, &vec[2], 4);
    native_at<std::uint16_t>(target, 0x00) = 0;
    native_at<Object>(target, 0x04) = target_entity;
    native_at<std::uint8_t>(target, 0x00) = 1;
    native_at<std::uint16_t>(target, 0x02) = native_at<std::uint16_t>(target_entity, 0x174);
    native_at<float>(target, 0x14) = 0.0f;
    host.entity_issue_command_0077d600(entity, command, target, 1);
}

// 00464870 (cSetParent). Looks up the running entity's track (EBX, not
// null-tested) and the ReferPath entity's track; with the second present calls
// 00458D50(ECX first track, Num1, parent track, &{OffsetX, OffsetY, OffsetZ},
// player time +4). RET 4.
void run_set_parent_00464870(Seg5Host& host, Object action, Object entity) {
    Object player = movie_player(host);
    Object track = movie_find_track_by_entity_0045c690(player, entity);
    Object parent = movie_find_track_by_entity_0045c690(player, native_at<Object>(action, 0x14));
    if (parent == nullptr) return;
    const float offset[3] = {native_at<float>(action, 0x34), native_at<float>(action, 0x3C),
                             native_at<float>(action, 0x38)};
    entity_track_set_parent_00458d50(host, track, native_at<float>(action, 0x20), parent, offset,
                                     native_at<float>(player, 0x04));
}

// 00464350 (cUnParent). 00458A70(ECX the running entity's track, unchecked,
// Num1, player time +4). RET 4.
void run_unparent_00464350(Seg5Host& host, Object action, Object entity) {
    Object player = movie_player(host);
    Object track = movie_find_track_by_entity_0045c690(player, entity);
    entity_track_unparent_00458a70(host, track, native_at<float>(action, 0x20), native_at<float>(player, 0x04));
}

// 004648F0 (pForceRelease). ECX (the action) is not read. When the byte at
// entity + 9C0h + [00F876B8]*8 is set, 007BBBA0(ECX entity). RET 4.
void run_force_release_004648f0(Seg5Host& host, Object, Object entity) {
    const std::uint32_t slot = host.local_slot_00f876b8();
    if (native_at<std::uint8_t>(entity, 0x9C0 + slot * 8) != 0) {
        host.unit_request_ordnance_release_007bbba0(entity);
    }
}

// 00465180 (ForceDepthCharge). ECX (action) is kept for Num1. For each child
// answering IsKindOf(1Eh) and IsKindOf(24h) whose [child+3F4h]+80h is 8 and
// whose group matches Num1 (or Num1 is 0): child vtable+1F0h(). RET 4.
void run_force_depth_charge_00465180(Seg5Host& host, Object action, Object entity) {
    for_each_child(entity, [&](Object child) {
        if (!host.is_kind_vcall_5c(child, 0x1E)) return;
        if (!host.is_kind_vcall_5c(child, 0x24)) return;
        if (native_at<std::int32_t>(native_at<Object>(child, 0x3F4), 0x80) != 8) return;
        if (!gun_group_selected(action, child)) return;
        host.weapon_fire_vcall_1f0(child);
    });
}

// 00464910 (SetGunFire). Enum "OpenFire" (00CE53C0, through 00425850) gives
// 1, anything else -1 (FFh). An entity answering IsKindOf(18h) writes the byte
// to +9B0h of each of its [entity+3CCh] members: member i is the pointer at
// entity+3D0h+4i for i <= 4 and null beyond (the image then writes to
// address 9B0h). Otherwise an entity answering IsKindOf(0Fh) writes its own
// +9B0h. RET 4.
void run_set_gun_fire_00464910(Seg5Host& host, Object action, Object entity) {
    if (host.is_kind_vcall_5c(entity, 0x18)) {
        const std::uint8_t value = host.native_string_equals_ci_00425850(action + 0x2C, "OpenFire") ? 1 : 0xFF;
        for (std::int32_t i = 0; i < native_at<std::int32_t>(entity, 0x3CC); ++i) {
            Object member = static_cast<std::uint32_t>(i) <= 4 ? native_at<Object>(entity, 0x3D0 + i * 4) : nullptr;
            native_at<std::uint8_t>(member, 0x9B0) = value;
        }
        return;
    }
    if (!host.is_kind_vcall_5c(entity, 0x0F)) return;
    const bool open = host.native_string_equals_ci_00425850(action + 0x2C, "OpenFire");
    native_at<std::uint8_t>(entity, 0x9B0) = static_cast<std::uint8_t>(open ? 1 : -1);
}

// 004649B0 (TogglePeriscope). entity+1235h = (Enum is "Out", _stricmp); a
// null Enum stores 0. RET 4.
void run_toggle_periscope_004649b0(Object action, Object entity) {
    native_at<std::uint8_t>(entity, 0x1235) = enum_data_equals(action, "Out") ? 1 : 0;
}

// 00464390 (ForceArtilleryAngle): RET 4, no effect.
void run_force_artillery_angle_00464390(Object, Object) {}

// 00465210 (ForceTorpedo). ECX is not read. For each child answering
// IsKindOf(1Eh) and IsKindOf(20h): the 004646B0 loop inlined (0072CF00(i,
// 0.0f, 0) for every barrel), then child vtable+1D8h(0, 80.0f, 80.0f) (the
// float at 00CE5444 pushed twice). RET 4.
void run_force_torpedo_00465210(Seg5Host& host, Object, Object entity) {
    for_each_child(entity, [&](Object child) {
        if (!host.is_kind_vcall_5c(child, 0x1E)) return;
        if (!host.is_kind_vcall_5c(child, 0x20)) return;
        for (std::int32_t i = 0; i < native_at<std::int32_t>(child, 0x448); ++i) {
            host.gun_set_barrel_reload_timer_0072cf00(child, i, 0.0f, 0);
        }
        constexpr float kTorpedoArg = 80.0f;  // 00CE5444 (0x42A00000)
        host.weapon_fire_vcall_1d8(child, 0, kTorpedoArg, kTorpedoArg);
    });
}

// 00465410 (SetGunFireTargetOnly). With a non-null entity answering
// IsKindOf(5) and a non-null [entity+6DCh]: that object's +59h = Enum is
// "TargetOnly" (00CE546C, 00425850). RET 4.
void run_set_gun_fire_target_only_00465410(Seg5Host& host, Object action, Object entity) {
    if (entity == nullptr) return;
    if (!host.is_kind_vcall_5c(entity, 5)) return;
    Object director = native_at<Object>(entity, 0x6DC);
    if (director == nullptr) return;
    native_at<std::uint8_t>(director, 0x59) = host.native_string_equals_ci_00425850(action + 0x2C, "TargetOnly") ? 1 : 0;
}

// 00465450 (TurnGun). For each child answering IsKindOf(1Eh) and
// IsKindOf(24h) whose group matches Num1 (or Num1 is 0):
// 0085ABA0(ECX child, Num2, Num3). RET 4.
void run_turn_gun_00465450(Seg5Host& host, Object action, Object entity) {
    for_each_child(entity, [&](Object child) {
        if (!host.is_kind_vcall_5c(child, 0x1E)) return;
        if (!host.is_kind_vcall_5c(child, 0x24)) return;
        if (!gun_group_selected(action, child)) return;
        host.turning_gun_set_target_angles_0085aba0(child, native_at<float>(action, 0x24),
                                                    native_at<float>(action, 0x28));
    });
}

// 004643D0 (cFOV) and 004643E0 (cFOV_Now): RET 4, no effect.
void run_fov_004643d0(Object, Object) {}
void run_fov_now_004643e0(Object, Object) {}

// 004649F0 (DontMove). entity+62h = (Enum is "Stay", 00CE53D0). RET 4.
void run_dont_move_004649f0(Object action, Object entity) {
    native_at<std::uint8_t>(entity, 0x62) = enum_data_equals(action, "Stay") ? 1 : 0;
}

// 00464AB0 (CameraShake), __thiscall but the entity argument is not read.
// The offsets are latched first. `camera` = Enum is "CameraPos" (_stricmp)
// or "CameraAndLookAt" (00425850); `look_at` = "LookAtPos" or
// "CameraAndLookAt". With a movie player and a camera track (0045DCA0):
// 0045AF70(ECX track, ActivateTime, OffsetX, OffsetY, OffsetZ, max(Num1, 0),
// 1) for the camera and the same with 0 for the look-at point. The clamp is
// COMISS Num1 > 0 (a NaN gives 0). RET 4.
void run_camera_shake_00464ab0(Seg5Host& host, Object action, Object) {
    const float x = native_at<float>(action, 0x34);
    const float y = native_at<float>(action, 0x3C);
    const float z = native_at<float>(action, 0x38);
    const bool camera = enum_data_equals(action, "CameraPos") ||
                        host.native_string_equals_ci_00425850(action + 0x2C, "CameraAndLookAt");
    const bool look_at = enum_data_equals(action, "LookAtPos") ||
                         host.native_string_equals_ci_00425850(action + 0x2C, "CameraAndLookAt");
    Object player = movie_player(host);
    if (player == nullptr) return;
    Object track = movie_find_camera_track_0045dca0(host, player);
    if (track == nullptr) return;
    if (camera) {
        const float num1 = native_at<float>(action, 0x20);
        const float amplitude = num1 > 0.0f ? num1 : 0.0f;
        camera_track_add_shake_0045af70(host, track, native_at<float>(action, 0x04), x, y, z, amplitude, 1);
    }
    if (look_at) {
        const float num1 = native_at<float>(action, 0x20);
        const float amplitude = num1 > 0.0f ? num1 : 0.0f;
        camera_track_add_shake_0045af70(host, track, native_at<float>(action, 0x04), x, y, z, amplitude, 0);
    }
}

// 00464C40 (Binocular). The entity argument is not read. With
// views = [[00E198C4]+84h] (re-read before each call) and views+20h and
// views+70h both non-null, the Enum (00425850) selects:
//   "Binocular": +6Ch(1), +20h(1), +70h(0)
//   "Periscope": +6Ch(1), +70h(1), +20h(0)
//   "OFF":       +20h(0), +70h(0)
// each a vtable+34h call on the view object. The last call of each case is a
// tail jump. RET 4.
void run_binocular_00464c40(Seg5Host& host, Object action, Object) {
    auto views = [&host]() { return native_at<Object>(host.hud_00e198c4(), 0x84); };
    if (native_at<Object>(views(), 0x20) == nullptr) return;
    if (native_at<Object>(views(), 0x70) == nullptr) return;
    if (host.native_string_equals_ci_00425850(action + 0x2C, "Binocular")) {
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x6C), 1);
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x20), 1);
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x70), 0);
        return;
    }
    if (host.native_string_equals_ci_00425850(action + 0x2C, "Periscope")) {
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x6C), 1);
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x70), 1);
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x20), 0);
        return;
    }
    if (host.native_string_equals_ci_00425850(action + 0x2C, "OFF")) {
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x20), 0);
        host.view_set_active_vcall_34(native_at<Object>(views(), 0x70), 0);
    }
}

// 00464D70 (TakeOffHack). entity+64h = Num1 (FLD/FSTP). RET 4.
void run_take_off_hack_00464d70(Object action, Object entity) {
    native_at<float>(entity, 0x64) = native_at<float>(action, 0x20);
}

// 00464D80 (WingTipEffect). entity+96Ah = (Enum is "On", 00CE5430). RET 4.
void run_wing_tip_effect_00464d80(Object action, Object entity) {
    native_at<std::uint8_t>(entity, 0x96A) = enum_data_equals(action, "On") ? 1 : 0;
}

// 00464DC0 (MoveToPoint). entity+748h = (Enum is "Enable", 00CE5434). RET 4.
void run_move_to_point_00464dc0(Object action, Object entity) {
    native_at<std::uint8_t>(entity, 0x748) = enum_data_equals(action, "Enable") ? 1 : 0;
}

// 00464E00 (WideBlackBars). Enum "Enable" (_stricmp) -> 005CB0F0, otherwise
// 005CB130, both __thiscall(ECX [[00E198C4]+A0h]). The entity is not read.
// RET 4.
void run_wide_black_bars_00464e00(Seg5Host& host, Object action, Object) {
    if (enum_data_equals(action, "Enable")) {
        host.hud_movie_screen_show_black_bars_005cb0f0(native_at<Object>(host.hud_00e198c4(), 0xA0));
        return;
    }
    host.hud_movie_screen_hide_black_bars_005cb130(native_at<Object>(host.hud_00e198c4(), 0xA0));
}

// 00464E50 (UseCatapult). value = float(Num1 / 180.0 * 3.1415927410125732)
// (FDIV/FMUL by the doubles at 00CE3D20 and 00CE3D28, stored as float). A
// stack message: 0075B430(7Bh), then +20h = value, +18h word 0, +1Ah byte 0,
// +4 = 1, +1Ch byte 0, vtable 00CE53AC (00464710 inlined, same fields), and
// 0077C2A0(ECX entity, &message, 2, 0). The message has no destructor call
// (its base destructor only resets the vtable). RET 4.
void run_use_catapult_00464e50(Seg5Host& host, Object action, Object entity) {
    const float value = static_cast<float>(static_cast<double>(native_at<float>(action, 0x20)) / 180.0 *
                                           3.1415927410125732);
    alignas(4) std::uint8_t message[0x24]{};
    host.session_message_construct_base_0075b430(message, 0x7B);
    native_at<float>(message, 0x20) = value;
    native_at<std::uint16_t>(message, 0x18) = 0;
    native_at<std::uint8_t>(message, 0x1A) = 0;
    native_at<std::uint32_t>(message, 0x04) = 1;
    native_at<std::uint8_t>(message, 0x1C) = 0;
    native_at<std::uint32_t>(message, 0x00) = 0x00CE53ACu;
    host.session_route_message_0077c2a0(entity, message, 2, 0);
}

// 00464EE0 (HideEntity). With IsKindOf(5): entity+68h = Enum is "Hide"
// (00CE543C, 00425850). RET 4.
void run_hide_entity_00464ee0(Seg5Host& host, Object action, Object entity) {
    if (!host.is_kind_vcall_5c(entity, 5)) return;
    native_at<std::uint8_t>(entity, 0x68) = host.native_string_equals_ci_00425850(action + 0x2C, "Hide") ? 1 : 0;
}

// 004646B0.
void gun_reset_all_barrel_reload_timers_004646b0(Seg5Host& host, Object gun) {
    for (std::int32_t i = 0; i < native_at<std::int32_t>(gun, 0x448); ++i) {
        host.gun_set_barrel_reload_timer_0072cf00(gun, i, 0.0f, 0);
    }
}

namespace {
Object cast_to_kind(Seg5Host& host, Object object, std::int32_t kind) {
    if (object != nullptr && host.is_kind_vcall_5c(object, kind)) return object;
    return nullptr;
}

void fill_command_target(Seg5Host& host, Object target, Object entity, float value) {
    const float* vec = host.static_vector_00f87574();
    std::memcpy(target + 0x08, &vec[0], 4);
    std::memcpy(target + 0x0C, &vec[1], 4);
    std::memcpy(target + 0x10, &vec[2], 4);
    native_at<std::uint16_t>(target, 0x00) = 0;
    native_at<Object>(target, 0x04) = entity;
    if (entity == nullptr) {
        native_at<std::uint8_t>(target, 0x00) = 0;
        native_at<std::uint16_t>(target, 0x02) = 0;
    } else {
        native_at<std::uint8_t>(target, 0x00) = 1;
        native_at<std::uint16_t>(target, 0x02) = native_at<std::uint16_t>(entity, 0x174);
    }
    native_at<float>(target, 0x14) = value;
}
} // namespace

// 00465000 / 00465020 / 00465040 / 00465060.
Object unit_cast_to_kind24_00465000(Seg5Host& host, Object object) { return cast_to_kind(host, object, 0x24); }
Object unit_cast_to_kind20_00465020(Seg5Host& host, Object object) { return cast_to_kind(host, object, 0x20); }
Object unit_cast_to_vehicle_00465040(Seg5Host& host, Object object) { return cast_to_kind(host, object, 5); }
Object unit_cast_to_submarine_00465060(Seg5Host& host, Object object) { return cast_to_kind(host, object, 8); }

// 00465080: returns ECX (MOV EAX,ECX at 00465088).
Object command_target_from_entity_00465080(Seg5Host& host, Object target, Object entity, float value) {
    fill_command_target(host, target, entity, value);
    return target;
}

// 00464F70: the same stores; EAX is left holding the entity argument (a void
// body), which is what this returns.
Object command_target_from_entity_00464f70(Seg5Host& host, Object target, Object entity, float value) {
    fill_command_target(host, target, entity, value);
    return entity;
}

// 00464500 / 00464590 / 004647B0.
bool session_message46_is_kind_of_00464500(std::int32_t type) { return type == 0x46; }
bool session_message49_is_kind_of_00464590(std::int32_t type) { return type == 0x49 || type == 0x46; }
bool session_message7b_is_kind_of_004647b0(std::int32_t type) {
    return type == 0x7B || type == 0x49 || type == 0x46;
}

// 004645B0: 0075B480(ECX message, stream), then 004290B0(ECX stream, the
// zero-extended byte +1Ch).
void session_bool_message_write_004645b0(Seg5Host& host, Object message, Object stream) {
    host.session_message_write_extended_header_0075b480(message, stream);
    host.bit_cursor_write_boolean_004290b0(stream, native_at<std::uint8_t>(message, 0x1C));
}

// 004645D0: 0075B4C0(ECX message, stream), then 00428D70(ECX stream+4,
// &message+1Ch).
void session_bool_message_read_004645d0(Seg5Host& host, Object message, Object stream) {
    host.session_message_read_extended_header_0075b4c0(message, stream);
    host.bit_cursor_read_boolean_00428d70(stream + 4, message + 0x1C);
}

// 00464710.
Object session_message7b_construct_00464710(Seg5Host& host, Object message, float value) {
    host.session_message_construct_base_0075b430(message, 0x7B);
    native_at<std::uint16_t>(message, 0x18) = 0;
    native_at<std::uint8_t>(message, 0x1A) = 0;
    native_at<std::uint8_t>(message, 0x1C) = 0;
    native_at<std::uint32_t>(message, 0x04) = 1;
    native_at<std::uint32_t>(message, 0x00) = 0x00CE53ACu;
    native_at<float>(message, 0x20) = value;
    return message;
}

// ---------------------------------------------------------------------------
// Entity and camera tracks.
// ---------------------------------------------------------------------------

namespace {
Object list_head(Object list) { return native_at<Object>(list, 0x04); }
Object node_next(Object node) { return native_at<Object>(node, 0x00); }
Object node_prev(Object node) { return native_at<Object>(node, 0x04); }

// The checked-iterator tests between the steps below (container non-null and
// equal to the list, node not the head) repeat the loop conditions and call
// 00BF6713 only when they fail; they cannot fail on these paths.
} // namespace

// 004561E0.
void entity_track_stop_actions_004561e0(Seg5Host& host, Object track) {
    Object list = track + 0x108;
    native_at<Object>(track, 0x114) = list;
    native_at<Object>(track, 0x118) = node_next(list_head(list));
    while (native_at<Object>(track, 0x118) != list_head(list)) {
        host.action_stop_vcall_04(native_at<Object>(native_at<Object>(track, 0x118), 0x08));
        native_at<Object>(track, 0x118) = node_next(native_at<Object>(track, 0x118));
    }
}

// 00456290. The first node is read before the cursor container is stored.
void entity_track_rewind_00456290(Seg5Host& host, Object track) {
    Object first = node_next(list_head(track + 0x108));
    native_at<Object>(track, 0x140) = track + 0x108;
    native_at<Object>(track, 0x144) = first;
    host.track_reset_vcall_04(track);
    native_at<std::int32_t>(track, 0x14C) = 0;
}

// 004562D0. Each sum is FLD/FADD/FSTP float. The count is FILD of the
// signed size plus 2^32 (00CE3978) when negative, stored as float; each
// component is then sum / count on the x87 stack and stored as float (the
// same as a float division: the image runs at 53-bit precision).
void entity_track_mean_offset_004562d0(Object track, float* out) {
    Object list = track + 0xF4;
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_z = 0.0f;
    native_at<Object>(track, 0x100) = list;
    native_at<Object>(track, 0x104) = node_next(list_head(list));
    while (native_at<Object>(track, 0x104) != list_head(list)) {
        Object node = native_at<Object>(track, 0x104);
        sum_x = native_at<float>(node, 0x14) + sum_x;
        sum_y = native_at<float>(node, 0x18) + sum_y;
        sum_z = native_at<float>(node, 0x1C) + sum_z;
        native_at<Object>(track, 0x104) = node_next(node);
    }
    const float count = static_cast<float>(native_at<std::uint32_t>(track, 0xFC));
    out[0] = sum_x / count;
    out[1] = sum_y / count;
    out[2] = sum_z / count;
}

// 004567D0. On a match the scratch iterator is stepped forward (keeping the
// next node) and back, the node is unlinked and freed (00BF65AC) unless it is
// the head, the size is decremented, and the cursor becomes the saved
// (container, next node).
void entity_track_remove_action_004567d0(Seg5Host& host, Object track, Object action) {
    Object list = track + 0x108;
    native_at<Object>(track, 0x114) = list;
    native_at<Object>(track, 0x118) = node_next(list_head(list));
    while (native_at<Object>(track, 0x118) != list_head(list)) {
        if (native_at<Object>(native_at<Object>(track, 0x118), 0x08) == action) {
            Object next = node_next(native_at<Object>(track, 0x118));
            native_at<Object>(track, 0x118) = next;
            Object container = native_at<Object>(track, 0x114);
            native_at<Object>(track, 0x118) = node_prev(native_at<Object>(track, 0x118));
            Object node = native_at<Object>(track, 0x118);
            if (node != list_head(list)) {
                native_at<Object>(node_prev(node), 0x00) = node_next(node);
                native_at<Object>(node_next(node), 0x04) = node_prev(node);
                host.crt_free_00bf65ac(node);
                native_at<std::int32_t>(track, 0x110) -= 1;
            }
            native_at<Object>(track, 0x140) = container;
            native_at<Object>(track, 0x144) = next;
            return;
        }
        native_at<Object>(track, 0x118) = node_next(native_at<Object>(track, 0x118));
    }
}

namespace {
// The clear shared by 004569A0 and 004555F0: unhook the head, zero the
// size, then free every former node.
void list_clear_nodes(Seg5Host& host, Object list) {
    Object node = node_next(list_head(list));
    native_at<Object>(list_head(list), 0x00) = list_head(list);
    native_at<Object>(list_head(list), 0x04) = list_head(list);
    native_at<std::uint32_t>(list, 0x08) = 0;
    while (node != list_head(list)) {
        Object next = node_next(node);
        host.crt_free_00bf65ac(node);
        node = next;
    }
}
} // namespace

// 004555F0.
void action_list_tidy_004555f0(Seg5Host& host, Object list) {
    list_clear_nodes(host, list);
    host.crt_free_00bf65ac(list_head(list));
    native_at<Object>(list, 0x04) = nullptr;
}

// 004569A0.
void action_list_destroy_004569a0(Seg5Host& host, Object list) {
    list_clear_nodes(host, list);
    action_list_tidy_004555f0(host, list);
}

// 00459F40. The node's next is read before its string is released.
void named_list_clear_00459f40(Seg5Host& host, Object list) {
    Object node = node_next(list_head(list));
    native_at<Object>(list_head(list), 0x00) = list_head(list);
    native_at<Object>(list_head(list), 0x04) = list_head(list);
    native_at<std::uint32_t>(list, 0x08) = 0;
    while (node != list_head(list)) {
        void* data = native_at<void*>(node, 0x10);
        Object next = node_next(node);
        if (data != nullptr) {
            const std::uint32_t bytes = native_at<std::uint32_t>(node, 0x0C) + 1;
            Object pool = host.sized_pool_singleton_00419cc0(data, bytes, 1);
            host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
        }
        host.crt_free_00bf65ac(node);
        node = next;
    }
}

// 00458050. +0 and +0Ch..+30h are FLD/FSTP float copies (a signalling NaN
// would come out quiet); +34h is a dword move.
Object record38_assign_00458050(Seg5Host& host, Object dest, Object source) {
    native_at<float>(dest, 0x00) = native_at<float>(source, 0x00);
    Object dest_name = dest + 0x04;
    Object source_name = source + 0x04;
    if (dest_name != source_name) {
        host.native_string_resize_0041dd40(dest_name, native_at<std::uint32_t>(source_name, 0), 1);
        if (native_at<std::uint32_t>(source_name, 0) != 0) {
            host.crt_memcpy_00bf7680(native_at<void*>(dest_name, 4), native_at<void*>(source_name, 4),
                                     native_at<std::uint32_t>(dest_name, 0));
        }
    }
    for (std::uint32_t offset = 0x0C; offset <= 0x30; offset += 4) {
        native_at<float>(dest, offset) = native_at<float>(source, offset);
    }
    native_at<std::uint32_t>(dest, 0x34) = native_at<std::uint32_t>(source, 0x34);
    return dest;
}

// 00456710. The EH frame only brackets the call.
void list_value_construct_00456710(Seg5Host& host, Object place, Object source) {
    (void)host;
    if (place != nullptr) shake_channel_copy_construct_00455880(place, source);
}

namespace {
constexpr double kBlendLead = 0.20000000298023224;  // 00CE3D10 (double)
constexpr double kBlendRate = 200.0;                // 00CE4D70 (double)

// FSUB float, FSTP float: one component difference (53-bit precision, then
// float; equal to a float subtraction).
float sub_f(float a, float b) { return static_cast<float>(static_cast<double>(a) - static_cast<double>(b)); }
float add_f(float a, float b) { return static_cast<float>(static_cast<double>(a) + static_cast<double>(b)); }

// The velocity tail shared by both blends: d is stored as three floats, then
// FLD d.x; FLD double 200; FMUL ST(1),ST(0) (DC form, bytes checked); FXCH;
// FSTP -> 200*d.x; FLD d.y; FMUL ST(0),ST(1) -> 200*d.y; FMUL d.z -> 200*d.z.
// The three products go to `first` and again to `second`.
void store_velocity(Object track, std::uint32_t first, std::uint32_t second, const float d[3]) {
    const float vx = static_cast<float>(static_cast<double>(d[0]) * kBlendRate);
    const float vy = static_cast<float>(static_cast<double>(d[1]) * kBlendRate);
    const float vz = static_cast<float>(kBlendRate * static_cast<double>(d[2]));
    native_at<float>(track, first + 0) = vx;
    native_at<float>(track, first + 4) = vy;
    native_at<float>(track, first + 8) = vz;
    native_at<float>(track, second + 8) = vz;
    native_at<float>(track, second + 0) = vx;
    native_at<float>(track, second + 4) = vy;
}
} // namespace

// 00458A70. The first two path reads use ECX = the parent track, the last two
// the track itself; all pass flag 1 and the same stack buffer.
void entity_track_unparent_00458a70(Seg5Host& host, Object track, float duration, float movie_time) {
    Object parent = native_at<Object>(track, 0x154);
    if (parent == nullptr) return;
    host.entity_vcall_ac(native_at<Object>(track, 0x11C), 0);
    float buffer[3];
    native_at<std::int32_t>(track, 0x14C) = 0;
    const float* p = entity_track_eval_position_004581e0(host, parent, buffer, movie_time, 1);
    const float start[3] = {add_f(p[0], native_at<float>(track, 0x1C8)),
                            add_f(p[1], native_at<float>(track, 0x1CC)),
                            add_f(p[2], native_at<float>(track, 0x1D0))};
    native_at<float>(track, 0x164) = start[0];
    native_at<float>(track, 0x168) = start[1];
    native_at<float>(track, 0x16C) = start[2];
    native_at<float>(track, 0x158) = movie_time;
    const float t0 = static_cast<float>(static_cast<double>(movie_time) - kBlendLead);
    const float* q = entity_track_eval_position_004581e0(host, parent, buffer, t0, 1);
    const float q_copy[3] = {q[0], q[1], q[2]};
    const float d0[3] = {sub_f(q_copy[0], native_at<float>(track, 0x164)),
                         sub_f(q_copy[1], native_at<float>(track, 0x168)),
                         sub_f(q_copy[2], native_at<float>(track, 0x16C))};
    store_velocity(track, 0x174, 0x180, d0);

    const double t2_wide = static_cast<double>(duration) + static_cast<double>(movie_time);  // FST double
    const float t2 = static_cast<float>(t2_wide);
    entity_track_eval_position_004581e0(host, track, buffer, t2, 1);
    // The end point is read from the stack buffer (MOVSS [ESP+1Ch..24h]), not
    // through the returned pointer.
    native_at<float>(track, 0x19C) = buffer[0];
    native_at<float>(track, 0x1A0) = buffer[1];
    native_at<float>(track, 0x1A4) = buffer[2];
    native_at<float>(track, 0x190) = t2;
    const float t3 = static_cast<float>(t2_wide - kBlendLead);
    const float* s = entity_track_eval_position_004581e0(host, track, buffer, t3, 1);
    const float s_copy[3] = {s[0], s[1], s[2]};
    const float end[3] = {native_at<float>(track, 0x19C), native_at<float>(track, 0x1A0),
                          native_at<float>(track, 0x1A4)};
    const float d1[3] = {sub_f(s_copy[0], end[0]), sub_f(s_copy[1], end[1]), sub_f(s_copy[2], end[2])};
    store_velocity(track, 0x1AC, 0x1B8, d1);
    native_at<float>(track, 0x150) = duration;
    native_at<float>(track, 0x1C8) = 0.0f;
    native_at<float>(track, 0x1CC) = 0.0f;
    native_at<float>(track, 0x1D0) = 0.0f;
    native_at<std::int32_t>(track, 0x14C) = 3;
}

// 00458D50. The first and second path reads use ECX = the track, then the
// parent (EBX) twice. The end points add the offset (offset + path for x on
// the second read; the sum is the same either way round).
void entity_track_set_parent_00458d50(Seg5Host& host, Object track, float duration, Object parent,
                                      const float* offset, float movie_time) {
    Object entity = native_at<Object>(track, 0x11C);
    if (native_at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float t0 = static_cast<float>(static_cast<double>(movie_time) - kBlendLead);
    native_at<float>(track, 0x164) = native_at<float>(entity, 0xFC);
    native_at<float>(track, 0x168) = native_at<float>(entity, 0x100);
    native_at<float>(track, 0x16C) = native_at<float>(entity, 0x104);
    native_at<float>(track, 0x158) = movie_time;
    float buffer[3];
    const float* q = entity_track_eval_position_004581e0(host, track, buffer, t0, 1);
    const float q_copy[3] = {q[0], q[1], q[2]};
    const float d0[3] = {sub_f(q_copy[0], native_at<float>(track, 0x164)),
                         sub_f(q_copy[1], native_at<float>(track, 0x168)),
                         sub_f(q_copy[2], native_at<float>(track, 0x16C))};
    store_velocity(track, 0x174, 0x180, d0);

    const double t2_wide = static_cast<double>(duration) + static_cast<double>(movie_time);  // FST double
    const float t2 = static_cast<float>(t2_wide);
    const float* r = entity_track_eval_position_004581e0(host, parent, buffer, t2, 1);
    const float end[3] = {add_f(r[0], offset[0]), add_f(r[1], offset[1]), add_f(r[2], offset[2])};
    native_at<float>(track, 0x19C) = end[0];
    native_at<float>(track, 0x1A0) = end[1];
    native_at<float>(track, 0x1A4) = end[2];
    native_at<float>(track, 0x190) = t2;
    const float t3 = static_cast<float>(t2_wide - kBlendLead);
    const float* s = entity_track_eval_position_004581e0(host, parent, buffer, t3, 1);
    const float s_sum[3] = {add_f(offset[0], s[0]), add_f(s[1], offset[1]), add_f(s[2], offset[2])};
    const float end_again[3] = {native_at<float>(track, 0x19C), native_at<float>(track, 0x1A0),
                                native_at<float>(track, 0x1A4)};
    const float d1[3] = {sub_f(s_sum[0], end_again[0]), sub_f(s_sum[1], end_again[1]),
                         sub_f(s_sum[2], end_again[2])};
    store_velocity(track, 0x1AC, 0x1B8, d1);
    native_at<Object>(track, 0x154) = parent;
    native_at<float>(track, 0x150) = duration;
    native_at<float>(track, 0x1C8) = offset[0];
    native_at<float>(track, 0x1CC) = offset[1];
    native_at<float>(track, 0x1D0) = offset[2];
    native_at<std::int32_t>(track, 0x14C) = 1;
}

namespace {
void release_native_string(Seg5Host& host, Object string) {
    void* data = native_at<void*>(string, 4);
    if (data == nullptr) return;
    const std::uint32_t bytes = native_at<std::uint32_t>(string, 0) + 1;
    Object pool = host.sized_pool_singleton_00419cc0(data, bytes, 1);
    host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
}

// The inlined NativeString(const char*) of 0045DB40 for the "" at 00CE3A0C:
// {0, 0}, 0041DD40(0, 1), then memcpy of size + 1 bytes when data is set.
void make_empty_native_string(Seg5Host& host, Object string) {
    static const char kEmpty[1] = {0};  // 00CE3A0C
    native_at<std::uint32_t>(string, 0) = 0;
    native_at<void*>(string, 4) = nullptr;
    host.native_string_resize_0041dd40(string, 0, 1);
    if (native_at<void*>(string, 4) != nullptr) {
        host.crt_memcpy_00bf7680(native_at<void*>(string, 4), kEmpty, native_at<std::uint32_t>(string, 0) + 1);
    }
}
} // namespace

// 0045B100. The EH states only order the member destructors.
Object entity_track_construct_0045b100(Seg5Host& host, Object track) {
    native_at<std::uint32_t>(track, 0x00) = 0x00CE4DB8u;
    native_at<Object>(track, 0xF8) = host.list_buy_head_node_00455520(track + 0xF4);
    native_at<std::uint32_t>(track, 0xFC) = 0;
    native_at<Object>(track, 0x100) = nullptr;
    native_at<Object>(track, 0x104) = nullptr;
    native_at<Object>(track, 0x10C) = host.list_buy_head_node_00455540(track + 0x108);
    native_at<std::uint32_t>(track, 0x110) = 0;
    native_at<Object>(track, 0x114) = nullptr;
    native_at<Object>(track, 0x118) = nullptr;
    native_at<std::uint32_t>(track, 0x120) = 0;
    native_at<void*>(track, 0x124) = nullptr;
    native_at<Object>(track, 0x140) = nullptr;
    native_at<Object>(track, 0x144) = nullptr;
    host.timed_value_construct_004658d0(track + 0x158, 0.0f);
    host.timed_value_construct_004658d0(track + 0x190, 0.0f);
    native_at<Object>(track, 0x144) = native_at<Object>(track, 0x10C);
    native_at<Object>(track, 0x140) = track + 0x108;
    native_at<std::int32_t>(track, 0x14C) = 0;
    native_at<float>(track, 0x148) = 0.0f;
    for (std::uint32_t offset = 0x04; offset < 0xF4; offset += 4) native_at<float>(track, offset) = 0.0f;
    return track;
}

// 0045B220. EH states order the member destructors; the explicit body is the
// first 00459F40 call and the action walk.
void entity_track_destroy_0045b220(Seg5Host& host, Object track) {
    native_at<std::uint32_t>(track, 0x00) = 0x00CE4DB8u;
    named_list_clear_00459f40(host, track + 0xF4);
    Object list = track + 0x108;
    native_at<Object>(track, 0x114) = list;
    native_at<Object>(track, 0x118) = node_next(list_head(list));
    while (native_at<Object>(track, 0x118) != list_head(list)) {
        Object action = native_at<Object>(native_at<Object>(track, 0x118), 0x08);
        if (action != nullptr) {
            host.action_destroy_00465610(action);
            host.crt_free_00bf65ac(action);
        }
        native_at<Object>(track, 0x118) = node_next(native_at<Object>(track, 0x118));
    }
    list_clear_nodes(host, list);
    release_native_string(host, track + 0x194);
    release_native_string(host, track + 0x15C);
    release_native_string(host, track + 0x120);
    action_list_tidy_004555f0(host, list);  // the same inlined clear + free head + head = 0
    named_list_clear_00459f40(host, track + 0xF4);
    host.crt_free_00bf65ac(native_at<Object>(track, 0xF8));
    native_at<Object>(track, 0xF8) = nullptr;
}

// 0045BAD0. The value is latched (MOVSS to a stack slot) before the
// capacity test; the slow path passes that slot's address.
void entity_track_collect_sample_times_0045bad0(Seg5Host& host, Object track, Object out) {
    Object list = track + 0xF4;
    native_at<Object>(track, 0x100) = list;
    native_at<Object>(track, 0x104) = node_next(list_head(list));
    while (native_at<Object>(track, 0x104) != list_head(list)) {
        float value = native_at<float>(native_at<Object>(track, 0x104), 0x08);
        Object first = native_at<Object>(out, 0x04);
        const std::uint32_t size =
            first == nullptr ? 0 : static_cast<std::uint32_t>(native_at<Object>(out, 0x08) - first) >> 2;
        if (first != nullptr &&
            size < (static_cast<std::uint32_t>(native_at<Object>(out, 0x0C) - first) >> 2)) {
            float* last = native_at<float*>(out, 0x08);
            *last = value;
            native_at<float*>(out, 0x08) = last + 1;
        } else {
            alignas(4) std::uint8_t result[8];
            host.float_vector_insert_one_0045a120(out, result, out, native_at<float*>(out, 0x08), &value);
        }
        native_at<Object>(track, 0x104) = node_next(native_at<Object>(track, 0x104));
    }
}

// 0045BD00.
Object camera_track_construct_0045bd00(Seg5Host& host, Object track) {
    entity_track_construct_0045b100(host, track);
    native_at<std::uint32_t>(track, 0x00) = 0x00CE4DCCu;
    native_at<Object>(track, 0x1E0) = host.list_buy_head_node_004555d0(track + 0x1DC);
    native_at<std::uint32_t>(track, 0x1E4) = 0;
    native_at<Object>(track, 0x1EC) = host.list_buy_head_node_004555d0(track + 0x1E8);
    native_at<std::uint32_t>(track, 0x1F0) = 0;
    native_at<std::uint32_t>(track, 0x1D4) = 0;
    return track;
}

// 0045D8A0. The image's self-assignment test compares track+120h with the
// address of the by-value argument; they never coincide.
void entity_track_set_name_0045d8a0(Seg5Host& host, Object track, std::uint32_t size, void* data) {
    Object name = track + 0x120;
    host.native_string_resize_0041dd40(name, size, 1);
    if (size != 0) host.crt_memcpy_00bf7680(native_at<void*>(name, 4), data, native_at<std::uint32_t>(name, 0));
    if (data != nullptr) {
        Object pool = host.sized_pool_singleton_00419cc0(data, size + 1, 1);
        host.sized_pool_return_block_00bd1510(pool, data, size + 1, 1);
    }
}

// 0045EFB0.
Object engine_movie_create_track_0045efb0(Seg5Host& host, std::uint32_t size, void* data) {
    static const char kCameraTrackName[] = "CameraPosition 01";  // 00CE4DA4
    alignas(4) std::uint8_t literal[8];
    native_at<std::uint32_t>(literal, 0) = 0;
    native_at<void*>(literal, 4) = nullptr;
    host.native_string_resize_0041dd40(literal, 0x11, 1);
    void* literal_data = native_at<void*>(literal, 4);
    const std::uint32_t literal_size = native_at<std::uint32_t>(literal, 0);
    if (literal_data != nullptr) host.crt_memcpy_00bf7680(literal_data, kCameraTrackName, literal_size + 1);
    alignas(4) std::uint8_t name[8];
    native_at<std::uint32_t>(name, 0) = size;
    native_at<void*>(name, 4) = data;
    const bool camera = host.native_string_equals_insensitive_00435c40(literal, name);
    if (literal_data != nullptr) {
        Object pool = host.sized_pool_singleton_00419cc0(literal_data, literal_size + 1, 1);
        host.sized_pool_return_block_00bd1510(pool, literal_data, literal_size + 1, 1);
    }
    Object track = nullptr;
    if (camera) {
        Object memory = host.operator_new_00bf681b(0x1F4);
        track = memory != nullptr ? camera_track_construct_0045bd00(host, memory) : nullptr;
    } else {
        Object memory = host.operator_new_00bf681b(0x1D4);
        track = memory != nullptr ? entity_track_construct_0045b100(host, memory) : nullptr;
    }
    if (data != nullptr) {
        Object pool = host.sized_pool_singleton_00419cc0(data, size + 1, 1);
        host.sized_pool_return_block_00bd1510(pool, data, size + 1, 1);
    }
    return track;
}

// 0045DB40.
void engine_movie_start_0045db40(Seg5Host& host, Object player) {
    native_at<std::int32_t>(player, 0x60) = 1;
    host.game_set_cinematic_mode_004cd0f0(host.game_00e188a8(), 0, 0, 1);
    alignas(4) std::uint8_t first[8];
    make_empty_native_string(host, first);
    constexpr float kFadeLevel = 0.99f;      // 00CE4E0C (0x3F7D70A4)
    constexpr float kFadeDuration = -0.01f;  // 00D7A300 (0xBC23D70A)
    host.narrative_arm_blackout_fade_005b9ba0(native_at<Object>(host.hud_00e198c4(), 0xA4), kFadeLevel,
                                              kFadeDuration, first);
    release_native_string(host, first);
    alignas(4) std::uint8_t second[8];
    make_empty_native_string(host, second);
    host.narrative_begin_sound_fade_005b9760(native_at<Object>(host.hud_00e198c4(), 0xA4), 1.0f, 1.0f, second);
    release_native_string(host, second);
    host.hud_movie_screen_005cb160(native_at<Object>(host.hud_00e198c4(), 0xA0));
}

// 00457160.
void entity_track_bind_00457160(Seg5Host& host, Object track) {
    native_at<std::uint32_t>(track, 0x1D4) = native_at<std::uint32_t>(host.game_00e188a8(), 0x19FC);
    host.movie_track_bind_entities_00456dd0(track);
}

// 0045AF70.
void camera_track_add_shake_0045af70(Seg5Host& host, Object track, float time, float x, float y,
                                     float z, float key_spacing, std::uint8_t camera) {
    shake_list_add_0045a320(host, camera != 0 ? track + 0x1DC : track + 0x1E8, time, x, y, z,
                            key_spacing);
}

// 0045BD90.
void camera_track_update_0045bd90(Seg5Host& host, Object track, float time) {
    host.entity_track_update_00459020(track, time);
    camera_track_update_shakes_0045acc0(host, track);
}

// 0045BE40. The EH states only order the member destructors.
void camera_track_destroy_0045be40(Seg5Host& host, Object track) {
    action_list_destroy_004569a0(host, track + 0x1E8);
    action_list_destroy_004569a0(host, track + 0x1DC);
    entity_track_destroy_0045b220(host, track);
}

// 0045C470.
void game_rebuild_local_player_units_0045c470(Seg5Host& host, Object game) {
    native_at<std::uint8_t>(game, 0x193C) = 0;
    host.game_build_local_player_unit_lists_004c3cb0(game);
}

// 0045C510.
void win32_file_writer_close_handle_0045c510(Seg5Host& host, Object writer) {
    void* handle = native_at<void*>(writer, 0x14);
    if (handle != nullptr) host.close_handle_import_00ce225c(handle);
    native_at<void*>(writer, 0x14) = nullptr;
}

// 0045CA40.
void win32_file_writer_destroy_0045ca40(Seg5Host& host, Object writer) {
    void* handle = native_at<void*>(writer, 0x14);
    native_at<std::uint32_t>(writer, 0x00) = 0x00CE4E04u;
    if (handle != nullptr) host.close_handle_import_00ce225c(handle);
    native_at<std::uint32_t>(writer, 0x00) = 0x00CE4DFCu;
    native_at<void*>(writer, 0x14) = nullptr;
}

// 0045CAC0.
void win32_touch_file_0045cac0(Seg5Host& host, Object path) {
    alignas(4) std::uint8_t writer[0x18]{};
    native_at<std::uint32_t>(writer, 0x08) = 0;
    native_at<std::uint32_t>(writer, 0x0C) = 2;
    native_at<std::uint8_t>(writer, 0x10) = 0x20;
    native_at<std::uint8_t>(writer, 0x04) = 0;
    native_at<std::uint32_t>(writer, 0x00) = 0x00CE4E04u;
    native_at<void*>(writer, 0x14) = nullptr;
    const char* name = native_at<const char*>(path, 0x04);
    if (name == nullptr) name = "";  // 00E18511, a .bss byte (zero at load) used as ""
    if (host.win32_file_writer_open_009501b0(writer, name)) {
        if (native_at<void*>(writer, 0x14) != nullptr) host.close_handle_import_00ce225c(native_at<void*>(writer, 0x14));
        native_at<void*>(writer, 0x14) = nullptr;
    }
    native_at<std::uint32_t>(writer, 0x00) = 0x00CE4E04u;
    if (native_at<void*>(writer, 0x14) != nullptr) host.close_handle_import_00ce225c(native_at<void*>(writer, 0x14));
}

// ---------------------------------------------------------------------------
// Camera shake channels and track searches (cc13-x87).
//
// Every x87 sequence below was traced from the bytes with the stack depth at
// each step (docs/CC13_SEG5.md, "Camera shake"). Precision model: a value the
// image keeps in an x87 register is a double here and is rounded to float
// exactly where the image stores it (FSTP dword); that reproduces the 53- and
// 64-bit results. The main thread runs at 24-bit precision control
// (docs/CC13_FPU_PRECISION.md), where every register operation also rounds to a
// 24-bit significand; the doc lists, per function, the register intermediates
// where the two models can differ. A single operation on two floats followed by
// a float store is written as a float operation: it is correctly rounded in
// either model.
// ---------------------------------------------------------------------------

namespace {
constexpr float kShakeNegZero = -0.0f;                  // 00D7A208 (float 80000000h), SUBSS minuend
constexpr float kShakeSpacingLow = 0.25f;               // 00CE3868 (float)
constexpr float kShakeSpacingHigh = 1.0f;               // FLD1
constexpr double kShakeWindow = 4.0;                    // 00D7A328 (double)
constexpr double kShakeEndOffset = 1000000.0;           // 00CE4C08 (double)
constexpr double kShakeEpsilon = 0.10000000149011612;   // 00D7A3A0 (double, 0.1f widened)
constexpr double kShakeFadeHalf = 0.5;                  // 00D7A280 (double)
constexpr float kShakeClampHigh = 1.0f;                 // 00D7A24C (float)
constexpr float kSampleNearestStart = 999999.875f;      // 00CE4C00 (float 497423FEh)
constexpr float kActionNearestStart = 9999.0f;          // 00CE4C04 (float 461C3C00h)
constexpr float kDefaultFovDegrees = 35.0f;             // 00CE4D90 (float)
constexpr double kPiOfFloat = 3.1415927410125732;       // 00CE3D28 (double 400921FB60000000h)
constexpr double kHalfTurnDegrees = 180.0;              // 00CE3D20 (double)

Object shake_key(Object channel, std::int32_t index) {
    return channel + 0x1C + index * 0x10;
}

// The inlined key moves in 00454A50 copy four floats with FLD/FSTP.
void move_key(Object dest, Object source) {
    for (std::uint32_t offset = 0; offset < 0x10; offset += 4) {
        native_at<float>(dest, offset) = native_at<float>(source, offset);
    }
}

// One component draw: hi is the amplitude (FLD/FSTP copy), lo is SUBSS
// (-0.0f - amplitude); ECX = 1.
float draw_component(Seg5Host& host, float amplitude) {
    return host.random_uniform_float_range_00bd2f10(1, kShakeNegZero - amplitude, amplitude);
}

// FLD a; FSUB b; FSTP float; then the sign cleared (FABS or AND 7FFFFFFFh).
float abs_difference(float a, float b) {
    return std::fabs(a - b);
}
} // namespace

// 00454970: four FLD/FSTP DWORD copies, which quiet a signalling NaN (cc13-fixture10).
Object shake_key_copy_00454970(Object dest, Object source) {
    for (std::uint32_t offset = 0; offset < 0x10; offset += 4) {
        native_at<std::uint32_t>(dest, offset) = cc13x87::x87_fld_fstp_bits(native_at<std::uint32_t>(source, offset));
    }
    return dest;
}

// 00455880. The keys go through the CRT's `vector copy constructor iterator'
// (00453ED0) with element size 10h, count 5 and 00454970; that helper calls the
// copy for each element in order and only adds unwinding, which a float copy
// never needs, so the five calls are made directly.
Object shake_channel_copy_construct_00455880(Object dest, Object source) {
    native_at<float>(dest, 0x00) = native_at<float>(source, 0x00);
    native_at<float>(dest, 0x04) = native_at<float>(source, 0x04);
    native_at<float>(dest, 0x08) = native_at<float>(source, 0x08);
    native_at<std::uint8_t>(dest, 0x0C) = native_at<std::uint8_t>(source, 0x0C);
    native_at<float>(dest, 0x10) = native_at<float>(source, 0x10);
    native_at<float>(dest, 0x14) = native_at<float>(source, 0x14);
    native_at<float>(dest, 0x18) = native_at<float>(source, 0x18);
    for (std::int32_t i = 0; i < 5; ++i) {
        shake_key_copy_00454970(shake_key(dest, i), shake_key(source, i));
    }
    native_at<std::uint8_t>(dest, 0x6C) = native_at<std::uint8_t>(source, 0x6C);
    return dest;
}

// 00454A50.
void shake_channel_shift_and_redraw_keys_00454a50(Seg5Host& host, Object channel,
                                                  std::int32_t index, std::uint8_t forward,
                                                  std::int32_t shift) {
    if (shift != 0) {
        if (shift > 0) {
            // 00454A6B..00454B66: keys 4 down to `shift` take key - shift
            // (unrolled by four; the order is descending throughout).
            for (std::int32_t i = 4; i >= shift; --i) move_key(shake_key(channel, i), shake_key(channel, i - shift));
        } else {
            // 00454B68..00454C3D: keys 0 up to 4 + shift take key - shift.
            for (std::int32_t i = 0; i < shift + 5; ++i) move_key(shake_key(channel, i), shake_key(channel, i - shift));
        }
    }
    if (forward == 0) {
        // 00454D8B: backwards from index - 1. The next key's time is copied to
        // a double slot, r * spacing is subtracted in ST and stored as float.
        for (std::int32_t i = index - 1; i >= 0; --i) {
            Object key = shake_key(channel, i);
            const double next_time = native_at<float>(key, 0x1C);
            const float r = host.random_uniform_float_range_00bd2f10(1, kShakeSpacingLow, kShakeSpacingHigh);
            const float time = static_cast<float>(
                next_time - static_cast<double>(r) * static_cast<double>(native_at<float>(channel, 0x08)));
            native_at<float>(key, 0x0C) = time;
            // FLDZ; FCOMIP ST1; JB: BL = 1 unless 0 < time (NaN gives 0).
            const bool expired = time <= 0.0f;
            native_at<float>(key, 0x00) = native_at<std::uint8_t>(channel, 0x0C) != 0 || expired
                                              ? 0.0f : draw_component(host, native_at<float>(channel, 0x10));
            native_at<float>(key, 0x04) = native_at<std::uint8_t>(channel, 0x0C) != 0 || expired
                                              ? 0.0f : draw_component(host, native_at<float>(channel, 0x14));
            native_at<float>(key, 0x08) = native_at<std::uint8_t>(channel, 0x0C) != 0 || expired
                                              ? 0.0f : draw_component(host, native_at<float>(channel, 0x18));
        }
    } else {
        // 00454C4E: forwards from index + 1; r * spacing + the previous key's
        // time in ST, stored as float.
        for (std::int32_t i = index + 1; i < 5; ++i) {
            Object key = shake_key(channel, i);
            const float r = host.random_uniform_float_range_00bd2f10(1, kShakeSpacingLow, kShakeSpacingHigh);
            native_at<float>(key, 0x0C) = static_cast<float>(
                static_cast<double>(r) * static_cast<double>(native_at<float>(channel, 0x08)) +
                static_cast<double>(native_at<float>(shake_key(channel, i - 1), 0x0C)));
            native_at<float>(key, 0x00) = native_at<std::uint8_t>(channel, 0x0C) == 0
                                              ? draw_component(host, native_at<float>(channel, 0x10)) : 0.0f;
            native_at<float>(key, 0x04) = native_at<std::uint8_t>(channel, 0x0C) == 0
                                              ? draw_component(host, native_at<float>(channel, 0x14)) : 0.0f;
            native_at<float>(key, 0x08) = native_at<std::uint8_t>(channel, 0x0C) == 0
                                              ? draw_component(host, native_at<float>(channel, 0x18)) : 0.0f;
        }
    }
}

// 00454EF0.
float* shake_channel_evaluate_00454ef0(Seg5Host& host, Object channel, float* out, float time) {
    // FLD time; FSUB [ESI]; FSTP float.
    const float rel = time - native_at<float>(channel, 0x00);
    // COMISS 0, rel; JA: NaN carries on.
    if (native_at<std::uint8_t>(channel, 0x0C) != 0 || 0.0f > rel) {
        const float* fixed = host.static_vector_00f87574();
        out[0] = fixed[0];
        out[1] = fixed[1];
        out[2] = fixed[2];
        return out;
    }
    // spacing * 4.0 stays in ST for both window tests.
    const double window = static_cast<double>(native_at<float>(channel, 0x08)) * kShakeWindow;
    const float first_time = native_at<float>(shake_key(channel, 0), 0x0C);
    bool redraw = static_cast<double>(first_time) - window > static_cast<double>(rel);
    float last_time = 0.0f;
    if (!redraw) {
        last_time = native_at<float>(shake_key(channel, 4), 0x0C);
        redraw = static_cast<double>(rel) > static_cast<double>(last_time) + window;
    }
    if (redraw) {
        // 004550BF: re-anchor key 2 on rel with fresh components (no frozen
        // test here), then redraw both sides of it.
        Object key = shake_key(channel, 2);
        native_at<float>(key, 0x0C) = rel;
        native_at<float>(key, 0x00) = draw_component(host, native_at<float>(channel, 0x10));
        native_at<float>(key, 0x04) = draw_component(host, native_at<float>(channel, 0x14));
        native_at<float>(key, 0x08) = draw_component(host, native_at<float>(channel, 0x18));
        shake_channel_shift_and_redraw_keys_00454a50(host, channel, 2, 1, 0);
        shake_channel_shift_and_redraw_keys_00454a50(host, channel, 2, 0, 0);
        out[0] = native_at<float>(key, 0x00);
        out[1] = native_at<float>(key, 0x04);
        out[2] = native_at<float>(key, 0x08);
        return out;
    }
    float result[3];
    if (first_time > rel) {
        // 00454F5F: before the window, slide the keys up by three.
        shake_channel_shift_and_redraw_keys_00454a50(host, channel, 3, 0, 3);
        const float* r = shake_channel_evaluate_00454ef0(host, channel, result, time);
        out[0] = r[0];
        out[1] = r[1];
        out[2] = r[2];
        return out;
    }
    if (rel > last_time) {
        // 00454F9D: past the window, slide the keys down by three.
        shake_channel_shift_and_redraw_keys_00454a50(host, channel, 1, 1, -3);
        const float* r = shake_channel_evaluate_00454ef0(host, channel, result, time);
        out[0] = r[0];
        out[1] = r[1];
        out[2] = r[2];
        return out;
    }
    // 00454FB9: the first key from 1 whose time exceeds rel (5 when none
    // does, as for a NaN rel; the image then reads past the keys too).
    std::int32_t i = 1;
    for (; i < 5; ++i) {
        if (native_at<float>(shake_key(channel, i), 0x0C) > rel) break;
    }
    Object previous = shake_key(channel, i - 1);
    Object next = shake_key(channel, i);
    const double previous_time = native_at<float>(previous, 0x0C);
    // (rel - previous) and (next - previous) stay in ST; the quotient is stored.
    float f = static_cast<float>((static_cast<double>(rel) - previous_time) /
                                 (static_cast<double>(native_at<float>(next, 0x0C)) - previous_time));
    f = bsp::unit_clamp_00415690(f, 0.0f, kShakeClampHigh);
    const float a0 = native_at<float>(next, 0x00) * f;
    const float a1 = native_at<float>(next, 0x04) * f;
    const float a2 = native_at<float>(next, 0x08) * f;
    const float g = 1.0f - f;
    const float b0 = native_at<float>(previous, 0x00) * g;
    const float b1 = native_at<float>(previous, 0x04) * g;
    const float b2 = g * native_at<float>(previous, 0x08);
    out[0] = b0 + a0;
    out[1] = b1 + a1;
    out[2] = b2 + a2;
    return out;
}

// 00456420.
Object shake_channel_construct_00456420(Seg5Host& host, Object channel, float time, float x,
                                        float y, float z, float key_spacing) {
    native_at<float>(channel, 0x00) = time;
    native_at<float>(channel, 0x04) = static_cast<float>(static_cast<double>(time) + kShakeEndOffset);
    native_at<float>(channel, 0x08) = key_spacing;
    // (y*y + x*x) + z*z in ST, stored as float, then compared with 0.1.
    const float magnitude = static_cast<float>(
        (static_cast<double>(y) * y + static_cast<double>(x) * x) + static_cast<double>(z) * z);
    const bool frozen = kShakeEpsilon > static_cast<double>(magnitude);
    native_at<std::uint8_t>(channel, 0x0C) = frozen ? 1 : 0;
    const float arguments[3] = {x, y, z};
    const float* amplitude = frozen ? host.static_vector_00f87574() : arguments;
    native_at<float>(channel, 0x10) = amplitude[0];
    native_at<float>(channel, 0x14) = amplitude[1];
    native_at<float>(channel, 0x18) = amplitude[2];
    native_at<float>(channel, 0x28) = 0.0f;
    const float* fixed = host.static_vector_00f87574();
    native_at<float>(channel, 0x1C) = fixed[0];
    native_at<float>(channel, 0x20) = fixed[1];
    native_at<float>(channel, 0x24) = fixed[2];
    shake_channel_shift_and_redraw_keys_00454a50(host, channel, 0, 1, 0);
    return channel;
}

// 004569E0. The checked-iterator tests repeat the loop condition and are
// omitted.
float* shake_list_evaluate_004569e0(Seg5Host& host, Object list, float* out, float time) {
    const float* fixed = host.static_vector_00f87574();
    if (native_at<std::uint32_t>(list, 8) == 0) {
        out[0] = fixed[0];
        out[1] = fixed[1];
        out[2] = fixed[2];
        return out;
    }
    float sum[3] = {fixed[0], fixed[1], fixed[2]};
    Object node = native_at<Object>(native_at<Object>(list, 4), 0);
    while (node != native_at<Object>(list, 4)) {
        Object channel = node + 8;
        if (native_at<float>(channel, 0x00) > time) break;  // FCOMIP; JA (sorted by start)
        Object next = native_at<Object>(node, 0);
        // (end + 0.5) - time in ST, stored; time - start stored.
        const float to_end = static_cast<float>(
            static_cast<double>(native_at<float>(channel, 0x04)) + kShakeFadeHalf - static_cast<double>(time));
        const float from_start = time - native_at<float>(channel, 0x00);
        // FCOMIP; JBE keeps to_end, also for NaN.
        const float nearer = to_end > from_start ? from_start : to_end;
        const float ramp = nearer + nearer;  // FADD ST0,ST0; FSTP float
        if (!(0.0f > ramp)) {
            bool apply = true;
            float weight = kShakeClampHigh;
            if (!(ramp > kShakeClampHigh)) {  // COMISS; JBE
                weight = ramp;
                apply = ramp > 0.0f;          // COMISS 0.0f (first dword of 00D7A218); JBE skips
            }
            if (apply) {
                float offset[3];
                const float* v = shake_channel_evaluate_00454ef0(host, channel, offset, time);
                const float p0 = v[0] * weight;
                const float p1 = v[1] * weight;
                const float p2 = weight * v[2];
                sum[0] = p0 + sum[0];
                sum[1] = p1 + sum[1];
                sum[2] = p2 + sum[2];
            }
        }
        node = next;
    }
    out[0] = sum[0];
    out[1] = sum[1];
    out[2] = sum[2];
    return out;
}

// 0045A320. The checked-iterator tests either repeat the loop condition or
// test the inserted node against the head, so none can fire on a valid list.
void shake_list_add_0045a320(Seg5Host& host, Object list, float time, float x, float y, float z,
                             float key_spacing) {
    alignas(4) std::uint8_t channel[0x70];
    shake_channel_construct_00456420(host, channel, time, x, y, z, key_spacing);
    Object last = nullptr;
    Object node = native_at<Object>(native_at<Object>(list, 4), 0);
    while (node != native_at<Object>(list, 4)) {
        last = node;
        Object next = native_at<Object>(node, 0);
        const float start = native_at<float>(node, 8);
        if (kShakeEpsilon > static_cast<double>(abs_difference(time, start))) {
            const float dx = native_at<float>(node, 0x18) - x;
            const float dy = native_at<float>(node, 0x1C) - y;
            const float dz = native_at<float>(node, 0x20) - z;
            if (kShakeEpsilon > static_cast<double>(abs_difference(native_at<float>(node, 0x10), key_spacing))) {
                // (dy*dy + dx*dx) + dz*dz in ST, stored as float: an equal
                // channel is already queued.
                const float distance = static_cast<float>(
                    (static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) + static_cast<double>(dz) * dz);
                if (kShakeEpsilon > static_cast<double>(distance)) return;
            }
        }
        if (start > time) {
            // 0045A4DE: insert before `node`.
            Object fresh = host.shake_list_buy_node_00457f30(list, node, native_at<Object>(node, 4), channel);
            host.shake_list_increment_size_00459ae0(list, 1);
            native_at<Object>(node, 4) = fresh;
            native_at<Object>(native_at<Object>(fresh, 4), 0) = fresh;
            return;
        }
        node = next;
    }
    // 0045A52A: the last channel ends where the new one starts, then push_back.
    if (native_at<std::uint32_t>(list, 8) != 0) {
        native_at<float>(last, 0x0C) = native_at<float>(channel, 0x00);
    }
    Object head = native_at<Object>(list, 4);
    Object fresh = host.shake_list_buy_node_00457f30(list, head, native_at<Object>(head, 4), channel);
    host.shake_list_increment_size_00459ae0(list, 1);
    native_at<Object>(head, 4) = fresh;
    native_at<Object>(native_at<Object>(fresh, 4), 0) = fresh;
}

// 0045ACC0. The image reuses the destroyed strings' stack slots as the
// 004569E0 output buffers; separate locals hold the same values.
void camera_track_update_shakes_0045acc0(Seg5Host& host, Object track) {
    if (native_at<std::uint32_t>(track, 0x1D4) == 0) return;
    alignas(4) std::uint8_t name[8];
    host.native_string_construct_0041e870(name, "CameraPosition 01");  // 00CE4DA4
    Object eye_entity = host.entity_registry_find_by_name_00925180(
        native_at<Object>(host.game_00e188a8(), 0x19CC), name);
    release_native_string(host, name);
    host.native_string_construct_0041e870(name, "CameraLookat 01");  // 00CE4D94
    Object target_entity = host.entity_registry_find_by_name_00925180(
        native_at<Object>(host.game_00e188a8(), 0x19CC), name);
    release_native_string(host, name);

    const float time = native_at<float>(native_at<Object>(host.game_00e188a8(), 0x38), 4);
    if (native_at<std::uint8_t>(eye_entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(eye_entity);
    float eye[3] = {native_at<float>(eye_entity, 0xFC), native_at<float>(eye_entity, 0x100),
                    native_at<float>(eye_entity, 0x104)};
    float offset[3];
    const float* s = shake_list_evaluate_004569e0(host, track + 0x1DC, offset, time);
    eye[0] = s[0] + eye[0];
    eye[1] = s[1] + eye[1];
    eye[2] = s[2] + eye[2];

    if (native_at<std::uint8_t>(target_entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target_entity);
    float target[3] = {native_at<float>(target_entity, 0xFC), native_at<float>(target_entity, 0x100),
                       native_at<float>(target_entity, 0x104)};
    const float* t = shake_list_evaluate_004569e0(host, track + 0x1E8, offset, time);
    target[0] = t[0] + target[0];
    target[1] = t[1] + target[1];
    target[2] = t[2] + target[2];

    // FCOS and FSIN of the float roll (+148h, read twice), each stored as
    // float; the up vector is (sin, cos, 0).
    const float roll_cos = static_cast<float>(std::cos(static_cast<double>(native_at<float>(track, 0x148))));
    const float roll_sin = static_cast<float>(std::sin(static_cast<double>(native_at<float>(track, 0x148))));
    alignas(4) std::uint8_t matrix[0x40];
    Object view = host.matrix_build_look_at_00b63f10(matrix, eye, target, roll_sin, roll_cos, 0.0f);
    host.camera_set_view_matrix_00b71490(native_at<Object>(track, 0x1D4), view);

    const std::int32_t mode = native_at<std::int32_t>(native_at<Object>(host.game_00e188a8(), 0x38), 0x60);
    float fov = kDefaultFovDegrees;
    if (mode == 1 || mode == 2) fov = camera_track_fov_at_0045a5a0(host, track, time);  // FSTP float
    // fov * pi / 180 in ST, stored as float.
    const float radians = static_cast<float>(static_cast<double>(fov) * kPiOfFloat / kHalfTurnDegrees);
    host.camera_set_field_of_view_00b6fbb0(native_at<Object>(track, 0x1D4), radians);
}

// 00455BC0. The checked-iterator tests inside the walk repeat the loop
// condition; the one after the step back fires when no sample precedes `time`.
Object* entity_track_find_sample_before_00455bc0(Seg5Host& host, Object track, Object* out,
                                                 float time) {
    Object list = track + 0xF4;
    native_at<Object>(track, 0x100) = list;
    native_at<Object>(track, 0x104) = native_at<Object>(native_at<Object>(track, 0xF8), 0);
    for (;;) {
        Object node = native_at<Object>(track, 0x104);
        if (node == native_at<Object>(list, 4)) {
            native_at<Object>(track, 0x100) = list;
            native_at<Object>(track, 0x104) = native_at<Object>(list, 4);
            break;
        }
        if (native_at<float>(node, 8) > time) break;  // FCOMIP; JA
        native_at<Object>(track, 0x104) = native_at<Object>(node, 0);
    }
    Object previous = native_at<Object>(native_at<Object>(track, 0x104), 4);
    native_at<Object>(track, 0x104) = previous;
    if (previous == native_at<Object>(native_at<Object>(track, 0x100), 4)) host.checked_iterator_report_00bf6713();
    out[1] = native_at<Object>(track, 0x104);
    out[0] = native_at<Object>(track, 0x100);
    return out;
}

// 00455CD0. When no sample was taken (empty list) the image reports through
// 00BF6713 and then reads [0 + 4]; the read is kept (volatile) rather than
// guarded.
Object entity_track_find_nearest_sample_00455cd0(Seg5Host& host, Object track, float time) {
    float best = kSampleNearestStart;
    Object best_container = nullptr;
    Object best_node = nullptr;
    Object list = track + 0xF4;
    native_at<Object>(track, 0x100) = list;
    native_at<Object>(track, 0x104) = native_at<Object>(native_at<Object>(track, 0xF8), 0);
    while (native_at<Object>(track, 0x104) != native_at<Object>(list, 4)) {
        Object node = native_at<Object>(track, 0x104);
        // FCOMIP best, |time_n - time|; JBE skips (also for NaN).
        if (best > abs_difference(native_at<float>(node, 8), time)) {
            best_container = native_at<Object>(track, 0x100);
            best_node = node;
            best = abs_difference(native_at<float>(native_at<Object>(track, 0x104), 8), time);
        }
        native_at<Object>(track, 0x104) = native_at<Object>(node, 0);
    }
    if (best_container == nullptr) host.checked_iterator_report_00bf6713();
    if (best_node == *reinterpret_cast<Object volatile*>(best_container + 4)) host.checked_iterator_report_00bf6713();
    return best_node + 8;
}

// 00455E30. As 00455BC0 over the action list.
Object* entity_track_find_action_before_00455e30(Seg5Host& host, Object track, Object* out,
                                                 float time) {
    Object list = track + 0x108;
    native_at<Object>(track, 0x114) = list;
    native_at<Object>(track, 0x118) = native_at<Object>(native_at<Object>(track, 0x10C), 0);
    for (;;) {
        Object node = native_at<Object>(track, 0x118);
        if (node == native_at<Object>(list, 4)) {
            native_at<Object>(track, 0x114) = list;
            native_at<Object>(track, 0x118) = native_at<Object>(list, 4);
            break;
        }
        if (native_at<float>(native_at<Object>(node, 8), 4) > time) break;  // FCOMIP; JA
        native_at<Object>(track, 0x118) = native_at<Object>(node, 0);
    }
    Object previous = native_at<Object>(native_at<Object>(track, 0x118), 4);
    native_at<Object>(track, 0x118) = previous;
    if (previous == native_at<Object>(native_at<Object>(track, 0x114), 4)) host.checked_iterator_report_00bf6713();
    out[1] = native_at<Object>(track, 0x118);
    out[0] = native_at<Object>(track, 0x114);
    return out;
}

// 00455F50. The result register starts as MOV EBP,[ESP+1Ch], the bits of
// the time argument; with a non-empty list and no action nearer than 9999.0
// those bits are returned as the pointer.
Object entity_track_find_nearest_action_00455f50(Seg5Host& host, Object track, float time) {
    (void)host;
    Object list = track + 0x108;
    Object head = native_at<Object>(track, 0x10C);
    if (native_at<Object>(head, 0) == head) return nullptr;
    std::uint32_t bits = 0;
    std::memcpy(&bits, &time, sizeof bits);
    Object best = reinterpret_cast<Object>(static_cast<std::uintptr_t>(bits));
    float best_distance = kActionNearestStart;
    native_at<Object>(track, 0x114) = list;
    native_at<Object>(track, 0x118) = native_at<Object>(native_at<Object>(list, 4), 0);
    while (native_at<Object>(track, 0x118) != native_at<Object>(list, 4)) {
        Object node = native_at<Object>(track, 0x118);
        if (best_distance > abs_difference(native_at<float>(native_at<Object>(node, 8), 4), time)) {
            best = native_at<Object>(native_at<Object>(track, 0x118), 8);
            best_distance = abs_difference(native_at<float>(native_at<Object>(native_at<Object>(track, 0x118), 8), 4), time);
        }
        native_at<Object>(track, 0x118) = native_at<Object>(node, 0);
    }
    return best;
}

// 00454340.
Object* list_iterator_post_decrement_00454340(Seg5Host& host, Object* it, Object* out) {
    out[0] = it[0];
    out[1] = it[1];
    if (it[0] == nullptr) host.checked_iterator_report_00bf6713();
    it[1] = native_at<Object>(it[1], 4);
    if (it[1] == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
    return out;
}

// 004552C0.
Object list_iterator_deref_004552c0(Seg5Host& host, const Object* it) {
    if (it[0] == nullptr) host.checked_iterator_report_00bf6713();
    if (it[1] == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
    return it[1] + 8;
}

// 004552F0.
Object* list_iterator_post_increment_004552f0(Seg5Host& host, Object* it, Object* out) {
    out[0] = it[0];
    out[1] = it[1];
    if (it[0] == nullptr) host.checked_iterator_report_00bf6713();
    if (it[1] == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
    it[1] = native_at<Object>(it[1], 0);
    return out;
}

// 00458160. The two checked-iterator tests on the last node cannot fire on a
// non-empty list.
float entity_track_last_sample_time_00458160(Seg5Host& host, Object track) {
    Object list = track + 0xF4;
    Object head = native_at<Object>(list, 4);
    if (native_at<Object>(head, 0) == head) return 0.0f;  // FLDZ
    Object last = native_at<Object>(native_at<Object>(list, 4), 4);
    alignas(4) std::uint8_t copy[0x38];
    host.record38_copy_construct_00457ff0(copy, last + 8);
    release_native_string(host, copy + 4);
    return native_at<float>(copy, 0);  // FLD dword: exact
}

namespace {
constexpr double kTrackBlendLead = 0.03999999910593033;  // 00CE4D68 (double, 0.04f widened)
constexpr double kTrackTimeScale = 100.0;                // 00D7A220 (double)
constexpr double kSplineTwo = 2.0;                       // 00D7A308 (double)
constexpr double kSplineThree = 3.0;                     // 00D7A2B0 (double)
constexpr double kSplineFour = 4.0;                      // 00D7A328 (double)
constexpr double kSplineFive = 5.0;                      // 00D7A370 (double)
constexpr double kSplineHalf = 0.5;                      // 00D7A280 (double)
constexpr double kSplineOne = 1.0;                       // 00D7A210 (double)

// The four sample records' inlined destructors, in the image's order.
void release_records(Seg5Host& host, Object d, Object c, Object b, Object a) {
    release_native_string(host, d + 4);
    release_native_string(host, c + 4);
    release_native_string(host, b + 4);
    release_native_string(host, a + 4);
}
} // namespace

// 004581E0. Records: a (ESP+4Ch, the sample at or before `time`), b (+84h,
// the next one), c (+0F4h, the one before a) and d (+0BCh, the one after b);
// 38h bytes each: +0 time, +4 NativeString, +0Ch position, +18h roll in
// degrees, +1Ch and +28h the two tangent points. The checked-iterator tests
// that repeat a condition already tested are omitted; the ones that can fire
// on a valid list call the hook.
float* entity_track_eval_position_004581e0(Seg5Host& host, Object track, float* out, float time,
                                           std::uint8_t exact) {
    alignas(4) std::uint8_t a[0x38];
    alignas(4) std::uint8_t b[0x38];
    alignas(4) std::uint8_t c[0x38];
    alignas(4) std::uint8_t d[0x38];
    host.timed_value_construct_004658d0(a, 0.0f);
    host.timed_value_construct_004658d0(b, 0.0f);
    host.timed_value_construct_004658d0(c, 0.0f);
    host.timed_value_construct_004658d0(d, 0.0f);

    if (exact == 0) {
        // time + 0.04 (double) in ST against the stored +190h; JB skips, so a
        // NaN skips too.
        if (static_cast<double>(time) + kTrackBlendLead >= static_cast<double>(native_at<float>(track, 0x190))) {
            if (native_at<std::int32_t>(track, 0x14C) == 1) native_at<std::int32_t>(track, 0x14C) = 2;
            if (native_at<std::int32_t>(track, 0x14C) == 3) {
                native_at<std::uint32_t>(track, 0x154) = 0;
                native_at<std::int32_t>(track, 0x14C) = 0;
            }
        }
    }
    const std::int32_t state = native_at<std::int32_t>(track, 0x14C);
    if (state != 0 && exact == 0) {
        if (state != 1 && state != 3) {
            // 004582DF: attached, the parent offset.
            out[0] = native_at<float>(track, 0x1C8);
            out[1] = native_at<float>(track, 0x1CC);
            out[2] = native_at<float>(track, 0x1D0);
            host.record38_destroy_00456d10(d);
            host.record38_destroy_00456d10(c);
            host.record38_destroy_00456d10(b);
            host.record38_destroy_00456d10(a);
            return out;
        }
        // 00458357: the blend from +158h to +190h; c and d keep their
        // constructed values.
        record38_assign_00458050(host, a, track + 0x158);
        record38_assign_00458050(host, b, track + 0x190);
    } else {
        Object list = track + 0xF4;
        if (native_at<std::uint32_t>(track, 0xFC) == 1) {
            // 0045837A: one sample, its position (the first-node test cannot fire).
            Object first = native_at<Object>(native_at<Object>(track, 0xF8), 0);
            out[0] = native_at<float>(first, 0x14);
            out[1] = native_at<float>(first, 0x18);
            out[2] = native_at<float>(first, 0x1C);
            release_records(host, d, c, b, a);
            return out;
        }
        const float last_time = entity_track_last_sample_time_00458160(host, track);
        if (time > last_time) {  // FCOMI; JBE (NaN goes to the search)
            // 0045845A: --end(), copied out; with an empty list the image
            // reports twice and copies from the head node.
            native_at<Object>(track, 0x100) = list;
            native_at<Object>(track, 0x104) = native_at<Object>(track, 0xF8);
            Object node = native_at<Object>(native_at<Object>(track, 0x104), 4);
            native_at<Object>(track, 0x104) = node;
            if (node == native_at<Object>(native_at<Object>(track, 0x100), 4)) host.checked_iterator_report_00bf6713();
            if (native_at<Object>(track, 0x104) == native_at<Object>(native_at<Object>(track, 0x100), 4)) {
                host.checked_iterator_report_00bf6713();
            }
            alignas(4) std::uint8_t e[0x38];
            host.record38_copy_construct_00457ff0(e, native_at<Object>(track, 0x104) + 8);
            out[0] = native_at<float>(e, 0x0C);
            out[1] = native_at<float>(e, 0x10);
            out[2] = native_at<float>(e, 0x14);
            release_native_string(host, e + 4);
            release_records(host, d, c, b, a);
            return out;
        }
        // 00458554.
        Object it[2];
        entity_track_find_sample_before_00455bc0(host, track, it, time);
        Object cur[2] = {it[0], it[1]};
        // No sample at or before `time`: the dereference and increment tests.
        if (it[1] == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
        cur[1] = native_at<Object>(it[1], 0);
        if (it[1] == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
        record38_assign_00458050(host, a, it[1] + 8);
        if (cur[1] == native_at<Object>(track, 0xF8)) {
            // 004585C9: no later sample, a's position.
            out[0] = native_at<float>(a, 0x0C);
            out[1] = native_at<float>(a, 0x10);
            out[2] = native_at<float>(a, 0x14);
            release_records(host, d, c, b, a);
            return out;
        }
        record38_assign_00458050(host, b, cur[1] + 8);
        Object before = a;
        if (it[1] != native_at<Object>(native_at<Object>(list, 4), 0)) {
            Object old[2];
            list_iterator_post_decrement_00454340(host, it, old);
            before = list_iterator_deref_004552c0(host, it);
        }
        record38_assign_00458050(host, c, before);
        list_iterator_post_increment_004552f0(host, cur, it);
        it[0] = cur[0];
        it[1] = cur[1];
        Object old[2];
        list_iterator_post_decrement_00454340(host, cur, old);
        // MOV [ESP+20h], head: cur's node slot is reused for end().
        Object end = native_at<Object>(list, 4);
        Object after = it[1] == end ? b : list_iterator_deref_004552c0(host, it);
        record38_assign_00458050(host, d, after);
    }

    // 004586EA: the parameter. (time - a) / 100 stays in ST; (b - a) / 100 is
    // stored, and the quotient is stored.
    const double a_time = native_at<float>(a, 0x00);
    const float span = static_cast<float>((static_cast<double>(native_at<float>(b, 0x00)) - a_time) / kTrackTimeScale);
    const float u = static_cast<float>(((static_cast<double>(time) - a_time) / kTrackTimeScale) / static_cast<double>(span));

    // 00458718: the rolls in radians, each (r / 180) * pi in ST, stored.
    const float roll_c = static_cast<float>(static_cast<double>(native_at<float>(c, 0x18)) / kHalfTurnDegrees * kPiOfFloat);
    const float roll_a = static_cast<float>(static_cast<double>(native_at<float>(a, 0x18)) / kHalfTurnDegrees * kPiOfFloat);
    const float roll_b = static_cast<float>(static_cast<double>(native_at<float>(b, 0x18)) / kHalfTurnDegrees * kPiOfFloat);
    const float roll_d = static_cast<float>(kPiOfFloat * (static_cast<double>(native_at<float>(d, 0x18)) / kHalfTurnDegrees));

    // 0045875D..004587D7: Catmull-Rom, all in ST, one store.
    {
        const double p0 = roll_c;
        const double p1 = roll_a;
        const double p2 = roll_b;
        const double p3 = roll_d;
        const double t = u;
        const double quadratic = ((p0 * kSplineTwo - kSplineFive * p1) + kSplineFour * p2) - p3;
        const double low = (quadratic * t) * t + ((p2 - p0) * t + p1 * kSplineTwo);
        const double cubic = ((p1 * kSplineThree - p0) - p2 * kSplineThree) + p3;
        native_at<float>(track, 0x148) = static_cast<float>((((cubic * t) * t) * t + low) * kSplineHalf);
    }

    // 004587DD: cubic Hermite on the position. The basis products stay in ST
    // until each basis value is stored; u*u, u*u*u, (2u)u, (3u)u and ((2u)u)u
    // keep the image's association.
    const float e0 = native_at<float>(b, 0x0C) - native_at<float>(b, 0x1C);
    const float e1 = native_at<float>(b, 0x10) - native_at<float>(b, 0x20);
    const float e2 = native_at<float>(b, 0x14) - native_at<float>(b, 0x24);
    const double uu = static_cast<double>(u) * u;
    const double uuu = static_cast<double>(u) * uu;
    const float h11 = static_cast<float>(uuu - uu);
    const float t0 = e0 * h11;
    const float t1 = e1 * h11;
    const float t2 = h11 * e2;
    const float g0 = native_at<float>(a, 0x0C) - native_at<float>(a, 0x28);
    const float g1 = native_at<float>(a, 0x10) - native_at<float>(a, 0x2C);
    const float g2 = native_at<float>(a, 0x14) - native_at<float>(a, 0x30);
    const double two_uu = (kSplineTwo * u) * u;
    const float h10 = static_cast<float>((uuu - two_uu) + u);
    const float s0 = g0 * h10;
    const float s1 = g1 * h10;
    const float s2 = h10 * g2;
    const double three_uu = (kSplineThree * u) * u;
    const double two_uuu = two_uu * u;
    const float h01 = static_cast<float>(three_uu - two_uuu);
    const float v0 = native_at<float>(b, 0x0C) * h01;
    const float v1 = h01 * native_at<float>(b, 0x10);
    const float v2 = h01 * native_at<float>(b, 0x14);
    const float h00 = static_cast<float>((two_uuu - three_uu) + kSplineOne);
    const float w0 = native_at<float>(a, 0x0C) * h00;
    const float w1 = native_at<float>(a, 0x10) * h00;
    const float w2 = h00 * native_at<float>(a, 0x14);
    const float x0 = w0 + v0;
    const float x1 = w1 + v1;
    const float x2 = w2 + v2;
    const float y0 = x0 + s0;
    const float y1 = x1 + s1;
    const float y2 = x2 + s2;
    out[0] = y0 + t0;
    out[1] = y1 + t1;
    out[2] = y2 + t2;
    release_records(host, d, c, b, a);
    return out;
}

// 004564E0. The tests inside the walk repeat the loop condition; the one on
// the result fires when no action precedes `time` (or the list is empty).
Object* action_list_find_before_004564e0(Seg5Host& host, Object track, Object* out, float time,
                                         Object list) {
    (void)track;
    Object node = native_at<Object>(native_at<Object>(list, 4), 0);
    for (;;) {
        if (node == native_at<Object>(list, 4)) {
            node = native_at<Object>(native_at<Object>(list, 4), 4);
            break;
        }
        if (native_at<float>(native_at<Object>(node, 8), 4) > time) {  // FCOMIP; JA
            node = native_at<Object>(node, 4);
            break;
        }
        node = native_at<Object>(node, 0);
    }
    if (node == native_at<Object>(list, 4)) host.checked_iterator_report_00bf6713();
    out[1] = node;
    out[0] = list;
    return out;
}

namespace {
constexpr float kFovLeadIn = 0.01f;                     // 00D7A238 (float 3C23D70Ah)
constexpr double kFovLeadOut = 0.009999999776482582;    // 00D7A358 (double, 0.01f widened)
constexpr double kFovEdge = 0.02;                       // 00CE4D88 (double 3F947AE147AE147Bh)

// push_back on a list<Action*>: 00455710(list, head, head->prev, &value),
// 00459A40(1), then the two links.
void action_list_push_back(Seg5Host& host, Object list, Object* value) {
    Object head = native_at<Object>(list, 4);
    Object prev = native_at<Object>(head, 4);
    Object node = host.action_list_buy_node_00455710(list, head, prev, value);
    host.action_list_increment_size_00459a40(list, 1);
    native_at<Object>(head, 4) = node;
    native_at<Object>(native_at<Object>(node, 4), 0) = node;
}

float fov_of(Object node) {
    return native_at<float>(native_at<Object>(node, 8), 0x20);
}
} // namespace

// 0045A5A0. Stack: the list at ESP+14h, the frame actions at +11Ch, +94h,
// +0D8h and +50h. The checked-iterator tests that cannot fire on this list
// (it always holds the four frame actions) are omitted.
float camera_track_fov_at_0045a5a0(Seg5Host& host, Object track, float time) {
    alignas(4) std::uint8_t list[0x0C];
    native_at<Object>(list, 4) = host.list_buy_head_node_004555b0(list);
    native_at<std::uint32_t>(list, 8) = 0;
    const float now = native_at<float>(native_at<Object>(host.game_00e188a8(), 0x38), 0);
    alignas(4) std::uint8_t first_frame[0x44];
    alignas(4) std::uint8_t lead_in[0x44];
    alignas(4) std::uint8_t lead_out[0x44];
    alignas(4) std::uint8_t last_frame[0x44];
    host.movie_action_fov_construct_00466020(first_frame, 0.0f);
    host.movie_action_fov_construct_00466020(lead_in, kFovLeadIn);
    // now - 0.01 (double) in ST, stored as float, reloaded as the argument.
    host.movie_action_fov_construct_00466020(
        lead_out, static_cast<float>(static_cast<double>(now) - kFovLeadOut));
    host.movie_action_fov_construct_00466020(last_frame, now);
    native_at<float>(first_frame, 0x20) = kDefaultFovDegrees;
    native_at<float>(lead_in, 0x20) = kDefaultFovDegrees;
    native_at<float>(lead_out, 0x20) = kDefaultFovDegrees;
    native_at<float>(last_frame, 0x20) = kDefaultFovDegrees;
    Object value = first_frame;
    action_list_push_back(host, list, &value);
    value = lead_in;
    action_list_push_back(host, list, &value);

    // 0045A701: the track's actions, iterator +114h/+118h.
    native_at<Object>(track, 0x114) = track + 0x108;
    native_at<Object>(track, 0x118) = native_at<Object>(native_at<Object>(track, 0x10C), 0);
    while (native_at<Object>(track, 0x118) != native_at<Object>(track, 0x10C)) {
        Object action = native_at<Object>(native_at<Object>(track, 0x118), 8);
        if (native_at<std::int32_t>(action, 0x10) == 0x12) {
            value = action;
            action_list_push_back(host, list, &value);
        }
        // FCOMIP time, ActivateTime; JBE skips (NaN too).
        if (native_at<std::int32_t>(action, 0x10) == 0x16 && time > native_at<float>(action, 4)) {
            Object due = action;
            action_list_push_back(host, list, &due);
            action_list_push_back(host, list, &due);
        }
        native_at<Object>(track, 0x118) = native_at<Object>(native_at<Object>(track, 0x118), 0);
    }
    value = lead_out;
    action_list_push_back(host, list, &value);
    value = last_frame;
    action_list_push_back(host, list, &value);

    if (native_at<std::uint32_t>(list, 8) < 5) {
        // 0045A8D8: no FOV action, the first frame's FOV; the list is
        // cleared inline here (the other exits call 00456670).
        const float fov = fov_of(native_at<Object>(native_at<Object>(list, 4), 0));
        host.action_destroy_00465610(last_frame);
        host.action_destroy_00465610(lead_out);
        host.action_destroy_00465610(lead_in);
        host.action_destroy_00465610(first_frame);
        Object node = native_at<Object>(native_at<Object>(list, 4), 0);
        native_at<Object>(native_at<Object>(list, 4), 0) = native_at<Object>(list, 4);
        native_at<Object>(native_at<Object>(list, 4), 4) = native_at<Object>(list, 4);
        native_at<std::uint32_t>(list, 8) = 0;
        if (node != native_at<Object>(list, 4)) {
            Object next = nullptr;
            do {
                next = native_at<Object>(node, 0);
                host.crt_free_00bf65ac(node);
                node = next;
            } while (next != native_at<Object>(list, 4));
        }
        host.crt_free_00bf65ac(native_at<Object>(list, 4));
        return fov;
    }

    float fov = 0.0f;
    if (kFovEdge > static_cast<double>(time)) {  // FCOMIP; JBE (NaN carries on)
        fov = fov_of(native_at<Object>(native_at<Object>(list, 4), 0));
    } else {
        const float last_time = entity_track_last_sample_time_00458160(host, track);
        // 0.02 + time in ST against the last sample time; JBE (NaN searches).
        if (kFovEdge + static_cast<double>(time) > static_cast<double>(last_time)) {
            fov = fov_of(native_at<Object>(native_at<Object>(list, 4), 4));
        } else {
            Object it[2];
            action_list_find_before_004564e0(host, track, it, time, list);
            Object node = it[1];
            // Dereference and increment tests: fire when no action precedes.
            if (node == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
            Object next = native_at<Object>(node, 0);
            if (node == native_at<Object>(it[0], 4)) host.checked_iterator_report_00bf6713();
            Object p1 = native_at<Object>(node, 8);
            if (next == native_at<Object>(list, 4) ||
                node == native_at<Object>(native_at<Object>(list, 4), 0)) {
                fov = native_at<float>(p1, 0x20);
            } else {
                // 0045AAF0. The outer two control values are read from the
                // same two nodes as the inner ones: [ESI+8] (this node) for
                // the first and [EBP+8] (the next node) for the last, so the
                // curve runs through (p1, p1, p2, p2).
                Object p2 = native_at<Object>(next, 8);
                const double t1 = native_at<float>(p1, 4);
                // (time - t1) / 100 stays in ST; (t2 - t1) / 100 is stored,
                // then the quotient is stored.
                const double scaled = (static_cast<double>(time) - t1) / kTrackTimeScale;
                const float span = static_cast<float>((static_cast<double>(native_at<float>(p2, 4)) - t1) / kTrackTimeScale);
                const float u = static_cast<float>(scaled / static_cast<double>(span));
                const double a = native_at<float>(native_at<Object>(node, 8), 0x20);
                const double b = native_at<float>(p1, 0x20);
                const double c = native_at<float>(p2, 0x20);
                const double d = native_at<float>(native_at<Object>(next, 8), 0x20);
                const double t = u;
                const double quadratic = ((a * kSplineTwo - b * kSplineFive) + c * kSplineFour) - d;
                const double low = (kSplineTwo * b + (c - a) * t) + (quadratic * t) * t;
                const double cubic = ((b * kSplineThree - a) - kSplineThree * c) + d;
                fov = static_cast<float>((t * ((cubic * t) * t) + low) * kSplineHalf);
            }
        }
    }
    host.action_destroy_00465610(last_frame);
    host.action_destroy_00465610(lead_out);
    host.action_destroy_00465610(lead_in);
    host.action_destroy_00465610(first_frame);
    host.action_list_destroy_00456670(list);
    return fov;
}

} // namespace bsp::cc13_seg5

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::cc13_seg5 {
void movie_action_clear_camera_node_00453580(Object action) noexcept {
    if (native_at<Object>(action, 0x1d4)) native_at<Object>(action, 0x1d4) = nullptr;
}
void movie_action_capture_camera_node_004542b0(Seg5Host& host, Object action) {
    Object game = host.game_00e188a8();
    Object actor = native_at<Object>(game, 0x1ed4);
    native_at<Object>(action, 0x1d4) = native_at<Object>(actor, 0x1bc);
}
TrackTimeIterator* entity_track_find_time_00455a80(Object track, TrackTimeIterator* out, float time,
                                                  TrackTimeSearchHost& host) {
    Object list = track + 0x108;
    auto& container = native_at<Object>(track, 0x114);
    auto& node = native_at<Object>(track, 0x118);
    *out = {nullptr, nullptr};
    container = list;
    node = native_at<Object>(native_at<Object>(track, 0x10c), 0);
    bool found = false;
    for (;;) {
        Object head = native_at<Object>(track, 0x10c);
        if (!container || container != list) host.invalid_parameter_00bf6713();
        if (node == head) break;
        if (!container) host.invalid_parameter_00bf6713();
        if (node == native_at<Object>(container, 4)) host.invalid_parameter_00bf6713();
        if (time < native_at<float>(native_at<Object>(node, 8), 4)) {
            *out = {container, node};
            found = true;
        }
        if (!container) host.invalid_parameter_00bf6713();
        if (node == native_at<Object>(container, 4)) host.invalid_parameter_00bf6713();
        node = native_at<Object>(node, 0);
    }
    if (!found) {
        container = list;
        node = native_at<Object>(list, 4);
        if (!container) host.invalid_parameter_00bf6713();
        node = native_at<Object>(node, 4);
        if (node == native_at<Object>(container, 4)) host.invalid_parameter_00bf6713();
        *out = {container, node};
    }
    return out;
}
void vehicle_launch_message_read_00464810(Object message, Object stream, VehicleLaunchReadHost& host) {
    host.read_extended_header_0075b4c0(message, stream);
    Object cursor = stream + 4;
    host.read_boolean_00428d70(cursor, message + 0x1c);
    // 00D7A248 = 7F7FFFFF (FLT_MAX), verified in the image.
    host.read_numeric_float_004293f0(cursor, message + 0x20, 0, 1, 0x1.fffffep+127f, 32);
}
}
