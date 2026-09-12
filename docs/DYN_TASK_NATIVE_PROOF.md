# Original-byte proof of Dyn task scheduling and lifetime

The bounded fixture closes the original-byte execution gap in
`DYN_TASK_MANAGER.md`. It executes the original constructor, worker adjustment,
CRT entry, worker loop, batch scheduler and destructor with actual Win32
resources. The reconstructed source agrees on six state snapshots, 16 LIFO
callbacks, queue growth/reuse, one-worker replacement and allocation/free order.
No production source or native Ghidra annotations changed in this packet.

| Original body, inclusive | Executed fixture coverage |
| --- | --- |
| `00C37740..00C377ED` | complete successful constructor path with count 1 |
| `00C37690..00C3773A` | initial 0-to-1 creation, 1-to-1 stop/join/recreation, final 1-to-0 shutdown |
| `00C37680..00C3768F` | two actual worker entries, both returning zero |
| `00C375F0..00C37676` | semaphore wake, LIFO pop, virtual task call, nonfinal/final completion decrement, shutdown exit |
| `00C33140..00C3324B` | batches of 4, 7, 2, and 3; queue capacity 4 then 7, reuse, group 0 and completion wait |
| `00C40FF0..00C41069` | successful shutdown, native closures, handle-array/null/queue frees and critical-section deletion |

Each complete span was read through the project-verifying `bsp.py ghidra`
wrappers and matched byte-for-byte against the installed executable. The
fixture inherits the engine packet's image at base `30000000h`, delta
`2FC00000h`. An independent Capstone pass over each complete span changes only
in-image absolute four-byte operands; all resulting bytes match the inherited
image. Relative calls and branches remain unchanged. The runtime compares all
six mapped spans against that verified image before and after native execution;
none of their instructions are hooked or replaced. The executable image,
original/relocated span hashes, every relocation and runtime artifacts are
recorded in `reports/dyn_task_native_proof.json`.

The fixture binds 13 IAT entries directly to real Win32 APIs. External native
`operator_new` and `free` addresses enter allocator bookkeeping backed by
`malloc/free`. The native `_beginthreadex` address enters a bridge that asserts
the six original arguments, including the relocated `00C37680` entry,
manager address and manager+8 output location, then calls the host CRT's real
`_beginthreadex`. The original game's CRT internals are not executed. Both
native workers have real handles, priority 2 and IDs matching manager+8.
Fixture tasks are actual compiled polymorphic objects: their first vtable slot
is callable with ECX=task and their writable group field is at +4. The task
payload merely records execution order and thread identity; no physics-payload
or engine-task claim is made.

Constructor/destructor entry uses their recovered stack arguments and `RET8` /
`RET4` conventions. The adjustment wrapper installs ESI=manager and pushes the
count; the scheduler wrapper installs ESI=manager and EAX=task vector, pushes
the count, and relies on native `RET4`. The fixture preserves the host's ESI
around those calls. The module's earlier caller/body/cleanup evidence is retained
in the proof report's call rows.

The original and source phases each construct one worker, submit 4/7/2 tasks,
replace the worker with a new worker, submit 3 tasks, and destroy the manager.
Their allocation sizes are `[4,16,28,4]`; release allocation IDs are
`[2,1,4,0,3]`, where zero is the unconditional native `free(nullptr)`.
Six snapshots agree across **1316 canonical 32-bit words**: complete manager
storage and allocated queue cells, with 36 critical-section placeholder words
excluded from byte comparison. Allocation pointers, fixture task pointers,
event/semaphore handles and thread IDs are canonicalized. The 24-byte OS
critical-section representation is masked; its real spin count, recursion/owner
idle state and lifecycle are checked separately. Numeric handle values, thread
scheduling, internal OS critical-section bytes and task object payload bytes
are not differential comparisons.

Native destruction closes shutdown and semaphore handles and joins workers but
retains worker handles and all 100 completion events. Each phase separately
closes two retained worker handles and 100 completion events after proving their
native lifetime. The fixture therefore performs **204 extra handle closures**
across the native/source phases, outside the attributed native sequence. Every
tracked task-manager allocation is released, and the mapped image, watchdog
thread and fixture event are also cleaned up.

This is one bounded original-byte differential, not a stress suite or gameplay
validation. It uses only group 0 and quiescent worker replacement. It does not
exercise growth with already pending queue entries, concurrent submitters,
nonzero completion groups, empty batches, failed OS calls, thread creation
failure, allocation failure/overflow, task exceptions or native SEH rollback.
The prior source-only fixture remains the evidence for overlapping groups.
Successful entry into all six functions does not claim every branch executes.

Reproduce in the preserved fixture directory with
`cmd /c local\build_dyn_task_native_probe.cmd` and
`local\dyn_task_native_probe.exe`. The command compiles the actual committed
module, uses MSVC Win32, embeds a manifest, and places the host executable at
`20000000h`. To repeat byte/relocation verification from the preserved image,
run `python local/prepare_dyn_task_native_probe.py local/dyn_task_native_image.bin`
while the configured original Ghidra project is live. The focused build passed
with `/W4 /WX`; the two existing math tests were already passed for the unchanged
module in the implementation packet and were not rerun for these documentation
and fixture-only additions.
