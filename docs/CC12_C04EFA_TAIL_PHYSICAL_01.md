# CC12 C04EFA conditional tail-byte record

The separately captured byte at listing address `00C04EFA` is `c3`. Under the
verified program's default 32-bit x86 operand-size interpretation, manual
analysis and one fresh detailed Capstone 5.0.7 decode agree that it encodes a
near `RET`. Actual per-address CPU context, stack state and runtime address
mapping remain unverified. This is a conditional physical record, without a
reconstructed C++ body, binary replacement, ABI claim or runtime validation.

The saved owner is the eight-address range `00C04EF3..00C04EFA`. Its first seven
bytes were physically captured in the preceding packet at count 65052 and
epoch 7572. This packet recaptured only the final byte, at count 65151 and
epoch 9433. Seven historical bytes plus one current byte do **not** establish a
single-epoch physical capture of the current eight-byte function body.

## Frozen inputs and provenance

The original finite freeze retained eight core files, two supplemental Root
files and sixteen distinct actual Git objects. Main and this worktree were
`3eb329e7e9844d3ed8a75f04bd576bf25e8217e5`; the separately observed origin/main
was `ab1be437ad532d34c751fa4b653c3a509ad0934d`. All ten working/LF, index and
commit bindings were checked. Source tree identities were recorded without
reading Source bodies: src `f2efed3eb4188625f71dd64fdd7090411bac58a2`, include
`8bd7c8a84efd0d38ea851bf38c972eb8e9a792bc`.

The core contains the accepted C04EF3 physical and ENTRY epoch-rebaseline
document/report pairs, target configuration and three CLI producers. The
supplemental pair is the accepted Root entry-body physical batch. Exactly 132
selected historical originals were copied and pinned, including the previous
manual, decoder, raw capture and four Root receipts. Earlier helper sources
were preserved as bytes, without executing them or recursively expanding their
referenced archives. The accepted incoming BF6707 edge is carried through the
accepted C04EF3 record; its old body archives were not reopened.

Later Root-reported Main, origin and Source tree identities appear separately
in the machine report. They do not replace the original freeze or attribute
its evidence to a later integration state.

## First original capture

Root reviewed the complete producer, driver and transport guard, then launched
one capture within `Root_window_01`. The original invocation, ready-file bytes,
authorization, HTTP responses, stdout, stderr and result are retained. The
window is cooperative; GUI analysis and other harnesses were not frozen.

The twelve Native commands comprised opening count, paired ENTRY metadata,
paired tail metadata, paired one-byte tail reads, paired closing tail metadata,
paired closing ENTRY metadata and closing count. They produced 52 HTTP
responses and 24 CLI observations with 72 physical CLI-file reads. Original
exit status was zero; stderr was empty. The live count remained 65151 and all
eight complete typed payloads used epoch 9433. The snapshot count printed in
the full count stdout was 65149, and is explicitly distinct from the live count.

All current ENTRY payloads equal the complete accepted ENTRY payload after
excluding only its two top-level modification-number fields. Tail controls
established the exact query, no new exact function entry at FA, the unchanged
complete EF3 owner record, four agreeing listing aliases, a unique FA
instruction start and an exact one-byte instruction wholly inside the owner.
The prebyte gate was physically written before either byte request. Both reads
returned `c3`, size 1, SHA-256
`ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e`.
Complete closing payloads equal the corresponding opening payloads. Neither
`00C04EFB` nor a target, provider, stack slot or CPU context was queried.

## Manual record and first detailed decoder

The complete manual was physically written and pinned at
`2026-10-11T20:31:57.000295+00:00`, before the fresh decoder source was authored
and before its first import. Its SHA-256 is
`efeb033c87d05558651b28fa3e1fa2ec19624bf6c3df31a1c6a42775f9f5666c`.
Root then fully read the manual and both decoder sources and launched the
decoder exactly once. The child wrote the before-import record before importing
Capstone and saved full detail before accepting the comparisons. Exit status
was zero, stderr empty, and all seven prepinned inputs remained unchanged.

The detailed 32-bit record reports RET id 633, opcode `c3`, no prefixes or
explicit operands, no encoded ModRM/SIB/displacement/immediate, zero encoding
offsets, ESP read/write in the decoder's default stack model, no EFLAGS effects,
and groups `ret` and `not64bitmode`. Numeric zero placeholder fields are not
extra encoded fields. All seven manual/decoder comparisons passed.

Under the stated operand-size condition and ordinary normal completion, the
instruction reads a four-byte little-endian return offset from the stack and
loads it into CPU EIP. SS.B=1 uses SS:ESP and advances full ESP by four modulo
2^32. SS.B=0 uses SS:SP and advances SP by four modulo 2^16; the four-byte
operand width does not prove a full-ESP-plus-four update. The saved decoder's
ESP model is not evidence of the live SS.B value. CS is not popped, EFLAGS are
unchanged by ordinary RET semantics, and the opcode encodes no automatic EBP or
callee-saved-register restoration. Stack access, code limit, alignment,
paging, fetch and any enabled control-protection checks remain conditional.

The popped value is a CPU return offset, not an authenticated listing address.
Using a numeric listing coordinate as EIP requires an additional mapping
condition, for example listing address equals runtime linear address and
CS.base is zero; neither premise was verified. The listing end-exclusive
cursor `00C04EFB` is not a sequential execution successor of this RET.

Saved default and effective TERMINATOR flow categories each agree separately
with the physical decode. Both have no fallthrough or targets; override is
NONE and fallthrough is not overridden. Saved `is_computed=false` does not
authenticate a static return value or override the unobserved stack source.

The accepted historical BF6707 call has bytes `e8e7e70000`, signed rel32 59367,
listing continuation BF670C and listing target C04EF3. Returning to CPU EIP
BF670C additionally requires the unverified mapping and the actual stack slot
to contain that value. The historical preceding AND targets DS:0109EEA8 under
its own stated decode condition. Unknown DS/SS bases and stack pointers prevent
a proof that this memory location is disjoint from the return slot. No preserved
return-slot value or particular runtime return destination is established.

## Scope and retained incidents

Original preparation, capture and decoder artifacts remain unchanged. Earlier
packet incidents and preexecution corrections remain in their historical
scopes. The current worker recorded one publication-stage display truncation
while inspecting saved JSON, with process exit zero. Root separately retained
two inspection process errors and one display truncation. These are not
capture or decoder failures; those first executions succeeded. Complete Root
review and first-launch originals are copied and pinned as a finite named set.

The publication phase performs only bounded offline packaging. There are no
additional Native calls, decoder executions, Source-body reads, absence census,
target/provider queries, Ghidra mutations, builds, tests, probes or runtime
observations. No guarded final-proof helper is constructed or run. The report
retains complete captured payloads, independent default/effective comparisons,
manual-before-import evidence and finite original artifact pins. The local
manifest and post-commit handoff identify the public artifacts and original
publication streams separately.
