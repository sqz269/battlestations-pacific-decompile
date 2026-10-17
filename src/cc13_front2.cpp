// cc13_front2: callee functions that block wiring of reconstructed callers.
// Evidence, ABI and uncertainty per function: docs/CC13_FRONT2.md.
#include "bsp/cc13_front2.hpp"

#include "bsp/camera_decomposition.hpp"
#include "bsp/camera_position_modes.hpp"
#include "bsp/segment2_camera_cc13.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/world_ocean.hpp"

#include <cstring>

namespace bsp {

// The existing x87 kernel of 0042D0D0's arithmetic (src/material_effect_plane.cpp):
// the same instructions and spill offsets, then MOVSS copies of the spill to
// its destination, RET 4. Declared here rather than through its header, which
// pulls in D3D9.
float* __fastcall transform_native_effect_direction_0042d0d0_no_normalize(
    float* destination, const float* source, const float* matrix);

namespace {
template <class T>
T& at(std::uint8_t* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<T*>(base + offset);
}

// 00D7A260: 000080BFh, the idle value 0042DCD0 stores into +3D4h.
constexpr float kFocusTimerIdle = -1.0f;
// 00CE3C64: 3FC90FDBh (float pi/2).
constexpr float kHalfPi = 1.57079637f;
} // namespace

float* vector3f_transform_direction_optional_normalize_0042d0d0(float* destination,
                                                                const float* source,
                                                                const float* matrix,
                                                                std::uint8_t normalize) {
    // 0042D0D8..0042D14A: the three products-and-sums are spilled to the
    // stack (ESP+10h..+18h after PUSH ESI) before the branch at 0042D14E. The
    // kernel writes exactly that spill into its destination.
    float spill[3];
    transform_native_effect_direction_0042d0d0_no_normalize(spill, source, matrix);
    if (normalize != 0) {
        // 0042D150: LEA EDX,[ESP+10h]; ECX is still the destination; CALL 00419510.
        const OceanVec3 n = normalize_00419510(OceanVec3{spill[0], spill[1], spill[2]});
        destination[0] = n.x;
        destination[1] = n.y;
        destination[2] = n.z;
        return destination;   // 0042D159 MOV EAX,ESI
    }
    destination[0] = spill[0];   // 0042D162..0042D17D MOVSS copies
    destination[1] = spill[1];
    destination[2] = spill[2];
    return destination;
}

bool camera_mover_update_focus_0042dcd0(std::uint8_t* mover, float dt, std::uint8_t add_parent_yaw,
                                        Cc13Front2Host& host) {
    // 0042DCD9: a parent at +3ACh, then COMISS 0.0,[+3D4h] / JNC: the focus
    // branch runs when +3D4h is above zero or unordered.
    if (at<std::uint32_t>(mover, 0x3AC) != 0 && !(0.0f >= at<float>(mover, 0x3D4))) {
        const std::uint8_t follow_entity = at<std::uint8_t>(mover, 0x3D8);
        if (follow_entity == 0 || at<std::uint32_t>(mover, 0x3C4) != 0) {   // 0042DCF3/0042DCFC
            // 0042DD30..0042DD55: the fixed focus point +3C8h..+3D0h.
            float d[3] = {at<float>(mover, 0x3C8), at<float>(mover, 0x3CC), at<float>(mover, 0x3D0)};
            if (follow_entity != 0) {   // 0042DD5B
                std::uint8_t* const target = at<std::uint8_t*>(mover, 0x3C4);
                if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
                d[0] = at<float>(target, 0xFC);    // world position
                d[1] = at<float>(target, 0x100);
                d[2] = at<float>(target, 0x104);
            }
            if (at<std::uint8_t>(mover, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(mover);   // 0042DDA8
            // 0042DDAD..0042DDD4: FSUB and float store of each component.
            d[0] = d[0] - at<float>(mover, 0xFC);
            d[1] = d[1] - at<float>(mover, 0x100);
            d[2] = d[2] - at<float>(mover, 0x104);
            // 0042DDE1/0042DDE5: FLD z then FLD x, so ST1 = z (y operand), ST0 = x.
            // The result is float-stored at 0042DDEE before 00438B10.
            float yaw = static_cast<float>(host.ci_atan2_00bf701a(d[2], d[0]));
            yaw = wrapped_angle_subtract_00438b10(yaw, kHalfPi);   // 0042DDFA, second operand [00CE3C64]
            if (add_parent_yaw != 0) {   // 0042DDFF
                std::uint8_t* const parent = at<std::uint8_t*>(mover, 0x3AC);   // reloaded at 0042DE0A
                if (parent != nullptr) {
                    if (at<std::uint8_t>(parent, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(parent);
                    // 0042DE24..0042DE38: ECX = parent+CCh, EDX = output X (a stack
                    // local), stack output Y (the add_parent_yaw slot), output Z.
                    float angle_x = 0.0f;
                    float angle_y = 0.0f;
                    float angle_z = 0.0f;
                    extract_camera_matrix_angles_0042d2e0(
                        *reinterpret_cast<const CameraMatrix*>(parent + 0xCC), angle_x, angle_y, angle_z);
                    yaw = wrapped_angle_add_00438aa0(yaw, angle_y);   // 0042DE4F
                }
            }
            // 0042DE58: 00419510(ECX = a stack local, EDX = d); only y is read back.
            const OceanVec3 n = normalize_00419510(OceanVec3{d[0], d[1], d[2]});
            yaw = wrapped_angle_add_00438aa0(yaw, at<float>(mover, 0x3E0));   // 0042DE80
            const float offset_pitch = at<float>(mover, 0x3E4);   // pushed at 0042DE92, before 0042CF10
            const float pitch = wrapped_angle_add_00438aa0(camera_asin_clamped_0042cf10(n.y), offset_pitch);   // 0042DE9D/0042DEA6
            // 0042DEB2..0042DEBE: FMUL and a float store, the same value for both steps.
            const float step = at<float>(mover, 0x3DC) * dt;
            // 0042DEDD: 0042C750(ECX = mover; yaw, step, pitch, step), RET 10h;
            // its AL is not read.
            seg2_cc13::camera_mover_turn_toward_0042c750(at<float>(mover, 0x384), at<float>(mover, 0x388), yaw, step,
                                              pitch, step);
            return true;   // 0042DEE3
        }
        at<float>(mover, 0x3D4) = kFocusTimerIdle;   // 0042DCFE
        at<std::uint8_t>(mover, 0x3D8) = 0;          // 0042DD0E
    }
    at<float>(mover, 0x3E0) = 0.0f;   // 0042DD15
    at<float>(mover, 0x3E4) = 0.0f;   // 0042DD1D
    return false;
}

bool input_device_table_any_dynamic_device_button_down_00a91020(std::uint8_t* table,
                                                                Cc13Front2Host& host) {
    // Three checked vectors of device pointers, 24h apart: +6Ch the container
    // proxy, +70h first, +74h last, +78h end of storage.
    for (std::uint32_t device_class = 0; device_class < 3; ++device_class) {   // 00A91087..00A91098
        std::uint8_t* const vector = table + 0x6C + device_class * 0x24;
        std::uint32_t it = at<std::uint32_t>(vector, 4);   // 00A91030
        if (it > at<std::uint32_t>(vector, 8)) host.invalid_parameter_00bf6713();   // 00A91032..00A9103A
        for (;;) {
            const std::uint32_t last = at<std::uint32_t>(vector, 8);   // 00A91040, reloaded every pass
            if (at<std::uint32_t>(vector, 4) > last) host.invalid_parameter_00bf6713();   // 00A91043..00A9104B
            if (it == last) break;   // 00A91059
            if (!(it < at<std::uint32_t>(vector, 8))) host.invalid_parameter_00bf6713();   // 00A9105D
            void* const device = *reinterpret_cast<void**>(it);
            if (device != nullptr && host.input_device_vcall_2c(device)) return true;   // 00A91067..00A91076
            if (!(it < at<std::uint32_t>(vector, 8))) host.invalid_parameter_00bf6713();   // 00A91078
            it += 4;
        }
    }
    return false;
}

void gamepad_force_clear_all_channels_00a95cb0(Cc13Front2Host& host) {
    std::uint8_t* table = host.input_device_table_00f8bbf4();   // 00A95CB0
    // 00A95CB6..00A95CCE: the count is taken once (SAR 2 of the byte span; 0 for a null begin).
    std::int32_t count = 0;
    if (at<std::uint32_t>(table, 0xB8) != 0) {
        count = static_cast<std::int32_t>(at<std::uint32_t>(table, 0xBC) - at<std::uint32_t>(table, 0xB8)) >> 2;
    }
    for (std::int32_t i = 0; i < count; ++i) {   // JL, signed
        if (i != 0) table = host.input_device_table_00f8bbf4();   // 00A95CE0, reloaded after each call
        void* channel = nullptr;   // 00A95CFF: a null begin or i past the size passes ECX = 0
        const std::uint32_t begin = at<std::uint32_t>(table, 0xB8);
        if (begin != 0 &&
            static_cast<std::uint32_t>(i) <
                static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(table, 0xBC) - begin) >> 2)) {
            // 00A95D03..00A95D29: the checked operator[] repeats the test.
            const std::uint32_t begin2 = at<std::uint32_t>(table, 0xB8);
            if (begin2 == 0 ||
                !(static_cast<std::uint32_t>(i) <
                  static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(table, 0xBC) - begin2) >> 2))) {
                host.invalid_parameter_00bf6713();
            }
            channel = reinterpret_cast<void**>(at<std::uint32_t>(table, 0xB8))[i];
        }
        host.gamepad_force_clear_requests_00a95890(channel);   // 00A95D2C
    }
}

void session_transport_broadcast_except_00784790(std::uint8_t* transport, void* message,
                                                 void* exclude_peer, Cc13Front2Host& host) {
    // The peer list at transport+8h (EBP): +4h is the head node; a node is
    // {next +0, prev +4, peer +8}.
    std::uint8_t* const list = transport + 8;
    std::uint8_t* const head = at<std::uint8_t*>(transport, 0xC);   // 00784791
    std::uint8_t* const first = at<std::uint8_t*>(head, 0);
    if (first == head) host.invalid_parameter_00bf6713();   // 0078479C: ++ on end() of an empty list
    std::uint8_t* node = at<std::uint8_t*>(first, 0);   // 007847AA: the walk starts at the second node
    for (;;) {
        if (node == at<std::uint8_t*>(list, 4)) break;   // 007847B2..007847BE
        if (node == at<std::uint8_t*>(list, 4)) host.invalid_parameter_00bf6713();   // 007847C0 dereference check
        void* const peer = at<void*>(node, 8);   // 007847D0
        bool send = true;
        if (exclude_peer != nullptr) {   // 007847CE
            if (node == at<std::uint8_t*>(list, 4)) host.invalid_parameter_00bf6713();   // 007847D5
            send = exclude_peer != at<void*>(node, 8);   // 007847DF
        }
        if (send) {
            std::uint8_t* const lock = at<std::uint8_t*>(static_cast<std::uint8_t*>(peer), 4);
            host.enter_critical_section_ce2218(lock);   // 007847E8
            at<std::int32_t>(lock, 0x18) += 1;           // 007847F2, the lock depth
            host.session_transport_serialize_message_00783c80(transport, peer, message);   // 007847FC
            std::uint8_t* const lock_again = at<std::uint8_t*>(static_cast<std::uint8_t*>(peer), 4);   // 00784801
            at<std::int32_t>(lock_again, 0x18) -= 1;
            host.leave_critical_section_ce2210(lock_again);   // 00784809
        }
        if (node == at<std::uint8_t*>(list, 4)) host.invalid_parameter_00bf6713();   // 0078480F
        node = at<std::uint8_t*>(node, 0);   // 00784819
    }
}

namespace {
// An 8-byte NativeString block on the stack: +0 length, +4 data.
struct NativeStringBlock {
    std::uint32_t length;
    char* data;
};

// The inline destructor the image repeats after each temporary: when data is
// non-null, 00BD1510(00419CC0(), data, length + 1, 1).
void destroy_native_string(NativeStringBlock& s, Cc13Front2Host& host) {
    if (s.data != nullptr) {
        void* const pool = host.sized_storage_pool_get_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

constexpr std::uint32_t kRenderer = 0x00F8D394;
constexpr std::uint32_t kGame = 0x00E188A8;
constexpr std::uint32_t kDownloadManager = 0x00F8A304;
constexpr std::uint32_t kXenonManager = 0x00F8ABE8;
constexpr std::uint32_t kLanguageTable = 0x00F88974;
constexpr std::uint32_t kLocalization = 0x00F8BC4C;
constexpr std::uint32_t kShadowService = 0x00F8BBF0;
constexpr std::uint32_t kRenderService = 0x00F8D39C;
constexpr std::uint32_t kPlatform = 0x0109CF04;
constexpr std::uint32_t kGuiGate = 0x00E198C4;
constexpr std::uint32_t kLatchBlock = 0x00F88A3C;   // +1 byte, +4 dword, +8 byte, +0Ch flags
constexpr std::uint32_t kRegionString = 0x0108FF24;

// 00CE3800, 00CE3E18, 00D7A24C.
constexpr float kFoliageScales[3] = {0.5f, 0.699999988f, 1.0f};
} // namespace

void settings_apply_all_008d5b50(std::uint8_t* settings, Cc13Front2Host& host) {
    host.settings_apply_audio_008d5430(settings);   // 008D5B6C
    host.renderer_set_texture_detail_bias_00b1ffb0(host.load_global_pointer(kRenderer),
                                                   2 - at<std::int32_t>(settings, 0x68));   // 008D5B80
    host.renderer_vcall_f0(host.load_global_pointer(kRenderer), at<float>(settings, 0x64));   // 008D5B9A
    host.game_set_stick_options_004b46e0(host.load_global_pointer(kGame), at<std::uint8_t>(settings, 0x41),
                                         at<std::uint8_t>(settings, 0x42), at<std::uint8_t>(settings, 0x43),
                                         at<std::uint8_t>(settings, 0x44));   // 008D5BB6
    host.game_apply_camera_display_settings_004dcdf0(host.load_global_pointer(kGame));   // 008D5BC1

    // 008D5BC6..008D5C01: no download manager yet, a Xenon manager with a
    // selected user: new(844h) and, when that succeeds, construct it there.
    if (host.load_global_pointer(kDownloadManager) == nullptr) {
        std::uint8_t* const xenon = host.load_global_pointer(kXenonManager);
        if (xenon != nullptr && host.xenon_manager_has_selected_user_00a3e510(xenon)) {
            void* const storage = host.operator_new_00bf681b(0x844);
            if (storage != nullptr) host.download_manager_construct_009955f0(storage);
        }
    }

    if (std::uint8_t* game = host.load_global_pointer(kGame); game != nullptr) {   // 008D5C12
        if (at<std::uint32_t>(game, 0x19FC) != 0) {
            // 008D5C24..008D5C5D: a three-float stack table indexed by +54h with no bounds test.
            const float scales[3] = {kFoliageScales[0], kFoliageScales[1], kFoliageScales[2]};
            at<float>(at<std::uint8_t*>(game, 0x19FC), 0x4C) = scales[at<std::int32_t>(settings, 0x54)];
            game = host.load_global_pointer(kGame);   // 008D5C62
        }
        if (at<std::uint32_t>(game, 0x19F0) != 0) {   // 008D5C67
            at<std::uint8_t>(at<std::uint8_t*>(at<std::uint8_t*>(game, 0x19F0), 0xA8), 0xF8) =
                at<std::uint8_t>(settings, 0x6C);
        }
    }
    at<std::uint8_t>(host.media_manager_get_or_create_004c1710(), 0x14) = at<std::uint8_t>(settings, 9);   // 008D5C85

    // 008D5C90..008D5CC6: the language entry (20h bytes) font path at +18h;
    // an empty one selects "" (00CE3A0C), a null data pointer the text at 00F88A3C.
    std::uint8_t* entry = host.load_global_pointer(kLanguageTable) + at<std::int32_t>(settings, 4) * 0x20;
    const char* font_path = "";
    if (at<std::uint32_t>(entry, 0x18) != 0) {
        font_path = at<const char*>(entry, 0x1C);
        if (font_path == nullptr) font_path = reinterpret_cast<const char*>(host.global_address(kLatchBlock));
    }
    NativeStringBlock language_path{};   // ESP+0Ch after the EBX push
    host.native_string_assign_0041e870(&language_path, font_path);
    NativeStringBlock root{0, nullptr};   // 008D5CDB/008D5CE3
    host.native_string_resize_0041dd40(&root, 6, 1);   // 008D5CEB
    if (root.data != nullptr) std::memcpy(root.data, "Fonts\\", root.length + 1);   // 00CFEFA8
    host.font_registry_reload_paths_00ac3610(host.font_system_get_registry_007371d0(), &root,
                                             &language_path);   // 008D5D1C/008D5D23
    destroy_native_string(root, host);            // 008D5D28
    destroy_native_string(language_path, host);   // 008D5D4A

    // 008D5D6C..008D5DA1: the lanfile data pointer at entry+4, or the 00F88A3C text.
    entry = host.load_global_pointer(kLanguageTable) + at<std::int32_t>(settings, 4) * 0x20;
    const char* lanfile = at<const char*>(entry, 4);
    if (lanfile == nullptr) lanfile = reinterpret_cast<const char*>(host.global_address(kLatchBlock));
    NativeStringBlock language{};
    host.native_string_assign_0041e870(&language, lanfile);
    host.localization_set_language_00aa09d0(host.load_global_pointer(kLocalization), &language);
    destroy_native_string(language, host);   // 008D5DA6

    // 008D5DC8..008D5DE2: rumble on only when game+634h and settings+40h are both clear.
    const std::uint8_t rumble = (at<std::uint8_t>(host.load_global_pointer(kGame), 0x634) == 0 &&
                                 at<std::uint8_t>(settings, 0x40) == 0)
                                    ? 1
                                    : 0;
    host.gamepad_force_set_enabled_00a94c50(rumble);

    if (std::uint8_t* const manager = host.load_global_pointer(kDownloadManager); manager != nullptr) {   // 008D5DE7
        if (at<std::uint8_t>(manager, 0x1C) != 0) {   // 008D5DF5
            host.download_manager_vcall_24(manager);
            host.download_manager_filter_known_names_0043ea80(host.load_global_pointer(kDownloadManager),
                                                              settings + 0x98);   // 008D5E0F
            host.download_manager_vcall_14(host.load_global_pointer(kDownloadManager), settings + 0x98);
            host.download_manager_vcall_20(host.load_global_pointer(kDownloadManager));
        }
        NativeStringBlock table_name{};
        host.native_string_assign_0041e870(&table_name, "globals");   // 00CF7774
        host.localization_register_table_name_00aa0d30(host.load_global_pointer(kLocalization), &table_name);
        destroy_native_string(table_name, host);   // 008D5E58
        host.localization_reload_tables_00aa06d0(host.load_global_pointer(kLocalization), 1);   // 008D5E81
    }

    // 008D5E86..008D5ED4: first-call latches in 00F88A3C..00F88A4B (flags at +0Ch).
    std::uint8_t* const latches = host.global_address(kLatchBlock);
    std::uint32_t flags = at<std::uint32_t>(latches, 0xC);
    if ((flags & 1) == 0) {
        flags |= 1;
        at<std::uint32_t>(latches, 0xC) = flags;
        at<std::uint8_t>(latches, 8) = at<std::uint8_t>(settings, 0x8D);
    }
    std::uint8_t changed = 0;   // 008D5EA4, a stack byte
    if ((flags & 2) == 0) {
        flags |= 2;
        at<std::uint32_t>(latches, 0xC) = flags;
        at<std::uint32_t>(latches, 4) = at<std::uint32_t>(settings, 0x70);
    }
    if ((flags & 4) == 0) {
        flags |= 4;
        at<std::uint32_t>(latches, 0xC) = flags;
        at<std::uint8_t>(latches, 1) = at<std::uint8_t>(settings, 0x84);
    }

    if (at<std::uint32_t>(host.load_global_pointer(kGame), 0x19EC) != 0) {   // 008D5ED9
        NativeStringBlock group{};
        host.native_string_assign_0041e870(&group, "FoliageGroup");   // 00D15B28
        void* const node = host.scene_find_node_by_name_00b72270(
            at<void*>(host.load_global_pointer(kGame), 0x19EC), &group, 0);   // 008D5F0F
        destroy_native_string(group, host);   // 008D5F14
        if (node != nullptr) {   // 008D5F3C
            const float factor = at<std::uint8_t>(settings, 0x86) != 0 ? 1.0f : 0.0f;
            host.scene_node_set_visibility_factor_00b6da70(node, factor, 1);   // 008D5F67
        }
        std::uint8_t* game = host.load_global_pointer(kGame);
        if (at<std::uint32_t>(game, 0x19E8) != 0) {   // 008D5F71
            void* const clouds = host.atmosphere_get_cloud_manager_00bbddf0(at<void*>(game, 0x19E8));
            host.cloud_manager_set_enabled_00bbcfe0(clouds, at<std::uint8_t>(settings, 0x74));   // 008D5F8C
            game = host.load_global_pointer(kGame);
        }
        if (at<std::uint32_t>(game, 0x19F0) != 0) {   // 008D5F96
            at<std::uint8_t>(at<std::uint8_t*>(at<std::uint8_t*>(game, 0x19F0), 0xA8), 0xF8) =
                at<std::uint8_t>(settings, 0x6C);
            const std::uint32_t value = at<std::uint32_t>(settings, 0x70);
            if (at<std::uint32_t>(latches, 4) != value) {   // 008D5FB7
                host.ocean_effect_vcall_10(
                    at<void*>(at<std::uint8_t*>(host.load_global_pointer(kGame), 0x19F0), 0xA8), value);
                changed = 1;
                at<std::uint32_t>(latches, 4) = at<std::uint32_t>(settings, 0x70);
            }
        }
        if (at<std::uint8_t>(latches, 1) != at<std::uint8_t>(settings, 0x84)) {   // 008D5FED
            host.shadow_service_vcall_08(host.load_global_pointer(kShadowService), at<std::uint8_t>(settings, 0x84));
            changed = 1;
            at<std::uint8_t>(latches, 1) = at<std::uint8_t>(settings, 0x84);
        }
        if (at<std::uint8_t>(latches, 8) != at<std::uint8_t>(settings, 0x8D)) {   // 008D6019
            changed = 1;
            at<std::uint8_t>(latches, 8) = at<std::uint8_t>(settings, 0x8D);
        }
    }

    host.render_service_set_motion_blur_and_samples_00b0d580(
        host.load_global_pointer(kRenderService), at<std::uint8_t>(settings, 0x8D),
        at<std::uint32_t>(settings, 0x58));   // 008D603C
    std::uint32_t size_buffer[2];   // ESP+24h at 008D604F
    const std::uint32_t* const size = host.renderer_vcall_80(host.load_global_pointer(kRenderer), size_buffer);
    if (size[0] != at<std::uint32_t>(settings, 0x14) || size[1] != at<std::uint32_t>(settings, 0x18)) {
        host.render_resources_release_members_00b0fc00(host.load_global_pointer(kRenderService));   // 008D606C
    }
    // 008D6075 pushes the whole dword holding the changed byte (its upper
    // bytes are stale stack); 00B29E60 reads only the low byte.
    host.renderer_change_presentation_mode_00b29e60(
        host.load_global_pointer(kRenderer), at<std::uint32_t>(settings, 0x14), at<std::uint32_t>(settings, 0x18),
        at<std::uint8_t>(settings, 0x1E), at<std::uint32_t>(settings, 0x58), at<std::uint8_t>(settings, 0x60),
        changed);   // 008D6092
    host.platform_vcall_0c(host.load_global_pointer(kPlatform), at<std::uint8_t>(settings, 0x1E),
                           at<std::uint32_t>(settings, 0x14), at<std::uint32_t>(settings, 0x18));   // 008D60AF
    if (host.load_global_pointer(kGuiGate) == nullptr) {   // 008D60B1
        host.xlive_on_reset_device_00a4d46a(host.load_global_pointer(kRenderer) + 0x1A28);
    }
    host.render_resources_initialize_members_00b107f0(
        host.load_global_pointer(kRenderService), at<std::uint8_t>(settings, 0x8C),
        at<std::uint8_t>(settings, 0x8D), at<std::uint32_t>(settings, 0x58));   // 008D60E6
    if (host.load_global_pointer(kGuiGate) == nullptr) {   // 008D60EB
        host.gui_manager_relayout_all_pages_wide_screen_00aa4ef0(host.gui_manager_get_or_create_004c12b0());
    }
    host.render_service_set_film_effect_00b0d080(host.load_global_pointer(kRenderService),
                                                 at<std::uint32_t>(settings, 0x90));   // 008D610D

    // 008D6112..008D6153: the geography region into the global NativeString at
    // 0108FF24 (resize without keep, then copy length bytes when data is non-null).
    const char* const region = host.win32_get_user_geography_region_00439100();
    const std::uint32_t region_length = region != nullptr ? static_cast<std::uint32_t>(std::strlen(region)) : 0;
    std::uint8_t* const region_string = host.global_address(kRegionString);
    host.native_string_resize_0041dd40(region_string, region_length, 0);
    if (at<char*>(region_string, 4) != nullptr) {
        std::memcpy(at<char*>(region_string, 4), region, at<std::uint32_t>(region_string, 0));
    }
}

void net_entity_read_replication_state_0077fad0(std::uint8_t* entity, Cc13Front2Host& host) {
    host.sentity_default_init_slot_a4_009295b0(entity);   // 0077FAEC

    // 0077FAF1..0077FBF6: init record kind 2 with record data +C5h set.
    std::uint8_t* record = at<std::uint8_t*>(entity, 0xC0);
    if (record != nullptr && at<std::int32_t>(record, 4) == 2) {
        std::uint8_t* const data = at<std::uint8_t*>(record, 8);
        if (at<std::uint8_t>(data, 0xC5) != 0) {
            if (at<std::uint8_t>(data, 0xC6) != 0) {   // a group leader
                if (at<std::uint32_t>(entity, 0x284) == 0) host.unit_group_create_for_leader_0070db20(entity);
                at<std::uint32_t>(at<std::uint8_t*>(entity, 0x284), 0x4FC) = at<std::uint8_t>(data, 0xCA);
                host.unit_group_apply_formation_shape_0070efd0(at<void*>(entity, 0x284),
                                                               at<std::uint8_t>(data, 0xCB));   // 0077FB53
            } else if (void* const leader = host.entity_handle_table_resolve_00521e30(
                           at<std::uint16_t>(data, 0xC8));
                       leader != nullptr) {   // 0077FB64
                host.unit_group_join_or_merge_0077f940(entity, leader);
                std::uint8_t* const member =
                    host.unit_group_find_member_record_0070d080(at<void*>(entity, 0x284), entity);   // 0077FB80
                if (at<std::int32_t>(at<std::uint8_t*>(entity, 0x284), 0x4FC) == 0x18) {   // 0077FB8B
                    at<float>(member, 0x4) = at<float>(data, 0xCC);
                    at<float>(member, 0x8) = at<float>(data, 0xD0);
                    at<float>(member, 0xC) = at<float>(data, 0xD4);
                } else {   // 0077FBB1..0077FBF6, in the image's store order
                    at<float>(member, 0x10) = at<float>(data, 0xD8);
                    at<float>(member, 0x20) = at<float>(data, 0xEC);
                    at<float>(member, 0x14) = at<float>(data, 0xDC);
                    at<float>(member, 0x24) = at<float>(data, 0xF0);
                    at<float>(member, 0x18) = at<float>(data, 0xE0);
                    at<float>(member, 0x28) = at<float>(data, 0xF4);
                    at<float>(member, 0x1C) = at<float>(data, 0xE4);
                    at<float>(member, 0x2C) = at<float>(data, 0xF8);
                }
            }
        }
    }

    // 0077FBF9..0077FD97: init record kind 3, the saved Lua tables.
    record = at<std::uint8_t*>(entity, 0xC0);
    if (record != nullptr && at<std::int32_t>(record, 4) == 3) {
        alignas(4) std::uint8_t saved[0x14];    // the by-value argument area at ESP-14h
        alignas(4) std::uint8_t reader[0x14];   // the reader at ESP+14h
        host.sentity_get_saved_entity_lua_data_009238a0(entity, saved);   // 0077FC1D
        host.lua_reader_construct_004425c0(reader, saved);                // 0077FC26
        host.lua_reader_enter_key_00bd8e20(reader, 0, "_netEntity");      // 00D03E20
        host.lua_reader_enter_key_00bd8e20(reader, 0, "_reconData");      // 00D03E14
        host.recon_detection_set_read_archive_00804f10(entity + 0x1E4, reader);   // 0077FC7C
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_read_field_00bd6830(reader, 0, "hasPlrCmd", 3, entity + 0x290);   // 00D03E08
        if (host.lua_reader_has_key_00bd5eb0(reader, 0, "formation")) {   // 00D03DFC
            host.lua_reader_enter_key_00bd8e20(reader, 0, "formation");
            void* const group = host.unit_group_create_from_scene_0070ecf0(reader);
            at<void*>(entity, 0x284) = group;   // stored before the leave (0077FD0D)
            host.lua_reader_leave_key_00bd7a20(reader);
        }
        host.lua_reader_vcall_04(reader, 0, "lastBanto");   // 00D03DF0
        host.save_reader_read_unit_handle_006e6b10(entity + 0x2B0, reader);   // 0077FD39
        host.lua_reader_vcall_08(reader);
        host.lua_reader_read_field_00bd6830(reader, 0, "lastBantoPlayerIndex", 1, entity + 0x2D8);   // 00D03DD8
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_destroy_stack_instance_00441a20(reader);   // 0077FD97
    }

    // 0077FD9C..0077FE26: "reconlevel" (00D03DA0) as a new table in the entity's Lua self object.
    NativeStringBlock name{0, nullptr};
    host.native_string_resize_0041dd40(&name, 10, 1);
    if (name.data != nullptr) std::memcpy(name.data, "reconlevel", name.length + 1);
    alignas(4) std::uint8_t self_object[0x14];   // ESP+28h
    void* const object = host.mission_entity_lua_self_object_00927b40(entity, self_object);   // 0077FDE0
    host.lua_object_set_new_table_00b67580(object, &name);   // 0077FDF1
    host.lua_object_destruct_00b67700(self_object);          // 0077FDFF
    destroy_native_string(name, host);                       // 0077FE04

    // 0077FE2B..0077FE50: kinds 5 and 18h raise the generate channel.
    if (host.entity_vcall_5c_is_kind_of(entity, 5) || host.entity_vcall_5c_is_kind_of(entity, 0x18)) {
        host.warning_manager_dispatch_generate_00985590(host.load_global_pointer(0x00F8A0C4), entity);
    }
    if (at<std::uint32_t>(entity, 0x15C) != 0) {   // 0077FE55, JBE against 0
        host.entity_vcall_0c(entity, entity + 0x15C, 0);
    }
}

} // namespace bsp
