# Compact resource writer predecessor boundary

The approved prefix establishes immediate ECX provenance for the retained
`00CD8688` store to `0109042C`: after calling `006FAC20`, the selected code reads
old `[EAX+4]`, writes its wrapping increment back there, then the known store
publishes the old value. This supports a minimal ordinary Source publication
fragment proposal. It does not establish an initializer or original owner/ABI.

Baseline: `f49eea54c5f7071d9eafee27d32e743fcbfd106b`. The only tracked changes
are this document and `reports/cc12_compact_resource_writer_predecessor_boundary.json`.
Complete receipts and frozen inputs are under
`local/cc12_compact_writer_predecessor_boundary/`.

## Metadata gate and bounded inspection

The lease covers known target `0109042C`, the known six-byte writer
`00CD8688..00CD868D`, and the approved physical prefix
`[00CD8679,00CD8688)`. Initial exact typed metadata queried the target, writer,
and prefix start. The current target is a defined four-byte `DataDB` code unit;
no target value or data bytes were read. The writer is an exact six-byte
`InstructionDB`, with matching default/effective fallthrough to `00CD868E`,
override `NONE`, and no fallthrough override. Its physical bytes were not reopened.

At `00CD8679`, exact instruction is null and the containing instruction starts
at `00CD8674`, length six. That outside start was returned as metadata only.
The first byte of the approved prefix is therefore a tail byte. It was compared
but not decoded; no preceding bytes were opened.

Exact typed metadata for every prefix address identifies four whole instructions
inside the envelope: `00CD867A` length five, then `00CD867F`, `00CD8682`, and
`00CD8685`, each length three. Only their 14-byte suffix was decoded, each
separately from its verified start and length. No similar-family pattern was
used to choose or infer the instructions.

Every queried exact/containing function is null, with complete empty function
records. The prefix remains unowned. No nearest-function inference, pseudocode,
whole-body listing, repair, prototype or function creation was used.

## Exact physical result

The approved 15 bytes are `01e8a125a2ff8b48048d5101895004`. Original PE and
Ghidra bytes match exactly, SHA-256
`409a1f3b199b73a8db2c6375f0a5110359b2f019b67f732ecf0c578638c35afe`.

The PE extraction reused the retained `.text` mapping in the SkinModel
predecessor report: `00CD8515` -> file offset 9,274,645. The approved Compact
start maps to offset 9,275,001. The selected read was one unbuffered seek/read
of exactly 15 bytes. No new PE header was parsed or another semantic window
opened. A separately authorized identity-only whole-file hash was recomputed:
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The original size and modification timestamp match the retained identity and
remain unchanged across hashing and the selected read. Whole-file hashing did
not decode or interpret additional bytes.

| Address | Length | Verified instruction |
| --- | --- | --- |
| `00CD867A` | 5 | `call 006FAC20` |
| `00CD867F` | 3 | `mov ecx, dword ptr [eax+4]` |
| `00CD8682` | 3 | `lea edx, [ecx+1]` |
| `00CD8685` | 3 | `mov dword ptr [eax+4], edx` |

On normal continuation, let `p` be EAX after the call and `n` the 32-bit value
read at `p+4`. ECX retains `n`; EDX receives `n+1` modulo 2^32; that increment
is stored at `p+4`. The previously accepted `MOV [0109042C],ECX` at `00CD8688`
then publishes `n`. Neither LEA nor the increment store changes ECX.

This is conditional local behavior: the call and accesses must complete.
The identity, extent, lifetime and original ABI of `p` remain unproved here.
Counter-word/target separation is not inferred. If `p+4` and `0109042C` alias,
the final known store overwrites the increment with the old value.

## Flow, identity, and Source qualifications

At `00CD867A`, default and effective flow are `UNCONDITIONAL_CALL` to
`006FAC20`, with fallthrough `00CD867F`, override `NONE`, and no fallthrough
override. Automatically returned target metadata records the current descriptive
name `BSP_TypeId_GetCounterSingleton`, `no_return=false`, `is_thunk=false`, and
null direct thunk target. No callee body, prototype, bytes or data were opened.
The three subsequent instructions have matching `FALL_THROUGH` flow, no target
arrays or overrides; the last falls through to the known writer. Those metadata
flags are not proof of Native return behavior or ABI.

All 26 raw typed responses pass strict offline replay at modification `10`,
with actual `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`, and `ram` address space. Every
response and batch has matching before/after modification numbers; repeated
payloads agree. The byte command retained all five full raw HTTP responses
before parsing, including identity checks before/after. The final eight typed
responses bracket the physical observation at the same modification number.
All 31 raw response hashes replay. This describes stable database observations,
not running-game memory. Runtime Java CodeSource remains unattested.

Complete current Source files and baseline Git versions are frozen. The
existing `TypeIdCounterLifetime::get_006fac20` and `TypeIdCounterStorage.next_id_04`
provide the genuine shared Source contract. The existing postprocess context
borrows `compact_type_0109042c`; `B79BC0`'s Source passes that same cell's address
to its compact range. The instance-postprocess header associates the
`B78750/B922C0` typed view with `0109042C`. These consumer/provider contracts
do not create new backing, initialization, or Native callee evidence.

Compact remains distinct from camera `01090288`, SkinModel `01090344`,
skined-mesh resource `01090454`, and excluded `01090370`. Prior camera and
SkinModel artifacts remain unchanged. No other stream's files or addresses
were edited or opened for new Native analysis.

## Minimal Source proposal only

A future ordinary void Source fragment can cover the retained 20-byte local
sequence `00CD867A..00CD868D`. It should borrow the actual current
`volatile uint32_t` word at `0109042C` and the SAME genuine
`TypeIdCounterLifetime`, call its real getter, capture old `next_id_04`, store
the wrapping increment to the same returned counter, then store the captured
old value to the target last. Both stores must be volatile and ordered even
when their words alias. Private Source contexts/locals/bindings must not alias
those storage cells, and the supplied bindings/lifetimes must remain valid.

No Source implementation is added here. The proposal carries no original
argument/return-register ABI, guard, descriptor layout/name/parents, complete
initializer, owner, production backing/caller, or CRT claim. Whole-function and
game equivalence remain held. No further Native scope is proposed by this
packet; the partial instruction before the call is outside this local proposal.

There were no C++/CMake/ledger/provider/configuration edits, GPR mutations,
POST/script endpoints, restart, builds, tests, probes, links, or runtime execution.
