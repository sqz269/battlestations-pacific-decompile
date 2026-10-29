# Movie subtitle source dependency, 2026-10-28

Packet `codex-movie-subtitle-source` selectively recovers the subtitle loader and
schedule callbacks from `52f52c7432` on source baseline `d0a2656fe`. The switch
`kMovieSubtitlesBound` stays **OFF**. This is a source dependency checkpoint,
not acceptance of subtitle playback or the complete native `00AB0520` body.
No ledger records, CMake registrations, installation files or saves are changed.
The historical ON flip `d98cadc27e`, stale documentation and old pair receipt are
not imported.

## Original Source and ABI

Before the analysis batch, `config/target.json` was checked for project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`;
`bsp.py ghidra ensure --status` reported that program live and `ghidra count`
matched the saved 77,629-function snapshot. No Ghidra mutations were made.
Evidence was read with `bsp.py lookup/show`, including assembly for register
inputs and float stores.

- `00AB0520..00AB0EA2`, descriptive hypothesis
  `BSP_GuiMovie_LoadSubtitles`: the assembly saves ECX as the movie at
  `00AB053E`, reads the stack path at `00AB057F`, and ends with `RET 4` at
  `00AB0EA0`. The live Ghidra auto-signature currently lists no parameters;
  it is not used as ABI proof. The new C++ interface is not binary compatible.
- The function clears first (`00AB0545 -> 00AB00C0`), probes `.mws` through
  `[0109CEEC]` vtable+8 into movie+104h, selects the wide/normal Text child,
  then opens its own Lua owner and reads `subtitles` and `extratitles`.
- `00B67720` pushes a numeric key and calls `lua_gettable`. The recovered
  loader uses that lookup operation rather than the parked patch's raw lookup.
- `00AB08E0/00AB0912` store start/end as float32. `00B66270` already narrows
  `lua_tonumber` to float32. The typed loader also explicitly narrows to float.
- `00AB0935/00AB0969` pass default zero to `00B662F0`. That helper accepts
  exact Lua BOOLEAN (type 1), otherwise returning the supplied default.
  The parked loader's truthiness conversion was corrected: numeric, string
  and table values do not become true flags.

`movies/us_outro.mws` in the installed game was read without modification. It
contains one plain subtitle row with literal false/true flags. This is data
inspection, not execution or a census of every packed/virtual subtitle asset.

## Implemented boundary and retained limitations

`load_movie_subtitle_script_00ab0520` creates and closes a separate Lua state,
executes supplied source, and projects subtitle rows into `MovieSubtitle`.
No libraries are opened. The original owner's library set remains unknown;
the helper does not reproduce the native owner/reference storage or all
malformed-script error propagation. It does not process `extratitles`.
There are no new global objects, static constructors or registrations.

The bridge has schedule preparation, selection and clearing behind OFF. Its
OFF callback method names and unimplemented logging are preserved. New ON-side
logs name only the schedule projection, not completion of native widget bodies.
Later profile/session source changes on main are retained.

The following dependencies still block full activation:

1. Movie page loading and child lookup return null in this bridge, leaving the
   selected movie absent on the normal player path. Callback reachability must
   be established, not inferred from an unchanged gameplay pair.
2. Decoder opening and open-handle checks are unimplemented. No real
   BinkGetRealtime triple supplies the bridge's clock fields; zero frame rate
   marks absence rather than fabricated playback time.
3. Preparation uses a working-directory-relative stream, not the native
   file-store probe or a supplied game-root/VFS binding. Schedule parse success
   is a separate projection flag and is not claimed to reproduce movie+104h.
4. Text child selection, localisation and display are absent. The retained
   selected key is logged, not displayed. Native extratitles and cloned-widget
   lifetime/clear behaviour remain outside this packet.
5. Native subtitle options +14h and the language comparison are not bound.
   Their false fields are marked unavailable inputs, not recovered defaults.
6. The original Lua library/error contract and the complete native storage,
   ABI and ownership paths still need their own evidence and bindings.

The existing ledger already credits a `00AB0520` reconstruction. This packet
adds partial helper coverage and **zero additional native-function body credit**.
It does not establish ABI compatibility, visual playback, gameplay acceptance
or an ON verdict.

## Validation and next gate

The owned worktree passed `scripts/build.ps1` (MSVC Win32 Release, existing
four-node build). Both existing CTest checks, `reconstructed_math` and
`tool_tests`, passed (2/2, 20.77 s). The first build caught MSVC C4702 in the
constant OFF guards; explicit constexpr if/else blocks corrected that warning.
Both build logs are retained. Results and source/artifact hashes are recorded
in `reports/cc16_movie_subtitle_integration.json`; no new tests were added.
Root supplied the saved accepted `d0a2656fe` baseline executable hash
`574b8a0f84a8fb24284e832207a4a353d3a7c06c70b6140b4d61dbfef2b773fd`.

No game or ad hoc probe is launched by this packet. Root owns later baseline
identity, callback reachability, decoder/widget/settings dependency work and
matched Normal gameplay/playback gates. Keep OFF until those obligations close.
