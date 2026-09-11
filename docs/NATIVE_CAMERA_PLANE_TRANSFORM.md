# Native camera plane transform

This packet reconstructs complete B62D10 (184 bytes) and B65BA0 (129 bytes).
Descriptive names are hypotheses. Both original entries take ECX source and
stack destination/matrix, return the destination in EAX and use RET8. The new
fastcall interfaces take ECX source, EDX destination and one stack matrix
pointer, returning EAX destination with RET4. Native caller integration is
separate from these raw storage operations.

B62D10 snapshots source components with x87 FLD/FSTP float pairs in Y,X,Z,W
order. Those stores retain the original status and NaN quieting behavior.
The complete original register-stack schedule then performs row-vector matrix
multiplication with extended intermediates and four ordered float outputs.
No alternative dot-product grouping or premature rounding is introduced.
The source snapshot protects destination/source overlap, while a destination
overlapping the matrix can modify matrix entries read by later components.

B65BA0 first copies the source's raw bits with MOVSS into a local float4.
It calls the complete B62D10 operation with a separate local destination.
After that call returns, it reloads all four results before publishing them
to the caller's destination with MOVSS. The original intermediate stores
back into the source snapshot are retained. Thus a final output overlapping
the matrix cannot change the preceding calculation. No helper callback or
projected matrix representation replaces the full child.

Both operations borrow readable source/matrix and writable destination storage
for the accesses actually reached. There are no null checks, bounds checks,
normalization, matrix inverse/transpose construction, owner operations or
exception rollback. An access fault or hardware exception can preserve an
already-executed prefix; the wrapper's local result delays caller-visible
output until its child completes.

The strict worker Win32 build, both existing CTests and eight original seed
checks passed. One ignored probe compared six original/full-library pairs: a
dense multiplication, direct matrix/output aliasing, protected wrapper aliasing,
source/output overlap with a masked signaling NaN, and a matrix access fault
in each function. The direct inner alias produced 90,188,295,411 while the
wrapper preserved 90,100,110,120. A fault at matrix+34 left the inner first
output written and the wrapper caller output untouched.

The probe compares 1,906 observable bytes and independently verifies projections
from 12 full 752-byte raw results. State includes the whole fixture arena and
readable matrix prefix, x87 control/status/tag and live 80-bit values, MXCSR,
and XMM0..3. Instruction/data pointers, inactive registers and fault PCs remain
in the raw evidence without an equality claim. Unmasked floating exception
dispatch was not tested.

All inner x87 opcodes and memory offsets match the original exactly. The
wrapper preserves all 19 MOVSS instructions; only the alpha snapshot stack
offset changes with the new argument layout. Full COFF sections and relocations,
one exact archive object and 13 unchanged original/source code stages were
verified. See the audit report for the sealed artifact path and hashes.

Both functions are integrated into main. The strict Win32 main build and
both existing CTests passed, followed by independent original-body replay
against the actual main library. No permanent tests, original caller ABI
or gameplay result are claimed.


## Primary validation

The primary reviewed both sources and the focused fixture, verified all
35 immutable worker pins and both current source/header files, then froze
the actual main library
`fb144c4f4e87afe14d9c7c783716b54976544b2426d775f61bd86dc3bf94d9ed`.
The exact main archive member matches all worker code/directive sections
and relocations; debug/checksum metadata differences are recorded. The
fixture does not recompile the provider.

Fresh guarded live/installed-PE checks cover all 313 original bytes, and
eight native seeds matched. The unchanged six-case replay passed with
1,906 compared observable bytes and all 12 raw results independently
projected. The proof includes 169 complete linked COFF sections and
13 complete original/linked text postimage stages. Both original bodies
and their relative child call execute without code edits, operand
relocations or provider bridges.

The existing Ghidra names and prior comments were preserved, reviewed
evidence was appended and saved, and both exports and complete function
records were refreshed. The tracked report pins the read-only primary
bundle under `local/camera_plane_primary/`. Runtime limits listed above
remain unchanged.
