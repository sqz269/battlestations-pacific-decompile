# GOAL15E avoidance manager lifetime

Names are hypotheses. Existing reviewed and library/compiler-generated names,
older typed callback APIs and their evidence remain intact. This packet adds
concrete raw-storage manager construction, destruction, scalar deletion and a
standalone hull-list clear using the existing actual SDK/heap/hull providers.

| Routine | Inclusive Native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00421500` | `00421500-004215AA`, 171 bytes | ECX manager, no stack arguments, RET, EAX same manager; Native FH3 | complete normal construction and recovered vector/list/base unwind operations via current Source EH |
| `0041F980` | `0041F980-0041FA10`, 145 bytes | ECX manager, no stack arguments, RET; Native FH3 | complete normal destruction and recovered list/vector/base unwind operations via current Source EH |
| `00420680` | `00420680-0042069D`, 30 bytes | ECX manager, stacked flags DWORD, RET 4, EAX original pointer | complete scalar-delete instruction schedule |
| `0041E200` | `0041E200-0041E23D`, 62 bytes | ECX actual list header, no stack arguments, RET | complete standalone hull-list clear schedule |

Live analysis used the target-verifying `bsp.py` client: project file
`C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base `00400000`. Whole owned listings,
caller argument setup, both EH maps/actions and consumed cleanup/provider
bodies were read. All four owned listings have zero byte gaps. The bridge
does not expose typed flow/no-return properties; no repair or Ghidra write
was performed. Primary owns shared source registration and integration.

## Actual storage, contexts and virtual identity

The existing `Cc13AvoidZoneManagerNative` is exactly 78h bytes. Its twenty
group slots occupy +08h..+54h, count +58h, body vector +60h/+64h/+68h, list
head +70h and list size +74h. The list header passed to clear is manager+6Ch:
proxy +0, head +4, size +8. Existing typed hull nodes/handles/data and group
storage are reused; no replacement manager, copied owner or layout is made.

`NativeAvoidManagerLifetimeContext` borrows the **addresses** of the actual
`00E188A8` game-publication cell and actual `00E17624` manager-publication cell.
The address bindings/context must remain valid and fixed for the operation;
the values in those cells may change. The complete manager-clear provider
receives the actual game-cell address and reloads its publication as already
established. Base cleanup writes zero to the actual manager cell
unconditionally, including when its current value is another pointer.

Source construct/destruct use `__fastcall(manager, context)`, retaining ECX
receiver and adding EDX context. Source scalar deletion also retains the
stacked flags DWORD and RET 4. The real const
`kNativeAvoidManagerLifetimeSourceVtable` slot 0 points to that concrete scalar.
A typed Source table call provides context in EDX. An original virtual CALL
EDX instead puts its target in EDX and therefore does **not** satisfy this
Source contract. No hidden context recovery, original arbitrary dispatch or
production installation is asserted.

Derived Native `00CE38F4` is represented by that relocated actual Source table.
Final base `00CE3818` remains its numeric Native **profile marker** on the raw
destructed receiver; it is deliberately not a Source-callable table. The
existing actual base scalar provider was inspected and also uses that numeric
profile protocol. No fabricated base slot, extra virtual slot, RTTI or recovered
Native class identity is supplied. With flag 0 the raw receiver survives in
this destructed base-profile state; virtual deletion through the old derived
table is not valid again without reconstruction.

## Normal schedules and real providers

Construction writes the derived Source table, activates base cleanup, then
zeros vector first/last/end in order. It activates vector cleanup before the
head allocation. The concrete `buy_native_hull_list_head_0041bf50` allocates
10h bytes and writes only the two self-links. The constructor stores head,
list size 0, group count 0, then zeros slots 0 through 18. It activates list
cleanup **before** zeroing slot 19 and calling the actual SDK lock creator;
the returned lock is stored at +04h only after return. Proxy +5Ch/+6Ch and
unread sentinel payload words are untouched. No prior allocation is released.
There is no normal manager-publication store here: the Native singleton
factory publishes its constructor result later. That factory/registration
protocol is not reconstructed by this packet.

Destruction writes the derived Source table and activates state 2 before
calling the complete concrete manager clear. That provider deletes groups
through their actual Source tables, resets the group count, and performs the
complete existing cache queue/list operation using the actual publication
cell. State 2 remains active during actual critical-section release. Then
state 1 is selected **before** the standalone hull-list clear and unconditional
head free/null. Vector first is captured and conditionally freed, then
first/last/end are zeroed. State 1 remains active through that tail, the
unconditional manager-cell zero and final base-profile store. Capacity-like
unread fields and proxies are not reset.

Scalar deletion calls the destructor with the original Source context, reads
bit 0 from the low byte of the still-stacked flags **after return**, conditionally
frees the captured object, returns the original pointer even after that free,
and RET 4. Other flags add no behavior.

The standalone clear captures the first node, self-links head->next, reloads
the current head before self-linking previous, compares against current head,
then zeros size without disturbing comparison flags. Each iteration captures
next before the actual embedded-hull destruction and node free, then compares
against a freshly loaded head. The sentinel/proxy remain allocated. It adds
no rollback if hull destruction or free fails. This entry is required on EH
paths even though ordinary manager clear has already emptied the list.

Concrete dependencies are:

- Existing `clear_native_avoid_manager_0041f600` and its full GOAL15D group,
  queue and cache providers.
- `buy_native_hull_list_head_0041bf50` from the independent head packet,
  commit `003cc906cace2c63d26db9668fda23f2e6ae02f1`.
- Existing `create_native_tracked_critical_section_00bd1860` and
  `release_native_tracked_critical_section_0041cc80`, using the genuine 1Ch
  malloc-backed raw tracked section and actual Win32 APIs. Creation adds no
  allocation cleanup if OS initialization fails; release captures slot and
  section, drains positive depth, deletes/frees and nulls only after return.
- Existing repaired typed hull destructor, native `00C37450` at `0041E225`,
  with the actual singleton allocation/free pair, retaining its single captured data pointer
  and post-call field reloads. Nodes/headers/vector/objects use the genuine
  existing `singleton_lifetime_free`, matched to current Source allocations.

These are real services, not success callbacks. Current SDK/CRT/OS/heap,
exception, error-state, synchronization and callee/frame identities remain
qualified against the original private runtime. No original runtime address
is called as executable Source memory.

## Full recovered unwind operations

Both maps have magic `19930522`, maxState 3, no catch blocks/IP map or
exception-spec list, and EH flags 1. Their three entries form 2 -> 1 -> 0 -> -1.

| Owner | Handler / FuncInfo / map | state 0 action | state 1 action | state 2 action |
| --- | --- | --- | --- | --- |
| constructor | `00C5E4FE` / `00D84934` / `00D8491C` | `00C5E4E0` | `00C5E4E8` | `00C5E4F3` |
| destructor | `00C5E30E` / `00D844B8` / `00D844A0` | `00C5E2F0` | `00C5E2F8` | `00C5E303` |

Each action loads the captured manager from `[EBP-10h]`. State 2 adds 6Ch and
tail-transfers to the existing list tidy: standalone clear, unconditional
current-head free, then head null. State 1 adds 5Ch and runs the existing
vector tidy: capture current first, conditional free, then null first/last/end.
State 0 runs the existing base cleanup: unconditionally zero actual E17624,
then write CE3818. These consumed helper/metadata bodies are evidence; this
packet does not claim those unowned entries separately.

The Source scope guard performs precisely those cleanup operations from its
current logical state, lowering the state before each action. It never releases
the critical section on unwind, never clears partial groups or queues bodies
on unwind, and never rolls back fields or counts. Thus a failed head allocation
at state 1 runs vector/base cleanup without reading an unconstructed list;
a later construction failure at state 2 additionally tidies the constructed
list. In destruction, clear/critical failures at state 2 include list tidy;
a normal list-clear/head-free failure after transition to state 1 does not
retry list cleanup. Only vector/base cleanup remains.

The guard is dismissed only after the corresponding complete normal tail.
Successful current C++ cleanup propagates the original current exception;
no catch replaces it. An unexpected exception during noexcept unwind cleanup
uses current C++ termination policy. Fixed local volatile pointers refer only
to the actual concrete entries and retain emitted current EH phases despite
MSVC's naked-body nonthrow inference. They provide no injectable service.
Compiler barriers around memcpy DWORD accesses retain ordered stores and fresh
loads without adding CPU fences or a C++ type-aliasing assumption.

Optimized Source constructor/destructor main bodies are 208/230 bytes and
contain real current `__CxxFrameHandler3` registration. Both emitted unwind
funclets reach the staged guard; its actual list/vector/base operations and
state order were inspected. Source frame, cookie, function pointers, handler,
funclet and exception propagation machinery are not Native FH3/SEH ABI proof.

## Validation and limits

- Strict Win32 `/std:c++17 /EHsc /MD /W4 /WX /permissive- /fp:strict /O2`
  compilation passed; `local/goal15e_manager_lifetime_compile.cmd`.
- `local/goal15e_compare_manager_bytes.py` passed for all 62 list-clear bytes
  and 30 scalar-delete bytes after excluding exactly four verified CALL rel32
  operands. It checks exact lengths, sites/types/actual targets, real Source
  table relocation, current ctor/dtor registration/provider relocations and
  generated guard actions. Optional argv1 selects the registered object.
- Native constructor/destructor are not byte-normalized or claimed equivalent
  to their current Source EH implementations. Complete normal/guard instruction
  schedules were inspected separately.
- All eleven Native direct-call rows passed the live call checker.
- The single real-heap lifecycle fixture passed using this actual constructor/
  SDK lock, populated standalone list clear, signed group insertion/find, populated-group clear, typed Source
  flag-0/flag-1 deletion, both actual publication cells, preserved proxies,
  unconditional foreign-pointer publication reset and bounded current
  `_heapwalk` observations of group/record/lock/head/body frees and retained
  receiver/queue. The ignored peer fixture is
  `local/output/avoid_group_factory_probe.cpp`; its script supports an
  `archive-only` rerun. The passing overlay uses three new Source modules and
  actual existing Release archives with `/MD` and `/MANIFEST:EMBED`. It does
  not exercise exception propagation or prove whole-process leak absence.
  The standalone phase frees a real node and its hull data while restoring
  sentinel self-links/count 0 and retaining sentinel, proxies and actual lock.
- Primary owns final registered build/CTest/object/actual-archive fixture,
  annotation save/export and integration. No injected exception case, Native
  execution, original FH3/SEH/frame/CRT/heap, drop-in ABI, game or gameplay
  equivalence follows from Source compilation, metadata or a normal fixture.
