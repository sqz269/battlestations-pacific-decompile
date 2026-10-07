# Pilot-bot active prepend source (CC11)

Packet `cc11_land_active_prepend` adds one complete conditional normal SOURCE facade over
the existing borrowed owner view and REQUIRED allocation/free services. Actual task arena,
parked producer, subsequent task callbacks, cached lifetimes, original ABI and game binding
remain unbound. The source compiles and the extended existing owner fixture passes. Names
are descriptive hypotheses, not recovered symbols.

Target `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`,
base00400000 was verified by the BSP client before each live batch. Worker access was
read/export only. No Ghidra, ledger, configuration, CMake or GameUnitsHost writes occurred.

| Range, end exclusive | Coverage | Original ABI | Source entry |
| --- | --- | --- | --- |
| 00999F50..0099A001 | complete conditional normal active prepend; invalid placement/fault domains excluded | ECX=bot, stack task, RET4 | prepend_native_pilot_bot_active_task_00999f50 |
| 0099ADC7..0099AE2D | paired parked-producer fragment read only; no tick source port | tick ECX=bot, stack dt, RET4; individual factory/register inputs below | unbound dependency |

## Exact prepend order

Active entries/count/capacity are bot+58h/+5Ch/+60h. The source uses existing stable
`NativePilotBotTaskHandle` values and borrowed field references. It does not inspect a task
header, vtable, kind, plane or retained squadron. There are no task slot callbacks, retirement,
hooks, scalar deletion or drain in this routine.

When count equals capacity,00999F5D computes `2 * old_capacity + 1` and00999F61 publishes
capacity BEFORE allocation00999F6A through00BF55BE. Native request size is four bytes per
entry; source host storage size is recorded separately. Successful disjoint live allocation and
representable native/source requests are required, with no fallback or rollback supplied.

00999F78 reads the original incoming stack task (ESP+Ch in this branch);00999F7C writes it
to replacement[0] BEFORE copying the occupied old prefix to replacement[1..count]. It then
frees the old array at00999FB2 when nonnull.00999FBA increments active count, and only
AFTER that00999FBE publishes the replacement base. The new source statements preserve
this count-before-base order. The active-append helper cannot substitute: append grows2*n+2
and publishes its base before its later incoming-entry/count writes.

The saved listing has a CRT continuation gap at00999FB7. Verified live bytes
`83C40483465C01897E58` are ADD ESP,4; count increment; replacement publication. Source
continues after free. No worker listing repair, flow override or annotation was performed.
Primary's separate `reports/pilot_bot_active_flow_repair_cc11.json` repairs only the three
append/retirement call-site gaps; it does not repair this prepend entry.

With spare capacity,00999FD0/D3 first copies the old tail to entries[count]. The unsigned
counter at00999FD8 then walks backward, copying each predecessor to its next slot. It writes
the incoming stack task (ESP+8h) to entries[0] at00999FF7 LAST, then increments count.
No allocation/free/provider callback occurs on this branch.

## Required normal domain

Count must be at most capacity, occupied handles/storage must remain live and disjoint,
and the incoming value must have the real stable mapping required by its caller. Allocation
and free must return normally, with no reentry or changes to owner array fields/occupied
entries. The source copies the incoming handle value across these calls. No profile/layout,
factory, global allocator or no-op service is invented.

The valid spare-capacity branch REQUIRES count>0. For native count0<capacity,00999FD0
reads before base,00999FD8 underflows and the backward loop has no safe empty-list branch.
The source retains its first tail copy and unsigned counter shape and explicitly excludes this
invalid domain. It does not replace it with a safe zero-count shift. Empty count0/capacity0
instead takes the full branch and normally grows to1.

Null allocations, placement-address wrap/null guards, overflow, invalid handles/storage,
private EH/SEH/new-handler/fault behavior, structural mutation/concurrency and original
machine fault/instruction timing are outside the source contract. Source `uintptr_t` handles
and new C++ signatures are not binary task/array layouts or original entry replacements.

## Eight observed caller sites

All eight live direct xrefs were checked with bounded instruction context. Function attribution
was verified independently. Factory internals and callers' actual task/world domains are not
ported here. The uniform prepend inputs are ECX=owner and a pushed task value.

| Site | Containing function | Observed input setup/domain |
| --- | --- | --- |
| 007B6223 | no containing Ghidra function | nonnull result009BC030 is pushed; owner comes through object+4h -> +10h. Exact live CALL exists; body membership unresolved |
| 007B657D | 007B6240 | nonnull result009BC030 is pushed; owner through object+4h -> +10h |
| 009BB47F | 009BB380 | pushes ESI after task-specific profile00D205E0 and bytes+404h/+408h writes; null-allocation path can pass0. These fields are not a shared land-task layout |
| 009CBB19 | 009CB8B0, existing StrafeGoAway Enter annotation | nonnull result009BC030 is pushed; owner through object+4h -> +10h |
| 009CBE27 | 009CBB30, existing StrafeGoAway Tick annotation | nonnull result009BC030 is pushed; owner through object+4h -> +10h |
| 0099A4AD | 0099A4A0, existing InstallTakeoffTask annotation |009CFF40 called with DL=1, result pushed without a null check |
| 0099AE1C | 0099ACD0 | parked-producer result009BBFC0 pushed without a null check; prior count>0 guard admits valid spare branch |
| 0099B113 | 0099ACD0 | bounded later call setup only: head slot30 gate,009CFF40 with DL=0, result pushed without a null check. Unrelated planner/x87 logic unported |

This audit does not prove every other caller reaches prepend only in valid count/allocation/task
domains. Null factory results and actual ownership/lifetime require their separate producer
contracts; no generic task-header interpretation or safe default is derived from these sites.

## Paired parked producer remains external

The relevant tick fragment first requires active count>0, plane+184h nonzero, formation
leader predicate007B8AD0 true, and current head slot38 true. It allocates an8-byte component
through00BF681B at0099ADF4, writes profile00D1F31C at0099AE03 and plane at+4h at0099AE09,
then passes ECX=bot/EDX=component to009BBFC0 at0099AE14. That producer allocates a418h
typeF task whose constructor009BB640 stores the component at task+3F8h. The returned task
is prepended, the ACTUAL resulting owner head is reloaded and receives slot54 at0099AE2B,
then bot+74h is reset. Those callbacks cannot be replaced by a no-op or synchronous promotion.

Successful real producer/component/task domains are required for that subsequent unguarded
slot54 call. Component profile00D1F31C's four entries are recovered separately in
[PILOT_BOT_PARKED_CALLBACK_CC11.md](PILOT_BOT_PARKED_CALLBACK_CC11.md); its planeID
stream methods do not prove observer invalidation or task/component/plane lifetime. This
packet does not construct the component/task, invoke slot54, route BE, or port the broad tick.

## Validation and remaining boundaries

MSVC x86 compiler19.51.36244.0/toolset14.51.36231 compiled the changed source object and
existing owner fixture with `/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`. The fixture linked
with `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF` and executed successfully. No full CMake
build or game run occurred; primary owns main integration/build.

Only `local/cc11_land_owner_probe.cpp` was extended with one prepend scenario: full growth
0->1->3, incoming81 before old80 prefix/free, captured81 despite SOURCE external input slot
changing to90 during allocation with both mappings live, then valid spare shift to82/81/80
without base/capacity changes. There are no hooks/deletes/drains during prepend. Existing
owner/active-retirement checks still pass. Provider allocations, task maps, external input-slot
change and cleanup are SOURCE fixture effects, not native mutation, stack/ABI, factory or
lifetime proof. Invalid zero-count spare capacity is deliberately not executed.

The existing verifier passed13 nonvirtual rows with0 failures:12 exact-site/function checks
and one raw callee-only check without caller containment. Two virtual rows remain explicitly
indirect. Raw007B6223's exact CALL was independently read through live instruction context.
JSON parsing and diff checks passed. Exact receipts accompany
`reports/pilot_bot_active_prepend_cc11.json`. Actual stable task arena, callers' complete
producer/profile/lifetime domains, parked component ownership, cached task+404h squadron and
task+3FCh plane lifetime, deferred BE routing/receipt/reindex, scheduler/observer services,
original ABI and game validation remain external. No live Boolean host or death cleanup is wired.

Primary integration: main be2a8ff8260c004ae60324c42a177db3a47e36c0 Win32 Release passes all3existing CTests.
The independent existing owner fixture and12full+1rawcallee-only native checks pass;
two virtual calls remain excluded. Primary cleared00999FB2 CALL_RETURN and decoded
its ADD ESP4 continuation, saved the project and refreshed the export. Global CRT flags
remain intact; repair receipt: reports/pilot_bot_prepend_flow_repair_cc11.json.
Original ABI and actual producer/arena/lifetime/game binding remain unproved.
