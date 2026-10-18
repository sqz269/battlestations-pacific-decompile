# oct18-tA long-tail reconstruction

Scope: the 82 rows of `codex_tail_A.txt` supplied by cc16-progress on 2026-10-18.
Exact address leases only. No game-host wiring or game launches. New C++ interfaces
under `bsp::oct18_tail_a` are build-tested source reconstructions, not original
register ABI replacements. Object offsets assume MSVC Win32 and valid image-shaped
objects. Descriptive names remain hypotheses.

## Batch 1: game virtual leaves (17 functions)

Evidence is the saved per-address Ghidra decompiler exports; original register
accesses at 006D0D20, 006D0D30 and 006D1F40 were also checked in the assembly.
The existing reviewed BSP names and prior plate comments are retained.

| Address | Ordinary behavior | Original ABI |
|---|---|---|
| 006CA5B0 | return 4 | EAX, RET, no read inputs |
| 006CEBE0 | no-op | RET 4; ECX/stack word ignored |
| 006CEC10 | no-op | RET 0Ch; ECX/three words ignored |
| 006CF3F0 | return 1 | AL, RET; upper EAX unspecified |
| 006CFE30 | no-op | RET 4; ECX/word ignored |
| 006D0D20 | return interior pointer self+15Ch | ECX -> EAX, RET |
| 006D0D30 | load pointer at self+8C8h | ECX -> EAX, RET |
| 006D1640 | no-op | RET, inputs not read |
| 006D1D20 | return category 16 | EAX, RET |
| 006D1D40 | return 1 | AL, RET; upper EAX unspecified |
| 006D1D60 | ignore sync message | RET 4; ECX/word ignored |
| 006D1D70 | return 20 | EAX, RET |
| 006D1DE0 | return 6 | EAX, RET |
| 006D1EE0 | return 0 | EAX, RET |
| 006D1F40 | load byte 00E17BF2 | AL, RET; explicit borrowed global in C++ |
| 006D1F90 | no-op | RET 4; ECX/word ignored |
| 006F7F30 | return `undefined command name` | EAX -> literal 00CFB38C, RET |

No-op parameter meanings remain unknown beyond existing vtable evidence. The
command string has matching contents but a new source address. There is no
binary ABI, original allocation-address, runtime or gameplay claim.

Validation: `scripts/build.ps1` passed MSVC Win32 Release and both existing
CTest checks (reconstructed_math, tool_tests). Per-address naming evidence was
appended through the ledger CLI, applied with the serialized Ghidra annotator
(which verifies project/program, records old annotations and saves), and all 17
affected exports were refreshed. Ignored build/annotation receipts are under
`local/oct18_tA_batch1_build.log` and `local/tail_a_batch1_annotations.log`.

## Held rows

Skipped while cc15-rB owns 00735F30-00758EB0: 00737A50, 00737F30, 007420B0,
00742180, 007422A0, 00748A40, 0074CCA0, 0074E180, 00758140, 00758390.

## Batch 2: ordinary bodies and game thunks (19 functions)

Saved decompilers and assembly establish the three ECX camera pointer loads
0042ABD0 (+388h), 00435390 (+398h), 00519340 (+3ACh); key constructor 00438F80
(ECX key, stack text, EAX self, RET4; store text then byte +4=0); 006E2850
(FLD [ECX+3E0h], ST0, RET); 00644230 (EAX=[00E188D8], RET); and 004C9C90
(RET; ignored ECX message). Borrowed globals are explicit source parameters.

00436690 initializes a fresh 14h checkpoint record (0,-1,-1,empty native string),
ECX->EAX, RET. 00437490 copies three words then initializes/deep-copies its string,
ECX destination/stack source, EAX self, RET4. 0048E1F0 constructs two adjacent
native strings from two headers (RET8), and 0048E500 from another pair (RET4).
Both return self from incoming ECX. These four bodies compose the existing
complete raw-pool string resize/destruction source, retain fresh-header/self-alias
ordering and reread header fields after callbacks. Pair unwind destroys only the
completed first string. C++ scope cleanup models that ownership without claiming
the original FH3/FS exception ABI. String overlap uses memmove semantics, as the
original CRT memcpy supports overlap. No stand-in pool or heap was added.

Disk bytes prove five game tail jumps: 004A6B90 and 004AB030 -> 0048DBD0;
0054ED00 -> 004F71D0; 0060C610 -> 0042A910 -> 00923840; 006EB600 -> 0072D7E0.
All forward ECX and return through the target with no stack arguments. Source
wrappers reuse the existing target implementations. 0054ED00 inherits the target's
typed screen/table API and explicit slot-0 id argument; it does not add raw screen
dispatch. 004A6B90/004AB030 and 006EB600 inherit required target service contracts.

004F1870/004F1890 are ECX entity, RET: parent registration/removal first, then
the world list at CURRENT [entity+30h]+30h. Removal erases only the first payload
match. Their required services name the actual parent/list callees. 00450750 is
ECX dialog owner, one stack output vector, RET4; EDX is scratch. It retains the
two-word checked map iterator, appends node+0Ch keys, then increments, preserving
the order and rereading the sentinel. 00BF6713 is a returning invalid-parameter
binding (verified complete saved body); validation calls preserve the captured
register values and the subsequent reads, without a no-return assumption.
CRT/STL iteration/append remain bindings.
No game hosts were switched to any of these bodies.

Batch 2 validation: `scripts/build.ps1` passed MSVC Win32 Release and both
existing CTests, including after the returning-validation correction. Names and
classifications were appended using the ledger CLI; the locked annotator saved
prior comments and refreshed all 24 affected exports. Receipts are ignored files
`local/oct18_tA_batch2_checked_build.log` and `local/tail_a_batch2_annotations.log`.

## Library/compiler-generated rows (5)

| Address | Classification evidence |
|---|---|
| 00435640 | MSVC scalar deleting destructor: stamp 00CE3784, test flags bit0, free self, return self, RET4. Existing archive writer destructor projection remains unchanged. |
| 0044E4F0 | MSVC STL tree `_Erase` instantiation: nil byte +19h, recurse right, capture left, destroy NativeString key, free node, iterate left. |
| 004512E0 | Same STL tree `_Erase` schedule with nil byte +55h and mapped sequence-entry destructor before key/node release; compiler EH scaffolding. |
| 00550F70 | One JMP 00550EF0; existing target is the compiler-generated vector `_Tidy`: walk 1Ch elements/destructors, free allocation, zero begin/end/capacity. |
| 006DA6B0 | Same STL tree `_Erase` instantiation as 0044E4F0, nil +19h, key cleanup and node free, right recursion/left iteration. |

These get `library_instantiation_out_of_scope` function records, keeping prior
game-facing fragments and existing Ghidra names. They are bindings rather than
new game-body credit. Template identity is inferred from the complete standard
container destruction schedule, native layouts, and existing map/vector users.

## Batch 3: 11 further bodies, two existing bodies reindexed

All eleven saved bodies/assembly were read, and the three thunks were verified
from disk bytes. Source interfaces remain distinct from original register ABIs.

| Address | Behavior and original ABI |
|---|---|
| 00423960 | ECX layer, stack signed dimension, RET4. Extent=12000, multiplier=2, width=double24000/dimension via original FILD/FDIVR64/FSTP32; wrap dimension squared to DWORD, resize/zero current byte vector. Constants are explicit borrowed globals. |
| 00423AF0 | ECX registry, RET. Walk the captured list sentinel, reset layers to dimension10/width2400, resize100/zero. Current sentinel and vector fields remain rechecked after returning handlers. |
| 004C8AA0 | No native arguments, RET. Validate/capture global DWORD range, compose existing whole 004685A0 checked erase, reset sentinel/count then free each node after capturing next. Current canonical CRT heap required. |
| 006C6540 | ECX air-ops block, one ignored stack word, RET4. Remove ready head through container+C0, update observer field+38 with unregister/store/register order, then disable captured plane. |
| 00646C50 | ECX HUD root, RET. Reset+6C/string+70, delete child pointers in vector+104, null each captured slot, then erase range/update end. Fresh vector reads per iteration and returning validation are retained. |
| 00642B80 | ECX marker screen; stack unit/three flag words/kind, RET14h. Iterate CURRENT unit+394 vector, skip nulls, call00642040 with booleanized final input and fixed marker kind2. |
| 006DFE40 | JMP00928860, ECX entity/three stack pointers, RET0Ch via target. Reuse existing whole placement implementation. |
| 006EB7C0 | JMP00879AD0, ECX class, RET. Reuse existing complete raw damageable binder/context. |
| 00767D70 | JMP00765000, ECX message/stack cursor, RET4 via target. Reuse existing typed message codec; typed snapshot/native-layout boundary unchanged. |
| 00759120 | ECX mother-ship class, RET. Existing ship-class base first, last named liftexitpoint/index2 and runwaycenter/index0 groups, copy first point to+808/+814, then FLD32/FSTP32 from+818 to+80C. Counted SBO names and cleanup compose existing raw helpers. |
| 004FF580 | ECX tactical-library screen, RET. Playing/entry-state checks, actual pooled `movies/`+filename strings, destroy filename, play(mode1/value00CEB364/flag0), completion image key004FE7A0, fade current sound+50, release path. Current player/selection publications are reloaded at the native call points. |

The HUD memmove path admits valid original vector headers; current positive tail
moves use ordinary memmove with the same size. No invalid pointer/fault/private
stack or concurrent-memory identity is claimed. New virtual services are required
bindings at named original call sites, never no-op/default implementations. The
existing base model binder's documented source projections remain its boundary.
The fresh source 00759120 can replace the labelled carrier-class host stand-in,
but the owner must decide and perform that switch; this lane changed no host.

Existing complete ordinary source was inspected and gets function records without
new bodies: `construct_native_input_settings_006ab6b0` and
`invoke_native_input_action_deadline_callback_006965a0`. Existing fragments and
their qualification remain. The constructor composes the complete prefix/table
loader with ordinary C++ cleanup, not original FH3/SEH. The callback loads the
borrowed game publication and forwards to the existing complete deadline recorder.

Batch 3 validation: MSVC Win32 `scripts/build.ps1` and both existing CTests passed
(`local/oct18_tA_batch3_build.log`). The locked annotator preserved prior comments,
saved the program and refreshed the affected exports
(`local/tail_a_batch3_annotations.log`). No native differential or game run was made.

The queued band75 lease became active during the follow-up. The exact record and
appended naming evidence in `OCT18_TAIL_A_PENDING.json` have now been applied,
Ghidra saved and the export refreshed. This closes the prior one-record delay.
00759120 host replacement belongs to cc9; this lane leaves the host unchanged.

Four rows already have full function records/source and are left unchanged:
004837D0 (native unit-list erase), 004845A0 (native list payload remove), 004B7EC0
(raw list-header initialization), and 006AA460 (raw input-settings destruction).

## Follow-up batch 1: ten deferred bodies

All ten caller bodies were swept through the complete saved listing, with disk
bytes resolving jump-table, EH and x87 ambiguities. Required original callees
are explicit abstract contracts; they have no defaults or stand-in bodies.
This is caller-body reconstruction, not implementation credit for those callees.
Raw Win32 storage, current singleton references and returning invalid-parameter
handlers remain exposed. No host wiring, game run or original binary/FH3/SEH ABI
compatibility is claimed. Ordinary C++ cleanup models the observed ownership.

| Address | Recovered behavior and material contract |
|---|---|
| 00428800 | ECX center, radius and ignored second word, RET8. Checked tree and three lists; pose-cache refresh and 16 ordered FLD/FSTP copies; strict ordered radius checks consume AABB ST0 before spilling. Random modulo retains empty-vector division failure. |
| 00436C50 | ECX manager and three Lua references, RET0Ch. Full recursive bool/number/string/table copy, repeated key reads and temporary lifetime order. Uses actual native Lua/string helpers plus required missing setters and CRT floor. Ordered x87 number-minus-floor test preserves infinity/NaN behavior; no cycle/depth guard added. |
| 0049C000 | ECX self, four stack words, RET10h, EAX self. Full randomized 76-byte record population. Resize's original 76-byte uninitialized stack fill is an explicit preimage input. Raw random ST0 is consumed directly by CRT conversion or FIMUL; original SSE integer conversion, x87 sine/cosine and float spills retained. D8B418 unwind states destroy vector0048E160 then base00489DD0. |
| 004A5620 | Complete checked global record walk/map publication; composes existing constructor004A50D0. Captured ends survive returning validation; constructor-only failure frees170h allocation. D8BE9C unwind verified. |
| 004DD5B0 | Both initialization branches; file-block/string lifetimes, Lua init/script, three input backend configurations, callbacks, sound queue, safe zone and menu registration. Raw singleton references and required primitives replace no host. |
| 004E4000 | Full shell transition: statistics/title disposal, conditional loading block/config, callback-controlled state3 gate, three menu lifetimes, Lua GC/music/invite, checked achievement lookup/live grant and session flag clearing. Original unknown config word/padding supplied as28-byte stack preimage. D9061C ten EH states swept. |
| 005C7B80 | Complete mission-record assignment: native strings/vectors, interlocked picture owner and virtual deletion, both headers, ordered float copies and tree replacement. EAX self, RET4. |
| 006D0D40 | Literal JMP00958A30; mandatory whole-target service forwards self. The existing partial target implementation is deliberately not bound. No new target-body credit. |
| 006E35A0 | Literal JMP0072E6D0; mandatory whole-target service forwards self. Unverified host projection is not bound. No new target-body credit. |
| 00768530 | ECX stream, RET, EAX message. All233 table entries:214 allocated tags,9 direct-getter tags,10 defaults,215 physical entries,22 allocation cleanup states. Constructors/getters and current read-slot8 are required bindings. Unknown tags use native entryESP-10h preimage; null allocation still dispatches. Multiplayer receive factory is reconstructed but unwired. |

Factory evidence is `reports/oct18_tail_a_factory.json` and the generated route
include. SHA256 and full disk span are recorded. Current Ghidra ownership is only
113 bytes in two ranges;214 raw case call sites have no containing function.
`verify_report_calls.py` therefore reports214 ownership failures, not a pass.
The exact-preimage body-only repair script is prepared in
`tools/ghidra_scripts/RepairOct18FactoryBody.java`, but the server refused the
locked RPC because `GHIDRA_MCP_ALLOW_SCRIPTS` is disabled. The script is unexecuted
and unvalidated. Shared server settings and instruction flow were left alone.
This metadata blocker does not hide the independently audited source body.

Validation: MSVC Win32 Release and both existing CTests pass through
`scripts/build.ps1` (`local/tA_deferred_first10_build.log`). No native differential,
ABI, private-stack, nonlocal Lua error, full fault/concurrency or gameplay proof.
The locked annotator preserves prior naming comments, records old values and
saves; refreshed exports for the factory expose the current ownership limitation.

## Follow-up batch 2: the four large parents

All four complete pseudocode bodies and listings were swept. The call report
`reports/oct18_tail_a_deferred.json` records original disk-body hashes, indirect
call sites, and all499 direct sites. `verify_report_calls.py` checked all499 against
the original live Ghidra program with zero failures. This verifies call evidence,
not execution equivalence. Every required primitive binding must implement the
actual original target and effects; none has a default implementation.

| Address | Caller behavior and contract |
|---|---|
| 004D56E0 | Complete player-drop redistribution: sampled current mission count, untouched eight-slot stack preimage, eligible unit filtering, descending stable score insertion, all-eight-slot team selection, virtual idle fallback, type53h message construction/routing and weight subtraction, final party-slot assignment and list disposal. Raw temporary message bytes are explicit. D8EFB4 states0/1 unwind list004C8070 and message004B5CE0. No player-count clamp or synthesized empty slots. |
| 004E27E0 | Complete raw token walk, checked iterator effects, case-sensitive match sequence, replace/difficulty, inert option groups, SubScene/quote handling, absolute scene-path trimming, profile read/state requests/scene publication and all modes/duel flags. Uses actual native SBO/string bodies with required remaining CRT/STL/game calls. Temp prefix lifetimes extend across the OR chain. Native star-token back edge does not advance and can loop indefinitely. Empty-name cleanup is normal-only as in the original parent. |
| 00648C20 | Full selected/controlled/override unit choice, counted weapon-label lifetimes, ticker/localization/cached label, role-command frame mapping, name ellipsis, ownership/payload icons and health gauges. Directly composes the four audited float getter leaves at939F70/80/C0/B0. Raw health and modifier bindings feed an x87 kernel with the native operand order and spills, including the mid-calculation UCOMISS flags and arithmetic before the zero-count branch. Existing bounded HUD projection is unchanged. |
| 004DE610 | Full world/scene/camera/viewport construction and publication, display/foliage, optional point effects and retained ownership, default/configured lighting/caustics/shores, render resources/shadows, atmosphere/cloud/sky configuration, fog, ocean/weather and all mission subsystem creation/reset/settings/input calls. All45 D8FD28 unwind states reviewed. State0 cleanup is0042B100; camera state4 isB71350; other allocation states use BF65AC. Color values are captured before getter callbacks; effect arguments transfer retained ownership. |

World uses one required register/stack primitive dispatch interface for its83
native callees. It performs the parent control flow, object stores, interlocked
reference operations, string lifetimes and x87 kernels itself. Borrowed image
lookups must return actual cells/data without side effects; raw output-reference
callees must initialize their specified output. `00B107F0` remains a required whole
target, and the existing partial source is not selected. FOV index0..2 is the
ordinary settings domain; no bounds clamp is added. Existing renderer, scene,
front-end and HUD hosts were not switched. Other target-body projections retain
their qualifications and receive no new completion credit.

`scripts/build.ps1` passed MSVC Win32 Release and both CTests after all14 bodies,
including the HUD flag-order correction (`local/tA_deferred_all14_build.log`).
This is source/build and static call-site proof. No native differential, original
register ABI/FH3/SEH/private-frame/full-fault/concurrency or live game proof.

Follow-up disposition: all14 requested caller rows reconstructed, none skipped.
Across the original82-row lane:61 new caller bodies,2 prior bodies reindexed,
5 prior library/compiler classifications,4 already complete rows unchanged and
10 rows originally left to cc15. The00759120 metadata delay is closed; its host
replacement is still cc9's decision. The factory body-ownership repair remains
blocked by the shared server script gate; receive wiring remains absent.

## Complete lane disposition (82 rows)

| Address | Disposition |
|---|---|
| 00423960 | new source body |
| 00423AF0 | new source body |
| 00428800 | follow-up source body with required callee contracts |
| 0042ABD0 | new source body |
| 00435390 | new source body |
| 00435640 | library/compiler |
| 00436690 | new source body |
| 00436C50 | follow-up source body with required callee contracts |
| 00437490 | new source body |
| 00438F80 | new source body |
| 0044E4F0 | library/compiler |
| 00450750 | new source body |
| 004512E0 | library/compiler |
| 004837D0 | already complete; unchanged |
| 004845A0 | already complete; unchanged |
| 0048E1F0 | new source body |
| 0048E500 | new source body |
| 0049C000 | follow-up source body with required callee contracts |
| 004A5620 | follow-up source body with required callee contracts |
| 004A6B90 | new source body |
| 004AB030 | new source body |
| 004B7EC0 | already complete; unchanged |
| 004C8AA0 | new source body |
| 004C9C90 | new source body |
| 004D56E0 | follow-up source body with required callee contracts |
| 004DD5B0 | follow-up source body with required callee contracts |
| 004DE610 | follow-up source body with required callee contracts |
| 004E27E0 | follow-up source body with required callee contracts |
| 004E4000 | follow-up source body with required callee contracts |
| 004F1870 | new source body |
| 004F1890 | new source body |
| 004FF580 | new source body |
| 00519340 | new source body |
| 0054ED00 | new source body |
| 00550F70 | library/compiler |
| 005C7B80 | follow-up source body with required callee contracts |
| 0060C610 | new source body |
| 00642B80 | new source body |
| 00644230 | new source body |
| 00646C50 | new source body |
| 00648C20 | follow-up source body with required callee contracts |
| 006965A0 | existing ordinary body reindexed |
| 006AA460 | already complete; unchanged |
| 006AB6B0 | existing ordinary body reindexed |
| 006C6540 | new source body |
| 006CA5B0 | new source body |
| 006CEBE0 | new source body |
| 006CEC10 | new source body |
| 006CF3F0 | new source body |
| 006CFE30 | new source body |
| 006D0D20 | new source body |
| 006D0D30 | new source body |
| 006D0D40 | follow-up source body with required callee contracts |
| 006D1640 | new source body |
| 006D1D20 | new source body |
| 006D1D40 | new source body |
| 006D1D60 | new source body |
| 006D1D70 | new source body |
| 006D1DE0 | new source body |
| 006D1EE0 | new source body |
| 006D1F40 | new source body |
| 006D1F90 | new source body |
| 006DA6B0 | library/compiler |
| 006DFE40 | new source body |
| 006E2850 | new source body |
| 006E35A0 | follow-up source body with required callee contracts |
| 006EB600 | new source body |
| 006EB7C0 | new source body |
| 006F7F30 | new source body |
| 00737A50 | held by cc15-rB |
| 00737F30 | held by cc15-rB |
| 007420B0 | held by cc15-rB |
| 00742180 | held by cc15-rB |
| 007422A0 | held by cc15-rB |
| 00748A40 | held by cc15-rB |
| 0074CCA0 | held by cc15-rB |
| 0074E180 | held by cc15-rB |
| 00758140 | held by cc15-rB |
| 00758390 | held by cc15-rB |
| 00759120 | new source body |
| 00767D70 | new source body |
| 00768530 | follow-up source body with required callee contracts |
