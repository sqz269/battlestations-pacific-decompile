// Packet cc13-sweepA4: segments 0..14 leftovers, fourth pass (cc13_sweepA4 marker).
// See include/bsp/cc13_sweepA4.hpp and docs/CC13_SWEEPA4.md. Each body carries
// its address, the original ABI and every simplification next to it.
// Common simplification: SEH unwind actions are not modelled (no body here
// throws itself).

#include "bsp/cc13_sweepA4.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_sweepA4 {

namespace {

constexpr std::uint32_t kGame = 0x00e188a8;  // [00E188A8] the game object

Raw raw(void* pointer) { return Raw{static_cast<std::uint8_t*>(pointer)}; }

}  // namespace

// ---------------------------------------------------------------------------
// 00447510 BSP_GameDynamicsList_Add
// __thiscall(ECX = list, const spec* [ESP+110h], bool [ESP+114h]), RET 8 at
// 00447B71. The three callers pass ECX = [00E188A8]+30h.
// spec: +0h node, +4h lifetime, +8h..+10h linear velocity, +14h..+1Ch angular
// velocity, +20h optional shape, +24h..+2Ch anchor (node space), +30h, +34h
// the descriptor owner, +54h.
// Constants: 1.0f 00D7A24C, 1000.0f 00CE3804, 2.0f 00CE3958, the doubles 10.0
// 00CE3DC0, 1.1 00CE3DF0, 1.0 00D7A210, 5.0 00D7A370.
// x87 forms from the bytes: 004479EE DE F1 is FDIVRP ST1 (ST1 = ST0 / ST1);
// 00447A01 DE FA is FDIVP ST2 (ST2 = ST2 / ST0); 00447A13 D8 F1 FDIV ST0,ST1;
// 00447A8C D8 F9 FDIVR ST0,ST1 (ST0 = ST1 / ST0). The products and sums are
// evaluated in x87 registers; here they are float (the image stores each
// partial result to a float slot except where noted).
// EH state 0 (00447648) guards the descriptor's shape vector; the unwind
// funclet is not modelled.
// ---------------------------------------------------------------------------
void game_dynamics_list_add_00447510(Raw list, Raw spec, bool scale_velocity, SweepA4Host& host) {
    DynBodyDescriptorRaw desc;
    // 0044752F..00447634: the descriptor defaults.
    desc.mass = 1.0f;
    desc.inertia[0] = desc.inertia[1] = desc.inertia[2] = 1.0f;
    desc.rows[0][0] = 1.0f; desc.rows[0][1] = 0.0f; desc.rows[0][2] = 0.0f;
    desc.rows[1][0] = 0.0f; desc.rows[1][1] = 1.0f; desc.rows[1][2] = 0.0f;
    desc.rows[2][0] = 0.0f; desc.rows[2][1] = 0.0f; desc.rows[2][2] = 1.0f;
    desc.position[0] = desc.position[1] = desc.position[2] = 0.0f;
    desc.linear_velocity[0] = desc.linear_velocity[1] = desc.linear_velocity[2] = 0.0f;
    desc.angular_velocity[0] = desc.angular_velocity[1] = desc.angular_velocity[2] = 0.0f;
    desc.linear_damping = 0.0f;
    desc.angular_damping = 0.0f;
    desc.shapes = nullptr;
    desc.shape_count = 0;
    desc.shape_capacity = 0;
    desc.lock_torque_to_row1 = 0;
    desc.flags = 0;
    desc.max_linear_speed = 1000.0f;   // 0044761B
    desc.max_angular_speed = 1000.0f;  // 00447624, overwritten at 0044779A
    desc.owner = nullptr;
    desc.contact_listener = nullptr;
    // 00447543: EBP = the physics world [[00E188A8]+18h], read before anything else.
    void* const world = raw(host.global_pointer(kGame)).ptr(0x18);

    // 00447642..00447663: refresh the node's world matrix unless node+5Ch bit 1
    // is set, then copy its 4x4 at node+F0h into the descriptor rows and position.
    Raw node = raw(spec.ptr(0x00));
    if ((node.at<std::uint8_t>(0x5c) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(node.p);
    host.physics_pose_from_matrix_00c336c0(&desc.rows[0][0], node.field(0xf0));

    // 00447668..004477A3: position += anchor * rows (each component is a dot
    // with one column), then both velocities and the owner from the spec.
    const float ax = spec.at<float>(0x24);
    const float ay = spec.at<float>(0x28);
    const float az = spec.at<float>(0x2c);
    const float t0 = ax * desc.rows[0][0] + ay * desc.rows[1][0] + az * desc.rows[2][0];
    const float t1 = ax * desc.rows[0][1] + ay * desc.rows[1][1] + az * desc.rows[2][1];
    const float t2 = ax * desc.rows[0][2] + ay * desc.rows[1][2] + az * desc.rows[2][2];
    desc.position[0] = desc.position[0] + t0;
    desc.linear_velocity[0] = spec.at<float>(0x08);
    desc.linear_velocity[1] = spec.at<float>(0x0c);
    desc.position[1] = t1 + desc.position[1];
    desc.position[2] = desc.position[2] + t2;
    desc.linear_velocity[2] = spec.at<float>(0x10);
    desc.angular_velocity[0] = spec.at<float>(0x14);
    desc.angular_velocity[1] = spec.at<float>(0x18);
    desc.angular_velocity[2] = spec.at<float>(0x1c);
    desc.max_angular_speed = 2.0f;  // 0044779A
    desc.owner = spec.ptr(0x34);    // 004477A3
    // 0044772D COMISS position.y, 0 with JBE: at or below zero, or unordered,
    // takes 2.0f for both dampings; strictly above takes 0.
    if (desc.position[1] > 0.0f) {
        desc.angular_damping = 0.0f;
        desc.linear_damping = 0.0f;
    } else {
        desc.angular_damping = 2.0f;
        desc.linear_damping = 2.0f;
    }

    // 004477D2..00447887: with a shape at spec+20h, shape+8h = 4 and
    // shape+0Ch = 0Dh, then the inlined push_back onto the descriptor vector
    // (capacity * 2 + 2 when full, element copy loop, free of the old block).
    if (spec.ptr(0x20) != nullptr) {
        raw(spec.ptr(0x20)).at<std::uint32_t>(0x08) = 4;
        raw(spec.ptr(0x20)).at<std::uint32_t>(0x0c) = 0x0d;
        if (desc.shape_count == desc.shape_capacity) {
            desc.shape_capacity = desc.shape_capacity * 2 + 2;
            void** const grown = static_cast<void**>(host.operator_new_00bf55be(desc.shape_capacity * 4));
            for (std::uint32_t i = 0; i < desc.shape_count; ++i) {
                if (grown + i != nullptr) grown[i] = desc.shapes[i];
            }
            if (desc.shapes != nullptr) host.free_00bf6989(desc.shapes);
            desc.shapes = grown;
        }
        if (desc.shapes + desc.shape_count != nullptr) desc.shapes[desc.shape_count] = spec.ptr(0x20);
        desc.shape_count += 1;
    }

    // 0044788C: the body.
    void* const body = host.physics_world_create_body_00c5d580(world, &desc);

    // 0044789C..00447951: when [body+70h] is 0, the node's box (vtable +4Ch,
    // called twice into the same out buffer) relative to the anchor becomes
    // the body box: lo from the first call's +0h..+8h, hi from the second's
    // +0Ch..+14h.
    if (host.physics_body_field_70_00c31dc0(body) == nullptr) {
        std::uint8_t box_out[0x18];
        const float* first = host.vcall_node_box_4c(node.p, box_out);
        float lo[3] = {first[0], first[1], first[2]};
        const float* second = host.vcall_node_box_4c(raw(spec.ptr(0x00)).p, box_out);
        float hi[3] = {second[3], second[4], second[5]};
        lo[0] = lo[0] - spec.at<float>(0x24);
        lo[1] = lo[1] - spec.at<float>(0x28);
        lo[2] = lo[2] - spec.at<float>(0x2c);
        hi[0] = hi[0] - spec.at<float>(0x24);
        hi[1] = hi[1] - spec.at<float>(0x28);
        hi[2] = hi[2] - spec.at<float>(0x2c);
        host.physics_body_set_box_00c31f60(body, lo, hi);
    }

    // 00447956..004479D4: extent = second - first of the body bounds;
    // mass = dy * dx * dz / 10.0 (double divide, stored as float).
    float first_bound[3];
    float second_bound[3];
    host.physics_body_bounds_00c31f90(body, first_bound, second_bound);
    const float dx = second_bound[0] - first_bound[0];
    const float dy = second_bound[1] - first_bound[1];
    const float dz = second_bound[2] - first_bound[2];
    const float mass = static_cast<float>(static_cast<double>(dy) * dx * dz / 10.0);

    // 00447974 CMP bool, 0 / 004479D8 JZ: with the flag, divisor =
    // (mass + 1.1 - 1.0) / 1.1 (doubles), and the descriptor's linear velocity
    // is divided by it in place.
    if (scale_velocity) {
        const float divisor = static_cast<float>((static_cast<double>(mass) + 1.100000023841858 - 1.0) /
                                                 1.100000023841858);
        desc.linear_velocity[0] = desc.linear_velocity[0] / divisor;
        desc.linear_velocity[1] = desc.linear_velocity[1] / divisor;
        desc.linear_velocity[2] = desc.linear_velocity[2] / divisor;
    }
    host.physics_body_set_linear_velocity_00c37e50(body, desc.linear_velocity);  // 00447A34

    // 00447A39..00447AC1: box inertia, k = mass / 5.0 (double, stored float).
    const float dx2 = dx * dx;
    const float dy2 = dy * dy;
    const float dz2 = dz * dz;
    const float k = static_cast<float>(static_cast<double>(mass) / 5.0);
    // The sums stay in x87 registers until the products are stored.
    const float inertia[3] = {static_cast<float>(k * (static_cast<double>(dy2) + dz2)),
                              static_cast<float>((static_cast<double>(dx2) + dz2) * k),
                              static_cast<float>(k * (static_cast<double>(dx2) + dy2))};
    host.physics_body_set_mass_00c37f40(body, mass);
    host.physics_body_set_inertia_00c37e70(body, inertia);

    // 00447AC6..00447B40: the 20h record, push_back onto list+10h.
    DynamicsRecordRaw record;
    record.node = spec.ptr(0x00);
    record.lifetime = spec.at<std::uint32_t>(0x04);
    record.field_08 = spec.at<std::uint32_t>(0x30);
    record.body = body;
    record.anchor[0] = spec.at<std::uint32_t>(0x24);
    record.anchor[1] = spec.at<std::uint32_t>(0x28);
    record.anchor[2] = spec.at<std::uint32_t>(0x2c);
    record.field_1c = spec.at<std::uint32_t>(0x54);
    host.dynamics_record_push_back_00447480(raw(list.field(0x10)), &record);

    // 00447B45..00447B5B: the descriptor's shape block is freed.
    if (desc.shapes != nullptr) host.free_00bf6989(desc.shapes);
}

// ---------------------------------------------------------------------------
// 004E2200 BSP_Game_RunConsoleCommandQueue
// __thiscall(ECX = game), plain RET at 004E2760; EBP-aligned frame (AND ESP,
// -8), EH state byte at [ESP+1A4h]. Sole caller 004E4A40 BSP_Game_OnMove.
// The queue is the std::list<string> at game+638h (head node at +63Ch, count
// +640h); requests go to game+5D8h through 004D3ED0.
// Frame slots, kept as separate storage here: the command string ESP+20h,
// temporaries ESP+3Ch / +60h / +7Ch (legacy strings), native strings ESP+18h
// and ESP+10h (the latter also the request dword and the destructor-flag
// dword at different times), ESP+58h (the erase iterator, later a native
// string), the 255-byte directory buffer ESP+98h.
// Literals: "frames" 00CE7F54, "term" 00CE7F4C, "pause" 00CE4AB0, "quit"
// 00CE7F44, ".scn" 00CE7888, "scenes" 00CE7F3C, "universe/scenes/" 00CE7F28,
// "command_line_mission" 00CE7F10; 00E18B1C is a zero-filled .bss char (the
// empty string the image uses for a null native string).
// CRT callees are bound directly: 00BF8417 _atol, 00BF9440 strstr, 00BF9611
// tolower.
// ---------------------------------------------------------------------------
namespace {

// The inlined c_str of a legacy string: the heap pointer at +4h when the
// capacity at +18h is 10h or more, else the buffer at +4h.
const char* legacy_c_str(Raw string) {
    return string.at<std::uint32_t>(0x18) >= 0x10 ? static_cast<const char*>(string.ptr(0x04))
                                                  : reinterpret_cast<const char*>(string.field(0x04));
}

// The front node of the list at game+638h; the checked-iterator report
// 00BF6713 when the list is empty (the image then continues).
void* console_front_node(Raw queue, SweepA4Host& host) {
    void* const head = queue.ptr(0x04);
    void* const node = raw(head).ptr(0x00);
    if (node == head) host.invalid_parameter_noinfo_00bf6713();
    return node;
}

// The inlined pop: copy the front string, then 004CECF0 erase(begin()).
void console_pop_front_into(Raw queue, Raw command, Raw iterator_out, SweepA4Host& host) {
    void* const node = console_front_node(queue, host);
    host.legacy_string_assign_substring_00408120(command, raw(raw(node).field(0x08)), 0, 0xffffffffu);
    host.list_erase_004cecf0(queue, iterator_out, queue, raw(queue.ptr(0x04)).ptr(0x00));
}

}  // namespace

void game_run_console_command_queue_004e2200(Raw game, SweepA4Host& host) {
    std::uint32_t destroy_flags = 0;  // [ESP+10h] = 0 at 004E2229
    // 004E222D / 004E2239: a queued command and no pending state request.
    if (game.at<std::uint32_t>(0x640) == 0) return;
    if (game.at<std::uint32_t>(0x5e8) != 0) return;
    Raw queue = raw(game.field(0x638));

    LegacyStringStorage command_storage;
    Raw command = raw(command_storage.bytes);
    NativeStringStorage iterator_storage;  // ESP+58h, the erase result
    // 004E2267..004E2273: the inlined empty construction.
    command.at<std::uint32_t>(0x18) = 0x0f;
    command.at<std::uint32_t>(0x14) = 0;
    command.at<std::uint8_t>(0x04) = 0;
    console_pop_front_into(queue, command, raw(iterator_storage.bytes), host);  // EH state 0

    LegacyStringStorage temp3c_storage;
    Raw temp3c = raw(temp3c_storage.bytes);
    NativeStringStorage string18_storage;
    Raw string18 = raw(string18_storage.bytes);
    std::int32_t request = 0;

    const auto finish_command = [&]() {
        // 004E2737..004E2748: the inlined destructor of the command string.
        if (command.at<std::uint32_t>(0x18) >= 0x10) host.free_00bf65ac(command.ptr(0x04));
    };
    const auto finish_frame = [&]() {
        // 004E271D..004E2732: temp3c, then string18 (state 0), then the command.
        host.legacy_string_destroy_004072d0(temp3c);
        host.native_string_destroy_storage_0041dd20(string18);
        finish_command();
    };

    // 004E2296: "frames", only while commands remain and the state at
    // game+5D4h is 5, 0Dh, 8 or 15h; any other state falls to "term".
    if (host.legacy_string_compare_substring_004beb60(command, 0, command.at<std::uint32_t>(0x14), "frames", 6) ==
            0 &&
        game.at<std::uint32_t>(0x640) != 0) {
        const std::uint32_t state = game.at<std::uint32_t>(0x5d4);
        if (state == 5 || state == 0x0d || state == 8 || state == 0x15) {
            // 004E22DE..004E230A: the count is the next command.
            console_pop_front_into(queue, command, raw(iterator_storage.bytes), host);
            const long frames = std::atol(legacy_c_str(command));  // 004E231F
            if (frames <= 0) {                                    // 004E2329 JLE
                finish_command();
                return;
            }
            // 004E232F..004E23A2: requeue the count less one, then "frames".
            host.native_string_from_int_004260b0(string18, static_cast<std::int32_t>(frames - 1));  // state 1
            const char* text = static_cast<const char*>(string18.ptr(0x04));
            if (text == nullptr) text = static_cast<const char*>(host.global_address(0x00e18b1c));
            host.legacy_string_construct_from_cstring_00407290(temp3c, text);  // state 2
            host.list_push_back_004d3900(queue, temp3c);
            host.legacy_string_destroy_004072d0(temp3c);  // state 1
            host.legacy_string_construct_from_cstring_00407290(temp3c, "frames");  // state 3
            host.list_push_back_004d3900(queue, temp3c);
            finish_frame();
            return;
        }
    }

    // 004E23A7..004E243F: the three fixed requests.
    if (host.legacy_string_compare_substring_004beb60(command, 0, command.at<std::uint32_t>(0x14), "term", 4) == 0) {
        request = 0x10;
        host.game_enqueue_state_request_004d3ed0(raw(game.field(0x5d8)), &request);
        finish_command();
        return;
    }
    if (host.command_line_matches_token_004c2df0(command, "pause")) {
        request = 0x12;
        host.game_enqueue_state_request_004d3ed0(raw(game.field(0x5d8)), &request);
        finish_command();
        return;
    }
    if (host.command_line_matches_token_004c2df0(command, "quit")) {
        request = 0x13;
        host.game_enqueue_state_request_004d3ed0(raw(game.field(0x5d8)), &request);
        finish_command();
        return;
    }

    // 004E2444..004E24A6: a scene path, when longer than four characters and
    // ending in ".scn" (the suffix temporary is destroyed through flag bit 0).
    bool is_scene = false;
    if (command.at<std::uint32_t>(0x14) > 4) {
        host.legacy_string_construct_substring_004cdbe0(command, temp3c, command.at<std::uint32_t>(0x14) - 4,
                                                        0xffffffffu);  // state 4
        destroy_flags = 1;
        is_scene = host.command_line_matches_token_004c2df0(temp3c, ".scn");
    }
    if ((destroy_flags & 1) != 0) host.legacy_string_destroy_004072d0(temp3c);  // state 0
    if (!is_scene) {
        finish_command();
        return;
    }

    // 004E24AC..004E24D1: a lowered native copy of the path (state 5).
    host.native_string_construct_0041e870(string18, legacy_c_str(command));
    host.string_lowercase_ascii_004bcc00(string18);
    // 004E24D6..004E254E: without "scenes" in it, the path is prefixed with
    // "universe/scenes/". The image tests strstr's result minus the data
    // against -1 after the null test, so only a null result prefixes.
    const char* const lowered = static_cast<const char*>(string18.ptr(0x04));
    const char* found = nullptr;
    if (lowered != nullptr) found = std::strstr(lowered, "scenes");
    if (lowered == nullptr || found == nullptr || static_cast<std::int32_t>(found - lowered) == -1) {
        LegacyStringStorage prefix_storage;  // ESP+60h
        LegacyStringStorage joined_storage;  // ESP+7Ch
        Raw prefix = raw(prefix_storage.bytes);
        Raw joined = raw(joined_storage.bytes);
        host.legacy_string_construct_from_cstring_00407290(prefix, "universe/scenes/");  // state 6
        host.game_split_command_token_004d0860(joined, prefix, command);             // state 7
        host.legacy_string_assign_substring_00408120(command, joined, 0, 0xffffffffu);
        host.legacy_string_destroy_004072d0(joined);
        host.legacy_string_destroy_004072d0(prefix);  // state 5
    }

    // 004E2553..004E2633: temp3c = the path; when the current directory is a
    // case-insensitive prefix of it, the prefix (and one '\' or '/') is cut.
    host.legacy_string_copy_construct_004116a0(temp3c, command);  // state 8
    char directory[0x104];  // ESP+98h, through ESP+19Bh
    if (host.get_current_directory_a_import_00ce2264(0xff, directory) != 0) {
        const std::uint32_t length = static_cast<std::uint32_t>(std::strlen(directory));
        bool matches = true;
        for (std::uint32_t i = 0; i < length; ++i) {
            const char* const at = host.legacy_string_at_004bcb10(temp3c, i);
            // MOVSX: the chars reach the CRT sign-extended, as here.
            const int mine = std::tolower(directory[i]);  // 004E25B9, first
            if (mine != std::tolower(*at)) {              // 004E25C6, 004E25D2 JNZ
                matches = false;
                break;
            }
        }
        if (matches) {
            std::uint32_t cut = length;
            if (*host.legacy_string_at_004bcb10(temp3c, length) == '\\' ||
                *host.legacy_string_at_004bcb10(temp3c, length) == '/')
                cut = length + 1;
            LegacyStringStorage rest_storage;  // ESP+60h
            Raw rest = raw(rest_storage.bytes);
            host.legacy_string_construct_substring_004cdbe0(temp3c, rest, cut, 0xffffffffu);  // state 9
            host.legacy_string_assign_substring_00408120(temp3c, rest, 0, 0xffffffffu);
            host.legacy_string_destroy_004072d0(rest);  // state 8
        }
    }

    // 004E2638..004E2666: game+2198h = "command_line_mission" (state 0Ah).
    NativeStringStorage string10_storage;
    Raw string10 = raw(string10_storage.bytes);
    host.native_string_construct_0041e870(string10, "command_line_mission");
    host.game_assign_console_string_2198_004c89a0(game, string10);
    host.native_string_destroy_storage_0041dd20(string10);  // state 8

    // 004E266B..004E2694: the scene record and its selection.
    host.game_build_scene_record_004e1d70(game, legacy_c_str(temp3c), 0, -1, 0);
    host.game_select_scene_record_004c6890(game, static_cast<std::int32_t>(game.at<std::uint32_t>(0x5f0) - 1));

    // 004E2699..004E2700: with [[0109CECC]+21h] set, the newest storage slot
    // name; when it differs (case-insensitively) from the empty save name, the
    // player profile at [00E188A8]+650h is asked to read it with the empty
    // continuation 00BD53C0.
    void* const storage_manager = host.global_pointer(0x0109cecc);
    if (raw(storage_manager).at<std::uint8_t>(0x21) != 0) {
        host.storage_manager_get_newest_slot_name_004c5e00(storage_manager, string10);  // state 0Bh
        NativeStringStorage empty_storage;  // ESP+58h
        Raw empty = raw(empty_storage.bytes);
        host.profile_manager_construct_empty_save_name_00436710(empty);
        const bool differs = host.native_string_not_equal_case_insensitive_00449af0(string10, empty);
        host.native_string_destroy_storage_0041dd20(empty);
        if (differs) {
            host.player_profile_request_read_007ff100(raw(raw(host.global_pointer(kGame)).field(0x650)), string10,
                                                      0x00bd53c0u);
        }
        host.native_string_destroy_storage_0041dd20(string10);  // state 8
    }

    // 004E2705..004E2718: request 0Ah.
    request = 0x0a;
    host.game_enqueue_state_request_004d3ed0(raw(game.field(0x5d8)), &request);
    finish_frame();
}

// ---------------------------------------------------------------------------
// 004E3AA0 BSP_Game_OnInit ("GGame::OnInit", 00CE823C)
// __thiscall(ECX = game), plain RET at 004E3FF3; EH state byte [ESP+50h].
// Frame: native string ESP+14h (the block name), the 1Ch FileBlock ESP+2Ch,
// native string ESP+0Ch (each folder / path in turn, and the new-result slot),
// the checked string vector ESP+1Ch (proxy +0, first +4, last +8, end +0Ch).
// Progress fractions (floats): 0.12 00CE81A8, 0.14 00CE81A4, 0.15 00CE7818,
// 0.16 00CE81A0, 0.17 00CE819C, 0.18 00CE8198, 0.22 00CE3D38, 0.24 00CE8194,
// 0.26 00CE8190, 0.28 00CE77BC, 0.30 00CE69C8, 0.32 00CE818C.
// No null guard follows any operator new: a failed allocation stores 0 and the
// image then dereferences it for the vtable call (kept: the hook receives null).
// ---------------------------------------------------------------------------
namespace {

// The inlined release of an 8-byte string {length +0, data +4}: with data
// set, 00BD1510(00419CC0(), data, length + 1, 1).
void release_native_string(Raw string, SweepA4Host& host) {
    if (void* const data = string.ptr(4); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      string.at<std::uint32_t>(0) + 1, 1);
    }
}

}  // namespace

void game_on_init_004e3aa0(Raw game, SweepA4Host& host) {
    game.at<std::uint32_t>(0x5d4) = 3;  // 004E3AC2
    // 004E3AC8..004E3ADC: delete [00E198BC] (slot 0, flag 1) and clear it.
    if (void* const old = host.global_pointer(0x00e198bc); old != nullptr) {
        host.vcall_scalar_deleting_destructor_slot0(old, 1);
        *static_cast<void**>(host.global_address(0x00e198bc)) = nullptr;
    }
    // 004E3AE2: once only.
    if (game.at<std::uint8_t>(0x719c) != 0) return;
    game.at<std::uint8_t>(0x719c) = 1;

    // 004E3AEF..004E3B37: the profiling FileBlock "GGame::OnInit" (state 2
    // until 004E3FD5); its name string is released right away.
    NativeStringStorage name_storage;
    Raw name = raw(name_storage.bytes);
    host.native_string_construct_0041e870(name, "GGame::OnInit");
    alignas(4) std::uint8_t block_storage[0x1c];
    Raw block = raw(block_storage);
    host.file_block_construct_00be0a30(block, name, 1);
    release_native_string(name, host);

    // 004E3B3C..004E3C99: the effect texture atlases.
    NativeStringStorage path_storage;
    Raw path = raw(path_storage.bytes);
    for (const char* const folder :
         {"Particles\\Textures\\", "Effects\\Flares\\Textures\\", "Effects\\Traceline\\", "Effects\\Numbers\\"}) {
        host.native_string_construct_0041e870(path, folder);
        host.texture_atlas_load_directory_00aeff40(host.global_pointer(0x00f8c26c), path);
        release_native_string(path, host);
    }
    host.native_string_construct_0041e870(path, "interface/textures/common.ats");
    host.texture_atlas_load_00af0060(host.global_pointer(0x00f8c26c), path);
    release_native_string(path, host);

    host.game_create_enum_registry_and_universe_library_004d3040(game);  // 004E3CA0

    // 004E3CA5..004E3CFF: a one-entry string vector {"interface/_Common.lua"}
    // handed to 00B6D3C0 (state 8 while it lives).
    alignas(4) std::uint8_t vector_storage[0x10] = {};
    Raw scripts = raw(vector_storage);
    scripts.ptr(0x04) = nullptr;
    scripts.ptr(0x08) = nullptr;
    scripts.ptr(0x0c) = nullptr;
    host.native_string_construct_0041e870(path, "interface/_Common.lua");
    host.native_string_vector_append_00450540(scripts, path);
    release_native_string(path, host);
    host.lua_globals_reset_unless_loaded_00b6d3c0(scripts);

    // 004E3D04..004E3EF6: the managers, each new + constructor (EAX = 0 when
    // new fails) stored before its init call.
    void* object = host.operator_new_00bf681b(0x84);
    object = object != nullptr ? host.frame_stats_screen_construct_0053ba10(object) : nullptr;
    game.ptr(0x19c8) = object;
    host.vcall_manager_init_10(object);
    host.loading_screen_report_progress_0057bec0(0.12f);

    object = host.operator_new_00bf681b(0x70);
    object = object != nullptr ? host.manager_e18db0_construct_0051f990(object) : nullptr;
    *static_cast<void**>(host.global_address(0x00e18db0)) = object;
    host.vcall_manager_init_10(object);

    object = host.operator_new_00bf681b(0x20);
    object = object != nullptr ? host.manager_e19900_construct_006b9450(object) : nullptr;
    *static_cast<void**>(host.global_address(0x00e19900)) = object;  // 004E3DAF, no init call
    host.loading_screen_report_progress_0057bec0(0.14f);

    object = host.operator_new_00bf681b(0x14);
    object = object != nullptr ? host.manager_e19698_construct_0060d290(object) : nullptr;
    *static_cast<void**>(host.global_address(0x00e19698)) = object;
    host.vcall_manager_init_10(object);
    host.loading_screen_report_progress_0057bec0(0.15f);
    host.loading_screen_report_progress_0057bec0(0.16f);

    object = host.operator_new_00bf681b(0xac);
    object = object != nullptr ? host.manager_e1930c_construct_0054a8d0(object) : nullptr;
    *static_cast<void**>(host.global_address(0x00e1930c)) = object;
    host.vcall_manager_init_10(object);
    host.loading_screen_report_progress_0057bec0(0.17f);
    host.loading_screen_report_progress_0057bec0(0.18f);

    host.scene_database_create_singleton_0046f350();  // 004E3E60
    host.loading_screen_report_progress_0057bec0(0.22f);

    object = host.operator_new_00bf681b(0x14b8);
    object = object != nullptr ? host.manager_21a0_construct_0091d640(object) : nullptr;
    game.ptr(0x21a0) = object;
    host.manager_21a0_init_0091b2e0(object);
    host.loading_screen_report_progress_0057bec0(0.24f);

    object = host.operator_new_00bf681b(0x10);
    object = object != nullptr ? host.manager_21d8_construct_006afe80(object) : nullptr;
    game.ptr(0x21d8) = object;
    host.manager_21d8_init_006ad180(object);
    host.loading_screen_report_progress_0057bec0(0.26f);

    // 004E3EFB..004E3F31.
    host.award_tracker_reload_hint_tables_006921b0(host.award_tracker_get_singleton_004e1ca0());
    if (host.global_pointer(0x00f8a2fc) == nullptr) {
        host.online_system_create_00992a50();
        host.vcall_attach_game_4(host.global_pointer(0x00f8a2fc), game.p);
    }
    host.entity_id_table_reset_00951560(host.global_address(0x00f89a08));
    host.entity_id_table_reset_00951560(host.global_address(0x00f89a5c));
    host.loading_screen_report_progress_0057bec0(0.28f);
    host.mission_lua_run_global_script_folders_00886900(game.ptr(0x1a08));
    host.loading_screen_report_progress_0057bec0(0.30f);
    host.game_tuning_get_singleton_0042e740();
    host.loading_screen_report_progress_0057bec0(0.32f);
    host.game_settings_get_singleton_00424c40();
    host.game_singleton_00f8a71c_get_or_create_004d6ce0();
    host.soldier_globals_get_singleton_00485520();
    host.game_singleton_00e19a64_get_or_create_004c1d10();
    host.plane_part_classes_load_globals_004a9bd0();
    host.ship_explosion_load_globals_004af290();
    host.game_apply_command_line_options_004e27e0(game);  // 004E3F93

    // 004E3F98..004E3FDD: the vector's strings and block (state 2), the
    // vector cleared, then the FileBlock destroyed (state -1).
    if (scripts.ptr(0x04) != nullptr) {
        host.native_string_range_destroy_00432050(scripts.ptr(0x04), scripts.ptr(0x08));
        host.free_00bf65ac(scripts.ptr(0x04));
    }
    scripts.ptr(0x04) = nullptr;
    scripts.ptr(0x08) = nullptr;
    scripts.ptr(0x0c) = nullptr;
    host.file_block_destroy_00bdcb30(block);
}

// ---------------------------------------------------------------------------
// 004EA080 BSP_SceneClass_ReadStockLists
// __thiscall(ECX = scene property bag), plain RET at 004EA64E; EBP-aligned
// frame, EH state [ESP+0DCh]. No direct caller: class-table row 4Dh
// ("SpawnPoint") holds it as its reader (src/scene_entity_factory.cpp:57).
// Literals: "JapanList" 00CE56D0, "AlliedList" 00CE56C4, "Stock %d" 00CE56DC,
// "Count" 00CE5710, "Type" 00CE4780, "VehicleClass." 00CE9248,
// ".Catapult.LaunchedClass" 00CE9258. 00BF7680 is the CRT memcpy, bound
// directly. A property is used as a sub-bag only when its +4h kind is 6; its
// value is at +0Ch.
// The two loops are the same code with the frame slots permuted (JapanList
// strings at ESP+14h/48h/2Ch/38h/50h, AlliedList at 20h/58h/14h/40h/2Ch) and
// party 1 (004EA14F) versus 0 (004EA401).
// Quirk kept: the AlliedList loop starts at the index the JapanList loop
// stopped on (EDI and [ESP+28h] carry over at 004EA39F), not at 1.
// ---------------------------------------------------------------------------
namespace {

// The inlined construction of a native string from a literal of `length`
// characters: {0, 0}, 0041DD40(length, 1), then memcpy(data, text, length
// re-read + 1) when data is set.
void construct_native_literal(Raw string, const char* text, std::uint32_t length, SweepA4Host& host) {
    string.at<std::uint32_t>(0) = 0;
    string.ptr(4) = nullptr;
    host.native_string_resize_0041dd40(string, length, 1);
    if (void* const data = string.ptr(4); data != nullptr) std::memcpy(data, text, string.at<std::uint32_t>(0) + 1);
}

// One "Stock %d" loop; returns the index it stopped on.
std::int32_t read_stock_list(void* list_bag, std::int32_t index, std::int32_t party, SweepA4Host& host) {
    char key[0x20];  // ESP+60h
    for (;; ++index) {
        host.format_to_buffer32_00467310(key, "Stock %d", index);
        void* const entry = host.scene_property_bag_find_008f2260(list_bag, key);
        if (entry == nullptr || raw(entry).at<std::int32_t>(0x04) != 6) return index;
        void* const stock = raw(entry).ptr(0x0c);
        if (stock == nullptr) return index;
        // No null test on either property (004EA137 / 004EA3E9, 004EA14C / 004EA3FE).
        if (raw(host.scene_property_bag_find_008f2260(stock, "Count")).at<std::uint32_t>(0x0c) == 0) continue;
        const std::int32_t type = raw(host.scene_property_bag_find_008f2260(stock, "Type")).at<std::int32_t>(0x0c);
        host.vehicle_class_mark_party_required_0095ba60(type, party);
        host.scene_entity_register_hidden_stock_unit_0046be90(host.global_pointer(0x00e18680), type);

        // "VehicleClass." + type + ".Catapult.LaunchedClass" read from the
        // Lua globals as an integer, default -1.
        NativeStringStorage suffix_storage, number_storage, prefix_storage, left_storage, path_storage;
        Raw suffix = raw(suffix_storage.bytes);
        Raw number = raw(number_storage.bytes);
        Raw prefix = raw(prefix_storage.bytes);
        Raw left = raw(left_storage.bytes);
        Raw path = raw(path_storage.bytes);
        construct_native_literal(suffix, ".Catapult.LaunchedClass", 0x17, host);
        host.native_string_from_int_004260b0(number, type);
        construct_native_literal(prefix, "VehicleClass.", 0x0d, host);
        host.native_string_concat_004261a0(prefix, left, number);
        host.native_string_concat_004261a0(left, path, suffix);
        cc13_sweepA::LuaObjectStorage globals;
        cc13_sweepA::LuaObjectStorage value;
        bool found = false;  // [ESP+36h] / [ESP+37h], not read afterwards
        cc13_sweepA::LuaObjectStorage* const resolved = host.lua_object_resolve_dot_path_00b68d70(
            host.lua_state_owner_get_globals_00b67980(raw(host.global_pointer(kGame)).field(0x1a0c), &globals), &value, path, &found);
        const std::int32_t launched = host.lua_object_get_integer_or_default_00b66380(resolved, -1);
        host.lua_object_destruct_00b67700(&value);
        host.lua_object_destruct_00b67700(&globals);
        release_native_string(path, host);
        release_native_string(left, host);
        release_native_string(prefix, host);
        release_native_string(number, host);
        release_native_string(suffix, host);
        // 004EA350 / 004EA604: CMP -1, JLE skips.
        if (launched > -1) {
            host.vehicle_class_mark_party_required_0095ba60(launched, party);
            host.scene_entity_register_hidden_stock_unit_0046be90(host.global_pointer(0x00e18680), launched);
        }
    }
}

// A sub-bag property (kind 6) or null.
void* find_sub_bag(void* bag, const char* key, SweepA4Host& host) {
    void* const property = host.scene_property_bag_find_008f2260(bag, key);
    if (property != nullptr && raw(property).at<std::int32_t>(0x04) == 6) return raw(property).ptr(0x0c);
    return nullptr;
}

}  // namespace

void scene_class_read_stock_lists_004ea080(Raw bag, SweepA4Host& host) {
    if (host.game_get_effective_game_mode_004bca50(raw(host.global_pointer(kGame))) == 4) return;  // 004EA0B9
    const std::int32_t stopped = read_stock_list(find_sub_bag(bag.p, "JapanList", host), 1, 1, host);
    read_stock_list(find_sub_bag(bag.p, "AlliedList", host), stopped, 0, host);
}

// ---------------------------------------------------------------------------
// 004507D0 BSP_PanelSequence_ReadProperties
// __thiscall(ECX = panel sequence, archive [ESP+40h at entry+4]), RET 4 at
// 00450D2F; EH state [ESP+3Ch] covers only the panel-slot temporary. Callers
// 00451A90 BSP_PanelOwner_StartDialogFromLua, 00452130 BSP_PanelOwner_ReadSaveState.
// The read-side twin of 0044B5B0 (cc13_sweepA3). Keys are {0, the image's
// literal}: "priority" 00CE4A84, "requestTime" 00CE4A78, "suppressInterruptMsg"
// 00CE4A60, "defaultPause" 00CE4A50, "nextUnit" 00CE4A44, "sequence" 00CE4A38,
// "breakRequested" 00CE4A28, "lastPanel" 00CE4A1C, "panelStates" 00CE4A10,
// "active" 00CE4A08, "name" 00CE49B8, "characterID" 00CE49FC.
// Sequence layout: +4h priority, +8h requestTime, +0Ch defaultPause (floats),
// +10h suppressInterruptMsg (byte), +14h checked vector of command pointers
// (first +18h, last +1Ch, end +20h), +24h nextUnit, +28h checked vector of
// 14h panel slots (first +2Ch, last +30h), +38h breakRequested (byte), +3Ch
// lastPanel.
// Uncertainty: a tag-3 default is written with MOV byte [EAX+4] only, so its
// upper three bytes are whatever the reused stack slot held; zero here.
// ---------------------------------------------------------------------------
namespace {

using cc13_sweepA3::TaggedValue;

std::uint32_t pointer_bits(const void* pointer) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

TaggedValue image_key(std::uint32_t literal, SweepA4Host& host) {
    return TaggedValue{0, pointer_bits(host.global_address(literal))};
}

}  // namespace

void panel_sequence_read_properties_004507d0(Raw sequence, void* archive, SweepA4Host& host) {
    // 004507E8..004509A3: the scalar properties.
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a84, host), TaggedValue{2, pointer_bits(sequence.field(0x04))},
                                      TaggedValue{2, 0});  // XORPS 0.0f
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a78, host), TaggedValue{2, pointer_bits(sequence.field(0x08))},
                                      TaggedValue{2, *static_cast<const std::uint32_t*>(host.global_address(0x00f876a4))});
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a60, host), TaggedValue{3, pointer_bits(sequence.field(0x10))},
                                      TaggedValue{3, 0});
    // 004508C9: the default is [[00E188A8]+21E4h]+30h.
    const std::uint32_t pause_default =
        raw(raw(host.global_pointer(kGame)).ptr(0x21e4)).at<std::uint32_t>(0x30);
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a50, host), TaggedValue{2, pointer_bits(sequence.field(0x0c))},
                                      TaggedValue{2, pause_default});
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a44, host), TaggedValue{1, pointer_bits(sequence.field(0x24))},
                                      TaggedValue{1, 0});

    // 00450960..00450A14: group "sequence"; its first command is a new 18h
    // object (vtable 00CE49DC, +8h..+14h zero) whose +4h is cleared without a
    // null test, given the front name of the list at [[00E188A8]+21E4h]+4h
    // (node +0Ch, an 8-byte string; the inlined assign), then push_back.
    host.vcall_sink_begin_4(archive, image_key(0x00ce4a38, host));
    void* command = host.operator_new_00bf681b(0x18);
    if (command != nullptr) {
        Raw c = raw(command);
        c.at<std::uint32_t>(0x00) = 0x00ce49dc;
        c.at<std::uint32_t>(0x08) = 0;
        c.at<std::uint32_t>(0x0c) = 0;
        c.at<std::uint32_t>(0x10) = 0;
        c.at<std::uint32_t>(0x14) = 0;
    }
    raw(command).at<std::uint32_t>(0x04) = 0;
    {
        Raw names = raw(raw(raw(host.global_pointer(kGame)).ptr(0x21e4)).field(0x04));
        void* const head = names.ptr(0x04);
        void* node = raw(head).ptr(0x00);
        if (node == head) host.invalid_parameter_noinfo_00bf6713();
        Raw source = raw(raw(node).field(0x0c));
        Raw target = raw(raw(command).field(0x10));
        if (target.p != source.p) {
            host.native_string_resize_0041dd40(target, source.at<std::uint32_t>(0), 1);
            if (source.at<std::uint32_t>(0) != 0)
                std::memcpy(target.ptr(4), source.ptr(4), target.at<std::uint32_t>(0));
        }
    }
    Raw commands = raw(sequence.field(0x14));
    host.command_vector_push_back_0044eb90(commands, &command);

    // 00450A19..00450AEC: elements 1.. while the archive has them; each is
    // created by 0044BE50 and appended (inlined push_back when there is spare
    // capacity, else 0044E6E0 insert at end after the checked-iterator test).
    for (std::uint32_t index = 1; host.vcall_archive_has_14(archive, TaggedValue{1, index}); ++index) {
        host.vcall_sink_begin_4(archive, TaggedValue{1, index});
        void* created = host.dialog_command_create_from_archive_0044be50(archive);
        const std::uintptr_t first = reinterpret_cast<std::uintptr_t>(commands.ptr(0x04));
        const std::uintptr_t size = first == 0 ? 0 : (reinterpret_cast<std::uintptr_t>(commands.ptr(0x08)) - first) >> 2;
        if (first != 0 && size < ((reinterpret_cast<std::uintptr_t>(commands.ptr(0x0c)) - first) >> 2)) {
            *static_cast<void**>(commands.ptr(0x08)) = created;
            commands.ptr(0x08) = static_cast<std::uint8_t*>(commands.ptr(0x08)) + 4;
        } else {
            void* const last = commands.ptr(0x08);
            if (first > reinterpret_cast<std::uintptr_t>(last)) host.invalid_parameter_noinfo_00bf6713();
            void* out_iterator[2];
            host.command_vector_insert_0044e6e0(commands, out_iterator, commands, last, &created);
        }
        host.vcall_sink_end_8(archive);
    }
    host.vcall_sink_end_8(archive);  // 00450AF1

    // 00450AFA..00450B7C.
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a28, host), TaggedValue{3, pointer_bits(sequence.field(0x38))},
                                      TaggedValue{3, 0});
    host.vcall_archive_read_default_c(archive, image_key(0x00ce4a1c, host), TaggedValue{1, pointer_bits(sequence.field(0x3c))},
                                      TaggedValue{1, 0});

    // 00450B7E..00450D1B: group "panelStates" only when present; record i
    // from 0 while present: push_back a cleared 14h slot (byte +0, two 8-byte
    // strings +4h / +0Ch), destroy the temporary, then read into back().
    if (!host.vcall_archive_has_14(archive, image_key(0x00ce4a10, host))) return;  // no close
    host.vcall_sink_begin_4(archive, image_key(0x00ce4a10, host));
    Raw slots = raw(sequence.field(0x28));
    for (std::uint32_t index = 0;; ++index) {
        if (!host.vcall_archive_has_14(archive, TaggedValue{1, index})) break;
        host.vcall_sink_begin_4(archive, TaggedValue{1, index});
        alignas(4) std::uint8_t slot_storage[0x14];
        Raw slot = raw(slot_storage);
        slot.at<std::uint32_t>(0x04) = 0;
        slot.at<std::uint32_t>(0x08) = 0;
        slot.at<std::uint32_t>(0x0c) = 0;
        slot.at<std::uint32_t>(0x10) = 0;
        slot.at<std::uint8_t>(0x00) = 0;
        host.panel_slot_vector_push_back_00450600(slots, slot);  // EH state 0
        host.panel_sequence_slot_destroy_0044b530(slot);         // state -1
        // 00450C36..00450C75: the checked back() tests.
        const std::uintptr_t first = reinterpret_cast<std::uintptr_t>(slots.ptr(0x04));
        const std::uintptr_t last = reinterpret_cast<std::uintptr_t>(slots.ptr(0x08));
        if (first > last) host.invalid_parameter_noinfo_00bf6713();
        const std::uintptr_t back = last - 0x14;
        if (back > reinterpret_cast<std::uintptr_t>(slots.ptr(0x08)) ||
            back < reinterpret_cast<std::uintptr_t>(slots.ptr(0x04)))
            host.invalid_parameter_noinfo_00bf6713();
        if (back >= reinterpret_cast<std::uintptr_t>(slots.ptr(0x08))) host.invalid_parameter_noinfo_00bf6713();
        host.vcall_archive_read_10(archive, image_key(0x00ce4a08, host), TaggedValue{3, static_cast<std::uint32_t>(back)});
        host.vcall_archive_read_10(archive, image_key(0x00ce49b8, host), TaggedValue{0, static_cast<std::uint32_t>(back + 4)});
        host.vcall_archive_read_10(archive, image_key(0x00ce49fc, host),
                                   TaggedValue{0, static_cast<std::uint32_t>(back + 0x0c)});
        host.vcall_sink_end_8(archive);
    }
    host.vcall_sink_end_8(archive);  // 00450D16
}

// ---------------------------------------------------------------------------
// 0045DE50 BSP_MovieAction_Create
// __stdcall(type [ESP+4], float activate time [ESP+8], comment = 8-byte
// native string by value {length [ESP+0Ch], data [ESP+10h]}), RET 10h at
// 0045E552, EAX = the action. Sole caller 0045E5E0. EH state [ESP+18h]: 0
// covers the by-value comment, 1..1Fh the new block of each case.
// The switch (CMP 1Fh / JA, jump table 0045E558) was decoded from the bytes
// (local/sweepA4_jt.py): every case is operator new(size), and when non-null
// the constructor __thiscall(block, float time); a null block gives a null
// action (0045DEB4). Index 4 shares the out-of-range target 0045E4E6, which
// takes the action pointer from the activate-time argument slot ([ESP+24h],
// the float's bits) and goes on to write its +8h string through it; kept.
// Tail 0045E4EA..0045E539: the inlined assign of the comment to action+8h
// (skipped only when action+8h is the argument itself), then the inlined
// release of the argument's storage.
// ---------------------------------------------------------------------------
namespace {

struct MovieActionFactoryCase {
    std::uint32_t size;         // operator new size
    std::uint32_t constructor;  // 0 = no case (index 4)
};

// Index -> case, read from the jump table targets.
constexpr MovieActionFactoryCase kMovieActionFactory[32] = {
    {0x44, 0x00465c40},  // 0 SetTarget
    {0x44, 0x00465c70},  // 1 SetParent
    {0x44, 0x00465cd0},  // 2 EngineFailure
    {0x44, 0x00465db0},  // 3 ForceRelease
    {0, 0},              // 4 no case
    {0x44, 0x00465ca0},  // 5 UnParent
    {0x44, 0x00465de0},  // 6 ForceDepthCharge
    {0x44, 0x00465e10},  // 7 SetGunFire
    {0x44, 0x00465e40},  // 8 TogglePeriscope
    {0x44, 0x00465e70},  // 9 ForceArtilleryAngle
    {0x44, 0x00465ea0},  // 10 ForceArtilleryTarget
    {0x44, 0x00465ed0},  // 11 ForceTorpedo
    {0x44, 0x00465f00},  // 12 SetGunFireEnabled
    {0x44, 0x00465f30},  // 13 SetGunFireDisabled
    {0x44, 0x00465f60},  // 14 SetGunFireTargetOnly
    {0x44, 0x00465f90},  // 15 TurnGun
    {0x44, 0x00465fc0},  // 16 SetDepth
    {0x44, 0x00465ff0},  // 17 ForceAAFlak
    {0x44, 0x00466020},  // 18 FOV
    {0x44, 0x00466080},  // 19 BlackOut
    {0x48, 0x00466170},  // 20 ParticleEffect
    {0x44, 0x004661c0},  // 21 DontMove
    {0x44, 0x00466050},  // 22 FOVNow
    {0x44, 0x004661f0},  // 23 SetGear
    {0x44, 0x00466220},  // 24 CameraShake
    {0x44, 0x00466250},  // 25 Binocular
    {0x44, 0x00466280},  // 26 TakeOffHack
    {0x44, 0x004662b0},  // 27 WingTipEffect
    {0x44, 0x004662e0},  // 28 MoveToPoint
    {0x44, 0x00466310},  // 29 WideBlackBars
    {0x44, 0x00466340},  // 30 UseCatapult
    {0x44, 0x00466370},  // 31 HideEntity
};

}  // namespace

void* movie_action_create_0045de50(std::int32_t type, float activate_time, std::uint32_t comment_length,
                                   void* comment_data, SweepA4Host& host) {
    void* action;
    // CMP EAX,1Fh / JA: unsigned, so a negative type is out of range.
    const std::uint32_t index = static_cast<std::uint32_t>(type);
    if (index > 0x1f || kMovieActionFactory[index].constructor == 0) {
        std::uint32_t bits;
        std::memcpy(&bits, &activate_time, sizeof bits);
        action = reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));  // 0045E4E6
    } else {
        void* const block = host.operator_new_00bf681b(kMovieActionFactory[index].size);
        action = block != nullptr
                     ? host.movie_action_type_construct(kMovieActionFactory[index].constructor, block, activate_time)
                     : nullptr;
    }
    // 0045E4EA: assign the comment (the argument slot is the source; its
    // address cannot equal action+8h here, the comparison the image makes).
    std::uint32_t argument[2] = {comment_length, pointer_bits(comment_data)};
    Raw target = raw(static_cast<std::uint8_t*>(action) + 8);
    if (target.p != reinterpret_cast<std::uint8_t*>(argument)) {
        host.native_string_resize_0041dd40(target, comment_length, 1);
        if (comment_length != 0) std::memcpy(target.ptr(4), comment_data, target.at<std::uint32_t>(0));
    }
    // 0045E51B: release the by-value argument (state -1).
    if (comment_data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), comment_data,
                                                      comment_length + 1, 1);
    }
    return action;
}

// ---------------------------------------------------------------------------
// 004E8BF0 BSP_SceneHeader_Assign
// __thiscall(ECX = header, const header* [ESP+14h after four pushes]), RET 4
// at 004E944B, EAX = header. Sole caller 004F1D70.
// A member-wise copy in the image's own order, read from the listing with
// local/sweepA4_copyscan.py: 'f' fields go through FLD/FSTP float, 'd' dwords
// and 'b' bytes through a register, and 'S' 8-byte native strings are the
// inlined assign (skipped when the two strings are the same object; else
// 0041DD40(source length, 1) and, when the source length re-read is not 0,
// memcpy(data, source data, the new length)).
// Simplification: the 'f' copies are float assignments here; the image's
// FLD/FSTP pair would quiet a signalling NaN, which this does not model.
// ---------------------------------------------------------------------------
namespace {

enum class CopyKind : std::uint8_t { F, D, B, S };

struct CopyStep {
    CopyKind kind;
    std::uint16_t offset;
};

constexpr CopyStep kSceneHeaderCopy[] = {
    {CopyKind::F, 0x00}, {CopyKind::F, 0x04}, {CopyKind::F, 0x08}, {CopyKind::B, 0x0c}, {CopyKind::F, 0x10},
    {CopyKind::F, 0x14}, {CopyKind::F, 0x18}, {CopyKind::S, 0x1c}, {CopyKind::S, 0x24}, {CopyKind::F, 0x2c},
    {CopyKind::F, 0x30}, {CopyKind::F, 0x34}, {CopyKind::F, 0x38}, {CopyKind::F, 0x3c}, {CopyKind::F, 0x40},
    {CopyKind::S, 0x44}, {CopyKind::F, 0x4c}, {CopyKind::F, 0x50}, {CopyKind::F, 0x54}, {CopyKind::F, 0x58},
    {CopyKind::F, 0x5c}, {CopyKind::B, 0x60}, {CopyKind::D, 0x64}, {CopyKind::F, 0x68}, {CopyKind::F, 0x6c},
    {CopyKind::F, 0x70}, {CopyKind::D, 0x74}, {CopyKind::F, 0x78}, {CopyKind::F, 0x7c}, {CopyKind::D, 0x80},
    {CopyKind::D, 0x84}, {CopyKind::D, 0x88}, {CopyKind::D, 0x8c}, {CopyKind::D, 0x90}, {CopyKind::D, 0x94},
    {CopyKind::D, 0x98}, {CopyKind::D, 0x9c}, {CopyKind::D, 0xa0}, {CopyKind::D, 0xa4}, {CopyKind::D, 0xa8},
    {CopyKind::D, 0xac}, {CopyKind::D, 0xb0}, {CopyKind::D, 0xb4}, {CopyKind::D, 0xb8}, {CopyKind::D, 0xbc},
    {CopyKind::D, 0xc0}, {CopyKind::D, 0xc4}, {CopyKind::D, 0xc8}, {CopyKind::D, 0xcc}, {CopyKind::F, 0xd0},
    {CopyKind::F, 0xd4}, {CopyKind::F, 0xd8}, {CopyKind::F, 0xdc}, {CopyKind::F, 0xe0}, {CopyKind::F, 0xe4},
    {CopyKind::F, 0xe8}, {CopyKind::F, 0xec}, {CopyKind::F, 0xf0}, {CopyKind::F, 0xf4}, {CopyKind::F, 0xf8},
    {CopyKind::F, 0xfc}, {CopyKind::B, 0x100}, {CopyKind::F, 0x104}, {CopyKind::F, 0x108}, {CopyKind::F, 0x10c},
    {CopyKind::F, 0x110}, {CopyKind::F, 0x114}, {CopyKind::D, 0x118}, {CopyKind::D, 0x11c}, {CopyKind::D, 0x120},
    {CopyKind::D, 0x124}, {CopyKind::F, 0x128}, {CopyKind::D, 0x12c}, {CopyKind::D, 0x130}, {CopyKind::D, 0x134},
    {CopyKind::D, 0x138}, {CopyKind::S, 0x13c}, {CopyKind::F, 0x144}, {CopyKind::F, 0x148}, {CopyKind::F, 0x14c},
    {CopyKind::F, 0x150}, {CopyKind::D, 0x154}, {CopyKind::D, 0x158}, {CopyKind::D, 0x15c}, {CopyKind::D, 0x160},
    {CopyKind::D, 0x164}, {CopyKind::D, 0x168}, {CopyKind::D, 0x16c}, {CopyKind::D, 0x170}, {CopyKind::D, 0x174},
    {CopyKind::D, 0x178}, {CopyKind::D, 0x17c}, {CopyKind::D, 0x180}, {CopyKind::D, 0x184}, {CopyKind::D, 0x188},
    {CopyKind::D, 0x18c}, {CopyKind::D, 0x190}, {CopyKind::F, 0x194}, {CopyKind::F, 0x198}, {CopyKind::F, 0x19c},
    {CopyKind::D, 0x1a0}, {CopyKind::D, 0x1a4}, {CopyKind::D, 0x1a8}, {CopyKind::D, 0x1ac}, {CopyKind::F, 0x1b0},
    {CopyKind::F, 0x1b4}, {CopyKind::F, 0x1b8}, {CopyKind::F, 0x1bc}, {CopyKind::F, 0x1c0}, {CopyKind::F, 0x1c4},
    {CopyKind::F, 0x1c8}, {CopyKind::F, 0x1cc}, {CopyKind::F, 0x1d0}, {CopyKind::F, 0x1d4}, {CopyKind::F, 0x1d8},
    {CopyKind::F, 0x1dc}, {CopyKind::F, 0x1e0}, {CopyKind::D, 0x1e4}, {CopyKind::D, 0x1e8}, {CopyKind::D, 0x1ec},
    {CopyKind::D, 0x1f0}, {CopyKind::F, 0x1f4}, {CopyKind::F, 0x1f8}, {CopyKind::F, 0x1fc}, {CopyKind::F, 0x200},
    {CopyKind::F, 0x204}, {CopyKind::S, 0x208}, {CopyKind::S, 0x210}, {CopyKind::S, 0x218}, {CopyKind::F, 0x220},
    {CopyKind::F, 0x224}, {CopyKind::F, 0x228}, {CopyKind::F, 0x22c}, {CopyKind::F, 0x230}, {CopyKind::S, 0x234},
    {CopyKind::S, 0x23c}, {CopyKind::S, 0x244}, {CopyKind::S, 0x24c}, {CopyKind::S, 0x254}, {CopyKind::B, 0x25c},
    {CopyKind::F, 0x260}, {CopyKind::F, 0x264}, {CopyKind::F, 0x268}, {CopyKind::F, 0x26c}, {CopyKind::F, 0x270},
    {CopyKind::F, 0x274}, {CopyKind::F, 0x278}, {CopyKind::F, 0x27c}, {CopyKind::S, 0x280},
};

}  // namespace

Raw scene_header_assign_004e8bf0(Raw header, Raw source, SweepA4Host& host) {
    for (const CopyStep& step : kSceneHeaderCopy) {
        switch (step.kind) {
        case CopyKind::F: header.at<float>(step.offset) = source.at<float>(step.offset); break;
        case CopyKind::D: header.at<std::uint32_t>(step.offset) = source.at<std::uint32_t>(step.offset); break;
        case CopyKind::B: header.at<std::uint8_t>(step.offset) = source.at<std::uint8_t>(step.offset); break;
        case CopyKind::S: {
            Raw target = raw(header.field(step.offset));
            Raw from = raw(source.field(step.offset));
            if (target.p == from.p) break;
            host.native_string_resize_0041dd40(target, from.at<std::uint32_t>(0), 1);
            if (from.at<std::uint32_t>(0) != 0) std::memcpy(target.ptr(4), from.ptr(4), target.at<std::uint32_t>(0));
            break;
        }
        }
    }
    return header;
}

// ---------------------------------------------------------------------------
// 00459020 BSP_MovieEntityTrack_Update
// __thiscall(ECX = entity track, float time [ESP+7Ch]), RET 4 at 004598EE;
// vtable 00CE4DB8 slot +0Ch, caller 0045BD90. No EH frame.
// Track layout used here: +4h..+0F3h a 20-entry ring of direction vectors,
// +0F4h the point list (head +0F8h, count +0FCh), +100h a checked iterator
// member {container, node}, +11Ch the entity, +148h roll (radians), +14Ch
// the mode (2 = follow the parent track), +154h the parent track,
// +1C8h..+1D0h the follow offset.
// Constants (doubles unless noted): 0.1 00D7A3A0, 0.099 00CE4D80, 21.0
// 00CE4D78, 180.0 00CE3D20, pi (the float widened) 00CE3D28, 0.0 00D7A258;
// floats 1.0 00D7A24C, -1.0 00D7A260. Each is the float value widened, which
// is how the static_cast<double>(...f) literals below reproduce them.
// x87 forms from the bytes: 004595CE DC F9 FDIV ST1,ST0 (ST1 / ST0), 004595DA
// D8 F1 FDIV ST0,ST1, 004597B4 D8 74 FDIV m32 (ST0 / len). Equality tests are
// LAHF / TEST AH,44h / JP: only an ordered equal falls through.
// Quirks kept: the time argument is overwritten (004590F9..00459153) and the
// new value reaches 004560E0; the follow-mode direction is (1, 0, 1); the
// pitch path divides tan(angle) by the horizontal length (FDIV m32).
// Simplification: FCOS/FSIN and the CRT sqrt (00BF7030) are the C library's
// double functions here, rounded to float as the image stores them.
// ---------------------------------------------------------------------------
void movie_entity_track_update_00459020(Raw track, float time, SweepA4Host& host) {
    // 00459028..00459057.
    if (track.ptr(0x11c) != nullptr && raw(track.ptr(0x11c)).at<std::uint8_t>(0x5e) != 0) track.ptr(0x11c) = nullptr;
    if (track.ptr(0x11c) == nullptr || track.at<std::uint32_t>(0xfc) == 0) {
        host.movie_track_run_due_actions_004560e0(track, time);  // 004598E2
        return;
    }

    float position[3];   // [ESP+34h]
    float direction[3];  // [ESP+28h]; the call takes y from XMM1 (direction_y)
    float direction_y = 0.0f;
    bool follow = false;
    // The parent entity's world translation (+0FCh, +100h, +104h after
    // 00414DB0 when its +0C8h flag is clear) plus the follow offset.
    const auto follow_parent = [&]() {
        Raw parent = raw(raw(track.ptr(0x154)).ptr(0x11c));
        if (parent.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(parent.p);
        position[0] = parent.at<float>(0xfc) + track.at<float>(0x1c8);
        position[1] = parent.at<float>(0x100) + track.at<float>(0x1cc);
        position[2] = parent.at<float>(0x104) + track.at<float>(0x1d0);
        follow = true;
    };

    if (track.at<std::uint32_t>(0x14c) == 2) {
        follow_parent();  // 0045906A..004590D9
    } else {
        // 004590DE..00459153: the member iterator is set to begin(); when
        // time + 0.1 is past the first point's time, time becomes that time
        // minus 0.1.
        Raw list = raw(track.field(0xf4));
        Raw member = raw(track.field(0x100));
        member.ptr(0) = list.p;
        member.ptr(4) = track.ptr(0xf8);
        std::uint32_t previous[2];
        host.list_iterator_post_increment_00454340(member, previous, 0);
        if (member.ptr(0) == nullptr) host.invalid_parameter_noinfo_00bf6713();
        if (member.ptr(4) == raw(member.ptr(0)).ptr(4)) host.invalid_parameter_noinfo_00bf6713();
        const float first_time = raw(member.ptr(4)).at<float>(0x08);
        if (static_cast<double>(time) + static_cast<double>(0.1f) > static_cast<double>(first_time)) {
            time = *static_cast<const float*>(host.list_iterator_deref_004552c0(member));
            time = static_cast<float>(static_cast<double>(time) - static_cast<double>(0.1f));
        }
        float at[3];
        const float* const point = host.movie_track_point_at_time_004581e0(track, at, time, 0);
        position[0] = point[0];
        position[1] = point[1];
        position[2] = point[2];

        if (track.at<std::uint32_t>(0x14c) == 2) {  // 0045916D, re-read
            follow_parent();
        } else {
            // 004591DE..0045931A: the look-at target.
            void* const entity = track.ptr(0x11c);
            float target[3];
            if (host.vcall_entity_is_kind_5c(entity, 6) && raw(entity).at<std::uint8_t>(0x748) != 0) {
                const float end_time = host.movie_track_time_00458160(track);
                alignas(4) std::uint8_t copy_storage[0x28];  // [ESP+40h]
                if (time > end_time) {  // 0045920F FCOMI / JBE
                    member.ptr(0) = list.p;
                    member.ptr(4) = list.ptr(4);
                    const Raw last = host.list_iterator_pre_decrement_00454310(member);
                    host.movie_point_copy_construct_00457ff0(copy_storage, host.list_iterator_deref_004552c0(last));
                } else {
                    std::uint32_t found[2];  // [ESP+14h]
                    host.movie_track_find_point_00455bc0(track, found, time);
                    std::uint32_t step[2] = {found[0], found[1]};  // [ESP+1Ch]
                    host.list_iterator_post_step_004552f0(raw(step), found, 0);
                    host.movie_point_copy_construct_00457ff0(copy_storage, host.list_iterator_deref_004552c0(raw(step)));
                }
                target[0] = raw(copy_storage).at<float>(0x0c);
                target[1] = raw(copy_storage).at<float>(0x10);
                target[2] = raw(copy_storage).at<float>(0x14);
                host.native_string_at4_destroy_storage_00456d10(copy_storage);
            } else {
                // 004592DB: the path 0.099 ahead, flag 1.
                const float ahead = static_cast<float>(static_cast<double>(time) + static_cast<double>(0.099f));
                float at_ahead[3];
                const float* const p = host.movie_track_point_at_time_004581e0(track, at_ahead, ahead, 1);
                target[0] = p[0];
                target[1] = p[1];
                target[2] = p[2];
            }

            // 0045931F..004593C3: direction = target - position (the position
            // itself when all three are exactly 0), normalized.
            direction[0] = target[0] - position[0];
            direction[1] = target[1] - position[1];
            direction[2] = target[2] - position[2];
            if (direction[0] == 0.0f && direction[1] == 0.0f && direction[2] == 0.0f) {
                direction[0] = position[0];
                direction[1] = position[1];
                direction[2] = position[2];
            }
            float normalized_out[3];
            const float* const n = host.vector3_normalize_00419510(normalized_out, direction);
            const float nx = n[0], ny = n[1], nz = n[2];

            // 004593C9..00459424: an all-zero ring entry 0 seeds all 20.
            Raw ring = raw(track.field(0x04));
            const float zero[3] = {0.0f, 0.0f, 0.0f};
            if (host.vector3_exact_equals_00453f40(&ring.at<float>(0), zero)) {
                for (int i = 0; i < 20; ++i) {
                    ring.at<float>(i * 12 + 0) = nx;
                    ring.at<float>(i * 12 + 4) = ny;
                    ring.at<float>(i * 12 + 8) = nz;
                }
            }
            // 00459426..00459592: the ring sum, in order.
            float sx = 0.0f, sy = 0.0f, sz = 0.0f;
            for (int i = 0; i < 20; ++i) {
                sx = ring.at<float>(i * 12 + 0) + sx;
                sy = ring.at<float>(i * 12 + 4) + sy;
                sz = sz + ring.at<float>(i * 12 + 8);
            }
            // 00459598..004595E4: the mean of the 20 and the new direction.
            const float ax = static_cast<float>(static_cast<double>(sx + nx) / 21.0);
            const float ay = static_cast<float>(static_cast<double>(sy + ny) / 21.0);
            const float az = static_cast<float>(static_cast<double>(sz + nz) / 21.0);
            // 004595F0..004596D3: shift the ring down one and append.
            for (int i = 0; i < 19 * 3; ++i) ring.at<float>(i * 4) = ring.at<float>(i * 4 + 12);
            ring.at<float>(0xe4) = nx;  // track+0E8h
            ring.at<float>(0xe8) = ny;
            ring.at<float>(0xec) = nz;
            direction[0] = ax;
            direction[2] = az;
            direction_y = ay;
            // 004596BE..00459727: straight up or down falls back to (1, 0, 1).
            if (ax == 0.0f && az == 0.0f && (ay == -1.0f || ay == 1.0f)) {
                direction[0] = 1.0f;
                direction[2] = 1.0f;
                direction_y = 0.0f;
            }
        }
    }
    if (follow) {  // 00459729
        direction[0] = 1.0f;
        direction[2] = 1.0f;
        direction_y = 0.0f;
    }

    // 00459738..004598A0: unless entity+62h, place the entity.
    Raw entity = raw(track.ptr(0x11c));
    if (entity.at<std::uint8_t>(0x62) == 0) {
        const float pitch_degrees = entity.at<float>(0x64);
        if (!(pitch_degrees == 0.0f)) {  // JNP 004597C5 skips only an ordered 0
            // 0045975C..0045976C: the products and the sum stay in x87 registers.
            const float horizontal_sq = static_cast<float>(static_cast<double>(direction[0]) * direction[0] +
                                                           static_cast<double>(direction[2]) * direction[2]);
            const float horizontal = static_cast<float>(std::sqrt(static_cast<double>(horizontal_sq)));
            const float radians = static_cast<float>(static_cast<double>(pitch_degrees) / 180.0 *
                                                     static_cast<double>(3.14159265f));
            const float angle = static_cast<float>(static_cast<double>(radians) + 0.0);
            direction_y = host.math_tangent_x87_float_00412e20(angle) / horizontal;
            direction[1] = direction_y;  // 004597BB
        }
        const float roll = track.at<float>(0x148);
        const float cosine = static_cast<float>(std::cos(static_cast<double>(roll)));
        const float sine = static_cast<float>(std::sin(static_cast<double>(roll)));
        const float place_direction[3] = {direction[0], direction_y, direction[2]};
        const float roll_time[4] = {sine, cosine, 0.0f, time};
        host.vcall_entity_place_4c(entity.p, position, place_direction, roll_time);
        Raw placed = raw(track.ptr(0x11c));
        void* node = placed.ptr(0x48);
        placed.at<std::uint8_t>(0xc8) = 0;
        placed.at<std::uint8_t>(0x10c) = 0;
        for (; node != nullptr; node = raw(node).ptr(0x44)) host.scene_node_invalidate_subtree_pose_0042ed50(node);
    }

    // 004598A2..004598D0: kind 5 gets +2F4h = 0.0f when +68h is set, else 1.0f.
    void* const unit = track.ptr(0x11c);
    if (host.vcall_entity_is_kind_5c(unit, 5))
        raw(unit).at<float>(0x2f4) = raw(track.ptr(0x11c)).at<std::uint8_t>(0x68) != 0 ? 0.0f : 1.0f;
    host.movie_track_run_due_actions_004560e0(track, time);
}

// ---------------------------------------------------------------------------
// 00526180, the unit pick screen's in-place target-cycle refresh
// __thiscall(ECX = screen), plain RET at 00526719; EBP-aligned frame, EH
// state [ESP+64h]. Sole caller 00526720 (cc13_sweepA3, which reaches it
// through SweepA3Host::unit_pick_refresh_cycle_list_00526180; a SweepA4Host
// can implement that hook with this function).
// screen+0C0h the player, +0C4h a checked vector of 18h observer handles
// (first +0C8h, last +0CCh; handle +0h vtable, +10h byte, +14h target),
// +0D4h the current index. Literals: 00CECDB8 "target cycle: removing %s
// from pos %d of %d ptr %d", 00CECD80 "... removing DEAD ...", 00CECD68
// "             new ptr %d", 00CECD50 "target cycle: adding %s", 00CECD28
// "target cycle: new pointer is %d for %s" (004254B0 has an empty body).
// Phase 1 drops handles whose target is null or filtered by 00803D40 (the
// inlined erase: 00524000 copy-down, slot-0 destructor of the last element,
// last -= 18h) and moves the index. Phase 2 collects the accepted units of
// the list at [00E188A8]+19B8h (005226D0, cc13_sweepA2, called directly) not
// yet held, records the current target, appends every held target, clears
// the vector, sorts the {unit, distance} records (00524E90) and rebuilds the
// handles (register / push_back / unregister / destroy), the index following
// the current target.
// Uncertainty: the three tag dwords passed to 00524000 are stale stack bytes
// with the low byte cleared in the image; 0 here. The player kind tests
// (+5Ch with 5, then 18h) have their results discarded, as in the image.
// ---------------------------------------------------------------------------
namespace {

std::int32_t handle_count(Raw handles) {
    const auto first = reinterpret_cast<std::intptr_t>(handles.ptr(4));
    if (first == 0) return 0;
    return static_cast<std::int32_t>((reinterpret_cast<std::intptr_t>(handles.ptr(8)) - first) / 0x18);
}

std::uint8_t* bytes_at(void* pointer) { return static_cast<std::uint8_t*>(pointer); }

}  // namespace

void unit_pick_refresh_cycle_list_00526180(Raw screen, SweepA4Host& host) {
    Raw handles = raw(screen.field(0xc4));
    std::int32_t kept = 0;          // [ESP+18h]
    std::uint32_t destroy_flag = 0; // [ESP+1Ch]
    const auto checked_deref = [&](const std::uint8_t* at) {
        if (at >= bytes_at(handles.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    };

    // --- phase 1, 005261A8..0052635D ---------------------------------------
    std::uint8_t* it = bytes_at(handles.ptr(4));
    if (it > bytes_at(handles.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    for (;;) {
        std::uint8_t* const last = bytes_at(handles.ptr(8));
        if (bytes_at(handles.ptr(4)) > last) host.invalid_parameter_noinfo_00bf6713();
        if (it == last) break;
        checked_deref(it);
        if (raw(it).ptr(0x14) != nullptr) {
            checked_deref(it);
            if (host.recon_filter_undetected_unit_00803d40(raw(host.global_pointer(0x00e188d8)).ptr(0x54),
                                                           raw(it).ptr(0x14)) != 0) {
                checked_deref(it);
                it += 0x18;
                ++kept;
                continue;
            }
        }
        checked_deref(it);  // 00526232
        if (raw(it).ptr(0x14) != nullptr) {
            checked_deref(it);
            void* const target = raw(it).ptr(0x14);
            const std::int32_t count = handle_count(handles);
            const std::uint32_t pointer = screen.at<std::uint32_t>(0xd4);
            host.debug_print4_004254b0("target cycle: removing %s from pos %d of %d ptr %d",
                                       pointer_bits(host.vcall_unit_name_10(target)), static_cast<std::uint32_t>(kept),
                                       static_cast<std::uint32_t>(count), pointer);
        } else {
            host.debug_print_004254b0("target cycle: removing DEAD from pos %d of %d ptr %d",
                                      static_cast<std::uint32_t>(kept), static_cast<std::uint32_t>(handle_count(handles)),
                                      screen.at<std::uint32_t>(0xd4));
        }
        // 005262CF..00526318: erase(it).
        std::uint8_t* const saved = it;  // [ESP+2Ch]
        host.observer_handle_copy_range_00524000(it + 0x18, handles.ptr(8), it, 0, 0, 0);
        std::uint8_t* const end = bytes_at(handles.ptr(8));
        for (std::uint8_t* doomed = end - 0x18; doomed != end; doomed += 0x18)
            host.vcall_scalar_deleting_destructor_slot0(doomed, 0);
        handles.ptr(8) = bytes_at(handles.ptr(8)) - 0x18;
        // 0052631C..0052635A: the index follows (signed compare).
        const std::int32_t index = screen.at<std::int32_t>(0xd4);
        if (index > kept)
            screen.at<std::int32_t>(0xd4) = index - 1;
        else if (index == kept)
            screen.at<std::int32_t>(0xd4) = -1;
        host.debug_print1_004254b0("             new ptr %d", screen.at<std::uint32_t>(0xd4));
        it = saved;
    }

    // --- phase 2, 00526362.. -------------------------------------------------
    void* const player = host.global_pointer(0x00e188d8);
    Raw units = raw(raw(host.global_pointer(kGame)).field(0x19b8));
    if (!host.vcall_entity_is_kind_5c(player, 5)) host.vcall_entity_is_kind_5c(player, 0x18);
    alignas(4) std::uint8_t records_storage[0x10] = {};  // [ESP+30h]: first +4h, last +8h, end +0Ch
    Raw records = raw(records_storage);
    records.ptr(4) = nullptr;
    records.ptr(8) = nullptr;
    records.ptr(0xc) = nullptr;
    std::uint32_t record[2];  // [ESP+28h]
    for (void* node = units.ptr(4); node != nullptr; node = raw(node).ptr(4)) {
        void* const unit = raw(node).ptr(8);
        if (!cc13_sweepA2::unit_pick_is_command_candidate_005226d0(unit, host)) continue;
        // 005263C5..00526424: already held?
        bool held = false;
        for (std::int32_t i = 0; handles.ptr(4) != nullptr && i < handle_count(handles); ++i) {
            if (handles.ptr(4) == nullptr || i >= handle_count(handles)) host.invalid_parameter_noinfo_00bf6713();
            if (raw(bytes_at(handles.ptr(4)) + i * 0x18).ptr(0x14) == unit) {
                held = true;
                break;
            }
        }
        if (held) continue;
        host.unit_distance_record_construct_00522860(raw(record), unit);
        host.distance_vector_push_back_00525d90(records, raw(record));
        host.debug_print1_004254b0("target cycle: adding %s", pointer_bits(host.vcall_unit_name_10(unit)));
    }
    if (records.ptr(4) == nullptr) return;  // 0052646C
    if (((bytes_at(records.ptr(8)) - bytes_at(records.ptr(4))) >> 3) == 0) {
        host.free_00bf65ac(records.ptr(4));  // 005266FE
        return;
    }

    // 00526481..00526500: the current target (a temporary empty handle when
    // the index is negative, destroyed through flag bit 0).
    alignas(4) std::uint8_t handle_storage[0x18];  // [ESP+40h]
    Raw temp = raw(handle_storage);
    Raw current_handle{};
    const std::int32_t index = screen.at<std::int32_t>(0xd4);
    if (index >= 0) {
        if (handles.ptr(4) == nullptr || static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>(handle_count(handles)))
            host.invalid_parameter_noinfo_00bf6713();
        current_handle = raw(bytes_at(handles.ptr(4)) + index * 0x18);
    } else {
        temp.at<std::uint32_t>(0x04) = 0;
        temp.at<std::uint32_t>(0x08) = 0;
        temp.at<std::uint32_t>(0x0c) = 0;
        temp.at<std::uint32_t>(0x14) = 0;
        temp.at<std::uint8_t>(0x10) = 1;
        temp.at<std::uint32_t>(0x00) = 0x00ce75cc;
        destroy_flag = 1;
        current_handle = temp;
    }
    void* const current = current_handle.ptr(0x14);  // [ESP+24h]
    if ((destroy_flag & 1) != 0) host.observer_link_destroy_base_004b7ef0(temp);

    // 00526505..0052655B: every held target joins the records.
    std::uint8_t* held_it = bytes_at(handles.ptr(4));
    if (held_it > bytes_at(handles.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    if (bytes_at(handles.ptr(4)) > bytes_at(handles.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    for (; held_it != bytes_at(handles.ptr(8)); held_it += 0x18) {
        checked_deref(held_it);
        host.unit_distance_record_construct_00522860(raw(record), raw(held_it).ptr(0x14));
        host.distance_vector_push_back_00525d90(records, raw(record));
        checked_deref(held_it);
    }

    // 0052655D..005265B2: clear the handles (player slot 0 meanwhile).
    screen.ptr(0xc0) = nullptr;
    void* const handles_last = handles.ptr(8);
    if (handles.ptr(4) > handles_last) host.invalid_parameter_noinfo_00bf6713();
    void* const handles_first = handles.ptr(4);
    if (bytes_at(handles_first) > bytes_at(handles.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    std::uint32_t erase_out[2];
    host.vector_erase_range_00524dc0(handles, erase_out, handles.p, handles_first, handles.p, handles_last);
    screen.at<std::int32_t>(0xd4) = -1;
    screen.ptr(0xc0) = host.global_pointer(0x00e188d8);

    // 005265B4..005265E6: sort the records.
    if (records.ptr(4) > records.ptr(8)) host.invalid_parameter_noinfo_00bf6713();
    if (records.ptr(4) > records.ptr(8)) host.invalid_parameter_noinfo_00bf6713();
    host.ptr_float_intro_sort_00524e90(records.ptr(4), records.ptr(8),
                                       static_cast<std::int32_t>((bytes_at(records.ptr(8)) - bytes_at(records.ptr(4))) >> 3));

    // 005265EB..005266F9: rebuild the handles in sorted order.
    for (std::uint32_t j = 0;; ++j) {
        if (records.ptr(4) == nullptr) return;  // 00526602
        const auto count = static_cast<std::uint32_t>((bytes_at(records.ptr(8)) - bytes_at(records.ptr(4))) >> 3);
        if (j >= count) break;
        void* const unit = static_cast<void**>(records.ptr(4))[j * 2];
        temp.at<std::uint32_t>(0x04) = 0;
        temp.at<std::uint32_t>(0x08) = 0;
        temp.at<std::uint32_t>(0x0c) = 0;
        temp.at<std::uint32_t>(0x00) = 0x00ce74fc;  // state 2
        temp.ptr(0x14) = unit;
        temp.at<std::uint8_t>(0x10) = 1;
        if (unit != nullptr) host.observer_register_pair_00694a60(unit, temp.p);
        temp.at<std::uint32_t>(0x00) = 0x00ce75cc;  // state 3
        host.handle_vector_push_back_00525ed0(handles, temp);
        temp.at<std::uint32_t>(0x00) = 0x00ce74fc;  // state 4
        if (temp.ptr(0x14) != nullptr) host.observer_unregister_pair_006952a0(temp.ptr(0x14), temp.p);
        host.callback_owner_destroy_00695870(temp);  // state 0
        if (records.ptr(4) == nullptr ||
            j >= static_cast<std::uint32_t>((bytes_at(records.ptr(8)) - bytes_at(records.ptr(4))) >> 3))
            host.invalid_parameter_noinfo_00bf6713();
        if (static_cast<void**>(records.ptr(4))[j * 2] == current) {
            screen.at<std::uint32_t>(0xd4) = j;
            if (records.ptr(4) == nullptr ||
                j >= static_cast<std::uint32_t>((bytes_at(records.ptr(8)) - bytes_at(records.ptr(4))) >> 3))
                host.invalid_parameter_noinfo_00bf6713();
            host.debug_print2_004254b0("target cycle: new pointer is %d for %s", j,
                                       pointer_bits(host.vcall_unit_name_10(static_cast<void**>(records.ptr(4))[j * 2])));
        }
    }
    host.free_00bf65ac(records.ptr(4));  // 005266FE
}

}  // namespace bsp::cc13_sweepA4
