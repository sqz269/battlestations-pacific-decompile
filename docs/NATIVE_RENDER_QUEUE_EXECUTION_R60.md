# Raw render queue and end-frame composition (R60)

`00B1EBE0` now executes the actual 0x34-byte queue through the recovered raw
command provider. `00B2D8E0` calls it directly after the raw queue getter.
The end-frame `NativeRendererEndFrameRemaining` callback interface is removed.

## Recovered contract

The complete 187-byte queue body uses signed current counts and reloads the
current command list around calls. On context changes it publishes queue+30
before retaining the incoming actual+4 atomic and releasing the previous owner.
It then reloads the indexed command and calls genuine `B1D950`. After execution
it releases the **current** queue context and clears the field after that call.
Current control+20 equal to zero causes a fresh indexed-command lookup,
`B1DDD0` destruction and ordinary free. The final current control decides whether
`B1CC80` shrinks the list to zero. No outer cleanup or rollback is added.

The source accepts borrowed execution/owner/lifetime contexts and persistent
per-command frame slots. It consumes each supplied slot once and leaves child
frame preparation and retirement to their established callers. An initially
nonpositive count requires no execution context. Destruction requires the same
actual owner associations and raw string-pool publication cells as execution.
Invalid negative array headers on a reached resize remain outside the domain.

Command `B1DDD0[541]` and group `B1D760[187]` destruction now have raw
AA8/AA4/AA0 string-pool overloads. They reuse the recovered release, array and
model schedules. Final name cleanup disarms before obtaining the current pool
and returning the captured buffer with current length plus one. A normal raw
getter failure can therefore propagate after disarming; a second failure during
unwind terminates. Existing string-storage overloads retain their contracts.

The original exception maps, handler bytes, profile words, call sites and prior
annotations are recorded in
[the machine-readable report](../reports/native_render_queue_execution_r60.json).
Unreferenced alignment at B2D96A, B2DAED and B1DA6D is retained without repair.

## Validation

- Strict MSVC Win32 build (`/MD /W4 /WX /fp:strict`) and three existing CTests.
- Fifteen focused probe checks: full copied native queue versus source for
  empty/nonpositive counts, retained contexts, a real final old-context deletion,
  and command/group/raw-pool cleanup with control zero; updated end-frame and
  wrapper inactive exits also pass.
- Whole queue, command and context comparison in retained cases; source frame
  consumption, actual atomic counts, canonical retirement, one pool registration,
  and manager drain checked. Freed objects are not read.
- Linked machine inspection confirms direct end-frame/queue/command edges and
  raw destructor/pool overloads. Fourteen same-process modules are resolved by
  file handle and verified as I386 in both memory and the physical file.
- Exact compiler/source/build/probe/native evidence is sealed before integration;
  the report records archive hashes and the later combined-build receipt.

The native fixture relocates IAT/global/profile cells and uses ABI adapters to
genuine source destruction, resize, terminal deletion and readiness providers.
The copied command returns at its inhibited-renderer readiness gate. The native
terminal adapter restores the numeric context profile before source dispatch.
These adaptations are explicit; original helper bodies and native EH are not
executed by this probe.

## Remaining boundary

The source interfaces are not binary ABI replacements. Active rendering, jobs,
selected material passes, active end-frame, callback-driven list mutation,
failure injection, original FH3/SEH/private-stack behavior and gameplay remain
unproved. The application still needs a complete constructed owner graph and
startup/frame-loop integration. Passing this packet does not establish a
runnable game or visual parity.
