# Parent-process pipe dependency

The recovered startup requires an already available parent-process pipe peer.
This bounded, read-only investigation identifies the client and the installed
launch configuration, but **does not identify an executable or library that
provides the original peer**. Loading the genuine Microsoft XLive DLL alone has
not been shown to provide it. The current installed EXE fails its embedded
Authenticode hash check, so its launch arrangement is not evidence of an
unmodified distribution's bootstrap.

## Native caller evidence

The complete known direct chain is `A4C264 -> A4C030`, then
`A4C0EA -> A5DE34`, then `A5DE50 -> A5E844`. At `A4C0E7..A4C0EA`, startup
pushes the output slot, the created stop event, and EBX=0. The wrapper forwards
that mode without changing it. Both pipe functions use native
`__stdcall(mode, stopEvent, output*)`, signed HRESULT in EAX, `RET 0Ch`.

`A5E8A7..A5E8C0` distinguishes zero from any nonzero mode. Zero obtains the
current process's parent PID through `A5E557`; nonzero obtains the current PID.
Both format the fixed UTF-16 string at `D25D80`, `\\.\pipe\%08x`.
The mode-zero branch opens that path at `A5EA95`; the nonzero branch creates it
through `CreateNamedPipeW` at `A5EA4F`.

Ghidra caller/xref queries found only the chain above. An independent scan of
the target's file-backed sections found exactly those three relative CALL/JMP
candidates and no literal DWORD references to their target addresses. The only
literal reference to the path is the immediate at `A5E8D7`. Thus **no mode-one
caller was found in this target**. Computed calls, transformed addresses,
runtime patches, and code absent from this image are outside that conclusion.

The CreateNamedPipeW IAT slot `CE21D0` has two raw references: the actual call at
`A5EA4F` and the otherwise unreferenced thunk at `A607EE`. The latter is
`FF 25 D0 21 CE 00`, a single terminal JMP of length6, exclusive end `A607F4`.
Ghidra currently has no function at that thunk. It is a real import thunk,
not a missing server caller or an interior branch requiring a new entry.

## Required peer behavior

The parent PID must survive the Toolhelp lookup and `OpenProcess(400h)` check.
Its creation time must compare strictly earlier than the child's. The matching
pipe must already exist when the client calls `CreateFileW` with read/write
access, no sharing, `OPEN_EXISTING`, and overlapped I/O. The client then selects
message-read mode2. There is no pipe-wait/retry path. A later IPC-create failure
cleans up and reaches the genuine `_exit(0)` path, not a normal startup failure
return; see [XLIVE_IPC.md](XLIVE_IPC.md).

The unused nonzero branch supplies the compatible server construction policy:
own-PID name, `CreateNamedPipeW` open flags `40080003h`, mode6, one instance,
1024-byte input/output buffers, timeout5000, and an ACL denying NETWORK access
while granting the logon group generic read/write. This establishes the
implementation's intended parent/server pairing. A different process could
choose the same name; the client does not authenticate the creator PID merely
by formatting it.

A valid handle is insufficient. The peer must exchange the actual encoded
frames and keep their protocol state synchronized. The game sends a two-DWORD
payload `{8, counter + 27h}`, increments its counter, and accepts only an
eight-byte reply `{8, ~current_counter}` after decoding. These are payload
requirements inside the recovered framing, not a substitute plaintext pipe
protocol. The first worker phase sleeps14897ms; subsequent I/O waits use5000ms.
See [XLIVE_PIPE_TRANSPORT.md](XLIVE_PIPE_TRANSPORT.md),
[XLIVE_PIPE_PROTOCOL.md](XLIVE_PIPE_PROTOCOL.md), and
[XLIVE_PIPE_GLOBALS.md](XLIVE_PIPE_GLOBALS.md).

## Installed artifact findings

The inspected game root is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`. Full SHA-256 hashes,
sizes, relevant string offsets, and the scope of each scan are retained in
`reports/xlive_pipe_peer_contract.json`.

| Artifact | Verified local evidence | Consequence |
| --- | --- | --- |
| `battlestationspacific.exe` | Eidos version1.2.0.0; target SHA-256 starts `b682a82c`; Authenticode `HashMismatch` | It is the current reconstruction target, but its signature does not validate these bytes as the original release. |
| Adjacent `xlive.dll` | Unsigned; Team Wolfpack; its version description identifies an XLiveLessNess adaptation/rewrite for BSP | It is a replacement runtime. No original peer role was established. |
| `update.exe` | Valid Eidos signature; description says Update Launcher; InstallShield UpdateLauncher PDB and patch-wizard strings | It is an updater candidate, with no matching pipe-format or protocol-constant evidence. |
| `sus_prog.exe` | PE32+; no company/version identity; Rust debug PDB basename `sus_prog.pdb` | Its role is unidentified. The filename is not evidence of a launcher or peer. |
| `C:/Windows/SysWOW64/xlive.dll` | Valid Microsoft signature; version2.0.0673.0; ASCII `\\.\pipe\GFWLive\` at file offset22376 | The observed GFWL namespace is different from the required PID pipe. |
| Installed GFWL Client2.0.0675.0 | Valid Microsoft signatures for `GFWLClient.exe`, `GFWLive.exe`, `GFWLUpdate.dll`, `XLiveServices.dll`; client contains UTF-16 `\\.\pipe\GFWLive\` at502594 and507104 | These artifacts do not establish the required parent peer. |
| `bsp.exe.cfg` / `bsp.exe.cat` | Config contains LIVE title/version/LAN settings; catalog has a valid Microsoft LIVE signature and members named `bsp.exe` and `bsp.exe.cfg` | Catalog membership and signing do not identify a launcher or validate the differently named, modified game EXE. No peer setting is present in the config. |

The twelve inspected binaries include the artifacts above, `xlivefnt.dll`,
`unins000.exe`, and `steam.exe`. None except the game contains the exact
ASCII/UTF-16 parent-pipe format or either sampled64-byte constant fragment
from target VAs `D25D9C` and `E12AC8`. This is an exact-byte negative result,
not proof that differently implemented, encoded, or dynamically loaded code
cannot provide the service. The uninstaller has an explicitly different
`InnoSetup64BitHelper` pipe name. PE32+ imports for `sus_prog.exe` were not
parsed; an empty import list is not claimed.

The current local Steam appinfo record for app8170 contains one launch entry:
`executable = battlestationspacific.exe`, with no arguments or wrapper field.
The appmanifest identifies `C:/Program Files (x86)/Steam/steam.exe` as
LauncherPath and installed build251889. The signed Valve Steam executable is
version10.96.30.42. The appinfo record is at file offsets157729..160266
(exclusive); the report records its file hash and parser bounds. This proves
the cached launch configuration, not the actual process parent or a peer
created by Steam. `8170_install.vdf` installs DirectX and GFWL prerequisites
and supplies no separate peer launch command.

## Provenance and runnable boundary

`PIPEIPC_*` names remain analyst-assigned, medium-confidence block labels.
The target has direct linked bodies rather than calls through a PIPEIPC DLL
import. Neither those names, the dense encoded tables, a Microsoft catalog,
nor proximity to XLive startup establishes vendor/library authorship.
No SecuROM, Sony, Microsoft, or other vendor attribution is asserted for this
protocol. No verified vendor implementation was found to bind instead.

A runnable reconstruction still needs evidence for the original process that
creates the PID-named server before launching the child and implements the
matching frame exchange. The precise next evidence is a verified original
distribution/bootstrap artifact and its launch path, or an authorized trace of
an existing original launch that records the parent image and the server's
CreateNamedPipe call. The current modified installation and its cached launch
entry cannot resolve that boundary statically. They also cannot justify a
fabricated successful peer, skipping the IPC call, or claiming that genuine
XLive loading completes startup.

This packet changes documentation only. Queries verified the existing BSP
Ghidra project/program; no Ghidra state or installed artifacts were written.
No game, endpoint, IPC worker, or process under inspection was launched.
No runtime handshake or build validation is claimed.
