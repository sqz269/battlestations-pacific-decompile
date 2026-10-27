// The ocean spectrum and its per-frame step. Packet cc9_physics1_sea,
// docs/OCEAN_WAVE_FIELD.md sections 8 and 9; addresses in bsp/ocean_spectrum.hpp.

#include "bsp/ocean_spectrum.hpp"

#include <cmath>
#include <cstddef>
#include <string>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/pc24_flightrender_cc13.hpp"
#include "bsp/scene_file.hpp"

// pc24w1-sweep1-3, kPc24FlightRenderCc13Bound (OFF): ON = per-op PC24 (x87_*24) in the
// spectrum leaves the main thread runs at 24-bit precision control; OFF keeps each double
// chain rounded once at the float store. Sites: 00BF0DF0 s and ku * sigma + mean; 00B94CA0
// arg, dot and the A * E * D / k^4 tail; 00B95750 l2; 00B952D0 butterflies and twiddle
// step; 00B95D30 the h(k, t) complex step. Movers: the ocean wave heights (h0 at load and
// h(t) per frame), so wakes, the ocean height samplers and everything floating on them.

namespace bsp {
namespace {

constexpr double kTwoToMinus32 = 2.3283064365386963e-10;  // 00D63B80
constexpr double kTwoTo32 = 4294967296.0;                  // 00CE3978
constexpr double kOceanClockScale = 10.0;                  // 00CE3DC0
constexpr double kWindDirectionScale = 3.1415927410125732; // 00CE3D28, 0078C9EF

float f32(double value) noexcept { return static_cast<float>(value); }

// The 0..63 index of wave number -32..31 (AND 3Fh at 00B95DAC / 00B95E47).
int wrap64(int n) noexcept { return n & (kOceanGridSide - 1); }

// FYL2X / F2XM1 / FSCALE: base^power for base > 0.
double x87_pow(double base, double power) noexcept { return std::pow(base, power); }

// 00B94DA5..00B94DFB and 00B94E01..00B94E61: |value|^power, 0 when value == 0.
double signed_pow_or_zero(float value, float power) noexcept {
    if (value == 0.0f) return 0.0;
    const float base = value < 0.0f ? -0.0f - value : value;
    return x87_pow(base, power);
}

}  // namespace

float ocean_inv_sqrt2_0109048c() noexcept {
    const float root = f32(std::sqrt(2.0));     // 00CD8877, FSTP
    return f32(1.0 / static_cast<double>(root)); // 00CD8884 FDIVRP, 00CD8886
}

float ocean_normal_draw_00bf0df0(RandomState& state, float mean, float sigma) {
    float u1 = 0.0f;
    float u2 = 0.0f;
    float s = 1.0f;
    do {
        // 00BF0E8D: FILD of the tempered word, +2^32 when negative, times 2^-32.
        const float a = f32(static_cast<double>(random_next_u32_00ba2c20(state)) * kTwoToMinus32);
        u1 = f32(static_cast<double>(a) * 2.0 - 1.0);                  // 00BF0EAC..00BF0EB0
        const float b = f32(static_cast<double>(random_next_u32_00ba2c20(state)) * kTwoToMinus32);
        u2 = f32(static_cast<double>(b) * 2.0 - 1.0);                  // 00BF0F30..00BF0F34
        if constexpr (kPc24FlightRenderCc13Bound) {
            // 00BF0F40 FMUL ST0, 00BF0F44 FMULP, 00BF0F46 FADDP, each rounded at PC24.
            s = f32(cc13x87::x87_add24(cc13x87::x87_mul24(u2, u2), cc13x87::x87_mul24(u1, u1)));
        } else {
        s = f32(static_cast<double>(u1) * u1 + static_cast<double>(u2) * u2);  // 00BF0F48
        }
    } while (s >= 1.0f);                                                // 00BF0F50 FCOMI, JAE
    static_cast<void>(kTwoTo32);
    const float log_s = f32(std::log(static_cast<double>(s)));         // 00BF0F60, 00BF0F65
    const float ratio = f32(static_cast<double>(log_s) * -2.0 / s);     // 00BF0F6D, 00BF0F73
    const float k = f32(std::sqrt(static_cast<double>(ratio)));         // 00BF0F7F
    const float ku = f32(static_cast<double>(k) * u1);                  // 00BF0F8D
    float out;
    if constexpr (kPc24FlightRenderCc13Bound) {
        out = f32(cc13x87::x87_add24(cc13x87::x87_mul24(ku, sigma), mean));  // 00BF0F99, 00BF0F9D
    } else {
    out = f32(static_cast<double>(ku) * sigma + mean);                  // 00BF0F99, 00BF0F9D
    }
    return out;
}

float ocean_phillips_00b94ca0(const OceanSpectrum& sp, float kx, float ky) {
    const float ky2 = f32(static_cast<double>(ky) * ky);                // 00B94CB0
    const float kx2 = f32(static_cast<double>(kx) * kx);                // 00B94CBE
    const float sum = f32(static_cast<double>(ky2) + kx2);              // 00B94CC6
    float k = f32(std::sqrt(static_cast<double>(sum)));                 // 00B94CCE
    if (kOceanMinWaveNumber > k) k = kOceanMinWaveNumber;               // 00B94CE9
    const float inv_k = f32(1.0 / static_cast<double>(k));              // 00B94D07
    const float kL = f32(static_cast<double>(sp.largest_wave_78) * k);  // 00B94D10
    const float kl = f32(static_cast<double>(k) * sp.smallest_wave_7c); // 00B94D17
    const float kL2 = f32(static_cast<double>(kL) * kL);                // 00B94D29
    const double neg_inv = -1.0 / static_cast<double>(kL2);             // 00B94D31 FDIVR -1.0
    const float kl2 = f32(static_cast<double>(kl) * kl);                // 00B94D3D
    float arg;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // The 00B94D31 quotient stays on the stack, rounded at PC24, into the 00B94D41 FSUB.
        arg = f32(cc13x87::x87_sub24(cc13x87::x87_div24(-1.0, kL2), kl2));
    } else {
    arg = f32(neg_inv - kl2);                                           // 00B94D45
    }
    const float e = f32(std::exp(static_cast<double>(arg)));            // 00B94D4D, 00B94D52
    const float kxn = f32(static_cast<double>(inv_k) * kx);             // 00B94D81
    const float kyn = f32(static_cast<double>(inv_k) * ky);             // 00B94D94
    float dot;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // 00B94D89 FMUL +6Ch, 00B94D9C FMUL +70h, 00B94D9F FADDP, each rounded at PC24.
        dot = f32(cc13x87::x87_add24(cc13x87::x87_mul24(kxn, sp.wind_cos_6c),
                                     cc13x87::x87_mul24(kyn, sp.wind_sin_70)));
    } else {
    dot = f32(static_cast<double>(kxn) * sp.wind_cos_6c +
              static_cast<double>(kyn) * sp.wind_sin_70);               // 00B94DA1
    }
    const float directional = f32(signed_pow_or_zero(dot, sp.direction_power_84));
    const float k4 = f32(signed_pow_or_zero(k, 4.0f));                  // 00B94E57
    float out;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // 00B94E6E FMUL e, 00B94E72 FMUL D, 00B94E76 FDIV k^4, each rounded at PC24.
        out = f32(cc13x87::x87_div24(
            cc13x87::x87_mul24(cc13x87::x87_mul24(sp.wave_height_80, e), directional), k4));
    } else {
    // 00B94E67..00B94E7A: A * E * D / k^4, rounded once.
    out = f32(static_cast<double>(sp.wave_height_80) * e * directional / k4);
    }
    return out;
}

void ocean_spectrum_build_00b95750(OceanSpectrum& sp) {
    float l2;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // The product, then 00B95771 FDIV by the qword gravity, each rounded at PC24.
        l2 = f32(cc13x87::x87_div24(cc13x87::x87_mul24(sp.wind_speed_74, sp.wind_speed_74),
                                    kOceanGravity));
    } else {
    l2 = f32(static_cast<double>(sp.wind_speed_74) * sp.wind_speed_74 / kOceanGravity);
    }
    sp.largest_wave_78 = l2;                                                       // 00B9578F
    sp.smallest_wave_7c = f32(static_cast<double>(l2) * sp.small_wave_factor_8c);  // 00B95798

    RandomState state{};
    random_seed_00bf0cf0(state, kOceanSpectrumSeed);                               // 00B9579B
    const float inv_sqrt2 = ocean_inv_sqrt2_0109048c();

    sp.h0_10.assign(static_cast<std::size_t>(kOceanSpectrumH0Side * kOceanSpectrumH0Side), {});
    std::size_t cell = 0;
    for (int m = -kOceanSpectrumHalf; m <= kOceanSpectrumHalf; ++m) {
        const float ky = f32(m * kOceanWaveNumberStep);                            // 00B957C3
        for (int n = -kOceanSpectrumHalf; n <= kOceanSpectrumHalf; ++n) {
            const float g1 = ocean_normal_draw_00bf0df0(state, 0.0f, 1.0f);         // 00B957D9
            const float g2 = ocean_normal_draw_00bf0df0(state, 0.0f, 1.0f);         // 00B957F4
            const float kx = f32(n * kOceanWaveNumberStep);                        // 00B95814
            const float p = ocean_phillips_00b94ca0(sp, kx, ky);                   // 00B9581F
            const float root = f32(std::sqrt(static_cast<double>(p)));             // 00B95824
            const float scale = f32(static_cast<double>(inv_sqrt2) * root);        // 00B95844
            sp.h0_10[cell].re = f32(static_cast<double>(scale) * g2);              // 00B95856
            sp.h0_10[cell].im = f32(static_cast<double>(scale) * g1);              // 00B9585E
            ++cell;
        }
    }

    const std::size_t grid = static_cast<std::size_t>(kOceanGridSide * kOceanGridSide);
    sp.inv_k_60.assign(grid, 0.0f);
    sp.omega_50.assign(grid, 0.0f);
    std::size_t i = 0;
    for (int m = -kOceanSpectrumHalf; m < kOceanSpectrumHalf; ++m) {
        const float a = f32(m * kOceanWaveNumberStep);
        const float a2 = f32(static_cast<double>(a) * a);                          // 00B958DD
        for (int n = -kOceanSpectrumHalf; n < kOceanSpectrumHalf; ++n, ++i) {
            const float b = f32(n * kOceanWaveNumberStep);
            const float b2 = f32(static_cast<double>(b) * b);                      // 00B958F5
            const float sum = f32(static_cast<double>(b2) + a2);                   // 00B95901
            float k = f32(std::sqrt(static_cast<double>(sum)));                    // 00B9590E
            if (kOceanMinWaveNumber > k) k = kOceanMinWaveNumber;                  // 00B95924
            sp.inv_k_60[i] = f32(1.0 / static_cast<double>(k));                    // 00B95959
            const float gk = f32(static_cast<double>(f32(std::sqrt(static_cast<double>(sum)))) *
                                 kOceanGravity);                                   // 00B95A06
            sp.omega_50[i] = f32(std::sqrt(static_cast<double>(gk)));              // 00B95A14
        }
    }
    // 00B95A8E: vtable+10h, 00B96E60, a bare RET.
}

void ocean_spectrum_set_all_00b96b90(OceanSpectrum& sp, float direction, float speed,
                                     float height, float power, float choppy, float small_factor) {
    sp.wind_cos_6c = f32(std::cos(static_cast<double>(direction)));  // 00B96B95 FCOS
    sp.wind_sin_70 = f32(std::sin(static_cast<double>(direction)));  // 00B96BA8 FSIN
    sp.wind_speed_74 = speed;
    sp.wave_height_80 = height;
    sp.direction_power_84 = power;
    sp.choppy_88 = choppy;
    sp.small_wave_factor_8c = small_factor;
    ocean_spectrum_build_00b95750(sp);                               // 00B96BFA
}

void ocean_spectrum_set_wind_00b96c10(OceanSpectrum& sp, float direction, float speed) {
    sp.wind_cos_6c = f32(std::cos(static_cast<double>(direction)));  // 00B96C15
    sp.wind_sin_70 = f32(std::sin(static_cast<double>(direction)));  // 00B96C28
    sp.wind_speed_74 = speed;                                        // 00B96C3D
    ocean_spectrum_build_00b95750(sp);                               // 00B96C42
}

void ocean_spectrum_set_height_00b96c50(OceanSpectrum& sp, float height, float choppy) {
    sp.wave_height_80 = height;                                      // 00B96C56
    sp.choppy_88 = choppy;                                           // 00B96C64
    ocean_spectrum_build_00b95750(sp);                               // 00B96C6C
}

void ocean_spectrum_construct_00b97100(OceanSpectrum& sp) {
    sp = OceanSpectrum{};
    sp.h_20.assign(static_cast<std::size_t>(kOceanGridSide * kOceanGridSide), {});
    sp.direction_power_84 = 0.5f;                                    // 00CE3800
    // 00B97295..00B972DA: 00D638CC..00D638E0.
    ocean_spectrum_set_all_00b96b90(sp, 1.570796012878418f, 32.0f, 60.0f, 2.0f,
                                    0.019999999552965164f, 4.999999873689376e-05f);
}

void ocean_fft64_00b952d0(int dir, float* x, float* y) {
    constexpr int n = kOceanGridSide;
    constexpr int m = 6;                                             // 00B95367
    // 00B952E7..00B95343: bit reversal.
    int j = 0;
    for (int i = 0; i < n - 1; ++i) {
        if (i < j) {
            const float tx = x[i];
            x[i] = x[j];
            x[j] = tx;
            const float ty = y[i];
            y[i] = y[j];
            y[j] = ty;
        }
        int k = n >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    const float sign = static_cast<float>(-dir);                     // 00B952E3 NEG, CVTSI2SS
    const float inv_sqrt2 = ocean_inv_sqrt2_0109048c();
    float c1 = -1.0f;                                                // 00D7A260
    float c2 = 0.0f;
    int l2 = 1;
    for (int l = 0; l < m; ++l) {
        const int l1 = l2;
        l2 <<= 1;
        float u1 = 1.0f;                                             // 00D638E4
        float u2 = 0.0f;
        for (int jj = 0; jj < l1; ++jj) {
            for (int i = jj; i < n; i += l2) {
                const int i1 = i + l1;
                float t1;
                float t2;
                if constexpr (kPc24FlightRenderCc13Bound) {
                    // 00B953DE..00B953E8 and 00B953F0..00B953F6: each product rounded
                    // at PC24 before the FSUBP / FADDP.
                    t1 = f32(cc13x87::x87_sub24(cc13x87::x87_mul24(u1, x[i1]),
                                                cc13x87::x87_mul24(u2, y[i1])));
                    t2 = f32(cc13x87::x87_add24(cc13x87::x87_mul24(u1, y[i1]),
                                                cc13x87::x87_mul24(u2, x[i1])));
                } else {
                t1 = f32(static_cast<double>(u1) * x[i1] - static_cast<double>(u2) * y[i1]);
                t2 = f32(static_cast<double>(u1) * y[i1] + static_cast<double>(u2) * x[i1]);
                }
                x[i1] = f32(static_cast<double>(x[i]) - t1);
                y[i1] = f32(static_cast<double>(y[i]) - t2);
                x[i] = f32(static_cast<double>(x[i]) + t1);
                y[i] = f32(static_cast<double>(y[i]) + t2);
            }
            float z;
            if constexpr (kPc24FlightRenderCc13Bound) {
                // 00B95443..00B95454 and 00B9545A..00B95464, products rounded first.
                z = f32(cc13x87::x87_sub24(cc13x87::x87_mul24(u1, c1),
                                           cc13x87::x87_mul24(u2, c2)));
                u2 = f32(cc13x87::x87_add24(cc13x87::x87_mul24(u1, c2),
                                            cc13x87::x87_mul24(u2, c1)));
            } else {
            z = f32(static_cast<double>(u1) * c1 - static_cast<double>(u2) * c2);
            u2 = f32(static_cast<double>(u1) * c2 + static_cast<double>(u2) * c1);
            }
            u1 = z;
        }
        // 00B95488..00B954E1.
        const float r2 = f32(std::sqrt(static_cast<double>(f32(1.0 - c1))));
        c2 = f32(static_cast<double>(f32(static_cast<double>(r2) * inv_sqrt2)) * sign);
        const float r1 = f32(std::sqrt(static_cast<double>(f32(static_cast<double>(c1) + 1.0))));
        c1 = f32(static_cast<double>(r1) * inv_sqrt2);
    }
    if (dir == 1) {                                                  // 00B954EB
        for (int i = 0; i < n; ++i) {
            x[i] = f32(x[i] / 64.0);
            y[i] = f32(y[i] / 64.0);
        }
    }
}

void ocean_fft2d_00b95aa0(int dir, std::vector<OceanComplex>& cells) {
    constexpr int n = kOceanGridSide;
    float re[n];
    float im[n];
    for (int row = 0; row < n * n; row += n) {                       // 00B95AC0..00B95BDE
        for (int i = 0; i < n; ++i) {
            re[i] = cells[static_cast<std::size_t>(row + i)].re;
            im[i] = cells[static_cast<std::size_t>(row + i)].im;
        }
        ocean_fft64_00b952d0(dir, re, im);
        for (int i = 0; i < n; ++i) {
            cells[static_cast<std::size_t>(row + i)] = {re[i], im[i]};
        }
    }
    for (int col = 0; col < n; ++col) {                              // 00B95BF0..00B95D12
        for (int i = 0; i < n; ++i) {
            re[i] = cells[static_cast<std::size_t>(col + i * n)].re;
            im[i] = cells[static_cast<std::size_t>(col + i * n)].im;
        }
        ocean_fft64_00b952d0(dir, re, im);
        for (int i = 0; i < n; ++i) {
            cells[static_cast<std::size_t>(col + i * n)] = {re[i], im[i]};
        }
    }
}

void ocean_spectrum_advance_00b95d30(OceanSpectrum& sp, float t) {
    // 00B95D33: the byte 00E130B4 is 1 on disk and has no writer.
    if (sp.h_20.size() != static_cast<std::size_t>(kOceanGridSide * kOceanGridSide)) {
        sp.h_20.assign(static_cast<std::size_t>(kOceanGridSide * kOceanGridSide), {});
    }
    constexpr int side = kOceanSpectrumH0Side;
    constexpr int last = side * side - 1;
    std::size_t w = 0;
    for (int m = -kOceanSpectrumHalf; m < kOceanSpectrumHalf; ++m) {
        for (int n = -kOceanSpectrumHalf; n < kOceanSpectrumHalf; ++n, ++w) {
            const float phase = f32(static_cast<double>(sp.omega_50[w]) * t);   // 00B95DE9
            const float c = f32(std::cos(static_cast<double>(phase)));           // 00B95DF1
            const float s = f32(std::sin(static_cast<double>(phase)));           // 00B95DFB
            const int idx = (m + kOceanSpectrumHalf) * side + (n + kOceanSpectrumHalf);
            const OceanComplex k0 = sp.h0_10[static_cast<std::size_t>(idx)];
            const OceanComplex km = sp.h0_10[static_cast<std::size_t>(last - idx)];
            // 00B95E65..00B95EA5: (a + b) c - (c2 + d) s and (d - c2) c + (b - a) s,
            // a/c2 = h0(-k).re/.im, b/d = h0(k).re/.im.
            const double a = km.re;
            const double b = k0.re;
            const double cm = km.im;
            const double d = k0.im;
            OceanComplex& out = sp.h_20[static_cast<std::size_t>(wrap64(m) * kOceanGridSide + wrap64(n))];
            if constexpr (kPc24FlightRenderCc13Bound) {
                // 00B95E71..00B95E91 and 00B95E97..00B95EA3: every sum, difference and
                // product rounded at PC24.
                out.re = f32(cc13x87::x87_sub24(cc13x87::x87_mul24(cc13x87::x87_add24(a, b), c),
                                                cc13x87::x87_mul24(cc13x87::x87_add24(cm, d), s)));
                out.im = f32(cc13x87::x87_add24(cc13x87::x87_mul24(cc13x87::x87_sub24(d, cm), c),
                                                cc13x87::x87_mul24(cc13x87::x87_sub24(b, a), s)));
            } else {
            out.re = f32((a + b) * c - (cm + d) * s);                            // 00B95E93
            out.im = f32((d - cm) * c + (b - a) * s);                            // 00B95EA5
            }
        }
    }
    ocean_fft2d_00b95aa0(-1, sp.h_20);                                           // 00B9609A
}

OceanWaveGridView ocean_spectrum_grid(const OceanSpectrum& sp) noexcept {
    OceanWaveGridView view;
    if (!sp.h_20.empty()) {
        view.cells = &sp.h_20.front().re;
        view.cell_count = sp.h_20.size();
    }
    return view;
}

OceanSceneWaves ocean_scene_waves_from_header(const ScenePropertyBlock& header) {
    OceanSceneWaves waves;
    const ScenePropertyBlock* ocean = nullptr;
    for (const auto& child : header.blocks) {
        if (child.first == "g_Ocean") ocean = &child.second;
    }
    if (ocean == nullptr) return waves;
    if (const SceneProperty* simple = ocean->find("0_SimpleOcean")) {
        if (simple->has_boolean) waves.simple_ocean = simple->boolean_value != 0;
    }
    const ScenePropertyBlock* block = nullptr;
    for (const auto& child : ocean->blocks) {
        if (child.first == "Waves") block = &child.second;
    }
    if (block == nullptr) return waves;
    waves.authored = true;
    const auto read = [block](const char* key, float& out) {
        const SceneProperty* p = block->find(key);
        if (p != nullptr && p->has_float) out = p->float_value;
    };
    read("Falloff", waves.falloff_2c);
    read("LayerScale", waves.layer_scale_30);
    read("TimeScale", waves.time_scale_34);
    read("WindDirection", waves.wind_direction_4c);
    read("WindSpeed", waves.wind_speed_50);
    read("WaveHeight", waves.wave_height_54);
    read("ChoppyWavesFactor", waves.choppy_58);
    read("HeightScale", waves.height_scale_5c);
    return waves;
}

void ocean_live_field_build(OceanLiveField& ocean, const OceanSceneWaves& waves) {
    ocean = OceanLiveField{};
    ocean_spectrum_construct_00b97100(ocean.spectrum);                           // 0078DB20
    ocean.field.flat_f9 = waves.simple_ocean;                                    // 004DF7E1
    ocean.field.amplitude_24 = 0.0f;                                             // 00BA19C9
    // 0078C9B0.
    ocean_spectrum_set_height_00b96c50(ocean.spectrum, waves.wave_height_54, waves.choppy_58);
    const float direction = f32(static_cast<double>(waves.wind_direction_4c) * kWindDirectionScale);
    ocean_spectrum_set_wind_00b96c10(ocean.spectrum, direction, waves.wind_speed_50);  // 00B9A46B
    ocean.height_scale_b8 = waves.height_scale_5c;                               // 0078CA0E
    ocean.time_scale_20 = waves.time_scale_34;                                   // 00B9A4E4
    ocean.field.inv_tile_b4 = f32(1.0 / static_cast<double>(waves.layer_scale_30));  // 00B9A4D2
}

void ocean_live_field_render_frame_00ba0090(OceanLiveField& ocean, float mission_clock) {
    const float t = f32(static_cast<double>(mission_clock) * kOceanStepTimeScale);   // 0078D184
    ocean.clock_01090494 = f32(static_cast<double>(t) * kOceanClockScale);          // 00BA00BA
    const float spectrum_time = f32(static_cast<double>(t) * ocean.time_scale_20);  // 00BA00C8
    ocean_spectrum_advance_00b95d30(ocean.spectrum, spectrum_time);                 // 00B96C94
    ocean.height_scale_2c = ocean.height_scale_b8;                                  // 00BA0B55
    ocean.wave_scale_28 = f32(static_cast<double>(ocean.height_scale_b8) * ocean.spectrum.choppy_88);
    ocean.field.amplitude_24 = ocean.height_scale_2c;                               // 00BA0B93
    ++ocean.render_steps;
}

namespace {
OceanLiveField g_ocean_live_field;
bool g_ocean_live_field_ready = false;
}  // namespace

OceanLiveField& ocean_live_field_process() { return g_ocean_live_field; }

bool ocean_live_field_ready() { return kWaveFieldLiveAmplitudeBound && g_ocean_live_field_ready; }

OceanSceneWaves ocean_live_field_publish_scene(const ScenePropertyBlock& header_properties) {
    const OceanSceneWaves waves = ocean_scene_waves_from_header(header_properties);
    if (!kWaveFieldLiveAmplitudeBound) return waves;
    ocean_live_field_build(g_ocean_live_field, waves);
    g_ocean_live_field_ready = true;
    return waves;
}

void ocean_live_field_render_job_0068a0d0(float mission_clock) {
    if (!ocean_live_field_ready()) return;
    ocean_live_field_render_frame_00ba0090(g_ocean_live_field, mission_clock);
}

std::string ocean_live_field_add_landscape_008830aa(const std::string& model_name,
    const float landscape_world[16], const OceanCoverageFileReader& read_file) {
    if (!ocean_live_field_ready()) return std::string();
    std::string model_bytes;
    if (!read_file(model_name, model_bytes)) model_bytes.clear();
    return ocean_coverage_add_landscape_00ba1370(g_ocean_live_field.coverage, model_bytes,
        landscape_world, read_file, 0);
}

float ocean_live_field_coverage_mask(float x, float z) {
    if (!ocean_live_field_ready()) return 1.0f;
    return ocean_coverage_mask_00b9cf50(g_ocean_live_field.coverage.regions, x, z);
}

float ocean_live_field_water_height_0078cf20(float x, float z) {
    if (!ocean_live_field_ready()) return 0.0f;  // the flat sea: 0.0 * 1.0
    struct LiveHost final : OceanHeightHost {
        float wave_height_0078c890(float u, float v) override {
            return ocean_wave_field_sample_0078c890(g_ocean_live_field.field,
                ocean_spectrum_grid(g_ocean_live_field.spectrum), u, v);
        }
        float coverage_mask_00b9cf50(float u, float v) override {
            return ocean_coverage_mask_00b9cf50(g_ocean_live_field.coverage.regions, u, v);
        }
    } host;
    return ocean_water_height_0078cf20(x, z, host);
}

}  // namespace bsp

namespace bsp {
__declspec(naked) void ocean_default_slot4_00b96e60() { __asm { ret } }
__declspec(naked) void ocean_default_slot5_00b96e70() { __asm { ret } }
__declspec(naked) void ocean_default_slot6_00b96e80() { __asm { ret } }
__declspec(naked) void ocean_default_slot7_00b96e90() { __asm { ret } }
} // namespace bsp
