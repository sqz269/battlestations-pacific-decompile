# Game resource parser registration

Addresses: 006fb000, 00716fe0, 007170b0, 00717180, 00717250, 00717320, 00717e80, 007186e0, 00718760, 007187e0, 007258f0.

`00717e80` registers five concrete parser singletons with the existing resource
manager, in the order below. Exact name literals and primary-vtable slots are
verified against both saved Ghidra bytes and the installed PE. These are
descriptive reconstruction names, not recovered symbols.

| Order | Name literal | Singleton getter | Global pointer | Primary vtable | Name getter (`+4`) | Parse entry (`+8`) |
|---|---|---|---|---|---|---|
| 1 | `Aux` | `00717320` | `00e19b88` | `00cfd840` | `00718760` | `0071b5c0` |
| 2 | `ZoneDesc` | `00717250` | `00e19b8c` | `00cfd830` | `007187e0` | `00719240` |
| 3 | `Note` | `00717180` | `00e19b84` | `00cfd820` | `007186e0` | `00719000` |
| 4 | `GeomMesh` | `00716fe0` | `00e19bd0` | `00cfd800` | `007258f0` | `00727a90` |
| 5 | `ConvexObject` | `007170b0` | `00e19a90` | `00cfd810` | `006fb000` | `006faf00` |

## Registration and singleton ABI

The complete wrapper is `00717e80..00717ee6` (103 bytes), with no explicit
arguments and a plain `RET`. For each row it calls manager getter `004c1400`,
preserves that returned pointer in ESI, obtains the parser singleton, then calls
registration `00b80a50` with ECX=manager and the parser pointer on the stack.
Manager construction, default registrations, and map insertion semantics belong
to the separate manager packet.

Each singleton getter is 206 bytes, takes no explicit input, returns the parser
in EAX, and ends with `RET`. The first `MOV EAX,FS:[0]` overwrites incoming EAX;
the decompiler's apparent input register is not a caller-supplied parameter.
On first use, it rechecks its global under the singleton lifetime manager's
lock, allocates eight bytes, and installs two interfaces:

- Offset `+0`: primary parser vtable shown above.
- Offset `+4`: lifetime interface; final vtables are `00cfd83c`, `00cfd82c`,
  `00cfd81c`, `00cfd7fc`, and `00cfd80c`, respectively.

It registers the adjusted `object+4` interface through `00bd0c30` and returns
the primary object. Null allocation takes the native zero-global path; this
packet does not claim registration is safe under allocation failure. Lifetime
manager internals are reused dependencies, not new reconstruction here.

## Exact names

All five name getters were missing Ghidra function definitions at audit time.
Each has a complete 33-byte body through `RET 4`. ECX's parser value is unused;
the stack argument is a hidden native-string output pointer. The function
passes the literal to `0041e870` with ECX=output, returns that same output in
EAX, and removes the four-byte output argument.

The literal addresses are `00cfd8bc` (`Aux`), `00cfd8c0` (`ZoneDesc`),
`00cfd8b4` (`Note`), `00cfdbcc` (`GeomMesh`), and `00cfb6d8`
(`ConvexObject`). Exact spelling is recovered here; case folding and matching
semantics must come from the manager and dispatcher.

## Evidence and limits

[The audit report](../reports/game_resource_parser_registration_audit.json)
records original ABI, spans, SHA-256 values, prior annotations, and proposed
Ghidra names/comments. Eleven full code spans (1,298 bytes) and four data spans
(168 bytes) match the installed executable. Every code span was independently
decoded completely through its final return. Raw exports and disassemblies are
under ignored `exports/bsp/parallel_game_resource_parser_registration/`.

The parse slots identify dispatch targets; this packet does not establish the
Aux, ZoneDesc, GeomMesh, or ConvexObject payload formats. Note parser `00719000`
is covered separately by [RESOURCE_NOTE_PARSER.md](RESOURCE_NOTE_PARSER.md).
This wrapper contains no `CloudSystem` registration; it is not a proof that no
other registration site exists globally.

This is byte-verified evidence. No C++ source, Ghidra metadata, shared ledgers,
tests, build, or game state was changed by this packet. It makes no native ABI,
fixture-parsing, or game-validation claim.
