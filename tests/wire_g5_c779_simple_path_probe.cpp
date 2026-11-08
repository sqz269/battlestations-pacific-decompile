// Focused borrowed-source lifetime contract. Link against rebuilt bsp_core with
// MSVC Win32 /MD and /link /MANIFEST:EMBED. This does not run the game.
#include "bsp/path_movcam_seg46_argon.hpp"
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
        std::cout << "simple path live-vector contract passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
