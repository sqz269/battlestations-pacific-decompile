#include "bsp/bullet_engagement_range.hpp"
#include "bsp/flak_note_seg36_37_argon.hpp"
#include "bsp/gameplay_seg53_cc13.hpp"

#include <cmath>

namespace bsp {
namespace {

// The native computes these on the x87 stack: a float operand is loaded at
// extended precision, a `FMUL double` keeps the double constant exact, and the
// store back to a float field is the only rounding. Modelling the chain in
// double and casting once at the store reproduces that.
float store_float(double value) noexcept { return static_cast<float>(value); }

}  // namespace

int weapon_class_sub_type_for_lua_type(const std::string& type) noexcept {
    const ProjectileClassInfo* info = projectile_class_for_lua_type(type);
    return info != nullptr ? info->sub_type : 0;
}

int weapon_class_refined_sub_type(const WeaponClassFinaliseInput& in) noexcept {
    // 006E9968: only sub-type 1 and sub-type 4 are rewritten.
    if (in.sub_type == kProjectileSubTypeBullet) {
        // 006E996D-006E99D8. +14h is the `Name` string's character data; null
        // falls back to the empty literal 00E199AC. _strstr against "AA" is
        // case-sensitive, and the stored value is 2 + (found ? 1 : 0).
        const bool anti_air = in.name.find(kBulletAntiAirNameMarker) != std::string::npos;
        return anti_air ? kProjectileSubTypeBulletAntiAir : kProjectileSubTypeBulletPlain;
    }
    if (in.sub_type == kProjectileSubTypeArtillery) {
        // 006E99E7-006E9A39. Both COMISS put the threshold on the left and the
        // branch falls through only on a strict greater-than, so the tier holds
        // when the threshold exceeds BOTH DamageMin and Blast.BlastDamageMin.
        if (kArtilleryTierLightBelow > in.damage_min &&
            kArtilleryTierLightBelow > in.blast_damage_min) {
            return kProjectileSubTypeArtilleryLight;
        }
        if (kArtilleryTierMediumBelow > in.damage_min &&
            kArtilleryTierMediumBelow > in.blast_damage_min) {
            return kProjectileSubTypeArtilleryMedium;
        }
        return kProjectileSubTypeArtilleryHeavy;
    }
    return in.sub_type;
}

WeaponClassFinaliseResult weapon_class_derive_base_engagement_range(
    const WeaponClassFinaliseInput& in, bool already_finalised) noexcept {
    WeaponClassFinaliseResult out;
    out.muzzle_speed = in.muzzle_speed;
    out.fly_time = in.fly_time;
    out.time_scale = in.time_scale;
    out.sub_type = in.sub_type;
    out.engagement_range = 0.0f;

    // 006E9896: the latch. A second call falls straight to 006E9A41, writes
    // nothing and still returns true in AL.
    if (already_finalised) {
        out.latched = true;
        out.accepted = true;
        return out;
    }
    out.latched = true;

    const int kind = in.sub_type;
    if (kind == kProjectileSubTypeArtillery || kind == kProjectileSubTypeArtilleryLight ||
        kind == kProjectileSubTypeArtilleryMedium || kind == kProjectileSubTypeArtilleryHeavy) {
        // 006E98BC. The authored Range is the engagement range unchanged, and
        // V0 is REPLACED by the speed that reaches it on a flat 45-degree arc,
        // v = sqrt(g * R). +5Ch keeps the ratio of the authored V0 to it.
        // 006E98D1 rounds the product to a float before the sqrt call, so the
        // rounding happens twice.
        const float product = store_float(static_cast<double>(in.range) * kWeaponClassGravity);
        const float impact_speed = store_float(std::sqrt(static_cast<double>(product)));
        out.engagement_range = in.range;
        out.time_scale = store_float(static_cast<double>(in.muzzle_speed) / impact_speed);
        out.muzzle_speed = impact_speed;
    } else if (kind == kProjectileSubTypeBullet || kind == kProjectileSubTypeBulletPlain ||
               kind == kProjectileSubTypeBulletAntiAir || kind == kProjectileSubTypeFlak) {
        // 006E9941. An authored Range is converted into a flight time first,
        // then multiplied straight back out, so a class that authors Range
        // round-trips to it. A class that does not keeps its authored FlyTime
        // and the range becomes FlyTime * V0. The COMISS/JBE pair takes the
        // division only on a strictly positive, ordered Range.
        if (in.range > kWeaponClassRangeEpsilon) {
            out.fly_time = store_float(static_cast<double>(in.range) / in.muzzle_speed);
        }
        out.engagement_range =
            store_float(static_cast<double>(out.fly_time) * in.muzzle_speed);
    } else if (kind == kProjectileSubTypeDepthCharge) {
        out.engagement_range = kDepthChargeEngagementRange;  // 006E9923
    } else {
        out.engagement_range = kWeaponClassDefaultEngagementRange;  // 006E9932
    }

    out.sub_type = weapon_class_refined_sub_type(in);
    out.accepted = true;  // 006E99DB / 006E9A0D / 006E9A2D / 006E9A41: AL = 1

    return out;
}

WeaponClassFinaliseResult weapon_class_derive_engagement_range(
    const WeaponClassFinaliseInput& in, bool already_finalised) noexcept {
    if (in.sub_type == kProjectileSubTypeFlak) {
        return flak_note_seg36_37_argon::FlakBulletClass_DeriveEngagementRange_0070c0b0(
            in, already_finalised);
    }
    auto out = weapon_class_derive_base_engagement_range(in, already_finalised);

    const int kind = in.sub_type;
    if (kind == kProjectileSubTypeTorpedo) {
        // Goal 5 chain 788: bsp.py show 00855A90 --asm, 00855A95..00855AF5.
        // Original ABI: ECX descriptor, RET 0, AL success. The base runs first;
        // its +54h and the cached descriptor's +E0h/+E4h feed the existing leaf
        // body. Its +60h goes to gun.max_range, and +ECh/AL remain the result's
        // terminal speed/acceptance. This is a semantic adapter, not that ABI.
        // Both FMULs round at the main thread's PC24 precision. The prior
        // double chain rounded only once. Even when the base latch is set,
        // 00855A9A resumes here and recomputes the leaf fields.
        gameplay_seg53_cc13::TorpedoClassFields fields;
        fields.fly_time_54 = out.fly_time;
        fields.max_fall_e0 = in.max_fall;
        fields.water_travel_speed_e4 = in.water_travel_speed;
        const auto leaf = gameplay_seg53_cc13::torpedo_class_derive_00855a90(
            fields, out.accepted);
        out.engagement_range = leaf.cached_range_60;
        out.terminal_fall_speed = leaf.terminal_fall_speed_ec;
        out.accepted = leaf.ok;
        // This host's additional swim-speed output is not a field 00855A90
        // writes; preserve its existing producer for the swim integrator.
        out.swim_speed = store_float(static_cast<double>(in.water_travel_speed) *
                                     kTorpedoSwimSpeedFraction);
    }
    return out;
}

float weapon_class_engagement_range(const WeaponClassFinaliseInput& in) noexcept {
    return weapon_class_derive_engagement_range(in).engagement_range;
}

}  // namespace bsp
