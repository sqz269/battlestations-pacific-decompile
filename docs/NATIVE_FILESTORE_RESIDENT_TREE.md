# Native FileStore resident-tree erase and clear

Addresses: `00BE4920`, `00BE4940`, `00BE4980`, `00BE50E0`, `00BE6760`,
`00BE7690`, `00BE7A30`, `00BE72C0`, `00BE7BB0`.

The nine actual-storage routines in `src/native_filestore_resident_tree.cpp`
cover complete logical bodies in the explicit-service source domain. Names are
descriptive hypotheses. [The report](../reports/native_filestore_resident_tree.json)
records original bytes, every direct CALL, callers, raw boundaries and validation.
The packet adds no generic container/library implementation or alternate owner.

| Entry / inclusive logical end | Bytes | Coverage | Original ABI |
| --- | ---: | --- | --- |
| BE4920..BE493B | 28 | complete | ECX node; EAX maximum; RET |
| BE4940..BE495A | 27 | complete | ECX node; EAX minimum; RET |
| BE4980..BE49D1 | 82 | complete | ECX tree; stack node; RET4; right rotation |
| BE50E0..BE512D | 78 | complete | ECX tree; stack node; RET4; left rotation |
| BE6760..BE6A12 | 691 | complete in stated exception domain | ECX tree; stack output, owner, node; EAX output; RET0C |
| BE7690..BE7758 | 201 | complete | ECX tree; stack output, first-owner/node, last-owner/node; EAX output; RET14 |
| BE7A30..BE7A63 | 52 | complete | ECX tree; native EAX0; RET |
| BE72C0..BE733B | 124 | complete | ECX actual provider; RET; no semantic return |
| BE7BB0..BE7BE3 | 52 | complete | ECX tree; native EAX0; RET |

The total is 1,335 bytes. Every span was re-read through the verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` wrapper and matched the
installed PE. Ghidra was read-only. BE6760's stored body ends BE69DC;
BE7A30 ends BE7A53; BE7BB0 ends BE7BD3. Their raw continuations respectively
BE69DD..BE6A12, BE7A54..BE7A63 and BE7BD4..BE7BE3 have no containing Ghidra
function. These spans are labelled continuations, not recovered new functions.

## Storage and established dependencies

BE55E0 allocates actual 1Ch nodes and writes links +0/+4/+8, color+18=1 and
nil+19=0. BE7FA0 writes the resident tree at provider+14: current head at
tree+4, count at +8, sentinel self-links and nil+19=1. BE63E0 produces the
payload+C and links; existing BE6250/BE5D70 establish its length/data/retained
owner words at node+C/+10/+14. The existing `NativeFileStoreNameIterator`
supplies the eight-byte owner/node layout. All these producers were checked
or reconciled against their existing declarations and foundation/subtree evidence.

The already established native VFS mount tree supplied a source implementation
pattern only. The FileStore listing independently establishes the 1Ch layout,
every rotation/transplant/rebalance branch, payload destruction and raw tails.
The source uses the existing BE4C30 iterator, BE5D70 payload destructor,
BE6720 subtree destructor, owning native exception transport, string storage,
stream-dispatch service and real shared CRT allocation/free.

## Erase, range and clear behavior

BE6760 captures the original node in EBX at BE67D3 and also stores it in its
stack local. It advances the by-value iterator at BE67DE before touching links.
For two real children, BE67FD reloads the advanced node into ECX; BE6806 reaches
BE686F..BE68C5. This successor transplant is real, despite the decompiler's
unreachable warning. It moves the successor's links, repairs parent/root links,
and swaps successor/original colors. Both symmetric black-node fixups use the
actual +18/+19 bytes, preserve the sentinel's parent, and update root/extrema.

BE69CE destroys the **original** node's payload+C. BE69D8 frees that original
node, with ADD ESP,4 in the raw continuation. Only after these potentially
observable calls does BE69DD load the current unsigned tree count; zero stays
zero, otherwise it decrements. It then publishes the advanced owner followed
by node and returns output with RET0C. A throwing retained-owner terminal leaves
the already-mutated topology and count/output uncommitted; the reused payload
guard releases its current string, but this routine does not free the node.
There is no input owner==destination-tree comparison.

The invalid-nil route assigns exactly 27 bytes of the iterator diagnostic,
arms state0 at BE67B2, constructs the existing logic-error owner, publishes
D6926C and invokes the original out-of-range throw metadata D863A8. FuncInfo
E015DC uses map E015D4 `{previous=-1, action=CC6D00}`; CC6D00 loads the
temporary at EBP-50 and tail-jumps 4072D0. Source reuses
`NativeHardwareLayoutInvalidIterator` plus the same completed-temporary cleanup.
It has new host RTTI and C++ exception ABI, not original FH3 identity.

BE7690 captures first owner/minimum, validates the owner, and checks the final
owner only when the first node equals that captured minimum. A full range
destroys the current root via BE6720, then reloads the head separately for root,
minimum and maximum reset, zeroes count, and returns `{tree,current-minimum}`.
The partial path repeatedly validates matching owners, advances the stored first
iterator before erasing its captured previous value, and returns its final pair.
Each returning invalid-parameter callback continues at the original next step.

BE7A30 and BE7BB0 are distinct 52-byte bodies differing only in CALL offsets.
Both call full-range erase, free the **current** sentinel, then set current
head/count to zero while leaving tree+0 unchanged. BE7BB0 is reached by the
provider's constructor state3 action CC6E60 and destructor state1 action
CC6DF8. Both actions load captured provider EBP-18, add14 and **tail-jump**;
the xref endpoint incorrectly labels their transfers as CALLs. The source's
two entry names share the established identical implementation.

BE72C0 loops while current provider+1C is nonzero, captures current resident
minimum, validates against the captured head, and reads retained owner+4.
When that count differs from1 it performs two more current-head validations,
captures the current reference count and name, substitutes the original shared
empty byte for a null name, then calls 4254B0 with three cdecl arguments
(ADD ESP,0C). That callee's complete body is RET: source preserves all observable
reads/validation without inventing a logging service. It erases the captured
minimum and rereads count. Pending storage at provider+20 is untouched.

## Verification and limits

`local/resident_av/build03.log` records the strict Win32 build and both existing
CTests passing after seed verification. All report CALL and tail-jump rows pass
the live mechanical checker. The reusable ignored driver
`local/resident_av/run_fixture.py --repo <built-checkout> --attempt <new-name>`
compiles and freezes each new run; it never modifies prior attempts.

Final `attempt02` passed 39 original/source states and 366 checks, including
deep/direct successor deletion, both balancing directions and near-nephew
rotations, full/partial/empty ranges, returning invalid validation, current-count
mutation to zero, ClearStore reference counts1/2 and null names, and both
destructor entries. Source-only checks cover throwing terminal cleanup and the
owning invalid-nil exception. `attempt01` remains immutable with its earlier
37-state/343-check result before the additional wrapper was requested.

Before execution, 1,038 files were physically frozen and marked read-only:
all linked archive members were compared with the actual current object files,
their C++ sources were copied, BSP headers were copied, and the library,
probe source/object/executable/map, compiler command, build log and verified
native bytes were sealed. Post-run hashing found all preexecution files unchanged.
The native fixture changes only listed direct CALL displacements. Original
tree branches remain intact; payload/iterator/subtree calls use their existing
source implementations, with real CRT string allocation/free and actual
InterlockedDecrement/callable terminal dispatch. Validation and diagnostic RET
are explicit fixture boundaries; accidental original EH entry terminates the
fixture. No unresolved production call is stubbed to obtain a link.

This establishes bounded original-tree/source comparison and focused ownership
composition. It does not independently retest original dependency bodies,
prove original static CRT/FH3/SEH, throwing lazy pool release, arbitrary stack
aliasing, concurrent mutation, ABI replacement or gameplay. The production
interfaces retain `NativeStringStorage::release`'s existing noexcept boundary.
No permanent tests or game-install changes were made; proposed Ghidra annotations
and body-tail repair remain the integrator's locked task.

## Correction from docs/NATIVE_FILESTORE_PROVIDER_LIFETIME.md

AV integration completed the proposed locked Ghidra annotations and required stored-body repairs, preserved prior comments, saved the project and refreshed affected exports. The combined Win32 build and both existing CTests passed. The fresh `local/native-av-worker-deliveries/resident/local/resident_av/integrated02` fixture passed 39 native/source comparisons. All physical inputs remained unchanged; actual linked objects matched archive members.

The current source/header/object gate and preserved worker attempts are recorded in `reports/native_av_integration.json`. Earlier worker-only annotation and provider-composition limitations above are historical; actual provider composition is now separately tested with one actual pool/raw manager domain. Original FH3/SEH, zero-reference provider stream terminal coverage and gameplay remain qualified.
