#pragma once

namespace bsp {

// ASSEMBLY CALLERS ONLY. Neither declaration describes an ordinary safe C++
// call: input/output lives on the x87 stack and the second entry also returns
// register/flag state that C++ does not model. No host exp2/pow is called.
// Current x87 control, rounding, precision, status, stack and exception state
// apply. These entries do not save/reset that environment or translate faults.

// Complete original 00C08390..00C083A5 (21 bytes), descriptive source name.
// Requires ST0 input and two free x87 stack slots. On normal return replaces
// that ST0 with the original FRNDINT/F2XM1/FSCALE sequence's result. General
// registers and integer EFLAGS are untouched. No stack arguments, plain RET.
// Caller must consume the resulting ST0. This is not a C++ floating return.
void __cdecl exp2_native_crt_x87_00c08390();

// Complete original 00BFED32..00BFED5A (40 bytes) with one declared operand
// adaptation: FMUL [00E15850] becomes FMUL [EDX+disp32(0)], same six-byte size.
// EDX borrows a live half cell corresponding to original E15850, expected
// immutable 0.5 (bytes 00 00 00 00 00 00 E0 3F) under the binding contract;
// that cell is read only when FMUL is reached.
// No value snapshot, context copy, constant replacement or callback is used.
// ECX has no input meaning to the algorithm, but upper 24 bits survive; only
// CL is written. AX receives the last waiting FSTSW status; upper EAX survives.
// EDX and nonvolatile registers survive. Both exits use RET, no stack args.
// Requires ST0 input and one free x87 slot. On every normal path consumes the
// original ST0, leaving the older x87 stack beneath it. CL is 0 if the first
// equality test rejects, 1 if the first accepts and second rejects, or 2 if
// both accept. This gives integer parity where rounding/precision leave the
// half-product classification exact; reduced x87 precision can round a large
// odd integer's half to an integer. Unordered/exceptional behavior follows
// the instructions;
// CL is not a validated C++ enum or a general nonfinite classification.
// Integer flags follow the final executed SAHF or INC CL, including retained
// bits; caller must capture needed CL/AX/EFLAGS before clobbering them. Both
// original waiting FSTSW AX operations remain, including FWAIT delivery of
// pending unmasked x87 exceptions and the then-current status-word read.
// An unused ECX argument only
// places the borrowed pointer in EDX; it does not make C++ invocation safe.
void __fastcall classify_native_crt_power_exponent_00bfed32(
    void* unused_ecx, const void* actual_half_00e15850);

} // namespace bsp
