# Raw emitter definition base lifetime

`native_particle_emitter_definition_lifetime.cpp` reconstructs AFA100 (378
bytes) and AFA350 (30 bytes) using the application's actual string cells and
F8D344 parameter pool. The existing host-binding definition interfaces and
runtime emitter-container interfaces remain available.

AFA100 stamps D5DBC4, then captures, disposes, returns and clears the seven
parameter slots at 24, 28, 2C, 20, 30, 34 and 38. The two retained-object rows
start at 3C and 54, with signed counts at 4C and 68. Each row is walked forward
using current counts after every outgoing terminal call. Row pointers and counts
remain stale. No null-member check, bounds check or count reset is introduced.

Zero-reference children with eleven confirmed numeric table profiles use their
actual BD30E0 terminal chain. The scalar call reloads the current table and uses
flags 1, without a second reference decrement:

| Table | Scalar target |
|---|---|
| D5DDC0 | B01130 particle type base |
| D5DB00 | AF8BB0 Object |
| D5DF30 / D5DCC0 | B05CE0 / B008C0 Axial |
| D5DFB0 / D5DCEC | B07C60 / B008E0 Floating |
| D5DFF4 / D5DD18 | B089C0 / B00900 Sprite |
| D5E048 | B0A820 Tracer |
| D5DC38 | AFACE0 Layer |
| D5DBC4 | AFA350 emitter definition base |

All these table pairs and the complete BD30E0 body match the installed image.
Other child profiles require actual callable Win32 vtables. Object model
terminals retain the documented canonical-renderer binding boundary; this change
does not introduce a substitute renderer owner or generic destructor callback.

## Unwind and original interface

Native AFA100 receives ECX=actual 80h owner and returns with RET. AFA350 receives
ECX=owner plus stack flags and returns the same owner in EAX with RET4; it frees
only after successful destruction when flags bit zero is set.

Handler CBAFF3 points to FuncInfo DF2EB8 and map DF2EA8. State 1 invokes
CBAFE8 -> 41DD20(owner+8), then state 0 invokes CBAFE0 -> BD30F0(owner).
The source guard is `noexcept`, so a second cleanup exception terminates. Normal
cleanup captures the name pointer before dropping to state 0, and drops to
state -1 before the base call. Name data/length fields remain stale.

## Validation and limits

The accompanying report retains full live/installed byte equality, prior
Ghidra documentation and reconstruction records, direct and indirect call sites,
the two-state map, all eleven table pairs and the focused probe receipt.
Independent assembly review found no discrepancy in parameter order, retained
row traversal, table reloads, or normal/unwind state changes.

The focused probe compares both complete original bodies against the new source
with genuine pool/base/CRT providers and the actual InterlockedDecrement import.
It covers seven ordered parameter returns, callback changes to current row
counts, nonterminal references, full 80h object images, scalar flags 2/3, and
unchanged fields. Source-only cases exercise a throwing retained terminal and
all eleven numeric-profile scalar chains. The recorded build and probe results
are in `reports/native_particle_emitter_base_lifetime_orch4.json`.

The new interfaces add an explicit context and are not binary replacements.
Native FH3/SEH, allocation failures, concurrent mutation and gameplay remain
unvalidated. The original-code probe executes normal paths; exception evidence
combines source execution with the original state map and listing.
