# Native general camera matrix inversion

The explicit raw entry `invert_native_camera_matrix_00b632d0` promotes the
existing private instruction schedule to the complete B632D0..B63B28 body.
Original ABI is ECX destination, EDX source, EAX destination and plain RET.
The inputs are raw 64-byte matrix views. Descriptive names are hypotheses.

The function loads the target's immutable +1 and negative-zero words,
snapshots sixteen source DWORDs on its own stack with `REP MOVSD`, then
publishes a destination identity matrix before the native elimination.
Pivot comparisons, row swaps, x87 float spills, SSE comparisons/subtractions,
and final stores retain their original order. The operation introduces no
singularity check, epsilon, fallback identity, helper call or owner work.
Ambient x87/SSE state and the original direction-flag calling convention
remain inputs. Source/destination overlap is handled by the original stack
snapshot; no C++ array overlay or floating expression substitutes for it.

The two reconstructed constant words are fixed by fresh original-image
evidence: D7A208 is 80000000 and D7A24C is 3F800000, both in a nonwritable
section. Their addresses relocate in the rebuilt module. A modified target
with different constant data is outside this recorded executable contract.

The strict main Win32 build and both existing CTests passed; all eight
fresh native seeds matched. The complete 2,136-byte COFF section matches
the original after only two constant-address relocations, with exact
constant contents in compiled `.rdata`. The original eight unreachable
alignment bytes at B635D8..B635DF are retained. The exact object occurs
once in the frozen actual main library.
No new test case, original-body runtime fixture or gameplay claim is added
for this promotion. Complete instruction identity and existing checks are
the intended verification boundary. The raw camera hierarchy still needs
separate integration.


The audit seals the actual main library/object, source/header, build and
command logs, full original capture and all relocation details under
`local/camera_general_inverse/`. The existing Ghidra name and prior comments
were preserved, reviewed evidence was appended and saved, and the export
and complete function record were refreshed. The older semantic wrapper
remains available.
