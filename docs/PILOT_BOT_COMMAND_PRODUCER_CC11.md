# Pilot-bot command task producer (CC11)

`0099A170..0099A49D` has a complete conditional normal caller reconstruction in
`install_native_pilot_bot_command_task_0099a170`, with required providers in
`native_pilot_bot_command_producer.hpp`. The original entry takes ECX=bot and
ends in plain RET. The new source API is not its binary ABI or a task arena.
Every descriptive name is a hypothesis, not a recovered symbol.

The old `AttackCommandHost` projection conflates descriptors, captured targets,
and the land air block/current squadron. Its behavior is retained for its current
typed callers; its header now states that it cannot supply this complete producer.
No GameUnitsHost adapter, task layout, global or default callback is invented.

## Captures and fresh observations

Initial bot+50h -> plane+9D4h reads at0099A176/179 obtain a squadron. A null
squadron exits. Its virtual114 call at0099A191 returns the captured controller
(EBP), followed by actual0071BE40 at0099A197. An initial null singleton exits;
it does not enter the default-task arm. Descriptor0071EB60 at0099A1A9 and
target resolver00521EA0 at0099A1B0 produce the captured target (ESI).

Land00E08FA0 with a null target rewrites to returntobase00E08F98. Attackmove
00E08F78 with a nonnull target reloads the CURRENT squadron and calls007EEC50
at0099A1E2 with ECX=squadron and stack captured target/1/1. Returntobase also
reloads current squadron and calls007F16D0 at0099A201 with a real1Ch output
record; this caller consumes only output word0 as the resolved singleton.
The original captured target remains distinct. The required source service
must execute the complete admitted return-to-base behavior and its side effects;
the returned-token signature does not claim the native output-record ABI.

Resolved null chooses009C3C40, while moveto chooses009C3BE0. Each arm re-probes
0071EB60 on the SAME captured controller, at0099A210 or0099A22D, and passes
that fresh descriptor in EDX. Initial and resolved null are separate paths.

Land uses CAPTURED target as ECX to006BCD20 at0099A3E9 with DL=1. The flag's
meaning is not proved here. A null block exits. It then reloads CURRENT squadron
at0099A3F8/3FB and passes that squadron on the stack to006C4790 at0099A404,
with ECX=returned block. AL false exits.009B41C0 at0099A415 receives the BLOCK
in EDX, not the initial target. This distinction drove the focused source fixture.

## Factory arms

| Singleton | Required factory | Argument in EDX | Gate |
|---|---|---|---|
| Resolved null |009C3C40|Fresh descriptor|None|
| Moveto00E08F68|009C3BE0|Fresh descriptor|None|
| Moveonpath00E08F80|009BDBB0|No supplied argument|None|
| Divebomb00E08F20|009C8C70|Captured target|Nonnull, virtual5C kind2 true|
| Levelbomb00E08F28|009B9030|Captured target|Nonnull, virtual5C kind2 true|
| Dropkamikaze00E08F30|009AEBE0|Captured target|Nonnull, virtual5C kind2 true|
| Torpedo00E08F18|009D4E30|Captured target|Nonnull,009229F0 with EDX6 true|
| Strafe00E08F40|009CD300|Captured target|Nonnull only|
| Rocket00E08F48|007B7FD0|Captured target|Nonnull, kind2 true then kind18h false|
| Kamikaze00E08F50|009AF720|Captured target|Nonnull, virtual5C kind2 true|
| Dogfight00E08F58|009AB570|Captured target|Nonnull, virtual5C kind2 true|
| Land00E08FA0|009B41C0|Returned air block|Block/current-squadron gate above|
| Closetoship00E08FA8|009A2F40|Captured target|Nonnull,009229F0 with EDX6 true|
| Depthcharge00E08F38|009A6970|Captured target|Nonnull, virtual5C kind8 true|
| Retreat00E08F90|009CA2B0|No supplied argument|None|
| Stop00E08F88|009BADB0|No supplied argument|None|

Every factory has ECX=same bot. Unsupported tokens return without a factory.
A zero factory result returns without append. A nonzero result is appended to
the same owner's active array through the reviewed0099A020 facade, at the
native0099A490 boundary. There is no task hook, deletion, retired drain or head54
call in this producer. Later scheduler/retirement behavior remains separate.

## Source domain and validation

Every provider is pure virtual and required. It must bind actual same-bot
world/controller/descriptor/target/block observations and all admitted factory
arms. Nonzero task handles need a stable live mapping, actual arena ownership
and valid profiles through active/retired callbacks. No allocator/task arena,
serializer, target-class substitute or runtime adapter is supplied.

Providers must return normally and cannot reenter this operation, mutate owner
array structure or invalidate captured mappings. Current squadron and descriptor
observations may change between callbacks: each fresh native read is retained,
while the captured controller and target remain fixed. Initial null squadron
and command are admitted early exits; later callees require their actual valid
receiver domains. Successful disjoint representable2*n+2 allocation is required
for append. Faults, overflow, private EH and structural reentrancy are excluded.

One ignored focused probe compiles the ACTUAL production attack-command and
owner sources under MSVC Win32 `/O2 /W4 /WX /Gy /Gw`, and links with
`/MANIFEST:EMBED`. It checks distinct target400/block500/current-squadron101
after initial100, stable controller200, fresh descriptor301 after300, resolved
null after attackmove, rewritten null-target land through return-to-base/moveto,
initial null without descriptor dispatch, zero-result no append, and retained
retired task900 without hook/deletion/drain. The actual production append path
allocates the fixture active array. Unused world/predicate/factory methods reject
their fixture domain; this is not a complete native provider or original execution.

Probe compilation and execution pass. Native call-site evidence comes from the
complete saved assembly plus live verifier rows in the report; indirect virtual
calls are explicitly excluded from direct-call counts. No broad tracked tests,
mode/policy changes or game run are introduced. Primary main build receipt is
recorded separately after integration. Original callee ABI, private EH, actual
arena/world lifetimes, arbitrary task profiles and gameplay parity remain unbound.
