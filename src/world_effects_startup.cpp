// Reconstruction of the phase-9 world-content startup of cSkeletonAppMidway::Init.
// Addresses: 00bbcb40, 00af0060, 00740840, 00ad9ac0, 00ad71c0, 00af1450, 00af0b10.
// Evidence: docs/APP_INIT_WORLD_EFFECTS.md; reports/app_init_world_effects.json.
#include "bsp/world_effects_startup.hpp"

#include <algorithm>
#include <cctype>

namespace bsp {
namespace {

char lower_ascii(char c) {
    return static_cast<char>(
        std::tolower(static_cast<unsigned char>(c)));
}

// 004bcc00 BSP_String_LowercaseAscii.
std::string lowercased(const std::string& text) {
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(), lower_ascii);
    return out;
}

// 004cad40 BSP_String_ReplaceSubstrings("\\", "/") as used by 00aef3c0.
std::string with_forward_slashes(const std::string& text) {
    std::string out(text);
    std::replace(out.begin(), out.end(), '\\', '/');
    return out;
}

// __stricmp on equal-length spans, the shape 00435c40 and 00449af0 use here.
bool equals_ignoring_case(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (lower_ascii(a[i]) != lower_ascii(b[i])) return false;
    }
    return true;
}

// The native rfind returns -1 when the separator is absent; keep that sign so
// the "same directory depth" comparison at 00aef6ec matches the original.
int last_separator_index(const std::string& text, char separator) {
    for (int i = static_cast<int>(text.size()) - 1; i >= 0; --i) {
        if (text[static_cast<std::size_t>(i)] == separator) return i;
    }
    return -1;
}

} // namespace

std::string atlas_directory_prefix_00af0060(const std::string& descriptor_path) {
    const int backslash = last_separator_index(descriptor_path, '\\');
    const int slash = last_separator_index(descriptor_path, '/');
    const int last = std::max(backslash, slash);
    if (last < 0) return "./"; // 00af0258, constant 00d5d7d0
    return descriptor_path.substr(0, static_cast<std::size_t>(last) + 1);
}

bool atlas_path_has_ats_suffix_00af0060(const std::string& descriptor_path) {
    if (descriptor_path.size() < 4) return false;
    return equals_ignoring_case(
        descriptor_path.substr(descriptor_path.size() - 4), ".ats");
}

bool atlas_split_name_matches_00aef3c0(const std::string& requested,
    const std::string& candidate) {
    const std::string a = with_forward_slashes(lowercased(requested));
    const std::string b = with_forward_slashes(lowercased(candidate));

    // 00aef5f4: the exact match short-circuits before any of the split rules.
    if (equals_ignoring_case(a, b)) return true;

    // 00aef62e: a candidate shorter than the request can never carry a suffix.
    if (b.size() < a.size()) return false;

    // 00aef6ec: both must sit at the same directory depth and in the same
    // directory. The native code compares the two rfind('/') results directly,
    // so a differing index rejects even when the directory text would match.
    const int dir_a = last_separator_index(a, '/');
    const int dir_b = last_separator_index(b, '/');
    if (dir_a != dir_b) return false;
    if (dir_a >= 0) {
        const std::size_t dir_len = static_cast<std::size_t>(dir_a);
        if (!equals_ignoring_case(a.substr(0, dir_len), b.substr(0, dir_len))) {
            return false;
        }
    }

    // 00aef70a and 00aef748: both names must end in ".ats".
    if (a.size() < 4 || !equals_ignoring_case(a.substr(a.size() - 4), ".ats")) {
        return false;
    }
    if (b.size() < 4 || !equals_ignoring_case(b.substr(b.size() - 4), ".ats")) {
        return false;
    }

    // 00aef78e: stem(a) + "_" against the first stem(a)+1 bytes of b. The
    // native lengths are len(a)-4 and len(a)-3, both taken from the request.
    const std::string prefix = a.substr(0, a.size() - 4) + "_";
    return equals_ignoring_case(prefix, b.substr(0, a.size() - 3));
}

std::string foliage_indexed_key_00ae84e0(const std::string& stem, int one_based_index) {
    return stem + std::to_string(one_based_index);
}

std::vector<std::uint16_t> build_particle_quad_indices_00af1450() {
    std::vector<std::uint16_t> indices;
    indices.reserve(kParticleIndexCount);
    for (std::uint32_t base = 0; base < kParticleQuadVertexLimit; base += 4) {
        const std::uint16_t v = static_cast<std::uint16_t>(base);
        indices.push_back(v);
        indices.push_back(static_cast<std::uint16_t>(v + 1));
        indices.push_back(static_cast<std::uint16_t>(v + 2));
        indices.push_back(static_cast<std::uint16_t>(v + 2));
        indices.push_back(static_cast<std::uint16_t>(v + 1));
        indices.push_back(static_cast<std::uint16_t>(v + 3));
    }
    return indices;
}

std::size_t load_texture_atlas_00af0060(const std::string& descriptor_path,
    WorldEffectsStartupHost& host, std::size_t* missing_out) {
    host.log_line(std::string("Loading atlas: ") + descriptor_path);

    // 00af013b: a path that is not a .ats descriptor is dropped silently.
    if (!atlas_path_has_ats_suffix_00af0060(lowercased(descriptor_path))) return 0;

    const std::string directory = atlas_directory_prefix_00af0060(descriptor_path);
    const std::vector<std::string> candidates =
        host.find_files_with_extension(directory, "ats");

    // 00af02f0: the guard walks the manager's texture array and compares each
    // entry against the *requested* descriptor path, not against the candidate
    // being considered. Loaded textures carry .dds paths taken from the atlas
    // header, so this comparison cannot succeed for a .ats request; it is
    // reproduced because it is in the binary, not because it can fire.
    const std::vector<std::string> registered = host.registered_atlas_texture_paths();
    const bool already_registered = std::any_of(registered.begin(), registered.end(),
        [&descriptor_path](const std::string& entry) {
            return equals_ignoring_case(entry, descriptor_path);
        });

    std::size_t loaded = 0;
    std::size_t missing = 0;
    for (const std::string& candidate : candidates) {
        if (!atlas_split_name_matches_00aef3c0(descriptor_path, candidate)) continue;
        if (already_registered) continue;

        // 00aef280: resolve through the VFS before parsing.
        std::string resolved = candidate;
        if (!host.vfs_resolve_existing(resolved)) {
            host.log_line(std::string("Atlas file not found: ") + candidate);
            ++missing;
            continue;
        }
        host.load_atlas_descriptor(resolved);
        ++loaded;
    }
    if (missing_out != nullptr) *missing_out = missing;
    return loaded;
}

FoliagePublishResult publish_foliage_enabled_00ad71c0(bool enabled,
    WorldEffectsStartupHost& host) {
    FoliagePublishResult result{};
    result.enabled = enabled;
    host.set_foliage_enabled_flag(enabled); // 00ad71cd, byte 00f8c20c

    // Both loops re-read the flag global on every element (00ad7211 and
    // 00ad7271 reload 00f8c20c rather than keeping the argument in a register).
    const std::size_t types = host.foliage_type_count();
    for (std::size_t i = 0; i < types; ++i) {
        host.set_foliage_type_enabled(i, enabled); // 00ae0710
        ++result.type_calls;
    }
    const std::size_t groups = host.foliage_group_count();
    for (std::size_t i = 0; i < groups; ++i) {
        host.set_foliage_group_enabled(i, enabled); // 00ae2c80
        ++result.group_calls;
    }
    return result;
}

ParticleShaderSet build_particle_shader_set_00af1450(WorldEffectsStartupHost& host) {
    ParticleShaderSet set{};

    // 00af14e0..00af1528: index buffer first, filled with the quad pattern and
    // unlocked before any material is created.
    set.index_buffer = host.create_index_buffer(kParticleIndexCount);
    host.fill_index_buffer(set.index_buffer, build_particle_quad_indices_00af1450());

    // 00af152a: the shared particle atlas texture. One reference is held across
    // the whole construction and released at the end (00af1957).
    void* atlas = host.load_texture(kParticleAtlasTexture);

    // 00af1552 and 00af156c: the sprite vertex format and its declaration.
    set.sprite_vertex_format = host.load_vertex_format(kParticleSpriteVertexFormat);
    set.sprite_vertex_declaration = host.create_vertex_declaration(set.sprite_vertex_format);

    // 00af157e..00af1750: five materials, each given the atlas in slot 0.
    for (int i = 0; i < 5; ++i) {
        set.sprite_materials[i] = host.create_material_for_effect(kParticleSpriteShaders[i]);
        host.set_material_texture_slot(set.sprite_materials[i], 0, atlas);
    }

    // 00af1780..00af1930: the floating-particle format, declaration and shader.
    set.floating_vertex_format = host.load_vertex_format(kParticleFloatingVertexFormat);
    set.floating_vertex_declaration = host.create_vertex_declaration(set.floating_vertex_format);
    set.floating_material = host.create_material_for_effect(kParticleFloatingShader);
    host.set_material_texture_slot(set.floating_material, 0, atlas);

    host.release_texture(atlas);
    return set;
}

WorldEffectsStartupResult run_world_effects_startup(
    const WorldEffectsStartupSettings& settings, WorldEffectsStartupHost& host) {
    WorldEffectsStartupResult result{};

    // 0073d8cc, 00bbcb40. Slot 0 is the caustics class (00bbc5f0), slots 1..3
    // the shore-wave class (00bbc740).
    for (int slot = 0; slot < 4; ++slot) {
        result.water_sources.slots[slot] = host.create_water_texture_source(
            slot, kWaterTextureSourceNames[slot], slot == 0 ? 0 : 1);
    }
    host.publish_water_texture_source_table(result.water_sources);

    // 0073de42, 00af0060.
    std::size_t missing = 0;
    result.atlases_registered = load_texture_atlas_00af0060(
        settings.startup_atlas_path, host, &missing);
    result.atlases_missing = missing;

    // 0073de8c, 00740840. Every field is read for every entry; a key absent
    // from the table yields the Lua nil conversion of the accessor rather than
    // being skipped, so no entry is ever rejected here.
    result.decals = host.read_decal_table(kDecalTablePath);
    for (DecalDefinition& decal : result.decals) {
        (void)host.load_vertex_format(kDecalVertexFormat); // 00740410, per record
        (void)host.load_texture(decal.texture_name);       // +64h
        (void)host.load_shader(decal.shader_name);         // +48h
    }
    host.publish_decal_system(result.decals);

    // 0073deba, 00ad9ac0 then 00ae84e0.
    result.foliage_types = host.read_foliage_table(kFoliageTablePath);
    host.publish_foliage_system(result.foliage_types);

    // 0073deda, 00ad71c0, gated on the Foliage setting byte 00f88a06.
    result.foliage_publish = publish_foliage_enabled_00ad71c0(
        settings.foliage_enabled, host);

    // 0073df01, 0073df2c and 0073df57 belong to the renderer-device packet and
    // run between here and the particle step; they are not modelled.

    // 0073df82, 00af1450.
    result.particle_shaders = build_particle_shader_set_00af1450(host);
    host.publish_particle_shader_set(result.particle_shaders);

    // 0073e02b, 00af0b10, in phase 10 after the Bink and network objects.
    result.foliage_groups.shader_time_scalar = kFoliageGroupManagerTimeSeed;
    result.foliage_groups.critical_section = host.create_critical_section();
    host.publish_foliage_group_manager(result.foliage_groups);

    return result;
}
}
