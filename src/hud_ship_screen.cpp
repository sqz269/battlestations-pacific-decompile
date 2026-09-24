#include "bsp/hud_ship_screen.hpp"

#include <cmath>

// Packet cc9_ship_screen_update, part 1. docs/SHIP_SCREEN_UPDATE.md.

namespace bsp {
namespace {

constexpr int kKindSubmarine = 8;                    // 0064A972 PUSH 8
constexpr float kStickBackLimit = -0.5f;             // 00CE69D0
constexpr float kStickFullAngle = 2.356194496154785f;   // 00E08CAC
constexpr float kStickBackAngle = -1.1780972480773926f; // 00CF5C68
constexpr double kStickBackScale = 1.1780972480773926;  // 00CF5C60
constexpr double kStickOffset = 0.5;                 // 00D7A280
constexpr double kStickRange = 1.5;                  // 00CE3D78
constexpr float kStickSnap = 0.0010000000474974513f; // 00D7A23C
constexpr double kStickEaseDivisor = 3.0;            // 00D7A2B0
constexpr float kStickEaseArg = 0.10000000149011612f;   // 00CF5C1C
constexpr double kFlashDecayRate = 4.0;              // 00D7A328
constexpr double kFlashHideBelow = 0.05000000074505806; // 00D7A270

const ShipScreenWidget kFlashWidgets[4] = {ShipScreenWidget::Flash0, ShipScreenWidget::Flash1,
                                           ShipScreenWidget::Flash2, ShipScreenWidget::Flash3};

}  // namespace

void ship_screen_relation_icon_0064a960(ShipScreenState& screen, ShipScreenHost& host) {
    if (!screen.has_unit) return;                                   // 0064A96B
    if (!host.unit_is_kind_of(kKindSubmarine) && host.unit_in_formation()) {  // 0064A974, 0064A980
        host.set_visible(ShipScreenWidget::Relation, true);         // 0064A998
        // 0064A99A..0064A9CB: +88h(state, 0, 1.0f), state = (leader != unit).
        host.select_state(ShipScreenWidget::Relation, host.unit_leads_formation() ? 0 : 1, 0,
                          1.0f);
        return;
    }
    host.set_visible(ShipScreenWidget::Relation, false);            // 0064A9DE
}

void ship_screen_flashes_0064abd0(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    // 0064ABDD: a clear enable zeroes all four intensities first.
    if (!screen.flash_enabled_1ac) {
        for (float& v : screen.flash_intensity) v = 0.0f;
    }
    // 0064ABFA..0064AC0C: one factor, float-stored.
    const float factor = static_cast<float>(1.0 - static_cast<double>(dt) * kFlashDecayRate);
    for (float& v : screen.flash_intensity) v = v * factor;         // 0064AC10..0064AC3F
    for (int i = 0; i < 4; ++i) {
        const float v = screen.flash_intensity[i];
        if (kFlashHideBelow > static_cast<double>(v)) {             // 0064AC42 and siblings
            host.set_visible(kFlashWidgets[i], false);
            continue;
        }
        host.set_alpha(kFlashWidgets[i], v > 1.0f ? 1.0f : v);      // widget +4Ch
        host.set_visible(kFlashWidgets[i], true);
    }
}

void ship_screen_stick_0064a9f0(ShipScreenHost& host, float ease) {
    const float t = host.unit_throttle();                          // 0064A9F9
    float angle;
    if (kStickBackLimit > t) {                                      // 0064AA09 JA
        angle = kStickBackAngle;
    } else if (t > 1.0f) {                                          // 0064AA1E JA
        angle = kStickFullAngle;
    } else if (t == 0.0f) {
        angle = 0.0f;
    } else if (t == 1.0f) {
        angle = kStickFullAngle;
    } else if (t == kStickBackLimit) {
        angle = kStickBackAngle;
    } else if (t > 0.0f) {
        angle = static_cast<float>(static_cast<double>(kStickFullAngle) * t);   // 0064AA68
    } else {
        const double scaled = static_cast<double>(t) * kStickBackScale;       // 0064AA73
        angle = static_cast<float>(scaled + scaled);
    }
    if (ease != 0.0f) {                                             // 0064AA87..0064AA8E
        const float current = host.rotation(ShipScreenWidget::Stick);   // [+48h]+48h
        angle = static_cast<float>((static_cast<double>(angle) - current) * ease + current);
    }
    host.set_rotation(ShipScreenWidget::Stick, angle);              // 0064AABF, +44h
}

float ship_screen_ease_stick_0064e358(float current, float throttle) noexcept {
    // 0064E358..0064E39F: clamp((throttle + 0.5) / 1.5, 0, 1) through 00415620.
    float target = static_cast<float>((static_cast<double>(throttle) + kStickOffset) / kStickRange);
    if (target < 0.0f) target = 0.0f;
    if (target > 1.0f) target = 1.0f;
    if (current == target) return current;                           // 0064E3B8 FUCOMIP
    const float gap = std::fabs(static_cast<float>(static_cast<double>(target) - current));
    if (gap < kStickSnap) return target;                             // 0064E3E2
    return static_cast<float>((static_cast<double>(target) - current) / kStickEaseDivisor +
                              current);                               // 0064E3E4..0064E3EC
}

void ship_screen_update_0064dd30(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    if (screen.slide_pending_119) host.relation_slide_block();        // 0064DD5F
    host.pipe_sight_block(dt);                                        // 0064DE92
    ship_screen_relation_icon_0064a960(screen, host);                 // 0064E30E
    screen.flash_enabled_1ac = host.flash_view_mode_is_1();           // 0064E31A..0064E335
    ship_screen_flashes_0064abd0(screen, host, dt);                   // 0064E33B
    if (!screen.active_05 || !screen.has_unit) return;                // 0064E340..0064E352
    screen.stick_44 = ship_screen_ease_stick_0064e358(screen.stick_44, host.unit_throttle());
    ship_screen_stick_0064a9f0(host, kStickEaseArg);                  // 0064E410
    host.remainder_from_0064e415(dt);                                 // 0064E415
}

}  // namespace bsp
