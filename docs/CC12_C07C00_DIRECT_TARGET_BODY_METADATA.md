# C07C00: complete body metadata and physical evidence

Root admitted exactly `00C07C00..00C07C44` after independently reviewing both metadata walks. Paired exact byte ranges, two context-zero text captures, a third complete typed pass, the complete manual decode, and fresh offline Capstone agree on **69 bytes and 21 instructions**. The saved `__SEH_prolog4` name remains unauthenticated metadata.

The current frozen base and observed Main are `7f17b3d462f9d2de5e94df22a04b855b73d702d9`. Eight positive inputs have exact committed-blob, index and working-LF equality. The selected current baseline contains 14 actual Git objects; `src` and `include` are retained only as subtree identities, with no Source contents read. A separate successful earlier freeze captured 15 objects when Main advanced; its original producer, pins, command streams and object data remain intact. That baseline retry is not a functional failure.

Root's original admission and all **578 initial artifacts**, including **567 worker preimage copies**, were verified and copied before any body byte or text query. Later independent Root manual/decoder artifacts are outside that initial copy count. The first successful metadata seal, its original counters, the supplemental handoff seal and subsequent inspection history remain separate.

| Address | Physical bytes | Saved context-zero instruction |
| --- | --- | --- |
| `00C07C00` | `68907CC000` | `PUSH 0xc07c90` |
| `00C07C05` | `64FF3500000000` | `PUSH dword ptr FS:[0x0]` |
| `00C07C0C` | `8B442410` | `MOV EAX,dword ptr [ESP + 0x10]` |
| `00C07C10` | `896C2410` | `MOV dword ptr [ESP + 0x10],EBP` |
| `00C07C14` | `8D6C2410` | `LEA EBP,[ESP + 0x10]` |
| `00C07C18` | `2BE0` | `SUB ESP,EAX` |
| `00C07C1A` | `53` | `PUSH EBX` |
| `00C07C1B` | `56` | `PUSH ESI` |
| `00C07C1C` | `57` | `PUSH EDI` |
| `00C07C1D` | `A19055E100` | `MOV EAX,[0x00e15590]` |
| `00C07C22` | `3145FC` | `XOR dword ptr [EBP + -0x4],EAX` |
| `00C07C25` | `33C5` | `XOR EAX,EBP` |
| `00C07C27` | `50` | `PUSH EAX` |
| `00C07C28` | `8965E8` | `MOV dword ptr [EBP + -0x18],ESP` |
| `00C07C2B` | `FF75F8` | `PUSH dword ptr [EBP + -0x8]` |
| `00C07C2E` | `8B45FC` | `MOV EAX,dword ptr [EBP + -0x4]` |
| `00C07C31` | `C745FCFEFFFFFF` | `MOV dword ptr [EBP + -0x4],0xfffffffe` |
| `00C07C38` | `8945F8` | `MOV dword ptr [EBP + -0x8],EAX` |
| `00C07C3B` | `8D45F0` | `LEA EAX,[EBP + -0x10]` |
| `00C07C3E` | `64A300000000` | `MOV FS:[0x0],EAX` |
| `00C07C44` | `C3` | `RET` |

All four listing aliases independently identify each exact `InstructionDB` start and length. The owner record is complete over one 69-byte range, with saved `no_return=false`, `is_thunk=false`, recursive-thunk=false and null direct thunk target. All three complete direct payloads agree at each start. Default and effective flow/target/fallthrough fields are independently retained: 20 `FALL_THROUGH` units followed by the `C07C44` `TERMINATOR`, with no override or targets. The derived end-exclusive cursor `C07C45` was not queried.

The physical `RET` derives its destination from the current stack word. The saved TERMINATOR boolean and absence of a metadata target do not authenticate that destination or a calling convention.

## Per-instruction state boundaries

Every one of the seven `PUSH` operations and the final `RET` requires a fresh predicate: actual current SS has 32-bit stack addressing, the stated 32-bit operand decoding applies, and the instruction completes normally. With current ESP `S`, PUSH reads its selected operand before storing four little-endian bytes at `SS:mod32(S-4)` and updating ESP; RET reads the current `SS:S` word for near EIP and advances ESP to `mod32(S+4)`. These are local predicates, not inferred runtime attributes.

The `64` prefix at `C07C05` selects FS for the explicit source read, while the implicit PUSH store uses current SS. At `C07C3E`, `64 A3` writes current EAX to FS offset zero. The actual descriptors, bases and memory translations are unknown. FS:0 can alias SS locations, including the then-current return word; the final RET therefore does not prove an original caller return destination. All memory loads consume current contents after any preceding potentially aliasing stores.

The ESP/EBP SIB and displacement fields are decoded explicitly in the report. `LEA` computes offsets without memory reads or segment translation. Writing EBP at `C07C14` does not establish a frame or register ABI. `SUB ESP,EAX` uses current values modulo 32 bits. Its arithmetic flags and the XOR flag effects are kept separate from Capstone's saved register/group/eflags metadata.

`C07C90` is only a pushed numeric immediate, `E15590` only an encoded DS offset, and FS:0 only an encoded segment offset. No cells, providers, prototypes, cookie identity, exception chain, library prologue identity, OS service, caller argument roles or fault behavior were queried or authenticated. There is no CALL or conditional branch in the admitted bytes.

## Reproducible evidence and limits

The 73 original Native CLI commands comprise 63 complete typed payloads, two exact byte reads, two context-zero batches over all 21 starts, and six count brackets. Project/program verification and all three CLI physical-producer before/after checks are retained for every command. Function count stayed **64730** and modification number **98**. No GPR mutation, Source read, build, test, probe or runtime/game check occurred.

The manual producer and all physical fields were completed and pinned before fresh Capstone 5.0.7 execution. Every instruction agrees on prefixes, opcode, lengths, ModRM, SIB, displacements, immediates, operand size/order and addressing fields. Capstone's ancillary metadata is retained separately. Root performed its own independent decode, which will be covered by its separate PRIMARY review.

The finite evidence ZIP and its distinct physical copy each passed the fully read, pinned pure replay on the first execution. The hook admitted exactly one supplied-ZIP read, one execution of the exact precompiled helper and one exclusive receipt write; every other audited event was denied, with the retained denial list empty. No Capstone execution, import, process, socket, or external evidence read occurred under the hook. The ZIP includes no nested archives.

- Evidence archive: **15578026 bytes**, SHA-256 `d5c8af52b63971f30c62b3a2d62782f8d922e9b36f08ad55e1895d4c880ffed5`; **1354 members**.
- Replay: **38215 bytes**, SHA-256 `c4b4977b6fc945c4c8a81128a1d879fdd0745f7769db12dc139eb54147249bd5`.
- Guard: **3325 bytes**, SHA-256 `49aa2f8c102ba0fd32d38d7ace97a8041d17fdfd3ca7bbb7d6cecbb8c44bdb1e`.

Current execution failures are zero for Native capture, offline producers and strict replays. The worker retains one read-only path-inspection failure and three display truncations; Root retains one separate display truncation. These affected tool projections, not original captures or streams. The original zero-count seal remains untouched and its later supplements state their counter scopes. A draft admission-hash literal was corrected during physical helper review before the first replay execution; its preimage and successful authoring records are retained. The successful 15-object pre-capture freeze and accepted-positive histories are counted separately.

The final delivery contains exactly this document and [the complete report](../reports/cc12_c07c00_direct_target_body_metadata.json), along with original commit streams and actual final Git objects. The current selection is 14 baseline objects plus six new output objects; this does not claim that the full archive, including retained historical copies, contains only 20 Git objects. Eight positive inputs remain unchanged. This packet establishes bounded static evidence, not an implementation, drop-in ABI, library/provider identity or runtime equivalence.
