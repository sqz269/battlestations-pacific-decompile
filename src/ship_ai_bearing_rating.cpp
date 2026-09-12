// Packet cc_ai_bearing_rating. Evidence: docs/SHIP_AI_BEARING_RATING.md.
//
// Every expression below is transcribed from the listing of 0095EB40
// (body 0095EB40-0095F079) and 0095F080 (body 0095F080-0095F167), not from the
// decompiler output; the pseudocode dropped the by-reference arguments of
// 00415510 and 00415690, the `FDIV [EDI+20h]` of the fire term and the `LEA
// ECX,[EDI+14h]` that makes 00605070 an in-place wrap.
//
// The image computes the per-mount product in x87 registers and rounds to float
// only where it stores. The `static_cast<float>` calls below sit exactly at the
// image's stores; the `double` intermediates sit exactly where it keeps a value
// on the stack.

#include "bsp/ship_ai_bearing_rating.hpp"

#include <cmath>

namespace bsp {
namespace {

// 00CE3828, 00CE3D18 and 00CE3D28, the three doubles 00605070 uses.
constexpr double kTurn = 6.283185307179586;   // 00CE3828
constexpr double kNegHalfTurn = -3.14159265358979; // 00CE3D18
constexpr double kHalfTurn = 3.14159265358979;     // 00CE3D28

// 00415690, `__fastcall(float* v, const float* lo, const float* hi)`, RET 4.
// The order matters: the low bound is applied first and wins outright.
float clamp_in_place_00415690(float value, float low, float high) noexcept
{
    if (low > value) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

// 00415510, `__fastcall(const float* a, const float* b)`, RET 0, result in ST0.
float min_by_ref_00415510(float a, float b) noexcept
{
    return (b <= a) ? b : a;
}

}  // namespace

float ship_ai_firepower_wrap_angle_00605070(float radians) noexcept
{
    // 00605074..0060507C: fmod against the 2*pi double, then the two fixups.
    float wrapped = static_cast<float>(std::fmod(static_cast<double>(radians), kTurn));
    if (kNegHalfTurn >= static_cast<double>(wrapped)) {
        return static_cast<float>(static_cast<double>(wrapped) + kTurn);
    }
    if (static_cast<double>(wrapped) > kHalfTurn) {
        return static_cast<float>(static_cast<double>(wrapped) - kTurn);
    }
    return wrapped;
}

float ship_ai_firepower_lerp_clamped_00419010(float x0, float y0,
                                              float x1, float y1,
                                              float t) noexcept
{
    // 0041901E FUCOMIP / 00419026 JP: an exact equality returns y0 and never
    // divides.
    if (x0 == x1) {
        return y0;
    }
    const float u = (t - x0) / (x1 - x0);
    const float v = y0 + u * (y1 - y0);
    // 00419063..004190CE clamps into [min(y0,y1), max(y0,y1)].
    const float low = (y0 <= y1) ? y0 : y1;
    const float high = (y0 <= y1) ? y1 : y0;
    if (v > high) {
        return high;
    }
    if (v < low) {
        return low;
    }
    return v;
}

ShipAiFirepowerDamageBand ship_ai_firepower_damage_band(
    const ShipAiFirepowerProjectileClass& projectile) noexcept
{
    // 0095ED05..0095ED41 and 0095ED3B..0095ED71.
    ShipAiFirepowerDamageBand band;
    band.lower = projectile.blast_damage_min;
    if (band.lower < projectile.damage_min) {
        band.lower = projectile.damage_min;
    }
    band.upper = projectile.blast_damage_max;
    if (band.upper < projectile.damage_max) {
        band.upper = projectile.damage_max;
    }
    return band;
}

float ship_ai_firepower_kill_fraction(const ShipAiFirepowerDamageBand& band,
                                      float armour) noexcept
{
    // 0095ED75..0095ED9D: interp(lower -> 1.0f, upper -> 0.0f) at `armour`.
    return ship_ai_firepower_lerp_clamped_00419010(band.lower, 1.0f,
                                                   band.upper, 0.0f, armour);
}

float ship_ai_firepower_excess_damage(const ShipAiFirepowerDamageBand& band,
                                      float armour, float damage_cap) noexcept
{
    // 0095EE45 FCOMI / JBE, then one x87 chain per arm with a single float store
    // at 0095EE66.
    float excess;
    if (band.lower > armour) {
        excess = static_cast<float>(
            (static_cast<double>(band.lower) + static_cast<double>(band.upper))
                * kShipAiFirepowerHalf
            - static_cast<double>(armour));
    } else {
        excess = static_cast<float>(
            (static_cast<double>(band.upper) - static_cast<double>(armour))
            * kShipAiFirepowerHalf);
    }
    // 0095EE63..0095EE79: clamp(excess, 0.0f, query.damage_cap) in place.
    return clamp_in_place_00415690(excess, 0.0f, damage_cap);
}

float ship_ai_firepower_shot_rate(float window_seconds, float cycle_period) noexcept
{
    // 0095EE0F COMISS / 0095EE18 JBE.
    if (cycle_period > 0.0f) {
        return window_seconds / cycle_period;
    }
    return 1.0f;
}

float ship_ai_firepower_output_cap(float damage_cap, float window_seconds) noexcept
{
    // 0095EF90..0095EFC8. The divide is x87 against the 5.0 double and rounds to
    // float at 0095EF9B before the compare against 1.0f.
    float factor = static_cast<float>(static_cast<double>(window_seconds)
                                      / kShipAiFirepowerCapWindowDivisor);
    if (kShipAiFirepowerCapFloor > factor) {
        factor = kShipAiFirepowerCapFloor;
    }
    return static_cast<float>(static_cast<double>(damage_cap)
                              * static_cast<double>(factor));
}

float ship_ai_firepower_contribution(const ShipAiFirepowerQuery& query,
                                     const ShipAiFirepowerProjectileClass& projectile,
                                     const ShipAiFirepowerTickDamage& tick,
                                     float shots,
                                     float hit_probability,
                                     int rounds) noexcept
{
    const ShipAiFirepowerDamageBand band = ship_ai_firepower_damage_band(projectile);
    const float armour = (projectile.sub_type == kProjectileSubTypeTorpedo)
                             ? query.armour_torpedo
                             : query.armour;
    const float kill = ship_ai_firepower_kill_fraction(band, armour);
    const float excess = ship_ai_firepower_excess_damage(band, armour, query.damage_cap);

    // 0095EE29..0095EE39. FIMUL folds the round count in as an integer; the
    // product is stored to float at 0095EE39.
    const float product = static_cast<float>(
        static_cast<double>(shots) * static_cast<double>(hit_probability)
        * static_cast<double>(rounds) * static_cast<double>(kill));

    // 0095EE7E..0095EE8E, the flooding argument, stored to float.
    const float water_argument = static_cast<float>(
        static_cast<double>(projectile.water_damage) * static_cast<double>(product));

    // 0095EE92..0095EE96, the base term; the image keeps it as a double.
    const double base = static_cast<double>(product) * static_cast<double>(excess);

    // 0095EE9A..0095EEA9, the fire argument, stored to float.
    const float fire_argument = static_cast<float>(
        base * static_cast<double>(projectile.fire_chance)
        * static_cast<double>(projectile.fire_damage)
        / static_cast<double>(query.damage_threshold));

    // 0095EEBD..0095EED4: min against the window, scaled, added to the base.
    const float water_term = min_by_ref_00415510(query.window_seconds, water_argument);
    const float partial = static_cast<float>(
        static_cast<double>(water_term) * static_cast<double>(tick.water_tick_damage)
        + static_cast<double>(static_cast<float>(base)));

    // 0095EEE5..0095EEFA.
    const float fire_term = min_by_ref_00415510(query.window_seconds, fire_argument);
    return static_cast<float>(
        static_cast<double>(fire_term) * static_cast<double>(tick.fire_tick_damage)
        + static_cast<double>(partial));
}

bool ship_ai_firepower_category_enabled(const ShipAiFirepowerQuery& query,
                                        int category) noexcept
{
    // 0095EBD7..0095EC1E. A category that no gate names falls through every test
    // and is skipped, so 0, 5, 0Ah and 0Bh can never contribute.
    switch (category) {
    case 1:
        return query.allow_machine_gun != 0;
    case 2:
    case 3:
    case 4:
    case 6:
        return query.allow_artillery != 0;
    case 7:
        return query.allow_torpedo != 0;
    case 8:
    case 9:
        return query.allow_depth_charge != 0;
    default:
        return false;
    }
}

ShipAiFirepowerBucket ship_ai_firepower_bucket(int projectile_sub_type) noexcept
{
    // 0095EEF3..0095EF57, in the image's test order.
    if (projectile_sub_type == kProjectileSubTypeTorpedo) {
        return ShipAiFirepowerBucket::kTorpedo;
    }
    if (projectile_sub_type == kProjectileSubTypeDepthCharge) {
        return ShipAiFirepowerBucket::kDepthCharge;
    }
    if (projectile_sub_type == 4 || projectile_sub_type == 5
        || projectile_sub_type == 6 || projectile_sub_type == 7) {
        return ShipAiFirepowerBucket::kArtillery;
    }
    if (projectile_sub_type == 1 || projectile_sub_type == 2
        || projectile_sub_type == 3
        || projectile_sub_type == kProjectileSubTypeFlak) {
        return ShipAiFirepowerBucket::kSmallCalibre;
    }
    return ShipAiFirepowerBucket::kNone;
}

ShipAiFirepowerResult ship_ai_firepower_rating_0095eb40(ShipAiFirepowerQuery& query,
                                                        ShipAiFirepowerHost& host)
{
    ShipAiFirepowerResult result;

    // 0095EB4E..0095EB5D: the four outputs are zeroed before the early exit.
    query.out_artillery = 0.0f;
    query.out_small_calibre = 0.0f;
    query.out_torpedo = 0.0f;
    query.out_depth_charge = 0.0f;

    // 0095EB62..0095EB99: FCOMIP / JC, so the call returns 0 unless the range is
    // strictly inside the unit's overall weapon range.
    if (query.range >= host.unit_max_weapon_range()) {
        return result;
    }

    // 0095EB9E: LEA ECX,[EDI+14h] then 00605070 wraps word 5 in place.
    query.bearing = ship_ai_firepower_wrap_angle_00605070(query.bearing);

    float total = 0.0f;
    float artillery = 0.0f;
    float small_calibre = 0.0f;
    float torpedo = 0.0f;
    float depth_charge = 0.0f;

    for (int category = 0; category < kShipAiFirepowerCategoryCount; ++category) {
        if (host.category_device_count(category) == 0) { // 0095EBB3
            continue;
        }
        if (query.range > host.category_max_range(category)) { // 0095EBCB JA
            continue;
        }
        if (!ship_ai_firepower_category_enabled(query, category)) {
            continue;
        }

        for (NativeHandle node = host.category_list_head(category); // 0095EC24
             node != 0;
             node = host.list_next(node)) {                        // 0095EF6D
            const NativeHandle device = host.list_device(node);     // 0095EC3A

            if (!host.device_is_turning_gun(device)) { // 0095EC46, vtable[5Ch](22h)
                continue;
            }
            if (!host.device_is_operational(device)) { // 0095EC52, 00729F10
                continue;
            }

            int rounds;
            if (query.use_ready_rounds != 0) { // 0095EC5F
                // 0095EC65..0095EC74: a torpedo category asks for rounds ready
                // now, every other category for rounds ready inside the horizon.
                const float horizon =
                    (category == static_cast<int>(GunneryCategory::kTorpedo))
                        ? kShipAiFirepowerTorpedoReadyHorizon
                        : query.ready_horizon_seconds;
                rounds = host.device_ready_rounds(device, horizon); // 0095EC84
            } else {
                if (host.device_is_destroyed(device)) { // 0095EC8B
                    continue;
                }
                rounds = host.device_barrel_count(device); // 0095EC98
            }
            if (rounds == 0) { // 0095EC9E
                continue;
            }

            const int function = host.device_weapon_function(device); // 0095ECAA
            const NativeHandle ammo = host.device_ammo_record(device); // 0095ECB7

            // 0095ECB0..0095ECCC: the flak alternate, only for Function 6 with
            // the artillery gate clear and the machine-gun gate set.
            if (function == static_cast<int>(GunneryCategory::kLightArtilleryFlak)
                && query.allow_artillery == 0 && query.allow_machine_gun != 0) {
                host.ammo_select_flak_alternate(ammo); // 0095ECD4, 0095CF80
            }

            const ShipAiFirepowerProjectileClass projectile =
                host.ammo_projectile_class(ammo); // 0095ECDB

            if (query.range > projectile.max_range) { // 0095ECE9 JA
                continue;
            }

            const float armour =
                (projectile.sub_type == kProjectileSubTypeTorpedo) // 0095ECF3
                    ? query.armour_torpedo
                    : query.armour;
            const ShipAiFirepowerDamageBand band =
                ship_ai_firepower_damage_band(projectile);
            const float kill = ship_ai_firepower_kill_fraction(band, armour);
            if (!(kill > 0.0f)) { // 0095EDB0 JNC
                continue;
            }

            // 0095EDC9, 006EB060 with ECX = the projectile class.
            const float hit = host.weapon_hit_probability(projectile.handle,
                                                          query.range,
                                                          query.target_length);
            if (!(hit > 0.0f)) { // 0095EDDC JNC
                continue;
            }

            if (query.require_bearing != 0) { // 0095EDE2
                // 0095EDFA, 0085B7D0(this = device, projectile class, bearing, range).
                if (!host.device_can_bear(device, projectile.handle,
                                          query.bearing, query.range)) {
                    continue; // 0095EE01
                }
            }

            const float shots = ship_ai_firepower_shot_rate(
                query.window_seconds, host.ammo_cycle_period(ammo)); // 0095EE07

            const ShipAiFirepowerTickDamage tick = host.gameplay_tick_damage();

            const float contribution = ship_ai_firepower_contribution(
                query, projectile, tick, shots, hit, rounds);

            total += contribution; // 0095EF04
            switch (ship_ai_firepower_bucket(projectile.sub_type)) {
            case ShipAiFirepowerBucket::kTorpedo:
                torpedo += contribution; // 0095EF0E
                break;
            case ShipAiFirepowerBucket::kDepthCharge:
                depth_charge += contribution; // 0095EF1D
                break;
            case ShipAiFirepowerBucket::kArtillery:
                artillery += contribution; // 0095EF3B
                break;
            case ShipAiFirepowerBucket::kSmallCalibre:
                small_calibre += contribution; // 0095EF59
                break;
            case ShipAiFirepowerBucket::kNone:
                break; // 0095EF63 FSTP ST0
            }
        }
    }

    // 0095EF90..0095F077: one cap for every output and for the return value.
    const float cap = ship_ai_firepower_output_cap(query.damage_cap, query.window_seconds);

    query.out_small_calibre = (cap <= small_calibre) ? cap : small_calibre; // 0095EFEF
    query.out_artillery = (cap <= artillery) ? cap : artillery;             // 0095F00B
    query.out_depth_charge = (cap <= depth_charge) ? cap : depth_charge;    // 0095F027
    query.out_torpedo = (cap <= torpedo) ? cap : torpedo;                   // 0095F043

    result.total = (cap <= total) ? cap : total; // 0095F04A
    result.artillery = query.out_artillery;
    result.small_calibre = query.out_small_calibre;
    result.torpedo = query.out_torpedo;
    result.depth_charge = query.out_depth_charge;
    return result;
}

void ship_ai_firepower_range_profile_0095f080(ShipAiFirepowerQuery query,
                                              float out[kShipAiFirepowerProfileSamples],
                                              bool prefer_long_range,
                                              ShipAiFirepowerHost& host)
{
    // 0095F083..0095F0E2: the same wrap as 00605070, inlined by the compiler.
    query.bearing = ship_ai_firepower_wrap_angle_00605070(query.bearing);

    // 0095F087: the sweep always starts at 50 m, whatever the caller's word 0.
    query.range = kShipAiFirepowerProfileFirstRange;

    int remaining = kShipAiFirepowerProfileSamples; // EDI, 0095F0F0
    int index = 0;                                  // ESI, 0095F0F5

    do {
        float score = ship_ai_firepower_rating_0095eb40(query, host).total; // 0095F109

        if (prefer_long_range) {
            // 0095F11A FISUB: the remaining sample count, as an integer.
            score = static_cast<float>(static_cast<double>(score)
                                       - static_cast<double>(remaining));
        }

        // 0095F122..0095F134: the store index is clamped into [0, 59].
        int slot = index;
        if (slot < 0) {
            slot = 0;
        } else if (slot > kShipAiFirepowerProfileSamples - 1) {
            slot = kShipAiFirepowerProfileSamples - 1;
        }
        out[slot] = score; // 0095F152

        // 0095F136..0095F14E: the next sample is 50 m further out.
        query.range = static_cast<float>(static_cast<double>(query.range)
                                         + kShipAiFirepowerProfileStep);
        --remaining;
        ++index;
    } while (remaining > 0); // 0095F15C
}

}  // namespace bsp
