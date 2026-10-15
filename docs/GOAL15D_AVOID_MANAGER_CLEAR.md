# Full avoidance manager clear

Owned address: 0041F600. The new actual-storage entry covers the entire manager
clear, including its cache-cleanup tail. Existing descriptive name
`BSP_AvoidZoneManager_ClearGroups` is retained. No recovered symbol is claimed.

| Routine | Native inclusive body | Bytes | Coverage | Native ABI |
|---|---|---|---|---|
| BSP_AvoidZoneManager_ClearGroups | 0041F600..0041F639 | 58 | complete | ECX manager, no stack inputs, tail JMP to 0041E960; EBX/ESI/EDI restored first |

`clear_native_avoid_manager_0041f600` is a naked Win32 source fastcall entry with
ECX the existing manager and EDX the borrowed address of the actual 00E188A8
publication cell. It saves EDX once before the native schedule, restores it with
POP after restoring the native nonvolatile registers, and tail-jumps into the
concrete `clear_native_avoid_cache_0041e960`. The added POP preserves flags.
No game pointer is cached and no global publication or manager is substituted.
The extra context argument and current source providers are explicit boundaries;
this is not an installed original-game ABI or gameplay binding.

## Complete slot and tail schedule

1. Zero manager `+58h` once at 0041F605. The group count is not the loop bound.
2. Walk exactly twenty DWORD slots beginning at manager `+08h`, ending at `+54h`.
   Each slot is loaded at its iteration (0041F614).
3. If the loaded pointer is nonnull, load that group's CURRENT vtable, then slot
   zero, push flags 1 and CALL EDX with ECX still the group (0041F61A..0041F620).
   This is actual virtual deleting dispatch, not an abstract success callback.
4. Only after a normal return, write zero to the SAME manager slot (0041F622),
   even if the provider republished it. Null slots skip the dispatch and store.
   Advance to the next slot; subsequent slots are reloaded and observe changes.
5. Restore registers and manager ECX, restore the extra source EDX context, then
   tail-compose full cache cleanup. The native tail is
   `0041F635 JMP 0041E960`; it is not the historical 0041F62F annotation.

Count is not reset again after providers run. No retry, exception rollback,
reentrant shape repair, added null check or ownership policy is introduced.
Required record/vtable/stack/control backing and provider lifetimes must remain
valid wherever the native schedule reaches them.

## Records, producers and concrete providers

The existing `Cc13AvoidZoneManagerNative` schema is reused: 78h bytes, twenty
`group_slots` at `+08h..+57h`, count `+58h`, cached body vector `+60h/+64h/+68h`,
and hull list sentinel/count `+70h/+74h`. Static assertions check the size and
the group slot/count offsets. No second manager class is declared.

The full constructor body at 00421500 was read: it initializes all twenty slots
and count to zero and constructs the cached vector/list fields. The group
producer at 00417CA0 was read: it allocates 14h-byte groups, installs native
vtable 00CE3858, clears zone pointer/count/capacity, sets the layer key, then
inserts the pointer into the manager's existing slot array. This agrees with
`Cc13AvoidZoneNativeGroup` in `cc13_avoid1.hpp`.

The native deleting entry 0041E940 was inspected in full assembly: it calls
0041DE70, tests flags bit 0, conditionally frees the receiver, returns its old
address in EAX, and uses RET4. The matched source group lifetime packet supplies
the genuine `kNativeAvoidGroupLifetimeSourceVtable`, whose slot zero is
`scalar_delete_native_avoid_group_0041e940(group,unused_edx,flags)`. EDX contains
the loaded dispatch target at the call; the source deleting entry ignores that
placeholder and consumes stacked flags with RET4. The manager does not hardcode
that address: every actual group vtable must select a compatible deleting entry.
The parallel group packet owns the complete clear/destructor/deleting provider
and its qualified source EH/CRT composition.

The full cache tail is already reconstructed against the actual publication
cell. It queues body DWORDs through the genuine dynamics queue, performs the
native checked-range continuation, then destroys actual hull handles and frees
nodes with the matched current heap pair. It retains vector capacity and the
list sentinel. Current CRT heap/handler/exception identities, source context
frames and virtual-provider backing remain qualified against the original game.
No fake allocator/free, synthetic deleting callback or shadow cache is supplied.

## Evidence and historical partial API

The configured bridge verifies project `bsp` at `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 image base 00400000 before live queries.
The full native manager assembly, all three native consumer argument setups,
the constructor, group producer and deleting entry were read. Live xrefs and a
fresh disk census agree on exactly three calls and no literal-address hits:

* `0041F9AD CALL 0041F600` in manager destruction.
* `00424D22 CALL 0041F600` in scene rebuild.
* `004DA808 CALL 0041F600` in session teardown.

Each supplies the manager in ECX and no stack arguments. The new source API
requires the explicit publication-cell address in addition.

The historical `avoid_zone_table_clear_0041f600(AvoidZoneTable&)` and its ledger
record remain a `partial_projection_build_tested` semantic API. They are not
relabelled as this complete raw-storage implementation or silently rebound.
Its old source comment names 0041F603 for the count reset and 0041F62F for an
unprojected/unread tail. The actual instructions are count store 0041F605 and
tail site 0041F635. This packet records that correction while preserving the
old source/evidence and adding a separate complete function record.

## Verification and integration

Strict MSVC x86 `/std:c++17 /EHsc /W4 /WX /permissive- /O2 /c /FAcs` compilation
passed. The source kernel has 60 bytes. A live-native/COFF comparison removes
only entry PUSH EDX and final POP EDX, masks the sole tail-JMP rel32 operand,
and verifies every other byte against all 58 native bytes. The indirect CALL
EDX, argument push, loops, stores and flags-producing instructions remain exact.

Ignored reproducer: `local/output/compare_avoid_manager_clear_bytes.py`, with
optional positional COFF-object and native-byte-listing paths. Defaults are
the worker object and `local/output/native_avoid_manager_clear_bytes.txt`.
Primary can pass its registered
`build/win32/bsp_core.dir/Release/native_avoid_manager_clear.obj` after build.

One ignored real-provider combined manager/group/cache fixture passed with the
new manager/group source objects over the primary's existing registered Release
archive. It uses the actual matched source vtable, a populated group with a real
zone/record array/record and null zone slot, and another group in manager slot 19.
It verifies slot/count cleanup, queued body words, destroyed hull cache, retained
vector capacity/sentinel/proxies, and flags-zero group retention with array cleanup.
The actual current `_heapwalk` confirms eleven tracked group/zone/record/hull
blocks are used before clear and no longer used afterward, while the body
storage, sentinel and preallocated queue remain used. Preallocating the queue
prevents reuse of the tracked addresses before that check. Flags-zero leaves
the group used and releases its array. This is bounded source-heap evidence,
not a global no-leak or original-runtime claim.
No synthetic deleting callback or heap mock is used. Its source/script are
`local/output/avoid_manager_clear_probe.cpp` and
`local/output/run_avoid_manager_clear_probe.cmd`; the executable embeds a manifest.
The script accepts an archive path and optional second argument `archive-only`
to reproduce against the final registered library without overlay objects.

That final registered-archive fixture and full build remain primary-owned;
the worker does not infer native execution from the source probe/byte comparison.
No new test target or framework is added. Primary registers `src/native_avoid_manager_clear.cpp`
in `bsp_core`, performs saved annotation/export refresh, and runs the full
required Win32 build/CTest. The worker changes only four owned outputs and this
address's appended name/new function ledger records. Current provider/global
binding, original SEH and game/runtime equivalence are not claimed.
