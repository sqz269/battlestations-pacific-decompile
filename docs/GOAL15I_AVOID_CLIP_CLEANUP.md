# Complete avoidance cleanup children

Addresses: `004167C0`, `00412EF0`, `00412DC0`.

| Native range, inclusive | Coverage | New raw Source entry | Original physical ABI |
|---|---|---|---|
| `004167C0..004167F7`,56B | complete | `clear_native_avoid_corner_records_004167c0` | ECX actual record header, no stack args, plain RET; ESI/EDI preserved |
| `00412EF0..00412EFD`,14B | complete | `free_native_current_pointer_cell_00412ef0` | ECX actual pointer-cell address, no stack args, plain RET |
| `00412DC0..00412DD9`,26B | complete | `free_and_clear_native_avoid_pointer_00412dc0` | ECX actual owner address, no stack args, plain RET; ESI preserved |

All public Source entries are naked Win32 one-argument fastcall; no Source EDX
placeholder is needed. They return void and make no semantic EAX/volatile-register
promise. Existing descriptive/library names `BSP_AvoidZone_ClearOwnedCornerRecords`,
`TMPL_FreePointer_00412ef0`, `TMPL_FreeAndClear_00412dc0`, comments and older records
are retained. The older `AvoidZoneAllocationAccess` projection in avoid_zone_owner
remains a separate, unchanged interface; no callback provider is added here.

## Whole memory/call schedules

Record clear consumes actual pointer-array+0 and count+4. The existing24h native
zone/24h corner-record schemas and the zone-producing callers reconcile this
prefix; no new class, virtual table or destructor identity is invented. Count is
treated as raw unsigned DWORD bits even though an older accessor view is signed.
Save ESI/EDI, capture header and initial count, zero index. TEST count/JBE skips
only zero. The exact three-byte padding LEA at004167CD (`8D 49 00`) is retained.

Each iteration reloads CURRENT pointer array from saved header+0, loads indexed
record and calls real free UNCONDITIONALLY, including a null slot. Immediately
after free, reload CURRENT count from header+4; ADD index,1; ADD ESP,4; unsigned
CMP/JB determines continuation. No array/count snapshot or negative-count guard
replaces these reads. On normal nonempty exit POP EDI precedes the count-zero
store; zero-entry uses its separate MOV count,EDI branch. Restore registers/RET.
Only count is written. Pointer-array allocation and its dangling entries, capacity,
layer, entity and bounds remain; no record-field inspection or virtual destruction.

Pointer-cell free captures `*actual_cell` once at invocation. If zero, return
without free or any store. Otherwise push that captured value, call real free,
then POP ECX from the actual pushed argument word and RET. The owner cell is
neither reread nor reset. POP is preserved, not replaced with ADD ESP,4 or a
restore of the original cell address. A later invocation consumes its then-current
cell value rather than an earlier cached cleanup owner.

Free-and-clear captures owner ECX in ESI and its pointer once. If nonnull, push
the pointer, call real free, ADD ESP,4, then zero `[captured_owner]`. It reuses
saved ESI, with no post-free owner/pointer reload. The final store overwrites
any intervening write to that pointer word. Null skips BOTH free and store.
Count/capacity/trailing words are untouched. No record walk, reset of other
fields, null repair, extra local unwind frame or rollback is introduced.

## Actual services and consumers

The three owned calls and the actual Native free thunk chain are:

```text
004167D6 CALL 00BF65AC
00412EF7 CALL 00BF6989
00412DCA CALL 00BF6989
00BF6989 JMP 00BF65AC
00BF65AC JMP 00BF9DC8
```

Both thunks and the returning CRT body were read. Null and normal release paths
return. All Source calls bind the established cdecl `singleton_lifetime_free`,
backed by genuine current `std::free` and paired with the accepted current malloc/
new-handler allocation domain. Native static heap/errno/SEH and current Source
CRT/frame identities remain distinct; neither thunk is a fake deleting callback.

Live refs and physical E8/E9/literal census agree on seven normal clear calls
and four cleanup tail transfers. All receiver/stack setups were read:

```text
0041A1E3 CALL 004167C0
0041A6F2 CALL 004167C0
0041AC37 CALL 004167C0
0041B473 CALL 004167C0
0041C49C CALL 004167C0
0041D17D CALL 004167C0
0041D2D7 CALL 004167C0
00C5E156 JMP 00412EF0
00C5E15E JMP 00412EF0
00C5E133 JMP 00412DC0
00C5E193 JMP 00412DC0
```

Clear receives an actual zone directly or the captured owned zone loaded from
its owner/group slot. The clip sites use EDI zone at normal rebuild and early
bounds rejection; the preceding PUSH EBP at0041A6EF is not a child argument.
These children consume no stack args. EF0's actions form actual pointer-cell
addresses with LEA ECX,[EBP-A4h] and LEA ECX,[EBP-7Ch]. DC0's actions load actual
saved receiver pointers from [EBP-10h] and [EBP-68h]. Full action bodies were
read. Parent EH maps/state transitions and clip arithmetic remain separately
owned work; these children do not transplant Native FH3/frame addresses into
current C++ unwinding. No Original class/virtual ABI installation is asserted.

## Verification and limits

Project/program checked: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
base00400000. Full56/14/26B bodies, live bounds, both thunk bodies, free body,
all actual call/cleanup receivers, existing raw schemas and concrete Source free
service were inspected. Strict MSVC Win32 `/MD /std:c++17 /EHsc /W4 /WX /permissive- /O2 /fp:strict /FAcs /c`
passes. No x87 or custom register-ABI reconstruction is required by these children.

`local/goal15i_avoid_clip_cleanup/compare_bytes.py [COFF-object] [live-directory] [PE-path]`
accepts the primary registered object. All96 owned COFF/live/original-disk bytes
match after ONLY three free CALL operands are masked. The exact cdecl free symbol,
REL32 kind, E8 opcode and separate original BF65AC/BF6989 targets are verified.
All84 fixed bytes match, including padding, every RET, register POP and zero store.

One ignored fixture at `local/goal15i_avoid_clip_cleanup/cleanup_probe.cpp` uses
genuine current allocation/free and existing actual record/zone storage. Count3
includes a null slot; two actual records are released, count zeroed, pointer array/
entries and all remaining metadata retained. A count0 repeat retains that array.
The actual pointer cell is changed from retained allocation A to fresh B before
EF0 invocation: B is released, A remains allocated and the cell is not reset.
The same case uses DC0 to release A, zero pointer+0 and preserve trailing guard
words+4/+8. Genuine `_heapwalk` observations check tracked blocks before/after.
No fake free provider or new test framework exists. Count/array reloads across
free are additionally established by the exact instruction proof; no dynamic
mutation-during-free or global no-leak claim is made.

Run `local/goal15i_avoid_clip_cleanup/run_probe.cmd [archive] [archive-only]`.
Default overlays only this module on the primary existing Release archive;
archive-only links its registered copy. Manifest is embedded. Fixture, call-report
and diff checks pass. Primary owns CMake, full registered build/archive rerun,
saved annotation/export and integration. Every reached raw/stack/control cell
needs valid backing and every freed value must match the current heap domain.
No Native runtime/SEH/hardware-fault differential or gameplay validation is claimed.
