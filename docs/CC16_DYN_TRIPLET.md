# Dyn triplet comparisons

`src/cc16_dyn_triplet.cpp` recovers the complete `00C39040..00C390F9`
integer body and the previously undefined `00C48140..00C48167` wrapper.
Names describe observed behavior and remain hypotheses.

The 186-byte leaf compares three raw DWORDs for equality under cyclic
rotation. It first compares right/left in order and stops on the first
mismatch. Only then does it capture right words 1,2,0 and compare that
rotation against left. On another mismatch it captures left words 0,1,2,
arranges 1,2,0 and compares against fresh right reads. All-success returns
full EAX one; final mismatch returns zero. It performs no input writes,
external calls or floating-point arithmetic. Conditional read epochs and
32-bit address arithmetic are retained; no eager two-record snapshot,
validation guard, allocation or provider is introduced.

Original leaf ABI: EAX left, EDX right, no stack arguments, plain RET;
EBX/ESI/EDI saved and EBP untouched. The explicit C++ interface does not
supply this register ABI, exact fault state or instruction identity.

The 40-byte wrapper receives left in EAX and right as its sole stack
argument, then returns with RET4. Its native loads capture left[1],
left[2], left[0], arrange the local triplet as [left2,left1,left0] and call
the leaf at `00C4815D`. The C++ wrapper directly composes the recovered
leaf with that actual local triplet and unchanged right pointer.

## Analysis boundary correction

The pre-recovery index attributed call `00C4815D` to preceding function
`00C48100`. Its exported assembly actually ends at `00C4813F`; live
instruction-context reported no instruction at the call. Disk disassembly
of `00C48140` proves a distinct stack frame, single direct call and RET4 at
`00C48165..67`, followed by eight INT3 bytes before the next defined entry
`00C48170`. The wrapper has no overlap with its preceding or following
function. Live bytes for the full 40-byte span match the installed image.

The locked definition tool defines exactly `[00C48140,00C48168)` and saves
the existing BSP program. Its durable receipt is
`reports/cc16_dyn_triplet_definition.json`. No neighboring body, padding or
unknown no-return property is changed. Snapshot/index refresh updates the
caller attribution; the old indexed owner is not treated as ABI evidence.

## Integration limits

The source is registered directly in `bsp_core`. Build/call-site receipts
are recorded in `reports/cc16_dyn_triplet.json`. No new test is added for
this small integer translation; existing compilation/checks and complete
assembly inspection supply proportionate validation.

The leaf replaces its metadata-only placeholder. The new wrapper is outside
the original 29-function referenced-body census. Neither source body is
declared native-ABI compatible or game validated. Genuine original root
reachability and live caller custody remain open; zero currently known
incoming references would not by itself prove a complete original dead chain.

The combined Win32 Release build on accepted base `90d44021a` passed;
both existing suites passed in 11.55 seconds. Exact call verification passed
one row with zero failures. The corrected snapshot contains 77,631 functions.
Locked naming/comments were saved, both exports refreshed, and prior analysis
annotations retained in `local/ghidra-annotations-20261029T042530Z.json`.
