# Native mission self-object refresh: qualified Source candidate

This unregistered, unbuilt candidate implements only `00928B73..00928B9A`,
the **40-byte / 11-operation** refresh inside `00928A00`. It uses the existing
real getter, tracked-object assignment and destructor. It does not implement
the 634-byte parent, repair its abstract host interface, or introduce a
production consumer. Source/build/fixture/Original-ABI/startup/gameplay credit
remains zero pending the primary agent's review and admission.

Public function:
`refresh_native_mission_entity_lua_self_00928b73_fragment` in
`include/bsp/native_mission_entity_lua_self_refresh.hpp` and
`src/native_mission_entity_lua_self_refresh.cpp`.

## Exact Native basis

The primary agent accepted the complete parent gate: 634 bytes, 187 operations,
32 calls, Original PE/live bytes and every saved/live instruction start. Its
accepted readiness report and saved annotation receipt are pinned in the
companion report. This candidate reuses that gate, with no new live Native
query, body expansion or Ghidra/GPR mutation.

The fragment's bytes have SHA-256
`e11e16a45648bc60d225be1933839bd360253224720e0ebf6281d8d764a382eb`.
It executes after fresh-table creation and the preceding table/globals cleanup,
only on the parent path that does not take descriptor kind 3.

| Native site | Candidate operation |
| --- | --- |
| 00928B73..00928B7A | Construct a distinct temporary through actual 00927B40 using the same receiver key and current world publication |
| 00928B7F..00928B89 | Use the getter's returned actual pointer as assignment source; assign into the existing persistent self through 00B67690 |
| 00928B8E..00928B96 | Destroy the temporary through 00B67700 before any later ID write |

Native persistent self is at stabilized S+20; the distinct temporary is at
S+48. All offsets are hexadecimal. State 6 follows completed getter output
and precedes assignment; state 1 precedes normal temporary destruction. The
new function is an ordinary C++ interface and reproduces no original stack or
register ABI. Descriptive names remain hypotheses.

## Actual storage and provider contract

The caller supplies the same actual R+178 `NativeString` header, the actual
current E188A8 publication cell and an already constructed persistent
`NativeLuaObjectStorage` at a stable address. The function creates one distinct,
uninitialized 14h temporary; the existing getter constructs its live object.
There is no semantic `LuaObject` copy, new service callback, numeric pointer
translation, vtable cast, copied key or replacement owner.

The exact existing providers are:

- `construct_native_mission_entity_lua_self_00927b40` in
  `src/native_mission_entity_lua_self.cpp:31`: captures this call's current
  E188A8 world, uses its constructed owner at +1A0C, performs actual
  globals/thisTable/current-key lookup and returns its actual output pointer.
- `assign_native_lua_object_00b67690` in `src/native_lua_objects.cpp:150`:
  releases the bound destination before reading current source fields, then
  copies owner/kind/index/tracked and registers the destination's real address.
- `destroy_native_lua_object_00b67700` in `src/native_lua_objects.cpp:146`:
  uses the actual tracking domain and current object fields.

No owner, index, count, world value or source-field snapshot is cached by the
candidate. Getter cleanup can move its output index; destination release can
move that index again before assignment reads it; temporary destruction can
affect the persistent object's current index. All these reads and mutations
remain in the real providers. Later ID/Dead/Ptr work must use current persistent
self fields.

The old persistent owner and new getter owner are not assumed identical. Both
actual owners/states, Lua stack objects, key storage and tracking domains must
remain valid for their uses through callbacks and cleanup. Existing capacity
requirements are 50 tracked slots and five references per slot. Stable object
addresses and distinct output storage are required; arbitrary aliases, invalid
storage or a semantic record presented as a Native receiver are not admitted.

## Explicit C++ cleanup behavior

The getter runs before the assignment try block. If it throws, cleanup of
any completed output remains the existing getter provider's responsibility.
The candidate does not destroy an output whose construction did not return.

After getter success, an assignment exception causes one destruction attempt
on the completed temporary. If destruction returns, the original C++ exception
is rethrown. **If destruction itself throws, that cleanup exception replaces
the original assignment exception.** No consumed flag is created and cleanup
is not retried. Partial assignment/release effects remain; there is no rollback
or replay.

Normal temporary destruction sits outside the assignment catch. A normal-path
destruction exception therefore propagates after that one attempt without a
second cleanup call. These are explicit new C++ exception semantics. Native
saved-stack/FH3 cleanup, Lua longjmp/nonlocal error paths, hardware faults and
original handler equivalence remain unproved. No exception test was run.

## Minimal focused validation design

No new test or probe was created. The existing historical getter/setter fixtures
are described in the accepted readiness report, but their reported local files
were absent at the checked locations. Their historical outcomes are not a test
of this new function.

For the primary agent's eventual validation, one real-Lua case is sufficient
for the concrete risk: use real Lua 5.1.1 and canonical actual owner/tracked
object providers, retain an old table in the persistent self, and replace the
actual `thisTable[key]` slot with a new table. Arrange the persistent object
below the fresh getter result on the same tracked stack so destination release
must move the temporary's index. Invoke this fragment, then verify that the
persistent object identifies the new table, its actual pointer is registered
once, the temporary registration is gone, tracking/count/index state is valid,
and opaque destination bytes remain unchanged. A subsequent existing real
setter should reach the new table while the retained old table remains unchanged.
This validates refresh identity and tracking; it does not test the later Native
ID setter or establish production receiver ownership. No mock rebind method is
needed. Exception behavior remains a separately qualified domain.

## Current context and admission boundary

The worker synced to the actual current main tip recorded in the report,
which includes accepted ancestor `0514c5d525f323c07c06c00833070b4343abf47a`.
The Root Source121 receipt remains the existing build authority: 121 Root raw
inputs, 121 worker LF-normalized inputs and four actual Root artifacts replay
successfully. The single raw worker difference is LF/CRLF in the pending-registry
constructor readiness report. Existing context is 37 captured/replayed objects,
41 positive Core definitions and three passing checks, with 35 prior objects
unchanged. Those checks did not compile or exercise this unregistered candidate.

The separate Root startup smoke reached three ticks and two successful Presents
at the press-start page, with no mission frames. It used the preceding Source121
executable and does not execute this candidate or establish unit/model ownership.

The primary agent owns CMake registration, compilation, emitted code and C++
cleanup review, and Source admission. This worker changed exactly the header,
C++ file, document and report. No build, test, probe, ledger, GPR, CMake or
application-consumer change occurred. Production numbering remains held on
genuine receiver construction/lifetime/profile, current-target dispatch and
actual model-numbering services.

Evidence: `reports/cc12_native_mission_entity_lua_self_refresh_source.json`.
