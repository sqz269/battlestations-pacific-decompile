# Native shadow command queue (EQ)

This packet reconstructs four complete normal bodies, 132 native bytes, using
the existing actual pointer-array, scene traversal, instance upload and frame-job
dispatch providers. It adds no command owner, queue object, retention, allocator,
successful unknown-job callback, cleanup or rollback.

| Entry | Inclusive native extent | Bytes | Coverage |
| --- | --- | ---: | --- |
| B1EB80 | B1EB80..B1EBD5 | 86 | Complete normal body within existing provider domains |
| A8AE50 | A8AE50..A8AE75 | 38 | Complete normal body within existing provider domains |
| B1BF20 | B1BF20..B1BF23 | 4 | Complete literal MOV/RET body |
| B1BF30 | B1BF30..B1BF33 | 4 | Complete literal MOV/RET body |

## Queue, original ABI and publication order

B1EB80 receives ECX actual queue and two public DWORDs: command, then flags.
RET8 consumes them. It has no meaningful declared return and preserves ESI/EDI.
The actual embedded pointer-array header is queue+14: current data+14, signed
count+18 and signed capacity+1C. Only equality between current count and capacity
triggers growth. Capacity is doubled with DWORD wrap, then signed-clamped to
minimum1 before the actual B1C6C0 reserve call. Counts greater than capacity do
not become a new repair/growth branch.

After reserve, the body freshly reads count, data and the ORIGINAL public
command word, in that order, before computing/testing data+count*4 for null. It
conditionally writes the command, then increments the CURRENT count word after
the store. Thus a raw destination alias that changes count is observed. Finally
it reads the ORIGINAL public flags low byte. No bool normalization or early
command/flag snapshot is substituted. Flags0 does not dereference access or
command; a null command word can be appended on that native path.

Flags nonzero captures current command+28 context before command+4 scene, pushes
the context, calls actual EL traversal, then calls actual B1E990 upload on the
same captured command. Traversal may reuse its outgoing context argument backing;
the public queue arguments remain in place. The queue count/publication survives
a later traversal or upload failure; the native body has no FH3 frame or cleanup.
This is not a promise that C++ exceptions unwind through the source naked frame.

The B1CC20 specialization is deliberately not used: its current implementation
reads the source cell only after a nonnull destination test. That would omit
B1EB80's required public command read on the computed-null path.

## Job and literal getters

A8AE50 receives one original public command DWORD, ignores original ECX job,
preserves ESI and returns RET4. It captures the command once, invokes B1BF30,
PUSHes the returned context BEFORE invoking B1BF20, then uses returned scene as
ECX for traversal. It uploads the original captured command afterward. The
getters are exactly `MOV EAX,[ECX+28h]; RET` and `MOV EAX,[ECX+4]; RET`: no
reference lookup, extra ownership or copied context. Source explicitly emits
C3 for each one-byte RET; generated whole-four-byte identity remains pending.

## Source interfaces and stack storage

Both main entries are Win32 naked fastcall functions. ECX retains its original
role and EDX supplies `NativeInstanceGroupUploadAccess*`. Let S be original
entry ESP. A single private access word is pushed at S-4; original public
words are neither copied nor moved.

In the queue, after ESI/EDI saves, ESP=S-12: command is ESP+10h, flags ESP+14h
and private access ESP+8. Initial native capacity MOV/CMP stays before the
ESI/header/EDI setup; pushes and LEA preserve its flags. After PUSH context,
access is ESP+0Ch; its reference field at +0 supplies the SAME `access.render`
to EL. Traversal RET4 returns to S-12. The upload adapter receives the saved
upload access from ESP+8. Final pops and private4 removal precede RET8.

In the job, after private access and ESI saves, command is ESP+0Ch (S+4).
After PUSH returned context, private access is ESP+8. The later scene getter
does not move that context word. Traversal RET4 leaves private access at ESP+4;
the upload call is followed by POP ESI/private4 removal/RET4. Native command
capture and each outgoing context slot retain their identities.

The reserve adapter receives ECX actual `NativeRenderPointerArrayStorage`, one
requested public word and RET4, then calls existing
`reserve_native_render_command_pointers_00b1c6c0`. The upload adapter receives
ECX captured command and EDX the same upload access, then invokes
`upload_native_instance_groups_00b1e990(command, access)` with no native public
word. These adapters receive no additional native body credit. Their C++ frames,
compiler register effects and exception behavior are explicit source ABI limits.

## Real job-dispatch composition

`NativeShadowFrameJobDispatch` implements the existing `NativeFrameJobDispatch`
chain, following the preparation/effect job pattern. It borrows the actual live
D5B570 table, fixed upload access and actual remaining dispatcher. The table
pointer is fixed; words remain current. Construction requires slot0 A8AE50.
Each execution uses an integer-only raw DWORD MOV to capture the actual owner's
current profile. Other profiles are passed unchanged to the real remaining
dispatcher. D5B570 requires a fresh table slot0 equal to A8AE50; a changed slot
throws a source diagnostic and does not acquire a successful default route.

The supported branch invokes the actual naked job entry with the original
argument bits as command and the same access. It creates a new C++ argument
slot, so scheduler-frame argument aliases are not claimed. It does not retain
the command or job, resolve a second identity registry, create an owner, or
substitute a table snapshot. Actual owner/table/access/dispatcher backing must
remain valid through execution and reentry.

## Providers, caller evidence and limits

B1C6C0 uses the established actual pointer/count/capacity storage, signed min1
growth and shared CRT malloc/new-handler/free domain. This packet preserves its
existing valid-storage and allocation restrictions; it does not extend its C++
implementation to arbitrary malformed headers, wrapped invalid buffers, native
CRT hooks, or instruction-level alias/exception equivalence. Its returned
header state is freshly consumed by the naked queue.

EL requires canonical borrowed node/camera/model/Traceline table identities and
real current target dispatch. B1E990 requires actual model/scene bindings,
mapping/collection services and its existing finite generator/profile domain
(including B556F0/B55780). These valid live storage and nonthrowing service
contracts are prerequisites. No fake successful provider closes missing parent
behavior, arbitrary rendering, FH3/SEH, concurrency or gameplay.

Fresh xrefs show two queue callers. A8EC50 pushes flag0 and its captured EDI
command, calls the actual 4C11F0 queue getter, moves returned EAX to ECX and calls
B1EB80. B1047E uses the same sequence with flag1. Both preparations and relevant
producer/register evidence are retained from sealed EF/EB. The former later
schedules the captured command against the shadow job; both parent bodies
remain outside this packet. The sole A8AE50 native reference is D5B570 slot0;
each getter has its single direct call in that job.

The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All four complete native spans plus D5B570 slot0 match fresh guarded BSP live
bytes (five spans, 136 bytes). The report pins EM, retained caller evidence,
actual source/provider hashes, original placeholder names/prototypes/comments,
and every numeric native CALL with its containing function. No Ghidra metadata
was changed; placeholder prototypes were not treated as register-ABI evidence.
The canonical live call audit checks nine direct rows; all nine pass.

This is a clean source/evidence candidate. No build, seed verification, new test,
runtime fixture or game execution was run here. Primary will review the source
and coordinate the single EQ+EP build. Generated acceptance must inspect both
naked bodies, exact getter bytes, adapter calling conventions, all stack exits,
raw current-profile reads and both dispatcher routes. Build and runtime claims
remain pending; no parent A8EA20/A8BD20 or full job-queue lifetime credit is added.
