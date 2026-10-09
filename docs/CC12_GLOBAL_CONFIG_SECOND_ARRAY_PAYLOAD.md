# GlobalConfig second-array payload attribution

The missing contract is now narrowed to an **actual publication into the three
slots at `GlobalConfig+2CC/+2D0/+2D4`**. The native objective-sound producer is
`008DD460`, reached by objective announcements. Its complete normal body creates
a tracked sound into a **stack temporary**, releases that temporary and updates
the timestamp at `GlobalConfig+2D8`. It does not publish the sound into the second
array. Consequently this packet does not authorize a raw second-array Source
provider or mark RawGlobalConfig ready.

This read-only packet inspected main commit
`b9de0885012052dc5def4702b5d1b509cb835384`. Only this document and
`reports/cc12_global_config_second_array_payload.json` are changed. Descriptive
names below are hypotheses, and no new names, types, comments or Source bodies
were written to Ghidra or the repository.

## Recovered producer and publication destinations

| Edge | Native evidence | Destination established |
| --- | --- | --- |
| Lua `ObjectiveSounds` to sample assignment | `0087F6EB` forms `owner+2C0`; `0087F6F9` calls `008DBE90`. The index starts at zero and increments at `0087F73D`, without a three-entry cap. | `008DBEDC` stores the sample-cache acquisition result at `[block+index*4]`. |
| Objective announcement to playback | `008E1BEB` pushes index zero; `008E1BED` obtains GlobalConfig; `008E1BF4` adds `2C0`; `008E1BFA` calls `008DD460`. | The first-array sample selected by index zero. |
| Objective state change to playback | `008E1D88..008E1D93` computes and pushes `2-(completed_byte!=0)`; `008E1D9B` adds `2C0`; `008E1DA1` calls `008DD460`, subject to its player/objective/silent gates. | First-array sample one or two. |
| Playback to tracked factory | `008DD516` reads `[EBP+index*4]`; `008DD545` calls `00A7E490`. | A caller-owned stack output reference and the Sound manager's active array. |
| Playback completion | `008DD54A..008DD56A` releases and clears the stack output; `008DD5A3` writes `[EBP+18]`. | Temporary cleanup and `owner+2D8` timestamp only. |

`008DD460` receives `ECX=GlobalConfig+2C0` and one stack index and returns with
`RET 4`. EBP retains that receiver. At `008DD53E`, `LEA ECX,[ESP+1C]` computes a
stack address; the following instruction pushes that address as the factory's
hidden output pointer. After `00A7E490` returns, the result is loaded from
`[ESP+0C]`. It is decremented at `008DD558`, conditionally invokes current virtual
slot zero at `008DD568`, and is cleared at `008DD56A`. No pointer derived from
`EBP+0C/+10/+14` is supplied as the output. The only receiver-relative write in
this complete body is the timestamp store at `008DD5A3`.

The native time gate is the x87 sequence at `008DD475..008DD490`; its exact bytes
and instructions are retained. No rewritten floating-point behavior is proposed.
The body lazily resolves `InGameGUI`, resolves `Normal`, and supplies flag one to
the tracked factory. Ghidra's decompiled signatures and temporaries obscure the
receiver and hidden output; the statements above follow the instruction listing.

`00A7E490` has `ECX=manager`, stack `output*, sample*, class, type, flag`, and
`RET 14`. Sample kind zero selects the **current** manager virtual `+0C`, kind
one selects current virtual `+10`, and other kinds produce null. For a nonnull
result, calls at `00A7E503` or `00A7E5A6` append via `00A7D5C0` with
`ECX=manager+8C`. The hidden output receives a separate retained reference before
the factory temporary is consumed. This identifies the manager publication; it
does not provide a GlobalConfig publication.

The live caller query for `008DD460` returned `008E1B90` and `008E1D30`. This is a
bounded direct-call observation. It does not prove a whole-program absence of
other aliases or indirect second-array producers. The general effect-name
acquisition wrapper `00871BA0` is not on either recovered objective-sound edge.

## Conditional actual channel profile and virtual ABI

For sample kind zero and current manager profile `00D5B000` or `00D5B44C`, live
table bytes resolve manager virtual `+0C` to `00A7F640`. A successful native
factory allocates `5Ch` and writes actual profile `00D5ABF8` at `00A7F6BD`.
This attributes a native factory return under those conditions. **It does not
attribute a current second-array pointee**, nor establish the manager's current
runtime profile or the selected sample kind in an executed game.

| Actual `00D5ABF8` entry | Target | Relevant contract |
| --- | --- | --- |
| `+00` | `00BD30E0` | ECX object, no stack argument, plain RET. No reference decrement. If nonnull, read current virtual `+04` and call it with flag one. |
| `+04` | `00A7D5A0` | ECX object, stack deleting flags, RET 4. Call `00A7BF40`; free storage when `flags & 1` is set; return original pointer. |
| `+08` | `00A7A570` | ECX object, stack immediate byte, RET 4. Set request byte `+51`; use current virtual `+14` and sample options to select deferred or immediate channel stopping. |
| `+14` | `00A7A630` | The established nonvirtual-channel query used by the stop body. |

The aggregate destructor's stop call is `008DBDF5`, with ECX equal to the
captured second pointee and stack flag zero. At `008DBDF7` it **reloads the slot**
after that callback. It decrements the actual reloaded object's `+4` exactly
once and only then calls its current virtual `+00` if zero. The two slot
destructors `004C3810` and `00524180` use the same actual decrement/current
zero-slot rule. They clear the slot after the callback; the aggregate's later
reverse destructor passes preserve callback repopulation behavior.

Thus, even for a positively established `D5ABF8` object, the zero-reference
entry is `00BD30E0`, which dispatches to current virtual `+04` with deleting
flags. `SoundChannelRuntime::release_reference` decrements its projected
`references_04` itself, and `SoundInstance` explicitly has a nonnative C++
layout. Neither is a compatible callback for the already-decremented raw object.
The conditionally recovered native virtual targets do not remove that Source
storage boundary.

## Unchecked Lua indexing does not supply the missing sound contract

Indices three, four and five in `008DBE90` numerically reach the second-array
addresses. The stored value remains a **sample-cache result**. This is a
potential out-of-range sample write, not evidence that those slots receive a
channel or event object. No authored-value count or zero-slot fixture is used
to suppress that distinction.

For the established actual sample profile `00D5B074`, the first two live DWORDs
are `00BD30E0` and `00A82E80`. The bytes immediately at `profile+8` are
`0A 4C 6F 6F` (`0x6F6F4C0A`), the beginning of adjacent literal data. They do not
establish a sound-stop virtual entry. A fourth sample reaching a second slot
therefore cannot justify binding `00A7A570` as that slot's stop behavior.

## Exact remaining edge and evidence limits

The next required attribution is a native instruction publishing an actual
nonnull sound/effect payload to `block+0C/+10/+14` (`owner+2CC/+2D0/+2D4`),
together with its input domain and the pointee's actual current profile at stop
and release. A raw Source allocation/storage producer and current-profile
stop/non-decrementing final-release implementation would then be necessary.
`008DD460 -> 00A7E490` now has a proven **stack** output destination; adding a
second-array assignment there would invent native behavior. The kind-one
factory's possible profiles were not expanded because they do not supply that
missing publication instruction.

All live batches used `bsp.py ghidra`, whose client verifies saved project `bsp`,
program `/battlestationspacific.exe`, x86 language and image base before querying;
configuration points to `C:/Users/sqz269/bsp.gpr`. The report embeds exact live
bytes and decoded disk instructions for **13 code spans / 2,655 bytes**, including
12 complete normal bodies and one bounded Lua-loader window. It also embeds
four data spans / 84 bytes. Every recorded live span matched the preserved
installed PE, and every code span decoded fully. The report records the binary
hash and the nine inspected Source/tool/config input hashes.

This is static native attribution and Source review. No build, fixture, native
code execution, GPR write, game startup or gameplay validation occurred. The
original PE was read only. RawGlobalConfig application admission remains open
at the specific publication-and-payload contract above.
