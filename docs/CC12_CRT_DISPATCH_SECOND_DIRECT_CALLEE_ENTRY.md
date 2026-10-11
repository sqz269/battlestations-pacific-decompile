# Gated first instruction at 00C059A8

The saved instruction at `ram:00C059A8` is the single byte `51`, decoded as
`PUSH ECX`. This packet establishes that entry instruction and its conditional
current-stack effect. It does not establish the rest of the function, a library
service, a parameter role, a calling convention, or whether the function returns.

## Positive input and admission

The frozen base is `3c85f1c584331ddffd68b31d2f4305c735c15209`. Eight positive
tracked inputs comprise the accepted `00BFBA09` body document/report and their
PRIMARY document/report, `config/target.json`, and the existing
`tools/bsp.py`, `tools/ghidra_export.py`, and `tools/ghidra_typed_flow.py`.
The accepted body supplies the physical edge at `00BFBA12`: `E8 91 9F 00 00`.
Its signed displacement is `+40849`, so `00BFBA17 + 40849 = 00C059A8`.
That calculation establishes a numeric direct target only.

Fourteen actual commit/tree/blob objects authenticate those inputs, their tree
joins, working-file LF normalization, index blobs, and observed-main ancestry.
Main is allowed to advance after capture. The `src` and `include` subtree IDs
are retained as identity only; their contents were not read.

Phase 1 used count, exact-address typed metadata A/B, then count. Root independently
validated the complete original responses and admitted precisely one byte plus
context-zero text at `00C059A8`. The first Root gate execution passed. Its receipt
is 1,797,876 bytes, SHA-256
`64c0aed1518d7b7a1dedb49f970f97a274eb2c38f42ae31cb6b27d7ccf5a71de`.
The worker physically copied all 170 actual Root artifacts, including 164 original
preimage copies, rather than assuming a template count.

## Saved metadata and bounded captures

All three typed responses preserve the complete original direct payload, wrapper,
raw text, base64, digest, HTTP result, before/after project identity, and original
CLI streams. Each direct payload is 2,720 bytes, SHA-256
`788606ddf5b30f4f4dc3bcf59c9e14ed34ec6c7fb4b408260c71b18ef8efd286`.
The project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
language `x86:LE:32:default`, image base `00400000`, default address space `ram`.
Modification number remained 98 and function count remained 64,730.

The four aliases—`instruction_at`, `instruction_containing`, `code_unit_at`, and
`code_unit_containing`—all return the exact one-byte `InstructionDB` at
`ram:00c059a8`, kind `instruction`, with `defined: null`. Flow override is `NONE`;
default and effective flow are `FALL_THROUGH`, the fall-through is unoverridden
`ram:00c059a9`, and both target lists are empty. That next cursor was not queried.

The at-query and containing owner are both `ram:00c059a8`. Its saved name is
`LIBCRT_unmatched_00c059a8`, with `no_return: false`, `is_thunk: false`,
`thunk_target_recursive: false`, and no direct thunk target. These are saved
metadata, not authenticated library identity or return behavior. The complete
owner AddressSet contains six ranges, totalling 399 addresses:

| Inclusive saved range | Address count |
| --- | ---: |
| `00C059A8..00C05A29` | 130 |
| `00C05A2D..00C05A6D` | 65 |
| `00C05A71..00C05AB0` | 64 |
| `00C05AB6..00C05AD4` | 31 |
| `00C05AD8..00C05AF6` | 31 |
| `00C05AFA..00C05B47` | 78 |

The automatic ranges were retained only as metadata. Their bodies and gaps were
not walked. The function is not treated as a contiguous 399-byte interval.

Phase 2 used count, two exact one-byte reads, two exact-address text reads with
context zero, closing typed metadata, then count. The seven phase-2 commands and
all eleven Native CLI commands passed on their first actual executions. Every
command has all three existing physical CLI producers pinned immediately before
and after execution; all were unchanged. No prototype or neighboring address was
queried. Metadata uses the existing GET producer; text uses the existing read
interface's POST endpoint. These are producer-based method facts, not a separate
network trace.

Both byte captures equal `51`, SHA-256
`4ae81572f06e1b88fd5ced7a1a000945432e83e1551e6f721ee9c00b8cc33260`.
Both original text responses are byte-identical, 157 bytes, SHA-256
`a1b2121fbc9920c736ad079f22862aeaa62e44d8d437156b1b1a8a7bb73ed0cd`.
They report only `PUSH ECX`, with empty before/after context lists.

## Offline instruction and current-stack relation

An actual offline Capstone 5.0.7 x86/32 decode consumed only the admitted byte.
The original decoder stdout, record, producer prepin, module hash, and loaded DLL
hash are retained. It returned one one-byte `push ecx`, a four-byte ECX operand,
reads of ESP and ECX, a write to ESP, and no arithmetic EFLAGS change.
The pure replay validates this retained decode; it does not import Capstone.

The manual decode is independent: `51 = 50 + 1`; the low three opcode bits select
ECX from the x86 register table. There is no prefix, ModR/M, SIB, displacement,
or immediate. The operand is 32 bits in the admitted decoding mode.

Conditional on normal completion with **current SS and 32-bit stack addressing**,
let `S` be the incoming ESP and `C` the incoming ECX. Then:

```text
ESP' = (S - 4) modulo 2^32
current SS:ESP' receives LE32(C)
ECX and arithmetic EFLAGS are unchanged by this instruction
```

This relation does not authenticate a live SS descriptor, memory mapping,
exception outcome, segment aliasing, earlier-call register preservation, or any
argument/temporary-save purpose. `00C059A9` is only the arithmetic next cursor and
saved fall-through. The remaining 398 owner bytes remain unqueried.

## Proof, original failures, and delivery boundary

The finite evidence archive and its distinct physical copy each contain 400
members and are 6,968,747 bytes, SHA-256
`6d93a1e09a9c65e12a5d41e946fe48ac872198525c33cf7c43f82e1ef314e9b3`.
Both strict child executions passed first time: exactly one archive read, one
execution of the exact pinned helper code object, and one exclusive receipt
write; `unexpected_denials` is empty. Every other audit event is denied.
The complete 23,052-byte replay helper and 3,325-byte guard were physically read
and pinned before execution. The replay validates every member pin, actual Git
objects/tree joins, complete metadata, CLI argv/streams and physical producer
pins, all Root copies, manual fields, actual prior decode, and failure provenance.
No earlier proof archive is an input.

Preparation had three disclosed incidents. One bounded inspection command omitted
the `rg --` option terminator and exited 1. One initial admission helper treated
Root's preimage dictionary as a list and raised TypeError before any phase-2
Native query. Its exact original producer was copied before correction, and its
original prepin, result, empty stdout, and 552-byte stderr remain retained. A
unique corrected invocation passed after inspecting the actual dictionary schema.
One combined inspection display was capped by 149 tokens; the missing physical
producer portion was read in a smaller chunk before execution. The two shell/tool
inspection incidents have exact-command incident records and conversation-output
provenance, not invented separate original stdout/stderr files. Root gate and
Root inspection failures were zero. These incidents do not alter the Native or
strict replay first-pass counts.

The public report pins this document and the proof receipts. Final delivery adds
the actual worker commit, parent, selected trees, two output blobs, original Git
streams, working/index comparisons, and two further distinct-copy strict replays.
Those artifacts and receipts necessarily postdate publication and are handed to
Root separately; this document does not embed its own final commit or archive hash.

No Source implementation, build, fixture, probe, Ghidra mutation, Original code
execution, game/OS/audio runtime, provider, callable cell, or library validation
was performed. Only the two packet outputs are committed; Root owns integration.
