# Oct18B library binding B: telemetry and XLive

Audit-only supplement, 2026-10-18, branch `agent/oct18b-libB-net`, base
`3d581c328`. No host implementation, reconstruction, Ghidra mutation, game run,
SDK execution, account operation, or installer execution was performed.

## Census and identity

The current index has **362 telemetry-family functions** using
`tag_category='block_telemetry' OR coalesce(ledger_name,name) LIKE 'TELEMETRY_%'`.
The literal snapshot-name prefix has 360: 32 `TELEMETRY_tmpl_*` and 328
`TELEMETRY_iostdnet_*`, from `0094CAE0` through `00A5DDA0`. The other two are
`00A586D0 BSP_String_FormatFixed80` and `00A586F0 BSP_String_FormatFixed30`.
These are inventory counts, not 362 individually proven binding contracts.

This is the statically linked **iostdnet + iometrics** library, with a game-side
Eidos metrics client. Identity is supported independently of the prefix:

* PE RTTI strings include `iostdnet::ZString` at `00E0F2B0`,
  `iometrics::ZHttpTransmitter` at `00E0ED84`, and the ordered-map/tree template
  spellings at `00E0E700` and following addresses.
* `00A52560` installs the two ZString/IHashable vptrs, uses the allocator in
  `00F8AF18`, and allocates shared string data with length at `+4`, reference
  count at `+0`, and 16-bit characters from `+0C`.
* `00A50210` installs transmitter vptrs at `+0` and `+10`, constructs its URL,
  queue and synchronization/thread members. It is not a Winsock import thunk.
* The original image contains the endpoint at `00D012E8`:
  `http://bp-g4w-game.metrics.eidos.co.uk!0x534307E8.xlsp:1000`.
  The game client copies it in `src/cc15_seg73_41_61.cpp:283`.
* The PE imports XLive by ordinal, but has no separately imported telemetry DLL.
  `00A5CFE0` explicitly calls the XLive `XOnlineStartup` thunk after installing
  allocator callbacks; the telemetry network support depends on XLive too.

The namespace/endpoint identification is high confidence. An exact external
SDK release, redistributable binary, or compatible source package was not
established. Do not infer one from the generic word telemetry.

## Original callers: both indexed and disk-byte evidence

`bsp.py callers` was cross-checked with `tools/callsite_census.py`, which scans
`.text` E8/E9 rel32 calls/jumps and `.text/.rdata/.data` absolute dwords. Live
instruction context checked the important positive callsites. A raw census's
nearest function start is an attribution hint, not a recovered boundary.

| Target | Original callsite and interpretation |
|---|---|
| `00757FE0` game metrics constructor | `0073E4AC CALL`, inside `0073D410 BSP_Application_Initialize`; constructor calls `007524D0` metrics init. |
| `00A50210` HTTP transmitter constructor | `00752627 CALL`, inside that metrics init. Current source equivalent is `src/cc15_seg73_41_61.cpp:294`. |
| `00A51D40` library startup | Indexed caller `00752020`, callsite `0075208A`. Disk scan additionally finds `0075084D`, absent from Ghidra's listing. Raw bytes establish a separate short body starting `00750840`: allocator call at `00750845`, startup call at `0075084D`, `MOV AL,1; RET`. The census labels it nearest owner `00750820`, but that scalar deleting destructor ends at `0075083D`; **do not attribute this extra call to it**. |
| `00A4D610` string stream writer | `00751859 CALL` from game `00751850 BSP_TelemetryStream_Write`; also `00A506C9` from iometrics stored-data serialization. |
| `00A52560` ZString constructor | Indexed game callers `00753570` hardware report, `00753810` mission-start report, and `007556A0` mission-end report, besides library callers. |
| `0094CAE0`, `0094CAF0` template cleanup | Nine and three raw EH-funclet jumps respectively; these are cleanup reachability, not nine/three independent gameplay entrypoints. |
| `0094D280` tree operation | `0094E619 CALL` from another telemetry template; body manipulates native tree nodes, colors and traversal-stack slots. |
| `0094E660` iterator constructor | No indexed caller and zero direct/absolute references in this scan. This is one **unreached candidate**, not proof the whole template family is dead or that no computed/interior reference exists. |
| `00A586D0`, `00A586F0` fixed format wrappers | `00A59CAE` and `00A59CC3 CALL` from telemetry Content-Range formatting; wrappers call `_vsprintf_s` with `0x50` and `0x1E` capacities. |

An index-only direct-edge query over the two library ranges found 206 external
caller starts and 231 edges; 172 caller starts were named `Unwind@...`. This is
an indexed lower bound, not an exhaustive callsite total. It also misses the
unlisted `00750840` body above.

## Current rebuilt-host reachability

The telemetry library rows have no reconstructed source body in this census.
The **game-side** metrics wrappers are compiled into `bsp_core`
(`cmake/startup.cmake:641`), with an abstract `Cc15MetricsHost` boundary in
`include/bsp/cc15_seg73_41_61.hpp:119`. Source search found no concrete subclass
or application call to `metrics_system_construct_00757fe0`; its declaration and
definition are present, and the constructor calls the reconstructed init.
`src/game_hosts.cpp` has no metrics owner/wiring. Likewise the older
`TelemetryHost` and `UnitDamageHost::telemetry_enabled/telemetry_event` interfaces
are not proof of a native telemetry-library binding.

Therefore **current concrete application telemetry is unreached/unwired by
source evidence**, while the original game demonstrably reaches the family.
This conclusion does not depend on missing Ghidra references. It is not a claim
about future hosts, every binary/linker configuration, or dynamic execution.

XLive is different. `GameStartupHost::SoundServices` loads the selected XLive
module (`src/game_hosts.cpp:1420`) and constructs the device adapter. Renderer
binding passes that adapter and the actual shared online publication
(`src/game_hosts.cpp:2498`). However **online-manager construction remains
explicitly unimplemented** at `src/game_hosts.cpp:2529`; the shared online and
pump slots start null (`1401` and `1403`). Thus:

* Ordinal **5002 render** is a concrete unconditional call in the reached
  native frame path, `src/native_renderer_end_frame.cpp:186`.
* Ordinal **5030 pretranslate** is a concrete message-loop call,
  `src/game_hosts.cpp:1210` and `src/xlive_library.cpp:56`.
* Ordinals **5005/5006 device attach/detach** are wired but gated by nonnull
  actual online publication in
  `src/native_renderer_device_recreation_actual.cpp:251` and `304`.
  They are not currently unconditional application calls.
* Startup, notification, sign-in, storage and achievement bodies/adapters exist,
  but compiling them is not evidence the current application constructs their
  native online owner. `src/native_online_manager_lifetime.cpp:105`/`121` and
  `src/native_online_pump.cpp:55` show the future owner/pump contract.

## XLive binding contracts and substitutions

The original PE imports **106 XLive ordinals**. The four renderer/platform
thunks are genuine six-byte `JMP [IAT]` entries:

| Thunk / ordinal | IAT | Original byte-checked caller | Current source and class |
|---|---|---|---|
| `00C2F1C0` / 5005 | `00CE25D0` | `00B29972`, device recreation | `src/native_xlive_device_adapter.cpp:33`; **bind-safe ABI**, **layout-bound** actual D3D device and presentation-parameter pointers; currently online-gated. |
| `00C2F1C6` / 5006 | `00CE25D4` | `00B29859`, device recreation | adapter `:38`; **bind-safe ABI**, **behaviour-sensitive** overlay lifecycle; currently online-gated. |
| `00C2F1CC` / 5002 | `00CE25D8` | `00B2DAA2`, end frame | `src/native_renderer_end_frame.cpp:104`/`:186`; **bind-safe ABI**, **behaviour-sensitive** overlay rendering; reached. |
| `00C2F1D2` / 5030 | `00CE25DC` | `00BEC1D8` run loop and `00BECD0D` load pump | `src/xlive_library.cpp:55`; **behaviour-sensitive** message ownership; reached on messages. |

The device adapter forwards the actual pointer pair to `__stdcall` ordinal
5005 and no arguments to 5006, keeping the loaded library alive through its
owner. The stub's `_XLiveOnCreateDevice@8` and `_XLiveOnDestroyDevice@0` exports
match these stack contracts (`tools/xlive_stub/xlive_stub.def:25`). No concrete
argument-count or pointer-layout mismatch was found in this adapter. Ghidra's
default `undefined function(void)` thunk signatures are not ABI proof.

The offline stub has **31 exports**. A bounded literal-import scan of the XLive,
native-online, profile and end-frame adapter sources found 32 distinct ordinals;
the missing one is **5331 XUserReadProfileSettings**:

* Original thunk `00A4D4A0` is called at `008D4613` and `008D4644` by
  `008D45D0` (two-pass size/query profile reads).
* Current `src/native_profile_settings.cpp:21` resolves 5331 and throws if absent;
  `src/game_native_settings_process.cpp:20` does this lazily on a profile read.
* The application binds the library at `src/game_hosts.cpp:1436`, but native
  control import is gated on online phase 2 and signed-in byte `+119`
  (`src/native_profile_settings.cpp:62`). Therefore this is a **latent missing
  export**, not a demonstrated current default-host startup failure.

| Boundary | Classification and concrete risk | Recommendation / effort |
|---|---|---|
| Native iostdnet strings/maps/pools, including `00A52560`, template band and game `ZStringSlot` | **layout-bound**. 12-byte ZString, vptrs, allocator/refcount ownership, native nodes and by-value 24-byte pairs cross the game boundary. `std::string`/`std::map` are not binary substitutes. | Preserve the vendor boundary. A compatible exact library plus Win32 ABI adapter would be substantial work; no package/version was identified. Current unwired telemetry needs no replacement body. |
| iometrics collector/transmitter/startup/shutdown | **behaviour-sensitive**, also layout-bound. Threads, queue progress, allocator callbacks and XLive startup are observable. | Keep excluded from reconstructed-game credit. An offline/no-send policy, if requested later, belongs at the game metrics host and needs explicit initialization/queue/shutdown behavior; it is not a stock SDK equivalence claim. Medium work for a bounded offline game host, large work for real SDK parity. |
| `00A586D0` / `00A586F0` | **bind-safe** narrow CRT delegation with fixed capacities; not proof the containing HTTP code is safe to replace. | Preserve MSVC `_vsprintf_s` varargs, capacity and invalid-parameter behavior. Small wrapper effort if ever needed. |
| Stub setup/device/render/message returns | **behaviour-sensitive offline substitute**: startup/device/render return success while making no SDK/overlay attachment; pretranslate returns FALSE so the game owns messages. | Safe scope is an explicitly selected offline harness with no overlay or online feature claim. Retain strict ordinal/stack decoration. No code change is needed for this audit. |
| Stub notification listener/queue | **behaviour-sensitive**: listener is a static address, not a Win32 kernel handle, and queue is permanently empty (`xlive_stub.cpp:202`/`:212`). No notify9/10/11/14/profile/update/invite callbacks occur. | Only valid for current empty-queue consumers. Do not pass the identity to Wait/CloseHandle or expect sign-in/profile/invite transitions. Current native lifetime documents no listener close; no current close mismatch was found. |
| Stub sign-in/profile outputs | **behaviour-sensitive**: signed-out state, error returns, untouched output buffers. Native sign-in substitutes name byte 0 and XUID 0 after failures (`native_online_signin.cpp:130`); sign-in-info fields are read only on success (`native_online_signin_ui.cpp:286`). These inspected fallbacks do not establish every output contract. | Preserve immediate errors and caller preimages; never return success with uninitialized output structures. Add 5331 only with a reviewed failure/output-size contract if a future offline host reaches it. Small bounded export work, not online/profile parity. |
| Stub storage/achievement/UI overlapped operations | **behaviour-sensitive**: submission fails immediately; progress returns pending and result/extended-error remain incomplete (`xlive_stub.cpp:376`-`:422`). No completion callback/event is ever delivered. Native storage branches on submission status (`native_online_storage.cpp:105`); achievement pump only retires after successful result (`native_online_achievements.cpp:114`). | Do not advertise asynchronous, callback, storage, achievement or sign-in equivalence. Forced online use can leave retained/retried work. Keep unreachable offline, or define explicit bounded failure/retirement behavior at the game host. Medium state-machine review before enabling. |
| Installed game-directory XLive override | **behaviour-sensitive and unsafe for unrestricted rebuilt-host binding**: current SHA-256 is `71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2`, 3,833,856 bytes. This exactly matches `reports/cc12_xlive_fixed_patch_contract.json`, which establishes a fixed-RVA game patch and an IPC-initializer bypass. | Do not equate this override with stock GFWL. Existing `scripts/run.ps1:17`/`:25`/`:33` selects an extracted, hash-verified redist runtime via `tools/prepare_xlive_runtime.py`; preserve that explicit selection. This audit did not extract or execute it, and does not newly validate genuine SDK startup. |

## Evidence limits and handoff

Every live `bsp.py ghidra` call constructs the repository Client and verifies
project `bsp` (`C:/Users/sqz269/bsp.gpr`), program
`/battlestationspacific.exe`, x86 language and image base before querying.
`ghidra count` matched the 76,774-function snapshot. Relevant commands were
`lookup`, `show` (assembly only when required), `callers`, raw disassembly,
instruction context, PE import/RTTI reads and rel32/absolute-dword census.

No runtime equivalence, successful live callback, ABI differential, build,
gameplay or overlay proof was produced. Existing historical reports are cited
as historical findings; the installed override identity was rehashed now. The
private redist runtime was absent at the checked main-worktree default path.
Append-only name-ledger notes cover `00A52560`, `0094E660`, `00C2F1CC` and
`00C2F1D2`. No existing names/evidence were replaced and no final tags were
changed; the primary integrator owns Ghidra annotation, aggregate classification
and integration.
