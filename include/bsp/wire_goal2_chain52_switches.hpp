#pragma once

namespace bsp {
// Goal 2, chain 52 (game_hosts_text.cpp): GameTextHost::build_run writes each
// glyph quad through the complete 00AB98F0 body (glyph_quad_00ab98f0) over an
// image-shaped Text context and vertex-writer view of the host's scratch quad,
// instead of the 00AB98F0..00AB9C5D prefix fragment.
inline constexpr bool kWireGoal2Chain52Bound = false;
}
