# Native VFS mount-tree cleanup

Addresses: `00BD94D0`, `00BD94F0`, `00BD9530`, `00BDA0E0`, `00BDF7E0`,
`00BE0080`, `00BE0D00`, `00BE16C0`.

The source reconstructs the complete logical bodies against actual Win32
storage. It releases key strings and node allocations, without releasing the
provider at node+18. Both partial and full range erasure are implemented.
Names are descriptive hypotheses. The explicit C++ service parameters and
exception transport are new interfaces, not binary replacements.

| Entry | Logical end exclusive | Bytes | Native ABI | Coverage |
| --- | --- | ---: | --- | --- |
| BD94D0 | BD94EC | 28 | ECX node, EAX maximum, RET | complete |
| BD94F0 | BD950B | 27 | ECX node, EAX minimum, RET | complete |
| BD9530 | BD9582 | 82 | ECX tree, stack node, RET4, result unspecified | complete |
| BDA0E0 | BDA12E | 78 | ECX tree, stack node, RET4, result unspecified | complete |
| BDF7E0 | BDF832 | 82 | ECX tree forwarded to recursion, stack node, RET4 | complete, returning string-pool domain |
| BE0080 | BE034C | 716 | ECX tree; stack output/owner/node; EAX output; RET0C | complete, source CRT/EH and returning string-pool domains |
| BE0D00 | BE0DC9 | 201 | ECX tree; stack output/first-owner/first-node/last-owner/last-node; EAX output; RET14 | complete, composed domains above |
| BE16C0 | BE16F4 | 52 | ECX tree, RET, EAX0 in native tail | complete, composed domains above |

The eight bodies total **1,266 bytes**. Fifteen bounded body/data spans totaling
1,498 bytes were compared between live Ghidra and the installed PE; complete
hex and SHA256 values are in [the report](../reports/native_vfs_mount_tree.json).
Six additional producer, unwind and exception-data spans add 283 bytes, for
21 verified spans and 1,781 bytes in total.
Each CLI batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language, and image base. The installed PE at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`
has SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Workers made no Ghidra mutations.

## Storage and call contracts

The owner has current head/count at +4/+8; its +0 word is preserved. Head links
are minimum/root/maximum at +0/+4/+8. Nodes have left/parent/right at +0/+4/+8,
an eight-byte string at +10/+14, provider pointer at +18, color at +20
(black=1), and nil byte at +21. Other payload and padding bytes are untouched.
`BDABF0` allocates 24h and initializes links and color/nil. `BE1DC0` at
BE1E34..BE1E51 establishes manager+3C's head, sets nil=1, self-links, and count0.
The insertion producer `BE05C0` writes all links, calls `BDEE60` with node+0C,
then writes color/nil. `BDEE60` copies its leading word, constructs/copies its
string at +4/+8, then copies its provider word at +0C and flag byte at +10.
These writes establish the cleanup offsets without inferring them from a map
type. Consumers BE06A0/BE0750 release provider+18 before calling BE0080.

| Callee | Body-reviewed contract and cleanup | Source composition |
| --- | --- | --- |
| BD94D0 / BD94F0 | Follow right/left until child's nil byte is nonzero; ECX node, RET, EAX selected node | This module |
| BD9530 / BDA0E0 | Right/left rotation with current head and parent reloads; ECX tree, stack node, RET4 | This module |
| BD97E0 | Actual checked iterator `{owner,node}` at +0/+4; ECX iterator, RET; invalid nil advance tail-calls CRT | `native_vfs_date_route` |
| 419CC0 | No consumed arguments; EAX current/lazily constructed pool; RET | `ActualNativeStringPoolStorage` |
| BD1510 | ECX pool; stack block/length+1/1, RET0C; third argument unused | Existing string-pool storage |
| BF65AC | Correct library `_free` thunk to returning CRT free; cdecl pointer, caller ADD ESP,4 | `singleton_lifetime_free` |
| BF6713 | Returning-capable invalid-parameter wrapper; no caller arguments, RET | Shared `SingletonLifetimeCallbacks` |
| 408720 | ECX actual SBO header; stack text/count, RET8, EAX header; reserve and counted copy | Existing native legacy SBO string |
| 411700 | ECX raw 28h exception destination; stack SBO source, RET4, EAX destination | Existing native legacy logic-error owner |
| BF6885 | Correct `__CxxThrowException@8`; object/ThrowInfo, RET8 if RaiseException returns | Existing owning `NativeHardwareLayoutInvalidIterator` source transport |

The three words pushed before 419CC0 are pending arguments of BD1510, not
getter arguments. Production callers pass the shared
`ActualNativeStringPoolStorage` through the `NativeStringStorage&` interface;
the bridge repeats the actual getter and return operation for each key. No
private pool or cached singleton is introduced. Its inherited `release` is
`noexcept`: failure while lazily reconstructing the native pool terminates in
the source interface. Native throwing-getter/FH3 equivalence is not claimed.

## Recovered control flow and exceptions

BDF7E0 recursively releases the current right subtree, reads the current key
data, captures the current left link, reads length+1 only for nonnull data,
returns the string, frees the node, then iterates the captured left. This
ordering preserves mutations made by string-pool callbacks and does not change
the tree's head/count.

BE0080 retains the input node, advances its by-value iterator, and transplants
the successor when both children are real. BE018C..BE01E3 is reachable
assembly despite the saved pseudocode's unreachable-block warning. The
transplant exchanges color bytes and updates the original node's surviving
links; repair handles both directions, red siblings, black siblings, near/far
children and root ascent. Nil replacement parent is held separately instead
of being written into the shared sentinel. String release occurs after
repair. The post-free tail reads the current unsigned count, decrements only
when nonzero, then writes the advanced owner before the advanced node.

BE0D00 captures minimum before first-owner validation. A first iterator equal
to that minimum also captures head and validates the last owner, even if the
last node later sends execution to the partial branch. Full erasure reloads
the current head between root/count/minimum/maximum resets and before returning
begin. Partial erasure compares owners before endpoint nodes, advances first
before calling iterator erase on its captured predecessor, and reloads first
after each call. BE16C0 calls full range, frees the current head, and then zeros
owner+4/+8.

Sentinel erase constructs the counted 27-byte message at CE44E0, arms state0
only after assignment, constructs 411700's logic-error owner, publishes native
out_of_range identity D6926C, and invokes ThrowInfo D863A8. Handler CC66C8 loads
FuncInfo E00CC4; its single unwind entry E00CBC maps state0 to -1 through
CC66C0, which destroys the completed temporary at frame-50 using 4072D0.
The source retains this arming point and cleanup using the canonical owning
exception transport, with source RTTI/catch and EH identities.

## Stored-body gaps for integration

| Span, end exclusive | Current saved analysis | Recovered instructions |
| --- | --- | --- |
| BDF821..BDF82C | Internal gap in BDF7E0's existing body | ADD ESP,4; test captured left nil; move left to current; loop |
| BE0312..BE034C | No containing function; BE0080 stored end is BE0311 inclusive | Current count decrement, output iterator, register/FS restoration, RET0C |
| BE16E4..BE16F4 | No containing function; BE16C0 stored end is BE16E3 inclusive | Caller cleanup, EAX0, head/count zero, restoration, RET |
| CC66C8..CC66D2 | No function at handler start | MOV EAX,E00CC4; JMP BF6B43; static EH evidence only |

BD94DD..BD94E0 is skipped `LEA ECX,[ECX]` alignment, and BE0D7F is a NOP.
Neither is a missing behavioral branch. Reported proposed names preserve the
previous FUN names; the integrator owns any locked body repair, annotation,
comment preservation, export refresh and project save. No global no-return
change or unsupported body extension is requested.

## Verification and limits

`scripts/build.ps1` passed MSVC Win32 `/W4 /WX /fp:strict`, with process-scoped
`MSBUILDDISABLENODEREUSE=1`. `verify-seeds` succeeded, followed by both existing
CTests: reconstructed math and native math differential. No permanent tests
were added.
`verify_report_calls.py` checked all 37 direct-call and tail-transfer rows with
zero failures. The manager unwind references at CC6934/CC69B4 are JMPs in
the actual listings, despite their call-labelled xref output.

One ignored differential fixture uses small raw trees to exercise nineteen
concrete cases: leaf/immediate/deep successor removal; both rotation directions
with red siblings and near/far children; partial/full/empty range; subtree
captured-left mutation; destructor clearing; post-release count mutation;
zero count; returning endpoint validations; and owning sentinel exception.
It compares release traces, output identity, current owner/head/extrema/count,
live node links/colors and untouched payload words. All nineteen cases pass.
This is bounded fixture evidence, not exhaustive tree-state proof.

The passing fixture is frozen under `local/mount_tree_as/frozen_v5/`; log and
manifests are `probe_run5.json`, `pre_run_manifest_v5.json` and
`post_run_manifest_v5.json`. Headers, source, object, library, probe source,
linked executable and original PE copy were hashed before the first run and
all remained unchanged afterward. The report records their hashes and command.
The original executable was only read, never launched or modified.

Original bytes execute in a private process, with original BD0000..E10000
mapped at +30000000. Relative internal calls remain relative. Four low-address
CALL operands (BDF80F/BE0300 pool getter, BE00C8 counted assign, BE00DA owner
construct) route to reviewed source services. Mapped BD1510/BF65AC/BF6713/
BF6885 entries redirect to pool return/free/validation/throw observation. The
absolute message operand at BE00AF is relocated; numeric profile/ThrowInfo
identities remain unchanged. The original invalid branch reaches its throw
observer, which verifies native owner/message/ThrowInfo, performs explicit
fixture cleanup, restores captured FS, and returns nonlocally. Source throw
and catch use the owning reconstructed type. Original FH3 dispatch and native
unwinding are not executed. Only this fixture's isolated nonlocal-return/FS
code suppresses C4611/C4733; production source has no warning suppressions.

Failed compilation and execution evidence is preserved: compile attempts
exposed those deliberate fixture portability warnings; runs1/2/4 failed to
reserve low-address windows; run3 passed its first sixteen cases then rejected
an incorrect one-callback expectation. Both native endpoint validations are
required, so that fixture expectation was corrected. No production source
change was needed after differential execution began. No game validation or
native binary ABI compatibility is claimed.
