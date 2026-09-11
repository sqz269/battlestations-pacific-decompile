# Point-effect child update (AG)

`update_point_effect_children_00867790` implements the complete
`00867790..008679C0` body (561 bytes, RET8 at `008679BE`) over the existing
actual 114h point storage and canonical child references. Native ECX is the
point; stack arguments are float delta and a borrowed reference-node pointer.
There is no entry lock or reference acquisition. The caller must keep the
point, definition and captured storage alive across callbacks.

Evidence is from existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`reports/point_effect_children.json` records hashes, call sites, annotation
history and the exact validation scope. Descriptive names remain hypotheses.

## Primary slots and restart

At entry the function captures the point's primary backing +0C and count +10,
and its definition's component backing +08. The two captured spans advance
together. Replacing either header during a callback does not replace these
iteration spans or the saved primary end.

For each nonnull primary event, current type +18 values 1 and 4 suppress
virtual +28. Other types receive delta and the original reference pointer.
The event is then reloaded from its captured slot. An inactive event bypasses
completion; otherwise current virtual +08 determines whether it finishes.
The slot is reloaded again. If this current event is active, its current
virtual +30 is captured, active +0C is cleared, and the captured method runs.
Thus a cancellation callback sees the event inactive.

`0086783D` appends the **current source slot** to auxiliary +18 through
`00867320`. It then reloads the source, releases its captured reference, and
clears that captured slot after terminal reentry. Native code also performs
an unconditional null store. See `POINT_EFFECT_ARRAY_MUTATIONS.md` for append's
allocation, alias and retain order.

After processing the event, an empty captured primary slot may restart if
point option +08 is zero. The current component virtual +08 runs first. A
nonzero low byte allows creation; otherwise the reloaded component must have
gate +10 equal to zero and sample +14 equal to the point's current +88 word.
This restart path does not test admitted +1C. It reloads the current component
again before its current virtual +18 factory, which transfers an owned result.
Only the destination backing is reloaded after the factory: current point +0C
plus the original row offset. Assignment publishes/retains the new result,
releases the old destination, then drops the factory temporary. Null results
and identity assignments use the established canonical ownership helpers.

## Auxiliary pass and counter

The second pass starts from current auxiliary +18. Every entry calls current
virtual +28 and then the reloaded entry's current virtual +08; there are no
null, active, or type gates here. A completed entry is erased at `0086797C`
through `00867210`. The iterator stays in place to visit the swapped-in tail.
Otherwise it advances. Current backing/count are reloaded for the end test
after every iteration; a callback must preserve the captured iterator's
required storage lifetime.

Both virtual +28 sites load the original delta with x87 FLD and spill it with
FSTP to a float argument. The reconstruction preserves this per-call operation,
including masked NaN conversion, rather than using an early bitwise snapshot.
After both passes, `00867996..008679AA` uses SSE COMISS/JBE against +0 at
`00D7A218`. Ordered positive delta increments point +88 modulo 2^32.

This proves +88 is a counter, correcting the earlier `owner_88` pointer
interpretation. It is now `uint32_t sample_count_88`; the constructor still
writes exactly zero and the 114h layout is unchanged. Separate `00867D00`
sampling reads this word using a signed >1 comparison. It remains a distinct
update phase and is not implemented by this packet.

## Actual application dispatch

`GameplayPointRows` implements the new restart-field/current-virtual boundary.
The verified base component leaf `0086B7B0..0086B7B2` is XOR AL,AL; RET:
it reads no fields, returns false in AL and preserves upper EAX. Its table
reference is confirmed, including `00D0D5F4+08`; the missing Ghidra function
was defined from those three verified bytes. Unknown current predicates
require `GameplayPointRemainingComponents::restart_current`.

`GameplayPointChildEvents` routes the existing three rumble companions to
their actual +0C/+18 fields and `NativeGamepadForceEvents` methods. Domain
lookup is pure and creates no allocation, retain or second count. Current
table +28 must be `00872150` (the established RET8 no-op); completion +08
must be `00872180`; deactivation resolves +30=`00872160` before clearing the
actual active byte and invoking cancellation. Other event classes require the
supplied remaining runtime. Component tables and domain mappings are binding
preconditions, never an assumption that unknown functions return success.

## Exception and validation boundary

The native handler `00C94F18` selects FuncInfo `00DC6DA0` and map `00DC6D98`:
state 0 -> -1 via `00C94F10`, which cleans the pending factory-result slot at
EBP-10 through `006CF070`. State zero is armed after the factory returns and
cleared before normal temporary release. Factory/append/update exceptions
retain preceding mutations; the final counter increment is not reached.
Canonical assignment and terminal actions are nonthrowing, so the typed
implementation has no throwing operation while a returned temporary is held.
Original SEH dispatch or exceptions from native intrusive terminals are not
reproduced by this boundary.

The full application fixture constructs the point through `008689C0`, using
the actual definition cache, four named component owners, three native 20h
rumble event owners, node/string pools, and real insertion/terminal domains.
It exercises primary type gates, live handles, inactive transfer, auxiliary
retirement, restart sample/option gates, wrapping count, current-table rejection,
and throwing factory behavior. Normal teardown retires the actual events,
definition, components and node. Device output is a controlled fixture sink;
this is not physical output or gameplay validation. The existing complete
creation/lifetime regression and strict Win32 build also pass.

An independent probe executes the complete original 561-byte child body and
131/115-byte erase/append bodies, including RET immediates and explicit ESP
balance. Controlled row/event callbacks replace slots during update, completion
and cancellation, replace row/header storage during factories, and grow the
auxiliary extent. Native and C++ field/slot identities, actual atomic counts,
active bytes, counter and callback traces agree. It also compares callback
float bits and counter results for signed zero, negative, subnormal, infinity,
quiet/signaling NaN and wraparound cases. The original append branch has
sufficient capacity in this probe; array growth is covered by the separate
array fixture. Its terminal callbacks observe zero counts without freeing
physical events; the application fixture above supplies physical lifetime
coverage. Neither whole-114h preimage equality nor FPU status-register parity
is asserted here. Original EH dispatch remains untested.

Remaining mission-frame work includes complete `00867D00` transform/sample
advancement, `00866C60` job dispatch and `00867EE0` manager iteration/retirement.
The existing world-ocean sample-only helper spills timer+delta too early for
native x87 threshold behavior; it cannot substitute for complete `00867D00`.
A runnable gameplay-validated rebuild remains unproven.
