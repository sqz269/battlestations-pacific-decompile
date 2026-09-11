# Startup movie, prompt and profile integration

Addresses: 00685070, 0067CC60, 007FDB20, 007F9290, 007F9340, 007FF100,
007FA710, 007FEFE0, 007FA670, 007FD780, 004F8970, 004F8A20, 004F89D0,
004F8F00, 00530F40, 00531F10, 00532DC0, 00AAFEE0, 00AADF40, 00AB0F90

## Scope and review

The second orchestrator works in `agent/orch2-20260910`, based at `4973646`,
with movie, prompt and profile workers in separate leased worktrees. The
other orchestrator continues render reconstruction on `main`. Ghidra batches
verify `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; all
mutations use the shared write lock. Workers keep Ghidra read-only.

The movie packet recovers controller/widget control flow and the default
game completion callback. The prompt packet recovers seven-slot selection,
preservation, responses and timed updates. The profile packet recovers reset,
name mirrors and read/write request ordering. See `GAME_MOVIE_PLAYER.md`,
`FRONTEND_PROMPT_SCREEN.md` and `GAME_PROFILE_RESET.md` for individual ABIs,
raw instruction evidence and remaining host calls.

Independent reviews found two profile defects before integration: both name
setters must mirror a nonempty display name in preference to the base name,
and the write-name comparison uses CRT C-string termination after checking
native header lengths. Their correction and verification are recorded in
the profile packet. Movie callback re-reads and adapter lifetime were also
reviewed against native instructions. The prompt review also caught queued
strings being released before their FIFO node was unlinked. The corrected
implementation splices the node out first, allowing release-time reentry to
restore the next prompt. The original fixture failed on the prior commit and
passed after correction. Native list-count decrement happens after node free;
the projected `std::list` size changes at unlink and is a documented limit.

## Startup callers

`logo_advance_or_finish` now calls the returning CRT invalid-parameter
handler in both original positions. A missing delay vector does not finish
the sequence: movie entry and all timestamp stores happen before that check.
The focused existing-target regression fixture repairs the vector in the
handler and checks the ordered events and resulting delay. Details are in
`GAME_TITLE_INIT.md`.

`StartupLogoMovieHost` binds the recovered callback registration, play and
enter functions. The caller supplies a plain completion function pointer;
no callback context is added to the native 34h movie object. The projected
C-string logo name is converted to an owning native string for this host
interface, copied into the widget by playback, then released. This allocation
is adapter scaffolding, not a recovered allocation in `00685070`. The fixed
player reference requires stable host ownership; it is not a replacement for
the native routine's repeated global-pointer loads.

`TitleProfileResetHost` connects `run_title_init` to the recovered profile
reset with shared profile/settings references. Remaining title services,
the logo clock, destruction, reentry and CRT handler stay abstract.

Press-start callsites `0067ce4c -> 007ff100` and `0067cea4 -> 007fa710`
request reading and writing respectively. Earlier host names reversed them.
The methods now carry their recovered request addresses; the two storage
predicates carry their virtual offsets. Native prompt text, branches and
side-effect order are preserved.

## Ghidra coverage and limitation

Nine previously undefined entries are defined from checked native byte
ranges, including prompt update and profile read completion. Their creation
and saved function bodies are recorded in `reports/startup_frontend_definitions.json`.
Reviewed names and evidence comments are applied after integration, preserving
old comments and logging prior values; affected exports are then refreshed.

The mission-progress destructor `007fd780` is deliberately only analyzed.
Its stored body ends at `007fd7c9`; the verified raw tail reaches the RET at
`007fd843`. The explicit `--tail-end` flow-repair option detects this otherwise
invisible truncation, verifies image bytes, decodes the tail and clears the
three CRT free overrides. Ghidra still reports the short stored function body.
The attempted body-extension endpoint reported script execution disabled;
no server settings were changed and the existing function was not recreated.
`reports/profile_tail_flow_repair.json` records partial repair, not a complete
decompilation. The tool now reports that distinction explicitly.

The prompt-list erase helper `00531f10` had a separate seven-byte gap after
the free at `00531f57`, containing stack cleanup and the list-count decrement.
The locked repair restored that block, left zero call gaps, saved the project
and refreshed its export. See `reports/prompt_list_flow_repair.json`.

## Validation boundary

All three worker builds passed MSVC Win32 Release and both existing CTests
after eight native seed checks. Each worker ran one ignored focused fixture:
movie callback replacement during close, prompt preservation/response/timeout,
and profile callback ownership plus review regressions. The logo regression
passed in the existing test target. Final combined build and annotation
results are recorded in `reports/startup_frontend_integration.json`.

The combined code at `53ddca9` passed MSVC Win32 Release and both CTests.
Movie `b6f62c5`, prompt `8d2a973` and profile `7b30145` are included. All
42 reviewed names/evidence comments were saved and read back with prior
comments preserved, and all 42 affected exports were refreshed. Nine new
function bodies reach their verified final instructions. Eighteen ignored
worker validation artifacts were retained with matching SHA256 hashes under
the integrator's `local/worker-validation/startup-20260910/` directory.

This is reconstructed and build-tested control flow with explicit codec,
renderer, GUI/input, score storage, archive and settings contracts. It is not
native ABI replacement, actual save I/O, movie playback or game validation.
