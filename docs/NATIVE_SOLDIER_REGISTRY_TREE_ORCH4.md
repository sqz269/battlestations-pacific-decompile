# SoldierClass registry string-key tree

Addresses: 004AFE50, 004B0CB0, 004B0A70, 004B0660, 004B0880,
004AF530, 004AFB50, 004AFBE0, 004AFAA0, 004AFAF0, 004B02A0,
004B01E0 and 004B0090. Allocator catch support: 004B0312.

## Result and evidence boundary

Thirteen complete native bodies (2,289 original bytes) now have strict MSVC
Win32 source in `src/native_soldier_registry_tree.cpp`. They implement the raw
SoldierClass registry lower-bound, checked hint and unique insertion, complete
red-black link/repair, iterator equality/successor/predecessor, both rotations,
raw-pool node construction/allocation and the mapped-cell lookup used by the
factory. The detached 21-byte allocator catch at 004B0312 is represented by the
004B02A0 source catch; its Ghidra definition/annotation is owned and repaired by
the primary integrator.

The actual tree has head at +4 and unsigned count at +8. Its 1Ch nodes contain
left/parent/right at +0/+4/+8, a pooled string at +C/+10, the SoldierClass
pointer cell at +14, red/black byte at +18 and nil byte at +19. The source uses
the real `NativeStringRawPoolContext`, complete 0041DD40/0041DD20 providers,
the established 00443D00 case-insensitive comparison and the existing owning
native-layout length-error source transport. It does not introduce a host map,
private pool or fake unresolved callback.

| Native | Complete source behavior |
| --- | --- |
| 004AFE50 | Lower bound by signed, case-insensitive string ordering; empty/nonempty ordering matches 00443D00. |
| 004AF530 | Validate captured left owner, then compare current iterator nodes. |
| 004AFB50 / 004AFBE0 | Successor and predecessor over raw links; returning invalid calls resume at the original reload points. |
| 004AFAA0 / 004AFAF0 | Left and right rotations, including root/head publication. |
| 004B0090 | State0 cleanup of the current temporary string header; mapped word untouched. |
| 004B01E0 | Initialize links; copy key through the real raw pool with post-callback reloads; copy current mapped word; stamp color/nil. |
| 004B02A0 | Allocate 1Ch, invoke 004B01E0, and on constructor failure free the captured raw node then rethrow. |
| 004B0660 | Enforce unsigned limit 15555554h, allocate before current count/head reloads, link, rebalance, blacken root and publish node before owner. |
| 004B0880 | Unique insertion; output publication is node, inserted byte, owner. Duplicate input remains unretained. |
| 004B0A70 | Checked hint insertion with exact minimum/head capture order; fallback copies only owner/node from 004B0880. |
| 004B0CB0 | Find or insert a zero-mapped copied key, release its captured temporary block, validate the returned iterator and return node+14h. |

All names are descriptive hypotheses, not recovered symbols.

## Callback and reload schedule

The checked-hint body deliberately retains values across returning validation
callbacks. It captures minimum before the first 00BF6713 call, then loads the
hint node after that callback. Its second path captures head before the next
00BF6713 call. Predecessor and successor use mutable two-word iterator copies;
the end iterator captures the current head before successor advancement.

004B01E0 clears the destination header, reads the current source length for
0041DD40, then rereads source length after allocation. On a nonempty source it
loads destination length, source data and destination data in native order and
uses the established overlap-safe BF7680 behavior. It reads mapped+8 only after
the copy. 004B02A0 state1 unwind invokes the RET-only placement cleanup, while
the detached catch 004B0312 frees the captured raw node and rethrows; it does
not destroy a key or roll back tree storage.

On a 004B0CB0 miss, only temporary key length/data are zeroed before the copy;
mapped+8 is cleared afterward. The ordinary path captures the temporary data
pointer before hint insertion, captures returned owner and node before cleanup,
then releases that captured pointer with the current temporary length. If hint
insertion throws in state0, 004B0090 instead observes the current header. After
either hit or miss, a returning validation callback is followed by a fresh head
read before the end-iterator decision.

## Shared lifetime contract

`NativeKeyboardTreeIterator` remains the common two-word iterator. The sibling
lifetime packet consumes the exact 004AFB50 successor and 004AFAA0/004AFAF0
rotations from this header for erase repair; it owns its minimum/maximum,
erase/range/destruction bodies and their exception behavior. The primary packet
owns the SoldierClass factory, LoopLengths reader and class teardown. Those
operations are not claimed here.

## Validation

Every complete body span and direct call row is recorded in
`reports/native_soldier_registry_tree_orch4.json`; full spans match the installed
PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`python tools/verify_report_calls.py` passes with nonzero checked rows. The strict
`scripts/build.ps1` MSVC Win32 build and both existing CTests pass. Independent
lifetime-packet fixture work compiled this API and exercised the shared raw
rotations/successor in native/source erase, transplant, fixup, range and destroy
comparisons; that evidence belongs to the lifetime packet, not this report.

Source completeness does not supply the original calling ABI, FH3/SEH metadata,
hardware-fault behavior, concurrent mutation guarantees or a drop-in binary.
No reconstructed game startup or gameplay validation is claimed.
