# Voice-line subtitle creation

`display_voice_subtitles_005b8510` reconstructs the complete control-flow and
call sequence of 005B8510. Native ABI is `__thiscall(line, NativeString*)`,
ECX = 38h line, one stack pointer, `RET 4`. Ghidra's zero-argument prototype
and exported `unaff_retaddr` are incorrect: 005B85CC reads the stack argument
at the original ESP+4. The named reconstruction is a hypothesis, not a
recovered symbol. Its typed C++ interface is not a binary replacement.

## Recovered behavior

1. Load the current manager through `[[00E198C4]+A4]`, clone manager+2C with
   null parent argument through 00AAB4C0, store line+14 and show the clone.
   Build the pooled 13-byte `Shortcut_Icon` name, recursively find the child
   with 00AA7E00, store line+34 and release the temporary before proceeding.
2. Compare the unsigned input length to zero, then reload the manager. When
   length is nonzero and byte 00F88989 is nonzero, show manager+30/+3C/+40
   in that order and submit the source to manager+34 and manager+38, both
   with localization enabled. Each pointer is read again after earlier calls.
3. Read manager+34 text measured width (+114), divide by binary64 960 and
   spill to float. Ask 00AB6BD0 for that same text object's normalized height.
   Add the binary64 height padding, then add binary64 0.0625 to the saved
   width and send the pair to manager+3C virtual +58.
4. Reload text+114 after that virtual call. Divide by 960, spill to float,
   multiply by binary64 0.5, add the binary64 decoration padding and spill
   again. Obtain manager+3C's resolved position through 00AA6750. Compute
   Y minus binary64 zero, Z minus binary64 one, then X minus the spilled
   offset, and set manager+40's resolved position through 00AA8240.
5. The empty/disabled path hides manager+30/+3C/+40 in order, then calls
   00ABBE50 on each text context with empty literal and flag 1. It does not
   skip cloning or the final line-state updates.
6. Reload the manager and walk backward from its +5C node, following node+0.
   Skip lines with null +14 widget; the first widget supplies its size.y
   plus its line+28. If none exists, use x87 zero. Write line+28, preserve
   the clone's local X and Z, apply this Y through 00AA7D00's recovered
   writes and publish local transform before local bounds. Show line+14 again.
7. Reload manager to write line+24 as signed UTF-16 code-unit length at
   text+EC times manager+7C plus manager+80, with no intermediate float spill.
   This uses text length, **not** the +110 line count. Reload manager for
   line+20 = manager+78, and reload once more for manager+60 = byte 1.
   Finally show the shortcut iff line+2C is nonzero. The final push retains
   manager-address upper bits after `SETNZ AL`; virtual +34 consumes bool AL.

## Canonical bindings

`VoiceSubtitleManagerView` contains references to manager fields, retaining
the captured manager identity while observing callback mutations of its
pointers. `manager_00e198c4_a4()` is invoked at all six native global reload
sites. Bind `last_line_5c` directly to the existing
`VoicePlaybackManager::lines_54.last_08`, never a second queue.

| Native manager field | View binding |
| --- | --- |
| +2C, +30, +3C, +40 | pointers to existing `GuiWidgetTransform` projections |
| +34, +38 | pointers to existing `GuiTextWidget` projections |
| +5C | existing `VoiceLineNode*` queue tail, aliased by reference |
| +60 | mutable byte `dirty_60` |
| +78, +7C, +80 | float `initial_78`, `per_character_7c`, `base_80` |

`VoiceLine::widget_14` and `shortcut_widget_34` must point to the canonical
`GuiWidgetTransform` projections. GUI factory, scene, font and localization
services remain the existing GUI modules' dependencies. The source directly
calls `clone_subtree`, `set_localised_source_00abaed0`, `widget_size`,
`resolved_position`, `set_resolved_position`, `recompose_local_transform`
and `refresh_local_bounds`. It does not introduce a second widget model.

`VoiceSubtitleHost` has required call contracts for virtual +34 visibility,
virtual +58 size, recursive child lookup 00AA7E00, literal submission
00ABBE50 and normalized height 00AB6BD0. No fallbacks are provided. The
existing layout-loader child lookup has a different `GuiLayoutWidget` tree,
so it cannot be silently substituted for the runtime transform projection.
The locale packet owns 00ABBE50 and is not changed here. The height callee
returns a float spilled/reloaded into ST0; the host must reproduce it, including
its font and multiline rules, rather than derive height from width or text count.
Projection accessors are side-effect-free and must expose live storage.

## Arithmetic and evidence

Local assembly implements the x87 arithmetic and float-spill boundaries.
Native double constants, verified from live bytes:

| Address | Binary64 bits | Meaning here |
| --- | --- | --- |
| 00CEC380 | 408E000000000000 | 960 width divisor |
| 00CEF290 | 3FB0000000000000 | 0.0625 width padding |
| 00CF0DE8 | 3F81111111111111 | height padding |
| 00CF0DE0 | 3F9DDDDDDDDDDDDE | decoration X padding |
| 00D7A280 | 3FE0000000000000 | 0.5 multiplier |
| 00D7A258 | 0000000000000000 | Y zero subtraction |
| 00D7A210 | 3FF0000000000000 | Z one subtraction |

005B8510's saved export was reviewed against live prototype/body metadata;
222 instructions cover 005B8510 through final instruction 005B8807 (a
five-byte jump, inclusive last byte 005B880B), total 764 bytes. The final
instruction is an out-of-line predecessor-height block that jumps backward
to the shared state update. It must not be truncated at `RET 4` 005B87F1.
Supporting live read-only disassembly verified 00AA67F0's position accessor
and 00AA7D00's XY/Z writes followed by 00AA7220 then 00AA70E0.
The live flow audit independently reported 222 listed instructions and zero gaps.
No Ghidra changes, comments, definitions or project saves were performed.

The routine is reconstructed against host projections. Concrete GUI helpers
retain their previously documented limits. Build and existing-check results
are recorded in `reports/voice_subtitles.json`: Release Win32 build and existing
`reconstructed_math` CTest (1/1) passed. That check does not execute this new
subtitle sequence. No tests were added. Native differential, original
ABI compatibility, full gameplay and visual validation are not established.
