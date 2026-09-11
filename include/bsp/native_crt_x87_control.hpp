#pragma once

namespace bsp {

// ASSEMBLY CALLERS ONLY. These declarations name custom stack/register entries;
// they are not ordinary safe C++ calls or generalized host FP-environment APIs.
// Neither entry saves the environment, clears status, catches exceptions or
// guarantees return after a fault. Current x87 state and FLDCW effects apply.

// Complete 00C083A5..00C083BC (23 bytes). Enter using CALL with a readable and
// writable packed CW DWORD at entry ESP+4, beneath the newly pushed return.
// Reads that current DWORD into EDX, ANDs 0300h, ORs 007Fh, writes DX to the
// upper word of the same slot (entry ESP+6), then FLDCWs that current word.
// The low saved-CW word is untouched. EDX returns the computed working word;
// integer flags remain those of OR EDX,7Fh. Other general registers survive.
// RET removes only its return address; caller still owns the packed DWORD.
// The implicit packed stack slot is deliberately not modeled as a C++ value.
void __cdecl prepare_native_crt_x87_control_00c083a5();

// Complete 00C0842E..00C0843B (13 bytes). JMP/TAIL ENTRY ONLY: never CALL this
// entry normally. At entry ESP points directly at the readable packed CW
// DWORD, and the real return address is at ESP+4. CMP reads the low word;
// if it differs from 027Fh, FLDCW rereads that current word. The equal path
// skips FLDCW even if the live x87 environment has changed independently.
// POP EDX then reads/consumes the full current packed DWORD; RET consumes the
// real return address below it. EDX returns that current DWORD, which need not
// match the earlier low-word comparison if actual stack storage changes.
// Integer flags remain those of CMP word ptr [ESP],027Fh; other general
// registers survive. No x87 register-stack value is pushed or popped.
void __cdecl restore_native_crt_x87_control_tail_00c0842e();

} // namespace bsp
