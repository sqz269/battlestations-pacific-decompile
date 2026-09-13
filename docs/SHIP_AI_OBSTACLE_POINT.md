# Obstacle footprint point predicates: 009D80C0 and 009D8160

Packet `orch6_obstacle_point_k` reconstructs both complete leaf instruction
schedules over the existing `ShipAiObstacleNode` semantic record. It adds
`ship_ai_obstacle_point_in_near_box_009d80c0` and
`ship_ai_obstacle_point_in_avoid_box_009d8160`, each taking a const node and a
const `std::array<float, 2>` point. Runtime integration and footprint production
remain with their existing owners. These are new typed interfaces, not binary
replacements for the native 90h node.

## Native bounds and caller contract

| Entry | Complete inclusive body | Instructions | Native ABI |
|---|---|---:|---|
| 009D80C0 | 009D80C0..009D8156 | 50 | ECX=node; stack point pointer; AL result; RET 4 |
| 009D8160 | 009D8160..009D8200 | 52 | ECX=node; stack point pointer; AL result; RET 4 |

Both bodies have zero calls and zero flow gaps. The saved Ghidra `void(void)`
signatures omit real inputs; assembly and callers establish the contract.
Near returns at 009D8148/009D8154; avoid at 009D81F2/009D81FE. Upper EAX bits
contain scratch or incoming data and are not part of the Boolean result.

The sole caller function is 009EB660..009EC277. Its three direct sites are:

- 009EB8C0 -> 009D8160: ECX is the current node, loaded through the iterator at
  009EB8AF/009EB8B3; the stack argument points to the straight probe at ESP+20.
  A true AL result branches to the blocked path at 009EB925.
- 009EB8CA -> 009D80C0: ECX is retained by the preceding predicate. EBX is the
  hull position at controller+184, established by 009EB6C6 after ESI receives
  the caller's first argument at 009EB693. A true result skips that neighbour.
- 009EBE0E -> 009D8160: ECX is loaded from the neighbour iterator at 009EBDE0;
  the arc probe pointer is ESP+3C at 009EBE09. True branches to 009EBEB3.

Caller owner-null/deleted-byte checks are separate from these predicates.
Neither predicate reads the owner, owner-deleted state, lifetime, or pass side.

## Geometry and comparison order

Near rejects any nonzero byte at native+68 before dereferencing the point.
Avoid first checks +68, then +69, and rejects either nonzero byte. The near
center is +20/+24; the avoid center is +44/+48. Both predicates use the SAME
two axes at +28/+2C and +30/+34. Avoid does not read +4C..58 corner axes.
Near extents are +38/+3C; avoid extents are +5C/+60.

The exact arithmetic sequence is:

1. Subtract center from each point component on x87 and spill each delta to
   float32.
2. Form `delta_z * axis0_z + delta_x * axis0_x` in the native x87 order and
   spill the sum to float32. Clear its sign bit with integer AND 7FFFFFFF.
3. Load extent0 and execute FCOMIP followed by JC. Reject if extent is below
   the absolute projection or the comparison is unordered.
4. Only after that succeeds, use the retained delta operands for
   `delta_z * axis1_z + axis1_x * delta_x`, spill, clear its sign bit, and
   perform the same extent1 comparison.

**Edges are inclusive.** Equality does not take JC. This corrects the prior
strict-inequality description attached to 009D8160. NaN comparisons reject;
no finite-value filter, normalization, epsilon, or extent repair is added.
Negative extents fail ordinary projections; signed-zero extents can accept a
zero projection. A float-spilled infinite projection can equal an infinite
extent and pass. All first-projection rejection paths pop the two retained
delta operands, so normal returns balance the x87 stack.

## Reused storage and real producers

`include/bsp/ship_ai_sector_scan.hpp` already defines the semantic node, and
`src/ship_ai_neighbour_box.cpp` already reconstructs 009EAE20/009EAFC0. This
packet maps native offsets to `offsetof(ShipAiObstacleNode, field)`; it never
casts the semantic node into a 90h native allocation or adds another node type.

Several existing field names are reversed relative to the producer's geometry:

| Native fields | Existing C++ fields | Producer interpretation |
|---|---|---|
| +28/+2C | axis_beam_x/z | hull forward axis |
| +30/+34 | axis_forward_x/z | hull beam axis |
| +38 | near_half_beam | half-length plus forward lookahead |
| +3C | near_half_length | half-beam |
| +5C/+60 | avoid_half_beam/length | extents tested against the same two axes |

009EAE20 writes cos/sin to +28/+2C at 009EAE9D/009EAEA8, constructs the other
axis as `(sin, -cos)` at 009EAEB8/009EAEC0, and stores the extents at
009EAF9C/009EAFAB. Existing producer documentation records the actual negative
zero subtraction used there. 009EAFC0 owns avoid refresh, including copied
near geometry and the +68/+69 validity gates; the point predicates consume
those results without replacing their production.

Original constructor 009E52E0 writes +69=0 at 009E5361 and +68=1 at 009E5377.
The existing semantic record defaults both bools to false, so a default C++
record is not equivalent to that native constructed state. In particular, zero
axes and zero extents with clear flags can accept arbitrary finite points.
The runtime binding must pass actually produced geometry and flag state. This
packet neither repairs defaults nor fabricates a footprint. Native nonzero
flag bytes can be represented as canonical Boolean values because these two
readers only compare each byte against zero; raw byte identity is not claimed.

## Source ABI adaptation

Private naked fastcall kernels use ECX for the semantic node and EDX for the
point. Pushing EDX reserves a source-owned point/dot-spill slot before the
original 8-byte scratch frame. Original ESP+C argument/dot accesses become
ESP+8; delta accesses stay ESP+0/+4. Original node reads use semantic offsets.
All floating-point instructions and branches retain their schedule. Each
original ADD ESP,8 / RET 4 tail becomes ADD ESP,8 / POP EDX / RET; EDX is scratch
and its popped value may be a dot product's bits. No source or native control
word is reset by these functions.

## Verification and limits

Win32 Release and both existing CTests passed. The first parallel build hit
an access-denied CMake `generate.stamp` timestamp race; retrying the unchanged
build succeeded. No tracked tests were added.

The ignored fixture `local/obstacle_point_probe.cpp` executes both complete
original byte spans and the compiled source over 16 bounded cases (32 calls).
It checks all Boolean outcomes, x87 TOP balance, and unchanged native node,
semantic node, and query storage. Cases include inclusive corners, the next
float outside an edge, a delta spill that rounds onto an edge, a dot spill on
a rotated edge, nonbinary native flag bytes, negative/zero extents, zero and
unnormalized axes, NaNs, infinite spilled dots, distinct near/avoid centers,
and deliberately different corner axes.

`local/prepare_obstacle_point_probe.py` verifies all 151+161 executed bytes
against both the installed PE and live Ghidra. The 321-byte contiguous mapping
also includes nine unexecuted alignment bytes. All branches stay inside their
function; neither body has an absolute memory operand or call, so arbitrary
VirtualAlloc placement needs no relocation or library hooks. Both result
records contain 32 equal canonical Boolean bytes. The fixture uses masked
x87 exceptions, 53-bit precision, and nearest rounding. It does not prove
unmasked exception delivery, every control-word mode, upper EAX or status-word
parity, the whole sector scanner, runtime node refresh, or gameplay.

Reproduce from this worktree after `./scripts/build.ps1`:

```powershell
python local/prepare_obstacle_point_probe.py
cmd /c local\build_obstacle_point_probe.cmd
./local/obstacle_point_probe.exe
python tools/verify_report_calls.py reports/ship_ai_obstacle_point.json
```

The preparation script needs installed `pefile` and `capstone`, the configured
game PE, and the existing read-only Ghidra connection. Probe compilation uses
VS 2026 Community vcvars32, /O2 /fp:strict, the current bsp_core.lib, and
/MANIFEST:EMBED. Exact source, image, result, listing, build-script, and log
hashes are preserved in `reports/ship_ai_obstacle_point.json`. Ghidra names
remain hypotheses; this worker made no Ghidra mutations.
