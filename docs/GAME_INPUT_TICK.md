# Per-frame input tick and the two timed action sets (packet `game_frame_input_tick`)

Addresses: 004bec00, 004b9f40, 004bf830, 004d11d0, 00a919f0, 00a91a50, 00a91e20, 00a922a0,
00a92370, 00a92aa0, 00a92c40, 00a926f0, 00a93da0, 004c43c0, 00874640

Packet owner `agent/game-input-tick`. Read-only Ghidra analysis plus a reconstruction in
`include/bsp/input_tick.hpp` and `src/input_tick.cpp`. Anchor is `BSP_Game_OnMove` (004e4a40),
whose phase list is in `docs/GAME_ON_MOVE_MAP.md`; this document covers its phase 3 (the input
poll at 004e4a63) and phase 15 (the two effect sets at 004e4e6c..004e4fe4).

## Headline correction: the two containers are red-black trees, not lists

`docs/GAME_ON_MOVE_MAP.md` describes `game+5B0h` and `game+5BCh` as `std::list` walks and marks
004d11d0 as a suspect STL length-error throw. Both readings are wrong, and the same evidence
settles them together.

004b9f40 and 004bf830 are `std::_Tree<...>::iterator::operator++`: descend to the minimum of the
right subtree, otherwise climb while the node is its parent's right child. They read `_Left` at
node+0h, `_Parent` at +4h, `_Right` at +8h and the `_Isnil` byte at +11h (004b9f40) or +25h
(004bf830). A list iterator has no `_Isnil` byte and no parent link.

004d11d0 is `std::_Tree<...>::erase(iterator)` for the +25h instantiation. Its three arguments are
the hidden return-iterator pointer plus one checked iterator passed by value as two dwords
(`_Mycont`, `_Ptr`), which is exactly the `push edi; push ebx; lea edx,[esp+38h]; push edx` shape at
004e4fd2. It calls 004bf830 to build the successor it returns, and its `invalid map/set<T> iterator`
immediate at 00ce44e0 is the MSVC debug message for a bad erase iterator, not a length error. The
existing `stl_throw_site` tag pointed at the debug-failure path rather than at the function.

004d41a0 confirms the container shape independently: it is the matching `erase(first, last)`
(`ret 14h` = return pointer plus two 8-byte iterators) and its whole-range fast path resets
`_Myhead->_Left`, `_Myhead->_Parent` and `_Myhead->_Right` to `_Myhead` and `_Mysize` to zero
(004d41f2..004d4204). That is `_Tree::clear`, and it fixes the container layout as
`{iterator list, _Myhead, _Mysize}` at +0h/+4h/+8h. Hence `game+5B0h`/`+5B4h`/`+5B8h` and
`game+5BCh`/`+5C0h`/`+5C4h`, and `begin()` is `_Myhead->_Left`, read at 004e4e72 and 004e4ef9.

| Container | Node size | Value | Meaning |
| --- | --- | --- | --- |
| `game+5B0h` | 14h (`_Color` +10h, `_Isnil` +11h) | one `int` at node+0Ch | action indices suppressed this frame |
| `game+5BCh` | 28h (`_Color` +24h, `_Isnil` +25h) | 18h bytes at node+0Ch | timed injected actions |

## The input singleton and its action records

004bec00 is the reviewed double-checked singleton getter; the constructor 00a93da0 writes vtable
00d5b630 at +0h, zeroes +4h..+18h, and stores 1 at +1Ch and +20h, so the object is 24h bytes.

The action table is `records = *(singleton+4h)` with `count = *(singleton+8h)`. Every walk computes
`end = records + count*30h` (00a92c7c, 00a922aa), and the two effect walks index it as
`records + id*30h` through `lea ecx,[id+id*2]; shl ecx,4` (004e4eb9, 004e4f8b). `+8h` is therefore a
count, not an end pointer. Record stride is 30h.

Recovered record fields:

| Offset | Type | Evidence | Meaning |
| --- | --- | --- | --- |
| +01h | byte | 00a92c88 | enable gate; a cleared record is skipped by the update |
| +10h/+14h | ptr/int | 00a9239b | binding array, stride 34h (input-settings layer, not this packet) |
| +1Ch | float | 00a92381 | previous-frame hold time |
| +20h | byte | 00a92384 | previous-frame down latch |
| +24h | float | 00a92387 | current-frame hold time |
| +28h | byte | 00a9238c | current-frame down latch |
| +2Ch | ptr | 00a92c95 | listener object, may be null |

The previous/current split is proved by the prologue of 00a92370 (00a9237a..00a9238c), which copies
+24h into +1Ch and +28h into +20h and then zeroes +24h and +28h before any binding is evaluated.
That makes the reviewed 004c43c0 test unambiguous: `+28h != 0 && +24h > 0 && (+20h == 0 || +1Ch <= 0)`
is "down now and not down last frame", a rising edge. All float comparisons in the packet go through
00d7a218, which holds eight zero bytes, so they are comparisons against 0.0f.

## 00a92c40, the per-frame update

`__thiscall(this = singleton, float seconds)`, `ret 4`. OnMove calls it at 004e4a72 with the raw
frame delta, immediately after 004bec00, as the first input work of the frame.

1. 00a92c57: `(*(00f8bbf4))->vtable[+4h](seconds)` with ECX = the backend object. The device backend
   sees the delta before any record is touched.
2. 00a92c5e..00a92c6d: read and clear the byte at backend+D4h. When it was set, 00a922a0 runs
   `00a91e80` over every record, re-resolving its binding array. This is the device-change rebind.
3. 00a92c88..00a92d00: for each record with +01h set, call 00a92370 (shift, then evaluate the
   bindings), then, only when +2Ch is non-null, call 00a91a50 with ECX = the listener and the three
   stack arguments `(seconds, previous_down, current_down)`. Both booleans use the same predicate,
   `latch != 0 && hold > 0.0f`, applied to +20h/+1Ch and +28h/+24h.
   The null test is obfuscated as `neg ecx; sbb ecx,ecx; test ecx,0F8BC00h` (00a92c9a), which is
   `+2Ch != 0`.
   Base and count are reloaded from the singleton on every iteration (00a92cef..00a92cfe), so a
   rebind that resizes the table mid-walk is observed.
4. 00a92d02: an optional plain function pointer at 00f8bbfc is called with no arguments.

## The listener object at record+2Ch

00a91e20 fixes its per-frame extent by clearing bytes +8h..+13h and floats +14h, +18h, +1Ch and +20h.
00a91a50 (`ret 0Ch`) is the classifier that fills it from `(delta, previous, current)`. Two of the
twelve flags are named by strings through 00a926f0: +0Bh is `fastRelease` (00d5b618/00d5b624, compared
with `__stricmp` 00bf7fbf) and +10h is `holdPress`. Two more are derived at the tail: +09h is
`+08h && +0Ah` (00a91b99) and +0Ch is `+0Bh && +0Dh` (00a91bac). The floats at +14h and +18h are
"time since the last press" and "time since the last release": each accumulates the delta and one is
zeroed on the branch that sets the other (00a91b09, 00a91b86). +1Ch and +20h accumulate held time,
and +20h has 00e12f28 subtracted from it when it crosses 00e12f24 (00a91b27), which is a repeat
interval. 00e12f20 is the threshold that gates the press, release and hold flags.

The classifier's exact branch structure is **analysed but not reconstructed**; it juggles three live
x87 values across the flag stores and the reconstruction leaves it to the host.

## The three injectors

All three are `__thiscall` on a record.

| Address | Shape | Effect |
| --- | --- | --- |
| 00a91e20 | `ret` (no stack args) | zero +1Ch/+20h/+24h/+28h; if +2Ch is non-null, clear the listener's twelve flags and four floats |
| 00a919f0 | `ret 4` (float) | +20h = +28h = 1 and +1Ch = +24h = amount |
| 00a92aa0 | `ret 8` (float, param) | +1Ch = 0, +20h = 0, +28h = 1, +24h = amount; then, when `*param != 0`, call 00a926f0 with ECX = +2Ch |

The asymmetry between the last two is the whole point of the timed set. 00a92aa0 leaves the previous
half clear, so 004c43c0 reports an edge on that frame; 00a919f0 sets both halves, so it does not.

00a926f0 takes the parameter block: if `param+4h` is a non-null C string equal to `fastRelease` under
`__stricmp`, it sets listener+0Bh and returns; otherwise it asks the block itself about `holdPress`
through the method at 00425850 and sets listener+10h on success.

## The timed-set entry, 18h bytes at node+0Ch

| Offset in value | Evidence | Meaning |
| --- | --- | --- |
| +00h (node+0Ch) | 004e4f66 | action index, scaled by 30h into the record table |
| +04h (node+10h) | 004e4f74 | hold time forced into the record |
| +08h (node+14h) | 004e4fbf | expiry threshold |
| +0Ch (node+18h) | 004e4f86 | parameter block, 8 bytes: object at +0h, name at +4h |
| +14h (node+20h) | 004e4f70, 004e4f9d | started latch |

## OnMove phase 15, 004e4e6c..004e4fe4

Guarded by `game+5D4h == 0Dh` (in-game) and `game+634h == 0`.

1. Walk `game+5B0h`. For each `int` value, fetch the singleton and call 00a91e20 on
   `records + id*30h`, then advance with 004b9f40. This suppresses those actions every frame the
   entry is present.
2. 004e4eda: if `game+5C4h` (the timed set's `_Mysize`) is non-zero, reset one more record. The
   native code adds a literal 30h to the record base (004e4eeb), so this is action index 1, a fixed
   index rather than a set member.
3. Walk `game+5BCh`. The iterator is advanced at 004e4f4b **before** the node is used, so the erase
   at 004e4fdf cannot invalidate it. For each node: if +20h is clear, call 00a92aa0 with
   `(node+10h, node+18h)` and latch +20h; otherwise call 00a919f0 with `node+10h`.
4. Erase test, 004e4fbf..004e4fcc: `fld [node+14h]; fld [00f876a4]; fcompi st(1); jb continue`. The
   node survives while the global is strictly below its threshold and is erased once the global
   reaches or passes it.

## Uncertainties

- **What 00f876a4 measures is not settled.** It has exactly one writer, 00874640 at 0087464c, which
  stores its float argument, mirrors it to 00f876a8 and derives `00f876b0 = (int)(argument / 0.05)`
  (the double at 00d7a270 is 0.05). All five reachable call sites that were read (004dfb70 at
  004e0194, 007713a0 at 007714e7, 00771fc0 at 00771fdc, 006b5960 at 006b5a85, and 004da780) pass
  0.0f. Other readers add it as a bias to per-object floats (008e9dbc, 008e9e3c, 008e9f1a). With only
  those writes the erase rule reduces to "erase when the threshold is not positive", which would make
  the set a one-shot queue. Either a writer exists that the xref index does not attribute to this
  address, or the threshold is not a clock. The reconstruction takes the global as an explicit
  argument rather than guessing.
- **Tree ordering is not recovered.** The comparator for either set was not read, so the visit order
  of `game+5B0h` and `game+5BCh` is unknown. The reconstruction preserves insertion order instead and
  says so at the type.
- **The producer of the timed set was not found.** No insertion site was identified, so the meaning
  of the parameter block's object at +0h (only tested for null here) and of the fixed guard index 1
  are open.
- **00a91a50's branch structure** is only partly recovered, as described above.
- **00425850** (the `holdPress` query on the parameter block) is outside this packet. The
  reconstruction decides it from the block's name string, which matches the observed shape but is not
  proved to be what 00425850 does.
- Records other than the fields above are unmodelled; +0h, +02h..+0Fh and +18h..+1Bh were not read.

## Reconstruction

`include/bsp/input_tick.hpp` and `src/input_tick.cpp`. `InputActionRecord`, `InputActionListener`,
`InputEffectParam` and `TimedInputEntry` carry the offsets above. `update_input_manager_00a92c40`
and `run_input_effect_lists_004e4e6c` reproduce the two walks over an injected `InputTickHost` with
one method per native call the packet cannot reconstruct (backend update, the dirty-bindings byte,
the rebind, the binding evaluation, the listener classifier, and the 00f8bbfc hook), in the style of
`bsp::run_application_frame`. `run_game_input_tick` sequences them the way OnMove does. The
reconstruction is not a binary-compatible replacement: it owns listeners by index and uses
`std::vector` where the native code uses two red-black trees.

One test case was added to `tests/math_tests.cpp`, covering the edge asymmetry between
00a92aa0 and 00a919f0.

## State reached

| Address | State |
| --- | --- |
| 004bec00 | analysed (already reviewed by `game_on_move_map`; re-read for the singleton layout) |
| 00a92c40 | reconstructed, build-tested |
| 00a92370 | prologue reconstructed and build-tested; the binding evaluation is analysed only |
| 00a91e20 | reconstructed, build-tested |
| 00a919f0 | reconstructed, build-tested |
| 00a92aa0 | reconstructed, build-tested |
| 00a926f0 | reconstructed, build-tested (the 00425850 branch is approximated) |
| 00a91a50 | analysed (flag map, accumulators and thresholds; classifier not reconstructed) |
| 00a922a0 | analysed (loop shape recovered; 00a91e80 not read) |
| 004b9f40 | analysed, identified as `_Tree::iterator::operator++`, N14 |
| 004bf830 | analysed, identified as `_Tree::iterator::operator++`, N28 |
| 004d11d0 | analysed, identified as `_Tree::erase(iterator)`, N28; previous tag superseded |
| 004c43c0 | analysed; the edge rule is reconstructed and build-tested here |
| 00874640 | analysed only, as the sole writer of 00f876a4 |

## What remains

- Find the insertion site for `game+5BCh` and the writer that makes 00f876a4 advance, which together
  decide whether the entries time out or are one-shot.
- Read the comparator for both sets and recover the visit order.
- Reconstruct 00a91a50 and name the three thresholds at 00e12f20, 00e12f24 and 00e12f28.
- Read 00a91e80 to describe the rebind that 00a922a0 drives, and connect the record's binding array
  at +10h/+14h to `docs/INPUT_SETTINGS_TREE_CLUSTER.md`.
- Correct `docs/GAME_ON_MOVE_MAP.md` phase 15 and its flow-break 4, which describe these containers
  as lists and 004d11d0 as a suspect throw site. That file is owned by another packet.

## Correction from docs/INPUT_ACTION_CLASSIFIER.md

The listener classifier at `00a91a50` is now reconstructed and called directly by
`update_input_manager_00a92c40`. The `InputTickHost::listener_classify` callback is
removed; the three timing globals are explicit `InputTickState::listener_thresholds`
with image defaults 0.25, 0.5 and 0.1 seconds. The native confirmation flags are
`+09 = +08 && !+0A` and `+0C = +0B && !+0D`, as the conditional jumps at
`00a91b94..00a91baa` prove. The earlier positive conjunctions above were incorrect.
See the classifier document for the timing branches and bounded rebind behavior.

`00425850` is a native-string comparison helper: ECX points to the eight-byte
length/data header, the C-string argument is on the stack, AL is the boolean
result, and it returns with `RET 4`. Its nonnull-data branch calls CRT `stricmp`;
with null data, the fixed nonempty `holdPress` argument cannot match. Thus the
modifier-name dispatch needs no unknown query behavior. `InputEffectParam::length`
replaces the incorrectly typed `object` pointer at +0, and the `00a92aa0` modifier
gate tests this length. Both comparisons use the host CRT instead of copied
ASCII-folding code; cross-CRT locale equivalence is not claimed.

The null-listener guard and projected record ownership remain intentional host
differences. This update does not establish the native ABI or game validation.
Combined checks are recorded in `reports/orch3_input_audio_session_integration.json`.

## Correction from docs/INPUT_BINDING_POLL.md (2026-09-10)

The frame update now calls the recovered full `00a92370` poll and `00a922a0`
rebind walk directly. `InputTickHost` exposes the backend device vectors and
inherits the three device-query slots plus CRT square-root boundary from
`InputBindingPollHost`; the former rebind and poll callbacks are removed.
Each enabled record shifts previous/current state exactly once, inside the full
poll, before listener classification. Disabled records are still rebound when
backend+D4 is dirty, but are not polled. Backend update and the optional final
hook retain their original order.

Record floats +1C/+24 are accumulated input values, not hold durations. The
legacy C++ member names remain for existing callers. Polling can produce
negative values; listener down predicates require a positive value and a true
latch. Binding ownership at +10/+14 is now represented by each record's vector.
Device groups and all referenced records/bindings must remain valid and stable
across device host calls; native pointer invalidation is not reproduced.

See `reports/input_binding_poll.json` for the 6000 native poll and 400 paired
helper comparisons. This integrates the recovered algorithm; concrete device
virtuals, original object layouts and gameplay input remain outside that proof.
