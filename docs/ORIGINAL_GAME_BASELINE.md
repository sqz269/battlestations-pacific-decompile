# Original-executable USN04 baseline

This packet captures the installed original `battlestationspacific.exe` as a
behavioral reference for the reconstruction. It is separate from the rebuilt
`bsp_game.exe` execution census in `local/CODEX_BRIEF_coverage_run.md`.

Current status: tooling and protection probes exist, but no original mission
or motion trace has been captured. Four guarded startup attempts failed; the
full restricted mode passes DirectInput but independently fails FMOD init.
The source installation and four original save/profile files remain unchanged.

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
Documents before original entry. The usable-input mode is `medium-restricted`:
a full restricted token, a fresh effective-access audit denying every checked
mutation right on the actual install/save objects and ancestors, and a verified
private copy writable by that token. This uses no `WRITE_RESTRICTED` flag.
The low-integrity mode independently blocks source writes, but fails the
DirectInput call on this host and cannot supply a mission baseline.
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
   ./tools/run_original_baseline.ps1 -RunPrepared -StageRoot <private-stage> -RuntimeSeconds 900 -TokenMode medium-restricted -HandleKnownXLiveDebugClose
   ```

   Keep the launch receipt, chosen mission, inputs, difficulty, options, and
   runtime duration with the capture. The packet checks private writes,
   protected write/delete denial, DirectInput creation, Personal-folder
   redirection with flag bits, and the original non-Personal folder callback
   using a manifested x86 probe before the corresponding original-game run.
3. Navigate to USN04 using Computer Use. Record observed menu/briefing/mission
   transitions with `tools/original_baseline.py event`. Screenshots substantiate
   visible state; they do not establish numeric trajectory agreement.
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
pointer chain, and records failures. The offsets come from
`docs/MOTION_DIFFERENTIAL.md` section 3; the entry subtraction was rechecked
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

Live run pending. Initial inventory found four protected files, and its first
verification found no changes. The standalone debugger was restored using the
repository's pinned `pybag==2.2.16` and `comtypes==1.4.16` in an ignored private
virtual environment. The motion sampler and provenance tools are implemented;
their preparation checks do not count as mission execution.

The first original launch (PID 43732) exited before original entry with
`C0000008`, at runtime address `F00E5E6A`. The console session was active;
original executable and protected Documents identities remained unchanged.
No new xlive log was produced. The installed emulator contains a bad
`CloseHandle(GetModuleHandleA(NULL))` call, returning to xlive RVA `0017C270`,
before its logger initializes. This is a specific debugger-sensitive startup
candidate, pending live module/stack attribution. Microsoft's
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
permits testing a separate medium-restricted mode; it is not mission execution.

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
