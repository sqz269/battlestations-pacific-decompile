# Native online notification-9 branch

Addresses: 00A40110 (fragment 00A401A9..00A401CC)

This packet reconstructs the selected notification-9 branch over the actual
manager allocation. It does **not** implement the notification drain, manager
construction, or platform resource-load event. Names are hypotheses. Analysis
used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, read-only.

| Containing routine | Source entry | Coverage |
|---|---|---|
| A40110 | `apply_native_online_notification9_fragment_00a401a9` | Partial: A401A9..A401CC only; A40110..A401A8 and A401CD..A404F3 remain outside this implementation |

The original entry is ECX=manager, no stack arguments, RET at A402F7. The branch
is reached through the six-DWORD table A404F4 after subtracting notification ID
9. It enters with captured ESI=manager, EBX=0 and a mutable DWORD parameter at
`[ESP+14]`; its final JMP reaches the next-notification fetch at A402C6. The new
C++ function exposes those branch inputs explicitly. Returning from this helper
does not stand in for returning from A40110.

## Storage and producer evidence

73DC50 pushes size 3F0 to allocator BF681B (cdecl ADD ESP,4), then 73DC7C calls
A40DF0 with ECX=that allocation and two callback words; the constructor RET8
confirms the count. Its base A3F530 publishes that same pointer at F8ABE8 before
derived initialization and lifetime registration. A40EB5 writes zero at +3E8
using BL=0 established at A40E29. The entire producer bodies were read, including
the later pump before the +12C/+14C writes.

`NativeOnlineManagerStorage` is exactly 3F0 bytes with only +3E8 named; other
bytes remain opaque. It supplies no initialization or ownership. The existing
`XLiveOwnerAllocation`, `XLiveManagerOwner`, and online/pump state types are
explicit projections and cannot be cast into this layout. Real constructor,
registration and deletion providers remain prerequisites for a live owner.

## Callback and writes

| Actual call site | Interface | Verified contract |
|---|---|---|
| A401B9 | Current F8ABEC function pointer | Optional; CL is SETNZ of the DWORD parameter, no stack arguments, RET. Startup's sole observed writer 4E555E installs 4CEB40. Its full 45-instruction body reads CL and performs game UI changes. |

A401A9 captures the pointer once. If it is nonnull, A401B2/B6 normalize the
current parameter before calling it. A401BB/BF then read and normalize the
parameter **again**, after the callback, and A401C2 stores that byte to the
captured manager. Changes to the published manager or callback slot do not
redirect this store. The parameter is a DWORD: 00000100 produces 1, not 0.
This producer writes 0 or 1, but later platform consumers must preserve the raw
byte they actually read; this branch does not justify projecting that storage
through a shared C++ bool.

The other direct 4CEB40 caller at 4DB2CC sets CL=1 and pushes no arguments;
it agrees with this callback ABI. The old typed notification implementation
captures a C++ bool before its hook and reuses it afterward. This packet instead
preserves the two distinct parameter reads established by assembly.

The hook type represents the observed x86 ABI directly. This packet supplies
no fake successful game UI callback or manager adapter. Callers must keep the
captured allocation, parameter cell and pointer slot alive through the hook and
exclude concurrent access or retirement. Synchronous hook mutations are allowed.
A C++ hook exception propagates before the final store and retains the hook's
effects. The helper acquires no resources; it does not reproduce original FH3.

## Remaining closure

Full A40110 still needs listener creation, A3F3E0 debounce, the repeated SDK
fetch/log/dispatch, A3F440 sign-in handling (both sign-in helpers reach A3EBD0),
A3E600 profile refresh, accepted-invite copying, connection/achievement handling,
F8ABF0 and F8A2FC callbacks, and update launch/update-system/exit branches. The
actual raw constructor/lifetime and concrete 4CEB40 UI integration also remain
separate. A409F0 and the actual BECCD0/BECB20 event therefore remain incomplete.
No no-function body is used by the implemented branch. The four complete
evidence bodies have no saved-flow gaps.

## Validation and limits

The focused fixture enters the exact 36 disk/live-verified branch bytes with
the original ESI/EBX/stack-cell contract. All original branch bytes and absolute
operands remain unchanged; the copied code and its relative-jump destination
move together, with a RET adapter at the location corresponding to A402C6
outside the branch. The F8ABEC absolute global operand is unchanged. It compares
every byte of two raw 3F0 allocations and the parameter,
hook and publication observations against the linked source. Four cases cover
the absent hook, high parameter bits, and both directions of post-hook parameter
mutation. The diagnostic hook also changes F8ABE8 and F8ABEC to check capture
timing. A source-only hook throw checks retention of its effects and omission
of the final store.

These are controlled raw preimages and a diagnostic ABI hook, not execution of
the full manager constructor, real 4CEB40 game callback, whole A40110, SDK, or
game frame. No active window or game state is changed. Build/fixture results,
original bytes, numeric call verification and transitive input hashes are
recorded in the report and ignored final artifact manifest.

The default MSVC Win32 Release build with /W4 /WX and both existing CTests
passed. All four original/source comparisons and the source-only exception
case passed. Nine evidence spans match disk/live bytes; all 60 numeric call
rows pass verification (seven indirect rows are explicitly described).
