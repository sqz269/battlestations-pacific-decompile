// Packet cc13-big47b (cc13_big47b marker). See include/bsp/cc13_big47b.hpp and
// docs/CC13_BIG47B.md. Bodies were read from the exported listings; every x87
// operand order was checked against the bytes, constants were read from the PE.
//
// Precision: the PLANE_SYNC codec runs where session messages are built and
// parsed, the main thread (24-bit precision control, docs/CC13_FPU_PRECISION.md).
// Under 24-bit PC each x87 add/sub/div here rounds once to float, which is what
// the float operations below compute. At 53-bit PC the FADD/FSUB of the double
// 150.0 / 80.0 and the FDIVs would round twice (to 53 then to 24 bits on FSTP);
// that is recorded as uncertainty, not modelled.

#include "bsp/cc13_big47b.hpp"

#include <cstring>

#include "bsp/cc13_seg47.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"

namespace bsp::cc13_big47b {
namespace {

using U = std::uint32_t;
using cc13_seg47::Raw;
using cc13_seg47::raw;

U bits_of(float v) noexcept {
    U b;
    std::memcpy(&b, &v, sizeof b);
    return b;
}
float float_of(U b) noexcept {
    float v;
    std::memcpy(&v, &b, sizeof v);
    return v;
}
// FLD m32 then FSTP m32: bitwise, except that a signalling NaN comes out quiet
// (invalid operation masked).
U x87_copy_bits(float v) noexcept {
    U b = bits_of(v);
    if ((b & 0x7F800000u) == 0x7F800000u && (b & 0x007FFFFFu) != 0) b |= 0x00400000u;
    return b;
}
float x87_copy(float v) noexcept { return float_of(x87_copy_bits(v)); }

// The image's magnitude: d when d > 0 (ordered), else -0.0f - d (00D7A208).
// A NaN takes the subtraction and stays NaN, so every "> threshold" below is
// false for it.
float magnitude(float d) noexcept { return d > 0.0f ? d : -0.0f - d; }

// .rdata constants (bytes read from the PE).
constexpr float kPosHigh_00d02f64 = 24000.0f;
constexpr float kPosLow_00d02f68 = -24000.0f;
constexpr float kHeightScale_00d05b5c = 2048.0f;
constexpr float kHeightBias_00ce3dd8 = 150.0f;   // stored as double 150.0; exact in float
constexpr double kHeightMax_00d05b68 = 2047.0;   // double, the compare
constexpr float kHeightMaxF_00d05b60 = 2047.0f;  // float, the stored value
constexpr float kPi_00d7a264 = 3.14159274f;      // 40490FDBh
constexpr float kAngleEps_00d05b58 = 0.0005f;    // 3A03126Fh
constexpr double kVelSmall_00ce3d68 = 60.0;
constexpr float kVelSmallScale_00ceb4b0 = 60.0f;
constexpr float kScale1024_00d05b54 = 1024.0f;
constexpr float kRateSmall_00ce3800 = 0.5f;
constexpr float kRateScale_00ce6630 = 6.0f;
constexpr float kRatioEps_00d7a238 = 0.01f;            // 3C23D70Ah
constexpr double kPairEps_00d7a270 = double(0.05f);    // 3FA99999A0000000h
constexpr float kPairHigh_00ce3d08 = 100.0f;
constexpr float kPairLow_00ce65d8 = -100.0f;
constexpr double kDivisorEps_00d7a3a0 = double(0.1f);  // 3FB99999A0000000h
constexpr float kDivisorBias_00cf1440 = 80.0f;         // stored as double 80.0; exact in float
constexpr float kRateHigh_00ce3918 = 8.0f;
constexpr float kRateLow_00ce3cc8 = -8.0f;
constexpr float kUnitEps_00d05b50 = 0.08f;  // 3DA3D70Ah
constexpr float kOne_00d7a24c = 1.0f;

void write_bool(NativeBitCursor* c, std::uint32_t v) { write_native_bool_bit_004290b0(c, v); }
void write_num(NativeBitCursor* c, float value, U zero_flag, U signed_flag, float scale, U bits,
               const NativeBitNumericContext& n) {
    // Both value and scale reach 004295C0 through FLD m32 / FSTP [ESP].
    write_native_numeric_float_004295c0(c, x87_copy_bits(value), zero_flag, signed_flag, x87_copy_bits(scale),
                                        bits, n);
}
void read_bool(NativeBitCursor* c, std::uint8_t* out) {
    read_native_bool_bit_00428d70(c, reinterpret_cast<bool*>(out));
}
void read_num(NativeBitCursor* c, float* out, U zero_flag, U signed_flag, float scale, U bits,
              const NativeBitNumericContext& n) {
    read_native_numeric_float_004293f0(c, reinterpret_cast<U*>(out), zero_flag, signed_flag, x87_copy_bits(scale),
                                       bits, n);
}

// The 1-bit-then-value delta field shared by the writer's sections: send when
// the value moved past the threshold, when the message forces it (+9Ch), on
// delivery 1, or when the record's counter says the peer is stale; otherwise
// a single 0 bit. `stale` is the counter test the image makes for that field.
// Returns whether the value was sent; on send the record keeps the value and
// the counter becomes 0 (moved) or -1 (re-sent unchanged).
struct DeltaSlot {
    std::size_t base_off;     // record float the value is compared with
    std::size_t counter_off;  // record dword
};

}  // namespace

// 007C7A40 BSP_SessionMessage_PLANE_SYNC_Write, __thiscall(ECX = message,
// cursor) RET 4, no SEH frame, SUB ESP,18h. Callers: slot +4h of 00D02E0C.
// The cursor goes to every bit writer as ECX unchanged. `delivery_one` is the
// SETZ of message+4 == 1 the image parks in its own argument slot (+24h/+2Ch).
// Record (+1Ch, the 007C2810 per-peer record): floats +0/+4/+8/+0Ch/+10h,
// +48h..+50h, +60h..+68h, +78h..+80h are the last values sent; dwords
// +18h..+28h, +3Ch..+44h, +54h..+5Ch, +6Ch..+74h are their counters (007B8550
// decrements them every call); +2Ch is the full-form countdown.
// The bool argument the image pushes for +A0h is EAX after SETZ AL (upper bytes
// stale); 004290B0 tests only the low byte, so the value passed is the flag.
void plane_sync_message_write_007c7a40(PlaneSyncMessage* m, NativeBitCursor* c, PlaneSyncHost& host,
                                       const NativeBitNumericContext& n) {
    write_native_session_message_header_0075b480(m, c);  // 007C7A4C
    const bool delivery_one = m->base.delivery_04 == 1;  // 007C7A51
    auto rec = [m] { return Raw{static_cast<std::uint8_t*>(m->peer_record_1c)}; };
    if (m->state_8c != 7) rec().at<std::int32_t>(0x2C) = 5;  // 007C7A66
    m->compact_a0 = rec().at<std::int32_t>(0x2C) == 0 ? 1 : 0;
    write_bool(c, m->compact_a0);                                // 007C7A80
    cc13_seg47::index_block_decrement_all_007b8550(rec());       // 007C7A88, ECX = record
    if (m->compact_a0 == 0) {
        m->level_94 = 0;
        m->skip_9d = 0;
    } else {
        write_native_unsigned_dword_bits_00429070(c, static_cast<U>(m->level_94), 2);  // 007C7AA1
        if (m->level_94 != 0)
            m->skip_9d = 0;
        else
            write_bool(c, m->skip_9d);  // 007C7AB9
    }

    // --- position and angles (007C7AE0-007C7DC0) ---
    if (m->skip_9d == 0) {
        const std::int32_t level = m->level_94;
        const U pos_bits = static_cast<U>(0x16 - level * 3);
        const U height_bits = static_cast<U>(0x14 - level * 3);
        auto clamp_pos = [](float v) {
            // COMISS low,v / JBE; COMISS v,high / JBE: NaN passes through.
            if (kPosLow_00d02f68 > v) return kPosLow_00d02f68;
            if (v > kPosHigh_00d02f64) return kPosHigh_00d02f64;
            return v;
        };
        write_num(c, clamp_pos(m->pos_x_2c), 0, 1, kPosHigh_00d02f64, pos_bits, n);  // 007C7B45
        // FLD +30h, FADD double 150.0, FSTP float: one rounding under 24-bit PC.
        float h = m->pos_y_30 + kHeightBias_00ce3dd8;
        if (0.0f > h)
            h = 0.0f;  // XORPS: +0.0
        else if (static_cast<double>(h) > kHeightMax_00d05b68)
            h = kHeightMaxF_00d05b60;
        write_num(c, h, 0, 0, kHeightScale_00d05b5c, height_bits, n);              // 007C7BA5
        write_num(c, clamp_pos(m->pos_z_34), 0, 1, kPosHigh_00d02f64, pos_bits, n);  // 007C7BF8

        const U angle_bits = static_cast<U>((7 - m->level_94) * 2);
        for (int i = 0; i < 3; ++i) {
            const std::size_t base = 0x48 + 4 * i, counter = 0x3C + 4 * i;
            const float v = m->angle_20[i];
            const bool moved = magnitude(v - rec().at<float>(base)) > kAngleEps_00d05b58;
            if (!moved && m->force_9c == 0 && m->level_94 != 0 && !delivery_one &&
                rec().at<std::int32_t>(counter) >= 0) {
                write_bool(c, 0);
                continue;
            }
            write_bool(c, 1);
            write_num(c, v, 0, 1, kPi_00d7a264, angle_bits, n);
            rec().at<float>(base) = x87_copy(m->angle_20[i]);
            rec().at<std::int32_t>(counter) = moved ? 0 : -1;
        }
    }

    if (m->compact_a0 == 0) {
        // --- full form: velocities and rates, unconditionally (007C7DD5-007C807F) ---
        for (int i = 0; i < 3; ++i) {
            const float v = m->vel_50[i];
            if (kVelSmall_00ce3d68 > static_cast<double>(magnitude(v))) {
                write_bool(c, 1);
                write_num(c, v, 0, 1, kVelSmallScale_00ceb4b0, 10, n);
            } else {
                write_bool(c, 0);
                write_num(c, v, 0, 1, kScale1024_00d05b54, 16, n);
            }
        }
        for (int i = 0; i < 3; ++i) {
            const float v = m->rate_68[i];
            if (kRateSmall_00ce3800 > magnitude(v)) {
                write_bool(c, 1);
                write_num(c, v, 0, 1, kRateSmall_00ce3800, 14, n);
            } else {
                write_bool(c, 0);
                write_num(c, v, 0, 1, kRateScale_00ce6630, 18, n);
            }
        }
    } else if (m->skip_9d == 0) {
        // --- compact form: delta fields (007C8091-007C867C) ---
        // The x87 copies +5Ch/+60h/+64h to locals, then +5Ch/+64h and +60h/+64h
        // (FDIVP ST2 / FDIVR, operand order confirmed in the bytes).
        const float rx = m->pair_5c / m->divisor_64;
        const float ry = m->pair_60 / m->divisor_64;
        std::uint8_t small = 0;  // byte [ESP+10h]; 004290B0 tests only that byte
        if (kRatioEps_00d7a238 > magnitude(rx) && kRatioEps_00d7a238 > magnitude(ry)) small = 1;
        write_bool(c, small);  // 007C8121
        const U pair_bits = static_cast<U>((8 - m->level_94) * 2);
        const U divisor_bits = static_cast<U>((9 - m->level_94) * 2);
        if (small) {
            rec().at<std::int32_t>(0x58) = 0;
            rec().at<std::int32_t>(0x54) = 0;
        } else {
            const DeltaSlot slots[2] = {{0x60, 0x54}, {0x64, 0x58}};
            float* const fields[2] = {&m->pair_5c, &m->pair_60};
            for (int i = 0; i < 2; ++i) {
                const float v = *fields[i];
                const bool moved =
                    static_cast<double>(magnitude(v - rec().at<float>(slots[i].base_off))) > kPairEps_00d7a270;
                if (!moved && !delivery_one && m->force_9c == 0 &&
                    rec().at<std::int32_t>(slots[i].counter_off) >= 0) {
                    write_bool(c, 0);
                    continue;
                }
                write_bool(c, 1);
                // 00415620(ECX = &field, EDX = &-100.0f local, [&100.0f local]) -> ST0.
                const float sent = clamp_float_by_ref_00415620(*fields[i], kPairLow_00ce65d8, kPairHigh_00ce3d08);
                write_num(c, sent, 0, 1, kPairHigh_00ce3d08, pair_bits, n);
                rec().at<float>(slots[i].base_off) = x87_copy(*fields[i]);
                rec().at<std::int32_t>(slots[i].counter_off) = moved ? 0 : -1;
            }
        }
        {
            const float v = m->divisor_64;
            const bool moved = static_cast<double>(magnitude(v - rec().at<float>(0x68))) > kDivisorEps_00d7a3a0;
            if (!moved && !delivery_one && m->force_9c == 0 && rec().at<std::int32_t>(0x5C) >= 0) {
                write_bool(c, 0);
            } else {
                write_bool(c, 1);
                float t = v + kDivisorBias_00cf1440;  // FADD double 80.0, FSTP float
                if (0.0f > t)
                    t = 0.0f;
                else if (t > kScale1024_00d05b54)
                    t = kScale1024_00d05b54;
                write_num(c, t, 0, 0, kScale1024_00d05b54, divisor_bits, n);  // 007C8391
                rec().at<float>(0x68) = x87_copy(m->divisor_64);
                rec().at<std::int32_t>(0x5C) = moved ? 0 : -1;
            }
        }
        const U rate_bits = static_cast<U>(0x0F - m->level_94 * 2);
        for (int i = 0; i < 3; ++i) {
            const std::size_t base = 0x78 + 4 * i, counter = 0x6C + 4 * i;
            const float v = m->rate_68[i];
            const bool moved = magnitude(v - rec().at<float>(base)) > kAngleEps_00d05b58;
            // +6Ch/+70h re-send once negative (JL 0); +74h only once above 12 (JG 0Ch).
            const std::int32_t count = rec().at<std::int32_t>(counter);
            const bool stale = i < 2 ? count < 0 : count > 0x0C;
            if (!moved && !delivery_one && m->force_9c == 0 && !stale) {
                write_bool(c, 0);
                continue;
            }
            write_bool(c, 1);
            float sent = v;
            if (kRateLow_00ce3cc8 > v)
                sent = kRateLow_00ce3cc8;
            else if (v > kRateHigh_00ce3918)
                sent = kRateHigh_00ce3918;
            write_num(c, sent, 0, 1, kRateHigh_00ce3918, rate_bits, n);
            rec().at<float>(base) = x87_copy(m->rate_68[i]);
            rec().at<std::int32_t>(counter) = moved ? 0 : -1;
        }
    }

    // --- tail (007C867F) ---
    write_bool(c, m->flag_9f);
    if (m->skip_9d == 0) {
        // +44h is clamped to [0,1] in the message itself (MOVSS store; NaN kept).
        {
            float v = m->unit_44;
            if (0.0f > v)
                v = 0.0f;
            else if (v > kOne_00d7a24c)
                v = kOne_00d7a24c;
            m->unit_44 = v;
        }
        struct UnitSlot {
            float PlaneSyncMessage::*field;
            std::size_t base_off, counter_off;
            U zero_flag, signed_flag, bits;
            bool counter_negative_test;  // JL 0 for +44h, JG 18h for the rest
        };
        const UnitSlot units[5] = {
            {&PlaneSyncMessage::unit_44, 0x0C, 0x18, 1, 0, 7, true},
            {&PlaneSyncMessage::unit_38, 0x00, 0x1C, 1, 1, 8, false},
            {&PlaneSyncMessage::unit_40, 0x08, 0x20, 1, 1, 8, false},
            {&PlaneSyncMessage::unit_3c, 0x04, 0x24, 1, 1, 8, false},
            {&PlaneSyncMessage::unit_48, 0x10, 0x28, 1, 0, 7, false},
        };
        for (const UnitSlot& u : units) {
            // Record minus message here (FLD record, FSUB message).
            const bool moved = magnitude(rec().at<float>(u.base_off) - m->*u.field) > kUnitEps_00d05b50;
            const std::int32_t count = rec().at<std::int32_t>(u.counter_off);
            const bool stale = u.counter_negative_test ? count < 0 : count > 0x18;
            if (!delivery_one && m->force_9c == 0 && !moved && !stale) {
                write_bool(c, 0);
                continue;
            }
            write_bool(c, 1);
            write_num(c, m->*u.field, u.zero_flag, u.signed_flag, kOne_00d7a24c, u.bits, n);  // FLD1 scale
            rec().at<std::int32_t>(u.counter_off) = 0;  // MOV [rec+cnt],EBP (EBP = 0), then overwritten
            rec().at<float>(u.base_off) = x87_copy(m->*u.field);
            rec().at<std::int32_t>(u.counter_off) = moved ? 0 : -1;
        }
        write_bool(c, m->flag_4e);
        write_bool(c, m->flag_75);
        write_bool(c, m->flag_74);
    }
    if (m->compact_a0 == 0) {
        write_native_byte_bits_00428ff0(c, m->state_8c, 3);  // 007C8A26
        write_bool(c, m->flag_9e);
        if (m->state_8c == 4) {
            const Raw tuning{static_cast<std::uint8_t*>(host.game_tuning_get_singleton_0042e740())};  // 007C8A43
            write_num(c, m->wire_rope_78, 0, 0, tuning.at<float>(0x51C), 10, n);                    // 007C8A61
        }
    }
    if (rec().at<std::int32_t>(0x2C) > 0) rec().at<std::int32_t>(0x2C) += -1;  // 007C8A6F
}

// 007C8A80 BSP_SessionMessage_PLANE_SYNC_Read, __thiscall(ECX = message,
// stream) RET 4, no SEH frame. Callers: slot +8h of 00D02E0C. Every bit
// reader gets stream+4 (EDI). The three per-field "small" bools of the full
// form are read into the low byte of the image's argument slot ([ESP+18h]);
// the compact form's "both small" bool into [ESP+13h]; here they are locals.
// The image never reads +9Ch, +94h on the full form stays as stored, and
// +4Ch/+4Dh are untouched.
void plane_sync_message_read_007c8a80(PlaneSyncMessage* m, NativeSessionReadStream* stream, PlaneSyncHost& host,
                                      const NativeBitNumericContext& n) {
    read_native_session_message_header_0075b4c0(m, stream);  // 007C8A8C
    NativeBitCursor* const c = &stream->cursor_04;
    m->skip_9d = 0;
    read_bool(c, &m->compact_a0);  // 007C8AA7
    if (m->compact_a0 != 0) {
        read_native_u32_bits_00428d10(c, reinterpret_cast<std::uint32_t*>(&m->level_94), 2);  // 007C8ABC
        if (m->level_94 == 0) read_bool(c, &m->skip_9d);
    } else {
        m->level_94 = 0;
    }

    if (m->skip_9d == 0) {
        const std::int32_t level = m->level_94;
        const U pos_bits = static_cast<U>(0x16 - level * 3);
        read_num(c, &m->pos_x_2c, 0, 1, kPosHigh_00d02f64, pos_bits, n);
        read_num(c, &m->pos_y_30, 0, 0, kHeightScale_00d05b5c, static_cast<U>(0x14 - level * 3), n);
        m->pos_y_30 = m->pos_y_30 - kHeightBias_00ce3dd8;  // FSUB double 150.0, FSTP float
        read_num(c, &m->pos_z_34, 0, 1, kPosHigh_00d02f64, pos_bits, n);
        const U angle_bits = static_cast<U>((7 - m->level_94) * 2);
        for (int i = 0; i < 3; ++i) {
            read_bool(c, &m->has_angle_a6[i]);
            if (m->has_angle_a6[i] != 0) read_num(c, &m->angle_20[i], 0, 1, kPi_00d7a264, angle_bits, n);
        }
    }

    if (m->compact_a0 == 0) {
        m->compact_76 = 0;
        for (int i = 0; i < 3; ++i) {
            std::uint8_t small = 0;
            read_bool(c, &small);
            if (small != 0)
                read_num(c, &m->vel_50[i], 0, 1, kVelSmallScale_00ceb4b0, 10, n);
            else
                read_num(c, &m->vel_50[i], 0, 1, kScale1024_00d05b54, 16, n);
        }
        for (int i = 0; i < 3; ++i) {
            std::uint8_t small = 0;
            read_bool(c, &small);
            if (small != 0)
                read_num(c, &m->rate_68[i], 0, 1, kRateSmall_00ce3800, 14, n);
            else
                read_num(c, &m->rate_68[i], 0, 1, kRateScale_00ce6630, 18, n);
        }
    } else if (m->skip_9d == 0) {
        const U pair_bits = static_cast<U>((8 - m->level_94) * 2);
        const U divisor_bits = static_cast<U>((9 - m->level_94) * 2);
        m->compact_76 = 1;
        std::uint8_t small = 0;
        read_bool(c, &small);  // 007C8DC3
        if (small != 0) {
            m->has_60_aa = 1;
            m->has_5c_a9 = 1;
            m->pair_60 = 0.0f;
            m->pair_5c = 0.0f;
        } else {
            read_bool(c, &m->has_5c_a9);
            if (m->has_5c_a9 != 0) read_num(c, &m->pair_5c, 0, 1, kPairHigh_00ce3d08, pair_bits, n);
            read_bool(c, &m->has_60_aa);
            if (m->has_60_aa != 0) read_num(c, &m->pair_60, 0, 1, kPairHigh_00ce3d08, pair_bits, n);
        }
        read_bool(c, &m->has_64_ab);
        if (m->has_64_ab != 0) {
            read_num(c, &m->divisor_64, 0, 0, kScale1024_00d05b54, divisor_bits, n);
            m->divisor_64 = m->divisor_64 - kDivisorBias_00cf1440;  // FSUB double 80.0, FSTP float
        }
        const U rate_bits = static_cast<U>(0x0F - m->level_94 * 2);
        for (int i = 0; i < 3; ++i) {
            read_bool(c, &m->has_rate_ac[i]);
            if (m->has_rate_ac[i] != 0) read_num(c, &m->rate_68[i], 0, 1, kRateHigh_00ce3918, rate_bits, n);
        }
    }

    read_bool(c, &m->flag_9f);  // 007C8F27
    if (m->skip_9d == 0) {
        read_bool(c, &m->has_44_a1);
        if (m->has_44_a1 != 0) read_num(c, &m->unit_44, 1, 0, kOne_00d7a24c, 7, n);
        read_bool(c, &m->has_38_a3);
        if (m->has_38_a3 != 0) read_num(c, &m->unit_38, 1, 1, kOne_00d7a24c, 8, n);
        read_bool(c, &m->has_40_a5);
        if (m->has_40_a5 != 0) read_num(c, &m->unit_40, 1, 1, kOne_00d7a24c, 8, n);
        read_bool(c, &m->has_3c_a4);
        if (m->has_3c_a4 != 0) read_num(c, &m->unit_3c, 1, 1, kOne_00d7a24c, 8, n);
        read_bool(c, &m->has_48_a2);
        if (m->has_48_a2 != 0) read_num(c, &m->unit_48, 1, 0, kOne_00d7a24c, 7, n);
        read_bool(c, &m->flag_4e);
        read_bool(c, &m->flag_75);
        read_bool(c, &m->flag_74);
    }

    bool rope_read = false;
    if (m->compact_a0 == 0) {
        read_native_u8_bits_00428c70(c, &m->state_8c, 3);  // 007C9040
        read_bool(c, &m->flag_9e);
        if (m->state_8c == 4) {
            const Raw tuning{static_cast<std::uint8_t*>(host.game_tuning_get_singleton_0042e740())};  // 007C9058
            read_num(c, &m->wire_rope_78, 0, 0, tuning.at<float>(0x51C), 10, n);
            rope_read = true;
        }
    } else {
        m->state_8c = 7;
        m->flag_9e = 0;
    }
    if (!rope_read) m->wire_rope_78 = 0.0f;  // 007C908B
    m->zero_7c = 0.0f;
    const volatile float* const g = host.global_vector_00f87574();  // MOVSS copies
    m->vec_80[0] = g[0];
    m->vec_80[1] = g[1];
    m->vec_80[2] = g[2];
}

namespace {
using cc13_seg47::ArchiveKey;
ArchiveKey name_key(const char* name) { return ArchiveKey{0, reinterpret_cast<std::uintptr_t>(name)}; }
ArchiveKey int_value(std::int32_t v) { return ArchiveKey{1, static_cast<std::uintptr_t>(static_cast<std::uint32_t>(v))}; }
// MOVSS [plane+off] -> MOVSS [pair+4]: bitwise.
ArchiveKey float_value(const Raw& plane, std::size_t off) { return ArchiveKey{2, plane.at<std::uint32_t>(off)}; }
// MOV CL,[plane+off] -> MOV byte [pair+4],CL: the upper three payload bytes
// are whatever the stack held; passed here as zero.
ArchiveKey bool_value(const Raw& plane, std::size_t off) { return ArchiveKey{3, plane.at<std::uint8_t>(off)}; }
}  // namespace

// 007C9860 BSP_Plane_SaveToArchive, __thiscall(ECX = plane; archive) RET 4;
// SEH frame 00C8C8CB whose only state (0, then -1) covers the PilotBot
// constructor. Every archive call is a vtable call on the archive (+04h enter,
// +08h leave, +0Ch write) with the key/value pairs built by value on the stack.
// Keys are the .rdata strings at the addresses noted. All reads are plain
// loads; no x87 arithmetic (the two float arguments are FLD/FSTP of -1.0f and
// FLDZ). Thread: whichever runs the save (main); precision is irrelevant here.
// The image constructs a fresh PilotBot at +DF4h without releasing the old
// one; this is kept.
void plane_save_to_archive_007c9860(PlaneSaveHost& host, Raw plane, Raw archive) {
    host.vehicle_unit_write_state_0095bb10(plane, archive);  // 007C987F
    host.archive_enter_04(archive, name_key("_plane"));      // 00D05DF8
    host.archive_write_0c(archive, name_key("originalEquipmentIndex"), int_value(plane.at<std::int32_t>(0xC54)));
    host.archive_write_0c(archive, name_key("forcedDrop"), int_value(plane.at<std::int32_t>(0xC58)));
    host.archive_enter_04(archive, name_key("_bulletBase"));  // 00CF936C
    host.bullet_base_write_state_006e6e80(plane.sub(0x72C), archive);
    host.archive_leave_08(archive);
    if (plane.ptr(0x9D4) != nullptr) {
        host.archive_write_0c(archive, name_key("squadron"),  // 00CF886C
                              ArchiveKey{4, reinterpret_cast<std::uintptr_t>(plane.ptr(0x9D4))});
        host.archive_write_0c(archive, name_key("formationIndex"), int_value(plane.at<std::int32_t>(0x9D0)));
        host.archive_write_0c(archive, name_key("squadronIndex"), int_value(plane.at<std::int32_t>(0x9D8)));
    }
    host.archive_enter_04(archive, name_key("control"));  // 00D05DAC
    host.archive_write_0c(archive, name_key("pwrInput"), float_value(plane, 0x9F0));
    host.archive_write_0c(archive, name_key("yawInput"), float_value(plane, 0x9E4));
    host.archive_write_0c(archive, name_key("rollInput"), float_value(plane, 0x9EC));
    host.archive_write_0c(archive, name_key("pitchInput"), float_value(plane, 0x9E8));
    host.archive_write_0c(archive, name_key("airBrakeInput"), float_value(plane, 0x9F4));
    host.archive_write_0c(archive, name_key("gunFire"), bool_value(plane, 0x9FA));
    host.archive_write_0c(archive, name_key("turboInput"), bool_value(plane, 0x9F9));
    host.archive_leave_08(archive);
    host.archive_enter_04(archive, name_key("dynamics"));  // 00D05D4C
    host.flight_controller_save_007d9b70(plane.sub(0xAB0), archive);
    host.archive_leave_08(archive);
    host.archive_enter_04(archive, name_key("ctrlParams"));  // 00D05D40
    host.archive_write_0c(archive, name_key("rollF"), float_value(plane, 0xBB8));
    host.archive_write_0c(archive, name_key("yawF"), float_value(plane, 0xBB0));
    host.archive_write_0c(archive, name_key("airBrakeF"), float_value(plane, 0xBC0));
    host.archive_write_0c(archive, name_key("pwrF"), float_value(plane, 0xBBC));
    host.archive_write_0c(archive, name_key("pitchF"), float_value(plane, 0xBB4));
    host.archive_leave_08(archive);
    host.archive_enter_04(archive, name_key("controlFactors"));  // 00D05D04
    host.archive_write_0c(archive, name_key("rollFactor"), float_value(plane, 0xBCC));
    host.archive_write_0c(archive, name_key("pitchFactor"), float_value(plane, 0xBD0));
    host.archive_write_0c(archive, name_key("pwrFactor"), float_value(plane, 0xBD4));
    host.archive_write_0c(archive, name_key("engFactor0"), float_value(plane, 0xBD8));
    host.archive_write_0c(archive, name_key("engFactor1"), float_value(plane, 0xBDC));
    host.archive_write_0c(archive, name_key("engFactor2"), float_value(plane, 0xBE0));
    host.archive_write_0c(archive, name_key("engFactor3"), float_value(plane, 0xBE4));
    host.archive_leave_08(archive);
    host.archive_enter_04(archive, name_key("controlShake"));  // 00D05CA0
    host.archive_write_0c(archive, name_key("rollShake"), float_value(plane, 0xBE8));
    host.archive_write_0c(archive, name_key("pitchShake"), float_value(plane, 0xBEC));
    host.archive_write_0c(archive, name_key("pwrShake"), float_value(plane, 0xBF0));
    host.archive_leave_08(archive);
    host.plane_landing_site_probe_007c5ac0(plane, -1.0f);  // 007C9FC2, FLD 00D7A260
    host.archive_write_0c(archive, name_key("bombing"), bool_value(plane, 0xC25));  // 00D05C74
    host.archive_write_0c(archive, name_key("bombDelay"), float_value(plane, 0xC28));
    host.archive_write_0c(archive, name_key("playerControllable"), bool_value(plane, 0xC0C));
    host.archive_write_0c(archive, name_key("numBombsTriggered"), int_value(plane.at<std::int32_t>(0xC20)));
    host.archive_write_0c(archive, name_key("lastGunState"), bool_value(plane, 0xC35));
    host.archive_write_0c(archive, name_key("shootingListIn"), bool_value(plane, 0xC40));  // 00D05C20
    host.archive_enter_04(archive, name_key("failures"));                                // 00D05C14
    for (std::int32_t i = 0; i < 4; ++i)
        host.archive_write_0c(archive, ArchiveKey{1, static_cast<std::uintptr_t>(i)}, bool_value(plane, 0xC36 + i));
    host.archive_leave_08(archive);
    host.archive_write_0c(archive, name_key("someFailure"), bool_value(plane, 0xC3A));  // 00D05C08
    host.archive_write_0c(archive, name_key("planestate"), int_value(plane.at<std::int32_t>(0x900)));
    host.archive_write_0c(archive, name_key("planeflown"), float_value(plane, 0x908));
    host.archive_write_0c(archive, name_key("planelanded"), bool_value(plane, 0x904));
    host.archive_write_0c(archive, name_key("planewaterfill"), float_value(plane, 0x90C));  // 00D05BD4

    void* const memory = host.operator_new_00bf681b(0xBC);  // 007CA294
    void* const bot = memory != nullptr ? host.pilot_bot_construct_0099a880(memory) : nullptr;
    plane.ptr(0xDF4) = bot;
    // No null test before the vtable load: a failed allocation faults here.
    host.pilot_bot_vcall_attach_04(raw(bot), plane.sub(0x310));  // 007CA2D3
    cc13_seg47b::plane_update_pose_derived_state_007c18b0(host, plane, 0.0f);  // 007CA2DD, FLDZ

    host.archive_write_0c(archive, name_key("gearsState"), int_value(plane.at<std::int32_t>(0xC08)));  // 00D05BC8
    {
        ArchiveSectionRef ref;
        host.archive_writer_ref_open_00bd5010(archive, &ref, name_key("partAnims"));  // 00D05BBC
        host.actuator_block_save_007deb70(raw(plane.ptr(0xDEC)), ref);              // 007CA335
    }
    host.archive_write_0c(archive, name_key("aiBayDoorState"), int_value(plane.at<std::int32_t>(0xDF8)));
    host.archive_write_0c(archive, name_key("bayDoorTrgState"), bool_value(plane, 0xDF0));  // 00D05B9C
    {
        ArchiveSectionRef ref;
        host.archive_writer_ref_open_00bd5010(archive, &ref, name_key("rotorManager"));  // 00D05B8C
        host.plane_state_block_save_007e0d20(plane.sub(0xC98), ref);                  // 007CA3C4
    }
    host.archive_leave_08(archive);  // closes "_plane"
}

}  // namespace bsp::cc13_big47b
