# Canonical actual clock owner and vector lifetime, CC10

The actual 1Ch particle clock now has one canonical C++ storage type containing
its real 0Ch vector header at +8. The genuine 4DE4B0 allocation branch default
placement-constructs that owner once before the seven original field stores.
The sampler owner name aliases the same type and its adapter returns the genuine
getter pointer directly. It no longer relies on matching independent layouts.

The four production files exactly match the reviewed frozen lifetime design.
No caller is activated, ownership credit added, or raw consumer rewritten.
Current F8D420 remains a live void* publication cell. Existing nonnull publication
must already designate a live canonical owner; the hit path does not adopt,
construct or replace arbitrary native memory.

## Lifetime and untouched fields

NativeParticleClockStorage is standard layout, size1Ch and alignment4. Its DWORD
profiles remain at +0/+4, actual NativeResourceRecordVectorStorage at +8, DWORD
at +14 and float at +18. Assertions cover each owner/header offset, pointer and
float widths, and trivial default construction/destruction. The nested header
contains the actual record pointer/count/capacity, with no projected copy.

Singleton malloc returns fresh backing aligned for this type. The source uses
the returned pointer from `::new (allocation) NativeParticleClockStorage;`,
without parentheses, braces, aggregate assignment or scalar member initializers.
This starts the canonical owner and its actual nested header. It does not value
initialize their unspecified representations. No separate header placement,
launder, union, second count, atomic construction or companion is introduced.
The source then performs exactly seven volatile typed scalar stores, in order:

| Native address | Owner offset | Bits | New/old COFF offset |
| --- | --- | --- | --- |
| 4DE516 | +4 | CE7D08 | 99h |
| 4DE51D | +8 | 0 | A2h |
| 4DE520 | +C | 0 | A9h |
| 4DE523 | +10 | 0 | B0h |
| 4DE526 | +14 | 0 | B7h |
| 4DE529 | +0 | CE7D38 | BEh |
| 4DE52F | +4 | CE7D24 | C4h |

Win32's existing pointer representation lowers the typed nullptr assignment
at +8 to the same DWORD zero store. The compiler retains a register-only
MOV ECX,EAX at A0h between the first two stores; this also existed in the old
source body. There is no added owner/header access or constructor call.

The constructor neither reads nor writes +18. Its live float remains
indeterminate until the genuine B19A10 setter stores it. No preimage read,
sentinel, allocator wrapper, zero initialization or default time is credited.
Default construction and scalar initialization remain distinct under the
[C++17 initialization rules](https://timsong-cpp.github.io/cppwp/n4659/dcl.init#7).
The design's object/subobject lifetime reasoning and precise source boundary
are preserved in the earlier read-only archive.

The same live vector subobject is passed from B1B4D0 complete+4 to B1A4F0
cache+4/complete+8, then through B1A3C0 append and 4DA180 reserve. Record backing
is separately allocated, and existing providers placement-construct actual
2Ch records during append/reserve/resize. The header is not reconstructed when
its data pointer changes. 4DE290 cleanup obtains the same complete+8 header;
array free retains the native stale data/capacity. Flag1 complete-owner storage
release ends its trivial owner/header lifetimes. No postfree access is added.

## Strict build and complete emitted comparison

Implementation baseline is09edb1f1da09abfbbcc8562b7d0b3ad4eff8cb60 after a safe
fast-forward to the published main. The disjoint explicit-clock shutdown change
is included in that baseline; its source is unmodified by this packet. Before
the build,26 source/config inputs, exact Git tree,27 tool/SDK identities, old
getter/adapter objects and1,340 previous frozen artifacts were pinned. Original
4DE4B0[199] was reread using verified read-only Ghidra CLI and equals the frozen
exact-disk bytes. Ghidra project/program/language/base checks ran per query.

One Release Win32 build completed2026-09-26 09:12:00.182739 to09:12:24.041068
UTC, exit0. /MD, /EHsc, /O2, /W4, /WX and /fp:strict were verified in the actual
project. All three existing CTests passed. No new tracked test or runtime case
was added. All consumers were rebuilt for the canonical alias signatures.
Actual launch and both wait tool-return objects are preserved separately.

The complete getter274-byte body and sampler adapter127-byte body are identical
to their exact frozen old library-member bodies:401 bytes total, every byte
decoded, instruction lists equal. Getter relocations are unchanged. The
adapter's only relocation-name difference is its DIR32 EH-handler symbol's
return type, from NativeSamplerLoaderSingletonStorage to NativeParticleClockStorage.
Its ordinary function name changes accordingly; this is recorded explicitly.
No source symbol change is presented as new native ABI or native byte evidence.

Four exact CMake objects match members in the frozen new core library: getter,
adapter, default sampler entry and material descriptor consumer. The emitted
B1B4D0 receiver symbol and material caller's adapter REL32 relocation name the
canonical type. Complete old/new getter bytes preserve the captured guard,
volatile publication loads/stores, recheck, allocation, registration and return
sequence; no duplicate nested construction or extra +18 operation is emitted.
Evidence is complete COFF/archive-member comparison, with no new linked-body
or callback/runtime claim.

Two analysis failures are preserved. The first helper name inspect.py shadowed
Python's inspect module during Capstone import and failed before object reads;
its source/transcript are retained. After a filename correction, full401-byte
comparison passed but a handwritten predicate wrongly expected register-zero
stores and omitted MOV ECX,EAX. That failed helper and actual raw tool return
are retained. The predicate was corrected to the exact old/new instructions.
Production source, objects and library were never changed or rebuilt for either
inspection correction.

## Remaining boundaries

This establishes canonical source owner/header lifetimes. It does not authorize
existing incompatible raw C++ pointer lvalues. The unchanged resize, shutdown
and cache operation helpers still access the typed header data pointer through
volatile uint32_t*, at the exact sites recorded in the readiness analysis.
Their broader record/string/profile interfaces retain their prior assumptions.
Those raw consumers are not silently fixed or validated by a type alias or
passing compiler test. No fully ISO-safe nonempty composition is claimed.

Genuine positive child/rate production C30570 is still absent. Platform5030/SDK,
caller MSG preimage, source0/W, nonempty cache/sampler execution, full application
binding/startup/draw and original-game behavior remain separate. No synthetic
header, FPS/count/child, postmessage, fallback or default is supplied. Native
register/private-stack/FH3/SEH/hardware-fault equivalence remains unproven.

Report: reports/native_clock_owner_header_lifetime_cc10.json. Frozen evidence:
local/cc10_clock_owner_header_lifetime_evidence.zip. Exact manifest, compiled
schemas and external freeze receipt are under
local/cc10_clock_owner_header_lifetime_implementation. The earlier read-only
lifetime design and all prior source/build/runtime archives remain immutable.
