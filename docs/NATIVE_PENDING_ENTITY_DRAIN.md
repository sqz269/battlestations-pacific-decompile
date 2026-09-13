# Actual pending entity queue drain

Address: `009273A0`. Source: `src/native_pending_entity_drain.cpp`.

| Routine | Original ABI | Coverage |
|---|---|---|
| `009273A0..009275D1` (562 bytes) | No inputs or stack arguments; callee-saved EBX/EBP/ESI/EDI; plain RET | Complete normal source behavior over the actual two raw owners, with required copy and virtual contracts. Original FH3/frame/SEH execution remains unproved. |

The source borrows `NativePendingEntityOwners`: destroy header at `F899A8` and kill header at `F899B4`. Each established 0Ch header has an untouched allocator word, sentinel pointer at +4, and count at +8. Each established 0Ch node has next/previous links and the canonical unit pointer at +8. This packet declares no second layout, queue, manager, endpoint or runtime binding. The existing semantic `entity_event_queues.cpp` remains separate.

The outer gate reads destroy count before conditionally reading kill count. A nonempty pass copy-constructs the destroy scratch and then kill scratch before changing either global ring. Only completed copies become caller-owned. It then clears destroy and kill globals in that order: capture head.next, self-link head.next, reload head and self-link its previous link, capture the comparison head, zero count, and free captured nodes in next-link order, reloading the header's current head after each free. Global sentinels survive. Whole-list `004C5940` frees its sentinel and cannot implement this clear.

Destroy traversal reads the current scratch head for each comparison, captures the current unit table and calls its actual slot74, validates the saved node against the reloaded head, then advances that saved node. Kill traversal compares against a captured head, executes the immediate-removal lifecycle, reloads the head for validation, reloads it again if the returning CRT validation service ran, and only then advances the saved node. No restart or repaired iterator is invented.

Normal scratch destruction starts with the final kill comparison head, without an extra initial header read. It detaches the kill ring, zeroes its count, frees nodes, and frees the last captured comparison sentinel. `0092756E` then captures the destroy head **before** `00927572` clears the kill head. Destroy nodes and its sentinel are released afterward and destroy head becomes null. Callback requeues remain in the actual global owners and are consumed on the next outer pass.

## Immediate-removal reuse

This mapping compares the complete inlined lifecycle with `009263C0` and its conditional notifier, including control edges and current/captured targets. Different scratch registers and CALL versus tail-JMP do not change the consumed effects; the native caller ignores EAX.

| Drain instruction(s) | Existing native wrapper instruction(s) | Preserved operation |
|---|---|---|
| `9274A1..74AC` | `9263C3..63CC` | Current table/slot18; first getter; null skips second getter and detach only |
| `9274AE..74BF` | `9263CE..63E1` | Reload table/slot18, capture actual `E188DC` before second getter, compare its result with that saved listener |
| `9274C1..74C3` | `9263E3..63E5` | ECX=0 into required `004BCA80` |
| `9274C8..74CC` | `9263EA..63EE` | Reload callback-mutable byte5E and skip remaining lifecycle when nonzero |
| `9274CE..74DA` | `9263F4..63FD` | Store5D=1,5E=1,5F=1,5C=0 in order |
| `9274DE..7509` | `925C45..5C71`, called at `926401` | Capture actual observer section, enter/depth++, sample unsigned count>0, depth--/leave same captured section |
| `92750A..7510` | `925C72..5C81` | Saved presence gates `00696330` after sampling lock release |
| `927515..751F` | `926406..6411` | Reload current profile after notification; invoke actual slot80 on the same canonical unit |

The complete listing's EBX writes are XOR at `9273C0`, SETA BL at `9274F8`, and XOR at `927521`. All intervening providers preserve the callee-saved register. Every completed kill lifecycle resets EBX; the skipped-byte5E path starts with zero. Therefore the inlined section-null/count comparisons against EBX are the existing wrapper's comparisons against zero. Flags and profile loads use borrowed actual storage, never snapshots of semantic units.

## Required and reused providers

| Boundary | Evidence and source contract |
|---|---|
| `00926FA0`, called at `9273DF/73F1` | Complete108-byte body read. ECX=destination, one source argument, EAX=destination, `RET4` at `927009`. Allocates a distinct sentinel, publishes destination+4/count0, then calls range copy. Preserves allocator word. **Required external provider**, including its own partial-construction rollback. No lock is present. |
| `009269B0`, called at `926FF2` | Complete276-byte listing and bytes read, including exception rollback. Seven DWORD arguments, `RET1C` at `926AC1`. Existing library/STL identity retained; no production STL port supplied. |
| `00781260` | Read remove-helper body and current xrefs. No direct drain edge. Called by cancel-pending `925A00` and `781350`; remains an external library contract for callers that need it. Not silently substituted or added to this drain. |
| `00BF65AC` | Existing canonical allocation/free provider, reused for all six explicit drain free sites. Every native call consumes one caller-cleaned DWORD; continuations use `ADD ESP,4`, sometimes after a captured head load or intervening list stores. |
| `00BF6713` | Actual `_invalid_parameter_noinfo`, which can return; no success shim. Invalid-storage execution is not covered by the valid-ring fixture. |
| `00694280/00696330` | Existing actual observer lifetime and event-delivery context, including shared Win32 section and actual observer edges/dispatch storage. |
| Unit slot74/18/80 and `004BCA80` | Required producer-selected virtual and renderer providers. Same canonical unit and actual observer aliases. No universal virtual default; no runtime binding added. |

## Cleanup, metadata, and evidence limits

Native map `DDA1F4` records state0→-1 through `CA6E50` (destroy scratch) and state1→0 through `CA6E58` (kill scratch). Both eight-byte funclets LEA their original frame locals and tail-jump `9252F0`, a five-byte thunk to `924A70`. Its complete 72-byte whole-list destruction matches canonical `004C5940` after normalizing the two E8 rel32 operands at offsets35 and55 to their same `BF65AC` targets; all other bytes match exactly. Source C++ guards destroy completed copies in reverse order on a source exception. The original ten-byte handler at `CA6E60` loads FuncInfo `DDA204` and jumps `BF6B43`. The first worker query found `no_ghidra_function`; root subsequently defined it as `EH_PendingEntity_Drain`, confirmed by a final worker read. Both observations are retained. No original FH3 exception was executed.

All 562 drain bytes and eleven dependency/data spans match the installed PE and live Ghidra memory. Source coverage has no unread normal-path ranges. Saved-body membership is a separate partial result: live queries at `927428`, `92745E`, `92755B`, `92756E`, `92759B`, `9275A0`, `9275AE`, and `9275B3` return `no_ghidra_function`. These are sampled missing points, not a claim to have enumerated each missing interval. In particular, the final two free call rows are retained and fail the mechanical containing-function check. The worker performed no Ghidra mutation; full before/after documentation records match. Root owns future annotation and body repairs.

Win32 Release build and both existing CTests passed. Two paired source/original-byte fixture cases cover an empty gate and FIFO destroy/kill visits with callback requeues into both actual global owners, forcing a second pass. Actual observer providers create/register/sample/deliver/clean up edges. Getter callbacks change current tables and the controlled publication; observer callbacks change flags and profiles. The fixture compares complete callback traces and actual free order/scratch-header states using a CRT free IAT tracer that forwards the real free. Global sentinels and observer quiescence are checked afterward.

The local fixture's valid-ring copy implementation is an explicitly isolated external contract, not evidence for full `926FA0/9269B0` or allocator rollback. Virtual/renderer callbacks are likewise explicit external contracts. The byte clone relocates all direct calls/globals/import cells and disables its original handler operand; native EH, corruption paths, asynchronous faults, original stack aliasing, runtime host binding, and gameplay remain unproved.

MSVC's compiled actual drain retains K-sentinel free at +1BB, D-head capture at +1C0, and K-head clear at +1C6; captured iterator validation/reload ordering is also preserved. MSVC moves pure comparisons after count stores while keeping the compared volatile head loads before those stores. There is no intervening callback or storage observation, so this is source behavior evidence, not instruction identity. Exact inputs, raw bytes, relocations, tools, object files, libraries, outputs, the initial fixture compile error, and hashes are retained by `local/drain_manifest.json` and the report.
