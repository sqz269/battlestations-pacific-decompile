#include "bsp/cc13_camera_controllers.hpp"
#include "bsp/cc13_x87_model.hpp"

namespace bsp::cc13cam {
void modifier_write_noop_0078e730(void*) noexcept {}
void modifier_read_noop_0078e740(void*) noexcept {}
int modifier_type_zero_0078e760() noexcept { return 0; }
}

#include <cctype>
#include <cmath>
#include <cstring>

#include "bsp/camera_affine.hpp"
#include "bsp/camera_position_modes.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/pc24_camera_cc13.hpp"
#include "bsp/ship_ai_bearing_rating.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/world_ocean.hpp"

// Evidence for every routine is in docs/CC13_CAMERA_CONTROLLERS.md. Names are
// hypotheses. x87 sequences are written as double expressions with a float
// cast at each native FSTP; the comments name the store.

namespace bsp::cc13cam {
namespace {

constexpr double kDegToRad = 0.01745329238474369;   // 00D046D8 (double)
constexpr float kGoAroundRadiusStart = 100.0f;      // 00CE3D08
constexpr float kOne = 1.0f;                        // 00D7A24C
constexpr float kHalf = 0.5f;                       // 00CE3800
constexpr float kEntityRefMin = 10000.0f;           // 00CE3D64
constexpr double kEntityRefBias = 9999.900390625;   // 00D04658 (double)
constexpr double kLinkedForward = 200.0;            // 00CE4D70 (double)

const ModifierLuaValue& nil_value() noexcept {
    static const ModifierLuaValue nil{};
    return nil;
}

// The curve shape both parsers use: a number is one key (0, v); anything else
// non-nil is read as a table (0, t[1]), (t[2], t[3]), ... until t[2k] is nil.
// `scale` applies to values only, after the float32 spill of GetNumber.
void parse_curve(ModifierCurve& curve, const ModifierLuaValue& v, bool degrees) {
    auto value = [&](const ModifierLuaValue& x) {
        const float n = x.number_f32();
        return degrees ? static_cast<float>(static_cast<double>(n) * kDegToRad) : n;
    };
    if (v.is_nil()) return;
    if (v.kind == ModifierLuaValue::Kind::Number) {
        curve.emplace_back(0.0f, value(v));
        return;
    }
    curve.emplace_back(0.0f, value(v.at(1)));
    for (int i = 2; !v.at(i).is_nil(); i += 2) {
        curve.emplace_back(v.at(i).number_f32(), value(v.at(i + 1)));
    }
}

// The linear-only interpolation 00791C90 uses for zoom and patterntime:
// (1-f)*prev + next*f, f unclamped, stored once (007923A7 / 007924E7).
float linear_curve(const ModifierCurve& curve, float t) noexcept {
    const CurveSample s = sample_modifier_curve(curve, t, kOne);
    const double f = s.fraction;
    float value;
    if constexpr (kPc24CameraCc13Bound) {
        // 00792386..0079239F: FLD1 FSUBRP (1 - f), FMUL prev, FMULP f * next, FADDP.
        value = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(cc13x87::x87_sub24(1.0, f), s.prev_value),
            cc13x87::x87_mul24(f, s.next_value)));
    } else {
        value = static_cast<float>((1.0 - f) * s.prev_value + static_cast<double>(s.next_value) * f);
    }
    return value;
}

// Gamecamera curves resolve each key through a callback (00792500/00792610)
// before blending; the scan itself is the same as sample_modifier_curve.
template <class Resolve>
CurveSample sample_resolved(const ModifierCurve& curve, float t, Resolve resolve) {
    CurveSample s{};
    float prev_time = 0.0f;
    std::size_t i = 0;
    for (; i < curve.size(); ++i) {
        if (t < curve[i].first) break;
        prev_time = curve[i].first;
        s.prev_value = resolve(i);
    }
    if (i < curve.size()) {
        s.fraction = static_cast<float>((static_cast<double>(t) - prev_time) /
                                        (static_cast<double>(curve[i].first) - prev_time));
        s.next_value = resolve(i);
    }
    return s;
}

std::uint32_t bits_of(float f) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &f, sizeof u);
    return u;
}

float float_of(std::uint32_t u) noexcept {
    float f;
    std::memcpy(&f, &u, sizeof f);
    return f;
}

constexpr float kTimeScale = 256.0f;      // 00CF8B3C
constexpr float kRadiusScale = 10000.0f;  // 00CE3D64
constexpr float kAngleScale = 100.0f;     // 00CE3D08

void write_weight(NativeBitCursor* c, float v, const NativeBitNumericContext& ctx) {
    write_native_numeric_float_004295c0(c, bits_of(v), 0, 0, bits_of(1.0f), 8, ctx);
}

void read_weight(NativeBitCursor* c, float& v, const NativeBitNumericContext& ctx) {
    std::uint32_t u = 0;
    read_native_numeric_float_004293f0(c, &u, 0, 0, bits_of(1.0f), 8, ctx);
    v = float_of(u);
}

void write_curve(NativeBitCursor* c, const ModifierCurve& curve, float scale,
                 const NativeBitNumericContext& ctx) {
    write_native_unsigned_dword_bits_00429070(c, static_cast<std::uint32_t>(curve.size()), 8);
    for (const auto& [time, value] : curve) {
        write_native_numeric_float_004295c0(c, bits_of(time), 0, 1, bits_of(kTimeScale), 0x12, ctx);
        write_native_numeric_float_004295c0(c, bits_of(value), 0, 1, bits_of(scale), 0x12, ctx);
    }
}

void read_curve(NativeBitCursor* c, ModifierCurve& curve, float scale,
                const NativeBitNumericContext& ctx) {
    std::uint32_t count = 0;
    read_native_u32_bits_00428d10(c, &count, 8);
    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t t = 0, v = 0;
        read_native_numeric_float_004293f0(c, &t, 0, 1, bits_of(kTimeScale), 0x12, ctx);
        read_native_numeric_float_004293f0(c, &v, 0, 1, bits_of(scale), 0x12, ctx);
        curve.emplace_back(float_of(t), float_of(v));
    }
}

}  // namespace

const ModifierLuaValue& ModifierLuaValue::field(const char* name) const noexcept {
    if (kind != Kind::Table) return nil_value();
    for (const auto& [key, value] : fields) {
        if (key == name) return value;
    }
    return nil_value();
}

const ModifierLuaValue& ModifierLuaValue::at(int lua_index) const noexcept {
    if (kind != Kind::Table || lua_index < 1 || static_cast<std::size_t>(lua_index) > array.size()) {
        return nil_value();
    }
    return array[static_cast<std::size_t>(lua_index) - 1];
}

float ModifierLuaValue::number_f32() const noexcept {
    return kind == Kind::Number ? static_cast<float>(number) : 0.0f;
}

// 0078FCF0 and its inlined copies (00791DCC..00791E35 and the four others).
float modifier_ease(float f) noexcept {
    if (0.0f > f) return 0.0f;
    if (f > kOne) return kOne;
    if (kHalf > f) {
        const float sq = f * f;             // FSTP after FMUL ST0
        return sq + sq;
    }
    const float r = kOne - f;               // FSTP
    const float sq = r * r;                 // FSTP
    // pc24-lint: allow sq + sq doubles a float exactly, so 1 - (sq + sq) has one rounding (00791E31..00791E35)
    return static_cast<float>(1.0 - (static_cast<double>(sq) + sq));
}

// 00791CE2..00791DC6: the scan. prev starts at (0, start); a key with t < time
// ends it; when nothing ends it the fraction and next value stay 0.
CurveSample sample_modifier_curve(const ModifierCurve& curve, float t, float start_value) noexcept {
    CurveSample s{start_value, 0.0f, 0.0f};
    float prev_time = 0.0f;
    std::size_t i = 0;
    for (; i < curve.size(); ++i) {
        if (t < curve[i].first) break;      // FCOMIP / JC
        prev_time = curve[i].first;
        s.prev_value = curve[i].second;
    }
    if (i < curve.size()) {
        // FLD t, FLD prev, FSUBP, FSUBR key, FDIVP, FSTP (00791DA3).
        if constexpr (kPc24CameraCc13Bound) {
            s.fraction = static_cast<float>(cc13x87::x87_div24(
                cc13x87::x87_sub24(t, prev_time), cc13x87::x87_sub24(curve[i].first, prev_time)));
        } else {
            s.fraction = static_cast<float>((static_cast<double>(t) - prev_time) /
                                            (static_cast<double>(curve[i].first) - prev_time));
        }
        s.next_value = curve[i].second;
    }
    return s;
}

// 00791E39..00791E90: w = s*(1-L) + f*L (FSTP), value = w*next + prev*(1-w).
float blend_modifier_curve(const CurveSample& s, float linear) noexcept {
    const double L = linear;
    const double ease = modifier_ease(s.fraction);
    float value;
    if constexpr (kPc24CameraCc13Bound) {
        // 00791E4C..00791E66: FSUBRP 1 - L, ease * (1 - L), L * f, FADDP, FSTP w.
        const float w = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(ease, cc13x87::x87_sub24(1.0, L)), cc13x87::x87_mul24(L, s.fraction)));
        // 00791E6A..00791E90: FSUBP 1 - w, (1 - w) * prev, w * next, FADDP, FSTP.
        value = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(cc13x87::x87_sub24(1.0, w), s.prev_value),
            cc13x87::x87_mul24(w, s.next_value)));
    } else {
        const float w = static_cast<float>(ease * (1.0 - L) + L * s.fraction);
        value = static_cast<float>(static_cast<double>(w) * s.next_value +
                                   static_cast<double>(s.prev_value) * (1.0 - w));
    }
    return value;
}

// ---------------------------------------------------------------------------
// type 1, input orbit

void orbit_input_construct_0078e790(OrbitInputModifier& m) noexcept {
    m = OrbitInputModifier{};
}

namespace {
// One axis of 007A0340: rate * dt * (1 - (1 - |a|)^2), signed by a. A NaN axis
// takes the negative arm (COMISS / JC). Each arm is one x87 chain.
double axis_delta(float a, float rate, float dt, bool& negative) noexcept {
    negative = !(a >= 0.0f);
    if (!negative) {
        const double r = 1.0 - a;
        return (1.0 - r * r) * dt * rate;
    }
    const double s = static_cast<double>(a) + 1.0;
    return (1.0 - s * s) * dt * rate;
}
constexpr float kPitchMin = 0.01f;                     // 00D7A238
constexpr double kPitchMaxD = 3.1315927505493164;      // 00D04D20 (double)
constexpr float kPitchMax = 3.1315927505493164f;       // 00D04D1C
}  // namespace

// 007A0340. dt = time - last when last >= 0, else 0. Yaw and pitch move with
// their axes; distance moves against its axis (a positive axis shortens it).
// Pitch is clamped to [0.01, 3.13159]. With decks, the two button pairs step
// the deck index through -1..count-1 (wrapping both ways); a deck sets the
// owner to transform 3 (keepall) at the deck's position and up. Without one
// the owner is transform 1 (keepy) at the origin plus the spherical offset.
void orbit_input_step_007a0340(OrbitInputModifier& m, float time, const ModifierInput& in,
                               OrbitInputHost& host) {
    float dt = 0.0f;
    if (m.last_time_20 >= 0.0f) dt = static_cast<float>(static_cast<double>(time) - m.last_time_20);
    if (m.owner_4 == nullptr) return;
    bool neg = false;
    double d = axis_delta(in.yaw_axis_8, m.yaw_rate_c, dt, neg);
    m.yaw_8 = static_cast<float>(neg ? m.yaw_8 - d : m.yaw_8 + d);
    d = axis_delta(in.pitch_axis_c, m.pitch_rate_14, dt, neg);
    m.pitch_10 = static_cast<float>(neg ? m.pitch_10 - d : m.pitch_10 + d);
    if (kPitchMin > m.pitch_10) m.pitch_10 = kPitchMin;
    else if (static_cast<double>(m.pitch_10) > kPitchMaxD) m.pitch_10 = kPitchMax;
    d = axis_delta(in.distance_axis_18, m.distance_rate_1c, dt, neg);
    m.distance_18 = static_cast<float>(neg ? m.distance_18 + d : m.distance_18 - d);

    if (m.deck_count_28 > 0) {
        if (in.next_b_5 && in.next_a_4) ++m.deck_index_24;
        if (in.prev_b_7 && in.prev_a_6) --m.deck_index_24;
        if (m.deck_index_24 < -1) m.deck_index_24 = m.deck_count_28 - 1;
        else if (m.deck_index_24 >= m.deck_count_28) m.deck_index_24 = -1;
    }
    ModifierKeyframe& k = *m.owner_4;
    if (m.deck_index_24 != -1) {
        k.transform_2c = 3;
        host.deck_row(k.parent_1c, m.deck_index_24, k.local_34, k.up_58);
        m.last_time_20 = time;
        return;
    }
    k.local_34 = {0.0f, 0.0f, 0.0f};
    k.transform_2c = 1;
    m.yaw_8 = ship_ai_firepower_wrap_angle_00605070(m.yaw_8);
    m.pitch_10 = ship_ai_firepower_wrap_angle_00605070(m.pitch_10);
    k.work_40 = k.local_34;
    const float sp = static_cast<float>(std::sin(static_cast<double>(m.pitch_10)));
    const float sy = static_cast<float>(std::sin(static_cast<double>(m.yaw_8)));
    const float x = static_cast<float>(static_cast<double>(sy) * sp * m.distance_18);
    const float cy = static_cast<float>(std::cos(static_cast<double>(m.yaw_8)));
    const float z = static_cast<float>(static_cast<double>(cy) * sp * m.distance_18);
    const float cp = static_cast<float>(std::cos(static_cast<double>(m.pitch_10)));
    const float y = static_cast<float>(static_cast<double>(cp) * m.distance_18);
    k.work_40[0] = static_cast<float>(static_cast<double>(k.work_40[0]) + x);
    k.work_40[1] = static_cast<float>(static_cast<double>(k.work_40[1]) + y);
    k.work_40[2] = static_cast<float>(static_cast<double>(z) + k.work_40[2]);
    m.last_time_20 = time;
}

// 00791640. |d| is d when d > 0 (COMISS 0.0), else -0.0 - d (00D7A208).
void orbit_input_track_tick_00791640(OrbitInputModifier& m) noexcept {
    const float a = m.distance_18 > 0.0f ? m.distance_18 : -0.0f - m.distance_18;
    const float r = static_cast<float>(static_cast<double>(a) * 1.399999976158142);   // 00D045F0, FSTP
    if (5.0f > r) m.distance_rate_1c = 5.0f;                                           // 00CE3850
    else m.distance_rate_1c = r > 300.0f ? 300.0f : r;                                 // 00CE3AE8
}

// 007916C0. d = pose row 3 - owner +34h (each FSTP); |d| from a float sum of
// squares, 0 unless the sum exceeds 1e-10 (double 00CE3820). Below 1e-5
// (00CE3C70) yaw and pitch are 0; otherwise pitch = acos(d.y/|d|) (0 above 1,
// pi (00D7A264) below -1) and yaw = atan2(d.x/|d|, d.z/|d|). The deck count
// is the parent's camera array size, 0 without a parent.
void orbit_input_begin_007916c0(OrbitInputModifier& m, std::size_t camera, OrbitInputHost& host) {
    ModifierKeyframe& k = *m.owner_4;
    const std::array<float, 16> w = host.camera_pose_in_parent(camera, k.parent_1c);
    const float x = static_cast<float>(static_cast<double>(w[12]) - k.local_34[0]);
    const float y = static_cast<float>(static_cast<double>(w[13]) - k.local_34[1]);
    const float z = static_cast<float>(static_cast<double>(w[14]) - k.local_34[2]);
    const float sum = static_cast<float>((static_cast<double>(x) * x + static_cast<double>(y) * y) +
                                         static_cast<double>(z) * z);
    float len = 0.0f;
    if (static_cast<double>(sum) > 1e-10) len = static_cast<float>(std::sqrt(static_cast<double>(sum)));
    m.distance_18 = len;
    if (1e-05 > static_cast<double>(len)) {
        m.pitch_10 = 0.0f;
        m.yaw_8 = 0.0f;
    } else {
        const float xn = static_cast<float>(static_cast<double>(x) / len);
        const float yn = static_cast<float>(static_cast<double>(y) / len);
        const float zn = static_cast<float>(static_cast<double>(z) / len);
        if (yn > 1.0f) m.pitch_10 = 0.0f;
        else if (-1.0f > yn) m.pitch_10 = 3.1415927410125732f;
        else m.pitch_10 = static_cast<float>(std::acos(static_cast<double>(yn)));
        m.yaw_8 = static_cast<float>(std::atan2(static_cast<double>(xn), static_cast<double>(zn)));
    }
    m.deck_count_28 = k.parent_1c != 0 ? host.deck_count(k.parent_1c) : 0;
}

// ---------------------------------------------------------------------------
// goaround

// 007A0650: all curves empty, the three linear weights 0.6 (00CE3D30).
void goaround_construct_007a0650(GoAroundModifier& m) noexcept {
    m = GoAroundModifier{};
}

// 007A3100. Keys in order: radius (+14h), theta (+24h, degrees), rho (+34h,
// degrees), zoom (+44h), patterntime (+54h), then radiuslinearblend (+8h),
// thetalinearblend (+Ch), rholinearblend (+10h) when they are numbers.
void goaround_parse_007a3100(GoAroundModifier& m, const ModifierLuaValue& table) {
    parse_curve(m.radius_14, table.field("radius"), false);
    parse_curve(m.theta_24, table.field("theta"), true);
    parse_curve(m.rho_34, table.field("rho"), true);
    parse_curve(m.zoom_44, table.field("zoom"), false);
    parse_curve(m.patterntime_54, table.field("patterntime"), false);
    const ModifierLuaValue& rl = table.field("radiuslinearblend");
    if (rl.kind == ModifierLuaValue::Kind::Number) m.radius_linear_8 = rl.number_f32();
    const ModifierLuaValue& tl = table.field("thetalinearblend");
    if (tl.kind == ModifierLuaValue::Kind::Number) m.theta_linear_c = tl.number_f32();
    const ModifierLuaValue& pl = table.field("rholinearblend");
    if (pl.kind == ModifierLuaValue::Kind::Number) m.rho_linear_10 = pl.number_f32();
}

// 00791C90. t = max(elapsed + offset, 0). Radius starts at 100, theta and rho
// at 0. The owner's working position is its authored position plus the
// spherical offset (rho the heading about Y, theta the elevation); zoom and
// patterntime are written only when their curve has keys.
void goaround_evaluate_00791c90(GoAroundModifier& m, float elapsed, float offset) noexcept {
    float t = static_cast<float>(static_cast<double>(elapsed) + offset);   // 00791CA2
    if (0.0f > t) t = 0.0f;
    const float radius = blend_modifier_curve(
        sample_modifier_curve(m.radius_14, t, kGoAroundRadiusStart), m.radius_linear_8);
    const float theta = blend_modifier_curve(sample_modifier_curve(m.theta_24, t, 0.0f), m.theta_linear_c);
    const float rho = blend_modifier_curve(sample_modifier_curve(m.rho_34, t, 0.0f), m.rho_linear_10);
    ModifierKeyframe& k = *m.owner_4;
    // 007921E6..00792262; FSIN/FCOS results are spilled to float.
    if constexpr (kPc24CameraCc13Bound) {
        const float sin_rho = static_cast<float>(std::sin(static_cast<double>(rho)));
        const float cos_theta = static_cast<float>(std::cos(static_cast<double>(theta)));
        // 00792202..00792216: FLD sin_rho, FCHS, FMUL radius, FMUL cos_theta, FADD local.
        k.work_40[0] = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(cc13x87::x87_mul24(-static_cast<double>(sin_rho), radius), cos_theta),
            k.local_34[0]));
        const float cos_rho = static_cast<float>(std::cos(static_cast<double>(rho)));
        // 00792235..00792247.
        k.work_40[2] = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(cc13x87::x87_mul24(cos_rho, radius), cos_theta), k.local_34[2]));
        const float sin_theta = static_cast<float>(std::sin(static_cast<double>(theta)));
        // 00792254..00792262.
        k.work_40[1] = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_mul24(sin_theta, radius), k.local_34[1]));
    } else {
        const float sin_rho = static_cast<float>(std::sin(static_cast<double>(rho)));
        const float cos_theta = static_cast<float>(std::cos(static_cast<double>(theta)));
        k.work_40[0] = static_cast<float>(-static_cast<double>(sin_rho) * radius * cos_theta + k.local_34[0]);
        const float cos_rho = static_cast<float>(std::cos(static_cast<double>(rho)));
        k.work_40[2] = static_cast<float>(static_cast<double>(cos_rho) * radius * cos_theta + k.local_34[2]);
        const float sin_theta = static_cast<float>(std::sin(static_cast<double>(theta)));
        k.work_40[1] = static_cast<float>(static_cast<double>(sin_theta) * radius + k.local_34[1]);
    }
    if (!m.zoom_44.empty()) k.zoom_c8 = linear_curve(m.zoom_44, t);
    if (!m.patterntime_54.empty()) k.smooth_d0 = linear_curve(m.patterntime_54, t);
}

// 007918B0 / 007A2690.
void goaround_write_bits_007918b0(const GoAroundModifier& m, NativeBitCursor* c,
                                  const NativeBitNumericContext& ctx) {
    write_weight(c, m.radius_linear_8, ctx);
    write_weight(c, m.rho_linear_10, ctx);
    write_weight(c, m.theta_linear_c, ctx);
    write_curve(c, m.radius_14, kRadiusScale, ctx);
    write_curve(c, m.theta_24, kAngleScale, ctx);
    write_curve(c, m.rho_34, kAngleScale, ctx);
    write_curve(c, m.zoom_44, kAngleScale, ctx);
    write_curve(c, m.patterntime_54, kAngleScale, ctx);
}

void goaround_read_bits_007a2690(GoAroundModifier& m, NativeBitCursor* c,
                                 const NativeBitNumericContext& ctx) {
    read_weight(c, m.radius_linear_8, ctx);
    read_weight(c, m.rho_linear_10, ctx);
    read_weight(c, m.theta_linear_c, ctx);
    read_curve(c, m.radius_14, kRadiusScale, ctx);
    read_curve(c, m.theta_24, kAngleScale, ctx);
    read_curve(c, m.rho_34, kAngleScale, ctx);
    read_curve(c, m.zoom_44, kAngleScale, ctx);
    read_curve(c, m.patterntime_54, kAngleScale, ctx);
}

// ---------------------------------------------------------------------------
// gamecamera / fpscamera

// 00792710 / 007A2A80.
void gamecamera_write_bits_00792710(const GameCameraModifier& m, NativeBitCursor* c,
                                    const NativeBitNumericContext& ctx) {
    write_weight(c, m.rho_linear_14, ctx);
    write_weight(c, m.theta_linear_10, ctx);
    write_curve(c, m.theta_18, kAngleScale, ctx);
    write_curve(c, m.rho_28, kAngleScale, ctx);
}

void gamecamera_read_bits_007a2a80(GameCameraModifier& m, NativeBitCursor* c,
                                   const NativeBitNumericContext& ctx) {
    read_weight(c, m.rho_linear_14, ctx);
    read_weight(c, m.theta_linear_10, ctx);
    read_curve(c, m.theta_18, kAngleScale, ctx);
    read_curve(c, m.rho_28, kAngleScale, ctx);
}

// 007A06D0(fps): empty curves, start time -1.0, both linear weights 0.8.
void gamecamera_construct_007a06d0(GameCameraModifier& m, bool fps) noexcept {
    m = GameCameraModifier{};
    m.fps_78 = fps;
}

// 007A3B20. thetalinearblend (+10h) and rholinearblend (+14h) when numbers;
// theta (else Vlook) into +18h and rho (else Hlook) into +28h; targetentity.Ptr
// into the +38h observer. A table value {Ptr = entity} becomes the marker
// count + 10000 and the entity is appended to +58h/+68h. A table value whose
// Ptr is not userdata pushes whatever the stack slot last held (007A407F reads
// [ESP+18h] unwritten on that path); this keeps the previous value pushed,
// or 0 for the first key.
void gamecamera_parse_007a3b20(GameCameraModifier& m, const ModifierLuaValue& table) {
    const ModifierLuaValue& tl = table.field("thetalinearblend");
    if (tl.kind == ModifierLuaValue::Kind::Number) m.theta_linear_10 = tl.number_f32();
    const ModifierLuaValue& rl = table.field("rholinearblend");
    if (rl.kind == ModifierLuaValue::Kind::Number) m.rho_linear_14 = rl.number_f32();

    auto parse_angles = [](ModifierCurve& curve, int& count, std::vector<std::size_t>& entities,
                           const ModifierLuaValue& v) {
        if (v.is_nil()) return;
        if (v.kind == ModifierLuaValue::Kind::Number) {
            curve.emplace_back(0.0f, static_cast<float>(static_cast<double>(v.number_f32()) * kDegToRad));
            return;
        }
        float carried = 0.0f;
        for (int i = 0; i == 0 || !v.at(i).is_nil(); i += 2) {
            const float time = i == 0 ? 0.0f : v.at(i).number_f32();
            const ModifierLuaValue& x = v.at(i + 1);
            if (x.kind == ModifierLuaValue::Kind::Table) {
                const ModifierLuaValue& ptr = x.field("Ptr");
                if (ptr.kind == ModifierLuaValue::Kind::Userdata) {
                    // FILD count, FADD 10000.0, FSTP (007A3FF9..007A4006).
                    carried = static_cast<float>(static_cast<double>(count) + kEntityRefBase);
                    entities.push_back(ptr.userdata);
                    ++count;
                }
            } else {
                carried = static_cast<float>(static_cast<double>(x.number_f32()) * kDegToRad);
            }
            curve.emplace_back(time, carried);
        }
    };
    const ModifierLuaValue* theta = &table.field("theta");
    if (theta->is_nil()) theta = &table.field("Vlook");
    parse_angles(m.theta_18, m.theta_entity_count_50, m.theta_entities_58, *theta);
    const ModifierLuaValue* rho = &table.field("rho");
    if (rho->is_nil()) rho = &table.field("Hlook");
    parse_angles(m.rho_28, m.rho_entity_count_54, m.rho_entities_68, *rho);

    const ModifierLuaValue& target = table.field("targetentity");
    if (target.kind == ModifierLuaValue::Kind::Table) {
        const ModifierLuaValue& ptr = target.field("Ptr");
        if (ptr.kind == ModifierLuaValue::Kind::Userdata) m.target_4c = ptr.userdata;
    }
}

namespace {
// The shared head of 00792500/00792610: the stored value, or the entity it
// marks. Returns 0 when the marker's index or entity is missing.
template <class Angle>
float resolve_key(const ModifierCurve& curve, const std::vector<std::size_t>& entities,
                  std::size_t index, Angle angle) {
    const float value = curve.at(index).second;
    if (value < kEntityRefMin) return value;                  // COMISS / JC
    // FSUB 9999.900390625, _ftol (00BF7420): truncation.
    const auto slot = static_cast<std::uint32_t>(
        static_cast<std::int64_t>(static_cast<double>(value) - kEntityRefBias));
    if (slot >= entities.size() || entities[slot] == 0) return 0.0f;
    return angle(entities[slot]);
}
}  // namespace

// 00792500: theta toward an entity is -atan2(y, sqrt(x*x + z*z)) in the
// parent's frame, the sum and the root each spilled to float.
float gamecamera_theta_key_00792500(const GameCameraModifier& m, std::size_t index,
                                    GameCameraModifierHost& host) {
    return resolve_key(m.theta_18, m.theta_entities_58, index, [&](std::size_t e) {
        const auto p = host.point_in_parent_frame(m.owner_4->parent_1c, e);
        const float sum = static_cast<float>(static_cast<double>(p[0]) * p[0] + static_cast<double>(p[2]) * p[2]);
        const float h = static_cast<float>(std::sqrt(static_cast<double>(sum)));
        const float a = static_cast<float>(std::atan2(static_cast<double>(p[1]), static_cast<double>(h)));
        return -a;
    });
}

// 00792610: rho toward an entity is atan2(x, z) in the parent's frame.
float gamecamera_rho_key_00792610(const GameCameraModifier& m, std::size_t index,
                                  GameCameraModifierHost& host) {
    return resolve_key(m.rho_28, m.rho_entities_68, index, [&](std::size_t e) {
        const auto p = host.point_in_parent_frame(m.owner_4->parent_1c, e);
        return static_cast<float>(std::atan2(static_cast<double>(p[0]), static_cast<double>(p[2])));
    });
}

// 00795EE0. +Ch holds the previous t, so dt = t - previous; nothing runs while
// dt <= 0 and the previous t is then kept. With a camera the two angle curves
// are blended (start value 0), routed to the camera by the parent's kind, the
// camera is updated by dt and its matrix becomes the owner's pose; a linked
// look-at key is put 200 units along the matrix's third row.
void gamecamera_evaluate_00795ee0(GameCameraModifier& m, float elapsed, float offset,
                                  GameCameraModifierHost& host) {
    const float t = static_cast<float>(static_cast<double>(elapsed) + offset);     // 00795EF4
    ModifierKeyframe& k = *m.owner_4;
    k.field_cc = 0.0f;                                                            // 00795EFF
    const float dt = static_cast<float>(static_cast<double>(t) - m.last_time_c);  // 00795F07
    if (!(dt > 0.0f)) return;                                                     // JBE 007965C2
    if (m.camera_8 != 0) {
        float theta = blend_modifier_curve(
            sample_resolved(m.theta_18, t, [&](std::size_t i) { return gamecamera_theta_key_00792500(m, i, host); }),
            m.theta_linear_10);
        float rho = blend_modifier_curve(
            sample_resolved(m.rho_28, t, [&](std::size_t i) { return gamecamera_rho_key_00792610(m, i, host); }),
            m.rho_linear_14);
        const std::size_t parent = k.parent_1c;
        if (!m.rho_28.empty() || !m.theta_18.empty() || m.target_4c != 0) {
            if (host.is_kind(parent, kKind0F) || host.is_kind(parent, kKind18)) {
                if (m.fps_78) {
                    if (m.target_4c != 0) {                                       // 00796267
                        const auto p = host.point_in_parent_cached_frame(parent, m.target_4c);
                        rho = static_cast<float>(std::atan2(static_cast<double>(p[0]), static_cast<double>(p[2])));
                        const float sum = static_cast<float>(static_cast<double>(p[0]) * p[0] +
                                                             static_cast<double>(p[2]) * p[2]);
                        const float h = static_cast<float>(std::sqrt(static_cast<double>(sum)));
                        theta = -static_cast<float>(std::atan2(static_cast<double>(p[1]), static_cast<double>(h)));
                    }
                    host.camera_set_angles_0042aa00(m.camera_8, rho, theta, true);
                } else {
                    host.camera_store_orbit_angles(m.camera_8, theta, rho);      // 0079634C
                }
            } else if (host.is_kind(parent, kKind06) && host.camera_3e8_is_zero(m.camera_8)) {
                host.camera_store_free_angles(m.camera_8, rho, theta);           // 00796225
            }
        }
        if (m.fps_78 && parent != 0 && host.is_kind(parent, kKind0F)) {
            host.fps_attach_007c7800(parent, m.camera_8);                        // 0079639A
        }
        host.camera_update_dc(m.camera_8, dt);
        const std::array<float, 16> w = host.camera_matrix_114(m.camera_8);
        k.matrix_7c = w;
        k.matrix_valid_bc = true;
        k.work_40 = {w[12], w[13], w[14]};
        k.work_up_64 = {w[4], w[5], w[6]};
        if (k.linked_20 != nullptr) {                                            // 007964DF
            ModifierKeyframe& l = *k.linked_20;
            l.modifier_flag_25 = true;
            l.up_58 = {w[4], w[5], w[6]};
            for (int i = 0; i < 3; ++i) {
                const float step = static_cast<float>(static_cast<double>(w[8 + i]) * kLinkedForward);
                l.local_34[i] = static_cast<float>(static_cast<double>(step) + k.work_40[i]);
            }
            l.work_up_64 = l.up_58;
            l.work_40 = l.local_34;
        }
    }
    m.last_time_c = t;                                                            // 007965BD
}

// 0079CBD0: built once (+8h non-null skips) and only with a parent. Kind 18h
// takes the entity's +3D0h first. The tests run in this order.
GameCameraChoice gamecamera_choose_camera_0079cbd0(const GameCameraModifier& m,
                                                   GameCameraModifierHost& host) {
    GameCameraChoice c{};
    if (m.camera_8 != 0 || m.owner_4 == nullptr || m.owner_4->parent_1c == 0) return c;
    std::size_t e = m.owner_4->parent_1c;
    if (host.is_kind(e, kKind18)) e = host.squadron_leader_3d0(e);
    c.entity = e;
    if (host.is_kind(e, kKind08)) c.cls = GameCameraClass::Kind08_00650830;
    else if (host.is_kind(e, kKind06)) c.cls = GameCameraClass::Kind06_ShipCaptain_0064b650;
    else if (host.is_kind(e, kKind0F)) c.cls = m.fps_78 ? GameCameraClass::Kind0F_Fps_006065d0
                                                       : GameCameraClass::Kind0F_00605130;
    else if (host.is_kind(e, kKind46)) c.cls = GameCameraClass::Kind46_0078e830;
    else if (host.is_kind(e, kKind45)) c.cls = GameCameraClass::Kind45_005176f0;
    return c;
}

// ---------------------------------------------------------------------------
// keyframe events

bool keyframe_event_parse_007a1f94(const ModifierLuaValue& event, std::size_t parent,
                                   bool camera_or_named, KeyframeEvent& out) {
    if (event.kind != ModifierLuaValue::Kind::Table || !camera_or_named) return false;
    const ModifierLuaValue& type = event.field("type");
    KeyframeEvent e{};
    const std::string& name = type.string;
    if (name == "noparent") e.kind = KeyframeEventKind::NoParent;
    else if (name == "noentity") e.kind = KeyframeEventKind::NoEntity;
    else if (name == "killed") e.kind = KeyframeEventKind::Killed;
    else if (name == "sunk") e.kind = KeyframeEventKind::Sunk;
    else return false;
    e.entity_24 = parent;                          // 007A207F / 007A2126 / 007A21AC / 007A2236
    e.function_4 = event.field("function").string; // 0079C000
    if (e.kind == KeyframeEventKind::Sunk) e.depth_28 = event.field("depth").number_f32();  // 007A22CB
    out = std::move(e);
    return true;
}

bool keyframe_event_check(const KeyframeEvent& e, KeyframeEventHost& host) {
    if (e.fired_c) return false;
    switch (e.kind) {
    case KeyframeEventKind::NoParent:   // 00791570: the entity's scene parent is gone
        return e.entity_24 != 0 && host.scene_parent_3c(e.entity_24) == 0;
    case KeyframeEventKind::NoEntity:   // 00791590
        return e.entity_24 == 0;
    case KeyframeEventKind::Killed:     // 007915B0
        return e.entity_24 == 0 || host.destroyed_5e(e.entity_24);
    case KeyframeEventKind::Sunk:       // 007915D0: FLD y, FLD depth, FCOMPI, JA
        if (e.entity_24 == 0 || host.destroyed_5e(e.entity_24)) return true;
        return e.depth_28 > host.world_y_100(e.entity_24);
    }
    return false;
}

void keyframe_event_fire_00791540(KeyframeEvent& e, KeyframeEventHost& host) {
    e.fired_c = true;
    if (!e.function_4.empty()) host.run_script_00887e50(e.function_4);
}

void keyframe_tick_0078e630(int state_f8, float dt, std::optional<KeyframeEvent>& event_ec,
                            KeyframeEventHost& host) {
    if (state_f8 == 0 || state_f8 == 3 || !(dt > 0.0f) || !event_ec) return;
    if (!keyframe_event_check(*event_ec, host)) return;
    keyframe_event_fire_00791540(*event_ec, host);
    event_ec.reset();
}

// ---------------------------------------------------------------------------
// path camera, 007972F0

namespace {
constexpr double kPathEndMargin = 0.009999999776482582;  // 00D7A358 (double)
constexpr double kPi = 3.1415927410125732;               // 00CE3D28 (double)
constexpr double kTwoPi = 6.2831854820251465;            // 00CE3828 (double)

// (t - knot[s]) / (knot[s+1] - knot[s]), the double chain stored once as float.
float segment_fraction(PathCameraHost& h, float t, int s) {
    const double d = static_cast<double>(t) - h.knot_time_007b05e0(s);
    const double next = h.knot_time_007b05e0(s + 1);
    return static_cast<float>(d / (next - h.knot_time_007b05e0(s)));
}

// 00797880..00797908: bring a pair within pi of each other by moving the
// larger one down by 2*pi, rounded to float.
void unwrap_pair(float& a, float& b) {
    if (static_cast<double>(a) - b > kPi) a = static_cast<float>(a - kTwoPi);
    else if (static_cast<double>(b) - a > kPi) b = static_cast<float>(b - kTwoPi);
}
}  // namespace

// 007972F0. Re-anchors the control points, keeps the parameter at the same
// fraction of its segment when the knots are re-derived (the snapshot +4FCh
// moves with it), clamps it to 0.01 before the last knot, samples the +464h path
// for the position and aims along its direction, then blends +4D0h's value and
// the records' theta/rho across each record's blend window with 0078FCF0.
void path_camera_update_007972f0(PathCamera& c, PathCameraHost& h) {
    if (!c.has_knot_path_474) return;
    const int seg = h.knot_segment_007b0610(c.parameter_468);
    const float u = segment_fraction(h, c.parameter_468, seg);
    for (std::size_t i = 0; i < c.records.size(); ++i) {
        const PathCameraRecord& r = c.records[i];
        const std::array<float, 3> p = r.anchor_14 == 0 ? r.point_18 : h.anchor_point(r.anchor_14, r.point_18);
        h.set_control_point_007ad180(static_cast<int>(i), p);
    }
    if (!h.knots_valid_18()) h.derive_knots_007af150();
    // 00797499..007974D4: t0 + (t1 - t0) * u with the knots just derived.
    const double next = h.knot_time_007b05e0(seg + 1);
    const double scaled = (next - h.knot_time_007b05e0(seg)) * u;
    const float moved = static_cast<float>(h.knot_time_007b05e0(seg) + scaled);
    c.snapshot_parameter_4fc = static_cast<float>((static_cast<double>(moved) - c.parameter_468) + c.snapshot_parameter_4fc);
    c.parameter_468 = moved;

    const int count = static_cast<int>(c.records.size());
    const float end = h.knot_time_007b05e0(count - 1);
    c.end_46c = end;
    const float limit = static_cast<float>(end - kPathEndMargin);
    const float t = limit > c.parameter_468 ? c.parameter_468 : limit;   // FCOMIP / JBE
    c.parameter_468 = t;
    std::array<float, 3> dir{};
    h.sample_world_007b04c0(t, c.position_394, dir);
    h.aim_at_point_00794070(c, {c.position_394[0] + dir[0], c.position_394[1] + dir[1],
                                c.position_394[2] + dir[2]});

    const int s = h.knot_segment_007b0610(t);
    const auto rs = static_cast<std::size_t>(s);
    const bool modes = c.records.at(rs).mode_48 != 0 || c.records.at(rs + 1).mode_48 != 0;
    const float v = segment_fraction(h, t, s);
    const float a = c.records.at(rs).blend_out_40;
    const float b = 1.0f - c.records.at(rs + 1).blend_in_3c;               // FSTP
    if (a > v) {
        c.node_value_4d0_34 = c.records[rs].node_value_44;
        if (modes) h.apply_record_mode_007954a0(c, s);
        return;
    }
    if (v > b) {
        c.node_value_4d0_34 = c.records[rs + 1].node_value_44;
        if (modes) h.apply_record_mode_007954a0(c, s + 1);
        return;
    }
    const float x = static_cast<float>((static_cast<double>(v) - a) / (static_cast<double>(b) - a));
    const float e = modifier_ease(x);
    const double one_minus_e = 1.0 - e;
    c.node_value_4d0_34 = static_cast<float>(one_minus_e * c.records[rs].node_value_44 +
                                             static_cast<double>(e) * c.records[rs + 1].node_value_44);
    if (!modes) return;
    h.apply_record_mode_007954a0(c, s);
    float a0 = c.theta_3c4, a1 = c.rho_3c8;
    h.apply_record_mode_007954a0(c, s + 1);
    float b0 = c.theta_3c4, b1 = c.rho_3c8;
    unwrap_pair(a0, b0);
    unwrap_pair(a1, b1);
    const float w = static_cast<float>(one_minus_e);
    const float p0 = static_cast<float>(static_cast<double>(b0) * e);
    const float p1 = static_cast<float>(static_cast<double>(b1) * e);
    const float r0 = static_cast<float>(static_cast<double>(a0) * w);
    const float r1 = static_cast<float>(static_cast<double>(a1) * w);
    c.theta_3c4 = static_cast<float>(static_cast<double>(r0) + p0);
    c.rho_3c8 = static_cast<float>(static_cast<double>(r1) + p1);
}

// ---------------------------------------------------------------------------
// aim deltas

namespace {
// 0079475A..007947A8 (and 007947AC..007947F6 for b).
std::array<float, 3> unit_or_zero(const std::array<float, 3>& v) {
    const float len = length_00419440(OceanVec3{v[0], v[1], v[2]});
    const float inv = len > 0.0f ? static_cast<float>(1.0 / len) : 0.0f;
    return {v[0] * inv, v[1] * inv, v[2] * inv};
}
}  // namespace

std::array<float, 2> camera_direction_delta_00794750(const std::array<float, 3>& a,
                                                     const std::array<float, 3>& b) {
    const std::array<float, 3> an = unit_or_zero(a);
    const std::array<float, 3> bn = unit_or_zero(b);
    const float yaw_a = static_cast<float>(std::atan2(static_cast<double>(an[0]), static_cast<double>(an[2])));
    const float yaw_b = static_cast<float>(std::atan2(static_cast<double>(bn[0]), static_cast<double>(bn[2])));
    std::array<float, 2> out{};
    out[1] = std::isfinite(yaw_a) && std::isfinite(yaw_b) ? wrapped_angle_subtract_00438b10(yaw_b, yaw_a) : 0.0f;
    const float pitch_a = camera_asin_clamped_0042cf10(an[1]);
    const float pitch_b = camera_asin_clamped_0042cf10(bn[1]);
    out[0] = std::isfinite(pitch_a) && std::isfinite(pitch_b)
                 ? -wrapped_angle_subtract_00438b10(pitch_b, pitch_a) : 0.0f;
    return out;
}

std::array<float, 2> camera_aim_delta_00794910(const AimCameraView& c, const AimTargetView& t, float scale) {
    if (0.0f > scale) scale = c.default_scale_4d0_54;
    std::array<float, 3> d{};
    if (c.use_local_point_4b4) {
        std::array<float, 3> p{};
        transform_point_004142e0(c.local_point_4b8, t.world_cc, p);
        for (int i = 0; i < 3; ++i) d[i] = p[i] - c.position_a4[i];
    } else {
        std::array<float, 3> r{};
        for (int i = 0; i < 3; ++i) r[i] = t.position_fc[i] - c.position_a4[i];
        const float dist = force_event_vector_length_0042b2f0(r);
        const float div = c.divisor_460;
        for (int i = 0; i < 3; ++i) {
            const float lead = ((dist * t.velocity_ec[i]) * scale) / div;   // three FSTPs
            const float aim = t.position_fc[i] + lead;                       // FSTP
            d[i] = aim - c.position_a4[i];                                   // FSTP
        }
    }
    return camera_direction_delta_00794750(c.forward_94, d);
}

// ---------------------------------------------------------------------------
// MovCam

std::array<float, 3> camera_ref_resolve_00793ac0(const CameraRef& ref, bool level, CameraRefHost& host) {
    if (ref.radius_24 > 0.0f) {                                           // COMISS 0.0 / JBE
        const std::array<float, 3> c = ref.entity_14 != 0 ? host.entity_position_fc(ref.entity_14) : ref.point_18;
        const std::array<float, 3> v = host.viewer_position();
        const std::array<float, 3> d = host.normalize_00419510({v[0] - c[0], v[1] - c[1], v[2] - c[2]});
        std::array<float, 3> out{};
        for (int i = 0; i < 3; ++i) {
            const float step = d[i] * ref.radius_24;                       // FSTP
            out[i] = step + c[i];                                          // FSTP
        }
        return out;
    }
    if (ref.entity_14 == 0) return ref.point_18;
    return host.entity_transform(ref.entity_14, ref.point_18, level);
}

void movcam_follow_00795180(MovCamFollow& c, float dt, MovCamFollowHost& host) {
    // FLD k, FSUB 1, FMUL dt, FDIV 0.05f, FADD 1, FMUL base, FSTP (007951A6).
    const double grow = (static_cast<double>(c.growth_3ec) - 1.0) * dt / static_cast<double>(0.05f) + 1.0;
    const float tol = static_cast<float>(grow * c.base_4f8);
    c.tolerance_3e8 = tol;
    if (0.001f > tol) {
        host.aim_at_target_00794130(c.target_3e4);
        return;
    }
    const std::array<float, 2> d = host.aim_delta_00794910(c.target_3e4);
    const std::array<float, 2> v{-d[0], -d[1]};
    const float sum = static_cast<float>(static_cast<double>(v[1]) * v[1] + static_cast<double>(v[0]) * v[0]);
    const float len = static_cast<double>(sum) > 1e-10 ? static_cast<float>(std::sqrt(static_cast<double>(sum))) : 0.0f;
    if (len > c.tolerance_3e8) {
        const float r = host.reciprocal_length_00419260(v);
        const float u0 = r * v[0];
        const float u1 = r * v[1];
        const float excess = len - c.tolerance_3e8;
        c.theta_3c4 = c.theta_3c4 + u0 * excess;
        c.rho_3c8 = (excess * u1) + c.rho_3c8;
        return;
    }
    c.base_4f8 = len;
    c.tolerance_3e8 = len;
}

namespace {
bool iequals(const std::string& a, const char* b) {
    std::size_t i = 0;
    for (; i < a.size() && b[i] != 0; ++i) {
        const auto x = static_cast<unsigned char>(a[i]);
        const auto y = static_cast<unsigned char>(b[i]);
        if (std::tolower(x) != std::tolower(y)) return false;
    }
    return i == a.size() && b[i] == 0;
}
}  // namespace

void movcam_interpolator_init_007900b0(MovCamInterpolator& r, const std::string& name,
                                       const std::array<std::uint32_t, 4>& args) {
    static const char* const kNames[] = {"turnHInput", "turnVInput", "walkHInput", "walkVInput",
                                         "zoomInput", "moveSpeed", "rotSpeed", "zoom"};
    for (std::uint32_t i = 0; i < 8; ++i) {
        if (iequals(name, kNames[i])) {
            r.id_0 = i + 1;
            break;
        }
    }
    r.args_4 = args;
    r.field_14 = 0;
}

namespace {
// (x*x + y*y) + z*z, stored float; sqrt only above 1e-10 (00CE3820). cc13-fixture2 follow-up:
// the MovCam callers (007943D0, 00794B80, 00794D60, 00794F70, 007A5A00) run on the main thread,
// so the sums of squares and the 00793730/00793800 divisions round to 24 bits (x87_*24).
float length_by_squares(const std::array<float, 3>& v) noexcept {
    const float sum = static_cast<float>(cc13x87::x87_add24(
        cc13x87::x87_add24(cc13x87::x87_mul24(v[0], v[0]), cc13x87::x87_mul24(v[1], v[1])), cc13x87::x87_mul24(v[2], v[2])));
    return static_cast<double>(sum) > 1e-10 ? static_cast<float>(std::sqrt(static_cast<double>(sum))) : 0.0f;
}
}  // namespace

std::array<float, 3> scale_to_length_00793730(const std::array<float, 3>& v, float length) noexcept {
    const float len = length_by_squares(v);
    if (9.999999974752427e-07f > std::fabs(len)) return {0.0f, 0.0f, 0.0f};    // 00D7A288
    const float s = static_cast<float>(cc13x87::x87_div24(length, len));          // FSTP 007937D6
    return {static_cast<float>(cc13x87::x87_mul24(v[0], s)), static_cast<float>(cc13x87::x87_mul24(s, v[1])),
            static_cast<float>(cc13x87::x87_mul24(s, v[2]))};
}

void movcam_steer_00794b80(MovCamMotion& m, const MovCamLimits& limits, const std::array<float, 3>& dir,
                           float max, bool square) noexcept {
    const float len = length_by_squares(dir);
    const float clamped = len > max ? max : len;
    const float want_len = static_cast<float>(static_cast<double>(limits.top_speed_34) / max * clamped);
    const std::array<float, 3> want = scale_to_length_00793730(dir, want_len);
    std::array<float, 3> diff{};
    for (int i = 0; i < 3; ++i) {
        const float cur = m.velocity_3a0[i] - m.reference_velocity_3b8[i];                // FSTP
        diff[i] = want[i] - cur;                                                // FSTP
    }
    const float accel = limits.accel_4c * 0.05f;                                // 00D0DE84, FSTP
    float f = static_cast<float>(static_cast<double>(length_by_squares(diff)) / accel);  // FSTP
    if (f > 1.0f) f = 1.0f;
    if (square) f = f * f;
    const std::array<float, 3> step = scale_to_length_00793730(diff, limits.accel_4c * f);
    for (int i = 0; i < 3; ++i) m.acceleration_3ac[i] = m.acceleration_3ac[i] + step[i];
}

void movcam_keep_above_ground_00790540(std::array<float, 3>& p, bool has_ground, float ground) noexcept {
    if (!has_ground) return;
    const float floor = static_cast<float>(static_cast<double>(ground) + 2.0);    // double 00D7A308
    if (!(p[1] > floor)) p[1] = floor;
}

namespace {
float length2_by_squares(const std::array<float, 2>& v) noexcept {
    const float sum = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(v[0], v[0]), cc13x87::x87_mul24(v[1], v[1])));
    return static_cast<double>(sum) > 1e-10 ? static_cast<float>(std::sqrt(static_cast<double>(sum))) : 0.0f;
}
}  // namespace

std::array<float, 2> scale_to_length_00793800(const std::array<float, 2>& v, float length) noexcept {
    const float len = length2_by_squares(v);
    if (9.999999974752427e-07f > std::fabs(len)) return {0.0f, 0.0f};
    const float s = static_cast<float>(cc13x87::x87_div24(length, len));
    return {static_cast<float>(cc13x87::x87_mul24(v[0], s)), static_cast<float>(cc13x87::x87_mul24(s, v[1]))};
}

void movcam_turn_00794d60(MovCamTurn& t, const MovCamTurnLimits& limits, const std::array<float, 2>& d,
                          float max, bool square, std::size_t target, float length_00414c60_of_d) noexcept {
    const float len = length2_by_squares(d);
    const float clamped = len > max ? max : len;
    const float want_len = static_cast<float>(static_cast<double>(limits.top_rate_38) / max * clamped);
    const std::array<float, 2> want = scale_to_length_00793800(d, want_len);
    std::array<float, 2> diff{};
    for (int i = 0; i < 2; ++i) {
        const float cur = t.rate_3cc[i] - t.reference_rate_3dc[i];
        diff[i] = want[i] - cur;
    }
    const float accel = 0.05f * limits.accel_50;
    float f = static_cast<float>(static_cast<double>(length2_by_squares(diff)) / accel);
    if (f > 1.0f) f = 1.0f;
    if (square) f = f * f;
    if (target != 0) {
        const float l = length_00414c60_of_d;
        if (t.target_3e4 == 0) t.tolerance_3e8 = l;
        else t.tolerance_3e8 = l > t.tolerance_3e8 ? t.tolerance_3e8 : l;
        t.growth_3ec = 0.875f;
        t.target_3e4 = target;
    }
    const std::array<float, 2> step = scale_to_length_00793800(diff, f * limits.accel_50);
    t.turn_acceleration_3d4[0] = t.turn_acceleration_3d4[0] + step[0];
    t.turn_acceleration_3d4[1] = step[1] + t.turn_acceleration_3d4[1];
}

}  // namespace bsp::cc13cam
