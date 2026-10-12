// Foliage per-landscape loading and bush material setup, segment 80. Packet
// cc13-gui6; evidence per routine in include/bsp/foliage_setup_cc13.hpp and
// docs/CC13_FOLIAGE_TERRAIN_SETUP.md.
#include "bsp/foliage_setup_cc13.hpp"

#include <string.h>

namespace bsp::foliage_setup_cc13 {

bool native_names_equal(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) {
        return false;
    }
    if (a.empty()) {
        return true;
    }
    return _stricmp(a.c_str(), b.c_str()) == 0;
}

// 00AD7FE0
bool set_bush_texture_name_00ad7fe0(BushTextureNames& names, const std::string& name) {
    if (name.empty()) {
        return false; // null chars or "" (00AD8001, 00AD8017)
    }
    // 00435C40 BSP_NativeString_EqualsInsensitive, taken to be the same
    // length-then-__stricmp compare as 00AD7030 (not re-read here).
    if (native_names_equal(names.texture, name)) {
        return false;
    }
    names.dirty = true;
    names.texture = name;
    const std::string stem = name.size() >= 4 ? name.substr(0, name.size() - 4) : name;
    names.normal = stem + "normal.dds";
    names.atlas = "Effects\\Foliage\\" + stem + ".ats";
    names.normal_atlas = "Effects\\Foliage\\" + stem + "normal.ats";
    return true;
}

// 00AD60D0
void refresh_bush_textures_00ad60d0(BushTextureNames& names, BushTextures& textures,
    BushTextureHost& host) {
    if (!names.dirty) {
        return;
    }
    if (textures.texture != nullptr) {
        host.release_atlas(names.atlas);
    }
    if (textures.normal_texture != nullptr) {
        host.release_atlas(names.normal_atlas);
    }
    host.load_atlas(names.atlas);
    textures.texture = host.load_texture(names.texture);
    host.load_atlas(names.normal_atlas);
    textures.normal_texture = host.load_texture(names.normal);
    names.dirty = false;
}

// 00AD7140
void set_bush_diffuse_factor_00ad7140(float& system_factor, const std::vector<void*>& bush_groups,
    float factor, BushGroupMaterialHost& host) {
    system_factor = factor; // +78h
    for (void* group : bush_groups) {
        host.set_group_diffuse_factor(group, factor); // 00AE22D0
    }
}

std::string bushes_path(const std::string& landscape_name) {
    return "terrain/" + landscape_name + "_bushes_pc.dat";
}

std::string trees_path(const std::string& landscape_name) {
    return "terrain/" + landscape_name + "_trees_pc.dat";
}

// 00AD92C0
void load_landscape_foliage_00ad92c0(bool foliage_enabled, const std::string& landscape_name,
    LandscapeFoliageHost& host) {
    if (foliage_enabled) { // 00AD92D8: byte 00F8C20C
        const std::string bushes = bushes_path(landscape_name);
        if (host.file_exists(bushes)) {
            host.load_bushes(bushes);
        }
        const std::string trees = trees_path(landscape_name);
        if (host.file_exists(trees)) {
            host.load_trees(trees);
        }
    }
    host.reapply_bush_diffuse_factor(); // 00AD94F1
}

// 00AD7030
int find_tree_group_type_00ad7030(const std::vector<std::string>& type_names,
    const std::string& model) noexcept {
    for (std::size_t i = 0; i < type_names.size(); ++i) {
        if (native_names_equal(type_names[i], model)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace bsp::foliage_setup_cc13
