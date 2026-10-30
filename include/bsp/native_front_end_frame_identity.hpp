#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame identity requires MSVC Win32.
#endif

namespace bsp {

// Complete 00517D30 [6B/2ops]: MOV EAX,0Ch; RET. No consumed receiver,
// argument, data cell or provider. CEC39C slot+0 names this native identity.
// This ordinary Source C++ callable does not bind the current raw vptr/slot0,
// authenticate a Frame, publish the raw95 registry or activate any caller.
// Its new Source ABI is distinct from the loaded game's private entry.
std::uint32_t native_front_end_frame_id_00517d30() noexcept;

} // namespace bsp
