# Actual physical file dates

This packet implements fifteen original bodies against caller-supplied Win32
storage, covering the established physical provider's complete date, replacement,
path and indexed-existence routes. It does not construct or populate the index.
The source is in `src/native_physical_file_date.cpp`; public contracts are in
`include/bsp/native_physical_file_date.hpp`. Descriptive names remain hypotheses.
The audit report contains the original bytes, current dependency hashes and
verification results.

The higher-level source domain is the established `D69168` physical profile:
current slot `+10 = BF3F70`, `+18 = BF39C0`, and `+1C = BF3970`. The implementation
reads the current table and selected word at each reached virtual call. Another
word raises an explicit **source boundary** (`std::invalid_argument`); this is
not a native exception or a claim that arbitrary original provider profiles are
implemented. No unknown slot is replaced by a callback or a guessed provider.
The complete-body claim for these routes is qualified to this concrete domain.

## Original bodies and ABIs

All addresses are original virtual addresses. `out` means caller-supplied raw
storage, not a projected object. All returns below are the specified result;
incidental high EAX bits on Boolean returns are not a C++ contract.

| Address | Bytes | Original inputs and return |
|---|---:|---|
| BF3A80 | 399 | ECX provider; stack out/name; EAX out; RET8 |
| BF39C0 | 188 | ECX provider; stack mutable name; AL Boolean; RET4 |
| BF3970 | 65 | ECX provider; stack out/suffix; EAX out; RET8 |
| BF3F70 | 586 | ECX provider; stack suffix; AL Boolean; RET4 |
| BF36D0 | 102 | ECX tree; stack iterator-out/key; EAX out; RET8 |
| BDA260 | 83 | ECX tree; stack key; EAX lower-bound node; RET4 |
| BD9860 | 99 | ECX iterator; no specified result; RET |
| BD92C0 | 41 | ECX left iterator; stack right iterator; AL equality; RET4 |
| 435C40 | 89 | ECX left header; stack right header; AL equality; RET4 |
| 449AF0 | 77 | ECX left header; stack right header; AL inequality; RET4 |
| 467CF0 | 95 | ECX source; stack needle/unsigned limit; EAX signed position; RET8 |
| 4261A0 | 166 | ECX left header; stack out/right; EAX out; RET8 |
| 425F40 | 53 | ECX destination; stack source; EAX destination; RET4 |
| 41E350 | 69 | ECX destination; stack nullable C string; EAX destination; RET4 |
| 41E870 | 83 | ECX destination; stack nonnull C string; EAX destination; RET4 |

These fifteen complete spans total **2,195 bytes**. BF3A80 crosses the saved
BF3ACC function split. BF39C0 still has no saved function record; its full body
ends at BF3A7B, including RET4. No function was created or annotated here.

## Storage and dependency contract

The physical record has root length/data at `+8/+C`, last-success length/data at
`+20/+24`, accept-all byte at `+28`, index tree at `+2C`, head at `+30`, and count
at `+34`. The pinned BF4D30 constructor writes D69168 and initializes these
fields. The reader accepts existing actual index storage; the nonempty-index
population writer remains unestablished.

Tree `+4` holds the head; head `+4` holds the root. Index nodes have left/parent/
right at `+0/+4/+8`, basename length/data at `+C/+10`, full name length/data at
`+14/+18`, and nil byte at `+1D`. Iterators contain owner/node DWORDs at `+0/+4`.
No tree snapshot, map, normalization pass, stream or provider lifetime is added.

The context borrows the application's actual `0109CEEC` publication slot by
reference. BF3A80 loads it afresh and reads that manager's current byte `+78`;
the provider is not a substitute manager. The string dependency is explicitly
`ActualNativeStringPoolStorage&`, bound to the application's actual publication,
small-return gate and canonical lifetime domain. Its allocation and release
paths already call the current owning-pool getter. There is no default CRT
allocator or separate lifetime domain in this adapter.

The reusable raw-header adapters retain the existing `NativeStringStorage&`
interface so other callers can compose the same owning bridge. Actual resize
41DD40 and destroy 41DD20 come from `native_string.cpp`; substring 469840 and its
copy constructor 426060 come from `native_pooled_string_substring.cpp`.
Lower-bound/find reuse the actual-header comparator 443D00 supplied by the
leaf-provider packet. No duplicate implementation is introduced for that address.

## Ordering and exceptional paths

435C40 first compares the recorded lengths; two nonempty equal-length headers
then use the current CRT `_stricmp`. 449AF0 gates only on zero lengths, so two
different nonzero lengths can compare equal. Reverse search 467CF0 uses an
unsigned minimum for the limit, a wrapping subtraction, and a signed-positive
position test. It never tests position zero. Each attempt rereads the current
header fields before CRT `strncmp`.

425F40 preserves identity; otherwise it resizes preserving data, rereads the
source length, and copies the current destination count. 4261A0 clears its
output even on identity, completes the initial copy, then arms output cleanup
for append failure. No cleanup is invented for initial-copy failure. Both
CString adapters perform the native inline byte scan. 41E350 uses preserve=false
and copies the current length; 41E870 clears the header first, uses preserve=true
and copies current length+1. Neither releases an abandoned pre-constructor buffer.

BF36D0 lower-bound runs before null-owner validation. Its fallback rereads the
current head after comparison, and output owner/node values are captured before
publishing them. BD92C0 validates owners before comparing current node fields.
BD9860 preserves each parent-climb iterator write. A nil-node advance tail-calls
the native invalid-parameter routine: a returning handler returns directly to
the caller. Other validation sites continue their native reads after a returning
handler. The existing `SingletonLifetimeCallbacks::invalid_parameter` is this
CRT boundary, not a provider implementation.

BF3970 concatenates root and suffix without inserting a separator, then rereads
the root length and converts only suffix-region `/` bytes to `\`, rereading the
output length/data as it walks. BF39C0 calls current +10 first; false leaves the
name unchanged. True invokes current +1C, copies from the **returned pointer**,
and destroys the hidden temporary. Cleanup is armed only after the builder
returns, and is disarmed before normal release.

BF3F70 checks empty name, equal-length cached success, then accept-all. Otherwise
it builds a path and rereads accept-all/count. Empty-index mode calls the real
`GetFileAttributesA`; any value except FFFFFFFF, including directory attributes,
is success. It caches the original suffix on success and assigns empty on miss.
Indexed mode derives the basename by searching `/`, reads the actual tree,
checks every equal-basename node's full name, and advances until the captured
end or a different basename. It performs no OS query or cache update. The slash
temporary has no newly invented cleanup state; basename and path states follow
the pinned unwind map.

BF3A80's disabled route writes five zeros from +10 down to +0 without reading
provider/name. The enabled route zero-initializes its copied name and copies
before arming cleanup. It ignores the replacement call's AL and calls real
`GetFileAttributesExA`, then `FileTimeToSystemTime` on the last-write FILETIME.
The conversion BOOL is ignored. Native stack +28 clears SYSTEMTIME seconds and
milliseconds; +2C clears a separate miss-result DWORD. Year/month/day and
hours/minutes remain unspecified before conversion. There is no added failure
exception or guaranteed zero tuple if conversion fails.

An attributes miss captures the original name data, calls `GetLastError`, and
reaches the full RET-only diagnostic 4254B0. Its absence of behavior is preserved;
no logger or formatter is invented. Normal output is ascending year, month, day,
seconds since midnight, milliseconds. The copied path's data pointer is captured
after output +C but before output +10; its release uses that capture and the
current copied length+1. Output publication precedes normal cleanup.

C++ exceptions follow the recovered ownership states, but this is not a binary
SEH replacement. Existing storage release is noexcept; a C++ exception while
recreating its pool terminates under that established interface. Existing
actual-string helpers omit zero-byte memcpy; the new adapters retain that
documented host boundary. Current CRT behavior and Win32 ANSI path behavior are
retained without added path checks, conversion guards or stream callbacks.

## Verification and integration

The final source passed the standard strict MSVC Win32 build (`/W4`, `/WX`,
`/fp:strict`) through an ignored CMake source hook, both existing CTests, and all
eight seed checks. No shared CMake or test source changed.

One ignored probe links the actual built `bsp_core.lib`. It compares eight
unchanged original bodies (667 bytes, including shared comparator 443D00) in a
common relocated arena, with CRT call destinations bound to the same host CRT.
Eighteen comparisons check returns and all 1,024 arena bytes, including length
semantics, reverse-search position zero, aliased find outputs, both iterator
successor branches, and returning invalid-parameter sites. These dynamically
cover the seven nonallocating helpers owned here; the other eight owned bodies
are not claimed as original-body differential fixtures.

The same probe performs thirteen checks through the actual owning pool and
rebuilt higher-level source: a real file with a fixed UTC last-write time,
path slash conversion, a changed manager publication disabling dates, attribute
miss, duplicate-basename indexed hit, next-basename miss, unchanged index cache,
empty/accept-all ordering, and cached success. The index is fixture-supplied
reader input, not evidence of an original population routine. No forced
conversion failure or native unwind exception was dynamically compared.

The audit pins 29 fresh guarded original spans (3,104 bytes), full-byte installed
PE agreement, and exact current source/build/probe artifacts. The 0109DBEC empty
byte was separately verified in Ghidra's zero-filled image region; it is not
misreported as a byte physically stored in the PE file. Input installation and
saved analysis were preserved. Reconstruction, build checks and scoped fixtures
are complete; original ABI compatibility and game validation are false.

The primary integrator still owns shared source registration and address/name
metadata. The upstream BDD340/BD9E80 actual visitor and manager traversal remain
separate integration work. These physical functions do not claim additional
provider profiles, general VFS closure, index construction or MSAR startup
reachability.
