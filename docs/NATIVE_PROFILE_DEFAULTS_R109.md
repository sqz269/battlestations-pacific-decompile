# Raw player-profile counter and settings defaults

Addresses: 005070c0, 005062e0, 00505ba0, 00505340, 00504bd0, 00502170, 004fa600, 004fbbe0, 004fbc90, 004fbd20, 004b93c0, 008d4820, 008d41c0, 008d45d0, 00a3e500, 00a3e510, 00a3eac0, 00a4d4a0, 007fdb20, 007fee20

## Result and scope

The final three required `NativePlayerProfileCalls` methods now have concrete raw defaults: counter indexing `005070C0`, control reset `008D4820`, and game-default reset `008D41C0`. Control reset includes the complete normal selected-user branch `008D45D0`. `NativePlayerProfileContext` supplies current manager/game publications and a typed SDK context. No synthetic selected account or successful SDK response is installed.

These complete storage-level providers for previously analyzed/projected routines. They add no unique game-function count. Library map helpers remain scoped contracts using existing source tree templates; this is not a generic STL port. The raw game constructor still needs its other subsystem providers and application ownership.

## Counter-map storage

`005070C0` takes ECX tree and one stack key-header pointer, returns a mapped DWORD pointer in EAX, and uses RET4. Header fields are allocator/head/count at 0/4/8. Nodes are 1Ch bytes with string length/data at C/10, mapped value at14, color18 and nil19. Equivalent case-insensitive keys retain their existing value; a missing key is deep-copied into a node with mapped zero. Profile reset subsequently writes RANK=1.

The implementation preserves lower-bound lookup, temporary copied key lifetime, hinted insertion, fallback unique insertion, rotations and root coloring. It reuses `detail/native_tree_insert_storage.hpp` and `less_native_string_headers_00443d00`. Unique insertion publishes node, inserted byte and owner in native order. The count guard remains unsigned15555554; source exceptions use the existing owning length-error transport and do not claim original FH3/RTTI compatibility.

The first differential run caught a source allocator-entry mismatch: original504BF3 callsBF681B directly, while the SDK response callsBF55BE. BF55BE itself jumps toBF681B. Both concrete source defaults now reuse the existing malloc/new-handler allocator and matching free; the fixture retains distinct native entry observations. No original heap ABI claim follows from this source reuse.

## Raw settings and SDK import

`008D41C0` performs only eight sparse stores: byte8=1,9=0,DWORDC=2,byte10=1,4A=1,B2=1,7C=1,DWORD80=0. `008D4820` writes control bytes42=0,43=0,41=1,40=0,44=1. It checks current manager+28==2, reloads the manager publication for byte119, and tail-dispatches selected-user restoration when nonzero.

`008D45D0` supplies four identifiers10040015/10040024/10040002/10040003 and a zero byte-count cell. It queries size with user0 and ignores that status, allocates the returned count, reloads current manager+11C, then queries again with the same mutable identifiers/size cells. A nonzero second status returns without freeing the response, preserving the native failure leak.

On success, response+4 supplies the preference array, reloaded for each preference. Difficulty at+20 maps2 to2,1 to0,other to1 and writes current game+6AC. Other fields set settings41 from+48==0,43 then42 from+70!=0, and40 from+98==0. The response is freed afterward.

The installed PE confirms A4D4A0 is `JMP [CE2624]`, imported from xlive.dll ordinal5331. `NativeProfileSettingsSdkRuntime` resolves that ordinal from the supplied loaded `XLiveLibrary`, or accepts the exact seven-DWORD stdcall function pointer. It forwards raw status and mutable buffers. It neither initializes the platform nor supplies an account. The fixture uses this actual adapter with a controlled stdcall import function; real SDK account behavior and resolver execution remain untested.

## Evidence and verification

`reports/native_profile_defaults_r109.json` retains 14,060 disk/live-matching bytes: 58 copied function bodies (12,064 bytes), read-only score-layout producer91CE90 (1,930), SDK thunk6, and constants/literals60. All386 direct call rows are checked against current Ghidra bodies, including350 copied-body calls and36 producer calls. The fixture additionally relocates four external JMPs, including8D485C to8D45D0.

Ghidra incorrectly suppressed ADD ESP,4 at8D46A3 after the free call. The locked repair restored those3 bytes; complete220-byte function recreation also restored membership that a min/max range alone concealed (old body_size217). No callee no-return flag changed. Prior names/comments were retained, with evidence-backed names added to six FUN entries; readback, saved analysis and refreshed exports accompany the report.

Strict MSVC Win32 build and all three existing CTests pass. Fifteen original/source cases compare1,487 ordered observations and101,473,516 bytes. Existing construction, repeated reset and populated score cleanup cases remain. Added cases cover both manager gates, all three difficulty mappings, nonzero selected byte80, publication replacement and mutation of borrowed SDK argument cells, second-query failure, and20 mixed-case/empty-key map indices yielding17 unique nodes. Comparisons include values, links/colors, key storage and ordered callback snapshots.

Existing actual CRT cleanup checks remain. A focused source map-allocation failure check confirms profile stage7FDDE3/unwind2, no node publication and replay rejection. Original native unwind is not executed. Original/source fixtures still control remaining game services, allocations, string-pool calls and SDK behavior; they are not application or gameplay validation. Application artifact comparison and immutable tested/integrated archives are recorded in the report. These raw routines remain unreached by the application; no runtime rerun is claimed.

## Follow-up contracts

- Bind the actual raw game/profile owner to application lifetime and complete its remaining constructor services.
- Connect the selected-user SDK context to the actual loaded XLive owner and validate platform/account behavior in the executable.
- Recover score-record construction/copy/reset separately from the cleanup already composed here.
- Retain explicit limits for native EH/ABI, arbitrary aliases/fault timing, malformed graphs, generic iterator/hint behavior and full gameplay.
