#pragma once
// Packet cc9_avoid_zone_draft_bodies (docs/GUNNERY_OPEN_ITEMS.md section 109). The static
// "glass wall" bodies the tail of 00424D00 builds at mission load: 00424DDF..00425487 reduces
// the nine AvoidZoneDepths class depths to {layer, class mask} pairs
// (avoid_zone_draft_layers_00424dfb) and calls 00423C50 for each pair, which takes the exact
// layer group, partitions each zone polygon (004F6F20), extrudes each piece to +-500 m
// (00423D45..0042414E), builds the 00C5DEB0 hull, drops a hull with fewer than four vertices
// (0042415F..0042416B) and creates one static Dyn body per piece (00424472): one kind-4 shape
// with restitution 0, friction 0, group = the class mask, mask 0, an identity local frame and
// the body at (meanX, 0, meanZ).
//
// Not the native manager layout: the bodies are returned in creation order (the manager's +5Ch
// vector) with the retained hull (+6Ch list node +8h) held by a shared handle. Names are
// hypotheses.
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_draft_layers.hpp"
#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {

struct AvoidZoneDraftBody {
    std::int32_t layer{0};
    std::uint32_t group{0};        // shape +2Ch from descriptor +08h: the pair's class mask
    std::size_t zone{0}, piece{0};
    float frame[12]{};             // the body 3x4: identity rows, translation (meanX, 0, meanZ)
    std::shared_ptr<AvoidZoneDynHullHandle> hull;   // the retained copy, descriptor +14h
};

struct AvoidZoneDraftBuildCensus {
    std::size_t pairs{0}, groups_missing{0}, zones{0}, zones_short{0}, pieces{0},
        pieces_short{0}, hulls_rejected{0}, bodies{0};
};

// The exact layer group's native zones, in group order (00423CA8: group +4h/+8h).
using AvoidZoneDraftGroupZones = std::function<AvoidZoneClearanceGroupView(std::size_t group)>;

std::vector<AvoidZoneDraftBody> avoid_zone_draft_bodies_00424ddf(const AvoidZoneTable& table,
    const AvoidZoneDraftGroupZones& zones, const AvoidZoneDraftDepthInputs& depths,
    const CameraAxesCrtAccess& crt, AvoidZoneDraftBuildCensus& census);

// 009394E5..009395E2 (the hull build 00937C90) and 0092BD70: the class bit a hull shape's mask
// carries beside 0Dh, from the class kind [[ctl+1Ch]+538h]->vtable[1Ch]() through the jump
// table 00939C90 (kinds 7..0Eh) and the class byte +808h (BigLandingShip / HeavyCruiser).
// Any other kind gives 0.
std::uint32_t hull_class_bit_009394fb(int kind, bool class_byte_808) noexcept;

}  // namespace bsp
