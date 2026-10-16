# GOAL15L raw world helpers and group owner removal

This batch adds four complete Source routines: world-border node allocation
`004C2310` (51 bytes), checked count increment `004CF450` (147 Native bytes),
x87 span `004B6C40` (59 bytes), and avoidance group owner removal `0041D150`
(137 bytes). Their 394 Native bytes contain 125 instructions. Existing typed
interfaces and historical evidence remain intact.

The node preserves late reads from actual argument cells after real allocation.
The span retains all 59 original bytes, including the actual argument spill and
x87 control/status behavior. Remove-owner preserves the pre-null association
read, current count/array reloads, same-slot retry and null-owner ordinary call
to the genuine raw group-clear routine. Only concrete provider call operands are
rebound. The count uses the genuine current SDK length_error policy; it does not
reproduce the Native legacy string, allocation schedule, FH3 or exception ABI.
Its independent header/addition reads also have different emitted load order.

The primary reviewed whole Source and Native entries. An independent worker
reviewed all three world helpers, installed/live bytes, COFF relocations/addends,
exception evidence, fixture machinery and artifact hashes. Ghidra physical
Original prototypes exclude Source-only EDX. Existing names/comments are
preserved, with the misleading count name updated to include its returning
increment path. Old annotation values, saved project and refreshed exports are
recorded in the batch receipt.

Registered-build and final archive-fixture results are recorded in
`reports/goal15l_reconstruction_batch.json`. These checks do not establish
Native private-CRT/frame/ABI identity, production world/entity composition,
deployment or gameplay. Whole map writer/layout/traffic and authored-global
producers remain separate open work.
