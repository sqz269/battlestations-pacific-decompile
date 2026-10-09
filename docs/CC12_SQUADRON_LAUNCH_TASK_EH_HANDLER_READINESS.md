# Squadron launch task exception-handler readiness

The task constructor, embedded-member constructor and cleanup use three distinct
Native descriptors. Their handler stubs and four cleanup actions account for
65 bytes; the shared frame-handler adapter adds54 bytes. All119 code bytes /
42 operations and140 selected descriptor/map bytes agree with fresh live GPR
reads and the original executable. This is a read-only audit with zero Source,
Original ABI, startup or gameplay credit.

The cleanup's second unwind record sends the current stored object plus20h to
`006569A0`. Normal cleanup sends the current object plus20h to `00653390`.
These are different physical targets; this packet proves no interchangeability.

## Native handler and action bytes

| Owner | Handler10-byte stub | Nine-word descriptor | Map records |
| --- | --- | --- | --- |
| `007F1DE0` constructor136 | `C8F3B8`: EAX=DBFDF4; tail BF6B43 | DBFDF4: 19930522,1,DBFDEC,0,0,0,0,0,1 | `(-1,C8F3B0)` |
| `007F0F80` member constructor92 | `C8F398`: EAX=DBFDC8; tail BF6B43 | DBFDC8: 19930522,1,DBFDC0,0,0,0,0,0,1 | `(-1,C8F390)` |
| `007F1E70` cleanup99 | `C8F3E3`: EAX=DBFE28; tail BF6B43 | DBFE28: 19930522,2,DBFE18,0,0,0,0,0,1 | `(-1,C8F3D0)`, `(0,C8F3D8)` |

The first word of each eight-byte map record is shown signed for readability;
the report retains raw DWORD values. The descriptor's second word matches the
number of selected records, and its third word points to those bytes. Their
interpretation as state transitions is supported by the related owned state
stores and MSVC-style frame layout, but the Native InternalCxxFrameHandler body
was not opened. No dispatch, exception-search, unwind-mode, catch, or failure
policy equivalence is proved solely by these tuples.

All four actions reload the actual DWORD at current EBP-10h at action entry.
None uses the normal owner's preserved ESI as its input. C8F3B0 and C8F3D0 tail
to875B30; C8F390 tails to695870. C8F3D8 adds20h to the loaded ECX and tails
to6569A0. The ADD wraps as DWORD arithmetic and writes arithmetic flags. Other
action instructions are MOV/JMP and do not write arithmetic flags. Actions add
no own pushes, calls, local frame, return, or EAX semantic result. Their outgoing
EDX and other registers are inherited; target return/stack/nonvolatile and frame
validity requirements remain external. The actual late saved word can differ
from a normal-path register capture if aliases or a child change frame backing.

All three handler entries have saved MOV instructions but no enclosing saved
Ghidra function. All four actions have existing Unwind functions. Neither entry
creation, listing repair, signature, name, comment, flow flag nor project save
was performed. Correct library and synthetic unwind names are retained.

## Complete54-byte shared adapter

`BF6B43..BF6B78` is saved as `FID_conflict:___CxxFrameHandler3`. Its incoming
descriptor is hidden EAX; four stack DWORDs are physically consumed as call
inputs without declaring semantic parameter names from the saved undefined(void)
prototype. It saves EBP, establishes EBP=entryESP-4, allocates eight bytes, saves
EBX/ESI/EDI, executes CLD, and stores incoming EAX at EBP-4. XOR EAX then supplies
three zero DWORDs. The descriptor word is pushed from its current local slot,
followed by late loads of entry stack words4,3,2,1 in that order.

The call at BF6B64 targets C07991, named `___InternalCxxFrameHandler` by metadata.
Its eight physical stack arguments, in callee order, are the four late caller
words, the selected descriptor, then three zero words. Required normal child
RET0 leaves the adapter to discard32 bytes. The child EAX is stored at EBP-8;
current saved EDI/ESI/EBX are popped before that result is reloaded into EAX.
ESP is restored from current EBP, current saved EBP is popped, and plain RET
leaves the original four caller argument slots in place. With intact stack and
normal callee preservation, caller ESP after return is entryESP+4.

CLD clears DF before the Native child; final DF is whatever the child leaves.
The final owned arithmetic-flag writer is ADD ESP,20h after the child. Those
flags survive the owned stores/pops/reload/ESP restore/RET; no child flag
preservation is implied. All eight pushed words, result locals, saved registers,
return addresses and hidden EBP frame assumptions need valid selected backing.
The body owns one CALL and one RET, no x87/SSE operation and no local EH frame.
This closes the adapter schedule, not the Native interpreter's behavior.

## Composition and Source boundary

The accepted constructor136/member92/cleanup99 reports retain their original
state-store widths and timing: cleanup writes full state1, then byte state0,
then full state-1 before its successive children. A byte store preserves the
current upper24 bits. This packet does not normalize state or claim exception
runtime consumption of a malformed or concurrently changed value. Accepted
owner windows were replayed against their pinned PE spans; no fresh parent,
child, caller, profile or neighboring Native body was opened.

Current875B30 Source is admitted, registered and built as an explicit-reference
borrower with RET8; these Native actions tail to a no-stack-argument entry with
plain RET. It cannot be bound directly into the Native action ABI. Current
observer lifetime providers also need explicit retained lifetime domains.
There is no Source653390/6569A0 provider or raw task-lifetime binding found in
the bounded current include/src search. This packet supplies no RAII substitute,
runtime stub, forced retention, frame thunk, raw allocation, producer or caller.

The existing Win32 build receipt covers67 inputs and four artifacts, with three
existing checks passing, twelve whole objects and fourteen positive public Core
roots. Its eleven prior object contracts are unchanged. Those pins were replayed
here; this packet runs no new build, test or executable and supplies no execution
evidence. Native original ABI, interpreter policy, pointer/lifetime composition,
startup and gameplay remain open.

Current complete evidence is in `reports/cc12_squadron_launch_task_EH_handler_readiness.json`.
