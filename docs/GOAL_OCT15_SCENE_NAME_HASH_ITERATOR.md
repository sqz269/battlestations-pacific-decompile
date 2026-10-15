# Raw scene name-hash iteration

Complete ordinary raw Source covers 00466BB0..00466BF0 (65B/22 instructions)
and 00466C00..00466C5B (92B/33 instructions). The descriptive native names
are retained as hypotheses. Both receive a 12-byte iterator in ECX, consume
no EDX or stacked inputs and use plain RET. Begin has no common semantic
EAX result; Next returns zero-extended EAX 0 or 1. Neither native body calls out.

The actual iterator has words table+0, node+4 and index+8. A table is tested
at +4 and heads are read at +8+index*4; node links are at +Ch. These offsets
describe observed accesses, without asserting a nominal container or ownership.
The existing projected structs/functions remain available separately.

Begin captures the table once and tests its +4 word before touching the index.
A zero test word clears node only. Otherwise it zeroes index and tests head0.
The scan reloads the actual index each iteration, increments and publishes it,
then reads that head. The bound test precedes increment, so head64 can be read.
It finally reloads index, reads the selected head again through the captured
table and publishes node. No consistency assumption removes the head64 path.

Next captures current node then its +Ch link. A nonzero link is published,
then current node is reloaded for the result. A zero link increments the actual
index, reloads it, and captures the scan table only when index is below64.
Scanning publishes each increment and never reads head64. After scanning it
reloads actual index, and if still below64 reloads CURRENT table for the final
head read; otherwise it clears node. All returning paths reload actual node
and return 0 or 1. Unsigned increment and address arithmetic wrap modulo32.

Reached actual storage may alias while every access stays valid. No native
private-frame, implicit Source argument-home, asynchronous mutation, register,
flags, fault, mangling or binary ABI identity is claimed. New-entry native
differential, application adoption and gameplay proof remain open. Independent
review, live/installed-byte evidence and default Win32 build are pending.
