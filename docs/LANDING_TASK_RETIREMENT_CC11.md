# Landing task retirement audit (cc11)

Packet: `cc11_land_retirement`, 2026-10-06. **Evidence only; no C++ change.**
This audit does not fix SHIP_AI_OPEN_ITEMS 202's leaderless landing loop.

The two live-retirement Boolean clears omit native cleanup, but adding a dequeue
at either clear would give it the wrong lifetime and order. Native retirement
retains the old task until a later bot drain. That drain first runs the old task's
`+58h` hook using its retained squadron, then destructs it using the retained
plane's **current** squadron. Those are different references.

## Target and evidence

All native reads used `python tools/bsp.py show ... --live` or `bsp.py ghidra`
commands. These call `ghidra_export.Client.verify()` before querying: project
`bsp`, program `/battlestationspacific.exe`, `x86:LE:32:default`, image base
`00400000`. The configured project file is `C:/Users/sqz269/bsp.gpr`. The server
was live and the live count matched the 64,696-function snapshot. No Ghidra
mutation, annotation, function creation, snapshot, or reimport was performed.

The whole destructor and dequeue control flow were read. Assembly was used for
ECX/stack argument provenance, exact offsets, virtual dispatch, and ordering.
Names in the following descriptions are hypotheses, not recovered symbols.

| Routine | Exact native body | Original entry/return | Coverage in this audit |
| --- | --- | --- | --- |
| `009B3F50` land-task destructor | `009B3F50-009B3FE3` | task in ECX; plain `RET`, member destructor | complete control flow; base cleanup calls identified, not reconstructed |
| `006C8800` all-deck dequeue | `006C8800-006C8831` | squadron in ECX; plain `RET`, register helper (`__fastcall` single argument) | complete |
| `006C7680` block dequeue | `006C7680-006C77DE` | block in ECX, squadron on stack; `RET 4` at `006C77DC`, `__thiscall` | complete control flow; notification helper consumed as its existing contract |
| `009B34D0` validate land site | `009B34D0-009B3551` | approach in ECX; member helper | complete |
| `0099A4C0` retire finished heads | `0099A4C0-0099A5F1` | bot in ECX; plain `RET` or tail-jump to `0099A170` | complete control flow; virtual eligibility bodies are contracts |
| `00999E40` retire all active tasks | `00999E40-00999EDF` | bot in ECX; member helper | complete decompile; no C++ projection |
| `00999EE0` drain retired tasks | `00999EE0-00999F41` | bot in ECX; plain `RET`, member helper | complete; indirect hooks resolved for the land-task vtable only |
| `0099ACD0` pilot bot tick | `0099ACD0-0099B1A5` | bot in ECX, float dt on stack; `RET 4` | partial: lifetime gates and `0099AD68-0099AEC2`; other planner/task dispatch is outside this packet |
| `009B33F0` land-task `+58h` hook | `009B33F0-009B3433` | task in ECX; plain `RET` or tail-jump to `007EFB60` | complete listing; retained-squadron callee is not reconstructed |
| `007EFB60` retained-squadron hook | `007EFB60-007EFB9A` | squadron in ECX; member helper | complete decompile; `007EEE50` branch remains unread |
| `007BCAA0` plane destroyed hook | `007BCAA0-007BCB27` | plane in ECX; plain `RET`, member hook | complete control flow; detach virtuals' targets/lifetimes are not proven |
| `007F3970` remove plane | `007F3970-007F3A5B` | squadron in ECX, plane and landing byte on stack; `RET 8` | complete control flow; helper internals outside this packet |
| `00926C80` destroy request | `00926C80-00926D8A` | entity in ECX, recurse on stack; `RET 4` | complete decompile of request; subsequent entity/task destruction remains partial |

Body ends above are live Ghidra body limits, which can include operand bytes
after the last instruction address. For example `006C77DC: RET 4` ends at
`006C77DE`. Calling conventions are inferred from the listing; Ghidra's generic
`undefined ...(void)` prototype is not an ABI contract.

## Old task resources and current plane storage

`009B3F50` restores vptrs, then reads **task+3FCh** at `009B3F8C`. If non-null,
it calls `007B8AD0` at `009B3F9E` on that plane. The latter tests only
**plane+9D8h == 0**; it does not test the squadron array's head. If true,
`009B3FAD` loads **plane+9D4h** into ECX and `009B3FB3` calls `006C8800`.
Only afterward does the destructor run approach cleanup `009B2C80` at
`009B3FBF` and base cleanup `007B4030` at `009B3FCE`. The scalar destructor
`009B4100` calls this body at `009B4103`, then frees the task when flag bit 0
is set (`RET 4` at `009B411B`). The destructor therefore does not substitute
the approach's retained squadron for a removed plane's null back pointer.

The drain's preceding `+58h` hook is `009B33F0`, from vtable
`00D1FFA0+58h = 00D1FFF8`. It reads **task+404h** at `009B33F3` and again at
`009B340B`/`009B3422`. This is the approach's squadron reference at
`task+3F8h+0Ch`, not task+3FCh's plane reference. If that retained squadron's
current command (virtual `+174h`) is absent or differs from `land` (`00E08FA0`),
it tail-jumps to `007EFB60` at `009B342D`; otherwise it returns.

`007EFB60` clears retained-squadron bytes `+3B0h` and `+3B8h`. When its live
count is greater than one and its current first plane is landed (`+904h`) in
mode 5 (`+900h`), it calls `007EEE50` with its second plane. That callee has
not been read here. It must be recovered before assuming the flight-leader
test at destruction is unaffected by the hook. The existing host record has
neither these two bytes nor an explicit retained land-task instance.

## Retirement precedes a later drain

`0099A4C0` copies the old active head into **bot+64h**, increments retired
count **+68h** (`0099A5A6-0099A5AA`), shifts the active vector **+58h** and
decrements its count **+5Ch**. When the active vector becomes empty, it
tail-jumps to command-task install `0099A170` at `0099A5EB`. There is no
land destructor/dequeue in this retirement routine.

`00999EE0` drains the retired vector in order. It calls the head task's virtual
`+58h` at `00999EFA`, calls its scalar deleting destructor with flag 1 at
`00999F0B`, then shifts the vector and decrements `+68h`. These calls act on
the retired task object, even when a new task has already been installed.

After the bot's destroyed/remote gates and think interval, `0099ACD0` has:

1. Under bot byte `+7Ch`: retire-all `00999E40` at `0099AD9C`, drain
   `00999EE0` at `0099ADA3`, then install `0099A170` at `0099ADAA`.
2. Unconditionally: another drain at `0099ADC2`.
3. Later: finished-head retirement `0099A4C0` at `0099AE7E`, possibly
   installing a replacement before the current task update.

Consequently a task retired by `0099A4C0` after that drain normally survives
until a later eligible bot think. Immediate dequeue at retirement would
change ordering relative to replacement-task install and landing requests.
Moving cleanup to the end of the current think also disagrees with this
schedule. The `+7Ch` path has a distinct drain-before-install ordering.

## Source paths audited

At the audit baseline, `src/game_hosts_units.cpp` has two explicit
`land_task_installed = false` writes:

- Retreat install, `install_retreat_task_009c9d00`, around line 7472, clears
  the old task Boolean/state immediately. It retains no old task record.
- Invalid land command, `run_land_task_tick_009b3eb0`, around line 29063,
  clears the Boolean/state/site/deck and attack command after `009B34D0`.
  The native approach validator itself only clears approach fields; bot
  head retirement and its later drain are separate operations.

There is also a replacement path: `install_land_task_core_009b41c0` returns
early for the same site/deck, but a different target overwrites the live
approach fields and sets the Boolean true. Its old land task needs a distinct
retired identity as well. A scan limited to the two false assignments misses
this case. The host's known eager command-install schedule is documented in
`SQUADRON_LAND_TASK.md` section 5bh; it is a separate substitution to reconcile.

`pilot_think_and_commit` gates dead planes and processes its think accumulator
before ticking the installed task, but carries no retired-task vector or
native `00999EE0` stage. `landing_squadron_of` resolves the registry's current
membership, corresponding to plane+9D4h. The source helper
`unit_is_flight_leader_007b8ad0` uses the first surviving registry member;
native `007B8AD0` instead reads the plane's stored +9D8h. They cannot be
silently interchanged for destructor coverage.

## Exact dequeue accounting

`006C8800` returns for null ECX. Otherwise it walks the list at `00E19948`
via `+BCh`, skipping wrappers with null `+4h` blocks or blocks with null
owners `+7Ch`. For each eligible block it calls `006C7680` at `006C8820`.

Under the block's `+4h` lock, `006C7680` searches the 14h queue records
at `+98h`, count `+9Ch`. **If no matching squadron queue record exists,
it does nothing else.** On a match:

1. `006C7711` calls `006BF7F0`, unregistering the queue record's observer
   pair and replacing it with the last record, then reducing the count.
2. `006C771D` calls `006C7540`. That helper removes assignment records in
   order by each assignment plane's **current** +9D4h; vector erasure
   shifts later records. It does not use the task's captured membership or
   the registry's departed-member history.
3. Under the `+0Ch` lock, it walks every 58h launch slot. For slot+28h
   equal to the squadron, state 2 first copies +8h to +0Ch; it sets state
   +2Ch to 3 and timer +30h to zero. A set +34h writes 5.0 from `00CE3850`
   and clears the byte. It **retains slot+28h**, unlike `006C65B0` and
   `006C5950`. Each changed slot is published at `006C7790 -> 006BF150`.

This is not merely queue erase. It neither returns stock nor frees a launch
slot's squadron identity. Host observer/replication substitutions must be
explicit if a future binding cannot carry the native notifications.

## Deferred death: proven and still open

`00926C80` marks +60h and queues the entity on the pending **destroyed-hook**
list. It is not an inline task/plane destructor. The neighboring flush
`009273A0` dispatches entity virtual +74h, whose plane-table entry is
`00926390`; that hook tail-dispatches +7Ch, whose plane entry is `007BCAA0`
(`00D19D28+7Ch = 00D19DA4`). This establishes the request-to-destroyed-hook
route, not the later task deletion time.

Within `007BCAA0`, `007BCAEB -> 007F3970` precedes the +DF4h and +6DCh
detach virtuals. `007F3A07` clears this plane's +9D4h before
`007F3A11 -> 007ED260` reindexes survivors. The source registry separately
retains departed-member indices; that history is not the native null pointer.

The task destructor's timing relative to this removal remains unproven.
In particular, the +6DCh virtual at `007BCB1A` is not established as task
deletion. The common `0071C4A0` body only sets bytes +10h/+11h; it is not a
destructor. Resolve the **actual** subobject vtable, pending detach service,
bot destructor, observer invalidation of task+3FCh/task+404h, and entity
kill/delete phase before claiming that a shot-down leader should dequeue.
Do not preserve a stale squadron pointer to make its destructor dequeue;
native `009B3F50` reads the current plane back pointer.

## Minimal concrete next work

First recover `009B33F0 -> 007EFB60 -> 007EEE50` as a bounded retained-hook
packet. Proposed output scope: `src/game_hosts_units.cpp`,
`include/bsp/game_hosts_units.hpp`, `include/bsp/plane_squadron_host.hpp`,
`src/plane_squadron_host.cpp`, `docs/LANDING_TASK_RETAINED_HOOK_CC11.md`,
`reports/landing_task_retained_hook_cc11.json`. The primary agent owns ledger,
CMake, packet, and Ghidra edits. Narrow the squadron files out if the recovered
contract can use existing storage; do not write them without a new lease.

Then represent each installed land task with a stable generation and explicit
retained plane and squadron identities. Move the **old record**, including its
retained squadron, to a per-bot retired FIFO before clearing or replacing the
live task. A new install gets a distinct record. At the eligible native drain
stage, run the old record's +58h hook against its retained squadron, then run
the destructor projection against its retained plane's current +9D8h/+9D4h.
Drain-before-install for +7Ch and drain-before-finished-head-retirement for
0099A4C0 must remain separate. Avoid looking up retained references by raw
`std::vector` pointers, which host registry growth can invalidate.

Before enabling that binding, use one focused host scenario that already
reaches a queued leader: change its command to retreat, and separately change
its land target. Log task generation at retire/drain, both squadron identities,
queue count, assignment count, and launch-slot state/identity. Assert the
native queue-hit guard, last-record replacement, stable assignment erasure,
and retained slot+28h. An unchanged USNOS 202 row is only a reach miss: the
existing section 204 test rows never exercised these live retire sites.

The separate death packet should follow the exact destruction ownership chain
above rather than reuse the live-retirement FIFO as a death workaround.

## Validation and limits

`reports/landing_task_retirement_cc11.json` carries exact direct call sites and
containing function starts for live `verify_report_calls.py` validation. The
worker ran that check and `git diff --check`. No new tests were added. No C++
changed, so an MSVC rebuild or executable smoke run would not validate this
static lifetime finding and was not run. No original-game trace, fixture
execution of this path, runtime repair, binary drop-in ABI compatibility, or
gameplay improvement is claimed.
