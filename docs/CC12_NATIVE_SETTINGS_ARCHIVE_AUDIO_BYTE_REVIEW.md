# Native settings archive audio byte review

The finite audio descriptor region in `008D6DC0` supplies destinations at settings
offsets `+20h`, `+28h`, `+2Ch`, and `+30h`. None names the sound-state byte at
`+24h`. Its 84 instructions contain 36 explicit memory writes, all to stack
records, and no local receiver store or block-copy instruction. This is actual
Native evidence for that region. It does **not** establish that the archive
reader preserves `+24h`: the four indirect providers' write widths and side
effects, and the rest of the large reader, remain unreviewed.

This read-only packet owns only `008D6DC0`, `008D79A0`, this document, and the
matching JSON report. It changes no Source, build registration, saved analysis,
prototype, listing, flow, game files, or runtime environment. Source1992 is
accepted context only; this review adds no build, startup, ABI, or game credit.

## Positive receiver and descriptor evidence

The assigned caller `008D79A0` loads the current storage object through
`0109CECC`. At `008D79BF` it tests that object's dword at `+8`; a nonzero value
jumps to `008D7A29` and skips the archive-read branch. In that branch the caller
calls `004425C0` with `ECX=ESP+20h`, then pushes the current `ESP+0Ch` address,
loads `ECX=00F88980` at `008D79E8`, and calls `008D6DC0` at `008D79F5`.
The first call's reader-construction role is the current Source interpretation;
its stack cleanup and installed table have not been recovered here.
The reader takes its stack argument into `ESI` at `008D6DDB` after 36 bytes of
pushes/locals, and copies `ECX` into `EDI` at `008D6DF7`. There is no explicit
`EDI` reassignment before the audio region. Its retained epilogue ends in
`RET 4`. These are listing-level observations; stored Ghidra prototypes remain
`undefined(void)` and were not repaired.

| Saved pseudocode key | Tag | Destination LEA | Actual target | Current virtual call |
| --- | --- | --- | --- | --- |
| `masterVolume` | 2 | `008D6F8C`, `[EDI+20h]` | `00F889A0` | `008D6FAF` |
| `musicVol` | 2 | `008D6FDC`, `[EDI+28h]` | `00F889A8` | `008D6FFF` |
| `sfxVol` | 2 | `008D702C`, `[EDI+2Ch]` | `00F889AC` | `008D704F` |
| `speechVol` | 2 | `008D707C`, `[EDI+30h]` | `00F889B0` | `008D709F` |

Each call reloads the current reader table from `[ESI]`, selects slot `+0Ch`,
and assigns `ECX=ESI` before `CALL EAX`. Tag and destination records are made
on the stack. Four `MOVSS` instructions read the cell at `00D7A24C` and copy
its contents into stack default records; they are not stores to settings
`+24h`. The cell's value and the four key strings were not fetched in this
packet. Key names above come from retained pseudocode; their referenced
addresses and the descriptor instructions come from the actual listing.

The current Source labels tag 2 as `GuiLuaFieldType::Float` and its value and
fallback implementations store one float. Under that **Source** contract the
four destination ranges are `F889A0..A3`, `F889A8..AB`, `F889AC..AF`, and
`F889B0..B3`, which do not overlap `F889A4`. The Native caller does not yet
positively bind its constructed reader to those implementations. A wider
Native write beginning at `+20h` could overlap `+24h`; this review claims no
Native provider width or absence of indirect/global writes.

## Timing and remaining boundaries

After a successful read, the caller conditionally frees its buffer, clears
the storage object's `+30h`, closes the archive, and destroys the reader.
It then captures the pending callback at `00F88958`, clears that cell, and
invokes the captured callback when nonnull. The storage object's identity,
aliasing, and all those callees' effects remain outside this packet.

The accepted sound-state writer review established the constructor's
`008D7736` byte store of 1 to settings `+24h`. Its accepted startup evidence
places the `008D8190` load at `0073DAA5` before the sound-state comparison at
`0073DAEE`. Those retained inputs are preserved here without reopening their
Native bodies. The assigned archive caller does not establish its invocation
order relative to `0073DAEE`. A storage-state gate is not proof of startup
order. This packet finds no new zero writer or supported pre-comparison
archive selector and does not justify bypassing the FMOD startup failure.

The reader interval `008D70A1..008D76A1`, preceding indirect providers,
keyboard/compatibility/string/downloaded-content/renderer paths, and external
callback schedules remain explicit frontiers. No whole-reader or call-graph
write-absence conclusion follows from the four descriptors.

## Retained evidence and replay

The report freezes 430 complete current Source/Git files from 46 selected
seeds, all 790 quoted-include edges, six accepted documents/reports, and 168
retained artifacts from the accepted writer review. All 40 previously admitted
Source files have equal LF contents. It retains the complete original PE
(`12,223,752` bytes; SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`),
the six complete saved exports for the two assigned functions, and four bounded
physical/listing windows totaling 933 bytes and 262 instructions: reader prefix
417/122, audio 320/84, caller 175/48, and reader epilogue 21/8. Retaining a
complete export does not claim review of its unselected instructions.

Five typed, read-only `bsp.py ghidra` commands captured current prototypes and
the four windows. Each used the configured `bsp` project, program
`/battlestationspacific.exe`, x86 language and `00400000` image base through the
client verifier, with autostart disabled. `C:/Users/sqz269/bsp.gpr` is the
configured project path; the bridge does not independently expose that full
path. Every physical window matches both its original-PE slice and its saved
live bytes. No GPR file was read or written. The saved `show` command for the
audio region prints 83 lines; the independently retained physical/listing
window and live byte capture cover all 84 instructions, including the final call.

Run `python local/native_settings_archive_audio_verify.py` from the worker
worktree. The replay checks immutable pins, Git blobs and quoted includes,
accepted artifact copies, original PE/header/window identity, saved/live listing
coverage, all descriptor/write predicates, and nine whole-file-backed Source
excerpts. It decodes only the four assigned Native slices offline and recomputes
the audit without changing its receipt. It runs no compiler, test executable,
Ghidra query, SDK, or game. Historical original-path fields are metadata;
retained copies are the replay inputs.

## Recommended next bounded packet

After primary acceptance, review the actual constructed reader's slot `+0Ch`
provider. The assigned caller positively identifies the call to `004425C0`.
Its construction role and stack cleanup require confirmation in that body.
Current Source comments suggest `00BD68D0` for the dispatcher and `00BD61C0` /
`00BD63B0` for value/default handling; these three are candidates, not proven
Native bindings. A proposed lease covers those four function entries and
`docs/CC12_NATIVE_SETTINGS_ARCHIVE_FLOAT_PROVIDER_REVIEW.md` plus its matching
report. Establish the installed table and actual slot first, then trace tag-2
destination width and effects. Any newly identified table/data address or
different callee needs explicit refreshed ownership before inspection.
Reuse this frozen caller evidence. No candidate body or unleased table was
opened by the present packet, and this recommendation authorizes no Source or
runtime change.
