#pragma once

// cc13_devwalk: 007C0D90 BSP_Plane_TickReleaseOrderIssue, the plane's release-order device walk,
// per rack in list order (packet cc13-devwalk, docs/CC13_DEVWALK.md). Read from the disk bytes
// 007C0D90-007C0F0D. Names are hypotheses, not recovered symbols.
//
// The image, __thiscall(ECX = unit), plain RET:
//   for each child on unit+48h (next +44h):
//     skip unless IsKindOf(25h) (vt+5Ch, 007C0DBF), it holds 2Ah (vt+210h(2Ah, 0), 007C0DD7)
//     and it is not busy (vt+1FCh, 007C0DEB);
//     desc = vt+220h(0) (007C0E01); fire it: vt+1F0h(delay so far) (007C0E17); found = 1;
//     a plane that is not IsKindOf(10h) (007C0E28), or a desc answering 2Ch, 2Bh or 33h
//     (007C0E3B / 007C0E4E / 007C0E61), leaves the walk at once (007C0EE0);
//     otherwise delay += U(lo, hi) (00BD2F10, ECX = 1, 007C0E88) * class+1F4h BombDelay, and a
//     desc answering 31h gets the target point 006E3F90 (007C0EC9); next child.
//   007C0ED9: nothing fired -> return. 007C0EE2: unit+C25h = 1. 007C0EEB: the LAST fired
//   desc answering 33h (a rocket) -> return; otherwise 007EEF30(ECX = [unit+9D4h], unit),
//   the release orders to the flight.

#include <cstddef>
#include <vector>

namespace bsp::cc13_devwalk {

// The behaviour switch. True: the units host walks its racks in list order and the FIRST
// eligible rack decides (a plane with bombs and rockets is decided by its first rack, not by
// the plane-wide ordnance mask); a rocket (33h) rack is fired and raises unit+C25h but issues no
// release orders (007C0EEB). False: the plane-wide mask decides, as before.
// With this switch ON, the values of the switches it bypasses are ignored: the walk replaces
// the bodies kPlaneLevelBombTaskBound's level branch, kLevelBomberIssueTailBound,
// kPlaneDeviceWalkBound, kReleaseIssueStageBound / kReleaseIssueStageValsBound and
// kRackBulletKindBound shape inside run_release_order_issue_007c0d90 (the level-bomber rack
// tick still follows lb_level_bomber_racks()).
// Single-class bombers are not moved by the per-rack decision itself, but an issue tick that
// meets a busy rack (vt+1FCh, dropBombs still set) now skips it and fires the next, faithful
// to 007C0DEB, where the mask path fired nothing new; that can move them.
inline constexpr bool kPlaneDeviceWalkPerRackBound = true;   // ON: OCEAN_WAVE_FIELD 97 (main cff560b45), physics23 pairs

// The draw sub-switch. True: the issue path's stream-1 draws are taken from the shared
// stream-1 generator (GameGunneryHost::death_mode_draw_00bd2f10(1, ...)), in the image's call
// order: the level bomber's next-rack delay draws (007C0E67-007C0E88), then the next-issue
// interval C28h (007CEA9A-007CEABE, under kReleaseIssueIntervalDrawBound). These are the
// only 00BD2F10 calls on the path (007CEA8D 007C0D90 through 007EEF30 and 006E3550, direct
// calls to depth 6; descriptor vtable calls are not followed). False: the units host's own
// release sequence (`#lbrk` and `#issue` keys), as before.
inline constexpr bool kPlaneDeviceWalkSharedDrawBound = false;

// 007C0E67 [00CE3860] and [00CE6448], the U(lo, hi) bounds of the next-rack delay draw.
inline constexpr float kRackDelayDrawLow = 0.9f;
inline constexpr float kRackDelayDrawHigh = 1.1f;

// Ordnance class ids (docs/ORDNANCE_KIND_IDENTITY.md).
inline constexpr int kOrdnanceDepthCharge = 0x2C;
inline constexpr int kOrdnanceTorpedo = 0x2B;
inline constexpr int kOrdnanceParatrooper = 0x31;
inline constexpr int kOrdnanceRocket = 0x33;

// Whether a round of this class answers 2Ah (vt+210h(2Ah, 0)): every droppable descriptor
// in the table (Bomb, Torpedo, DepthCharge, DummyTarget, DummyKamikazePlane, DummySubmarine,
// Paratrooper, Rocket, WaterMine).
inline bool ordnance_class_answers_2ah(int bullet_class) {
    switch (bullet_class) {
    case 0x2A: case 0x2B: case 0x2C: case 0x2E: case 0x2F: case 0x30: case 0x31: case 0x33:
    case 0x34:
        return true;
    default:
        return false;
    }
}

struct DeviceWalkRack {
    int bullet_class{0};   // the round's class id (desc vt+8 answers it)
    bool holds_round{false};   // vt+210h(2Ah, 0) with a round loaded
    bool busy{false};          // vt+1FCh
};

struct DeviceWalkFire {
    std::size_t rack{0};
    float delay{0.0f};         // the vt+1F0h argument
    bool target_point{false};  // 007C0EC9 006E3F90 (desc answers 31h)
};

struct DeviceWalkResult {
    std::vector<DeviceWalkFire> fired;
    bool found{false};             // 007C0E23
    bool raise_c25{false};         // 007C0EE2
    bool issue_orders{false};      // 007C0F01 007EEF30
    int last_fired_class{0};       // EDI at 007C0EE0
    int draws{0};                  // 007C0E88 calls
    float delay_after{0.0f};       // [ESP+0Ch] at 007C0ED9, after the last draw
};

// The walk. `level_bomber` is the owner's IsKindOf(10h); `draw()` returns one U(lo, hi) draw
// (stream 1) and `bomb_delay` is class+1F4h.
template <typename Draw>
DeviceWalkResult device_walk_007c0d90(const std::vector<DeviceWalkRack>& racks,
                                      bool level_bomber, float bomb_delay, Draw&& draw) {
    DeviceWalkResult out;
    float delay = 0.0f;   // [ESP+0Ch] = 0.0f (007C0DA4)
    for (std::size_t i = 0; i < racks.size(); ++i) {
        const DeviceWalkRack& r = racks[i];
        if (!r.holds_round || !ordnance_class_answers_2ah(r.bullet_class) || r.busy) continue;
        DeviceWalkFire f;
        f.rack = i;
        f.delay = delay;
        out.found = true;
        out.last_fired_class = r.bullet_class;
        if (!level_bomber || r.bullet_class == kOrdnanceDepthCharge ||
            r.bullet_class == kOrdnanceTorpedo || r.bullet_class == kOrdnanceRocket) {
            out.fired.push_back(f);
            break;   // 007C0E2C / 007C0E3F / 007C0E52 / 007C0E65 -> 007C0EE0
        }
        // 007C0E67-007C0EA0: the draw comes before the 31h test, so every level-bomber rack
        // that stays in the walk takes one.
        // FMUL [class+1F4h] / FADD [ESP+18h] / FSTP. The main thread runs x87 at 24-bit
        // precision control, so the product and the sum each round to float, as here.
        delay = delay + draw() * bomb_delay;
        ++out.draws;
        f.target_point = r.bullet_class == kOrdnanceParatrooper;
        out.fired.push_back(f);
    }
    out.delay_after = delay;
    if (!out.found) return out;   // 007C0EDE
    out.raise_c25 = true;
    out.issue_orders = out.last_fired_class != kOrdnanceRocket;
    return out;
}

}  // namespace bsp::cc13_devwalk
