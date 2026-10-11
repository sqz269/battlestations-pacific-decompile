# CC12 CRT dispatch first-call body: exact physical24-byte boundary

The saved owner at `00C0522E..00C05245` contains 24 contiguous bytes and 11 complete x86-32 instructions. Manual opcode/ModRM/immediate decoding, actual detailed Capstone 5.0.7, context-zero saved text, and typed instruction lengths agree throughout. This establishes physical operands and a qualified local CPU continuation. Saved `__getptd`, imported types, TLS/provider behavior, Original ABI, and Source implementation remain unproved.

The [machine-readable report](../reports/cc12_crt_dispatch_first_call_body_metadata.json) contains all eleven physical records, saved flow metadata, numeric targets, the CPU contract, and evidence pins. The finite supplied-ZIP proof is retained under `local/cc12_crt_dispatch_first_call_body_metadata/`.

## Physical instructions

| Address | Bytes | Instruction |
| --- | --- | --- |
| `00C0522E` | `56` | PUSH ESI |
| `00C0522F` | `E8 83 FF FF FF` | CALL `00C051B7` |
| `00C05234` | `8B F0` | MOV ESI,EAX |
| `00C05236` | `85 F6` | TEST ESI,ESI |
| `00C05238` | `75 08` | JNZ `00C05242` |
| `00C0523A` | `6A 10` | PUSH sign-extended value 16 |
| `00C0523C` | `E8 C8 67 FF FF` | CALL `00BFBA09` |
| `00C05241` | `59` | POP ECX |
| `00C05242` | `8B C6` | MOV EAX,ESI |
| `00C05244` | `5E` | POP ESI |
| `00C05245` | `C3` | RET |

The body SHA-256 is `cde242ec079bf21854f59956d9ccd96cada3bce642173044e6c8c16fc11ee0f1`. Both calls are physical E8 rel32 instructions; their signed displacements are -125 and -38968, with encoded return sites `00C05234` and `00C05241`. Targets remain numeric addresses. The internal branch lands on a proven complete instruction start. All eleven saved flow overrides are NONE; there is no physical gap.

## Qualified local CPU continuation

Each statement assumes the instruction is reached and completes without a fault under the recorded x86-32 decoding model. **All ESP equations additionally require an actual 32-bit SS stack-address attribute.** The live SS descriptor was not queried, and Ghidra's 32-bit program language does not authenticate it. These ESP equations are not asserted for a live 16-bit stack. Qualified arithmetic wraps modulo 2^32.

With entry ESP=T and ESI=V, the initial PUSH writes V to `SS:[T-4]`; the first call writes return address `C05234` to `SS:[T-8]`. No entry relation is carried across that opaque callee. If it returns at the encoded site, call its returned EAX=A1 and ESP=Q1; every other returned register, flag, and memory value is likewise unconstrained.

MOV ESI,EAX followed by TEST ESI,ESI makes the JNZ edge depend on A1. If A1 is nonzero, MOV EAX,ESI retains EAX=A1. POP ESI then reads current `SS:[Q1]`; RET reads current `SS:[Q1+4]`, leaving qualified ESP=Q1+8. These local instructions preserve the TEST arithmetic flags. The popped ESI and return target are not proven entry values.

If A1 is zero, PUSH 16 writes `SS:[Q1-4]`, and the second E8 writes its encoded return site to `SS:[Q1-8]`. The second callee is also opaque. If it returns with ESP=Q2, ESI=R2, and arithmetic flags=F2, POP ECX reads current `SS:[Q2]`, MOV EAX,ESI sets EAX=R2, POP ESI reads current `SS:[Q2+4]`, and RET reads current `SS:[Q2+8]`, leaving qualified ESP=Q2+12. The continuation preserves F2. **R2 is not assumed to preserve zero**, and there is no established relation between Q2, Q1, and T.

Each load/store samples its own execution-time memory. No callee-saved register convention, stack balance, argument type/count, disjointness, service, or original caller restoration is inferred. TEST's arithmetic flag behavior is recorded explicitly in the report; opaque returned flags are never substituted across a call.

## Evidence and retained failures

The eight actual-Git inputs are the accepted ENTRY and ENTRY_PRIMARY doc/report pairs, target configuration, and their exact three-tool contract (`bsp.py`, `ghidra_export.py`, `ghidra_typed_flow.py`). They are frozen at actual Main `a17679bd230751433612cad526c84a1bcb065ebb`. Source subtree IDs are identity only; no Source contents or preceding archives were traversed.

Root independently checked all 92 phase-one manifest members, actual finite Git objects, 16 commands, all 22 original direct metadata responses, and the complete 24-byte owner before admitting phase two. The paired byte reads and closing same-start metadata bring this packet to 22 Native commands, 33 direct typed responses, and six count checkpoints. All four aliases and complete saved owner ranges agree; count remained 64,730 and typed epoch 98. `00C05246`, instruction interiors, neighbors, callees, provider cells, and automatic EXTERNAL scalars were not queried or interpreted.

Two earlier offline failures are retained: the worker's overly strict validation-object schema, and Root's use of a preceding report schema (`saved_analysis` instead of `saved_metadata`). The worker resumed from the same accepted first response without repeating a Native query. Root's failed producer and labelled stdout/stderr transcriptions from its tool result are retained alongside its failure record and corrected gate. Neither failure changed the original captures or GPR. Phase-two collection and manual/Capstone/text decoding passed on their first executions.

The sealed proof contains actual frozen/staged/final Git objects and exactly these two outputs. The physically pinned replay guard allows one supplied ZIP read, one retained helper execution, and one exclusive receipt write after bootstrap. The distinct ZIP copy is checked under the same restriction; final receipts and commit/tree IDs accompany the handoff. Helper bodies are pinned before execution.

There were no GPR mutations, fresh prototypes, decompiles, Source edits, builds, new tests, execution probes, Original, OS audio, runtime, or gameplay runs. Provider behavior, original ABI compatibility, and a Source replacement require separate bounded evidence.
