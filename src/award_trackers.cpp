#include "bsp/award_trackers.hpp"

namespace bsp {

// 00692c3d, 00692cf0 and 00692d62 in native order: submarine first, then the
// two plane categories share one identifier, then the ship category.
const CategoryHint kBasicCategoryHints[3] = {
    { kUnitCategorySubmarine, "BASICSUB" },  // 00CEE384
    { kUnitCategoryPlane, "BASICPLANE" },    // 00CEE3C0, also category 18h
    { kUnitCategoryShip, "BASICSHIP" },      // 00CEE39C
};

// The compare chain at 00692e4b..00692f72, in the order the native tests it.
// Class 7 and class 0Ch carry no direct name; unit_class_hint_00692b60 splits
// them. Any id outside this table records nothing.
const UnitClassHint kUnitClassHints[15] = {
    { 0x00, "MOTHERSHIP" },       // 00CF7E38
    { 0x01, "DESTROYER" },        // 00CF7E2C
    { 0x02, "PT" },               // 00CF7E28
    { 0x0D, "SUBMARINE" },        // 00CF7E1C, tested third
    { 0x03, "BATTLESHIP" },       // 00CF7E10
    { 0x04, "CRUISER" },          // 00CF7E08
    { 0x05, "TROOP_TRANSPORT" },  // 00CF7DF8
    { 0x06, "LST" },              // 00CF7DF4
    { 0x07, nullptr },            // splits on 007edad0
    { 0x08, "DIVEBOMBER" },       // 00CF7DCC
    { 0x09, "TORPEDOBOMBER" },    // 00CF7DBC
    { 0x0A, "FIGHTER" },          // 00CF7DB4
    { 0x0B, "RECONPLANE" },       // 00CF7DA8
    { 0x0C, nullptr },            // splits on the ohka byte
    { 0x0F, "COMMANDBUILDING" },  // 00CF7D84
};

const char kCaptureFirstGetHint[] = "CAPTURE1STGET";
const char kLandingFirstGetHint[] = "LANDING1STGET";
const char kStrategicMapFirstGetHint[] = "SM1STGET";

std::size_t tick_hint_cooldowns_0068ec10(HintCooldown* entries, std::size_t count,
    float raw_delta) noexcept {
    if (entries == nullptr) {
        return 0;
    }
    // 0068eca6..0068ecbc: FLD value, FSUB delta, store back through the mapped
    // value accessor 00444be0. The subtraction is unconditional, so an entry
    // already below zero keeps sinking on the frame it is collected.
    std::size_t kept = 0;
    for (std::size_t i = 0; i < count; ++i) {
        entries[i].remaining -= raw_delta;
        // 0068ecc8: COMISS 0.0, value / JBE skip. Strictly negative expires.
        if (!(entries[i].remaining < 0.0f)) {
            entries[kept] = entries[i];
            ++kept;
        }
    }
    return kept;
}

bool should_drain_queued_hint_00692b00(const QueuedHintDrainInputs& in) noexcept {
    // 00692b0e, 00692b23, 00692b27, 00692b2d in that order.
    if (in.game_mode == kAwardTrackerSuppressedGameMode) {
        return false;
    }
    if (in.transition_blend_active) {
        return false;
    }
    if (in.queue_size == 0) {
        return false;
    }
    return !in.hint_active;
}

const char* unit_class_hint_00692b60(const UnitClassHintInputs& in) noexcept {
    if (in.class_id == kUnitClassLevelBomber) {
        // 00692ecf..00692f04. Two separate 007edad0 calls in the native, both
        // on the same object, so one reading is enough here.
        if (in.payload_kind == kPayloadParatrooper) {
            return "PARATROOPER"; // 00CF7D40
        }
        if (in.payload_kind == kPayloadOhka) {
            return "OHKA_PAYLOAD"; // 00CF7DE4
        }
        return "LEVELBOMBER"; // 00CF7DD8
    }
    if (in.class_id == kUnitClassKamikaze) {
        // 00692f40..00692f66.
        return in.ohka_variant ? "OHKA" : "KAMIKAZE"; // 00CF7DA0 / 00CF7D94
    }
    for (std::size_t i = 0; i < kUnitClassHintCount; ++i) {
        if (kUnitClassHints[i].class_id == in.class_id) {
            return kUnitClassHints[i].name;
        }
    }
    return nullptr;
}

const char* surface_weapon_hint_006926f0(int weapon_mode) noexcept {
    // The jump table at 006928e3. Modes 1 and 2 share one identifier.
    switch (weapon_mode) {
    case 1:
    case 2:
        return "AAFLAK";
    case 3:
        return "ARTILLERY";
    case 4:
        return "TORPEDO_SHIP";
    case 5:
        return "DC_SHIP";
    default:
        return nullptr;
    }
}

const char* air_weapon_hint_006926f0(int projectile_kind, bool machinegun_ready) noexcept {
    // 00692840..006928d5, an if chain rather than a table.
    if (projectile_kind == kProjectileBomb) {
        return "BOMB";
    }
    if (projectile_kind == kProjectileTorpedo) {
        return "TORPEDO_PLANE";
    }
    if (projectile_kind == kProjectileRocket) {
        return "ROCKET";
    }
    if (projectile_kind == kProjectileDepthCharge) {
        return "DC_PLANE";
    }
    if (projectile_kind == kProjectileParatrooperA || projectile_kind == kProjectileParatrooperB) {
        return "PARATROOPER";
    }
    // The fallback needs the byte at unit+C24h; without it the pass records
    // nothing and still calls 00690fd0 with the empty string it built at entry.
    return machinegun_ready ? "MACHINEGUN" : nullptr;
}

const char* environment_hint_00692580(const EnvironmentHintInputs& in) noexcept {
    // 00692669..00692694. Both scalars pass through _ftol first, so anything
    // that truncates to zero does not trip the test.
    if (static_cast<int>(in.water_amount) != 0) {
        return "WATER"; // 00CF7D00
    }
    if (static_cast<int>(in.fire_amount) != 0) {
        return "FIRE"; // 00CF7CF8
    }
    if (in.submerged) {
        return "PERISCOPE"; // 00CF7CEC
    }
    if (in.engine_signal) {
        return "ENGINE"; // 00CF7CE4
    }
    return nullptr;
}

ZoneProximity scan_capture_zones_00692fd0(float px, float py, float pz,
    const CaptureZoneSample* zones, std::size_t count) noexcept {
    ZoneProximity result;
    if (zones == nullptr) {
        return result;
    }
    for (std::size_t i = 0; i < count; ++i) {
        const CaptureZoneSample& zone = zones[i];
        // 0069314f..00693175: each difference is computed and stored as a
        // float, then squared and summed on the x87 stack. The accumulation is
        // done in double here to stay closer to that extended-precision sum.
        const float dx = px - zone.x;
        const float dy = py - zone.y;
        const float dz = pz - zone.z;
        const double distance_squared = static_cast<double>(dz) * dz
            + static_cast<double>(dx) * dx + static_cast<double>(dy) * dy;

        // 0069317f: FILD the int radius, then FMUL ST0,ST0.
        const double landing = static_cast<double>(zone.landing_radius);
        if (!(distance_squared <= landing * landing)) {
            continue;
        }
        if (zone.state != kCaptureZoneActiveState) {
            continue;
        }
        result.in_landing_radius = true;

        const double capture = static_cast<double>(zone.capture_radius);
        if (capture * capture < distance_squared) {
            // 00693282 breaks out of the whole walk, it does not advance to the
            // next zone. Any zone after this one is never examined.
            break;
        }
        result.in_capture_radius = true;
    }
    return result;
}

bool should_show_strategic_map_hint_00692960(const StrategicMapHintInputs& in) noexcept {
    // 0069298e, 006929a7, 006929dd, 006929f2, 00692a06, 00692a0c in order.
    if (in.game_mode == kAwardTrackerSuppressedGameMode) {
        return false;
    }
    if (in.transition_blend_active) {
        return false;
    }
    if (in.already_shown) {
        return false;
    }
    if (!in.map_available) {
        return false;
    }
    if (in.map_open) {
        return false;
    }
    return !in.hint_active;
}

void run_award_tracker_frame(AwardTrackerFrameHost& host, float raw_delta) {
    // 004e525e..004e52b5. Seven getter calls, each followed by mov ecx,eax and
    // one update. The order is the native order and nothing between them reads
    // a result, so the getter calls are the only shared step.
    host.resolve_singleton();
    host.tick_cooldowns(raw_delta);
    host.resolve_singleton();
    host.drain_queued_hint();
    host.resolve_singleton();
    host.update_unit_class_hint();
    host.resolve_singleton();
    host.update_weapon_hint();
    host.resolve_singleton();
    host.update_environment_hint();
    host.resolve_singleton();
    host.update_zone_first_get_hints();
    host.resolve_singleton();
    host.update_strategic_map_first_get_hint();
}
}
