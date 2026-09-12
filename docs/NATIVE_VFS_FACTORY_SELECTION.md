# Native VFS factory-list selection

Address: `00BDB040`. Existing name `BSP_VFS_CreateProviderForPath` is retained
as a descriptive hypothesis. Evidence is in
[native_vfs_factory_selection.json](../reports/native_vfs_factory_selection.json).

| Inclusive native range | Bytes | Coverage | Original ABI |
| --- | ---: | --- | --- |
| BDB040..BDB094 | 85 | complete in stated source domain | ECX manager; system/virtual raw headers on stack; EAX provider/null; RET8 |

`select_native_vfs_factory_00bdb040` traverses the actual factory list at
manager `+30`. It adds explicit create/invalid-parameter services without
constructing a manager, factory, pool, publication or ownership domain. This
supersedes the earlier typed selector projection for actual-storage callers;
the existing typed `VfsProviderFactories` implementation remains separate.

## Exact selection and validation

The initial head comes from manager `+34` and its next link supplies the first
cursor. Each loop reloads the current head, compares it with the retained
cursor, then independently reloads the head for the pre-call validation. The
factory is captured from cursor `+8`, its current table is loaded, and current
slot `+4` is captured before dispatch. Both original header pointers are passed
unchanged. A nonnull result returns immediately, without another head/next read.

After a null result, the routine compares the retained cursor with the current
head. A matching cursor calls the returning invalid-parameter service. It then
loads the **current next link of that retained cursor**, after validation has
returned, and continues against the current head. It never restarts from the
head, caches the next link across a callback, reads list count, changes links,
retains/releases a factory or provider, normalizes a path or adds a fallback.

`BDB050 CMP EDI,EDI`, `BDB052 MOV EBX,[EDI+4]`, then `BDB055 JZ` prove that the
CALL at `BDB057` cannot execute. The head read remains observable, but no source
invalid callback is introduced there. `BDB065` can be reached if the head changes
between its two reads; `BDB083` can be reached by ordinary callback mutation.
The source retains both returning validation boundaries. No atomic snapshot or
concurrency guarantee is added.

Register provenance comes from the entire 85-byte listing: EDI is the fixed
manager+30 list address; ESI is the retained cursor, updated only initially and
at `BDB088`; EBP captures the virtual header; EBX captures the current loop
head. The system header is loaded from its stack argument before each create
call. The source argument values have ordinary C++ by-value storage; arbitrary
original stack-slot aliases are outside this interface.

## Producers, calls and existing services

`BE1DC0` constructs the list at manager+30 using `BDA960`, stores its head at
`+34` and count zero at `+38`. `BDA960` allocates 0Ch and self-links next/previous.
`BDAB20` produces 0Ch nodes with next/previous/factory at `+0/+4/+8`; `BE0660`
appends one before the current sentinel and increments count. These current
producer bodies agree with the existing factory registration implementation.

| Site / containing function | Native target | Contract and evidence |
| --- | --- | --- |
| BDB057 / BDB040 | BF6713 | Unreachable CMP-self validation, preserved in report |
| BDB065 / BDB040 | BF6713 | Current pre-call sentinel validation, returning service |
| BDB078 / BDB040 | Captured factory table+4 | ECX actual factory; PUSH virtual then system; target RET8; EAX result |
| BDB083 / BDB040 | BF6713 | Current post-null sentinel validation, returning service |
| BE18A5 / BE1890 | Captured manager table+18 | ECX manager; same two headers; indirect CALL EDX |

Live data slots `D685CC` and `D68D1C` both contain BDB040. The factory table
`D688B4+4` contains BE8120. All known incoming/data contexts and the complete
selector listing were inspected. The callback interface preserves the captured
target and owner, rather than asking the dispatcher to select an owner or to
reload a table after the capture.

`BF6713` is retained under its existing `LIBCRT_unmatched_00bf6713` identity.
Its body pushes five zero arguments to BF66EF, adds ESP,14h, and returns. This
packet reuses `SingletonLifetimeCallbacks::invalid_parameter`; it does not
reconstruct or silently replace that broader CRT policy.

## Verification and boundaries

The strict MSVC Win32 build with `/W4 /WX /fp:strict` and
`MSBUILDDISABLENODEREUSE=1` passed. All eight seed comparisons and both existing
CTests passed. No tracked tests were added. The 85-byte body and seven supporting
spans match current live Ghidra and the installed PE; Ghidra remained read-only.
All three direct call rows pass mechanical live verification. The two indirect
rows are explicitly qualified and their register calls/slots were checked
against the listing and live bytes.

The ignored fixture passes five native/source cases, 67 checks: empty list;
first nonnull result with poisoned next link; null callback changing head followed
by returning validation repairing next; changed next factory/current table; and
genuine FileStore lazy creation, cache reuse, empty/nonmatching rejection and
unchanged cache after provider deletion. Actual list allocation/registration
and reconstructed FileStore factory/provider lifetime routines are linked.

The fixture executes the original selector bytes with only the three BF6713
CALL displacements rebound. Mutation cases are explicit callback-boundary
observations, not implementations of native physical/MPKG/MPAK factories. A
private D688B4 table preserves the original BE8120 entry value, whose private
code bridge invokes genuine reconstructed FileStore creation. The source
FileStore callee is shared by both sides and is not independently retested as
original code here. The actual string-pool context is borrowed; the empty
provider case does not establish populated string-pool/stream composition.

`local/factory_selection_aw/attempt01/manifest_before.json` seals 119 physical
artifacts before first execution, including 28 linked objects verified equal
to current objects and archive members, their sources and BSP header closure,
the archive, executable, native bytes, installed PE copy and driver. All hashes
remained unchanged. The relink driver accepts `--repo <built repo>` and
`--attempt <new directory>`; attempts and any failures cannot be overwritten.

Native CRT/FH3/SEH exception identity, concurrent mutation, arbitrary stack
aliases, complete startup mounting and gameplay remain unvalidated. BE1890 and
BE1740 belong to the parent's current mount packet; broader MPKG BB9D90 and
MPAK BB83A0 native factory reconstruction remains separate. No such target is
silently treated as a declining factory by this selector.
