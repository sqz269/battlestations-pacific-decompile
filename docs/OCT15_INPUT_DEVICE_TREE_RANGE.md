# Actual InputDevice tree range erasure

`src/native_input_device_tree_range.cpp` supplies whole defined `006A7AA0..006A7B1E` and `006A7B20..006A7B68`: 200 bytes/76 instructions. The 201-byte envelope includes undefined90 padding at7B1F, which is unexecuted and remains unpromoted. Whole coverage overlaps the preserved127B/49-start admitted prefix and newly closes73B/27-start partial path; cumulative unique native body coverage is200B, never327B.

Native ECX receives actualtree, incomingEDX is unused, and five stacked words are output/firstowner/firstnode/lastowner/lastnode; both exits return EAXoutput/RET14h. The distinct raw Source API adds a borrowed RawCtx as a sixth stacked input (RET18h). Source helper/private-frame/ambient-register identities are qualified.

Selection executes once. Firstowner is captured and tested, then CURRENT tree.head.left captured BEFORE the first owner diagnostic. Firstnode is read only AFTER it returns. Only matching firstnode reads lastowner, tests it and captures CURRENThead before any lastowner diagnostic; CURRENTlastnode is compared after that diagnostic. Returning diagnostics can change reached storage, so the admitted full-prefix service is not re-invoked after this selection. Its preconditions and extra repeated selection reads cannot represent that whole-entry path.

The selected full path directly invokes actual6A7540 on CURRENT head.root. After success it reloads head for head.parent=self, reloads head before count0/head.left=self, reloads head for head.right=self, then reloads head and captures head.left before reading output. Owner then node publication follows. There is no new null/count/owner guard, rollback or key cleanup.

The partial loop tests cachedowner; zero skips the CURRENTlastowner read, otherwise compares with that current endword. The diagnostic precedes cachednode versus CURRENTlastnode comparison, including empty exits. Actual mutable8B current is advanced before actual8368 erase of cachednode/cachedowner. Private8B erase output and EAX are ignored. CURRENT iterator.node is reloaded FIRST, then owner, before the next iteration. Both output values are captured before owner then node stores. Source volatile input words prevent eager firstnode/end/output snapshots; current is formed progressively rather than eagerly copying the initial pair.

Actual RawCtx subtree6a9cdda, eraser8368dd, provider-free5540C0 iterator and acceptedBF6713 diagnostic close all child contracts. The diagnostic uses actual selected currentSDK `_invalid_parameter_noinfo`, not fabricated semantic arguments or an original encoded CRT provider. Native fullcallEDX holds root and partial eraserEDX holds scratchoutput; accepted children ignore those words, so Source explicitly passes unusedEDX0. Source EBP remains its compiler frame pointer. Original mutable argument/frame addresses versus new private current/output objects are qualified.

There is no local Native EH/cleanup/action in the whole76-start parent. Child getter/diagnostic failures propagate and stop later parent resets/output/reloads; partial current has already advanced before eraser failure, and eraser can leave topology changed before normal pair getter failure. Accepted pair/record/key guards own their secondary CurrentCpp termination policy. This parent adds no guard/noexcept bridge/catch/provider/allocator model. Existing real AV/full/partial NativeStringStorage domains and127B admitted prefix stay unchanged; broader6AA460 lifetime/manager providers are outside the packet.

Fresh typed metadata and PE/live proof cover200 owned defined bytes/76 current starts, with1584 inspected bytes/515 starts including separated accepted dependencies and bounded real callers. Five real caller sites occur in three52B map destructors and two6AA460 windows; the three complete map bodies normalize identically after zeroing only two rel32 operand DWORDs, with actual targetsequence preserved. All28 provider fingerprints and six accepted Source/header blobs are checked. Every Ghidra batch uses project/program-verifying BSP tools; full owned listing and supporting spans are in `reports/oct15_input_device_tree_range.json`.

| Native transfer | Scope |
| --- | --- |
| 006A7ABA CALL 00BF6713 | owned defined |
| 006A7AD6 CALL 00BF6713 | owned defined |
| 006A7AEA CALL 006A7540 | owned defined |
| 006A7B2A CALL 00BF6713 | owned defined |
| 006A7B39 CALL 005540C0 | owned defined |
| 006A7B47 CALL 006A71B0 | owned defined |
| 006A7576 CALL 006A7540 | read only accepted dependency |
| 006A758F CALL 0055B690 | read only accepted dependency |
| 006A75AC CALL 00419CC0 | read only accepted dependency |
| 006A75B3 CALL 00BD1510 | read only accepted dependency |
| 006A75B9 CALL 00BF65AC | read only accepted dependency |
| 005540C8 CALL 00BF6713 | read only accepted dependency |
| 005540DA JMP 00BF6713 | read only accepted dependency |
| 006A71F7 CALL 00408720 | read only accepted dependency |
| 006A7209 CALL 00411700 | read only accepted dependency |
| 006A7220 CALL 00BF6885 | read only accepted dependency |
| 006A7231 CALL 005540C0 | read only accepted dependency |
| 006A72A2 CALL 00552C70 | read only accepted dependency |
| 006A72CB CALL 00552C50 | read only accepted dependency |
| 006A7392 CALL 00553F00 | read only accepted dependency |
| 006A73DB CALL 00552340 | read only accepted dependency |
| 006A7401 CALL 00553F00 | read only accepted dependency |
| 006A7424 CALL 00552340 | read only accepted dependency |
| 006A747D CALL 00553F00 | read only accepted dependency |
| 006A74A1 CALL 00552340 | read only accepted dependency |
| 006A74B3 CALL 0055B8B0 | read only accepted dependency |
| 006A74BD CALL 00BF65AC | read only accepted dependency |
| 00BF671A CALL 00BF66EF | read only accepted dependency |
| 006A7BB6 CALL 006A7AA0 | read only complete map caller |
| 006A7BBF CALL 00BF65AC | read only complete map caller |
| 006AA3F6 CALL 006A7AA0 | read only complete map caller |
| 006AA3FF CALL 00BF65AC | read only complete map caller |
| 006AA436 CALL 006A7AA0 | read only complete map caller |
| 006AA43F CALL 00BF65AC | read only complete map caller |
| 006AA4BD CALL 006A7AA0 | read only settings caller window |
| 006AA4C6 CALL 00BF65AC | read only settings caller window |
| 006AA60B CALL 006A7AA0 | read only settings caller window |
| 006AA614 CALL 00BF65AC | read only settings caller window |

MSVC Win32 build and existing three suites passed, 3/3 in9.93 seconds; no new tests were added. Twelve focused COFF checks pass:357B/129instructions/exact6calls/no local EH, twoRET18h exits, no premature node/end/output input loads, conditional CURRENT endword reads after diagnostics, balanced actual subtreeRET8/eraserRET10h/cdecliterator/zeroargdiagnostic calls, nodeTHENowner reload and both captured output-store pairs. Compiler reuses dead full-path endword slots for root/result only after final selection; original private-frame identity is qualified. The FIFO CMake gate was released before build. Fresh38JSONrows/38Markdownclaims pass with zero failures or unchecked claims. CurrentCRT/CurrentCpp/helper ABIs/privateframes/registers/flags/OriginalFH3/SEH/RTTI/fault/game limits remain explicit. No Ghidra mutation or push is performed by this worker.
