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

00759120's source is built, but its two ledger files in band00750000 are held by
cc15-rB2. Their local edits were restored to HEAD before committing. The queue is
`oct18-tA-ledger75`; the exact proposed function record and appended naming evidence
are in `docs/OCT18_TAIL_A_PENDING.json`. The coordinator can apply these using the
ledger CLI after that file lease clears. The address-owned Ghidra comment was
already saved/refreshed; its existing function name is unchanged. Thus47 new source
bodies are delivered,46 new source-function ledger records are published here,
and one source-function record is pending. Two existing bodies are separately
reindexed; the five library/compiler records carry no game-body credit.

Four rows already have full function records/source and are left unchanged:
004837D0 (native unit-list erase), 004845A0 (native list payload remove), 004B7EC0
(raw list-header initialization), and 006AA460 (raw input-settings destruction).

## Open rows (14; no completion credit)

These were triaged against existing source and current function boundaries. Large
parents were not fully audited/reconstructed in this lane; no projection is promoted
to a complete body. The saved native bodies remain available for follow-up work.

| Address | Remaining work / evidence boundary |
|---|---|
| 00428800 | Full blast-site traversal, pose-cache refresh and x87 radius checks; existing blast host reaches it through a service. |
| 00436C50 | Complete recursive Lua cross-state copy, x87 numeric coercion and cleanup; current profile implementation is a host projection. |
| 0049C000 | Full randomized point-announcement construction, vector/x87 normalization and unwind; existing source covers only XYZ fragment. |
| 004A5620 | Full runtime publication over checked record vector/map, constructor failure cleanup and returning-validation captures need completing; constructor004A50D0 source exists. This is game code, despite older STL-probable notes. |
| 004D56E0 | Dropped-player score ordering, redistribution and message publication need a complete body/ABI audit. |
| 004DD5B0 | Full initialization/Lua/file-block lifetime and branch contracts; existing input-init fragment is retained. |
| 004DE610 | Whole world construction and parent/subsystem contracts; current fog/header fragments do not cover the large parent. |
| 004E27E0 | Complete option parser and all side effects; large body with45 direct callees. |
| 004E4000 | Full shell-transition order and startup/UI effects; existing executable-effect fragment is retained. |
| 005C7B80 | Whole mission-record string/container/reference-count assignment; existing picture-owner projection is partial. |
| 00648C20 | Existing hud_root_update_rows is explicitly bounded, including non-x87-exact width rules; full457-line parent remains open. |
| 006D0D40 | JMP00958A30 verified, but existing target source explicitly omits the subobject branch. A forwarding wrapper would inherit an incomplete target. |
| 006E35A0 | JMP0072E6D0 verified; target is currently labelled host_reference_unverified, not an accepted complete standalone body. |
| 00768530 | Whole roughly190-case message factory. Many dependencies already have fragment records; factory/read-dispatch/unwind contract still needs its own complete reconstruction. |


## Complete lane disposition (82 rows)

| Address | Disposition |
|---|---|
| 00423960 | new source body |
| 00423AF0 | new source body |
| 00428800 | open; see above |
| 0042ABD0 | new source body |
| 00435390 | new source body |
| 00435640 | library/compiler |
| 00436690 | new source body |
| 00436C50 | open; see above |
| 00437490 | new source body |
| 00438F80 | new source body |
| 0044E4F0 | library/compiler |
| 00450750 | new source body |
| 004512E0 | library/compiler |
| 004837D0 | already complete; unchanged |
| 004845A0 | already complete; unchanged |
| 0048E1F0 | new source body |
| 0048E500 | new source body |
| 0049C000 | open; see above |
| 004A5620 | open; see above |
| 004A6B90 | new source body |
| 004AB030 | new source body |
| 004B7EC0 | already complete; unchanged |
| 004C8AA0 | new source body |
| 004C9C90 | new source body |
| 004D56E0 | open; see above |
| 004DD5B0 | open; see above |
| 004DE610 | open; see above |
| 004E27E0 | open; see above |
| 004E4000 | open; see above |
| 004F1870 | new source body |
| 004F1890 | new source body |
| 004FF580 | new source body |
| 00519340 | new source body |
| 0054ED00 | new source body |
| 00550F70 | library/compiler |
| 005C7B80 | open; see above |
| 0060C610 | new source body |
| 00642B80 | new source body |
| 00644230 | new source body |
| 00646C50 | new source body |
| 00648C20 | open; see above |
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
| 006D0D40 | open; see above |
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
| 006E35A0 | open; see above |
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
| 00768530 | open; see above |
