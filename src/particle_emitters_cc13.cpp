// Particle geometry pass and quad emitters, segment 82 (packet cc13_water2
// particles). Addresses 00B057C0, 00B06A90, 00B0C8B0, 00B05420, 00B05550 and
// 00B06B10. Evidence, ABI and uncertainty: include/bsp/particle_emitters_cc13.hpp
// and docs/CC13_PARTICLE_EMITTERS.md.

#include "bsp/particle_emitters_cc13.hpp"

#include <cmath>
#include <cstring>

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

}  // namespace bsp::particle_emitters
