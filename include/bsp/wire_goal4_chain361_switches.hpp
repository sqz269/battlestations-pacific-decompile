#pragma once

namespace bsp {

// Chain 361: the scene file reader 0046DF00 dispatches its top-level blocks as
// the image does (0046E9F7..0046EBD8). It peeks each keyword and consumes only
// `entity` (0046EA47 008D8960) before 0046CF40; the `traffic` (009514B0) and
// `SceneBrowserGroups` (00469E40) readers expect their own keyword, so the
// reader leaves it pending and the host's loaders call those whole bodies.
// False: the reader consumes every keyword, parses entities inline, and the
// loaders use the after-keyword entries.
inline constexpr bool kWireGoal4Chain361Bound = true;

}  // namespace bsp
