# Reproducible finite startup

The current Win32 reconstruction reaches and renders the title screen using the
genuine bundled Microsoft XLive runtime and a supported fullscreen display mode.
The new launcher prepares that runtime locally and waits for the actual GUI
process exit code:

```powershell
./scripts/run.ps1 -Frames 3
```

Build with `./scripts/build.ps1` first. The launcher derives the game root from
`config/target.json`; `-GameRoot` selects another installation. It extracts only
the verified GFWL 2.0.0673.0 package using the installed 7-Zip executable. The
installer and MSI are opened as archives and never executed. SHA256 identities
cover the redistributable, embedded CAB and all four resulting DLLs. An existing
file with an unexpected identity is rejected. DLLs and extraction provenance stay
under ignored `local/gfwl-private-runtime/`; no game or Windows DLL is replaced.

The launcher explicitly selects the private `xlive.dll` and preloads its matching
`msidcrl40.dll` through existing application switches. It selects the smallest
monitor, uses `--window-resolution fit`, and requests immediate presentation.
Settings use `local/run-personal/`; the log is `local/run/game.log`. These are
harness choices, not recovered game defaults. `-RuntimeDirectory`,
`-SettingsPersonalRoot`, `-LogPath`, and `-GameArguments` expose those choices.
Run this script with PowerShell 7, which supplies `ProcessStartInfo.ArgumentList`.

The previous default launch selected the installed game-local replacement
`xlive.dll`. During process attach it copied five NOP bytes to executable RVA
640F5E. The current executable image ends at RVA4D8000, so the write caused an
access violation before FMOD initialized. Selecting the genuine system XLive
alone then failed with Win32 error182: the current system credential DLL lacks
import ordinal43. Preloading the matching bundled credential DLL resolves that
loader dependency. The separate 960x540 fullscreen attempt failed Direct3D
device creation; `fit` selected a supported 1600x900 mode and succeeded. See
[the boundary audit](CC12_STARTUP_SOUND_BOUNDARY.md) for instruction and import
evidence.

Fresh offline extraction passed and the launcher subsequently completed a
three-frame run with exit code 0, a real Direct3D device, two presents, one skipped
present, and a 1600x900 screenshot. Audio startup issued 129 FMOD calls with no
reported FMOD errors. The title screenshot was inspected visually. A separate
30-frame run with the existing injected press-start action also exited with code 0 and
presented 29 frames, but its menu remained incomplete and recorded 77 unimplemented
host methods. Its screenshot is not proof of a working menu or gameplay.

The game C++ was unchanged for these runs, built at `7479af9fd`. This establishes
finite process startup, title rendering and the tested launcher configuration.
It does not establish faithful full startup/shutdown, sign-in, scenario gameplay,
save/load, or completion of the reconstruction goal. The three existing CTests
had passed for this C++ build; no new test suite was added for the launcher.

Evidence and exact local artifact identities:
[reports/cc12_reproducible_startup.json](../reports/cc12_reproducible_startup.json).
Runtime DLLs and screenshots are local artifacts and are not committed.
