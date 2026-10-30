# Independent raw Lua variant C++ cleanup review

Immutable candidate `a4774a47499cf3ad0f38af058a53fef1e71cea61` passes this
bounded Source/native/COFF/EH review, with no blocking finding or required
correction. It is ready for Root's separate strict/common build and link
admission. This review performs no compilation, link or reconstructed-body
execution and grants no runtime, activation or Original ABI credit.

The review worktree is frozen at `931c998604c5799fcfdf2fc03db2305f4402fb81`.
The comparison is immutable normal Source `17b77e69a79d82a5b89841edb23c2046cfe86cd1`,
design `01809626609ed59fa0ba1f2c763fbfa9607cd01f` and independent design review
`288e47ff98ef80ac51ad82805f3bf08af6fff670`. The complete 231-line header,
548-line implementation, 169-line implementation document and 4419-line
structured report were reviewed. Both named design documents and their complete
structured reports were also reviewed. All candidate data were pinned from Git
before reading them; mutable author Source was not used as the acceptance input.

All four candidate blobs, five prior immutable input pins, 21 unchanged
dependencies and 17 compile/static artifact receipts match. The original header
declaration prefix, 120-byte Frame, 24-byte Operation, normal signatures and both
legacy Host sources are preserved. The explicit 83a63 readiness report matches
its frozen SHA256; that establishes no new producer/host adoption.

## Native scopes and real action providers

The installed image SHA256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
An independent PE parser and x86 decoder verified all 11 spans: 751 bytes and
224 code instructions. The three original bodies account for 419 bytes and
138 instructions. DC9464 and DB10B8 each contain one state0 -> -1 unwind entry,
with C970D0/C830A0 and FuncInfo DC946C/DB10C0 respectively. The original EH
actions, handlers, profile data and reached whole-range evidence all match.
There was no live Ghidra batch or mutation; saved project/program identity is
inherited evidence, not a fresh live-program assertion.

| Local scope | Normal epoch | Qualified exceptional effect |
| --- | --- | --- |
| 884AF0 | Saved V; numerical D0E6F4; arm before clear. Capture CURRENT V+10h before demotion, then conditional late V+Ch length/getter/return. | Demote first; the full raw41DD20 contract reads CURRENT savedV+10h, then conditional CURRENT savedV+Ch. C970D0 physically loads saved V, adds Ch and jumps to41DD20. |
| 6EE890 | Right recursion is unarmed. After return capture CURRENT LEFT, retain K=N+Ch at the argument-home rewrite, arm before value destruction. Capture CURRENT K+8 before demotion. | Demote first; the full6EE020 contract reads CURRENT savedK+8, then conditional CURRENT savedK+4. C830A0 physically loads the overwritten argument-home VALUE and jumps to6EE020. |

Source retains K's VALUE at the real rewrite epoch. It supplies no mutable
Original private-stack home or arbitrary private-home alias/replacement proof.
No key is reconstructed from a later head/node/LEFT, failed-child metadata or
the buffer captured by a failed normal operation. A stale prior K during later
right recursion cannot arm cleanup. D0E6F4 and Native action addresses remain
evidence data, never callable Source tables or identity authentication.

The two private kernels correctly compose the existing raw-context overload of
`destroy_native_string_header_0041dd20` and
`return_native_lua_variant_string_fields_006ee020`. Each captures the actual
buffer before the null test; null skips length and services. A nonnull buffer
captures one late length, wraps length+1 as a DWORD, calls actual419CC0 and then
actualBD1510 with the captured buffer/size and live AA4 gate. Getter invocation
is retained for large returns and a disabled small-return gate. No pre-read/
helper-reread, header clear or extra variant/node/map/descriptor free appears.

Both modes borrow the same actual AA8 pool, AA0 raw-manager and AA4 gate
references. Matching deletion binding, manager/critical-section/ring/storage,
malloc/free domain and independent metadata must remain live through last use.
The semantic storage overload's different noexcept contract is not substituted.
Existing BD1510/free/CRT/OS boundaries remain unchanged.

## Complete emitted review and explicit normal drift

A separate COFF parser exactly agrees with every supplied section payload,
symbol, relocation and operation: 36 code sections, 5428 bytes, 1631 operations
and all 73 direct calls. Every operation was inspected. All three genuine
provider imports and only the existing C++ runtime/cookie imports remain.
All 73 calls resolve through their actual relocations, including six cookie
checks; no arbitrary callback, allocator, termination or exception-object
retention import was introduced.

All seven EH sections are accounted for, including every one of 656 bytes:
24 SafeSEH bytes and six xdata tables. Each handler lands on a decoded instruction
start. Four tables have maxState2 and one catch-all; qualified destructor and
subtree have maxState4 and two catch-alls. Their unwind maps are (-1,-1,1,1),
with zero action words. Source action calls occur in explicit handlers, not in
Native unwind-map pointers. Secondary targets at destructor1D7h/subtree260h set
their distinct flag and share the bare-rethrow tails at1E2h/26Bh.

| Body | Old normal public bytes/ops | New normal selector bytes | Complete normal kernel bytes/ops | Qualified kernel bytes/ops |
| --- | --- | --- | --- | --- |
| clear | 849/260 | 23 | 852/261 | 973/300 |
| destructor | 356/120 | 23 | 359/121 | 493/160 |
| subtree | 478/155 | 27 | 484/157 | 630/198 |

Independent comparison accounts for the added backing-argument pushes and
stack adjustments, right-call argument scheduling, branch displacements and
relocation retargeting. All original raw capture/status schedules remain.
Ten shared support bodies and normalized relocations exactly match17b77.
Normal EH handler raw code and each 88-byte normal xdata payload match the old
object; FuncInfo identities, handler offsets and symbol targets change.
SafeSEH grows12 ->24 bytes. There is no unchanged whole-object/EH claim.

The compiler's qualified destructor places EH state0 at7Bh and eligibility=1
at84h, before nonthrowing volatile profile/home stores. Saved home is present
atB5h before clearBFh. Normal bufferF4h precedes retained demotionFAh and physical
disarm104h. Exceptional demotion1B3h/stage1BBh precede action1C6h. The dead local
false store in that catch is eliminated. Subtree is physically unarmed before
rightC8h, armed before value138h, and disarmed after normal buffer14Eh.
These are synchronous C++ conclusions at actual throwing calls. They do not
establish Original fault/SEH/unmasked-FP/private-frame state epochs across pure
volatile operations. Metadata SSE bit copies add no floating arithmetic proof.

## Modes, status, custody and exception transport

All four internal edges select their caller's mode. Ordinary entry reachability
contains no action kernel or qualified body. Qualified reachability closes
clear -> subtree, subtree -> right subtree, subtree -> destructor and destructor
-> clear; it never crosses into a normal kernel. All eight emitted mode/status
edge receipts match independent decoding. Each returned non-completion exits
before later raw reads/free/reset/output, invokes no action, fabricates no throw
and forces no demotion. A stopped armed scope may retain native state0.

Both clear modes prepare frame/sidecar backing before88693F. The normal
D9h..17Fh and qualified126h..1CCh endpoint/predicate/root intervals have zero
CALLs, including the final site store. Both retain four post-subtree CURRENT
head reads, count/link resets, owner-then-node output, and a later separate
current-head free reload. No map.count/default/null/cycle admission was added.

Incoming observations precede exceptional demotion and remain separate from
action captures. Lexical handlers retain their own frame, sidecar and home;
retained inner frames never make active_depth-1 their index. Caller-owned
typed/disjoint/trivial backing must cover the actual active chain, including
range frames and nested maps. There is no fixed/count/history-derived limit,
allocation, journal replay, rollback, retry or abandonment/cleanup destructor.
Reentrant callbacks require independent backing and cannot mutate active records.

An unarmed catch retains/rethrows. An armed catch demotes, invokes its actual
action, records only action completion, and bare-rethrows the incoming exception
when cleanup returns. It neither completes/pops a failed frame nor resumes the
normal suffix. If cleanup itself throws, its distinct stage/captured custody and
secondary flag remain, and that actual escaping exception is rethrown. Outer
still-armed scopes may run their own actions against their own saved homes.
There is no wrapping, replacement, success conversion, forced termination or
extra object free. Passive records retain neither exception object's identity.
Returned buffers may leave stale headers and never authorize automatic retry.

The positive contract requires genuine C++ delivery across these Source scopes
and each invoked action returning normally. Genuine nonnull AA8, its matching
live valid owner/section/ring and returning existing operations supply a
sufficient subset; this is not a cached path or runtime precheck. Real lazy
completion remains allowed and can fail. Direct real-provider calls and ordinary
parent catches do not independently prove every naked/register/provider-boundary
failure is transported as a C++ exception. Native FH3/SEH, faults, nonlocal exit,
existing noexcept/CRT/process termination, primary-object retention and Original
secondary-terminal equivalence remain outside this review's claim.

## Archive custody and remaining admission

The historical Core archive `bb5551c5...` and its three exact provider members
were hash-verified before Root's concurrent common work changed the archive.
The refreshed archive `ec898b6f...` was then independently parsed: its first
linker index proves all three definitions, and their exact member hashes remain
unchanged. Both receipts are preserved. Root attributes the movement to its
Frame/string/VFS common builds, before merging this candidate. Neither archive
is presented here as a common build/link of the new qualified retirement Source.

Root must perform its separate candidate common compile/build/link admission.
Full host/queue/payload/vector/reserve/copy/scalar/producer/caller adoption,
CF/D0 callable transport, actual object/context authentication, provider failure
delivery, Native ABI/FH3/SEH and gameplay/activation remain separate gates.
This review changes only its two new evidence files and performs zero Source,
CMake, ledger, Ghidra, caller, peer, install, build, test, link or target-body
mutations/runs. New Original/vendor/duplicate, runtime and activation credit is0.
