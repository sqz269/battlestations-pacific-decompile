# The x87 control word the motion math runs at

Addresses: 00C2D14E 00C2D21A 00C0682E 00C06839 00BF6A58 00BF6A6E 00BFBC47 00BFBC63 00BFD221
00C273F8 00C2CE44 00C2C6FF 00B2AEB0 00B2AFB4 00B2AFF9 00B29670 00B298EE 00D6A678 00825F20

Packet `cc_cruise_command`, part 2, worker `agent/cc-cruise-command`, 2026-09-11 UTC. Ghidra
was read-only. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Descriptive names are hypotheses, not recovered symbols; the CRT names are the library's own.

## Answer

**The executable sets the x87 control word to `027Fh` (53-bit double precision) in its CRT
startup and never changes the precision field again. It then creates the Direct3D 9 device
without `D3DCREATE_FPU_PRESERVE`, so `d3d9.dll` drops the precision field to 24 bits for the
life of the device. The ship motion at `00825F20` therefore runs at a 24-bit mantissa,
round-to-nearest, all exceptions masked: control word `007Fh`.**

The last step happens inside `d3d9.dll`, not in this image. What the image proves is that it
never asks Direct3D to preserve the FPU and never raises the precision field afterwards; that
is as far as static evidence reaches. A run-time `FNSTCW` at a breakpoint on `00825F20`, or a
`_controlfp(0,0)` logged from the fixed step, would confirm `007Fh` directly.
`docs/MOTION_DIFFERENTIAL.md` already lists that read as the `motion_live_trace` follow-up.

## The control word the executable itself sets

The CRT is statically linked: there is no `_controlfp` import and no `msvcr*` name string.
One live chain sets the word, and it is the CRT's own startup.

| Step | Site | Instruction / bytes |
| --- | --- | --- |
| `___tmainCRTStartup` calls `_cinit(1)` | `00BFD221` | `PUSH EBX` with `EBX = 1` from `00BFD1AA XOR EBX,EBX` / `00BFD1AC INC EBX`, then `00BFD222 CALL 00BFBC47` |
| `_cinit` calls the FP initialiser through the pointer at `00D693D8` | `00BFBC63` | `CALL dword ptr [00D693D8]`, and `00D693D8` points to `00BF6A58` |
| `__fpmath` takes its non-zero-argument branch | `00BF6A6E` | body `e8 8b ff ff ff e8 31 fe 00 00 83 7c 24 04 00 a3 7c dd 09 01 74 05 e8 bb fd 00 00 db e2 c3`: `CMP [ESP+4],0` / `JZ` / `CALL 00C0682E` / `FNCLEX` |
| `__setdefaultprecision` asks for `_PC_53` under the `_MCW_PC` mask | `00C0683C` | body `56 68 00 00 03 00 68 00 00 01 00 33 f6 56 e8 b7 0b 02 00`: `PUSH 30000h` (mask), `PUSH 10000h` (`_PC_53`), `XOR ESI,ESI`, `PUSH ESI`, `CALL _controlfp_s 00C273F8` |
| the write | `00C2D21A` | `FLDCW [ESP+10h]` inside `_control87` `00C2D14E`, skipped by `00C2D200 CMP EAX,EDX / JZ` when nothing changed |

`00D6A678` holds `7f 02`, the CRT's own `027Fh` constant, and six transcendental prologs guard
their `FLDCW` with `CMP word ptr [ESP],0x27f`, which corroborates the value independently.

So after `_cinit` the process runs at `PC = 10b` (53-bit), `RC = 00b` (round to nearest even),
`IC = 0`, all six exception masks set.

**No game code calls `_controlfp`, `_control87`, `__control87_2` or `_fpreset`.** `__set_controlfp`
`00C2C6FF` and `___control87_2` `00C2CE44` have no cross-references at all, and there is no
real `FNINIT` (`DB E3`) in `.text`: all eleven byte matches are inside other instructions.

## Every `FLDCW` in the image

A scan of all `D9 /5` ModRM forms (`D9 28`-`D9 2F`, `D9 68`-`D9 6F`, `D9 A8`-`D9 AF`) over
`.text`, resolved against the live listing, finds **286** real `FLDCW` instructions. 266 of
them are the paired MSVC `_ftol` idiom, for example `0041BA90`: `FNSTCW [ESP+10h]`,
`OR EAX,0C00h`, `0041BAA5 FLDCW [ESP+14h]`, `FISTP`, `0041BAB1 FLDCW [ESP+10h]`. That pair
changes the rounding-control field only, restores it within three instructions, and never
touches precision control.

| Address | Containing function | What it loads | Permanent? |
| --- | --- | --- | --- |
| `00C2D21A` | `_control87` `00C2D14E` | the word `00C2C802` builds from (new, mask) | **permanent**, the only deliberate setter reached |
| `00C2CF17` | `___control87_2` `00C2CE44` | the same | dead code, no callers |
| `00C138C5` | `__ctrlfp` `00C138A7` | `[EBP+0Ch]`, after `FSTCW [EBP-4]` | restored, CRT error paths |
| `00BF84F7`, `00BFA087`, `00BFA97B`, `00BFE647`, `00BFE7A7`, `00BFEE0B` | CRT transcendental prologs | `[00D6A678]` = `027Fh`, guarded by `CMP word ptr [ESP],27Fh / JZ` | restored |
| `00C08436`, `00C08460`, `00C084C5` | `00C0842E`, `__math_exit` `00C0843B`, `00C08479` | `FLDCW [ESP]` after the same guard | restore |
| `00C083B7` | `00C083A5` | `AND EDX,300h ; OR EDX,7Fh ; FLDCW [ESP+6]` | temporary; keeps the caller's PC bits |
| `00C0837E` | `__startOneArgErrorHandling` `00C08347` | `[EBP+8]`, the caller's saved word | restore |
| `00C08002`, `00C08182`, `00C081E9` | `00C07FAA`, `__trandisp1`, `__trandisp2` | `[EBP-0A4h]` / `[EBP-0A2h]` | restore |
| `00A63740`, `00A6374B` | `str_format` `00A635F0` | `[EBP-8]` then `[EBP-2]` | paired, restored |
| `00C0ED8D`, `00C0EDCA`, `00C0EDD9` | `00C0ED3C`, a CRT long-double helper called only from `00BF8593` | `OR EAX,33Fh`, so `PC = 11b` | leaks on the `00C0EDB9` branch: `FNSTCW [ESP+24h]` there overwrites the word saved at `00C0ED7C`, so `00C0EDD9` restores `orig \| 33Fh`. A CRT-internal edge path, not startup. |
| 266 others | game and CRT | the `_ftol` rounding pair | restored |

`00825F20` contains no `FLDCW` at all. Its inline `_ftol` pairs only toggle rounding control.

`docs/NATIVE_CRT_X87_CONTROL.md` reconstructs `00C083A5` and `00C0842E` as assembly bodies;
this table is where they sit in the whole picture, and neither changes the process default.

## The Direct3D 9 device creation flags

Two call sites go through `IDirect3D9` vtable slot `+40h`, which is `CreateDevice`:

| Site | Containing function |
| --- | --- |
| `00B2AFF9` | `BSP_D3D9Renderer_InitializeDeviceAndResources`, body `00B2AEB0`-`00B2B1F1` |
| `00B298EE` | `BSP_D3D9Renderer_RecreateDevice`, body `00B29670`-... |

The `BehaviorFlags` argument is computed immediately before the first site:

```
00B2AFB4: TEST dword ptr [ESP+38h],0x10000     ; D3DDEVCAPS_HWTRANSFORMANDLIGHT
00B2AFBC: JZ  00B2AFD1
00B2AFBE: MOVZX ECX,word ptr [ESP+0E0h]        ; the vertex-shader version
00B2AFC6: CMP ECX,0x101
00B2AFCC: LEA ECX,[EBX+0x3f]                   ; EBX = 1, so 0x40
00B2AFCF: JAE 00B2AFD6
00B2AFD1: MOV ECX,0x20
00B2AFD6: ... 00B2AFE9: OR ECX,4 ... 00B2AFEC: PUSH ECX
```

`EBX` is written once in the whole function, `00B2AF41 MOV EBX,1`, so `LEA ECX,[EBX+3Fh]` is
`40h` and the `PUSH EBX` at `00B2AFF5` is `D3DDEVTYPE_HAL`. The flags are therefore

| Path | Value | Decoded |
| --- | --- | --- |
| hardware T&L and vertex shader >= 1.1 | `0x44` | `D3DCREATE_MULTITHREADED \| D3DCREATE_HARDWARE_VERTEXPROCESSING` |
| otherwise | `0x24` | `D3DCREATE_MULTITHREADED \| D3DCREATE_SOFTWARE_VERTEXPROCESSING` |

`00B298A7` in the recreate path has the identical sequence with the literal `40h` spelled out.
There is no third branch and no retry loop that passes different flags. `0x44` matches the
value `docs/GAME_EXECUTABLE.md` already records for the reconstructed renderer.

**`D3DCREATE_FPU_PRESERVE` (bit 1, `0x00000002`) is not set on either path.** Without it,
Direct3D 9 switches the x87 unit to single precision at device creation and leaves it there;
with it, Direct3D leaves the control word alone.

## Corrections

None. No earlier document stated a control-word value; `docs/MOTION_DIFFERENTIAL.md` and
`docs/SHIP_MOTION.md` both listed the question as open, and this settles the static half of it.

## What this changes for the reconstruction

The reconstruction builds with `/fp:strict` on SSE2, where a `float` multiply is IEEE single
and a `double` multiply is IEEE double, with no extended intermediate. The shipped executable
computes its `float` expressions on the x87 stack at a 24-bit mantissa, which rounds each
intermediate to single precision, so the two agree on `float` arithmetic far better than they
would have at 53 or 64 bits. Where the native code loads a `double` constant (the `0.05` at
`00D7A270`, the `1.0` pairs in `00825F20`) the x87 result is rounded to 24 bits and ours is
not, and those are the expressions a differential run should watch. No numbers here were taken
from the running game.

## no_ghidra_function

| Start | Inclusive end | Evidence for the boundary |
| --- | --- | --- |
| none | | Every address cited here lies inside a function Ghidra already has. `00C083A5` and `00C0842E`, which `docs/NATIVE_CRT_X87_CONTROL.md` calls raw bodies, are cited here only as rows of the `FLDCW` table and are not part of this packet's reconstruction. |

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `motion_live_trace` | `00825F20`, `00B2AFF9` | Already proposed by `docs/MOTION_DIFFERENTIAL.md`. Add one `FNSTCW` read at the top of the fixed step and one immediately after `CreateDevice` returns; that turns the `d3d9.dll` half of this answer from an inference about a documented API into an observation. |
| `crt_long_double_leak` | `00C0ED3C`, `00BF8593` | The one `FLDCW` in the image that can leave the precision field raised. It is reached only from a CRT long-double helper, and whether the game's call graph reaches `00BF8593` at all was not established. |

## Uncertainties

1. The 24-bit write is performed by `d3d9.dll`. It is documented behaviour of
   `IDirect3D9::CreateDevice` without `D3DCREATE_FPU_PRESERVE`, not an instruction in this
   image, and no run log was taken (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).
2. `00C0ED3C`'s leak path was decoded from the listing but its reachability from game code was
   not traced.
3. Whether any thread other than the one that creates the device sees a different control word
   was not examined; `D3DCREATE_MULTITHREADED` is set, and the x87 control word is per thread.
