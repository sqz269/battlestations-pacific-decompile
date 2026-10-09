# 007F0F80 member construction and register-input registration boundary

Baseline: `ac9205e2864b6d8604955dcd16a7032b8cc6d313`.
Packet: `cc12_squadron_launch_task_member_constructor_ABI_readiness`.
Only `007F0F80..007F0FDB` is owned. No Source implementation, Native admission,
Original drop-in ABI, build, runtime or gameplay credit is added.

## Whole owned evidence

All **92 bytes / 28 instructions** match live Ghidra, the original PE and Root's
owned gate record:
`733be1e59ad190e42f0a18c88b426e879a2c3c13b562c12c59c449b0ad33cd88`.
There are three blocks, three internal block edges, one conditional branch,
one direct call and one RET 4. Every byte and instruction boundary is accounted
for; there are no skipped bytes, x87 or SIMD operations. The report contains
the whole instruction graph and both ordinary-return stack paths.

The body clears M+4/+8/+C **before** reading its argument. It captures that
current argument once, tests the captured value, writes state/profile/endpoint/
flag fields, then branches using the still-live CMP flags. On the nonzero arm,
the child receives **ECX=captured endpoint and EDX=actual member**, with no
pushed arguments. This register-input boundary is physical instruction evidence;
the child body remains unopened.

## Entry, frame and ordered accesses

Let S be entry ESP and M entry ECX, copied to ESI at 007F0F99. A is the current
word at S+4 read later at 007F0FA8. It is not an entry snapshot or an automatic
copy of the parent's earlier captured argument.

| Slot relative to S | Owned use |
|---|---|
| +4 | One input word, read after the three zero stores. |
| +0 | Current return-address backing. |
| -4 | Initially -1; overwritten with zero at 007F0FAE. |
| -8 | Numeric handler C8F398, wholly unopened. |
| -C | Prior FS:[0] capture; registration node begins here. |
| -10 | PUSH entry ECX; rewritten with captured M at 007F0F9B. |
| -14 | Saved incoming ESI. |
| -18 | Return address pushed only on the nonzero registration call. |

FS:[0] is published as S-C at 007F0F8E. XOR EAX,EAX sets zero before the
ESI save/capture. The saved-this local is rewritten, then the following accesses
occur with ESP=S-14h:

| Site | Access or decision |
|---|---|
| 007F0F9F | DWORD [M+4] = 0. |
| 007F0FA2 | DWORD [M+8] = 0. |
| 007F0FA5 | DWORD [M+C] = 0. |
| 007F0FA8 | ECX = current [S+4], captured A. |
| 007F0FAC | CMP captured A with EAX=0. |
| 007F0FAE | DWORD [S-4] = EAX=0. |
| 007F0FB2 | DWORD [M+0] = CF6494. |
| 007F0FB8 | DWORD [M+14] = captured A. |
| 007F0FBB | BYTE [M+10] = 1. |
| 007F0FBF | JZ 007F0FC8 using the earlier CMP result. |

All four writes between CMP and JZ are MOV instructions and preserve its
arithmetic flags. Changing [S+4], [M+14] or other backing during those stores
does not retest the new memory value. If an early zero store aliases [S+4],
however, it can change A before the capture. Moving the capture to entry,
placing the zero stores later, or reloading the argument at the branch/call
would change the owned behavior.

The body has six direct member writes and no explicit member-field reads.
Its direct extent is 18h bytes, ending at M+17h; M+11h..M+13h are not directly
initialized. There is no member-null, size, ownership, identity or endpoint
class check. Zero A skips registration only; the state/profile/field writes
still occur.

## Nonzero child contract

At 007F0FC1, EDX receives current ESI=M. CALL 00694A60 at 007F0FC3 pushes only
its return address. Immediately before that call:

- ECX is captured A, even if later field writes changed its source word.
- EDX and ESI are M; EAX is zero. EBX, EDI and EBP have no owned writes.
- Flags are from CMP A,0: CF=OF=AF=0, ZF=0 on this arm, SF is A's high bit,
  and PF is even parity of A's low byte.
- Caller ESP is S-14h; child entry ESP is S-18h. A normal child must perform
  plain-RET-equivalent cleanup, returning ESP to S-14h with no argument pop.
- ESI must remain M for the caller's return value. EBX/EDI/EBP also require
  normal nonvolatile preservation because this body does not save them.
  Child effects on memory, saved ESI/chain/state, reentry, faults or unwinding
  are not certified by that required contract.

The child has fresh metadata span 00694A60..00694AE6, 135 bytes,
43 instructions and eight blocks. Metadata names its lock-getter, pair lookup,
edge creation and critical-section imports; none of those Native bodies was
opened. Neither `undefined ...(void)` nor the descriptive registration name
establishes its complete ABI or effects. On the zero arm EDX has no owned
write; on the nonzero arm its final value depends on the child.

## Tail, flags, aliases and EH

Both arms merge at 007F0FC8 with ESP=S-14h under the required child return.
ECX captures current [ESP+8]=[S-C] **before** EAX receives current ESI and
before POP ESI. FS:[0] is then restored from that captured ECX. ADD ESP,10h
advances from S-10h to S; RET 4 reads current [S] and leaves ESP=S+8.

EAX is current ESI at 007F0FCC, normally M, before the incoming ESI is restored
from current [S-14h]. Child corruption or aliases of that saved word can change
the value returned to the parent's ESI, even when this body itself restores it
with POP. The registration call's EAX result is discarded. ECX is the late
saved-chain capture, not A. There is no explicit member write after that capture.

Final arithmetic flags on both arms come from ADD ESP,10h. For 32-bit x equal
to actual pre-ADD ESP and r=(x+10h) mod 2^32: CF iff x>=FFFFFFF0h;
OF iff 7FFFFFF0h<=x<=7FFFFFFFh; AF=0; ZF iff r=0; SF is r's high bit;
PF is even parity of its low byte. Under the ordinary stack contract r=S.
RET preserves these flags; neither branch outcome supplies final ZF.

Frame, PUSH and CALL writes can alias member backing; member writes can alias
the input word, frame, state, chain, saved registers, endpoint backing or
return address. Normal return requires valid selected memory and intact code/
control stack. The zero argument does not make a null member safe. No earlier
write is automatically rolled back on a later fault.

State -1 precedes the remaining frame setup and initialization; state zero is written after
the late load/CMP but before CF6494, endpoint and enabled-byte stores. There is
no later explicit reset. C8F398 handler/table/funclet bytes remain unopened.
No destructor, cleanup order, rollback, throw or recovery behavior is inferred
from those state writes or from the different parent/registration handlers.

## Composition with the accepted 136-byte parent

The pinned parent computes its member receiver from current task+20h and pushes
its earlier captured Q. This child then performs another late capture A after
its own three zero stores. A equals Q only when that argument/backing survives
both constructors' intervening accesses.

Under the parent's receiver-placement and required register/backing contracts,
this body writes Task+24/+28/+2C=0, Task+20=CF6494, Task+34=A and Task+30=1.
After it returns, the parent writes CF6514 through current EDI, normally Task+20.
The parent's EAX result ignores the member-return EAX. Parent task+34 now has
an explicit owned store schedule; calling its value a squadron object or a
particular endpoint class still requires other evidence.

Together the two bodies reach a **38h selected direct-write extent**, through
Task+37h. Their unique direct stores cover 51 byte offsets, leaving Task+12h,
+13h and +31h..+33h directly unwritten. This is not a 38h allocation proof,
full layout, complete initialization or sufficient backing for all child effects.
The member's unopened registration child can still affect memory beyond the
owned stores. The parent's pre-member D08AE4 root write therefore remains
qualified as a later/current profile identity.

## Actual Source provider and remaining integration gap

`register_observer_pair_00694a60` is a concrete existing Source function in
`src/observer_edges.cpp`. It takes two actual `NativeObserverOwnerStorage&`
references and an explicit `NativeObserverLifetime&`. It acquires a guard
around the lifetime's actual lock section, looks for the pair, increments an
existing count or creates an edge. The create path uses the shared allocator
and appends to both endpoint arrays; the guard and failure behavior are Source
semantics inspected here, not a fresh Native-child reconstruction.

`NativeObserverOwnerStorage` is a 10h prefix with numeric profile at +0 and
data/count/capacity at +4/+8/+C. These offsets correspond to this constructor's
initial stores. That correspondence does not create a Source object, certify
its lifetime, or bind CF6494/CF6514 as callable Source tables. The member's
flag at +10 and captured endpoint at +14 extend beyond that generic prefix.

`GameObserverRuntime` retains an application lifetime domain with the actual
manager access, E198E0 lock-publication cell, E198E4 dispatch cell and services,
and binds it through `GameSingletonHost`. Other existing Source functions pass
their borrowed lifetime parameter to the registration provider. Source registration in
`cmake/startup.cmake` is present for the provider, lifetime and runtime.
This packet does not inspect their current library bodies, link selection or
runtime execution, and the C++ three-reference interface is not the Native
ECX/EDX/plain-RET call contract.

The selected production launch-task route is still the typed squadron-record
projection pinned by the parent audit. It supplies neither this raw member
construction nor a demonstrated actual endpoint/lifetime binding. No new
Source implementation for 007F0F80 or these member-profile constants was found
by exact searches of `src`, `include` and `tests`. Existing Source registration
availability does not close that producer/owner/ABI/lifecycle gap.

The accepted parent report's 27 pins and latest Source build report's 61 pins
match this checkout. Prior compiled/Core/EH/build receipts remain accepted
Source snapshots; there is no new build, library audit or Native EH admission.
Only the two audit outputs changed. No new Native child/profile/table/handler/
caller/neighbor read, Source/CMake/ledger/GPR mutation, probe or test was made.
