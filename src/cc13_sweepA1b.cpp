// Packet cc13-sweepA1 (worktree cc13-sweepA1b, marker sweepA1): segments 0..14
// leftovers. See include/bsp/cc13_sweepA1b.hpp and docs/CC13_SWEEPA1.md. Each
// body carries its address, the original ABI and every simplification next to
// it. Common simplification: SEH unwind actions are not modelled.
//
// x87 note (rule 4c): these run on the main thread (24-bit precision control);
// each x87 arithmetic result is modelled as rounded to float in the image's
// order.

#include "bsp/cc13_sweepA1b.hpp"

#include <cstring>

namespace bsp::cc13_sweepA1b {

namespace {

Raw raw(void* pointer) { return Raw{static_cast<std::uint8_t*>(pointer)}; }

std::int32_t& global_i32(SweepA1bHost& host, std::uint32_t address) {
    return *static_cast<std::int32_t*>(host.global_address(address));
}

const char* global_text(SweepA1bHost& host, std::uint32_t address) {
    return static_cast<const char*>(host.global_address(address));
}

}  // namespace

// ---------------------------------------------------------------------------
// 004D80D0 BSP_Game_UpdateMultiplayerInterface
// __fastcall(ECX = game), plain RET; EBP-framed with an 8-aligned ESP, SEH
// handler 00C66498. Read from the listing 004D80D0..004D861F (359
// instructions); the exported pseudocode drops the error-code selection.
// Globals: [00F8A2FC] network session, [00E198B4] multi menu, [00F8ABE8],
// [00E198C4], [00E19698] overlay, [00E188A8] game, dword 00F1B038 the
// session-end state.
// Simplifications: SEH states 0..8 are not modelled. The flag passed to
// 004D7BA0 is the SETZ byte; the image pushes the whole dword [ESP+10h], whose
// upper three bytes are stale.
// ---------------------------------------------------------------------------
void game_update_multiplayer_interface_004d80d0(Raw game, SweepA1bHost& host) {
    std::int32_t& end_state = global_i32(host, 0x00f1b038);
    auto multi_menu = [&host]() { return host.global_pointer(0x00e198b4); };

    // 004D80F3..004D811B
    if (void* const session = host.global_pointer(0x00f8a2fc); session != nullptr) {
        host.session_vcall_0c(session, game.at<float>(0x21ec));
    }
    if (multi_menu() != nullptr) host.multi_menu_update_online_interface_006890b0(multi_menu());
    const Raw session_block{game.field(0x1ef0)};
    host.session_poll_0076a750(session_block);

    // 004D8131..004D8139: no session kind: the end state is cleared and the
    // tail (including the broadcast) is skipped.
    const std::int32_t session_kind = game.at<std::int32_t>(0x1fe4);
    if (session_kind == 0) {
        end_state = 0;
        return;
    }

    // The shared "leave to the main interface" arm (004D81B0..004D824F and
    // 004D82C3..004D8300); `code` is evaluated only when the end state is not 2.
    auto leave_to_main = [&](auto code) {
        host.multi_menu_leave_to_main_interface_00688c70(multi_menu());
        if (end_state != 2) {
            host.multi_menu_push_request_interface_logged_00688ad0(multi_menu(), 0x1b);
            const std::int32_t value = code();
            host.multi_error_screen_set_error_code_005d2e70(raw(multi_menu()).ptr(0x50), value);
        }
        end_state = 0;
        host.multiplayer_interface_apply_pending_size_004bdf50(multi_menu());
    };
    // 004D827C..004D829C / 004D8500..004D8520: debug line, then request 10h.
    auto request_scene_term = [&](std::uint32_t format) {
        host.debug_print0_004254b0(global_text(host, format));
        const std::int32_t request = 0x10;
        host.game_enqueue_state_request_004d3ed0(Raw{game.field(0x5d8)}, &request);
    };
    // The three prompts (004D8321..004D8428, 004D845F..004D84F4): the empty
    // string, then the text, a by-value empty string, the menu command screen
    // and 00531B00(2, &text, 2, 004D7B10, 0, &empty, 0.0f, 0, by value, 1);
    // the text is destroyed first, then the empty string.
    auto raise_prompt = [&](std::uint32_t text_address) {
        alignas(4) std::uint8_t empty_storage[8];
        alignas(4) std::uint8_t text_storage[8];
        alignas(4) std::uint8_t by_value_storage[8];
        const Raw empty{empty_storage};
        const Raw text{text_storage};
        const Raw by_value{by_value_storage};
        host.native_string_construct_0041e870(empty, global_text(host, 0x00ce3a0c));
        host.native_string_construct_0041e870(text, global_text(host, text_address));
        host.native_string_construct_0041e870(by_value, global_text(host, 0x00ce3a0c));
        void* const menu = host.menu_command_screen_get_singleton_00425d10();
        host.menu_prompt_screen_raise_00531b00(menu, 2, text, 2, 0x004d7b10u, 0, empty, 0.0f, 0, by_value, 1);
        host.native_string_destroy_storage_0041dd20(text);
        host.native_string_destroy_storage_0041dd20(empty);
    };

    const std::int32_t interface_state = game.at<std::int32_t>(0x624);
    if (interface_state == 1 || interface_state == 2) {
        // 004D82A6..004D8525
        if (game.at<std::uint8_t>(0x610) == 0) {
            const std::int32_t mode = game.at<std::int32_t>(0x5d4);
            bool set_done = true;
            if (mode != 0xd && mode != 0xc) {
                leave_to_main([] { return 7; });
            } else if (end_state != 0) {
                if (end_state == 1) {
                    set_done = false;
                } else {
                    request_scene_term(0x00ce79c0);  // "... -> SCENE_AUTO_TERM"
                }
            } else if (session_kind == 2) {
                // "FE.multi_terminated_client" for state 1, else
                // "ingame.multi_gameclosed_xbox".
                raise_prompt(interface_state == 1 ? 0x00ce7a28 : 0x00ce7a08);
                game.at<std::uint8_t>(0x608) = 1;
                host.game_set_cinematic_mode_004cd0f0(game, 1, 0, 1);
                if (void* const overlay = host.global_pointer(0x00e19698); overlay != nullptr) {
                    raw(overlay).at<std::uint8_t>(4) = 0;
                }
            } else if (mode == 0xd) {
                raw(host.menu_command_screen_get_singleton_00425d10()).at<std::uint8_t>(0x288) = 1;
                raise_prompt(0x00ce79f4);  // "FE.multi_terminated"
            } else {
                set_done = false;
            }
            if (set_done) game.at<std::int32_t>(0x624) = 3;  // 004D8525
        }
    } else if (session_kind == 2 && host.session_is_client_stage3_or4_004b61c0(session_block) &&
               interface_state == 0 && game.at<std::uint8_t>(0x610) == 0) {
        // 004D8184..004D82A1
        const std::uint8_t kicked = raw(game.ptr(0x207c)).at<std::int32_t>(0x9c) == 3 ? 1 : 0;
        const std::int32_t mode = game.at<std::int32_t>(0x5d4);
        if (mode == 0xd || mode == 0xc) {
            if (end_state == 0) {
                host.game_show_session_lost_message_004d7ba0(game, kicked);
            } else if (end_state != 1) {
                request_scene_term(0x00ce7a44);  // "... -> SCENE_TERM"
            }
        } else {
            // 004D81D1..004D823C: 3 when kicked, 6 when [00F8ABE8]+128h is
            // set, 1 when the session exists and both +19Ch and +40h answer
            // false, else 2.
            leave_to_main([&]() -> std::int32_t {
                if (kicked != 0) return 3;
                if (raw(host.global_pointer(0x00f8abe8)).at<std::uint8_t>(0x128) != 0) return 6;
                void* const session = host.global_pointer(0x00f8a2fc);
                if (session == nullptr) return 2;
                if (host.session_vcall_19c(session)) return 2;
                if (host.session_vcall_40(host.global_pointer(0x00f8a2fc))) return 2;
                return 1;
            });
        }
    }

    // 004D852F..004D85D2: game mode 0Dh.
    if (game.at<std::int32_t>(0x5d4) == 0xd) {
        const Raw global_game = raw(host.global_pointer(0x00e188a8));
        const std::int32_t phase = global_game.at<std::int32_t>(0x614);
        const bool phase_checked = global_game.at<std::uint8_t>(0x61c) != 0 ||
                                   global_game.at<std::int32_t>(0x1fe4) != 0 || phase == 9 || phase == 8;
        bool select = phase_checked && (phase == 4 || phase == 5 || phase == 6 || phase == 7);
        if (!select) {
            select = !host.hud_query_006529e0(raw(host.global_pointer(0x00e198c4)).ptr(0xbc));
        }
        if (select) host.game_select_controlled_unit_004c3a80(game);
        // 004D859A: the active page record; no null check on [00E19698].
        const Raw page = raw(game.ptr(0x18cc + game.at<std::int32_t>(0x18ec) * 4));
        if (page.at<std::int16_t>(0x10) > 0 && page.at<std::uint8_t>(0x18) == 0) {
            raw(host.global_pointer(0x00e19698)).at<std::uint8_t>(4) = 0;
            raw(game.ptr(0x18cc + game.at<std::int32_t>(0x18ec) * 4)).at<std::uint8_t>(0x18) = 1;
        }
    }

    // 004D85D6..004D85EF
    const std::int32_t mode = game.at<std::int32_t>(0x5d4);
    if (mode == 0xc || mode == 0xd || mode == 0x12) host.session_broadcast_queued_messages_0076c500(session_block);
}

// ---------------------------------------------------------------------------
// 0046B730 (scene database kill pass)
// __fastcall(ECX = database), plain RET; EBP-framed with an 8-aligned ESP, SEH
// handler 00C61CEB. Read from the listing 0046B730..0046BD07 (272
// instructions); the pseudocode loses ESI in the second loop and the register
// inputs of the class create call.
// Database: +24h vector of record pointers (first +28h, last +2Ch, end
// +30h), +34h class map, +158h std::map (head node at +15Ch) keyed by the
// entities' +C4h value. Entities: list from [[world+8h]], next +44h, children
// count +50h, first child +48h (next +44h).
// Simplifications: SEH states are not modelled; the local vector is the 10h
// bytes the image keeps at [ESP+40h] (allocator, first, last, end). The
// checked-iterator failures 00BF6713 are called at the image's points and the
// C++ continues as the image does.
// ---------------------------------------------------------------------------
void scene_database_kill_pass_0046b730(Raw database, SweepA1bHost& host) {
    auto check = [&host]() { host.invalid_parameter_noinfo_00bf6713(); };
    auto world = [&host]() { return raw(host.global_pointer(0x00e188a8)).ptr(0x19cc); };
    const Raw map{database.field(0x158)};

    // map.find(key) != end, with the iterator check the image makes.
    auto registered = [&](void* const* key) {
        alignas(4) std::uint8_t iterator_storage[8];
        const Raw iterator{iterator_storage};
        host.database_map_find_00468db0(map, iterator, key);
        void* const end = map.ptr(0x4);
        if (iterator.ptr(0) == nullptr || iterator.ptr(0) != map.p) check();
        return iterator.ptr(4) != end;
    };
    // The i-th child (0 when the index is out of range, then walked while
    // non-null).
    auto child_at = [](Raw entity, std::int32_t index) -> void* {
        if (index < 0 || entity.at<std::int32_t>(0x50) <= index) return nullptr;
        void* child = entity.ptr(0x48);
        for (std::int32_t n = index; child != nullptr && n != 0; --n) child = raw(child).ptr(0x44);
        return child;
    };

    // 0046B74E..0046B763
    host.game_object_list_walk_004881f0(raw(host.global_pointer(0x00e188a8)).ptr(0x21d0));

    // 0046B768..0046B8E1: collect the unregistered entities.
    alignas(4) std::uint8_t victims_storage[0x10] = {};
    const Raw victims{victims_storage};  // +4 first, +8 last, +0Ch end
    for (void* entity = raw(raw(world()).ptr(0x8)).ptr(0x0); entity != nullptr; entity = raw(entity).ptr(0x44)) {
        const Raw e = raw(entity);
        void* key = e.ptr(0xc4);
        if (!registered(&key)) {
            void* value = entity;
            host.pointer_vector_push_back_004323d0(victims, &value);
            continue;
        }
        for (std::int32_t index = 0; index < e.at<std::int32_t>(0x50); ++index) {
            void* child_key = raw(child_at(e, index)).ptr(0xc4);
            if (registered(&child_key)) continue;
            void* value = child_at(e, index);
            void** const first = static_cast<void**>(victims.ptr(0x4));
            void** const last = static_cast<void**>(victims.ptr(0x8));
            void** const end = static_cast<void**>(victims.ptr(0xc));
            if (first != nullptr && static_cast<std::uint32_t>(last - first) < static_cast<std::uint32_t>(end - first)) {
                *last = value;
                victims.ptr(0x8) = last + 1;
            } else {
                if (first > last) check();
                alignas(4) std::uint8_t out_storage[8];
                host.pointer_vector_insert_one_004322d0(victims, Raw{out_storage}, victims, last, &value);
            }
        }
    }

    // 0046B8ED..0046B912: kill each (cause 2).
    for (std::uint32_t i = 0; victims.ptr(0x4) != nullptr; ++i) {
        void** const first = static_cast<void**>(victims.ptr(0x4));
        if (i >= static_cast<std::uint32_t>(static_cast<void**>(victims.ptr(0x8)) - first)) break;
        host.mission_entity_kill_00926d90(first[i], 2);
    }

    // 0046B914..0046B98D, then 0046BBD2..0046BC4B with mode 1.
    auto settle = [&](std::uint8_t mode) {
        host.game_run_extra_fixed_step_00874d00(mode);
        host.entity_world_flush_activations_00903670(world());
        host.game_run_extra_fixed_step_00874d00(mode);
        for (int n = 0; n < 3; ++n) {
            host.world_vcall_0c(world(), 0.0f);
            host.game_run_extra_fixed_step_00874d00(mode);
        }
    };
    settle(0);

    // 0046B992..0046B99B
    host.game_set_game_mode_004bc890(host.global_pointer(0x00e188a8), 8, 0);

    // 0046B9A0..0046BBB8: recreate the records whose class is unregistered.
    const Raw records{database.field(0x24)};  // +4 first, +8 last, +0Ch end
    void** cursor = static_cast<void**>(records.ptr(0x4));
    if (cursor > records.ptr(0x8)) check();
    for (;;) {
        void** const last = static_cast<void**>(records.ptr(0x8));
        if (records.ptr(0x4) > last) check();
        if (cursor == last) break;
        if (cursor >= records.ptr(0x8)) check();
        const Raw record = raw(*cursor);

        // 0046B9E5..0046BA61: class = map+34h lookup of the record's +0Ch name.
        const char* const class_name = static_cast<const char*>(record.ptr(0xc));
        alignas(4) std::uint8_t name_storage[8] = {};
        const Raw name{name_storage};
        host.native_string_resize_0041dd40(name, static_cast<std::uint32_t>(std::strlen(class_name)), 1);
        void* const name_data = name.ptr(4);
        const std::uint32_t name_length = name.at<std::uint32_t>(0);
        if (name_data != nullptr) std::memcpy(name_data, class_name, name_length + 1);
        std::int32_t hash = 0;
        void* const node = host.scene_class_map_find_node_00468fb0(Raw{database.field(0x34)}, name, &hash);
        const Raw class_record = raw(raw(node).ptr(0x8));
        if (name_data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), name_data,
                                                          name_length + 1, 1);
        }

        void* class_key = class_record.ptr(0x0);
        if (!registered(&class_key)) {
            // 0046BAAB..0046BB46: the entity named by the record's +54h/+58h
            // string, when that string differs (case-insensitively) from "".
            void* entity = nullptr;
            alignas(4) std::uint8_t empty_storage[8] = {};
            const Raw empty{empty_storage};
            host.native_string_resize_0041dd40(empty, 0, 1);
            void* const empty_data = empty.ptr(4);
            const std::uint32_t empty_length = empty.at<std::uint32_t>(0);
            if (empty_data != nullptr) std::memcpy(empty_data, global_text(host, 0x00ce3a0c), empty_length + 1);
            std::int32_t compare;
            if (record.at<std::uint32_t>(0x54) == 0) {
                compare = empty_length != 0 ? -1 : 0;
            } else if (empty_length == 0) {
                compare = 1;
            } else {
                compare = host.crt_stricmp_00bf7fbf(static_cast<const char*>(record.ptr(0x58)),
                                                    static_cast<const char*>(empty_data));
            }
            if (empty_data != nullptr) {
                host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(),
                                                              empty_data, empty_length + 1, 1);
            }
            if (compare != 0) {
                const char* entity_name = static_cast<const char*>(record.ptr(0x58));
                if (entity_name == nullptr) entity_name = global_text(host, 0x00e18560);
                entity = host.entity_registry_find_by_name_00925a90(world(), entity_name);
            }

            // 0046BB4A..0046BB94: create, then +C0h = a new property bag ref.
            void* const created = host.scene_class_create(
                class_record.ptr(0x4), class_record.at<std::uint32_t>(0x0), record.at<std::uint32_t>(0x8), entity,
                record.field(0x14), record.at<std::uint32_t>(0x4), 0);
            void* const block = host.operator_new_00bf681b(0xc);
            void* const bag = block != nullptr
                                  ? host.scene_property_bag_ref_construct_00922e20(block, record.at<std::uint32_t>(0x4))
                                  : nullptr;
            raw(created).ptr(0xc0) = bag;
        }

        // 0046BB9A..0046BBB4
        if (cursor >= records.ptr(0x8)) check();
        ++cursor;
    }

    // 0046BBBD..0046BBCD
    host.sentity_init_all_00925f20(0);
    host.sentity_init_all_00925f20(0);
    host.scene_resolve_deferred_references_0046aab0(database.p);
    settle(1);

    // 0046BC50..0046BC9F: delete every record (slot 0, flag 1).
    void** item = static_cast<void**>(records.ptr(0x4));
    if (item > records.ptr(0x8)) check();
    for (;;) {
        void** const last = static_cast<void**>(records.ptr(0x8));
        if (records.ptr(0x4) > last) check();
        if (item == last) break;
        if (item >= records.ptr(0x8)) check();
        if (*item != nullptr) host.vcall_scalar_deleting_destructor_slot0(*item, 1);
        if (item >= records.ptr(0x8)) check();
        ++item;
    }

    // 0046BCA1..0046BCE1: the inlined erase(begin, end).
    {
        void** const erase_last = static_cast<void**>(records.ptr(0x8));
        if (records.ptr(0x4) > erase_last) check();
        void** const erase_first = static_cast<void**>(records.ptr(0x4));
        if (erase_first > records.ptr(0x8)) check();
        if (erase_first != erase_last) {
            const std::int32_t tail = static_cast<std::int32_t>(static_cast<void**>(records.ptr(0x8)) - erase_last);
            const std::uint32_t bytes = static_cast<std::uint32_t>(tail) * 4;
            if (tail > 0) host.crt_memmove_s_00bf67a7(erase_first, bytes, erase_last, bytes);
            records.ptr(0x8) = erase_first + tail;
        }
    }

    // 0046BCE4..0046BCF2
    if (void* const first = victims.ptr(0x4); first != nullptr) host.free_00bf65ac(first);
}

// ---------------------------------------------------------------------------
// 004F3DA0 BSP_Triangle2f_ContainsPoint
// __fastcall(ECX = point, EDX = triangle), RET; AL (EAX = 1 / 0 on the
// containment exits, only AL cleared on the parameter-range exits). Read from
// the listing 004F3DA0..004F3ED7 and the live decompile (rule 4c: the x87
// listing prints reverse forms ambiguously). Every x87 result rounds to float
// (24-bit main thread), so each step is a float operation in the image's
// order. Constants: 1.0f (00D7A24C), 0.0f (00D7A218).
// Branches: e1x == 0 is the ordered-equal test (FUCOMIP, LAHF, TEST AH 44h,
// JP), so NaN takes the general arm. t < 0, t > 1, u < 0 and t + u > 1 each
// reject with NaN also rejecting (JC after FCOMI / COMISS).
// ---------------------------------------------------------------------------
bool triangle2f_contains_point_004f3da0(const float* point, const float* triangle) {
    const float dx = point[0] - triangle[0];
    const float dy = point[1] - triangle[1];
    const float e1x = triangle[2] - triangle[0];
    const float e1y = triangle[3] - triangle[1];
    const float e2x = triangle[4] - triangle[0];
    const float e2y = triangle[5] - triangle[1];
    float t;
    float u;
    if (e1x == 0.0f) {
        // 004F3E05..004F3E43
        t = dx / e2x;
        if (!(t >= 0.0f) || !(1.0f >= t)) return false;
        u = (dy - t * e2y) / e1y;
    } else {
        // 004F3E45..004F3E9B
        const float numerator = e1x * dy - dx * e1y;
        const float denominator = e2y * e1x - e1y * e2x;
        t = numerator / denominator;
        if (!(t >= 0.0f) || !(1.0f >= t)) return false;
        u = (dx - t * e2x) / e1x;
    }
    // 004F3E9D..004F3EC9
    if (!(u >= 0.0f)) return false;
    const float sum = t + u;
    return 1.0f >= sum;
}

// ---------------------------------------------------------------------------
// 004F4880 BSP_Segment2f_ClipAgainstLine
// __fastcall(ECX = P, EDX = A; [ESP+4] float radius, [ESP+8] const float* B,
// [ESP+0Ch] out0, [ESP+10h] out1), RET 10h; EAX = count. Read from the
// listing 004F4880..004F49E2 and the live decompile. Frame slots matter: the
// t output of 004F3630 is the slot that held n.x and the s output the slot
// that held the reciprocal length (the image's [ESP+8] and B's argument slot),
// so when 004F3630 leaves an output unwritten the old value is read, as in
// the image. Every x87 result rounds to float (24-bit main thread).
// Branches: s < -r and r < s reject (JC: NaN rejects); each candidate needs
// 0 <= x (JC after FCOMIP) and x <= 1.0f (COMISS, JC).
// ---------------------------------------------------------------------------
std::int32_t segment2f_clip_against_line_004f4880(const float* point, const float* a, float radius,
                                                  const float* b, float* out0, float* out1, SweepA1bHost& host) {
    // 004F4883..004F48A8
    float d[2];
    d[0] = b[0] - a[0];
    d[1] = b[1] - a[1];
    float slot_s = host.vector2f_reciprocal_length_00419260(d);  // [ESP+30h]: 1/|d|, then s
    // 004F48B0..004F48FF
    float slot_t[2];                                            // [ESP+8h]: n, then t in +0
    slot_t[0] = d[0] * slot_s;
    slot_t[1] = slot_s * d[1];
    const float m[2] = {-slot_t[1], slot_t[0]};
    const float q[2] = {point[0] - m[0], point[1] - m[1]};
    host.math_segment_parameters_2d_004f3630(a, d, q, m, &slot_t[0], &slot_s);

    // 004F4908..004F4924
    const float s = slot_s;
    if (!(s >= -radius)) return 0;
    if (!(radius >= s)) return 0;
    // 004F492A..004F4954
    const float square = radius * radius - s * s;
    const float half = host.crt_cisqrt_00bf7030(square);
    const float length = host.vector2f_length_with_cutoff_00414c60(d);
    const float offset = half / length;
    // 004F496A..004F49C2
    const float t = slot_t[0];
    const float t0 = t - offset;
    const float t1 = t + offset;
    std::int32_t count = 0;
    float* destination = out0;
    if (t0 >= 0.0f && 1.0f >= t0) {
        *destination = t0;
        count = 1;
    }
    if (t1 >= 0.0f && 1.0f >= t1) {
        if (count != 0) destination = out1;
        *destination = t1;
        ++count;
    }
    return count;
}

}  // namespace bsp::cc13_sweepA1b
