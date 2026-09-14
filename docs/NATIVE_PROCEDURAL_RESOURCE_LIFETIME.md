# Actual procedural resource lifetime

Addresses: 00bbc6d0, 00bbc7f0, 00c304a0, 00737390, 00b19750

This packet completes the five normal bodies below over the actual 34h
allocations produced by the existing BBC6F0/BBC810 factories. It also supplies
a stable `NativeProceduralResourceReference` companion that borrows the same
raw +04 reference count used by sampler, cache and render references. Names
are descriptive hypotheses. These explicit C++ interfaces are not binary ABI
replacements.

| Routine | Coverage, inclusive original body | Original ABI |
| --- | --- | --- |
| BBC6D0 caustics scalar deletion | complete, BBC6D0..BBC6ED, 30 bytes | ECX owner; stacked flags; captured owner in EAX; RET 4 |
| BBC7F0 shore-wave scalar deletion | complete, BBC7F0..BBC80D, 30 bytes | ECX owner; stacked flags; captured owner in EAX; RET 4 |
| C304A0 procedural destruction | complete normal body and source exception cleanup, C304A0..C3054B, 172 bytes | ECX owner; RET |
| 737390 pointer-vector resize | complete, 737390..7373DF, 80 bytes | ECX actual 12-byte header; stacked signed requested count; RET 4 |
| B19750 base destruction | complete, B19750..B1975A, 11 bytes | ECX owner; stamp then tail jump to BD30F0 |

C30470/B19980 establish +04=1, +08=0, +10/+14/+18=0 and byte +1C=0.
Final profiles D64478 and D644B4 have actual virtual 0=BD30E0 and virtual
4=BBC6D0 or BBC7F0. The field at +0C is not initialized by these constructors.
C304A0 does not read, release or clear +08/+0C. No string destruction is
invented for those words, and this packet adds no projected resource allocation.

C304A0 stamps D79B54, reads current count and current pointer data, captures
the last child, decrements its real +04 atomic, and invokes current virtual 0
only on zero. It then rereads count and decrements it if nonzero. Both data and
count are read again for the next iteration. A child terminal may change which
array or element is next, or clear the count. Null children are not skipped.
Normal completion calls 737390(0) on owner+10, frees the CURRENT array pointer,
then calls B19750. Data and capacity remain stale after free. B19750 writes
D5C104 and tail-forwards BD30F0's CEB130 stamp; it does not adjust ECX.

737390 compares signed requested count with signed capacity and calls the
existing 735FF0 reserve only when larger. Growth captures count after reserve,
uses wrapped 32-bit index*4 address arithmetic, rereads current data per slot,
zeroes each nonnull placement address, and publishes no intermediate count.
Shrink decrements the current count before the final requested-count store;
pointer elements have no destructor. The caller supplies valid aligned Win32
storage and valid native-domain capacities/counts.

BBC6D0/BBC7F0 capture the owner, call C304A0, then free only if bit 0 of the
stacked flags is set and destruction returned. EAX is the captured address
even after free. The returning ADD ESP,4 instructions at BBC6E5 and BBC805,
and the full C304A0 tail beyond C30528, are present in the installed executable.

The existing `NativeRenderActualOwners` domain owns canonical identity; this
packet introduces no identity map or automatic registration. A companion must
be admitted to the SAME domain as the sampler/cache references, one per actual
allocation. Construction borrows actual +04 without initialization or retain.
On zero it verifies current virtual 0=BD30E0, invokes the existing concrete
BD30E0 provider, then rereads current profile and requires recovered virtual 4.
A current supported profile may differ from the construction profile. On
successful destruction/free, a nonthrowing callback removes that same canonical
binding and may delete the companion. All actual profiles, context, owner domain
and retirement dependencies must remain alive; terminal dependencies must not
throw. Unadmitted identity/profile is an error, never a successful free.

The FH3 descriptor at E03BDC references the two-state map at E03BCC.
State 1 uses CC7EB8, adds 10h to captured owner and jumps to 737BF0:
resize current pointer array to zero, then free current data. State 0 uses
CC7EB0, forwarding via C30260 to B19750. Normal C304A0 sets state 0 before
its explicit resize/free, so a failure there must not repeat array cleanup.
Source C++ catches preserve these cleanup effects and skip scalar owner free
after failure. The unwind helper and funclets are consumed evidence, not added
packet implementations or leased reconstruction addresses.

The one-shot operation records reached owner/child/array identities and cleanup
completion. Failed operations reject replay and terminate if destroyed before
diagnostic acknowledgement. Recorded addresses may already have been freed.
After a failed canonical lookup, the real child count is already zero while
the pointer array and base cleanup have completed: the caller resolves that
retained child, frees remaining owner storage, and only then acknowledges.
Acknowledgement frees nothing. Original FH3 identity, native exception delivery,
SEH/hardware faults and binary ABI equivalence are outside this source contract.

Validation uses the repository Win32 build, both existing CTests, eight native
seeds, and one focused local fixture. Four schedules compare complete copied
original factories, vector resize and both scalar/destructor paths with source:
flags 0/3, reverse children, current-array/count mutation, and shared count.
Actual child terminals and allocator/vector-reserve dependencies use existing
source providers. Both fresh factory profiles are independently admitted
through actual 4DDB40 sampler release, including current profile changes.
Source canonical lookup failure proves vector/base unwind, skipped scalar free,
retained zero child, replay rejection and the failed-frame retirement guard.
The original native FH3 handlers are never executed by the fixture. Native
child virtual 0 is redirected to the same canonical source terminal, so this
is a qualified differential boundary, not original child-terminal execution.
All five packet original normal bodies execute. No game run is claimed.

Before root repairs, `verify_report_calls.py` checks 14 direct rows and reports
only C30536 outside the truncated Ghidra body. The report keeps that true call
row and the failure rather than hiding the boundary. Root must preserve old
comments, correct the return flow/body, apply proposed names, save, refresh
exports and rerun this check after lease release. BBC6D0/BBC7F0 need their
three-byte ADD ESP,4 gaps repaired; C304A0 needs its tail through C3054B.
The optional unowned 737BF0 cleanup likewise needs its returning tail through
737C06, under a separate root lease. 737390 and B19750 already have full bodies.

The committed report records exact bytes, ABI, host sites, old/proposed names,
corrections and limitations. `local/native-procedural-resource-final/manifest.json`
closes code, documentation/report, probe sources and inputs, executable/object,
outputs, library and toolchain hashes in an immutable local delivery directory.

## Integrated library validation

At `d260af28` this source is registered once in the default Win32 target.
The combined `scripts/build.ps1` build and both existing CTests pass. The
packet's focused fixture was compiled and run against the integrated
`bsp_core.lib`, with saved commands, stdout, exit status, dependency headers
and exact library hashes. The four current packet reports contain 55 checked
numeric call rows and zero failures. Source/header bytes match the reviewed
worker delivery. Fourteen saved names/comments were read back and exported
with previous comments preserved. Exact evidence: `local/checkpoints/d260af28/native-online-procedural-default/validation.json`.
This supersedes earlier worker-specific build or fixture-log limitations;
original ABI/FH3 delivery, full manager/pump adoption and gameplay remain open.

The integrator completed C304A0 through C3054B, both scalar stack-cleanup
gaps, and the separately leased 737BF0 cleanup through 737C06. Their final
listings have 56, 11, 11 and 10 instructions respectively, with zero gaps.
Handler CC7EC3..CC7ECC was defined and saved. The formerly out-of-body call
at C30536 now passes the unchanged 14-row call report. Earlier truncated
body observations remain preserved in the worker snapshot. The helper and
handler repairs add no extra reconstructed normal bodies.
