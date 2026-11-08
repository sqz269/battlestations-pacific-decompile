#pragma once

namespace bsp::game {

// 004C090C..004C0924's genuine null-listener branch. Borrows the renderer
// application that constructed the current F8D39C service; owns no native
// fields. False means that this host has no native render-resource service.
// A driven unit's unresolved virtual+18 handle must not use this entry.
bool clear_game_native_controlled_listener();

} // namespace bsp::game
