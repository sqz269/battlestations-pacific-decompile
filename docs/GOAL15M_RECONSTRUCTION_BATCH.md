# GOAL15M complete raw range helpers and manager lookup

Seven whole Source routines cover 456 Native bytes and 189 instructions:
six POD18 range/size helpers and the 66-byte avoidance manager layer lookup.
The public bodies preserve 448 fixed bytes; two verified Source child CALL
operands account for the remaining eight bytes. They contain 12 branches and
36 x87 operations. No callback, mutable global, CRT child, added frame or cleanup
guard is introduced by these modules.

The range helpers retain actual argument/local cells, byte-only tag clearing,
forward overlap, per-word x87 effects and exact stack cleanup. The lookup captures
signed count once, returns current slot0 even for nonpositive count, selects first
equality or last physical qualifying index, and reloads the chosen raw slot.
Existing typed projections and mandatory caller bindings remain distinct.

Primary review covers all seven entries; independent review covers the six
range helpers. Both inspect complete physical bodies, installed/live bytes,
actual emitted COFF and the small captured-Native-body fixtures. Registered
build, archive-only fixture and operand evidence is in
`reports/goal15m_reconstruction_batch.json`. Worker baseline builds leave these
new modules unregistered and are recorded separately from primary validation.

Ghidra names and old comments are retained. Five conventional range prototypes
and the manager lookup physical prototype exclude Source-only EDX placeholders.
The raw ECX-metadata dispatcher at `0048D2F0` has a precise physical contract in
comments, while its formal custom-storage signature remains unresolved because
the live bridge cannot apply it. Its displayed undefined cdecl placeholder is
not an Original ABI assertion. No pointer-this class or dummy Original EDX was
invented to hide that annotation gap.

Whole traffic/layout/resize/settings and actual world/entity composition remain
open. These bounded Source checks do not establish original template identity,
private stack-address identity, deployment, whole Native ABI or gameplay.
