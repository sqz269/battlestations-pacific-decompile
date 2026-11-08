// Borrowed-source lifetime and real commands-host copy/append regression. Link
// rebuilt bsp_game objects except game_main.obj, bsp_core/lua/zlib, and Windows
// libraries with MSVC Win32 /MD /link /MANIFEST:EMBED. No game loop runs.
#include "bsp/path_movcam_seg46_argon.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_commands.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

int main() {
    namespace pm = bsp::path_movcam_seg46_argon;
    try {
        std::vector<std::array<float, 3>> points{{1.0f, 2.0f, 3.0f}};
        pm::SimplePathSource* raw = nullptr;
        const auto result = pm::entity_path_source_create_for_slot_007b2250(raw, &points);
        std::unique_ptr<pm::SimplePathSource> source(raw);
        if (result != &raw || !source || source->ref_count_04 != 1
            || source->vtable_00 != 0x00D05318u
            || pm::simple_path_source_point_count_007b0e30(*source) != 1)
            throw std::runtime_error("source factory/count contract");

        // Reallocate the backing buffer after constructing the source. The
        // source must observe the owner's current vector, including replacement.
        points.reserve(points.capacity() + 8);
        points.push_back({4.0f, 5.0f, 6.0f});
        const auto point = source->slot_points_08.point(1);
        if (pm::simple_path_source_point_count_007b0e30(*source) != 2
            || point.x != 4.0f || point.y != 5.0f || point.z != 6.0f)
            throw std::runtime_error("source cached the old point buffer/count");
        points.clear();
        if (pm::simple_path_source_point_count_007b0e30(*source) != 0)
            throw std::runtime_error("source missed the emptied live vector");

        // The standalone bodies' Vec3 representation remains supported, and an
        // owner relocation can explicitly rebind the source without allocation.
        std::vector<pm::Vec3> standalone{{7.0f, 8.0f, 9.0f}};
        source->slot_points_08 = pm::SimplePathPoints{&standalone};
        if (pm::simple_path_source_point_count_007b0e30(*source) != 1
            || source->slot_points_08.point(0).z != 9.0f)
            throw std::runtime_error("source rebind/Vec3 contract");

        // Copying a begun path clears the successor's path_begun flag. An
        // append before its next begin updates the retained user vector but
        // deliberately does not refresh the old path_points snapshot. Exercise
        // actual public host calls: count and every point reader must agree.
        bsp::game::GameHostLog log;
        bsp::game::GameCommandsHost commands(log);
        std::vector<bsp::game::GameCommandUnit> units(2);
        for (std::size_t i = 0; i < units.size(); ++i) {
            units[i].index = i;
            units[i].name = "path-owner-" + std::to_string(i);
            units[i].object_id = static_cast<std::uint16_t>(i + 1);
            units[i].class_id = 7; // shipped destroyer class; fixture identity
        }
        commands.register_units(units);
        const float first[3] = {100.0f, 20.0f, 100.0f};
        const float appended[3] = {200.0f, 30.0f, 200.0f};
        commands.begin_loopback_drain_0076c600();
        commands.post_user_path_point_0071d340(0, first, false);
        commands.finish_loopback_drain_0076c600();
        if (!commands.begin_current_command_00835c70(0, first[0], first[2])
            || !commands.copy_director_state_00720450(1, 0))
            throw std::runtime_error("copy/append fixture did not build/copy a user path");
        commands.begin_loopback_drain_0076c600();
        commands.post_user_path_point_0071d340(1, appended, false);
        commands.finish_loopback_drain_0076c600();
        float out[3]{};
        if (!commands.path_final_point_007b1350(1, out)
            || out[0] != appended[0] || out[1] != appended[1] || out[2] != appended[2])
            throw std::runtime_error("copy+append final point read the stale snapshot");
        float x = 0.0f, z = 0.0f;
        if (!commands.path_cursor_point(1, 1, x, z) || x != appended[0] || z != appended[2])
            throw std::runtime_error("copy+append cursor point read the stale snapshot");
        commands.advance_path_cursor_00836bf0(1, first[0], first[2], 1.0f, 1.0f,
            0.0f, [](float, float) { return true; });
        if (!commands.path_cursor_point3(1, out)
            || out[0] != appended[0] || out[1] != appended[1] || out[2] != appended[2])
            throw std::runtime_error("copy+append follow step/current point used stale storage");
        std::cout << "simple path live-vector contract passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
