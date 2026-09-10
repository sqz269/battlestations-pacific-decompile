#pragma once
// Phase 9 of cSkeletonAppMidway::Init (0073d410): the world-content startup.
// Addresses: 00bbcb40, 00af0060, 00740840, 00ad9ac0, 00ad71c0, 00af1450, 00af0b10.
// Evidence: docs/APP_INIT_WORLD_EFFECTS.md; reports/app_init_world_effects.json.
// Every descriptive name here is a hypothesis, not a recovered symbol.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// 00af0060 BSP_TextureAtlas_Load: split-atlas name resolution
// ---------------------------------------------------------------------------

// Directory prefix used by 00af0060 at 00af014b..00af0261: the maximum of
// rfind('\\') and rfind('/'), then substring(0, index + 1). When neither
// separator occurs the native code assigns "./" (00d5d7d0) instead.
std::string atlas_directory_prefix_00af0060(const std::string& descriptor_path);

// Case-insensitive ".ats" suffix test at 00af00e3..00af0119. The native code
// lowercases the whole path first and compares the last four bytes with
// __stricmp against 00d5d23c (".ats"), so a path shorter than four bytes is
// rejected by the null check on the substring rather than by a length compare.
bool atlas_path_has_ats_suffix_00af0060(const std::string& descriptor_path);

// 00aef3c0, __fastcall(ECX = requested descriptor, EDX = enumerated candidate),
// AL result. Both sides are lowercased and every '\\' is replaced with '/'
// (00ce7894 -> 00ce7898) before comparison. The candidate matches when it is
// the requested path itself, or when it is a split part named
// "<requested without .ats>_<suffix>.ats" in the same directory. This is why
// "interface/textures/common.ats" loads common_dxt1.ats and common_dxt5_1..3.ats
// even though no file named common.ats exists in the installation.
bool atlas_split_name_matches_00aef3c0(const std::string& requested,
    const std::string& candidate);

// ---------------------------------------------------------------------------
// 00740840 BSP_DecalSystem_LoadDefinitions: scripts/datatables/decals.lua
// ---------------------------------------------------------------------------

inline constexpr const char* kDecalTablePath = "scripts/datatables/decals.lua";
inline constexpr const char* kDecalTableGlobal = "Decals";
inline constexpr const char* kDecalVertexFormat = "decal.mvfm"; // 00740410

// One 0x28-byte record allocated at 00740878 and constructed by 00740410.
// Offsets are the native ones; the reconstruction stores names rather than the
// loaded resource pointers because the resource objects are not reconstructed.
struct DecalDefinition {
    std::string name;             // +00 size, +04 data; the Lua table key
    std::int32_t max_count{0};    // +08  "Maxnum", read as int at 00b66290
    float size{0.0f};             // +0C  "Size"   (00cff278), float at 00b66270
    float radius{0.0f};           // +10  "Radius", float
    std::string texture_name;     // +18  "Texture", loaded via renderer vtable +64h
    std::string shader_name;      // +1C  "Shader",  loaded via renderer vtable +48h
    float life_time{0.0f};        // +20  "LifeTime", float
    float fade_out_time{0.0f};    // +24  "FadeOutTime", float
    // +14 holds the "decal.mvfm" vertex format from the record constructor
    // 00740410 (renderer vtable +34h); it is the same object for every record.
};

// ---------------------------------------------------------------------------
// 00ad9ac0 / 00ae84e0 BSP_FoliageSystem_LoadTypes: Effects\foliage\FoliageTypes.lua
// ---------------------------------------------------------------------------

inline constexpr const char* kFoliageTablePath = "Effects\\foliage\\FoliageTypes.lua";
inline constexpr const char* kFoliageTableGlobal = "Foliages";

// The parser at 00ae84e0 builds each key by appending a one-based index to a
// fixed stem (00ae8899..00ae8be4 build "model", "modeltype", "percent",
// "uptexture", "sidetexture", "scale", "height", "topheight" and pass each to
// 004caca0, which concatenates the running index). The installed table uses
// model1/modeltype1/percent1, model2/... exactly this way.
std::string foliage_indexed_key_00ae84e0(const std::string& stem, int one_based_index);

// Sub-entry selected when "model<N>" is present (00ae8e60 branch).
struct FoliageModelVariant {
    std::string model;          // "model<N>",     e.g. CoconutTree001.MMOD
    std::int32_t model_type{0}; // "modeltype<N>", 0 TREE, 1 CORAL, 2 SEAWEED
    std::int32_t percent{0};    // "percent<N>"
};

// Sub-entry selected when "model<N>" is absent but "uptexture<N>" is present
// (00ae9040 branch). Both textures plus three scalars describe the camera
// facing impostor quad that docs/GAME_RENDER_TAIL.md draws at 00af0c50.
struct FoliageImpostorVariant {
    std::string up_texture;    // "uptexture<N>"
    std::string side_texture;  // "sidetexture<N>"
    float scale{0.0f};         // "scale<N>"
    float height{0.0f};        // "height<N>"
    float top_height{0.0f};    // "topheight<N>"
    std::int32_t percent{0};   // "percent<N>"
};

struct FoliageTypeDefinition {
    std::string name; // the "name" field of the array element
    std::vector<FoliageModelVariant> models;
    std::vector<FoliageImpostorVariant> impostors;
};

// Diagnostic emitted at 00ae9200 when a sub-index has neither a model nor an
// uptexture. Kept verbatim; the original text is Hungarian.
inline constexpr const char* kFoliageMissingSourceMessage =
    "Foliage '%s' se modell, se uptexture nincs megadva!";

// ---------------------------------------------------------------------------
// 00ad71c0: publish the Foliage setting
// ---------------------------------------------------------------------------

// 00ad71c0 is __thiscall(this = foliage system 00f8c210, bool enabled), RET 4.
// The boolean arrives in AL from [ESP+4] (first instruction, which Ghidra's
// stored body omits). It writes the byte to 00f8c20c and then forwards it to
// every element of two pointer vectors, this+54h (00ae0710) and this+64h
// (00ae2c80). Init passes the "Foliage" game setting byte at 00f88a06
// (docs/APP_INIT_BOOTSTRAP.md, key Foliage, settings+86h).
struct FoliagePublishResult {
    bool enabled{false};
    std::size_t type_calls{0};  // this+58h..this+5Ch, 00ae0710
    std::size_t group_calls{0}; // this+68h..this+6Ch, 00ae2c80
};

// ---------------------------------------------------------------------------
// 00af1450 BSP_ParticleSystem_LoadShaders
// ---------------------------------------------------------------------------

inline constexpr const char* kParticleAtlasTexture = "Particles/Textures/atl_all.dds";
inline constexpr const char* kParticleSpriteVertexFormat = "particleaxialsprite.mvfm";
inline constexpr const char* kParticleFloatingVertexFormat = "ParticleFloating.mvfm";
inline constexpr const char* kParticleFloatingShader = "ParticleFloating.mshd";

// The four normal/soft permutations plus the distortion variant, in the native
// order 00af157e..00af1750. n<N>s<S> is the naming in shaderfx/common.
inline constexpr const char* kParticleSpriteShaders[5] = {
    "particleaxialsprite_n0s0.mshd",
    "particleaxialsprite_n0s1.mshd",
    "particleaxialsprite_n1s0.mshd",
    "particleaxialsprite_n1s1.mshd",
    "particleaxialsprite_dist.mshd",
};

// Index buffer created at 00af14ec with 60000 indices; the fill loop at
// 00af14fe..00af1520 walks a base vertex from 0 to 39999 in steps of four.
inline constexpr std::uint32_t kParticleIndexCount = 60000;
inline constexpr std::uint32_t kParticleQuadVertexLimit = 40000;

// Reproduces the fill loop exactly: per quad base v, the six indices are
// v, v+1, v+2, v+2, v+1, v+3. The native code stores 16-bit values from a
// 32-bit counter, so the base wraps through the signed short boundary at
// v = 32768 and the stored values continue as unsigned 16-bit.
std::vector<std::uint16_t> build_particle_quad_indices_00af1450();

// Projection of the 0x30-byte object allocated at 0073df5c. Resource handles
// are held as names because the renderer objects are outside this packet.
struct ParticleShaderSet {
    void* index_buffer{nullptr};              // +04
    void* sprite_materials[5]{};              // +08, +0C, +10, +14, +18
    void* sprite_vertex_format{nullptr};      // +1C, particleaxialsprite.mvfm
    void* sprite_vertex_declaration{nullptr}; // +20, from +1C
    void* floating_material{nullptr};         // +24, ParticleFloating.mshd
    void* floating_vertex_format{nullptr};    // +28, ParticleFloating.mvfm
    void* floating_vertex_declaration{nullptr}; // +2C, from +28
};

// ---------------------------------------------------------------------------
// 00bbcb40 BSP_WaterRenderer_RegisterTextureSources
// ---------------------------------------------------------------------------

// The 0x10-byte table stored at 01090900 holds four four-byte objects, each a
// bare vtable pointer whose base constructor takes the registry name. Slot 0
// uses 00bbc5f0 with vtable 00d644e8, slots 1..3 use 00bbc740 with vtable
// 00d644f0, so the caustics source and the three shore-wave sources are
// different classes. docs/GAME_WORLD_OCEAN.md names the same keys.
inline constexpr const char* kWaterTextureSourceNames[4] = {
    "CausticsTextureSource",
    "ShoreWaveTextureSource0",
    "ShoreWaveTextureSource1",
    "ShoreWaveTextureSource2",
};

struct WaterTextureSourceTable {
    void* slots[4]{}; // 01090900 +00, +04, +08, +0C
};

// ---------------------------------------------------------------------------
// 00af0b10: the foliage group manager singleton (00f8c274)
// ---------------------------------------------------------------------------

// 0x34 bytes allocated at 0073e005. The base constructor 00af06a0 publishes the
// instance into 00f8c274, which docs/GAME_RENDER_TAIL.md identifies as the
// foliage group manager whose +2Ch scalar reaches shader constant c33 Time.z
// through 00af0450 / 00af0460. Init seeds +2Ch from 00d7a24c, which holds 1.0f.
inline constexpr float kFoliageGroupManagerTimeSeed = 1.0f;

struct FoliageGroupManagerState {
    float shader_time_scalar{0.0f}; // +2C, seeded from 00d7a24c
    void* critical_section{nullptr}; // +30, from 00bd1860
};

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site. Nothing here has a default implementation:
// none of it stands in for unrecovered game behaviour.
struct WorldEffectsStartupHost {
    virtual ~WorldEffectsStartupHost() = default;

    // 00bbcb40 / 00bbc900. kind 0 is the caustics class, kind 1 the shore-wave
    // class; both register their name through 00b1b730.
    virtual void* create_water_texture_source(int slot, const std::string& name, int kind) = 0;
    virtual void publish_water_texture_source_table(const WaterTextureSourceTable& table) = 0;

    // 00af0060. TRIV_body_004254b0 with "Loading atlas: %s" and, from 00aef280,
    // "Atlas file not found: %s".
    virtual void log_line(const std::string& text) = 0;
    // 00886280 on the file system singleton 0109ceec: enumerate one directory
    // filtered by extension (the native call passes "ats" without a dot).
    virtual std::vector<std::string> find_files_with_extension(
        const std::string& directory, const std::string& extension) = 0;
    // The dedupe walk at 00af02f0..00af0340 over the manager's texture array
    // (base+10h, count+14h), reading each element's name string at +8.
    virtual std::vector<std::string> registered_atlas_texture_paths() = 0;
    // 00bdf4c0 BSP_VFS_ResolveExistingName, called from 00aef280.
    virtual bool vfs_resolve_existing(std::string& name) = 0;
    // 00af5850 then 00aeeaf0: read the descriptor into a text buffer and parse
    // it. The parser itself is docs/ATLAS_PARSER.md and src/texture_atlas.cpp.
    virtual void load_atlas_descriptor(const std::string& name) = 0;

    // 00b69d40 on the Lua state built at 00b66bd0 / 00b6a020, then the table
    // walk. The two definition tables are read through the host because the
    // Lua interpreter is not reconstructed.
    virtual std::vector<DecalDefinition> read_decal_table(const std::string& path) = 0;
    virtual std::vector<FoliageTypeDefinition> read_foliage_table(const std::string& path) = 0;

    // Resource manager 00f8d394 virtual slots seen in this packet.
    virtual void* load_vertex_format(const std::string& name) = 0;  // +34h and +38h
    virtual void* load_shader(const std::string& name) = 0;         // +48h
    virtual void* load_texture(const std::string& name) = 0;        // +64h
    virtual void* create_vertex_declaration(void* vertex_format) = 0; // +40h
    virtual void* create_index_buffer(std::uint32_t index_count) = 0; // +60h then +0Ch
    virtual void fill_index_buffer(void* buffer, const std::vector<std::uint16_t>& indices) = 0; // +10h
    virtual void release_texture(void* texture) = 0; // InterlockedDecrement then vtable +0

    // 00535320 BSP_Material_CreateForEffectName and 00b189f0
    // BSP_Material_SetTextureSlot.
    virtual void* create_material_for_effect(const std::string& shader_name) = 0;
    virtual void set_material_texture_slot(void* material, int slot, void* texture) = 0;

    // The singleton publications performed by the base constructors
    // 0073fa70 (00e1aea0), 00ad6950 (00f8c210), 00ad6810 (00f8c264),
    // 00af1140 (00f8c280) and 00af06a0 (00f8c274).
    virtual void publish_decal_system(std::vector<DecalDefinition> records) = 0;
    virtual void publish_foliage_system(std::vector<FoliageTypeDefinition> types) = 0;
    virtual void publish_particle_shader_set(const ParticleShaderSet& set) = 0;
    virtual void publish_foliage_group_manager(const FoliageGroupManagerState& state) = 0;

    // 00ad71c0: 00f8c20c then the two per-element calls 00ae0710 and 00ae2c80.
    virtual void set_foliage_enabled_flag(bool enabled) = 0;
    virtual std::size_t foliage_type_count() = 0;  // this+58h..this+5Ch, stride 4
    virtual std::size_t foliage_group_count() = 0; // this+68h..this+6Ch, stride 4
    virtual void set_foliage_type_enabled(std::size_t index, bool enabled) = 0;
    virtual void set_foliage_group_enabled(std::size_t index, bool enabled) = 0;

    // 00bd1860 BSP_CriticalSection_Create.
    virtual void* create_critical_section() = 0;
};

struct WorldEffectsStartupSettings {
    // Game setting "Foliage", byte at 00f88a06 (settings 00f88980 +86h).
    bool foliage_enabled{true};
    // The single atlas Init loads at 0073de2c, built from 00ce81c4.
    std::string startup_atlas_path{"interface/textures/common.ats"};
};

struct WorldEffectsStartupResult {
    WaterTextureSourceTable water_sources{};
    std::size_t atlases_registered{0};
    std::size_t atlases_missing{0};
    std::vector<DecalDefinition> decals;
    std::vector<FoliageTypeDefinition> foliage_types;
    FoliagePublishResult foliage_publish{};
    ParticleShaderSet particle_shaders{};
    FoliageGroupManagerState foliage_groups{};
};

// 00af0060 in isolation: the atlas descriptor load, including the split-part
// enumeration. Returns the number of descriptors handed to the parser.
std::size_t load_texture_atlas_00af0060(const std::string& descriptor_path,
    WorldEffectsStartupHost& host, std::size_t* missing_out);

// 00ad71c0 in isolation.
FoliagePublishResult publish_foliage_enabled_00ad71c0(bool enabled,
    WorldEffectsStartupHost& host);

// 00af1450 in isolation.
ParticleShaderSet build_particle_shader_set_00af1450(WorldEffectsStartupHost& host);

// The phase-9 sequence in Init order. 00bbcb40 actually runs earlier, at
// 0073d8cc in the platform phase, and 00af0b10 later, at 0073e02b in phase 10;
// both are included here because they own world-effect state that phase 9
// content depends on. External steps between them (00b14a10, 00b3c4c0,
// 00ad9f90 and the indirect 00f8c218 call) are not modelled.
WorldEffectsStartupResult run_world_effects_startup(
    const WorldEffectsStartupSettings& settings, WorldEffectsStartupHost& host);
}
