# Legacy CRT math exception runtime

`legacy_crt_87except_00c27489` reconstructs the installed VS2005 CRT exception
path used by the camera-axis square root. Its three cdecl arguments match
`CameraAxes87Except`: operation, a 32-byte CRT math exception record, and a
pointer to the caller's saved 16-bit x87 control word. The production adapter
performs actual x87 operations and calls Windows `RaiseException` directly.

Bind a persistent `LegacyCrtMathRuntime` before installing that function pointer.
It borrows the actual `00E16BD0` matherr-bypass global and the owning CRT's
per-thread errno accessor. The rebuilt process can supply its host `_errno`;
an adapter running within the original process must supply that runtime's
`00BFFB8B`. The original disk-backed bypass word is `0x2694`, not zero. Binding
does not initialize game TLS or copy the original game's security cookie.
Rebinding is atomic; previous binding storage must remain alive until existing
calls finish. Missing binding is an installation error checked before effects.

## Recovered behavior

The native dispatch table maps exception types 1/5 to invalid, 2 to divide by
zero, 3 to overflow plus inexact, 4 to underflow plus inexact, and 8 to inexact.
Type 7 becomes type 1 without entering the handle/raise route. Other types skip
that route. The mask is an internal CRT mask, distinct from x87 status bits.

`__handle_exc` at `00C13364` generates masked exception status through the
original x87 instructions and 80-bit constants at `00E166B0` and `00E166BC`.
Overflow chooses signed infinity or maximum finite binary64 according to the
saved rounding mode. Underflow decomposes the scaled intermediate using
`00C12F99`/`__set_exp` at `00C12EB0`, subtracts 1536 from the exponent, and shifts
the significand while tracking discarded bits. It preserves the original
zero, sign, NaN, and binary64-spill behavior. It does not use modern `sqrt`,
`frexp`, `_matherr`, `_controlfp`, or MXCSR as substitutes.

An unhandled mask enters `__raise_exc` at `00C13322`, which fixes the final
`__raise_exc_ex` argument to zero for binary64. `__raise_exc_ex` at `00C13048`
fills a 112-byte FPIEEE record and raises a continuable Windows exception with
one argument pointing to that record. Cause priority is invalid, divide by
zero, overflow, underflow, then inexact. The record includes current x87 status,
enables derived from the saved masks, precision, rounding, operation, operands,
and result. Operations `0x10`, `0x16`, and `0x1D` include the second operand;
operation 5 never reads the uninitialized second argument of unary sqrt.

After a continuing SEH/VEH handler, the routine reads the modified result and
control fields. Enable bits only clear corresponding mask bits; zero enable
bits do not set new masks. Assembly `00C132A8` loads `EDX=FFFFF3FF`, and the
precision restore at `00C132F2..00C13308` reuses it: it clears rounding bits
instead of precision bits before ORing the requested precision. This native
quirk is retained and confirmed by differential execution. The final raw
`FLDCW` changes the live x87 word; the caller's saved word is unchanged.

The installed function at `00C28545` is exactly `XOR EAX,EAX; RET`, consistent
with default `_matherr`. There is no indirect user callback in this body. The
adapter preserves the bypass-word read and executes the resulting errno route:
type 1 writes 33, types 2/3 write 34, and other types leave errno alone. Result
mutation comes from the real exception continuation route. The new C++ frame's
compiler-generated security checks belong to its own host runtime.

## Evidence and validation

`reports/legacy_crt_math_audit.json` records original ABIs, addresses, complete
body hashes, global bytes, assembly details, annotation preimages, and proposed
comment appends. Existing matched library names are preserved. Unmatched
library-role descriptions remain interpretations rather than recovered symbols.
The verified Ghidra target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; no analysis was imported into another project.

MSVC 19.51 Win32 `/W4 /WX /fp:strict /O2 /MD` compiled the new source. The
repository build and its two existing tests passed; integrator-owned CMake
registration is separate from that worker build. One ignored native
differential helper compared this implementation with 13 original function
bodies, whose complete bytes match both live Ghidra and the installed PE.
Its masked process passed 5,376 comparisons; its separate SEH process passed
16,128 comparisons including 12,096 actual continued Windows exceptions.

That helper relocates explicit absolute memory operands in the verified code,
uses the actual original constants and default-matherr body, and retains the
native security-cookie check with its original disk word. It binds the native
RaiseException IAT to Windows and redirects only the native `_errno` entry to
the same actual host CRT accessor used by the reconstruction. It does not
start the game or execute the original CRT initialization. Checks cover types
1..8, 14 binary64 classes/values including signed zero, subnormals, infinities,
quiet/signaling NaNs, all four rounding modes, 24/53/64-bit precision, bypass
words 0/0x2694, unary/binary records, and three continuation edit policies.
Compared outputs include result bits, type, errno, saved/live control words,
x87 exception flags, exception codes, and documented FPIEEE fields.

Reserved FPIEEE bytes are unspecified native stack storage; the reconstruction
zeros them. Reserved x87 precision encoding `01` has no established contract.
The differential checks exception flags, not instruction/data pointers or all
x87 condition-code bits. The unused binary32 `__raise_exc_ex` variant is not
exposed by this adapter. Full original CRT TLS/startup, arbitrary user patches
to its default `_matherr`, drop-in binary replacement, and gameplay validation
remain outside this bounded reconstruction.
