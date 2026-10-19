#pragma once

// The ocean spectrum behind the CPU water height, and the per-frame step that makes
// it live. Packet cc9_physics1_sea, docs/OCEAN_WAVE_FIELD.md sections 8 and 9.
//
// Semantic C++ interfaces for MSVC Win32, not binary replacements; names are
// hypotheses. The x87 bodies keep float operands and round at the native float
// stores, with the extended intermediates modelled in double, so the grid is close
// to the image's but not bit-exact.
//
// The spectrum is a 90h-byte refcounted object (vtable 00D638F8) the world
// constructor allocates (0078DB09) and constructs (0078DB20 -> 00B97100), and
// 00B9DD90 attaches as field+BCh. Each in-mission render frame 00BA0090 steps it
// through vtable+8 (00B96C80) and stores the amplitude field+24h = field+2Ch =
// field+B8h (Waves.HeightScale, 00BA0B93). 0078C890 then samples the 64x64 grid at
// spectrum+20h (00B960C0) and multiplies by that amplitude.
//
// Coverage: the height path is complete. Not modelled: the choppy displacement
// vectors at +2Ch/+3Ch (written 00B95EBF..00B9604C, transformed at 00B960A7), whose
// only reader is 00BA2B50 for the beam effect; the field's slot 0 00B9D7B0 that the
// step calls (render grid tiles); the clock global [01090494].

#include <cstdint>
#include <vector>

#include "bsp/ocean_coverage_regions.hpp"
#include "bsp/ocean_wave_field.hpp"
#include "bsp/random.hpp"

namespace bsp {

struct ScenePropertyBlock;

struct OceanComplex {
    float re{0.0f};
    float im{0.0f};
};

inline constexpr int kOceanSpectrumHalf = 32;          // the -32..32 / -32..31 loops
inline constexpr int kOceanSpectrumH0Side = 0x41;      // 00B958A8 cmp edi,1040h; 41h x 41h
inline constexpr int kOceanGridSide = 0x40;            // 64 x 64, 00B95AA0's 40h stride
inline constexpr std::uint32_t kOceanSpectrumSeed = 0x1105u;  // 00B95760 push 1105h
inline constexpr double kOceanWaveNumberStep = 6.283184051513672;  // 00D638E8, qword
inline constexpr double kOceanGravity = 9.8100004196167;           // 00CF9058, qword
inline constexpr float kOceanMinWaveNumber = 9.99999993922529e-09f; // 00CF7FE8
inline constexpr double kOceanStepTimeScale = 0.10000000149011612; // 00D7A3A0, 0078D184

// 0109048C, set by the static initializer 00CD8870: float(1 / float(sqrt(2.0)))
// (2.0 is the qword 00D7A308).
float ocean_inv_sqrt2_0109048c() noexcept;

// 00B97100's layout, as far as the height path reads it.
struct OceanSpectrum {
    float wind_cos_6c{0.0f};            // +6Ch, FCOS of the direction (00B96C15)
    float wind_sin_70{0.0f};            // +70h, FSIN (00B96C28)
    float wind_speed_74{0.0f};          // +74h
    float largest_wave_78{0.0f};        // +78h = float(+74h^2 / g), 00B9578F
    float smallest_wave_7c{0.0f};       // +7Ch = float(+78h * +8Ch), 00B95798
    float wave_height_80{0.0f};         // +80h, the Phillips constant
    float direction_power_84{0.5f};     // +84h, 0.5 (00CE3800) from the constructor
    float choppy_88{0.0f};              // +88h, cWaveScale's factor (00BA0B38)
    float small_wave_factor_8c{0.0f};   // +8Ch
    std::vector<OceanComplex> h0_10;    // +0Ch vector (begin +10h): 41h x 41h, h0(k)
    std::vector<OceanComplex> h_20;     // +1Ch vector (begin +20h): 64 x 64, h(k,t) then its IFFT
    std::vector<float> omega_50;        // +4Ch vector (begin +50h): sqrt(g k), 64 x 64
    std::vector<float> inv_k_60;        // +5Ch vector (begin +60h): 1 / max(k, 1e-8), 64 x 64
};

// 00BF0DF0 `float __thiscall(RandomState*, float mean, float sigma)`: the polar
// Box-Muller draw. u1, u2 = float(2 * float(u32 * 2^-32) - 1) until
// s = float(u1^2 + u2^2) < 1 (no zero test), then
// float(float(float(sqrt(float(float(ln s) * -2.0 / s))) * u1) * sigma + mean).
float ocean_normal_draw_00bf0df0(RandomState& state, float mean, float sigma);

// 00B94CA0 `float __thiscall(spectrum, float kx, float ky)`, RET 8: the Phillips
// term A * exp(-1/(kL)^2 - (kl)^2) * |k^ . w|^p / k^4, with k clamped to 1e-8,
// |k^ . w|^p = 0 when the dot is exactly 0, and both powers through FYL2X/F2XM1.
float ocean_phillips_00b94ca0(const OceanSpectrum& spectrum, float kx, float ky);

// 00B95750 `void __fastcall(spectrum)`: +78h/+7Ch, then h0 from a stack Mersenne
// state seeded with 1105h (never the shared stream 00BD2F10), the 1/k table and the
// dispersion table, then vtable+10h (00B96E60, a bare RET).
void ocean_spectrum_build_00b95750(OceanSpectrum& spectrum);

// 00B96B90 (seed, direction, speed, height, power, choppy, small), RET 1Ch; the seed
// argument is not stored. 00B96C10 (direction, speed), RET 8. 00B96C50 (height,
// choppy), RET 8. Each ends in 00B95750.
void ocean_spectrum_set_all_00b96b90(OceanSpectrum& spectrum, float direction, float speed,
                                     float height, float power, float choppy, float small_factor);
void ocean_spectrum_set_wind_00b96c10(OceanSpectrum& spectrum, float direction, float speed);
void ocean_spectrum_set_height_00b96c50(OceanSpectrum& spectrum, float height, float choppy);

// 00B97100: sizes the vectors, +84h = 0.5, then 00B96B90(1105h, 1.570796 (00D638CC),
// 32.0, 60.0, 2.0, 0.02, 0.00005) from 00D638CC..00D638E0.
void ocean_spectrum_construct_00b97100(OceanSpectrum& spectrum);

// 00B952D0 `void __thiscall(spectrum, int dir, float* re, float* im)`, RET 0Ch: a
// 64-point radix-2 FFT in place (bit reversal, then six butterfly passes with the
// twiddle recurrence c2 = float(float(sqrt(1 - c1)) * 1/sqrt2) * -dir,
// c1 = float(float(sqrt(1 + c1)) * 1/sqrt2)); dir == 1 also divides by 64.
void ocean_fft64_00b952d0(int dir, float* re, float* im);

// 00B95AA0 `void __thiscall(spectrum, int dir, vector*)`, RET 8: rows, then columns.
void ocean_fft2d_00b95aa0(int dir, std::vector<OceanComplex>& cells);

// 00B95D30 `void __thiscall(spectrum, float t)`, RET 4, gated by the byte 00E130B4
// (statically 1, never written): h(k,t) = h0(k) e^{iwt} + conj(h0(-k)) e^{-iwt} into
// +20h at row m & 3Fh, column n & 3Fh, then 00B95AA0(-1, +1Ch). Partial: the choppy
// vectors (00B95EBF..00B9604C and 00B960A7) are skipped (see the header comment).
void ocean_spectrum_advance_00b95d30(OceanSpectrum& spectrum, float t);

// The grid 00B960C0 samples.
OceanWaveGridView ocean_spectrum_grid(const OceanSpectrum& spectrum) noexcept;

// The scene's g_Ocean/Waves keys as the +990h block carries them (section 7(a)); a
// missing key keeps the 004CB420 default.
struct OceanSceneWaves {
    float falloff_2c{0.6f};          // Waves.Falloff
    float layer_scale_30{100.0f};    // Waves.LayerScale
    float time_scale_34{1.0f};       // Waves.TimeScale
    float wind_direction_4c{0.5f};   // Waves.WindDirection
    float wind_speed_50{64.0f};      // Waves.WindSpeed
    float wave_height_54{80.0f};     // Waves.WaveHeight
    float choppy_58{0.04f};          // Waves.ChoppyWavesFactor
    float height_scale_5c{1.0f};     // Waves.HeightScale
    bool simple_ocean{false};        // g_Ocean 0_SimpleOcean -> field+F9h
    bool authored{false};            // a g_Ocean/Waves block was found
};
OceanSceneWaves ocean_scene_waves_from_header(const ScenePropertyBlock& header_properties);

// The ocean object at world+A8h as the CPU height path needs it, plus its spectrum.
struct OceanLiveField {
    OceanSpectrum spectrum;
    OceanWaveFieldState field;       // +F9h, +B4h, +24h
    float time_scale_20{1.0f};       // +20h, 00B9A4C0
    float height_scale_b8{1.0f};     // +B8h, 0078CA0E
    float height_scale_2c{0.0f};     // +2Ch, 00BA0B55
    float wave_scale_28{0.0f};       // +28h, 00BA0B5F (render only)
    float clock_01090494{0.0f};      // [01090494], 00BA00BA
    std::uint32_t render_steps{0};   // calls of 00BA0090 since the last world build
    OceanCoverageRegions coverage;   // +620h / +624h, one landscape at a time (008830AA)
};

// 0078DAA0's ocean part (field defaults, the spectrum's 00B97100), then 0078C9B0's
// parameter calls: 00B96C50(+54h, +58h), 00B9A450 -> 00B96C10(float(+4Ch * pi),
// +50h), +B8h = +5Ch, 00B9A4C0(+2Ch, +30h, +34h).
void ocean_live_field_build(OceanLiveField& ocean, const OceanSceneWaves& waves);

// 0068A0D0 -> 0078CFF0 -> 00BA0090's height-path part, once per level-1 render
// frame: t = float(clock * 0.1); [01090494] = float(t * 10.0); the spectrum step
// 00B96C80(float(t * +20h)); +2Ch = +B8h; +24h = +2Ch (00BA0B93).
void ocean_live_field_render_frame_00ba0090(OceanLiveField& ocean, float mission_clock);

// Packet cc9_physics1_sea, docs/OCEAN_WAVE_FIELD.md section 9. True: the host builds
// the ocean from the mission scene's g_Ocean/Waves keys when the header pass reads
// them (0078C9B0 at 004DF87F), steps it at the render point of every in-mission
// frame (job B 004B4820 -> 0068A0D0 -> 00BA0090, after the fixed steps of the same
// frame), and every host reader of 0078C890 samples the live grid times the live
// amplitude. False: the readers keep the flat sea (amplitude 0.0, a zero grid) and
// the publish and step calls do nothing. ON together with kGunneryLiveWaterSurfaceBound
// (docs/OCEAN_WAVE_FIELD.md section 17).
inline constexpr bool kWaveFieldLiveAmplitudeBound = true;

// The host's one ocean object (world+A8h). `ocean_live_field_ready` is true once a
// scene was published while the switch is on.
OceanLiveField& ocean_live_field_process();
bool ocean_live_field_ready();
OceanSceneWaves ocean_live_field_publish_scene(const ScenePropertyBlock& header_properties);
// LABELLED: the host runs no BSP_Game_Render view branch, so every in-mission frame
// is taken as interface level 1 (the 3D view); the image skips the step on the map
// views (levels 2 and 3).
void ocean_live_field_render_job_0068a0d0(float mission_clock);

// 00882AC0's call at 008830AA for one landscape, once its world frame is known:
// adds the model's wave-damp regions (00BA1370) to the live ocean. Returns the
// log line's tail, or an empty string while the switch is off or no scene was
// published. LABELLED: the loader's reduction renderer+1D84h is taken as 0
// (2 - TextureDetail, which is 2 in this installation's options.txt).
std::string ocean_live_field_add_landscape_008830aa(const std::string& model_name,
    const float landscape_world[16], const OceanCoverageFileReader& read_file);

// 00B9CF50 over the live ocean's regions (1.0 outside them, and while the
// switch is off).
float ocean_live_field_coverage_mask(float x, float z);

// 0078CF20 over the live ocean: the wave height times the coverage mask, rounded
// once (0078CF69). 0.0 while the switch is off or before a scene was published.
float ocean_live_field_water_height_0078cf20(float x, float z);

}  // namespace bsp

namespace bsp {
// cOceanSpectrum vtable 00D638F8 slots 4..7: four distinct bare RET bodies.
// ECX is unused; no stack arguments, no defined return value, no state changes.
void ocean_default_slot4_00b96e60();
void ocean_default_slot5_00b96e70();
void ocean_default_slot6_00b96e80();
void ocean_default_slot7_00b96e90();
} // namespace bsp
