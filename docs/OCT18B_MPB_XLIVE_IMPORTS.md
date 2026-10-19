# oct18b-mpB: segment 72 XLive import forwarders

## Result and scope

All 46 assigned rows are six-byte XLive import thunks. Game bodies reconstructed: **0**;
library forwarders classified: **46**; assigned rows left open: **0**.
Every row is recorded as `library_instantiation_out_of_scope`. No replacement C++
implementation, host wiring, or library-body reconstruction is warranted.
The exact worklist was `cc16-progress/local/codex_mpB.txt`; only those rows were
leased. The unassigned entries at `00a4d566`, `00a4d56c`, and `00a4d572` were not changed.

## Evidence and original ABI

Live read-only typed metadata was captured on 2026-10-19 UTC in
`local/oct18b-mpB-typed-flow.json`. Both ends of the batch verified project `bsp`,
marker `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`. All 46 records have an exact function
entry, one complete six-byte body range, `is_thunk=true`, an external thunk target,
`COMPUTED_JUMP` flow, no fallthrough, and no flow override. The capture was accepted
at program modification 899. No function creation or listing repair was needed.

The installed PE was read without modification:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Each assigned body's disk encoding is `FF 25 <little-endian absolute IAT slot>`;
the decoded IAT slot belongs to an ordinal import from `xlive.dll`, as listed below.
A 294-byte live capture beginning at `00a4d4e8` agrees with this encoding.

Original ABI at the stub is a transparent tail transfer: `JMP dword ptr [IAT]`.
It pushes no return address, changes no argument register, stack pointer, or flags,
and uses the caller's existing return address. The imported callee determines
parameter widths, register/stack arguments, cleanup, and return value. A six-byte
stub does not establish a complete SDK prototype. Ghidra's `undefined name(void)`
is not evidence that these APIs take no arguments. Existing caller interfaces
retain their own evidenced contracts; no new ABI compatibility claim is made.

Uncertainty: the PE proves DLL plus ordinal, rather than textual export names.
The existing Ghidra XLive names are retained as library labels; their ordinal-to-name
mapping was not independently recovered from an XLive DLL. Nothing in these bodies
implements session state, messages, host IDs, or 13-bit handle arithmetic.

Before ledger writes, every assigned address was searched case-insensitively in
`include/` and `src/`. 30 rows already have address citations there.
The table gives the first citation per row; `none` means no literal address hit,
not proof that an unnumbered abstraction is absent. Full grep results, bytes, and
import slots are retained in `local/oct18b-mpB-evidence.json`.

## Assigned rows

All rows below have the common encoding and classification above.

| Address | Preserved Ghidra/library label | IAT slot | xlive.dll ordinal | Existing address citation |
|---|---|---|---:|---|
| `00a4d4e8` | `XSocketWSAGetLastError` | `00ce25f4` | 27 | `src/engine_seg11_35_66_cc13.cpp:4101` |
| `00a4d4ee` | `XSocketCreate` | `00ce25f0` | 3 | `include/bsp/online_seg71_cc13.hpp:708` |
| `00a4d4f4` | `XNetRegisterKey` | `00ce25ec` | 55 | `include/bsp/online_seg71_cc13.hpp:377` |
| `00a4d4fa` | `XNetCreateKey` | `00ce25e8` | 54 | `include/bsp/online_seg71_cc13.hpp:471` |
| `00a4d500` | `XSocketClose` | `00ce25e4` | 4 | `include/bsp/online_seg71_cc13.hpp:475` |
| `00a4d506` | `XSessionArbitrationRegister` | `00ce25e0` | 5333 | `include/bsp/online_seg71_cc13.hpp:521` |
| `00a4d50c` | `XSessionStart` | `00ce258c` | 5318 | `include/bsp/online_seg71_cc13.hpp:522` |
| `00a4d512` | `XSessionEnd` | `00ce2588` | 5332 | `include/bsp/online_seg71_cc13.hpp:523` |
| `00a4d518` | `XSessionJoinRemote` | `00ce26b8` | 5326 | `include/bsp/online_seg71_cc13.hpp:525` |
| `00a4d51e` | `XSessionDelete` | `00ce26b0` | 5330 | `include/bsp/online_seg71_cc13.hpp:526` |
| `00a4d524` | `XSessionCreate` | `00ce26ac` | 5300 | `include/bsp/online_cc13.hpp:693` |
| `00a4d52a` | `XSessionJoinLocal` | `00ce26a8` | 5327 | `include/bsp/online_seg71_cc13.hpp:720` |
| `00a4d530` | `XSessionLeaveLocal` | `00ce2614` | 5325 | `include/bsp/online_seg71_cc13.hpp:722` |
| `00a4d536` | `XSessionLeaveRemote` | `00ce2590` | 5336 | `include/bsp/online_seg71_cc13.hpp:724` |
| `00a4d53c` | `XNetRandom` | `00ce2594` | 53 | `include/bsp/online_seg71_cc13.hpp:715` |
| `00a4d542` | `XSocketHTONS` | `00ce2598` | 40 | `src/engine_seg11_35_66_cc13.cpp:4102` |
| `00a4d548` | `XSocketRecv` | `00ce259c` | 18 | none |
| `00a4d54e` | `XSocketRecvFrom` | `00ce269c` | 20 | `src/engine_seg11_35_66_cc13.cpp:4145` |
| `00a4d554` | `XSocketSend` | `00ce26a0` | 22 | none |
| `00a4d55a` | `XSocketSendTo` | `00ce26a4` | 24 | `src/engine_seg11_35_66_cc13.cpp:4070` |
| `00a4d560` | `XUserGetSigninInfo` | `00ce26bc` | 5267 | `src/native_online_signin_ui.cpp:135` |
| `00a4d578` | `XStorageBuildServerPath` | `00ce26cc` | 5344 | none |
| `00a4d57e` | `XStorageDownloadToMemoryGetProgress` | `00ce26d0` | 5307 | none |
| `00a4d584` | `XStorageDownloadToMemory` | `00ce26d4` | 5345 | none |
| `00a4d58a` | `XStorageUploadFromMemoryGetProgress` | `00ce26d8` | 5304 | none |
| `00a4d590` | `XStorageUploadFromMemory` | `00ce26dc` | 5305 | none |
| `00a4d596` | `XUserWriteAchievements` | `00ce26e0` | 5278 | none |
| `00a4d59c` | `XLiveGetUpdateInformation` | `00ce26e4` | 5022 | `src/native_online_notification_leaves.cpp:74` |
| `00a4d5a2` | `XLiveUpdateSystem` | `00ce26e8` | 5024 | none |
| `00a4d5a8` | `XInviteGetAcceptedInfo` | `00ce26ec` | 5315 | `include/bsp/session_polls.hpp:65` |
| `00a4d5ae` | `XNotifyGetNext` | `00ce26f0` | 651 | `include/bsp/online_cc13.hpp:218` |
| `00a4d5b4` | `XNotifyCreateListener` | `00ce26f4` | 5270 | none |
| `00a4d5ba` | `XShowSigninUI` | `00ce26f8` | 5260 | `include/bsp/native_online_signin_ui.hpp:49` |
| `00a4d5c0` | `XShowMessageBoxUI` | `00ce26fc` | 5266 | `include/bsp/native_online_signin_ui.hpp:43` |
| `00a4d5c6` | `XNetSetSystemLinkPort` | `00ce2700` | 84 | none |
| `00a4d5cc` | `XWSACleanup` | `00ce2704` | 2 | none |
| `00a4d5d2` | `XWSAStartup` | `00ce2708` | 1 | none |
| `00a4d5d8` | `XOnlineStartup` | `00ce270c` | 5310 | none |
| `00a4d5de` | `XLiveInitializeEx` | `00ce2710` | 5297 | none |
| `00a4d5e4` | `XUserMuteListQuery` | `00ce2714` | 5314 | `src/cc13_sweepC.cpp:1230` |
| `00a4d5ea` | `XOnlineCleanup` | `00ce2718` | 5311 | `include/bsp/online_cc13.hpp:82` |
| `00a4d5f0` | `XHVCreateEngine` | `00ce271c` | 5008 | `include/bsp/online_cc13.hpp:640` |
| `00a4d5f6` | `XNetQosListen` | `00ce2720` | 69 | `include/bsp/online_cc13.hpp:567` |
| `00a4d5fc` | `XNetQosLookup` | `00ce2724` | 70 | `include/bsp/online_cc13.hpp:831` |
| `00a4d602` | `XUserSetProperty` | `00ce2728` | 5276 | none |
| `00a4d608` | `XSessionSearchEx` | `00ce272c` | 5319 | `include/bsp/online_cc13.hpp:772` |

## Validation and integration

- Static classification: 46/46 exact assigned entries match six-byte external
  XLive thunks in live typed metadata and installed PE bytes/import tables.
- All existing Ghidra labels and comments are preserved. Classification evidence
  is appended through the reviewed naming ledger and the locked annotation tool;
  prior comments are retained in the tool's local annotation journal.
- The annotation tool skips renaming when the current name already matches the
  ledger. This permits comment updates for established library names such as
  `XSocketRecv` without triggering the bridge's token-subset collision guard
  against `XSocketRecvFrom`. Actual renames still use the guarded endpoint.
- Locked annotation batch: 46/46 applied, original library names retained, project
  saved; prior state recorded in `local/ghidra-annotations-20261019T040612Z.json`.
- All 46 affected Ghidra exports were refreshed with `bsp.py ghidra export --force`.
- `./scripts/build.ps1` passed for Release Win32; `reconstructed_math` and
  `tool_tests` both passed (2/2 CTest targets). Python syntax compilation also
  passed for `tools/ghidra_annotate.py`.
- No game runs or runtime/ABI compatibility validation were requested or performed.
