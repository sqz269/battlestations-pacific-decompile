# CC12 observer endpoint cleanup ABI readiness

This read-only packet establishes the complete instruction contracts of
`007EE620..007EE665` (70 bytes, 24 instructions) and `00653390..006533E7`
(88 bytes, 24 instructions). Both match fresh live GPR bytes, the original
file-backed PE spans and Root Astra's approved complete-body capture. Baseline:
`e115a399d53b152e3bba3b4201b814a7bf57986b`.

The first body reloads a signed live count after every iteration and writes its
final byte after restoring ESI. The second captures the current endpoint,
optionally unregisters it with the current owner, then always invokes
callback-owner cleanup. Actual Source providers exist for their subordinate
operations, but require explicit pending-owner/access and observer-lifetime
domains. Their availability does not prove selected raw-task binding or Original
register ABI compatibility.

The [companion report](../reports/cc12_observer_endpoint_cleanup_ABI_readiness.json)
contains all 48 decoded instructions, exact memory widths, complete CFG/stack
accounting, three child boundaries, Source excerpts and dependency hashes.
Neither subordinate Native bodies nor handler/profile words, callers or neighbors
were opened. Source, CMake, ledgers and Ghidra were not changed; no Source,
Native reconstruction, Original ABI, startup or gameplay credit is added.

## Complete physical evidence

The existing project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` were verified before the live analysis batches.
Live and snapshot counts remain 64,729. Original image SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Owned body | Coverage and graph | Exact SHA-256 |
| --- | --- | --- |
| `007EE620..007EE665` | 70 bytes, 24 instructions, 8 blocks, 11 edges; `.text`, RVA/file offset `003EE620` | `982a339db60ea4a42c68d12ee902902bca4d6e8b355132ed8faef1e0f328dd4f` |
| `00653390..006533E7` | 88 bytes, 24 instructions, 3 blocks, 3 edges; `.text`, RVA/file offset `00253390` | `83a09c9e3d3c3f6006cf2d3aab805129a3f0d58cb21584f49c720c963dc737ea` |

All bytes are file-backed and decoded, with no padding or unclassified bytes in
the spans. Both bodies end in a plain RET. Ghidra's displayed `undefined(void)`
signatures are metadata and do not establish their input or return contracts.
Root Astra approved the entire assembly of both bodies before this audit.

## `007EE620`: live count and ordered predicates

Let S be entry ESP and R entry ECX. The body saves EBP at `S-4`, ESI at `S-8`,
captures R into EBP and clears ESI to zero. The initial `CMP [EBP+3CCh],ESI`
uses the current signed count. JLE skips the entire iteration path when that
count is zero or negative.

Only the positive-count path saves EDI at `S-0Ch` and computes
`EDI=R+3D0h`. This is an inline entry address; it does not load an array-base
pointer. Each iteration performs these operations:

| Site | Ordered effect |
| --- | --- |
| `007EE635` | Load current `DWORD[EDI]` into ECX, selecting pointer P. |
| `007EE637` | Compare current `BYTE[P+5Ch]` with zero. Nonzero skips the second predicate and call. |
| `007EE63D` | Only after a zero byte, compare current `DWORD[P+900h]` with one. Non-one skips the call. |
| `007EE646` | When both predicates pass, push full DWORD one. |
| `007EE648` | Call `00926D90` with captured ECX=P and that stack argument. |
| `007EE64D` | Increment current ESI by one. |
| `007EE650` | Increment current EDI by four. |
| `007EE653` | Compare ESI with a fresh `DWORD[EBP+3CCh]` load; JL repeats using signed comparison. |

The count and entry values are live reads. A child can change subsequent count,
slots or entity fields; the initial count cannot replace later loads. The
second predicate is conditional, and no null-pointer check, pointer reload
after argument PUSH or post-child predicate revalidation occurs. Argument and
CALL pushes can alias backing without changing the already captured ECX or
predicate flags.

Under preserved loop registers and normal returns, ESI starts at zero and the
signed guard prevents it from advancing past INT_MAX before exit. That large
machine limit is not evidence of usable storage, a fixed initial iteration
count or a practical runtime bound. Address increments remain machine-width
operations; no allocation bounds are checked.

The receiver's fixed selected accesses include count `3CCh..3CFh` and final
byte `3ECh`, giving fixed selected prefix extent `3EDh`. Indexed DWORD reads
start at `3D0h+4*i` and can require additional backing. Index seven's slot would
start at `3ECh` if a count permits reaching it; nearby flag placement does not
prove seven entries or authorize a clamp. Each selected entity reads byte 5Ch;
the zero-byte arm additionally reads `900h..903h`. Neither selected extent
proves a complete class or allocation size.

## `007EE620`: call, restores and final flags

Immediately before the argument PUSH, ESP is `S-0Ch`. The full DWORD argument
one lands at `S-10h`; CALL places its return address at `S-14h`. The child must
return normally with **four bytes of argument cleanup**, restoring ESP to
`S-0Ch`. This is a RET4-equivalent requirement, not a new audit of its return
instruction.

The child must preserve EBP=R, ESI=current index, EDI=current slot and EBX. EAX
and EDX have no owned writes anywhere in the 70-byte body. At child entry they
hold incoming or previous-child residuals. The last predicate was an equal
DWORD comparison with one, so arithmetic flags are CF=OF=AF=SF=0 and ZF=PF=1;
PUSH and CALL preserve them. Hidden inputs, lifetime, reentrancy and nonnormal
control are not excluded by this boundary.

On the iteration path, `007EE65B` pops current saved EDI from `S-0Ch`. Both
paths then pop current saved ESI at `007EE65C`, leaving ESP=`S-4`.
`007EE65D` writes **BYTE `[current EBP+3ECh]=1`**, the body's sole explicit
receiver-field store. EBP is restored from current `[S-4]` at `007EE664`, then
plain RET reads current `[S]` and leaves ESP=`S+4`.

The final byte write occurs after ESI/EDI restoration but before EBP and return
slot consumption. If it aliases already-popped save storage, it cannot change
those restored register values retroactively. It can change a not-yet-popped
EBP or return/control word. The destination remains R only under the stated
child preservation requirements.

There is no universal EAX result: no calls leave incoming EAX/EDX intact;
otherwise they retain the last child's residuals, even after later iterations
skip calls. With no iterations, ECX remains incoming R. Otherwise it is the
last loaded P if that iteration skips the child, or the last child's ECX
residual if that iteration calls it.

Final arithmetic flags come from a CMP, because all tail POP/MOV/RET operations
preserve them:

- No iterations: initial `CMP n,0`; CF=OF=AF=0, ZF=`n==0`, SF=bit31(n),
  PF=even parity of n's low byte. Signed n is nonpositive on this path.
- One or more iterations: final `CMP i,n` after the increments and fresh count
  load. For `r=(i-n) mod 2^32`, CF is unsigned `i<n`; OF is
  `((i^n)&(i^r)&80000000h)!=0`; AF is `((i^n^r)&10h)!=0`; ZF is `r==0`;
  SF is bit31(r); PF is even parity of r's low byte. Exit requires signed
  `i>=n`, equivalently SF=OF.

No local FS/EH frame is installed in these bytes. Child or memory faults and
nonlocal exits can bypass the final byte write. No noexcept, finally, rollback,
idempotence or reentry guarantee is established.

## `00653390`: endpoint capture and cleanup order

Let S be entry ESP and M entry ECX. The body builds the familiar Native frame:
state FFFFFFFF at `S-4`, handler immediate `00C7AF48` at `S-8`, captured prior
FS0 at `S-0Ch`, published `FS0=S-0Ch`, local-this at `S-10h`, and saved incoming
ESI at `S-14h`. It captures M into ESI and rewrites local-this from ESI. The
handler target and its unwind data remain unopened.

| Site | Ordered effect |
| --- | --- |
| `006533AD` | Store `DWORD[M+0]=00CF6494`, before both children and the endpoint load. |
| `006533B3` | Capture E=`current DWORD[ESI+14h]` into ECX. |
| `006533B6` | TEST captured ECX. |
| `006533B8` | Store full `DWORD[S-4]=0`; MOV preserves TEST flags into JZ. |
| `006533C2` | Nonzero arm copies current ESI to EDX. |
| `006533C4` | Call `006952A0` with ECX=captured E, EDX=current M, no pushed arguments. |
| `006533C9` | Both arms copy current ESI to ECX before the next state store. |
| `006533CB` | Store full `DWORD[S-4]=FFFFFFFF`. |
| `006533D3` | Always call `00695870` with captured ECX, no pushed arguments. |

The state-zero write can change aliased endpoint backing without changing
captured E or the TEST decision. There is no endpoint reload. At the optional
unregister call, EAX still holds the initial prior-FS capture; EDX has just been
assigned current ESI. Flags are TEST-E flags: CF=OF=0, AF undefined, ZF=0 and
SF/PF from E. At the always-called destructor, EAX/EDX/flags are unregister
residuals on the nonzero arm; the zero arm retains the initial EAX/incoming EDX
and TEST-zero flags across MOVs.

Both children enter at `ESP=S-18h` and must return to `S-14h` with **zero
argument cleanup**. They must preserve ESI=M and EBX/EDI/EBP. This body only
saves/restores ESI and makes no owned EBX/EDI/EBP writes. Required backing,
hidden inputs, aliases and nonnormal child behavior remain separate obligations.

`006533D8` captures the current saved-chain word at `S-0Ch` before `006533DC`
pops ESI from `S-14h`. It restores FS0 from captured ECX, adds 10h to ESP and
plain-returns with ESP=`S+4`. Every normal path calls `00695870`, so final EAX
and EDX are that child's residuals; ECX is the late chain capture. No guaranteed
returned M or semantic result is proved.

Final arithmetic flags come from `ADD ESP,10h`. For pre-ADD x and
`r=(x+10h) mod 2^32`: CF is `x>=FFFFFFF0h`, OF is
`7FFFFFF0h<=x<=7FFFFFFFh`, AF=0, ZF=`r==0`, SF=bit31(r), PF=even parity of
low8(r). Normally x=`S-10h` and r=S. RET preserves those flags.

The only explicit member store is M0..3; the only explicit member read is
M14h..17h, selected extent 18h. The member's byte 10h flag is not tested and its
endpoint word is not zeroed by this body. `00CF6494` is the last owned profile
store, before both children; no final profile, full class, allocation extent or
idempotence follows. The two state writes are full DWORD stores, unlike the
parent99 body's byte-only state update.

## Accepted task composition and current Source

The accepted parent136, member92 and cleanup99 reports are pinned separately
as prior contracts; their Native bodies were not reopened. Under the established
placement, M is Task+20h and M14h maps to Task34h. Member92's registration uses
ECX=captured endpoint and EDX=M. This cleanup's unregistration uses current
M14h and current M. Matching the pair requires preserved actual identities and
lifetimes through intervening work.

Parent99 calls `007EE620` on its late Task34h value, then `00653390` on current
Task+20h. For parent entry P, either owned child enters at `S=P-18h` and its
plain RET returns to `P-14h`, satisfying the parent's expected stack shape if
the subordinate RET4/RET0 requirements hold. The first child's receiver is the
endpoint value, not automatically the task. Neither child establishes final
Task0 identity: aliases and unopened subordinate effects remain possible.

At this baseline, an exact address search finds no Source implementation of
either owned body. The concrete Source
`native_pending_entity_kill_00926d90(owners, actual_entity, cause, access)` is
present. Its actual entity view borrows same-identity volatile field lvalues;
the access supplies the actual lock owner, current virtual providers and
allocation/count operations. It locks a captured section, checks/publishes
fields, performs direct child recursion and appends to the actual kill owner.
Initialized pending owners and access must outlive these operations. The
Native call supplies ECX and stack one; the extra C++ domains are not implicit.

An existing projectile consumer retains these owners/access and forwards to
the concrete provider. That is a different Source adoption. The search also
finds a `UnitDamageHost` projection, other host methods and a cause-only helper;
their names do not select a substitute or establish the selected launch route.

Actual `NativeObserverLifetime` methods implement unregister-pair and
callback-owner destruction with explicit retained manager, mutable lock/dispatch
cells and services. Unregister uses locking, pair lookup, dispatch invalidation,
reference decrement and conditional removal. Destroy writes its **Source**
profile `00CE3CD4`, performs nested locked count/detach work, and frees allocation
on normal or caught/rethrown paths. These are Source observations, not a new
Native child or EH audit.

The runtime/singleton does retain an observer lifetime, but binding the selected
actual M/E pair to that domain remains unproved. The generic owner prefix is
10h bytes; it does not contain the M14h endpoint or establish the whole 18h
member's lifetime. A Source composition of the now-established call order still
needs real member/endpoint storage and lifetime binding.

A later Root notice from its separate task-EH review identifies the parent99
state-one unwind target as `006569A0`, distinct from normal cleanup `00653390`.
This packet records that notice without independently opening the handler,
action, map or target bytes; no new Root report is pinned at this baseline.
The targets are not established as equivalent, and the Original FH3 interpreter
and full lifetime/unwind composition remain open.

Seven current excerpts preserve the selected production route: typed
`PlaneSquadronHostRecord` launch fields in the registry vector, updated by
construction/tick/termination. This does not create a raw task/member or bind
its registration, cleanup and endpoint lifetime. Existing Source registrations
are pinned, with no new build or link-selection audit.

Cleanup99's 17 input pins remain equal. Parent136/member92 inputs differ only
at `CMakeLists.txt`, retaining cleanup99's exact two accepted Source208 additions.
Both current Source primary receipts replay all 65 inputs; their prior build,
Core and map results remain historical Source evidence. The packet independently
validates all 158 bytes, 48 instructions, stack merges and Source excerpt hashes.
No subordinate Native proof, fresh build, tests or probes were added.
