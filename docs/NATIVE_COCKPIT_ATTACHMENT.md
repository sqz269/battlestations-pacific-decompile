# Cockpit helper attachment setter, B3C650

Read-only reconstruction evidence, 2026-09-13. The current Ghidra program is
`C:/Users/sqz269/bsp.gpr` `/battlestationspacific.exe`. This packet does not
implement the setter: no existing concrete owner lookup covers every node that
can occupy helper+08. In particular, a camera-specific terminal profile cannot
be used for the arbitrary incoming node. This is a provider gate, not an
uncertain instruction extent.

| Routine | Coverage | Extent | ABI |
| --- | --- | --- | --- |
| B3C650 | complete static body; implementation blocked | B3C650..B3C6B6, 103 bytes, 43 instructions | ECX actual 24h helper, stack one raw node pointer, RET4; no meaningful result |

The body first loads the **current** helper+08 and, if nonzero, calls B6DA70
with ECX=current, float 0.0 and recurse byte 0. B6DA70 writes node+AC and
does not traverse children for recurse=0. It then captures helper+08 in EDI,
loads the incoming pointer from the stack, and compares them. If different,
it publishes incoming to helper+08 **before** incrementing incoming+04. It
decrements captured old+04; on zero, it dispatches the old object's **current**
virtual+00, which may retire its binding and reenter callbacks. Equal pointers
skip publication and reference transfers, but the visibility calls still run.
Finally it reloads the current helper+08 and hides it if nonzero. A callback
that changes +08 may therefore make the final target differ from incoming.

| Numeric transfer | Preparation and effect |
| --- | --- |
| B3C663 -> B6DA70 | ECX from helper+08 at B3C653, `FLDZ`, stack float 0 and recurse 0; hidden before captured old is compared |
| B3C67E -> `InterlockedIncrement` IAT CE221C | EAX incoming+04 after helper+08 publication at B3C675; only for changed nonnull incoming |
| B3C68C -> `InterlockedDecrement` IAT CE2220 | EAX captured old+04; only for changed nonnull old |
| B3C69C -> old virtual+00 | ECX captured old EDI, only if decrement returned zero; exact concrete profile varies with the old object |
| B3C6AF -> B6DA70 | ECX freshly reloaded helper+08 at B3C69E, stack float 0 and recurse 0; runs even on identity assignment |

Ghidra's one contained caller is 7BC610..7BC680. At 7BC660 the caller loads
entity+808 into EAX; 7BC66D loads service pointer F8D39C, 7BC673 takes its
+0C helper; 7BC676 pushes EAX and 7BC677 calls B3C650. The branch requires
entity+808, entity+80C and service+0C nonnull. The other xref is 7BC707:
disk bytes decode a candidate 7BC690..7BC797, with service+0C in ECX and
`PUSH 0` at 7BC705. Ghidra currently reports no containing function at either
7BC690 or 7BC707, so this candidate's attribution is pending root repair.
The candidate first calls B6DA70 on entity+80C with factor 1 and recurse 0;
that does not establish entity+80C as the node subsequently released by the
helper setter. Its post-call matrix writes likewise do not identify helper+08's
concrete class.

The available `SceneAttachmentRuntime::resolve_key` returns a live
`SceneNodeAttachment`, while `NativeNodeBinding` exposes the actual node+AC
word. `RenderCommandReference` can borrow a canonical +04 count, and
`NativeRenderActualOwners::resolve_actual` is documented as a zero-transition
lookup for known actual owners. None currently establishes a canonical mapping
from *every* helper+08 raw key to its matching node binding **and** its current
virtual+00 terminal owner. Before implementation, bind that mapping in the
shared owner domain and show that it includes both the entity+808 producer and
the old node on the zero argument path. Keep the 24h native helper storage
separate from any host companion. The setter must not allocate or initialize
the helper, invent a callback for old virtual+00, or assume camera ownership.

Original bytes B3C650..B3C6B6 were read through target-guarded BSP Ghidra
`bytes`, `disasm`, `proto`, and `xrefs` commands. This establishes static
control flow and ABI only; there is no build, differential fixture, or game
validation for an implementation in this packet.
