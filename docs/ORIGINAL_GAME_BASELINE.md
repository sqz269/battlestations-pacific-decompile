# Original-executable USN04 baseline

This packet captures the installed original `battlestationspacific.exe` as a
behavioral reference for the reconstruction. It is separate from the rebuilt
`bsp_game.exe` execution census in `local/CODEX_BRIEF_coverage_run.md`.

Captured the installed original running USN04 on 2026-10-29 (local time):
400 valid motion samples over 20 wall-clock seconds, with zero read failures.
The selected USS Lexington moved 303.05 metres and reported 16.10–16.20 m/s
forward speed; the visible HUD showed 31 knots / Full. All 400 samples matched
the controlled-unit global before and after the read. This supplies a native
motion reference; reconstruction parity has not yet been measured.

## Provenance

The installed executable is x86, preferred image base `00400000`, entry
`00BFD2BD`, 12,223,752 bytes. Its SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The installation is at `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
Its data contains Kantai Kessen references and replacement `xlive.dll`/`dxgi.dll`;
an installed-original capture must not be described as a clean vanilla capture.
The provenance receipt records the actual runtime DLL hashes and selected data
hashes. No equivalence between the installed DLL and a local source checkout is
assumed.

## Preservation and isolation

`tools/original_baseline.py inventory` reads the executable/runtime hashes and
inventories the actual Windows Documents known folder's
`Battlestations-Pacific` directory and the install's `profiles` directory.
`verify` compares file identities and timestamps afterwards. Evidence stays in
ignored `local/`; the tool never restores, overwrites, or changes source data.

Environment variables alone do not isolate the original game's Documents
access. Its `SHGetSpecialFolderPathA` import at `00CE22F0` requests
`CSIDL_PERSONAL`. The guarded launcher redirects that import to private
Documents before original entry. The successful mode is `medium-write-locked`:
a medium-integrity write-restricted token, an effective-access audit, and
69,700 inherited read/list handles opened without delete sharing on the source
objects and ancestors. The audit denies other mutation rights; these handles
cover the remaining parent delete-child permission. Independent parent handles
remain held until the owned job is empty. The receipt confirms job exit before
their release. This contract protects the unchanged trusted game; it is not a
hostile-code sandbox. The private copy remains writable. Low integrity fails
DirectInput on this host; full `medium-restricted` mode impairs FMOD init.
Only private staging directories may receive modified permissions/labels.
The original EXE on disk remains byte-identical. The folder-lookup hook is
instrumentation and must be disclosed with the run.

Use normal priority, one baseline at a time, the shared run-slot protocol, and
honor `C:/Users/sqz269/.bsp/DRAIN`. Check `query session` on startup/renderer
failure. Never stop another agent's process. A preparation receipt or successful
safety probe is not an original-game mission capture.

## Capture procedure

1. Record provenance and protected-file identities:

   ```powershell
   python tools/original_baseline.py inventory --game-root "I:/SteamLibrary/steamapps/common/Battlestations Pacific"
   ```

2. Prepare the private game copy and copied options/profile data, then start it
   through the guarded launcher. The wrapper retains a dedicated original-game
   lock and a shared run slot throughout the live run:

   ```powershell
   ./tools/run_original_baseline.ps1 -PrepareOnly -StageRoot <new-private-stage>
   python tools/original_baseline_guard.py prepare-medium --stage <private-stage> --receipt <private-stage>/guard-medium-prepare.json
   python tools/original_baseline.py native-options --stage <private-stage>
   $env:_NO_DEBUG_HEAP = '1'
   ./tools/run_original_baseline.ps1 -RunPrepared -StageRoot <private-stage> -RuntimeSeconds 3600 -TokenMode medium-write-locked -HandleKnownXLiveDebugClose -GameArguments @('universe/Scenes/missions/USN/usn_19_coralus.scn')
   ```

   Keep the launch receipt, chosen mission, inputs, difficulty, options, and
   runtime duration with the capture. The packet checks private writes,
   protected write/delete denial, DirectInput creation, Personal-folder
   redirection with flag bits, and the original non-Personal folder callback
   using a manifested x86 probe before the corresponding original-game run.
3. The installed mission tree maps USN04 to the scene above. The original
   `.scn` startup path loaded it directly: the log records `Loading scene` and
   `GGame::SceneInit()`, and screenshots show USS Lexington and the objectives
   `Protect the USS Lexington!` and later `Sink the Shoho!`. This is native
   direct-scene startup, whose setup has not been equated with campaign-menu
   startup. No manual movement inputs were used during the trace. Keep the
   game foreground: this build pauses when it loses focus. Record transitions
   with `tools/original_baseline.py event` and retain screenshots.
4. The guard owns the debug connection throughout the run. After USN04 is
   visibly running, create `discover-motion.request` in the private stage. This
   arms its one-time `00825F20` breakpoint. At entry, `unit = ECX - 310h`;
   `[ESP+4]` is incoming engine `dt`. The guard records PID/registers/unit/current
   controlled pointer/x87 control word in `launch.json`, restores the byte and
   instruction pointer, and resumes automatically. The marker can be used again
   after a transition; do not attach a second debugger concurrently.
5. Use the recorded PID and unit, or try the sampler's read-only
   `--discover-controlled` mode for a directly controlled ship. An exact
   controlled-global match is recorded separately; a different pointer leaves
   the control relationship unverified. Sample the fixed chosen ship:

   ```powershell
   python tools/original_motion_capture.py --pid <owned-PID> --unit <hex-unit> --seconds 20 --hz 20 --output local/original-baseline/usn04-motion.csv --expect-sha256 b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6
   ```

6. End only the owned run, retain its real exit/termination reason, and verify
   protected data:

   ```powershell
   python tools/original_baseline.py verify --before local/original-baseline/before.json
   ```

## Meaning and limits

The sampler requests only process-query and memory-read rights. It checks the
original executable's PE identity and hash, follows the unit/controller/body
pointer chain, and records failures. Schema 2 follows the native complete body
`B = [controller+2Ch]` to motion state `M = [B+4h]`; velocities are at `M+0/Ch`,
not directly in `B`. See `docs/ORIGINAL_MOTION_ABI.md` and
`docs/MOTION_DIFFERENTIAL.md` section 3. The entry subtraction was rechecked
against the live saved Ghidra program for this packet. The installed EXE's first
32 bytes at `00825F20` also match that saved program's bytes.

Samples use wall time and unsuspended, non-atomic remote reads. CSV `dt` and
`engine_dt` stay empty; `sample_dt` is a measured sampling interval. Pauses,
breakpoint discovery, slowdown, repeated states, and unknown random seeds must
not be described as a deterministic engine-tick replay.

`tools/motion_trace_compare.py` can compare a trace with ship-probe output once
class, initial state, orders, and time alignment are established. The probe's
ocean/gameplay assumptions still limit such a comparison. Matching function
coverage alone does not establish behavior, and a motion capture alone does
not establish objective, combat, AI, or visual parity.

## Results

Attempt 6, original PID 32132, produced
`usn04-original-motion.csv` and its schema-2 metadata from
`2026-10-30T00:56:17Z` through `00:56:37Z`. The sampler completed 400/400 reads,
skipped no deadlines, and ran for 20.0002385 seconds. The first-to-last sample
span was 19.9502816 seconds; net displacement was 303.053343 metres and the
wall-time secant speed was 15.190429 m/s. Reported forward velocity ranged from
16.101055 to 16.201943 m/s; maximum absolute yaw rate was 0.000470290 rad/s.
The difference between wall displacement/time and native velocity must remain
visible: this is external sampling, with rendering/focus/timing effects and
non-atomic reads, not a fixed-tick integration comparison.

The controlled unit was `2A32B7B0`, controller `17B41CD0`, complete body
`1A19BFB8`, and motion state `1A1BD3C8`. A separate one-shot entry observation
at `00825F20` recorded engine `dt=0.05000000074505806` and x87 control word
`027Fh`, but on another unit (`42B59148`), not Lexington. It does not fill in
the trace's missing per-sample engine delta.

After capture, the quit dialog did not complete. The operator terminated only
the verified private PID 32132. The original child exit was `FFFFFFFFh` and the
guard returned failure; the raw receipt's `crashed` phase reflects nonzero exit,
not a newly observed spontaneous gameplay crash. Capture completion and forced
cleanup are recorded separately. Source handles were released after job exit.
Final verification found all four original options/save files unchanged,
including timestamps; original EXE and xlive DLL hashes also still matched.

Evidence is retained under the main checkout's ignored
`local/original-baseline-20261029/`: trace/metadata, before/after inventories,
attempt receipts and logs, source audit/lock manifests, screenshots and report.
`attempt06-controlled-invalid-body-layout.json` is diagnostic evidence from
before the pointer fix; its velocity values are invalid and are not baseline
data. The selected trace uses the corrected schema-2 layout.
The original source-lock manifest receipt hashes LF-normalized text; its
Windows text writer emitted CRLF. The archive retains the raw file plus
`attempt-06-manifest-digest-note.json` with both hashes and the matching
normalized digest. The writer now explicitly emits LF so future receipt
digests match file bytes directly. This formatting correction does not alter
the recorded source paths, file identities or held handles.

Earlier startup diagnostics are retained below to explain the usable mode.

The first original launch (PID 43732) exited before original entry with
`C0000008`, at runtime address `F00E5E6A`. The console session was active;
original executable and protected Documents identities remained unchanged.
No new xlive log was produced. The installed emulator contains a bad
`CloseHandle(GetModuleHandleA(NULL))` call, returning to xlive RVA `0017C270`,
before its logger initializes. This is a specific debugger-sensitive startup
candidate, confirmed by the second attempt's live module/stack attribution. Microsoft's
[CloseHandle documentation](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle)
explains the extra invalid-handle exception when a debugger is present.

The second launch (PID 96968) established the folder hook and confirmed that
exact installed-emulator call in the live stack. Opt-in handling continued
only the first-chance invalid-handle exception with the matching DLL hash,
call instruction, and adjacent return/main-module argument. The game then
initialized renderer/shaders and crashed at `00A98342`, dereferencing a null
DirectInput interface. Its actual child exit was `C0000005`. The guard initially
returned zero for that crash; a focused nonzero-child probe exposed and fixed
that reporting bug. Guard/launcher success must never substitute for a mission
reached with child exit zero.

A manifested x86 probe using the same DirectInput call returned `S_OK` at medium
integrity and `E_FAIL`/null at low integrity. Privilege-removal controls isolated
low integrity as the cause. A `WRITE_RESTRICTED` candidate was rejected because
the real Documents directories retained delete-child permission. The candidate
full restricted token uses flags `DISABLE_MAX_PRIVILEGE` without
`WRITE_RESTRICTED`, restricting Users/current-logon/World SIDs, and a token-only
DefaultDACL addition for Users/current-logon. It supports DirectInput and denies
file writes, security/owner changes, deletion, and parent delete-child access in
owned fixtures. A read-only effective-access audit of 69,700 actual source
objects and ancestors also denied every checked mutation right. This evidence
permitted testing medium-restricted mode, but its FMOD failure prevented use.
The successful write-locked mode retains normal reads and covers delete-child
permission with verified source handles; focused probes cover 4 and 69,700
inherited handles, denied source deletion/rename/write, and allowed private writes.

Attempt 3 (PID 15144) reached a different failure: sound-cache alias removal
`00A855EC` read an uninitialized pointer (`BAADF00D`). Attempt 4 (PID 85096)
requested `_NO_DEBUG_HEAP=1` and failed at the same instruction with different
garbage, so the normal heap is not a sufficient fix. An independent manifested
FMOD probe using the installed version `00041804` found nine drivers in every
mode; normal medium and write-restricted medium returned init result zero,
while full restricted medium returned 61. No sound was played by that probe.

The copied settings also need native-reader normalization. `008D8190` opens
text mode, allocates the physical `ftell` length, ignores the translated `fread`
count, and scans the entire backing. The 226-byte source file's 15 CRLF pairs
therefore leave 15 indeterminate tail bytes. `SoundEnabled` consumes only its
key and `HardwareReported` has no text-reader handler, although the native
writer emits both. `tools/original_baseline.py native-options --stage ...`
produces a separate 177-byte private LF-only file omitting those two ineffective
lines, retains the source bytes as a private backup, and writes a receipt.
The launcher checks the original profile against its original hashes and the
private options against this separate receipt. It never substitutes private
hashes for the original preservation check.

Attempt 5 reached the original main menu under write-locked protection, then
hit its bounded 900-second deadline before mission entry. Attempt 6 used the
native scene argument and reached the mission without local-profile UI setup.
Neither the earlier timeout nor the final forced stop is a normal exit.

For future scripted orders, prefer a command queue drained from a verified
game-thread update/input hook after initialization. Each native call still
needs its exact x86 calling convention, live object lifetime and state checks.
Arbitrary remote-thread calls and fabricated engine ticks have not been
validated. No native movement/order injection was needed or implemented for
this capture.
