# Loader submission, pumping and stop lifetime

Addresses: `00509190`, `005092e0`, `00737a50`, `00504850`, `00506bf0`.

The loader advances after the VFS provider pump. Its stop routine waits for the
loader job count to become zero; it does not check FileStore pending keys or
active physical reads. Those three quantities require separate observation.
The FileStore completion callback selects the current loader's front job,
without identifying or retaining the job that submitted the request.

This audit extends [VFS_PENDING_LIFETIME.md](VFS_PENDING_LIFETIME.md) and the
currently integrated `FileStoreRequests` contract. All five complete byte spans
were freshly compared with the installed PE. The existing `bsp` project and
`/battlestationspacific.exe` were checked before every live read/export command.
Lengths, SHA-256 values, ABIs, evidence addresses and annotation proposals are in
[vfs_loading_pump_lifetime_audit.json](../reports/vfs_loading_pump_lifetime_audit.json).
Raw evidence is under `exports/bsp/parallel_vfs_loading_pump/`.
This packet contains no new C++ implementation or runtime test.

## Native fields and call order

The caller object has a pointer array at `+10h` and its count at `+14h`.
`00506bf0` shifts pointers left and decrements the count, establishing that the
stop condition is a job count, rather than a thread handle or physical-I/O count.
For the front job, `+0` is a numeric state, `+4` is a stop/retire byte, `+14h` is
the name wrapper passed to FileStore, and `+1Ch` is an optional later completion
function. The object at loader `+0Ch` exposes a virtual `+10h` readiness/busy
query; the precise worker interface is not reconstructed.

The ordinary application frame performs its earlier game/timing work, propagates
the sticky exit request, then executes:

1. `00737b79/00737b7f`: load VFS global `0109ceec`, call `00bdb0b0`.
2. `00737b84`: call accessor `004fde20`.
3. `00737b89/00737b8b`: pass the returned object in ECX to `00509190`.
4. `00737b98..00737bac`: remaining frame cleanup/accounting calls.

Thus a physical completion delivered by this frame's pump can make a job state 2
before the same frame advances that job. A request first submitted by this frame's
loader update occurs after that pump and needs a later pump for physical callback
delivery. Existing provider-pump semantics still permit requests submitted *inside*
callbacks to be examined later during that same pump; there is no contradiction.

## Loader update at 00509190

ECX is the loader object, with no stack arguments and a plain RET or tail call.
The complete span is `00509190..005092dc` inclusive. Ghidra's current void/no-arg
signature loses the ECX input; assembly restores it.

An empty loader returns immediately. If the current front's stop byte is nonzero
and its state is 0 or 2, the routine first retires that front through `00506bf0`.
It then reloads count and front; a newly exposed front can advance in the same
call. The following table describes the dispatch after that reload.

| State | Established behavior |
|---|---|
| 0 | Obtain the FileStore factory through `004fc150`, then its store through `00be80b0`; submit the job name at `+14h` and callback `00504850` to `00be7cd0`. |
| 1 | No dispatch branch: return while waiting for another writer to change state. A stop byte alone does not retire this state. |
| 2 | Call `00504790(job)`; set state to 3; call `00501620(loader, job)`; return. Those two callees remain opaque dependencies. |
| 3 | Query loader `+0Ch` virtual `+10h`; a true AL returns without advancing. If false, take the normal or stopped path described below. |
| Other | Return without dispatch. |

State 0 has two branches selected by the VFS byte at `+78h`. Its policy meaning
is not established here. Both branches pass the same request and callback.
With that byte nonzero, a false request result sets the job stop byte and leaves
state 0; a true result sets state 1. With the byte zero, a false result sets state
2 and a true result sets state 1 (`005092d0..005092d7`). This is a branch on the
native boolean result; there is no resident-versus-pending distinction here.

In state 3 with a false worker query and a clear stop byte, an optional job
`+1Ch` function receives the values from `job[+8]->+0Ch` and `job[+8]->+8`
at `00509228..00509233`. The routine then calls `00501670(&job[+8], 0)`.
If job `+0Ch` remains nonzero it calls `00501620(loader, job)` again; otherwise
it retires the front. The meaning and lifetime of that intermediate result are
unresolved. With the stop byte set, it calls `00483850(job[+8]+0Ch)` and repeatedly
retires fronts until loader count is zero. This branch contains no additional
provider pump and establishes no physical-read cancellation.

## Completion routing and duplicate requests

`00504850` is a separate 19-byte entry missing from Ghidra's function inventory.
Its two data references are the callback arguments at `00509285` and `005092bc`.
The complete instructions are:

```text
00504850  call 004fde20
00504855  mov eax,[eax+10h]
00504858  mov ecx,[eax]
0050485a  mov dword ptr [ecx],2
00504860  ret 8
```

It is a callee-cleanup two-argument callback, consistent with the existing
FileStore `(firstName, secondName)` callback; it reads neither argument.
It reacquires a loader through the same accessor as the frame and writes the
current first job's state. There is no captured origin, front/count validation,
name comparison, job retain/release, or stop-byte test in these 19 bytes.
The accessor's singleton construction/lifetime has not been expanded. A faithful
bounded integration must keep the accessor identity and intended front valid
through delivery; retaining an originating job would change this routing.
Within the inspected serialized update paths, a state-1 front is not retired,
even after its stop byte is set. That naturally holds the front during its
deferred physical read. A wrong-front write therefore requires another writer,
accessor replacement, or a callback belonging to a different request lifetime;
this packet proves the routing dependency, not a concurrent race in the game.

The current FileStore contract resolves names before duplicate checks. Resident
and pending duplicates both return native true without storing another callback.
A new request stores only the first callback and submits flags 2. Completion
erases the pending key, inserts the resident stream, then calls the stored
two-name callback. Terminal physical failure removes the OS read without
calling FileStore completion, leaving its pending key observable.

Combining these existing contracts with the newly recovered update gives:

| FileStore result | Loader state after submission | What can change it next |
|---|---|---|
| New request accepted | 1 | Its successful completion invokes the stored callback, which sets the *current* front to 2. |
| Already resident | 1 | This request registers no callback. The inspected paths supply no automatic transition. |
| Already pending | 1 | Only the originally stored callback remains. It need not be this loader callback or target the same current front. |
| Immediate rejection, VFS `+78h` nonzero | 0, stop byte set | A subsequent loader update retires that front. |
| Immediate rejection, VFS `+78h` zero | 2 | A subsequent update takes state-2 processing, or retires it if stopping. |
| Accepted read later fails physically | 1 | No FileStore callback occurs; the pending key remains, although the physical read may be gone. |

Under only these recovered writers, the resident and terminal-failure cases can
leave state 1 indefinitely. A pending duplicate has no new completion guarantee.
This is a control-flow consequence, not a reproduced game hang: job creation,
other state writers, and outer invariants may prevent those inputs in the game.
Do not silently synthesize callbacks, mark resident requests ready, remove failed
pending keys, or treat a true submission result as a completion promise.

## Retirement and stop waiting

`00506bf0` takes ECX loader, no stack arguments, and RET. For a valid nonempty
queue it calls job `+20h` virtual `+4` with argument 1 when nonnull, clears that
field, obtains the FileStore and calls `00be7130` with the job name, then destroys
the front through `005051a0` and `_free`. The FileStore callee's strings identify
RemoveFile, but its body and pending-map behavior were not audited here.

Ghidra incorrectly ends the ordinary nonnull destruction path after `_free`.
The integrator's focused inspection identified an erroneous `CALL_RETURN` flow
override on the call at `00506c39`; the `_free` thunk and target are already marked
returning. The repair belongs to this call site, not a global library flag.
The complete raw bytes at `00506c3e..00506c46` restore stack cleanup and clear
the old front pointer. Execution continues through the pointer shift at
`00506c54..00506c6a` and count decrement at `00506c6c`. It does not return early
before decrementing count. This is crucial to the stop loop's normal progress.

`005092e0`, identified previously by `GIAchievements::StopLoadingThread()`, takes
ECX loader, no stack arguments, and RET or a tail call. For count zero it returns
after the diagnostic call. Otherwise it calls global `00f8d394` virtual `+18h`,
sets the current front's stop byte to 1, and repeats while loader count is nonzero:

1. `00509320..00509322`: `Sleep(100)`.
2. `00509324..0050932a`: pump VFS global `0109ceec` through `00bdb0b0`.
3. `0050932f..00509331`: update this loader through `00509190`.
4. `00509336..0050933a`: test loader count again.

After count reaches zero it tail-calls global `00f8d394` virtual `+1Ch`.
The two surrounding virtuals' semantics remain unknown. There is no deadline,
retry cap, per-request cancellation call, FileStore-key query, or physical-active
count query in the complete stop body. The stop byte is initially written only
to the front job. Retiring a stopped state-0/2 front can expose and start a later
unmarked job; stop entry does not establish a blanket ban on new submission.

A state-1 job survives the stop flag until some other path changes its state.
With no such writer, the native loop has no termination guarantee even when
there is no active OS I/O. Conversely, loader count zero alone does not prove
all physical providers or other FileStore users have drained. Callback delivery
runs on the invoking pump's thread; these callers establish no exclusive thread
affinity, no lock, and no cancellation policy.

## Smallest implementation boundary

A concrete outer-frame fragment can preserve the observed sequence: run the
existing provider pump, then advance the supplied/current loader exactly once.
A concrete stop *iteration* can preserve `Sleep(100) -> pump -> loader update`
and report the remaining loader job count. Supplying the observed operations as
explicit dependencies keeps this scheduling boundary separate from the missing
parser/worker behavior; it is not a complete reconstruction of `00509190`.

For a host drain utility, report loader count, unique physical owner active-read
counts, and FileStore pending-key count separately. Stop submissions with the
existing host operation only when the caller requests that separate policy;
native stop entry does not do this. Stop a bounded diagnostic step on its caller's
budget and preserve still-live owners. A timeout, stalled-state result, or refusal
to accept new work is a host policy, not recovered native behavior. Do not destroy
the FileStore/controller/providers merely because the loader count reached zero,
or require pending-key zero as proof of OS completion. The pump report's
`remaining_observations` is a sum over mount visits and can count a provider more
than once, so it is not a unique active-read count.

Full loader advancement remains gated on the named-but-incomplete dependencies
`004fde20` (loader accessor lifetime), `00504790` (state-2 preparation),
`00501620` (worker submission), `00501670` (intermediate result mutation),
`00483850` (stopped result cleanup), `005051a0` (job destruction), `00be7130`
(FileStore removal), loader `+0Ch` virtual `+10h`, and global `00f8d394` virtuals
`+18h/+1Ch`. Creation of jobs, other state writers, exception/unwind behavior,
and safe replacement of the current loader/front remain outside this packet.
