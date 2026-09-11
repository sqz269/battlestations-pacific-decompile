#pragma once
#include <array>
#include <cstdint>
#include <string>

// BSP_Game_LoadSceneContents (004D4DF0), __fastcall void(GGame*), sole caller
// 004DFB70 BSP_Game_LoadMissionScene at 004E03E5. This is the routine behind the
// load_scene_contents() stub of bsp/mission_scene_load.hpp: it opens the "2_"
// file block, primes the render and effect resources the scene needs, then runs
// scene-file passes 2 (Registration) and 3 (Instantiate) with a resource
// resolution step wedged between them.
//
// The per-entity walk is NOT in 004D4DF0. Both passes happen inside
// 0046DF00 BSP_SceneFile_Read, which is already reconstructed in
// bsp/scene_file.hpp and documented in docs/SCENE_FILE_READER.md; 004D4DF0 only
// chooses the pass flags and does the work around the two calls. Evidence and
// uncertainties: docs/MISSION_SCENE_CONTENTS.md.
namespace bsp {

// ---------------------------------------------------------------------------
// Scene record fields this routine reads, as byte offsets from [game+5FCh].
//
// Producer caveat (docs/WORKER_VERIFICATION_CHECKLIST.md rule 4): every offset
// below was read at its consumer site inside 004D4DF0, 004D0EE0 and 004BA870.
// The header pass of 0046DF00 that writes them was not opened, so the key names
// the .scn file uses for these fields are unknown. docs/SCENE_RECORD_SIDE_BLOCKS.md
// records +0C24h.. only as "zero runs and further sub-objects".
//
// A native string in this image is eight bytes: size_t at +0, char* at +4
// (004D4E25/004D4E31 read the scene path that way).
struct SceneContentsRecordOffsets {
    static constexpr std::uint32_t kScenePath = 0x90C; // native string, 004D4E2C
    // Four colour-remap texture names, 004D4F6B/4F8F/4FAC/4FC3.
    static constexpr std::uint32_t kRemapTexture0 = 0xC24;
    static constexpr std::uint32_t kRemapTexture1 = 0xC2C;
    static constexpr std::uint32_t kRemapTexture2 = 0xC34;
    static constexpr std::uint32_t kRemapTexture3 = 0xC3C;
    // Effect preload list, 004D0F15/004D0F0C.
    static constexpr std::uint32_t kEffectNameArray = 0xC6C; // native string[]
    static constexpr std::uint32_t kEffectNameCount = 0xC70; // int
    static constexpr std::uint32_t kEffectHandleVector = 0xD50; // {ptr,size,cap}
    // Procedural cloud scatter, read by 004BA870.
    static constexpr std::uint32_t kCloudEnable = 0xC84; // int, < 1 skips
    static constexpr std::uint32_t kCloudBoxMin = 0xC88; // float[3]
    static constexpr std::uint32_t kCloudBoxMax = 0xC94; // float[3]
    static constexpr std::uint32_t kCloudCount = 0xCA0; // int
    static constexpr std::uint32_t kCloudKindWeights = 0xCA4; // float[3]
};

// ---------------------------------------------------------------------------
// Pure rules.

// File-block label opened at 004D4EEA: "2_" concatenated with the short mission
// name derive_scene_short_name (004CD7F0) produces. bsp/mission_scene_load.hpp
// states there is no "2_" block because 004DFB70 does not open one; it is opened
// here instead, so MissionLoadPhase's comment is incomplete, not wrong.
inline constexpr const char* kSceneContentsBlockPrefix = "2_"; // 00CE7914
std::string scene_contents_block_name(const std::string& short_name);

// 004D5181..004D51AE. The avoid-zone file name: the last four characters of the
// scene path are dropped unconditionally (the native code subtracts 4 from the
// length, it does not search for a dot) and ".nav" is appended.
inline constexpr const char* kAvoidZoneExtension = ".nav"; // 00CE7900
std::string scene_avoid_zone_path(const std::string& scene_path);

// 004D531F..004D5426. The entity-map probe name: everything from the LAST '.'
// onward is dropped (00467CF0 reverse-find of "." with a 7FFFFFFF limit, then
// 00469840 substring), then ".ema" is appended. With no '.' in the path the
// native reverse-find returns -1 and the substring yields the empty string, so
// the probe degrades to ".ema"; that case does not occur for a .scn path.
inline constexpr const char* kEntityMapExtension = ".ema"; // 00CE7838
std::string scene_entity_map_path(const std::string& scene_path);

// 004D52B6..004D52EF. The flag the routine keeps in [ESP+37h] and branches on
// twice: at 004D5461 (with the .ema probe, to force game mode 9) and at
// 004D5535 (inverted, to create the MultiScore entity). The native code inlines
// 004BCA50 and compares the result with 8, so this is
// effective_game_mode_004bca50(raw_mode, mode_forced, multiplayer) == 8.
// bsp/simulation_gate.hpp owns that helper; this wrapper only names the test.
bool scene_contents_single_player_layout(
    int raw_mode, bool mode_forced, bool multiplayer_session) noexcept;

// Game mode 004BC890 forces at 004D548E when the .ema probe resolves.
inline constexpr int kSceneEntityMapGameMode = 9;

// ---------------------------------------------------------------------------
// Procedural cloud scatter, 004BA870, __fastcall(scene record), run last.
//
// The three kinds come from the 18h-stride table at 00E081C8: the class name is
// inline at +0 and the minimum separation is a float at +14h. The table's fourth
// slot is the -1 terminator at 00E08208.
struct SceneCloudKind {
    const char* class_name;
    float min_separation;
};
inline constexpr std::array<SceneCloudKind, 3> kSceneCloudKinds{{
    {"CloudSmall", 100.0f}, // 00E081C8
    {"CloudMedium", 300.0f}, // 00E081E0
    {"CloudBig", 500.0f}, // 00E081F8
}};

// 004BA88E..004BA8B4. A roll in [0, w0+w1+w2] walks the weight array subtracting
// each entry and stops at the first index whose running remainder is <= 0. The
// native loop bound is `index < 3`, so a roll larger than the total (only
// reachable when a weight is negative) leaves index 3, one past the table; the
// caller then reads 00E081DC+3*18h, the terminator row. This projection clamps
// to the last real kind and flags the case instead of reproducing that read.
struct SceneCloudKindChoice {
    std::size_t index;
    bool past_table; // native would index the 00E08208 terminator row
};
SceneCloudKindChoice select_scene_cloud_kind(
    const std::array<float, 3>& weights, float roll) noexcept;

// 004BA8C7..004BA93B. A candidate point is rejected while any already-placed
// point is closer than the chosen kind's separation; the native comparison is
// `distance < placed_separation + candidate_separation`, with the distance
// forced to 0 when the squared distance is below the epsilon at 00CE3820.
// The attempt loop gives up after 1000 tries and keeps the last candidate.
inline constexpr int kSceneCloudMaxPlacementAttempts = 1000; // 004BA93B

// ---------------------------------------------------------------------------
// Which scene-file pass a BSP_SceneFile_Read call selects. The two argument
// values are the 3rd and 6th stack arguments of 0046DF00; see the pass table of
// docs/SCENE_FILE_READER.md, which owns the per-entity behaviour.
enum class SceneContentsPass {
    Registration = 2, // 004D54DE: (path, 0, 0, record, override, 1)
    Instantiate = 3, // 004D5530: (path, 0, 1, record, override, 0)
};

// ---------------------------------------------------------------------------
// The host. One method per native call site of 004D4DF0, in body order.
// Opaque handles stand for objects this packet does not reconstruct.
class SceneContentsHost {
public:
    virtual ~SceneContentsHost() = default;

    using EffectHandle = void*;
    using StreamHandle = void*;
    using EntityHandle = void*;

    // 004D4E11..004D4E21. Two bytes of the global game object, cleared before
    // anything else. Their readers were not identified in this packet.
    virtual void clear_scene_contents_flags() = 0; // [00E188A8]+19D0h, +19D1h

    // 004D4E25. Scene path from the record, or "" (00E18B1C) when null.
    virtual std::string record_scene_path() = 0; // [game+5FCh]+90Ch
    // 004D4E4B, __cdecl, ADD ESP,8. Scope marker, format "Loading scene %s".
    virtual void log_loading_scene(const std::string& path) = 0; // 004254B0

    // 004D4E50..004D4E5E. Gate on the Lua machine being open.
    virtual bool lua_machine_open() = 0; // [[game+1A08h]+4] != 0
    // 004D4E69, only when the gate holds.
    virtual void lua_run_string(const char* source) = 0; // 006B8AD0, (src,0,0,2)

    // 004D4E80. 004CD7F0 is already declared as derive_scene_short_name in
    // bsp/mission_scene_load.hpp; this method exists so the call site is named.
    virtual std::string derive_short_name(const std::string& scene_path) = 0;
    // 004D4EEA. The "2_" VFS file block, closed at 004D56BA by 00BDCB30.
    virtual void open_file_block(const std::string& label) = 0; // 00BE0A30
    virtual void close_file_block() = 0; // 00BDCB30

    // 004D4F78/4F95/4FB3/4FD0. Four distinct setters on the render-resources
    // singleton [00F8D39C], slots +66Ch/+670h/+674h/+678h. Each releases the
    // slot's old reference and reloads from the name through [00F8D394]
    // vtable +64h. 00503510 BSP_FrontEndPreview_Draw passes "ColourRemap.tga"
    // (00CEB588) to all four, which is what identifies them as remap textures.
    // Record read, not a call site: the four names sit at record+C24h with an
    // 8-byte stride (004D4F6B, 004D4F8F, 004D4FAC, 004D4FC3).
    virtual std::string record_remap_texture_name(std::size_t slot) = 0;
    virtual void set_remap_texture_0(const std::string& name) = 0; // 00B0FD70
    virtual void set_remap_texture_1(const std::string& name) = 0; // 00B0FDC0
    virtual void set_remap_texture_2(const std::string& name) = 0; // 00B0FE10
    virtual void set_remap_texture_3(const std::string& name) = 0; // 00B0FE60

    // 004D4FE1. 004D0EE0 __fastcall(record): clears the handle vector at
    // record+D50h, then acquires one effect per name in record+C6Ch[0..C70h).
    virtual void preload_record_effects() = 0; // 004D0EE0

    // 004D502B and the inline reference swap that follows it. The acquired
    // handle is published into the global at 00E18A78; the previous occupant is
    // released through its vtable slot 0 when its refcount reaches zero.
    virtual EffectHandle acquire_effect(const std::string& name) = 0; // 00871BA0
    virtual void publish_plane_rumble_effect(EffectHandle handle) = 0; // 00E18A78
    virtual void release_effect(EffectHandle handle) = 0; // inline, 004D5087

    // Avoid-zone block, 004D521E..004D5269, entered only when the file exists.
    virtual bool vfs_file_exists(const std::string& path) = 0; // [0109CEEC] vt+8
    virtual StreamHandle vfs_open_stream(const std::string& path, int mode) = 0; // 00BE4380
    virtual void load_avoid_zones(StreamHandle stream) = 0; // 004C17D0 then 004248A0
    virtual void vfs_release_stream(StreamHandle stream) = 0; // 00BE44A0

    // 004D5458. Probes the .ema name; unlike vfs_file_exists this one is
    // 00BDF4C0 on the same VFS global and resolves aliases.
    virtual bool vfs_resolve_existing(const std::string& path) = 0; // 00BDF4C0
    // 004D547B, __thiscall(game+650h, game+2198h). True when the mission key is
    // already present in the container at game+650h.
    virtual bool mission_key_registered() = 0; // 007F8D60
    // 004D548E, (game, mode, forced). Only reached with (9, false).
    virtual void set_game_mode(int mode, bool forced) = 0; // 004BC890

    // 004D5499/004D54A4, immediately before pass 2.
    virtual void clear_pending_class_ids() = 0; // 004C8AA0, 00F8A09C list
    virtual void clear_scene_database_nodes() = 0; // 00468C90 on [00E18680]

    // 004D54DE and 004D5530, __thiscall on the scene database [00E18680].
    // The per-entity work lives here; see docs/SCENE_FILE_READER.md.
    virtual void read_scene_file(
        const std::string& path, const std::string& override_name,
        SceneContentsPass pass) = 0; // 0046DF00
    // 004D54A9/004D54FD. Argument 5 of both reads, "" (00E18B1C) when null.
    virtual std::string record_override_name() = 0; // [game+604h]

    // Between the two passes, 004D54E9..004D54F8.
    virtual void destroy_scene_database_pending() = 0; // 004697B0, [00E18680]+28h
    virtual void build_class_preload_aliases() = 0; // 004D4720 on 010904E4
    virtual void resolve_preload_aliases() = 0; // 00BB5AC0 on [010904E8]

    // 004D554F..004D5631, after pass 3.
    virtual EntityHandle create_multi_score() = 0; // new(314h) then 00780DB0
    virtual void place_entity_identity(EntityHandle entity) = 0; // vt+98h
    virtual bool network_session_active() = 0; // [00E188A8]+1FE4h == 1

    // 004D56C5, after the file block closes. 004BA870 __fastcall(record).
    virtual void scatter_clouds() = 0; // 004BA870

    // Mode inputs for scene_contents_single_player_layout, read at 004D52B6.
    virtual int raw_game_mode() = 0; // game+614h
    virtual bool game_mode_forced() = 0; // game+61Ch
    virtual bool multiplayer_session() = 0; // game+1FE4h
};

// The routine itself, 004D4DF0 start to finish. Coverage: complete.
void load_scene_contents_004d4df0(SceneContentsHost& host);

} // namespace bsp
