#pragma once
// Screen 45h, the ship status and repair-crew HUD: its update 0064DD30
// (vtable 00CF5E30 +20h). Packet cc9_ship_screen_update,
// docs/SHIP_SCREEN_UPDATE.md. Names are descriptive hypotheses, not recovered
// symbols. New C++ interfaces over the screen's fields; not ABI-compatible.
//
// Part 1 covers the top-level gates and three block groups:
//   0064E30C 0064A960   the relation icon (+F8h)
//   0064E313 0064ABD0   the four flash icons at +18Ch
//   0064E340..0064E410  the throttle stick: +44h and 0064A9F0 on +48h
// Every other block of 0064DD30 is a host record in part 1.

#include <cstddef>
#include <cstdint>

namespace bsp {

// The screen's own fields part 1 reads and writes.
struct ShipScreenState {
    bool active_05{false};        // +5h, the registry's applied byte
    bool has_unit{false};         // +184h non-null
    bool slide_pending_119{false};// +119h, cleared by the enter (0064BE70); set only by
                                  // 0064A902, which part 1 does not reach
    float stick_44{-1.0f};        // +44h, the eased throttle; the enter stores -1.0 (0064BD7C)
    // +18Ch..+1ACh, the flash group 0064ABD0 walks.
    float flash_intensity[4]{};   // +19Ch..+1A8h
    bool flash_enabled_1ac{false};// +1ACh
};

// The widgets part 1 drives, by the screen offsets 0064C0F0 fills.
enum class ShipScreenWidget : int {
    Stick = 0x48,          // ship_stick_Icon
    Relation = 0xF8,       // ship_relation_Icon
    Flash0 = 0x18C,        // VillanasFelso..Jobb, in 0064C0F0's order
    Flash1 = 0x190,
    Flash2 = 0x194,
    Flash3 = 0x198,
};

struct ShipScreenHost {
    virtual ~ShipScreenHost() = default;
    // Widget virtuals.
    virtual void set_visible(ShipScreenWidget widget, bool visible) = 0;      // +34h
    virtual void set_rotation(ShipScreenWidget widget, float radians) = 0;   // +44h
    virtual float rotation(ShipScreenWidget widget) = 0;                     // widget+48h
    virtual void set_alpha(ShipScreenWidget widget, float alpha) = 0;        // +4Ch
    virtual void select_state(ShipScreenWidget widget, int state, int zero,
                              float one) = 0;                                // +88h
    // The unit at +184h.
    virtual bool unit_is_kind_of(int class_id) = 0;   // vtable +5Ch
    virtual bool unit_in_formation() = 0;             // [unit+284h] != 0
    virtual bool unit_leads_formation() = 0;          // 007788D0(unit) == unit
    virtual float unit_throttle() = 0;                // unit+980h
    // [[00E198C4]+4Ch]+30h == 1, the flash group's enable.
    virtual bool flash_view_mode_is_1() = 0;
    // The blocks part 1 does not bind, in listing order.
    virtual void relation_slide_block() = 0;          // 0064DD5F..0064DE90
    virtual void pipe_sight_block(float dt) = 0;      // 0064DE92..0064E307
    virtual void remainder_from_0064e415(float dt) = 0; // 0064E415..006500C1
};

// 0064A960, __thiscall(screen), RET.
void ship_screen_relation_icon_0064a960(ShipScreenState& screen, ShipScreenHost& host);
// 0064ABD0, __thiscall(flash group = screen+18Ch, float dt), RET 4.
void ship_screen_flashes_0064abd0(ShipScreenState& screen, ShipScreenHost& host, float dt);
// 0064A9F0, __thiscall(screen, float ease), RET 4.
void ship_screen_stick_0064a9f0(ShipScreenHost& host, float ease);
// 0064E358..0064E402: the eased throttle at +44h.
float ship_screen_ease_stick_0064e358(float current, float throttle) noexcept;
// 0064DD30, part 1.
void ship_screen_update_0064dd30(ShipScreenState& screen, ShipScreenHost& host, float dt);

} // namespace bsp
