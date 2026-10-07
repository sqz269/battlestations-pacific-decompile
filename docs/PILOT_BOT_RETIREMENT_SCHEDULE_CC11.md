# Pilot-bot retirement schedule (CC11)

Packet `cc11_land_scheduler`: **native schedule audit; no source scheduler binding**.
Target `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian,
base00400000 was verified by the BSP client before each live batch. Worker Ghidra access was
read-only. Names below are hypotheses or existing descriptive annotations.

The source owner already preserves retire/drain/member-cleanup ordering. This audit identifies
where a scheduler must call it and the required callback/producer domains. It creates no source
adapter, fabricated task state, GameUnitsHost wiring or death cleanup.

| Routine/range, end exclusive | Coverage | Original ABI |
| --- | --- | --- |
| 0099ACD0..0099B1A6 | partial: entry/time gates throughAD94; retirement/install/gates/head callbacks AD94..AEC2; plan publication B11D..B1A6. Unrelated task evaluation/planner AEC2..B11D excluded | ECX=bot, stack float dt, RET4 |
| 0099A4C0..0099A5F2 | complete caller assembly; virtual predicates resolved for land only | ECX=bot, RET or tail JMP0099A170 |
| 0099A170..0099A49D | complete caller assembly; actual task factories/providers remain external | ECX=bot, RET; factory calls use recovered register/stack inputs |
| 0099A020..0099A096 | complete bounded dependency: active append/publication | ECX=bot, stack task, RET4 |
| 0099A0A0..0099A168 | complete bounded dependency: retire leading +34h tasks | ECX=bot, RET |
| 00999F50..0099A001 | complete bounded dependency: active prepend/publication | ECX=bot, stack task, RET4 |
| 0099A9B0..0099A9DD;0099A9E0..0099AA18 | complete endpoint-exchange control flow; x87 float transfer in latter not ported | ECX=plan, stack endpoint/other plan, RET4 |

Auxiliary ranges were read only for their called contracts. No new function definitions,
annotations, ledger records or whole unrelated planner reconstruction were made.

## Gates and the data-byte trigger

Tick returns if bot+50h has no plane, or plane bytes+5Dh/+60h/+61h are set, or its current
plane+9D4h squadron has +61h set. That squadron dereference has no null guard. The admitted
tick domain therefore needs a live plane/current squadron; the later factory's null guard does
not admit a null squadron to the tick entry.

0099AD0F..AD68 accumulates dt into+70h using x87 loads, float stores and FCOMIP against
00D1F39C (`EC51B83D`, float bits3DB851EC). An early branch calls0099A9E0 between the two
embedded plan records selected by actual WORD publications00E0B6CC/00F876B8, then returns.
This is an interpolation/observer-copy path, not a task drain. No C++ elapsed-time predicate,
NaN/precision policy or default plan index is substituted. Any future source wrapper starts
only after an externally established eligible-tick gate; x87 eligibility is outside this packet.

After the eligible branch updates+80h,0099AD92 invokes the current squadron's +114h getter.
The actual table cell00D088D4=007ECFD0 returns squadron+348h. Bot **DATA byte+7Ch** is then
tested at0099AD94. It is not a bot vtable offset:0099AD9C/ADA3/ADAA are direct calls. The
producer-established bot profile00D1F348 has its documented methods through+44h; cell+7Ch
contains float data (`9A99D93F`), not an entry for this trigger. This corrects the initial brief's
ambiguous wording without changing the native schedule.

When the byte is set:

1. 0099AD9C calls retire-all00999E40.
2. 0099ADA3 drains00999EE0, including old hooks/scalar deletion.
3. 0099ADAA calls command-task install0099A170.
4. Only afterward0099ADB7 clears+7Ch;0099ADBB stores -1.0 bitsBF800000 to+74h.
5. 0099ADC2 performs the unconditional drain before the later task-finish pass.

Live xrefs list only ADA3/ADC2 as this tick's calls to00999EE0. No end-of-tick drain follows
finished-head retirement. A task retired at0099AE7E therefore survives into a later selected
drain, unless another explicitly invoked native drain path intervenes. Death's DF4 override is
such a separate path; retirement itself is not permission to destroy immediately.

## Interposed work and head identity

Between the unconditional drain and the finish pass is a separate parked intervention:
active count>0, plane+184h set,007B8AD0 says plane+9D8h=0, and head+38h returns true.
It allocates an8-byte predicate component with profile00D1F31C and plane at+4h, creates typeF task through
009BBFC0 at0099AE14, prepends it via00999F50 at0099AE1C, invokes the resulting head+54h
at0099AE2B and resets+74h. The factory allocates418h and constructor009BB640 stores the
component at task+3F8h. Component ownership is a separate dependency.
It must not be replaced by a no-op predicate or synchronous leader promotion.

Primary has independently recovered four profile00D1F31C entries:
00999A60/00999A80/0099A940/0099A980.
The first loads plane from component+4h and tests plane+184h==0, returning a boolean in AL
with upper EAX bits retained. The scalar destructor stamps00D05840, frees via00BF65AC for
flag bit0 and returns original this. The third reads `planeID` through the input
object's virtual+10h and writes the output word to component+4h. The fourth writes through
the output object's virtual+Ch with the cached plane word by value. Assembly flattens four
arguments, read(0,"planeID",4,&outputword) and write(0,"planeID",4,cached-plane-word), with
16-byte callee-pop; decompiler stack recovery is misleading. Literal4 is not established
as a byte count: serializer/type/resolver semantics remain external. Neither stream method
is established observer invalidation. These entries require valid component/plane/stream
domains; they do not establish retained-task or component lifetime. The read output slot
initially contains a stack-record address; a missing-field-to-null fallback is not present.
Primary's saved definitions/live-disk checks and producer audit are recorded in
[PILOT_BOT_PARKED_CALLBACK_CC11.md](PILOT_BOT_PARKED_CALLBACK_CC11.md) and
`reports/pilot_bot_parked_callback_cc11.json`.

006DEEC0(bot,1) at0099AE3F admits plane+1B0h party8 or00927F10's actual party-slot AI/mission
byte. Otherwise tick retires leading+34h tasks through0099A0A0, calls007B8DC0 (real plane+DECh
control bytes), then jumps to final publication. If the party gate passes,0074E230 at0099AE5F
requires control mode+900h in{4,5,6,7} before the finish pass. Other modes skip that pass.

Tick captures old active head EDI at0099AE75 (or0 if empty), calls0099A4C0 at0099AE7E, then
reloads new head EBX at0099AE8C (or0) and compares the identities at0099AE92. Only on change
does it refresh the CURRENT squadron through007ED5D0 when non-null, invoke new head+54h when
non-null, and reset+74h=-1. Same-head, replacement-head and empty-head paths must remain distinct.
This compares handles; it does not resolve the old task via current plane storage or recycle it.

## Finish retirement and queue publications

0099A4C0 first obtains the CURRENT squadron's real command block and0071BE40 singleton token.
For stop token00E08F88 it calls0099A0A0, which retires only leading tasks whose+34h is true.
Neither helper drains or calls+58h/destructors.

The main loop requires a non-null active head, head+38h true and head+34h false. With multiple
active tasks it retires that head without calling+40h. With exactly one, +40h result1 keeps it;
any other result retires it. Retirement preserves the old retired prefix, grows2*n+2, appends
the CURRENT active head pointer, shifts the active prefix, decrements active count and repeats.
If active becomes empty it tail-jumps0099A170; otherwise it returns. It never destroys a task.
Virtual predicates use the captured head; append reloads the current active storage after them.
Arbitrary predicate mutation/reentrancy is not a proven normal caller domain.

For the actual land profile00D1FFA0:

| Slot | Actual entry | Contract/domain |
| --- | --- | --- |
| +34h | raw0099B700 | XOR AL,AL;RET, false |
| +38h | 0099B710 | MOV AL,1;RET, true |
| +40h | 009B3560 | 2 without cached squadron/current command;1 for matching land target+428h with+424h present;else0. Uses old task+404h and actual command/parameter probes |
| +48h | raw007B4130 | XOR EAX,EAX;RET, null plan endpoint |
| +54h | 009B3C60 | real cruise/tuning/leader/retained-squadron effects; no source no-op |
| +58h | 009B33F0 | previously reconstructed old retained-task retirement hook |

Other task profiles' slots34/38/40/48/54 and the parked typeF callback domain are not generalized
from these land constants. Valid stable task identities and cached object lifetimes remain required.

0099A020 appends a factory task to active storage, growing2*n+2 and publishing replacement
after old-array free, then increments count.00999F50 instead prepends: when full, capacity is
**2*n+1**, it puts incoming task first and copies the old prefix behind it; with spare capacity
it shifts right. Reusing the owner's2*n+2 append rule for prepend would change the native contract.
The unused tail entries are not normalized by the retirement loops.

## Actual install inputs and incomplete source dependencies

0099A170 reads bot+50h->plane+9D4h, calls its +114h getter, then0071BE40 and0071EB60. An
initial null command returns. After target resolution, land with no target becomes returntobase;
attackmove calls007EEC50 with squadron/target/1/1, and returntobase calls007F16D0 with squadron
and a real output record. The sixteen factory arms/preconditions were read completely.

The existing `bot_install_command_task_0099a170` is a class/precondition source projection with
required opaque factories. It is not a stable-owner producer binding. Native move/default arms
re-probe0071EB60 and pass that descriptor in EDX. The land arm passes the CAPTURED resolved
target to006BCD20 with DL=1, checks the returned block against the CURRENT squadron through
006C4790, then passes that BLOCK in EDX to009B41C0. Existing `land_group_available(unit)` and
`create_bot_task(unit,command,target)` do not expose these distinct inputs/observations; real
providers cannot infer them from a command class or supply an empty default. Other target arms
use captured target EDX, while move-on-path/retreat/stop factories receive bot alone. Every
nonzero native factory result is appended at0099A490 via0099A020.

At final tick publication, head+48h supplies an endpoint for the actual selected embedded plan.
0099A9B0 at0099B14E unregisters the old endpoint via006952A0, stores plan+14h, then registers
the new endpoint via00694A60. The no-head path unregisters/clears+14h directly. For land,+48h
returns null: this plan edge is not proof that a land task or task+404h is retained alive.
0099A9E0 also exchanges that observer endpoint and transfers+18h through x87; neither a raw
token copy nor a retained task pointer can substitute for the actual observer services.

## Validation and next bounded packet

No C++ or gameplay change was made. The existing live call verifier passed60 direct/tail rows
with0 failures;11 virtual rows are explicitly indirect and not verified by that checker. JSON
parsing and git diff --check passed. Exact call-site rows and live byte continuations accompany
the report; existing owner/BE fixtures do not prove this scheduler. Native source facades and
original ABI/runtime/lifetime binding remain separate statuses. Root's saved owner call-site flow
repair is recorded in `reports/pilot_bot_owner_flow_repair_cc11.json`; it changed only the three
owner call-site overrides, not this tick/retirement analysis. Additional helper free continuations
were read as bytes without worker mutation.

Smallest next source packet: reuse NativePilotBotTaskOwnerView/required allocation/hook services
for active append0099A020, stop-head retire0099A0A0 and finish-head retire0099A4C0. Provide actual
singleton/parameter providers, land/generic task predicates and a real install/arena producer.
A separate eligible-tick prefix0099AD94..ADC7 can preserve reconfigure retire/drain/install/reset
and the unconditional drain, with x87/time eligibility external. A larger slice throughAEC2 must
also preserve parked prepend2*n+1, real callback creation, party/mode gates, old/new identity and
state callbacks; it cannot skip the interposed producer. Request explicit addresses/files for
those source ports before writing. Require actual observer-plan publication before claiming a
full scheduler binding. No cached+404h/plane+3FCh lifetime after base death notification, original
ABI/FH3/fault/reentrant behavior, factory allocator identity or GameUnitsHost integration is proved.
