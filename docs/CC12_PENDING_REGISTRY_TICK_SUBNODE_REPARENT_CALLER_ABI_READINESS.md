# CC12 tick subnode reparent caller ABI readiness

This read-only audit owns `00876020..00876112` (243 bytes). It establishes a
concrete use of node+4 as the old receiver supplied to `00875960`, followed by
association with the incoming receiver and insertion through that receiver's
list at +1C. The node's +18 word orders the scan as signed. It does not identify
a node class, constructor, profile, producer, or virtual deletion target.

The complete bytes, 80 instructions, 23 blocks, 32 edges, and current input
hashes are in
[`cc12_pending_registry_tick_subnode_reparent_caller_ABI_readiness.json`](../reports/cc12_pending_registry_tick_subnode_reparent_caller_ABI_readiness.json).
The baseline is `fccaff11e849bd760a6384c25a3762345addfe41`.

## Boundary and physical coverage

Project-aware live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit, base `00400000`.
The primary Astra reviewer had independently checked the full 80-operation
listing and receiver/stack gate before assigning this packet. Worker metadata,
decompile, listing, and physical decoding reveal no additional hidden input,
x87 operation, missing return, or unaccounted byte. No Ghidra repair is made.

All 243 live bytes equal the configured Original PE's file-backed `.text` at
RVA/file offset `00476020`. The body SHA-256 is
`f66358eff6131f7f5dc84feccbed5c0361386fb02db26f74420a3bbb3d907b99`;
the image SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All 80 instructions are reachable. The six-byte `LEA EBX,[EBX]` at `87608A`
is an owned reachable no-op, not an omitted span. There are ten conditional
branches, four internal jumps, one scan backedge, four physical calls, and
one `RET 4`. Metadata's two named calls omit the two indirect section imports.

The direct targets are `00875280` and `00875960`; the indirect operands are
IAT slots `00CE2218` and `00CE2210`, named Enter/LeaveCriticalSection by live
decompiler metadata. Slot contents and target bodies were not read. Eight
current index callers are recorded as metadata only; their names do not prove
node production. The only exact-address Source search hit is an existing
comment in `game_hosts_units.cpp`, not an implementation of this function.

## Frame, captured inputs, and old-parent handoff

Let S be entry ESP, R incoming ECX, and T=S-20h the body ESP after setup.
The function pushes state `FFFFFFFF`, handler address `00C964E8`, and prior
FS:[0], then publishes the frame at S-0C to FS:[0]. It allocates eight local
bytes and saves EBX/ESI/EDI at S-18/S-1C/S-20. EDI captures R at `87603B`.
The getter returns the registry; EBX captures K from registry+4 once.

TEST K precedes two flag-preserving local stores: guard profile `00CE37FC`
at T+0C=S-14 and K at T+10=S-10. Optional Enter(K) and actual depth ADD +1
at K+18 follow. Only then does `876060` load the node N into ESI from T+24,
the current original first argument word S+4. At `876064` ECX captures
oldParent=`DWORD[N+4]`. TEST oldParent precedes the state-zero store at
T+1C=S-4, which preserves those flags for the next branch.

If captured oldParent is nonzero and equals R, execution skips all node/list
stores and count changes. It still performs the surrounding lock/frame work;
it does not reorder a same-parent node after a changed priority. If oldParent
is zero, assignment proceeds directly, even if R is also zero. Otherwise
`876077..78` pushes current ESI and calls `00875960` with that captured
oldParent still in ECX. This is the concrete parent-to-unlink boundary.

After the child returns normally, `87607D` writes current EDI to current
ESI+4. Ordinary callee-save behavior makes these R and N; raw child spill
aliases can change those registers and are not hidden by a copied object.
The child can perform its own getter/section lookup while this caller's
section is held. No equality of those two section addresses is assumed.

## Exact ordered insertion

Receiver fields are head +1C, tail +20, count +24 (minimum extent 0x28).
Node fields used here are parent +4, previous +8, next +C, and ordering word
+18 (minimum extent 0x1C). These are descriptive raw-field labels. No node+0
profile is accessed, and no concrete type or producer is recovered.

After writing node+4, the code reads the receiver's current head. If nonzero,
it captures node+18 once in ECX at `876087`. The decompiler's apparent
per-iteration node-priority read is inaccurate: the loop retains this capture.
At `876090` it compares each current candidate's +18 as signed against that
capture. JG selects insertion when candidate priority is greater; otherwise
the current candidate+C is loaded and the scan repeats until null.

For an acyclic consistent list already sorted ascending, this inserts after
existing equal keys. There is no count-bound traversal, cycle detection,
membership validation, or sortedness check; malformed cycles may not terminate.

When the scan finds a greater candidate, insertion uses its current previous
link rather than retaining that candidate as a fixed next pointer:

1. At `8760AA`, load P=`DWORD[candidate+8]`; TEST P; store P into node+8.
   The store preserves TEST flags for the branch at `8760B2`.
2. If P is nonzero, freshly read P+C after the node+8 store, write that value
   to node+C, then write N to P+C. If P is zero, freshly read current head,
   write it to node+C, then set head=N.
3. At `8760C8`, reload current node+C after those writes. If nonzero, write N
   to that address+8; otherwise write tail=N.
4. At `8760EC`, ADD 1 to the then-current count after all link writes.

If the head is null or the scan reaches null, the append path checks current
count for zero, not signed positivity. Zero count writes head=N. Nonzero
count reads current tail and writes tail+C=N without a null-tail guard.
Both paths then reread current tail at `8760DC`, store it to node+8, set
tail=N, clear node+C, and ADD 1 to current count. Zero count does not force
node+8 to zero; a stale or aliased tail is still read. Both count and section
depth arithmetic wrap as raw DWORD ADDs without overflow/underflow guards.

Earlier node/neighbor/head writes may alias fields used by later reads.
Preserve the parent-first store, captured priority, node+8-before-next order,
fresh head/neighbor/next/tail reads, and count update after all link stores.

## Normal return, flags, and exception limits

Normal cleanup uses captured EBX=K, not a reload of guard+4. K nonzero causes
actual depth ADD -1 then Leave(K). There is no later explicit state store or
reset to FFFFFFFF. At `8760FF` ECX reloads the current saved prior-chain
word T+14=S-0C. Three pops restore current saved registers, FS:[0] receives
ECX, `ADD ESP,14h` removes the locals/frame, and `RET 4` leaves ESP=S+8.
Fixed-point stack analysis checks all 80 instructions and every merge, including
the zero-stack-delta scan cycle, under normal Original callee contracts.

Every normal return's final arithmetic flags come from `ADD ESP,14h` at
`87610D`, after any Leave call. Let k be ESP immediately before that ADD and
r=(k+14h) modulo 2^32. CF is k>`FFFFFFEB`; OF is k in
`7FFFFFEC..7FFFFFFF`; AF is `(k & F)>=C`; ZF is r=0; SF is r bit 31;
PF is even parity of r's low byte. RET preserves these flags. ECX is the
current saved prior-chain word, not the receiver or list address.

There is no common semantic EAX result. On the null-section path, same-parent
skip retains the getter result; zero-count append retains zero; nonzero-count
append retains its loaded tail; before-candidate insertion retains the fresh
node+C capture from `8760C8`. With a section, Leave may replace EAX/EDX.
These are architectural residuals, not an admitted return-value interface.

The handler immediate `00C964E8` remains unopened. State -1 persists through
getter, optional Enter/depth, node argument load, and oldParent read; state 0
is written afterward. The owned bytes establish that timing, not a particular
unwind action. The eight-byte guard layout matches the known Source
`00411EE0` contract, but this body has no direct guard-destroy call and no
handler-to-guard edge has been proved. No Native cleanup, recovery, rollback,
or balanced release is invented. Raw object aliases can also corrupt guard,
state, save slots, or chain words; valid selected backing and intact control
stack remain conditions for normal execution.

## Current Source context and remaining scope

All 57 inputs of the latest unlink-caller primary receipt match, including
52 Source files; its current document pin also matches. It records the
82-byte/29-operation Source adapter with four relocations, distinct RET0Ch,
unique Core membership and no final application-map selection. The unchanged
getter250 and raw unlink83 are current provider snapshots in that same review.

The older 53-input raw-unlink receipt now differs in CMakeLists.txt and
`native_legacy_exception_owner.cpp`; the older 38-input getter receipt differs
only in CMakeLists.txt. Those older snapshots are explicitly historical.
The current typed owner routes base construction through the admitted raw17
default constructor, and the latest receipt reviews its current Source EH
and terminate maps. That Source compiler/CRT context does not identify this
function's unopened Native handler or promote its qualified APIs to Original ABI.

The current getter needs two actual publication references. The admitted
Source unlink-caller additionally needs an actual node-input cell and uses
RET0Ch; it is not a direct replacement for this Original one-node/RET4 call.
The known Source guard destructor captures raw guard+4 before its profile
write and preserves +4, but no ownership or cleanup binding is inferred here.
No Source/CMake/ledger/Ghidra mutation, build, probe, test, Native child/handler
inspection, or additional credit is added. Concrete node production, profile,
slot-zero target, exception behavior, and game validation remain open.
