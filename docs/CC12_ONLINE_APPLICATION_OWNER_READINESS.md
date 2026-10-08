# Online application owner readiness

`0073DC7C` calls `00A40DF0`, whose complete raw-storage constructor already exists in `native_online_manager_lifetime.cpp`. The missing work is application ownership and genuine context composition. `GameNativeOnlineProcess` owns real CRT globals, pipe service adapters and the permanent IPC worker runtime; it explicitly **owns no endpoint**. Current `GameStartupHost` still leaves the actual manager and pump publications null.

A small nonnull caller arm is ready as a context-injected normal-return Source fragment. A complete production owner or menu activation is **not ready**: the compatible pipe peer, stable constructor-pump inputs/preimages, later publication producers and complete retirement graph must first be supplied. Ignoring missing IPC, selecting an invented offline status or skipping the constructor's pump would change the recovered behavior.

This is a read-only audit. The worker changed only this document and [cc12_online_application_owner_readiness.json](../reports/cc12_online_application_owner_readiness.json), with no Source, SDK/account/network/game execution, build, tests or Ghidra mutation. The report pins 50 current Source/header inputs, all byte-equal to the primary checkout at sealing, and retains bounded Original/analysis artifacts. Prior constructor fixtures in [NATIVE_ONLINE_STARTUP_R103.md](NATIVE_ONLINE_STARTUP_R103.md) are historical evidence, not newly executed live owner or peer proof.

## Original startup boundary

Only `0073DC50..0073DC9A` was decoded as the caller fragment: 75 bytes and 15 instructions. The giant `0073D410` was not reconstructed. Its relevant register/EH context was checked separately. The whole target is `00A40DF0..00A41026`, 567 bytes and 129 instructions. Every byte in both regions matches the installed Original PE and refreshed live Ghidra; their gap-free decode addresses match the listing. The read-only verifier passes 17 direct calls across the regions; the `GetUserDefaultLangID` IAT call is identified separately.

The caller requests `3F0h` bytes at `0073DC55 -> 00BF681B`, retains the allocation in its stack temporary, and records parent EH state `14h`. The nonnull arm pushes `00735520` then `00735510`, sets ECX to that allocation and calls `00A40DF0`. After return, `0073DC81` reloads **current `F8ABE8`**, then `0073DC87` writes `00737D60` to that current owner's `+18h`. It does not use the captured allocation or constructor EAX for this store. The reload/store also follows the allocation-null arm unconditionally; silently returning on null would not reproduce the full caller schedule. The next-system test and parent state reset follow at `0073DC8E/94`.

The target's native ABI is ECX = actual owner, two callback DWORD stack arguments, EAX = that same owner, `RET 8`. ESI retains the owner and EBX supplies zero. Its exception registration names `00CB427C`; observed state 0 follows base construction, then state 1 follows embedded-vector initialization. Existing C++ cleanup is vector then base; it does not invent IPC or SDK shutdown during constructor unwind. Parent/FH3 machine-handler equivalence is not established by this audit or by an ordinary Source owner.

The complete constructor performs:

1. Base construction, publication and singleton registration, then only the observed derived stores and callback `+20h/+24h` assignments.
2. Actual renderer device lookup and a separate reload of renderer `F8D394` for the mutable `+1A28h` presentation pointer; real LANGID and `XLiveInitializeEx`.
3. Actual `+3ACh` IPC slot creation, `XOnlineStartup`, full 400-byte `XWSAStartup` output handling, raw version check and conditional cleanup, full NTOHS return to the port setter, then listener creation/storage.
4. Concrete sign-in reset and **the complete native online pump**.
5. Only after the pump returns, clear `+14Ch` and `+12Ch`, without freeing a buffer a nested callback may have installed.

The startup adapter already forwards real ordinals 5297, 5310, 1, 2, 38, 84 and 5270 plus `GetUserDefaultLangID`, borrowing an already loaded library. Failed SDK return codes are not converted to success or generally treated as exceptions. The actual WSADATA version is read even after partial/failed writes.

## Exact constructor-pump inputs and ownership

| Required input | Existing provider or publication | Application obligation |
| --- | --- | --- |
| One actual `3F0h` manager | `NativeOnlineManagerStorage`; raw base/derived constructor and destructor family | Default-initialize the real allocation, preserve actual preimages, form every dependent context around that same identity; no whole-blob zeroing or projected `XLiveOwnerAllocation` |
| Base publication and registered deletion | `GameNativeSettingsProcess::online_00f8abe8()`, `GameSingletonHost::manager_publication_01090aa0()`, `native_deletion_bindings().native_online` | Install the complete lifetime context before base registration; keep mutually exclusive `xlive_owner` null; retain the context through the shared scalar drain |
| Renderer and startup adapter | Actual `GameNativeRendererApplication` publication; `NativeOnlineStartupSdkRuntime` | Borrow the canonical cell and same SoundServices XLive library. The public renderer accessor is `void* const volatile&`, while startup context currently requires `void* volatile&`; adjust the read-only borrowing interface faithfully instead of copying the pointer into a detached cell |
| Process IPC services | `GameNativeOnlineProcess::ipc()/pipes()` | Use the existing permanent services to create the actual raw `2Ch` endpoint into manager `+3ACh`; process initialization alone does not create it |
| Query/clock runtime | Existing `SoundServices::signin`, same module and actual clock publication | Retain the canonical clock, output image and query runtime; the initial pump samples the clock twice |
| Notification drain | `NativeOnlineNotificationRuntime`, `NativeOnlineNotificationOperation`, `NativeOnlineNotificationContext` | Supply defined id/parameter/invite-stack preimages, raw temporary headers and actual notification/client publication cells; retain them across callbacks and cleanup |
| Profile/update leaf | `NativeOnlineNotificationLeafRuntime` and `NativeOnlineProfileCallbackContext` | Use actual manager/game/string publications. When `00737D60` is reached, the game must have a constructed profile at `+650h` and name mirror at `+1FF0h` |
| Sign-in UI | `NativeOnlineSigninUiRuntime(library, LocaleTextResolver)` | Retain a genuine resolver, locale tables, caller-selected current Lua context when substitution is reached, and the explicit 28-byte sign-in-info preimage |
| Raw strings | `GameVfsHost::raw_strings()` | Borrow the same actual `AA8/AA4/AA0` domain in UI, notification and profile contexts; do not construct another pool |
| Storage/achievements | Existing raw SDK resolvers and complete pump providers | Resolve from the same loaded module; supply matching allocation/free and actual invalid-parameter/memmove CRT entries; retain manager/module/buffers through pending operations |
| Mutable pump globals | `E0E3EC` first-sample byte and `F8ABFC` saved float | Own and share the actual source cells with verified loader values 1 and zero bits, preserving pump updates |
| Input/platform publication | Existing shared online cell and `SoundServices::online_pump` borrowed by input/platform loading-message services | Publish that same actual pump and manager; keep the contexts stable through all consumers and normal shared drain, then retire consistently |

`NativeOnlinePumpContext` needs the complete `NativeOnlineNotificationContext`, sign-in UI calls and preimage/CRT, storage SDK, shared clock output and both mutable timing cells. The notification context in turn requires manager, operation, three publications, notification/sign-in/profile/update calls, actual strings, achievement imports, allocation memory and CRT. These are genuine dependencies of a constructor that already pumps; an empty owner wrapper cannot defer them until the next frame.

Preimages are material. The constructor preserves the vector prefix at `+360h`, overlap words `+398h/+39Ch` and other unassigned bytes. The initial pump can inspect `+12Ch` before the final constructor clear; the reset provider does not initialize it. Explicit current-process heap/stack capture may reuse the existing audited `capture_current_process_xlive_pipe_bytes` machine-byte leaf, over valid distinct owned ranges, to define representations without inventing zero defaults. Such capture is current-process evidence, not proof of Original allocation/stack byte equality or arbitrary pointer validity.

The initial and later callback requirements differ. A40DF0 clears `+18h` before its first pump. Only the caller installs `00737D60` after return. When later reached, that callback has no null-game fallback and needs the actual profile owner. The loader initializes `F8ABEC`, `F8ABF0` and `F8A2FC` to zero; live Ghidra matches those virtual `.data` zero-fill bytes. This permits the observed initial state, not a permanent missing-producer shortcut. Later `004E555E` installs `004CEB40` into `F8ABEC`; current Source exposes `install_startup_callback` only as an interface and call, with no production implementation. The actual session/client publication likewise has no current production owner.

The UI resolver also needs honest admission. Existing `LocaleLuaContext` can operate on a caller-selected actual current Lua state, but the private `GameLocaleTextRuntime` in `game_hosts_text.cpp` returns neutral `"invalid"` for context substitution. Reusing that behavior does not establish arbitrary online-UI fidelity. Use established real context selection or constrain only paths whose actual inputs prove that dependency is unreached.

## Genuine peer and retirement boundary

The concrete raw IPC initializer calls `open_00a5de34` with mode 0. The real transport finds the current process's parent PID and opens `\\.\pipe\%08x` using that PID. Nonzero mode contains a server implementation, but current Source callers of the open path use mode 0. The normal launch script starts `bsp_game.exe` with its genuine SDK/dependency paths and provides no demonstrated compatible server bootstrap.

A negative open or later creation failure runs existing cleanup, then `_exit(0)`. It is not an ignored HRESULT that A40DF0 can harmlessly pass through. A compatible real server and protocol must exist at the expected name for the admitted normal-return path. This audit does not establish which component supplies the genuine server, or whether genuine SDK initialization might establish relevant external state. It did not open, emulate or probe a pipe, launch a game, or call the SDK. A simulated peer or missing-peer ignore mode would not answer this gap.

The process owner retains its real CRT globals and immutable worker runtime until process exit. Applications must close their endpoints before CRT callbacks destroy the locks. The raw endpoint destructor sets the stop event, waits 1000 ms, ignores that wait result and then frees storage; the existing implementation preserves this behavior and does not prove a safe worker join. The application must establish normal lifetime/quiescence admission rather than claim that a retained services object cures the endpoint hazard. SDK/pool/renderer/context retention through the singleton drain and handling of interrupted partially published graphs are separate obligations.

## Smallest honest next entry

The smallest ready Source fragment is the admitted nonnull arm `0073DC70..0073DC8D`: accept an already allocated actual manager through its real `NativeOnlinePumpContext` and `NativeOnlineManagerStartupContext`, invoke complete A40DF0 with `735510/735520`, then reload the context's canonical `F8ABE8` cell and store `737D60` at current `+18h`. This can be named descriptively and implemented without expanding giant `0073D410`; it must be labeled a fragment, not a whole native function or complete application owner.

That fragment does **not** close allocation/failure policy, the context factory, genuine peer, pump publication or later consumers. A complete `GameNativeOnlineApplication` would own stable operation/preimage/context storage, establish the raw deletion binding before registration, invoke that schedule, and retain its borrowed services through normal shutdown. Current Source does not yet supply that complete closed graph. Immediate unconditional `GameStartupHost` or menu wiring is therefore not ready.

The useful next bounded work is to establish genuine peer/bootstrap provenance and the actual context/preimage producer factory, then close later notification/profile/locale producers as reached. Keep existing request/context-four code, initial pump behavior and real failure paths intact. Finite startup/presentation evidence remains separate from successful online initialization, functional menu progression and gameplay.
