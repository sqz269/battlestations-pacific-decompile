#include "bsp/lua_spawn_new.hpp"
#include <cstdio>
#include <stdexcept>

static void require(bool value) {
    if (!value) throw std::runtime_error("queued matrix reference changed");
}

int main() {
    bsp::SpawnRequestQueue queue;
    {
        float matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 120,25,-40,1};
        bsp::SpawnNewRequest first;
        first.id = "first";
        first.has_ref_pos = true;
        first.ref_pos[0] = 999; // Placement must consume the retained matrix.
        first.matrix_reference.emplace(matrix);
        bsp::SpawnNewRequest second = first; // 008F8610.
        second.id = "second";
        matrix[12] = -75;
        bsp::SpawnNewMatrixReference replacement(matrix);
        *second.matrix_reference = replacement; // 008F84A0.
        matrix[12] = 5000; // Constructor/copy own the matrix, never this array.
        queue.enqueue_00949530(first);
        queue.enqueue_00949530(second);
    }
    auto first = queue.erase_009439b0(0); // Shifts second through assignment.
    require(first.id == "first" && queue.size() == 1);
    auto frame = bsp::spawn_reference_frame_0094a140(first);
    require(frame.m[0] == 1 && frame.m[5] == 1 && frame.m[10] == 1);
    require(frame.m[12] == 120 && frame.m[13] == 25 && frame.m[14] == -40);
    auto second = queue.erase_009439b0(0);
    require(second.id == "second" && queue.empty());
    frame = bsp::spawn_reference_frame_0094a140(second);
    require(frame.m[12] == -75 && frame.m[13] == 25 && frame.m[14] == -40);
    queue.requeue_009478b0(second);
    auto retried = queue.erase_009439b0(0);
    require(bsp::spawn_reference_frame_0094a140(retried).m[12] == -75);
    std::puts("matrix-only SpawnNew ownership -> queue copy/shift/retry -> placement: PASS");
}
