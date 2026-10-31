#pragma once
#include <cstdint>

namespace bsp {
// Whole AB1230..AB125C normal field/validation schedule, Native ECX vectorF0,
// one DWORD index home, EAX data address, RET4. This ordinary Source pointer/
// borrowed-home ABI is different. The original HIGH STL/CG role keeps its name
// and credit; no new Native/vendor/helper/CG/duplicate-body credit is claimed.
//
// Borrow readable actual 0x10-byte vector backing through+08 and one real DWORD home.
// Proxy00/end0C are unused. This body forms no typed T[n] pointer/reference and
// dereferences no returned state. Valid index-home/vector DWORD aliases retain
// their reached load order; Source parameter/control/spill storage is separate.
// Read first04 before capturing the index home. Read last08 only when first04
// was nonzero; subtraction/SAR6 and unsigned carry comparison are actual DWORD
// operations. On invalid input call the EXISTING real BF6713 Source wrapper.
// If it returns, ALWAYS reload CURRENT first04 and add captured index<<6 modulo
// 2^32. No count clamp, nullptr/default return, index reread or repair is added.
//
// Caller keeps vector/home/handler state alive across the real SDK validation
// call and any exception/termination it supplies. Current Source CRT handler
// policy is inherited, not Original BF66EF/Watson/SEH/FH3/NoThrow equivalence.
// No noexcept/noreturn, cleanup, catch, allocator, owner, record or retry exists.
// Original register/private-stack/FLAGS/fault ABI and later state use remain
// separate contracts; this helper adds no producer/disposition/activation proof.
void* get_native_gui_icon_state_storage_00ab1230(void* actual_vector_f0,
    const volatile std::uint32_t& actual_index_home);
} // namespace bsp
