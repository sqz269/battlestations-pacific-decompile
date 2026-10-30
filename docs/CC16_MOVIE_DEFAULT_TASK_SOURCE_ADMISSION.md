# Raw Movie default storage and qualified task construction

The primary and independent source reviews accept two existing-body interface
improvements against published base `f90f5d178`. They add no native-body,
vendor-library, original-ABI, runtime or gameplay credit. Both production callers
remain unchanged; the full activation goal continues with gameplay runs allowed.

`54fb69ac5` adds `construct_native_gui_movie_default_00aafe20` for fresh aligned
130h raw Movie storage. It calls the real AA9390 base constructor with type10 and
the genuine stable D7A24C cell, then performs the exact fifteen derived stores
(54 bytes) in native order. F4 and every other unwritten byte remain untouched.
The D5C270 word is an identity token, not a callable C++ vtable. This wrapper does
not allocate the pool slot, own the base, catch base failure, supply the constant
producer or activate any decoder consumer. The older semantic projection and
its callers remain separately identified. See the default-storage author and
independent-review documents.

`237e6c518` adds `dyn_task_manager_construct_qualified_00c37740`, retaining the
old normal-path API and all 23 old emitted records. Its source captures the
worker count and initializes the three queue words before the outer try. After
the actual critical-section initializer returns, the inner cleanup is armed
regardless of its Boolean result. A covered later C++ exception deletes that
critical section, then the outer handler releases the CURRENT task+C backing
without clearing the queue fields. Neither handler frees the raw task object,
closes semaphore/event/worker handles, rolls back workers, or invokes the full
task destructor. The real memory domain and normally returning cleanup remain
preconditions. See the task construction-cleanup author and review documents.

The `/EHsc` compiler arms state0 before the count read and queue stores in the
new emitted task entry. There is no C++ throwing operation in that valid-input
prelude, so the covered C++ throwing epochs are preserved. This does not prove
the native store/fault epochs, SEH transport, FH3 table identity, secondary
cleanup failure, or a drop-in original entry ABI.

The common MSVC Win32 Release build at `923e0fb16` passes both existing suites
(12.18 seconds test time). Both actual source files occur once on `bsp_core`.
The manifest-embedded link-only probe resolves both actual entries and their
real dependencies; it was never executed and creates no fixture providers.
The four source pins match the reviewed candidate commits exactly. Ten frozen
build/link artifacts and their hashes are recorded under `local/source_batch3`.

The common Movie entry is 166 bytes, with a balanced EBP frame, compared with
the strict author/reviewer entry's 164 bytes. Static PE inspection independently
confirms all fifteen stores, their exact offsets/sizes/order, and the sole
direct call to the actual base entry. This is not a byte-identity claim. The
common task entry is 220 bytes and preserves the reviewed state0/state1 and
inner/outer handler ordering, including the stated compiler qualification.

Next are genuine task cleanup propagation into C55, the actual engine/Game
owner, raw Movie lifetime/property/loaded-hook and Page child support, and
real VFS/renderer/Lua/decoder/clock/audio custody. Required current-profile
gameplay activation gates remain open. The detailed machine-readable admission
is `reports/cc16_movie_default_task_source_admission.json`.
