#pragma once
#include <cstdint>

namespace bsp {
class NativeGuiIconCurrentCalls;
// Whole AB1710..AB17A9 normal schedule; Native ECX/three DWORD homes/RET0Ch.
// This ordinary Source interface changes the binary ABI and has no result.
// Borrow the SAME actual raw Icon and a real three-DWORD array [index, mode,
// ratioBits]. Keep their C++ backing/subobjects alive and the homes writable.
// The ratio home is overwritten with the clamped float at the reached store;
// valid home/Icon DWORD aliases retain their actual read/store order. The
// Source parameter/control/spill backing is separate from all borrowed data;
// original physical private-stack aliases/register/FLAGS/fault ABI are unproved.
//
// actual_one_00d7a24c is the SAME authenticated stable live float cell, borrowed
// from its existing read-only owner; its value is read only on the Native arm.
// No seed/copy/bank/owner/Frame/Acquired/profile/range guard is created here.
// The existing genuine CURRENT80 transport receives the actual receiver,
// captured numeric target and FULL original DWORD index. It must execute that
// actual target over the SAME raw Icon/Model/geometry domain; no semantic
// GuiIconWidget mirror or codeword-to-callable cast is supplied by this body.
//
// COMISS/FUCOMIP/LAHF and literal x87 pops remain explicit; no C++float clamp,
// FP reset, cleanup, retry, exception-origin policy or noexcept is added.
// Ambient x87/MXCSR and hardware/SEH/FH3/unmaskedFP limits stay caller contracts.
// Concrete default80, current88 callers, providers/custody and activation remain
// unbound. Existing semantic body/helper/vendor/Original credit is not duplicated.
void select_native_gui_icon_state_storage_00ab1710(void* actual_icon,
    volatile std::uint32_t (&actual_argument_homes)[3],
    const volatile float& actual_one_00d7a24c,
    NativeGuiIconCurrentCalls& current);
} // namespace bsp
