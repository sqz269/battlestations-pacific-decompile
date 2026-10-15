# Actual pooled-string in-place range services

This packet completes the ordinary Source effects of `0044BC50`, `0044C770`,
`0044D850` and `0044F170` over actual8h pooled-string headers. Original bodies
and register/stack schedules were read in full. Descriptive C++ names are
hypotheses; existing Ghidra STL names are retained. The saved project/program
was verified as `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

| Entry | Original ABI and behavior |
|---|---|
| `0044BC50` | cdecl/plainRET; three consumed pointers, known dispatchers supply three further ignored words. EAX decremented destination end. Capture endpoints; decrement source and destination by8 before identity comparison and assignment. Empty returns destination end without pointed access. |
| `0044C770` | cdecl first/last/value-header, plainRET, no semantic result. Capture the value address and walk destination forward by8; value fields remain live at every assignment. Identical headers skip all reads; empty skips value access. |
| `0044D850` | cdecl first/last/destination-end, plainRET. Send six physical words to backward child, then independently return captured destination-end minus wrapped SAR3(last-first)*8. Child EAX is ignored. |
| `0044F170` | ECX owner; stacked first/last/destination, RET0Ch, EAX child end. Complete consumed-input and private dead-word schedule matches accepted `00505AD0`. |

Each assignment composes the accepted complete actual `0048DA50` Source body
for a single captured8h source span. Its identity check, captured source
length for resize(preserve1), CURRENT source-length recheck, then CURRENT
destination length/source data/destination data reads exactly match both
inline native assignment schedules. It performs the accepted nonzero
overlap-safe copy. No replacement host string, buffer snapshot, callback,
allocator or generic STL algorithm is introduced. Backward traversal remains
backward; fill keeps its source-header address rather than copying its value.

Addresses and return computations use wrapping DWORD arithmetic with explicit
SAR3 sign extension. New Source entries add an actual raw-pool context. Known
dead metadata words and private-frame preimages are not external semantics;
Source dispatch supplies zero for unused tags. Valid mapped taken accesses,
actual pool/publication/manager lifetimes and child ownership contracts are
required. No range/alias validation, exception cleanup or rollback is added.
Current C++ exception/CRT overlap-copy/zero-byte-copy limits are inherited.
Native private frames, ambient flags/registers, original CRT/FH3/SEH/fault
identity and full binary ABI are not promised by the new interfaces.

An installed-PE comparison confirms full37B `0044F170` and `00505AD0` agree on
all33 nonrelocation bytes. Only the four relative operand bytes differ;
both targets resolve independently to `0044D0C0`. Their existing actual
uninitialized-copy and completed-prefix cleanup service is therefore reused.

Exact checked native sites:

```text
0044BC74 CALL 0041DD40
0044BC89 CALL 00BF7680
0044C78E CALL 0041DD40
0044C7A3 CALL 00BF7680
0044D87E CALL 0044BC50
0044F18C CALL 0044D0C0
```

Win32 Release and all three existing suites pass. A focused local comparison
uses the full original82B backward body with two fixed adapters to the same
accepted Source raw resize and current-CRT memmove services. Three one-byte
strings initially contain A/B/C; copying the first two into the right-overlap
destination must produce A/A/B, with unchanged length/data-pointer fields and
the expected returned destination cursor. Original and Source agree on all
checks; two resize and two copy invocations occur. Equal lengths require no
pool traffic. This is a bounded original-loop/Source-child composition proof;
it does not independently test Original children, allocation, exceptions,
other paths, fill, full ABI or gameplay. No tracked test suite is added.

The enclosing general insert-count function remains a separate reconstruction
packet. Its other branches and original EH ownership must be recovered
independently; these helpers do not establish its completion. Current typed
metadata resolves the older cached tail-gap report as stale; no live listing
repair gap is established.
Exact inputs, bytes and probe provenance are in the packet JSON report.
