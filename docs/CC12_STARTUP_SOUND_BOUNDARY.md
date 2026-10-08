# CC12 startup sound boundary: XLive attach patch and genuine SDK route

The pre-sound access violation came from the installed game-local `xlive.dll`
while Windows loaded it, before FMOD or the sound manager startup ran. Its
attach code copies five NOP bytes to the host executable base plus fixed RVA
`00640F5E`. That RVA belongs to the original game layout and falls outside the
reconstructed executable. The current Source passes the sound/startup boundary
when the existing options select genuine Microsoft XLive and its matching
credential dependency. No audio Source fix or installed-file patch was needed.

The primary agent subsequently completed a bounded three-frame run of the same
unmodified Source with `--window-resolution fit`: device creation succeeded,
two frames presented, one skipped, the loop finished with exit zero, and a
1600x900 screenshot was written. Forty-six host methods remain unimplemented.
This establishes limited startup/presentation, not gameplay or complete parity.

## Fault and fixed-RVA evidence

The no-debug subprocess capture records `0xC0000005` after 0.3433566 seconds,
with empty stdout/stderr and a last log record containing the settings summary.
The earlier PowerShell GUI invocation's shell zero was not a reliable child
exit code and is superseded by that capture. The first debugger-only
`0xC0000008` at `NtClose` is separate from the subsequently captured AV.

Two later debugger captures stop on the same write AV at XLive EIP `69BB9049`,
module RVA `00319049`, instruction `89 17` (`MOV [EDI],EDX`). EDI is `10640F5E`
and EDX is `90909090`. XLive is loaded from the installed game directory at
`698A0000`; loader frames carry DLL reason 1 (`PROCESS_ATTACH`). The stack
reaches `LoadLibraryExW` through the Source XLive loader. Stack unwinding lacks
full symbols, so the nearest exported-name label is not treated as the actual
faulting function name. The instruction/registers, module bounds and retained
dump provide the concrete fault evidence. No FMOD module load precedes it.

Read-only PE disassembly independently establishes how that address is made:

| XLive RVA | Evidence |
| --- | --- |
| `0016B76A..0016B77F` | Push null; call the `GetModuleHandleA` IAT entry; store the returned executable base at DLL global RVA `004D8450`. |
| `0018C242..0018C252` | Construct five bytes of `90` in a local buffer. |
| `0018C264..0018C271` | Read that executable-base global, add literal `00640F5E`, and pass destination, buffer and length five to the copy call. |
| `00319047..00319049` | Read a source DWORD, then write it through EDI: the exact fault instruction. |

The reconstructed PE is based at `10000000`, with `SizeOfImage=004D8000`; the
live debugger confirms `10000000..104D8000`. Its computed target `10640F5E`
is outside that image. The original installed executable is based at `00400000`
with `SizeOfImage=00E2F000`; target `00A40F5E` lies in `.text` and contains
`E8 ED B2 00 00`, one five-byte CALL. Thus the fixed original-layout write is
proved from the DLL bytes and runtime destination, rather than inferred merely
from the absence of the sound marker.

`SoundServices` constructs `XLiveLibrary` before core sound. Its loader calls
`LoadLibraryExW` before `GameSoundRuntime::startup`, raw sound/auxiliary manager
registration, the lifetime reorder, clock copy or the first EventSystem call.
Those later boundaries were candidates in the initial log-only diagnosis;
the AV capture rules them out as the site of this fault. The math binding at
`game_hosts.cpp:1990` repeats the same binding already made before the logged
renderer/settings work at line1885.

## Genuine SDK and required dependency

Current Source resolves an unspecified XLive path against the game working
directory. `--xlive-dll` supplies an explicit absolute path instead;
`--xlive-dependency` records absolute preload paths in order. `XLiveLibrary`
loads those dependencies before XLive and keeps them alive until after XLive
unloads. This existing route is sufficient for the verified launch.

The genuine `C:/Windows/SysWOW64/xlive.dll` is Win32 Microsoft GFWL
2.0.0673.0, 14,303,392 bytes, SHA-256
`79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385`.
Its Windows Authenticode signature validates as Microsoft. It exports every
ordinal imported by the original game. Its direct dependencies include the
VC90 CRT through an embedded assembly manifest and `msidcrl40.dll`.

Selecting that system XLive alone produced a normal Source error, Win32 182,
and exit one. The current system credential DLL is a 15,872-byte module with
SHA-256 `f86c15642ddbe787d015a8e801835bed54e1a30d404d9d8fa6042a6794e27427`.
PE comparison finds exactly one missing ordinal among XLive's 17 imports
from it: ordinal43, `ExportAuthState`. The retained genuine legacy credential
DLL is 1,089,440 bytes, version5.0.737.6, exports that ordinal and has a valid
Microsoft signature. Its SHA-256 is
`623cb6ca98e566357abbd0e76b15713921e1d7e1144c0c4f589a0407c7eff1ee`.

The primary copied four already extracted, genuine redistributable files into
its ignored `local/gfwl-private-runtime/`; hashes were independently rechecked:

| File | SHA-256 |
| --- | --- |
| `xlive.dll` | `79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385` |
| `msidcrl40.dll` | `623cb6ca98e566357abbd0e76b15713921e1d7e1144c0c4f589a0407c7eff1ee` |
| `ppcrlconfig.dll` | `649557d6349ea0658808952c1bbf9a111ec2283c29345c7f904919fd599e5d61` |
| `xlivefnt.dll` | `a07e20c09a0c3eac2ed3e6288d67060e82b70595053153866b1cfb4f9958f9a4` |

The matching files were found in the retained `orch3-20260910` worktree and
match `docs/XLIVE_PRIVATE_RUNTIME.md`'s bundled-redist extraction provenance.
No installer was run or game/Windows file changed. All 27 direct XLive
import-DLL candidates are available, including installed x86 VC90 CRT
9.0.30729.9635; with the legacy credential selected, each directly imported
name/ordinal exists in its inspected candidate. Static dependency availability
is distinct from the primary's successful live load below.

The installed FMOD DLL hashes remain
`31e7451aef6115b0aec353e4e508ff9f0fc14ea3a8957e5ddb805ede911700e8`
(core) and `4b6ae7ba8d3da780c23abb5e72a65ea50e5348a60a1b4aef43196e09ae771890`
(Event). Both are Win32 SDK4.18.4 and all13 inspected early exports are present.
Historical FMOD result61 was not the current fault.

## Tested boundary and reproducible launch recommendation

Every primary run here used Source revision
`7479af9fd70b41091bc077e9ea98b84dbc45f0bf` and executable SHA-256
`cb346fc3918014f20dfd57d22525ed3d1dd6e9aa8a3406cf4e7183044f17989a`.
The inspected21 chain source/header files match that clean integrator snapshot.
With explicit private XLive and legacy credential preload, sound initialization
and streamed-dialog initialization passed, followed by Phase6 parsers:
129 FMOD calls, zero errors. The 960x540 fullscreen run then failed separately
at D3D `CreateDevice`, `8876086C`, exit4. Changing only the requested resolution
to `fit` completed the bounded startup run at1600x900: device HRESULT0,
three loop ticks, two presents and one skipped present, exit0. The log records
VFS probes3/3, six fonts, GUI pages5/5 with29 widgets, and55 text glyphs.
The primary's screenshot review reports the title/map/login prompt; this
worker pins the image and its dimensions rather than claiming a gameplay test.

The practical retained launch route is a small local launcher that resolves
and hash-checks those four private files, then invokes the existing serialized
`tools/run_game.ps1` with explicit `-XLiveDll`, forwarding the matching
`--xlive-dependency` and `--window-resolution fit`. Keep an isolated settings
root and distinct log/screenshot names. The wrapper's current default selects
`xlive_stub.dll`; a direct game launch's current default selects the game-local
DLL that made the invalid write. Therefore the retained launcher must supply
both genuine paths explicitly and fail if its checked runtime is unavailable.
No new fallback, no-op DLL, skipped attach or binary patch is justified. This
worker recommends the route only; the primary owns launcher implementation.

The exact successful argument vector, DLL hashes, Source revision, executable,
logs, debugger dump and screenshot hashes are in the tracked JSON report.
The successful result/log/image hashes are respectively
`0a4881bc921f8caac682cacfd5fd6a40e4da3a607dd1cd0c4c70e8e7092c3553`,
`d7b49c01e9338dc599b21fc0a7ae955a4a02238f0fa58a29200a2a3740f06f47`, and
`47166e904a913122b4993a0a426b216667518469cc0219cb9dc3596fde4b55f2`.

Final evidence is retained in this worker's
`local/cc12_startup_sound_boundary_localized_20261008/`:50 files,
manifest SHA-256 `4e640c9ba53f278d994a887c47c47a44baa0429719e0b4a945036104a2f42f3c`.
The 233,774,274-byte primary dump is externally pinned as
`895305456d9c172914155351da0216d50abc908611cdff22270fe823cb28a940`;
it is not duplicated. The initial log-only archive is preserved separately.

This worker edited only this document and its report, ran no game process,
made no Source/configuration change, and made no live Ghidra query or mutation.
The finite startup evidence does not establish audible sound, complete
shutdown semantics, account/network service operation, missions or gameplay.
