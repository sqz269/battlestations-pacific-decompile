# Native game physics teardown (R132)

Addresses: `00C4DDE0`, `00C4DC60`, `00C4DAA0`, `00C43AA0`, `00C43C00`,
`00C37D30`, `00C32250`, `00406F20`, `00407210`, `00C421B0`, `00C35400`;
existing task-manager reference `00C40FF0`; parent `004DCF90`.

## Result

The parent call at `004DCFD3` now delegates to concrete physics teardown using
the game's original dynamics construction contexts and a retained child
operation. This closes the parent's last pure address-named method. Actual
virtual payload tables and application admission are still required.

Eleven new complete normal bodies total **1,894 bytes**. The existing task-manager
destructor supplies the remaining direct dependency. Its 122-byte reference
brings live Ghidra/original executable verification to **2,016 bytes**.

## Recovered contract

`C4DDE0` receives a captured world-list header in ECX. It destroys and frees
each nonnull world, reloading the list base/count after every slot. Afterwards
it captures the current `0109E9FC`, destroys that engine's storage, frees it,
then clears the current publication. It does not reset world-list entries/count
or the `0109E9F8` profile publication. The latter can retain a freed address.

The world destructor first clears both active body lists. For each body with
attachments, it invokes broad-phase slot 4, clears flag bit 3 and handle `+60`,
then walks attachments using `+208` captured before each scalar callback.
It clears `+70` and detaches manifolds. Both lists are processed before body
slots are recycled. Recycled bodies retain their allocation and link into the
existing free chain; dynamic bodies also return their motion slots.

Manifold cleanup removes the same pointer from both endpoint vectors, unlinks
the manifold's `D8/DC` links, and returns it to the manifold pool. Reference
removal swaps in the last element and decrements the count. The unusual unsigned
underflow branch remains in source; valid callers have a present element and a
nonempty vector. Its allocation branch is not covered by the fixture.

Scene destruction invokes actual SAP scalar slot `1C`, destroys the manifold
pool lock and pages, then releases task/event arrays in native order. World
cleanup releases task arrays, nested bucket storage, and motion/body pages.
Most freed fields remain stale, as in the original. Only the native explicit
null writes are added.

Profile-node deletion recursively visits its child vector, releases heap text,
sets SBO capacity to 15 and length to zero, clears the first inline byte, then
frees its vector and node. Engine storage cleanup deletes the root tree and
profile wrapper, stops/joins the real task manager through the existing source
service, frees that owner, then frees the world-pointer vector.

## Storage and ABI evidence

The implementation reuses existing `DynWorldStorage`, `DynSceneStorage`,
`DynManifoldContainerStorage`, `DynProfileNodeStorage`, `DynProfileStorage`,
`DynEngineStorage` and `DynTaskManagerStorage`. Their constructors establish the
pool/list/vector layouts. The same world and engine allocator callbacks must
remain available through teardown. Profile SBO text has its own existing
concrete heap service; it is released through that service.

Assembly establishes the private inputs missing from pseudocode: EBX world at
`C4DAA0`, ESI bucket owner at `407210`, EAX pool at `406F20`, and ESI body/EDX
manifold at `C37D30`. Other routines use the documented ECX or stack inputs.
These C++ functions provide explicit source interfaces; they are not drop-in
register-ABI replacements. Only the normal destruction schedule is supplied.

## Ghidra repair

False no-return annotations after CRT frees hid **31 continuations, 165 bytes**.
The profile-node body also required explicit restoration through `C35466` after
its last free. The first body-definition attempt remained truncated; the saved
repair records preserve that attempt, the explicit six-byte tail repair, and
the final complete 103-byte body. All mutations used the shared write lock.

Exact byte spans, call rows, before/after annotations and repair evidence are in
[`native_game_physics_lifetime_r132.json`](../reports/native_game_physics_lifetime_r132.json)
and its flow report. Names are descriptive hypotheses. Exports are refreshed.

## Validation and limits

Strict MSVC Win32 build and all three existing CTests pass. The focused ignored
probe matches **8 native/source pairs, 384 observations and 6,439,676 bytes**.
It uses real existing constructors for engines, profiles, worlds, scenes and
pools, separate world/engine allocator domains, and zero or one real worker
thread. Cases include zero/one/two worlds, null world slots, both populated body
lists, manifolds, nested buckets, inline/heap profile strings, and null current
engine publication after construction.

For zero-worker cases, the fixture supplies the existing task-manager contract's
required null `+0` handle-array pointer; its constructor leaves that word
untouched when the worker count is zero. This does not establish teardown of
arbitrary uninitialized zero-worker storage.

The comparison checks release ordering, storage before release, callback
effects and retained publication values. OS handles are normalized only in
known handle fields; the worker ID is checked unchanged before normalization.
OS critical-section bytes are omitted. Retained upper bytes of former heap
string pointers are checked against the captured pointer before normalization.
Otherwise unspecified fixture allocation bytes are initialized consistently.

SAP and attachment methods are controlled fixture callees. The SAP fixture
scalar frees its owner; residual SAP children are disposed separately after
comparison. The real task-manager service is shared by both envelopes. Fixture
cleanup also closes handles intentionally left open by the native task-manager
schedule. None of this proves the missing virtual methods or task scheduling.

A source attachment failure preserves failed-operation diagnostics and rejects
replay. The caller resolves fixture ownership before retiring diagnostics. This
does not reproduce native FH3/SEH unwinding. The existing parent probe still
matches **52 cases, 3,537 observations and 141,402,076 bytes**, with four source
failure cases; it controls the physics call and checks its child binding.

Allocator failures, private stack aliases, hardware faults, malformed storage,
concurrency, native exception ABI, application admission and gameplay remain
unproved. Closing the address methods alone does not make the raw game runnable.

## Follow-up

Supply and validate the actual SAP/attachment virtual lifetime methods, then
audit the remaining raw-game callback and admission requirements. Preserve the
existing allocator/publication domains through construction, use and teardown.
