#pragma once
#include <functional>

// The plane's wanderer: the object at plane+810h that 007C4560 BSP_PlaneWanderer_Construct
// builds and 007BE060 BSP_PlaneWanderer_FixedStep (007BE060-007BE9A0) steps from the plane
// fixed step 007CE040 (007CE0D2). 007D8230 adds its +0h..+8h to the published translation,
// so a formation member drifts around its station.
//
// docs/PLANE_WANDERER.md carries the listing evidence. Names are hypotheses; the tuning names
// are the game's own (PlaneGlobals Wanderer/*). Not a binary-compatible layout.

namespace bsp {

// 007C4560 copies these from the tuning block into statics (007C462B-007C47D6).
struct PlaneWandererTuning {
    float speed_range_1{0.0f};        // +1C0h -> [00F87268]
    float speed_range_2{0.0f};        // +1C4h -> [00F8726C]
    float roll_change_chance{0.0f};   // +1C8h -> [00E0B004]
    float roll_change_max{0.0f};      // +1CCh -> [00F87274]
    float roll_change_decay{0.0f};    // +1D0h -> [00F87270]
    float roll_change_speed{0.0f};    // +1D4h -> [00E0B008]
    float time_range_1{0.0f};         // +1D8h -> [00E0B00C]
    float time_range_2{0.0f};         // +1DCh -> [00E0B010]
    float offset_max{0.0f};           // +1E0h -> [00E0B014]
    float change_mul{0.0f};           // +1E4h -> [00E0B018]
    float accel_max{0.0f};            // +1E8h -> [00E0B01C]
    float speed_max{0.0f};            // +1ECh -> [00E0B024]
    float accel_decay_time{1.0f};     // +1F0h, [00E0B020] = AccelMax / it
    float speed_decay_time{1.0f};     // +1F4h, [00E0B028] = SpeedMax / it
    float offset_decay_time{1.0f};    // +1F8h, [00E0B02C] = OffsetMax / it
    float small_decal_mul{1.0f};      // +1FCh -> [00E0B030]
    float small_roll_decay_mul{1.0f}; // +200h -> [00E0B040]
    float small_accel_mul{1.0f};      // +204h -> [00E0B034]
    float small_time_mul{1.0f};       // +208h -> [00E0B038]
    float small_offset_mul{1.0f};     // +20Ch -> [00E0B03C]
};

// plane+810h. Offsets are the object's.
struct PlaneWandererState {
    float offset[3]{0.0f, 0.0f, 0.0f};  // +0h: 007D8230 adds t * this to the translation
    float speed[3]{0.0f, 0.0f, 0.0f};   // +0Ch: clamped to SpeedMax
    float accel[3]{0.0f, 0.0f, 0.0f};   // +18h: clamped to AccelMax
    float timer{0.0f};                  // +24h: U(0.5, 1.5) at construction, stream 1
    float roll{0.0f};                   // +28h
    float roll_target{0.0f};            // +2Ch
    float mul{1.0f};                    // +30h: WandererMul, reset to 1.0 every step
    bool enabled{true};                 // +34h: WandererEnabled
    bool constructed{false};            // host: the construction draw has been taken
};

struct PlaneWandererInputs {
    bool airborne{false};          // (plane+72Ch)->vtable[38h] (007BE0FB)
    bool player_slot_arm{false};   // plane+520h && plane+9D8h (007BE0CC-007BE0E3)
    bool large{false};             // 0047B850 / IsKindOf(10h) or (16h)
    float plane_speed{0.0f};       // plane->vtable[38h] (007BE55A)
    float height_9b4{0.0f};        // plane+9B4h, the height above ground
    bool local_player_role_1{false};  // 00927F30(plane, 1) (007BE8C8)
};

// Draw on stream 1 (00BD2F10 with ECX = 1): lo + (hi - lo) * U.
using PlaneWandererDraw = std::function<float(float lo, float hi)>;

// 007C4560's one draw: +24h = U(0.5, 1.5) (00CE3800, 00CE380C).
void plane_wanderer_construct_007c4560(PlaneWandererState& s, const PlaneWandererDraw& draw);

// 007BE060, single-player arm (game+1FE4h == 0). Returns true when the active branch ran.
bool plane_wanderer_fixed_step_007be060(PlaneWandererState& s, const PlaneWandererTuning& t,
                                        const PlaneWandererInputs& in, float dt,
                                        const PlaneWandererDraw& draw);

// 00439950: a uniform direction, z = U(-1, 1), phi = U(0, 2pi) (00CE3D9C), r = sqrt(1 - z^2).
void plane_wanderer_random_direction_00439950(float out[3], const PlaneWandererDraw& draw);

}  // namespace bsp
