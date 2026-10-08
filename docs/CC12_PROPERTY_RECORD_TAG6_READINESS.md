# Tag-6 property clone mapping audit

Tag 6 selects table cell `008F52D0`, whose actual DWORD points to `008F50ED`.
The complete selected arm is `[008F50ED,008F5134)`: 71 bytes, 21 instructions,
through its normal `RET`. It calls `008F41F0` and passes that returned EAX word
to the direct call at **`008F5118 -> 008EF780`**. `008F5115` is the preceding
`PUSH EAX`, not the call instruction.

**Source 0, ready Source 0, new Source packets 0.** This is a mapping and
caller-transport audit. Both callees belong to the active external lease
`agent/cc12_property_payload:cc12_property_owning_producer_design`, expiring
`2026-10-08T22:52:04+00:00`. Its worktree is
`J:/PROG/battlestations-pacific-decompile-cc12_property_payload`. The primary
integrator reserved callee inspection for that stream. No callee body was
read or substituted, and no implementation proposal follows from this audit.

## Indexing and exact bytes

The frozen earlier clone-entry contract loads the raw source tag from `+4`,
compares it unsigned with `0B`, and jumps through `[008F52B8 + tag*4]` without
an index adjustment. That contract previously witnessed tag 8 at `008F52D8`.
It therefore establishes tag 6 at `008F52B8 + 6*4 = 008F52D0`. This audit
reuses that frozen evidence; it does not re-query the old entry, tag-8 arm or
112-byte type-8 constructor.

The freshly read cell is `ED 50 8F 00`, SHA256
`2a4f80811fc7828978c24e593c616de77b63ef77719439d54c4a0f0e72ba0aad`.
The selected arm SHA256 is
`e13774b4eac8a8c1f666445c908ad604d586777101dac0eec941639a0d0d44c6`.
The arm has two local basic blocks: `[008F50ED,008F510D)` and
`[008F510D,008F5134)`. Its sole conditional branch leaves this owned span
for `008F528A`; that external allocation-null tail is named but not expanded.

All 75 owned bytes match the installed PE and live Ghidra before and after
inspection. All 21 instruction starts and operands agree gaplessly. Each
batch checks the configured `C:/Users/sqz269/bsp.gpr`, the installed PE hash,
and the actual live project/program/language/base through the standard
`bsp.py` verification path. The PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No Ghidra mutation or listing repair occurred.

## Observed control and data transport

| Address | Actual operation and its limit |
| --- | --- |
| `008F50ED` | Push allocation size `38h` |
| `008F50EF` | Call original private heap entry `00BF681B` |
| `008F50F4..50F9` | Move returned EAX to EDI, remove size argument, save EDI in caller scratch slot |
| `008F50FD..5107` | Test EDI, store state 2 without changing flags, branch to `008F528A` if zero |
| `008F510D` | Load ECX from source `[ESI+C]` |
| `008F5110` | Call `008F41F0`; its body and ownership contract are externally reserved |
| `008F5115..5118` | Push returned EAX, move allocated EDI to ECX, call `008EF780` |
| `008F511D..5121` | Read source `+34`, restore EDI, write that word through returned `[EAX+34]` |
| `008F5124..5133` | Restore ESI and old FS link, add `10h` to ESP, plain RET |

The arm does not pass a byte count or a copy flag to `008EF780`. The single
new stack word comes directly from `008F41F0`'s EAX. The source `+C` pointee,
producer output allocation, destination constructor layout and ownership
remain unverified. In particular, the arm writes the ordinal through returned
EAX; equality of that EAX with the earlier allocated EDI requires the actual
callee contract. A historical class name or a zero-callee index entry does
not supply that contract.

## Caller-side stack requirements

Let `S` be the clone's original entry ESP and `A=S-24` the selected arm's entry
ESP, derived from the frozen 45-byte entry. The saved slots are state at `S-4`,
handler `00CA4B6D` at `S-8`, old FS at `S-12`, scratch at `S-16`, saved ESI at
`S-20` and saved EDI at `S-24`. The arm writes state 2 and stores allocated
EDI into that scratch slot.

The allocation size push reaches `S-28`, and the allocation CALL enters at
`S-32`. An ordinary CDECL return followed by the observed caller `ADD4`
restores `S-24`. The call at `008F5110` has no explicit stack argument; normal
continuation requires it to return to that balance.

The subsequent `PUSH EAX` reaches `S-28`, and the call at `008F5118` enters at
`T=S-32`, with the new word at `[T+4]`. The visible continuation requires ESP
back at `S-24`; under an ordinary x86 return this requires `RET4`. This is a
caller-side requirement, **not a recovered RET opcode or callee ABI proof**.

Given that required balance, POP EDI reaches `S-20`, POP ESI reaches `S-16`,
and `[ESP+4]` supplies the saved FS link from `S-12`. Final `ADD16` reaches S,
then plain RET reaches `S+4`. EAX is left as the callee result, EDX becomes
the source ordinal, and ECX becomes the old FS link. Final arithmetic flags
would come from `(S-16)+16=S`; no transitive register, FP, ES or DF guarantee
is admitted. There are no local x87 or virtual-call instructions in the arm.
Any such transitive callee contract remains unexamined.

## Deferred prerequisite and preservation

The exact prerequisite is the owning-producer stream's eventual whole genuine
evidence for `008F41F0` and `008EF780`: complete control flow, physical ABI,
actual allocation/lifetime/free ownership and any class/virtual dependencies.
The private heap, allocation-null tail, default target `008F52A3`, EH handler,
whole clone and World remain named incomplete boundaries. The graph records
10 nodes and 12 edges, within the 24-item budget. It invents no virtual target
and does not replace the reserved candidate with another constructor.

The selected address is inside `BSP_SceneProperty_Clone`, not a function entry.
The proto tool returns the enclosing header and an unavailable-signature
diagnostic for `008F50ED`; both before/after outputs have exit code zero and
are identical. The existing enclosing comments are also preserved exactly.
Historical annotations are retained as metadata, not promoted to fresh body
or class proof.

The refused candidate lease and its exact owner/expiry are preserved. An
offline helper initially formed `arm_proto_before.stdout.txt` instead of the
captured `arm_proto_before_stdout.txt`. Its FileNotFoundError and three valid
partial binary outputs are retained. A new syntax-checked helper verifies
those outputs without rewriting them and completes the mapping. No Ghidra
query, successful phase or old native process was replayed for this repair.

The fresh worktree starts at `35a834a227c3e04de082241124e758ec120ada35`.
Its ignored `local/t6r` family preserves all 5622 prior artifacts: the type-8
family's 383 files including its seal, plus its 5239 earlier pins with their
original associations. Type-8 Source credit remains historical/pending and
supplies no dependency admission here. Seven strict input files and 19 frozen
context copies are checked; mutable main metadata and the lease registry are
snapshots, not falsely pinned live after the audit.

The report gives the exact family inventory, manifest hash, bookends, graph,
deferred lease and failure receipts. Only this document and its report are
committed. There is no C++, compiler, native probe, Source registration,
shared ledger change, Ghidra mutation, startup test or gameplay claim.
