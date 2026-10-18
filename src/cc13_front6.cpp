// Packet cc13-front6 (marker cc13_front6): wiring-frontier callees. Evidence,
// ABI and uncertainty per function: docs/CC13_FRONT6.md. Nothing here is wired.
#include "bsp/cc13_front6.hpp"

#include <cstdint>
#include <cstring>
#include <xmmintrin.h>

namespace bsp::cc13_front6 {
namespace {

std::uint32_t addr32(const void* p) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}

// A string key {tag 0, char* payload}; the payload is the image string's address.
TaggedValue key(std::uint32_t image_string, Front6Host& host) {
    return TaggedValue{0, addr32(host.global_address(image_string))};
}

}  // namespace

// ---------------------------------------------------------------------------
// One-instruction bodies.
// ---------------------------------------------------------------------------

// 0054D3C0: F3 0F 2C 41 54 (CVTTSS2SI EAX, [ECX+54h]); C3. Truncation; NaN and
// out-of-range inputs give 80000000h, as the intrinsic does.
std::int32_t hints_screen_seconds_left_0054d3c0(Raw screen) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(screen.at<float>(0x54)));
}

// 004B47E0: C3. Callers push their (x, y, format, ...) and clean the stack.
void debug_print_004b47e0(const void*, ...) noexcept {}

// 00ADEAF0: 8B 41 24; C3.
void* terrain_tile_model_00adeaf0(Raw tile) noexcept { return tile.ptr(0x24); }
// 00AF1130: 8B 41 24; C3.
void* particle_resources_sub_object_00af1130(Raw object) noexcept { return object.ptr(0x24); }
// 00ADAC30: 8B 41 38; C3.
std::int32_t height_field_cells_x_00adac30(Raw height_field) noexcept {
    return height_field.at<std::int32_t>(0x38);
}
// 00ADAC40: 8B 41 3C; C3.
std::int32_t height_field_cells_z_00adac40(Raw height_field) noexcept {
    return height_field.at<std::int32_t>(0x3C);
}
// 00BBDDC0: 8B 41 0C; C3.
void* atmosphere_source_00bbddc0(Raw manager_child) noexcept { return manager_child.ptr(0x0C); }
// 00B4CBB0: 8B 41 0C; C3.
void* render_pass_camera_00b4cbb0(Raw pass) noexcept { return pass.ptr(0x0C); }
// 00B0CF20: 32 C0; C3.
bool post_effect_enabled_00b0cf20(Raw) noexcept { return false; }
// 00A4C2C0: C3.
void empty_body_00a4c2c0(Raw) noexcept {}
// 006F7B50: C3.
void empty_body_006f7b50(Raw) noexcept {}
// 00778800: C3.
void empty_body_00778800(Raw) noexcept {}
// 00786CA0: C2 0C 00.
void empty_body_00786ca0(Raw, std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}

// 00B916F0: A1 E0 03 09 01; C3.
std::uint32_t quadtree_type_00b916f0(Front6Host& host) {
    return *static_cast<std::uint32_t*>(host.global_address(0x010903E0));
}
// 008D45C0: A1 78 89 F8 00; C3.
std::int32_t global_f88978_get_008d45c0(Front6Host& host) {
    return *static_cast<std::int32_t*>(host.global_address(0x00F88978));
}
// 0078E470: 8B 81 8C 03 00 00; C3.
void* camera_view_unit_0078e470(Raw view) noexcept { return view.ptr(0x38C); }
// 005662F0: 89 0D C0 86 E0 00; C3.
void mode_selector_set_005662f0(std::int32_t mode, Front6Host& host) {
    *static_cast<std::int32_t*>(host.global_address(0x00E086C0)) = mode;
}
// 008DA180: 8D 41 24; C3.
std::uint8_t* objective_manager_008da180(Raw owner) noexcept { return owner.field(0x24); }
// 007F7D90: C7 01 74 49 CE 00; C3.
void session_message_root_store_vtable_007f7d90(Raw object) noexcept {
    object.at<std::uint32_t>(0) = 0x00CE4974u;
}
// 007B9000: C6 81 01 0C 00 00 02; C3.
void run_surface_mark_released_007b9000(Raw object) noexcept { object.at<std::uint8_t>(0xC01) = 2; }
// 005CE750: C6 81 8D 00 00 00 01; C3.
void option_panel_set_flag_8d_005ce750(Raw panel) noexcept { panel.at<std::uint8_t>(0x8D) = 1; }
// 00472520: 8B 41 28; C3.
void* field_28_get_00472520(Raw object) noexcept { return object.ptr(0x28); }
// 00479BC0: 8B 41 28; C3.
void* vehicle_slot_class_00479bc0(Raw slot) noexcept { return slot.ptr(0x28); }

// 0047BBA0: MOV ECX,[ESP+4]; MOV EAX,[ECX]; MOV EDX,[EAX+8]; CALL EDX; RET 4.
// The caller's ECX ([game+21D0h] at 004E9EF6) is overwritten unread.
void* forward_object_vslot_08_0047bba0(Raw, void* object, Front6Host& host) {
    return host.object_vslot_08(object);
}

// ---------------------------------------------------------------------------
// 0077C160 BSP_NetEntity_WriteReplicationState, __thiscall(ECX = entity; archive) RET 4.
// 00928450(entity; archive), then "_netEntity" (00D03E20) and "_reconData"
// (00D03E14) are entered, 00805020(ECX = entity+1E4h; archive) writes the recon
// set, and one leave closes "_reconData". "hasPlrCmd" (00D03E08) = {3, byte +290h}.
// When [entity+284h] is set and its +14h is the entity: enter "formation"
// (00D03DFC), 0070E890(ECX = [entity+284h]; archive), leave. Enter "lastBanto"
// (00D03DF0), write "id" (00CF16BC) = {1, word +2C8h zero-extended}, leave;
// "lastBantoPlayerIndex" (00D03DD8) = {1, dword +2D8h}; the final leave closes
// "_netEntity". The type-3 value's upper three payload bytes are stack bytes the
// image does not write; they are zero here.
// ---------------------------------------------------------------------------
void net_entity_write_replication_state_0077c160(Raw entity, void* archive, Front6Host& host) {
    host.game_entity_write_archive_00928450(entity.p, archive);                    // 0077C16A
    host.archive_enter_04(archive, key(0x00D03E20, host));                          // 0077C188
    host.archive_enter_04(archive, key(0x00D03E14, host));                          // 0077C1A3
    host.recon_detection_set_write_archive_00805020(entity.field(0x1E4), archive);  // 0077C1AC
    host.archive_leave_08(archive);                                                 // 0077C1B8
    host.archive_write_0c(archive, key(0x00D03E08, host),
                          TaggedValue{3, entity.at<std::uint8_t>(0x290)});          // 0077C1E7
    std::uint8_t* const formation = static_cast<std::uint8_t*>(entity.ptr(0x284));
    if (formation != nullptr && Raw{formation}.ptr(0x14) == entity.p) {
        host.archive_enter_04(archive, key(0x00D03DFC, host));                      // 0077C208
        host.observer_member_write_ref_0070e890(formation, archive);                // 0077C20D
        host.archive_leave_08(archive);                                             // 0077C219
    }
    host.archive_enter_04(archive, key(0x00D03DF0, host));                          // 0077C22B
    host.archive_write_0c(archive, key(0x00CF16BC, host),
                          TaggedValue{1, entity.at<std::uint16_t>(0x2C8)});         // 0077C255
    host.archive_leave_08(archive);                                                 // 0077C25E
    host.archive_write_0c(archive, key(0x00D03DD8, host),
                          TaggedValue{1, entity.at<std::uint32_t>(0x2D8)});         // 0077C28D
    host.archive_leave_08(archive);                                                 // 0077C296
}

// ---------------------------------------------------------------------------
// 005D7070, __fastcall(ECX = entry index) RET -> EAX.
// count = [[00E188A8]+5F0h] (signed); index >= count -> 0. node = 004BE940(ECX =
// game+5F0h; index); kind = [node+1098h], 29h -> 26h, 2Ah -> 27h. Records: the
// 434h-byte vector [[00E198AC]+5Ch]+28h/+2Ch, each beginning with a NativeString
// {length, data}. For i = 0.. : stop with 0 when the vector is null or i >=
// size (signed byte difference / 434h, compared unsigned); the inlined checked
// index re-tests and calls 00BF6713; the key = 004260B0(&local; kind); equal
// lengths and (both empty, or _stricmp(record data, key data) == 0) match; the
// local is released (00419CC0 / 00BD1510(data, length + 1, 1)) when its data is
// set; a match returns i. The front-end manager and its menu are re-read each pass.
// ---------------------------------------------------------------------------
std::uint32_t find_menu_record_for_entry_kind_005d7070(std::int32_t entry_index, Front6Host& host) {
    std::uint8_t* const list = static_cast<std::uint8_t*>(host.global_pointer(0x00E188A8)) + 0x5F0;
    if (*reinterpret_cast<std::int32_t*>(list) <= entry_index) return 0;           // 005D7085
    const Raw node{host.linked_list_nth_node_004be940(list, entry_index)};          // 005D708C
    std::int32_t kind = node.at<std::int32_t>(0x1098);
    if (kind == 0x29) {
        kind = 0x26;
    } else if (kind == 0x2A) {
        kind = 0x27;
    }
    std::uint32_t i = 0;
    std::uint32_t offset = 0;
    for (;;) {
        const Raw menu{static_cast<std::uint8_t*>(
            Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198AC))}.ptr(0x5C))};
        auto size_of = [&]() {
            const std::int32_t bytes = static_cast<std::int32_t>(
                addr32(menu.ptr(0x2C)) - addr32(menu.ptr(0x28)));
            return static_cast<std::uint32_t>(bytes / 0x434);
        };
        if (menu.ptr(0x28) == nullptr || i >= size_of()) return 0;                 // 005D70CD / 005D70EB
        if (menu.ptr(0x28) == nullptr || i >= size_of()) {
            host.invalid_parameter_00bf6713();                                      // 005D7112
        }
        const Raw record{static_cast<std::uint8_t*>(menu.ptr(0x28)) + offset};
        NativeStr local{};
        const NativeStr* const key_string = host.native_string_from_int_004260b0(&local, kind);  // 005D7125
        bool match;
        const std::uint32_t record_length = record.at<std::uint32_t>(0);
        const std::uint32_t key_length = key_string->length;
        if (record_length != key_length) {
            match = false;
        } else {
            std::int32_t cmp;
            if (record_length == 0) {
                cmp = key_length != 0 ? -1 : 0;
            } else if (key_length == 0) {
                cmp = 1;
            } else {
                cmp = host.stricmp_00bf7fbf(static_cast<const char*>(record.ptr(4)), key_string->data);  // 005D7153
            }
            match = cmp == 0;
        }
        if (local.data != nullptr) {
            void* const pool = host.sized_pool_00419cc0();                          // 005D7173
            host.sized_pool_return_00bd1510(pool, local.data, local.length + 1, 1);   // 005D717A
        }
        if (match) return i;
        ++i;
        offset += 0x434;
    }
}

// ---------------------------------------------------------------------------
// 00A04240 BSP_AiHintWeight_EntityMultiplier, __fastcall(ECX = entity) RET -> ST0.
// key = the entity pointer. w = 1.0f (00D7A24C). it = 00A03B20(ECX = 00F8A740;
// &it, &key); the inlined checked compare with end ([00F8A744]) calls 00BF6713
// when it.container is null or not the map; when it.node != end (and after the
// two dereference checks, container null / node == [container+4]) w = [node+10h].
// Then m = 00F8A750 + 0Ch * [00E0E344]: the same find on m, end = [m+4]; when
// found w = float(w * [node+10h]) (FMUL m32, FSTP m32: one float rounding at any
// precision control). The first find's result is read from the local iterator,
// the second through the returned pointer (EAX), as the image does.
// ---------------------------------------------------------------------------
float ai_hint_weight_entity_multiplier_00a04240(void* entity, Front6Host& host) {
    const std::uint32_t key_value = addr32(entity);
    float weight = 1.0f;                                                            // 00A04243
    void* const map = host.global_address(0x00F8A740);
    MapIterator it{};
    host.map_find_00a03b20(map, &it, &key_value);                                   // 00A04267
    const std::uint8_t* const end = *static_cast<std::uint8_t**>(host.global_address(0x00F8A744));
    if (it.container == nullptr || it.container != map) host.invalid_parameter_00bf6713();  // 00A04282
    if (it.node != end) {
        if (it.container == nullptr) host.invalid_parameter_00bf6713();             // 00A04293
        if (it.node == Raw{static_cast<std::uint8_t*>(it.container)}.ptr(4)) {
            host.invalid_parameter_00bf6713();                                      // 00A0429D
        }
        weight = Raw{it.node}.at<float>(0x10);
    }
    const std::int32_t party = *static_cast<std::int32_t*>(host.global_address(0x00E0E344));
    std::uint8_t* const party_map = static_cast<std::uint8_t*>(host.global_address(0x00F8A750)) + party * 0xC;
    MapIterator it2{};
    const MapIterator* const found = host.map_find_00a03b20(party_map, &it2, &key_value);  // 00A042C9
    void* const container = found->container;
    std::uint8_t* const node = found->node;
    std::uint8_t* const party_end = static_cast<std::uint8_t*>(Raw{party_map}.ptr(4));
    if (container == nullptr || container != party_map) host.invalid_parameter_00bf6713();  // 00A042DE
    if (node != party_end) {
        if (container == nullptr) host.invalid_parameter_00bf6713();                // 00A042EC
        if (node == Raw{static_cast<std::uint8_t*>(container)}.ptr(4)) {
            host.invalid_parameter_00bf6713();                                      // 00A042F6
        }
        weight = Raw{node}.at<float>(0x10) * weight;
    }
    return weight;
}

}  // namespace bsp::cc13_front6
