# Retained land task cruise profile source

`cc11_land_cruise_profile` reconstructs the complete normal caller at
`009B3C60` with required actual leader and tuning providers. It also represents
the proved empty base entry `0099B660`. The new source remains unbound to the
native task arena, owner/scheduler, controller storage, and game. Existing
retained landing hook and deferred BE behavior are unchanged.

The source is in `include/bsp/plane_squadron_host.hpp` and
`src/plane_squadron_host.cpp`. The associated machine-readable evidence is
`reports/pilot_bot_land_cruise_cc11.json`. The worker used read-only BSP Ghidra
queries/exports; the BSP client verifies project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and image base `00400000` before live batches.

## Exact native coverage and original interface

| Entry | End exclusive | Coverage | Original interface |
| --- | --- | --- | --- |
| `009B3C60` | `009B3CE6` | Whole 37-instruction normal body through RET at CE5; complete conditional source caller | ECX=land task, plain RET; preserves ESI |
| `0099B660` | `0099B661` | Whole one-byte `C3` RET; live following bytes are `CC` padding | Empty base member entry, plain RET |
| `007B8AD0` | `007B8ADC` | Four-instruction assembly read; required external provider | ECX=plane; XOR EAX/EAX, compare plane+9D8 with zero, SETZ AL, RET |
| `0042E740` | `0042E800` | Complete normal listing read; required external singleton acquisition | EAX=singleton pointer, plain RET; lazy construction/EH/world binding external |

The land profile data xref `00D1FFF4` is `00D1FFA0 + 54h` and selects
`009B3C60`. This is the nonempty task slot +54 caller, separate from +48's
null endpoint and +58's retained pre-destructor hook. The base's empty source
body is justified by actual bytes, not a default/no-op adapter for an unresolved
callee. Source signatures and provider virtual calls are new C++ interfaces;
neither original register ABI nor profile layout is supplied by this packet.

## Normal order and live borrowed fields

The caller invokes the empty base at `009B3C63`, loads task+3FC at C68, and
calls the actual plane leader predicate at C6E. False AL returns before any
tuning call or controller effect. The predicate observes plane+9D8==0;
membership in a host container or a task endpoint is not a substitute.

For a leader, both tuning calls execute. Each returns its own borrowed +514h
float field, **then** the caller reloads the old task's cached squadron+404h.
No first tuning pointer, squadron pointer, or field snapshot is reused for the
second call. Providers can change these slots while executing their actual
contracts. The fixture exercises that capability as a source domain, not an
observed native reentrancy result.

| Channel | Tuning/cache | Block byte | Gate float | Latch byte | Output float |
| --- | --- | --- | --- | --- | --- |
| First | C77 / C7C | squad+38D | squad+380 | squad+3A9 | squad+394 |
| Second | CAE / CB3 | squad+38C | squad+37C | squad+3AA | squad+398 |

On each unblocked channel, `XORPS XMM0,XMM0; COMISS XMM0,[gate]; JBE skip`
permits only the ordered above result for zero versus the gate. With ordinary
non-denormal inputs this is a strictly negative gate. NaN, either zero, and
positive inputs skip; ambient MXCSR denormal handling also remains effective.
COMISS is executed before the latch test, including when the latch is nonzero.
If the comparison advances and the latch is zero, native FLD loads tune+514,
the shared dirty byte squad+3AD is written to 1, and FSTP stores the output.
Every unblocked channel clears its latch, whether the gate or latch prevented
an update. A blocked channel does not compare, clear its latch, or write its
output/dirty byte. The first channel's block does not suppress the second
tuning call; the first latch clear precedes that call.

The separate `PlaneSquadronCruiseProfileView` borrows these nine actual fields.
`LandTaskCruiseProfileView` borrows the live plane+3FC and cached-squadron+404
slots themselves. The existing `LandTaskRetainedHookView` lacks these fields
and leader/tuning providers, so it is preserved rather than overloaded with
unrelated command or endpoint semantics. +404 is never resolved through the
plane's current +9D4 storage. Both task slots retain their native identity;
neither view owns, retains, resolves, nor invalidates their objects.

## Numeric and provider boundaries

Two MSVC Win32 pointer kernels preserve the observed numeric operations.
`cruise_gate_ordered_negative` executes memory COMISS and SETA, the complement
of JBE, without a by-value float argument or C++ relational replacement.
`cruise_load_publish_store` executes memory FLD, MOV byte 1, and memory FSTP
in that order. It does not replace x87 with a raw DWORD copy or an SSE move.
Masked signaling NaNs can therefore quiet on the x87 load/store; source
payload memory is unchanged. MXCSR/x87 control and sticky exception behavior
remain ambient. The implementation supplies no fixed 1400 value, control-word
policy, exception reset, or portable float fallback.

The admitted normal domain requires MSVC Win32/SSE, masked FP exceptions,
no pending unmasked x87 exception, and an available x87 stack slot. The same
load/pop stack balance is preserved. FP instruction/data address registers,
native register clobbers, caller stack layout, exception handlers, and profile
dispatch are not made ABI-identical by these source helpers. Unmasked traps,
stack overflow, SEH/access faults, and asynchronous observation/concurrency
are excluded; their native partial effects are not claimed by a normal-flow
source projection.

All provider methods are required. `game_tuning_0042e740` must execute the
actual acquisition contract, including its lazy allocation/construction and
publication when reached. The normal native body includes allocation at
`0042E79F`, constructor `007E2A20` at E7B6, publication to `00F87440` at E7C4,
and world registration at E7D7. This packet implements none of that singleton
construction/EH context. Returning a default or cached source tuning snapshot
would not establish the required contract.

The actual plane must be live/nonnull for the leader predicate. On the leader
path, each post-tuning cached squadron must be nonnull and its faithfully
mapped fields and returned tuning field must remain valid through that
segment. Native supplies no null guards there. Structural mutation/reentry,
invalid aliases/mappings, and allocation/provider failure behavior remain
outside the admitted domain. Changes across the actual provider calls are
allowed, with fresh views and valid segment lifetimes; arbitrary changes
during a controller segment are not inferred.

Task creation, cached-pointer lifetime after base death notification, observer
invalidation, and safe owner/scheduler binding remain external. Existing owner
retire/drain timing is not changed, and this facade is not wired to Boolean
`GameUnitsHost`, synchronous promotion, task installation, BE receipt, or
death cleanup. See `LANDING_TASK_RETAINED_HOOK_CC11.md`,
`PILOT_BOT_RETIREMENT_SCHEDULE_CC11.md`, and
`PILOT_BOT_TASK_OWNER_SOURCE_CC11.md` for those separate contracts.

## Focused validation

MSVC 19.51.36244.0 x86 compiled the actual changed squadron-host TU with
`/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`. The existing ignored hook probe
was extended and linked with that object and the existing main
`build/win32/Release/bsp_core.lib`, `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF`.
The prior retired-task/clear-before-deferred-BE checks remain present.

The focused cruise scenario checks no tuning for a nonleader; new tuning and
cached squadron during each provider call; first reset before the second
call; blocked-first still calling tuning twice; blocked-second preserving
its latch; and suppression by a nonzero latch. A signaling tuning NaN
`7F801234` stores as quiet `7FC01234` without changing its source bits.
The fixture saves/restores its numeric environment, uses masked round-up
controls, and checks unchanged x87 control/TOP and MXCSR after that store.
A quiet NaN gate preserves its bits, skips the update, clears the unblocked
latch, and sets MXCSR invalid via COMISS. Result:

```text
PASS retained hook; fresh cruise tuning/cache; blocked/reset order; x87 quieting/control/stack; COMISS unordered
```

Object disassembly confirms `XORPS/COMISS/SETA` and adjacent
`FLD -> MOV byte 1 -> FSTP`. This proves the dirty-byte/store order in the
compiled source; no concurrent fault observer was added to the fixture.
Live `verify_report_calls.py` passes all four direct call rows. JSON parsing
and whitespace checks pass. This is compilation/source-fixture/call-site
evidence, not original entry execution, native singleton construction,
observer/lifetime proof, drop-in ABI compatibility, or gameplay validation.
The primary owns the full main Win32 build/integration.
