#pragma once

struct _EXCEPTION_RECORD;

namespace bsp {

// Complete native C2F25C[6] import thunk. Preserve the four actual Win32
// stdcall arguments and tail-jump to the real Windows RtlUnwind import.
// Supplies no OS exception implementation, validation or catch policy.
void __stdcall rtl_unwind_import_00c2f25c(
    void* actual_target_frame, void* actual_target_ip,
    _EXCEPTION_RECORD* actual_exception_record, void* actual_return_value);

// Complete native _EH4_GlobalUnwind, C0DCE6[26]. ECX supplies the actual
// target registration in the current native exception/unwind domain.
// Save EBP/EBX/ESI/EDI, invoke the real thunk with (registration, this entry's
// own restoration label, nullptr, nullptr), restore, then RET. The actual API
// must supply the stack state expected by that continuation; no frame, handler,
// cookie or FS owner is created. EAX/flags remain whatever the API produces.
void __fastcall global_unwind_native_crt_frame_00c0dce6(
    void* actual_target_registration);

// Both entries are naked Win32 instruction interfaces. Native frame validity,
// abnormal/nonlocal exits and OS unwind behavior remain actual caller/API
// obligations. No ordinary C++ unwind service, runtime equivalence or complete
// C17653/dispatcher ownership is established by these source symbols.
} // namespace bsp
