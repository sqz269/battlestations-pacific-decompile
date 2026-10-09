# Actual World child-retirement Source at 009035E0

This packet supplies the separately authorized actual-pointer Source for the
42-byte, 16-instruction body at `009035E0..00903609`. It implements
`bsp::retire_native_world_child_subtree_009035e0(void*)` as a guarded MSVC Win32
naked `__fastcall` function. The descriptive name is a hypothesis, not a recovered
symbol. The accepted read-only audit is
`reports/cc12_world_child_retirement_readiness.json`; its full native span and
current canonical input pins are retained in this packet's report.

The existing `destroy_child_subtree_009035e0(WorldExpiryHost&, void*)` projection
and its counted function record remain untouched. This packet earns no duplicate
Original function or byte credit. Source is written but is not worker build-tested,
emitted-code reviewed, ABI-admitted, or game-validated.

## Complete instruction mapping

| Native site | Native operation | Written Source |
| --- | --- | --- |
| 009035E0 | `PUSH ESI` | Save the caller's ESI. |
| 009035E1 | `MOV ESI,ECX` | Retain the actual receiver in ESI. |
| 009035E3 | `CMP DWORD [ESI+50h],0` | Test all 32 bits of the current gate. |
| 009035E7 | `JZ 009035FE` | Skip the child pointer read when the gate is zero. |
| 009035E9 | `LEA ESP,[ESP+00000000]` | Seven `_emit` bytes: `8D A4 24 00 00 00 00`. |
| 009035F0 | `MOV ECX,[ESI+48h]` | Reload the current first-child pointer on every iteration. |
| 009035F3 | `CALL 009035E0` | Direct call to this physical Source function. |
| 009035F8 | `CMP DWORD [ESI+50h],0` | Reload the parent's gate after the complete recursive call. |
| 009035FC | `JNZ 009035F0` | Repeat using a fresh child pointer. |
| 009035FE | `MOV EAX,[ESI]` | Fetch the receiver's then-current vptr. |
| 00903600 | `MOV EDX,[EAX]` | Fetch its then-current slot 0. |
| 00903602 | `PUSH 1` | Pass the full DWORD argument 1. |
| 00903604 | `MOV ECX,ESI` | Restore the actual receiver for its virtual target. |
| 00903606 | `CALL EDX` | Ordinary virtual call with a return to this body. |
| 00903608 | `POP ESI` | Restore the caller's ESI after the virtual target returns. |
| 00903609 | `RET` | Plain return; no incoming stack arguments are consumed here. |

The seven-byte LEA changes neither ESP nor flags and runs once on the initially
nonzero path. The self-call is the only direct Source dependency. Its native
opcode is at body offset 19 and its four-byte relative operand begins at offset
20 (`E8 FF FF FF`, relative displacement -24). The source names itself directly;
the primary emitted-code review must verify the complete body and physical
self-reference. Two internal short branches target `009035FE` and `009035F0`.

## Actual storage and call contract

The semantic input is the receiver in ECX; no incoming EDX value or explicit
incoming stack argument is consumed. The source's `void` type asserts no typed
result. ESI holds the receiver across recursive and virtual calls. Compatible
targets must preserve the native callee-saved registers; the virtual target must
consume the pushed four-byte argument because no `ADD ESP,4` follows its call.
The native target boundary also has EAX holding the current table and EDX holding
the current slot-0 target.

The gate at `+50h` is a full DWORD nonzero test, including values with the high bit
set. A zero gate never reads `+48h`. After each entire recursive child call,
including the child's own virtual call and its final POP/RET, the parent rereads
its gate and then its current `+48h` pointer if another iteration is required.
Neither a previous child pointer nor a snapshot of the gate is retained.

The parent's storage must survive all child returns and the late fetch of its own
vptr and slot 0. Its own virtual target may retire the receiver if the external
object contracts permit: this body performs no receiver read after that call,
only its stack POP and RET. This instruction-local fact does not establish a
concrete class, destructor implementation, allocator, unlink policy, or ownership.
Actual hierarchy mutation and lifetime/progress guarantees remain external.

There are no local node-field writes, successor reads, mark-state operations,
global accesses, x87 operations, allocation/free calls, local EH frame, or catch.
The source introduces no overlay, context, process cell, table, owner, callback,
token conversion, null guard, cycle guard, synthetic unlink, cleanup, `noexcept`,
or application binding.

## Failure and evidence limits

A null receiver faults at its first `+50h` read. A nonzero gate with a null child
forwards that null receiver into the recursive function. Cycles can recurse
without bound; a hierarchy that does not change can repeat or reach retired
storage. Completed child effects remain observable if a later operation fails.
This body supplies no rollback or recovery. Native unwind, fault, and gameplay
equivalence are unproven.

The report pins the accepted full native span (SHA-256
`a84482b641eec58c063a5e47549586e95b17485ffb141239c13634654f55e2f8`), all 16 decoded
sites and their source mapping, the accepted audit inputs, current canonical
inputs at baseline `683057d6d91f3d12c4e8c1299d8ff6ee75cff68f`, and the three source
and document outputs. Static validation checks mapping, unchanged inputs, scope,
and formatting. It does not execute the body or establish emitted instruction
identity.

Primary integration must register the source, build Win32, compare all 42 emitted
bytes with correct self-relocation handling, verify the physical definition in
the core library, and run the existing relevant checks before any Source
admission. No worker CMake, ledger, Ghidra, build, test, or probe changes occur.
This qualified leaf can remove the direct-service prerequisite for a separately
authorized actual-pointer `00903610` implementation only after that admission;
the full World and virtual-retirement production graph remains open.

## Primary registration and complete emitted review

The integrator registered this source in bsp_core. The normal MSVC Win32 build
and all three existing checks passed. The whole emitted function is 42 bytes /
16 instructions. The only relocation, operand20, targets the same physical
symbol index8, section3, value0. Rebinding it to `E8 FF FF FF` (-24) makes all
42 bytes equal the installed PE and accepted live capture, including both
branches and the seven-byte LEA ESP. One complete matching core member and
one positive public definition establish that physical self binding.

The final indirect operation is CALL EDX followed by POP ESI and plain RET.
Current parent gate/head reloads follow the entire recursive call; no returned
child or own post-virtual receiver dereference was inserted. The counted
projected function is preserved; admission adds one actual-storage fragment
and zero new Original functions or bytes. Its existing Ghidra name/comments
are retained with appended evidence and a refreshed export.

The qualified direct Source service for the separately reviewed 903610 pass
is now available. That caller's implementation and production ownership graph
are not admitted here. This root is absent from the game map. Actual slot-zero
methods, argument cleanup, parent lifetimes, hierarchy progress, Native fault /
exception behavior and gameplay remain open. Complete evidence and artifacts:
`reports/cc12_native_world_child_retirement_primary_review.json` and
`local/cc12_world_child_retirement_primary`. Worker document pins precede this
primary appendix and remain historical.
