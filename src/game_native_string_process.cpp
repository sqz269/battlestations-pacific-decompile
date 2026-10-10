#include "bsp/game_native_string_process.hpp"

namespace bsp::game {
void* volatile& GameNativeStringProcess::pending_registry_00f878cc() noexcept {
    return pending_registry_00f878cc_;
}

void* volatile& GameNativeStringProcess::allocation_stats_0109cefc() noexcept {
    return allocation_stats_0109cefc_;
}

const char* GameNativeStringProcess::property_empty_00e177e4() const noexcept {
    return &property_empty_00e177e4_;
}

const char* GameNativeStringProcess::query_empty_00e17654() const noexcept {
    return &query_empty_00e17654_;
}

GameNativeStringProcess& game_native_string_process() {
    static auto* const process = new GameNativeStringProcess;
    return *process;
}
} // namespace bsp::game
