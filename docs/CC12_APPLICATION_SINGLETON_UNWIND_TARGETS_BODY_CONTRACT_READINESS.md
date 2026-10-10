# Application unwind target body contracts

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** Exact bodies establish a receiver
profile store and a guard cleanup sequence. The guard contains a real
**indirect CALL through `CE2210`**, followed by RET. The import's contents and
behavior, original EH frame binding and callable Application ownership remain
unproved. The stored summary's zero call count does not describe that opcode.

Source is frozen at `d974067ee19938947daa59b7dabd0a6261a22989`: all 4,210 tracked
src/include files, Git blobs and working bytes, plus 38 supporting files.
Source and working bytes match the preceding metadata packet. All 619 prior
pins are physically copied unchanged, including nested
525/433/355/287/230/184/124 proofs and compiler-definition artifacts.

## Admission and exact code

The only leased ranges are `00412430..00412436` and `00411EE0..00411EF8`.
Before reading code, complete typed queries covered **every one of their 32
bytes**, including both entries and ends. Each byte belongs to the expected
single complete contiguous function and to an instruction contained wholly
within that range. No interior competing entry or range overlap was reported.
All 32 before/after raw typed responses agree at **epoch 95**. Entry metadata
also matches the preceding independently retained metadata capture.

Every Native batch verifies project `bsp`, `C:\Users\sqz269\bsp.gpr`, program
`/battlestationspacific.exe`, x86/32-bit little-endian, 64,730 functions. Exact
bytes and saved assembly cover only **7 + 25 = 32 bytes**. Independent decoding
matches all ten instructions and their per-byte ownership. Nothing was repaired.

| Address | Exact bytes | Instruction |
| --- | --- | --- |
| `412430` | `C7 01 18 38 CE 00` | `MOV DWORD PTR [ECX],CE3818` |
| `412436` | `C3` | `RET` |
| `411EE0` | `8B 41 04` | `MOV EAX,DWORD PTR [ECX+4]` |
| `411EE3` | `85 C0` | `TEST EAX,EAX` |
| `411EE5` | `C7 01 FC 37 CE 00` | `MOV DWORD PTR [ECX],CE37FC` |
| `411EEB` | `74 0B` | `JZ 411EF8` |
| `411EED` | `83 40 18 FF` | `ADD DWORD PTR [EAX+18h],-1` |
| `411EF1` | `50` | `PUSH EAX` |
| `411EF2` | `FF 15 10 22 CE 00` | `CALL DWORD PTR [CE2210]` |
| `411EF8` | `C3` | `RET` |

All instruction overrides are NONE. The branch is default/effective
`CONDITIONAL_JUMP`, and the two RETs are `TERMINATOR`. At `411EF2`, both default
and effective flow are **`COMPUTED_CALL`**, with fallthrough `411EF8`. Its
default target list is empty; the effective list contains saved analysis
reference `EXTERNAL:00000159`. That reference is not the runtime pointer stored
in `CE2210`, and no IAT value, external function or imported instruction byte
was read. The summary still says `call_count=0`; exact opcode and typed flow
establish one CALL. The earlier funclet JMPs and their CALL_RETURN overrides
remain distinct observations.

## Receiver and guard effects

The reset entry consumes the actual receiver in ECX, writes the profile DWORD
`CE3818` at offset zero and executes plain RET. It does not directly change a
data register or arithmetic flags. With a valid incoming return stack, RET
consumes only that return address: ESP increases by four, with no immediate
argument cleanup. The receiver write can fail before any normal return.

The guard entry consumes an actual record address in ECX. It first reads the
captured pointer from record+4 into EAX. TEST sets the condition before the
profile store; MOV preserves those flags, so the subsequent JZ still tests
the captured pointer. The profile write at record+0 therefore occurs on both
null and nonnull paths, after the +4 read.

For a null captured pointer, JZ reaches the local RET directly. No counter
access, argument push or import call occurs. EAX is zero, ECX is unchanged,
and the normal return consumes only the incoming return address. TEST defines
SF/ZF/PF, clears CF/OF and leaves AF undefined; MOV/JZ/RET do not change these
arithmetic flags.

For a nonnull captured pointer, the body performs a DWORD **ADD of -1** at
captured+18h, then pushes the captured pointer and calls through `CE2210`.
The decrement wraps modulo 2^32; it is not a DEC instruction and updates
arithmetic flags, including CF. The external call can change flags and
volatile registers. There is no local rollback if a counter access, stack
write, indirect call or imported operation fails after the profile write.

No direct instruction writes record+4. This does not exclude an aliased
counter address or imported side effect from changing that storage. Claims
about the complete record's preservation require the actual raw domain and
any relevant disjointness assumptions. The null test checks the captured
section pointer, not the validity of the guard record itself.

## Conditional nonnull return stack

Let `S` be ESP at guard entry, holding its incoming return address.

| Point | ESP | Stack content introduced by these instructions |
| --- | --- | --- |
| After `PUSH EAX` | `S-4` | Captured section pointer |
| At indirect callee entry | `S-8` | Return address `411EF8`; pointer at +4; original guard return at +8 |
| Required normal continuation at `411EF8` | `S` | Imported callee must have removed its one four-byte argument |
| After guard's final RET | `S+4` | Original guard return address consumed |

There is no local ADD ESP to discard that argument. A plain imported RET
without argument cleanup would leave ESP at `S-4`, so the final guard RET
would not consume the original return address. The exact body thus requires
callee cleanup for the intended normal continuation; it does not prove the
current import supplies it or returns at all. No imported-byte or OS-behavior
credit is claimed.

## Comparison to existing Source

The frozen `destroy_native_generic_singleton_base_00412430(void*) noexcept`
implementation writes one volatile `uint32_t` profile `CE3818` through its
supplied receiver. The frozen `destroy_native_singleton_guard_00411ee0(void*)`
implementation captures the record's +4 pointer, writes `CE37FC` at +0,
conditionally decrements the actual section's counter at +18h, then calls
`LeaveCriticalSection`. Its unsigned counter decrement has the same stored
modulo-2^32 arithmetic. The directly observed memory-operation ordering agrees
for valid raw storage; the native TEST's placement before the profile MOV
does not change that captured-value branch condition.

The Source API name is not evidence of the current pointer in `CE2210`.
Existing Source/build-list membership does not prove imported runtime effects,
compiler lowering, preserved Native registers/flags or original calling ABI.
These helpers use explicit C++ pointer arguments, while the original bodies
consume ECX and an already prepared return stack. Their Source interfaces are
not direct binary replacements.

Under the previously retained conditional action-frame binding, action 0
loads the saved original receiver into ECX and action 1 forms the actual
eight-byte guard record address. These target bodies accept those argument
forms, but establish no FH3-provided EBP, actual action invocation, exception
state order or retry/destruction policy. The constructor still has no EBP
operand. Original receiver, current publication, selected registration
argument and captured section remain separate identities.

## Custody, proof and next proposal

Preserve stable raw receiver/guard storage, the actual +14 member reference,
authority cells, borrowed services and escaped startup/shutdown/failure
records. `E1AE78` and normal callable retirement remain separate gates. No
new reset, unregister, free, retry or destructor policy is activated here.

Finite replay checks closed Source/Git corpora, 11 lexical queries, 17
excerpts, all inherited proofs, capture order, all 32 byte-ownership records,
exact code, decoding, typed flows and the conditional stack arithmetic.
Physical relocation copies only declared files plus the report; its guard
denies undeclared reads, writes, subprocesses and network. Actual Git replay
checks current/prior frozen blobs and both committed outputs.

Fresh bodies and prior target metadata are separate epoch-95 observations.
Actions remain epoch 92, map 90, descriptor 88, handler 85 and constructor 79.
No neighboring code, external target, IAT contents, descriptor suffix, other
funclet, installed CRT or OS/game state was inspected. No C++/CMake/ledger/GPR
change, repair, build, test, probe, Native execution, ABI equivalence or frame
activation is added.

Next single proposed packet:
`cc12_application_singleton_exceptional_source_composition_readiness`.
Use only frozen Source and retained constructor/state-map/action/target evidence
to assess an explicit C++ cleanup composition: actual receiver and guard
identities, arming order, escape/poststates and custody. Admit zero new Native
bytes, no new Ghidra work and no C++ implementation/build/tests in that readiness
gate. Keep such a Source composition separate from original FH3/EBP, current
IAT binding, external return behavior and binary/runtime equivalence.

Evidence: `reports/cc12_application_singleton_unwind_targets_body_contract_readiness.json`
and its declared local finite bundle.
