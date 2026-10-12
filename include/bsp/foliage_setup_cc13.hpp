#pragma once
// Foliage per-landscape loading and bush material setup, partition segment 80
// (00AD5590-00ADA240). Packet cc13-gui6, docs/CC13_FOLIAGE_TERRAIN_SETUP.md.
//
// The foliage system is the A8h object published at [00F8C210]
// (docs/APP_INIT_WORLD_EFFECTS.md, docs/CC13_SHADOW_CASCADES.md). Names are
// hypotheses, not recovered symbols. Engine objects (streams, groups,
// textures) stay behind host interfaces. Status: reconstructed and
// build-tested, not game-validated.

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::foliage_setup_cc13 {

// ---------------------------------------------------------------------------
// 00AD7FE0, bush texture names
// ---------------------------------------------------------------------------

// Foliage system +04h and +7Ch..+98h.
struct BushTextureNames {
    bool dirty{false};          // +04h, consumed by 00AD60D0
    std::string texture{};      // +7Ch, the scene property BushTextureName
    std::string normal{};       // +84h, stem + "normal.dds"
    std::string atlas{};        // +8Ch, "Effects\Foliage\" + stem + ".ats"
    std::string normal_atlas{}; // +94h, "Effects\Foliage\" + stem + "normal.ats"
};

// 00AD7FE0. __thiscall(system, NativeString by value), RET 8. Caller 004EA870
// with the scene property BushTextureName (00CE931C). Ignored when the
// argument's chars are null, when it is empty (00BF7FBF against 00CE3A0C) or
// when it equals +7Ch case-insensitively (00435C40). The stem is the name
// minus its last four characters (BSP_NativeString_Substring(0, length - 4),
// meant to drop ".dds"/".tga"); a name shorter than four characters is
// passed as a negative length to the substring routine, which this
// reconstruction does not model (it keeps the whole name instead).
// Returns true when the names changed.
bool set_bush_texture_name_00ad7fe0(BushTextureNames& names, const std::string& name);

struct BushTextureHost {
    virtual ~BushTextureHost() = default;
    // 00AEFA30 on the atlas name: releases a previously loaded atlas.
    virtual void release_atlas(const std::string& atlas) = 0;
    // BSP_TextureAtlas_Load.
    virtual void load_atlas(const std::string& atlas) = 0;
    // 00AEE150: texture handle for a name (stored at +9Ch / +A0h).
    virtual void* load_texture(const std::string& texture) = 0;
};

struct BushTextures {
    void* texture{nullptr};        // +9Ch (00AD57D0)
    void* normal_texture{nullptr}; // +A0h (00AD57E0)
};

// 00AD60D0. __fastcall(system). Only acts on the dirty flag: releases the
// old atlases when the corresponding texture handle is set (each checked on
// its own), loads atlas then texture for the colour map, then for the normal
// map, and clears the flag. The old texture handles are overwritten, not
// released here. Caller 004ED910.
void refresh_bush_textures_00ad60d0(BushTextureNames& names, BushTextures& textures,
    BushTextureHost& host);

// ---------------------------------------------------------------------------
// 00AD7140, the bush diffuse factor
// ---------------------------------------------------------------------------

struct BushGroupMaterialHost {
    virtual ~BushGroupMaterialHost() = default;
    // 00AE22D0 on one bush group: stores the factor at group +24h and
    // registers it as the material float parameter "cDiffuseFactor".
    virtual void set_group_diffuse_factor(void* group, float factor) = 0;
};

// 00AD7140. __thiscall(system, float), RET 4: +78h = factor, then every bush
// group of the +54h vector in order. Callers 004EA870 (scene property
// BushDiffuseFactor, 00CE932C) and 00AD92C0 (re-applies +78h after loading).
void set_bush_diffuse_factor_00ad7140(float& system_factor, const std::vector<void*>& bush_groups,
    float factor, BushGroupMaterialHost& host);

// ---------------------------------------------------------------------------
// 00AD92C0, per-landscape foliage files
// ---------------------------------------------------------------------------

struct LandscapeFoliageHost {
    virtual ~LandscapeFoliageHost() = default;
    // [0109CEEC] virtual +8: does the file exist.
    virtual bool file_exists(const std::string& path) = 0;
    // 00AD9050 BSP_FoliageSystem_LoadBushes(landscape, &path).
    virtual void load_bushes(const std::string& path) = 0;
    // 00AD91C0 BSP_FoliageSystem_LoadTrees(landscape, &path).
    virtual void load_trees(const std::string& path) = 0;
    // 00AD7140 with system +78h.
    virtual void reapply_bush_diffuse_factor() = 0;
};

// "terrain/" + name + "_bushes_pc.dat" (00AD92C0).
std::string bushes_path(const std::string& landscape_name);
// "terrain/" + name + "_trees_pc.dat" (00AD92C0).
std::string trees_path(const std::string& landscape_name);

// 00AD92C0. __thiscall(system, landscape), RET 4; only caller 00ADAC95. The
// landscape name is its native string at +20h. With the foliage enable byte
// 00F8C20C clear nothing is loaded; the diffuse factor is re-applied in
// either case (00AD94F1).
void load_landscape_foliage_00ad92c0(bool foliage_enabled, const std::string& landscape_name,
    LandscapeFoliageHost& host);

// ---------------------------------------------------------------------------
// 00AD7030, TreeGroup types by model name
// ---------------------------------------------------------------------------

// The type name compare of 00AD7030 (and of 00AD7B90 for landscape names):
// equal lengths, then __stricmp of the chars; two empty names are equal.
bool native_names_equal(const std::string& a, const std::string& b) noexcept;

// 00AD7030. Returns the index of the first type whose name matches, or -1
// when a new type must be constructed and appended (the caller then appends;
// the image doubles the capacity, minimum 1).
int find_tree_group_type_00ad7030(const std::vector<std::string>& type_names,
    const std::string& model) noexcept;

} // namespace bsp::foliage_setup_cc13
