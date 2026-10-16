# GOAL15U tagged B3 packet 05 integration

Twelve complete Source functions close the last twelve uninspected bodies
in the original 79-candidate inventory. They reference 62 Native bytes and
emit 56 Source bytes: eight exact whole entries (35 bytes), two meaningful
new argument interfaces (25 Native / 15 Source bytes), and two complete
empty functions with different RET encoding (two Native / six Source bytes).
This closes that screening inventory's inspection, not the whole B3 band.

Worker pin `e0bccf31fe66807ed25e9f014c77bfc4c12300d8` received independent
review, with no blocking findings. The primary verified all twelve complete
strict and registered executable sections, actual symbol offsets, absence of
executable relocations or undefined providers, fresh live/installed Native
bytes, and the entire actual Core archive member. Source integration
`939952a16` passed `scripts/build.ps1` and all three existing checks. No
routine tests or runtime probes were added.

The setter `00B33A90` and scaled-child operation `00B347C0` use meaningful
new Source ECX/EDX interfaces. Original consumes ECX plus an actual stack
word and RET 4; input read order, scratch state, cleanup and fault contexts
differ. Their normal data effects follow the mapped DWORD values, with
unsigned address arithmetic modulo 2^32. No Original ABI adapter is supplied.
`00B33F30` and `00B33F40` use entire Source RET 0 instructions instead of
one-byte Native RET. Return-stack effects agree with valid executable backing;
the extra instruction-fetch bytes are explicitly qualified.

Opaque parameters on the RET 4/8 Source envelopes describe cleanup slots;
Original never reads those cells. Byte-zero `00B33E80` preserves incoming
upper EAX in its actual emitted body and exposes only AL. Readable ranges may
alias; memcpy access avoids inventing aligned typed objects or full layouts.
The existing `RenderMiscHost::texture_format` virtual declaration and its
consumer remain unchanged and incomplete; the raw getter adds no binding.

Original names, signatures and comments were preserved before mutation.
Saved Ghidra prototypes describe Original physical storage: setter/scaled
use exactly one automatic ECX input plus one stack word, with no duplicated
receiver or fabricated EDX. Existing evidence and tags remain; affected
exports, typed metadata, snapshot and index were refreshed.

The inventory now has 53 complete Source entries / 251 Native bytes, including
46 exact / 194 bytes and seven qualified / 57 bytes. Twenty-six remain
deferred; nineteen have no established semantic blocker and are queued for
the next packet. The three RMW operations require preserving their actual
flags and access behavior. Two global bindings, the SSE stack copy and x87
load retain separate contract gates. Inspection alone is not reconstruction.

Full worker qualification-through-handoff time was 1205.493 seconds, including
a 474.548-second registered build. Independent review and primary integration
are additional costs; no controlled speedup or gameplay result is claimed.
Primary proof: `reports/goal15u_reconstruction_batch.json`. Preserved inputs:
`local/goal15u_b3_author/` and `local/goal15u_b3_review/`. Detailed worker doc
and report retain their `GOAL15S_TAGGED_LEAF_B3_05` names.
