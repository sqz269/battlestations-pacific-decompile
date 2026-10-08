# Genuine online peer bootstrap readiness

This bounded read-only audit did not identify the genuine compatible parent-server bootstrap. The existing transport implements both client and server modes, but the observed Original call path supplies mode zero. No new Source owner or launcher is admitted by the presence of the server branch.

## Original client and shared transport

The verified Original references are `A40F5E -> A4C250`, `A4C264 -> A4C030`, `A4C0EA -> A5DE34` and `A5DE50 -> A5E844`. A4C030 clears EBX at `A4C037` and pushes it as mode at `A4C0E9`; A5DE34 forwards that argument unchanged. Live references and a raw E8 scan across every executable PE section find exactly those callers of the four entries. No absolute pointer-byte references to them were found. This strengthens the previous Source-only observation; it does not prove absence of computed calls, external code or a separate launcher.

A4C030 creates the actual `2Ch` endpoint, opens the client pipe, establishes frame capacities/buffers and starts the worker before publishing its output. A negative open or later construction failure follows the existing cleanup and `_exit(0)`. Current `GameNativeOnlineProcess` retains the CRT globals, adapters and worker runtime; it creates neither an endpoint nor a parent server.

`A5E557` derives the client PID from the current process's parent in a Toolhelp snapshot. It opens that process with `400h` query access and accepts it only when its creation time is strictly older than the current process. `A5E844` formats `\\.\pipe\%08x` into 18 wide characters using that parent PID. The format at `00D25D80` has one observed reference, `A5E8D6`. The client opens the existing pipe for overlapped duplex I/O and requests message read mode.

For **any nonzero mode**, A5E844 instead uses its current PID and calls `CreateNamedPipeW(name, 40080003h, 6, 1, 400h, 400h, 5000, security)`. It denies the network SID and permits the logon SID. This complete branch exists in both the Original and current Source. No observed game-specific caller selects it, owns the compatible service, launches the child game or retains that service through the child's lifetime.

Creating the named pipe alone would not satisfy the client. Existing `native_online_ipc.cpp` has framed/encoded asynchronous exchanges, timeout/cleanup behavior, and an eight-byte marker/counter request with a strict marker/complement check on the response. This audit inspected that existing Source contract; it does not infer or implement a server from it. `XLive` and `BSP` function names remain analyst hypotheses, not recovered vendor provenance.

## Launch and installed-image evidence

The complete small `A3E560..A3E5C9` launcher is 106 bytes/31 instructions. It is the only observed Original `ShellExecuteExA` caller: it submits a zeroed `3Ch` request with caller-supplied executable/parameters, verb `open`, mask `400h` and show value 5. Its sole observed caller is `A40492` in notification drain. Current Source for that consumer launches `update.exe`, sleeps 200 ms and exits. Notifications can also be pumped inside construction; this is not evidence of an earlier parent-server bootstrap.

The Original's three `CreateProcessA` import references are in generic CRT bodies (`C02C75/C02DE5`, `C2A7B7`); indexed callers include Lua `io_popen` and CRT spawn wrappers. Those consumers can receive dynamic commands, so their existence neither identifies a title-specific server nor proves that none exists. The existing `0073CE20` Source parser handles cache/zip/frame/log/camera, IP/port/debug, automatic packaging/tests and memory-limit options. It has no peer/server switch. The inspected quoted Original parser literals corroborate part of that contract; other components' possible switches remain unestablished.

`scripts/run.ps1` prepares the pinned GFWL runtime and starts `bsp_game.exe`; it supplies no pipe server. The separate current Source native-data bootstrap starts a suspended child for data-band admission/handoff and has no pipe open/create call. That parent-child mechanism does not establish an online peer.

Nine installed images were pinned and inspected through PE imports, relevant ASCII/UTF-16 strings, version resources and related exports, without executing them:

| Candidate | Bounded result |
| --- | --- |
| Original game EXE | Exact parent-PID pipe format and client/server helper above; SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |
| Game-root `update.exe` | Version resource identifies the update launcher; no matching pipe import/string or genuine bootstrap contract established |
| Game-root `sus_prog.exe` | Unversioned AMD64 Rust debug image; no established Original-game provenance or matching pipe evidence |
| Game-root `xlive.dll` | Version resources identify AlterBSP/XLiveLessNess; retained separately from genuine SDK evidence |
| System `xlive.dll` | Microsoft 2.0.0673.0; SHA-256 equals the repository's verified redistributable payload; contains `\\.\pipe\GFWLive\` |
| Installed `GFWLClient.exe` | Microsoft 2.0.0675.0 managed client; contains the `GFWLive` namespace above |
| Installed `GFWLive.exe`, `GFWLUpdate.dll`, `XLiveServices.dll` | Microsoft 2.0.0675.0 components; bounded inventory establishes no matching parent-PID protocol/bootstrap |

The GFWLive namespace is not the game's parent-PID format. Its presence does not establish compatibility, and absent static imports/strings do not exclude dynamically resolved, managed or external behavior. No whole external helper body, genuine protocol owner or launch switch was recovered in this packet.

## Stopping boundary and validation

The next admission requires the **actual Original launcher/parent image and invocation**, a bounded producer that owns the matching transport/framing/request-reply service, and the real launch order, parent-PID relation and lifetime/retirement contract. No such producer was identified, so there is no honest next Source bootstrap packet from this audit alone. A simulated peer, invented switch, skipped IPC or ignored `_exit(0)` would not close the gap.

The six complete reviewed Original bodies total 1,891 bytes: A4C030 (501), A4C250 (44), A5DE34 (52), A5E844 (814), A5E557 (374) and A3E560 (106). Gap-free local decode covers 662 instructions; the bodies and a 36-byte data window containing the 28-byte pipe-format literal match the PE and pre/post live captures. The report retains 28 verified direct-call rows, 30 decoded direct IAT call sites, complete reference scans, 53 frozen Source/config/export inputs and nine installed-image identities. `0073D410` and A40DF0 were not re-expanded.

At final comparison, the primary checkout had a concurrent header adjustment making the renderer publication borrow `void* const volatile&`; that exact delta is retained separately and introduces no peer/bootstrap producer. Its target configuration also differed in line endings only. All other selected primary Source inputs matched the frozen worker inputs; all worker inputs and installed images were rechecked unchanged.

No Source or Ghidra changes, build, test, fixture replay, peer creation, SDK/account/network call, helper/service launch or game execution occurred. This is a static readiness audit, not runtime, native ABI or gameplay validation. Report: `reports/cc12_online_peer_bootstrap_readiness.json`. Evidence: `local/cc12_online_peer_bootstrap_readiness_evidence/` and its adjacent ZIP.
