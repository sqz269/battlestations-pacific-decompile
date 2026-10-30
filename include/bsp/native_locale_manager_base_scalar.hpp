#pragma once

#include "bsp/native_locale_manager_constructor.hpp"
#include <cstdint>

namespace bsp {

// 00737010, CFEA5C slot0: capture receiver; call7365E0; only then test the
// actual caller-owned DWORD's low byte and optionally free the saved receiver.
// Return address bits only, without reading freed storage. The ordinary Source
// interface adds references; it is not Original ECX/stack/EAX/RET4 ABI.
void* delete_native_locale_manager_base_00737010(void* actual_receiver,
    const volatile std::uint32_t& actual_flags,
    const NativeLocaleManagerConstructorContext&);

// Require genuine base-profile selection, the SAME actual publication cells,
// matched heap allocation, and disjoint live flags/context/cells/backing through
// base retirement. This is not full73C9F0 or permission to discard its payloads.
// Destructor escape prevents flag read/free; no local cleanup or retry exists.
// Canonical dispatch/owner/startup and Native ABI/FH3/SEH/runtime remain unbound.
} // namespace bsp
