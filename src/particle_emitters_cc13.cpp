// Particle geometry pass and quad emitters, segment 82 (packet cc13_water2
// particles, cc13_particles1). Addresses 00B057C0, 00B06A90, 00B0C8B0, 00B05420,
// 00B05550, 00B06B10, 00B08210, 00B09510, the slot +24h/+1Ch bodies, 00AF8500 and 00AF85B0. Evidence, ABI and uncertainty: include/bsp/particle_emitters_cc13.hpp
// and docs/CC13_PARTICLE_EMITTERS.md.

#include "bsp/particle_emitters_cc13.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <utility>

#include "bsp/camera_affine.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/model_bounds.hpp"
#include "bsp/native_camera_projection.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp::particle_emitters {
namespace {

template <typename T>
T rd(const void* base, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <typename T>
void wr(void* base, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(base) + offset, &value, sizeof value);
}

const void* ptr(const void* base, std::uint32_t offset) {
    return rd<const void*>(base, offset);
}

// Kind 0 is inlined by the native as value * t (FMUL [curve+4], no spill).
double integrate(ParticleCurves& curves, const void* parameter, float t) {
    const std::uint16_t kind = rd<std::uint16_t>(parameter, 0x0a);
    if (kind == 0) {
        return static_cast<double>(rd<float>(parameter, 4)) * t;
    }
    return kind == 1 ? curves.integrate_linear_00affd20(parameter, t)
                     : curves.integrate_hermite_00affcb0(parameter, t);
}

double evaluate(ParticleCurves& curves, const void* curve, float t) {
    const std::uint16_t kind = rd<std::uint16_t>(curve, 0x0a);
    if (kind == 0) {
        return rd<float>(curve, 4);
    }
    return kind == 1 ? curves.evaluate_linear_00affa70(curve, t) : curves.evaluate_cubic_00affae0(curve, t);
}

// B056B1..B056D9 and B0C8C3..B0C8FB: life percent, clamped to 100.
float life_percent(const void* state) {
    float percent = static_cast<float>(rd<float>(state, 0x40) * 100.0 / rd<float>(state, 0x44));
    if (percent > 100.0f) {
        percent = 100.0f;  // [00CE3D08]
    }
    return percent;
}

std::int32_t random_frame(const void* definition, ParticleRandom& random) {
    const std::int32_t lo = rd<std::int32_t>(definition, 0x50);
    const std::int32_t hi = rd<std::int32_t>(definition, 0x54);
    const std::uint32_t span = static_cast<std::uint32_t>(hi - lo) + 1u;
    return static_cast<std::int32_t>(random.next_u32_stream0() % span) + lo;
}

// B095EE..B09872 (Sprite) and AF863E..AF88B0 (Object): speed, gravity along
// matrix row 1, wind, optional def+48h along state+24h..2Ch, then the origin.
// Every product is spilled to float; the sum is stored to state+0..8.
void full_position(const void* definition, void* state, float t, const float matrix[16],
                   const float wind_e13028[3], ParticleCurves& curves, float scale, float p[3]) {
    // B095EE..B0965C: speed term, stored to state+0..8 first.
    const float ks = static_cast<float>(integrate(curves, ptr(definition, 0x2c), t) * scale);
    p[0] = rd<float>(state, 0x18) * ks;
    p[1] = rd<float>(state, 0x1c) * ks;
    p[2] = ks * rd<float>(state, 0x20);
    // B09669..B09741: gravity along matrix row 1, each product spilled.
    {
        const float g = static_cast<float>(integrate(curves, ptr(definition, 0x30), t));
        const float gx = static_cast<float>(g * matrix[4]);
        const float gy = static_cast<float>(matrix[5] * g);
        const float gz = static_cast<float>(g * matrix[6]);
        const float sx = gx * scale, sy = gy * scale, sz = gz * scale;
        const float weight = rd<float>(state, 0x48);
        p[0] = p[0] + sx * weight;
        p[1] = p[1] + sy * weight;
        p[2] = p[2] + weight * sz;
    }
    // B0974E..B097CC: wind [00E13028..30] times +4Ch.
    {
        const float ws = static_cast<float>(integrate(curves, ptr(definition, 0x34), t) * scale *
                                            rd<float>(state, 0x4c));
        p[0] = p[0] + ws * wind_e13028[0];
        p[1] = p[1] + ws * wind_e13028[1];
        p[2] = p[2] + ws * wind_e13028[2];
    }
    // B097D4..B09855: optional parameter +48h along the state vector +24h..2Ch.
    if (const void* extra = ptr(definition, 0x48); extra != nullptr) {
        const float v = static_cast<float>(scale * integrate(curves, extra, t));
        p[0] = p[0] + v * rd<float>(state, 0x24);
        p[1] = p[1] + rd<float>(state, 0x28) * v;
        p[2] = p[2] + v * rd<float>(state, 0x2c);
    }
    p[0] = p[0] + rd<float>(state, 0x0c);
    p[1] = rd<float>(state, 0x10) + p[1];
    p[2] = rd<float>(state, 0x14) + p[2];
    wr<float>(state, 0x00, p[0]);
    wr<float>(state, 0x04, p[1]);
    wr<float>(state, 0x08, p[2]);
}

}  // namespace

std::int32_t random_int_inclusive_00b057c0(std::int32_t lo, std::int32_t hi, ParticleRandom& random) {
    const std::uint32_t draw = random.next_u32_stream0();  // 00B057C8, ECX = 0
    return static_cast<std::int32_t>(draw % (static_cast<std::uint32_t>(hi - lo) + 1u)) + lo;
}

std::int32_t resolve_frame_index_00b06a90(const void* definition, std::int32_t frame) {
    const std::int32_t lo = rd<std::int32_t>(definition, 0x50);
    const std::int32_t hi = rd<std::int32_t>(definition, 0x54);
    if (rd<std::uint8_t>(definition, 0x62) != 0) {
        const std::int32_t span = hi - lo + 1;
        return span > 0 ? (frame - lo) % span + lo : lo;
    }
    if (frame < lo) {
        return lo;
    }
    return frame > hi ? hi : frame;
}

bool advance_texture_frame_00b0c8b0(void* state, ParticleCurves& curves, ParticleRandom& random,
                                    float& result) {
    const void* definition = ptr(state, 0x64);
    if (rd<std::uint8_t>(definition, 0x4c) == 0) {
        return false;
    }
    const float percent = life_percent(state);
    const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);
    const void* rate = ptr(definition, 0x5c);
    // B0C928 / B0C937 / B0C93E: kind 0 stays extended, the calls are spilled.
    const std::uint16_t kind = rd<std::uint16_t>(rate, 0x0a);
    double v = integrate(curves, rate, percent);
    if (kind != 0) {
        v = static_cast<float>(v);
    }
    const float frame = static_cast<float>(v * scale * rd<float>(state, 0x3c) + rd<float>(state, 0x34));
    if (rd<std::uint8_t>(definition, 0x61) != 0) {
        const float previous = rd<float>(state, 0x30);
        wr<float>(state, 0x30, frame);
        const float delta = static_cast<float>(static_cast<double>(frame) - previous);
        const float wrapped = static_cast<float>(std::fmod(static_cast<double>(frame), 1.0));  // 00BF857A
        const float sum = wrapped + delta;
        if (sum > 1.0f) {
            const float next = rd<float>(state, 0x38);
            wr<float>(state, 0x30, next);
            const float chosen = static_cast<float>(random_frame(definition, random));  // 00B0C9AA
            const float fraction = static_cast<float>(std::fmod(static_cast<double>(next), 1.0));
            wr<float>(state, 0x38, fraction + chosen);
        }
        result = delta;
        return true;
    }
    wr<float>(state, 0x30, frame);
    wr<float>(state, 0x38, static_cast<float>(frame + 1.0));  // [00D7A210]
    result = rd<float>(state, 0x30);
    return true;
}

void sort_rows_descending_00b05420(std::int32_t lo, std::int32_t hi, std::uint8_t* rows) {
    auto key = [rows](std::int32_t i) { return rd<float>(rows, static_cast<std::uint32_t>(i) * 8 + 4); };
    for (;;) {
        const std::int32_t mid = (lo + hi) >> 1;
        std::int32_t i = lo;
        std::int32_t j = hi;
        do {
            const float pivot = key(mid);  // re-read every step (B05440)
            while (key(i) > pivot) {
                ++i;
            }
            while (pivot > key(j)) {
                --j;
            }
            if (i > j) {
                break;
            }
            const std::uint16_t index_i = rd<std::uint16_t>(rows, static_cast<std::uint32_t>(i) * 8);
            const std::uint16_t index_j = rd<std::uint16_t>(rows, static_cast<std::uint32_t>(j) * 8);
            wr<std::uint16_t>(rows, static_cast<std::uint32_t>(i) * 8, index_j);
            wr<std::uint16_t>(rows, static_cast<std::uint32_t>(j) * 8, index_i);
            const float key_i = key(i);
            wr<float>(rows, static_cast<std::uint32_t>(i) * 8 + 4, key(j));
            wr<float>(rows, static_cast<std::uint32_t>(j) * 8 + 4, key_i);
            ++i;
            --j;
        } while (i <= j);
        if (lo < j) {
            sort_rows_descending_00b05420(lo, j, rows);
        }
        if (i >= hi) {
            return;
        }
        lo = i;
    }
}

void build_container_geometry_00b05550(void* container, const float view[16], float argument,
                                       ParticleGeometryHost& host) {
    wr<std::int32_t>(container, 0x28, 0);
    if (rd<std::uint32_t>(container, 0x18) == 0) {
        return;
    }
    const void* owner = ptr(ptr(container, 0x08), 0x0c);
    if (rd<std::uint8_t>(owner, 0x2c) == 0) {
        return;
    }
    auto* rows = rd<std::uint8_t*>(container, 0x0c);
    auto* states = rd<std::uint8_t*>(container, 0x14);
    const std::int32_t count = rd<std::int32_t>(container, 0x1c);
    if (rd<std::uint8_t>(owner, 0x24) != 0) {
        // B05586..B055D9: depth keys, then the sort.
        for (std::int32_t i = 0; i < count; ++i) {
            const std::uint16_t index = rd<std::uint16_t>(rows, static_cast<std::uint32_t>(i) * 8);
            const std::uint8_t* s = states + index * 0x6c;
            const double depth = (static_cast<double>(rd<float>(s, 4)) * view[6] +
                                  static_cast<double>(rd<float>(s, 0)) * view[2]) +
                                 static_cast<double>(rd<float>(s, 8)) * view[10] + view[14];
            wr<float>(rows, static_cast<std::uint32_t>(i) * 8 + 4, static_cast<float>(depth));
        }
        if (rd<std::int32_t>(container, 0x10) > 1 && count >= 2) {
            sort_rows_descending_00b05420(0, count - 1, rows);
        }
    }
    std::int32_t total = 0;
    for (std::int32_t i = 0; i < rd<std::int32_t>(container, 0x1c); ++i) {
        void* model = rd<void*>(container, 0x04);
        float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        if (rd<std::uint8_t>(model, 0x1b0) == 0 && rd<std::uint8_t>(model, 0x1b1) == 0) {
            float placement[16];
            std::memcpy(placement, static_cast<const std::uint8_t*>(model) + 0x298, sizeof placement);  // REP MOVSD
            host.multiply_00413920(matrix, host.model_local_matrix_00b6db60(model), placement);
        }
        const std::uint16_t index = rd<std::uint16_t>(rows, static_cast<std::uint32_t>(i) * 8);
        void* state = states + index * 0x6c;
        const float percent = life_percent(state);
        total += host.emit_quads_slot28(rd<void*>(state, 0x64), state, percent, 0, matrix, argument);
    }
    wr<std::int32_t>(container, 0x28, total);
}

std::int32_t emit_axial_quad_00b06b10(const void* definition, void* state, float t,
                                      const float matrix[16], const float wind_e13028[3],
                                      ParticleCurves& curves, ParticleRandom& random,
                                      const CameraAxesCrtAccess& crt) {
    // B06B19..B06B3D: the emitter's fade gate.
    void* emitter = rd<void*>(state, 0x68);
    const float emitter_alpha = rd<float>(emitter, 0x24);
    if (0.004f > emitter_alpha) {  // [00CEC178]
        return 0;
    }
    void* target = rd<void*>(emitter, 0x08);
    const bool second_buffer = rd<std::uint8_t>(ptr(state, 0x64), 0x65) != 0;
    auto* vertex = rd<std::uint8_t*>(target, second_buffer ? 0x1d0 : 0x1c0);

    // B06B59..B06D71: position = origin + speed, gravity and wind integrals.
    const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);
    float p[3];
    {
        const void* speed = ptr(definition, 0x2c);
        double k = integrate(curves, speed, t);
        if (rd<std::uint16_t>(speed, 0x0a) != 0) k = static_cast<float>(k);
        const float ks = static_cast<float>(k * scale);
        p[0] = ks * rd<float>(state, 0x18);
        p[1] = rd<float>(state, 0x1c) * ks;
        p[2] = ks * rd<float>(state, 0x20);
    }
    {
        const void* gravity = ptr(definition, 0x30);
        const float g = static_cast<float>(integrate(curves, gravity, t));  // spilled (B06C36)
        const float gx = static_cast<float>(g * matrix[4]);
        const float gy = static_cast<float>(matrix[5] * g);
        const float gz = static_cast<float>(g * matrix[6]);
        const float sx = gx * scale, sy = gy * scale, sz = scale * gz;
        const float weight = rd<float>(state, 0x48);
        p[0] = p[0] + sx * weight;
        p[1] = p[1] + sy * weight;
        p[2] = p[2] + weight * sz;
    }
    {
        const void* wind = ptr(definition, 0x34);
        double w = integrate(curves, wind, t);
        if (rd<std::uint16_t>(wind, 0x0a) != 0) w = static_cast<float>(w);
        const float ws = static_cast<float>(w * scale * rd<float>(state, 0x4c));
        p[0] = p[0] + ws * wind_e13028[0];
        p[1] = p[1] + ws * wind_e13028[1];
        p[2] = p[2] + ws * wind_e13028[2];
    }
    p[0] = p[0] + rd<float>(state, 0x0c);
    p[1] = p[1] + rd<float>(state, 0x10);
    p[2] = p[2] + rd<float>(state, 0x14);
    wr<float>(state, 0x00, p[0]);
    wr<float>(state, 0x04, p[1]);
    wr<float>(state, 0x08, p[2]);
    // B06D78..B06D83: frame advance, result discarded.
    if (rd<std::uint8_t>(definition, 0x4c) != 0) {
        float ignored;
        advance_texture_frame_00b0c8b0(state, curves, random, ignored);
    }

    // B06D89..B06EC6: alpha gate and premultiplied colour (x 255.0 [00CE4B48]).
    float alpha = static_cast<float>(rd<float>(state, 0x5c) * evaluate(curves, ptr(definition, 0x38), t) *
                                     emitter_alpha);
    if (0.004f > alpha) {
        return 0;
    }
    alpha = static_cast<float>(alpha * 255.0);
    const float red = static_cast<float>(evaluate(curves, ptr(definition, 0x3c), t) * alpha);
    const float green = static_cast<float>(evaluate(curves, ptr(definition, 0x40), t) * alpha);
    const float blue = static_cast<float>(evaluate(curves, ptr(definition, 0x44), t) * alpha);
    const std::int32_t blend_mode = rd<std::int32_t>(definition, 0x7c);

    // B06ECA..B070D6: axis; FollowDirection (+80h) uses the current velocity.
    float axis[3];
    if (rd<std::uint8_t>(definition, 0x80) == 0) {
        axis[0] = rd<float>(definition, 0x94);
        axis[1] = rd<float>(definition, 0x98);
        axis[2] = rd<float>(definition, 0x9c);
    } else {
        const float ks = static_cast<float>(evaluate(curves, ptr(definition, 0x2c), t) * scale);
        const float v1[3] = {rd<float>(state, 0x18) * ks, rd<float>(state, 0x1c) * ks, ks * rd<float>(state, 0x20)};
        const double g = evaluate(curves, ptr(definition, 0x30), t);
        const float g0 = static_cast<float>(matrix[4] * g), g1 = static_cast<float>(matrix[5] * g),
                    g2 = static_cast<float>(g * matrix[6]);
        const float s0 = g0 * scale, s1 = g1 * scale, s2 = scale * g2;
        const float weight = rd<float>(state, 0x48);
        float v2[3] = {s0 * weight + v1[0], s1 * weight + v1[1], weight * s2 + v1[2]};
        const float ws = static_cast<float>(evaluate(curves, ptr(definition, 0x34), t) * scale *
                                            rd<float>(state, 0x4c));
        const float velocity[3] = {ws * wind_e13028[0] + v2[0], ws * wind_e13028[1] + v2[1],
                                   ws * wind_e13028[2] + v2[2]};
        camera_vector_normalize_00419510(axis, velocity, &crt);
    }

    // B070D6..B07205: Length (+8Ch) and Width (+90h) times size +58h; Alignment +A0h.
    const float length = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x8c), t));
    const float width = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x90), t));
    float x0 = length, x1 = 0.0f, y1 = 0.0f;
    switch (rd<std::int32_t>(definition, 0xa0)) {
    case 0:  // Center
        x1 = -0.0f - length;
        y1 = -0.0f - width;
        break;
    case 1:  // Bottom
        x0 = length + length;
        y1 = -0.0f - width;
        break;
    case 2:  // Top
        x0 = 0.0f;
        x1 = static_cast<float>(-static_cast<double>(length) - length);
        y1 = -0.0f - width;
        break;
    default:  // Left/Right (3/4) are not handled: x1 = y1 = 0
        break;
    }

    // B07205..B07276: frames (floor 00BF85B0, truncation 00BF7420) and blend.
    const float frame = rd<float>(state, 0x30);
    const float next = rd<float>(state, 0x38);
    const std::int32_t frame_a = resolve_frame_index_00b06a90(
        definition, static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(frame)))));
    const std::int32_t frame_b = resolve_frame_index_00b06a90(
        definition, static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(next)))));
    const float blend = static_cast<float>(std::fmod(static_cast<double>(frame), 1.0));

    // B07280..B0732E: clamp to [0, 255] (NaN passes through), CVTTSS2SI, pack.
    auto clamp = [](float v) {
        if (0.0f > v) return 0.0f;
        return v > 255.0 ? 255.0f : v;  // double [00CE4B48], float [00CEDD44]
    };
    std::uint32_t argb = static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(blue))) |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(green)))) << 8 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(red)))) << 16 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(alpha)))) << 24;
    if (blend_mode == 1) {
        argb = (argb & 0x00ffffffu) | 0x01000000u;  // additive: alpha byte = 1
    }
    const std::uint16_t flag_half = blend_mode == 1 ? 0x3c00 : 0x0000;  // B073D9..B073EF

    // B07332..B07590: four 38h-byte vertices.
    const std::uint8_t* frames = rd<const std::uint8_t*>(definition, 0x68);
    const std::uint8_t* fa = frames + frame_a * 0x1c;
    const std::uint8_t* fb = frames + frame_b * 0x1c;
    struct Corner {
        std::uint32_t u, v;
        float x, y;
    };
    const Corner corners[4] = {{0x10, 0x12, x0, width}, {0x14, 0x12, x0, y1}, {0x10, 0x16, x1, width},
                               {0x14, 0x16, x1, y1}};
    for (const Corner& c : corners) {
        wr<float>(vertex, 0x00, p[0]);
        wr<float>(vertex, 0x04, p[1]);
        wr<float>(vertex, 0x08, p[2]);
        wr<float>(vertex, 0x0c, 0.0f);
        wr<std::uint32_t>(vertex, 0x10, argb);
        wr<float>(vertex, 0x14, axis[0]);
        wr<float>(vertex, 0x18, axis[1]);
        wr<float>(vertex, 0x1c, axis[2]);
        wr<float>(vertex, 0x20, blend);
        wr<std::uint16_t>(vertex, 0x24, rd<std::uint16_t>(fa, c.u));
        wr<std::uint16_t>(vertex, 0x26, rd<std::uint16_t>(fa, c.v));
        wr<std::uint16_t>(vertex, 0x28, rd<std::uint16_t>(fb, c.u));
        wr<std::uint16_t>(vertex, 0x2a, rd<std::uint16_t>(fb, c.v));
        wr<float>(vertex, 0x2c, c.x);
        wr<float>(vertex, 0x30, c.y);
        wr<std::uint16_t>(vertex, 0x34, 0x3c00);
        wr<std::uint16_t>(vertex, 0x36, flag_half);
        vertex += 0x38;
    }
    // B07590..B075C6: advance the selected buffer cursor, decrement its room.
    const std::uint32_t cursor = second_buffer ? 0x1d0 : 0x1c0;
    const std::uint32_t room = second_buffer ? 0x1cc : 0x1b8;
    wr<std::uint8_t*>(target, cursor, rd<std::uint8_t*>(target, cursor) + 0xe0);
    wr<std::int32_t>(target, room, rd<std::int32_t>(target, room) - 1);
    return 1;
}

std::int32_t emit_floating_quad_00b08210(const void* definition, void* state, float t, ParticleCurves& curves,
                                         ParticleRandom& random) {
    // B08213..B0823D: the emitter's fade gate; the cursor is read up front.
    void* emitter = rd<void*>(state, 0x68);
    const float emitter_alpha = rd<float>(emitter, 0x24);
    if (0.004f > emitter_alpha) {  // [00CEC178]
        return 0;
    }
    void* target = rd<void*>(emitter, 0x08);
    std::uint8_t* vertex = rd<std::uint8_t*>(target, 0x1c4);
    const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);  // [00D7A220]

    // B08264..B082C1: angle = +50h + +54h * (RotationSpeed integral * pi/180) * scale.
    const float angle = static_cast<float>(
        rd<float>(state, 0x54) * (integrate(curves, ptr(definition, 0x84), t) * 0.017453288659453392) * scale +
        rd<float>(state, 0x50));  // [00D5DAF8]

    // B082C5..B0833F: position = speed integral * scale * direction + origin.
    const float ks = static_cast<float>(integrate(curves, ptr(definition, 0x2c), t) * scale);
    const float d[3] = {rd<float>(state, 0x18) * ks, rd<float>(state, 0x1c) * ks, ks * rd<float>(state, 0x20)};
    wr<float>(state, 0x00, d[0]);
    wr<float>(state, 0x04, d[1]);
    wr<float>(state, 0x08, d[2]);
    wr<float>(state, 0x00, d[0] + rd<float>(state, 0x0c));
    wr<float>(state, 0x04, d[1] + rd<float>(state, 0x10));
    wr<float>(state, 0x08, d[2] + rd<float>(state, 0x14));
    // B08346..B08351: frame advance, result popped.
    if (rd<std::uint8_t>(definition, 0x4c) != 0) {
        float ignored;
        advance_texture_frame_00b0c8b0(state, curves, random, ignored);
    }

    // B08357..B084D6: alpha gate and premultiplied colour, as in 00B06B10.
    float alpha = static_cast<float>(rd<float>(state, 0x5c) * evaluate(curves, ptr(definition, 0x38), t) *
                                     emitter_alpha);
    if (0.004f > alpha) {  // unordered continues (JBE)
        return 0;
    }
    alpha = static_cast<float>(alpha * 255.0);  // [00CE4B48]
    const float red = static_cast<float>(evaluate(curves, ptr(definition, 0x3c), t) * alpha);
    const float green = static_cast<float>(evaluate(curves, ptr(definition, 0x40), t) * alpha);
    const float blue = static_cast<float>(evaluate(curves, ptr(definition, 0x44), t) * alpha);
    const std::int32_t blend_mode = rd<std::int32_t>(definition, 0x7c);

    // B084DA: the particle floats, y = 0. B084DF: Size (+88h) times size +58h.
    wr<float>(state, 0x04, 0.0f);
    const float size = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x88), t));

    // B084E3..B08549: frames (floor 00BF85B0, truncation 00BF7420) and blend (fmod 00BF857A).
    const float frame = rd<float>(state, 0x30);
    const std::int32_t frame_a = resolve_frame_index_00b06a90(
        definition, static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(frame)))));
    const std::int32_t frame_b = resolve_frame_index_00b06a90(
        definition,
        static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(rd<float>(state, 0x38))))));
    const float blend = static_cast<float>(std::fmod(static_cast<double>(frame), 1.0));

    // B0854D..B08605: clamp to [0, 255] (COMISS/FCOMIP with JBE: NaN passes through), pack.
    auto clamp = [](float v) {
        if (0.0f > v) return 0.0f;
        return v > 255.0 ? 255.0f : v;  // double [00CE4B48], float [00CEDD44]
    };
    std::uint32_t argb = static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(blue))) |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(green)))) << 8 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(red)))) << 16 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(alpha)))) << 24;
    if (blend_mode == 1) {
        argb = (argb & 0x00ffffffu) | 0x01000000u;  // additive: alpha byte = 1
    }

    // B0862C..B08787: four 24h-byte vertices when the cursor is non-null; the
    // room count drops either way (B0878D) and the result is 1.
    if (vertex != nullptr) {
        const std::uint8_t* frames = rd<const std::uint8_t*>(definition, 0x68);
        const std::uint8_t* fa = frames + frame_a * 0x1c;
        const std::uint8_t* fb = frames + frame_b * 0x1c;
        struct Corner {
            std::uint32_t u, v;
            float angle;
        };
        // Corner angles: +pi [00CE3D28], -pi/2 and +pi/2 [00CE3830], +0; doubles added in x87.
        const Corner corners[4] = {{0x14, 0x16, static_cast<float>(3.1415927410125732 + angle)},
                                   {0x10, 0x16, static_cast<float>(angle - 1.5707963705062866)},
                                   {0x14, 0x12, static_cast<float>(1.5707963705062866 + angle)},
                                   {0x10, 0x12, angle}};
        for (const Corner& c : corners) {
            wr<float>(vertex, 0x00, rd<float>(state, 0x00));
            wr<float>(vertex, 0x04, rd<float>(state, 0x04));
            wr<float>(vertex, 0x08, rd<float>(state, 0x08));
            wr<float>(vertex, 0x0c, c.angle);
            wr<std::uint32_t>(vertex, 0x10, argb);
            wr<std::uint16_t>(vertex, 0x14, rd<std::uint16_t>(fa, c.u));
            wr<std::uint16_t>(vertex, 0x16, rd<std::uint16_t>(fa, c.v));
            wr<std::uint16_t>(vertex, 0x18, rd<std::uint16_t>(fb, c.u));
            wr<std::uint16_t>(vertex, 0x1a, rd<std::uint16_t>(fb, c.v));
            wr<float>(vertex, 0x1c, blend);
            wr<float>(vertex, 0x20, size);
            vertex += 0x24;
        }
        wr<std::uint8_t*>(target, 0x1c4, rd<std::uint8_t*>(target, 0x1c4) + 0x90);
    }
    wr<std::int32_t>(target, 0x1bc, rd<std::int32_t>(target, 0x1bc) - 1);
    return 1;
}

std::int32_t emit_sprite_quad_00b09510(const void* definition, void* state, float t, const float matrix[16],
                                       float view_size_scale, const float wind_e13028[3], ParticleCurves& curves,
                                       ParticleRandom& random, SpriteParticleHost& host) {
    // B09516..B0954A: the emitter's fade gate.
    void* emitter = rd<void*>(state, 0x68);
    const float emitter_alpha = rd<float>(emitter, 0x24);
    if (0.004f > emitter_alpha) {  // [00CEC178]
        return 0;
    }
    void* target = rd<void*>(emitter, 0x08);
    const bool second_buffer = rd<std::uint8_t>(ptr(state, 0x64), 0x65) != 0;
    auto* vertex = rd<std::uint8_t*>(target, second_buffer ? 0x1d0 : 0x1c0);
    const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);  // [00D7A220]

    // B09586..B095EA: angle as in 00B08210.
    const float angle = static_cast<float>(
        rd<float>(state, 0x54) * (integrate(curves, ptr(definition, 0x84), t) * 0.017453288659453392) * scale +
        rd<float>(state, 0x50));  // [00D5DAF8]

    // B095EE..B09872: position, as 00AF85B0 (same order and spills).
    float p[3];
    full_position(definition, state, t, matrix, wind_e13028, curves, scale, p);
    // B09879..B0987F: frame advance, result popped.
    if (rd<std::uint8_t>(definition, 0x4c) != 0) {
        float ignored;
        advance_texture_frame_00b0c8b0(state, curves, random, ignored);
    }

    // B0988D..B09925: alpha (no 0.004 gate here) and size = +58h * curve(Size +88h).
    const float alpha = static_cast<float>(rd<float>(state, 0x5c) * evaluate(curves, ptr(definition, 0x38), t) *
                                           emitter_alpha);
    const std::int32_t blend_mode = rd<std::int32_t>(definition, 0x7c);
    const float size = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x88), t));

    // B09929..B09A38: near-camera fade. r = size * argument / view-space z,
    // clamped to [0, 1] (NaN passes); fade = (1 - r)^2 by the inline pow
    // (FYL2X/F2XM1/FSCALE, exponent 2.0f [00CE3958], negative base negated with
    // -0.0f [00D7A208]); fade = 0 when 1 - r == 0. Only gates, and only when
    // def+65h is clear: 0.007 [00D5E028] > fade * alpha returns 0.
    {
        CameraMatrix view;
        std::memcpy(view.data(), host.camera_view_00b6fcb0(), sizeof(float) * 16);  // REP MOVSD at B09944
        const double numerator = static_cast<double>(size) * view_size_scale;    // FSTP qword at B09946
        std::array<float, 3> local;
        transform_point_004142e0({p[0], p[1], p[2]}, view, local);
        float r = static_cast<float>(numerator / local[2]);
        if (0.0f > r) {
            r = 0.0f;
        } else if (r > 1.0f) {  // [00D7A24C]
            r = 1.0f;
        }
        float fade = 1.0f - r;
        if (fade == 0.0f) {
            fade = 0.0f;
        } else {
            if (0.0f > fade) fade = -0.0f - fade;
            fade = static_cast<float>(std::pow(static_cast<double>(fade), 2.0));
        }
        if (!second_buffer && 0.007000000216066837 > static_cast<double>(fade) * alpha) {
            return 0;
        }
    }

    // B09A4B..B09B06: alpha * 255 [00CE4B48]; raw colour curves.
    const float alpha255 = static_cast<float>(alpha * 255.0);
    const float red_curve = static_cast<float>(evaluate(curves, ptr(definition, 0x3c), t));
    const float green_curve = static_cast<float>(evaluate(curves, ptr(definition, 0x40), t));
    const float blue_curve = static_cast<float>(evaluate(curves, ptr(definition, 0x44), t));

    // B09B06..B09C02: the optional dynamic light at state+60h.
    if (void* light = rd<void*>(state, 0x60); light != nullptr) {
        float lp[3] = {p[0], p[1], p[2]};
        if (rd<std::uint8_t>(target, 0x1b0) != 0) {
            if ((rd<std::uint8_t>(target, 0x5c) & 2) == 0) {
                host.refresh_world_00b6db70(target);
            }
            lp[0] = p[0] + rd<float>(target, 0x120);
            lp[1] = p[1] + rd<float>(target, 0x124);
            lp[2] = p[2] + rd<float>(target, 0x128);
        }
        wr<float>(light, 0x1ec, lp[0]);  // inline copy of 00B094E0
        wr<float>(light, 0x1f0, lp[1]);
        wr<float>(light, 0x1f4, lp[2]);
        const float rgba[4] = {red_curve, green_curve, blue_curve, 1.0f};  // 1.0f [00D7A24C]
        host.light_set_base_diffuse_004b62e0(light, rgba);
        wr<float>(light, 0x1f8, 0.01f < size ? size : 0.01f);  // [00D7A238]
    }
    const float red = red_curve * alpha255;
    const float green = green_curve * alpha255;
    const float blue = alpha255 * blue_curve;

    // B09C2D..B09C9A: frames and blend as in 00B06B10.
    const float frame = rd<float>(state, 0x30);
    const std::int32_t frame_a = resolve_frame_index_00b06a90(
        definition, static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(frame)))));
    const std::int32_t frame_b = resolve_frame_index_00b06a90(
        definition,
        static_cast<std::int32_t>(static_cast<float>(std::floor(static_cast<double>(rd<float>(state, 0x38))))));
    const float blend = static_cast<float>(std::fmod(static_cast<double>(frame), 1.0));

    // B09C9A..B09D52: clamp to [0, 255] (NaN passes through), pack.
    auto clamp = [](float v) {
        if (0.0f > v) return 0.0f;
        return v > 255.0 ? 255.0f : v;  // double [00CE4B48], float [00CEDD44]
    };
    std::uint32_t argb = static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(blue))) |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(green)))) << 8 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(red)))) << 16 |
                         static_cast<std::uint32_t>(static_cast<std::uint8_t>(static_cast<std::int32_t>(clamp(alpha255)))) << 24;
    if (blend_mode == 1) {
        argb = (argb & 0x00ffffffu) | 0x01000000u;  // additive: alpha byte = 1
    }
    const std::uint16_t flag_half = blend_mode == 1 ? 0x3c00 : 0x0000;  // NEG/SBB/AND C400h/ADD 3C00h

    // B09D52..B09FAB: four 38h-byte vertices, corner angle = angle + {-pi, -pi/2,
    // +pi/2, 0} + pi/4 ([00CE3D28], [00CE3830], [00CEDCD0], doubles in x87).
    const std::uint8_t* frames = rd<const std::uint8_t*>(definition, 0x68);
    const std::uint8_t* fa = frames + frame_a * 0x1c;
    const std::uint8_t* fb = frames + frame_b * 0x1c;
    struct Corner {
        std::uint32_t u, v;
        float angle;
    };
    const double quarter = 0.7853981852531433;
    const Corner corners[4] = {{0x10, 0x16, static_cast<float>((angle - 3.1415927410125732) + quarter)},
                               {0x14, 0x16, static_cast<float>((angle - 1.5707963705062866) + quarter)},
                               {0x10, 0x12, static_cast<float>((1.5707963705062866 + angle) + quarter)},
                               {0x14, 0x12, static_cast<float>(angle + quarter)}};
    for (const Corner& c : corners) {
        wr<float>(vertex, 0x00, p[0]);
        wr<float>(vertex, 0x04, p[1]);
        wr<float>(vertex, 0x08, p[2]);
        wr<float>(vertex, 0x0c, c.angle);
        wr<std::uint32_t>(vertex, 0x10, argb);
        wr<float>(vertex, 0x14, 1.0f);
        wr<float>(vertex, 0x18, 1.0f);
        wr<float>(vertex, 0x1c, 1.0f);
        wr<float>(vertex, 0x20, blend);
        wr<std::uint16_t>(vertex, 0x24, rd<std::uint16_t>(fa, c.u));
        wr<std::uint16_t>(vertex, 0x26, rd<std::uint16_t>(fa, c.v));
        wr<std::uint16_t>(vertex, 0x28, rd<std::uint16_t>(fb, c.u));
        wr<std::uint16_t>(vertex, 0x2a, rd<std::uint16_t>(fb, c.v));
        wr<float>(vertex, 0x2c, size);
        wr<float>(vertex, 0x30, angle);
        wr<std::uint16_t>(vertex, 0x34, 0);
        wr<std::uint16_t>(vertex, 0x36, flag_half);
        vertex += 0x38;
    }
    // B09FAB..B09FF5: advance the selected cursor, decrement its room.
    const std::uint32_t cursor = second_buffer ? 0x1d0 : 0x1c0;
    const std::uint32_t room = second_buffer ? 0x1cc : 0x1b8;
    wr<std::uint8_t*>(target, cursor, rd<std::uint8_t*>(target, cursor) + 0xe0);
    wr<std::int32_t>(target, room, rd<std::int32_t>(target, room) - 1);
    return 1;
}

float* axial_bounding_sphere_00b05dd0(const void* definition, float sphere[4], const void* state, float t,
                                      const float wind_e13028[3], ParticleCurves& curves,
                                      const CameraAxesCrtAccess& crt) {
    // B05DD0..B05E6C: Length (+8Ch) and Width (+90h) times size +58h.
    const float length = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x8c), t));
    const float width = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x90), t));

    // B05E70..B05FE8: the axis, or with FollowDirection (+80h) the normalised
    // curve velocity. Unlike 00B06B10, gravity here acts on y only (no matrix).
    float axis[3];
    if (rd<std::uint8_t>(definition, 0x80) == 0) {
        axis[0] = rd<float>(definition, 0x94);
        axis[1] = rd<float>(definition, 0x98);
        axis[2] = rd<float>(definition, 0x9c);
    } else {
        const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);  // [00D7A220]
        const float ks = static_cast<float>(evaluate(curves, ptr(definition, 0x2c), t) * scale);
        const float vx = rd<float>(state, 0x18) * ks;
        const float vy0 = rd<float>(state, 0x1c) * ks;
        const float vz = ks * rd<float>(state, 0x20);
        const float vy = static_cast<float>(evaluate(curves, ptr(definition, 0x30), t) * scale *
                                            rd<float>(state, 0x48) + vy0);
        const float ws = static_cast<float>(evaluate(curves, ptr(definition, 0x34), t) * scale *
                                            rd<float>(state, 0x4c));
        const float velocity[3] = {ws * wind_e13028[0] + vx, ws * wind_e13028[1] + vy, ws * wind_e13028[2] + vz};
        camera_vector_normalize_00419510(axis, velocity, &crt);
    }

    // B05FE8..B061BF: Alignment +A0h moves the centre half a quad along the
    // axis (1 Bottom: +, 2 Top: -, otherwise the position); the radius is
    // sqrt(L*L + W*W) through 00BF7030, each product spilled to float.
    const std::int32_t alignment = rd<std::int32_t>(definition, 0xa0);
    const float sign = alignment == 1 ? 1.0f : alignment == 2 ? -1.0f : 0.0f;
    if (sign == 0.0f) {
        sphere[0] = rd<float>(state, 0x00);
        sphere[1] = rd<float>(state, 0x04);
        sphere[2] = rd<float>(state, 0x08);
    } else {
        sphere[0] = rd<float>(state, 0x00) + sign * (length * axis[0]);
        sphere[1] = rd<float>(state, 0x04) + sign * (axis[1] * length);
        sphere[2] = rd<float>(state, 0x08) + sign * (axis[2] * length);
    }
    const float ll = length * length;
    const float ww = width * width;
    sphere[3] = static_cast<float>(std::sqrt(static_cast<double>(ll + ww)));
    return sphere;
}

float* floating_bounding_sphere_00b07bf0(const void* definition, float sphere[4], const void* state, float t,
                                         ParticleCurves& curves) {
    sphere[0] = rd<float>(state, 0x00);
    sphere[1] = rd<float>(state, 0x04);
    sphere[2] = rd<float>(state, 0x08);
    sphere[3] = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x88), t) *
                                   0.5);  // [00D7A280]
    return sphere;
}

float* sprite_bounding_sphere_00b08950(const void* definition, float sphere[4], const void* state, float t,
                                       ParticleCurves& curves) {
    // Same body as 00B07BF0 byte for byte except the rel32 displacements of the
    // two curve calls, which reach the same 00AFFA70/00AFFAE0.
    return floating_bounding_sphere_00b07bf0(definition, sphere, state, t, curves);
}

float* object_bounding_sphere_00af82b0(const void* definition, float sphere[4], const void* state, float t,
                                       ParticleCurves& curves) {
    sphere[0] = rd<float>(state, 0x00);
    sphere[1] = rd<float>(state, 0x04);
    sphere[2] = rd<float>(state, 0x08);
    sphere[3] = static_cast<float>(rd<float>(state, 0x58) * evaluate(curves, ptr(definition, 0x84), t) *
                                   0.5);  // [00D7A280]
    return sphere;
}

float* tracer_bounding_sphere_00b0a3f0(float sphere[4], const void* state) {
    sphere[0] = rd<float>(state, 0x00);
    sphere[1] = rd<float>(state, 0x04);
    sphere[2] = rd<float>(state, 0x08);
    sphere[3] = 100.0f;  // [00CE3D08]
    return sphere;
}

CameraMatrix& compose_object_rotation_00af8500(CameraMatrix& matrix, CameraMatrix& destination, float angle) {
    // AF8500..AF8585: transpose in place (the state's matrix itself).
    std::swap(matrix[1], matrix[4]);
    std::swap(matrix[2], matrix[8]);
    std::swap(matrix[3], matrix[12]);
    std::swap(matrix[6], matrix[9]);
    std::swap(matrix[7], matrix[13]);
    std::swap(matrix[11], matrix[14]);
    // AF858A..AF859E: destination = (M' * Rz(angle)) * M'.
    CameraMatrix rotation;
    build_gui_rotation_z_00b64780(rotation, angle);
    CameraMatrix temporary;
    multiply_camera_matrices_00413920(temporary, matrix, rotation);
    multiply_camera_matrices_00413920(destination, temporary, matrix);
    return destination;
}

std::int32_t emit_object_00af85b0(const void* definition, void* state, float t, const float matrix[16],
                                  const float wind_e13028[3], ParticleCurves& curves, ObjectParticleHost& host) {
    // AF85B5..AF85BF: nothing without the model instance at state+34h.
    void* instance = rd<void*>(state, 0x34);
    if (instance == nullptr) {
        return 0;
    }
    const float scale = static_cast<float>(rd<float>(state, 0x44) / 100.0);  // [00D7A220]
    // AF85DF..AF863A: angle with RotationSpeed at def+80h (Object layout).
    const float angle = static_cast<float>(
        rd<float>(state, 0x54) * (integrate(curves, ptr(definition, 0x80), t) * 0.017453288659453392) * scale +
        rd<float>(state, 0x50));  // [00D5DAF8]
    float p[3];
    full_position(definition, state, t, matrix, wind_e13028, curves, scale, p);
    // AF88B4..AF88DE: node = instance+0Ch; node vtable +38h with the composed
    // matrix, then +2Ch with the state (its position xyz at +0).
    void* node = rd<void*>(instance, 0x0c);
    CameraMatrix composed;
    compose_object_rotation_00af8500(*rd<CameraMatrix*>(state, 0x30), composed, angle);
    host.node_set_local_matrix_slot38(node, composed);
    host.node_set_local_position_slot2c(node, static_cast<const float*>(state));
    return 0;
}

void submit_particle_model_render_00af73a0(void* model, void* context, ParticleModelRenderHost& host) {
    // AF73A7..AF73E3: pass gates on camera+198h and the system (+18Ch) flags.
    void* camera = rd<void*>(context, 0x08);
    const std::int32_t pass = rd<std::int32_t>(camera, 0x198);
    const void* system = ptr(model, 0x18c);
    if (pass == 2) return;
    if (pass == 1 && rd<std::uint8_t>(system, 0x7a) == 0) return;
    if (rd<std::uint8_t>(system, 0x70) == 1 && pass != 3) return;
    // AF73E9..AF73F2: ModelEffectOptions +5, "hide particles".
    if (host.hide_particles_option_0051f6b0()) return;
    // AF73F8..AF7412: world sphere (vtable +48h), classified against the camera.
    const std::uint32_t classification = host.classify_sphere_00b71530(camera, host.model_world_sphere_slot48(model));
    if (classification == 0xaaa) return;  // outside
    // AF7414..AF7424: fade +1D8h must exceed 0.0035 [00CF6548] (unordered skips).
    if (!(static_cast<double>(rd<float>(model, 0x1d8)) > 0.0035000001080334187)) return;
    // AF7426..AF7455: render entries of mesh +1B4h when its element 0 has +10h.
    void* mesh = rd<void*>(model, 0x1b4);
    if (rd<std::uint32_t>(host.mesh_element_00b732c0(mesh, 0), 0x10) != 0) {
        host.create_render_entries_00b72f80(mesh, context, model, 1.0f, 1365.0f /*[00D5DAAC]*/, 0);
    }
    // AF745A..AF7485: debug "show boundings" (0051F520 +4): [00F8D394] vtable
    // +C8h(sphere, camera, classification).
    if (host.show_boundings_option_0051f520()) {
        host.debug_draw_sphere_slot_c8(host.model_world_sphere_slot48(model), camera, classification);
    }
}

float particle_system_lod_fade_00af4130(const void* system, float x, float y, float z, const CameraMatrix& view_projection,
                                        float camera_fov, float& distance_out, const CameraAxesCrtAccess& crt) {
    const float near_distance = rd<float>(system, 0x68);
    const float far_distance = rd<float>(system, 0x6c);
    // AF4138..AF4192: sphere {position, +8Ch + +88h} through the view-projection.
    const float radius = rd<float>(system, 0x8c) + rd<float>(system, 0x88);
    std::array<float, 4> sphere{x, y, z, radius};
    transform_sphere_007c1180(sphere, view_projection, sphere);
    // AF4197..AF4228: FOV-normalised distance, d = |c| * (1/tan(20 deg)) / (1/tan(fov/2)).
    auto bits = [](float v) {
        std::uint32_t b;
        std::memcpy(&b, &v, sizeof b);
        return b;
    };
    const float k1 = static_cast<float>(1.0 / native_projection_tangent_00412e20(bits(0.349065781f)));  // [00D5D954]
    const double d1 = static_cast<double>(camera_vector_length_00419440(sphere.data(), &crt)) * k1;  // FSTP qword
    const float half_fov = static_cast<float>(camera_fov * 0.5);  // [00D7A280]
    const float k2 = static_cast<float>(1.0 / native_projection_tangent_00412e20(bits(half_fov)));
    const float d = static_cast<float>(d1 / k2);
    distance_out = d;  // FST [out]
    // AF4237..AF427C: 0 at or beyond far (+6Ch), 1 at or inside near (+68h),
    // otherwise 1 - (d - near) / (far - near). JB continues on unordered.
    if (!(d < far_distance) && !std::isnan(d) && !std::isnan(far_distance)) return 0.0f;
    if (!(near_distance < d) && !std::isnan(d) && !std::isnan(near_distance)) return 1.0f;
    const float q = static_cast<float>((static_cast<double>(d) - near_distance) /
                                       (static_cast<double>(far_distance) - near_distance));
    return static_cast<float>(1.0 - q);
}

float particle_model_lod_fade_00af6300(void* model, void* camera, ParticleModelLodHost& host,
                                       const CameraAxesCrtAccess& crt) {
    // AF6313..AF631D: refresh the world matrix unless +5Ch bit 1 is set.
    if ((rd<std::uint8_t>(model, 0x5c) & 2) == 0) {
        host.refresh_world_00b6db70(model);
    }
    float distance = 0.0f;
    const float fade = particle_system_lod_fade_00af4130(
        ptr(model, 0x18c), rd<float>(model, 0x120), rd<float>(model, 0x124), rd<float>(model, 0x128),
        host.view_projection_00b70490(camera), rd<float>(camera, 0x1c4), distance, crt);
    wr<float>(model, 0x1e0, distance);
    return fade;
}

bool particle_model_update_visibility_00af6360(void* model, void* camera, ParticleModelLodHost& host,
                                               const CameraAxesCrtAccess& crt) {
    const float visibility =
        static_cast<float>(particle_model_lod_fade_00af6300(model, camera, host, crt) * rd<float>(model, 0x1dc));
    wr<float>(model, 0x1d8, visibility);
    return static_cast<double>(visibility) > 0.0035000001080334187;  // [00CF6548]
}

float emitter_distance_fade_00afaa20(const void* emitter_definition, float x) {
    const float start = rd<float>(emitter_definition, 0x30);
    if (x <= start) return 0.0f;
    const float fade_out = rd<float>(emitter_definition, 0x3c);
    const float hold_end = rd<float>(emitter_definition, 0x38);
    const float end = fade_out + hold_end;  // spilled
    if (end <= x) return 0.0f;
    const float fade_in = rd<float>(emitter_definition, 0x34);
    const float full = fade_in + start;  // spilled
    if (full <= x && x <= hold_end) return 1.0f;
    if (start < x && x < full) {
        if (fade_in <= 0.0f) return 0.0f;  // [00D7A218]
        const float rise = x - start;
        return rise / fade_in;
    }
    if (hold_end < x && x < end && 0.0f < fade_out) {
        const float fall = x - hold_end;
        float v = 1.0f - fall / fade_out;
        if (v < 0.0f) v = 0.0f;  // 004155B0 BSP_Float_ClampByPointer(v, 0, 1.0 [00D7A24C])
        if (v > 1.0f) v = 1.0f;
        return v;
    }
    return 0.0f;
}

std::int32_t emitter_update_00aff7a0(void* emitter, void* context, float distance, ParticleEmitterHost& host) {
    void* container = rd<void*>(emitter, 0x10);
    if (container == nullptr) return 0;
    const float fade = emitter_distance_fade_00afaa20(ptr(emitter, 0x0c), distance);
    wr<float>(emitter, 0x24, fade);
    if (0.004f > fade) return 0;  // [00CEC178], JA: unordered continues
    host.container_update_slot0(container, context);
    return rd<std::int32_t>(container, 0x28);
}

std::int32_t emitter_build_geometry_00aff7f0(void* emitter, const float view[16], float distance, float argument,
                                             ParticleGeometryHost& host) {
    void* container = rd<void*>(emitter, 0x10);
    if (container == nullptr) return 0;
    const float model_visibility = rd<float>(ptr(emitter, 0x08), 0x1d8);
    const float fade = emitter_distance_fade_00afaa20(ptr(emitter, 0x0c), distance);
    const float alpha = model_visibility > fade ? fade : model_visibility;  // JBE keeps +1D8h on unordered
    wr<float>(emitter, 0x24, alpha);
    if (0.004f > alpha) return 0;
    build_container_geometry_00b05550(container, view, argument, host);
    return rd<std::int32_t>(container, 0x28);
}

void particle_model_update_emitters_00af5ee0(void* model, void* context, ParticleEmitterHost& host) {
    // The count +198h and the array +194h are re-read on every iteration.
    for (std::int32_t i = 0; i < rd<std::int32_t>(model, 0x198); ++i) {
        void* emitter = rd<void* const*>(model, 0x194)[i];
        emitter_update_00aff7a0(emitter, context, rd<float>(model, 0x1e0), host);
    }
}

void particle_model_build_geometry_00af5e90(void* model, const float view[16], float argument,
                                            ParticleGeometryHost& host) {
    // The count +198h and the array +194h are re-read on every iteration.
    for (std::int32_t i = 0; i < rd<std::int32_t>(model, 0x198); ++i) {
        void* emitter = rd<void* const*>(model, 0x194)[i];
        emitter_build_geometry_00aff7f0(emitter, view, rd<float>(model, 0x1e0), argument, host);
    }
}

void particle_model_build_frame_geometry_00af63a0(void* model, void* camera, ParticleModelFrameHost& host,
                                                  const CameraAxesCrtAccess& crt) {
    // AF63BE..AF63E7: visibility (00AF6360's store, without its compare) and cursor reset.
    wr<float>(model, 0x1d8, static_cast<float>(particle_model_lod_fade_00af6300(model, camera, host, crt) *
                                               rd<float>(model, 0x1dc)));
    wr<void*>(model, 0x1c0, nullptr);
    wr<void*>(model, 0x1d0, nullptr);
    const std::int32_t capacity_a = rd<std::int32_t>(model, 0x1f4);
    const std::int32_t capacity_b = rd<std::int32_t>(model, 0x1f0);
    void* mesh_a = rd<void*>(model, 0x1b4);
    void* mesh_b = rd<void*>(model, 0x1c8);
    // AF63ED..AF652A: lock the vertex streams (4 vertices per quad) under the
    // device lock; B is also handed to the renderer [00F8D39C] through 00B0D170.
    if (capacity_a != 0 || capacity_b != 0) {
        host.device_lock_enter();
        if (capacity_a != 0) {
            wr<void*>(model, 0x1c0, host.lock_vertex_stream(mesh_a, capacity_a * 4));
        }
        if (capacity_b != 0) {
            wr<void*>(model, 0x1d0, host.lock_vertex_stream(mesh_b, capacity_b * 4));
            host.renderer_attach_00b0d170(model, mesh_b);
        }
        host.device_lock_leave();
    }
    // AF652A..AF6569: the floating cursor (shared scratch [00F8C274]+28h) and the room counts.
    const std::int32_t capacity_c = rd<std::int32_t>(model, 0x1f8);
    wr<void*>(model, 0x1c4, capacity_c != 0 ? host.scratch_cursor_00f8c274_28() : nullptr);
    wr<std::int32_t>(model, 0x1cc, rd<std::int32_t>(model, 0x1f0));
    wr<std::int32_t>(model, 0x1b8, rd<std::int32_t>(model, 0x1f4));
    wr<std::int32_t>(model, 0x1bc, capacity_c);
    // AF654B..AF65E8: view = camera view (00B6FCB0), or model world (+F0h) *
    // view when +1B0h is set; argument = float(1 / camera+1C4h) (Sprite's
    // view_size_scale). Then the emitters build their geometry.
    const float inverse_fov = static_cast<float>(1.0 / rd<float>(camera, 0x1c4));
    CameraMatrix view;
    if (rd<std::uint8_t>(model, 0x1b0) == 0) {
        view = host.camera_view_00b6fcb0(camera);
    } else {
        if ((rd<std::uint8_t>(model, 0x5c) & 2) == 0) {
            host.refresh_world_00b6db70(model);
        }
        CameraMatrix world;
        std::memcpy(world.data(), static_cast<const std::uint8_t*>(model) + 0xf0, sizeof world);
        multiply_camera_matrices_00413920(view, world, host.camera_view_00b6fcb0(camera));
    }
    particle_model_build_geometry_00af5e90(model, view.data(), inverse_fov, host);
    wr<std::uint8_t>(model, 0x1d4, 1);
    // AF65ED..AF66DE: unlock the streams under the device lock.
    if (capacity_a != 0 || capacity_b != 0) {
        host.device_lock_enter();
        if (capacity_a != 0) host.unlock_vertex_stream(mesh_a);
        if (capacity_b != 0) host.unlock_vertex_stream(mesh_b);
        host.device_lock_leave();
    }
    // AF66E0..AF6779: used counts. +1F8h is overwritten with the floating count
    // used and flushed through 00AF0540 ([00F8C274], bytes = used * 4); each
    // mesh's element 0 gets vertices (+10h) = used * 4 and primitives (+18h) = used * 2.
    const std::int32_t used_a = rd<std::int32_t>(model, 0x1f4) - rd<std::int32_t>(model, 0x1b8);
    const std::int32_t used_b = rd<std::int32_t>(model, 0x1f0) - rd<std::int32_t>(model, 0x1cc);
    wr<std::int32_t>(model, 0x1f8, rd<std::int32_t>(model, 0x1f8) - rd<std::int32_t>(model, 0x1bc));
    if (rd<std::int32_t>(model, 0x1f8) != 0) {
        host.flush_scratch_00af0540(rd<std::int32_t>(model, 0x1f8) * 4);
    }
    wr<std::int32_t>(host.mesh_element_00b732c0(mesh_a, 0), 0x10, used_a * 4);
    wr<std::int32_t>(host.mesh_element_00b732c0(mesh_a, 0), 0x18, used_a * 2);
    wr<std::int32_t>(host.mesh_element_00b732c0(mesh_b, 0), 0x10, used_b * 4);
    wr<std::int32_t>(host.mesh_element_00b732c0(mesh_b, 0), 0x18, used_b * 2);
}

bool particle_system_sphere_visible_00af3f80(const void* system, float x, float y, float z, void* camera,
                                             ParticleModelRenderHost& host) {
    const float sphere[4] = {x, y, z, rd<float>(system, 0x8c) + rd<float>(system, 0x88)};
    return host.classify_sphere_00b71530(camera, sphere) != 0xaaa;
}

void particle_model_set_emitter_value_00af6790(void* model, const char* name, float value) {
    wr<float>(model, 0x2d8, value);
    const void* system = ptr(model, 0x190);
    // AF67AB..AF6813: the 108h records at system+0Ch, count system+10h (re-read per pass).
    for (std::int32_t i = 0; i < rd<std::int32_t>(system, 0x10); ++i) {
        auto* record = rd<std::uint8_t*>(system, 0x0c) + i * 0x108;
        bool match = name == nullptr;
        if (!match) {
            const char* own = rd<const char*>(ptr(record, 0xa0), 0x0c);
            match = own == nullptr ? name[0] == '\0' : _stricmp(own, name) == 0;  // 00BF7FBF
        }
        if (match) {
            wr<float>(record, 0xb4, value);
        }
    }
}

void particle_emitter_spawn_00aff700(void* emitter, void* a, void* b, float c, float d, float e,
                                     ParticleEmitterHost& host) {
    // AFF71D..AFF782: on first use operator new(30h) (00BF681B) and 00B053D0
    // BSP_NativeParticleEmitterContainer_Construct(model, emitter, def+20h); then
    // 00B04C80 BSP_NativeParticleEmission_Spawn(container, a, b, c, d, e).
    if (rd<void*>(emitter, 0x10) == nullptr) {
        wr<void*>(emitter, 0x10,
                  host.construct_container(rd<void*>(emitter, 0x08), emitter, rd<std::uint32_t>(ptr(emitter, 0x0c), 0x20)));
    }
    host.spawn(rd<void*>(emitter, 0x10), a, b, c, d, e);
}

void release_axial_state_00b00750(volatile std::int32_t& axial_count_00f8d380) {
    axial_count_00f8d380 = axial_count_00f8d380 - 1;
}

void release_floating_state_00b007a0(volatile std::int32_t& floating_count_00f8d384) {
    floating_count_00f8d384 = floating_count_00f8d384 - 1;
}

}  // namespace bsp::particle_emitters
