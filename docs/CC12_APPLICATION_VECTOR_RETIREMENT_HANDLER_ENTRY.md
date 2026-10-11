# Application vector retirement handler entry: 00C86208

The bounded entry at `00C86208` is the complete five-byte instruction
`B8 B8 57 DB 00` (`MOV EAX,00DB57B8`). Its saved flow is ordinary
`FALL_THROUGH`, with `NONE` override and no overridden fall-through. Both saved
function-at and function-containing references are explicitly null. No saved
name, function range, thunk/no-return flags or prototype is available.

The literal has a positive parent link: the retained Application parent shell
`007379A0..007379FC` is 93 bytes/28 instructions with SHA-256
`ef5b16ae5487c295eddb16bee693072e2f57ca13d59f437ca2b544ea90e38297`.
Its `007379A2` instruction is `68 08 62 C8 00`, pushing `00C86208` before
`FS:0` registration. This packet reuses those exact report bytes; it does not
re-query the parent or read the installed executable.

## Saved entry evidence

| Observation | Result |
| --- | --- |
| Project/program | `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` |
| Architecture | `x86:LE:32:default`, image base `00400000` |
| Saved epoch/count | `98` / `64730` |
| Listing aliases | Exact/containing instruction and code unit all identify `InstructionDB` at `00C86208`, length 5 |
| Whole instruction extent | `00C86208..00C8620C`; this is not a function-body extent |
| Function at/containing | Both null; `functions=[]` |
| Saved prototype CLI | `No function found for 00c86208`; signature endpoint also reports no function |
| Default/effective flow | `FALL_THROUGH`; call, jump, conditional, computed and terminal flags all false |
| Entry effect | Load the opaque immediate `00DB57B8` into EAX; no explicit memory access or control transfer |

Two corrected typed captures and a closing capture retain identical raw bytes,
SHA-256 `1d8828c8435ba6816facb988be049c4b8aa5ca818449a353a987a15161cf0b8b`.
The exactly five instruction bytes were requested only after the typed listing
established that extent. The whole saved prototype CLI stream is retained,
including its caught signature error. Its exit code zero does not indicate
that a prototype exists. Automatic companion and target objects remain opaque.

## Capture limitation and proof closure

The first successful typed CLI response was overwritten by the initial local
collector's command metadata because both used `typed.json`. Its raw response
and original collector preimage are unavailable. The retained failed-attempt
record and CLI streams disclose the loss; this response contributes no metadata
proof. The corrected collector uses distinct `.command.json` and `.raw.json`
paths. No claim is made that every initial raw response survived.

The [report](../reports/cc12_application_vector_retirement_handler_entry.json)
pins eight finite positive Git inputs and the frozen commit. The local packet
directory contains physical capture, sealing, pure replay and audit helpers,
all retained streams, actual frozen/staged/final Git objects, `proof.zip`, its
distinct physical copy, and separate observed replay receipts. The pure replay
checks the two added outputs, raw identity/metadata and entry bytes without a
live Ghidra or Source read. Original and copy archive pins, final commit and
audited replay results are supplied in the handoff to avoid self-referential
commit/ZIP claims in these outputs.

The accepted derived retirement Source context remains dormant. Source tree
IDs are recorded for identity only; this packet performs no Source census and
places no future equality condition on Root. No source, Ghidra or ledger edit,
build, test, probe, game execution, body/decompile/disassembly, xref, stack,
descriptor, data or target expansion occurred. No Original, runtime, FH3,
ownership, activation, complete-handler or state-0 action credit is earned.
Root receives this entry result before choosing any finite continuation.
