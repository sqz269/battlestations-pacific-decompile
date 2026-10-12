#pragma once
// The ocean's coverage regions (field+620h, count +624h): the wave-damp masks
// each landscape model carries. Packet cc9_physics1_sea,
// docs/OCEAN_WAVE_FIELD.md section 10. Names are hypotheses.
//
// Chain in the image:
//   * BSP_Landscape_LoadTerrain 00882AC0 at 008830AA calls 00BA1370
//     (`void __thiscall(ocean, node)`, RET 4) with ECX = [world+A8h] and the
//     terrain's render node +3CCh; 00BA1370 calls 00BA0C00(node) and appends
//     the node to ocean+5F4h.
//   * 00BA0C00 (`void __stdcall(node)`, ocean in EDI) walks the node's
//     geometry: every draw section whose effect name, after its last '/'
//     (00CE7898), contains "oceanwavedamp" yields one region. Its box is the
//     min/max x and z over ALL vertices of the section's vertex stream
//     (004768D0 per vertex, 00BA0F20..00BA0FB6; start 1e10 / -1e10 from
//     00CE4970 / 00CE4ADC), then shifted by the node's world translation
//     [node+120h] / [node+128h] (no rotation), scales 1/(x1-x0) and
//     1/(z1-z0) (00BA107B..00BA1099). The mask is the material's first
//     texture (+10h when the short +34h is >= 1, else none) read by 00B9E170.
//     Then it recurses into the node's children (+34h, next sibling +3Ch).
//   * 00B9E170 halves the texture's size until it is <= 80h, counting the
//     level; it refuses a zero size or a level past the texture's level count
//     (log 00D63960 / 00D6392C). It copies that level into a 1-level
//     A8R8G8B8 SYSTEMMEM texture through 00B3CD70 =
//     D3DXLoadSurfaceFromSurface(filter NONE), keeps byte 2 (red) of every
//     pixel into a w*h byte vector, and stores float(w) at +28h, float(h) at
//     +2Ch.
//
// Host representation (not the native layout or ABI):
//   * The model is the landscape's `models/terrain/<FilePath>.mmod`; each
//     Hierarchy item with a damp section yields its regions. The translation
//     added is always the landscape frame's (the cloned root node's [+120h] /
//     [+128h], 00ADE820 -> 00B6F8F0; cc13 audit8): item translations are never
//     applied. LABELLED: items are walked in file order (only the order of overlapping regions
//     depends on it; every terrain model of this installation has at most one
//     damp section).
//   * The texture goes through the VFS resolver (the 00B31C20 route); when it
//     does not resolve, 00B31C20's fallback `error.tga` is used. The loader
//     00B3FCAF keeps the file's levels when renderer+1D84h (2 - TextureDetail)
//     is 0; other values rescale through D3DX and are refused here.
//   * DXT1 and DXT5 colour blocks are decoded to red the way d3dx9_40 does:
//     red = r5 * float(1/31), the third/fourth colours a + (b - a) * float(1/3)
//     and b + (a - b) * float(1/3), DXT1's three-colour midpoint
//     a + (b - a) * 0.5 and its fourth colour 0, then int(float(v * 255) + 0.5),
//     every step in float. FITTED, not recovered from d3dx9_40: it matches the
//     actual DLL on all 54 damp textures and error.dds (2422 distinct DXT1
//     keys, 0 mismatches; local probe p1_dxt_probe).
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "bsp/ocean_wave_field.hpp"

namespace bsp {

// The regions plus the bitmaps they point into.
struct OceanCoverageRegions {
    std::vector<OceanCoverageRegion> regions;
    std::vector<std::unique_ptr<std::vector<std::uint8_t>>> bitmaps;
};

// Reads a VFS name through the resolver; false when it does not resolve.
using OceanCoverageFileReader =
    std::function<bool(const std::string& name, std::string& bytes)>;

// 00B9E170 over a texture file's bytes (DDS, DXT1 or DXT5) at the loader's
// reduction `texture_reduction` (renderer+1D84h). Fills bitmap, width and
// height; false (with `error`) where the image refuses or this host cannot
// decode.
bool ocean_coverage_read_mask_00b9e170(const std::string& dds_bytes, int texture_reduction,
    std::vector<std::uint8_t>& bitmap, float& width, float& height, std::string& error);

// 00BA1370 -> 00BA0C00 for one landscape: appends its regions in order and
// returns a one-line summary for the log.
std::string ocean_coverage_add_landscape_00ba1370(OceanCoverageRegions& out,
    const std::string& model_bytes, const float landscape_world[16],
    const OceanCoverageFileReader& read_file, int texture_reduction);

}  // namespace bsp
