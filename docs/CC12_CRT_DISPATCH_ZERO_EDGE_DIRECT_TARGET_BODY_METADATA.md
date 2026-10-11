# CRT dispatch zero-edge direct target body at `00BFBA09`

The saved body is exactly 36 bytes, with nine physical x86-32 instructions. Three direct calls are followed by a register-indirect call and a possible local return path. Every callee remains opaque; the saved label `__amsg_exit` does not establish a service, provider, nonreturn behavior or original ABI.

| Address | Exact bytes | Physical instruction |
| --- | --- | --- |
| `BFBA09` | `E8 5A A1 00 00` | `CALL C05B68`, rel32 `+41306`, encoded return `BFBA0E` |
| `BFBA0E` | `FF 74 24 04` | `PUSH dword ptr SS:[ESP+4]`, operand address uses pre-push ESP |
| `BFBA12` | `E8 91 9F 00 00` | `CALL C059A8`, rel32 `+40849`, encoded return `BFBA17` |
| `BFBA17` | `FF 35 80 55 E1 00` | `PUSH dword ptr DS:[00E15580]`; cell value unqueried |
| `BFBA1D` | `E8 BC 95 00 00` | `CALL C04FDE`, rel32 `+38332`, encoded return `BFBA22` |
| `BFBA22` | `68 FF 00 00 00` | `PUSH imm32 255` |
| `BFBA27` | `FF D0` | `CALL EAX`, unresolved current register target, encoded return `BFBA29` |
| `BFBA29` | `83 C4 0C` | `ADD ESP, sign_extend(0x0C)` |
| `BFBA2C` | `C3` | Near `RET` |

The complete code SHA-256 is `bed75f9fd672a6ebd1392287ceb50aaec8eee6903ea335fdd8b387c14c044da1`. Independent manual decoding checks every opcode, group/ModRM, SIB, displacement and immediate field and operand width/access against retained actual detailed Capstone 5.0.7 output. Both exact byte reads and both context-zero text batches agree. The immediate at `BFBA22` is a four-byte 255; it is not an imm8 sign extension of `-1`. The indirect call reads EAX, without another memory dereference.

## Metadata gate and capture bounds

The frozen actual Main commit is `8af0944e2d639e46a4eaaa0ff298328e69f89700`. Two independent metadata walks restart at `BFBA09` and advance using only each walk's just-captured exact instruction length. Both cover the nine complete units through `BFBA2C`, without gaps or overlaps. All four aliases at every start identify actual `InstructionDB` entries. Complete owner metadata records one 36-byte range, `no_return=false`, nonthunk, flow override `NONE`, matching default/effective flow/targets and unoverridden fallthroughs. Saved call and terminator flags agree with the physical instructions; the flags do not prove callee behavior.

Root's first gate execution passed after checking the 108 phase-one preimages, both original postexecution streams, 14 actual Git objects and eight exact input files. Only then were paired 36-byte reads, paired context-zero text at nine starts and closing exact-unit metadata captured. All 27 original typed response bodies match by instruction start. Across both phases there are 29 Native commands and six count checkpoints, all at epoch 98/count 64,730 in `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. GET provenance comes from the actual frozen `fetch_raw` implementation's explicit request method and retained URLs, without a separate wire telemetry claim.

The next cursor `BFBA2D` was never queried. No prototype, decompilation, neighboring instruction, callee, descriptor, provider or external cell read was admitted. Automatic companion objects remain uninterpreted metadata. All 115 Root producer/receipt/stream/manifest/preimage files are retained.

## Current-state CPU contract

Each statement assumes the named instruction is reached and completes without fault under x86-32 decoding. **Every implicit-stack ESP formula requires the current SS stack-address attribute to be 32-bit at that instruction.** Program language alone does not prove it. Every opaque return makes registers, flags, segments/descriptors, stack attributes and memory unconstrained again. With 16-bit SS, the stated implicit-stack ESP formulas are not asserted. Explicit `ADD ESP,12` is 32-bit register arithmetic independent of SS stack size. Qualified arithmetic wraps modulo 2^32.

For current entry `ESP=T`, the first E8 stores `BFBA0E` at current `SS:[T-4]`, sets `ESP=T-4` and enters numeric `C05B68`. It locally preserves other general registers and arithmetic flags. This local fact supplies no preservation across the callee.

If execution resumes at `BFBA0E` with new unknown `ESP=Q1`, PUSH first reads the current dword `V1` from `SS:[Q1+4]` using the old ESP, then writes `V1` at `SS:[Q1-4]`. The following E8 stores `BFBA17` at `SS:[Q1-8]` and enters `C059A8` with `ESP=Q1-8`. `V1` is not authenticated as an original argument or entry-stack value.

If execution resumes at `BFBA17` with new unknown `ESP=Q2`, PUSH first reads current dword `V2` from `DS:[00E15580]`, then writes it at `SS:[Q2-4]`. The next E8 stores `BFBA22` at `SS:[Q2-8]` and enters `C04FDE`. Only the cell's encoded address is established. Its contents, type and provider remain unqueried. Potential aliasing never justifies substituting a past read for the current operand read before the stack store.

If execution resumes at `BFBA22` with new unknown `ESP=Q3`, `EAX=A3`, PUSH writes 255 at `SS:[Q3-4]`. `CALL EAX` then stores `BFBA29` at `SS:[Q3-8]` and enters the unresolved current target `A3`. These local instructions preserve other general registers including EBP and arithmetic flags; the indirect callee has no established preservation.

If execution resumes at `BFBA29` with new unknown `ESP=Q4`, ADD sets `R=(Q4+12) mod 2^32` and `ESP=R`, without accessing memory. It sets CF from unsigned carry, ZF from zero, SF from bit31, PF from low-byte parity, AF from `((Q4 xor 12 xor R)&0x10) != 0`, and OF from `((~(Q4 xor 12)&(Q4 xor R))&0x80000000) != 0`. RET then reads unknown next EIP from current `SS:[R]` and, with current SS stack-address size 32, leaves `ESP=R+4`. RET preserves the ADD arithmetic flags. Neither this stack value nor the next EIP is authenticated as a restored caller state.

The accepted caller zero edge can supply `EAX=ESI=0` at initial `BFBA09`; the first local E8 carries those values into `C05B68` before that opaque callee executes. No zero value, old stack slot, segment attribute or other state is propagated through any opaque return. Argument counts/types, calling convention, cleanup convention and return behavior remain unproved.

## Finite replay and closure

The eight input files are the accepted ENTRY and PRIMARY doc/report pairs, `config/target.json`, and the three CLI tools `bsp.py`, `ghidra_export.py`, `ghidra_typed_flow.py`. Actual commit/root/directory/blob payloads establish them, including exact working-file equality. `src` and `include` tree IDs are identity-only; no Source payload or census and no prior archive recursion is included.

The core ZIP and a distinct copy passed on their first executions. The strong audit denies every unknown event, records every rejection and checks `denied=[]` even if an exception could be swallowed. Each run performed exactly one archive read, one supplied helper execution and one exclusive receipt write, with zero denied events. Actual Capstone decoding ran separately in a pinned producer; its complete result is inert data compared with the independent manual fields in pure replay. The pure helper neither imports nor constructs Capstone, and never executes archived producers.

After this public commit, the finite delivery proof adds actual final Git objects and the two output blobs, verifies exactly two changed paths and unchanged input/Source identities, and repeats original/copy strict replay. All evidence is under `local/cc12_crt_dispatch_zero_edge_direct_target_body_metadata/`. This packet has zero Native, capture, decoder, Root gate or core replay failures.

Public outputs are this document and its [JSON report](../reports/cc12_crt_dispatch_zero_edge_direct_target_body_metadata.json). No Source implementation, C++ build, new test, probe, GPR mutation, Original ABI compatibility, game validation, OS audio or runtime acceptance is claimed.
