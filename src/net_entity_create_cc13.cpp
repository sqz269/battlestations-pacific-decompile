// The generic net-entity create message. See include/bsp/net_entity_create_cc13.hpp
// and docs/CC13_MULTIPLAYER_SESSION.md.
#include "bsp/net_entity_create_cc13.hpp"

#include <cmath>
#include <cstring>

#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric_array.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp::net_cc13 {

namespace {

using U = std::uint32_t;

U float_bits(float value) noexcept {
    U bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float bits_float(U bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// Every numeric float of this codec: (zero 0, signed 1, FLT_MAX scale, 32 bits).
void write_float(NativeBitCursor* c, float value, const NativeBitNumericContext& n) {
    write_native_numeric_float_004295c0(c, float_bits(value), 0, 1, kFltMaxBits, 32, n);
}

float read_float(NativeBitCursor* c, const NativeBitNumericContext& n) {
    U bits = 0;
    read_native_numeric_float_004293f0(c, &bits, 0, 1, kFltMaxBits, 32, n);
    return bits_float(bits);
}

void write_float3(NativeBitCursor* c, const float (&values)[3], const NativeBitNumericContext& n) {
    U bits[3] = {float_bits(values[0]), float_bits(values[1]), float_bits(values[2])};
    write_native_numeric_float_array_00429790(c, bits, 3, 0, 1, kFltMaxBits, 32, n);
}

void read_float3(NativeBitCursor* c, float (&values)[3], const NativeBitNumericContext& n) {
    U bits[3] = {};
    read_native_numeric_float_array_004294f0(c, bits, 3, 0, 1, kFltMaxBits, 32, n);
    for (int i = 0; i < 3; ++i) values[i] = bits_float(bits[i]);
}

}  // namespace

EntityCreateMessage build_entity_create_00766fc0(std::uint8_t type, std::uint16_t word_1e,
                                                 const EntityCreateSource& entity,
                                                 bool with_name, EntityCreateHost& host) {
    EntityCreateMessage m;
    m.type_10 = type;                 // 0075B430(type)
    m.net_id_18 = 0;                  // 00766FED
    m.flag_1a = 0;
    m.has_name_bc = with_name ? 1 : 0;  // 00767016
    m.net_id_1c = entity.net_id_174;
    m.word_1e = word_1e;
    m.word_40 = entity.word_28c;
    m.parent_id_20 = entity.has_parent_3c ? entity.parent_net_id : 0;
    for (int i = 0; i < 3; ++i) m.position_24[i] = entity.position_a4[i];
    host.basis_angles_0042d2e0(m.angles_30);

    // 00767089..00767116: the length of matrix row 0 (x87 at the MSVC 53-bit
    // precision setting): (y*y + x*x) + z*z spilled to float, compared with
    // the double 1e-10 (00CE3820); sqrt of the spilled float, spilled again.
    const double x = entity.matrix_row0_74[0];
    const double y = entity.matrix_row0_74[1];
    const double z = entity.matrix_row0_74[2];
    const float square = static_cast<float>((y * y + x * x) + z * z);
    const float scale = static_cast<double>(square) > 1e-10
                            ? static_cast<float>(std::sqrt(static_cast<double>(square)))
                            : 0.0f;
    m.scale_104 = scale;
    // FSUB double 1.0 (00D7A210), spilled to float, sign bit cleared, then
    // compared with the double at 00D7A268 (1e-4f widened).
    const float delta = std::fabs(static_cast<float>(static_cast<double>(scale) - 1.0));
    m.non_unit_scale_100 = static_cast<double>(delta) > static_cast<double>(1e-4f) ? 1 : 0;

    m.byte_3c = entity.byte_5c;
    m.word_58 = 0;                    // 00767122, EDI = 0
    m.word_5c = 1;
    m.has_pairs_44 = 0;
    if (!host.is_kind_5c(0x2A)) {     // 0076713D
        m.has_words_88 = 1;
        for (int i = 0; i < 9; ++i) m.words_8c[i] = entity.words_1ac[i];
    } else {
        m.has_words_88 = 0;
    }
    m.word_b0 = entity.word_54;
    for (int i = 0; i < 9; ++i) m.words_64[i] = entity.words_188[i];  // REP MOVSD
    m.uniform_60 = 1;
    for (int i = 1; i < 9; ++i) {
        if (entity.words_188[i] != entity.words_188[0]) {
            m.uniform_60 = 0;
            break;
        }
    }
    if (host.is_kind_5c(5)) {
        m.has_b8_b4 = 1;
        m.word_b8 = host.vcall_12c();
    } else {
        m.has_b8_b4 = 0;
    }
    m.name_present_c0 = false;        // +C0h = 0
    if (m.has_name_bc != 0) {
        const char* name = host.name_10();  // 00438E40 duplicates it
        if (name != nullptr) {
            m.name_c0 = name;
            m.name_present_c0 = true;
        }
    }
    m.byte_c4 = entity.byte_58;
    m.in_group_c5 = 0;
    if (host.is_kind_5c(2) && host.group_present_284()) {
        m.in_group_c5 = 1;
        m.group_kind_ca = host.group_kind_4fc();
        m.group_byte_cb = host.group_byte_500();
        m.leader_id_c8 = host.group_leader_net_id();
        m.is_leader_c6 = host.group_leader_is_self() ? 1 : 0;
        if (m.is_leader_c6 == 0) {
            if (m.group_kind_ca == 0x18) {
                const EntityCreateGroupMember member = host.group_member(0x18);
                for (int i = 0; i < 3; ++i) m.member_vector_cc[i] = member.vector[i];
            } else if (m.group_kind_ca == 0x06) {
                const EntityCreateGroupMember member = host.group_member(0x06);
                for (int i = 0; i < 4; ++i) {
                    m.member_a_d8[i] = member.record_10[i];
                    m.member_b_ec[i] = member.record_20[i];
                }
            }
        }
    }
    m.float_108 = entity.float_304;
    m.word_10c = entity.word_180;
    return m;
}

void write_entity_create_00765000(const EntityCreateMessage& m, NativeBitCursor* c,
                                  const NativeBitNumericContext& n) {
    write_native_byte_bits_00428ff0(c, m.type_10, 8);
    write_native_word_bits_00429120(c, m.net_id_18, 12);
    write_native_bool_bit_004290b0(c, m.flag_1a);
    write_native_word_bits_00429120(c, m.net_id_1c, 12);
    write_native_signed_word_bits_00429030(c, m.word_1e, 10);
    write_native_word_bits_00429120(c, m.parent_id_20, 13);
    write_float3(c, m.position_24, n);
    write_float3(c, m.angles_30, n);
    write_native_bool_bit_004290b0(c, m.byte_3c);
    write_native_unsigned_dword_bits_00429070(c, m.word_40, 16);
    write_native_bool_bit_004290b0(c, m.has_pairs_44);
    if (m.has_pairs_44 != 0) {
        write_native_signed_word_bits_00429030(c, static_cast<U>(m.pairs_4c.size()), 16);
        for (const auto& pair : m.pairs_4c) {
            write_float(c, pair[0], n);
            write_float(c, pair[1], n);
        }
    }
    write_native_unsigned_dword_bits_00429070(c, m.word_58, 2);
    write_native_unsigned_dword_bits_00429070(c, static_cast<U>(m.word_5c + 1), 4);
    write_native_bool_bit_004290b0(c, m.uniform_60);
    if (m.uniform_60 == 0) {
        for (U word : m.words_64) write_native_unsigned_dword_bits_00429070(c, word, 7);
    } else {
        write_native_unsigned_dword_bits_00429070(c, m.words_64[0], 7);
    }
    write_native_bool_bit_004290b0(c, m.has_words_88);
    if (m.has_words_88 != 0) {
        for (std::int32_t word : m.words_8c)
            write_native_signed_dword_bits_00429090(c, static_cast<U>(word), 7);
    }
    write_native_unsigned_dword_bits_00429070(c, m.word_b0, 4);
    write_native_bool_bit_004290b0(c, m.has_b8_b4);
    if (m.has_b8_b4 != 0) write_native_unsigned_dword_bits_00429070(c, m.word_b8, 3);
    write_native_bool_bit_004290b0(c, m.has_name_bc);
    if (m.has_name_bc != 0) {
        // The image passes +C0h unchecked; a null name would fault in 004290D0.
        write_native_bit_string_004290d0(c, m.name_c0.c_str());
    }
    write_native_byte_bits_00428ff0(c, m.byte_c4, 4);
    write_native_bool_bit_004290b0(c, m.in_group_c5);
    if (m.in_group_c5 != 0) {
        write_native_byte_bits_00428ff0(c, m.group_kind_ca, 8);
        write_native_byte_bits_00428ff0(c, m.group_byte_cb, 8);
        write_native_word_bits_00429120(c, m.leader_id_c8, 12);
        write_native_bool_bit_004290b0(c, m.is_leader_c6);
        if (m.is_leader_c6 == 0) {
            if (m.group_kind_ca == 0x18) {
                write_float3(c, m.member_vector_cc, n);
            } else {
                // Interleaved: +D8h+i*4 then +ECh+i*4.
                for (int i = 0; i < 4; ++i) {
                    write_float(c, m.member_a_d8[i], n);
                    write_float(c, m.member_b_ec[i], n);
                }
            }
        }
    }
    write_native_bool_bit_004290b0(c, m.non_unit_scale_100);
    if (m.non_unit_scale_100 != 0) write_float(c, m.scale_104, n);
    write_float(c, m.float_108, n);
    write_native_unsigned_dword_bits_00429070(c, m.word_10c, 4);
}

void read_entity_create_00767e30(EntityCreateMessage& m, NativeSessionReadStream& stream,
                                 const NativeBitNumericContext& n) {
    NativeBitCursor* c = &stream.cursor_04;
    read_native_u8_bits_00428c70(c, &m.type_10, 8);
    read_native_word_bits_00428e30(c, &m.net_id_18, 12);
    bool flag = false;
    read_native_bool_bit_00428d70(c, &flag);
    m.flag_1a = flag ? 1 : 0;
    read_native_word_bits_00428e30(c, &m.net_id_1c, 12);
    // Unsigned 10-bit read (00428CB0): a negative word_1e written by 00429030
    // comes back zero-extended (-1 -> 3FFh).
    read_native_u16_bits_00428cb0(c, &m.word_1e, 10);
    read_native_word_bits_00428e30(c, &m.parent_id_20, 13);
    read_float3(c, m.position_24, n);
    read_float3(c, m.angles_30, n);
    read_native_bool_bit_00428d70(c, &flag);
    m.byte_3c = flag ? 1 : 0;
    read_native_u32_bits_00428d10(c, &m.word_40, 16);
    read_native_bool_bit_00428d70(c, &flag);
    m.has_pairs_44 = flag ? 1 : 0;
    if (m.has_pairs_44 != 0) {
        std::uint16_t count = 0;
        read_native_u16_bits_00428cb0(c, &count, 16);
        m.pairs_4c.resize(count, {0.0f, 0.0f});  // 00766E30(count, {0, 0}) on +48h
        for (auto& pair : m.pairs_4c) {
            pair[0] = read_float(c, n);
            pair[1] = read_float(c, n);
        }
    }
    read_native_u32_bits_00428d10(c, &m.word_58, 2);
    U word = 0;
    read_native_u32_bits_00428d10(c, &word, 4);
    m.word_5c = static_cast<std::int32_t>(word) - 1;
    read_native_bool_bit_00428d70(c, &flag);
    m.uniform_60 = flag ? 1 : 0;
    if (m.uniform_60 == 0) {
        for (U& w : m.words_64) read_native_u32_bits_00428d10(c, &w, 7);
    } else {
        read_native_u32_bits_00428d10(c, &m.words_64[0], 7);
        for (int i = 1; i < 9; ++i) m.words_64[i] = m.words_64[0];
    }
    read_native_bool_bit_00428d70(c, &flag);
    m.has_words_88 = flag ? 1 : 0;
    if (m.has_words_88 != 0) {
        for (std::int32_t& w : m.words_8c) read_native_i32_bits_00428d30(c, &w, 7);
    }
    read_native_u32_bits_00428d10(c, &m.word_b0, 4);
    read_native_bool_bit_00428d70(c, &flag);
    m.has_b8_b4 = flag ? 1 : 0;
    if (m.has_b8_b4 != 0) read_native_u32_bits_00428d10(c, &m.word_b8, 3);
    read_native_bool_bit_00428d70(c, &flag);
    m.has_name_bc = flag ? 1 : 0;
    if (m.has_name_bc != 0) {
        char* text = nullptr;
        allocate_native_bit_string_00428da0(c, &text);  // stored at +C0h in the image
        m.name_present_c0 = text != nullptr;
        m.name_c0 = text != nullptr ? text : "";
        singleton_lifetime_free(text);
    }
    read_native_u8_bits_00428c70(c, &m.byte_c4, 4);
    read_native_bool_bit_00428d70(c, &flag);
    m.in_group_c5 = flag ? 1 : 0;
    if (m.in_group_c5 != 0) {
        read_native_u8_bits_00428c70(c, &m.group_kind_ca, 8);
        read_native_u8_bits_00428c70(c, &m.group_byte_cb, 8);
        read_native_word_bits_00428e30(c, &m.leader_id_c8, 12);
        read_native_bool_bit_00428d70(c, &flag);
        m.is_leader_c6 = flag ? 1 : 0;
        if (m.is_leader_c6 == 0) {
            if (m.group_kind_ca == 0x18) {
                read_float3(c, m.member_vector_cc, n);
            } else {
                for (int i = 0; i < 4; ++i) {
                    m.member_a_d8[i] = read_float(c, n);
                    m.member_b_ec[i] = read_float(c, n);
                }
            }
        }
    }
    read_native_bool_bit_00428d70(c, &flag);
    m.non_unit_scale_100 = flag ? 1 : 0;
    if (m.non_unit_scale_100 != 0) m.scale_104 = read_float(c, n);
    m.float_108 = read_float(c, n);
    read_native_u32_bits_00428d10(c, &m.word_10c, 4);
}

}  // namespace bsp::net_cc13
