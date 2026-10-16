# Native font normal-host runtime review 36

Independent read-only audit of Source commit
`a53cd233266d3494fb952acbae24b0ea87e736c9` and the immutable receipt family
`local/cc12_native_font_host_adoption_36/` in the integration worktree.
No source, Ghidra, build artifact, runtime receipt, or installed game file was
changed. The reviewer did not launch another build, test, or game run.

## Verdict

The receipts support normal-host production of six canonical native font
resources, their use by the existing sprite Text bridge for visible Viper19
glyphs, and successful explicit retirement before shared renderer/VFS drain.
Both bounded runs exit zero and log final device/API COM releases of zero.
No blocking evidence-integrity discrepancy was found.

This is a reconstructed-host startup/rendering result. It does not establish
complete menu rendering, Arial rendering, native Text/material submission,
complete gameplay, native failure/unwind behavior, or complete Original ABI.

## Frozen build and source provenance

`build_snapshot/manifest.json` identifies the Source commit above, canonical
build exit zero, and three existing checks. All 32 listed frozen artifacts
match their recorded byte sizes and SHA256 values. All 17 selected source/header
snapshots also match the current worktree byte hashes and the committed source
content after LF/CRLF normalization. This checks the selected frozen files; it
does not assert that the manifest freezes every repository source file.

Both runtime manifests identify the same frozen executable, Source commit, and
clean build/host checkout. Their actual executable hash matches the snapshot.
The frozen XLive stub and each runtime's archived copy under
`runtime.log.inputs/xlive_stub.dll` also match their receipt hashes. The
launcher stores `inputs[].archived` relative to that inputs directory, as
`tools/run_game.ps1:149-169` specifies.

| Artifact | SHA256 |
| --- | --- |
| Build snapshot manifest | `06803a027fd6a7458b0dc08739ccce2a0e3bae4cbf0fbc128c6042d80ceb42e9` |
| Frozen `bsp_game.exe` | `e5d438b35c96b8cd7becaf5924d69132e5b55012eaf99f66d23e96236f68e49c` |
| Frozen/archived `xlive_stub.dll` | `2960a7ec8bc88216d20ad4e5edc066f35394c5455496bd96cb08eb931a0f0353` |

The frozen build log reports `reconstructed_math`, `native_math_differential`,
and `tool_tests` passed, three of three. These are existing checks recorded by
the parent build; they are not dedicated runtime font or full-game ABI tests.

## Native ownership and actual sheet borrowing

Both logs, lines 141-146, record the same six font names and decoded counts:
ViperTitle 116, Arial16 207, Viper19 116, Arial20 207, Arial15 207, Arial18 207.
Their native image profiles are `00D61948`. Each process reuses three distinct
actual image identities across the fonts: two GFX sheets and shared alpha.
Six font owners do not imply six independent producer graphs or six images.

Line 149 in both logs records `same_registry=1 same_descriptors=1
semantic_resources=0 image_refs=transferred dat_allocations=sole`. The frozen
`game_hosts_fonts.cpp:79-93` checks those conditions before emitting success:
each row has no semantic `FontResources`, has its same live native descriptor
association, and corresponds to a completed operation whose decoded allocation,
unpublished owner, returned raw slots, and pooled names were transferred/consumed.

The frozen sprite adapter supplies the concrete identity path:

- `game_hosts_text.cpp:92-98` returns the same live native `FontData` allocation;
  an invalid native association cannot fall back to a semantic copy.
- `game_hosts_text.cpp:101-137` resolves the owning image in the same actual
  registry, verifies that its live companion borrows the raw `+04` reference
  count, checks the 2D image profile, and obtains its existing COM sheet.
- `native_renderer_binding_getters.cpp:9-12` reads that sheet from actual `+10`.
  The adapter does not cast the logical native image into a COM object, create a
  semantic wrapper, AddRef a new font sheet, or copy the glyph map.
- `game_hosts_frontend.cpp:701,1009-1010` still creates `GameTextHost` and draws
  the resulting borrowed sheet through the fixed-function sprite bridge.

The adapter supports a semantic arm for other callers; the normal host's six
rows in these runs pass the native-only invariants. Source plus the successful
guards and receipts establish the exercised association. The logs do not contain
a separate per-glyph address trace or an ABI-equivalence measurement.

## Bounded run results and screenshot inspection

Both PNG files were inspected with `view_image`; each is 800 by 600.

| Evidence | `runtime_01` | `runtime_02` |
| --- | --- | --- |
| Loop ticks / successful presents / skipped presents | 2 / 1 / 1 | 8 / 7 / 1 |
| Synthetic press-start | disabled | requested at frame 1, injected |
| Menu state at summary | `PressStartPoll`, state 2 | `ScreenVisible`, state 5 |
| Built Text runs / glyphs | 2 / 55 | 2 / 55 |
| Text quads at final visible state | 55 | 9 |
| Fonts actually named in built Text runs | Viper19 | Viper19 |
| Logged concrete / unimplemented host methods | 123 / 43 | 148 / 74 |
| Exit code | 0 | 0 |

Run 01 visibly contains the map background, `MAIN MENU`, and the full prompt
`LOG INTO ALTERBSP THEN PRESS ENTER TO CONTINUE`. Log lines 356-358 attribute
46 prompt glyphs and nine title glyphs to Viper19, totaling 55 sprite glyph quads.

Run 02 visibly contains the flag/header/footer and `MAIN MENU`, with a blank
dark menu body. Lines 454-458 record the screen transition and a sprite rebuild
with nine remaining text quads; summary line 494 records `ScreenVisible`.
This proves the bounded transition and header display, not a complete menu body.
Neither run contains a built Arial Text run, so loaded Arial owners have no
visual Arial-drawing proof here. ViperTitle is loaded but is not a drawn Text run.

Both logs explicitly mark `GuiText::create_glyph_buffers` and
`GuiText::ensure_draw_sections` unimplemented (353-354). The normal frontend
does not install `GameNativeTextApplication` or its native factory, and native
frame command counts remain zero in these receipts. GUI shader/effect probes
exercise their separate native resource paths; their success does not make the
visible sprite text native Text/effect submission.

## Retirement and installation preservation

Each log has exactly one font-host retirement record with six fonts and cleared
registry bindings. The recorded sequence is independently ordered as follows:

| Event | Run 01 log line | Run 02 log line |
| --- | --- | --- |
| Native font host retirement | 368 | 471 |
| Shared singleton lifetime drained | 372 | 475 |
| Native renderer after raw drain | 373 | 476 |
| Final COM release `device=0 api=0` | 564 | 723 |

The retirement records also state sprite borrowers closed, before device
release, and before shared drain. Frozen host source orders frontend destruction,
explicit font retirement, then raw drain. The receipts therefore corroborate the
normal successful order reviewed in `CC12_NATIVE_FONT_HOST_RETIREMENT_REVIEW_36.md`.
They do not exercise interrupted loading/retirement, fallback destruction,
device recreation, or concurrent callback behavior.

The installed original executable at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`
was independently rehashed. Its SHA256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`, equal to
the snapshot's pre-run receipt. The manifests launch the frozen reconstructed
executable and place personal settings beneath the local run directories.
The hash claim concerns the original installed executable; no complete installed
asset-tree hash was supplied or inferred.

Receipt/log/screenshot hashes and the machine-readable result are recorded in
`reports/cc12_native_font_runtime_review_36.json`. None of these results expands
the producer's existing successful-path source boundary into AD4C30 original
transport/SEH/pool equivalence, AD53A0 native-tail equivalence, or AD51D0 reload.
