# FileStore completion review and combined validation

Addresses reviewed read-only: `00BE7760`, `00BE78B0`, `00BE7B20`, `00BE5CF0`,
`00BE6090`, `00BE6250`, `00BE7340`. This packet owns only this document and
`reports/native_filestore_completion_review_bk.json`; the primary owns all
completion source and the separate resident-insertion worker owns its helpers.

## Review result

Both capture-order corrections found during independent review are present in
the primary source snapshot. BE7760 now captures the found owner/node and
current resident head before its returning CRT check, matching BE77A6..BE77BB.
BE78B0 now captures the found owner before the first CRT check, reads the node
after that check, and reloads the erase owner after the second check while
retaining the captured node for callback capture and erasure. This matches
BE78F7..BE7922. These are static capture-order checks; the combined fixture
does not force the exceptional iterator-validation branches.

| Entry | Inclusive end | Bytes | Reviewed behavior |
|---|---|---:|---|
| BE7760 | BE78A0 | 321 | Normalize copied key; resident duplicate skips retain; new pair/node ownership and ordered cleanup |
| BE78B0 | BE797E | 207 | Normalize first; capture pending callback; erase pending; AddFile; callback(first, second) |
| BE7B20 | BE7B3E | 31 | Reacquire current factory; read its current cache+8; forward first, second, stream |
| BE5CF0 | BE5D6E | 127 | Release captured stream; clear current pair+8 after successful dispatch; release current key |
| BE6090 | BE6113 | 132 | Copy actual key; clear output stream; read caller stream cell; publish before retain |
| BE6250 | BE62D0 | 129 | Copy actual key; clear output stream; reread source pair+8; publish before retain |
| BE7340 | BE7453 | 276 | Unique case-insensitive resident insertion; preserve output bytes9..11 |

All seven main bodies were inspected completely. Their source counterparts
retain their full ordinary control flow; dependent predecessor/node-link
implementation is reviewed and integrated separately. There is no source
stub or projected map substituted for the resident tree in this packet.

The native request at BE7EEF supplies `(resolved, original, BE7B20, 2)`.
Physical completion at BF476D supplies `(stream, record+20, record+28)`.
BE7B20 rearranges those arguments to `(first, second, stream)` for BE78B0.
BE78B0 normalizes only its first-name copy for lookup/AddFile and forwards
both incoming headers to the captured user callback at BE7942. No stream
argument, callback-null guard, name swapping or submitting-store capture is
added. BE7922 erases with ECX=store+20 and the current saved iterator owner.

## FH3 and lifetime evidence

E016B0 contains BE7760's four cleanup states: normalized key via CC6DA0,
retained stream cell via CC6DA8/BE59C0, completed first pair via
CC6DB0/BE5CF0, completed second pair via CC6DB8/BE5D70. Normal code lowers
each state before that member's destruction. The source arms pair cleanup
only after its constructor returns, destroys second then first, then releases
the retained stream and normalized name. E016F4 sends completed BE78B0's
normalized name to CC6DD0/41DD20. E01438 sends BE5CF0's current pair key to
CC6C20/41DD20 if stream destruction throws.

Native handler entries CC6DC0..CC6DC9, CC6DD8..CC6DE1,
CC6C28..CC6C31, CC6C68..CC6C71 and CC6CA8..CC6CB1 lack Ghidra function
definitions; each complete ten-byte handler span is recorded. Existing
cleanup funclet ranges and every direct/tail call row are checked separately.
No Ghidra function, comment, annotation or saved project was changed.

Native BE59C0 rereads and clears its retained pointer cell during unwind.
The completion source excludes arbitrary native EH spill aliases and exposes
ordinary C++ cleanup instead. NativeStringStorage release is noexcept.
Neither native FH3/SEH identity nor throwing-getter/double-failure behavior
is established by ordinary source execution.

## Combined fixture

The ignored `local/completion_review_bk/completion_chain_probe.cpp` composes
the full source request BE7CD0, manager BDDA10/BDB0B0, actual BDD0A0 traversal,
physical BF43B0/BF46B0, adapter BE7B20, completion BE78B0 and AddFile BE7760.
It uses the real ActualNativeStringPoolStorage, BE7FA0 FileStore constructors,
NativeVfsRuntimeBindings provider/stream dispatch and independent retained
memory-stream owners. Original read-only profiles are loaded from the
installed executable. The two existing mount records are a controlled actual
24h-layout fixture graph; this is not evidence for manager mount construction.

Only candidate-name resolution and the terminal application callback are
controlled fixture services. Resolution maps normalized logical/asset to the
real completion_payload.bin; provider submission and pumping invoke full
source bodies. The final callback observes names, pending/resident counts and
the actual retained stream. It does not supply data, erase a key, populate the
resident tree, balance references or replace any completion routine.

The fixture poisons the old factory cache with a different empty FileStore,
publishes another factory/cache before pumping, and requires completion in
the original requested store. It checks no callback before the pump, pending
duplicate first-callback retention, pending erasure before the final callback,
257 real input bytes, callback reference count2 becoming cache count1 after
the physical pump returns, resident duplicate behavior, and stream/backing
counters returning to zero on full FileStore destruction.

The combined fixture passed. Seven complete translation units, 81 transitive
project headers and three existing libraries were frozen before compilation;
every original file matched its copy before and after freezing. This includes
the primary's completion/request/physical-pending/runtime-binding/lookup/pending
route units and the separate worker's complete resident-insertion unit.
MSVC Win32 C++20 /MD /O2 /W4 /WX /fp:strict compilation and manifest-embedded
linking passed. The report pins every frozen source/header/library, the probe
source/object/executable, and the real input bytes by SHA-256.

The run passed every observation above, including both duplicate paths and
full resident-owner cleanup. It executed rebuilt source against real Win32 I/O;
it did not execute the seven original machine-code bodies. BE5CF0's identical
pair cleanup is composed through the shared BE5D70 implementation in AddFile,
so this fixture does not establish a separate invocation of the BE5CF0 wrapper.
Native span agreement is separate static evidence. Original binary ABI,
native throwing-unwind execution, loader/application scheduling and gameplay
remain unproved. No full repository build is claimed by this worker.

## Evidence

Fresh verified read-only Ghidra queries target C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. The seven main spans total 1,223 bytes; fifteen
FH3/cleanup spans total 295 bytes. All 1,518 bytes match the installed PE.
Saved listings, bytes, prototypes and immutable source snapshots are under
`local/completion_review_bk/`. Commands and SHA-256 pins are in the report.
