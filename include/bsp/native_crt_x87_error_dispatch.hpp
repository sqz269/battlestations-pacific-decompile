#pragma once

namespace bsp {

// ASSEMBLY CALLERS ONLY. These names do not describe ordinary safe C++ calls,
// a C++ numeric return, or an original-address/drop-in ABI binding. Enter with
// EAX=error type, ECX=actual name pointer, EDX=operation, ST0=current result.
// At entry ESP: return; +4: packed saved CW; +8: enclosing return/opaque slot;
// +0Ch: actual first argument qword; +14h: second argument qword for binary.
// On normal return ST0 is replaced by the current result reloaded from the
// error record. RET removes only this entry's return. EBP/nonvolatile general
// registers survive. Volatile EAX/ECX/EDX follow the bound C27489 provider;
// no incidental original volatile-register equivalence is promised. Final
// integer flags come from CMP of the current saved CW with 027Fh.
//
// Before entry bind a persistent actual LegacyCrtMathRuntime using the existing
// bind_legacy_crt_math_runtime API. The raw record's lifetime is this stack
// frame; direct legacy_crt_87except_00c27489 receives it plus the actual saved
// CW pointer. Its continuation changes to result are reloaded after return.
// The provider reads, but does not write, that caller saved-CW pointer. Its
// actual global/errno binding and documented reserved-FPIEEE, precision01 and
// FP-status scope limits remain. No generic callback or new runtime is added.
// There is no added EH guard or cleanup. Current x87 control/status/exceptions
// apply; unhandled faults/nonlocal exits need not return or restore the CW.

// Complete C08330 binary path: 23-byte prefix and 51-byte shared tail, 74
// reached original bytes within the original 83-byte physical union. Reads
// current second-argument words BEFORE the result FSTP; common tail then writes
// name/first argument, calls C27489, reloads current result and saved CW.
void __cdecl dispatch_native_crt_binary_error_00c08330();

// Original __startOneArgErrorHandling C08347 path: complete 60 original bytes.
// New public entry is a five-byte JMP thunk to binary entry+17h; that thunk
// adds no register/flags/stack/FP effects. It is an explicit source entry
// binding, not original entry-address or general ABI compatibility.
// The unary path leaves record.argument2 unwritten; it neither copies the
// second caller argument nor zero-initializes those eight stack bytes.
// Operation/record validity remains the caller's contract; use the binary
// entry when a valid second argument is required, including operation1Dh.
void __cdecl dispatch_native_crt_unary_error_00c08347();

} // namespace bsp
