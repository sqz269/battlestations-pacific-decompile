# Options sound-byte copy and receiver review

The bounded Native copy step in `005F65C0` writes **screen to persistent settings**.
At `005F6629`, the caller supplies `ECX=00F88980` and the stack argument
`screen+D8h` to `005F52C0`. That helper reads its argument's byte `+24h` at
`005F5311` and writes its receiver's byte `+24h` at `005F5315`. Consequently this
invocation writes `00F889A4 = byte[screen+FCh]`.

This establishes an indirect persistent-byte writer. It does **not** establish
a writer of zero, a supported zero-selecting input, or execution before the
startup read at `0073DAEE`. The accepted direct query returning only that read
at `F889A4` was never an indirect-writer census.

## Exact receivers and copy ABI

| Native window | Recovered machine facts |
|---|---|
| `005F6030..005F60E8` | `6048 MOV ESI,ECX` retains the screen receiver. `6093 LEA ECX,[ESI+D8h]` feeds the settings-constructor call at `609E`; `60A3 LEA ECX,[ESI+194h]` feeds its second call at `60AE`. Both calls target `008D7710`. The screen constructor returns its receiver in EAX and uses plain RET. |
| `005F65E2..005F662D` | `EBP=ECX`; `ESI=EBP+D8h`. After the comparison and optional compatibility setter, `6623 PUSH ESI`, `6624 MOV ECX,F88980h`, `6629 CALL 005F52C0`. The optional setter's skipped and taken paths converge at this copy call. |
| `005F52C0..005F52C8` | The helper pushes EBX/ESI/EDI, sets `ESI=ECX`, and loads `EDI=[ESP+10h]`, which is the first argument at entry ESP+4. ESI is destination; EDI is source. |
| `005F5311..005F5317` | `MOVZX ECX,byte ptr [EDI+24h]`; `MOV byte ptr [ESI+24h],CL`. This copies the input byte exactly, without a constant zero or boolean normalization. |
| `005F54E2..005F54E9` | Restores EDI/ESI/EBX, returns the destination in EAX, and uses `RET 4`. This supports a thiscall-style destination receiver plus one stack source argument. |

The live stored prototypes say `undefined ...(void)` for these functions;
saved/decompiled signatures also obscure receivers. The table is recovered
machine-interface evidence, not approval of those prototypes or a new binary
replacement. No Ghidra prototype, body, listing, flow, name, or comment was
changed.

The admitted settings-constructor evidence writes `byte[receiver+24h]=1`.
Composed with the newly recovered receivers, normal completion of the options
constructor initializes `screen+FCh` and `screen+1B8h` to one. The first embedded
object supplies this copy call; the second object's role and later uses are
unresolved here. The admitted CRT route initializes the distinct persistent
object's `F889A4` to one before startup. This packet does not reanalyze those
previously admitted constructor/CRT bodies.

## Covered fields and limits

The 554-byte helper was read completely. It copies fixed scalar fields through
`+95h`, delegates the `+98h` and `+A4h` subobjects using matching source and
destination offsets, copies bytes `+B0h..+B2h`, and handles a `+B4h` string-like
subobject with a self-object check, resize call, and conditional memory copy.
The delegated bodies were not opened, and no full lifetime/ownership proof is
claimed. The receiver's `+00h` and intervening padding are not direct scalar
stores in this helper.

Audio `+20h`, `+28h`, `+2Ch`, and `+30h` use x87 FLD/FSTP pairs; the recovered
enable-byte direction does not depend on emulating them. This receipt makes no
bit-preserving NaN, exception, or rounding claim for those float operations.
The complete scalar-store offset/instruction inventory is in the report.

The large `005F65C0` body was kept scoped to its receiver/copy head and final
global-receiver calls. Its tail passes the global to `008D6C00` at `005F6E58`
and, on one branch, to `008D5B50` at `005F6EBC`; this does not establish what
those unreviewed callees write. Unrelated intermediate telemetry/string work
was not reconstructed.

## Specific Source/document discrepancy

`include/bsp/options_menu_screens.hpp:428` names the `005F65C0` host slot
`reload_from_settings()`. `docs/OPTIONS_MENU_SCREENS.md:344` describes it as
settings-to-screen reload. That characterization is contradicted by the exact
screen-to-global copy at `005F6629`. In contrast, the existing comment in
`include/bsp/game_settings.hpp:156` calls this an options commit. These are
Source/document labels; no implementation was changed and no broader assertion
about every options/menu transfer direction follows from this result.

## Zero provenance and ordering remain open

The current bytes prove that a zero **would** be copied if the source byte were
zero. They supply neither an original writer that puts zero into `screen+FCh`
nor an invocation path reaching this copy before startup `0073DAEE`. The
admitted `SoundEnabled` token path still consumes the name without storing a
numeric value, and no Source/CLI selector is justified.

Typed caller metadata returns `005F7050`, `005F7310`, `005F7C40`, and `005F8960`
for `005F65C0`, and `006898C0` for the constructor. Their bodies, indirect
dispatch, and lifecycle ordering were not analyzed in this packet. Names such
as update/list-event do not prove all routes occur after successful startup.
Zero provenance and a demonstrated pre-read route are separate remaining
obligations. Neither universal absence nor post-startup-only timing is claimed.

## Frozen evidence and replay

The [JSON receipt](../reports/cc12_options_sound_copy_abi_review.json) freezes
17 complete current Source/Git inputs, exact typed commands and outputs,
finite saved Native views, four original PE byte slices, and offline replay.
All 17 current inputs match their accepted Git preimages after EOL normalization.
The original PE SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All four PE slices (1,039 bytes) match live Ghidra bytes; 122 scoped saved/live
listing rows agree. The helper's saved export was absent, so its decompilation
and listing were captured through typed readonly queries without exporting or
mutating the shared analysis.

Each live command invokes the frozen `Client.verify()` through `bsp.py`, checking
project `bsp`, program `/battlestationspacific.exe`, x86 Win32, and image base
`00400000` before its query. `C:/Users/sqz269/bsp.gpr` is the configured project
file; the bridge does not independently expose its full live file path.
Autostart was disabled. The retained artifact directory is
`local/cc12_options_sound_copy_abi_review`; replay reads only those frozen files:

```powershell
python local/cc12_options_sound_copy_abi_review/replay.py
```

No C++ change, compiler/test invocation, Native execution, SDK call, game/OS
change, or Ghidra mutation occurred. Offline receipt replay establishes static
evidence consistency only; startup, binary compatibility, and gameplay remain
unvalidated.
