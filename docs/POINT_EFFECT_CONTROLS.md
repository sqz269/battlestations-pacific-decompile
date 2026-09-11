# Point-effect restart, stop, and consumed parent replacement

Addresses: `0042D9A0`, `00866F50`, `00867B10`.

`point_effect_controls.cpp` reconstructs three complete control functions over
the existing actual `114h` point storage, row/event companions, and singleton
lifetime domain. These are new typed Win32 interfaces, not native vtable
overlays. Names are descriptive hypotheses.

| Address | Proposed name | Complete bytes | Original ABI |
| --- | --- | --- | --- |
| `00866F50` | `BSP_PointEffect_RestartGatedRows` | `00866F50..0086703B`, 236 | ECX=point, no stack arguments, RET; no established return value |
| `00867B10` | `BSP_PointEffect_StopChildren` | `00867B10..00867BFE`, 239 | ECX=point, no stack arguments, RET; no established return value |
| `0042D9A0` | `BSP_PointEffect_ConsumeParentReference` | `0042D9A0..0042D9DF`, 64 | ECX=owned slot, stack=consumed reference, EAX=original slot address; both exits RET4 |

Live Ghidra bytes and the original executable agree across every listed byte,
including the three skipped alignment bytes inside restart and both parent
RET4 operands. SHA-256 values, call sites, and exception records are preserved
in [the report](../reports/point_effect_controls.json). The original executable
SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Indexed callers include `00867D00` for restart and parent replacement; stop has
66 callers. Caller relationships are static graph evidence.

## Restart

Restart captures the definition at point `+84`, clears bytes `+08` and `+0A`,
then captures that definition's row backing and count once. Every row advances
the output offset, including skipped rows. A nonzero current low byte `+10`
invokes the row's current virtual `+18`; admitted byte `+1C`, admission tests,
and existing event occupancy do not gate recreation.

After each factory returns one nullable owned reference, restart reloads the
current point `+0C` backing at the saved offset. Assignment captures the old
destination, publishes the result, retains the new reference, and releases the
captured old one. Identity skips assignment retain/release but still consumes
the returned temporary reference. A terminal callback can replace the slot;
restart does not overwrite that replacement afterward. Factory callbacks can
change future pointers within the captured definition span, while changes to
the definition header do not change its captured end. No manager locking,
array resize, auxiliary traversal, or sample/time reset occurs here.

The single native unwind state cleans the factory temporary through `006CF070`:
handler `00C94DE8`, FuncInfo `00DC6C44`, map `00DC6C3C`, state0 to -1 via
`00C94DE0` (`LEA ECX,[EBP-10]; JMP 006CF070`). Factory failure precedes this
state. Normal temporary release follows disarming it. Existing canonical
reference assignment and terminal operations are nonthrowing; no C++ rollback
is added to flags, earlier replacements, or terminal side effects.

## Stop

Stop calls the actual `00866440` singleton and captures its `+04` critical
section before changing flags. It enters the real OS section and increments
the actual recursion field at section `+18`. It tests the old byte `+08` and
clears `+0A`; an already stopped point only unlocks. Otherwise it sets `+08=1`
and captures the primary entry span once without changing count/capacity.

For each nonnull captured slot, a nonzero child active byte `+0C` causes current
virtual `+30` to be captured, the byte to be cleared, and that captured method
to run. The existing `deactivate_virtual_30` contract provides this ordering.
Stop then passes the address of this same source slot to complete `00867320`.
Cancellation may have replaced or cleared it; append reads that current slot,
including any subsequent reserve side effects. Stop reloads it again after
append, releases its current reference, and clears it after terminal reentry.
The final unconditional null store overwrites terminal callback replacement
without releasing that replacement, as the native body does. Initially null
slots are skipped. Header replacement does not redirect the captured span.

Exit decrements the captured section recursion field before OS leave. Native
EH only releases this captured lock: handler `00C94F58`, FuncInfo `00DC6E00`,
map `00DC6DF8`, state0 to -1 via `00C94F50`
(`LEA ECX,[EBP-14]; JMP 00411EE0`). Failure retains cleared active/point flags
and any completed append/release side effects. Singleton getter failure
precedes point flag changes. The source calls actual singleton and array
implementations, with required current child implementations supplied by the
same application domain; unsupported event families have no success fallback.

## Consumed parent replacement

`0042D9A0` captures the old slot. If nonnull, it decrements that exact parent's
actual `+04`, invokes its current terminal action at zero, and only afterward
writes null and then the consumed input. Initially null slots receive only the
input store. There is no incoming retain, same-pointer shortcut, registration,
or parent hierarchy operation. An old==incoming call still releases old and
publishes input. `PointEffectInstanceLinks::parent_reference` must map the
existing transform to the same actual owner and current terminal action.
There is no local native EH frame. Native terminal failure would prevent
publication; throwing virtual00 is outside the canonical nonthrowing C++
reference contract.

## Validation and limits

The focused ignored probe directly compiled the new source with the existing
Win32 core using `/W4 /WX /fp:strict /EHsc /O2` and `/MANIFEST:EMBED`. It compared
the complete original 236/239/64-byte control bodies, plus original 115-byte
append, against typed results and callback observations. It checked stack
balance, both parent RET4 paths, captured spans and current slots, low-byte
gating independent of admission, same-pointer/null factory results, terminal
overwrite, cancellation replacement/null source, inactive/already-stopped
paths, parent consumed identity/no-retain, and real Win32 lock release.
Representative C++ factory/cancellation exceptions preserved partial state.

This probe deliberately uses controlled ABI owners/callbacks with one actual
atomic count per owner, preallocated append capacity, and a published manager
whose native layout adapter points to the same real Win32 critical section.
It does not execute actual event/node physical destruction, native exception
dispatch, append growth, or a game frame. Physical application-chain validation
and the full CMake build belong to primary integration. No gameplay or drop-in
ABI compatibility claim follows from these source and fixture checks.

Ignored evidence: `local/extract_point_controls_ah.py`,
`local/point-controls-byte-evidence-ah.json`, `local/point-controls-extraction-ah.log`,
`local/point_controls_native_ah.cpp`, `local/run_point_controls_native_ah.cmd`,
and `local/point-controls-native-ah.log`.

## Primary integration validation

The combined strict Win32 build and both existing CTests passed. The actual manager fixture now invokes restart and released-parent stop through complete advancement, then child completion and owning retirement, with real event/node/definition/component/string destruction. See [LIVE_EFFECT_UPDATE.md](LIVE_EFFECT_UPDATE.md) and `local/live-effect-frame-probe-ah.log`. The earlier standalone-probe limits remain scoped to that probe; primary application-chain validation is complete for these cases.

Recovered signatures, names and evidence comments were saved in Ghidra, checked by readback with prior comments preserved, and re-exported. Native exception ABI, physical device output and gameplay remain unvalidated.
