#pragma once
#include "bsp/gui_listbox_runtime.hpp"

namespace bsp {
struct GuiListboxPointerServices {
    const GuiListboxFrameServices& frame;
    // SAME current F8BC84 binding. Native ECX points to its one-byte output.
    // Do not substitute Listbox144 or globalF8BC08, which belong to frame40.
    // Callable transport is dispatch metadata borrowing actual global/owner
    // state; it must not capture an independent mutable game-state copy.
    const std::function<std::uint8_t()>& activation_callback_00f8bc84;
    const volatile float& page_delay_00d5bba0;
};

//00696430: ECX output byte, no stack args, bare RET. Registered as F8BC84 by
//4DD6DB/E0 -> AA6BE0; calls4C43C0 action55 on the CURRENT E188A8 game, then
//copies AL. The required action provider resolves that actual current game.
std::uint8_t read_gui_listbox_pointer_activation_00696430(
    const std::function<std::uint8_t(std::int32_t)>& action_004c43c0);
} // namespace bsp
