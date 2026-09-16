#include "bsp/game_native_string_process.hpp"

namespace bsp::game {
GameNativeStringProcess& game_native_string_process() {
    static auto* const process = new GameNativeStringProcess;
    return *process;
}
} // namespace bsp::game
