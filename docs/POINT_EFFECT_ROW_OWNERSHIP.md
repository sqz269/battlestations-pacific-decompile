# Point-effect row result ownership

`point_effect_row_ownership.cpp` implements complete typed bodies for `006FBEB0`
and `006CF070`, plus the bounded constructor stage `008682D5..0086838C`.
The helpers operate on the existing `RenderCommandReference` slots. They neither
construct a companion nor create, initialize, or copy a reference counter.
Callers bind each companion to the exact owner's actual `+04` atomic count and
its required nonthrowing current virtual `+00` terminal action.

## Target and ABI evidence

Read-only `bsp.py ghidra` queries verified project `bsp`, configured file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language,
and image base `00400000` before each batch. The project file exists. The saved
helper names were `FUN_006fbeb0` and `FUN_006cf070`, with no saved entry comments.
Names below are descriptive hypotheses. The integrator owns Ghidra annotation,
save, refreshed exports, and the `008680B0` ledger; this worker did not mutate
Ghidra or add another fragment record for that address.

| Body | Native ABI and boundary |
| --- | --- |
| `006FBEB0` | ECX destination slot; stack pointer to source slot; EAX same destination; `RET4` at `6FBEEE`, inclusive end `6FBEF0`; 29 instructions. |
| `006CF070` | ECX owned slot; no stack arguments; `RET` at `6CF098`; no established return-register result; 19 instructions. |
| Constructor stage | EBP actual instance, beginning at `8682D5`; ends immediately before manager insertion at `86838D`. This new free function does not preserve native internal registers. |

The containing constructor's corrected ABI is ECX actual `114h` storage, seven
DWORD stack slots (consumed template, parent, third word, matrix pointer,
transform byte, option byte, tail word), EAX same storage, `RET1C`. The old
exported comment's `RET14` is stale and contradicted by the assembly.

## Assignment and cleanup

`6FBEB0..B4` reads the incoming source slot before `6FBEBA` captures the old
destination. Equal pointer identities return without writing or touching either
count, including when the source and destination are the same slot. Otherwise
`6FBEC2` publishes incoming; `6FBECA` increments its actual `+04` when nonnull;
`6FBED8` decrements the captured old owner's count; zero invokes its current
virtual `+00` with ECX old owner at `6FBEE8`. It never clears or rewrites the
destination after that callback. Thus terminal reentry can replace the published
value and that replacement survives. The existing canonical assignment helper
already has exactly this ordering and is reused after the source-slot load.

`6CF074` captures its slot's old pointer. For nonnull old, `6CF07E` decrements
the actual count and zero invokes current virtual `+00` at `6CF08E`. Only after
return does `6CF090` clear the captured slot, overwriting a replacement made by
terminal reentry. The empty branch does not write the slot. No owner or companion
is accessed after terminal return; the original slot storage must remain valid.
Both APIs inherit the existing nonthrowing terminal contract, with sequentially
consistent atomic increments/decrements. They introduce no extra ownership.

## Constructor row stage

| Instructions | Operation and live-state rule |
| --- | --- |
| `8682D5..8682E2` | Project current instance `+84`, read its actual `+0C` count, call canonical `8672A0` on instance entries `+0C`. |
| `8682E7..8682F3` | Load current instance `+110`, refresh its canonical world only if valid bit2 is clear. |
| `8682F8..868309` | Load current `[E188A8]+19FC` after the preceding refresh; conditionally refresh its world. |
| `86830E..86832D` | If current option `+08` is nonzero, skip rows. Otherwise reload current `+84`; capture `+08` pointer before `+0C` count, and derive a fixed end with wrapping DWORD arithmetic. |
| `868330..868344` | Load current pointer from captured row slot; read its `+10` byte, then `+1C` only if needed; invoke its current virtual `+18(instance)` only when both are nonzero. |
| `868346..86835B` | Capture factory EAX result; reload current instance entries backing at `86834C`; assign at the original byte offset via `6FBEB0`. |
| `868360..86837D` | Release the original returned ESI reference after assignment; null is valid. |
| `86837F..86838C` | Advance source cursor and output byte offset by four; compare to the captured end; complete the bounded stage. |

`PointEffectRowRuntime` supplies only pure projections of the exact current
template, row and global reference fields, plus the required actual row factory.
Its template projection reuses `EffectAdmissionTemplateView` so admission and
construction can borrow the same actual pointer-slot storage. No row list is
copied and no row result is manufactured. The factory must map its exact nullable
native result to the stable borrowed-count companion, transferring the one
returned reference on success. Its implementation remains unrecovered here.

The loop captures source extent once, while later source slot values remain
live. Factory or terminal callbacks can change those later slots, template
pointer/count, output backing, or option. Changes to template extent and option
do not retroactively change the active loop. Output storage is captured after
each factory; after assignment begins, its captured destination must stay alive
through old-owner terminal reentry. The normal temporary release uses the
captured factory result, even if reentry replaced the assigned output value.

## Exceptions and boundaries

Factory invocation occurs while native unwind state remains5; state7 is armed
only after a successful result, immediately before assignment. Its funclet
`C9501A` points ECX at the saved temporary and jumps to `6CF070` at `C9501D`.
Normal code restores state5 before releasing captured ESI. Under the canonical
nonthrowing assignment/terminal contract no C++ exception can arise between
return of the factory and consumption of that temporary, so no extra destructor
or rollback is inserted. A factory exception transfers no result; earlier row
assignments and resized state remain for the constructor's actual outer unwind.
Arbitrary throwing native terminal vtables remain outside this typed contract.

MSVC Win32 is compile enforced. Unsigned cursor/offset arithmetic follows native
DWORD wrap. The existing array API's nonnegative valid-count/span requirements
apply; malformed headers or insufficient storage are not silently repaired.
These are typed companions, not executable native vtable pointers or binary
replacements. Manager insertion (`4D1100`/`867500`), template argument release,
member unwind, allocation and the complete constructor remain outside this
stage. Nothing changes the original game installation or enters the game.

## Validation

Direct MSVC Win32 `/std:c++17 /O2 /W4 /WX /fp:strict /MD /EHsc` compilation
passed. One ignored fixture, linked with `/MANIFEST:EMBED`, passed borrowed-count
ownership, captured extent with live later-row replacement, output backing reload
after the factory, retain-before-terminal ordering, nested assignment, original
temporary consumption, null results, skipped slots, identity assignment, and
clear-after-terminal reentry. It also checked canonical world refresh before
the option gate. It links the strict new source object, the existing entry-array
object, and the built core library. An initial fixture setup used null required
scene callbacks and failed during binding construction; the final fixture uses
the canonical scene type and attach functions and passed.

After current seed-byte verification, `scripts/build.ps1` passed both existing
CTests, `reconstructed_math` and `native_math_differential`. These do not exercise
the new bodies; the separate fixture does. Results and hashes are recorded in
`reports/point_effect_row_ownership.json`. The integrator owns CMake registration.
No permanent tests were added, no original installation changed, and no ABI or
gameplay validation was performed.
