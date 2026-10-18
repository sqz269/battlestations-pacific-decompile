// cc13_left1: leftover game bodies with no host (packet cc13-left1, docs/CC13_LEFT1.md). Names
// are hypotheses; see the header for the hooks. Each body was transcribed from its full export
// listing.

#include "bsp/cc13_left1.hpp"
#include "bsp/cc13_camera_controllers.hpp"

#include <array>

namespace bsp::cc13_left1 {

// ---------------------------------------------------------------------------
// 005B8810 BSP_AmbientMessage_PlaySound (provisional), __thiscall(ECX = object; clip by value at
// ESP+4..+0Fh, entity at +10h), RET 10h, EAX = object; SEH handler 00C7215B, EH states 0..2.
// Evidence, listing 005B8810..005B89EE:
//  - 005B8830..005B8859: [+4h]..[+10h] = 0 and [+0h] = 0 (+14h is not written), the observer
//    sub-object at +18h: [+1Ch]/[+20h]/[+24h]/[+2Ch] = 0, byte [+28h] = 1, [+18h] = 00CE75CC.
//  - 005B8863..005B8884: the inlined set-observed with the old value a constant 0 (ECX = EBX):
//    nothing when entity == 0; 006952A0 is unreachable (Ghidra drops 005B8872); [+2Ch] = entity,
//    then 00694A60(ECX = entity, EDX = object+18h).
//  - 005B8889..005B88B6: 00702CC0(ECX = temporary slot, &clip, owned resource 0), 005B82C0(ECX =
//    object, EAX), 005B7FC0(temporary).
//  - 005B88BD: 007027B0(object) == 0: the "NOT FOUND" trace 00CF0E40 with ([record+4h] or
//    00E1952E, clip index), then return.
//  - 005B88F7..005B8977: camera = [[00E188A8]+19FCh]; 00B6DB70(camera) when byte camera+5Ch has
//    bit 2 clear; camera +120h..+128h copied; 00414DB0(entity) when byte entity+C8h is 0;
//    d = entity+FCh..+104h - camera position (FSUB, FSTP float each); 0042B2F0(&d).
//  - 005B897C..005B89AF, bytes DD 05 D8 0D CF 00 / D9 C0 / DE E2 (FSUBRP ST2,ST0) / DE F9
//    (FDIVP ST1,ST0): v = (2000.0 - length) / 2000.0, FSTP float; FCOMIP with 0.25f [00CE3868],
//    JBE keeps v (so NaN is kept), else v = +0.0; 00702130(object, v).
//  - 005B89B4..005B89D2: the "played" trace 00CF0E14 with the same two arguments.
// Uncertainty: modelled at 24-bit x87 precision (each operation rounds to float); at 53-bit the
// quotient would round once more on the float store. The only caller is not established (no
// rel32 or absolute reference); the object is the 30h element 005B83E0 walks at owner+64h.
// ---------------------------------------------------------------------------
void* ambient_message_voice_construct_005b8810(Left1Host& host, void* object, AmbientClip12 clip,
                                               void* entity) {
    at<std::uint32_t>(object, 0x04) = 0;
    at<std::uint32_t>(object, 0x08) = 0;
    at<std::uint32_t>(object, 0x0C) = 0;
    at<std::uint32_t>(object, 0x10) = 0;
    at<std::uint32_t>(object, 0x00) = 0;
    void* const observer = add(object, 0x18);
    at<std::uint32_t>(observer, 0x04) = 0;
    at<std::uint32_t>(observer, 0x08) = 0;
    at<std::uint32_t>(observer, 0x0C) = 0;
    at<std::uint32_t>(observer, 0x14) = 0;
    at<std::uint8_t>(observer, 0x10) = 1;
    at<std::uint32_t>(observer, 0x00) = kAmbientObserverVtable;

    void* const old_target = nullptr;  // 005B8863 MOV ECX,EBX
    if (old_target != entity) {
        if (old_target != nullptr) host.observer_unregister_pair_006952a0(old_target, observer);
        at<void*>(observer, 0x14) = entity;
        if (entity != nullptr) host.observer_register_pair_00694a60(entity, observer);
    }

    alignas(4) std::uint8_t temporary[0x18];
    void* const started = host.voice_slot_construct_and_start_00702cc0(temporary, &clip, 0);
    host.voice_slot_assign_005b82c0(object, started);
    host.voice_slot_destroy_005b7fc0(temporary);

    // 005B88C6 / 005B89B4: the text is read at each trace, after the calls before it.
    const auto trace = [&](const char* format) {
        const char* const text = at<const char*>(clip.record, 4);
        const std::uint32_t args[2] = {
            text != nullptr ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text)) : kEmptyText,
            static_cast<std::uint32_t>(clip.index)};
        host.trace_printf_004254b0(format, args, 2);
    };
    if (host.voice_slot_poll_state_007027b0(object) == 0) {
        trace("Ambient message sound NOT FOUND for: %s (%d)");
        return object;
    }

    void* const camera = ptr(host.game_global(), 0x19FC);
    if ((at<std::uint8_t>(camera, 0x5C) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(camera);
    const float camera_position[3] = {at<float>(camera, 0x120), at<float>(camera, 0x124),
                                      at<float>(camera, 0x128)};
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float d[3] = {at<float>(entity, 0xFC) - camera_position[0],
                        at<float>(entity, 0x100) - camera_position[1],
                        at<float>(entity, 0x104) - camera_position[2]};
    const float length = host.vector3_length_threshold_0042b2f0(d);
    const float fade = static_cast<float>(kAmbientFadeDistance);
    float volume = (fade - length) / fade;
    if (kAmbientMinimumVolume > volume) volume = 0.0f;
    host.voice_slot_set_volume_00702130(object, volume);
    trace("Ambient message sound played for: %s (%d)");
    return object;
}

// ---------------------------------------------------------------------------
// 00605630 BSP_Math_DirectionAngleDeltas (provisional), __fastcall(ECX = float* yaw, EDX =
// float* pitch; a.xyz at ESP+4, b.xyz at ESP+10h), RET 18h. Sole caller 006082D0
// BSP_PlaneHudScreen_UpdateBombFire.
// Evidence: listing 00605630..006057DA, compared instruction by instruction with 00794750 (the
// path camera's aim deltas): after normalising both mnemonic streams, the only differences are
// the prologue's MOV EDI,EDX and the output stores (00605752 FSTP [ESI], 00605759 MOVSS [ESI],
// 006057C4 FSTP [EDI], 006057D1 MOVSS [EDI] here; a local pair copied to [ESI]/[ESI+4] there).
// Bytes checked here: 00605652 D9 E8 DE F1 and 006056A4 D9 E8 DE F1 (FLD1; FDIVRP ST1,ST0 = 1 /
// length); 00605673 D9 C0 DE CA (FMULP ST2,ST0). So yaw = 00438B10(atan2(b.x, b.z), atan2(a.x,
// a.z)) over the normalised vectors and pitch = -00438B10(0042CF10(b.y), 0042CF10(a.y)), each 0
// when either angle fails 00BFA4EC; camera_direction_delta_00794750 computes exactly that.
// Uncertainty: the image writes *yaw before the two 0042CF10 calls, the reuse writes both after
// them (the callees touch no caller memory). Precision as documented for 00794750.
// ---------------------------------------------------------------------------
void direction_angle_deltas_00605630(float* yaw, float* pitch, const float a[3], const float b[3]) {
    const std::array<float, 2> delta =
        cc13cam::camera_direction_delta_00794750({a[0], a[1], a[2]}, {b[0], b[1], b[2]});
    *yaw = delta[1];
    *pitch = delta[0];
}

// ---------------------------------------------------------------------------
// 006D5220 helpers.
// ---------------------------------------------------------------------------
namespace {

constexpr std::size_t kHangarData = 0x830;      // Hangar* (12-byte records)
constexpr std::size_t kHangarSize = 0x834;
constexpr std::size_t kHangarCapacity = 0x838;
constexpr std::size_t kAirOpsBlock = 0x72C;
constexpr std::uint32_t kReaderVtable = 0x00CE44FCu;
constexpr std::uint32_t kVisitorBaseVtable = 0x00CE374Cu;
constexpr std::uint32_t kTypeNameTable = 0x00E08138u;  // const char*[part type]

std::uint32_t text_word(const char* text) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text));
}
std::uint32_t address_word(const void* p) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
std::uint8_t* hangar_slot(void* airfield, std::uint32_t index) noexcept {
    // The image adds without a null test of the base; only the sum is tested.
    return reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(address_word(ptr(airfield, kHangarData)) + index * 0x0Cu));
}

// The inlined resize of 006D539B..006D545F (kind 3) and 006D56BE..006D5786 (kind 2): grow to
// `count` records when count > size (the new block holds `bytes`, a fresh capacity of count),
// value-initialise the added records, then size = count in every case.
void hangar_resize(Left1Host& host, void* airfield, std::uint32_t count, std::uint32_t bytes) {
    if (count > at<std::uint32_t>(airfield, kHangarSize)) {
        if (count > at<std::uint32_t>(airfield, kHangarCapacity)) {
            at<std::uint32_t>(airfield, kHangarCapacity) = count;
            auto* const block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(bytes));
            for (std::uint32_t k = 0; k < at<std::uint32_t>(airfield, kHangarSize); ++k) {
                auto* const to = reinterpret_cast<std::uint8_t*>(
                    static_cast<std::uintptr_t>(address_word(block) + k * 0x0Cu));
                if (to == nullptr) continue;
                const std::uint8_t* const from = hangar_slot(airfield, k);
                at<std::uint32_t>(to, 0) = at<std::uint32_t>(const_cast<std::uint8_t*>(from), 0);
                at<std::uint32_t>(to, 4) = at<std::uint32_t>(const_cast<std::uint8_t*>(from), 4);
                at<std::uint32_t>(to, 8) = at<std::uint32_t>(const_cast<std::uint8_t*>(from), 8);
            }
            if (void* const old = ptr(airfield, kHangarData); old != nullptr) host.free_00bf6989(old);
            at<void*>(airfield, kHangarData) = block;
        }
        const std::uint32_t size = at<std::uint32_t>(airfield, kHangarSize);
        if (size < count) {
            for (std::uint32_t k = size; k != count; ++k) {
                std::uint8_t* const record = hangar_slot(airfield, k);
                if (record == nullptr) continue;
                at<std::uint32_t>(record, 0) = 0;
                at<std::uint32_t>(record, 4) = 0;
                at<std::uint32_t>(record, 8) = 0;
            }
        }
    }
    at<std::uint32_t>(airfield, kHangarSize) = count;
}

// 006D57AA..006D57DC (and the two copies after it): the global record map, inlined. Ids below
// [00F89A10] index the table [00F89A54] from [00F89A0C], the rest the table [00F89AA8] from
// [00F89A60]; stride 10h, the dword at +0Ch. 32-bit wrapping arithmetic as in the image.
std::uint32_t record_map_value(Left1Host& host, std::uint16_t id) {
    const std::int32_t value = id;
    std::uint32_t base;
    std::uint32_t table;
    if (value < static_cast<std::int32_t>(host.image_dword(0x00F89A10u))) {
        base = host.image_dword(0x00F89A0Cu);
        table = host.image_dword(0x00F89A54u);
    } else {
        base = host.image_dword(0x00F89A60u);
        table = host.image_dword(0x00F89AA8u);
    }
    const std::uint32_t address = ((static_cast<std::uint32_t>(value) - base) << 4) + table + 0x0Cu;
    return *reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(address));
}

void release_native_string(Left1Host& host, void* string) {
    if (ptr(string, 0x04) != nullptr) {
        void* const pool = host.sized_pool_singleton_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, ptr(string, 0x04), at<std::uint32_t>(string, 0x00) + 1u, 1u);
    }
}

// The inlined reader destructor: vtable 00CE44FC, then the LuaObject vector at +4h.
void destroy_reader(Left1Host& host, void* reader) {
    at<std::uint32_t>(reader, 0) = kReaderVtable;
    host.lua_object_vector_tidy_00441210(add(reader, 4));
}

// Kind 3, 006D5278..006D55A5: the saved Lua data.
void read_kind3_lua(Left1Host& host, void* airfield) {
    alignas(4) std::uint8_t lua_object[0x14] = {};
    host.saved_entity_lua_data_009238a0(airfield, lua_object);
    alignas(4) std::uint8_t reader[0x14] = {};
    host.lua_reader_construct_004425c0(reader, lua_object);
    host.lua_reader_enter_key_00bd8e20(reader, 0, text_word("_airField"));  // 00CF8EA0
    // Frame floats S+94h / S+24h and references S+34h / S+2Ch / S+18h: the image leaves them
    // uninitialised and a missing key keeps the previous value; zero here.
    float width = 0.0f;
    float length = 0.0f;
    host.lua_reader_read_field_00bd6830(reader, 0, text_word("runwayWidth"), 2, address_word(&width));    // 00CF8E94
    host.lua_reader_read_field_00bd6830(reader, 0, text_word("runwayLength"), 2, address_word(&length));  // 00CF8E84
    host.air_ops_block_set_runway_dimensions_006bf0d0(add(airfield, kAirOpsBlock), width, length);
    host.lua_reader_enter_key_00bd8e20(reader, 0, text_word("hangars"));    // 00CF8E7C
    std::uint32_t offset = 0;                                                // S+14h
    std::uint32_t entity_id = 0;
    std::uint32_t entry_path = 0;
    std::uint32_t exit_path = 0;
    for (std::uint32_t i = 1;; ++i, offset += 0x0C) {
        if (!host.lua_reader_has_key_00bd5eb0(reader, 1, i - 1)) break;
        hangar_resize(host, airfield, i, offset + 0x0C);
        std::uint8_t* const record = static_cast<std::uint8_t*>(ptr(airfield, kHangarData)) + offset;
        host.lua_reader_enter_key_00bd8e20(reader, 1, i - 1);
        host.reader_vslot_10(reader, 0, text_word("entityID"), 4, address_word(&entity_id));      // 00CF8E70
        at<std::uint32_t>(record, 0) = entity_id;
        host.reader_vslot_10(reader, 0, text_word("entryPathID"), 4, address_word(&entry_path));  // 00CF8E64
        at<void*>(record, 4) = host.entity_path_interface_007ac9d0(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(entry_path)));
        host.reader_vslot_10(reader, 0, text_word("exitPathID"), 4, address_word(&exit_path));    // 00CF8E58
        at<void*>(record, 8) = host.entity_path_interface_007ac9d0(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(exit_path)));
        host.lua_reader_leave_key_00bd7a20(reader);
    }
    host.lua_reader_leave_key_00bd7a20(reader);
    void* const director = ptr(airfield, 0x8C8);
    host.reader_vslot_04(reader, 0, text_word("gameUnit"));  // 00CF8E4C
    host.weapon_director_visit_enables_00836510(director, reader);
    host.reader_vslot_08(reader);
    host.lua_reader_leave_key_00bd7a20(reader);
    destroy_reader(host, reader);
}

// Kind 2, 006D56B5..006D589A: the compiled table, three ushort ids per hangar (stride 6).
void read_kind2_table(Left1Host& host, void* airfield, void* source) {
    const std::uint32_t count = at<std::uint32_t>(source, 0x138);
    hangar_resize(host, airfield, count, count * 0x0Cu);
    if (at<std::int32_t>(source, 0x138) <= 0) return;
    std::int32_t k = 0;
    do {
        const std::uint32_t data = address_word(ptr(airfield, kHangarData));
        auto* const record = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(data + k * 0x0Cu));
        at<std::uint32_t>(record, 0) =
            record_map_value(host, at<std::uint16_t>(ptr(source, 0x13C), k * 6u + 0));
        at<void*>(record, 4) = host.entity_path_interface_007ac9d0(reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            record_map_value(host, at<std::uint16_t>(ptr(source, 0x13C), k * 6u + 2)))));
        at<void*>(record, 8) = host.entity_path_interface_007ac9d0(reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            record_map_value(host, at<std::uint16_t>(ptr(source, 0x13C), k * 6u + 4)))));
        ++k;
    } while (k < at<std::int32_t>(source, 0x138));
}

void refresh_pose(Left1Host& host, void* airfield) {
    if (at<std::uint8_t>(airfield, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(airfield);
}
void* scene_bag(void* airfield) { return ptr(ptr(airfield, 0xC0), 0x08); }

// Kind 1, 006D589F..006D600A: the scene property bag.
void read_kind1_scene(Left1Host& host, void* airfield) {
    // 006D589F..006D5924: CaptainCameraPos, default the position + (0, 20.0 [00CE3D88], 0). Bytes
    // D9 EE ... DC C1 (FADD ST1,ST0) / D9 C9: x + 0.0 and 0.0 + z, each rounded to float.
    refresh_pose(host, airfield);
    const float position_default[3] = {at<float>(airfield, 0xFC) + 0.0f, at<float>(airfield, 0x100) + 20.0f,
                                       0.0f + at<float>(airfield, 0x104)};
    float captain[3];
    const float* const captain_out = host.scene_position_from_named_entity_006d2190(
        captain, scene_bag(airfield), "CaptainCameraPos", position_default);  // 00CF8F40
    at<float>(airfield, 0x8CC) = captain_out[0];
    at<float>(airfield, 0x8D0) = captain_out[1];
    at<float>(airfield, 0x8D4) = captain_out[2];

    // 006D592A..006D59B4: AirBaseCameraPos, default (-30, 50, -100) [00CF8F3C/00CEB4D4/00CE65D8]
    // through the world matrix +0CCh.
    refresh_pose(host, airfield);
    const float camera_local[3] = {-30.0f, 50.0f, -100.0f};
    float camera_default[3];
    host.transform_affine_point_004142e0(camera_local, camera_default, add(airfield, 0xCC));
    float camera[3];  // S+3Ch
    host.scene_position_from_named_entity_006d2190(camera, scene_bag(airfield), "AirBaseCameraPos",
                                                   camera_default);  // 00CF8F28

    // 006D59B9..006D5A34: AirBaseLookAtPos, default the transformed origin.
    refresh_pose(host, airfield);
    const float origin_local[3] = {0.0f, 0.0f, 0.0f};
    float look_default[3];
    host.transform_affine_point_004142e0(origin_local, look_default, add(airfield, 0xCC));
    float look[3];  // S+48h
    host.scene_position_from_named_entity_006d2190(look, scene_bag(airfield), "AirBaseLookAtPos",
                                                   look_default);  // 00CF8F14

    // 006D5A39..006D5A97: d = camera - look; bytes D9 C1 DE CA D9 C2 DE CB D9 C9 DE C2 DC C8 DE C1:
    // (dy*dy + dx*dx) + dz*dz, FSTP float; FLD1 / FCOMIP, JBE: when 1.0 > d2 (NaN: no),
    // camera.x += 1.0 [00D7A210].
    const float dx = camera[0] - look[0];
    const float dy = camera[1] - look[1];
    const float dz = camera[2] - look[2];
    const float d2 = (dy * dy + dx * dx) + dz * dz;
    if (1.0f > d2) camera[0] = camera[0] + 1.0f;

    // 006D5A99..006D5BEE: both points into the airfield's local frame (00414E10 each time), the
    // matrix +888h set to identity (004134F0), forward row +8A8h = look - camera, up row +898h =
    // (0, 1, 0), 0085DC80, translation row +8B8h = the local camera.
    float camera_in_local[3];  // S+88h
    host.transform_affine_point_004142e0(camera, camera_in_local,
                                         host.entity_pose_derived_affine_inverse_00414e10(airfield));
    float look_in_local[3];    // S+3Ch
    host.transform_affine_point_004142e0(look, look_in_local,
                                         host.entity_pose_derived_affine_inverse_00414e10(airfield));
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    void* const view = add(airfield, 0x888);
    host.matrix_copy_4x4_004134f0(view, identity);
    const float fx = look_in_local[0] - camera_in_local[0];
    const float fy = look_in_local[1] - camera_in_local[1];
    const float fz = look_in_local[2] - camera_in_local[2];
    at<float>(airfield, 0x8A8) = fx;
    at<float>(airfield, 0x8AC) = fy;
    at<float>(airfield, 0x8B0) = fz;
    at<float>(airfield, 0x898) = 0.0f;
    at<float>(airfield, 0x89C) = 1.0f;  // 00D7A24C
    at<float>(airfield, 0x8A0) = 0.0f;
    host.matrix_orthonormalize_basis_rows_0085dc80(view);
    at<float>(airfield, 0x8B8) = camera_in_local[0];
    at<float>(airfield, 0x8BC) = camera_in_local[1];
    at<float>(airfield, 0x8C0) = camera_in_local[2];

    // 006D5BF6..006D5DD3: "Hangar %d" for i = 1..10 (00CF8F08).
    for (std::int32_t i = 1; i <= 10; ++i) {
        char name[32];  // S+48h, 00467310 writes at most 32 bytes
        host.format_to_buffer32_00467310(name, "Hangar %d", i);
        void* const section = host.scene_property_bag_find_008f2260(scene_bag(airfield), name);
        if (section == nullptr || at<std::int32_t>(section, 4) != 6) continue;
        void* const bag = ptr(section, 0x0C);
        if (bag == nullptr) continue;
        void* const part = host.entity_nth_part_of_type_0c_006d2120(airfield, i - 1);  // S+14h
        const char* const object_name = host.scene_property_string_value_008f0df0(
            host.scene_property_bag_find_008f2260(bag, "Object"));  // 00CF8F00
        if (object_name == nullptr || *object_name == '\0') continue;
        void* const entity = host.entity_registry_find_by_name_00925a90(ptr(airfield, 0x30), object_name);
        if (entity == nullptr) continue;
        if (!host.entity_vslot_5c_is_kind(entity, 0x1B)) continue;
        if (part == nullptr) continue;
        // 006D5CA7..006D5D66: the inlined push_back (capacity 2n + 2 when full).
        if (at<std::uint32_t>(airfield, kHangarSize) == at<std::uint32_t>(airfield, kHangarCapacity)) {
            const std::uint32_t capacity = at<std::uint32_t>(airfield, kHangarCapacity) * 2u + 2u;
            at<std::uint32_t>(airfield, kHangarCapacity) = capacity;
            auto* const block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * 0x0Cu));
            for (std::uint32_t k = 0; k < at<std::uint32_t>(airfield, kHangarSize); ++k) {
                auto* const to = reinterpret_cast<std::uint8_t*>(
                    static_cast<std::uintptr_t>(address_word(block) + k * 0x0Cu));
                if (to == nullptr) continue;
                std::uint8_t* const from = hangar_slot(airfield, k);
                at<std::uint32_t>(to, 0) = at<std::uint32_t>(from, 0);
                at<std::uint32_t>(to, 4) = at<std::uint32_t>(from, 4);
                at<std::uint32_t>(to, 8) = at<std::uint32_t>(from, 8);
            }
            if (void* const old = ptr(airfield, kHangarData); old != nullptr) host.free_00bf6989(old);
            at<void*>(airfield, kHangarData) = block;
        }
        if (std::uint8_t* const slot = hangar_slot(airfield, at<std::uint32_t>(airfield, kHangarSize));
            slot != nullptr) {
            at<std::uint32_t>(slot, 0) = 0;
            at<std::uint32_t>(slot, 4) = 0;
            at<std::uint32_t>(slot, 8) = 0;
        }
        at<std::uint32_t>(airfield, kHangarSize) += 1;
        std::uint8_t* const record = hangar_slot(airfield, at<std::uint32_t>(airfield, kHangarSize) - 1u);
        at<void*>(record, 0) = host.entity_as_kind1b_006d3440(entity);
        at<void*>(record, 4) = host.entity_path_interface_007ac9d0(host.entity_registry_find_by_name_00925a90(
            ptr(airfield, 0x30), host.scene_property_string_value_008f0df0(
                                     host.scene_property_bag_find_008f2260(bag, "EntryPath"))));  // 00CF8EF4
        at<void*>(record, 8) = host.entity_path_interface_007ac9d0(host.entity_registry_find_by_name_00925a90(
            ptr(airfield, 0x30), host.scene_property_string_value_008f0df0(
                                     host.scene_property_bag_find_008f2260(bag, "ExitPath"))));   // 00CF8EE8
        host.land_fort_set_master_00744a20(ptr(record, 0), airfield, part);
    }

    // 006D5DD9..006D600A: every part of [+348h, +34Ch) whose type (+4h) is not 0Bh or 0Ch takes
    // the scene object "Section_<type name>_<part+8h>" as its master's fort.
    for (std::uint32_t k = 0;; ++k) {
        if (ptr(airfield, 0x348) == nullptr) break;
        if (k >= static_cast<std::uint32_t>((at<std::int32_t>(airfield, 0x34C) - at<std::int32_t>(airfield, 0x348)) >> 2))
            break;
        if (ptr(airfield, 0x348) == nullptr ||
            k >= static_cast<std::uint32_t>((at<std::int32_t>(airfield, 0x34C) - at<std::int32_t>(airfield, 0x348)) >> 2))
            host.invalid_parameter_00bf6713();
        void** const element = static_cast<void**>(ptr(airfield, 0x348)) + k;
        const std::int32_t type = at<std::int32_t>(*element, 4);
        if (type == 0x0B || type == 0x0C) continue;
        // Strings S+2Ch "_", S+18h type name, S+A8h "Section_", S+A0h, S+98h, S+34h; each header
        // is uninitialised in the image before 0041E870 / the concatenations write it.
        alignas(4) std::uint8_t underscore[8] = {};
        alignas(4) std::uint8_t type_name[8] = {};
        alignas(4) std::uint8_t section[8] = {};
        alignas(4) std::uint8_t joined1[8] = {};
        alignas(4) std::uint8_t joined2[8] = {};
        alignas(4) std::uint8_t full[8] = {};
        host.native_string_assign_0041e870(underscore, "_");  // 00CE7890
        const std::uint32_t type_text =
            host.image_dword(kTypeNameTable + static_cast<std::uint32_t>(at<std::int32_t>(*element, 4)) * 4u);
        host.native_string_assign_0041e870(type_name,
                                           reinterpret_cast<const char*>(static_cast<std::uintptr_t>(type_text)));
        void* const s3 = host.native_string_assign_0041e870(section, "Section_");  // 00CF8EDC
        void* const t1 = host.native_string_concat_004261a0(s3, joined1, type_name);
        void* const t2 = host.native_string_concat_004261a0(t1, joined2, underscore);
        host.native_string_concat_int_004263b0(t2, full, at<std::int32_t>(*element, 8));
        release_native_string(host, joined2);
        release_native_string(host, joined1);
        release_native_string(host, section);
        release_native_string(host, type_name);
        release_native_string(host, underscore);
        const char* text = static_cast<const char*>(ptr(full, 4));
        if (text == nullptr) text = "";  // 00E1996C, the image's ""
        void* const entity = host.entity_registry_find_by_name_00925a90(
            ptr(airfield, 0x30),
            host.scene_property_string_value_008f0df0(host.scene_property_bag_find_008f2260(scene_bag(airfield), text)));
        host.land_fort_set_master_00744a20(entity, airfield, element);
        release_native_string(host, full);
    }
}

// 006D55B0..006D56B4: the common tail.
void airfield_common_tail(Left1Host& host, void* airfield) {
    void* const block = add(airfield, kAirOpsBlock);
    host.air_ops_block_trivial_006ba690(block, host.airfield_site_create_006cfa80(airfield));
    void* const source = ptr(airfield, 0xC0);
    if (source != nullptr && at<std::int32_t>(source, 4) == 3) {
        alignas(4) std::uint8_t lua_object[0x14] = {};
        host.saved_entity_lua_data_009238a0(airfield, lua_object);
        alignas(4) std::uint8_t reader[0x14] = {};  // S+B0h
        host.lua_reader_construct_004425c0(reader, lua_object);
        host.lua_reader_enter_key_00bd8e20(reader, 0, text_word("_airBase"));  // 00CF88BC
        host.air_ops_block_read_traffic_table_006bef30(block, reader);
        host.lua_reader_leave_key_00bd7a20(reader);
        destroy_reader(host, reader);
        at<std::uint32_t>(reader, 0) = kVisitorBaseVtable;  // 006D566F
    }
    host.air_ops_block_tick_006ba670(block, 0.0f);
    host.airfield_rebuild_aim_points_006d2980(airfield);
    host.air_ops_holder_copy_owner_frame_006bee40(ptr(airfield, 0x7AC), 0.0f);
}

}  // namespace

// ---------------------------------------------------------------------------
// 006D5220 BSP_AirField_ReadHangarAndMarkerProperties (provisional), __fastcall(ECX = airfield),
// RET; SEH handler 00C81EEF (EH states 0..0Eh: the reader at S+48h is 0Bh/0Ch, the "_airBase"
// reader at S+B0h 0Dh/0Eh, the section strings 0..0Ah). Listing 006D5220..006D600E read whole.
// Evidence:
//  - 006D5247 0095E5B0(airfield); 006D524C..006D5272: [airfield+0C0h] null -> tail; its +4h kind
//    1 -> 006D589F, 2 -> 006D56B5, 3 -> 006D5278, anything else -> the tail 006D55B0.
//  - Kind 3: the reader over 009238A0's LuaObject (004425C0), "_airField" (00BD8E20), the two
//    {0, name} / {2, &float} pairs runwayWidth / runwayLength (00BD6830) into 006BF0D0(+72Ch),
//    "hangars": while 00BD5EB0 finds index i - 1, the inlined resize to i records, then
//    entityID / entryPathID / exitPathID through vtable +10h with {4, &ref}; the paths through
//    007AC9D0. Then "gameUnit" through vtable +4h, 00836510([+8C8h], reader), vtable +8h, the
//    leaves 00BD7A20 and the inlined destructor (00CE44FC, 00441210).
//  - Kind 2: count [[+0C0h]+8]+138h, the inlined resize to count, three ushort ids per hangar
//    (stride 6) through the inlined global record map; object raw, both paths through 007AC9D0.
//  - Kind 1: see read_kind1_scene (CaptainCameraPos +8CCh, the view matrix +888h from
//    AirBaseCameraPos / AirBaseLookAtPos, "Hangar %d" 1..10 into the record vector with
//    00744A20, and the "Section_" part masters).
//  - Tail: 006BA690(+72Ch, 006CFA80()), "_airBase" 006BEF30 for kind 3 (that reader's vtable
//    reset to 00CE374C after the tidy), 006BA670(+72Ch, 0.0f), 006D2980, 006BEE40([+7ACh], 0.0f).
// Uncertainty: the frame floats/references of kind 3 and the section string headers are
// uninitialised in the image (zero here); a missing Lua key leaves the previous iteration's
// reference in place in both. Kind 1 arithmetic is modelled at 24-bit x87 precision (the
// constants 20.0 and 1.0 are exact, so float operations match); 53-bit is not modelled. The
// 006D5E14 00BF6713 check cannot fire (the loop test precedes it) and is kept. No RNG.
// ---------------------------------------------------------------------------
void airfield_read_hangar_and_marker_properties_006d5220(Left1Host& host, void* airfield) {
    host.unit_read_common_lua_properties_0095e5b0(airfield);
    void* const source = ptr(airfield, 0xC0);
    if (source != nullptr) {
        const std::uint32_t kind = at<std::uint32_t>(source, 4);
        if (kind == 1) {
            read_kind1_scene(host, airfield);
        } else if (kind == 2) {
            read_kind2_table(host, airfield, ptr(source, 0x08));
        } else if (kind == 3) {
            read_kind3_lua(host, airfield);
        }
    }
    airfield_common_tail(host, airfield);
}

}  // namespace bsp::cc13_left1
