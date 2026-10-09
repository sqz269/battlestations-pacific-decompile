# Type10/11 complete helper TEXT proposal (CC12)

This packet supplies the complete proposed Win32 probe, Root recipe, primary gate and two independent readers as **unexecuted TEXT, Source credit 0**. It does not qualify the current constructors. No proposed helper was imported or executed; no compiler, linker, Native byte query, provider or target ran. The only committed changes are this document and its report.

The sealed local draft is `J:/PROG/battlestations-pacific-decompile-cc12_type10_type11_complete_helper_TEXT_draft/local/t1011draft`. Its `artifact_manifest.json` seals every file recursively except exactly the two root names `artifact_manifest.json` and `receipt.json`. The draft receipt gives the exact count and manifest hash. Earlier static audit output is retained as history; **`TEXT_static_audit_r02.json` is the selected final static audit**. Its checks are AST/literal/layout arithmetic and strict input bookends, not machine-code or runtime checks.

The proposed fresh Root family is **`J:/PROG/battlestations-pacific-decompile/local/type1011p1`**, confirmed absent at the final static audit. The worker did not create it. All future phases fail on changed selected inputs; preparation refuses an existing family and later phases refuse a sealed family. No accepted family, object, stage or prior process is replayed.

## Selected contracts

The current Main Source definitions, headers and complete quoted-header closure are frozen: **12 files**, including both complete constructor TUs and the complete canonical `singleton_lifetime.cpp` TU. The six core files agree with the selected Source-only case worksheet. The exact **28-file** worksheet is frozen in full, including its manifest SHA256 `791bae6e107512ddb0db06dd06d444b05e5b823666f2220b32023cd988e20457`. Current case/readiness metadata is frozen as generation context; historical readiness absence statements are not treated as current Source absence.

| Constructor | Whole Native extent | Instructions | Native SHA256 | The only rebound operand offsets |
|---|---|---:|---|---|
| Type10 | `[008EF3E0,008EF454)`,116 bytes | 36 | `00c1508701d10a6508b556eff0feb4737f51a4ec289059dce77a796cc571919b` | `[51,55)`, `[66,70)` |
| Type11 | `[008EF460,008EF4D7)`,119 bytes | 37 | `fdb6bb6f351939abd63f85028eea15df069113701aba2de727bb9a16393978da` | `[54,58)`, `[69,73)` |

ECX carries fresh/unowned56-byte root storage, EDX is the unused register formal, and count/input/full flag are three stack DWORDs at target-entry `T+4/+8/+C`. Both return root in EAX with RET0C. Type10 copies4*count bytes; Type11 copies12*count. The low flag byte alone selects retention or copy. The full56-byte result has29 written and27 preserved bytes, including all owner+30 bytes; marker+2C equals1 in both paths and proves no ownership policy.

All eight worksheet cases appear literally and identically in the proposed probe and independent recorded reader. The selected static audit compares these literals with the frozen worksheet. Counts4/1 and2/1, all distinct high flag bits, low bytes00/00/80/01, poisons and complete opaque inputs are retained. Interior NULs do not shorten a payload.

## Proposed execution and capture

The proposal uses three fresh lanes for every case: raw Source, raw rebound Native, and ordinary typed Source. This means24 separate56-byte root allocations,12 copied children and24 calls. All roots, input arrays and copied children remain live through all observations and disjointness checks. Cleanup frees each copied child once through the same current canonical free, then each root once. Borrowed inputs are never child-freed. Only guard bytes inside the40-byte input arrays are claimed; there are no adjacent heap/root guard claims.

The naked raw caller saves the ordinary caller state, pushes all three full arguments, seeds GPRs and DF0 flags, and takes actual PUSHFD/PUSHAD snapshots before and after the target. It also copies the actual argument DWORDs from the live stack. The pre-snapshot PUSHAD saved-ESP slot is exactly target-entry `T`; the post saved-ESP plus4 must equal `T+16`. The snapshots are restored around capture bookkeeping, so REP MOVSD does not replace the target's observed flags. The ordinary lane makes no raw register/flag assertion.

Retain checks ECX=root, EDX=input and mask8D5=44, including AF0. Copy checks all six arithmetic flags from the observed `(uint32(T-24)+16)`, yielding `uint32(T-8)`; ECX/EDX provider residuals remain unconstrained. The proposal records all GPRs and flags but makes no ES/FPU/MXCSR promise. Actual thread stack limits and simultaneous live extents establish the proposed disjointness domain.

The packed observation format is40-byte header +24*352-byte records +8*580-byte provider rows +2*139-byte rebound-code rows = **13,406 bytes**. It includes complete before/after roots, input arrays, full copied bytes, actual pointers, target addresses, all raw GPR/flags and stack arguments. Rebound Native buffers are fresh RW allocations changed to RX; only the two E8 rel32 operands bind the corresponding emitted current adapter and genuine memcpy thunk. Both actual rebound bodies are serialized and checked again while live.

Loaded malloc/free/memcpy/_callnewh identity is recorded before the work and after all canonical cleanup: resolved address, module base, canonical file path, file ID/volume/size and32 actual entry bytes. The independent reader compares the selected current Win32 provider's export address, file identity and relocated entry bytes. This is bounded entry/provider identity evidence, not a provider-wide implementation proof. Actual provider paths and hashes are future Root inputs, never invented worker observations.

## Full gate and independent readers

The recipe builds **four fresh TUs** (`type10`, `type11`, `canonical`, `probe`) with MSVC Win32, `/MD /O2 /W4 /WX /EHsc /Gy /GL-`. It records explicit compile/link plans, all searched header/library pins, actual consumed includes and frozen input copies. Linking uses an embedded asInvoker manifest, fixed image base52000000, `/OPT:REF /OPT:NOICF` and an explicit map. Neither compiler nor linker has run in this packet.

The primary gate and external code reader have separate raw COFF parsers. Both consume the whole primary/auxiliary symbol tables, section records, all relocation records and overflow relocation headers. Weak fallback resolution uses the complete TagIndex field, follows primary-symbol fallback chains, rejects cycles, and separately proves actual retained mode1/3 fallback definitions, map VAs, full bodies and every relocation. Mode2, unsupported formats or unknown retained relocations stop; no mode3 occurrence is assumed.

All retained fresh executable sections, whole main, raw caller, ordinary public callers, constructors, adapters and canonical bodies are covered. Associative retained COMDAT data requires an established base. Every retained section is compared with the linked image after all actual relocation addends. Root must provide exact fresh support-helper extents and evidence in `linked_plan.json`, following `linked_plan.schema.json`; next-symbol distances are not accepted as an automatic body proof. The plan covers every actually retained normal/cold/import/cookie/stack helper, including relevant std/delete/free paths. Old Header13 helper sizes are not imported as obligations.

Both CFG readers reject unknown edges, instruction-interior destinations and internal unreachable bytes. Only an entire **maximal trailing unreachable NOP/INT3 suffix** is admitted. Source constructors retain their complete COFF extent; any bytes beyond the116/119 Native extent must be exactly that independently proven suffix. Register-indirect imports require all-predecessor IAT facts, including partial-register invalidation. The one raw capture call is explicitly bounded to its reviewed global slot and two Source/fresh Native targets. The sole unexpanded local frontier is the actual cookie helper's jump to `___report_gsfailure`/`__report_gsfailure`. Named external nonreturn imports remain external boundaries, with complete local thunk bytes checked; EH/OOM is not exercised or qualified.

The independent code reader imports neither primary recipe nor gate, and reads primary coverage only after reconstructing its own objects/map/relocations/CFG/Native checks. It independently parses the packed gate. The separate recorded reader imports neither helper, starts from the actual binary file, recomputes all case bytes/flags/stack/pointer/lifetime expectations, checks every accepted linked input pin and verifies the same two Root receipts used for launch. Neither reader can spawn a target or other process.

## Dependencies, receipts and Root order

The current query closure is the frozen repository `tools/ghidra_export.py` standard-library HTTP Client plus its target configuration. Its current `read_memory` response uses `hex`; project `bsp` and `/battlestationspacific.exe` are verified before/after every future query batch. There is no ghidra_bridge/jfx_bridge requirement. The actual installed analysis TEXT closure is **42 Python files**: capstone37, pefile1, ordlookup4. pefile's required ordlookup import was inspected. The proposed runtime pins package membership/DLLs and the interpreter/standard-library/toolchain closure before import. LIBCAPSTONE_PATH and implicit Python/compiler/link options are rejected; packaged capstone.dll is required. No selected dependency was imported by the worker.

All meaningful checks use explicit exceptions, not Python assertions. Every helper import, compiler/linker/reader gateway and target launch checks unoptimized `python -B -E`, selected inputs and the Root text receipt. The recipe has one restricted tool-process gateway and one target-process call; exact allowed command lists prevent another executable route. Compile/link/gate/reader/Native phases and the target have exclusive attempt records. A failed target cannot be retried in that family.

Root's intended order is:

1. Independently inspect the complete sealed TEXT candidate. Execute only its `prepare` metadata phase into the absent family, then staged `recipe.py review` to produce the exact text selection candidate. Root authors **`Root_text_review_receipt.json`**; the recipe never creates an acceptance receipt.
2. Run staged `native` and `build`. Inspect the four actual objects, full map and PE, then author fresh `linked_plan.json` with exact support extents/section bases/capture site. Run `gate`, including the external independent code reader. An unknown format, helper, edge or extent stops for a new reviewed candidate/family.
3. Select exact actual current provider disk inputs, run `prelaunch`, and independently review the full actual linked artifacts and independent results. Root authors **`Root_linked_manual_gate_receipt.json`** with ordered keys `executable, gate, coverage, objects, provenance, launch_ast`.
4. Run `launch` once, then `recorded` for the independent decoder and post Native/provider/input bookends. `seal` includes all utilities, outputs, attempts, failures, selected inputs and receipts. Source publication remains a separate Root decision.

The launch AST hash is recorded by the selected static audit. The same exact receipt basename and ordered six keys are checked on both sides of the exclusive target-attempt creation. Final Root-family sealing excludes only its two exact root manifest/receipt names. This draft has no Root acceptance receipt and no result from those future phases.

This proposal remains limited to positive bounded counts, nonwrapping readable/disjoint spans, fresh raw storage, DF0 and successful current allocation/copy/free. Zero/overflow/alias/reentry/failure, naked-frame unwinding, Native private CRT/EH, phase dispatch, owner/class/recursive lifetime, complete clone/publication and game execution remain unqualified. AST checks and metadata sealing do not establish emitted code, physical ABI compatibility or a passing fixture.
