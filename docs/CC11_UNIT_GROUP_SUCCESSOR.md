# Raw unit-group successor selection

`native_unit_group_successor_0070d8d0` reconstructs the complete
`0070D8D0..0070D97C` entry: **173 bytes, 62 instructions, no calls, no absolute
global references and no relocations**. The installed PE, fresh frozen PE,
live Ghidra bytes, compiled Source COFF, linked Source and both executed
bodies are byte-identical, SHA-256
`2ba0dbe85b2ea831d1a89b3336552f6c67d95416ddbe0d7ad25373795eb3bc2a`.
There are no listing gaps. Ghidra's floating argument/return inference is
incorrect: the native result is a pointer in EAX.

The raw entry receives the actual aligned 508h group in ECX and the excluded
entity pointer at `[entryESP+4]`, returning an actual entity/null with `RET4`.
The C++ fastcall declaration names an unused EDX argument to preserve that
register/stack arrangement. It supplies no provider or extra native input.
The naked Source retains every original instruction, including the seven-byte
alignment LEA and the native scratch ECX result from the final local-slot pop.

The complete contract is:

- A nonnull exclusion distinct from a nonnull `group+14` leader returns that
  leader immediately, before type/count/column access.
- Otherwise, type `group+4FC == 18h` returns null before count or `+500` access.
- On the scan path, the initial signed `+4F8` read precedes the unconditional
  `+500 = 0` store. A nonpositive initial count returns null after that store.
- Each record is 34h bytes, starting at group+18. Skip a null entity, a
  nonzero byte at entity+5D, or the excluded identity. Encountering the current
  leader stops the scan and saves that pointer.
- The first eligible candidate is accepted without a key load. Later
  candidates use record+20, with candidate FLD, best FLD, FCOMIP, FSTP, JBE
  exactly as at `0070D93D..0070D948`. Equality and unordered comparison retain
  the earlier candidate. The best key is loaded from its actual record again.
- After every ordinary index/record advance, `0070D954` reloads signed count
  from `+4F8`. This is not the captured-bound contract of callback `0070ECA0`.
- A selected earlier candidate overrides the saved leader-stop result. Its
  actual entity word is reloaded before return. Only group+500 can be written.

The existing semantic `GameUnitHost` helper is not used. It operates on
indexed host records and differs on type18 and leader-first behavior. This
packet changes neither that helper nor the group callback/detach bodies.

Actual caller witnesses establish the raw contract: `0070E4DD` passes the
departing entity with the group still in ECX; `0077BDC5` and `0077BDD7` use
the actual group loaded from unit+284 and push the current unit. Their byte
spans and live call-row checks are separate from this entry's zero-call body.
The retained name remains provisional until primary metadata integration.

One fresh connected fixture family constructs one actual raw group through
the existing complete Source `0070DAB0` using pinned CF4888 bits `4479C000`.
Seven guarded 648h unit-storage inputs carry actual pointer identities,
byte+5D and unit+284 fields. These are explicitly borrowed live raw inputs;
the fixture does not claim complete unit construction or group publication.
All Original/Source pairs reuse exactly the same group and unit addresses.
Only the possible +500 change is restored between the two invocations.

The family passed **261 checks, eight cases and sixteen exact pointer-result
comparisons**. Cases cover early follower return before type18, type18 null,
negative count with column reset, leader-first without a key read, holes/dead/
excluded members and first tie, earlier-best overriding leader stop, unordered
candidate followed by a denormal, and first-NaN retention with a signaling
candidate. These are one fixture, not eight independent native harnesses.
Every guarded 528h group span and all seven guarded 668h unit spans were
checked; no byte outside the expected +500 word changed. Both entries retain
their exact RET4 balance and native scratch ECX behavior.

FNSAVE/FRSTOR adapters preserve the caller's complete ambient state while
seeding two live x87 stack values and recording each entry's effects. Tested
control words are `037F`, `007F`, `027F` and `0A7F`. Both paths preserve CW,
TOP6, tag0FFF, both live values and seeded sticky/condition bits. All 80 saved
register bytes, status words, opcode and operand address compare exactly.
For comparison paths the last FP instruction is entry+75h in both code copies;
integer-only paths retain the seed instruction pointer and status. The
qNaN/denormal case produces `SW 7500 -> 7503`; the signaling-candidate case
preserves the existing precision flag and produces `7520 -> 7521`.
No Source comparison substitute, FP-control adjustment, callback service or
record copy participates in the entry.

Three translation units were freshly compiled from six frozen actual inputs:
the new entry, existing constructor and probe. The actual constructor COFF
in this build is 132 bytes / 39 instructions, distinct from older compilation
receipts. Its complete body and the two complete ABI/state adapters are
recorded. The entry's first trial object was already byte-identical; the one
full fixture execution passed without a repair or rerun.

The build used MSVC Win32 `/O2 /Gy /W4 /WX /fp:strict /EHsc /MD`, an embedded
I386 asInvoker manifest, a complete map and actual include/library output.
All 175 host includes, six searched toolchain libraries, compiler/linker and
environment-script hashes are pinned. No BSP support library, old probe
binary or old fixture is linked or executed. These are file/environment and
static import receipts, not a runtime DLL-load trace. Ten prior reports and
all **2,971** referenced artifacts remain unchanged. New evidence is inventoried
under `local/cc11_unit_group_successor_20261007_a` by the report.

The component is reconstructed, standalone Win32 build-tested and checked
against the complete Original entry. The worker performed no shared metadata,
CMake, Ghidra mutation or global build; primary integration owns those steps.
Exact raw-entry evidence does not establish native class replacement, complete
unit/group ownership, original CRT, faults, unmasked exception transport,
concurrent mutation, world integration or game parity. Callback `0070ECA0`
and detach `0070E4C0` still require their actual virtual+5C, wake-handoff and
speed-refresh dependencies. No type-service work is included here.
