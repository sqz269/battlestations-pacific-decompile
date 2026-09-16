# Raw Particle component Lua reader

Packet `orch4-particle-component-reader-r22` reconstructs the complete 657-byte
`00871D00..00871F90` body. Native ECX is the actual 34h Particle component,
the sole stack argument is an actual 14h LuaObject, and the body returns with
`RET4`. EDX is incidental. Names are descriptive hypotheses.

## Actual storage and behavior

The concrete `00868BF0` base reader executes first. The reader then gets
`Particle` and `UnderWater` from the supplied definition. UnderWater uses
`00B66000`, the **boolean type predicate**: both Lua `false` and Lua `true`
produce byte1; numbers, strings and nil produce byte0. This is not the Lua
boolean-value getter. UnderWater's Lua object is destroyed before the Particle
table predicate runs.

For a table, the actual Lua iterator yields key/value objects and the reader
loads each **value**, in Lua iteration order. An empty table appends nothing.
For a scalar, the Particle object's string supplies one resource name. Both
paths invoke the actual singleton getter, then concrete acquisition on its
returned owner+4 with `(name, 0, 0, 1)`. Acquisition's cache hit retains the
resource; a fresh load uses its initial reference without an extra retain.

The temporary name is returned to the current raw string pool before the
resource's +70 underwater byte is written. The table branch captures the
temporary data and length before acquisition; the scalar branch rereads them
after acquisition. The normal name return leaves its header stale. On an
exception while acquisition is active, the name unwind instead reads the
current header through `0041DD20`.

The component's actual pointer-vector header at +28h is reused. If current
count equals capacity, doubled DWORD capacity is interpreted as signed and
clamped to1 when <=1, then passed to `0086A4D0`. Count and data are reread after
reserve. The pointer store is skipped only when the computed DWORD destination
address is zero; count is incremented regardless. Existing entries are kept.
No new resource reference is acquired during append, and no resource is
released if name return, reserve or a later operation throws. There is no
rollback of previous appends, base fields, or underwater writes.

## Persistent source invocation and domains

The source invocation owns one stable actual8h name and three actual14h Lua
objects. Each cache acquisition has its own retained typed child invocation,
including the genuine loader/resolver/parser descendants. The source vector
of child frames is bookkeeping for those lifetimes; native component storage
continues to use `0086A4D0` and the actual +28h header. Invocation destruction
does not replay cleanup or release resources. A failed frame and all borrowed
domains must survive their child provider obligations; failed VFS frames
currently require process lifetime.

The context borrows the actual cache singleton publications, the acquisition
context and the live CRT conversion mode. The application supplies the same
string, VFS, parser and resource lifetime domains to acquisition and teardown.
No host string/vector substitutes for game storage, no generic acquisition
callback, and no fallback loader is introduced.

## Exception evidence

Handler `00C96160` selects FH3 info `00DC8134`, magic19930522, maxState6,
unwind map `00DC8158`. Six actions at `00C96130..00C9615F` implement:

| State | Next | Action | Cleanup |
|---:|---:|---|---|
| 0 | -1 | C96130 | Particle Lua object |
| 1 | 0 | C96138 | UnderWater Lua object |
| 2 | 0 | C96140 | Iterator key |
| 3 | 2 | C96148 | Iterator value |
| 4 | 3 | C96150 | Table temporary name |
| 5 | 0 | C96158 | Scalar temporary name |

Normal cleanup disarms each state before calling its destructor. A cleanup
exception during active C++ unwinding terminates rather than replacing the
original exception. Lua longjmp, native hardware faults and original FH3 frame
identity are separate unvalidated boundaries.

## Validation

The report records complete PE/live matching bytes for the body, all actions,
handler, FH3 info and unwind map, plus saved old Ghidra documentation. All32
body call rows pass the repository's live checker. Independent source review
against the full assembly and six-state FH3 map found no mismatch.

The temporary probe compares the complete copied original body with the source
using actual Lua5.1.1 objects and concrete string-pool, cache-acquisition,
resource and vector providers. Its three cases cover scalar false UnderWater,
a three-value table with numeric UnderWater, and an empty table. Each starts
with a prior resource reference. It compares the pointer-normalized complete
34h component image, appended resource names, +70 bytes and reference counts,
checks growth and balanced Lua tracking, and drains the genuine component,
cache and pool lifetimes. The source's one-shot replay rejection is also checked.
Only the caller's original machine-code body executes; call operands are rebound
to the proved source providers, and original FH3/hardware-SEH paths do not run.

The report records final MSVC Win32 `/W4 /WX /MD /fp:strict` build, three CTests,
and probe receipts. These are new C++ entry points; no original binary ABI,
exceptional-path differential, executable replacement or gameplay validation is
claimed.
