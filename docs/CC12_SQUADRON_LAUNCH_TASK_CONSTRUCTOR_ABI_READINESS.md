# 007F1DE0 raw task construction and reparent delivery

Baseline: `a1e8251119f4712e0fabd5b9d1669e1659c41dc1`.
Packet: `cc12_squadron_launch_task_constructor_ABI_readiness`.
Only `007F1DE0..007F1E67` is owned. The descriptive function/class name is
provisional. This is a readonly ABI/access audit, with no Source or Native
admission, Original drop-in compatibility, runtime or gameplay credit.

## Established boundary

The complete **136 bytes / 41 instructions** match live Ghidra and the original
PE, including Root Astra's prior whole-function gate:
`6c3b892b4f05e0e83c225b4db2383c610876cc60480f42d104b4215a3f8e7b5c`.
There is one straight-line block, two direct calls and one RET 8; no padding,
unclassified bytes, branches or x87 operations. All 41 instruction boundaries
and the ordinary-return stack progression are accounted for in the report.
The original PE identity is pinned separately; only owned bytes were decoded.

This body supplies a concrete raw-task producer boundary: it writes the task
header/link fields and priority, pushes the captured task address, and calls
00876020 with the captured parent in ECX. This is physical call-site evidence,
not merely a metadata edge. Normal delivery of that same task value requires
the outgoing argument/control-stack backing to survive intervening writes.

The last owned root-profile store is **D08AE4 before 007F0F80**. The unopened
child can affect memory, including through aliases, so this does not prove the
root profile after that call or at a later cleanup. The tail writes CF6514 to
the member addressed by current EDI. It does not rewrite the root profile.

## Entry, frame and child boundaries

Let S be entry ESP and T the task address captured from entry ECX into ESI.
P is the current word at S+4 captured into ECX at 007F1DFA, after the initial
frame writes but **before all explicit task writes**. Q is the current word at
S+8 read at 007F1E2B, **after the first child normally returns**. Q is not an
entry snapshot and may have changed through task/frame aliases or child effects.

| Slot relative to S | Owned use |
|---|---|
| +8 | Second input, read late into EAX. |
| +4 | Parent receiver, captured early into ECX. |
| +0 | Current return-address backing read by RET 8. |
| -4 | State: initially -1; current EBX stored at 007F1E38, normally zero. |
| -8 | Numeric handler address C8F3B8, unopened. |
| -C | Prior FS:[0] capture; registration node begins here. |
| -10 | Initial PUSH ECX; rewritten with captured T at 007F1E02. |
| -14 / -18 / -1C | Saved EBX / ESI / EDI words. |
| -20 | First outgoing task word, later second outgoing Q word. |
| -24 | CALL's return-address write at each call. |

FS:[0] is set to S-C at 007F1DEE. The early parent load occurs with ESP=S-18;
the task is pushed at 007F1E01, leaving ESP=S-20. Both callees need a normal
RET-4-equivalent cleanup, returning this caller to ESP=S-1C. The second child
has not been opened, so this is a required contract, not a recovered child ABI.

At 007F1E26 -> 00876020:

- ECX is captured P; ESI is T; EBX is zero; EDI is still incoming EDI.
- The word pushed at S-20 was T. Subsequent task stores or CALL's return write
  can alias backing, and the accepted reparent body reads its node argument
  only after getter/lock activity. Its observed node need not remain T without
  the corresponding backing/side-effect conditions.
- EAX still carries the earlier FS:[0] capture; EDX is not changed by owned
  instructions. Arithmetic flags are from XOR EBX,EBX: CF/OF/SF clear,
  ZF/PF set, AF undefined. No subsequent owned instruction before this call
  changes those flags.
- The accepted baseline 243-byte callee audit supplies its receiver/node,
  stack and alias-sensitive insertion contract. This worker does not open
  its separate C964E8 handler; see the later Root notice below.
  This constructor requires normal nonvolatile preservation and valid backing;
  callee save/restore labels alone do not defeat stack aliasing.

After that return, Q is captured, all 128 bits of XMM0 are zeroed with XORPS,
and EDI is computed as current ESI+20h. Q is pushed, ECX receives EDI, and only
then the state word is written from EBX. Under normal first-child preservation,
this writes state zero. The root-profile and timer stores follow.

At 007F1E47 -> 007F0F80:

- ECX and EDI are the computed member address M; ESI is the preserved T,
  EBX is zero, and EAX still holds captured Q under the preceding contract.
- XMM0 is zero. The outgoing stack word was Q; the intervening profile/timer
  stores can alter that word if task backing overlaps it.
- EDX and the arithmetic flags inherit the first child's return. MOV, LEA,
  PUSH, XORPS and MOVSS here do not replace arithmetic flags.
- Require normal RET-4-equivalent cleanup and preservation of ESI/EDI/EBX/EBP
  for the stated ordinary composition. Do not infer the child's full ABI,
  writes, registration, ownership, exceptions or hidden inputs from its name.

Fresh child **metadata only** reports 007F0F80..007F0FDB, 92 bytes,
28 instructions, three blocks, and a callee label
`BSP_Observer_RegisterPair @ 00694A60`. Neither function body was opened.

## Exact task-write schedule and aliases

Before the reparent call, owned writes occur in this order:

| Site | Access |
|---|---|
| 007F1E06 | DWORD [T+0] = CFD99C. |
| 007F1E0C / 0F / 12 | DWORD [T+4], [T+8], [T+C] = EBX = 0. |
| 007F1E15 / 18 | BYTE [T+10] = BL = 0; BYTE [T+11] = 1. |
| 007F1E1C / 1F | DWORD [T+14] = 0; DWORD [T+18] = 2. |

After the first child, state write and member-argument setup:

| Site | Access |
|---|---|
| 007F1E3C | DWORD [current ESI+0] = D08AE4. |
| 007F1E42 | MOVSS [current ESI+1C] = low DWORD of zeroed XMM0. |
| 007F1E47 | Call unopened member child with computed M and outgoing Q word. |
| 007F1E4C | ECX = current saved-chain word [S-C], **before the next write**. |
| 007F1E50 | DWORD [current EDI] = CF6514, normally [T+20]. |

There are no explicit owned task-field reads or null/extent/validity checks.
The direct task-write footprint ends at +23h; bytes +12h/+13h are not directly
initialized. A 38h allocation, task+34h contents and any extra member backing
are not proved by this body. The first child needs its selected parent/list
and neighbor backing; the unopened member child may require more. Thus 24h is
the owned direct footprint, not a sufficient size for the complete composition.

The initial zero parent/link words and priority 2 are concrete ordered writes.
They do not guarantee the values later observed by reparent after its getter,
lock and possible aliasing effects. The early captured parent address survives
later task writes in ECX, while memory at that receiver can still change.

Do not relocate either PUSH after initialization, snapshot Q at entry, move
the state store, replace the late FS-chain capture with a post-member reload,
or recompute EDI after the second child. These change behavior under aliases.
Task/member writes can affect outgoing words, saved registers, state, chain,
return addresses or receiver backing; CALL/PUSH writes can in turn affect task
fields. Normal return requires valid selected memory and intact control flow.
Earlier writes are not automatically rolled back when a later access faults.

## Normal exit, flags and EH limits

After the late chain capture and member write, the body pops current saved EDI,
moves current ESI into EAX, pops current saved ESI and EBX, then restores
FS:[0] from the **captured** ECX. A member-write alias of [S-C] therefore does
not change that already captured restoration value. Aliases of the register
spill words can change the values restored by POP.

ADD ESP,10h changes ESP from S-10h to S; RET 8 reads current [S] and advances
ESP to S+Ch. EAX equals T only under the required child-preservation conditions;
unconditionally at the owned MOV it is current ESI, before the caller's saved
ESI is restored. ECX is the late chain capture. EDX and XMM0 can retain unknown
second-child outputs; no final XMM0-zero guarantee is made. EBP is not directly
written by this body, but its preservation also depends on the children.

All final arithmetic flags come from ADD ESP,10h, not either child or XORPS.
With x the 32-bit pre-ADD ESP and r=(x+10h) mod 2^32:
CF iff x >= FFFFFFF0h; OF iff 7FFFFFF0h <= x <= 7FFFFFFFh; AF=0;
ZF iff r=0; SF is r's high bit; PF is even parity of r's low byte. RET preserves
these flags. Under the ordinary stack contract r=S.

State -1 is installed before the first call. State zero is written after the
late second-input capture and PUSH, before D08AE4/timer initialization and the
second call. There is no later explicit state reset. C8F3B8 handler/table bytes
are unopened: no cleanup target, unwind order, rollback, recovery, throw or
reentrant behavior is inferred. Current Source compiler EH is a separate domain.

## Source context and next bounded lead

The prior selection's actual Source route remains a typed
`PlaneSquadronHostRecord` with `launch_task_*` fields. Its producer and directly
scheduled task tick do not construct this raw object or invoke these native
profiles. Current Source hooks for unit tick-node construction/destruction are
still abstract in the checked roots. This audit does not manufacture that bridge.

The admitted Source69 adapter requires three actual reference arguments and
RET 0Ch; the Original 00875960 child uses a node value and RET 4. The Source
getter additionally requires both actual publication-cell references. The
accepted readonly243 consumer is not a new callable Source constructor path.

At this baseline, both latest 61-pin primary reports replay **61/61**. The older
Source69 report replays **56/57** with CMake-only drift; older reports and the
selection snapshot retain their explicitly recorded version differences.
Latest primary evidence retains all 20 legacy-owner functions and eight Source
EH sections unchanged, as well as Source69/raw83/getter bodies. Build/Core/EH
receipts are accepted Source snapshots reused by report pin, not a worker build,
artifact reread, or proof of Native C8F3B8/C964E8 semantics.

The evidence-selected data lead is **00D08AE4+0**, the slot-zero word of
the last root profile written by owned code. This is a numeric, unopened
profile-slot candidate within this worker's evidence, not an independently
resolved function address or guaranteed later profile. Proving the post-child
profile requires the separate 007F0F80 contract
or other bounded evidence. CFD99C is the earlier root-profile phase; CF6514 is
the member-profile write. Neither substitutes for the cleanup loop's actual
current node profile. No profile/slot/handler/caller/child/neighbor Native bytes
were opened by this worker; only the exact owned 136-byte body was decoded.

After this baseline, Root reported a separate reparent EH packet published at
`3cff8776a`; it is not imported or recounted here, and the actual Native
interpreter/dispatch contract remains separate. Root also reported independent
word-only identities `CFD99C+0 -> 0071C4D0` and `D08AE4+0 -> 007F1EE0`.
These later leads are communication context, not this worker's data reads or
target-body evidence. They remain conditional on the current profile and do
not close the unopened member child, constructor handler C8F3B8, target ABI,
deletion effects or runtime behavior.
