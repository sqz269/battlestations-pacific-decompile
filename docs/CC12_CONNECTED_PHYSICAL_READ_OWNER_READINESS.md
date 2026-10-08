# Connected physical read through the genuine Source owner

**Ready for a separate Source implementation packet.** The current ordinary `read_native_physical_stream_00bf5030` already has a closed Source producer and lifetime path through `GameNativeVfsApplication`. No missing Source body, stream class or handle producer blocks a bounded diagnostic in the existing `bsp_game` executable. This readiness packet changes exactly this document and its JSON report; it changes no Source/header/build recipe/metadata and performs no build, test, probe, diagnostic execution or Ghidra batch. Baseline: `fb8053305f33176da804ba6a4da29ed97acfb106`. Additional Original function credit: **zero**.

## Actual owner, producer and release path

The application owns the real A0 allocation and publication. `NativeVfsOwnerServices` binds `streams().physical.manager_0109ceec` to that same cell, and binds the stream pool and batch lock to the same singleton lifetime. Use the existing public `publish_and_borrow_raw_failure_manager()` API after `initialize_core()` to qualify and publish the Source C3 target. The ordinary Source read accepts its explicit owning context and calls `+90` directly with a C++ fastcall type only if `ReadFile` fails. It does **not** call the raw notifier. No raw-notifier calling-convention assumption is required or granted here.

| Stage | Existing complete Source path |
| --- | --- |
| Directory and mount | `runtime.mount` -> `BE1890` -> factory selector `BDB040` -> finite binding -> `BF4DF0`; actual provider slot from `BF34D0`, directory constructor `BF4D30`/base `BB5590`, actual mount registration `BE1740` |
| Open | `bindings.open` -> `BDF310` -> `BDD0A0`/`BDA690` -> finite provider binding -> `BF4BA0` |
| Stream and HANDLE | `BF42A0` registers the real10h stream pool; `BF3770` acquires actual20h backing under the shared lock; `BF50D0` initializes it; `BF3970` builds the real path; `BF5590`/`BF52A0` call `CreateFileA` and cache `GetFileSizeEx` |
| Read | `bindings.read` reads the current `D691B0+24` identity, selects named Source `BF5030`, makes one real `ReadFile`, updates cached position and actual count |
| Release | Decrement actual `stream+4` from1 to0, then `bindings.zero_reference` -> finite `BF55A0` -> `BF5090` calls `CloseHandle`, stores `FFFFFFFF`, leaves `CEB130` -> `BF5190` appends dead backing to the same pool |
| Shared drain | `GameSingletonHost::shutdown` deletes that stream pool with `BF4370`/`BF3930`; the real VFS manager walks its mount and finite-dispatches `BF4DD0`/`BF4C70`/`BB5380`, returns the actual directory slot to the process pool, retires the runtime, then normal CRT exit cleans process owners |

These numeric values are checked data identities used by existing finite C++ dispatch. No Original code or transplanted numeric slot is invoked. The exact actual directory producer requires the system name to end in **backslash**. Its nonpersistent directory owns the root header and a real empty index sentinel. No scratch provider, raw-constructor class inference, manufactured stream or borrowed fake handle is needed.

## One concrete diagnostic

Add an optional `--qualify-vfs-physical-read-owner` path at the existing `WinMain` branch after canonical data handoff, keeping the normal parent and canonical child. Proposed implementation touches only `src/game_main.cpp`, `src/game_hosts.cpp` and `include/bsp/game_hosts.hpp`, plus its own new evidence document/report. A separate flag preserves the existing failure-owner case. No new target, test framework, provider implementation or public runtime API is needed.

Use this existing, stable installed regular file as read-only data:

| Item | Exact value |
| --- | --- |
| System root | `I:\SteamLibrary\steamapps\common\Battlestations Pacific\` |
| Virtual prefix | `cc12_physical_read` |
| Logical name | `cc12_physical_read/battlestationspacific.exe` |
| Mount arguments | priority0, flags1, device `FFFFFFFF`; exactly one mount |
| Open flags | `2`: `GENERIC_READ`, `FILE_SHARE_READ`, `OPEN_EXISTING`; no create/truncate/write route |
| Whole file | **12,223,752 bytes**, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |
| One read | requested **64**, expected actual **64**, expected cached position **64** |
| Prefix identity | SHA-256 `c4bfe3c9fa083643c55a84e22d1e46201fc3aba7cf52ccfb25d517cf1b82da05` |

Exact expected64 bytes, retained from the read-only whole-file copy:

```text
4d5a90000300000004000000ffff0000b80000000000000040000000000000000000000000000000000000000000000000000000000000000000000020010000
```

Construct the actual pooled name with `NativeString::assign_0041e870(services.strings, logical_name)`. Open with `services.bindings.open(actual_manager_table, actual_manager, name, 2)`. Keep that returned production stream and all application services alive; invoke `services.bindings.read(actual_stream_table, stream, bytes, 64, &actual)`. The real `runtime.read` wrapper already expresses this sequence, but a diagnostic using its existing borrowed bindings exposes the actual stream/HANDLE and the complete release boundary for observation without adding an API.

| Observation point | First fields and required result |
| --- | --- |
| Genuine startup | Same manager across application/runtime/publication; `D68D04`; qualified Source `+90`; actual singleton records; current stream-pool/lock cells |
| Mounted | Non-null actual `D69168` directory, refs1, exact owned root header and bytes, nonpersistent byte+28=0, real mount record |
| Opened | Whole20h stream: `D691B0`, refs+4=1, real HANDLE+8, position+10/+14=0, cached size+18/+1C=12223752/0; +C is observed but not interpreted. Log actual HANDLE disk type, file metadata and final path using queries on that handle. Actual pool10h and shared lock are registered; in this fresh case pool count/capacity are0 while backing is borrowed out |
| Read returned | Exact64 bytes and actual64; position64/high0; same live handle/ref1/size/profile/+C. Current genuine manager/callback/publication remain valid |
| Release returned | Ref decrement result0; complete finite zero-reference call returned; actual pool count0->1, capacity0->1, current last cell equals saved stream address. End the live stream borrow. Any optional dead20h observation is reacquired from the current pool slot and never used as a live stream; expected ref0/HANDLE `FFFFFFFF`/profile `CEB130` |
| Shared drain returned | Read only still-live publication cells: VFS, singleton, stream pool and batch lock become null. Existing public request hits the retired guard before owner access. Destroy drained application then host and return normally |

Open and mount legitimately change manager counters, `+18`, mount storage and singleton registrations; do not reuse the earlier failure-only diagnostic's whole-A0/no-registration-change expectation. `BF5030` updates cached position with actual count and leaves size unchanged. The existing destructor discards `CloseHandle`'s Boolean result: completed Source release and its cleared field do not independently prove that OS return value. Do not query an intentionally invalid handle to create an error case.

Finish all stream observations before complete zero-reference release; then release the pooled name before shared drain. Keep application/runtime/host/canonical data and Source code alive through that drain and CRT exit. Declare the owning pointers outside the diagnostic's try block and reuse the established logged `_Exit(3)` policy for unexpected partial-state failure. Do not retry, run another read, improvise cleanup or inspect old object/registration addresses after drain. Expected bytes and full installed-file hash must be rechecked before and after the future run.

## Retained static evidence and limits

`audit02/manifest.json` freezes **4,446 physical rows**, with complete Source inputs, existing compiler/link records, whole Original and Source PEs, actual normal map and whole core archive. All69 selected Source/header files match the successful prior-build capture. The supplementary cleanup capture adds eight whole objects and eleven copied rows. Together: **43 complete objects**, **39 unique complete core archive members** and four direct game objects, containing4,982 function/compiler-storage extents. Whole bytes/relocations are retained for all extents. A broad singleton dispatcher includes compiler switch-table data: linear decoding covers3108 of its3109 bytes, so no complete instruction interpretation is claimed for that extent. The other4,981 extents decode completely. The initial audit01 attempt stopped at an overly strict all-linear-decoded assertion; audit02 is the sealed successful capture.

The existing normal map/PE replays **45 whole selected bodies and all282 ordered relocation operands**, including actual open480B, ordinary read103B, recycle125B, stream delete62B and pool delete167B. Compiler-local labels are restricted to their actual object, or anchored by exact bounded same-section COFF symbol offsets. Six additional complete COFF bodies lack standalone symbols in that map; the report lists them and makes no separate linked-entry or blanket inlining claim. No new code was linked or executed. Original retained spans/import rows are documentary references, never callable inputs.

The captured manifest hash is `1ed8241318eba9dcde907556769a416a77b48ffc73e9ae4e1fa6551f433ef686`. Complete linked proof: `11cb6fe5c658d3c533ecf591d8a4c9a71f2759232d71b6110eeb0b24d2dfc836`. Cleanup closure: `3c85515abfcdfb73c9c62eac78e98af3c1a38952cabe72af9a5d65efcc25c9dd`. Everything is retained under `local/cc12_connected_physical_read_owner_readiness_20261008a`; the [JSON report](../reports/cc12_connected_physical_read_owner_readiness.json) records exact artifact paths, hashes, owner contracts, fields and every linked-body identity.

The next authorized Source packet should freeze its actual inputs, implement the three CLI files, run the normal Win32 build and existing three checks, qualify changed CLI/protected provider bodies against its fresh map/PE, then perform **one** parent/canonical-child diagnostic only after its static gate. A successful case qualifies ordinary Source production/read/release/drain on this file. It does not exercise failure, establish the raw `BF5030` register/stack/flag ABI, prove Original EH equivalence, or validate gameplay. The raw-read ABI remains a separate packet if subsequently requested.

## Primary review

Independent primary review sealed the retained inputs, all43 current objects and39 unique current archive members, and replayed all45 whole linked bodies /282 operands against both retained and current normal images. The prior69-provider capture has two deliberate current differences: the accepted canonical property-pool Source/header additions; the other67 remain identical. The current56-function process provider and its six added helpers are separately qualified. One previous collector attempt treated a DATA span as an instruction span; the corrected audit preserves that failed attempt. No new build or execution occurred.

Receipt: `local/cc12_read_and_type6_readiness_primary_review/receipt.json`, SHA256 `ca1d7af3fb75be1418c524c2aca250fbf9e6709dbde31ff1f4710461378557e3`.
