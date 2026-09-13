# Native wreck effect-handle release fragment

Addresses: `00824F39..00824FE4`, contained in `00824B60`.

This packet reconstructs **only the 172-byte fragment** that releases the
unit's three owning effect handles in session mode 4. It returns the required
continuation `00824FE5`; callers must still execute the leak/sink work and the
rest of the wreck handler. This is a new borrowed C++ API, not the complete
`00824B60` implementation, a native ABI replacement, or a gameplay binding.

| Range | Coverage |
| --- | --- |
| `00824F39..00824FE4` | complete normal fragment, required external call providers |
| `00824B60..00824F38` | not reconstructed here: effect generation, two grow/copy loops and intrusive-reference cleanup |
| `00824FE5..008252B0` | not reconstructed here: leak/sink physics, random anchors, registration, final virtual release and base notification |

The entire stored listing was read. Entry ESI is the same whole unit captured
from ECX at `824B7E`; no intervening instruction writes ESI. EBP's class capture,
EDI and the earlier x87 values are not consumed by this fragment. Incoming
paths skip the effect loop or complete it; the local FH3 state is -1, including
the write at `824F02`. The fragment has no x87 instruction or local cleanup.

The full 1,873-byte envelope was retained with PE/live parity, together with
the ten-byte handler, 36-byte FuncInfo, 24-byte unwind map and 77-byte cleanup
area. The previous pseudocode incorrectly treats `_free` as terminal at
`824CD6` and `824EB4`: each has a three-byte `ADD ESP,4` continuation at
`824CDB`/`824EB9` before pointer publication. Those bytes are not listed as
stored instructions. They remain part of the unreconstructed earlier range;
this packet neither removes their continuation nor ports a container body.

`NativeUnitWreckEffectReleaseView` borrows the actual `+9EC/+9F0/+9F4` pointer
cells of the same unit as its canonical observer alias. Initializer
`822C20` stores and retains these owners at `8246D0`, `823FBA`, and `82406F`.
The old `UnitInstanceState` bow/stern presence Booleans are not pointer owners
and are not cast or reused. Game and session mappings likewise lend the
actual game+38 and session+60 fields; they are not replacement objects.

The fragment captures `E188A8` once. It checks the captured game's session
pointer, reloads that same field, and compares the current session word with
4. It never retests the mode after effect callbacks. For each handle, in
9EC/9F0/9F4 order, it captures the current value and skips null. A nonnull value
is pushed before `4D1100`, remains pending on the native stack during that
no-argument getter, and becomes `8674C0`'s filter. Assignment then acts on the
**current** cell, so callbacks can change the owner being released or later
handles. A callback failure stops before subsequent operations.

| Sites | Callee and original contract |
| --- | --- |
| `824F70/824FA0/824FD0` | `4D1100`, no inputs, EAX actual manager, plain RET; reuse existing canonical singleton implementation and actual publication/domain |
| `824F77/824FA7/824FD7` | `8674C0`, ECX captured manager, stack captured pointer, RET4; captured +10/+14 span, pointer filter (null means all), call `8673B0` on each match |
| `824F80/824FB0/824FE0` | `484620`, ECX actual handle cell, stack literal zero, EAX cell, RET4; decrement current old owner+4, terminal virtual0 at zero, clear after callback, adopt replacement without retain |

Both unresolved providers are required and named by address. Their bodies
were read, including the null-filter domain and assignment callback order.
There is no default successful release, raw-clear production substitute,
private effect manager, newly ported library body or guessed virtual table.

The focused fixture runs the original 172 bytes and reconstructed source over
the same raw owner storage for eight deterministic seeds. It uses the actual
existing `4D1100` fast provider and explicit shared capture contracts for the
two unresolved callees. The capture callbacks replace current/later cells,
change manager publication and change mode after entry, exposing incorrect
capture/reload/clear ordering. These callbacks validate the fragment's call
sequence, not complete callee refcount behavior. One source-only throwing
provider checks that no later cell is touched; native FH3 transport is unproved.

All original branches to `824FE5` reach a fixture RET at that exact boundary.
Nine relative CALL operands and the game publication operand are relocated;
the original and relocated operands are retained. Inputs and output records,
full PE/live evidence, tools, objects, libraries, compiler commands, build and
existing CTest results remain in `local/wreck_proof`. No installed-game or
successful whole-handler execution claim follows from this fragment proof.
