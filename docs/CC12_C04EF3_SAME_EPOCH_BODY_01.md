# CC12: complete saved body at `00C04EF3` in one capture epoch

The complete eight-byte saved owner `00C04EF3..00C04EFA` is physically accounted
for by one paired capture in one bracketed Ghidra modification epoch. Under the
stated 32-bit decoding condition, its bytes encode a seven-byte dword AND with
zero and a one-byte near return. Root accepted this saved-body result after
reviewing the independent full-body manual and the first detailed decoder run.
Actual CPU mode, mapping, execution, memory aliasing and calling convention
remain unverified.

| Listing coordinate | Bytes | Conditional decode |
| --- | --- | --- |
| `00C04EF3` | `83 25 A8 EE 09 01 00` | `and dword ptr [0109EEA8], 0` |
| `00C04EFA` | `C3` | `ret` |

`00C04EFB` is only the exclusive end of this saved byte range. It was not
queried and is not a RET fallthrough or a proven execution successor. The
machine-readable record is
[`cc12_c04ef3_same_epoch_body_01.json`](../reports/cc12_c04ef3_same_epoch_body_01.json).

## Frozen baseline and finite lineage

The packet froze actual Main, worker HEAD and separately observed `origin/main`
at `900e0e1d5e45ec27721b8a0fd1fdf9f555231005`. The ten core inputs were the
accepted tail, preceding entry-byte and entry-epoch doc/report pairs, target
configuration and three CLI sources. Two additional Root batch doc/report inputs
were bound separately. Twelve working/LF/index/commit bindings and eighteen
distinct actual Git objects were verified. Source was identified only by tree
IDs: `src=5b4c4ae8359835b5fd9e56b0e9fd4dc839288a79` and
`include=e5c174553b669e588381310b9c8cf9a418f9f392`. No Source body or absence
census was opened. Root's later observation of
`e03bdd4215ba2dbe2183fde5bb8ee6ee9f81a8ec` is separate from this frozen baseline.

The historical selection contains exactly 206 original files: 29 named tail
primaries, 146 files already named by the accepted tail capture manifest and 31
named Root originals. It retains the earlier first failures, corrections and
inspection incidents in their original scopes. It does not recursively expand
the earlier 617-file worker or 450-file Root archives. Historical helper sources
were retained as data and were not rerun.

The accepted historical `00BF6707` direct-call bytes `E8 E7 E7 00 00` have a
signed displacement of 59367, giving listing continuation `00BF670C` and listing
target `00C04EF3`. That edge was not recaptured here. Earlier entry-only and
tail-only captures remain historical evidence. The present eight-byte result
comes from a fresh complete pair and does not splice those earlier captures.

## Capture and first decoder evidence

Root launched the refined capture driver exactly once under its bounded
cooperative window. It performed 12 Native commands and received 52 original
HTTP responses: 20 project checks, 20 program-info checks, eight complete typed
payloads, two eight-byte reads and two counts. There were 24 CLI observations
and 72 physical reads of the three pinned CLI sources. The opening and closing
live function counts were both 65174; the snapshot count remained 65149. Both
count outputs said `live function count 65174; snapshot 65149; changed`.

The eight complete typed payloads remained at modification epoch `10371`.
The program and count observations establish function count 65174; program-info
payloads do not supply a modification epoch. Opening and closing ENTRY and TAIL
pairs were checked in full;
comparisons with accepted historical records excluded only the two top-level
modification-number fields. Saved body ownership is exactly eight contiguous
addresses, with a unique seven-byte instruction at the entry and a unique
one-byte instruction at the tail. The four listing aliases agree with each
corresponding instruction record. A physical prebyte gate was saved before the
first byte read. The two reads then agreed on exactly these eight bytes:

```text
8325a8ee090100c3
SHA-256 575916c4179889b86e93e4583f8ab1f1b5c5e333c9e773f03932fb1b8beb65e9
```

This is a bracketed Ghidra evidence capture under a cooperative window; the GUI,
analysis and other harnesses were not globally frozen. No target, provider,
stack-slot, successor-byte or per-address CPU-context query was added.

Before Native capture, Root found that the original transport inherited
urllib redirect following. That finding produced separately named refined
capture, driver and transport sources. The refined transport rejects redirects,
retains their response through HTTPError, and permits no retry or fallback. The
original sources, 519-input seal and readiness remain unchanged and unexecuted.
The refined capture bound 532 finite inputs. This was a source-review correction,
not a failed Native attempt.

The independent full-body manual was physically written and pinned at
`2026-10-11T21:20:45.941897Z`, before the new decoder source was authored and
before its first import. Root read the manual and both complete decoder sources
before launching the decoder once. Capstone 5.0.7 in detailed 32-bit mode decoded
exactly two contiguous instructions of lengths seven and one. All thirteen
comparisons passed, the first process exited zero with empty stderr, and all
seven prepinned input files remained unchanged. Root's independent first audit
also passed. Capstone's generic AND flag metadata is kept separate from the
manual's zero-result specialization, and its default ESP model does not prove
the live stack-address size.

The preserved capture manifest still says `CAPTURED_PENDING_MANUAL`, and the
predecoder manual still says final admission awaits Root. Those are immutable
phase records. The final report separately records Root's accepted complete
saved-body result and its conditions; it does not rewrite the earlier records.

## Conditional instruction and composed effects

The interpretation assumes non-64-bit x86 with default 32-bit operand and address
sizes, consistent with the saved `x86:LE:32:default` program language. Neither
instruction has an operand- or address-size prefix. The actual execution mode,
segment descriptors, privilege, protection and paging state were not queried.

For `83 /4`, ModRM `25` selects a memory operand with a 32-bit absolute
displacement and no base, index or SIB. Little-endian displacement bytes
`A8 EE 09 01` give offset `0109EEA8`; the default segment is DS. The encoded
immediate byte `00` sign-extends to a dword zero. On ordinary normal completion,
the processor reads the original dword and writes its AND-with-zero result back.
The read remains part of this instruction's read/modify/write semantics. There
is no LOCK prefix, and no atomicity guarantee is established.

The result is zero, giving CF=0, OF=0, SF=0, ZF=1 and PF=1; AF is undefined.
There is no GPR data operand or stack-pointer change in the AND. `DS:0109EEA8`
is an effective offset, not an authenticated runtime linear or physical address,
provider identity or recovered global symbol.

`C3` is a near return with a four-byte return offset under the stated operand
size. It has no explicit operand, ModRM, displacement, immediate or extra stack
cleanup. With SS.B=1 it reads at SS:ESP and advances ESP by four modulo 2^32.
With SS.B=0 it uses SS:SP and advances SP modulo 2^16; full ESP+4 does not follow
from the operand size alone. It loads the popped CPU return offset after
applicable checks without popping CS, preserves the arithmetic flags, and does
not automatically restore EBP or other callee-saved GPRs. No explicit return-value
assignment or high-level calling convention is established.

If these two encodings are actually fetched in order and both complete normally,
the ordinary sequence is the DS dword read/zero write followed by the stack
return-offset read and near return. The AND's stated arithmetic flags survive
the RET. A later RET or fetch fault does not establish rollback of a completed
AND; this is not a function-level transaction model. Segmentation, paging,
alignment, stack, destination-fetch and control-protection conditions can
prevent normal completion and were not observed.

Listing coordinates are not authenticated CPU EIP offsets. A numerical CPU EIP
claim additionally needs a mapping premise, for example that listing addresses
equal runtime linear addresses and CS.base is zero. Neither premise was checked.
The popped return is an unobserved CPU offset; even a zero offset would not prove
linear destination zero or successful transfer. Returning to CPU EIP `00BF670C`
would additionally require the actual post-write return slot to contain that
offset. The historical incoming edge alone does not establish it.

## Stack and instruction-memory alias conditions

For ordinary coherent byte-addressed memory with stable translations/backing and
no intervening writes, let `W_j` be each of the four backing bytes written by the
AND and `R_i` each backing byte of the subsequent four-byte stack read. Each
return byte becomes zero if its `R_i` aliases any `W_j`; otherwise its prior value
is retained under this model. Disjoint backing preserves the return-slot bytes;
partial overlap zeros only the aliased bytes; complete coverage zeros the popped
offset. None of these cases was observed. Different segment offsets or different
linear addresses do not prove distinct backing; MMIO/device semantics are outside
this ordinary-memory model.

The write may also alias instruction-fetch backing, including the later RET.
Runtime mapping, permissions, fetch timing and self-modifying-code visibility
were not observed. The composition therefore explicitly assumes the processor
actually executes the two captured encodings; decoding saved bytes alone does
not prove that a later runtime fetch still obtains `C3`.

## Saved flow, incidents and boundary

The complete raw default and effective flow fields are retained independently.
ENTRY has `FALL_THROUGH`, no flow override and both saved fallthroughs at
`00C04EFA`. TAIL has `TERMINATOR`, no flow override, no fallthrough and no targets.
All eight typed comparisons agree with their relevant manual flow interpretation.
The tail's saved `is_computed=false` does not authenticate a static return target,
and the saved function name does not establish its purpose.

Worker preparation had no inspection-process or producer failure. One
post-capture display truncation occurred while displaying Root's large receipt;
one later PowerShell table abbreviated the schema-key display. Compact follow-up
reads resolved both display limitations without changing originals or rerunning
Native or the decoder. These remain distinct from Root's inspection incidents,
the redirect source-review finding and historical failure scopes. Publication
execution records and the finite manifest retain their exact first results.

This packet changes only its documentation and evidence report. It performs no
additional Native operation after capture, decoder rerun, Source-body read,
Source-absence census, C++ change, build, test, probe, runtime check, GPR mutation
or guarded final-proof helper. The complete saved eight-byte owner is accepted
under the explicit mode and mapping conditions. ABI compatibility, runtime
execution, gameplay behavior and an authenticated function purpose remain open.
