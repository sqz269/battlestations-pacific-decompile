# Raw mission-progress ownership and profile cleanup

Addresses: `00920E10`, `007FD780`, `00907CA0`, `005826B0`, `007F8540`,
`007FD510`, `007FD5F0`, `0058D860`, `0058B520`, `007FA880`, `007F89F0`,
`007FDB20`, `007FEE20`, `004D05E0`.

## Result

Profile reset now directly destroys and constructs the actual 24h mission-progress
owner. The owner contains three native tree headers, constructed at +0, +C and
+18 and destroyed in reverse order. The raw profile has concrete default bindings
for its counter/transient collection cleanup and existing string-list cleanup.
The required score-record destructor, map operation and settings bodies remain
explicit dependencies.

The complete normal owner schedules are reconstructed: `920E10` is 171 bytes and
`7FD780` is 196 bytes. New C++ interfaces are not native binary entry points.
`920E10` previously had only an ownership fragment; `7FD780` already had an analyzed
host-function record. This packet adds one unique reconstructed function address.
The nine collection helpers below are scoped storage contracts, not generic STL
ports or additional game-function counts.

Sources: `include/bsp/native_mission_progress_owner.hpp`,
`src/native_mission_progress_owner.cpp`, `include/bsp/native_profile_collections.hpp`,
`src/native_profile_collections.cpp`, and the existing native player-profile and
render-resource-record modules. Evidence is in
`reports/native_mission_progress_owner_r107.json` and
`reports/native_mission_progress_flow_r107.json`.

## Native contracts

| Address | Scope established |
| --- | --- |
| 920E10 | ECX owner, EAX same, RET. Create score/counter/counter sentinels; preserve allocator words +0/+C/+18. |
| 7FD780 | ECX owner, RET. Clear full ranges +18,+C,+0; free each current sentinel; clear head/count. The caller separately frees the captured 24h owner. |
| 907CA0 / 5826B0 / 7F8540 | Allocate 2A0h/1Ch/2Ch nodes; links +0/+4/+8 zeroed through independent DWORD-address guards; color/nil at 29C/29D,18/19,28/29. Payload remains uninitialized. |
| 7FD510 | Right subtree first; capture left before required `593570(node+14)`; then reload key data/length, release string and free node; continue along saved left. |
| 58B520 | Right subtree first; capture key data before left; release using current length+1, free node, continue along captured left. |
| 7FA880 / 7F89F0 | Right subtree first; capture left before pair destruction; release pair's string at header+14/data+18 before header0/data4; free node, continue along saved left. |
| 7FD5F0 / 58D860 | Destructor-generated full current range only. After subtree cleanup, reload current head for each parent/left/right store, clear count, return output owner and current begin. |
| 7FDB20 / 7FEE20 | Actual mission-progress composition; preserve captured prior owner across callback mutation of profile+64; child diagnostic states retained. |
| 4D05E0 | Existing native list algorithm gains a `NativeStringStorage` overload. Node frees still use real source CRT allocation ownership. |

The full-range helpers do not implement checked partial-range erase. The original
functions' unreached partial paths call `4FBD20`, `581290`, `58B250`, and `7FD1C0`;
the fixture binds these to a terminal unexpected-path diagnostic.

The source operation records exact failure stages and rejects replay. Its explicit
diagnostic acknowledgment does not free the retained graph. Original FH3 cleanup
when sentinel construction or score-payload destruction throws remains open.

## Ghidra repair

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, verified
before analysis/mutation batches. All mutations used the owning lease/write lock.

The first `7FD780` tail repair decoded 122 bytes after the first free but left the
stored function ending at `7FD7C9`. A subsequent locked recreation of that existing
function restored `7FD780..7FD843`, with `disassemble_first=false`, preserving its
reviewed name and prior plate comment. The final RET is at `7FD843`.

Free-site `CALL_RETURN` overrides at `7FD589` and `7FA8A7` hid the saved-left loop
continuations. Clearing them and decoding the 14/11-byte tails restored both loops;
their final exports have no remaining call gaps. Callee no-return flags were not
changed. A forced snapshot refreshed the body/call index although function count
did not change.

A read-only inline-script properties query was rejected because script execution
is disabled. The error is retained in the flow report; scripts stayed disabled.
Successful dedicated repair/read tools supply the evidence.

Unlisted alignment bytes are reviewed separately: seven-byte `LEA ESP,[ESP]` at
`7FDE59`, and a one-byte NOP after RET14 at each of `7FD66F` and `58D8DF`. They are
unreachable padding, not omitted executable logic.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass. No tests were added
  to the repository suite.
- Live Ghidra and the installed immutable PE match for 4,681 bytes: 4,621 function
  bytes in 16 bodies, 36 constant bytes and 24 literal bytes. All 126 direct CALL
  rows are mechanically checked against their containing functions and targets.
- Eight original/source cases match 437 ordered observations and 20,867,992 bytes.
  Captures include the 71A0h game, five publication slots, 4000h allocation arena,
  C0h settings, 800h prior-owner/key area, arguments and the Dyn descriptor.
- Cases cover zero/A5 construction, name aliasing, null parent/owner allocations,
  prior-owner destruction with callback mutation, publication replacement,
  repeated profile reset, rank-header mutation, returning vector validators,
  populated score/two-counter cleanup and populated transient cleanup. The score
  callback changes the left link and key header: saved-left traversal and current
  key release still agree.
- Source-only failure checks retain game/profile/mission construction stages
  through the second mission sentinel allocation, populated score destruction at
  `7FD81B`/state -1, and late Dyn failure. Each rejects replay.
- Source-only actual-heap checks verify three sentinel allocations/frees, opaque
  words and payload preservation, plus two nonempty alias-list nodes/strings using
  the new generic storage overload.

The fixture executes copied original bodies with rebased direct CALLs and
controlled remaining providers. It does not compare native exception unwinding,
private stack/register identity, arbitrary aliases, malformed graphs, returning
null node allocators, concurrent mutation or independent payload implementations.
String release uses the existing noexcept source boundary.

The application executable matches R106 except timestamp fields. Two application
object hashes changed (`game_native_renderer_application.obj` and
`game_native_vfs_application.obj`); no all-object identity claim is made. The raw
game/profile owner remains outside the application's constructed graph. Runtime
was not rerun for this packet; the prior D3D9 device failure and gameplay acceptance
remain open. Tested and integrated artifact receipts are recorded in the report.

## Follow-up packet

Recover the complete `00593570` mission-score record destructor and its owned
containers. Current stored body ends prematurely at `59363F` after a container
sentinel free. The visible prefix releases strings at +264, +25C and +254 and begins
container cleanup at +240; this is evidence for further analysis, not a complete
payload port. Preserve the required source dependency until its real body is ready.
