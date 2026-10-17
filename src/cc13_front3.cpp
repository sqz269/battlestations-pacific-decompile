// cc13_front3: flight and gun blockers in segments 40/47/48 (packet
// cc13-front3, docs/CC13_FRONT3.md). Names are hypotheses; see the header for
// the hooks. Each function's comment gives the native address, ABI and
// evidence (Capstone disasm-raw over the disk bytes plus the Ghidra
// pseudocode). EH frames are noted, not modelled: the image's EH state stores
// only select unwind actions. No function here draws from the shared RNG
// 00BD2F10.

#include "bsp/cc13_front3.hpp"

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_front3_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

// The inlined NativeString destructor: if (data) 00419CC0() then
// 00BD1510(pool; data, length + 1, 1). Layout {length @+0, data @+4}.
void destroy_native_string(void* string, Cc13Front3Host& host) {
    void* data = at<void*>(string, 4);
    if (data != nullptr) {
        const std::uint32_t bytes = at<std::uint32_t>(string, 0) + 1u;
        void* pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
    }
}

}  // namespace

// ---------------------------------------------------------------- segment 40

// 00732F50 BSP_GunClassBase_Destruct, __thiscall(ECX = gun class), plain RET.
// Body 00732F50-00733029 (EH frame 00C85BEF, states 3 -> 0 -> -1). Stores the
// class vtable 00CE4534 at +0h; releases the counted object at +C8h
// (InterlockedDecrement of +4h, slot 0 at zero) and clears the slot; frees the
// +BCh block (_free 00BF65AC, null-guarded) and zeroes +BCh/+C0h/+C4h; the
// same for +9Ch with +9Ch/+A0h/+A4h; then 00442A30(ECX = +74h; 0), an
// unguarded _free 00BF6989 of the dword at +74h, and the base destructor
// 004420D0(ECX = class). The two vectors' "clear" is three zero stores, not a
// call.
void cc13_front3_gun_class_base_destruct_00732f50(void* gun_class, Cc13Front3Host& host) {
    at<std::uint32_t>(gun_class, 0x00) = 0x00CE4534u;                       // 00732F6F
    void* counted = at<void*>(gun_class, 0xC8);
    if (counted != nullptr) {                                                // 00732F87
        if (host.interlocked_decrement(&at<std::int32_t>(counted, 4)) == 0) {
            host.vcall_00_release(counted);                                  // 00732F9D
        }
        at<void*>(gun_class, 0xC8) = nullptr;                                // 00732F9F
    }
    if (at<void*>(gun_class, 0xBC) != nullptr) {
        host.crt_free_00bf65ac(at<void*>(gun_class, 0xBC));                  // 00732FB0
    }
    at<std::uint32_t>(gun_class, 0xBC) = 0;
    at<std::uint32_t>(gun_class, 0xC0) = 0;
    at<std::uint32_t>(gun_class, 0xC4) = 0;
    if (at<void*>(gun_class, 0x9C) != nullptr) {
        host.crt_free_00bf65ac(at<void*>(gun_class, 0x9C));                  // 00732FD5
    }
    at<std::uint32_t>(gun_class, 0x9C) = 0;
    at<std::uint32_t>(gun_class, 0xA0) = 0;
    at<std::uint32_t>(gun_class, 0xA4) = 0;
    host.gun_records_resize_00442a30(ptr(gun_class, 0x74), 0);               // 00732FF9
    host.crt_free_00bf6989(at<void*>(gun_class, 0x74));                      // 00733001
    host.device_class_destruct_004420d0(gun_class);                          // 00733013
}

// 00734DB0 BSP_MissionNarrative_ApplyMissionInfoMessage, __fastcall(ECX =
// message), plain RET. Body 00734DB0-00734E41 (EH frame 00C85E08). When
// [game+21E8h] is non-null: a local NativeString is constructed from ""
// (00CE3A0C) by 0041E870, 00734870(ECX = [game+21E8h]; message+18h, &string,
// 0) appends the entry, then the string is destroyed inline. The game global
// is re-read for the call (00734DE7), as the image does.
void cc13_front3_mission_narrative_apply_mission_info_00734db0(void* message, Cc13Front3Host& host) {
    if (at<void*>(host.game_00e188a8(), 0x21E8) == nullptr) {                // 00734DCD
        return;
    }
    alignas(4) std::uint8_t text[8];
    host.native_string_assign_0041e870(text, "");                           // 00734DE2
    void* list = at<void*>(host.game_00e188a8(), 0x21E8);                    // 00734DF4
    host.narrative_list_append_00734870(list, ptr(message, 0x18), text, 0);  // 00734E06
    destroy_native_string(text, host);                                       // 00734E0B-00734E2D
}

// 0072B540 BSP_Gun_SetControlledAudioMix, __thiscall(ECX = gun; bool), RET 4.
// Body 0072B540-0072B58E. With a non-null effect set at +47Ch (the NEG/SBB/
// TEST 0E186ECh idiom is a plain non-null test), calls 0072AA10(ECX = set;
// mix) with 0.0f when the flag byte is set and 1.0f (00D7A24C) when clear.
void cc13_front3_gun_set_controlled_audio_mix_0072b540(void* gun, bool controlled, Cc13Front3Host& host) {
    void* effects = at<void*>(gun, 0x47C);
    if (effects == nullptr) {                                                // 0072B551
        return;
    }
    host.gun_effect_set_set_mix_0072aa10(effects, controlled ? 0.0f : 1.0f); // 0072B56B / 0072B589
}

// 0072D830 BSP_Gun_HandleNetworkMessage, __thiscall(ECX = gun; message),
// RET 4, AL. Gun vtable slot +164h. Switch on the byte at message+10h through
// the index table 0072D8B0 (63h bytes, base 4Bh) and jump table 0072D898:
// 4Bh / 4Ch -> true with nothing done (0072D878); 68h -> slot +1C8h(message);
// 69h -> slot +1CCh(message); ACh -> 0072D5B0(ECX = gun; [message+20h], 0);
// ADh -> slot +1DCh(); every handled case returns true. Anything else
// (including kinds below 4Bh, via the unsigned JA) tail-jumps to
// 0043F860(ECX = gun; message), whose AL is returned.
bool cc13_front3_gun_handle_network_message_0072d830(void* gun, void* message, Cc13Front3Host& host) {
    switch (at<std::uint8_t>(message, 0x10)) {
    case 0x4B:
    case 0x4C:
        return true;                                                         // 0072D878
    case 0x68:
        host.vcall_1c8(gun, message);                                        // 0072D876
        return true;
    case 0x69:
        host.vcall_1cc(gun, message);                                        // 0072D886
        return true;
    case 0xAC:
        host.gun_select_fire_mode_0072d5b0(gun, at<std::uint32_t>(message, 0x20), 0);  // 0072D854
        return true;
    case 0xAD:
        host.vcall_1dc(gun);                                                 // 0072D866
        return true;
    default:
        return host.entity_handle_messages_4b4c_0043f860(gun, message);     // 0072D891
    }
}

// 0072BD50 BSP_GunBot_ClearFireTarget, __thiscall(ECX = bot), plain RET.
// Body 0072BD50-0072BDC3; primary vtable slot +30h of the gun bots. The
// target record is at bot+38h. When 00521EA0 resolves it to an object, the
// image resolves it a second time and calls 006952A0(ECX = object, EDX =
// bot+1Ch; the compiler's null-adjust gives 0 for a null bot). Then the record
// is reset: +8h..+10h = the static vector 00F87574, word +0h = 0, dword +4h =
// 0, byte +0h = 0, word +2h = 0, float +14h = 0.0f, in the image's order.
void cc13_front3_gun_bot_clear_fire_target_0072bd50(void* bot, Cc13Front3Host& host) {
    void* record = ptr(bot, 0x38);
    if (host.command_target_resolve_object_00521ea0(record) != nullptr) {    // 0072BD59
        void* owner = bot != nullptr ? ptr(bot, 0x1C) : nullptr;             // 0072BD62-0072BD6B
        void* observed = host.command_target_resolve_object_00521ea0(record);  // 0072BD6F
        host.observer_unregister_pair_006952a0(observed, owner);             // 0072BD78
    }
    const float* zero = host.static_vector_00f87574();
    at<float>(record, 0x08) = zero[0];                                       // 0072BD85
    at<float>(record, 0x0C) = zero[1];
    at<float>(record, 0x10) = zero[2];
    at<std::uint16_t>(record, 0x00) = 0;                                     // 0072BDA7
    at<std::uint32_t>(record, 0x04) = 0;
    at<std::uint8_t>(record, 0x00) = 0;
    at<std::uint16_t>(record, 0x02) = 0;
    at<float>(record, 0x14) = 0.0f;                                          // 0072BDBD
}

// 00729F70 BSP_Gun_ReleasePlayerControl, __thiscall(ECX = gun), plain RET
// (the last call is a tail JMP). Body 00729F70-00729F96: slot +154h(0, 8),
// slot +1E8h(0), then 00728000(ECX = gun), which clears the five bots' fire
// targets.
void cc13_front3_gun_release_player_control_00729f70(void* gun, Cc13Front3Host& host) {
    host.vcall_154(gun, 0, 8);                                               // 00729F7F
    host.vcall_1e8(gun, 0);                                                  // 00729F8D
    host.gun_clear_bot_fire_target_00728000(gun);                            // 00729F92
}

// ---------------------------------------------------------------- segment 47

// 007B34F0 BSP_ScenePath_LoadSourceHolder, __thiscall(ECX = path; holder),
// RET 4. Body 007B34F0-007B3725 (EH frame 00C8BFAB; 0A4h bytes of locals: the
// point at ESP+14h, the 14h Lua reader at ESP+20h, the 80h name buffer at
// ESP+34h). Holder kinds at holder+4h (a null holder skips to the tail):
//   1 (property bag at holder+8h): path+20h = Find("Group") ? entry+0Ch : 0;
//     table = [Find("PathPoints")+0Ch] (unguarded); then, starting with
//     "Point%002i" of 00415870(path) (the current point count), while the
//     entry exists and entry+4h == 6: xyz = [Find([entry+0Ch]; "Pos")+0Ch..+14h]
//     (unguarded), path slot +8h(&xyz), and the next name from the new count.
//   2 (object at holder+8h): for i while begin [obj+114h] != 0 and i <
//     (end [obj+118h] - begin) / 12 (signed IMUL 2AAAAAABh divide, unsigned
//     compare): the checked index re-tests both and reports through 00BF6713,
//     then path slot +8h(begin + 12 * i). begin is re-read each pass.
//   3: 009238A0(ECX = [path+14h]; the by-value 14h LuaObject argument slot),
//     004425C0(ECX = reader; that LuaObject), 00BD8E20(reader; {0,
//     "_pathBaseEntity"} (00D05308)), path slot +18h(&reader), 00BD7A20(reader),
//     00441A20(reader).
// Tail: byte path+18h == 0 -> 007AF150(ECX = path).
// The Find results are dereferenced without null tests where the image has
// none ("PathPoints", "Pos").
void cc13_front3_scene_path_load_source_holder_007b34f0(void* path, void* holder, Cc13Front3Host& host) {
    if (holder != nullptr) {                                                 // 007B351C
        const std::int32_t kind = at<std::int32_t>(holder, 4);
        if (kind == 1) {
            void* bag = at<void*>(holder, 8);
            void* group = host.scene_property_bag_find_008f2260(bag, "Group");   // 007B3536
            at<std::uint32_t>(path, 0x20) = group == nullptr ? 0u : at<std::uint32_t>(group, 0x0C);
            void* points = host.scene_property_bag_find_008f2260(at<void*>(holder, 8), "PathPoints");  // 007B3552
            void* table = at<void*>(points, 0x0C);
            char name[0x80];
            host.format_string128_004f9a80(name, "Point%002i", host.vector_size_00415870(path));  // 007B355C-007B356C
            void* entry = host.scene_property_bag_find_008f2260(table, name);    // 007B357B
            while (entry != nullptr && at<std::int32_t>(entry, 4) == 6) {        // 007B3580-007B3593
                void* pos = host.scene_property_bag_find_008f2260(at<void*>(entry, 0x0C), "Pos");  // 007B35A3
                float xyz[3] = {at<float>(pos, 0x0C), at<float>(pos, 0x10), at<float>(pos, 0x14)};
                host.vcall_08_append_point(path, xyz);                           // 007B35D8
                host.format_string128_004f9a80(name, "Point%002i", host.vector_size_00415870(path));  // 007B35DC-007B35EC
                entry = host.scene_property_bag_find_008f2260(table, name);      // 007B35FB
            }
        } else if (kind == 2) {
            void* object = at<void*>(holder, 8);
            auto count = [&]() {
                const std::int32_t bytes = static_cast<std::int32_t>(
                    at<std::uint32_t>(object, 0x118) - at<std::uint32_t>(object, 0x114));
                return static_cast<std::uint32_t>(bytes / 12);
            };
            for (std::uint32_t i = 0;; ++i) {
                if (at<std::uint32_t>(object, 0x114) == 0 || i >= count()) {     // 007B3620-007B3648
                    break;
                }
                if (at<std::uint32_t>(object, 0x114) == 0 || i >= count()) {     // 007B364E-007B3672
                    host.crt_invalid_parameter_00bf6713();                       // 007B3674
                }
                host.vcall_08_append_point(path, reinterpret_cast<const float*>(
                    static_cast<std::uint8_t*>(at<void*>(object, 0x114)) + 12u * i));  // 007B3689
            }
        } else if (kind == 3) {
            alignas(4) std::uint8_t lua_object[0x14];
            host.saved_entity_lua_data_009238a0(at<void*>(path, 0x14), lua_object);  // 007B36A5
            alignas(4) std::uint8_t reader[0x14];
            host.lua_reader_construct_004425c0(reader, lua_object);              // 007B36AE
            host.lua_reader_enter_key_00bd8e20(reader, 0, "_pathBaseEntity");    // 007B36D0
            host.vcall_18_load_from_reader(path, reader);                        // 007B36E1
            host.lua_reader_leave_key_00bd7a20(reader);                          // 007B36E7
            host.lua_reader_destroy_00441a20(reader);                            // 007B36FB
        }
    }
    if (at<std::uint8_t>(path, 0x18) == 0) {                                 // 007B3700
        host.camera_path_derive_knots_007af150(path);                        // 007B3708
    }
}

// 007C11E0 BSP_Plane_NotifyFlightState, __thiscall(ECX = plane; byte
// immediate), RET 4. Body 007C11E0-007C1429. With state s = plane+900h:
//   a = !(s == 6 || (s == 7 && !(5.0f (00CE3850) >= plane+908h)))   (COMISS/JB:
//       an unordered +908h takes the a = 0 arm)
//   b = s in {7, 6, 4, 5}
// The two actuator channels of the block at [plane+DECh] are +28h (flag +29h,
// value +2Ch, moving +30h) and +44h (flag +45h, value +48h, moving +4Ch);
// either moving byte sets the block's +11h.
//   immediate == 0: flag != a -> flag = a, moving = 1 (value untouched); same
//     for b. Then when s is none of 7/6/4/5/3: +9F0h = 0.0f, controller
//     (plane+AB0h) +3Ch..+44h and +48h..+50h = the static vector 00F87574,
//     007D9C80(ECX = +AB0h), 007DDEA0(ECX = +C98h), 007EB380(ECX = +C98h; 0.0f).
//   immediate != 0: value v = a ? 1.0f (00D7A24C) : 0.0f; when flag != a or
//     !(value == v) (FUCOMPI/LAHF/TEST 44h/JNP: a NaN value rewrites) the value,
//     flag and moving byte are written; same for b. Then
//     007EB270(ECX = +C98h; [+72Ch] slot +38h()).
// Tail (state re-read at 007C13CB): s in {7, 6, 3} -> byte +C0Ch = 1 (the
// image's SETE over a constant 0; the 4/+904h arm is unreachable there). Then
// s == 1, s == 0 or byte +5Ch == 0 -> 007BC550(ECX = plane).
void cc13_front3_plane_notify_flight_state_007c11e0(void* plane, bool immediate, Cc13Front3Host& host) {
    std::int32_t s = at<std::int32_t>(plane, 0x900);
    const bool a = !(s == 6 || (s == 7 && !(5.0f >= at<float>(plane, 0x908))));  // 007C11E9-007C1208
    const bool b = s == 7 || s == 6 || s == 4 || s == 5;                    // 007C120A-007C1223
    if (immediate) {
        void* block = at<void*>(plane, 0xDEC);
        const float va = a ? 1.0f : 0.0f;
        if (at<std::uint8_t>(block, 0x29) != static_cast<std::uint8_t>(a)
            || !(at<float>(block, 0x2C) == va)) {                            // 007C124D-007C1267
            at<float>(block, 0x2C) = va;
            at<std::uint8_t>(block, 0x29) = static_cast<std::uint8_t>(a);
            at<std::uint8_t>(block, 0x30) = 1;
        }
        if (at<std::uint8_t>(block, 0x30) != 0) at<std::uint8_t>(block, 0x11) = 1;
        block = at<void*>(plane, 0xDEC);                                     // 007C1281
        const float vb = b ? 1.0f : 0.0f;
        if (at<std::uint8_t>(block, 0x45) != static_cast<std::uint8_t>(b)
            || !(at<float>(block, 0x48) == vb)) {                            // 007C128C-007C12A6
            at<float>(block, 0x48) = vb;
            at<std::uint8_t>(block, 0x45) = static_cast<std::uint8_t>(b);
            at<std::uint8_t>(block, 0x4C) = 1;
        }
        if (at<std::uint8_t>(block, 0x4C) != 0) at<std::uint8_t>(block, 0x11) = 1;
        const std::uint32_t value = host.vcall_38_dword(ptr(plane, 0x72C));  // 007C12CD
        host.plane_fx_on_flight_state_007eb270(ptr(plane, 0xC98), value);   // 007C12D6
    } else {
        void* block = at<void*>(plane, 0xDEC);
        if (at<std::uint8_t>(block, 0x29) != static_cast<std::uint8_t>(a)) {  // 007C12E6
            at<std::uint8_t>(block, 0x29) = static_cast<std::uint8_t>(a);
            at<std::uint8_t>(block, 0x30) = 1;
        }
        if (at<std::uint8_t>(block, 0x30) != 0) at<std::uint8_t>(block, 0x11) = 1;
        block = at<void*>(plane, 0xDEC);                                     // 007C12FC
        if (at<std::uint8_t>(block, 0x45) != static_cast<std::uint8_t>(b)) {
            at<std::uint8_t>(block, 0x45) = static_cast<std::uint8_t>(b);
            at<std::uint8_t>(block, 0x4C) = 1;
        }
        if (at<std::uint8_t>(block, 0x4C) != 0) at<std::uint8_t>(block, 0x11) = 1;
        s = at<std::int32_t>(plane, 0x900);                                  // 007C1318
        if (s != 7 && s != 6 && s != 4 && s != 5 && s != 3) {
            at<float>(plane, 0x9F0) = 0.0f;                                  // 007C134E
            void* controller = ptr(plane, 0xAB0);
            const float* zero = host.static_vector_00f87574();
            at<float>(controller, 0x3C) = zero[0];
            at<float>(controller, 0x40) = zero[1];
            at<float>(controller, 0x44) = zero[2];
            at<float>(controller, 0x48) = zero[0];
            at<float>(controller, 0x4C) = zero[1];
            at<float>(controller, 0x50) = zero[2];
            host.flight_controller_body_to_world_velocity_007d9c80(controller);  // 007C13AB
            host.plane_sub_block_reset_timer40_007ddea0(ptr(plane, 0xC98));      // 007C13B8
            host.plane_fx_step_007eb380(ptr(plane, 0xC98), 0.0f);                // 007C13C5
        }
    }
    s = at<std::int32_t>(plane, 0x900);                                      // 007C13CB
    if (s == 7 || s == 6 || s == 3) {
        at<std::uint8_t>(plane, 0xC0C) = 1;                                  // 007C140C
    }
    if (s == 1 || s == 0 || at<std::uint8_t>(plane, 0x5C) == 0) {            // 007C1412-007C141F
        host.plane_sync_elevator_attachment_007bc550(plane);                 // 007C1423
    }
}

// 007CB7F0 BSP_Plane_OnWaterContact, __thiscall(ECX = plane), plain RET.
// Body 007CB7F0-007CB9D2 (EH frame 00C8CA50; locals: NativeString at
// ESP+8, a 24h-byte session message at ESP+10h).
//   zero_min = desc([plane+538h])+198h MinWaterSpd == 0.0f (00D7A218; UCOMISS,
//     LAHF/TEST 44h/JP: unordered counts as not equal).
//   Return at once unless 007BC5B0(plane) or zero_min.
//   Achievement: when +1B0h < 8 (signed), slot +5Ch(14h), 8.0f (00CE3918) >
//     slot +204h() (FCOMI/JBE: unordered skips), 0.1 (double 00D05E28) >
//     +9F0h (same), 0043F080(plane) and byte +C34h: 0090F6C0(ECX =
//     [game+21A0h]; plane, 3).
//   When zero_min: byte +C8h == 0 -> 00414DB0(plane); then +E0h > 0.8f as a
//     double (00CE3D40; FCOMI/JA: ordered above) skips the "powerlost" step.
//   Otherwise (or when +E0h is not above it), unless [game+1FE4h] == 2: a
//     NativeString "powerlost" (00D059F8) through slot +194h(&string, 0), then
//     the string's inline destructor.
//   Tail (007CB92C, state s = +900h): s == 6 returns. A client
//     ([game+1FE4h] == 2) calls 007C1430(plane; 6) when s is 7, 4 or 5. A host
//     builds message C3h: 0075B430(&m; C3h), +4h = 1, word +18h = 0, byte
//     +1Ah = 0, byte +1Ch = 0, vtable 00D03504, byte +20h = (byte)s, byte
//     +21h = 6, then 0077C2A0(ECX = plane; &m, 7, 0). The message is not
//     destroyed on the normal path (the EH state 1 covers only an unwind).
void cc13_front3_plane_on_water_contact_007cb7f0(void* plane, Cc13Front3Host& host) {
    void* desc = at<void*>(plane, 0x538);
    const bool zero_min = at<float>(desc, 0x198) == 0.0f;                    // 007CB81A-007CB82B
    if (!host.plane_is_down_or_human_held_007bc5b0(plane) && !zero_min) {   // 007CB82D-007CB838
        return;
    }
    if (at<std::int32_t>(plane, 0x1B0) < 8                                   // 007CB83E
        && host.vcall_5c_has_ordnance_kind(plane, 0x14)) {                   // 007CB850
        const float slot_204 = host.vcall_204_float(plane);                  // 007CB860
        if (8.0f > slot_204                                                  // 007CB862-007CB86C
            && 0.1 > static_cast<double>(at<float>(plane, 0x9F0))           // 007CB86E-007CB87E
            && host.unit_is_alive_and_visible_0043f080(plane)                // 007CB882
            && at<std::uint8_t>(plane, 0xC34) != 0) {                        // 007CB88B
            void* recorder = at<void*>(host.game_00e188a8(), 0x21A0);
            host.achievement_record_0090f6c0(recorder, plane, 3);            // 007CB8A2
        }
    }
    bool powerlost = true;
    if (zero_min) {
        if (at<std::uint8_t>(plane, 0xC8) == 0) {
            host.entity_pose_refresh_world_00414db0(plane);                  // 007CB8B6
        }
        if (static_cast<double>(at<float>(plane, 0xE0)) > 0.800000011920929) {  // 007CB8BB-007CB8CB
            powerlost = false;
        }
    }
    if (powerlost && at<std::int32_t>(host.game_00e188a8(), 0x1FE4) != 2) {  // 007CB8CD-007CB8DA
        alignas(4) std::uint8_t name[8];
        host.native_string_assign_0041e870(name, "powerlost");               // 007CB8E5
        host.vcall_194_named(plane, name, 0);                                // 007CB903
        destroy_native_string(name, host);                                   // 007CB905-007CB927
    }
    const std::int32_t s = at<std::int32_t>(plane, 0x900);                   // 007CB92C
    if (s == 6) {
        return;
    }
    if (at<std::int32_t>(host.game_00e188a8(), 0x1FE4) == 2) {              // 007CB941
        if (s == 7 || s == 4 || s == 5) {
            host.plane_set_flight_state_007c1430(plane, 6);                  // 007CB9BD
        }
        return;
    }
    alignas(4) std::uint8_t m[0x24];
    host.session_message_construct_base_0075b430(m, 0xC3);                   // 007CB955
    at<std::uint32_t>(m, 0x04) = 1;
    at<std::uint16_t>(m, 0x18) = 0;
    at<std::uint8_t>(m, 0x1A) = 0;
    at<std::uint8_t>(m, 0x1C) = 0;
    at<std::uint32_t>(m, 0x00) = 0x00D03504u;
    at<std::uint8_t>(m, 0x20) = static_cast<std::uint8_t>(s);
    at<std::uint8_t>(m, 0x21) = 6;
    host.session_route_message_0077c2a0(plane, m, 7, 0);                     // 007CB994
}

// ---------------------------------------------------------------- segment 48

// 007DC830 BSP_PlaneFlight_FreeFlightStep, __thiscall(ECX = controller =
// unit+AB0h; float step), RET 4. Body 007DC830-007DCCE0, no EH frame, 30h
// bytes of locals. Read from the disk bytes (Capstone) with ESP tracked
// through every push and the RET 14h of 00419010 / 007DAFD0.
//   ctl+FCh = 0 (the free-flight mode), ctl+4h = unit+9F9h, ctl+5h =
//   unit+9F8h, then 007DB680(ctl; step, 0.0f, 0.0f, 0).
//   r = float(007D99C0(ctl) / desc([ctl+0Ch])+184h).
//   Stall block when StallRangeMax [00F87310] > r (FCOMI/JA: ordered) or the
//   dead-meat byte unit+5Dh is set:
//     m = Interp(StallRangeMin [00F8730C], 1, StallRangeMax, 0, r).
//     Dead meat (unit+5Dh read once into BL): spin = any of unit+C36h..C38h;
//       t = Interp(0, 0, SpinStallMulTime [00F87350], SpinStallMul [00F8734C],
//       unit+C3Ch) when spinning, else Interp(0, 0, StallMulTime [00F87360],
//       StallMul [00F8735C], unit+C3Ch); m = t unless m > t (FCOMPI/JA, so
//       a NaN m takes t).
//     When m > 0 (COMISS/JBE: ordered):
//       g = Interp(x0, 1, x1, 0, unit+C64h) with (x0, x1) = (00F87318,
//       00F87314) alive, (SpinStallMulOnPitch 00F87354, SpinStallMulOffPitch
//       00F87358) spinning, (StallMulOnPitch 00F87364, StallMulOffPitch
//       00F87368) otherwise;
//       d = float(unit+908h - 1.0 (double 00D7A210)), clamped: 0 when 0 > d
//       (FCOMI/JBE: a NaN d stays), else 1.0f when d > 1.0f (COMISS/JBE);
//       f = float(g * m * d) on the x87 (products kept wide, one rounding);
//       f > 0 (ordered): 007DAFD0(ctl; &{0, 0, 1}, &{0, -1.0f (00D7A260), 0},
//         float(f * desc+17Ch), step, 1.5f (00CE380C)).
//       Then, when unit+5Dh (re-read) is set: ctl+24h..2Ch += step * ctl+F0h..F8h
//       (each product stored as a float first).
//   ctl+94h set: v = unit slot +34h(&v), v.y = 0, len = 0042B2F0(&v),
//     h = Interp(0, 4.0f (00CE3D34), 69.444443f (00D0688C, 250 km/h), 20.0f
//     (00CE3930), len); u = 1 - |unit+C64h|, w = |unit+C68h| (the absolute
//     value is -0.0f (00D7A208) - x when x is not > 0); p = float(w *
//     0.800000011920929 (double 00CE3D40) + 0.6000000238418579 (double
//     00CEFF98)); u = 0 when 0 > u (NaN stays); q = 1.0f when p > 1.0f, else p
//     (NaN stays); 007DAFD0(ctl; &{0, 0, 1}, &{0, -1.0f, 0}, float(q * u * h),
//     step, 1000.0f (00CE3804)). The image stores that force over its own
//     step argument slot after loading step; the call gets the original step.
//   ctl+8Ch = 0.0f.
// Every read of [ctl+8h] matches a reload in the image (007DC8D2, 007DCAF2,
// 007DCB58, 007DCBAC).
void cc13_front3_plane_flight_free_flight_step_007dc830(void* controller, float step, Cc13Front3Host& host) {
    at<std::uint32_t>(controller, 0xFC) = 0;                                 // 007DC841
    void* unit = at<void*>(controller, 8);
    at<std::uint8_t>(controller, 4) = at<std::uint8_t>(unit, 0x9F9);         // 007DC85D
    at<std::uint8_t>(controller, 5) = at<std::uint8_t>(unit, 0x9F8);         // 007DC86B
    host.plane_flight_core_law_007db680(controller, step, 0.0f, 0.0f, 0);    // 007DC86E
    void* desc = at<void*>(controller, 0x0C);
    const float r = static_cast<float>(
        static_cast<double>(host.plane_flight_forward_speed_007d99c0(controller))
        / static_cast<double>(at<float>(desc, 0x184)));                     // 007DC878-007DC886
    const float stall_range_max = host.dynamics_mirror_float(0x00F87310u);
    if (stall_range_max > r || at<std::uint8_t>(at<void*>(controller, 8), 0x5D) != 0) {  // 007DC894-007DC89F
        float m = host.interpolate_clamped_00419010(host.dynamics_mirror_float(0x00F8730Cu), 1.0f,
                                                    stall_range_max, 0.0f, r);  // 007DC8C9
        unit = at<void*>(controller, 8);                                     // 007DC8D2
        const bool dead = at<std::uint8_t>(unit, 0x5D) != 0;
        const bool spinning = at<std::uint8_t>(unit, 0xC36) != 0 || at<std::uint8_t>(unit, 0xC37) != 0
            || at<std::uint8_t>(unit, 0xC38) != 0;
        if (dead) {
            const float t = spinning
                ? host.interpolate_clamped_00419010(0.0f, 0.0f, host.dynamics_mirror_float(0x00F87350u),
                                                    host.dynamics_mirror_float(0x00F8734Cu),
                                                    at<float>(unit, 0xC3C))
                : host.interpolate_clamped_00419010(0.0f, 0.0f, host.dynamics_mirror_float(0x00F87360u),
                                                    host.dynamics_mirror_float(0x00F8735Cu),
                                                    at<float>(unit, 0xC3C));  // 007DC944
            if (!(m > t)) {                                                  // 007DC955-007DC961
                m = t;
            }
        }
        if (m > 0.0f) {                                                      // 007DC970-007DC973
            float g;
            if (!dead) {
                g = host.interpolate_clamped_00419010(host.dynamics_mirror_float(0x00F87318u), 1.0f,
                                                      host.dynamics_mirror_float(0x00F87314u), 0.0f,
                                                      at<float>(unit, 0xC64));
            } else if (spinning) {
                g = host.interpolate_clamped_00419010(host.dynamics_mirror_float(0x00F87354u), 1.0f,
                                                      host.dynamics_mirror_float(0x00F87358u), 0.0f,
                                                      at<float>(unit, 0xC64));
            } else {
                g = host.interpolate_clamped_00419010(host.dynamics_mirror_float(0x00F87364u), 1.0f,
                                                      host.dynamics_mirror_float(0x00F87368u), 0.0f,
                                                      at<float>(unit, 0xC64));
            }                                                                // 007DCA1A
            float d = static_cast<float>(static_cast<double>(at<float>(unit, 0x908)) - 1.0);  // 007DCA2B-007DCA37
            if (0.0f > d) {                                                  // 007DCA41-007DCA45
                d = 0.0f;
            } else if (d > 1.0f) {                                           // 007DCA58-007DCA5B
                d = 1.0f;
            }
            const float f = static_cast<float>(static_cast<double>(g) * m * d);  // 007DCA6E-007DCA7A
            if (f > 0.0f) {                                                  // 007DCA82-007DCA86
                const float force = static_cast<float>(static_cast<double>(f) * at<float>(desc, 0x17C));
                const float point[3] = {0.0f, 0.0f, 1.0f};
                const float direction[3] = {0.0f, -1.0f, 0.0f};
                host.flight_controller_apply_force_at_point_007dafd0(controller, point, direction, force,
                                                                     step, 1.5f);  // 007DCAE6
            }
            if (at<std::uint8_t>(at<void*>(controller, 8), 0x5D) != 0) {     // 007DCAF2-007DCAF9
                const float a = step * at<float>(controller, 0xF0);
                const float b = at<float>(controller, 0xF4) * step;
                const float c = step * at<float>(controller, 0xF8);
                at<float>(controller, 0x24) = at<float>(controller, 0x24) + a;
                at<float>(controller, 0x28) = b + at<float>(controller, 0x28);
                at<float>(controller, 0x2C) = at<float>(controller, 0x2C) + c;  // 007DCB40
            }
        }
    }
    if (at<std::uint8_t>(controller, 0x94) != 0) {                           // 007DCB4B
        float v[3];
        host.vcall_34_vector(at<void*>(controller, 8), v);                   // 007DCB65
        v[1] = 0.0f;
        const float len = host.vector3_length_threshold_0042b2f0(v);         // 007DCB74
        const float h = host.interpolate_clamped_00419010(0.0f, 4.0f, 69.44444274902344f, 20.0f, len);  // 007DCBA3
        unit = at<void*>(controller, 8);                                     // 007DCBAC
        const float c64 = at<float>(unit, 0xC64);
        const float t = c64 > 0.0f ? c64 : -0.0f - c64;
        float u = 1.0f - t;                                                  // 007DCBEB-007DCBF3
        const float c68 = at<float>(unit, 0xC68);
        const float w = c68 > 0.0f ? c68 : -0.0f - c68;                      // 007DCBE8-007DCC05
        const float p = static_cast<float>(static_cast<double>(w) * 0.800000011920929
                                           + 0.6000000238418579);           // 007DCC0B-007DCC1B
        if (0.0f > u) {                                                      // 007DCC23-007DCC29
            u = 0.0f;
        }
        const float q = p > 1.0f ? 1.0f : p;                                 // 007DCC3F-007DCC53
        const float force = static_cast<float>(static_cast<double>(q) * u * h);  // 007DCC80-007DCCB6
        const float point[3] = {0.0f, 0.0f, 1.0f};
        const float direction[3] = {0.0f, -1.0f, 0.0f};
        host.flight_controller_apply_force_at_point_007dafd0(controller, point, direction, force, step,
                                                             1000.0f);       // 007DCCCB
    }
    at<float>(controller, 0x8C) = 0.0f;                                      // 007DCCD3
}

}  // namespace bsp
