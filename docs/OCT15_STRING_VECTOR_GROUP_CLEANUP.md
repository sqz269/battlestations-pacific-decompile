# Actual string-vector group range cleanup

This packet reconstructs complete range `006A6EE0..006A6F27` and dispatcher
`006A7190..006A71A5` with the existing actual raw-pool context. Source is
`src/native_string_vector_group_cleanup.cpp`; declarations are in
`include/bsp/native_string_vector_group_cleanup.hpp`. Each range element is
an actual16B outer header owning actual16B inner checked-vector headers whose
backings contain8B strings. No new group class, host callback, allocator model,
validation guard or rollback is introduced. Source names are hypotheses;
existing Ghidra names and prior evidence are retained.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete cached pseudocode/native listings and
live72/22-byte envelopes, prototypes, direct callees and xrefs were checked
through `bsp.py ghidra`. Neither owned entry creates a local EH frame or catch.

| Entry | Original physical interface |
|---|---|
| `006A6EE0` | ECX first, EDX last; two unconsumed stacked DWORDs, RET8; no semantic result. Source retains those words and adds stacked raw-pool context. |
| `006A7190` | ECX opaque receiver word, incoming EDX unconsumed; stacked first/last, RET8; no semantic result. Source retains the physical words and adds context. |

The range captures endpoints once and walks by16 using DWORD wrap/equality.
Equal endpoints skip all field reads, pool work and frees. At each current
outer header, native code captures begin at +4; only nonnull begin reads end
at +8 and destroys the captured range of16B inner headers. It then reloads
CURRENT begin for free and unconditionally clears +4, +8 and +C in order,
including null-begin elements. +0 is opaque and is not written. Only after
successful per-element cleanup does the outer cursor advance by16. The
range does not release its own enclosing backing allocation.

The full per-element instruction/callee schedule matches accepted actual
`0055AFF0`: that helper destroys the16B inner range through `0055A3A0`,
frees current begin via the accepted current CRT service, and performs the
same ordered unconditional clears. The metadata difference is explicit:
`006A6EE0` saves original first in a private scratch word and duplicates it
as the second stacked word to `0055A3A0`, with current header as the first.
`0055AFF0` supplies current header twice. The complete accepted `0055A3A0`
body reads neither incoming metadata word, so this difference changes no
observable storage operation in the supported Source domain. Its internal
first capture and actual string-pool/CRT services are the accepted ones.
Consequently Source composes full `0055AFF0` once per current element.

The dispatcher loads its native stacked last twice into EAX/EDX, pushes last
then receiver as dead metadata, loads first into ECX and forwards to the
range. The receiver word is never dereferenced or treated as an allocator or
semantic owner. Source forwards the same DWORD values. Native private-stack
mutation between loads is outside this new by-value Source interface.

Native register residue is not made into a fabricated result. Both Source
interfaces are void; the dispatcher leaves the child's native residue, which
is the initially loaded last on an empty range. The additional context changes
stack cleanup. Original entry/stack/register and private spill identities are
not established by these signatures.

Cleanup failures propagate immediately in the current Source C++ domain.
They skip remaining current-element free/clears and later16B elements; earlier
effects remain. There is no catch, retry, synthetic rethrow or extra validation.
Captured range endpoints stay fixed while current-begin frees preserve later
provider effects. The caller provides valid reached storage and raw-pool/current
CRT lifetimes accepted by those services. No extra null, ownership, range,
alignment, capacity or overlap guard is supplied.

The older `destroy_native_input_settings_group_range_006a6ee0` in
`src/native_input_settings_vector_storage.cpp` remains unchanged as a separate
bounded settings-storage projection with `NativeStringStorage&`. Its native
fixtures and documented structural-mutation limits remain attached to that
projection, not to the new raw-context interface. Older host projections and
the pending cleanup-wrapper packet are also preserved separately.

The dispatcher has two incoming catch references, while the range is also
used by tidy/insertion/settings paths. Incoming instruction checks establish
interface evidence only. In particular, the settings destructor separately
frees CURRENT enclosing backing after this range returns; that free is not
added inside the new range service, and its caller is not promoted here.

Checked native CALL instructions:

```text
006A6EFF CALL 0055A3A0
006A6F08 CALL 00BF65AC
006A719E CALL 006A6EE0
0055B004 CALL 0055A3A0
0055B00D CALL 00BF65AC
006A7974 CALL 006A6EE0
006A7670 CALL 006A6EE0
006A77D9 CALL 006A6EE0
006AA579 CALL 006A6EE0
006A7604 CALL 006A6EE0
006A7810 CALL 006A7190
006A78A3 CALL 006A7190
```

Win32 Release and all three existing suites pass; CTest took8.95 seconds.
All12 report rows and12 documentation claims pass live checks, with no
failures, unchecked claims or warnings. Results are recorded in
`reports/oct15_string_vector_group_cleanup.json`.
No new tests are supplied; existing native math seeds do not exercise these
entries. Source/build proof does not establish native ABI, original heap/CRT,
handler/exception/FH3/SEH/fault or cleanup-failure unwind identity, original-game
execution or gameplay compatibility.
