# Application retirement conditional unwind-row candidate

Two exact reads of `00DB57B0..00DB57B7` agree:
`FF FF FF FF 00 62 C8 00`.
SHA-256: `cfa35011665839ef6c0c3aab2f144f09b31c86c04a4d0bd8d25e8e763bdbd923`. Independent little-endian
decoders return these raw numeric values:

| Offset | Hexadecimal DWORD | Unsigned value |
| --- | --- | --- |
| +0 | `FFFFFFFF` | 4294967295 |
| +4 | `00C86200` | 13132288 |

Signed32 word0 is `-1`. Only if the classic absolute EH-row layout applies, signed word0=-1 is a possible predecessor-index sentinel. This is a hypothesis, not authenticated unwind behavior.
Only under that unverified layout is word1 `00C86200` a
possible action address. It remains opaque and was not queried. No Original
FH3 layout, maxstate/state count, action invocation or receiver is established.

## Positive selection and gate

The accepted twelve-byte prefix has raw words `19930522`, `00000001`,
`00DB57B0`. This new packet explicitly admits the conditional classic absolute
EH-row0 candidate at that numeric third word, for exactly eight bytes. The
preceding packet's opaque-word observation alone did not authorize traversal.
Its accepted carrier context remains the MOV to EAX, physical JMP to the
shared wrapper without a return push, and the wrapper's physical dispatcher
call carrying D. No earlier archive or Native body was reopened.

Before bytes were read, two eight-address typed batches identified a defined
`DataDB` at `00DB57B0`, length8. Exact-at exists only at its first address;
all seven interior exact-at values are null, and every containing alias names
the same unit. Instruction/function references are null, function lists empty
and instruction flow absent. No undefined surrogate or function was invented.
Saved DataDB definition and size do not authenticate EH-row field roles.

Root independently admitted the two exact8B reads. The retained gate hash is
`292084fed8222def41ca95365c3f61bdd85325e88135b7097bff51107305521f`; its physical producer is
retained opaque and pinned. Paired and closing metadata agree in all24 raw
responses at epoch98; four count bookends remain64730 in
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Proof and boundaries

The [report](../reports/cc12_application_vector_retirement_unwind_row.json)
pins eight finite positive Git inputs: the immediately accepted prefix worker
and Root doc/report pairs, target configuration and three CLI producers.
Actual frozen/staged/final Git objects, exactly two output blobs, full physical
helpers/producers, raw responses and CLI streams are retained. Original and
distinct-copy strict ZIP replays admit one supplied archive read and one
declared receipt write. Actual object/entry counts and final pins are delivered
in the handoff; no inherited counts or self-referential hashes are assumed.

Earlier proofs remain immutable. No action target, map tail or remaining24B
descriptor tail was queried. Source subtree IDs are identity only; no Source
census or future Root equality condition is imposed. No Source/Ghidra/ledger
change, build, test, probe, game or OS/audio execution occurred.

The shared wrapper's externally returned ESP, EBP, EAX and memory remain
unknown, with aliases preserved. Its own EBP does not bind an Original action
frame or receiver. The conditional numeric row earns no Original, FH3, ABI,
actual state-0 action, ownership, activation or runtime credit. Parent Source
stays blocked until Root establishes actual action receiver and target behavior
and the remaining contracts. Any possible action entry needs a separate gate.
