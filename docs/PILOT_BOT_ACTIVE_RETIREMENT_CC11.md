# Pilot-bot active append and retirement source (CC11)

Packet `cc11_land_active_retirement` reconstructs the complete conditional normal caller
control flow of three routines over the existing borrowed owner views. All allocation,
controller, task predicate and COMPLETE command-install services are required. The source
facades compile and pass the focused owner fixture; actual task ownership/lifetime, original
entry/profile ABI and game binding remain unbound. Names are descriptive hypotheses.

The BSP client verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000` before each live batch. Worker Ghidra access
was read/export only. No ledger, configuration, CMake or GameUnitsHost changes were made.

| Native range, end exclusive | Coverage | Original ABI | Source entry |
| --- | --- | --- | --- |
| 0099A020..0099A096 | complete conditional normal active append | ECX=bot; stack task; RET4 | append_native_pilot_bot_active_task_0099a020 |
| 0099A0A0..0099A168 | complete conditional normal leading-slot34 retirement | ECX=bot; RET | retire_native_pilot_bot_leading_tasks_0099a0a0 |
| 0099A4C0..0099A5F2 | complete conditional normal finished-head caller flow; required called services remain external | ECX=bot; RET or tail JMP0099A170 | retire_native_pilot_bot_finished_tasks_0099a4c0 |

## Native storage and publications

Active entries/count/capacity are bot+58h/+5Ch/+60h; retired entries/count/capacity are
bot+64h/+68h/+6Ch. Existing `NativePilotBotTaskOwnerView` exposes references to source
fields, with opaque stable task identities. It is not a raw layout overlay or an owning pointer.
An empty zero-capacity array may have a null base. Occupied handles, all allocated storage
and every captured callback target must remain live, and the arrays must be disjoint.

All three growth blocks use `2 * old_capacity + 2`. They publish the new capacity before
allocation through00BF55BE, request four native bytes per entry, copy only the occupied prefix,
free the old array through00BF6989 when nonnull, then publish the replacement pointer. Source
allocation requests separately carry host bytes based on `sizeof(NativePilotBotTaskHandle)`.
Successful disjoint allocation and representable native/source requests are admitted; no
failure rollback, global allocator substitute or original new-handler/FH3 behavior is supplied.

0099A020 writes its incoming task at active[count], then increments active count. It does not
prepend, retire, call hooks or destroy tasks. Prepend00999F50 has a different `2*n+1` growth
rule and is deliberately not reused. Native placement-null guards are outside this successful
valid-storage domain; the source adds no invented validation/default path.

Both retirement helpers append the current active head to the retained retired prefix,
increment retired count, shift the active prefix left and decrement active count. They leave
the unused tail slot untouched. Neither calls task+58h, scalar deletion or retired drain.
The existing later FIFO drain still runs the hook, reloads head, deletes with flag1, then shifts.

## Callback order and captured identity

0099A0A0 asks only the valid nonnull current head's slot34, as an AL boolean. If false it
returns; if true it retires that head and repeats while active count remains nonzero. Native
has no null-head guard here and does not install a replacement when the list becomes empty.

0099A4C0 first follows the CURRENT bot+50h plane -> plane+9D4h squadron -> virtual+114h
command-controller getter. The actual profile cell00D088D4 is007ECFD0, whose complete body
loads squadron+348h and returns. It then calls0071BE40 and compares the returned interned
identity with stop00E08F88. Stop invokes0099A0A0 before the main finished-head loop.

Live0071BE40 assembly reads controller+30h: mode1 returns+54h, mode2 returns+188h, otherwise
zero. Its final RET is at0071BE5A (end exclusive0071BE5B). Existing typed command getters
can implement that service only from an actual admitted controller projection; a command-class
ordinal or task's cached squadron is not the native singleton/current-controller observation.
The source interface requires both real controller and token providers; the fixture supplies a
source controller mapping and an explicit stop token, not a native getter runtime test.

The loop captures the active head at0099A500. A null captured head exits. It calls slot38,
then slot34, using that SAME captured task even if a callback changes the current head slot.
Slot38 false or slot34 true exits. With multiple active tasks it skips slot40 and retires;
with exactly one, full EAX result1 keeps the head and every other result retires it.

Retirement captures the active array BASE after predicates at0099A548 (0099A0CA in the
leading helper), then reads that base's current head at0099A5A6/0099A128 after any retired
growth. Appending the earlier captured predicate target would lose this distinction. Source
preserves the base capture and later head read. The admitted provider domain excludes base,
count and capacity mutation/reentry. Only controlled current-head slot replacement is allowed
for a predicate, with BOTH captured and replacement mappings kept live throughout the call.
This limited source observation domain is not evidence that native predicates perform such
mutation, nor an ownership solution for the displaced task.

The loop repeats after retirement. When the active count is empty it invokes the required
complete0099A170 installer at the native tail-call boundary; otherwise it returns. The
installer may append through0099A020 as the explicit native producer path. It must preserve
the retired FIFO and may not implement immediate cleanup or synchronous drain.

## Required installer and world domain

`NativePilotBotActiveRetirementCalls` has no default providers. A live implementation must
bind the actual controller and AL/full-EAX task predicates to this same owner, keep every
callback-bearing observation in order, and supply the complete ordinary0099A170 behavior.
The caller's task/profile/world domain must identify which predicates and factory arms it admits.
Unsupported profiles/controllers/world domains are excluded, not answered by invented defaults.

The required installer contract is the actual producer recovered in
[PILOT_BOT_RETIREMENT_SCHEDULE_CC11.md](PILOT_BOT_RETIREMENT_SCHEDULE_CC11.md): initial
current singleton/descriptor/target probes, attackmove and returntobase resolution, all sixteen
factory arms with their real target predicates, descriptor re-probes for default/moveto,
captured target -> air-block resolution/admission for land, real stable task allocation/ownership,
and append of each nonzero result. Initial null command returns; a resolved null command takes
the native default factory. Zero factory result creates no active entry. All services and actual
objects must be valid in the admitted ordinary successful domain.

The existing `AttackCommandHost` class/precondition projection omits distinct descriptor,
captured target, returned air block and current squadron inputs. It is not called by these new
facades and cannot fulfill the complete install promise without first recovering those contracts.
The fixture installer explicitly produces source task100 and appends it; it exercises the caller
boundary only and is not the complete native factory implementation.

## Listing continuations and validation

Saved assembly exports still show CRT call-site gaps. Live bytes at0099A074/0099A115/0099A593
are respectively `83C404897E58`, `83C404897E64`, `83C404897E64`: ADD ESP,4 followed by
replacement pointer publication. Ordinary source control flow continues after free. Worker did
not repair listing gaps, change no-return flags or annotate definitions. Parent's separate owner
flow repair affected00999E40/0099A720, not these three entries.

MSVC x86 compiler19.51.36244.0, toolset14.51.36231 compiled the changed source object and
extended existing owner fixture with `/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`.
The fixture linked with `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF` and executed successfully;
PE machine014C/optional header010B and a nonempty resource directory were inspected. There
was no full CMake build or game run; primary owns main build/integration.

Only the existing source fixture `local/cc11_land_owner_probe.cpp` was extended. One added
scenario checks active growth0->2->6 and prefix publication; stop-leading retirement; multiple
heads skipping40; captured81 predicates/current-head83 retirement; sole82 state1 keep then
state0 retirement; empty-list installer producing100; and delayed FIFO hooks/deletes for
90/80/83/82 while new100 stays active. Existing owner growth/reloaded-drain/disable/member
teardown checks remain. Allocation, controller/task maps, controlled slot replacement and
installer effects are SOURCE-only. They do not prove native reentrancy, allocator identity,
producer completeness, cached pointer lifetime or gameplay behavior.

The existing live verifier passed9 direct/tail rows with0 failures;5 virtual rows remain
explicitly indirect. Exact receipts are in `reports/pilot_bot_active_retirement_cc11.json`.
JSON parsing and diff checks passed. No new broad tests or framework were added.

Native scheduler timing, parked predicate ownership, stable task arena/profile adapters,
actual task+404h squadron/task+3FCh plane lifetime after base death notification, observer-plan
publication, deferred BE sender/readiness/FIFO/receipt/reindex binding, x87 eligibility and
original binary ABI/private EH/game validation remain separate dependencies. No broad tick
port, Boolean GameUnitsHost binding or speculative death cleanup is introduced.

Primary integration: Win32 Release main f1de65be60f050bd78714e6dc6ba8d03041c2e23 passes all3 existing CTests;
independent source-owner fixture and9 direct/tail call rows pass. The primary repaired
the three call-site CALL_RETURN overrides at0099A06F/0099A110/0099A58E and decoded
their ADD ESP4 continuations, saved the project and refreshed all3 exports. Global
CRT flags and non-call alignment gaps were left intact. Receipt: reports/pilot_bot_active_flow_repair_cc11.json.
These checks do not bind the actual producer/arena or establish original ABI/game parity.
