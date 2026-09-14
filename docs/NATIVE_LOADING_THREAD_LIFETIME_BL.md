# Native loading worker lifetime audit

Packet `orch4_native_loading_thread_lifetime_bl` is a read-only audit based on
`de1cb16c93d69da1ba015d79cfefe1600b51820d`. The [report](../reports/native_loading_thread_lifetime_bl.json)
pins fresh installed/live spans, direct calls, indirect imports, dataflow and
listing gaps. No source, runtime, CMake, metadata, tests or Ghidra state changed.

**No loader-owned stop/wake/join/CloseHandle sequence was found in the bounded
domain.** The complete start, worker, drain and normal/EH teardown paths were
checked, together with all ten currently known getter call sites, the loader
publication's literal references, relevant Win32 import references and the
nearest WinMain/CRT teardown. The worker returns to an indefinite event wait
after ordinary completion. The owner can later delete that event and
free its context without a preceding thread join in these paths. This is an
unclosed lifetime contract, not evidence that a proposed stop protocol exists.

## Three separate states

| Storage | Producer/consumer | Meaning established by instructions |
|---|---|---|
| Loader DWORD `+4` | Constructor `4FDBCC` writes0; worker `501540/5015F5` reads it | Worker exit condition, checked only after a wait returns |
| Job byte `+4` | `509308` and `5092AA` write1; update/append inspect the byte | Stop/cancel this job; not the worker exit DWORD |
| Loader HANDLE `+1C` | Constructor writes0; `501623` tests and `501640` writes CreateThread's result | Lazily created worker thread handle; no close/join read found for this field |

Loader count `+14`, FileStore pending names and physical reads are different
domains. Drain tests only the loader count. It pumps current VFS callbacks,
but neither zero jobs nor an unsignaled event proves all physical IO has ended.

## Start and worker dataflow

`501620..50165D` (62 bytes) takes ECX loader and one job stack argument, RET4.
Only a zero `loader+1C` triggers
`CreateThread(NULL,0,501510,loader+4,0,NULL)`. Its HANDLE is stored without a
success check. Creation occurs before inspecting job work count `+0C`, so an
empty job can create an unsignaled, waiting worker. Only nonzero work count
publishes `loader+8=job` and dispatches event virtual `+4` to signal. A failed
CreateThread still allows that publication/signal; a later call can retry
because the stored handle is zero. No failure completion is synthesized.

`501510..501615` (262 bytes) is a one-argument thread procedure, RET4,
EAX=1 on its observed exit path. EDI captures its actual argument `loader+4`:
EDI+0 is the worker stop DWORD, EDI+4 the current job, and EDI+8 the event.
It calls current event virtual `+8` to wait indefinitely and ignores the API
result. A nonzero stop DWORD then exits. Otherwise it performs these reads:

1. Read current published job, first work record, and that record's string data;
   null data selects the existing empty string at `E18D78`.
2. Measure its C string and allocate/copy an owning temporary through `41DD40`
   and `BF7680`. The temporary does not borrow the source header's stored length.
3. **Read current job/work again**, capture this work record in ESI, then call
   `7188A0` with ECX temporary header. Incoming EAX is not a resource argument.
4. Store returned EAX into captured work `+0C`, release the temporary, call the
   **current** event's reset slot `+0C`, then its current wait slot `+8`.
5. Check the worker stop DWORD after that wait and repeat or return1.

The record receiving a result is the capture after temporary construction;
it need not be the record from which the name was first read if a dependency
changes publication. A source worker must retain this schedule. The raw
state is not made safe by projecting it onto a host queue or C++ atomics.

Worker FH3 handler `C68C98` selects `D91F68`; the one-entry map at `D91F60`
unwinds through `C68C90 -> 41DD20` for the temporary string. There is no
worker-owned stop, reset, signal, join or handle cleanup in that unwind map.
The exception boundary beyond this raw CreateThread entry remains unproved.

`7188A0..7188BB` captures factory `7175D0`, reacquires resource manager
`4C1400`, and calls `B80720`. Current source `marker_classes.cpp` routes this
through `MarkerClassHost::load_and_cache_resource_00b80720`, explicitly an
unreconstructed operation in the header. The wrapper and factory names do not
make the worker's resource loading, ownership or thread context runnable.

## Draining and teardown

`5092E0..50934C` (109 bytes), ECX loader/RET, logs then returns immediately
when job count is zero. Otherwise it calls current `F8D394` virtual `+18`,
sets only the initial front job's stop byte, and repeats Sleep100, current
`0109CEEC -> BDB0B0`, and `509190(loader)` while count remains nonzero. It
finally reacquires `F8D394` and tail-dispatches virtual `+1C`. It does not
write the loader stop DWORD, signal a worker exit or examine its HANDLE.

Those external bracket methods are now bounded more precisely. Renderer base
construction `B283F0` publishes its parent through `B25F40` and stamps
`D5E628`; derived construction `B32410` stamps `D5F0A8`. In both profiles,
slots `+18/+1C` are `B28450/B28460`. **Each is a single native RET byte**,
confirmed in the live program and installed PE. They hide no stop sequence
for these profiles. The dispatch still uses the current external owner and
table; this does not permit an unconditional no-op for an unproved profile.

`509190..5092DC` polls current event `+10` in state3. Nonzero raw AL returns
without advancing. With AL=0 it either dispatches the result and removes work,
or releases the stopped result and retires queued jobs. It can leave a worker
waiting again after job completion; it contains no loader stop/HANDLE access.
Full FileBlock/resource-provider reconstruction is another packet's scope.

`509FD0..50A052` (131 bytes), ECX loader/RET, stamps `CEB198`, drains, clears
and frees the queue, scalar-deletes current event `+0C` with flags1, clears that
pointer and current `E18D4C`, then stamps `CE3818`. `50ACE0..50ACFD`
(30 bytes), ECX loader/stack flags/RET4, invokes it and optionally frees the
outer owner. Neither accesses loader stop `+4`, worker job `+8` or thread
HANDLE `+1C`. Event deletion `BD19B0` closes **event+4**, not loader+1C.

Destructor FH3 handler `C6983E` selects `D92D3C`, with three unwind-map
entries at `D92D24`. They invoke queue cleanup at this+10 (`C69833 -> 4FCBC0`),
worker-context event cleanup at this+4 (`C69828 -> 4FAA90`), and base cleanup
(`C69820 -> 4F93A0`). `4FAA90..4FAAAA` reads its own +8, scalar-deletes that
event and clears the pointer. **It also contains no stop or join.** Queue
cleanup's returning tail is decoded separately from its incomplete listing.

## Bounded upstream closure

All ten known `4FDE20` call sites consume its result in enqueue (`50AB9E`),
last-job result-callback assignment (`50ABA8..50ABB7`), work append
(`508E2E/50AC7A`), drain (`50A9E9/50B3FB/51504B/51633C`), frame update
(`737B8B`) or current-front completion (`504855..504860`). None routes the
owner to a thread-stop service. Enqueue and append were inspected through
their complete bodies to separate job fields from owner fields.

The live publication xrefs initially show seven uses. A bounded literal scan
of `.text` finds eight: undefined `4FB190..4FB1B8` supplies the missing write
at `4FB198`. This is a base scalar deletion body: clear current `E18D4C`,
stamp `CE3818`, optionally free. Adjacent undefined `4FB180..4FB188` stamps
`CEB15C`, whose sole established slot is `4FB190`. These bodies add no stop,
wake, wait or handle close. Raw scanning avoids treating absent Ghidra xrefs
as proof of absent instructions. It does not prove every possible pointer alias.

The relevant bounded import queries returned 46 CloseHandle references, 12
WaitForSingleObject, six WaitForMultipleObjects, nine CreateThread, four
ExitThread and three ResumeThread references, all below their query limits.
The loader's only CreateThread reference is `50163A`; there is no loader
close/join import call site. For comparison, actual renderer-worker deletion
`B33B50` sets its own stop byte, signals, waits on its own `+8` handle and closes
that handle. It is a distinct owner/layout, not the loader's missing destructor.
No TerminateThread, GetExitCodeThread or SuspendThread import exists in the
inspected PE import table; dynamic API lookup/other aliases are not ruled out.

WinMain's `8F846E` CloseHandle consumes EBP captured from CreateMutexA at
`8F8301`. It closes the single-instance mutex, not the loader worker. Before
that, `8F8455 -> BD0400` drains registered singleton pointers through their
current first virtual slot. `CEB198[0]=50ACE0`, so this routes back to the
non-joining loader destructor. Later `BD30D0` only destroys the random-thread
registry critical section at `1090AC0`; it is not a generic worker join.

After WinMain returns, the observed CRT path can call `BFBDBB`, whose common
exit routine eventually calls `BFBA53 -> ExitProcess`. This is an external
process-exit path, not proof that the loader was safely quiescent before its
event/context were freed. Untraced virtual dispatch, CRT exit callbacks,
dynamic providers and arbitrary aliases prevent a universal absence claim.
No runtime trace or safe destruction ordering has been established.

## Next bounded contract and limits

The smallest independent source-body packet exposed by this audit is
`4FAA90`: accept the actual 0Ch worker context at loader+4, load its current
event pointer +8, scalar-delete the actual event with flags1 when nonnull,
then clear that current context field. It must add no stop/wake/join logic.
It can be implemented without wiring any worker into production. Separately,
the two concrete renderer RET leaves and base `4FB180/4FB190` bodies are
small evidence-backed candidates after definition/ownership review.

A future `501620/501510` implementation must borrow the actual loader,
existing native event and string domains, and a completed resource loader;
preserve publication/rereads and exact raw thread argument/RET4 ABI; and state
the unresolved owner/event/HANDLE lifetime explicitly. Production binding
cannot be justified by this audit. If the chosen source architecture needs
reusable clean shutdown, that is a new policy requiring an explicit decision,
not a recovered native stop protocol. The current frame source still reports
the unimplemented loader boundary at `GameFrameHost::update_loading_queue`.

Outstanding listing definitions are `4FB180`, `4FB190`, `B28450`, `B28460`
and the two FH3 handlers. `4FCBC0` lacks its decoded `4FCBD2..4FCBD6`
ADD ESP4/POP ESI/RET continuation. No repairs or annotations were attempted.
All 54 fresh spans match live Ghidra and the installed PE; all 97 direct
CALL/tail-JMP rows pass `verify_report_calls.py`. Evidence is static only:
exact bytes, current membership, direct-call checks and source inspection.
No build, fixture, original instruction
execution, gameplay, thread termination or IO-drain validation is claimed.
