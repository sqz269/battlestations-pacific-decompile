# CC12 pending-registry tick subnode unlink caller ABI readiness

This read-only audit owns `00875960..008759A4` (69 bytes). The function captures
an incoming ECX receiver, obtains the current registry section, optionally
enters it, and calls `00874E60` with receiver+1C and a node read late from the
first stack argument. It then clears current EDI+4 and optionally leaves the
captured section. This establishes a concrete Original caller of the admitted
unlink routine; it does not establish a node producer or virtual-slot binding.

The full instruction/CFG accounting and current evidence pins are in
[`cc12_pending_registry_tick_subnode_unlink_caller_ABI_readiness.json`](../reports/cc12_pending_registry_tick_subnode_unlink_caller_ABI_readiness.json).
The baseline is `0bd8828b70d1c543e9e934778699e90aaf23193e`, containing the
published Source admission at `9f055bd7b42ef14366fa32b49805c21d41ff1adc`.

## Boundary and resolved receiver gate

Live project-aware queries used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit, image base `00400000`.
Metadata reports 69 bytes, 25 instructions, five blocks, six edges, complexity
3, zero recorded parameters, and two named calls. All 69 live bytes equal the
configured Original PE's file-backed `.text` at RVA/file offset `00475960`.
The body SHA-256 is
`c0ecd0fa7e59f3a5e2077de82b52378ca3a06fc5565f4847176556edba5e2323`;
the image SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The decompiler exposes only a stack parameter and omits the receiver used by
`MOV EBX,ECX` and `LEA ECX,[EBX+1C]`. The worker stopped at that omission;
the primary Astra reviewer independently checked the live target and whole
listing and cleared the gate before physical verification continued. No
prototype, listing, annotation, or other Ghidra state was changed. There is
no x87 instruction or unaccounted body byte.

Independent decoding accounts for all 25 reachable instructions, five blocks,
six edges, two conditional branches, no tail transfer, and one `RET 4`.
The physical body has four calls: direct `00875280` and `00874E60`, plus
indirect calls through `00CE2218` and `00CE2210`. Decompiler import metadata
identifies the latter as EnterCriticalSection and LeaveCriticalSection;
their slot contents and implementations were not read. Metadata's two-call
count therefore describes only the named direct edges here.

Current index metadata lists one caller, `00876020`; its body was not read.
The index retains older FUN labels for the two children despite their current
saved names and Source admissions. Addresses and current primary receipts
govern this audit. No exact-address Source implementation of `00875960` exists.

## Exact receiver, node, and lock flow

Let S be entry ESP, R the incoming ECX receiver, and K the captured section.
The prologue saves EBX/ESI/EDI at S-4/S-8/S-12 and captures R in EBX before
calling the getter. Getter/import normal return and nonvolatile-register
contracts are required for the following ordinary paths.

| Address | Operation and timing |
| --- | --- |
| `875960..63` | Save three registers, then EBX=entry ECX=R. |
| `875965` | Call `00875280`; this call site pushes no arguments. |
| `87596A..6F` | ESI=K=`DWORD[getter EAX+4]`; test K and skip Enter/depth when zero. |
| `875971..78` | Push K, call `[00CE2218]`, then actual `ADD DWORD[K+18],1`. |
| `87597C` | Only now load EDI=N from `[ESP+10]`, ordinarily the original `[S+4]`. |
| `875980..84` | Push N; LEA ECX=R+1C; call `00874E60` with this list/node pair. |
| `875989..92` | TEST ESI; then write zero to `[current EDI+4]`; MOV preserves TEST flags for JZ. |
| `875994..99` | K nonzero: actual `ADD DWORD[K+18],-1`, push K, call `[00CE2210]`. |
| `87599F..A2` | Restore EDI/ESI/EBX from current save slots; common `RET 4`. |

The node argument is not captured before the getter or optional Enter/depth
update. Those operations can affect its later stack-word read under aliasing.
R remains a captured address; LEA adds 1C modulo 2^32 and does not dereference
a receiver field. Through the leaf, the embedded list touches receiver+1C,
+20, and +24; minimum raw receiver extent is 0x28. The leaf requires node
extent 0x10 and selected writable neighbors; this caller additionally writes
node+4. There is no explicit receiver profile or node+0 access in this body,
nor a read of node+4 to obtain a parent. Calling +4 an owner/back-reference is
a hypothesis, not a recovered type or proven producer relationship.

K is captured once from the getter result+4 and reused for Enter, both depth
updates, and Leave. The raw depth DWORD is at K+18, after the Win32 section
storage; the accessed backing extends through 0x1C. The increment occurs
after Enter returns, and the decrement after the post-leaf +4 clear. Both
are actual ADD read/modify/write operations with raw 32-bit wrap and no guard.
No section reload, balanced-release exception handler, null/membership check,
allocation, free, virtual dispatch, or loop is present in the owned caller.

## Leaf delivery, return, and alias limits

At the direct leaf call, ECX=R+1C and the first leaf stack word is captured N.
EDX is an unspecified incoming volatile residual; the admitted leaf overwrites
it before data use. Its current Source API has an explicit unused EDX argument
solely to produce that register/stack placement. The getter's current Source
API instead requires two borrowed publication-cell references. Neither Source
declaration is an automatic replacement for this Original call site's ABI.

Ordinarily the leaf preserves EBX/ESI/EDI, returns N in EAX, and pops its one
stack argument. Its known spill-alias limit matters here: if leaf field stores
overwrite its saved EDI, the caller's following store uses that changed EDI,
not EAX or a fresh copy of N. More general overlap can alter the caller's save
slots, argument word, or return address. All four syntactic control-flow paths
have balanced stack deltas under the no-argument getter, RET4 leaf, and one-word
stdcall import contracts. The normal return leaves ESP=S+8. This proves neither
intact stack contents nor successful return from arbitrary backing.

On the null-section path, EAX remains the leaf's captured N. The last flag
writer is TEST ESI with zero: CF=OF=SF=0, ZF=PF=1, AF undefined. The MOV, pops,
and RET preserve those flags; the leaf's earlier arithmetic flags are replaced.
On the nonnull-section path Leave may replace EAX/ECX/EDX and arithmetic flags;
neither a common semantic return value nor common residual flags are established.
Caller use of the residual return value remains unexamined.

Invalid getter results, section/node/neighbor backing, or stack memory may
fault at their corresponding access. A failure after Enter can leave the
section held or depth changed; no owned unwind cleanup undoes earlier writes.
Runtime import resolution, callee exception behavior, concurrency, and fault
equivalence remain outside this audit.

## Current Source evidence and remaining scope

All 53 latest unlink-primary input pins and its current doc pin match.
That primary receipt admits the unique Core member with an exact 83-byte body,
zero relocations, three passing existing checks, and absence from the final
application map. The getter's older 38-input receipt has 37 current matches;
only CMakeLists.txt differs. All 30 getter Source inputs and its doc pin match;
the latest unlink receipt's 48 Source inputs also match. Compiled receipts
remain their accepted build snapshots; this worker did not reread libraries
or execute code, and promotes none of this to Original runtime ABI proof.

This function directly composes receiver+1C with a supplied node. It does not
reveal who creates that node or supplies its profile. The `00874F00` cleanup's
ECX=node/stack flag1 virtual-call contract does not identify this receiver/node
wrapper as slot zero merely because both end in RET4. A concrete producer,
profile, and deletion target remain open. This packet changes only its document
and report, adds no Source/CMake/ledger/Ghidra mutation, build, probe, or test,
and grants no Source, Native, Original-ABI, integration, or game-validation credit.
