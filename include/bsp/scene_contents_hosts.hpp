#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/mission_scene_contents.hpp"

// The four scene-contents steps that BSP_Game_LoadSceneContents (004D4DF0)
// delegates to, and that bsp/mission_scene_contents.hpp still declares only as
// host methods:
//
//   1. 004D0EE0 BSP_SceneRecord_PreloadEffects   (preload_record_effects)
//   2. 004C17D0 + 004248A0 + 004239E0            (load_avoid_zones, the .nav file)
//   3. 004BA870 BSP_SceneRecord_ScatterClouds    (scatter_clouds)
//   4. 0046DF00+0x100..+0x7FF, the Weathers walk (the weather descriptor)
//
// bsp/mission_scene_contents.hpp owns the outer routine, the scene-record
// offsets it reads and select_scene_cloud_kind; this header adds only what is
// inside the four steps. Evidence, offset tables and uncertainties:
// docs/SCENE_CONTENTS_HOSTS.md.
namespace bsp {

// ---------------------------------------------------------------------------
// Step 1. 004D0EE0 BSP_SceneRecord_PreloadEffects, __fastcall(SceneRecord*).
//
// 004D0EFB takes EBX = record + D50h, the handle vector, and 004D0F05 resizes
// it to 0 through 004CB160. The loop at 004D0F15..004D0F73 walks
// i < [record+C70h] and for each i acquires
// 00871BA0(out, EDX = [record+C6Ch] + i*8, 1) and pushes the returned handle
// with 004CAF50; the temporary at [ESP+10h] is then released with
// InterlockedDecrement on +4h and the vtable slot-0 destructor at zero.
// "Preload" is exactly that: the name list is resolved to refcounted effect
// handles the record then holds, and nothing is instantiated.
//
// The PlaneRumble handle of 004D502B is NOT part of this routine; it is the
// caller's own acquire, published into the global at 00E18A78.
class SceneEffectPreloadHost {
public:
    virtual ~SceneEffectPreloadHost() = default;

    // record+C70h, the count, re-read on every iteration (004D0F6D).
    virtual int record_effect_name_count() = 0;
    // record+C6Ch + index*8, one native string per entry.
    virtual std::string record_effect_name(int index) = 0;

    // 004CB160 on record+D50h with 0: release every held handle.
    virtual void clear_effect_handles() = 0; // 004D0F05
    // 00871BA0 BSP_EffectHandle_AcquireByName(out, name, 1). Reconstructed as
    // acquire_gameplay_effect_by_name_00871ba0 in src/gameplay_effect_acquisition.cpp.
    virtual SceneContentsHost::EffectHandle acquire_effect_by_name(const std::string& name) = 0; // 004D0F24
    // 004CAF50 on record+D50h: push_back of the acquired handle.
    virtual void push_effect_handle(SceneContentsHost::EffectHandle handle) = 0; // 004D0F34
    // The temporary's release, InterlockedDecrement([temp+4]) then vtable[0].
    virtual void release_temporary(SceneContentsHost::EffectHandle handle) = 0; // 004D0F4F
};

void preload_scene_record_effects_004d0ee0(SceneEffectPreloadHost& host);

// ---------------------------------------------------------------------------
// Step 2. The .nav file: 004C17D0 BSP_AvoidZoneRegistry_GetSingleton, 004248A0
// LoadFromStream and 004239E0, the per-element deserializer.
//
// The element class is TerrainGridLayer, not AvoidZone: the writer 0041EF90
// emits the literal at 00CE38D0 and every shipped .nav carries it. The four
// scalars and the grid are the six fields 0041EF90 writes and 004239E0 reads
// back, in the same order.
struct TerrainGridLayerOffsets {
    static constexpr std::uint32_t kVTable = 0x00; // 00CE38CC, one slot (0041F640)
    static constexpr std::uint32_t kHalfExtent = 0x04; // float, 12000.0 in every shipped file
    static constexpr std::uint32_t kDimension = 0x08; // int n, 240 in every shipped file
    static constexpr std::uint32_t kCellSize = 0x0C; // float, 24000.0 / n
    static constexpr std::uint32_t kFileScalar = 0x10; // float, per-file, see the doc
    static constexpr std::uint32_t kSlopeLimit = 0x14; // float, tan(angle)
    static constexpr std::uint32_t kGridVector = 0x18; // vector<uint8_t>, _Myfirst at +1Ch
    static constexpr std::uint32_t kSize = 0x28; // operator new(28h) at 00424904
};

// 00423960, __thiscall(layer, int n): the defaults the constructor and the
// reset 00423AF0 install before any file is read.
inline constexpr float kTerrainGridHalfExtent = 12000.0f; // 00CE3968
inline constexpr double kTerrainGridFullExtent = 24000.0; // 00CE3960, the numerator of the cell size
inline constexpr float kTerrainGridDefaultFileScalar = 2.0f; // 00CE3958
inline constexpr int kTerrainGridDefaultDimension = 10; // 00423AF0
inline constexpr float kTerrainGridDefaultAngleRadians = 0.17453286f; // 00CE3990, ten degrees

// One TerrainGridLayer as the .nav file carries it. The grid is n*n bytes, one
// per cell, row-major in file order; the reader stores it verbatim.
struct TerrainGridLayerRecord {
    std::string class_name; // "TerrainGridLayer"
    float half_extent = 0.0f; // +04h
    int dimension = 0; // +08h
    float cell_size = 0.0f; // +0Ch
    float file_scalar = 0.0f; // +10h
    float slope_limit = 0.0f; // +14h
    std::vector<std::uint8_t> grid; // +18h, dimension * dimension bytes
};

struct TerrainGridNavFile {
    std::string class_name; // "TerrainGrid", read and discarded by 004248A0
    std::vector<TerrainGridLayerRecord> layers;
    std::size_t bytes_consumed = 0;
    bool ok = false;
    const char* error = nullptr;
};

// The grammar of an AvoidZone .nav file, derived from 004248A0 / 004239E0 and
// checked byte-exact against all 253 shipped files (local/nav_survey.py):
//
//   file  := str name ; i32 count ; layer[count]
//   layer := str name ; f32 half_extent ; i32 n ; f32 cell ; f32 scalar ;
//            f32 slope ; u8 grid[n*n]
//   str   := u32 length ; char[length]        (no terminator)
//
// Every integer and float is little-endian; the reader trusts the counts, so a
// truncated file is a caller error. This projection refuses instead.
TerrainGridNavFile parse_terrain_grid_nav(const std::uint8_t* data, std::size_t size);

// 0041DF40, __thiscall(registry, float value, bool value_is_angle), RET 8.
// With value_is_angle the value is first put through tan (00412E20, FSINCOS
// then FDIVP, so radians). The walk keeps the layer with the largest
// slope_limit that is still strictly below the requested slope, and starts from
// the first layer in the list, which is what an unmatched query returns.
// Returns an index into `layers`; -1 only for an empty list.
int select_terrain_grid_layer(
    const std::vector<TerrainGridLayerRecord>& layers, float value, bool value_is_angle) noexcept;

// The sequence of 004D5232..004D5269 inside the caller plus 004248A0 itself.
class SceneAvoidZoneHost {
public:
    virtual ~SceneAvoidZoneHost() = default;

    // 004C17D0: the double-checked singleton on 00E17620 under the lifetime
    // manager's critical section. Constructing it (00424730) installs one
    // default ten-degree layer, which 004248A0 does not remove.
    virtual void ensure_registry() = 0; // 004C17D0
    // 004248A0 prologue, 0041DED0 on the registry, and 0041E000 at the end.
    virtual void begin_load() = 0; // 004248C1
    virtual void end_load() = 0; // 00424975
    // Stream vtable +60h (string), +38h (int), +44h (float), +24h (raw bytes).
    virtual std::string read_string() = 0;
    virtual int read_int() = 0;
    virtual float read_float() = 0;
    virtual void read_bytes(std::uint8_t* out, std::size_t count) = 0;
    // operator new(28h) at 00424904, then the link into the list at registry+8h.
    virtual void append_layer(const TerrainGridLayerRecord& layer) = 0; // 0042494F
};

void load_avoid_zones_004248a0(SceneAvoidZoneHost& host);

// ---------------------------------------------------------------------------
// Step 3. 004BA870 BSP_SceneRecord_ScatterClouds, __fastcall(SceneRecord*).
//
// ESI = record + C88h for the whole body, so every offset below is ESI-relative
// in the listing. The weight total at 004BA8A3..004BA8CE is
// ((w0 + 0.0) + w1) + w2 with each partial rounded back to float; the constant
// the decompiler shows as _DAT_00d7a258 is the double 0.0 of that first add.
float scene_cloud_weight_total(const std::array<float, 3>& weights) noexcept;

// 004BAA11..004BAA72, the rejection test for one already-placed point. The
// native squares the component differences in x87, and when the squared
// distance is at or below the double at 00CE3820 (about 9.99e-11) it uses 0.0
// instead of calling sqrt (00BF7030). A candidate is rejected as soon as
// placed_separation + candidate_separation > distance.
inline constexpr double kSceneCloudDistanceEpsilon = 9.9999999392252903e-11; // 00CE3820

bool scene_cloud_candidate_rejected(
    const std::array<float, 3>& candidate,
    const std::array<float, 3>& placed,
    float placed_separation,
    float candidate_separation) noexcept;

// The 4x4 the native builds on the stack at 004BAB17..004BAB9C: row-major, 1.0f
// (00D7A24C) on the diagonal, the accepted point in the fourth row.
std::array<float, 16> scene_cloud_placement_matrix(const std::array<float, 3>& point) noexcept;

class SceneCloudScatterHost {
public:
    virtual ~SceneCloudScatterHost() = default;

    virtual int cloud_gate() = 0; // record+C84h, < 1 returns at once (004BA879)
    virtual int cloud_count() = 0; // record+CA0h
    virtual std::array<float, 3> cloud_box_min() = 0; // record+C88h/C8Ch/C90h
    virtual std::array<float, 3> cloud_box_max() = 0; // record+C94h/C98h/C9Ch
    virtual std::array<float, 3> cloud_kind_weights() = 0; // record+CA4h/CA8h/CACh

    // 00BD2F10, ECX = 1 on every one of the five call sites, two float stack
    // arguments (low, high). The native draws, in this order per point: the
    // kind roll, then x, y, z, then one more draw in [-pi, +pi] (00CE684C to
    // 00D7A264) whose result is popped unused at 004BAB02. That draw still
    // advances the stream, so the projection keeps it.
    virtual float random_range(float low, float high) = 0;

    // 0046D930(class_name, "Cloud" (00CE7524), 0) with ECX = [00E18680].
    // Reconstructed: docs/SCENE_ENTITY_CREATE.md.
    virtual SceneContentsHost::EntityHandle create_cloud_entity(const char* class_name) = 0; // 004BAB12
    virtual void place_entity(SceneContentsHost::EntityHandle entity, const std::array<float, 16>& transform) = 0; // vt+88h
    virtual void activate_entity(SceneContentsHost::EntityHandle entity) = 0; // vt+D8h
};

inline constexpr const char* kSceneCloudParentName = "Cloud"; // 00CE7524
inline constexpr float kSceneCloudYawLow = -3.14159274f; // 00CE684C
inline constexpr float kSceneCloudYawHigh = 3.14159274f; // 00D7A264

void scatter_scene_clouds_004ba870(SceneCloudScatterHost& host);

// ---------------------------------------------------------------------------
// Step 4. The weather-descriptor pass, 0046DF00 + 0x1A4 (0046E0A4) through
// + 0x7FF (0046E6FE), which runs on all three passes of the scene-file reader.
//
// The table is the Weathers global of SCRIPTS\datatables\Weather.lua (00CE59DC),
// read into a throwaway Lua state the pass opens (00B6A020 with library mask 4)
// and closes again (00B669A0 at 0046E6F9).
struct WeatherSubScene {
    std::string id; // "ID" (00CE59B4)
    std::string descriptor; // "Descriptor" (00CE59A8), a .ptr path or ""
    // The four shadow keys, read out of this same Lua row, not out of the .ptr.
    bool has_static_shadow_texture = false;
    std::string static_shadow_texture; // "g_StaticShadowTexture" (00CE597C)
    bool has_shot_offset_x = false;
    float shot_offset_x = 0.0f; // "ga_StaticShadowShotOffsetX" (00CE5940)
    bool has_shot_offset_z = false;
    float shot_offset_z = 0.0f; // "ga_StaticShadowShotOffsetZ" (00CE58FC)
    bool has_shot_size = false;
    float shot_size = 0.0f; // "ga_StaticShadowShotSize" (00CE58BC)
};

struct WeatherEntry {
    std::string scene_file; // "sceneFile" (00CE59C4)
    std::vector<WeatherSubScene> sub_scenes; // "SubScenes" (00CE59B8)
};

// What one sub-scene row resolves to. The pass visits every sub-scene of the
// matched entry; `selected` marks the one whose Descriptor is applied to the
// reader's own property bag and whose shadow keys are pushed into it. Every
// other non-empty Descriptor is still tokenized and parsed, into a bag that is
// destroyed immediately (0046E563..0046E5A3), so the file is touched but its
// values are dropped.
enum class WeatherDescriptorUse {
    Skipped, // Descriptor was the empty string: no file is opened at all
    ParsedAndDiscarded, // 0046E563: parsed into a local bag, then 008F5410
    AppliedToReaderBag, // 0046E76C: parsed into the reader's bag, shadow keys written
};

// 0046E530..0046E766. The reader's overrideName argument (arg 5, [EBP+18h])
// picks the sub-scene: a row is selected when _stricmp(overrideName, ID) == 0,
// and when either side is absent or empty the empty-ID row is the selected one.
// Every other row is ParsedAndDiscarded. The comparison is case-insensitive
// (00BF7FBF) and there is no "first match wins" rule: the loop does not stop,
// so a table with two matching IDs applies the last one.
WeatherDescriptorUse weather_sub_scene_use(
    const WeatherSubScene& row, const char* override_name) noexcept;

// 0046E1E0..0046E6D0. Weathers is walked from index 1 until an entry is nil;
// an entry matches when _stricmp(entry.sceneFile, scenePath) == 0, with the
// same both-empty rule. Returns the index of the matched entry or -1. The loop
// does not break on a match either, so the last matching entry wins.
int select_weather_entry(
    const std::vector<WeatherEntry>& entries, const char* scene_path) noexcept;

// The four console-variable names the pass writes into the property bag. The
// value comes from the Lua row; the key looked up with 008F2260 is the long
// name, and the value lands at the property's +0Ch.
inline constexpr const char* kWeatherShadowTextureKey = "g_Terrain.g_StaticShadowTexture"; // 00CE595C
inline constexpr const char* kWeatherShotOffsetXKey = "g_Terrain.ga_StaticShadowShotOffsetX"; // 00CE5918
inline constexpr const char* kWeatherShotOffsetZKey = "g_Terrain.ga_StaticShadowShotOffsetZ"; // 00CE58D4
inline constexpr const char* kWeatherShotSizeKey = "g_Terrain.ga_StaticShadowShotSize"; // 00CE5898
inline constexpr const char* kWeatherLuaPath = "SCRIPTS\\datatables\\Weather.lua"; // 00CE59DC
inline constexpr const char* kWeatherTableName = "Weathers"; // 00CE59D0
inline constexpr const char* kWeatherDescriptorDelimiters = "{}(),;:="; // 00CE599C
inline constexpr unsigned kWeatherLuaLibraryMask = 4; // 0046E0BB

class SceneWeatherPassHost {
public:
    virtual ~SceneWeatherPassHost() = default;

    // 00B66BD0 then 00B6A020(mask), and 00B669A0 at the end.
    virtual void open_lua_state(unsigned library_mask) = 0; // 0046E0CC
    virtual void close_lua_state() = 0; // 0046E6F9
    // 00B69D40 BSP_LuaStateOwner_RunScriptWithOverrides(owner, &path, 0): the
    // file plus every VFS override of it, in search order.
    virtual void run_script(const std::string& path) = 0; // 0046E119
    // 00B67980 globals, 00B67800 "Weathers", the 00B67720/00B65FB0 walk. The
    // whole table read is one host method because this packet does not port
    // Lua 5.1.1; docs/LUA_OBJECT_API.md owns the accessors.
    virtual std::vector<WeatherEntry> read_weathers_table() = 0; // 0046E145..0046E6D0

    // 008D9CF0 tokenizer over the descriptor path with "{}(),;:=", then
    // 008F5A00 into the bag named by `use`, then 008D9C30.
    virtual void parse_descriptor(const std::string& path, WeatherDescriptorUse use) = 0;
    // 008F2260 on the reader's bag, then 008F3370 for the string key and a
    // float store at +0Ch for the three numeric ones. A key whose Lua value is
    // nil is skipped, and so is a key the bag does not already contain.
    virtual void set_bag_string(const char* key, const std::string& value) = 0; // 0046E7FB
    virtual void set_bag_float(const char* key, float value) = 0; // 0046E892
};

// The pass as a whole. `scene_path` is argument 1 of 0046DF00 and
// `override_name` argument 5; both are compared case-insensitively.
void run_weather_descriptor_pass_0046df00(
    SceneWeatherPassHost& host, const char* scene_path, const char* override_name);

} // namespace bsp
