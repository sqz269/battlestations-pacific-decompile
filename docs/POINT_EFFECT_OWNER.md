# Point consumer ownership

This packet reconstructs the complete orchestration of `0049C940` and
`008689C0` through explicitly required actual constructors. It also implements
the proven XYZ copy inside `0049C000`. It does **not** reconstruct that whole
random record constructor or the effect object constructor `008680B0`.
The new interfaces are in `point_effect_owner.hpp` and `point_effect_owner.cpp`.
They can be composed by the existing `UnitTimerHost` bindings; this packet does
not alter or supply those live bindings and does not close all of `00956600`.

## Evidence and original ABI

Live read batches used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by `python tools/bsp.py ghidra count`
before each batch (63100 functions). All three owners were exported through
`bsp.py ghidra export`; raw exports remain in shared ignored `exports/bsp/`.
Assembly resolves hidden arguments, x87 copies, global overlap, and EH state.
No Ghidra mutations or game execution occurred.

| Address | Native ABI and full extent | Reconstructed scope |
| --- | --- | --- |
| `0049C940` | ECX manager; stack point, secondary XYZ, signed descriptor; EAX owner/null; RET0C at `0049C9B6`, length3, inclusive end `0049C9B8`. 40 instructions/121 bytes. | Negative descriptor returns null without allocation. Otherwise allocate 30h; null allocation returns null. Reload actual manager+08 after allocation, call required constructor in raw storage, return that owner. Constructor throw frees raw storage. |
| `0049C000` | ECX 30h raw owner; stack base-owner, point, secondary XYZ, descriptor; EAX owner; RET10 at `0049C6D4`, length3, inclusive end `0049C6D6`. 512 listed instructions, 1751-byte envelope. | Only the six x87 position load/store operations at `0049C1DB..0049C1F3`. Record selection, vector checking, random construction, and base lifetime remain required. |
| `008689C0` | ECX fresh output-slot; EDX parent transform; five stack words: consumed template, point, transform byte, option byte, tail word. EAX output-slot; RET14 at `00868BE5`, length3, inclusive end `00868BE7`. 172 instructions/552 bytes. | Lock, original point stores, optional eligibility transform, real eligibility boundary, 114h allocation, required constructor, reference transfer, and cleanup order. |

`0049C000` has one listed gap, `0049C0BD..0049C0BF`. Live bytes `8D 49 00`
decode to an unreachable three-byte `LEA ECX,[ECX]` after the unconditional
jump at `0049C0BB`. It is padding, not an omitted callee or fabricated routine.
There are no missing starts among the three requested owners. The helper EH
handlers at `00C63303`, `00C6333B`, `00C95022`, and `00C9515C` lack separate
Ghidra function starts; bounded disk decoding and matching live bytes establish
their ten-byte MOV/JMP bodies. No functions were created.

## Original point and object ownership

In `0049C940`, `0049C985` reads manager+08 only after allocation. The required
`0049C000` constructor initializes its base via `00496920`, sets vtable CE6850,
zeros vector pointers +24/+28/+2C, and writes owner+04 = 2. This is not assumed
to be the effect owner's reference-count contract. Descriptor lookup
`004AED50`, record selection `004AB9A0`, vector resizing `00495E20`, random
service `00BD2F10`, conversion `00BF7420`, and returning invalid-parameter
handler `00BF6713` remain unreconstructed dependencies of this constructor.

Each 4Ch record receives captured XYZ at +08/+0C/+10. The source is read again
for each record after validation, so a constructor-wide early snapshot would
change observable order. The bounded copier takes existing selected record
storage and performs the six ordered FLD/FSTP operations. It preserves x87
conversion/trap behavior and source/destination overlap order. It does not
retain the timer's stack address or replace record allocation/selection.
Secondary XYZ contributes to a separate vector at record+14/+18/+1C; its
meaning is unresolved here. Descriptive announcement/record names are hypotheses.

`008689C0` first adopts its by-value template argument, then obtains actual
`00866440` manager+04. It captures that section, enters the OS critical section,
and increments that same section's +18 counter. The section projection reuses
`SystemSingletonCriticalSection`; it is **not** the singleton lifetime manager's
different +10 field. Leaving decrements +18 before the OS call, even if the
manager's section slot changed while inside. Both normal exits and EH confirm it.

The wrapper writes input X/Y/Z to `F87640/F87644/F87648`, the indices12/13/14
of the actual shared matrix `F87610`, even when the template is null. It never
substitutes a default matrix or copies the other thirteen elements. A nonzero
transform byte refreshes the actual parent with canonical `00B6DB70` if flag2
is clear, then uses canonical `004142E0` into disjoint stack scratch. Only the
eligibility point is transformed. The constructor still receives the actual
shared matrix address, which can reflect intervening/reentrant host writes.
Current `[E188A8]+19FC` is loaded after the optional parent transform.
The unit timer supplies all three trailing options as zero (`009568AA..AC`).

The required `0086A650` includes template-entry visibility writes, not just a
read-only predicate. Null template and false eligibility return a null output.
Accepted eligibility allocates114h; null allocation returns null. A nonnull
allocation adds one template reference and transfers it as `008680B0`'s
by-value argument. That constructor independently retains its member+84 and
consumes its argument on success and exception. On success it returns the
canonical companion for the actual constructed object with native +04 count1.
The wrapper clears/publishes the output, increments the new owner's count,
then releases its temporary while still locked. It finally unlocks and consumes
the incoming template reference. No old output word is released; storage is fresh.

`RenderCommandReference` is reused because assembly establishes an atomic
increment/decrement at actual+04 and virtual+0 only at zero, with ECX owner and
no destructor flags. Both `0041DE40` and `00440A30` confirm this exact primitive.
Production companions must borrow the actual count and terminal action, with
no second owned diagnostic count. Terminal callbacks and deallocation must be
nonthrowing in this C++ host domain. No arbitrary throwing terminal ABI is claimed.

## Exception evidence and remaining contracts

`0049C940` handler C6333B loads FuncInfo D8B488. Its state0 map at D8B480
dispatches C63330, which frees the raw allocation through `00BF65AC`.
Ghidra's saved C63330 body stops at its CALL; disk bytes additionally establish
POP ECX at C63339 and RET at C6333A. These are evidence, not new source routines.

`008689C0` handler C9515C loads FuncInfo DC6FDC and map DC7000. State3 frees
the raw allocation (C95149); state2 leaves the captured section (C95128 ->
411EE0); state1 releases the original by-value template (C95120 -> 41DE40).
State0 conditionally releases an already-published output (C95130). State4
names a temporary-release funclet but is not assigned in the listed body.
`008680B0`'s own FuncInfo DC6E94/map DC6EB8 has initial argument cleanup
C94FD0 -> 41DE40; its normal tail also releases that argument. Therefore a
constructor exception consumes the extra argument before caller raw-free,
then unlocks, then releases the original argument. The C++ wrapper preserves
that order. Throws before publication leave output untouched. With the declared
nonthrowing terminal domain, no remaining throwing operation follows output
publication, so the native conditional output unwind is unreachable in this API.

Real manager getter `00866440`/constructor `00866230`, current parent/reference
transforms, actual template owners, `0086A650`, the complete `008680B0` owner
and destruction/registration chain, `0049C000` with all named dependencies,
and allocation/deallocation remain required bindings. No success-returning
factory or substitute owner is provided. The wrapper owns the proven ordering;
the boundaries own the unresolved original behavior and must throw/reject an
unsupported binding rather than pretend to construct it.

## Verification

`./scripts/build.ps1` passed MSVC Win32 `/W4 /WX`, then both existing CTests
(`reconstructed_math`, `native_math_differential`) passed. The one ignored
`local/point_effect_owner_fixture.cpp` also passed with `/W4 /WX /fp:strict`
and `/link /MANIFEST:EMBED`. It exercises null/rejected/accepted paths,
getter/eligibility/allocation/constructor exceptions, exact trailing arguments,
original global XYZ versus transformed eligibility XYZ, borrowed atomic counts,
captured lock identity, extra argument consumption before raw-free, raw-free
before unlock, final original reference release after unlock, descriptor gate,
manager slot reload after allocation, and the ordered record copy.
This is a focused host-contract fixture, not a native differential effect test.
Logs and fixture hash are recorded in `reports/point_effect_owner.json`.

These are exported, bounded-reconstructed, build-tested, and fixture-tested
new C++ interfaces. They are not drop-in ECX/object ABI replacements and are
not game-validated. Original game installation and saved analysis were untouched.
