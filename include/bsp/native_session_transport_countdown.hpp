#pragma once
#include <cstdint>

namespace bsp {
struct NativeSessionTransportCountdownBindings {
    const volatile double& maximum_step_d7a280;       // actual double 0.5
    const volatile float& limited_step_ce3800;       // actual float 0.5
    const volatile float& minimum_countdown_d7a260;  // actual float -1.0
};

// Complete [00782840,00782862): native ECX transport, stack float, RET4.
// Capture actual transport+48 through MOVSS; COMISS against actual float
// D7A260; publish the captured stack word before JBE; otherwise x87 FSUB/FSTP.
void tick_native_transport_countdown_00782840(void* actual_transport,
    std::uint32_t original_step_word,
    const NativeSessionTransportCountdownBindings&) noexcept;

// Complete [0076C4A0,0076C4F3): native ECX session, stack float, RET4.
// x87 FCOMIP/JBE caps only ordered-above steps. Visit actual secondary+18C,
// then freshly read primary+188 after the complete leaf returns. Each reached
// call has its own native FLD/FSTP argument stage, including uncapped steps.
void tick_native_session_transport_countdowns_0076c4a0(void* actual_session,
    std::uint32_t original_step_word,
    const NativeSessionTransportCountdownBindings&) noexcept;

// New cdecl Source interfaces: raw DWORD arguments preserve input float bits;
// they are not native ABI replacements. Borrow actual valid session/transport
// backing and live constant cells. Both transport pointers may alias. No
// allocation, virtual provider, CRT helper, callback, clamp-to-zero or FP
// control normalization is introduced. Masked exception/available x87-slot
// verification does not establish unmasked traps, private faults or gameplay.
} // namespace bsp
