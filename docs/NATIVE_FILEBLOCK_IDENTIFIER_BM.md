# Actual FileBlock identifier and nonempty split

This packet reconstructs `00BDF950..00BDFD7F` (1072 bytes) and
`00BD20A0..00BD216C` (205 bytes) over the actual Win32 string headers and
`NativeStringVectorStorage`. The source touches the FileBlock name at
`+14/+18`; the owner's other 20 bytes retain their existing contents.
The [report](../reports/native_fileblock_identifier_bm.json) records the
installed-image/live-Ghidra byte comparisons, native direct call rows,
exception tables, current dependency hashes and focused validation.

`split_native_string_nonempty_on_byte_00bd20a0` takes a borrowed actual source
header, delimiter byte, actual destination vector and `NativeStringStorage`.
It appends deep copies without clearing the destination. It skips leading,
repeated and trailing delimiters, scans by stored length, and constructs each
substring before arming its cleanup. Both scan phases capture source data;
source length is reread only after that iteration's temporary return. Embedded
NUL is scanned as a byte, while the existing `469840` substring still performs
native `strncpy` padding. Native ABI: ECX source, DL delimiter, stack vector,
RET4, no stable result contract.

`prepare_native_fileblock_identifier_00bdf950` accepts actual FileBlock storage
and the same string-storage domain. Native ABI: ECX FileBlock, no stack
arguments, RET, no stable result contract. It replaces bytes selected by the
exact `D68520` set: space, `<>=?:;"*+,./\|`. Its `strcspn` stopping offset can
point to an embedded NUL before the stored length; that byte is replaced and
the same offset is scanned again. A length at most 32 returns after this pass.

Long names split on underscore into the actual 0Ch vector of 8h headers. The
native signed comparisons use `count >= 3` and DWORD-wrapped `3*count-1 <= 32`;
ordinary valid counts 3 through 11 abbreviate. The original name is cleared
before abbreviation. For each token except the last, it constructs `_`, then
either a deep copy of the token when `atol != 0` or a one-byte string from the
token's first byte, concatenates token representation plus underscore, appends
that result, and releases concatenation, representation and underscore.
The final numeric token appends directly; the final nonnumeric token uses
`54AA70`. `atol` accepts decimal prefixes and signs; this is not a whole-token
numeric validator, and a zero-valued token abbreviates. Long numeric tokens
can leave the result longer than 32 bytes.

Other counts construct `substring(-64,64)` first and `substring(0,64)` second,
then concatenate **second plus first**, copy into the name, and return the
three completed temporaries in reverse order. The established substring body
makes the negative slice empty. An 81-byte name with two tokens therefore
becomes its first 64 bytes, without an added 32-byte clamp or replacement hash.

## Aligned stack and exception evidence

The identifier aligns ESP to 8 at `BDF953`, installs handler `CC6690`, subtracts
44h and saves EBX, the original frame register, ESI and EDI. Let `F` be the
aligned frame top and `S=F-60h` the steady body ESP. The EH state is `S+5C`,
and vector is `S+44 = F-1C`. The repurposed body EBP is zero initially and
later holds a returned header; it must not be used as the compiler-local base.
The handler's FuncInfo is `E00C1C`, with ten unwind entries at `E00C40`, no try
blocks. The action thunks use the handler's reconstructed frame register.

| State | Next | Action | Actual local |
|---|---|---|---|
| 0 | -1 | CC6640 -> 427880 | vector F-1C |
| 1 | 0 | CC6678 -> 41DD20 | fallback negative slice F-34 |
| 2 | 1 | CC6680 -> 41DD20 | fallback prefix F-2C |
| 3 | 2 | CC6688 -> 41DD20 | fallback concatenation F-24 |
| 4 | 0 | CC6648 -> 41DD20 | numeric underscore F-4C |
| 5 | 4 | CC6650 -> 41DD20 | copied numeric token F-34 |
| 6 | 5 | CC6658 -> 41DD20 | numeric concatenation F-3C |
| 7 | 0 | CC6660 -> 41DD20 | byte branch underscore F-44 |
| 8 | 7 | CC6668 -> 41DD20 | one-byte token F-24 |
| 9 | 8 | CC6670 -> 41DD20 | byte concatenation F-2C |

Only a successfully returned temporary advances the caller's state. Callee
substring/concat cleanup remains in its existing source body. The source keeps
these state transitions; it adds no rollback of an incomplete constructor or
the vector reserve's native partial allocations. On normal exit `BDFD53`
sets state -1 before `427110(count0)` and freeing current vector backing at
`BDFD65`. Unwind state0 instead calls actual `427880`, preserving reverse name
returns and current backing free. Split has handler `CC55B8`, FuncInfo
`DFF674`, map `DFF66C`: its sole state0 action `CC55B0` destroys the completed
substring at reconstructed frame-14 through `41DD20`.

## Membership repair and validation limits

`BDFD6A..BDFD6C` is the decoded `83 C4 04` (`ADD ESP,4`) following the final
free. Both endpoints currently have no containing Ghidra function. A later
coordinated annotation batch should attach precisely these three already
decoded bytes to `BDF950`'s body, preserve its name/comments, refresh its export
and save. This packet performs no Ghidra mutation, listing repair, no-return
change or script enablement. The earlier three-byte alignment padding at
`BDF97D..BDF97F` is not executable continuation and is left alone.

The strict MSVC Win32 translation-unit compile and ignored fixture pass. The
fixture uses a raw 1Ch FileBlock, actual 8h strings and the 0Ch vector, with a
tracing string allocator. It checks numeric/zero-token abbreviation including
a result longer than 32, final-byte append, the 64-byte fallback, embedded-NUL
sanitization, split append/padding, unchanged owner prefix, and one allocation
failure during numeric concatenation. The failure checks the exact returned
allocation order: concat's partial output, token copy, underscore, and reversed
vector strings. The frozen library and fixture hashes appear in the report.

These explicit-storage C++ interfaces are not original binary ABI replacements.
The existing `NativeStringStorage` boundary has a `noexcept` return and permits
throwing host allocation; it cannot reproduce a native pool-getter exception
on return. The host CRT supplies `strcspn` and Win32 `atol`, rather than mapping
the original CRT's locale or errno state. Verified empty fallback `0109CEF0`
is represented by an empty literal; mutation of that native cell is outside
this interface. Valid distinct compiler locals, valid actual storage and the
existing dependency allocation/overlap domain are required; arbitrary stack
aliases, hardware faults and exact FH3/SEH identity are unverified. The primary
integrator owns CMake/runtime wiring and repository-wide build checks. No game
execution, complete FileBlock lifetime binding or gameplay validation is claimed.
