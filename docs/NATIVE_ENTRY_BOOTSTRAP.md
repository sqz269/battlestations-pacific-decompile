# Native entrypoint and online startup gates

The configured target enters the ordinary, identified CRT startup and then
`008F81F0`. No executable entry trampoline, PE TLS callback, or explicit
self-launch bootstrap was found. On the normal-return application path,
online initialization is reached after the window callback, with no offline
option or XLive-result guard. The required parent-PID pipe peer remains
unidentified; loading XLive alone has not been shown to supply it.

This audit also finds a concrete reconstruction mismatch: **native WinMain
passes the literal `cachedload`, and the application parser reads that
argument; `GameStartupHost` currently parses `GetCommandLineA()` instead.**

## Exact image entry

The configured file is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Its PE32 image base is `00400000`; AddressOfEntryPoint is RVA `007FD2BD`,
VA `00BFD2BD`, file offset8377021. Disk and Ghidra agree on the entry bytes:

```text
00BFD2BD  E8 9C AE 01 00  CALL 00C1815E  (__security_init_cookie)
00BFD2C2  E9 16 FE FF FF  JMP  00BFD0DD  (___tmainCRTStartup)
```

The terminal JMP starts at `BFD2C2`, length5, exclusive end `BFD2C7`.
The following bytes belong to a different function. Both TLS directory RVA
and size are zero, so this EXE supplies no PE TLS callback array. Delay-import
and CLR directories are also zero. Its four sections are `.text`, `.rdata`,
`.data`, and `.rsrc`. The security directory is the certificate table at file
offset `BA7000`, size5384. The earlier audit's Authenticode `HashMismatch`
still limits release authenticity; agreement with Ghidra establishes which
bytes were analyzed, not that the distribution is pristine.

Ghidra identifies `___tmainCRTStartup` and `__cinit` as Visual Studio2005
Release library matches. They remain CRT boundaries, not new reconstructed
game functions. At `BFD24F` the CRT calls `008F81F0` with image base, null
previous instance, the result of `__wincmdln`, and the selected show value.
CRT allocation/runtime-initialization errors can terminate before WinMain.
`__cinit` first checks C initializers at `CE36E8..CE3708` (exclusive), then
invokes the nonnull C++ initializer entries at `CE2734..CE36E4`. The latter
contains343 nonzero entries in1004 slots. Their complete transitive behavior
was not audited here. Table hashes and counts are in the report.

After those initializers, CRT tests an optional callback at `0109FED8` and
would pass `(0,2,0)` only after its nonwritable-image test. The slot is zero
in Ghidra and lies in the PE's zero-filled `.data` tail; its static xrefs are
only the reads in `__cinit`. This does not introduce an EXE TLS bootstrap.
Imported DLL initialization still precedes the EXE entry and is outside this
bounded control-flow proof.

## Gates from WinMain to IPC

| Stage | Native condition and consequence |
| --- | --- |
| `008F81F0` COM/Game Explorer | Failed COM initialization/security or failed CoCreateInstance continues toward startup. If Game Explorer exists, vslot18 receives the output BOOL; `8F82AC` tests that output, ignoring the method HRESULT. Output zero calls `_exit(0)` at `8F82F0`. Unwritten output is not a defined permission result. |
| Single-instance check | Only a nonnull mutex handle plus `GetLastError()==B7h` takes the already-running message/return path. Failed CreateMutexA does not block startup. |
| Application dispatch | `8F8429` calls `73D410`; `8F8432` then calls the platform loop without testing a return value. The incoming WinMain command-line and show arguments are never read. |
| `73D410` setup | First-time singleton, allocation, debug/settings, and logging branches select subsystem work. A raw normal-return CFG has1162 instructions and one return at `73E52A` (`RET8`, length3, exclusive end `73E52D`). No path reaches that return while bypassing the online allocation at `73DC50`. Callee exits, exceptions, and indirect callback behavior are not assumed to return. |
| Window callback | `73DC25` invokes platform vslot4. At `73DC27`, EAX is overwritten with the title-string pointer before cleanup; its callback result is not tested. Window failure is therefore not an online skip branch in the caller. |
| Online allocation | `73DC50` requests3F0 bytes. `73DC6E` skips the constructor only for null allocation; nonnull calls `A40DF0` at `73DC7C` with callbacks `735510`/`735520`. Immediately afterward, `73DC81..73DC87` dereferences global `F8ABE8` and writes callback18=`737D60`. The null branch does not establish a supported offline mode. |
| Manager initialization | `A40DF0` stores the XLiveInitializeEx result, then calls `A4C250` at `A40F5E` without testing it. `A4C250` calls `A4C030` at `A4C264`. The current owner implementation preserves this order. |
| IPC failure | As established in [XLIVE_IPC.md](XLIVE_IPC.md), later create failures clean up and call `_exit(0)`. A successful mode-zero open requires the already existing parent-PID pipe. |

Disk bytes and Ghidra also agree at `73DC50..73DC8E`, `A40F40..A40F63`,
and `A4C250..A4C27F` (exclusive ranges). No patched-out call or added gate
is present in those inspected on-disk spans.

## What the command-line parser actually receives

At `8F841E` WinMain pushes the pointer `CE8168` (`"cachedload"`), then pushes
zero at `8F8423`, sets ECX to the application, and calls `73D410`.
Application initialization lowers ESP by148h after its prologue. Thus:

- `73D4C2: MOV EBP,[ESP+150h]` loads the second native argument, `cachedload`.
  The shader/debug substring scans use this EBP value.
- `73D926..73D92D` stores the first argument, zero, into `E1AE7C`.
- `73D933: MOV ECX,EBP`; `73D93C` duplicates the string through `438E40`;
  `73D945` stores it in `E1AE78`; `73D94A` invokes parser `73CE20`.

The sole known application-initialize caller is WinMain. Consequently the
configured path feeds this fixed string to the parser, regardless of OS launch
arguments. The parser's13 recognized token forms are documented in
[APP_INIT_BOOTSTRAP.md](APP_INIT_BOOTSTRAP.md): cachedload, nozip, fixfps,
1frame, filelog, freecam, ip:, localport:, connectport:, debug:, auto,
memlimit, and nomemlimit. None is an online/IPC-disable option. The earlier
substring flags genshaders, devshaders, hiresmode, reloadresources, and devrr
likewise do not select such a branch, and this entry supplies none of them.

`src/winmain_startup.cpp` correctly forwards `(0, kApplicationInitializeMode)`.
In the parent checkout inspected at `96e7d2e`,
`GameStartupHost::application_initialize` logs but discards `mode`, and
`run_initialize_phases` parses `GetCommandLineA()` at lines706..707. That
changes the native parser input. The standalone parser remains useful with an
explicit supplied string; the error is at its caller's binding. No source was
edited in this evidence-only packet.

The current executable host also remains a partial startup implementation:
`game_main.cpp` calls `run_win_main`, while `game_hosts.cpp` documents missing
sound/later owners and has no call to `construct_xlive_manager_00a40df0`.
The separately reconstructed owner has the genuine IPC call. A successful
partial executable run therefore does not validate the native online path or
resolve the missing peer.

## Process creation and self-launch search

All literal references to the imported CreateProcessA slot `CE20D0` are calls
at `C02C75`, `C02DE5`, `C2A7B7`, and the unreferenced thunk `C2F2AA`.
The first two are in the stream-mode/command-shell function `C0297A`, whose
only direct caller is Lua `io_popen` at `A64703`. The third is in the CRT
spawn family `C2A630`, reached through `C1B2C4`, `C1B33C`/`C1B04E`, and
the COMSPEC/command.com/cmd.exe function `BFFC8A`; its non-CRT caller is
Lua `os_execute` at `A63A22`. The Lua functions are referenced by their
registration tables, not direct startup calls. Exact rel32 and literal-pointer
results are retained in the report. Scripts or indirect calls were not
assumed incapable of invoking these general facilities.

The only ShellExecuteExA call is `A3E5B7` in `A3E560`, reached from update
notification handling at `A40492`. Its unused import thunk is `A4D3EC`.
The normal initial notification pump is after IPC creation, so this launch
cannot supply the peer needed by that earlier open. It uses an executable and
parameters from notification handling; no self-launch claim follows from the
API name. No CreateProcessW, ShellExecuteExW, or WinExec import/name was found.
There is no additional known mode-one pipe call; see
[XLIVE_PIPE_PEER_CONTRACT.md](XLIVE_PIPE_PEER_CONTRACT.md).

This supports **no explicit bootstrap in the inspected native entry chain**,
not a proof covering imported DLL initialization, every static initializer,
arbitrary script execution, computed addresses, or runtime patches. No vendor
or DRM attribution is established. The genuine remaining dependency is still
the original parent/server launch arrangement and compatible frame peer, not
a fabricated success path or an inferred command-line bypass.

The report records two existing CRT SEH listing gaps with exact instruction
boundaries. They contain filters/handlers, not missing ordinary entrypoints;
no new functions or Ghidra writes are requested by this packet. Verification
used read-only PE parsing, bounded Ghidra/raw assembly, byte comparisons,
and JSON/whitespace checks. No inspected process, game, pipe, or worker was
started, and no build/runtime validation is claimed.
