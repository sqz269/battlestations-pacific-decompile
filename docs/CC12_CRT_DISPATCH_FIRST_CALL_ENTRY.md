# CRT dispatcher first-call entry: `00C0522E`

The accepted dispatcher calls numeric address `00C0522E` at `00C07997`.
The admitted first instruction is the complete one-byte unit `56`, **PUSH ESI**.
It subtracts four from the current 32-bit stack pointer and stores the current
ESI at the new top of stack, if the instruction completes normally. Nothing
beyond this first instruction has been reconstructed in this packet.

## Frozen inputs and authorization

The worker is frozen at commit `40ba8f8bcfd8aa77e81699c63c48734ac7eb11d7`,
root tree `0cdc9bb917a3d6789d70009c123c8e9df0b94de0`. Its eight finite Git
inputs are the `CC12_APPLICATION_CRT_DISPATCH_BODY_LISTING_METADATA` and
`CC12_APPLICATION_CRT_DISPATCH_BODY_PRIMARY` doc/report pairs, `config/target.json`,
`tools/bsp.py`, `tools/ghidra_export.py`, and `tools/ghidra_typed_flow.py`.
Actual commit, root tree, four path-parent trees, and eight blobs are retained.
The `src` and `include` subtrees are identified only; their contents were not read.

Root independently accepted the paired exact-entry metadata before admitting
two exact one-byte reads, context-zero saved text, and the first four prototype
metadata lines. The complete prototype stdout is retained; its 153-byte suffix
is opaque. Root's producer, receipt, and original metadata preimages are copied
under distinct `Root_preimage_` names. Root admitted no body expansion.

## Saved metadata and observed instruction

Both opening captures and the closing capture identify the same saved project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 32-bit language,
modification number 98, and 64,730 functions. Each complete raw typed response is
2,141 bytes with SHA-256
`2a701d8a97e1a39712fd14fa0835bcb7f47103719330e92e6d0d029d360ff794`.

All four exact/containing instruction and code-unit aliases identify
`InstructionDB` at `00C0522E`, length one. Function-at and function-containing
both refer to the saved library name `__getptd`. Its complete saved owner range
is `00C0522E..00C05245`, 24 bytes; it is marked non-thunk and not no-return.
These are saved metadata, including the saved `_ptiddata __cdecl __getptd(void)`
prototype. They do not authenticate the named service or an original ABI.

The unit has override `NONE`; default and effective flow are `FALL_THROUGH` to
numeric address `00C0522F`, with no targets and no overridden fallthrough.
That next address was not queried. The other **23 bytes** of the owner remain
unread. Paired complete CLI byte results agree on `56`; context-zero saved text
reports `PUSH ESI` with empty neighbors. Independent manual opcode decoding and
an actual detailed Capstone 5.0.7 decode agree on a four-byte ESI operand, reads
of ESP/ESI, an ESP write, and no flags modification. The pure replay checks the
retained Capstone result without loading Capstone.

## Conditional composition with the accepted caller

Only the retained five-byte `E8 92 D8 FF FF` at `00C07997` is decoded from the
accepted 228-byte dispatcher image. Signed rel32 is `-10094`; adding it to
`00C0799C` gives numeric target `00C0522E`. No new caller or target-body query
is used.

Use the accepted caller's current-state symbols: dispatcher entry ESP is `S`,
entry ESI is `I`, and the established frame value is `H=S-4`. Conditional on the
accepted prefix and these two instructions completing normally:

| Point | Current ESP | Relevant write |
| --- | --- | --- |
| Before the retained direct CALL | `S-0x10` | Accepted prefix state |
| At `00C0522E` | `S-0x14` | `[S-0x14]=00C0799C` |
| After the admitted PUSH ESI | `S-0x18` | `[S-0x18]=I` |

For this single entry unit, ESI remains `I`, EBP remains `H`, and flags retain
their incoming values. Those values need not be known. The return address
written by CALL is now at `[ESP+4]`. This current stack composition proves no
later pop, return, stack balance, frame preservation, object layout, allocator,
thread-local service, or returned pointer.

The accepted caller's eventual return symbols remain `ESP=Q0`, `EBP=F0`,
`EAX=P`, and memory `M0`, with other returned registers, flags and cleanup
unknown. This packet adds no return guarantee.

## Evidence closure and limits

`reports/cc12_crt_dispatch_first_call_entry.json` records the bounded findings
and pins. Complete Native CLI command/stdout/stderr records, raw typed and
context responses, Root preimages, actual manual/Capstone producers and
results, and physical replay/guard helpers are retained under
`local/cc12_crt_dispatch_first_call_entry/`.

The finite ZIP carries exactly eight Git input blobs and exactly these two
owned outputs. Its pure helper verifies actual Git object/path membership,
all member lengths/hashes, complete unit metadata, admitted byte/text/prototype
scope, independent decode agreement, and the conditional stack arithmetic.
After bootstrap imports and physical helper read/hash/compile, the strict
guard permits one ZIP read, one execution of that pinned helper, and one
exclusive receipt write; all other audit events are denied. A distinct copied
ZIP and physical helper/guard copies receive a separate guarded replay and
receipt. Receipt and archive hashes stay outside the ZIP to avoid circular pins.
The first guarded attempt denied a lazy ZIP filename-codec import. Its full
archive, producer/report preimages, and failure streams are retained. The codec
is now preloaded during bootstrap; the admitted audit events remain unchanged.

This is read-only saved-analysis and offline evidence. There are no source,
ledger, or Ghidra changes; no original executable reads; no build, tests, probes,
native invocation, game validation, or ABI compatibility claim. OriginalFH3
remains unauthenticated and unusable for invocation.
