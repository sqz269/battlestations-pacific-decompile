# Actual InputDevice tree erase-one

`src/native_input_device_tree_erase.cpp` supplies whole `006A71B0..006A74F7` (840 bytes/235 instructions) and the whole 8-byte `00C7F580` action as an explicit logical completed-message service. Existing actual AV cleanup and admitted full-prefix range services remain unchanged. Ordinary `006A7AA0` range Source is not promoted here.

Native eraser inputs are ECX tree, unused incoming EDX and stacked output/owner/node, with EAX output and RET0Ch. Source fastcall adds the borrowed actual RawCtx as a fourth stacked argument (RET10h). The action receives a saved SBO directly; native `LEA ECX,[runtimeEBP-50h]` is evidence, not a fabricated frame API. Source EBP remains the compiler frame pointer.

The tree uses opaque DWORD0/head4/count8; actual nodes use left0/parent4/right8, pair+C and color98/nil99. The nil byte is tested before owner diagnostics. On the normal path the original removed node is cached and a private actual8B iterator is advanced before any topology edits. Simple replacement preserves nil-test/parent-capture order, current head/root updates and independently captured extrema/min/max calls. The successor path retains the original removed node and copies links and swaps colors in native read/write order. Both fixup sides and all six side-specific rotations preserve current loads, child rereads and color stores; the last replacement color write precedes pair destruction.

Pair destruction receives ORIGINAL node+C after rebalance. Only after it returns is ORIGINAL node unconditionally freed. Then CURRENT count is read and every nonzero DWORD decremented; zero alone skips the store. Owner/output/advancednode are all captured before output owner then node publication, preserving reached output aliases. A normal RawCtx getter failure propagates after topology changes and leaves node free, count and output uncommitted. No outer guard, rollback, scalar-delete, extra key cleanup or new validation is added. The accepted pair/record owns its armed unwind; a secondary C++ cleanup throw terminates under its current C++ guard policy.

The error path initializes the actual1Ch SBO and assigns 27 bytes, `invalid map/set<T> iterator`. Only after assignment completes does it arm completed-message cleanup, before constructing/throwing the existing owning `NativeHardwareLayoutInvalidIterator`. Raw native FuncInfo DACA80 uses one state, map DACA78: state0 to -1/action C7F580. Read-only handler C7F588 selects that FuncInfo and tails to FH3. The action destroys the saved SBO through actual4072D0; assignment failure has no completed-message guard. Native profile D6926C and ThrowInfo D863A8/three catchable types are recorded. Current C++ throw/catch/RTTI/CRT/FH3/SEH/fault/private-frame identity is not claimed.

Bindings are accepted rotations f0c01b6, pair3d1f01d6, actual provider-free5540C0 iterator, existing4B Raw min/max, legacy SBO/owning exception and currentCRT free. Throwing iterator/pair calls remain typed C++ calls outside the ordered mutation assembly. Rotation APIs are void: four sites reload sibling EAX and two final sites discard residue. Existing Raw is statically4B/trivially copyable with pointer at0; emitted caller/callee cleanup and result handling are checked separately from native ABI evidence.

Fresh evidence matches 848 owned bytes/237 current starts, 1388 inspected code bytes/413 starts, and 285 raw EH/type/literal bytes against PE/live. Producer windows prove allocation9Ch, pair+C and color98/nil99; the bounded ordinary caller proves stacked output/owner/node and RET0Ch without supplying its unresolved range Source. Every live tool batch verifies C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe. Full typed envelopes, decoded owned listing, raw maps/types, accepted dependency fingerprints and all transfers are in `reports/oct15_input_device_tree_erase.json`.

| Native transfer | Scope |
| --- | --- |
| 006A71F7 CALL 00408720 | owned whole |
| 006A7209 CALL 00411700 | owned whole |
| 006A7220 CALL 00BF6885 | owned whole |
| 006A7231 CALL 005540C0 | owned whole |
| 006A72A2 CALL 00552C70 | owned whole |
| 006A72CB CALL 00552C50 | owned whole |
| 006A7392 CALL 00553F00 | owned whole |
| 006A73DB CALL 00552340 | owned whole |
| 006A7401 CALL 00553F00 | owned whole |
| 006A7424 CALL 00552340 | owned whole |
| 006A747D CALL 00553F00 | owned whole |
| 006A74A1 CALL 00552340 | owned whole |
| 006A74B3 CALL 0055B8B0 | owned whole |
| 006A74BD CALL 00BF65AC | owned whole |
| 00C7F583 JMP 004072D0 | owned whole |
| 00C7F58D JMP 00BF6B43 | read only accepted dependency or handler |
| 005540C8 CALL 00BF6713 | read only accepted dependency or handler |
| 005540DA JMP 00BF6713 | read only accepted dependency or handler |
| 0055B8D8 CALL 0055B690 | read only accepted dependency or handler |
| 0055B8F5 CALL 00419CC0 | read only accepted dependency or handler |
| 0055B8FC CALL 00BD1510 | read only accepted dependency or handler |
| 006A7B39 CALL 005540C0 | read only ordinary caller window |
| 006A7B47 CALL 006A71B0 | read only ordinary caller window |
| 0055BB66 CALL 00BF681B | read only node layout producer |
| 0055BB9D CALL 0055BA00 | read only node layout producer |

MSVC Win32 build and existing three suites passed, 3/3 in7.32 seconds; no new tests were added. Nine focused COFF checks pass with no finding: parent942B/15calls including compiler security-cookie check, Raw min/max34B/36B, rotations83B/81B, exact stack cleanup/nonvolatile preservation, typed original-node/pair calls, CURRENTcount/orderedoutput and post-assignment message activation. Public argument words are compiler-cached in private Source slots; original private argument/frame identity is qualified. Fresh report25rows/Markdown25claims pass with zero failures or unchecked claims. The FIFO CMake gate was released before build. Valid reached owned storage/currentCRT/rawpool ownership is required; original private argument aliases, hardware faults, original CRT diagnostic policy and game/runtime compatibility remain outside the Source proof. No Ghidra mutation or push is performed by this worker.
