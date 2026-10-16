# Mapped raw Node transform projection — packet132

`NativeNodeMappedRawTransformProfiles` now borrows the genuine, already initialized `GameNativeReadOnlyData` aliases for actual Node `D62C88`, Model `D62DE8` and Group `D634F8`. Reference overloads of the existing raw matrix setter `B6DB10` and bounds notifier `B6DBC0` use that narrow projection. Both APIs share the real matrix/notification implementation; no broad renderer-service implementation, owner or numeric-table copy was added.

Root reviewed the complete final header/CPP and the optimized constructor, mapped functions, private projection and old pointer setter: **PASS**. Strict MSVC Win32 C++17 compilation passed. Required `scripts/build.ps1` rebuilt the already registered translation unit and passed the two existing canonical checks. No new test, behavior probe, raw transform execution or production activation was performed.

## Scope and Original evidence

The isolated worktree retained clean `f01edb0bac0d89746162a635b9a3082f9fc14c07` after Root captured packet129. Only the two existing raw-transform Source files and this document/report changed. Leases cover `B6DB10[DB10,DB59)` and `B6DBC0[DBC0,DBD9)`; no Main merge, CMake/shared Source/ledger/Ghidra changes were needed. The existing Ghidra names remain appropriate because this adds a concrete projection for already reconstructed whole bodies. It recovers no new Native body.

The existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was verified by BSP wrappers. Current metadata still reports `B6DB10` as73B/22 starts and `B6DBC0` as25B/8 starts. Packet129's complete Original bytes/listings and three actual table slices are reused and frozen; no new broad export or annotation occurred. Original executable SHA-256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Native `B6DB10` takes `ECX=actual node`, one stack source pointer, preserves `ESI` and returns `RET4`. Native `B6DBC0` takes `ECX=actual node` and returns or tail-dispatches through current attachment/current slot`3C`. The new C++ overloads use Source fastcall `ECX=actual`, `EDX=context reference`, plus the setter's stack source. These interfaces are not Native ABI replacements. The genuine104B `4134F0` x87 matrix-copy primitive and existing descendant body `B6DA30` remain unchanged.

|Actual profile|current`38`|current`3C`|current`40`|
|---|---|---|---|
|Node`D62C88`|`B6DB10`|`B6DBC0`|`B6DBE0`|
|Model`D62DE8`|`B6DB10`|`B6DBC0`|`B6DBE0`|
|Group`D634F8`|`B6DB10`|`B6DBC0`|`B8E6B0`|

Only the reached`3C` route is admitted here. Group's different`40` target is not generalized to Node world-transform behavior. The setter does not read the incoming receiver's vptr or slot`38`; its caller remains responsible for its own genuine reached`38` dispatch.

## Availability and reached reads

The context constructor calls real `data_at(profile,40h)` for all three aliases, checks their exact Original addresses, then stores its borrowed alias set. It reads no receiver or table payload, owns no mapping, and cannot map a missing band. The real `data_at` implementation only checks section bounds and bands already held by that same service, returning the Original VA itself. An unavailable band fails construction before the alias-set member stores.

At each reached nonnull attachment dispatch, the mapped projection loads that receiver's **current volatile raw`+00`**, requires equality with one of those actual aliases, then reads its **current volatile alias`+3C`** and requires `B6DBC0`. There is no callback, arbitrary getter, partial abstract subclass, copied table, default profile or fixed execution of an Original code address. The context's numeric aliases select the genuine Source body.

The shared setter first runs the actual matrix-copy primitive to receiver`+B0`. Current byte`5C & A` gates the remaining path. It captures current`A0`, clears current`138 & FFFFFFCF`, stores`5C=0`, then performs that captured attachment's reached notification. Only after notification returns does it reload current`34` and invoke the genuine descendant invalidator when nonnull. No profile read was introduced on a path with no attachment dispatch.

The shared notifier clears the current receiver's `138 & FFFFFFC3`, reloads its current`A0`, returns on null, and otherwise validates current vptr/current`3C` before continuing the genuine tail loop. It adds no host recursion, cycle cutoff, vector operation or empty-success fallback. Unsupported family/target errors occur after whatever native mutations have already been consumed. Those are Source provider-boundary failures, not evidence about a Native virtual's exception origin or fault timing.

The original `NativeTracelineRenderServices*` API and its existing broad resolver behavior remain. Pointer and `nullptr` calls keep selecting that overload; existing registered debug callers compiled unchanged. The new reference overload cannot bind `nullptr`. As with any overload addition, an untyped address-of expression must select its intended function type; the original decorated pointer symbols remain present.

## Borrowed lifetime and failure contract

The caller must keep the genuine data service, the full noncopyable context, source/receiver storage and every transitive reached graph alive, valid, serialized and separated from context storage through normal return or failure. Actual node/Model/Group provenance and raw prefix validity are caller proof; a matching numeric vptr alone is not admission authority. The context does not retain the service, graph, creators or pool slots and performs no automatic cleanup.

A caller that can unwind must retain its complete independent provider/frame/creator/diagnostic graph before entering, including after an unsupported reached target leaves flags or matrix bytes changed. Do not rediscover dead fields, replay or automatically retry the operation. This helper supplies no reference-count, forced-scalar/ancestor barrier or cleanup policy. It does not close current child/timed recursive providers or admit a normal GUI Text runtime producer.

## Exact optimized comparison and validation

Before editing, all98 packet129 artifacts were verified and the old header/CPP, current canonical object and three shared build artifacts were copied to `local/node_mapped_transform132_before/`. Old and final strict builds used the same MSVC x86 C++17 `/EHsc /MD /O2 /W4 /WX /fp:strict /FAs` options.

|Existing exported function|Old/new optimized object result|
|---|---|
|descendant invalidator|60B, identical machine bytes and ordered relocations|
|old pointer notifier|147B, identical machine bytes and ordered relocations|
|old pointer matrix setter|285B/83 instructions each; all14 relocation rows identical; machine bytes differ|

For the setter, the optimizer swaps actual/profile register roles `EBX ↔ EDI`. Root independently decoded all83 instructions: after that role swap, only the first seven pure save/argument-capture instructions and epilogue register-pop order differ; the remaining instructions agree. The report does not claim byte or register-ABI equality for that setter. Full old/new COFF code and relocation rows are preserved.

Final optimized listing `local/node_mapped_transform132_strict.asm` contains constructor310–402, mapped notifier814–915, mapped setter926–1133, private current-profile projection2248–2287 and mapped target validation2418–2474. Root reviewed those full ranges plus old pointer setter609–806. Reached loads, no-attachment behavior, matrix/flags/notification/current-child order and the tail loop passed.

The canonical build emitted an `LNK4006` duplicate `spawn_request_id_matches` warning from the untouched `lua_binding_spawn`/`lua_spawn_new` objects. It still completed successfully; the strict owned translation unit had no warning/error. The two existing checks passed; this is build/static evidence, not raw execution, Native FH3/SEH/IAT equivalence or runtime graph validation.

The bounded provider gap from129 is now closed for a genuine supplied supported graph. Whole raw Text cursor `AB8910`, rebuild `ABB1D0`/`ABA8D0`, and whole copy `ABB2C0` remain separate downstream work. Exact Source/objects/logs/reused Original evidence and hashes accompany [the report](../reports/cc12_native_node_mapped_raw_transform_132.json) and `local/node_mapped_transform132_final_artifact_manifest.json`. Only the four owned files are committed; no future build or reuse follows until Root captures them.
