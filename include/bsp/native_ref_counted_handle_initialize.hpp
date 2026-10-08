#pragma once

namespace bsp {

// Original 00415680..00415688: ECX is one writable DWORD slot; EAX returns it.
// Clear that word without reading or releasing its previous contents. Plain
// RET; no stack arguments, provider, class construction or lifetime management.
// ECX/EDX, flags, nonvolatile registers and FP state are untouched by the body.
void* __fastcall raw_initialize_native_ref_counted_handle_00415680(
    void* actual_slot) noexcept;

} // namespace bsp
