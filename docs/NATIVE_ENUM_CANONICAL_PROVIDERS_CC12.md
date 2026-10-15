# Canonical enum pool and empty-string providers

This packet supplies ordinary Source bindings and complete lifecycle wrappers
for the distinct enum table E175E8 and enum symbol E17578 owners, plus a
distinct retained E186ED initial-NUL authority. Existing enum wrappers,
generic allocator algorithms, CMake, ledgers, Ghidra and application wiring
are unchanged. No CEnum object, fake phase graph or global registry is made.

## Original closure and direct accepted providers

The accepted Original recovery is
`reports/cc12_enum_pool_lifecycle_Original_review.json`: four complete bodies,
64 bytes and 16 instructions in total, with matching disk/live bytes and
saved/exported annotations. Their ordinary schedules are:

| Whole range | Owner and behavior |
| --- | --- |
| CC8A10..CC8A26 exclusive, 22B/6 instructions | ECX=E175E8; call whole00411050; register CD9290 through real BF6FF5 atexit; return actual registration result. |
| CC8A50..CC8A66 exclusive, 22B/6 instructions | Same schedule for DISTINCT E17578 and CD9280. |
| CD9290..CD929A exclusive, 10B/2 instructions | Same E175E8 owner; tail-jump whole00410A60. |
| CD9280..CD928A exclusive, 10B/2 instructions | Same E17578 owner; tail-jump whole00410A60. |

The existing actual38h owner/14h slot generic allocator and canonical E188B4
list contracts are reused, not re-proved. E17540/E175B0 property owners remain
distinct. Alias callbacks CD9F70/CD9F80/CDEB60 are outside this packet.

## Explicit Source startup and lifetime

Each low-level binding first installs the accepted real CE37A4/00410CD0 trim
service, then retains that exact owner/list pair. Its initializer calls
whole00411050 and real `std::atexit` with its matching complete Source
callback. It returns the real registration status without a new failure
rollback. The callback destroys the SAME owner through whole00410A60.
These raw wrappers have no once guard or retry policy, matching Original's
ordinary sequence; their binding is a one-time host precondition.

`GameNativePhysicalPoolProcess` appends two independent aligned38h cells,
mutexes, startup states and status caches. Explicit methods
`initialize_enum_table_pool_once_00cc8a10()` and
`initialize_enum_symbol_pool_once_00cc8a50()` mirror the accepted property
owner Source policy. Under each owner's mutex, returned state reuses the
cached real result; threw state rejects retries. A fresh attempt commits
threw BEFORE binding/construction and becomes returned only after the raw
initializer returns. Nonzero registration status still leaves an initialized
owner; no callback registration or rollback is invented.

The storage accessors require their own returned state and return the same
private process cells. Construction/factory access starts no native pool.
All owners retain the same canonical list, not new per-owner lists. Their
separate mutexes do not serialize shared-list mutation: callers must provide
that serialization. Keep the process, list, providers and bindings alive
through CRT callbacks; retire real enum payloads, pooled keys and all table/
symbol slots BEFORE pool pages. A numeric phase word or borrowed pool pointer
alone does not establish payload provenance or ownership permission.

The process factory completes its existing C++ bookkeeping construction and
registers its destructor before callers can perform explicit pool startup.
Reverse CRT ordering keeps that process/list available to its registered pool
callbacks. Registration failure, repeated raw lifecycle, malformed owners,
overflow, arbitrary aliases, reentry, concurrent trim/mutation and fault or
exceptional parity remain subject to accepted provider limits.

## Verified append-only layouts

Compiler layouts verify every established field's offset and declaration.

| MSVC Win32 Source layout | Before | After |
| --- | --- | --- |
| Physical process size | 180h | 260h |
| E175E8 cell / mutex / state / result | absent | 180h / 1B8h / 1E8h / 1ECh |
| E17578 cell / mutex / state / result | absent | 1F0h / 228h / 258h / 25Ch |
| String process size | 40h | 40h |
| Existing library E18560 byte | 3Eh | 3Eh |
| New enum E186ED byte | tail padding | 3Fh |

The new enum byte consumes former tail padding; size growth was not assumed.
The string constructor and retained heap factory physically zero the DWORD
at3Ch, including the new byte at3Fh. Its getter returns exactly `this+3Fh`.
Old property/query/clone/library byte roles remain distinct and unchanged.
Only a const pointer escapes for the new stable-zero Source authority;
Original writable DATA is not claimed universally immutable.

## Validation and limits

Before-edit strict objects and layout output are pinned. The first strict
changed-TU compile passed for all three TUs using MSVC Win32
`/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict` (C++17/current MD CRT).
Complete before/after COFF includes all code, symbols, auxiliary records and
code/metadata relocations. All 23 old generic pool code sections and bindings
are unchanged. Every old process member except the two constructors and two
factories retains byte-identical code; those four affected bodies are reviewed
with the new methods, callbacks, EH funclets and real cookie relocations.
SSE block copies/zeroing are ordinary compiler memory operations here.

The required repository build preserved its original handle to natural
success. Both configured existing CTest checks passed in6.51s. These existing
TUs are registered, and the build compiled the changed files. Source, layout,
strict and build receipts are in `local/WholeEnumCanonicalProvidersSource01/`;
the report records full hashes and review readiness.

No new test, probe executable or target runtime ran. This does not establish
Original register/class/FS/SEH ABI, historical/private CRT, exceptional/fault
equivalence, populated enum traffic, global application startup/publication
or game behavior. Ordinary Source/strict readiness is separate from primary
integrator semantic admission and any later Main artifact/runtime gate.
