# Options audio-enable provenance review

The two additional `005F52C0` invocations copy the persistent settings **into
the first embedded screen object**. At both `005F5571` and `005F5C4A`, the source
argument is `00F88980` and the destination receiver is `screen+D8h`. Composing
these operands with the accepted helper's exact `+24h` byte transfer gives:

```text
005F5571: byte[screen+FCh] = byte[00F889A4]
005F5C4A: byte[screen+FCh] = byte[00F889A4]
```

Neither invocation targets the second embedded object at `screen+194h` or its
enable byte `screen+1B8h`. These are value-propagation routes, not demonstrated
sources of zero. They are the opposite direction from the previously accepted
`005F6629` invocation, which copies `screen+FCh` back into `00F889A4`.

## Copy receivers and bounded conditions

| Caller | Exact machine operands | Condition within inspected window |
|---|---|---|
| `005F54F0` | `5511 MOV ESI,ECX`; `5566 PUSH F88980h`; `556B LEA ECX,[ESI+D8h]`; `5571 CALL 005F52C0` | `screen+78h == 0` branches to the copy. The other branch clears that flag, requests interface `Eh`, and returns at `5565`, bypassing this copy. |
| `005F5BD0` | `5BEB MOV ESI,ECX`; `5C3F PUSH F88980h`; `5C44 LEA ECX,[ESI+D8h]`; `5C4A CALL 005F52C0` | The initial `screen+25Ch` branch and normal completion of the optional title/string work converge at `5C3F`. Exceptional/non-returning callee behavior is not established. |

The accepted helper facts remain admitted evidence: ECX is destination,
`[entry ESP+4]` is source, `005F5315` copies source `+24h` to destination `+24h`,
EAX returns destination, and `RET 4` removes the source argument. Its body was
not reopened. Likewise, the admitted `008D7710` default-one store and the
previous constructor receiver recovery initialize `screen+FCh` and
`screen+1B8h` to one on normal construction. Neither fact supplies a later zero.

## Exact page-2 and row dispatch

`005F42D5` retains the incoming screen receiver in ESI. The selected row returned
by `00A9C920` is moved to EBP at `42E2`; a negative row exits. At `42EC` the
function marks `screen+250h` dirty, then reads page global `00E19650`, subtracts
one, bounds the resulting index, and dispatches through `005F4984`.

For page 2, the index is `2-1=1`: the dword at `005F4988` is `005F459C`, a
confirmed instruction start (`CMP EBP,3`). Rows above 3 take the shared return
at `005F43A2`. Valid rows dispatch through the four dwords at `005F49DC`:

| Row | Confirmed target | Direct volume store | Settings-relative field | Audio application |
|---|---|---|---|---|
| 0, master | `005F45AC` | `45EF FSTP float[ESI+F8h]` | `+20h` | `45E9 LEA ECX,[ESI+D8h]`; call `008D5430` at `45F5` |
| 1, music | `005F4602` | `4645 FSTP float[ESI+100h]` | `+28h` | receiver at `463F`; call at `464B` |
| 2, speech | `005F4658` | `469B FSTP float[ESI+108h]` | `+30h` | receiver at `4695`; call at `46A1` |
| 3, effects | `005F46C0` | `4703 FSTP float[ESI+104h]` | `+2Ch` | receiver at `46FD`; call at `4709` |

The targets, store operands, receiver instructions, and call destinations are
checked against retained original bytes. Each row obtains its integer step
from `[ESP+14h]` after the shared 16-byte prologue adjustment, corresponding to
the first stack argument at entry ESP+4. The x87 arithmetic and clamp call were
not emulated; no exceptional-value, rounding, or bit-equivalence claim follows.

Speech loads the preview receiver from `screen+A4h` at `46A6`; effects load
`screen+A0h` at `470E`. Both load its vtable, restore the local frame, replace
the stack argument with one, fetch virtual slot `+34h`, and tail-jump at
`46BE`/`4726`. These are preview-call operands, not stores into an enable byte.
The invoked audio/clamp/preview bodies were not reopened or analyzed here.

All 91 instructions from `005F459C` through `005F4726` were inspected. The only
direct screen-relative stores in those audio blocks are the four FSTP stores
above. There is no direct store to `screen+FCh`, `screen+1B8h`, or the global
enable cell in that finite code. This does not exclude writes performed by
delegated calls, other branches, other functions, or external code.

## Actual zero stores and remaining provenance

The inspected main-list head contains actual immediate-zero stores:
`005F5513 MOV byte[ESI+252h],0` and, on the early-return branch,
`005F5542 MOV byte[ESI+78h],0`. Their receiver is the screen, and neither offset
is an embedded settings enable byte. In the audio rows, XORPS followed by MOVSS
also writes zero bits to stack temporaries; it does not target screen storage.
No instruction in this packet establishes an actual zero value for the enable
byte. The two recovered copies would propagate zero if the persistent source
already contained zero, but that conditional is not new zero provenance.

No lifecycle route or ordering relative to startup read `0073DAEE` was audited.
This packet establishes neither pre-startup reachability nor exclusively
post-startup behavior. The second embedded object's later value sources,
delegated writes, and any genuine zero-producing route remain separate
frontiers. No exhaustive absence claim or supported selector is inferred.

## Source readiness and evidence

Current `src/options_menu_screens.cpp` routes the audio page through
`step_ranged`, `apply_audio`, and speech/effect preview methods.
`include/bsp/options_menu_screens.hpp` supplies the matching four field offsets,
but these host methods are abstract, and `OptionsScreenState` deliberately keeps
volume/settings storage behind the host. `build_main_list()` is also an
abstract slot. Those declarations do not implement or validate the two newly
recovered raw copies. The accepted `reload_from_settings()` naming discrepancy
for `005F65C0` remains; no Source change or new interface is introduced.

The [receipt](../reports/cc12_options_audio_enable_provenance_review.json) freezes
17 complete Source/Git inputs, exact commands and outputs, finite saved/live
views, and a complete readonly original PE. All inputs match Git after EOL
normalization. Original PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Seven windows totaling 768 bytes match both the retained PE mapping and live
Ghidra bytes; 106 saved/live instruction rows and 39 exact byte/call checks pass.

Saved exports for `005F54F0` and `005F5BD0` were unavailable. Typed live queries
confirm both now have stored function bodies; the older comment saying there
is no `005F5BD0` function is stale. No export or analysis repair was performed.
Each live command invokes the frozen `Client.verify()` before its query,
checking project `bsp`, program `/battlestationspacific.exe`, x86 Win32, and image
base `00400000`. `C:/Users/sqz269/bsp.gpr` is configured; its full live GPR path
is not independently exposed. Autostart was disabled.

Portable replay reads only `local/cc12_options_audio_enable_provenance_review`:

```powershell
python local/cc12_options_audio_enable_provenance_review/replay.py
```

Its audit hook rejects filesystem opens outside that evidence directory and
writes. The complete retained PE is checked by hash and section mapping; the
installation path is provenance text only. Replay passed with zero installation
opens. No C++ change, compiler/test invocation, Native execution, Ghidra
mutation, SDK/game/OS change, startup validation, ABI replacement, or gameplay
credit is claimed.
