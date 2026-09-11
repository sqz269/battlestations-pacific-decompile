#pragma once

#include "bsp/native_hardware_layout_owner.hpp"

namespace bsp {

// B47640: native ECX actual 0Ch record, EAX same address, RET. Ordered writes
// declaration+00 = null, frequency+04 = 1, extra+08 = 0; no retain/release.
void* initialize_native_hardware_layout_record_00b47640(void* actual_record) noexcept;

// B48C00: native ECX actual 44h owner, EAX same address, RET. Initialize
// CEB130/ref1, install D61D10, initialize exactly four records, then count0
// and stride0. The fixed B47640 leaf cannot throw on valid writable storage.
// Preserve derived COM+40 and the allocation's separate slab token+44.
void* initialize_native_hardware_layout_base_00b48c00(void* actual_owner) noexcept;

// B48A00: native ECX owner, stack declaration pointer, RET4. Actual atomic
// references and current D61D1C profile dispatch; uses the existing owner
// context's CPU declaration pool/type-size table. Retain the temporary,
// increment the current count before reading the target, publish a changed
// pointer before retain/release, write metadata1/0, then release temporary.
// No four-record bound, null substitution, overflow guard or rollback.
// Every reached address must be valid and every dispatched profile supported
// by NativeHardwareLayoutOwnerContext. Storage aliases are preserved.
void append_native_hardware_layout_stream_00b48a00(
    void* actual_owner, void* actual_declaration, NativeHardwareLayoutOwnerContext&);

// B47D60: native ECX owner, RET. Capture the initial signed-count test before
// clearing stride, then sum each current declaration+CC with DWORD wrapping.
// Reload count for each signed loop test. No null or bounds checks.
void recompute_native_hardware_layout_stride_00b47d60(void* actual_owner) noexcept;

// New MSVC Win32 interfaces; not original calling conventions or game proof.
} // namespace bsp
