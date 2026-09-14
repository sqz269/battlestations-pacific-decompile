# Native loading queue frontier

Addresses: `00737B79`, `004FDE20`, `004FDBA0`, `00504850`, `00509190`,
`005092E0`, `00501620`, `00501510`, `00504790`, `00506BF0` and the bounded
dependencies listed below. Read-only audit; no implementation or annotation.

Worker baseline is `21581aad90ce0ecd1a1f11dcd0063ebf2470c32e`. The primary
checkout was observed at `fd6a5f3ce5f553560e02a492283a3eecc722900d`, with
uncommitted native pending/runtime/frame integration. The companion
[report](../reports/native_loading_queue_frontier_bk.json) pins observed source
hashes; these are an audit snapshot, not a claim about later primary builds.

## Next production edge

The complete listing at `737B79` loads current `0109CEEC`; `737B7F` calls
`BDB0B0`. Then `737B84` calls loader getter `4FDE20`, `737B89` moves its
EAX result into ECX, and `737B8B` calls `509190`. The primary's observed frame
pump now follows the current-publication path. `GameFrameHost::update_loading_queue`
still records the explicit unimplemented `4FDE20` boundary. Its loader callback
dispatcher is intentionally not installed in production.

The next source call must use this actual loader domain, preserving its
publication and queued storage. A new host queue or a completed FileStore
callback that captures the original job would implement different behavior.

`504850..504862` is exactly 19 bytes: call **current** `4FDE20`, load loader
`+10h`, load its first job pointer, write job state `2`, RET8. Neither name
argument is read. There is no empty/count check, identity match, origin
capture, retain/release, or stop-byte test. A FileStore callback remains tied
to whichever loader/front job exists when it runs.

## Actual storage and synchronization

`4FDE20..4FDEDC` returns publication `E18D4C`. Its slow path obtains the
existing lifetime manager `415350`, captures manager `+10h` critical section,
enters and increments section `+18h`, rechecks publication, allocates `20h`,
calls `4FDBA0`, publishes, reacquires the lifetime manager and registers the
**current** publication through `BD0C30`. It then decrements/leaves the
captured section and returns current publication. Original ABI: no inputs,
EAX loader, RET. Publication precedes registration; do not invent rollback
on registration failure. Constructor failure and lock cleanup have separate
native EH states and require inspection in the implementation packet.

`4FDBA0..4FDBF8` writes this `20h` owner, proven by its producer stores:

| Offset | Storage / initialization |
|---|---|
| `+0` | Numeric profile `CEB198`; first slot is `50ACE0` scalar deletion |
| `+4` | Worker stop DWORD, initially zero |
| `+8` | Published worker job pointer, initially zero |
| `+0C` | Actual event owner returned by `BD1970`, CL=1 |
| `+10/+14/+18` | Job-pointer array / signed count / capacity, initially zero |
| `+1C` | Worker thread HANDLE, initially zero |

The event is initially unsignaled and manual-reset. Its actual `D6821C`
slots are delete `BD19B0`, signal `BD1910`, infinite wait `BD17C0`, reset
`BD1960`, and poll `BD1920`. The first four and construction already have
actual-storage C++ in `native_event_owner`. Poll does not: it calls
`WaitForSingleObject(handle,0)` and returns AL=1 for signaled, AL=0 for
timeout/abandoned/failure (reading GetLastError on failure), and preserves
the low byte of other unexpected statuses. A blanket C++ bool conversion
would change that last behavior.

`505530..5055B5` produces an actual `24h` job. It initializes state DWORD
`+0`, stop **byte** `+4` (padding `+5..+7` is not written), work-array
pointer/count/capacity `+8/+0C/+10`, owning package-name header `+14/+18`,
optional result callback `+1C`, and FileBlock pointer `+20`.
`5055C0..505651` allocates `24h`, constructs this job and appends its pointer
to loader `+10h`, growing through `4FB310` when count equals capacity.

Each work record is `10h`: owning name header `+0/+4`, raw callback context
`+8`, and raw resource result `+0C`. Work-record resize/removal/destruction
only own the string. `504D20` attaches work to the last job, but its
`4FE700` pair helper and `5048F0` append helper remain outside this audit's
bounded ready packets. The job callback at `509233` receives
`(firstRecord.result, firstRecord.context)` as two callee-cleaned arguments.

## Update and lifetime dependencies

`509190..5092DC` is a complete 333-byte native state machine, currently
named/evidence-only in the repository:

| State / condition | Required behavior and dependencies |
|---|---|
| Entry stopped front in state 0 or 2 | `506BF0` retires it; reload queue/count/front |
| State 0 | Read current VFS `+78`; current FileStore getter `4FC150`, lazy store `BE80B0`, then `BE7CD0(job+14,504850)` |
| State 0, accepted | Write state 1, including resident/pending duplicate returns without callbacks |
| State 0, rejected | Nonzero VFS `+78`: set job stop byte. Otherwise write state 2 |
| State 1 | No advancing branch |
| State 2 | `504790` prepares FileBlock; write state 3; `501620` starts/publishes work |
| State 3, signaled event | Poll event `+10` and return while AL is nonzero |
| State 3, normal completed work | Optional result callback, `501670(work,0)`; more records restart `501620`, otherwise retire front |
| State 3, stopped | `483850(&firstWork.result)` releases current resource; repeatedly retire queued jobs |

`501620` creates a thread only when loader HANDLE `+1C` is zero:
`CreateThread(NULL,0,501510,loader+4,0,NULL)`. Only a nonzero job work count
publishes job `+8` and signals the event. No new handle check or no-work
completion is present. `501510` waits, checks the worker stop DWORD, copies
the first work name as a C string, calls `7188A0` with ECX name, stores its
result into the captured work record, resets, waits, and checks stop again.
Incoming EAX is not an argument to `7188A0`.

`7188A0` has a source wrapper in `marker_classes.cpp`, but its
`MarkerClassHost::load_and_cache_resource_00b80720` is explicitly an
unreconstructed operation. The actual `7175D0` factory owner is available;
that does not implement factory creation, resource manager cache/miss
dispatch, structured resource parsing, or resource ownership on the worker.
Do not mark the worker runnable because the wrapper's ledger says reconstructed.

`504790` creates a `1Ch` FileBlock after taking substring start13,
length=`storedNameLength-18`, then calls `BE0A30(temp,1)`. The actual FileBlock
constructor and its identifier/VFS block-entry/exit domain remain missing;
the existing string-copy fragment is not a FileBlock implementation.

`506BF0` releases job FileBlock through current virtual `+4(1)`, uses current
FileStore getter/lazy cache and `BE7130(job name)`, destroys job storage,
frees the job, clears the captured front slot, shifts pointers and decrements
count. `BE7130` removes only the resident name; it does not erase pending
keys or cancel physical reads. `483850` decrements an actual resource `+4`,
calls current resource virtual `+0` at zero, then clears the current result
cell. Both entry bodies lack native C++ implementations.

`5092E0` marks only the initial front stopped, then loops while loader count
is nonzero: Sleep100, pump current VFS, update the loader. It brackets that
loop with current global `F8D394` virtual `+18/+1C`. The two methods are
still explicit dependencies, not methods on the loader itself.
`509FD0` drains through that routine, clears/frees the queue, destroys the
event, clears `E18D4C`, and writes base profile `CE3818`; `50ACE0` optionally
frees the outer owner. Neither writes the worker stop DWORD, wakes a worker
for exit, waits on its HANDLE, nor closes it. Prior worker shutdown remains
unproved. Zero loader jobs, zero FileStore pending keys, and zero physical
reads are distinct conditions.

## Ready disjoint reconstruction packets

All listed addresses and proposed source files were unleased at the initial
check. The final refresh found `BE7130` and its proposed source files already
claimed by `agent/orch4-vfs-pending-bk:orch4_native_filestore_remove_bk`.
Owner entries and work storage remained unleased. These are **source-body**
packets; no individual packet alone enables the complete production loader.
Recheck leases at assignment. The report contains exact address/file lists
and input/output contracts.

| Packet | Addresses | Files / completion condition |
|---|---|---|
| `native_loading_queue_owner_entries` | `4FDE20`, `4FDBA0`, `504850`, `BD1920` | New `native_loading_queue_owner.hpp/.cpp`; borrow actual E18D4C and lifetime manager publications, reuse actual events, implement current-loader callback and raw-AL poll. Do not register a placeholder deleting destructor: lifecycle integration still needs `509FD0/50ACE0/5092E0` |
| `native_loading_queue_work_storage` | `4FB310`, `4FB4B0`, `501670`, `5018A0`, `5019D0`, `5051A0`, `505530`, `5055C0` | New `native_loading_queue_work_items.hpp/.cpp`; complete actual pointer/work vectors, job construction/enqueue/storage destruction; preserve raw context/results and normal/EH allocation order |
| `native_filestore_remove` | `BE7130` | New `native_filestore_remove.hpp/.cpp`; normalize actual local header, existing native resident find/erase, current string cleanup; no pending mutation |

Use `gpt-6-astra`, `fork_turns:none`, `reasoning_effort:xhigh` for the
work-storage packet because its reserve listings require control-flow/body
repair review. The other two packets can use `gpt-5.6-sol` with the same
fork/effort settings and the explicit ABIs above. All integrate through the
primary; no worker edits shared runtime bindings, CMake, or another packet's
files. The next update/lifetime packet depends on all three plus FileBlock
and resource release/source dispatch. The thread packet remains gated on
actual resource loading and a proven thread-context/exit lifetime contract.

## Listing holes to preserve in the handoff

Current live `proto` queries return no containing function for these decoded
continuations, despite outer functions having wider min/max bounds:

| Range | Parent / required continuation |
|---|---|
| `4FB361..4FB369` | `4FB310`: ADD ESP4, publish new array/capacity, POP EBX |
| `5019A4..5019B2` | `5018A0`: capture requested capacity, ADD ESP4, publish new storage/capacity, restore registers |
| `506C3E..506C46` | `506BF0`: ADD ESP4, clear captured front pointer |
| `50ACF5..50ACF7` | `50ACE0`: ADD ESP4 after outer free |

`5051FE` and `50A01C` currently do belong to their job/loader destructor
functions. Therefore the earlier blanket truncated-destructor warning is
stale for those two sites. Decoding is not proof of Ghidra body membership.
This audit changes neither. Live queries use the verified existing `bsp.gpr`
and `/battlestationspacific.exe`; no inline Ghidra script was attempted.

Validation is static: original call instructions, ABI cleanup, producer
stores, current function membership and exact installed/live byte spans.
All 27 recorded spans match the installed image; `verify_report_calls.py`
passed all 91 direct call rows. Indirect dispatch remains separately stated
evidence rather than a direct-target verifier result.
No C++ build, fixture, original instruction execution or gameplay claim is
made by this frontier packet.
