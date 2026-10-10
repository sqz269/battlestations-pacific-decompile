# Locale initializer post-push call: exact entry contract

The admitted bytes at **CC8D3F..CC8D43**, **E8 B1 E2 F2 FF**, prove a direct
32-bit near CALL to **BF6FF5**. The instruction pushes **CC8D44** as its return
address. This establishes call-entry mechanics only; target parameter use,
cleanup, actual return, registration and initializer completion remain unproved.

## Fresh gate and closed capture

Before bytes, all five exact/per-byte gates matched the separate metadata
packet: one unowned five-byte InstructionDB, consistent boundaries, override
NONE, default/effective UNCONDITIONAL_CALL to BF6FF5, fallthrough CC8D44 and
no fallthrough override. The site's target record retains saved _atexit,
no_return=false and nonthunk metadata. No target signature or body was queried.

Every batch verified identity. Typed captures additionally verified the actual
**C:/Users/sqz269/bsp.gpr** marker, **/battlestationspacific.exe**,
x86:LE:32:default and image base 00400000. Fresh before/after epoch is **98**
and function count **64730**. Five typed response pairs and three exact
count/bytes/text pairs match. All Native captures closed before packaging.

Exact-site assembly used context0, with one returned start and empty before/
after arrays. No preceding PUSH refresh, CC8D44 query, target/body/signature,
IAT/CRT/OS, CD9410 callback or FH3/EH data was read. There are **five fresh code
bytes and zero data bytes**. Evidence is saved Ghidra memory/listing; no fresh
original PE comparison or Native execution is claimed. The unit remains unowned.

## Original instruction entry mechanics

In the verified 32-bit mode, opcode E8 consumes signed little-endian rel32
-859471. CC8D44 plus that displacement modulo 2^32 is BF6FF5.
With ESP=T immediately before this CALL, normal instruction transfer gives:

| State | Effect on target entry |
| --- | --- |
| EIP | BF6FF5, unchanged code segment |
| ESP | T-4 modulo 2^32 |
| Stack memory | [SS:T-4]=CC8D44; bytes 44 8D CC 00 |
| Other general registers | unchanged by CALL |
| EFLAGS | unchanged by CALL |

The old stack word at T is now at target-entry ESP+4. These effects end at
target entry: target execution may change state, and no after-return register,
flag, stack-cleanup or status contract is established. Instruction faults and
target exceptions/failure behavior are outside this normal transfer contract.

## Conditional composition across pinned epochs

The accepted epoch95 PUSH, if it completes with ESP=S, leaves ESP=S-4 and
[SS:S-4]=CD9410. If this epoch98 CALL then transfers normally, target-entry
ESP=S-8, [SS:S-8]=CC8D44 and [SS:entry ESP+4]=CD9410. That stack location is
established conditionally; formal parameter use or callback semantics are not.

If S is also the earlier prefix-entry stack value, and its epoch92 call to
436230 actually returns normally under the separately accepted receiver-return,
plain-RET/no-stacked-argument ABI, EAX=E1768C remains through the PUSH and this
CALL's target entry. The earlier callee may clobber ECX/EDX and flags; their
supplied values are merely preserved by PUSH/CALL and remain unspecified.

This is conditional static composition of distinct snapshots. The saved
_atexit label and no_return=false flag do not prove a calling convention,
argument interpretation, target normal return, cleanup, EAX status, registration
success, callback ownership or lifetime. No actual activation or complete
initializer body is established.

## Finite Source and physical proof

Frozen HEAD **a5245948c287cdf1775c0e9f9658bdb0d7f9df8e** closes **4217** Source paths,
**4266** actual selected Git blobs and **11** commit/tree objects. Forty-nine
named dependencies pin tooling and accepted contracts. Actual hashes, full
Source subtree closure and complete enumerated searches replay successfully.
All current Source equals the prior post-push metadata snapshot 0b3db9a64.
Its earlier exact two-file renderer delta and five renderer objects remain
separately authenticated inside the prior archive, without fresh build credit.

Prior post-push metadata core and actual-output delivery archives are physically
nested and replayed. Earlier PUSH/initializer evidence retains epoch95/92 credit;
Root's separate epoch98 comments do not retroactively refresh those snapshots.
The historical Source3356 renderer JSON retains its exact 9db7bcdc... pin and no
build/test/runtime credit. Canonical Source owner, shared-head identity and
borrower/callback lifetimes remain incomplete.

Original and physically copied archives pass ZIP-only guarded replay. External
file reads, processes, network and native-code loading are blocked; three
negative self-probes pass. The ZIP and eleven named replay files are distinct
physical copies with identical bytes. A separate postcommit Git proof pins both
immutable output blobs. Delivery replay requires prior_replay.py alongside
delivery_replay.py and guarded_delivery_replay.py; complete helper lists are
in the report.

Archive: `local/cc12_locale_initializer_post_push_call_contract/evidence.zip`

SHA256: `a3a07caebc4256a76ffb9eabd679c4a969248c2be01f6af6b07e0fc89f09b32f` (140,393,829 bytes).

## Smallest separate metadata proposal

Raw CALL arithmetic and fresh saved fallthrough identify **CC8D44** as the next
metadata candidate only. Its ownership, unit and body remain unread. A separate
packet may query exact identity/ownership/boundary/length/flow before proposing
any bytes. Stop on unknown, changed or overlapping metadata without widening
or repair; no initializer hull or target/body scope is proposed. No C++, GPR,
build, tests, Native probes, ABI execution or game validation were added. Full
pins and uncertainty are in
`reports/cc12_locale_initializer_post_push_call_contract.json`.
