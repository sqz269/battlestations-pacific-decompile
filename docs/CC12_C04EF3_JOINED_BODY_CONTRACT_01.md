# Conditional joined body contract at 00C04EF3

The complete eight-byte saved body `83 25 a8 ee 09 01 00 c3` was captured twice within one bracketed Ghidra epoch, with live function count 65174 and epoch 10371. Root accepted the capture and the subsequent fresh manual/first-decoder gate. This admits the saved bytes and their conditional 32-bit decode; it does not establish execution mode, runtime instruction fetches, memory mappings, a caller, an ABI, startup or gameplay.

The semantic candidate was pinned before that fresh capture and was peer-reviewed without required changes. It originally composed a seven-byte capture at 65052/7572 with a one-byte capture at 65151/9433. Both that candidate and its original pins remain unchanged. A separate final admission qualification connects the later complete physical evidence to the same conditional model; historical pending-admission fields remain historical records.

The public report binds the initial six-file/ten-object Git freeze, seventeen selected historical originals, fifteen selected capture/peer originals, and the separately pinned final decoder/manual acceptance inputs. It does not recursively expand the Root receipts' transitive copies.

The contract assumes ordinary legacy non-64-bit x86 with default 32-bit operand and address sizes. It describes normal completion with ordinary little-endian RAM, stable applicable descriptors and translations, no intervening writes or asynchronous changes, and successful complete data/stack accesses and return checks. These are model conditions, not observed runtime facts. Fault delivery, partial effects, device memory, interrupts and additional control-protection effects are outside this success contract.

The fixed eight candidate bytes `8325a8ee090100c3` and their stable code translation must remain applicable at the two reached instruction fetch/decode steps. In particular, the AND write may also alias backing memory supplying the RET opcode. The model requires the captured `C3` to remain applicable at its reached fetch/decode; separation of that code byte from the data write is one sufficient condition, not an observed fact. Self-modifying-code behavior is not modeled.

## Ordered memory effects and aliasing

Let `M0` map abstract backing-byte identities to initial bytes. Let `D=0x0109EEA8`, `P=entry ESP`, and `S=P` for `SS.B=1` or `S=P & 0xFFFF` for `SS.B=0`. Define:

- `d_i = backing(DS,D+i)`, `i=0..3`, and `W={d_0,d_1,d_2,d_3}`.
- `r_j = backing(SS,S+j)`, `j=0..3`, and `R={r_0,r_1,r_2,r_3}`.

Backing identity determines aliasing; different segment offsets do not prove disjoint memory. The complete four-byte accesses must be valid. The per-byte notation does not silently wrap bytes of a crossing stack operand modulo 16 bits.

The AND first reads `X = sum(256^i * M0[d_i])`, computes `X & 0 = 0`, and writes zero to its four data bytes. The read remains part of the instruction even with immediate zero. On normal completion, `M1[b]=0` when `b` is in `W`, and `M1[b]=M0[b]` otherwise. This states the final memory effect without promising byte-store order, atomicity or another agent's observations. Neither accepted opcode has a LOCK prefix; no asynchronous memory policy was observed.

The later RET reads the updated memory:

```
Q0 = sum(256^j * M0[r_j])
Q  = sum(256^j * (0 if r_j is in W else M0[r_j]))
```

Thus `W` and `R` disjoint is sufficient for `Q=Q0`. Under this stable ordinary-memory model, preservation holds exactly when every overlapping original return byte was already zero. Full overlap makes `Q=0`; partial overlap zeros only the overlapping return bytes. The four-byte data write can also alias other caller-frame storage, so it proves no general preservation of caller locals or saved registers.

## Flags, stack pointer and destination

Normal AND completion sets `CF=0`, `OF=0`, `ZF=1`, `SF=0` and `PF=1`; `AF` is undefined. Ordinary RET does not change these arithmetic flags or make AF defined.

With `SS.B=1`, RET reads the four-byte offset from `SS:ESP` and sets `ESP=(P+4) mod 2^32`. With `SS.B=0`, it reads four bytes from `SS:SP`, updates `SP=(SP+4) mod 2^16`, and preserves the upper ESP bits in this ordinary instruction model. Operand size still makes the popped offset four bytes; stack-address size selects the stack pointer. There is no encoded extra stack cleanup and no CS selector pop.

RET loads `EIP=Q`, a CPU code-segment offset. Runtime linear destination is the applicable `CS.base+Q`, subject to checks and mapping. A saved listing address is a separate coordinate. If `phi(L)` maps listing address `L` to runtime linear code, then `rho(L)=(phi(L)-CS.base) mod 2^32` is its conditional CPU offset. `00C04EFA` is a listing successor after AND, and `00C04EFB` is an end-exclusive boundary, not a RET successor.

The accepted historical CALL at listing `00BF6707` has bytes `e8 e7 e7 00 00`: its listing next address is `00BF670C`, and adding signed displacement 59367 gives `00C04EF3`. If that CALL actually executes under a compatible mapping, it pushes `rho(00BF670C)`, not automatically the number `00BF670C`. Identifying `Q0` with that value needs actual incoming-call and stack/frame evidence; identifying `Q` with it also needs return-slot preservation through the AND. Neither was observed here.

Neither opcode assigns EAX in the ordinary local model. That does not recover an EAX result, a declared return type, a `void` signature, a callee-preservation contract or any C/C++ ABI. RET does not automatically restore EBP or other saved general-purpose registers.

## Unobserved concrete models

For these paper models only, choose `SS.B=1`, zero DS/SS/CS bases, ordinary RAM with identity backing, valid flat segment ranges, disabled alignment checking, no extra control protection or interference, and captured code separate from data and stack. Each stated return destination is mapped as valid executable code. These are possible conditions, not Windows/game runtime observations. Set the initial return word to illustrative `00BF670C` (bytes `0c 67 bf 00`); no actual push of that value is claimed.

| Model | Entry ESP | Return bytes overlapped by data at 0109EEA8..0109EEAB | Popped EIP | Final ESP |
| --- | --- | --- | --- | --- |
| Disjoint | 02001000 | None | 00BF670C | 02001004 |
| Full alias | 0109EEA8 | 0, 1, 2, 3 | 00000000 | 0109EEAC |
| Partial alias | 0109EEA7 | 1, 2, 3 | 0000000C | 0109EEAB |

In the full-alias model, the AND reads and zeros the entire return word; offset zero is explicitly a valid hypothetical return target. In the partial model, `0c` at `0109EEA7` survives while the following three return bytes are zeroed. Its unaligned four-byte stack read is explicitly valid. Both show that an otherwise valid local sequence can change the return offset, even if a later caller contract were to establish an actual pushed initial value.

## Boundary

This packet performs no Native/context/target/provider/return-slot query, decoder import, Source body read or absence census, C++ edit, build, test, probe, runtime action, GPR write, archive creation or old-helper execution. It supplies no ABI, startup or gameplay validation. The later same-epoch byte/decode gate is accepted at the recorded capture epoch; the execution-model assumptions remain unverified. Two bounded-inspection display truncations are retained separately in the candidate and admission records. No current helper, capture or decoder failure occurred in this contract packet through final-admission preparation.
