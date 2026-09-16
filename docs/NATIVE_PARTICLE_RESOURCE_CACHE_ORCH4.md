# Raw particle resource cache owner and lifetime

## Scope and status

Ten complete source bodies cover the actual cache key copier, resource retain/release,
vector destruction, inner and outer owner destruction, scalar deleters, base reset,
and singleton publication. The subsequent r21 packets supply the `0086BA60`
resource loader, complete `00AF4BA0` parser and loaded-resource lifetime
composition. See `NATIVE_PARTICLE_RESOURCE_LOADER_RAW_ORCH4.md`,
`NATIVE_PARTICLE_RESOURCE_PARSER_RAW_ORCH4.md` and
`NATIVE_PARTICLE_LOADED_LIFETIME_ORCH4.md`. The outer `00870DD0` cache
acquisition body and application wiring remain separate work.
Validation results and complete disk/live byte hashes are retained in
`reports/native_particle_resource_cache_orch4.json`.

All recovered names below are descriptive hypotheses. The existing compiler
scalar-deleting-destructor names are retained. Target: saved `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, installed x86 image at the configured Steam path.

## Native entries and layout

| Entry | Complete bytes | Original ABI and behavior |
| --- | ---: | --- |
| `0086A200` | 17 | ECX outer; RET. Clear F87668, then stamp CE3818. |
| `0086BA10` | 80 | ECX ignored; stack output/source/ignored third; EAX output; RET0C. Zero both output words, then copy unless same header. |
| `00871370` | 23 | ECX vector; RET. Resize0, then free current data pointer. |
| `00871400` | 21 | ECX ignored; stack resource; EAX same resource; RET4. Atomic increment LONG+4. |
| `00871420` | 31 | ECX ignored; stack resource; RET4. Atomic decrement LONG+4; zero invokes current resource vslot0 with no stack arguments. |
| `00871480` | 95 | ECX inner; RET. Stamp D0DAF0, clear records, destroy vector. |
| `00871730` | 30 | ECX inner; stack flags; EAX original address; RET4. Destroy, free iff bit0. |
| `00871AE0` | 77 | ECX outer; RET. Destroy inner+4, then reset singleton base. |
| `00871B30` | 30 | ECX outer; stack flags; EAX original address; RET4. Destroy, free iff bit0. |
| `00871BD0` | 192 | No native arguments; EAX outer; RET. Raw shared-manager guarded publication and registration. |

The outer allocation is 18h bytes: outer profile+0, inner profile+4, record vector
data/count/capacity at outer+8/+C/+10, and a zero-initialized word+14. The inner
owner is 14h bytes with its vector at+4. Each record is 2Ch bytes, as recovered by
the separate raw-record packet. Resource retain/release consume the record's
resource pointer, **not** the cache owner or a record.

The original tables are D0DB54 (outer scalar delete 871B30), D0DB40 (inner slots
871730, 86BA10, 86BA60, 871400, 871420), and D0DAF0 (base slots 8716E0, purecall,
purecall, 871400, 871420). These stored DWORDs are native identities, not newly
manufactured callable C++ vtables. Record clearing recognizes the two verified
slot10 identities and composes the genuine release body. Other current tables
remain actual callable Win32 vtable inputs.

## Current-read and exception contracts

`86BA10` overwrites the output before its self-header case; existing output
storage is not released. Raw string resize uses `NativeStringRawPoolContext`.
After resize it tests the current source length and captures current destination
length/source data/destination data in that order. Copy uses the native BF7680
overlap behavior. The ignored third stack argument is explicitly represented.

The getter's fast path returns its captured first publication. The slow path
fetches the actual raw singleton manager, captures its critical section+10,
enters it, and increments the actual tracked word at section+18. It then rechecks
F87668, allocates exactly18h through the source CRT provider, initializes fields
in original order and publishes. A **second manager lookup precedes the current
publication read** used for registration. It decrements and leaves the original
captured section before the final publication read. Registration failure keeps
the publication and allocation; no unregister, retry or rollback is added.

All three relevant FH3 tables have one unwind state and zero try blocks:

| Owner | FuncInfo / unwind map | State0 action |
| --- | --- | --- |
| 871480 | DC7FF4 / DC7FEC | C96030 adjusts saved inner+4 and tail-calls871370. |
| 871AE0 | DC80B8 / DC80B0 | C960D0 tail-calls86A200 on saved outer. |
| 871BD0 | DC80E4 / DC80DC | C960F0 tail-calls411EE0 on the captured eight-byte guard. |

These are true unwind actions. Source guards use `noexcept` destructors so a
second exception during cleanup terminates. In 871480 state0 is disarmed before
normal vector resize/free, exactly as the listing's state=-1 store specifies.
Outer cleanup clears publication even when inner destruction throws. The getter
arms its guard only after entry and depth increment; it stays armed during the
normal depth decrement and Leave. Hardware exceptions and original EH stack-slot
aliasing are outside the C++ exception-domain claim.

The parent integrator independently verified bytes and repaired previously absent
86BA10 function metadata plus the post-free instruction tails of871370 and871480.
The old metadata/comments are retained in the report. No worker Ghidra mutation
was performed. BF6989's library identity was preserved.

## Resource-loader dependency closure

`0086BA60..0086BB75` is278 bytes. It initializes a1Ch TextBuffer, copies and
lowercases a native name, allocates90h, constructs AF45D0 and stamps D0D418,
loads the name through AF5850, parses through AF4BA0, releases the temporary
name, and destroys the TextBuffer through AF5620. The original h8 audit found
these prerequisites missing. Their current source status is:

| Required body | Bytes / evidence | Current status |
| --- | --- | --- |
| AF45D0..AF46D6 | 263; 72 listed instructions | Complete raw constructor: `NATIVE_PARTICLE_RESOURCE_LOADING_ORCH4.md`. |
| AF5850..AF592C | 221; 81 listed instructions | Complete concrete VFS loader: `NATIVE_PARTICLE_TEXT_LOADER_ORCH4.md`; explicit caller-retained failure frame and runtime-service wiring boundary. |
| AF4BA0..AF55AD | 2574; 762 listed instructions,120 CALLs | Complete raw parser: `NATIVE_PARTICLE_RESOURCE_PARSER_RAW_ORCH4.md`; retained recursive invocation and explicit incoming builder state. |
| AF5620..AF5651 | 50;20 listed instructions | Complete raw text-buffer cleanup: `NATIVE_PARTICLE_RESOURCE_LOADING_ORCH4.md`. |

The report preserves every CALL instruction from all four complete byte spans.
The raw Layer lifetime, pooled line/token/suffix helpers, parameter builder,
AFAD00 Layer reader, AF4700 bounds reader and AF40E0/AF9F50 preparation dispatch
are now reconstructed. See `NATIVE_PARTICLE_LAYER_READER_ORCH4.md`,
`NATIVE_PARTICLE_BOUNDS_READER_ORCH4.md`,
`NATIVE_PARTICLE_PARAMETER_BUILDER_RAW_ORCH4.md` and
`NATIVE_PARTICLE_PREPARATION_RAW_ORCH4.md` for their original-code comparisons
and explicit source-interface limits.

The complete AF4BA0 parser composes the AF9FB0 emitter factory, three emitter
parsers, nested B00CE0 particle factory and all five particle parsers. Its strict
Win32 build and three CTests pass; fifteen copied-body comparisons cover
numeric modes, malformed/EOF inputs, Layer and nested emitters. The 86BA60
loader now composes AF5850 and AF4BA0 through the existing VFS providers.
These caller fixtures bridge to genuine source children; they do not prove
every descendant independently or establish full application loading.

Both audited flow defects were repaired and saved under the write lock:
AF5850's AF58FB..AF590C failure tail and AF5620's AF5630..AF5632 stack adjustment.
Full live/PE preflight and prior-state evidence are recorded in
`reports/native_particle_flow_repairs_orch4_h8.json`; affected exports are current.

## Validation boundaries

Complete owned bodies total596 bytes. Every owned body, table, EH map/thunk and
audited loader span was compared byte-for-byte between the installed PE and live
saved-program memory, with SHA256 retained. Source uses actual raw record/string
and singleton providers; there is no host map, replacement Lua owner, or injected
unimplemented loader. Strict-build and call-row results are recorded separately
in the report. Original register ABI, callable profile relocation, exact FH3/CRT
identity, complete cache acquisition and gameplay remain unvalidated. The cache
and string contexts must borrow the same application manager publication.

After integrating the genuine record prerequisite `75211eccbe5`, the plain
`./scripts/build.ps1` passed both existing CTests (`reconstructed_math` and
`tool_tests`). The focused temporary probe linked the resulting `bsp_core.lib`
and passed original/source raw singleton publication and registration, section
depth restoration, atomic wrap and zero-release dispatch, plus normal and
self-alias key copying through the real raw string pool. A source-only exception
case let resource slot0 change the current record count and throw; the actual
record/vector cleanup propagated that exception and the outer base cleared
publication and stamped CE3818. Original EH execution, OOM/fault cases and second
exception termination were not dynamically tested. Probe source, build/run logs,
and artifact hashes are referenced in the report; no permanent test was added.

Additional raw dependencies are now reconstructed: AFBF60 runtime conversion,
the four emitter constructors, seven particle constructors, four emitter
parameter/enum/flag helpers, and the shader/Layer-name helpers. Their evidence
is linked from `NATIVE_PARTICLE_RESOURCE_LOADING_ORCH4.md`. Loaded-resource
release now carries the supplied parameter pool into AF4280/AF46E0, routes
all three derived emitter profiles through their scalar deleters, and uses the
Object loader's resource-container references for model cleanup. Applications
must supply both optional lifetime-domain pointers with the same objects used
for loading. This composition passed a focused cache-to-Object cleanup fixture.
The 00870DD0 acquisition algorithm, application context installation, original
ABI/FH3/SEH, and gameplay remain unvalidated. No runnable particle system is claimed.
