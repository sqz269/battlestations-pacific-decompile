#include "bsp/mission_scene_contents.hpp"

#include "bsp/simulation_gate.hpp"

namespace bsp {

std::string scene_contents_block_name(const std::string& short_name)
{
    // 004D4E87..004D4EEA: a two-character native string "2_" is built first and
    // the short name is concatenated onto it, so the prefix leads.
    return std::string(kSceneContentsBlockPrefix) + short_name;
}

std::string scene_avoid_zone_path(const std::string& scene_path)
{
    // 004D517E: ADD EBX,-4 on the copied length, then 00469840 substring(0, len)
    // and 004261A0 concat. There is no dot search and no length guard; the
    // native substring clamps a negative length to the whole string because the
    // argument is unsigned, which this projection reproduces by clamping to 0.
    const std::size_t keep = scene_path.size() >= 4 ? scene_path.size() - 4 : 0;
    return scene_path.substr(0, keep) + kAvoidZoneExtension;
}

std::string scene_entity_map_path(const std::string& scene_path)
{
    // 004D531F..004D5347: 00467CF0 reverse-finds "." with limit 7FFFFFFF, then
    // 00469840 takes substring(0, index). A miss returns -1 in the native code
    // and the substring is then empty.
    const std::size_t dot = scene_path.find_last_of('.');
    const std::string stem = dot == std::string::npos ? std::string() : scene_path.substr(0, dot);
    return stem + kEntityMapExtension;
}

bool scene_contents_single_player_layout(
    int raw_mode, bool mode_forced, bool multiplayer_session) noexcept
{
    // 004D52B6..004D52EF is 004BCA50 inlined followed by CMP EAX,8 / SETZ.
    return effective_game_mode_004bca50(raw_mode, mode_forced, multiplayer_session)
        == kDefaultSinglePlayerGameMode;
}

SceneCloudKindChoice select_scene_cloud_kind(
    const std::array<float, 3>& weights, float roll) noexcept
{
    // 004BA88E: the roll is drawn from [0, w0 + constant + w1 + w2]; the constant
    // at 00D7A258 was not decoded, so the caller supplies the roll directly.
    float remaining = roll;
    std::size_t index = 0;
    while (index < weights.size()) {
        remaining -= weights[index];
        if (remaining <= 0.0f) {
            return SceneCloudKindChoice{index, false};
        }
        ++index;
    }
    // The native loop leaves index 3, one past the last authored kind.
    return SceneCloudKindChoice{weights.size() - 1, true};
}

void load_scene_contents_004d4df0(SceneContentsHost& host)
{
    // 004D4E11.
    host.clear_scene_contents_flags();

    // 004D4E25..004D4E4B.
    const std::string scene_path = host.record_scene_path();
    host.log_loading_scene(scene_path);

    // 004D4E50..004D4E69.
    if (host.lua_machine_open()) {
        host.lua_run_string("collectgarbage(\"collect\")"); // 00CE4DE0
    }

    // 004D4E80..004D4EEA.
    host.open_file_block(scene_contents_block_name(host.derive_short_name(scene_path)));

    // 004D4F60..004D4FD0. Unconditional, one slot each.
    host.set_remap_texture_0(host.record_remap_texture_name(0));
    host.set_remap_texture_1(host.record_remap_texture_name(1));
    host.set_remap_texture_2(host.record_remap_texture_name(2));
    host.set_remap_texture_3(host.record_remap_texture_name(3));

    // 004D4FE1.
    host.preload_record_effects();

    // 004D4FE6..004D50BF.
    host.publish_plane_rumble_effect(host.acquire_effect("PlaneRumble")); // 00CE7908

    // 004D50C4..004D5269. The avoid-zone file, loaded only when it exists.
    const std::string avoid_zone_path = scene_avoid_zone_path(scene_path);
    if (host.vfs_file_exists(avoid_zone_path)) {
        SceneContentsHost::StreamHandle stream = host.vfs_open_stream(avoid_zone_path, 2);
        host.load_avoid_zones(stream);
        host.vfs_release_stream(stream);
    }

    // 004D52B6..004D52EF. Computed once, branched on twice.
    const bool single_player_layout = scene_contents_single_player_layout(
        host.raw_game_mode(), host.game_mode_forced(), host.multiplayer_session());

    // 004D52F4..004D5493. The .ema probe forces mode 9 for a mission whose key
    // the container at game+650h has not already seen.
    const bool entity_map_present = host.vfs_resolve_existing(scene_entity_map_path(scene_path));
    if (single_player_layout && entity_map_present && !host.mission_key_registered()) {
        host.set_game_mode(kSceneEntityMapGameMode, false);
    }

    // 004D5499..004D54A4.
    host.clear_pending_class_ids();
    host.clear_scene_database_nodes();

    const std::string override_name = host.record_override_name();

    // 004D54DE. Pass 2, Registration.
    host.read_scene_file(scene_path, override_name, SceneContentsPass::Registration);

    // 004D54E9..004D54F8. The registration pass leaves the used vehicle class
    // ids in the global list at 00F8A09C (0095C640 -> 0095C550, reached from
    // the pass-2 tail of 0046DF00); these three steps drain it into resolved
    // "Class_<name>" preload aliases before anything is instantiated.
    host.destroy_scene_database_pending();
    host.build_class_preload_aliases();
    host.resolve_preload_aliases();

    // 004D5530. Pass 3, Instantiate.
    host.read_scene_file(scene_path, override_name, SceneContentsPass::Instantiate);

    // 004D5535..004D5631.
    if (!single_player_layout && host.network_session_active()) {
        host.place_entity_identity(host.create_multi_score());
    }

    // 004D56A8..004D56C5.
    host.close_file_block();
    host.scatter_clouds();
}

} // namespace bsp
