#include "bsp/game_native_geometry_globals.hpp"

namespace bsp::game {
GameNativeGeometryGlobals& game_native_geometry_globals() {
    static GameNativeGeometryGlobals process;
    return process;
}
} // namespace bsp::game
