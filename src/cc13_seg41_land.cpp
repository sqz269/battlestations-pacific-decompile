// Segment 41 land convoy, land fort and land vehicle reconstructions
// (cc13-seg41). Evidence and ABI: include/bsp/cc13_seg41_land.hpp and
// docs/CC13_SEG41.md.
#include "bsp/cc13_seg41_land.hpp"

#include <cstring>

#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_write.hpp"

namespace bsp::cc13_seg41 {
namespace {

std::uint32_t float_bits(float value) {
    std::uint32_t bits;
    static_assert(sizeof bits == sizeof value);
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}
std::uint32_t* float_slot(float& value) { return reinterpret_cast<std::uint32_t*>(&value); }

// 32-bit operator new[] size: count * 2 with the 0xFFFFFFFF saturation the
// image computes (MUL / SETO / NEG / OR).
std::size_t word_array_bytes(std::uint32_t count) {
    const std::uint64_t bytes = static_cast<std::uint64_t>(count) * 2u;
    return bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::size_t>(bytes);
}

}  // namespace

// ---- LANDCONVOY_CREATE ----------------------------------------------------

LandConvoyCreateMessage& land_convoy_create_construct_00741a40(
    LandConvoyCreateMessage& m, std::uint16_t word, const std::uint8_t* convoy,
    const net_cc13::EntityCreateSource& source, net_cc13::EntityCreateHost& base_host, ConvoyMessageHost& host) {
    m.base = net_cc13::build_entity_create_00766fc0(0xCF, word, source, true, base_host);  // 00741A70
    m.vtable_00 = kLandConvoyCreateVtable_00cff2bc;
    m.member_ids_128 = nullptr;
    m.path_entity_id_110 = 0;
    if (auto* path = at<std::uint8_t*>(convoy, 0x344); path != nullptr) {
        m.path_entity_id_110 = at<std::uint16_t>(host.path_entity_007aca30(path), 0x174);  // 00741AA0
    }
    m.rows_114 = at<std::int32_t>(convoy, 0x358);
    m.columns_118 = at<std::int32_t>(convoy, 0x354);
    m.float_11c = at<float>(convoy, 0x360);
    m.float_120 = at<float>(convoy, 0x364);
    m.float_124 = at<float>(convoy, 0x35C);
    const auto count = static_cast<std::uint32_t>(m.columns_118 * m.rows_114);
    m.member_ids_128 = static_cast<std::uint16_t*>(host.operator_new_00bf55be(word_array_bytes(count)));
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m.columns_118 * m.rows_114); ++i) {
        // std::vector<entity*> at +394h: first +398h, last +39Ch; checked index.
        auto first = at<std::uint32_t>(convoy, 0x398);
        if (first == 0 || ((at<std::uint32_t>(convoy, 0x39C) - first) >> 2) <= i) host.invalid_iterator_00bf6713();
        first = at<std::uint32_t>(convoy, 0x398);
        if (at<std::uint32_t>(ptr(first), i * 4) == 0) {
            m.member_ids_128[i] = 0;
        } else {
            if (first == 0 || ((at<std::uint32_t>(convoy, 0x39C) - first) >> 2) <= i) host.invalid_iterator_00bf6713();
            m.member_ids_128[i] = at<std::uint16_t>(ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(convoy, 0x398)), i * 4)), 0x174);
        }
    }
    m.reverse_12c = at<std::uint8_t>(convoy, 0x3A9);
    m.speed_130 = at<float>(convoy, 0x368);
    return m;
}

void land_convoy_create_destruct_00741180(LandConvoyCreateMessage& m, ConvoyMessageHost& host) {
    m.vtable_00 = kLandConvoyCreateVtable_00cff2bc;
    host.free_00bf65ac(m.member_ids_128);  // 00741190
    host.base_destruct_007668e0(m);        // 0074119B JMP
}

bool land_convoy_create_is_type_00741420(std::uint32_t type) {
    return type == 0xCF || type == 0x47 || type == 0x46;
}

void land_convoy_create_write_00741450(const LandConvoyCreateMessage& m, NativeBitCursor* cursor,
                                       const NativeBitNumericContext& numeric) {
    net_cc13::write_entity_create_00765000(m.base, cursor, numeric);
    write_native_word_bits_00429120(cursor, m.path_entity_id_110, 13);
    write_native_unsigned_dword_bits_00429070(cursor, static_cast<std::uint32_t>(m.rows_114), 8);
    write_native_unsigned_dword_bits_00429070(cursor, static_cast<std::uint32_t>(m.columns_118), 8);
    write_native_numeric_float_004295c0(cursor, float_bits(m.float_11c), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    write_native_numeric_float_004295c0(cursor, float_bits(m.float_120), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    write_native_numeric_float_004295c0(cursor, float_bits(m.float_124), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m.rows_114 * m.columns_118); ++i) {
        write_native_word_bits_00429120(cursor, m.member_ids_128[i], 12);
    }
    write_native_bool_bit_004290b0(cursor, m.reverse_12c);
    write_native_numeric_float_004295c0(cursor, float_bits(m.speed_130), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
}

void land_convoy_create_read_00741570(LandConvoyCreateMessage& m, NativeSessionReadStream& stream,
                                      const NativeBitNumericContext& numeric, ConvoyMessageHost& host) {
    net_cc13::read_entity_create_00767e30(m.base, stream, numeric);
    NativeBitCursor* c = &stream.cursor_04;
    read_native_word_bits_00428e30(c, &m.path_entity_id_110, 13);
    read_native_u32_bits_00428d10(c, reinterpret_cast<std::uint32_t*>(&m.rows_114), 8);
    read_native_u32_bits_00428d10(c, reinterpret_cast<std::uint32_t*>(&m.columns_118), 8);
    read_native_numeric_float_004293f0(c, float_slot(m.float_11c), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    read_native_numeric_float_004293f0(c, float_slot(m.float_120), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    read_native_numeric_float_004293f0(c, float_slot(m.float_124), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
    const auto count = static_cast<std::uint32_t>(m.columns_118 * m.rows_114);
    m.member_ids_128 = static_cast<std::uint16_t*>(host.operator_new_00bf55be(word_array_bytes(count)));  // 00741624
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m.columns_118 * m.rows_114); ++i) {
        read_native_word_bits_00428e30(c, &m.member_ids_128[i], 12);
    }
    bool reverse = false;
    read_native_bool_bit_00428d70(c, &reverse);
    m.reverse_12c = reverse ? 1 : 0;
    read_native_numeric_float_004293f0(c, float_slot(m.speed_130), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
}

void land_convoy_send_create_00741d40(std::uint8_t* convoy, std::uint32_t peer,
                                      const net_cc13::EntityCreateSource& source,
                                      net_cc13::EntityCreateHost& base_host, ConvoyMessageHost& host) {
    LandConvoyCreateMessage m;
    land_convoy_create_construct_00741a40(m, 0, convoy, source, base_host, host);  // 00741D65
    host.send_to_peer_00779fc0(convoy, peer, m);                                    // 00741D84
    m.vtable_00 = kLandConvoyCreateVtable_00cff2bc;                                 // inlined 00741180
    host.free_00bf65ac(m.member_ids_128);
    host.base_destruct_007668e0(m);
}

std::uint8_t* land_convoy_create_entity_00743b70(const LandConvoyCreateMessage& m, const EntityIdTable& ids,
                                                 ConvoyCreateEntityHost& host) {
    void* memory = host.operator_new_00bf55be(0x3CC);
    host.memset_00bf79f0(memory, 0, 0x3CC);  // the image clears before its null test
    std::uint8_t* convoy = memory != nullptr ? host.convoy_construct_004f2410(memory, m.base.net_id_1c) : nullptr;
    std::uint8_t* path_owner = ids.entity(m.path_entity_id_110);
    at<std::uint8_t*>(convoy, 0x344) = path_owner != nullptr ? host.path_interface_007ac9d0(path_owner) : nullptr;
    at<std::int32_t>(convoy, 0x358) = m.rows_114;
    at<std::int32_t>(convoy, 0x354) = m.columns_118;
    at<float>(convoy, 0x360) = m.float_11c;
    at<float>(convoy, 0x364) = m.float_120;
    at<float>(convoy, 0x35C) = m.float_124;
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m.columns_118 * m.rows_114); ++i) {
        std::uint32_t value = dword_of(ids.entity(m.member_ids_128[i]));
        std::uint8_t* vector = convoy + 0x394;  // {alloc, first +4, last +8, end +0Ch}
        const std::uint32_t first = at<std::uint32_t>(vector, 4);
        const std::uint32_t size = first == 0 ? 0 : (at<std::uint32_t>(vector, 8) - first) >> 2;
        if (first != 0 && size < ((at<std::uint32_t>(vector, 0xC) - first) >> 2)) {
            const std::uint32_t last = at<std::uint32_t>(vector, 8);
            at<std::uint32_t>(ptr(last), 0) = value;
            at<std::uint32_t>(vector, 8) = last + 4;
        } else {
            const std::uint32_t last = at<std::uint32_t>(vector, 8);
            if (first > last) host.invalid_iterator_00bf6713();
            std::uint32_t inserted = 0;
            host.vector_insert_00743350(vector, &inserted, vector, last, &value);  // 00743D0F
        }
    }
    at<std::uint8_t>(convoy, 0x3A9) = m.reverse_12c;
    at<float>(convoy, 0x368) = m.speed_130;
    return convoy;
}

void land_convoy_get_property_007416f0(std::uint8_t* convoy, void* lua, const std::int32_t* key,
                                       const float& clock_00f876a4, LuaPropertyHost& host) {
    host.unit_get_property_00779bb0(convoy, lua, key);  // 007416FF
    const char* text = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(key[1]));
    if (text != nullptr && host.stricmp_00bf7fbf(text, "InitialSize") == 0) {  // 00CFF31C
        host.push_integer_00b664b0(lua, at<std::int32_t>(convoy, 0x3B8));
        return;
    }
    if (host.key_equals_00425850(key, "ActualSize")) {
        host.push_integer_00b664b0(lua, at<std::int32_t>(convoy, 0x3BC));
        return;
    }
    if (host.key_equals_00425850(key, "CollectedDamage")) {
        host.push_float_00b66480(lua, at<float>(convoy, 0x3C0));
        return;
    }
    if (host.key_equals_00425850(key, "LastHit")) {
        const float since = clock_00f876a4 - at<float>(convoy, 0x3C4);  // FSUB, FSTP float
        host.push_float_00b66480(lua, since);
        return;
    }
    if (host.key_equals_00425850(key, "Reverse")) {
        host.push_boolean_00b66450(lua, at<std::uint8_t>(convoy, 0x3A9) != 0);
        return;
    }
    float value;
    if (host.key_equals_00425850(key, "Speed")) {
        value = at<float>(convoy, 0x368);
    } else if (host.key_equals_00425850(key, "Offset")) {
        value = at<float>(convoy, 0x3AC);
    } else if (host.key_equals_00425850(key, "DistanceTraveled")) {
        value = at<float>(convoy, 0x3B4);
    } else {
        return;
    }
    host.push_boolean_00b66450(lua, value != 0.0f);  // 00741827 UCOMISS [00D7A218]; NaN -> true
}

namespace {
const char* const kRomanThousands_00e0cc5c[] = {"M", "MM", "MMM"};
const char* const kRomanHundreds_00e0cc38[] = {"C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
const char* const kRomanTens_00e0cc14[] = {"X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
const char* const kRomanUnits_00e0cbf0[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
}  // namespace
const char* const* const kRomanDigitTable_00e0cc68[4] = {kRomanThousands_00e0cc5c, kRomanHundreds_00e0cc38,
                                                          kRomanTens_00e0cc14, kRomanUnits_00e0cbf0};

std::int32_t* land_convoy_member_name_00742a70(std::int32_t* out, std::int32_t n, NativeStringHost& host) {
    std::int32_t built[2] = {0, 0};  // {len, data}
    if (static_cast<std::uint32_t>(n - 1) > 0xF9Eu) {  // 00742AAB: outside 1..3999
        host.assign_0041e870(out, "N/A");               // 00CECE08
        return out;
    }
    const char* const* const* table = kRomanDigitTable_00e0cc68;
    std::int32_t length = 0;
    for (std::int32_t divisor = 1000; divisor > 0; divisor /= 10, ++table) {
        if (divisor > n) continue;
        const std::int32_t digit = n / divisor;
        n %= divisor;
        std::int32_t piece[2];
        host.assign_0041e870(piece, (*table)[digit - 1]);  // 00742AEC
        const auto piece_len = piece[0];
        auto* piece_data = ptr(static_cast<std::uint32_t>(piece[1]));
        if (piece_len != 0) {
            host.resize_0041dd40(built, static_cast<std::uint32_t>(piece_len + length), 1);
            host.memcpy_00bf7680(ptr(static_cast<std::uint32_t>(built[1])) + length, piece_data,
                                 static_cast<std::size_t>(piece_len));
            length = built[0];
        }
        if (piece_data != nullptr) host.release_00419cc0_00bd1510(piece_data, static_cast<std::size_t>(piece_len) + 1u);
    }
    out[0] = 0;
    out[1] = 0;
    if (out != built) {
        host.resize_0041dd40(out, static_cast<std::uint32_t>(length), 1);  // 00742B85
        if (length != 0) {
            host.memcpy_00bf7680(ptr(static_cast<std::uint32_t>(out[1])), ptr(static_cast<std::uint32_t>(built[1])),
                                 static_cast<std::size_t>(out[0]));
        }
    }
    if (built[1] != 0) host.release_00419cc0_00bd1510(ptr(static_cast<std::uint32_t>(built[1])), length + 1u);
    return out;
}

void entity_ref_kill_and_clear_007412f0(std::uint32_t* slot, std::uint32_t value, EntityKillHost& host) {
    if (*slot != 0) {
        host.kill_00926d90(*slot, value);
        *slot = 0;
    }
}

void free_and_null_00741310(void** slot, ConvoyMessageHost& host) {
    if (*slot != nullptr) {
        host.free_00bf65ac(*slot);
        *slot = nullptr;
    }
}

// ---- LANDFORT_CREATE / LANDFORT_EFFECT -------------------------------------

LandFortCreateMessage& land_fort_create_construct_00743db0(
    LandFortCreateMessage& m, std::uint16_t word, std::uint8_t* fort, const net_cc13::EntityCreateSource& source,
    net_cc13::EntityCreateHost& base_host, FortMessageHost& host) {
    m.base = net_cc13::build_entity_create_00766fc0(0xD0, word, source, true, base_host);  // 00743DDF
    m.vtable_00 = kLandFortCreateVtable_00cff358;
    m.handle_110.vtable_00 = kEntityHandleRefVtable_00cff354;
    host.handle_assign_00809290(m.handle_110, fort != nullptr ? fort + 0x72C : nullptr);  // 00743E0C
    return m;
}

void land_fort_send_create_00743f90(std::uint8_t* fort, std::uint32_t peer, const net_cc13::EntityCreateSource& source,
                                    net_cc13::EntityCreateHost& base_host, FortMessageHost& host) {
    LandFortCreateMessage m;
    const auto word = static_cast<std::uint16_t>(at<std::uint32_t>(at<std::uint8_t*>(fort, 0x538), 0x70));
    land_fort_create_construct_00743db0(m, word, fort, source, base_host, host);
    host.send_to_peer_00779fc0(fort, peer, m);           // 00743FD8
    m.vtable_00 = kLandFortCreateVtable_00cff358;         // 00743FEC
    m.handle_110.vtable_00 = kEntityHandleRefVtable_00cff354;
    host.base_destruct_007668e0(m);                       // 00743FFF
}

void entity_handle_write_00744760(const EntityHandleRef& handle, NativeBitCursor* cursor) {
    write_native_bool_bit_004290b0(cursor, handle.present_04);
    if (handle.present_04 != 0) write_native_word_bits_00429120(cursor, handle.id_06, 13);
}

bool land_fort_create_is_type_007447c0(std::uint32_t type) { return type == 0xD0 || type == 0x47 || type == 0x46; }

void land_fort_create_write_007447f0(const LandFortCreateMessage& m, NativeBitCursor* cursor,
                                     const NativeBitNumericContext& numeric) {
    net_cc13::write_entity_create_00765000(m.base, cursor, numeric);
    write_native_bool_bit_004290b0(cursor, m.handle_110.present_04);
    if (m.handle_110.present_04 != 0) write_native_word_bits_00429120(cursor, m.handle_110.id_06, 13);
}

void land_fort_create_read_00744830(LandFortCreateMessage& m, NativeSessionReadStream& stream,
                                    const NativeBitNumericContext& numeric) {
    net_cc13::read_entity_create_00767e30(m.base, stream, numeric);
    bool present = false;
    read_native_bool_bit_00428d70(&stream.cursor_04, &present);
    m.handle_110.present_04 = present ? 1 : 0;
    if (m.handle_110.present_04 != 0) read_native_word_bits_00428e30(&stream.cursor_04, &m.handle_110.id_06, 13);
}

std::uint8_t* land_fort_create_entity_007474e0(const LandFortCreateMessage& m, FortCreateEntityHost& host) {
    std::uint8_t* vehicle_class = host.vehicle_class_00964790(m.base.word_1e, true);  // 007474EB
    std::uint8_t* unit = host.class_create_unit_28(vehicle_class, m.base.net_id_1c);   // 007474FC
    // +C0h is a char* in the image (null when absent); the codec model keeps it
    // as a string plus a presence flag.
    const char* source = m.base.name_present_c0 ? m.base.name_c0.c_str() : nullptr;
    std::uint32_t length = 0;
    if (source != nullptr) {
        while (source[length] != '\0') ++length;
    }
    host.native_string_resize_0041dd40(unit + 0x154, length, 0);  // 00747529
    if (at<void*>(unit, 0x158) != nullptr) host.memcpy_00bf7680(at<void*>(unit, 0x158), source, at<std::uint32_t>(unit, 0x154));
    host.set_model_group_state_00876ec0(unit, 0);  // 00747546
    return unit;
}

namespace {
void effect_construct_head(LandFortEffectMessage& m, const NativeSessionMessageContext& ctx) {
    construct_native_session_message_0075b430(&m.storage, 0xD1, ctx);
    m.id_18 = 0;
    m.relay_1a = 0;
}
}  // namespace

LandFortEffectMessage& land_fort_effect_construct_00744870(LandFortEffectMessage& m, std::uint32_t kind,
                                                           std::uint32_t value, const NativeSessionMessageContext& ctx,
                                                           SessionMessageHost& host) {
    effect_construct_head(m, ctx);
    m.has_point_28 = 0;
    m.storage.delivery_04 = 1;
    m.storage.profile_00 = host.vtable(0x00CFF37Cu);
    m.kind_1c = kind;
    m.value_20 = value;
    return m;
}

LandFortEffectMessage& land_fort_effect_construct_point_007448b0(LandFortEffectMessage& m, std::uint32_t kind,
                                                                 float point, const NativeSessionMessageContext& ctx,
                                                                 SessionMessageHost& host) {
    effect_construct_head(m, ctx);
    m.storage.delivery_04 = 1;
    m.has_point_28 = 1;
    m.storage.profile_00 = host.vtable(0x00CFF37Cu);
    m.kind_1c = kind;
    m.point_24 = point;
    return m;
}

void land_fort_effect_write_00744920(const LandFortEffectMessage& m, NativeBitCursor* cursor,
                                     const NativeBitNumericContext& numeric, SessionMessageHost& host) {
    host.write_header_0075b480(&m, cursor);
    write_native_bool_bit_004290b0(cursor, m.has_point_28);
    write_native_unsigned_dword_bits_00429070(cursor, m.kind_1c, 3);
    if (m.has_point_28 != 0) {
        write_native_numeric_float_004295c0(cursor, float_bits(m.point_24), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
        return;
    }
    write_native_unsigned_dword_bits_00429070(cursor, m.value_20, 3);
}

void land_fort_effect_read_00744990(LandFortEffectMessage& m, NativeSessionReadStream& stream,
                                    const NativeBitNumericContext& numeric, SessionMessageHost& host) {
    host.read_header_0075b4c0(&m, stream);
    NativeBitCursor* c = &stream.cursor_04;
    bool has_point = false;
    read_native_bool_bit_00428d70(c, &has_point);
    m.has_point_28 = has_point ? 1 : 0;
    read_native_u32_bits_00428d10(c, &m.kind_1c, 3);
    if (m.has_point_28 != 0) {
        read_native_numeric_float_004293f0(c, float_slot(m.point_24), 0, 1, kFltMaxBits_00d7a248, 32, numeric);
        return;
    }
    read_native_u32_bits_00428d10(c, &m.value_20, 3);
}

// ---- MLandFort ---------------------------------------------------------------

namespace {
void release_dword_field(std::uint8_t* base, std::uint32_t offset, RefCountedHost& host) {
    auto* object = reinterpret_cast<RefCounted*>(at<std::uint8_t*>(base, offset));
    if (object != nullptr) {
        if (host.interlocked_decrement(&object->refs_04) == 0) host.release_slot0(object);
        at<std::uint32_t>(base, offset) = 0;
    }
}
void store_land_fort_vtables(std::uint8_t* fort) {
    at<std::uint32_t>(fort, 0x000) = 0x00CFF3F8u;
    at<std::uint32_t>(fort, 0x010) = 0x00CFF3E0u;
    at<std::uint32_t>(fort, 0x024) = 0x00CFF3D8u;
    at<std::uint32_t>(fort, 0x170) = 0x00CFF3D4u;
    at<std::uint32_t>(fort, 0x1E4) = 0x00CFF3CCu;
    at<std::uint32_t>(fort, 0x310) = 0x00CFF3B4u;
    at<std::uint32_t>(fort, 0x38C) = 0x00CFF3B0u;
    at<std::uint32_t>(fort, 0x72C) = 0x00CFF3ACu;
}
}  // namespace

std::uint8_t* land_fort_construct_00745940(std::uint8_t* fort, std::uint32_t row, LandFortHost& host) {
    host.unit_base_construct_0095cc90(fort, row, 0);  // 00745967
    host.ref_slot_construct_00809270(fort + 0x72C);   // 00745978
    store_land_fort_vtables(fort);
    at<std::uint32_t>(fort, 0x740) = 0;
    at<std::uint32_t>(fort, 0x744) = 0;
    at<std::uint32_t>(fort, 0x0C4) = 0x1B;
    at<std::uint32_t>(fort, 0x738) = 0;
    at<std::uint32_t>(fort, 0x73C) = 0;
    release_dword_field(fort, 0x740, host);  // the inlined reference-slot reset (always null here)
    at<std::uint32_t>(fort, 0x740) = 0;
    at<float>(fort, 0x750) = 0.0f;
    return fort;
}

void land_fort_destruct_00745aa0(std::uint8_t* fort, void* const& game_00e188a8, LandFortHost& host) {
    store_land_fort_vtables(fort);
    if (auto* fire = at<std::uint8_t*>(fort, 0x740); fire != nullptr) {
        host.effect_stop_00867b10(fire);  // 00745B24
        at<std::uint8_t>(fire, 9) = 1;
    }
    if (const auto body = at<std::uint32_t>(fort, 0x754); body != 0) {
        host.physics_remove_body_00c34f70(at<std::uint8_t*>(static_cast<std::uint8_t*>(game_00e188a8), 0x18), body);
    }
    release_dword_field(fort, 0x744, host);
    release_dword_field(fort, 0x740, host);
    host.ref_slot_destruct_00809650(fort + 0x72C);  // 00745BAD
    host.unit_base_destruct_00959940(fort);         // 00745BBC
}

std::uint8_t* land_fort_allocate_00747000(std::uint32_t vehicle_class, std::uint32_t row, LandFortHost& host) {
    void* memory = host.operator_new_00bf55be(0x758);
    host.memset_00bf79f0(memory, 0, 0x758);
    std::uint8_t* fort = memory != nullptr ? land_fort_construct_00745940(static_cast<std::uint8_t*>(memory), row, host)
                                           : nullptr;
    host.set_vehicle_class_009553d0(fort, vehicle_class);  // 00747063
    at<std::uint32_t>(fort, 0x354) = vehicle_class;
    return fort;
}

void land_fort_init_lua_and_weapons_00743e90(std::uint8_t* fort, LandFortHost& host) {
    host.attach_lua_self_0077e830(fort);
    void* memory = host.operator_new_00bf681b(0x240);
    std::uint32_t director = 0;
    if (memory != nullptr) {
        director = host.weapon_director_construct_008363e0(memory, fort != nullptr ? fort + 0x310 : nullptr, 0);
    }
    at<std::uint32_t>(fort, 0x74C) = director;
    if (at<std::uint32_t>(at<std::uint8_t*>(fort, 0xC0), 4) == 1) host.instantiate_class_devices_0095a880(fort, 0, 1, 0);
}

void land_fort_apply_fire_target_order_00744a90(std::uint8_t* fort, const std::uint8_t* message, FortOrderHost& host) {
    CommandTargetRecord record;
    record.word_00 = at<std::uint16_t>(message, 0x24);
    record.word_02 = at<std::uint16_t>(message, 0x26);
    record.dword_04 = at<std::uint32_t>(message, 0x28);
    for (int i = 0; i < 4; ++i) record.floats_08[i] = at<float>(message, 0x2C + 4 * i);
    const std::uint32_t command = host.command_from_ordinal_0077a050(message);  // 00744ADF
    if (command == 0x00E08EF8u) {
        const std::uint32_t target = host.resolve_target_00521ea0(record);     // 00744AF1
        host.set_fire_target_00835860(at<std::uint32_t>(fort, 0x74C), target, 1);
        return;
    }
    if (command == 0x00E08F00u || command == 0x00E08F08u) {
        host.set_fire_target_00835860(at<std::uint32_t>(fort, 0x74C), 0, 1);  // 00744B21
    }
}

std::uint32_t* linked_list_unlink_00744f70(LinkedNodeList& list, std::uint32_t* node, FreeHost& host) {
    auto* prev = ptr(node[0]);
    if (prev == nullptr) {
        list.head_04 = reinterpret_cast<std::uint32_t*>(ptr(node[1]));
    } else {
        at<std::uint32_t>(prev, 4) = node[1];
    }
    const std::uint32_t previous = node[0];
    if (node[1] != 0) {
        auto* next = reinterpret_cast<std::uint32_t*>(ptr(node[1]));
        next[0] = previous;
        --list.count_00;
        host.free_00bf65ac(node);
        return next;
    }
    --list.count_00;
    list.tail_08 = reinterpret_cast<std::uint32_t*>(ptr(previous));
    host.free_00bf65ac(node);
    return nullptr;
}

void linked_list_clear_00745200(LinkedNodeList& list, FreeHost& host) {
    while (list.count_00 != 0) {
        std::uint32_t* node = list.head_04;
        if (node[0] == 0) {
            list.head_04 = reinterpret_cast<std::uint32_t*>(ptr(node[1]));
        } else {
            at<std::uint32_t>(ptr(node[0]), 4) = node[1];
        }
        if (node[1] == 0) {
            list.tail_08 = reinterpret_cast<std::uint32_t*>(ptr(node[0]));
        } else {
            at<std::uint32_t>(ptr(node[1]), 0) = node[0];
        }
        --list.count_00;
        host.free_00bf65ac(node);
    }
}

void land_fort_set_effect_anchor_00745130(std::uint8_t* effect, const float* anchor, EffectAnchorHost& host) {
    at<float>(effect, 0x100) = anchor[0];
    at<float>(effect, 0x104) = anchor[1];
    at<float>(effect, 0x108) = anchor[2];
    std::uint8_t* parent = at<std::uint8_t*>(effect, 0x8C);
    if (parent != nullptr) {
        if ((at<std::uint8_t>(parent, 0x5C) & 2) == 0) host.refresh_world_matrix_00b6db70(parent);  // 00745170
        float local[16];
        const float* world = host.matrix_multiply_00413920(&at<float>(effect, 0xD0), local, &at<float>(parent, 0xF0));
        host.set_transform_34(at<std::uint8_t*>(effect, 0x110), world);  // 0074519F
        return;
    }
    host.set_transform_34(at<std::uint8_t*>(effect, 0x110), &at<float>(effect, 0xD0));  // 007451BC
}

void save_reader_read_observer_ref_007453c0(void* reader, std::uint32_t key_tag, std::uint32_t key_value,
                                            std::uint32_t* out, SaveReaderHost& host) {
    std::uint32_t local = 0;
    host.read_10(reader, key_tag, key_value, 4, &local);  // 007453E6, value {4, &local}
    *out = local;                                         // 007453EF
}

void land_fort_serialize_007455f0(std::uint8_t* fort, ArchiveWriter& archive, FortArchiveHost& host) {
    host.unit_serialize_0095bb10(fort, archive);  // 007455FB
    archive.begin_04(0, "_landFort");              // 00CFF3A0
    if (at<std::uint32_t>(fort, 0x738) != 0) {
        archive.begin_04(0, "gazdi");                                          // 00CFF398
        archive.write_0c(0, "ID", 4, at<std::uint32_t>(fort, 0x738));         // 00CE59B4
        std::int32_t section = -1;
        for (std::uint32_t i = 0;; ++i) {
            std::uint8_t* master = at<std::uint8_t*>(fort, 0x738);
            const std::uint32_t first = at<std::uint32_t>(master, 0x348);
            if (first == 0 || ((at<std::uint32_t>(master, 0x34C) - first) >> 2) <= i) break;
            if (first == 0 || ((at<std::uint32_t>(master, 0x34C) - first) >> 2) <= i) host.invalid_iterator_00bf6713();
            if (at<std::uint32_t>(master, 0x348) + i * 4 == at<std::uint32_t>(fort, 0x73C)) {
                section = static_cast<std::int32_t>(i);
            }
        }
        archive.write_0c(0, "section", 1, static_cast<std::uint32_t>(section));  // 00CFF390
        archive.end_08();
    }
    const std::uint32_t director = at<std::uint32_t>(fort, 0x74C);
    archive.begin_04(0, "gameUnit");  // 00CF8E4C
    host.director_visit_008360c0(director, archive);
    archive.end_08();
    archive.end_08();
}

namespace {
const float kIdentity4x4[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};  // [00D7A24C] = 1.0f on the diagonal
void release_slot(RefCounted*& slot, RefCountedHost& host) {
    if (slot != nullptr) {
        RefCounted* object = slot;
        if (host.interlocked_decrement(&object->refs_04) == 0) host.release_slot0(object);
        slot = nullptr;
    }
}
}  // namespace

void land_fort_create_damage_effect_00745fa0(std::uint8_t* fort, std::uint32_t index, std::uint32_t anchor,
                                             FortEffectHost& host, EffectAnchorHost& anchor_host) {
    std::uint8_t* cls = at<std::uint8_t*>(fort, 0x538);
    std::uint8_t* parent = at<std::uint8_t*>(fort, 0x4A4);
    float matrix[16];
    for (int i = 0; i < 16; ++i) matrix[i] = kIdentity4x4[i];
    std::uint8_t* list = cls + 0x150;  // {.., first +4, last +8}
    const std::uint32_t first = at<std::uint32_t>(list, 4);
    if (first == 0 || ((at<std::uint32_t>(list, 8) - first) >> 2) <= index) host.invalid_iterator_00bf6713();
    auto* definition = reinterpret_cast<RefCounted*>(ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(list, 4)), index * 4)));
    if (definition != nullptr) host.interlocked_increment(&definition->refs_04);  // 0074606C
    RefCounted* out = nullptr;
    RefCounted** result = host.effect_create_matrix_008687c0(&out, parent, definition, matrix, 1, 0, 0);
    auto* effect = reinterpret_cast<std::uint8_t*>(*result);  // 0074607D
    release_slot(out, host);
    at<std::uint8_t>(effect, 9) = 1;
    if (anchor != 0) {
        land_fort_set_effect_anchor_00745130(effect, &at<float>(at<std::uint8_t*>(cls, 0x28), anchor * 0x0C),
                                             anchor_host);  // 007460C5
    }
}

void land_fort_create_state_effect_007460e0(std::uint8_t* fort, std::uint32_t kind, float duration,
                                            FortEffectHost& host) {
    auto stop_and_reset = [&](std::uint32_t offset) {
        if (std::uint8_t* effect = at<std::uint8_t*>(fort, offset); effect != nullptr) {
            host.effect_stop_00867b10(effect);
            at<std::uint8_t>(effect, 9) = 1;
            host.ref_slot_reset_00484620(&at<std::uint8_t*>(fort, offset), 0);
        }
    };
    if (kind == 7) {
        stop_and_reset(0x740);
        if (at<std::uint32_t>(fort, 0x744) != 0) return;
        std::uint8_t* cls = at<std::uint8_t*>(fort, 0x538);
        if (at<std::uint32_t>(cls, 0x160) == 0) return;
        const float* matrix = reinterpret_cast<const float*>(host.class_effect_frame_00744c40(cls + 0x18, 0, 1, 0, 0) + 0x18);
        std::uint8_t* parent = at<std::uint8_t*>(fort, 0x4A4);
        auto* definition = reinterpret_cast<RefCounted*>(at<std::uint8_t*>(at<std::uint8_t*>(fort, 0x538), 0x160));
        if (definition != nullptr) host.interlocked_increment(&definition->refs_04);
        RefCounted* out = nullptr;
        RefCounted** result = host.effect_create_008689c0(&out, parent, definition, matrix);  // 007461B5
        host.ref_slot_assign_004845d0(&at<std::uint8_t*>(fort, 0x744), result);                // 007461C5
        release_slot(out, host);
    } else {
        stop_and_reset(0x744);
        if (at<std::uint32_t>(fort, 0x740) != 0) return;
        std::uint8_t* cls = at<std::uint8_t*>(fort, 0x538);
        const float* matrix = reinterpret_cast<const float*>(host.class_effect_frame_00744c40(cls + 0x18, 0, 1, 0, 0) + 0x18);
        cls = at<std::uint8_t*>(fort, 0x538);
        std::uint8_t* parent = at<std::uint8_t*>(fort, 0x4A4);
        const std::uint32_t first = at<std::uint32_t>(cls, 0x144);
        if (first == 0 || ((at<std::uint32_t>(cls, 0x148) - first) >> 2) <= kind) host.invalid_iterator_00bf6713();
        auto* definition = reinterpret_cast<RefCounted*>(ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(cls, 0x144)), kind * 4)));
        if (definition != nullptr) host.interlocked_increment(&definition->refs_04);
        RefCounted* out = nullptr;
        RefCounted** result = host.effect_create_008689c0(&out, parent, definition, matrix);  // 007462C0
        host.ref_slot_assign_004845d0(&at<std::uint8_t*>(fort, 0x740), result);                // 007462D0
        release_slot(out, host);
    }
    at<float>(fort, 0x748) = duration;  // 00746303
}

void release_string_storage_007469b0(std::uint8_t* object, StringReleaseHost& host) {
    if (void* data = at<void*>(object, 0x0C); data != nullptr) {
        host.release_00419cc0_00bd1510(data, at<std::uint32_t>(object, 0x08) + 1u);
    }
}
void release_string_storage_00747c10(std::uint8_t* object, StringReleaseHost& host) {
    release_string_storage_007469b0(object, host);  // identical body, __stdcall RET 4
}

void ref_member_set_release_00746a00(RefCounted** set, RefCountedHost& host) {
    for (int i = 9; i >= 0; --i) release_slot(set[i], host);
}

void ref_array_tidy_00747870(DwordVector& v, RefArrayHost& host) {
    host.resize_005471b0(v, 0);
    host.free_00bf6989(v.data_00);
}

void register_shadow_factor_00747560(std::uint8_t* node, const float* factor, ShadowFactorHost& host) {
    const std::uint32_t type = host.generated_model_type_00b74310();  // 0074758F
    if (host.node_is_kind_0c(node, type)) {
        const std::int32_t geometries = host.geometry_count_00b74650(node);
        for (std::int32_t g = 0; g < geometries; ++g) {
            std::uint8_t* geometry = host.geometry_00b74640(node, g);
            const std::int32_t elements = host.element_count_00b72b40(geometry);
            for (std::int32_t e = 0; e < elements; ++e) {
                std::uint8_t* element = host.element_00b732c0(geometry, e);
                std::int32_t name[2] = {0, 0};
                host.native_string_resize_0041dd40(name, 0x0D, 1);
                if (name[1] != 0) host.memcpy_00bf7680(ptr(static_cast<std::uint32_t>(name[1])), "cShadowFactor",
                                                       static_cast<std::size_t>(name[0]) + 1u);  // 00CFF638
                host.material_register_float_00b18b20(at<std::uint8_t*>(element, 0x20), name, factor);  // 0074763E
                if (name[1] != 0) host.native_string_release_00419cc0_00bd1510(ptr(static_cast<std::uint32_t>(name[1])),
                                                                               static_cast<std::size_t>(name[0]) + 1u);
                std::uint8_t* color = host.material_color_00b179f0(at<std::uint8_t*>(element, 0x20), 0);  // 0074766E
                at<float>(color, 0x0C) = *factor;  // FLD / FSTP
            }
        }
    }
    for (std::uint8_t* child = at<std::uint8_t*>(node, 0x34); child != nullptr; child = at<std::uint8_t*>(child, 0x3C)) {
        register_shadow_factor_00747560(child, factor, host);  // 007476AA
    }
}

bool stationary_is_kind_of_00748b40(const std::uint8_t* entity, std::uint32_t kind) {
    return kind == 0x36 || kind == 0 || kind == at<std::uint32_t>(entity, 0xC4);
}
bool land_fort_class_is_kind_of_00749030(std::uint32_t kind) { return kind == 0x1B || kind == 5 || kind == 4; }
std::uint32_t vehicle_class_create_instance_null_00749150(std::uint32_t) { return 0; }

void stationary_prop_destruct_00748b70(std::uint8_t* prop, void* const& game_00e188a8, StationaryHost& host) {
    at<std::uint32_t>(prop, 0x00) = 0x00CFF678u;
    at<std::uint32_t>(prop, 0x10) = 0x00CFF65Cu;
    at<std::uint32_t>(prop, 0x24) = 0x00CFF654u;
    if (const auto body = at<std::uint32_t>(prop, 0x1A8); body != 0) {
        host.physics_remove_body_00c34f70(at<std::uint8_t*>(static_cast<std::uint8_t*>(game_00e188a8), 0x18), body);
    }
    release_dword_field(prop, 0x184, host);
    host.entity_destruct_base_00925780(prop);  // 00748BFC
}

std::uint8_t* land_fort_allocate_stationary_prop_00748c40(std::uint32_t vehicle_class, StationaryHost& host) {
    void* memory = host.operator_new_00bf55be(0x1AC);
    host.memset_00bf79f0(memory, 0, 0x1AC);
    std::uint8_t* prop = memory != nullptr ? host.stationary_prop_construct_00748a40(memory) : nullptr;
    host.prop_set_class_0084f7f0(prop, vehicle_class);  // 00748C9E
    return prop;
}

std::uint8_t* land_fort_class_construct_00749180(std::uint8_t* cls, FortClassHost& host) {
    host.vehicle_class_base_construct_00749050(cls);
    at<std::uint32_t>(cls, 0x00) = 0x00CFF790u;
    at<std::uint32_t>(cls, 0x6C) = 0x00CFF78Cu;
    for (std::uint32_t offset : {0x13Cu, 0x144u, 0x148u, 0x14Cu, 0x154u, 0x158u, 0x15Cu, 0x160u}) {
        at<std::uint32_t>(cls, offset) = 0;
    }
    return cls;
}

void land_fort_class_destruct_00748ec0(std::uint8_t* cls, FortClassHost& host) {
    at<std::uint32_t>(cls, 0x00) = 0x00CFF790u;
    at<std::uint32_t>(cls, 0x6C) = 0x00CFF78Cu;
    release_dword_field(cls, 0x160, host);
    for (std::uint32_t vector_offset : {0x150u, 0x140u}) {
        std::uint8_t* vector = cls + vector_offset;
        if (at<std::uint32_t>(vector, 4) != 0) {
            host.destroy_range_004cc760(at<std::uint32_t>(vector, 4), at<std::uint32_t>(vector, 8), vector, cls);
            host.free_00bf65ac(ptr(at<std::uint32_t>(vector, 4)));
        }
        at<std::uint32_t>(vector, 4) = 0;
        at<std::uint32_t>(vector, 8) = 0;
        at<std::uint32_t>(vector, 0xC) = 0;
    }
    release_dword_field(cls, 0x13C, host);
    host.vehicle_class_base_destruct_0095f1c0(cls);  // 00748FBF
}

// ---- MLandingShip --------------------------------------------------------------

namespace {
std::uint16_t id_or_zero(const std::uint8_t* entity) { return entity != nullptr ? at<std::uint16_t>(entity, 0x174) : 0; }
void message_head(std::uint8_t* message, std::uint32_t type, const NativeSessionMessageContext& ctx) {
    construct_native_session_message_0075b430(reinterpret_cast<NativeSessionMessageStorage*>(message), type, ctx);
}
void store_profile(std::uint8_t* message, std::uint32_t image_vtable, SessionMessageHost& host) {
    reinterpret_cast<NativeSessionMessageStorage*>(message)->profile_00 = host.vtable(image_vtable);
}
}  // namespace

void create_motion_controller_00749790(std::uint8_t* unit, LandingShipHost& host) {
    void* memory = host.operator_new_00bf681b(0x390);
    at<std::uint32_t>(unit, 0x1018) = memory != nullptr ? host.motion_controller_construct_00939f40(memory, unit) : 0;
}

void landing_ship_on_destroyed_007497f0(std::uint8_t* ship, LandingShipHost& host) {
    if (std::uint8_t* pad = at<std::uint8_t*>(ship, 0x1200); pad != nullptr) host.pad_release_006ac550(pad);
    host.ship_on_destroyed_00824b60(ship);
}

std::uint8_t* landing_ship_open_construct_00749aa0(std::uint8_t* message, const NativeSessionMessageContext& ctx,
                                                   SessionMessageHost& host) {
    message_head(message, 0xA6, ctx);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    store_profile(message, 0x00CFF8D4u, host);
    return message;
}

bool landing_ship_open_is_type_00749ad0(std::uint32_t type) { return type == 0xA6 || type == 0x46; }

std::uint8_t* landing_ship_create_construct_00749bf0(std::uint8_t* message, std::uint8_t* ship, std::uint8_t* a,
                                                     std::uint8_t* b, std::uint32_t c, LandingShipHost& host) {
    host.ship_create_construct_00813a20(message, at<std::uint32_t>(at<std::uint8_t*>(ship, 0x538), 0x70), ship, 0xA4);
    at<std::uint32_t>(message, 0x000) = 0x00CFF8E8u;
    at<std::uint16_t>(message, 0x12C) = id_or_zero(a);
    at<std::uint16_t>(message, 0x12E) = id_or_zero(b);
    at<std::uint32_t>(message, 0x130) = c;
    return message;
}

bool ship_create_is_type_00749c70(std::uint32_t type) { return type == 0x8A || type == 0x47 || type == 0x46; }

void landing_ship_create_write_00749cd0(const std::uint8_t* message, NativeBitCursor* cursor, LandingShipHost& host) {
    host.ship_create_write_00813b10(message, cursor);
    write_native_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x12C), 13);
    write_native_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x12E), 12);
    write_native_signed_dword_bits_00429090(cursor, at<std::uint32_t>(message, 0x130), 4);
}

std::uint8_t* land_at_pad_construct_00749d90(std::uint8_t* message, std::uint8_t* pad, std::uint8_t* building,
                                             const NativeSessionMessageContext& ctx, SessionMessageHost& host) {
    message_head(message, 0xA5, ctx);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1A) = 0;
    store_profile(message, 0x00CFF908u, host);
    at<std::uint16_t>(message, 0x1C) = id_or_zero(pad);
    at<std::uint16_t>(message, 0x1E) = id_or_zero(building);
    return message;
}

void landing_ship_replicate_0074a5a0(std::uint8_t* ship, std::uint32_t peer, LandingShipHost& host) {
    alignas(4) std::uint8_t message[0x134];
    landing_ship_create_construct_00749bf0(message, ship, at<std::uint8_t*>(ship, 0x1200), at<std::uint8_t*>(ship, 0x1204),
                                           at<std::uint32_t>(ship, 0x1208), host);  // 0074A5D8
    host.send_to_peer_00779fc0(ship, peer, message);                                // 0074A5F3
    host.ship_create_destruct_0080e380(message);                                    // 0074A607
}

bool unit_is_rocket_armed_0074a8d0(const std::uint8_t* unit, const char* empty_00e1aec2, LandingShipHost& host) {
    if (at<std::uint8_t>(at<const std::uint8_t*>(unit, 0x538), 0x809) != 0) return true;
    std::int32_t total = 0;
    std::int32_t rockets = 0;
    for (std::int32_t i = 0; i < 12; ++i) {
        const std::uint8_t* list = unit + 0x394 + i * 0x0C;
        if (at<std::uint32_t>(list, 0) == 0) continue;
        if (i != 2 && i != 3 && i != 4 && i != 6) continue;
        for (const std::uint8_t* node = at<const std::uint8_t*>(list, 4); node != nullptr;
             node = at<const std::uint8_t*>(node, 4)) {
            const std::uint8_t* entity = at<const std::uint8_t*>(node, 8);
            const std::uint8_t* type = at<const std::uint8_t*>(at<const std::uint8_t*>(at<const std::uint8_t*>(entity, 0x354), 0x74), 0x34);
            const char* name = at<const char*>(type, 0x1C);
            ++total;
            if (name == nullptr) name = empty_00e1aec2;
            if (host.strncmp_00bf75b3(name, "Rocket", 6) == 0) ++rockets;  // 00CFA6B4
        }
    }
    return total > 7 && rockets * 2 > total;
}

std::uint8_t* landing_ship_construct_0074bb00(std::uint8_t* ship, std::uint32_t row, LandingShipHost& host) {
    host.ship_base_construct_0081ed40(ship, row);
    at<std::uint32_t>(ship, 0x000) = 0x00CFFA30u;
    at<std::uint32_t>(ship, 0x010) = 0x00CFFA18u;
    at<std::uint32_t>(ship, 0x024) = 0x00CFFA10u;
    at<std::uint32_t>(ship, 0x170) = 0x00CFFA0Cu;
    at<std::uint32_t>(ship, 0x1E4) = 0x00CFFA04u;
    at<std::uint32_t>(ship, 0x310) = 0x00CFF9ECu;
    at<std::uint32_t>(ship, 0x38C) = 0x00CFF9E8u;
    at<std::uint32_t>(ship, 0x72C) = 0x00CFF9E4u;
    for (std::uint32_t offset : {0x1198u, 0x119Cu, 0x11A0u, 0x1200u, 0x1204u, 0x1208u}) at<std::uint32_t>(ship, offset) = 0;
    at<std::uint32_t>(ship, 0x0C4) = 0x0C;
    at<float>(ship, 0x1210) = 0.0f;
    return ship;
}

void landing_ship_detach_0074bd20(std::uint8_t* ship, LandingShipHost& host) {
    host.ship_detach_base_006dffc0(ship);
    std::uint8_t* list = at<std::uint8_t*>(ship, 0x30) + 0xA8;
    for (std::uint8_t* node = at<std::uint8_t*>(list, 4); node != nullptr; node = at<std::uint8_t*>(node, 4)) {
        if (at<std::uint8_t*>(node, 8) == ship) {
            host.list_erase_004837d0(list, node);  // 0074BD4F
            return;
        }
    }
}

void mothership_detach_from_world_lists_00758fe0(std::uint8_t* ship, MotherShipDetachHost& host) {
    host.unit_instance_detach_mid_lists_006dffc0(ship);             // 00758FE3
    std::uint8_t* const list = at<std::uint8_t*>(ship, 0x30) + 0x84;
    for (std::uint8_t* node = at<std::uint8_t*>(list, 4); node != nullptr;
         node = at<std::uint8_t*>(node, 4)) {
        if (at<std::uint8_t*>(node, 8) == ship) {
            host.list_erase_004837d0(list, node);                  // 0075900F
            return;
        }
    }
}

void landing_ship_destruct_0074bd60(std::uint8_t* ship, LandingShipHost& host) {
    if (void* block = at<void*>(ship, 0x1198); block != nullptr) host.free_00bf65ac(block);
    at<std::uint32_t>(ship, 0x1198) = 0;
    at<std::uint32_t>(ship, 0x119C) = 0;
    at<std::uint32_t>(ship, 0x11A0) = 0;
    host.ship_base_destruct_0081f3a0(ship);
}

std::uint8_t* landing_ship_create_0074be00(std::uint32_t vehicle_class, std::uint32_t row, LandingShipHost& host) {
    void* memory = host.operator_new_00bf55be(0x122C);
    host.memset_00bf79f0(memory, 0, 0x122C);
    std::uint8_t* ship =
        memory != nullptr ? landing_ship_construct_0074bb00(static_cast<std::uint8_t*>(memory), row, host) : nullptr;
    host.set_vehicle_class_009553d0(ship, vehicle_class);
    at<std::uint32_t>(ship, 0x354) = vehicle_class;
    return ship;
}

// ---- MLandVehicle -----------------------------------------------------------

void land_vehicle_init_lua_and_devices_0074ccf0(std::uint8_t* vehicle, LandVehicleHost& host) {
    host.attach_lua_self_0077e830(vehicle);
    host.instantiate_class_devices_0095a880(vehicle, 0, 1, 0);
}

void land_vehicle_on_death_0074cd10(std::uint8_t* vehicle, LandVehicleHost& host) {
    if (std::uint8_t* convoy = at<std::uint8_t*>(vehicle, 0x738); convoy != nullptr) {
        host.convoy_member_died_007422a0(convoy, vehicle);
    }
    host.unit_on_death_00951fb0(vehicle);
}

void land_vehicle_send_create_0074cdb0(std::uint8_t* vehicle, std::uint32_t peer,
                                       const net_cc13::EntityCreateSource& source,
                                       net_cc13::EntityCreateHost& base_host, LandVehicleHost& host) {
    GroundVehicleCreateMessage m;
    const auto word = static_cast<std::uint16_t>(at<std::uint32_t>(at<std::uint8_t*>(vehicle, 0x538), 0x70));
    m.base = net_cc13::build_entity_create_00766fc0(0xCD, word, source, true, base_host);  // 0074CDE4
    m.vtable_00 = kGroundVehicleCreateVtable_00cffd18;
    host.send_to_peer_00779fc0(vehicle, peer, &m);  // 0074CE0B
    m.vtable_00 = kGroundVehicleCreateVtable_00cffd18;
    host.base_destruct_007668e0(m);                 // 0074CE27
}

void land_vehicle_replicate_sync_0074d030(std::uint8_t* vehicle, const std::uint32_t* peer_ref, LandVehicleHost& host) {
    const std::uint32_t value = *peer_ref == 0 ? 8u : at<std::uint32_t>(ptr(*peer_ref), 0x20);
    const std::uint8_t party = host.party_00927850(vehicle, value);  // 0074D04E
    host.build_sync_0077b280(vehicle, peer_ref, party, 0.0f, 1, 200.0f, 3, 500.0f, 5, 1000.0f, 0x0C, 2000.0f, 0x14,
                             3.402823466e+38f, 100);  // 0074D0E9
}

void land_vehicle_send_sync_0074d100(std::uint8_t* vehicle, const std::uint32_t* peer_ref, std::uint32_t,
                                     const NativeSessionMessageContext& ctx, SessionMessageHost& messages,
                                     LandVehicleHost& host) {
    alignas(4) std::uint8_t message[0x1C];
    message_head(message, 0xCE, ctx);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 0;
    store_profile(message, 0x00CFFD4Cu, messages);
    host.send_to_peer_00779fc0(vehicle, *peer_ref, message);  // 0074D153
    store_profile(message, 0x00CE4974u, messages);             // the root profile (inlined destructor)
    host.release_peer_00779f80(vehicle, peer_ref);             // 0074D16B
}

void land_vehicle_apply_sync_0074d230(std::uint8_t* vehicle, const std::uint8_t* message, const std::uint32_t& tick_00f876b0,
                                      const float& step_00f876ac, LandVehicleHost& host) {
    host.matrix_set_004134f0(vehicle + 0x674, kIdentity4x4);  // 0074D2AC
    host.motion_resync_008746a0(vehicle + 0x310, tick_00f876b0 - at<std::uint32_t>(message, 0x0C));
    host.motion_step_slot04(vehicle + 0x310, step_00f876ac);  // 0074D2DD
}

void points_push_back_0074d2f0(std::uint8_t* vector, const float* point, LandVehicleHost& host) {
    if (at<std::int32_t>(vector, 4) == at<std::int32_t>(vector, 8)) {
        std::int32_t grown = at<std::int32_t>(vector, 8) * 2;
        if (grown <= 1) grown = 1;
        host.points_reserve_0074d190(vector, grown);
    }
    float* slot = reinterpret_cast<float*>(at<std::uint8_t*>(vector, 0) + at<std::int32_t>(vector, 4) * 0x0C);
    if (slot != nullptr) {
        slot[0] = point[0];
        slot[1] = point[1];
        slot[2] = point[2];
    }
    ++at<std::int32_t>(vector, 4);
}

void land_vehicle_serialize_0074d3b0(std::uint8_t* vehicle, ArchiveWriter& archive, LandVehicleHost& host) {
    host.unit_serialize_0095bb10(vehicle, archive);
    archive.begin_04(0, "_landVehicle");                                      // 00CFFD80
    archive.write_0c(0, "convoyID", 4, at<std::uint32_t>(vehicle, 0x738));   // 00CFFD74
    archive.write_0c(0, "convoyPosition", 1, at<std::uint32_t>(vehicle, 0x73C));  // 00CFFD64
    archive.end_08();
}

std::uint8_t* ground_vehicle_create_entity_0074d710(const GroundVehicleCreateMessage& m, LandVehicleHost& host) {
    std::uint8_t* vehicle_class = host.vehicle_class_00964790(m.base.word_1e, true);  // 0074D71B
    std::uint8_t* unit = host.class_create_unit_28(vehicle_class, m.base.net_id_1c);   // 0074D72C
    const char* source = m.base.name_present_c0 ? m.base.name_c0.c_str() : nullptr;
    std::uint32_t length = 0;
    if (source != nullptr) {
        while (source[length] != '\0') ++length;
    }
    host.native_string_resize_0041dd40(unit + 0x154, length, 0);
    if (at<void*>(unit, 0x158) != nullptr) host.memcpy_00bf7680(at<void*>(unit, 0x158), source, at<std::uint32_t>(unit, 0x154));
    return unit;
}

void land_vehicle_class_bind_model_0074da10(std::uint8_t* cls, LandVehicleHost& host) {
    host.class_bind_base_0095f500(cls);  // 0074DA2E
    for (std::uint32_t index = 1;; ++index) {
        // MSVC std::string on the stack: buffer/pointer +0, size +10h, capacity +14h.
        alignas(4) std::uint8_t name[0x18];
        at<std::uint32_t>(name, 0x14) = 0x0F;
        at<std::uint32_t>(name, 0x10) = 0;
        name[0] = 0;
        host.std_string_assign_00408720(name, "kemeny", 6);  // 00CFFD90
        std::uint8_t* node = host.model_find_node_00718870(at<std::uint8_t*>(cls, 0x50), name, index);
        if (at<std::uint32_t>(name, 0x14) >= 0x10) host.free_00bf65ac(at<void*>(name, 0));
        at<std::uint32_t>(name, 0x14) = 0x0F;
        at<std::uint32_t>(name, 0x10) = 0;
        name[0] = 0;
        if (node == nullptr) return;
        const std::uint32_t first = at<std::uint32_t>(node, 0x48);
        if (first == 0 || (static_cast<std::int32_t>(at<std::uint32_t>(node, 0x4C) - first) / 0x0C) == 0) {
            host.invalid_iterator_00bf6713();
        }
        const float* vertex = reinterpret_cast<const float*>(ptr(at<std::uint32_t>(node, 0x48)));
        const float point[3] = {vertex[0], vertex[1], vertex[2]};
        std::uint8_t* points = cls + 0x140;
        if (at<std::int32_t>(points, 4) == at<std::int32_t>(points, 8)) {
            std::int32_t grown = at<std::int32_t>(points, 8) * 2;
            if (grown <= 1) grown = 1;
            host.points_reserve_0074d190(points, grown);
        }
        float* slot = reinterpret_cast<float*>(at<std::uint8_t*>(points, 0) + at<std::int32_t>(points, 4) * 0x0C);
        if (slot != nullptr) {
            slot[0] = point[0];
            slot[1] = point[1];
            slot[2] = point[2];
        }
        ++at<std::int32_t>(points, 4);
    }
}

void land_vehicle_on_destroyed_0074db70(std::uint8_t* vehicle, void* const& game_00e188a8, LandVehicleHost& host) {
    auto* game = static_cast<std::uint8_t*>(game_00e188a8);
    if (at<std::uint32_t>(vehicle, 0x70) == 1) {
        if (at<std::uint8_t>(vehicle, 0xC8) == 0) host.refresh_pose_00414db0(vehicle);  // 0074DB92
        game = static_cast<std::uint8_t*>(game_00e188a8);
        const std::uint32_t scene = at<std::uint32_t>(game, 0x19EC);
        auto* definition = reinterpret_cast<RefCounted*>(at<std::uint8_t*>(at<std::uint8_t*>(vehicle, 0x538), 0x138));
        if (definition != nullptr) host.interlocked_increment(&definition->refs_04);
        RefCounted* out = nullptr;
        RefCounted** result = host.effect_create_at_008685e0(&out, scene, definition, &at<float>(vehicle, 0xFC), 0, 0);
        auto* effect = reinterpret_cast<std::uint8_t*>(*result);  // 0074DBDE
        release_slot(out, host);
        at<std::uint8_t>(effect, 9) = 1;
        if (const auto sound = at<std::uint32_t>(at<std::uint8_t*>(vehicle, 0x538), 0x13C); sound != 0) {
            host.sound_play_0047bb70(at<std::uint32_t>(static_cast<std::uint8_t*>(game_00e188a8), 0x21D0), sound, vehicle);
        }
    }
    host.unit_destroyed_00959450(vehicle);  // 0074DC29
    auto* it = reinterpret_cast<RefCounted**>(at<std::uint8_t*>(vehicle, 0x72C));
    RefCounted** const end = it + at<std::uint32_t>(vehicle, 0x730);
    for (; it != end; ++it) {
        auto* effect = reinterpret_cast<std::uint8_t*>(*it);
        if (effect == nullptr) continue;
        host.effect_stop_00867b10(effect);
        at<std::uint8_t>(effect, 9) = 1;
        release_slot(*it, host);
        *it = nullptr;
    }
    host.effect_vector_resize_004c9550(vehicle + 0x72C, 0);       // 0074DCA2
    host.kill_00926d90(vehicle, at<std::uint32_t>(vehicle, 0x70));  // 0074DCAD
}

std::uint8_t* land_vehicle_construct_0074dcc0(std::uint8_t* vehicle, std::uint32_t row, LandVehicleHost& host) {
    host.unit_base_construct_0095cc90(vehicle, row, 0);
    at<std::uint32_t>(vehicle, 0x000) = 0x00CFFDE0u;
    at<std::uint32_t>(vehicle, 0x010) = 0x00CFFDC8u;
    at<std::uint32_t>(vehicle, 0x024) = 0x00CFFDC0u;
    at<std::uint32_t>(vehicle, 0x170) = 0x00CFFDBCu;
    at<std::uint32_t>(vehicle, 0x1E4) = 0x00CFFDB4u;
    at<std::uint32_t>(vehicle, 0x310) = 0x00CFFD9Cu;
    at<std::uint32_t>(vehicle, 0x38C) = 0x00CFFD98u;
    at<std::uint32_t>(vehicle, 0x72C) = 0;
    at<std::uint32_t>(vehicle, 0x730) = 0;
    at<std::uint32_t>(vehicle, 0x734) = 0;
    at<std::uint32_t>(vehicle, 0x0C4) = 0x19;
    return vehicle;
}

bool land_vehicle_is_kind_of_0074dd90(const std::uint8_t* unit, std::uint32_t kind) {
    return kind == 0x19 || kind == 5 || kind == 4 || kind == 2 || kind == 1 || kind == 0 ||
           kind == at<std::uint32_t>(unit, 0xC4);
}

void land_vehicle_vslot_b4_0074ddd0(std::uint8_t* vehicle, void* const& game_00e188a8, LandVehicleHost& host) {
    auto* game = static_cast<std::uint8_t*>(game_00e188a8);
    const std::uint32_t slot = at<std::uint32_t>(game, 0x18EC);
    host.ai_query_008ddf00(at<std::uint32_t>(game, 0x21A4 + slot * 4), vehicle);
}

void land_vehicle_detach_0074de50(std::uint8_t* vehicle, LandVehicleHost& host) {
    host.unit_detach_base_006d3620(vehicle);
    for (std::uint32_t offset : {0x54u, 0x144u}) {
        std::uint8_t* list = at<std::uint8_t*>(vehicle, 0x30) + offset;
        for (std::uint8_t* node = at<std::uint8_t*>(list, 4); node != nullptr; node = at<std::uint8_t*>(node, 4)) {
            if (at<std::uint8_t*>(node, 8) == vehicle) {
                host.list_erase_004837d0(list, node);
                break;
            }
        }
    }
}

void land_vehicle_destruct_0074deb0(std::uint8_t* vehicle, LandVehicleHost& host) {
    host.effect_vector_resize_004c9550(vehicle + 0x72C, 0);
    host.free_00bf6989(at<void*>(vehicle, 0x72C));
    host.unit_base_destruct_00959940(vehicle);
}

std::uint8_t* land_vehicle_allocate_0074df10(std::uint32_t vehicle_class, std::uint32_t row, LandVehicleHost& host) {
    void* memory = host.operator_new_00bf55be(0x740);
    host.memset_00bf79f0(memory, 0, 0x740);
    std::uint8_t* vehicle =
        memory != nullptr ? land_vehicle_construct_0074dcc0(static_cast<std::uint8_t*>(memory), row, host) : nullptr;
    host.set_vehicle_class_009553d0(vehicle, vehicle_class);
    at<std::uint32_t>(vehicle, 0x354) = vehicle_class;
    return vehicle;
}

void land_vehicle_load_saved_state_0074dfc0(std::uint8_t* vehicle, LandVehicleHost& host) {
    host.unit_load_common_0095e5b0(vehicle);
    const std::uint8_t* load = at<const std::uint8_t*>(vehicle, 0xC0);
    if (load != nullptr && at<std::uint32_t>(load, 4) == 3) {
        std::uint8_t record[0x14];
        host.saved_lua_data_009238a0(vehicle, record);            // 0074E005
        alignas(4) std::uint8_t reader[0x0C];                     // ESP+0Ch..+17h in the image frame
        host.lua_reader_construct_004425c0(reader, record);       // 0074E00E
        host.lua_reader_enter_00bd8e20(reader, VariantPair{0, dword_of("_landVehicle")});
        std::uint32_t convoy = 0;
        host.lua_reader_read_10(reader, VariantPair{0, dword_of("convoyID")}, VariantPair{4, dword_of(&convoy)});
        at<std::uint32_t>(vehicle, 0x738) = convoy;
        host.lua_reader_read_00bd6830(reader, VariantPair{0, dword_of("convoyPosition")},
                                      VariantPair{1, dword_of(vehicle + 0x73C)});  // 0074E0A2
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_destroy_00441a20(reader);
    }
    host.unit_vslot_148(vehicle, 0x1FF, 8);  // 0074E0D2
}

// ---- plane units -----------------------------------------------------------

namespace {
void erase_unit_node(std::uint8_t* list, std::uint8_t* unit, PlaneUnitHost& host) {
    for (std::uint8_t* node = at<std::uint8_t*>(list, 4); node != nullptr; node = at<std::uint8_t*>(node, 4)) {
        if (at<std::uint8_t*>(node, 8) == unit) {
            host.list_erase_004837d0(list, node);
            return;
        }
    }
}
}  // namespace

std::uint8_t* recon_plane_construct_0074e0f0(std::uint8_t* plane, std::uint32_t row, PlaneUnitHost& host) {
    host.plane_base_construct_007cfd20(plane, row, 0);
    at<std::uint32_t>(plane, 0x000) = 0x00D00070u;
    at<std::uint32_t>(plane, 0x010) = 0x00D00058u;
    at<std::uint32_t>(plane, 0x024) = 0x00D00050u;
    at<std::uint32_t>(plane, 0x170) = 0x00D0004Cu;
    at<std::uint32_t>(plane, 0x1E4) = 0x00D00044u;
    at<std::uint32_t>(plane, 0x310) = 0x00D0002Cu;
    at<std::uint32_t>(plane, 0x38C) = 0x00D00028u;
    at<std::uint32_t>(plane, 0x72C) = 0x00CFFFE8u;
    at<std::uint32_t>(plane, 0x0C4) = 0x14;
    at<std::uint32_t>(plane, 0xAAC) = 0x0B;
    return plane;
}

std::uint8_t* large_recon_plane_construct_0074e2d0(std::uint8_t* plane, std::uint32_t row, PlaneUnitHost& host) {
    recon_plane_construct_0074e0f0(plane, row, host);
    at<std::uint32_t>(plane, 0x000) = 0x00D00308u;
    at<std::uint32_t>(plane, 0x010) = 0x00D002F0u;
    at<std::uint32_t>(plane, 0x024) = 0x00D002E8u;
    at<std::uint32_t>(plane, 0x170) = 0x00D002E4u;
    at<std::uint32_t>(plane, 0x1E4) = 0x00D002DCu;
    at<std::uint32_t>(plane, 0x310) = 0x00D002C4u;
    at<std::uint32_t>(plane, 0x38C) = 0x00D002C0u;
    at<std::uint32_t>(plane, 0x72C) = 0x00D00280u;
    at<std::uint32_t>(plane, 0x0C4) = 0x16;
    at<std::uint32_t>(plane, 0xAAC) = 0x0B;
    return plane;
}

std::uint8_t* large_recon_plane_create_0074e540(std::uint32_t vehicle_class, std::uint32_t row, PlaneUnitHost& host) {
    void* memory = host.operator_new_00bf55be(0xE94);
    host.memset_00bf79f0(memory, 0, 0xE94);
    std::uint8_t* plane =
        memory != nullptr ? large_recon_plane_construct_0074e2d0(static_cast<std::uint8_t*>(memory), row, host) : nullptr;
    host.set_vehicle_class_009553d0(plane, vehicle_class);
    at<std::uint32_t>(plane, 0x354) = vehicle_class;
    return plane;
}

std::uint32_t plane_unit_vslot1c0_0074e160(const std::uint8_t* unit) { return at<std::uint32_t>(unit, 0x808); }

bool plane_unit_vslot10c_0074e1a0(std::uint8_t* plane, PlaneUnitHost& host) {
    const double ratio = host.speed_ratio_38(plane);
    return ratio > 3.0;  // [00CE3854] = 3.0f; JBE also takes the unordered case
}

void plane_set_forward_speed_0074e1e0(std::uint8_t* plane, float speed, PlaneUnitHost& host) {
    host.set_forward_speed_007d9e80(plane + 0xAB0, speed);
}

bool plane_control_mode_is_free_flight_0074e210(const std::uint8_t* control) { return at<std::uint32_t>(control, 0x1D4) == 7; }

bool plane_unit_state_admits_ai_0074e440(const std::uint8_t* plane, std::uint32_t) {
    const std::uint32_t state = at<std::uint32_t>(plane, 0x900);
    return !(state == 7 || state == 6 || state == 4 || state == 5 || state == 2 || state == 3);
}

void plane_unit_register_in_world_lists_0074e5d0(std::uint8_t* plane, PlaneUnitHost& host) {
    host.register_base_00928560(plane);
    for (std::uint32_t offset : {0x30u, 0x48u, 0x54u, 0xCCu}) {
        host.list_insert_00484540(at<std::uint8_t*>(plane, 0x30) + offset, plane);
    }
}

void plane_unit_detach_0074e610(std::uint8_t* plane, PlaneUnitHost& host) {
    host.unit_detach_base_006d3620(plane);
    erase_unit_node(at<std::uint8_t*>(plane, 0x30) + 0x54, plane, host);
    erase_unit_node(at<std::uint8_t*>(plane, 0x30) + 0xCC, plane, host);
}

void recon_plane_detach_0074e6c0(std::uint8_t* plane, PlaneUnitHost& host) {
    plane_unit_detach_0074e610(plane, host);
    erase_unit_node(at<std::uint8_t*>(plane, 0x30) + 0x108, plane, host);
}

void large_recon_plane_detach_0074e750(std::uint8_t* plane, PlaneUnitHost& host) {
    plane_unit_detach_0074e610(plane, host);
    erase_unit_node(at<std::uint8_t*>(plane, 0x30) + 0x120, plane, host);
}

void unit_subobject_10d4_destruct_0074e7f0(std::uint8_t* object, PlaneUnitHost& host) {
    at<std::uint32_t>(object, 0) = 0x00D00514u;
    for (std::uint32_t offset : {0x1Cu, 0x18u, 0x20u}) {
        if (void* block = at<void*>(object, offset); block != nullptr) host.free_00bf6989(block);
    }
}

std::uint8_t* ship_sink_construct_0074eab0(std::uint8_t* message, std::uint32_t count, const float* values,
                                           const NativeSessionMessageContext& ctx, SessionMessageHost& host) {
    message_head(message, 0x92, ctx);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1A) = 0;
    store_profile(message, 0x00D00518u, host);
    at<std::uint32_t>(message, 0x1C) = count;
    for (std::uint32_t i = 0; i < count; ++i) at<float>(message, 0x20 + i * 4) = values[i];
    return message;
}

bool ship_sink_is_type_0074eb00(std::uint32_t type) { return type == 0x92 || type == 0x46; }

// ---- mission Lua triggers ----------------------------------------------------

namespace {
// The inlined native-string assignment: when the objects differ,
// 0041DD40(dst, src.len, 1) and memcpy(dst.data, src.data, dst.len) for a
// non-empty source.
void assign_string(std::uint8_t* destination, const std::uint8_t* source, TriggerHost& host) {
    if (destination == source) return;
    host.native_string_resize_0041dd40(destination, at<std::uint32_t>(source, 0), 1);
    if (at<std::uint32_t>(source, 0) != 0) {
        host.memcpy_00bf7680(at<void*>(destination, 4), at<const void*>(source, 4), at<std::uint32_t>(destination, 0));
    }
}
void release_string_at(std::uint8_t* string, TriggerHost& host) {
    if (void* data = at<void*>(string, 4); data != nullptr) {
        host.native_string_release_00419cc0_00bd1510(data, at<std::uint32_t>(string, 0) + 1u);
    }
}
}  // namespace

void trigger_list_add_0074fab0(std::uint8_t* list, std::uint8_t* trigger, TriggerHost& host) {
    host.trigger_attach_04(trigger, list);
}

void mission_lua_variant_init_tag6_0074fb30(std::uint8_t* variant, TriggerHost& host) {
    host.variant_construct_00886920(variant);
    at<std::uint32_t>(variant, 4) = 6;
}

bool trigger_list_invoke_by_name_0074fbb0(std::uint8_t* list, const std::int32_t* name, TriggerHost& host) {
    bool fired = false;
    for (std::uint8_t* trigger = at<std::uint8_t*>(list, 0x1C); trigger != nullptr;
         trigger = at<std::uint8_t*>(trigger, 0x0C)) {
        if (!host.trigger_is_active_08(trigger, 1)) continue;
        if (at<std::uint8_t>(trigger, 0x24) == 0) continue;
        if (!host.string_equals_00435c40(trigger + 0x1C, name)) continue;
        host.trigger_fire_10(trigger);
        fired = true;
    }
    return fired;
}

void native_string_release_0074fc10(std::int32_t* string, TriggerHost& host) {
    release_string_at(reinterpret_cast<std::uint8_t*>(string), host);
}

void lua_trigger_destruct_0074fc40(std::uint8_t* trigger, TriggerHost& host) {
    at<std::uint32_t>(trigger, 0) = 0x00D00554u;
    release_string_at(trigger + 0x28, host);
    release_string_at(trigger + 0x1C, host);
    host.trigger_base_destruct_00875b30(trigger);
}

bool stub_arg_is_one_0074fcd0(std::uint32_t value) { return value == 1; }

std::uint8_t* flagged_string_copy_construct_0074fd40(std::uint8_t* out, const std::uint8_t* source, TriggerHost& host) {
    at<std::uint32_t>(out, 0) = 0;
    at<std::uint32_t>(out, 4) = 0;
    assign_string(out, source, host);
    at<std::uint8_t>(out, 8) = at<std::uint8_t>(source, 8);
    return out;
}

void lua_trigger_fire_callback_0074fdd0(std::uint8_t* trigger, void* const& game_00e188a8, TriggerHost& host) {
    alignas(4) std::uint8_t args[0x0C] = {};  // std::vector {begin, end, capacity}
    alignas(4) std::uint8_t owner_variant[0x14];
    host.variant_from_entity_00884a90(owner_variant, at<std::uint32_t>(at<std::uint8_t*>(trigger, 4), 0x28));
    host.args_push_back_006edf00(args, owner_variant);
    host.variant_destruct_00884af0(owner_variant);
    alignas(4) std::uint8_t name_variant[0x14];
    host.variant_default_008849b0(name_variant);
    if (at<std::uint8_t>(trigger, 0x24) != 0) {
        host.variant_set_string_00886a10(name_variant, trigger + 0x1C);
    } else {
        host.variant_construct_00886920(name_variant);  // the inlined 0074FB30
        at<std::uint32_t>(name_variant, 4) = 6;
    }
    host.args_push_back_006edf00(args, name_variant);
    host.lua_call_00887e50(at<std::uint32_t>(static_cast<std::uint8_t*>(game_00e188a8), 0x1A08), 0, trigger + 0x28, args,
                           0, -1);  // 0074FE89
    host.trigger_fire_10(trigger);
    host.variant_destruct_00884af0(name_variant);
    host.args_resize_006b88e0(args, 0);
    host.free_00bf6989(at<void*>(args, 0));
}

void proximity_trigger_on_target_gone_0074ffe0(std::uint8_t* observer, std::uint32_t entity, TriggerHost& host) {
    if (entity == host.resolve_target_00521ea0(observer + 0x14)) host.trigger_fire_10(observer - 0x30);
}

void line_of_sight_trigger_on_target_gone_00750190(std::uint8_t* observer, std::uint32_t entity, TriggerHost& host) {
    if (entity == at<std::uint32_t>(observer, 0x14)) host.trigger_fire_10(observer - 0x30);
}

void observer_trigger_destruct_00750090(std::uint8_t* trigger, TriggerHost& host) {
    host.observer_destruct_00695870(trigger != nullptr ? trigger + 0x30 : nullptr);
    lua_trigger_destruct_0074fc40(trigger, host);
}

std::uint8_t* entity_as_kind18_007501f0(std::uint8_t* entity, TriggerHost& host) {
    if (entity != nullptr && host.entity_is_kind_5c(entity, 0x18)) return entity;
    return nullptr;
}

void squad_ordnance_trigger_attach_007504c0(std::uint8_t* trigger, std::uint8_t* owner, TriggerHost& host) {
    host.owner_register_00876020(owner, trigger);
    at<std::uint32_t>(trigger, 0x30) = at<std::uint32_t>(owner, 0x28);
}

std::uint8_t* line_of_sight_trigger_create_00750550(std::uint8_t* target, float range, TriggerHost& host) {
    void* memory = host.operator_new_00bf681b(0x50);
    return memory != nullptr ? host.line_of_sight_construct_007500e0(memory, target, range) : nullptr;
}

std::uint8_t* proximity_trigger_create_007505c0(const std::uint8_t* target, float range, TriggerHost& host) {
    void* memory = host.operator_new_00bf681b(0x60);
    if (memory == nullptr) return nullptr;
    std::uint8_t record[0x18];
    at<std::uint16_t>(record, 0) = at<std::uint16_t>(target, 0);
    at<std::uint16_t>(record, 2) = at<std::uint16_t>(target, 2);
    at<std::uint32_t>(record, 4) = at<std::uint32_t>(target, 4);
    for (std::uint32_t offset = 8; offset < 0x18; offset += 4) at<float>(record, offset) = at<float>(target, offset);
    return host.proximity_construct_0074fee0(memory, record, range);  // 00750632
}

std::uint8_t* squad_ordnance_trigger_create_00750660(TriggerHost& host) {
    auto* t = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x34));
    if (t == nullptr) return nullptr;
    at<std::uint32_t>(t, 0x04) = 0;
    at<std::uint32_t>(t, 0x08) = 0;
    at<std::uint32_t>(t, 0x0C) = 0;
    at<std::uint8_t>(t, 0x10) = 0;
    at<std::uint32_t>(t, 0x18) = 2;
    at<std::uint8_t>(t, 0x11) = 1;
    at<std::uint32_t>(t, 0x1C) = 0;
    at<std::uint32_t>(t, 0x20) = 0;
    at<std::uint8_t>(t, 0x24) = 0;
    at<std::uint32_t>(t, 0x28) = 0;
    at<std::uint32_t>(t, 0x2C) = 0;
    at<std::uint32_t>(t, 0x14) = 1;
    at<std::uint32_t>(t, 0x00) = 0x00D005C0u;
    at<std::uint32_t>(t, 0x30) = 0;
    return t;
}

std::uint8_t* lua_trigger_construct_007506b0(std::uint8_t* trigger, const std::uint8_t* name,
                                             const std::uint8_t* callback, TriggerHost& host) {
    at<std::uint32_t>(trigger, 0x04) = 0;
    at<std::uint32_t>(trigger, 0x08) = 0;
    at<std::uint32_t>(trigger, 0x0C) = 0;
    at<std::uint8_t>(trigger, 0x10) = 0;
    at<std::uint8_t>(trigger, 0x11) = 1;
    at<std::uint32_t>(trigger, 0x14) = 0;
    at<std::uint32_t>(trigger, 0x18) = 2;
    at<std::uint32_t>(trigger, 0x00) = 0x00D00554u;
    at<std::uint32_t>(trigger, 0x1C) = 0;
    at<std::uint32_t>(trigger, 0x20) = 0;
    assign_string(trigger + 0x1C, name, host);
    at<std::uint8_t>(trigger, 0x24) = at<std::uint8_t>(name, 8);
    at<std::uint32_t>(trigger, 0x28) = 0;
    at<std::uint32_t>(trigger, 0x2C) = 0;
    assign_string(trigger + 0x28, callback, host);
    at<std::uint32_t>(trigger, 0x14) = 1;
    return trigger;
}

void lua_trigger_set_callback_name_00750790(std::uint8_t* trigger, const std::uint8_t* name,
                                            const std::uint8_t* callback, TriggerHost& host) {
    assign_string(trigger + 0x1C, name, host);
    at<std::uint8_t>(trigger, 0x24) = at<std::uint8_t>(name, 8);
    assign_string(trigger + 0x28, callback, host);
}

// ---- telemetry ---------------------------------------------------------------

void telemetry_stream_write_00751850(std::uint8_t* stream, std::uint32_t value, TelemetryHost& host) {
    host.stream_write_00a4d610(value, stream + 4);
}

bool metrics_session_is_type_00752bf0(std::uint32_t type) { return type == 0xE9; }

void metrics_session_read_00752c30(std::uint8_t* message, NativeSessionReadStream& stream, TelemetryHost& host) {
    read_native_u8_bits_00428c70(&stream.cursor_04, &at<std::uint8_t>(message, 0x10), 8);
    host.read_owned_string_00429f20(&stream.cursor_04, message + 0x18);
}

void telemetry_list_tidy_00753530(std::uint8_t* object, TelemetryHost& host) {
    std::uint8_t* vector = object + 0x10;
    if (at<std::uint32_t>(vector, 4) != 0) {
        host.destroy_range_00752900(at<std::uint32_t>(vector, 4), at<std::uint32_t>(vector, 8), vector, object);
        host.free_00bf65ac(at<void*>(vector, 4));
    }
    at<std::uint32_t>(vector, 4) = 0;
    at<std::uint32_t>(vector, 8) = 0;
    at<std::uint32_t>(vector, 0xC) = 0;
}

// ---- MMothership ---------------------------------------------------------------

void air_ops_holder_step_00758090(std::uint8_t* holder, float dt, MothershipHost& host) {
    const float heading = static_cast<float>(host.owner_heading_50(at<std::uint8_t*>(at<std::uint8_t*>(holder, 4), 0x7C)));
    at<float>(holder, 0x88) = heading;
    const float acc = at<float>(holder, 0x94) + dt;  // FADD then FSTP float
    at<float>(holder, 0x94) = acc;
    if (!(static_cast<double>(acc) > 0.20000000298023224)) return;  // [00CE3D10]; JBE
    const double r = host.wrapped_angle_delta_00438b10(heading, at<float>(holder, 0x90));
    const double half = 0.5;  // [00D7A280]
    const double scaled = r * half;                                  // DC C9: ST1 = ST1 * ST0
    const float old_acc = at<float>(holder, 0x94);
    at<float>(holder, 0x94) = 0.0f;
    const double term = scaled / static_cast<double>(old_acc);       // DE FA: ST2 = ST2 / ST0
    const double smooth = half * static_cast<double>(at<float>(holder, 0x8C));
    at<float>(holder, 0x8C) = static_cast<float>(term + smooth);     // DE C1, FSTP float
    at<float>(holder, 0x90) = at<float>(holder, 0x88);
}

namespace {
void store_mothership_vtables(std::uint8_t* ship) {
    at<std::uint32_t>(ship, 0x000) = 0x00D01630u;
    at<std::uint32_t>(ship, 0x010) = 0x00D01614u;
    at<std::uint32_t>(ship, 0x024) = 0x00D0160Cu;
    at<std::uint32_t>(ship, 0x170) = 0x00D01608u;
    at<std::uint32_t>(ship, 0x1E4) = 0x00D01600u;
    at<std::uint32_t>(ship, 0x310) = 0x00D015E8u;
    at<std::uint32_t>(ship, 0x38C) = 0x00D015E4u;
    at<std::uint32_t>(ship, 0x72C) = 0x00D015E0u;
}
float abs_by_negative_zero(float x) { return x > 0.0f ? x : -0.0f - x; }  // [00D7A208] = -0.0f
}  // namespace

void mothership_destruct_00758150(std::uint8_t* ship, MothershipHost& host) {
    store_mothership_vtables(ship);
    at<std::uint32_t>(ship, 0x1188) = 0x00D015C4u;
    host.block_destruct_006ca410(ship + 0x1188);
    host.ship_base_destruct_0081f3a0(ship);
}

void mothership_init_pass_c_00758210(std::uint8_t* ship, MothershipHost& host) {
    host.ship_init_pass_c_0081f980(ship);
    at<std::uint32_t>(ship, 0x11A8) = 1;
}

void mothership_update_00758240(std::uint8_t* ship, float dt, MothershipHost& host) {
    host.ship_update_008255b0(ship, dt);
    host.block_update_006ba670(ship + 0x1188, dt);
}

void mothership_update_motion_00758270(std::uint8_t* ship, float dt, MothershipHost& host) {
    host.ship_update_motion_00825f20(ship, dt);
    host.deck_update_006cdc70(ship + 0xE78, dt);
    if (std::uint8_t* holder = at<std::uint8_t*>(ship, 0xEF8); holder != nullptr) {
        air_ops_holder_step_00758090(holder, dt, host);
    }
}

bool mothership_may_follow_007582b0(std::uint8_t* ship, std::uint32_t a, std::uint32_t b, MothershipHost& host) {
    if (host.block_may_follow_006ba6a0(ship + 0x1188, a, b)) return true;
    return host.ship_may_follow_008162b0(ship, a, b);
}

bool mothership_supports_command_007582f0(std::uint8_t* ship, const char* command, MothershipHost& host) {
    if (host.compare_00438e10(command, "attackmove") == 0) return false;      // 00CECCA8
    if (host.compare_00438e10(command, "launchairstrike") == 0) return true;  // 00CF85E0
    return host.ship_supports_command_00812b00(ship, command);
}

void mothership_get_property_00758340(std::uint8_t* ship, std::uint32_t lua, std::uint32_t key, MothershipHost& host) {
    host.ship_get_property_00815870(ship, lua, key);
    host.block_get_property_006c6630(ship + 0x1188, lua, key);
}

void mothership_reset_condition_00758370(std::uint8_t* ship, MothershipHost& host) {
    host.ship_reset_condition_0080e410(ship);
    at<std::uint8_t>(ship, 0x11A5) = 0;
    at<std::uint8_t>(ship, 0x11A4) = 0;
}

bool air_ops_block_has_queue_records_007584b0(std::uint8_t* block, MothershipHost& host) {
    std::uint8_t* section = at<std::uint8_t*>(block, 8);
    if (section != nullptr) {
        host.enter_section(section);
        ++at<std::int32_t>(section, 0x18);
    }
    std::int32_t count = 0;
    if (const auto first = at<std::uint32_t>(block, 0xA8); first != 0) {
        count = static_cast<std::int32_t>(at<std::uint32_t>(block, 0xAC) - first) / 20;
    }
    const bool any = count != 0;
    if (section != nullptr) {
        --at<std::int32_t>(section, 0x18);
        host.leave_section(section);
    }
    return any;
}

std::uint8_t* mothership_construct_00758550(std::uint8_t* ship, std::uint32_t row, MothershipHost& host) {
    host.ship_base_construct_0081ed40(ship, row);
    host.block_construct_006cac00(ship + 0x1188, ship);
    at<std::uint32_t>(ship, 0x1188) = 0x00D015C4u;
    store_mothership_vtables(ship);
    at<std::uint32_t>(ship, 0x0C4) = 9;
    at<std::uint32_t>(ship, 0x11A8) = 1;
    return ship;
}

bool mothership_vslot228_00758630(std::uint8_t* ship, MothershipHost& host) {
    const float roll = abs_by_negative_zero(at<float>(ship, 0x984));
    if (!(static_cast<double>(roll) > 0.05000000074505806)) return false;  // [00D7A270]
    const float speed = static_cast<float>(host.controller_speed_0092d730(at<std::uint8_t*>(ship, 0x1018)));
    if (!(abs_by_negative_zero(speed) > 1.0f)) return false;  // [00D7A24C]
    if (air_ops_block_has_queue_records_007584b0(ship + 0x1188, host)) return true;
    return at<std::uint32_t>(ship, 0x11C0) != 0;
}

void mothership_step_elevator_frame_007586d0(std::uint8_t* ship, float dt, MothershipHost& host) {
    float m[16];
    for (int i = 0; i < 16; ++i) m[i] = at<float>(ship, 0x1288 + 4 * i);
    const float d0 = m[4] * dt;  // DE CA then FSTP float
    const float d1 = m[5] * dt;  // D8 C9
    const float d2 = dt * m[6];  // FMUL float ptr
    m[12] -= d0;
    m[13] -= d1;
    m[14] -= d2;
    host.node_set_transform_38(at<std::uint8_t*>(ship, 0x1284), m);
}

std::uint8_t* elevator_move_construct_00758b90(std::uint8_t* message, std::uint8_t* entity, std::uint8_t flag,
                                               const NativeSessionMessageContext& ctx, SessionMessageHost& host) {
    message_head(message, 0x87, ctx);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    store_profile(message, 0x00D01870u, host);
    at<std::uint8_t>(message, 0x23) = entity != nullptr ? 1 : 0;
    at<std::uint8_t>(message, 0x22) = flag;
    at<std::uint16_t>(message, 0x20) = id_or_zero(entity);
    return message;
}

bool elevator_move_is_type_00758bf0(std::uint32_t type) { return type == 0x87 || type == 0x49 || type == 0x46; }

void elevator_move_write_00758c20(const std::uint8_t* message, NativeBitCursor* cursor, SessionMessageHost& host) {
    host.write_header_0075b480(message, cursor);
    write_native_bool_bit_004290b0(cursor, at<std::uint8_t>(message, 0x1C));
    write_native_bool_bit_004290b0(cursor, at<std::uint8_t>(message, 0x22));
    write_native_bool_bit_004290b0(cursor, at<std::uint8_t>(message, 0x23));
    if (at<std::uint8_t>(message, 0x23) != 0) write_native_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x20), 12);
}

void elevator_move_read_00758c70(std::uint8_t* message, NativeSessionReadStream& stream, SessionMessageHost& host) {
    host.read_header_0075b4c0(message, stream);
    NativeBitCursor* c = &stream.cursor_04;
    read_native_bool_bit_00428d70(c, reinterpret_cast<bool*>(message + 0x1C));
    read_native_bool_bit_00428d70(c, reinterpret_cast<bool*>(message + 0x22));
    read_native_bool_bit_00428d70(c, reinterpret_cast<bool*>(message + 0x23));
    if (at<std::uint8_t>(message, 0x23) != 0) {
        read_native_word_bits_00428e30(c, &at<std::uint16_t>(message, 0x20), 12);
        return;
    }
    at<std::uint16_t>(message, 0x20) = 0;
}

std::uint8_t* elevator_move_entity_00758cf0(const std::uint8_t* message, const EntityIdTable& ids) {
    if (at<std::uint8_t>(message, 0x23) == 0) return nullptr;
    return ids.entity(at<std::uint16_t>(message, 0x20));
}

std::uint8_t* mothership_create_00758d30(std::uint32_t vehicle_class, std::uint32_t row, MothershipHost& host) {
    void* memory = host.operator_new_00bf55be(0x12C8);
    host.memset_00bf79f0(memory, 0, 0x12C8);
    std::uint8_t* ship =
        memory != nullptr ? mothership_construct_00758550(static_cast<std::uint8_t*>(memory), row, host) : nullptr;
    host.set_vehicle_class_009553d0(ship, vehicle_class);
    at<std::uint32_t>(ship, 0x354) = vehicle_class;
    return ship;
}

void mothership_serialize_00758dc0(std::uint8_t* ship, ArchiveWriter& archive, MothershipHost& host) {
    at<std::uint32_t>(ship, 0x11A8) = 1;
    host.ship_serialize_00818340(ship, archive);
    archive.begin_04(0, "_airBase");  // 00CF88BC
    host.block_serialize_006c19d0(ship + 0x1188, archive);
    archive.end_08();
    archive.begin_04(0, "_motherShip");  // 00D01884
    archive.end_08();
}

void mothership_bind_elevator_node_00758e40(std::uint8_t* ship, MothershipHost& host) {
    std::uint8_t* model = at<std::uint8_t*>(ship, 0x360);
    std::uint8_t* scene = model != nullptr ? at<std::uint8_t*>(model, 0x160) : nullptr;
    std::uint8_t* node = host.find_node_0071ad50(scene, "elevator_2");  // 00D01890
    at<std::uint8_t*>(ship, 0x1284) = node;
    host.matrix_set_004134f0(ship + 0x1288, host.node_world_matrix_00b6db60(node));
}

void mothership_controller_refresh_00758e80(std::uint8_t* ship, float dt, MothershipHost& host) {
    host.ship_update_00811ab0(ship, dt);
    if (std::uint8_t* holder = at<std::uint8_t*>(ship, 0xEF8); holder != nullptr) host.holder_refresh_006bee40(holder, dt);
}

bool mothership_handle_message_00758eb0(std::uint8_t* ship, const std::uint8_t* message, const EntityIdTable& ids,
                                        const std::int32_t& tick_00f876b0, MothershipHost& host) {
    if (host.message_is_type_0c(message, 0x72)) {
        host.ship_vslot_220(ship, message);
        return true;
    }
    if (host.message_is_type_0c(message, 0x73)) {
        host.ship_vslot_224(ship, message);
        return true;
    }
    if (host.message_is_type_0c(message, 0x74)) {
        host.ship_vslot_220(ship, message);
        return true;
    }
    if (host.message_is_type_0c(message, 0x75)) {
        host.ship_vslot_224(ship, message);
        return true;
    }
    if (host.message_is_type_0c(message, 0x87)) {
        const float seconds = static_cast<float>(tick_00f876b0 - at<std::int32_t>(message, 0x0C));  // CVTSI2SS
        std::uint8_t* elevator = at<std::uint8_t*>(ship, 0x11C4);
        std::uint8_t* entity = elevator_move_entity_00758cf0(message, ids);
        host.elevator_move_50(elevator, at<std::uint8_t>(message, 0x22), entity, seconds);
        return true;
    }
    if (host.block_handle_message_006cd6c0(ship + 0x1188, message)) return true;
    return host.ship_handle_message_00821e80(ship, message);
}

void record4c_resize_tail_0074be90(std::uint8_t* vector, std::uint32_t n, LandingShipHost& host) {
    std::uint8_t fill[0x4C];  // deliberately not initialised: the image copies stack garbage
    host.record4c_resize_0074ba20(vector, n, fill);
}

// ---- LandConvoy member vector kill (packet cc13-convoy) ---------------------

void land_convoy_kill_member_vector_00742210(std::uint8_t* convoy, bool hard, LandConvoyKillVectorHost& host) {
    // 0074222D MOV BL,[ESP+14h]; 0074226C..00742271 SETNE / ADD 1.
    const std::int32_t cause = hard ? 2 : 1;
    const auto check = [&](std::size_t i) {
        if (!(i < host.member_slot_count(convoy))) host.invalid_parameter_noinfo_00bf6713();
    };
    for (std::size_t i = 0;; ++i) {
        if (i == host.member_slot_count(convoy)) break;             // 00742231, 00742247
        check(i);                                                    // 0074224B
        if (host.member_slot(convoy, i) != nullptr) {                // 00742255
            check(i);                                                // 0074225A
            if (std::uint8_t* member = host.member_slot(convoy, i); member != nullptr) {  // 00742264
                host.mission_entity_kill_00926d90(member, cause);   // 00742275
                host.null_member_slot(convoy, i);                    // 0074227A
            }
        }
        check(i);                                                    // 00742280
    }
}

}  // namespace bsp::cc13_seg41

#include "bsp/native_plane_float_readers.hpp"

namespace bsp::cc13_seg41 {
namespace {
std::uint32_t oct18b_x87_float_copy(const volatile void* input) {
    std::uint32_t bits;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        fstp dword ptr [bits]
    }
    return bits;
}
void* oct18b_copy_16_dwords(const std::uint8_t* source, void* out) {
    auto* destination = static_cast<std::uint8_t*>(out);
    for (std::uint32_t i = 0; i < 16; ++i) {
        std::uint32_t word;
        std::memcpy(&word, source + i * 4, 4);
        std::memcpy(destination + i * 4, &word, 4);
    }
    return out;
}
bool oct18b_runway_name(const std::uint8_t* name, MothershipFailureSourceHost& host) {
    const char* text = at<const char*>(name, 4);
    return text != nullptr && host.stricmp_00bf7fbf(text, "RunwayFailure") == 0;
}
}
void frame_hook_process_cloth_00737870(float dt, FrameClothHost& host) {
    const std::uint32_t bits = oct18b_x87_float_copy(&dt);
    std::memcpy(&dt, &bits, 4);
    host.flag_cloth_process_frame_008e2560(dt);
}
std::uint8_t land_convoy_slot164_false_00741110(void*, std::uint32_t) noexcept { return 0; }
void landing_ship_create_read_00749d20(std::uint8_t* message, NativeSessionReadStream& stream, LandingShipReadHost& host) {
    host.ship_create_read_00813bd0(message, stream);
    read_native_word_bits_00428e30(&stream.cursor_04, &at<std::uint16_t>(message, 0x12c), 13);
    read_native_word_bits_00428e30(&stream.cursor_04, &at<std::uint16_t>(message, 0x12e), 12);
    read_native_i32_bits_00428d30(&stream.cursor_04, &at<std::int32_t>(message, 0x130), 4);
}
void land_at_pad_write_00749e10(const std::uint8_t* message, NativeBitCursor* cursor) {
    write_native_session_message_header_0075b480(message, cursor);
    write_native_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x1c), 13);
    write_native_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x1e), 12);
}
void land_at_pad_read_00749e40(std::uint8_t* message, NativeSessionReadStream& stream) {
    read_native_session_message_header_0075b4c0(message, &stream);
    read_native_word_bits_00428e30(&stream.cursor_04, &at<std::uint16_t>(message, 0x1c), 13);
    read_native_word_bits_00428e30(&stream.cursor_04, &at<std::uint16_t>(message, 0x1e), 12);
}
void land_vehicle_create_read_0074ccb0(net_cc13::EntityCreateMessage& message, NativeSessionReadStream& stream, const NativeBitNumericContext& numeric) {
    net_cc13::read_entity_create_00767e30(message, stream, numeric);
}
std::uint8_t land_vehicle_handle_message_0074cd90(void* vehicle, void* message, LandVehicleMessageHost& host) {
    return host.unit_handle_message_0095abe0(vehicle, message);
}
void land_vehicle_message_write_0074cef0(const void* message, NativeBitCursor* cursor) { write_native_session_message_header_0075b480(message, cursor); }
void land_vehicle_message_read_0074cf00(void* message, NativeSessionReadStream& stream) { read_native_session_message_header_0075b4c0(message, &stream); }
void land_vehicle_message_write_0074d000(const void* message, NativeBitCursor* cursor) { write_native_session_message_header_0075b480(message, cursor); }
void land_vehicle_message_read_0074d010(void* message, NativeSessionReadStream& stream) { read_native_session_message_header_0075b4c0(message, &stream); }
void land_vehicle_register_ai_0074ddf0(std::uint8_t* vehicle, void* const& game, LandVehicleHost& host) {
    land_vehicle_vslot_b4_0074ddd0(vehicle, game, host);
}
std::uint8_t* plane_observer_0074e170(std::uint8_t* plane) noexcept { return plane ? plane + 0x72c : nullptr; }
void* plane_copy_matrix_e00_0074e270(const std::uint8_t* plane, void* out) { return oct18b_copy_16_dwords(plane + 0xe00, out); }
void* plane_copy_matrix_e40_0074e290(const std::uint8_t* plane, void* out) { return oct18b_copy_16_dwords(plane + 0xe40, out); }
float plane_observer_speed_0074e3c0(const std::uint8_t* observer) { return native_plane_cached_speed_007b8e60(observer - 0x72c, nullptr); }
float* plane_observer_vector_0074e3f0(const std::uint8_t* observer, float* out) {
    const auto* plane = observer - 0x72c;
    for (std::uint32_t i = 0; i < 3; ++i) {
        const std::uint32_t bits = oct18b_x87_float_copy(plane + 0xac8 + 4 * i);
        std::memcpy(out + i, &bits, 4);
    }
    return out;
}
void ship_sink_write_0074eb20(const std::uint8_t* message, NativeBitCursor* cursor, const volatile float& scale, const NativeBitNumericContext& numeric) {
    write_native_session_message_header_0075b480(message, cursor);
    write_native_unsigned_dword_bits_00429070(cursor, at<std::uint32_t>(message, 0x1c), 6);
    for (std::uint32_t i = 0; i < at<std::uint32_t>(message, 0x1c); ++i) {
        const auto scale_bits = oct18b_x87_float_copy(&scale);
        const auto value_bits = oct18b_x87_float_copy(message + 0x20 + 4 * i);
        write_native_numeric_float_004295c0(cursor, value_bits, 0, 1, scale_bits, 32, numeric);
    }
}
void ship_sink_read_0074eb80(std::uint8_t* message, NativeSessionReadStream& stream, const volatile float& scale, const NativeBitNumericContext& numeric) {
    read_native_session_message_header_0075b4c0(message, &stream);
    read_native_u32_bits_00428d10(&stream.cursor_04, &at<std::uint32_t>(message, 0x1c), 6);
    for (std::uint32_t i = 0; i < at<std::uint32_t>(message, 0x1c); ++i) {
        const auto scale_bits = oct18b_x87_float_copy(&scale);
        std::uint32_t value_bits;
        read_native_numeric_float_004293f0(&stream.cursor_04, &value_bits, 0, 1, scale_bits, 32, numeric);
        // Original MOVSS publishes the raw bits, with no second x87 load.
        std::memcpy(message + 0x20 + 4 * i, &value_bits, 4);
    }
}
float mothership_air_ops_speed_00758200() noexcept { return 25.0f; }
std::uint8_t* mothership_air_ops_class_00758230(const std::uint8_t* block) {
    std::uint8_t* owner_class;
    std::memcpy(&owner_class, block - 0xc50, sizeof owner_class);
    return owner_class + 0x830;
}
std::uint8_t mothership_get_failure_007589d0(std::uint8_t* ship, const std::uint8_t* name, std::uint32_t frame, std::uint8_t* out, MothershipFailureSourceHost& host) {
    if (oct18b_runway_name(name, host)) { *out = ship[0x11a4]; return 1; }
    if (host.native_string_equals_00425850(name, "HangarFailure")) { *out = ship[0x11a5]; return 1; }
    return host.ship_get_failure_008135c0(ship, name, frame, out);
}
std::uint8_t mothership_failure_expired_00758a50(std::uint8_t* ship, const std::uint8_t* name, std::uint32_t frame, const NativeSessionMessageContext& ctx, std::uint32_t clear_value, MothershipFailureSourceHost& host) {
    const bool runway = oct18b_runway_name(name, host);
    if (!runway && !host.native_string_equals_00425850(name, "HangarFailure"))
        return host.ship_failure_expired_00813660(ship, name, frame);
    // The image leaves reserved message bytes uninitialised. Only the same
    // fields are written here; route_message must consume the message at call.
    alignas(4) std::uint8_t message[0x24];
    construct_native_session_message_0075b430(reinterpret_cast<NativeSessionMessageStorage*>(message), runway ? 0x73 : 0x75, ctx);
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint16_t>(message, 0x18) = 0;
    message[0x1a] = 0;
    message[0x1c] = 0;
    at<std::uint32_t>(message, 0x20) = clear_value;
    at<const std::uint32_t*>(message, 0) = host.vtable(runway ? 0x00cf8b54 : 0x00cf8b7c);
    host.route_message_0077c2a0(ship, message, 7, 0);
    return 1;
}
}
