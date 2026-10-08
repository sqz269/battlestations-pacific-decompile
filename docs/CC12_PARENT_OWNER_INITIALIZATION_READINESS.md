# Parent owner initialization readiness (CC12)

This read-only audit closes the physical owner-construction witness at
`004C B030`. It awards **Source 0 / ready packets 0**. The native constructor
places the category 1 root at parent `+0x24` in a genuine allocation and array
construction phase. Its callback, iterator, sentinel, exception cleanup, and
owning class lifetime still need independent admission. No Source, fixture,
build, runtime execution, or Ghidra mutation occurred.

The baseline is `7c79302e7276263fe41dc079c589920336c7a918`. Exactly one whole native function,
one bounded direct caller, one caller zero origin, and one literal phase DATA
word were qualified. The graph contains 12 nodes and 11 edges (23 of 24).

## Complete native body and physical ABI

The complete body is `[004C B030, 004C B0AE)`: **126 bytes, 39 instructions**.
SHA-256: `2294a34491aee760a23d8f05b80e88bc1cbcab4445b2a85978d1ad7265ac6028`. All installed PE, live listing, byte
ranges, instruction starts, and before/after bytes agree. Each live batch
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and the installed
PE hash. The full byte sequence and instruction rows are in the report and
sealed `candidate_physical.json`.

Entry ECX is the actual storage root; there are no stack arguments. Incoming
EDX is unused as a formal. Saved ESI retains the root and supplies full EAX on
ordinary return. The exit at `004C B0AD` is plain RET. EBP, ESI, and EDI are saved
and restored; EBX is not directly changed. Their backend preservation requires
the actual ordinary callee contracts. Outgoing ECX holds the previous FS
exception-chain head; EDX is volatile.

For entry ESP S, the exception frame and saved registers reach S-28. The five
iterator arguments reach S-48; the iterator must consume 20 bytes. The sentinel
call has no stacked arguments. Final ADD ESP, 0x10 changes S-16 to S, followed by
RET to S+4. Arithmetic flags come from this final ADD; no runtime flags or blanket
floating-point/DF contract is claimed across the unexpanded backends.

The constructor installs a real FS exception frame using handler `00C6 5651`,
uses state 0 before the iterator and state 1 before sentinel construction, and
restores the previous FS head on ordinary exit. Exceptional paths and compiler
cleanup remain unclosed. These are named Astra/compiler-EH prerequisites.

The only CALLs are at `004C B076` to `00BF 7CD1`, operand offsets `[71,75)`, and
`004C B088` to `004C 3080`, operand offsets `[89,93)`. No callee body was expanded.

## Genuine storage and category root phase

The actual caller window `[004D E651, 004D E6A1)` is **80 bytes, 23 instructions**,
SHA-256 `92dd57a6b97084101200e8f4c912816679252dbc66fbb59f0097e25ee41b2951`. It belongs to native
caller `004D E610`; this is a witness window, not recovery of the whole caller.

It pushes allocation size `0x4BC` at `004D E651`, calls original allocator
`00BF 55BE` at `004D E656`, and retains the same EAX allocation in EDI. EBX is
zeroed at `[004D E634, 004D E636)`. The caller passes EDI, zero, and `0x4BC` to
zero-fill `00BF 79F0` at `004D E664`. Its ADD ESP, 0x10 also removes the original
allocation-size argument. This establishes the normal fresh non-null phase;
zero-fill precedes the null test, so no null safety is inferred.

MOV ECX, EDI at `004D E67B` precedes the genuine constructor CALL at `004D E67D`.
There is no stacked constructor argument. Returned EAX is published to the
caller's actual owner at ESI `+0x19CC` at `004D E696`, before the post-constructor
CALL `0090 37F0` at `004D E69C` with the same root and two stack values 1.

The constructor stores literal phase ID `00CE 7784` at root `+0`, zeros DWORDs
`+0x0C/+0x10/+0x14`, then passes this exact array descriptor to `00BF 7CD1`:

| Field | Actual value |
| --- | --- |
| Destination | parent `+0x18` |
| Element stride | `0x0C` |
| Element count | `0x61` (97) |
| Exclusive end | parent `+0x4A4` |
| Constructor callback | `004B 7EC0`, PUSH at `004C B065` |
| Destructor callback | `004C 2D30`, PUSH at `004C B060` |
| Category 1 root | parent `+0x24`, covering `+0x24..+0x2F` |

The push order establishes callback roles. Historical `GAME_WORLD_CONSTRUCT`
comment/table evidence reverses those roles; its frozen association is retained,
and the actual instructions govern this audit. No saved comment was mutated.
The category root has genuine pre-zeroed storage and a real array construction
descriptor. Its callback instructions, invocation, node ownership, and completed
lifetime are not admitted merely from that layout.

The actual `004C 3080` sentinel result is stored at `+0x4B4`, size `+0x4B8` is
zeroed, and byte `+0x4AC` becomes 1 at the end. The constructor does not directly
write `+4/+8`, `+0x4A4/+0x4A8`, or `+0x4B0`; backend side effects remain qualified.
The later header constructor and registration phase remain separate. This body
does not publish any subject `+0x30` link or register a category 1 subject.

Actual four-byte DATA at `00CE 7784` is `B0 B0 4C 00` (DWORD `004C B0B0`), SHA-256
`a844dc466308c491ccef3dfce8236bae3329f506274a5e8a429123ed832a7a0b`. This is literal phase/DATA identity;
no Source vtable, dispatch, deleting destructor, or owning class was supplied.

## Remaining admission boundaries

Existing `construct_world_object_004cb030` returns a typed `WorldObjectLayout`
through a new C++ interface. That semantic Source does not establish this native
ECX/root/EAX/FS/callback/sentinel ABI. Accepted append, erase, and removal Source
can operate on their genuine current-canonical coherent 12-byte nodes and
borrowed payloads. They do not bind the native owner callbacks, array destructor,
class deletion, sentinel, private heap, or subject observer lifetime.

The next separately authorized boundary is whole callback `004B 7EC0`, with one
actual iterator-callback invocation witness. **It was not queried in this audit.**
The iterator `00BF 7CD1`, handler `00C6 5651`, destructor callback `004C 2D30`,
sentinel constructor `004C 3080`, deleting-owner phase `004C B0B0`, original
allocation/zero backends, and later header `0090 37F0` remain named incomplete
dependencies. Root alone may register an independently ready primitive.

## Preserved evidence and seal

All 9,410 prior artifact pins, 30 consumed input pins, and 22 frozen historical
files matched before and after. Accepted Source/Original families were preserved
without recipe or execution replay. Root's accepted removal primary seal
`local/rm46p1/seal.json` remains
`5a0f371ed1bbf9efd77bd0021ece880ac1b486dd9c0ba69429aba1f8ea130428`
(114 listed artifacts plus seal). Old path/hash metadata remains a historical
association, without repinning it to live metadata.

The first attempted caller read at `004D E64C` was rejected because that address
lies inside MOV at `004D E64A`. The unchanged failed helper and query artifacts
are included. A new stage read the first valid PUSH at `004D E651`; no listing
repair or successful query replay occurred.

The ignored immutable family is `local/po24`: 209 listed
artifacts plus the two seal files, 211 actual
files. Exact-root exclusions are only the receipt and manifest themselves;
frozen nested receipts/manifests and failure artifacts are included.

| Sealed file | SHA-256 |
| --- | --- |
| `proposal.json` | `568ac6c757f4db3c570c39acb7b9d017945672fa7de1d9581214b9ae3afe6f8d` |
| `readiness_receipt.json` | `12513e149ee97938b80bc67567c3b9dcb2589907256c394dd85a639ebd1248cd` |
| `artifact_manifest.json` | `035ab27532440944341ec2659f13b93e7ebc2e26a22c23180b516cfb1729ebe6` |

Postprocessing was local only. Native owning parent/subject/game closure,
startup, and gameplay remain unvalidated.
