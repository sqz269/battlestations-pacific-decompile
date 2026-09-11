#pragma once

#include "bsp/panel_sequence_types.hpp"

namespace bsp {

// Native panel+4 map: node1Ch has links0/4/8, NativeString key+C,
// signed character value+14, color+18 and sentinel flag+19. Reuse the exact
// length-zero/CRT case-insensitive comparator already recovered for the queue.
// This is a standard-container projection, not a native tree memory layout.
using PanelCharacterMap = std::map<NativeString, std::int32_t, PanelSequenceNameLess>;

} // namespace bsp
