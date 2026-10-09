# Observer member cleanup exception-handler readiness

The handler used by Native cleanup88 at `00653390` selects one unwind action:
load ECX from current `[EBP-10h]`, then tail-jump to `00695870`. Its descriptor/map
and the accepted cleanup88 state stores support a bounded Source design with
cleanup armed during unregister and disarmed **before** normal destruction.
That prevents the guard from retrying normal destruction when it throws. Native
interpreter dispatch, frame selection and failure during unwind remain unproved.

This read-only packet changes only this document and its report. It adds no
Source binding, Ghidra mutation, build/test/probe or Source/Original ABI credit.
The initial published main was `24f6f23898a647cd2825764f624c5ed70310f8f6`;
after the prior observer audit was published, the worker merged current main
`9b624342e14ef3a89c59e9a204a42491dcb0c29b`. The final evidence baseline is worker
merge `2bfbdf4753a8230078d2140ebd68221bc6f51f59`.

## Exact owned code and selected data

The initial lease covered handler `00C7AF48`. Its descriptor pointer was reported
to Root and the exact 36-byte descriptor range approved and leased before reading.
The descriptor then selected one eight-byte map; that range was reported and
leased before reading. Finally the selected action was reported and leased before
its metadata/body was opened. No neighboring EH body or data was read.

| Selected object | Exact window | Bytes / decoded content |
| --- | --- | --- |
| Handler | `00C7AF48..00C7AF51` | 10 bytes: `MOV EAX,00DA7B14`; `JMP 00BF6B43` |
| Descriptor | `00DA7B14..00DA7B37` | 36 bytes / nine DWORDs, below |
| One map record | `00DA7B0C..00DA7B13` | 8 bytes: `(FFFFFFFF,00C7AF40)` |
| Selected action | `00C7AF40..00C7AF47` | 8 bytes: `MOV ECX,[EBP-10h]`; `JMP 00695870` |

Descriptor DWORDs in order are:

```text
19930522 00000001 00DA7B0C 00000000 00000000
00000000 00000000 00000000 00000001
```

The count/pointer pair selects one record. The record's first raw DWORD is
`FFFFFFFF`, shown as signed -1 only for readability, and its second word selects
the action. Interpreting record index zero as state0 and its first word as the
next state follows the accepted MSVC-style descriptor layout and the matching
owner state stores. No internal dispatch, catch, search, unwind-mode or failure
policy is inferred from the tuples alone.

All 18 code bytes / four operations and 44 selected data bytes match fresh live
reads and the original installed PE. Its full SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The handler hash is `d3ded63692c174109e0260f1b748a29576ef68e663c575c94f169e6211887bf5`;
the action hash is `a8f148e7a6b2fbed593b2b5a3c9fdf83eab352dac4df37190216db28602ad8a0`.
The report retains exact descriptor/map bytes and hashes.

Every CLI batch verified project `bsp`, `/battlestationspacific.exe`, x86 LE32
and image base `00400000`; configured `C:/Users/sqz269/bsp.gpr` exists. No stronger
server absolute-project-path claim is made. Function count remains 64729.
`00C7AF48` has no saved function at its entry, but exact instruction-context reads
show both MOV and JMP starts. `Unwind@00c7af40` has a saved eight-byte / two-op
function and matching listing. Its pseudocode renders a call followed by return;
the physical body has zero CALLs, zero RETs and one tail JMP. No function creation,
listing repair, metadata/library rename, script enabling or project save occurred.

## Handler and action contracts

At handler entry H=ESP, MOV replaces EAX with descriptor address `00DA7B14`, then
JMP reaches the accepted `00BF6B43` adapter. The two instructions leave ESP, EBP,
ECX/EDX/EBX/ESI/EDI and all flags unchanged. They add no argument words, return
address, local frame, FS access, x87/SSE operation or conditional branch.

The pinned [FH3 adapter audit](CC12_SQUADRON_LAUNCH_TASK_EH_HANDLER_READINESS.md)
establishes the complete 54-byte / 27-op adapter as context. It accepts hidden EAX
and four physical caller stack words, forwards those plus descriptor and three
zeros to `00C07991`, then conditionally returns through its original caller slot.
The Native interpreter is unopened. Its actual semantic argument types, action
selection and restoration of an unwind-owner frame are not supplied by this
handler. The inherited report's top-level `full_Root_Astra_assembly_read` is true;
its nested capture `Root_Astra_whole_body_gate` flag remains false. Both values
are retained without silently rewriting that capture or reopening the adapter.

At action entry A=ESP and U=EBP, the only memory access is a four-byte read at
`U-10h`, delivered as ECX to `00695870`. There is no receiver capture from normal
ESI, null test, pointer adjustment or fallback. ESP remains A; EBP, EAX, EDX,
EBX/ESI/EDI and all flags remain the action's incoming values. The destroy entry
therefore receives the existing action caller's return slot and any existing
stack words. A compatible plain RET from the target returns through current
`[A]` and leaves ESP=A+4; concrete return, nonvolatile preservation and backing
validity remain requirements of the target/provider chain.

The action does not reproduce the target body. The current pinned
[pair/owner audit](CC12_OBSERVER_PAIR_RELEASE_OWNER_ABI_READINESS.md) supplies its
193-byte / 61-instruction contract without reopening `00695870` or `006952A0`.
The receiver must designate valid actual owner storage for that contract. An
unusable or zero reloaded word is not repaired by the action.
The action itself never clears or advances the owner's state. Whether the
interpreter advances state before invoking it, or retries it after a failure,
is outside this proof; no Native exactly-once unwind guarantee is claimed.

## Cleanup88 state, FS and late frame dependencies

The pinned [cleanup88 contract](CC12_OBSERVER_ENDPOINT_CLEANUP_ABI_READINESS.md)
uses S=entry ESP, M=entry owner. Its frame contains:

| Location | Accepted owner operation |
| --- | --- |
| `S-4` | Initial DWORD `FFFFFFFF`, then full DWORD0, then full DWORD `FFFFFFFF` |
| `S-8` | Handler immediate `00C7AF48` |
| `S-0Ch` | Saved prior FS:[0]; owner publishes FS:[0]=S-0Ch |
| `S-10h` | Entry ECX pushed, then rewritten from current ESI before profile/endpoint work |
| `S-14h` | Saved incoming ESI |

The owner writes profile `00CF6494` at `[M]`, then captures endpoint E from current
`[ESI+14h]` and TESTs it. At `006533B8` it writes **full DWORD0** to state after
capturing E; the store preserves TEST flags into the zero-endpoint branch.
On the nonzero arm, unregister receives ECX=captured E and EDX=current ESI.
Both normal arms then capture current ESI into ECX at `006533C9`, write **full
DWORD `FFFFFFFF`** at `006533CB`, and call destroy at `006533D3`. None of these
state stores is a byte update. Receiver capture precedes the disarming store.

Under the expected descriptor/state interpretation, state0 covers the armed
interval including unregister; state -1 is present before normal destruction.
The map's selected action is therefore consistent with destroying the member
after unregister failure, while the later normal destruction runs with this
owner cleanup disarmed. This does not prove the unopened interpreter's behavior
for malformed states, async faults, exceptions during cleanup or changed backing.

For the action's `[U-10h]` to select the accepted saved owner word `[S-10h]`,
**U must equal S**. FS's published registration link is S-0Ch. The accepted
shared adapter instead establishes its own EBP from its own handler-entry ESP;
that adapter EBP cannot simply be equated with the owner S. Establishing the
action frame basis belongs to the unopened interpreter/caller machinery.

Even with the intended frame basis, the action reads the **current** saved word.
It can differ from normal-path ESI or a previously captured owner if aliases or
children change that backing. Current state, saved owner, chain, registers and
return slots must remain valid. The normal epilogue restores FS from its current
saved chain; the action itself performs no FS operation or chain restoration.
The accepted owner body was not reopened or repackaged as a new Native proof.

## Bounded Source design and current provider evidence

A later Source cleanup88 can use an explicitly qualified armed guard around the
unregister phase, then disarm the guard **before** calling normal destruction.
The guard may invoke destruction once if unregister exits by a Source C++
exception. Normal destruction belongs outside that armed interval, so its failure
does not cause a second guard-driven destruction attempt. Profile publication and
endpoint capture should retain their accepted order relative to arming.

An explicit owner reference, actual volatile endpoint-cell reference and retained
`NativeObserverLifetime` service provide a concrete Source interface. Such a
guard does not emulate arbitrary writes to Native `[S-10h]` or prove the action's
EBP relation. Public cleanup need not promise noexcept. If destruction throws
during unwind through a nonthrowing guard destructor, current C++ termination
policy must be stated explicitly; it is not established Native FH3 behavior.
No guard, wrapper, production registration or double-exception policy is added
by this packet.

Current Source providers remain present: `NativeObserverLifetime` unregister and
destroy operate through retained manager/lock/dispatch/service state and actual
16-byte owner storage. `GameObserverRuntime` supplies its existing naked
`_invalid_parameter_noinfo` tail helper. The separate admitted Source17 entry
and actual current UCRT import do not establish Native BF66EF handler equivalence.
A fresh bounded address/name search found the existing Source `007EE620` cleanup
entry, but no `00653390` or owned EH entry definition in current include/src. This
is not a broad claim that observer providers are missing.

All 80 Source input hashes, four Source80 artifacts, seven actual provider Source
files and eight selected receipt/document pins were replayed. The Source80 build
at `16:28:10.509150Z..16:28:29.144081Z` passed three existing checks and its receipt
records 16 whole objects / 18 positive Core roots. These are the recorded artifacts
at this packet's capture time; a later build is a separate domain. No object graph
or import table was re-parsed here. Current project/SDK selection remains v145,
Win32 Release, `MultiThreadedDLL`, `ExceptionHandling=Sync`, SDK `10.0.26100.0`,
with the real zero-argument CRT declaration at corecrt.h line371.

The [report](../reports/cc12_observer_member_cleanup_EH_readiness.json) contains the
four decoded operations, exact descriptor/map, current pins and conditional
composition. No Native interpreter, child, other EH body or unselected data was
opened. Original ABI, runtime, startup and gameplay equivalence remain unproved.
