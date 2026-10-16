# Original `_strchr` interior entry readiness

This readiness-only packet closes the native provider boundary at `00BF86F6`
for the incoming state already established by the whole `_strstr` review.
Root and independent review accept the whole provider contract and the concrete
ordinary `_strstr` Source plan below. The separate authorization is recorded in
`reports/cc12_native_crt_strstr_readiness.json`; compiler and Main acceptance
remain pending, and new Source credit is zero.
The existing `_strchr` reconstruction and its credit are unchanged.

## Exact boundary and evidence

Fresh typed metadata identifies `00BF86F6` as a one-byte InstructionDB start,
opcode `53`, `PUSH EBX`. Its containing function is the correctly named
`_strchr` at `00BF86F0`; no separate function starts at the interior address.
Instruction-at and instruction-containing agree. No alternative decoding,
instruction creation, function creation, annotation, or listing repair was used.

| Saved body range, inclusive | Bytes | Instructions |
|---|---:|---:|
| `00BF86E0` through `00BF86E4` | 5 | 3 |
| `00BF86F0` through `00BF87AD` | 190 | 86 |
| Complete existing body | 195 | 89 |

All 195 selected bytes equal live Ghidra and the installed PE. Independent
x86 decoding agrees with all 89 saved instruction texts, starts, lengths,
owners, and default/effective flow targets and fallthroughs. Every flow
override is NONE. All non-return control transfers stay within these saved
starts; there are no native call instructions or absolute-memory operands.

The eleven bytes from `00BF86E5` through `00BF86EF` were not selected, captured
as a target window, or decoded, and receive zero instruction credit. Reading
the full installed PE for its hash, headers, and disk mapping is separate.
Concatenating only the two selected ranges gives SHA-256
`22f27502570ab4aa937590c5aebf8c378db90379f04e561945d1db27bff36e8e`.

## Interior native inputs and binding

Let S be ESP on arrival at the interior entry, H the DWORD in SS:[S+4], and
b a BYTE represented by the full incoming EAX as zero-extended b. The general
contract below admits b=0 through 255; the established `_strstr` tail supplies
nonzero b=needle[0], after observing needle[1]=0.

| Incoming state | Actual use |
|---|---|
| EAX = zero-extended b | Required full-register preparation; arbitrary upper EAX bits are not admitted by this contract. |
| SS:[S+4] = H | Source pointer DWORD, loaded after saving EBX. |
| SS:[S] | Existing return address, consumed by the eventual plain RET. |
| SS:[S+8] | Not read by this interior path. It may still contain the needle pointer. |
| ECX and EDX | No incoming scan-pointer or data input; EDX is overwritten from the source home and ECX is used as scratch. |
| EBX, ESI, EDI, EBP | Values preserved on normal return; only registers actually needed by a path are saved. |
| Incoming arithmetic flags | Not consumed as a condition; subsequent instructions establish the flags used by branches. |
| DF, ES | Neither controls an instruction in this body; both are unchanged. |
| DS, SS | DS addresses explicit payload reads; SS addresses the argument home and implicit stack accesses. Neither segment register is written. |

The first four instructions from the interior start are, in order: save EBX
at SS:[S-4], copy the full EAX into EBX, shift EAX left eight, and load EDX
from SS:[ESP+8], which is SS:[S+4] after that save. Thus H becomes the cursor.
The incoming ECX needle pointer and incoming EDX partial-register state are
irrelevant to the provider's scan. No further needle byte or second argument
home is read. The saved return address remains the return destination.

This differs from entering at `00BF86F0`: the skipped six bytes zero EAX and
then read AL from SS:[S+8]. Redirecting the numeric tail to the normal entry
would read the low address byte of the needle pointer, not needle[0].

## Complete payload schedule and control

There are exactly three explicit payload read sites: BYTE at `00BF8708`,
aligned DWORD at `00BF8728`, and conditional DWORD reread at `00BF876A`.
There are no payload writes, WORD reads, string instructions, REP, SSE, x87,
globals, tables, locale/TLS, allocation, callbacks, or outgoing providers.

While the retained EDX cursor is not four-byte aligned, read one BYTE into CL,
advance EDX by one, compare that captured BYTE with BL, and only after a
mismatch test it for zero. A match reaches the saved pre-entry tail, which
forms EAX=EDX-1, restores EBX, and returns. A nonmatching zero restores EBX
and returns EAX=0. Reaching alignment proceeds to the DWORD loop. This path
does not access a DWORD merely because the eventual word loop exists.

At alignment, the two copies of b in EBX/EAX are combined; EDI is saved,
EBX is shifted/combined into four repeated bytes, and ESI is saved. With
M=0x7EFEFEFF and K=0x81010100, each iteration performs this exact schedule:

1. Read W as one aligned DWORD at the retained cursor. Form D=W XOR repeated_b,
   T=W+M, and U=D+M using DWORD arithmetic.
2. Form the candidate expression C=(~D XOR U) AND K and the zero expression
   Z=(~W XOR T) AND K. EDX advances by four before either candidate branch.
3. If C is nonzero, read a second DWORD W2 at EDX-4. This is a real reread,
   not reuse of W and not four new BYTE loads. Inspect the captured W2 lanes
   in increasing address order, testing equality to b before zero for each
   lane. The original tests AL, AH, shifts by sixteen, then tests AL, AH.
   A match returns that lane address; a nonmatching zero returns zero.
   If all four lanes are nonzero/nonmatching, repeat the DWORD loop.
4. If C is zero, Z=0 repeats. Otherwise a nonzero Z AND 0x01010100 returns
   zero. When that subset is zero, T AND 0x80000000 controls compensation:
   nonzero repeats and zero returns zero. Preserve both masks and this branch.

Step 2 describes the resulting expressions: in the actual instruction order,
the unmasked expressions are formed, EDX advances, C is masked/tested, and
Z is masked/tested only on the no-candidate path. No condition consumes an
incoming flag. The reread is executed only on the candidate path.

## Returns and private stack protocol

The unaligned path saves/restores only EBX. The aligned path additionally
saves EDI then ESI, and restores ESI then EDI then EBX. Aligned match returns
form EAX=EDX-4, -3, -2, or -1 for lanes zero through three. The lane-three
return restores ESI and EDI before its LEA; the other aligned match returns
form EAX before those restores. All are ordinary plain RET instructions:
ESP returns to S before RET and becomes S+4 afterward. Neither argument home
is cleaned by the provider. There is no new return address pushed by this
tail path and no attempt to return to the `_strstr` body.

EBP is untouched. EBX/ESI/EDI recover their incoming values; ECX and EDX are
scratch outputs. A successful match leaves arithmetic flags from the equal
BYTE comparison: CF=OF=SF=0, ZF=PF=1, AF=0. Failure's final XOR gives
CF=OF=SF=0, ZF=PF=1, with AF undefined. DF and the segment registers remain
unchanged. No physical ABI, frame-alias, exception, or fault equivalence is
claimed for a future C++ interface by this static native contract.

## Ordinary domain for the whole `_strstr` recommendation

Use stable live ordinary storage with the usual flat DS/SS interpretation.
Permit raw unaligned/interior addresses, high-bit bytes, read-only payload,
overlap between source and needle, and ordinary caller-owned stack buffers.
No ownership transfer, nonoverlap rule, SDK string object, complete unused
terminator extent, or manufactured padding is required.

Require every actually reached BYTE or entire DWORD read to be valid and
determinate. On the one-byte-needle route, an aligned DWORD can include bytes
after a NUL or match, so those actually read bytes must genuinely exist and
be readable. A candidate requires the actual second DWORD read at the same
address. This is a path-specific reached-access obligation, not a blanket
padding requirement on every input. An unaligned early return requires only
its reached BYTE accesses and advances.

Preserve nonwrapping 32-bit address geometry within the actual backing
allocation or one-past; one-past is never dereferenced. The provider advances
by one after every alignment BYTE and by four after each first DWORD, even
on the iteration that stops. The original instructions themselves use
wrapping DWORD arithmetic; this ordinary domain admits the nonwrapping
subset and does not replace the native arithmetic with a signed cap.

The accepted prior whole `_strstr` evidence supplies the other paths. Empty
needle returns the supplied source value, including null, without a source
payload read or source advance; it does not require needle[1]. A nonempty
needle reads needle[1] before route selection. Longer-needle comparisons
retain the cached first two bytes and the same-byte retry after a second-byte
mismatch. They test each reached extension needle byte for zero before the
corresponding source byte. The first extension source load is followed by
ESI+2 before comparison, even when it mismatches or the next needle byte
terminates; the later partner load precedes ECX+2 and comparison. Those
actual advances require genuine within-allocation/one-past geometry, without
inventing readability for a skipped partner byte. Historical candidate
identity and every local success/failure return must remain intact.

Exclude alias protocols involving Original or proposed implementation-private
argument homes, saved registers, return addresses, scratch, or call frames.
Any proposed binding/metadata storage must be disjoint from reached payload.
This does not exclude ordinary caller stack buffers or source/needle overlap.
Exclude MMIO, concurrent modification, invalid lifetime/mapping, observable
fault/partial-progress behavior, and private exception machinery. The Original
bodies impose no DF-dependent traversal or ES requirement; future emitted
Source still needs its own compiler/ABI proof under its declared ordinary
calling environment. Result identity is borrowed storage/raw null, with no
new ownership, allocation, or error service.

## Existing Source reuse and remaining gate

The current Source/header hashes match the earlier report, and their entire
algorithm was reviewed against the fresh native body. The existing public
helper accepts actual two-word parameter backing: it reads the low BYTE at
binding+4 first, then the pointer DWORD at binding+0. Its BYTE alignment loop,
first DWORD, candidate reread, lane order, masks, compensation, and result
addresses match the contract above. Its binding reads are metadata operations,
not the interior native register interface.

The Source computes the candidate mask before its cursor increment and forms
the zero expression only on the no-candidate path. The native body forms both
unmasked expressions before advancing, then masks/tests them as described
above. These pure unsigned computations preserve the ordinary reached-read
and result behavior; no native arithmetic-instruction or flag-order identity
is inferred from the Source.

Passing the original `_strstr` argument homes to that helper would again use
the low byte of the needle pointer. Possible ordinary reuse instead needs
genuine stable private backing containing H and the prepared b, disjoint from
reached payload, plus review of the resulting whole implementation. No such
adapter or new API is implemented or authorized here. Existing normal-entry
build, object, and fixture receipts remain historical evidence; none was
rerun or relabelled as proof of this interior-entry composition.

The native dependency and listing blockers are resolved for the stated tail
state and domain. Root may accept the positive whole `_strstr` readiness
recommendation and separately authorize a concrete Source packet. That packet
must preserve all local branches and the genuine provider algorithm, then
establish its complete emitted helper/dependency/access/return behavior under
the applicable Source gates. This packet provides no compiler, build, test,
probe, emulation, execution, physical replacement ABI, caller, or game credit.

## Evidence boundaries and preservation

Base: `a29bdee6b64409d277e51a492680945a7286f711`. The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Existing Source, Root-published `_strstr` reports, author 91-file evidence, and
the earlier 30-file independent review are external inputs. Root's scoped
existing-index lookup receipt `476d94` is external ledger routing only.
No caller body or wider dependency was read.

The accepted metadata, body, and full-flow batches have internally stable
counters 6878, 6881, and 6884 respectively. They do not establish global or
cross-batch database immutability. One full-flow attempt failed while Windows
denied the CLI's final atomic rename; its pending file, accepted temporary
file, and failure receipt are retained. The identical supported query into
fresh paths succeeded; no temporary file was substituted for a successful
CLI result. Two preparation failures and a capped reader output are likewise
retained and qualified. No failed or unseen output supplies a proof claim.

The new immutable family is
`local/NativeCrtStrchrInteriorReadiness01`, including fresh before-pins,
captures, independent verification, publication snapshots, preservation
receipts, and authenticated raw records for this task. Its seal and subsequent
independent verification are separate receipts. All prior evidence remains
unchanged. See `reports/native_crt_strchr_interior_readiness.json` for the
structured recommendation and evidence references.
