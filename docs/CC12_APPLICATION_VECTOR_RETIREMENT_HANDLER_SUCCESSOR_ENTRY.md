# Application handler successor instruction: 00C8620D

The complete saved five-byte instruction at `00C8620D` is
`E9 31 09 F7 FF`: a physical `JMP rel32` to numeric address `00BF6B43`.
The exact saved text also says `JMP 0x00bf6b43`. Separately, Ghidra has a
`CALL_RETURN` override, with default `UNCONDITIONAL_JUMP` and effective
`CALL_TERMINATOR` flow. The physical jump does not push a return address.
The existing override was observed and left unchanged.

The positive candidate came from the accepted five-byte `00C86208`
`MOV EAX,00DB57B8` instruction with ordinary physical and saved fall-through:
`00C86208 + 5 = 00C8620D`. These adjacent observed units do not establish
a saved function or complete handler-body extent.

## Gate and physical evidence

Paired typed captures at exact `00C8620D` established all four listing aliases
as `InstructionDB`, length 5. Both function-at and function-containing are null,
with an empty function list. Saved name, ranges, thunk/no-return flags and
prototype remain unavailable; no function was created or repaired. No separate
prototype endpoint was queried in this narrower packet.

Root independently checked those captures before admitting the exact unit's
two byte reads and context-zero saved text. The retained Root gate has SHA-256
`147b170a2848a33b8ec9fb17344ba1664db8ff05a78f6823224a881918e6f15c`.
Both reads returned the same five bytes. The saved text has no preceding or
following instruction rows. The signed little-endian displacement is
`-587471`; `00C8620D + 5 - 587471 = 00BF6B43`.

| Observation | Result |
| --- | --- |
| Exact unit extent | `00C8620D..00C86211` (instruction only) |
| Physical opcode | `E9`, near relative unconditional jump |
| Saved flow override | `CALL_RETURN` |
| Default flow | `UNCONDITIONAL_JUMP`: jump true, call/conditional/computed/terminal false |
| Effective flow | `CALL_TERMINATOR`: call/terminal true, jump/conditional/computed false |
| Fall-through override | false |
| Function at/containing | Both null, function list empty |
| Project/program | `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` |
| Epoch/function count | `98` / `64730` throughout |

Paired gate and closing raw responses are identical with SHA-256
`645af50b3017737382038c15a8ac401e8692952d383a2ddd02e4b206aeac0b90`.
Whole raw responses and every CLI stream are retained. Automatic target and
companion objects stay opaque. `00DB57B8` was not read, classified or followed;
`00BF6B43` is only the numeric destination decoded from this admitted unit.

## Proof and limits

The [report](../reports/cc12_application_vector_retirement_handler_successor_entry.json)
pins eight finite positive Git inputs: the preceding worker and Root entry
doc/report pairs, target configuration and three exact CLI producer files.
The local proof contains actual frozen/staged/final Git objects and exactly two
output blobs, retained Root gate and captures, physical helpers and their used
preimages. Original and distinct-copy ZIP replays allow one supplied archive
read and one declared receipt write. Observed final pins and receipts are
delivered in the handoff, avoiding self-referential commit/archive claims.

The earlier proof remains intact. Its disclosed initial raw loss is not
repaired or hidden by this packet; every attempt in this successor packet was
retained without an overwrite. Source subtree IDs are identity only, with no
Source census or future Root equality requirement. No descriptor, action/map,
target body, full handler, xref, stack or continuation was read. No Source,
Ghidra or ledger mutation, build, test, probe or game execution occurred.
No Original, FH3, ABI, state-0 action, ownership, activation or runtime credit
is earned. A descriptor gate requires separate Root scope.
