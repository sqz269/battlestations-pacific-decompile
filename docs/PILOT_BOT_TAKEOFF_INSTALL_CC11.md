# Pilot-bot takeoff install source (CC11)

Packet `cc11_land_takeoff_install` adds the complete conditional normal0099A4A0 SOURCE
caller over the existing owner/prepend facade and a REQUIRED same-owner takeoff factory.
Actual factory/constructor/profile/arena/world lifetimes and original ABI remain unbound.
Names below are descriptive hypotheses or existing annotations, not recovered symbols.

The BSP client verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, base00400000 before every live batch. Worker access was read/export
only. No Ghidra, ledger, configuration, CMake or GameUnitsHost changes were made.

| Range, end exclusive | Coverage | Original ABI | Source status |
| --- | --- | --- | --- |
| 0099A4A0..0099A4B4 | complete conditional normal caller | ECX=bot, RET | install_native_pilot_bot_takeoff_task_0099a4a0 |
| 009CFF40..009CFFA6 | complete native normal factory body read; private EH not ported | ECX=bot, DL=flag low byte; EAX=task or0, RET | required factory service; no factory C++/arena binding |
| 009CF8E0..009CF9B4 | complete native assembly read for parameter consumption/return and called initialization boundaries | ECX=task, stack owner/flag word, RET8; EAX=task | constructor/profile/state/called services external |

## Wrapper and zero results

The whole wrapper is nine instructions: preserve bot in ESI, set DL=1, call009CFF40,
push returned EAX, restore SAME bot into ECX, call00999F50, restore ESI and return.
There is no command getter, generic command installer, result test, task slot callback,
retirement, hook, destruction, drain, BE message or death operation.

`NativePilotBotTakeoffInstallCalls::create_takeoff_task_009cff40` is pure virtual and receives
this same owner view and the exact low native byte1. A live provider must execute the complete
admitted actual factory/constructor/arena contract. The source caller then invokes existing
prepend UNCONDITIONALLY, including a returned handle0. Neither lossy `AttackCommandHost`
nor the separate current-command producer API is used as a substitute for this takeoff factory.

Handle0 is the copied native NULL task token; it does not denote a live task. It is distinct from
invalid NULL array allocation. A zero factory result can therefore create a NULL active head
and increment active count with valid array storage. Later nonnull-only predicates/hooks/state
consumers require their own actual profile/lifetime domain; this wrapper does not supply a
guard, safe fallback or cleanup for them.

Prepend reads active fields AFTER the factory returns. The post-factory fields must admit
its normal domain: count<=capacity and either count==capacity or0<count<capacity; live
disjoint storage, successful representable array allocation and stable borrowed fields. Native
count0<capacity underflows the spare branch. The wrapper does not inspect/repair it, and the
source must not validate only the pre-factory count or infer the factory preserves every field.
The actual owner mapping remains stable throughout; no reentrant wrapper/array operation or
invalidating field lifetime is admitted. Actual factory effects must leave a valid prepend domain.

## Required factory and byte input

009CFF40 saves owner ECX and raw EDX, allocates4D4h at009CFF61 through00BF681B and tests
the result. Nonnull allocation calls009CF8E0 at009CFF7D with ECX=new task and stacked
owner/raw flag word. Constructor returns the task in EAX. NULL allocation takes009CFF93,
zeros EAX at009CFF98 and returns normally. Task-allocation NULL has an explicit native
return arm; the separate array-allocation service required by prepend still must succeed.

009CF8E0 consumes owner from its first stack argument and tests only the LOW BYTE of the
second at009CF95E. Whole constructor assembly contains no other use of that flag word.
It first checks its retained plane+904h and+900h, otherwise branches on the flag byte, and
has another plane-field/SSE selection branch for flag0. Only parameter consumption is used
by this source API; the state selection, constructor services and plane lifetime are not ported.
The ordinary flag1 call can therefore use a source uint8_t1 without inventing upper-EDX
meaning. An original register/stack/private-EH adapter remains a separate unfinished contract.

The constructor calls0099C6F0 with owner/literalDh,009CF710 with owner+50h plane, selected
embedded state virtual+4h, and009F9980 with task. It stamps raw profiles00D21228/00D2121C/
00D21218 and stores its selected state at task+310h. These are native observations, not source
layout definitions or no-op services. Their actual called behavior, profile/plane/state/callback
contexts, allocation ownership and arena mapping remain required external factory dependencies.

## Callers and domain limits

The wrapper's sole live direct xref is007CA5BA in007CA3F0, existing
`BSP_Plane_HandleTouchdownOrCrash`. Immediately beforehand007CA5B4 loads plane+DF4h
into ECX. Caller assembly has no direct DF4 null check or active-count/capacity observation
before this wrapper. Its other branches and called services do not prove a post-factory array
domain or retained task lifetime. No touchdown/physics source behavior is reconstructed here.

The factory has two observed callers: this wrapper writes DL=1 at0099A4A1; later tick code
writes DL=0 at0099B107 and calls009CFF40 at0099B10B, then unconditionally prepends at
0099B113. That tick path is bounded call evidence only and is not wired into this new facade.
The native low byte is preserved rather than given an invented command/state enum meaning.

## Validation

MSVC x86 compiler19.51.36244.0/toolset14.51.36231 compiled the changed source object and
existing owner fixture with `/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`. The fixture linked
with `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF` and executed successfully. No full CMake
build or game run occurred; primary owns main integration/build.

One case was added to the existing ignored owner fixture: its explicit SOURCE factory asserts
same owner/flag1, records a NULL outcome, and then the caller must prepend that NULL to a
fresh count0/capacity0 owner. The observed sequence is factory1:null ->array growth1, ending
with count1/capacity1, a valid nonnull array and head handle0. An accidental result guard or
factory/prepend reorder fails this case. No later nonnull-only task callback or invalid spare
domain is executed. Existing owner/active-retirement/prepend cases still pass. This is not a
native factory allocation, constructor, arena, ABI, NULL-OOM runtime or gameplay test.

The existing verifier passed9 direct native rows with0 failures; one constructor state virtual
row remains explicitly indirect. JSON parsing and diff checks passed. Exact receipts accompany
`reports/pilot_bot_takeoff_install_cc11.json`. Actual owner/task/plane/profile bindings,
complete constructor services, arena reclamation, cached lifetimes after base death notification,
private EH/SEH/fault/overflow/concurrency, original ABI and gameplay validation remain open.
No generic command installer, head54/tick/BE/death logic is introduced.
