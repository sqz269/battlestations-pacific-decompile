#pragma once

namespace bsp {

// Goal 4 chain 312. True: the front-end branch of 004CA440 BSP_Game_Render runs
// its 00735B50 BSP_SoundFrame_ServiceInline call (004CA5D4) from
// MenuFrameHost::game_render, so the started sound core's per-frame update
// (00A87BF0 -> 00A7E630: the streamed-dialog owner's 00A789C0 update, the FMOD
// listener and EventSystem::update, the active-entry pass) runs once per
// front-end frame. False: the front-end render step only logs 004CA440.
inline constexpr bool kWireGoal4Chain312Bound = true;

}  // namespace bsp
