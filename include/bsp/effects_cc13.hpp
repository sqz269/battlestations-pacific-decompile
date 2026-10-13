#pragma once

// Effect instances of the gameplay-effect components (band 00865000-00875FFF,
// packet cc13-effects1, docs/CC13_EFFECTS.md).
//
// Each component (sound, splash, flare, waterdrops, light, thunderstorm) creates an
// instance object that the point-effect child update 00867790 drives through the
// instance's primary vtable: +08h IsComplete, +28h Update(dt, reference node),
// +30h Stop. The instance starts with the 1Ch registered-effect prefix
// (NativeRegisteredEffectPrefixStorage) and keeps its payload pointer at +1Ch.
//
// These are new C++ interfaces, not binary replacements. Calls the image makes to
// other subsystems go through EffectInstanceServices; virtual calls on the payload
// go through EffectTrackedSound. Nothing in the game host calls this file.
//
// RNG: the sound instance constructor 00873AE0 draws twice from the shared thread
// generator with ECX = 0, stream 0 (00873B63 raw word, 00873C39 uniform). The host
// must not reproduce that: every draw goes through EffectPresentationRandom, keyed
// per image site, so a host binding can give effects their own presentation
// stream and keep gameplay stream 0 untouched (RNG rule, docs/CC13_RNG_STREAM_CENSUS.md
// section 12).

#include "bsp/live_effect_event_registry.hpp"
#include "bsp/point_effect_instance.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp::effects_cc13 {

using Vec3 = std::array<float, 3>;

// One key per image draw site. The image uses stream 0 (ECX = 0) at both.
enum class EffectRandomSite : std::uint32_t {
    sound_sample_00873b63 = 1, // image 00BD2FC0, raw 32-bit word, ECX = 0
    sound_pitch_00873c39 = 2,  // image 00BD2F10, U(lo, hi), ECX = 0
};

// Presentation-only random source. A binding must not forward to stream 0.
class EffectPresentationRandom {
public:
    virtual ~EffectPresentationRandom() = default;
    virtual std::uint32_t next_u32(EffectRandomSite site) = 0;
    virtual float uniform(EffectRandomSite site, float lo, float hi) = 0;
};

// The fields of the tracked sound object that the instance code touches directly.
struct EffectTrackedSoundFields {
    std::uint8_t dirty_14;  // set to 1 whenever pitch_20 or level_40 is written
    float pitch_20;         // PitchRnd factor, 1 +/- PitchRnd
    float level_40;         // clamped to [0, 1] by 00872FB0
    void* sample_4c;        // sample record, read by 00A81860
};

struct EffectSoundParameterRange {
    std::int32_t id;
    float lo;
    float hi;
};

// The tracked sound held at instance +1Ch (created by 00A7E490). Virtual offsets
// are the sound object's own vtable; the reference count is at sound +4.
class EffectTrackedSound {
public:
    virtual ~EffectTrackedSound() = default;
    virtual void stop_08(std::int32_t mode) = 0;
    virtual bool is_finished_0c() = 0;
    virtual EffectSoundParameterRange parameter_range_28(std::int32_t index) = 0;
    virtual void set_parameter_2c(std::int32_t id, float value) = 0;
    virtual void set_3d_attributes_34(const Vec3& position, const Vec3& velocity) = 0;
    virtual EffectTrackedSoundFields& fields() = 0;
    virtual void retain() = 0;
    virtual void release() = 0; // InterlockedDecrement(+4); deletes at zero
};

// Sound component definition fields read by the instance (allocation 3Ch,
// docs/GAMEPLAY_EFFECT_SOUND.md).
struct EffectSoundDefinitionFields {
    void* const* samples_20;
    std::uint32_t sample_count_24;
    std::uint8_t persistent_2c;
    std::int32_t class_index_30;
    std::int32_t type_index_34;
    float pitch_rnd_38;
};

// Calls the instance code makes outside the band. Each name carries the image address.
class EffectInstanceServices {
public:
    virtual ~EffectInstanceServices() = default;
    // 00A7E490 on the sound manager [00F8BBD8]; returns a retained sound or null.
    virtual EffectTrackedSound* create_tracked_sound_00a7e490(void* sample,
        std::int32_t class_index, std::int32_t type_index, std::uint8_t persistent) = 0;
    virtual void scale_sound_volume_00a79880(EffectTrackedSound& sound, float factor) = 0;
    // byte +15h of the options record 00A81860 returns for the sample
    virtual bool sample_flag15_00a81860(void* sample) = 0;
    // 006E0CD0: point node +110h world translation (+120h..+128h) after refreshing
    // its world matrix (00B6DB70) when node+5Ch bit 1 is clear
    virtual Vec3 point_world_position_006e0cd0(const PointEffectInstanceStorage& point) = 0;
    // 00B6DFA0 BSP_Node_UnlinkAndRelease
    virtual void unlink_and_release_node_00b6dfa0(void* node) = 0;
    // 00B7C160 BSP_PointLight_UnlinkAllModels
    virtual void unlink_light_models_00b7c160(void* light) = 0;
    // 00B8B670 BSP_Flare_PairTest
    virtual bool flare_pair_test_00b8b670(void* flare) = 0;
    // 00B8D150 BSP_Flare_UpdateFromCachedOcclusionSamples(flare, [00E188A8]+19FCh, dt)
    virtual void flare_update_00b8d150(void* flare, float dt) = 0;
    // 00BC1150 BSP_Waterdrops_IsFinished
    virtual bool waterdrops_finished_00bc1150(void* waterdrops) = 0;
    // byte 00F889FC: waterdrops enabled
    virtual bool waterdrops_enabled_00f889fc() = 0;
    // 00B9E4A0 on ([00E188A8]+19F0h)+A8h: BSP_OceanWaveField_RemoveWaterRing
    virtual void remove_water_ring_00b9e4a0(void* ring) = 0;
};

// Splash ring payload: the instance only reads and writes its timer.
struct EffectSplashRingFields {
    float timer_20;
};

// --- sound instance, primary vtable 00D0DD78 / secondary 00D0DD74, allocation 20h ---

struct EffectSoundInstanceStorage {
    NativeRegisteredEffectPrefixStorage prefix; // type_18 = 2
    EffectTrackedSound* sound_1c;
};

// 00873AE0, ECX instance, stack (definition, point), RET8, called by 00868D00 after
// operator_new(20h). Draws the sample index and the pitch factor through `random`.
EffectSoundInstanceStorage& construct_effect_sound_instance_00873ae0(
    EffectSoundInstanceStorage& instance, const EffectSoundDefinitionFields& definition,
    PointEffectInstanceStorage& point, EffectInstanceServices& services,
    EffectPresentationRandom& random);

void stop_effect_sound_instance_00872630(EffectSoundInstanceStorage&);
bool effect_sound_instance_is_complete_00872650(EffectSoundInstanceStorage&);
void scale_effect_sound_instance_volume_00872670(EffectSoundInstanceStorage&, float factor,
    EffectInstanceServices&);
void set_effect_sound_instance_pitch_00872690(EffectSoundInstanceStorage&, float pitch);
void set_effect_sound_instance_parameter_fraction_008726c0(EffectSoundInstanceStorage&,
    std::int32_t index, float t);
bool effect_sound_instance_sample_flag15_00872720(EffectSoundInstanceStorage&,
    EffectInstanceServices&);
void update_effect_sound_instance_00872f40(EffectSoundInstanceStorage&, float dt,
    void* reference_node, const PointEffectInstanceStorage& point, EffectInstanceServices&);
void set_effect_sound_instance_level_00872fb0(EffectSoundInstanceStorage&, float level);
// Body of 00873DE0 between the table writes: stop(1), release, clear.
void destruct_effect_sound_instance_00873de0(EffectSoundInstanceStorage&);

// --- splash instance, secondary vtable 00D0DC24 ---

struct EffectSplashInstanceStorage {
    NativeRegisteredEffectPrefixStorage prefix;
    EffectSplashRingFields* ring_1c;
};

bool effect_splash_instance_is_complete_008721c0(const EffectSplashInstanceStorage&);
void stop_effect_splash_instance_00872df0(EffectSplashInstanceStorage&, EffectInstanceServices&);
void update_effect_splash_instance_00872e30(EffectSplashInstanceStorage&, float dt);

// --- node-payload instances: flare 00D0DC5C, waterdrops 00D0DBB4, light 00D0DBEC ---

struct EffectNodeInstanceStorage {
    NativeRegisteredEffectPrefixStorage prefix;
    void* payload_1c;
    std::uint8_t suppressed_20; // waterdrops only: IsComplete returns false while set
};

bool effect_flare_instance_is_complete_008720f0(EffectNodeInstanceStorage&, EffectInstanceServices&);
void update_effect_flare_instance_008720a0(EffectNodeInstanceStorage&, float dt, EffectInstanceServices&);
void stop_effect_flare_instance_008720d0(EffectNodeInstanceStorage&, EffectInstanceServices&);
bool effect_waterdrops_instance_is_complete_00872120(EffectNodeInstanceStorage&, EffectInstanceServices&);
void stop_effect_waterdrops_instance_00872bf0(EffectNodeInstanceStorage&, EffectInstanceServices&);
void stop_effect_light_instance_00872d30(EffectNodeInstanceStorage&, EffectInstanceServices&);

// --- thunderstorm instance 00D0DDCC ---

// 00872110: XOR AL,AL; RET. A storm instance never reports completion.
constexpr bool effect_thunderstorm_instance_is_complete_00872110() noexcept { return false; }
// 00872100: a bare RET.
constexpr void stop_effect_thunderstorm_instance_00872100() noexcept {}

// --- point effect ---

// 00865FE0, ECX point, RET; callers 0081C050, 0048A1E0, 007C3700.
bool point_effect_is_finished_00865fe0(const PointEffectInstanceStorage& point);

} // namespace bsp::effects_cc13
