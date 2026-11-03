#pragma once

namespace bsp {
// Goal 2, chain 90 (g2-w5): three 008D5B50 sinks of QuickSettingsApplyHost in
// src/game_profile_load.cpp, bound in src/game_hosts.cpp:
// - 00A7ABF0 set_sound_class_levels_00a7abf0 on the running sound owner's
//   SoundManagerLevels ([00F8BBD8], 008D54D9);
// - 00B1FFB0 set_native_renderer_texture_detail_bias_00b1ffb0 on the actual
//   renderer [00F8D394] (008D5B80);
// - 004C1710 get_native_media_manager_004c1710 over the GUI manager
//   application's focus-owner context, then 008D56B0's +10h store; the drain's
//   native_media arm (00A4C620, vtable 00D24D94) is bound for that object.
// OFF keeps every stand-in's unimplemented record and binds nothing.
inline constexpr bool kWireGoal2Chain90Bound = false;
}
