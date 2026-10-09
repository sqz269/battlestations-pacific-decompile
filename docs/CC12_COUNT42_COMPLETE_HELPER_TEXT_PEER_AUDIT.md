# COUNT42 complete-helper TEXT peer audit

The chosen draft has no concrete writer/reader or dispatch contradiction found in this independent TEXT review. This is a Source0 audit, supplies no Root approval, and does not authorize a compiler, helper, or target process. Root must still select actual dependencies, inspect fresh emitted code, author both receipts, and validate one process.

The baseline is `e995b86aef0e4e503ad1a45c3e93ff513ff56afa`. The selected `count42draft` contains exactly69 files and its manifest is `0c1d5722342f774c22d156f32eb3de4e629b536fa6cb6c2f21bce6e269f8c183`. All69 bytes, the separately identified17 case inputs, and current Main/worktree Source/header bytes are frozen under `local/count42peer`. Metadata is a generation snapshot. The future Main `local/count42complete01` was absent and was neither created nor opened.

## Leaf and selected cases

The current whole Source literal42 and probe Native literal42 match SHA `e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7`. The padded fastcall leaf receives ECX and unused EDX, has no stack argument, writes nothing, calls nothing, and uses no providers. It reads little-endian DWORD tag+4 and only on tags9/10/11 reads unsigned size+0x24. Default does not read size. There are no invented positivity/divisibility guards or numeric-payload semantics.

|Case|Tag|Size bits|EAX|Raw EDX|Defined mask/bits|
|---|---|---|---|---|---|
|0|9|00000007|00000001|D00DFEED|C5/01|
|1|10|FFFFFFFC|3FFFFFFF|D00DFEED|C5/04|
|2|11|FFFFFFFF|15555555|AAAAAAAA|C5/04|
|3|FFFFFFFF|A76CD291 unread poison|00000000|D00DFEED|8C5/44|

These are algebraic expectations. SHR2/3 AF/OF and default XOR AF are undefined and excluded. ECX/nonvolatiles/DF/ES are unchanged by the actual literal leaf; EDX retains MUL-high only on tag11. The same four proposed cases have distinct fill bytes A7/5C/D2/69; the full expected40-byte inputs are recorded once in the report.

## Wire agreement and assertion boundaries

The packed probe ([probe.cpp.txt lines6..12,120..140](../local/count42peer/selected69/template/probe.cpp.txt)) writes the exact little-endian header `[32344343,1,16,192]`, then four lanes per case: raw Native, raw Source, ordinary Native, ordinary Source. Raw DF is1 and ordinary DF0. Sixteen records192 plus header16 equals3088 bytes, with no JSON runtime or trailing bytes. Each record has identity32, input-before40 at32, input-after40 at72, and two actual Image40 processor captures at112/152. Each image is ten DWORDs EDI/ESI/EBP/ESP/EBX/EDX/ECX/EAX/EFLAGS/zero-extended ES. The Source assembler captures processor state rather than filling expectations; actual emitted capture correctness remains Root review.

[recorded_reader.py.txt lines50..83](../local/count42peer/selected69/template/recorded_reader.py.txt) agrees on widths, order, fill/tag/size placement, seed values and every input byte. All lanes check returned EAX, observed initial seeds, captured nonvolatiles/ESP/ES and DF. Only raw lanes assert ECX, EDX, arithmetic flags and raw physical parity; ordinary volatile outputs remain observations. ES equality is a selected recorded predicate. ECX is recorded and compared to the fixed map `_g_frame`; no loaded module, HMODULE, FileID, MEM_IMAGE or provider identity is serialized. Unchanged input bytes cannot establish unread default size; that comes from complete literal/static review.

## Preparation, receipts and sole process

[prepare_root.py.txt lines13..43](../local/count42peer/selected69/template/prepare_root.py.txt) verifies the selected draft inventory, original inputs, absolute mandatory tool pins and full recursive Capstone selection before exclusive materialization. It creates no approval receipt and launches nothing. Five materialized Python entries share the same1..38 bootstrap, requiring optimization0, `-B -E`, fixed family identity, a Root TEXT receipt, exact11 materialized-file pins, and full control/case-review predicates before selected helper execution. They expose actual interpreter, argv and environment.

[recipe.py.txt lines62..128](../local/count42peer/selected69/template/recipe.py.txt) has a sole `subprocess.run` gateway with fixed compiler/linker/readers/target argv, explicit environment/cwd, no shell, exclusive attempts/stdout/stderr, and timeouts. It compiles two fresh TUs, links without default libraries at fixed Win32 base with custom entry and embedded manifest, and consumes no old objects or BSP archives. The target basename is `count42_complete_probe.exe`. Immediately before dispatch, it rechecks TEXT/tool pins; target and recorded reader also require Root's linked receipt. That receipt binds code, map, objects, inventory, gate, actual byte reader, entry/process records and all prelaunch family files, permitting only explicit runtime successor names. The independent actual byte reader runs as its own recorded Python child before target approval. No helper can manufacture either Root receipt.

## Complete code and dependency obligations

The inventory exposes raw COFF sections/symbols/auxiliaries/relocations/directives, map text and executable bytes. Linear decoding is inspection material. [gate.py.txt lines58..128](../local/count42peer/selected69/template/gate.py.txt) requires every executable raw byte exactly once, explicit padding, complete42 literals, decoded control targets and fallthrough, required public definitions and the three import names GetStdHandle/WriteFile/ExitProcess. All weak records are rejected; unknown register or complex indirect transfers stop. Its one allowed non-IAT indirect edge is the four named, distinct capture targets. It does not guess aliases or register-held import reaching definitions.

Root must verify complete function extents/aliases, every symbolic relocation and physical operand, static/code-only ownership, wholeMain, ordinary wrappers, capture and all real helpers. Those semantics are explicit receipt obligations; the structural gate does not mechanically prove them. The separate stdlib byte walker independently checks all executable section bytes/ranges and both literals, and keeps qualified=false. No generic cookie/stack/EH body size is assumed from prior families; any emitted helper needs actual complete evidence or a stop.

The templates require compiler/linker/Python/kernel32_lib/originalPE pins, pin any extra selection.tools rows, and verify the entire Capstone package and loaded native filename. Candidate stdlib/package imports and required Source headers are listed in the report as TEXT obligations. Actual transitive compiler backends, showIncludes headers, interpreter/native dependencies and selected child environment remain Root selection/review work; these are not supplied installed facts or a hermeticity claim. Leaf provider count0 remains separate from probe output imports.

## Failures, bookends and handoff

Exclusive attempts/logs retain failures and exception stops; nonzero return records consume an attempt. Phase entry/finally check original case/Source pins and exact17 membership. A failed/stopped fixed01 attempt needs a newly reviewed template/family, not retry or receipt repinning. Root authors the final all-files runtime seal including all stops and logs; the candidate contains no automatic final runtime acceptance.

This peer's own AST/text/algebra checks execute only newly authored standard-library audit utilities. No selected module import, compiler, linker, Native/Ghidra/provider query, target, old helper, fixture replay, Source change or Root write occurred. Own `receipt.json` and `artifact_manifest.json` bind the exact recursive inventory and committed two metadata blobs; they are preservation evidence, not a runtime receipt. Current independently qualified consumers and historical Source associations are unchanged.
