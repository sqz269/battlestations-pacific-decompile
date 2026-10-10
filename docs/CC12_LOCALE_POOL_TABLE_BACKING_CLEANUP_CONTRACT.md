# CC12 locale pool table backing cleanup contract

`00435560` loads the current pointer at `[ECX]` once. If it is zero, the routine
returns immediately. Otherwise it passes that captured pointer to `00BF6989`,
then consumes the argument slot and returns when the free call returns normally.
It neither accesses count/capacity nor clears any member. In the accepted
constructor-action context, ECX is the address of the pool's table member at
`pool+28h`. This establishes the null-table/capacity-32 leaf path, while complete
owner construction remains **NO_GO**.

The machine record is
[the cleanup report](../reports/cc12_locale_pool_table_backing_cleanup_contract.json).
The accepted [action forwarding contract](CC12_LOCALE_UNWIND_PRIMARY.md),
[lifecycle](CC12_LOCALE_LIFECYCLE_PRIMARY.md), and
[profile](CC12_LOCALE_PROFILE_PRIMARY.md) retain their existing limits. The
[borrowed recycling helpers](CC12_LOCALE_HELPERS_PRIMARY.md) remain a separate
Source and artifact contract.

## Body gate and the returning-free gap

Complete live typed ownership reports 13 stored bytes in exactly two ranges:
`00435560..0043556B` and `0043556D`. Their 14-byte hull is below the authorized
128-byte limit. The worker extended its lease to `00435560..0043556D` before
reading assembly or bytes. No boundary was inferred from a neighboring function.

All 14 bytes match the retained original executable, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Independent x86 decoding gives exactly seven instructions:

```text
00435560  MOV  EAX,[ECX]
00435562  TEST EAX,EAX
00435564  JZ   0043556D
00435566  PUSH EAX
00435567  CALL 00BF6989
0043556C  POP  ECX
0043556D  RET
```

Ghidra's stored listing omits the one-byte `POP ECX` at `43556C`; live typed
metadata classifies it as undefined data with no containing function. The call
at `435567` retains `CALL_RETURN` / effective `CALL_TERMINATOR`, with no effective
fallthrough, although the target metadata says `no_return=false`. Original bytes
establish the in-hull continuation under normal return. The seven-instruction
normal-return graph has seven internal edges, no padding, and no out-of-hull
continuation. No listing, flow, body, prototype or no-return repair is made.

## Current pointer, fields and native free route

For a valid table-member receiver, the only object-memory access in this body is
the initial DWORD load. A zero pointer branches straight to RET: it makes no
free call and performs no count, capacity, table-entry, slab or payload access.
It does not inspect the receiver's following DWORDs, regardless of their values.

A nonzero pointer is pushed unchanged as the sole explicit argument to
`00BF6989`. There is no pointer reload after that call and no local object-memory
write on either path. This body therefore does not clear the pointer, zero the
count/capacity, iterate slots, or construct an idempotent reset. There is no local
EH frame, state store, catch, section operation or rollback. Effects inside the
unread free target are not folded into these local-access claims.

Live direct-target metadata identifies `00BF6989` as `_free`, a thunk whose
nonrecursive direct thunk target is `00BF65AC`, also named `_free`; both have
`no_return=false`. No thunk body was read. Separately retained lifecycle bytes
show the constructor's old-table release at `4362E4` and normal destructor's
table release at `435A80` both call exactly `BF6989`. The destructor's slab-free
call at `435A68` names `BF65AC`. These are native target-identity and routing
facts. They do not establish a replacement Source allocator, a new allocation
domain, the whole free implementation, or permission to substitute another heap.

The accepted constructor prefix zeroes table pointer at `+28h`, count at `+2Ch`
and capacity at `+30h`, arms state 2, and publishes capacity 32 before its 80h
allocation call at `4362B2`. If the pointer remains null on allocation failure,
and action `C5EEB3` is invoked with the required frame, it passes `pool+28h` to
this routine. The leaf then returns without reading or clearing count/capacity,
even with count 0 and capacity 32. Actual exception dispatch and failure
invocation remain unproved; this is a conditional leaf contract, not an executed
allocation-failure or unwind result.

## Register and return contract

The stored `undefined FUN_00435560(void)` prototype omits the incoming ECX
receiver. The routine takes no explicit caller stack argument and uses a plain
RET. On the null path, EAX is zero, ECX retains the receiver, and the final
defined arithmetic flags are from `TEST 0,0` (ZF/PF set, SF/CF/OF clear; AF is
undefined). No meaningful API return value is established for the whole routine.

The nonnull path pushes one argument. After a normally returning free with its
expected caller-cleaned stack convention, `POP ECX` restores the local stack
position and loads ECX from that argument slot. Under the ordinary intact-slot
contract this is the captured backing pointer, not the member receiver. EAX
and flags after the call are whatever the free target leaves; POP and RET do
not redefine them. The local instructions do not touch EBX, ESI, EDI or EBP;
whole-call preservation and exception behavior still depend on the unread target.
No original ABI-compatible binary replacement was built or executed.

The earlier action requires dispatcher `EBP=S=R+0Ch`, where S is constructor
entry ESP and R its FS registration. Actual setup of that frame remains unproved.
The effects of action targets `403970` and `402F70` also remain incomplete. This
packet does not expand those bodies, any free thunk, destructor body, EH
descriptor or handler window.

## Evidence and the next finite gate

Source capture HEAD is `039ac535e75ff259c92a55700fdfea423960db3e`. All 4,217
Source files are byte-equal to actual committed Git blobs. Retained commit/tree
objects prove both path membership and a closed census of all 4,210 tracked
`include/` and `src/` paths plus seven explicit prior Source inputs. Twelve named
dependency blobs and four tool/config blobs are retained. The only Source
changes from the earlier action capture are the later particle-record header
and implementation; neither epoch receives another epoch's artifact credit.

Six retained read-only BSP query outputs verify the existing BSP project and
program. Three typed captures are equal at modification 83, including complete
before/after records for entry, call, gap and RET. These captures were closed
before Root's separately scheduled later annotation writes. The installed
executable was not reread. Fresh bytes total 14; the prior 1,365 native bytes
remain separate, preserved epochs. No Source, CMake, ledger or GPR edit occurs.

The proof ZIP is
`local/cc12_locale_pool_table_backing_cleanup_contract/evidence.zip`: 47,353,512
bytes, 4,261 payload files / 77,463,945 uncompressed bytes, SHA-256
`3074a46ab7bc051dc84d98f6f1b7dfa4d8f2d12badc05c4831eb3e66eff14a82`.
Both it and an independent physical copy pass guarded, read-only, ZIP-only
replay. The replay verifies nested prior proofs, manifests, actual Git objects,
the closed Source paths, original PE slices, full hull/CFG, preserved gap/flow
metadata and prior same-target call bytes. External reads, directory access,
process/network calls and dynamic loading are denied after guard installation.

**GO:** accept this bounded native cleanup-leaf contract. **NO_GO:** complete
owning constructor, native unwind/ABI claim, canonical owner activation or game
composition. The next proposed finite packet is `402F70` section cleanup:
metadata/prototype/typed-flow first, then only a separately approved complete
body/hull and exact lease, with no OS-import body expansion. `403970` base unlink
and FH3 frame setup remain named separate dependencies. Canonical storage,
producer lifetime, payload-before-pool teardown and runtime ownership remain
unproved. No build, test, probe, runtime/game execution or OS change is added.
