# Native no-sound selection readiness

The original startup chain can select genuine NOSOUND when the actual settings
byte at `F88980+24h` is zero. The reviewed production construction and options
path do not provide a supported way to make it zero. Current Source preserves
the observed ignored-keyword behavior; this is not a missing recovered handler.

At `0073DAEE`, Native startup compares byte `F889A4` with zero. It pushes EDI,
executes `SETZ DL`, pushes EDI again, sets ECX to the allocated sound receiver,
pushes EDX and calls `00A88770` at `0073DAFD`. EDI is the retained zero register.
Only DL changes; upper EDX bits remain their preimage. The sound constructor
reads only that first stack byte, negates it with `SETZ AL` and writes receiver
`+70h`. The ordinary contract is ECX=self, three stack dwords and `RET 0Ch`;
the latter two words are ignored. Ghidra's current prototype models an unused
EDX pseudo-parameter through `__fastcall`; no ABI or prototype repair was made.

| Settings +24h | Disabled argument | Sound +70h | Output selection |
|---|---|---|---|
| Zero | 1 | 0 | Real `System_SetOutput(NOSOUND=2)` |
| Nonzero | 0 | 1 | Ordinary driver/speaker configuration |

Both paths still reach real `EventSystem_Init(512,16,null,0)`. File callbacks,
resource/bank construction and sound configuration remain necessary. The branch
does not supply a fake length, output or success and does not skip FMOD startup.

Prior admitted R157 evidence resolves the old direct-writer uncertainty:
`CD2D80` sets ECX to `F88980` and calls `8D7710`; `8D7736` writes byte `[ESI+24h]=1`.
This indirect field store is not a direct xref to `F889A4`. The current raw
process owner constructs the same BCh settings storage once and retains its
contexts and CRT shutdown registration. The application loader uses that owner;
its compatibility read view copies the actual flag. Conditional selected-user
profile import is retained complete Source: it writes control bytes +40h–43h
and game difficulty +6ACh, with no +24h store. That importer was not reopened
in Native analysis during this packet.

The exact options path is `008D85CD` push `SoundEnabled`, `008D85D2` match/consume
the keyword, `008D85D7` test AL, then `008D85D9` jump back to the loop on a match.
There is no value read or flag store. The following numeric `0` or `1` remains
current and follows unknown-token logging and one-token consumption at `008D8621`.
Thus `SoundEnabled 0` leaves the enable byte unchanged and is not silently
accepted as false. Current raw and prior projected parsers both preserve this.
Writing the token from the serializer does not establish a functioning selector.

One newly authorized, project/program-verified typed xrefs query for `F889A4`
(limit 20) returned only `0073DAEE READ`. That result does not enumerate indirect
field stores or external mutation and does not prove absence of every possible
Native selector. No additional writer body was identified or opened. The current
Source command-line parser has no no-sound/headless selector; it is a Source
harness interface. Retained evidence that WinMain ignores its incoming command
line is limited to that routine, not every original command-line consumer.
`--frames` still follows normal sound/window/device startup.

Calling the exposed Source sound API with `disabled=true` preserves the existing
branch semantics, but is not an admitted production config/CLI selection chain.
Adding parsing to the ignored keyword or a new override would be an intentional
behavior change. No bounded reconstruction code packet is justified by these
findings. Indirect or external +24h writers beyond the admitted owner/profile/
loader contracts remain explicit discovery frontiers; this packet does not
invent their addresses or behavior.

The [receipt](../reports/cc12_native_nosound_selection_readiness.json) freezes
complete selected Source, Git preimages, existing evidence, exact Native windows,
query commands/output, hashes and a pure offline replay. The prior committed
audio receipt is unchanged and remains the normal startup endpoint boundary.
This packet performs no C++ edit, compilation, game launch, FMOD initialization,
environment/config change, GPR mutation or listing repair. Startup, binary ABI
and gameplay credit remain zero; independent reconstruction can continue.
