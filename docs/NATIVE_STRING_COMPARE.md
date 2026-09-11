# Actual-header string comparison and character replacement

This packet reconstructs complete functions `00425850` and `004259F0` in
`src/native_string_compare.cpp`. Both operate on the existing eight-byte native
string header: DWORD length at0, data pointer at4. The interface follows the
existing actual-header helpers in `native_string.hpp`; pass a `NativeString`
address or an actual header. It introduces no new owner, allocator, capacity,
or `std::string` state.

## Equality against a C string:00425850

Native ABI: ECX is the actual header; the candidate C-string pointer is the
single stack argument; `RET4`. **Only AL is the boolean contract.** Upper EAX
bits retain pointer/length-derived values on some null branches. For example,
null data compared with a260-byte candidate returns native EAX=`100h`, while
AL correctly means false. The C++ `bool` models AL, not a full-DWORD result.

| Captured data pointer | Candidate | Native result |
| --- | --- | --- |
| Null | Null | True, without reading length |
| Null | Nonnull | Scan the complete candidate through its terminator; true iff its length is zero; recorded header length is ignored |
| Nonnull | Null | True iff the recorded header length is zero; no buffer byte is read |
| Nonnull | Nonnull | Actual CRT `_stricmp(data,candidate)==0`; recorded length is ignored |

The null-data scan uses volatile byte reads so optimization cannot reduce its
observable full scan to a first-byte check. The nonnull pair delegates to the
actual CRT. It does not use an ASCII-only fold, independent locale, or synthetic
comparison callback. The original target at `00BF7FBF` retains its library
identity `__stricmp` and is not reconstructed or renamed.

## Mutable byte replacement:004259F0

Native ABI: ECX is the actual header; stack slots supply old byte, new byte,
and signed32-bit start; `RET12`. Only the low bytes of the first two slots are
consumed. No return value is established, and there are no calls.

Null initial data returns before the start/length logic. Negative start clamps
to zero; nonnegative start beyond length returns. A following unsigned
`index >= length` check also excludes the equal-length case. The loop captures
the old/new bytes, reloads data at every iteration, adds the DWORD index,
conditionally replaces a matching byte, increments the index with unsigned
wrap, and reloads length for the loop test. It includes embedded NUL bytes and
does not resize or automatically touch the terminator.

The implementation retains the data and length reloads. If a byte store aliases
the header's length, the new bound controls the next test. If it aliases the
data pointer, the next iteration uses that changed pointer. Unmatched bytes are
not rewritten. The caller must provide readable/writable storage for the actual
accesses; there is no invented null-header, capacity, or invalid-pointer guard.

## Existing helpers and adoption boundary

`00435C40` is a different operation: two native headers, with length equality
checked before comparison. It cannot substitute for this C-string operation.
Existing caller-local comparisons in `winmain_startup.cpp`, `vehicle_class.cpp`,
input, and GUI code consume projected C strings rather than this complete
header contract. They were inspected but are outside this packet's ownership;
no consumers or shared header were changed. There was no existing complete
actual-header replacement function to reuse. Descriptive names in this packet
are hypotheses, not recovered symbols; earlier00425850 ledger evidence is
preserved by appending.

## Evidence and verification

Read-only Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Both entries and full bodies already exist:
equality's last instruction is `425899 RET4`, length3, exclusive end `42589C`;
replacement ends at `425A2A RET12`, length3, exclusive end `425A2D`. No missing
entries or flow gaps were found. Current live prototypes still omit the native
register/stack arguments; the report supplies the corrected contracts for
parent-owned metadata updates.

Strict MSVC Win32 Release build and both existing CTests passed. Existing native
seed verification passed. One ignored differential fixture executes copied,
disk/Ghidra-matching game instruction bodies: nine equality cases and eight
replacement cases cover the null/recorded-length distinctions, AL-only result,
embedded NUL, signed starts, and actual length/data header aliases. Equality's
sole relative CALL is rebound to this process's genuine `_stricmp`; replacement
has no dependencies. This verifies the game control flow against the same CRT
boundary, not all locale behavior of the original VS2005 CRT. No original
process entry, game, IPC endpoint, or worker thread was executed.

These source interfaces are not drop-in binary replacements and are not
game-validated. The report records exact body hashes, call-rebinding scope,
verification results, and remaining consumer adoption.
