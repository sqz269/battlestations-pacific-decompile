Primary compiled review: Registered pool helper compiled to51 bytes/24 instructions. It captures the buffer first, reads length only for a nonnull buffer and retains that length across the actual raw-manager getter. The compiler forms the pure uint32 size+1 after the getter from retained ESI; Native ADD/flags timing is not claimed. Both actual provider relocations and all instructions were reviewed.

Normal MSVC Win32 build 2026-10-09T17:59:46.563985+00:00 to 2026-10-09T18:00:05.036630+00:00 passed three existing checks. The shared receipt pins97 Source/build inputs and four artifacts, replays all19 prior reviewed objects unchanged and captures two unchanged existing pool-provider objects plus the two new objects (23 total). Core contains27 selected positive roots, including the two existing getter overloads; this helper uses the raw-manager overload only. New public roots are absent from the application map. No Native ABI, selected production binding, startup or gameplay credit. The candidate section below is an immutable worker-time snapshot; its Source88 build artifacts are historical.

# CC12 Lua variant string-field cleanup Source candidate

`return_native_lua_variant_string_fields_006ee020` is a qualified C++
candidate for the complete `006EE020..006EE03D` body. It accepts the actual
length and buffer cells plus the actual pool publication, manager publication
and return gate. It reads the buffer first. Null skips the length and both
services; nonnull captures length once, computes unsigned 32-bit length+1,
gets the actual pool and returns the captured block and size. No field is
cleared or reread by this wrapper.

Baseline: `e585bda8178d152fdda52c69de6d5ecfbace4336`. The candidate is
unregistered and unbuilt. The integrator owns normal build, complete emitted
review and admission. Exactly the new header, implementation, this document
and [report](../reports/cc12_native_lua_variant_string_fields_cleanup_source.json)
are changed; there is no production caller or Source credit in this packet.

## Native evidence and qualified C++ mapping

Root Astra reviewed all 30 bytes / 12 instructions. Fresh Root live bytes,
the saved listing and the independently replayed Original PE agree:
`c7c6c84748f6b5b4a3d6d30dea7efcc591a12a139f26b09d9b3144fd247dc270`.
The body has two direct CALLs and one plain RET, with no owned field stores.

| Native operation | Candidate behavior |
| --- | --- |
| Load DWORD `[N+8]`, TEST, conditional RET | One volatile buffer-cell read and null return |
| Load DWORD `[N+4]` | One volatile length-cell read, only after nonnull buffer |
| PUSH 1 | Unused Native physical word; no invented Source parameter |
| ADD length,1 | Unsigned 32-bit wrap, including FFFFFFFF to zero |
| PUSH size, PUSH captured buffer | Retain those captured values in C++ locals |
| CALL `00419CC0`, copy result to ECX | Call the real raw-publication getter overload |
| CALL `00BD1510` | Pass returned pool, captured block/size and actual gate |
| RET | Return through the new void C++ interface |

The Native caller does not clean the three pushed DWORDs itself, so its two
children must collectively consume 12 bytes on compatible ordinary return.
That Native combined cleanup remains accepted context. This C++ API has five
reference parameters and makes no register, flags, frame, call-stack or
arbitrary Native stack-alias equivalence claim. No exact Source byte count is
predicted before compilation. There is no new Native child-body analysis.

The earlier [cleanup-stage audit](CC12_MISSION_LUA_VARIANT_CLEANUP_EH_STAGE_READINESS.md)
recorded only metadata for this child. Its two distinct actions select the
receiver from different current frame words before tail transfer. That
snapshot is unchanged. A Source field-reference function does not supply
those Native frame deliveries, prove exception-dispatch policy or bind either
action. No Native FS adapter, guessed member class or header cast is added.

## Actual field and pool contracts

The public parameters, in order, are `volatile std::uint32_t&` for actual
N+4, `void* volatile&` for actual N+8, `NativeStringPoolStorage* volatile&`
for actual 01090AA8, `void* volatile&` for actual 01090AA0, and
`volatile std::uint32_t&` for actual 01090AA4. These are real cells supplied
by the caller, with valid typed C++ backing, alignment and lifetime. The
interface does not manufacture their relative layout or use proxy cells.
Null buffer avoids value reads of the other fields/services; it does not
authorize invalid C++ reference arguments.

For a nonnull buffer the caller's existing domain is passed directly to
`native_string_pool_get_or_create_00419cc0(publication, manager)`. Its raw
overload is concrete and registered. A populated pool publication is returned
directly. Otherwise its existing implementation uses the actual manager,
captures that manager's section, double-checks the pool, constructs/publishes
it, obtains the registration manager and registers the current publication,
then releases the captured section and reloads the pool cell. This wrapper
adds no owner allocation or separate lifetime domain. The getter's own
existing lazy allocation remains possible, and its canonical domain must
already have the required pool deletion binding.

The concrete `return_native_string_pool_00bd1510` takes the actual pool,
captured block and size, and the real live shutdown gate. For size at least
150 it invokes current CRT free without reading the pool or gate. Smaller
returns first read the gate; a nonzero gate returns without accessing the
pool. Otherwise the existing helper uses the actual pool's section, depth
and exact-size ring. Its own declaration is noexcept.

The getter still runs for every nonnull buffer, including large frees and
disabled small returns. The wrapper must not pre-read the gate to skip it.
Length and block are already captured when it runs; changes to the original
cells during that call do not replace those captures. The gate is supplied
by reference to the actual return helper. No new pool, buffer, header,
allocation provenance, ownership or repeated-retirement policy is provided.

## Failure, build and evidence boundaries

The public wrapper has no noexcept specification, catch, retry or cleanup
guard. A getter exception escapes before the return helper is invoked.
The helper's inherited noexcept, pool locking and current CRT policies remain
its own Source boundaries. This does not establish Native exception, hardware-
fault, reentry or frame-policy equivalence. Retained buffer/length bytes are
not proof that the buffer remains live after a successful return.

The current primary build receipt is
`reports/cc12_native_observer_member_cleanup_primary_review.json`: 88 inputs,
four artifacts, three existing checks, nineteen whole objects and 22 public
Core roots, with its normal build ending `2026-10-09T17:23:47Z`. Its pins and
artifacts are checked as current inherited evidence. The actual pool header,
implementation and registration are pinned separately; they are not claimed
to be among that receipt's 88 selected inputs or nineteen reviewed objects.
No graph payload or old historical pin array is copied into this report.

Validation consists of the complete selected PE/Root-byte replay, Source
capture-order and call review, relevant current dependency pins, receipt
identity checks and staged `git diff --check`. This worker adds no CMake,
ledger, Ghidra, build, test, probe, forced retention or consumer change.
