#pragma once
// Screen 50h, the in-mission warning screen (GUI_Warning): its update
// 00683020 (vtable 00CF7184 +20h, __thiscall(screen, float dt), RET 4, body
// 00683020..006832E9). Packet cc9_screen_50h, docs/SHIP_SCREEN_UPDATE.md
// section 16. Names are descriptive hypotheses, not recovered symbols. A new
// C++ interface over the screen's fields; not ABI-compatible.
//
// Four alerts, each with a flag, a hold timer and a pulse phase:
//   0 stall (a plane, 007C6E10 on [unit+3D0h])       00682ED0 "ingame.warning_stall"
//   1 oxygen (a submarine, unit+127Ch below 0.2)     00682CB0 "ingame.warning_o2"
//   2 shallow water (a ship, byte unit+1011h)         00682D60 "ingame.warning_shallowwater"
//   3 exit zone (00681F40 near the world edge)       00682E10 "ingame.warning_exitezone"

#include <cstddef>

namespace bsp {

struct WarningScreenState {
    float dt_78{0.0f};             // +78h, the update's dt
    float hold[4]{};               // +8h..+14h, set to 1.0 when an alert's test holds
    float phase[4]{};              // +18h..+24h, the pulse phase each show advances
    bool active[4]{};              // +28h..+2Bh
    bool held_2c{false};           // +2Ch, cleared by the register 00682740
    bool sound[4]{};               // +40h..+4Ch non-null: the tracked sound 00682800 made
    std::size_t last_unit_74{0};   // +74h, the controlled unit last frame (index + 1; 0 none)
};

enum class WarningWidget : int {
    Text = 0x50,        // warning_text
    Icon1 = 0x54,       // warning_1_Icon
    Icon2 = 0x58,       // warning_2_Icon
    FirstGroup = 0x68,  // first_Group
};

struct WarningScreenHost {
    virtual ~WarningScreenHost() = default;
    // The controlled unit 00E188D8.
    virtual std::size_t controlled_unit() = 0;        // index + 1, 0 when none
    virtual bool controlled_flag_5d() = 0;            // byte unit+5Dh
    virtual bool controlled_is_kind_of(int class_id) = 0;   // vtable +5Ch
    virtual bool plane_stall_007c6e10() = 0;          // 007C6E10([unit+3D0h]) != 0
    virtual bool ship_shallow_1011() = 0;             // byte unit+1011h
    virtual float submarine_127c() = 0;               // unit+127Ch
    virtual bool submarine_below_00852860() = 0;      // 00852860(unit)
    virtual bool pose_current_c8() = 0;               // byte unit+C8h
    virtual void refresh_pose_00414db0() = 0;
    // 00681F40([00E188A8], &unit+FCh, 0.0): the position near the world edge.
    virtual bool near_world_edge_00681f40() = 0;
    // Widgets.
    virtual void set_visible(WarningWidget widget, bool visible) = 0;   // +34h
    virtual void set_alpha(WarningWidget widget, float alpha) = 0;      // +4Ch
    // The tracked sound at +40h+4i: its virtual +8(stop flag), its +Ch
    // "finished" test, and the release (InterlockedDecrement on +4h, delete).
    virtual void sound_stop(int alert, bool flag) = 0;
    virtual bool sound_finished(int alert) = 0;
    virtual void sound_release(int alert) = 0;
    // The four show routines; each advances its phase, shows first_Group,
    // sets the text, pulses the alphas and starts the alert's sound.
    virtual void show_alert(WarningScreenState& screen, int alert) = 0;
};

// 00683020.
void warning_screen_update_00683020(WarningScreenState& screen, WarningScreenHost& host,
                                    float dt);

}  // namespace bsp
