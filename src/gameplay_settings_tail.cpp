// Tail of the gameplay tuning settings object (packet cc2_settings_tail).
// Addresses: 00836EF0 00836F80 00B685C0 0093C120 0093C210. See
// docs/GAMEPLAY_SETTINGS_TAIL.md for the evidence behind every value here.
#include "bsp/gameplay_settings_tail.hpp"

namespace bsp {

// The eleven keys in the order 00836F80 pushes them: 00D0A17C, then 00D0A160
// down to 00D0A064 in steps of 1Ch.
const char* const kWeaponHitAccuracyKeys[kWeaponHitAccuracyKeyCount] = {
    "TargetReferenceSizes",      // 00D0A17C, 00836FA0 and 00836FF2
    "Accuracy_10percent_Range",  // 00D0A160, 00837043 and 00837094
    "Accuracy_20percent_Range",  // 00D0A144, 008370E5 and 00837136
    "Accuracy_30percent_Range",  // 00D0A128, 00837187 and 008371D8
    "Accuracy_40percent_Range",  // 00D0A10C, 00837229 and 0083727A
    "Accuracy_50percent_Range",  // 00D0A0F0, 008372CB and 0083731C
    "Accuracy_60percent_Range",  // 00D0A0D4, 0083736D and 008373BE
    "Accuracy_70percent_Range",  // 00D0A0B8, 0083740F and 00837460
    "Accuracy_80percent_Range",  // 00D0A09C, 008374B1 and 00837502
    "Accuracy_90percent_Range",  // 00D0A080, 00837553 and 008375A4
    "Accuracy_100percent_Range", // 00D0A064, 008375F5 and 00837646
};

const GameplaySettingsStringKey kGameplaySettingsStringKeys[kGameplaySettingsStringKeyCount] = {
    // ShipAvoidance.RightOfWayValues[n], read positionally: element [1] is the
    // class name and element [2] the float. 0083BD70 asks for element 1 of the
    // entry, so there is no key string.
    {nullptr, 0x0083BD7Fu, 0u, kRightOfWayValuesVectorOffset,
     GameplaySettingsStringSink::ClassKeyedValue,
     "append {class index, float} through 0083A880 to the vector at +1DCh"},
    {"CollisionEffect", 0x0083E016u, 0x00D0ADACu, kCollisionEffectHandleOffset,
     GameplaySettingsStringSink::EffectHandle, "effect handle stored at +3A4h (0083E08C)"},
    {"FailureName", 0x0083E7C4u, 0x00D0AC48u, 0u, GameplaySettingsStringSink::FailureRecord,
     "borrowed pointer kept at [ESP+88h] through the failure descriptor build; the record "
     "store is in the unread 0083E5D8..0083E9xx block"},
    {"SectionName", 0x0083E8DAu, 0x00D0AC30u, 0u, GameplaySettingsStringSink::FailureRecord,
     "copied to a stack NativeString and consumed by 00438E10 at 0083E956, which selects a "
     "slot in the table at 00E08138; the record store is in the unread block"},
    {"SinkEffect", 0x0083EAECu, 0x00D0ABF4u, kSinkEffectHandleOffset,
     GameplaySettingsStringSink::EffectHandle, "effect handle stored at +72Ch (0083EB60)"},
    {"BulletEffect", 0x008414A7u, 0x00D0A300u, kFreeCameraShotBulletEffectOffset,
     GameplaySettingsStringSink::EffectHandle, "free-camera-shot record +24h (008414E6)"},
    {"ExplosionEffect", 0x0084159Au, 0x00D0A2F0u, kFreeCameraShotExplosionEffectOffset,
     GameplaySettingsStringSink::EffectHandle, "free-camera-shot record +28h (008415D9)"},
    {"SplashEffect", 0x0084168Du, 0x00D0A2E0u, kFreeCameraShotSplashEffectOffset,
     GameplaySettingsStringSink::EffectHandle, "free-camera-shot record +2Ch (008416CF)"},
    // The only 00B685C0 site. The fallback at 00CE3A0C is the empty string.
    {"Name", 0x008418A4u, 0x00CE8ED0u, kFreeCameraShotNameOffset,
     GameplaySettingsStringSink::NativeStringField,
     "free-camera-shot record +0h, a NativeString copied at 008418BE..008418D4"},
};

void apply_weapon_hit_accuracy_defaults_00836ef0(WeaponHitAccuracyProfile& out) noexcept {
    // 00836EF0 in order: +0h from 00CE3D08, +4h from 00CE386C, then the twenty
    // range slots from 00CE3800. The interleaved store order (+30h before +8h)
    // is a scheduling detail, not a dependency.
    out.small_target_size = 100.0F;
    out.large_target_size = 200.0F;
    for (int i = 0; i < kWeaponHitAccuracyBucketCount; ++i) {
        out.small_target_accuracy[i] = 0.5F;
        out.large_target_accuracy[i] = 0.5F;
    }
}

void load_weapon_hit_accuracy_profile_00836f80(WeaponHitAccuracyProfile& profile,
                                               WeaponHitAccuracyTableHost& host) {
    // 00836FA0..00837025: the first key fills the two adjacent scalars.
    profile.small_target_size = host.read_number(kWeaponHitAccuracyKeys[0], 1, profile.small_target_size);
    profile.large_target_size = host.read_number(kWeaponHitAccuracyKeys[0], 2, profile.large_target_size);
    // 00837043..0083767A: ten identical blocks, element [1] then element [2].
    for (int i = 0; i < kWeaponHitAccuracyBucketCount; ++i) {
        const char* const key = kWeaponHitAccuracyKeys[i + 1];
        profile.small_target_accuracy[i] = host.read_number(key, 1, profile.small_target_accuracy[i]);
        profile.large_target_accuracy[i] = host.read_number(key, 2, profile.large_target_accuracy[i]);
    }
}

float repair_damage_divisor(int task_priority, int step_priority, float settings_multiplier,
                            float difficulty_modifier) noexcept {
    // 0093C126..0093C143 and 0093C216..0093C233: the settings value only applies
    // at the step's own priority, otherwise the 1.0f at 00D7A24C is used.
    const float base = (task_priority == step_priority) ? settings_multiplier : 1.0F;
    // 0093C187 and 0093C277: the modifier multiplies the divisor. The caller
    // passes 1.0f when the gate at 0093C151..0093C165 fails.
    return base * difficulty_modifier;
}

}  // namespace bsp
