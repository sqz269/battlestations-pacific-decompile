#include "bsp/game_hosts_session_messages.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>

// Exercise the public local-codec paths: a wrong receiver decision must reject
// delivery, while valid director messages must preserve their decoded payloads.
int main() {
    try {
        bsp::HostSessionMessages messages;
        const auto option = messages.queue_director_sub_kind_5a(2u, 1u);
        const auto clear = messages.queue_clear_command_5d(1u, -1);
        const float point[3]{125.0f, -0.0f, -250.0f};
        const auto path = messages.queue_user_path_point_5f(point);
        if (option.sub_kind != 2u || option.value != 1u
            || clear.arm != 1u || clear.index != -1
            || !path || path->header.base.type_10 != 0x5f
            || path->present_2c != 1u
            || std::memcmp(path->vector_20, point, sizeof point) != 0
            || !messages.queue_repair_body_9f(true)) {
            std::fputs("director/local-unit round-trip payload mismatch\n", stderr);
            return 1;
        }
        std::puts("5A/5D/5F director routing and 9F unit routing: PASS");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "local message routing failed: %s\n", error.what());
        return 2;
    }
}
