# Input listener classification and device rebinding

Addresses: 00a91a50, 00a91e80

Packet `orch3_input_action_classifier`, 2026-09-10. Both functions were read from
the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, through
the project-verifying `bsp.py ghidra` commands. No Ghidra mutation was performed.
Names below are hypotheses, not recovered symbols. Implementation lives in
`include/bsp/input_action_classifier.hpp` and `src/input_action_classifier.cpp`.

## Classifier: 00a91a50

Native ABI: ECX points to the listener, then stack arguments are a float delta
and two dword argument slots whose low bytes hold previous/current down state.
Both exits use `RET 0Ch`; final instruction is 00a91bbb, length 3. Existing
`InputActionListener` is reused as explicit C++ state, without asserting native
layout or ABI compatibility. No native calls occur inside this function.

The image float defaults, read at 00e12f20..28, are respectively 0.25f
(`0000803e`), 0.5f (`0000003f`) and 0.1f (`cdcccc3d`), in little-endian byte order.
The current xref query finds only reads from this routine. They remain explicit
`InputActionTimingThresholds` inputs rather than invented mutable globals.

| Offset | Existing C++ member | Recovered behavior |
| --- | --- | --- |
| +08h | `pressed` | Quick rising edge, within 0.25 of last release; not every rising edge |
| +09h | `press_confirmed` | +08h AND NOT +0Ah |
| +0Ah | `press_aux` | On a rising edge, previous quick-press latch +11h |
| +0Bh | `fast_release` | Falling edge within 0.25 of last press |
| +0Ch | `release_confirmed` | +0Bh AND NOT +0Dh |
| +0Dh | `release_aux` | On a falling edge, previous quick-release latch +12h |
| +0Eh | `hold_fired` | Continuing down state whose repeat accumulator exceeds 0.5 |
| +0Fh | `held` | Current down state with held duration at least 0.25 |
| +10h | `hold_press` | First qualifying held frame while +13h is armed |
| +11h | `state_a` | Quick-press latch, set by +08h event |
| +12h | `state_b` | Quick-release latch, set by +0Bh event |
| +13h | `state_c` | Armed on rising edge; consumed by +10h, cleared while up |
| +14h | `since_press` | Historical member name: actually time since last **release** |
| +18h | `since_release` | Historical member name: actually time since last **press** |
| +1Ch | `held_time` | Accumulates while currently down, zeroed while up |
| +20h | `held_time_biased` | Down-time accumulator with one 0.1 subtraction per repeat |

Only +08h/+0Ah/+0Bh/+0Dh/+0Eh/+0Fh/+10h are cleared on entry. Both edge clocks
advance by the delta, including while up. If either updated clock is strictly
greater than 0.25, both +11h and +12h clear before the edge is handled. At exactly
0.25 the old latches remain, but a quick edge needs strictly less than 0.25.

While currently down, both held clocks advance. A rising edge arms +13h, copies
+11h into +0Ah, possibly sets the quick-press flag/latch, and zeroes +18h. A
continuing down frame instead tests +20h strictly greater than 0.5, sets +0Eh
and subtracts 0.1 once. A large delta does not catch up with multiple repeats.
The held test then runs on either branch: +1Ch >= 0.25 sets +0Fh, and an armed
+13h additionally sets +10h then clears +13h. Thus a large rising-edge delta
can fire `hold_press` immediately, but cannot fire the repeat flag that frame.

While up, +1Ch/+20h/+13h clear. A falling edge copies +12h into +0Dh, possibly
sets quick-release flag/latch using updated +18h < 0.25, then zeroes +14h. The
two final exclusion flags are derived after either branch.

### x87 evidence and corrections to earlier notes

The stores to stack scratch at 00a91a6e/00a91a84 and 00a91ac8/00a91ada force
each sum to binary32 before it is compared. The x87 stack does not retain an
unrounded accumulator across any threshold comparison here.

| Comparison | Instructions | Ordered rule |
| --- | --- | --- |
| Clear both latches | 00a91a97 `FCOMI` + `JA`; 00a91a9d `FCOMI` + `JBE` | either clock > threshold |
| Quick rising edge | 00a91afa `FCOMIP threshold,clock`; 00a91afe `JBE` skips | clock < threshold |
| Continuing repeat | 00a91b1e `FCOMI accumulator,delay`; 00a91b22 `JBE` skips | accumulator > delay |
| Held / armed hold | 00a91b3c `FCOMIP held,threshold`; 00a91b40 `JC` skips | held >= threshold |
| Quick falling edge | 00a91b7a `FCOMIP threshold,clock`; 00a91b7e `JBE` skips | clock < threshold |

Unordered comparisons set carry/zero and fail these event predicates. In the
initial OR, a NaN clock does not prevent the other finite clock clearing the
latches. Negative delta is used as supplied. The adapter does not clamp delta,
normalize infinities, reset poisoned clocks, or promise NaN payload equivalence.

`docs/GAME_INPUT_TICK.md` previously described +09h/+0Ch as ANDs with their
auxiliaries. The `JNZ` instructions at 00a91b97 and 00a91baa prove the auxiliaries
are negated. The old two clock names also have reversed semantic meanings.
The classifier is now reconstructed; its old host boundary is unnecessary.
The current lookup lists three callers (00a92a20, 00a92c40, 00a92d40), so the
earlier sole-caller statement is also stale. This packet only reconstructs the
classifier; the integrator owns updating the old comments and manager wiring.

## Rebinding: 00a91e80

Native ABI: ECX points to the 30h action record, no stack arguments, final
instruction 00a9204b is `RET`, length 1. Ghidra labels this `__fastcall`, but EDX
is overwritten from ECX at entry and is not a second input. The action binding
array uses pointer +10h and count +14h, with a 34h stride.

Each primary binding contains valid byte +00h, signed device class +04h,
unsigned device index +08h, cached device +0Ch, required modifier array at
+18h/count+1Ch and forbidden array at +24h/count+28h. Each modifier has 14h
stride, class +00h, unsigned index +04h and cached device +08h. Modifier input
code +0Ch and its final dword are untouched and omitted from this adapter.
The role labels are supported by 00a92370: its first array must pass each
device's virtual +20h predicate, its second must not return 1. Polling itself
remains external to this packet.

The device group expression is `backend + 6Ch + class*24h`, where backend is
the pointer at 00f8bbf4. Group+04h is vector begin, group+08h vector end; device
index must be unsigned-less-than `(end - begin) >> 2`, and a null begin yields
null. A valid index may also contain a null device. This vector grouping is
distinct from the fixed attachment slots modelled by `InputDeviceTable`.
The cached object type reuses existing `InputDevice`; no new device class or
device virtual calls are introduced.

Resolution order is observable:

1. Clear primary valid byte. Class -1 skips the binding, leaving its old cache.
2. Resolve and write primary cache. Null skips both modifier arrays.
3. Resolve/write required modifiers in order, stopping at the first null.
4. Only after all required entries resolve, do the same for forbidden entries.
5. Only after both arrays complete, set the primary valid byte to 1.

Empty modifier arrays succeed. Later unvisited cached pointers survive any
failure. The next primary binding is still attempted. No input values, listener
state, device state, or other binding fields are reset by this function.

Only the primary class -1 has a native sentinel check. Every visited class must
index a valid group; the native has no class bounds check. The C++ adapter keeps
this as a documented precondition and checks device index only. Valid, stable,
non-aliased vectors are also required. Three repeated bounds-check failure
calls to library address 00bf6713 (at 00a91eec, 00a91f59, 00a91fe9) cannot be
reached after the immediately preceding identical successful check under this
contract. The library diagnostic is not ported or replaced with a stub. Corrupt
vector state, concurrent mutation and native pointer arithmetic overflow are
outside the adapter's contract.

## Validation and remaining boundaries

Win32 MSVC Release build passed with existing `reconstructed_math` and
`native_math_differential` CTests (2/2). `verify-seeds` matched all eight existing
native seeds to the current disk image before the build.

One local differential experiment copied the 366 verified native classifier
bytes (SHA-256 `78e0df2eae9a7252372c28150caa1687b50c609e98b5af2627962b4d0ccc006f`)
into a separate Win32 process. All six absolute threshold operands were
relocated to explicit local inputs; branch bytes were unchanged. All 25,088
state comparisons passed: every previous/current pair, all eight latch
combinations, threshold equality/adjacent values, negative deltas, signed zeros,
infinities, NaN clocks/delta and NaN threshold inputs. All twelve flags matched;
the four float outputs matched bitwise except that NaN payloads were ignored.
Scratch probe and logs remain under `local/`; no tracked test was added.

The classifier is reconstructed, build-tested and locally native-differential
tested under the probe's masked floating-point environment. The rebind is
reconstructed, assembly-reviewed and build-tested, without a native differential
run. Neither is a binary replacement or game-validated. Signal/exception behavior
for signaling NaNs and non-default FPU modes is not established. Backend creation,
binding evaluation, modifier construction, input-settings linkage and live device
changes remain caller contracts or future packets. Both functions already exist
in Ghidra; there are no missing-function ranges or flow repairs to apply.
