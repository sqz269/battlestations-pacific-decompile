#pragma once

namespace bsp {

// Original 00B106C0: ECX actual writable 12Ch-byte diagnostics record;
// EAX retains that same address, ESI preserved, no stack arguments, RET.
// Initializes raw empty storage in original DWORD store order. The caller
// owns its lifetime and must not use this as destruction of an existing record.
void* __fastcall initialize_native_material_diagnostics_record_00b106c0(void*);

} // namespace bsp
