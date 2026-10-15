# Actual InputDeviceTree subtree destruction

Whole `006A7540..006A75E0` is161B/51 instructions; parent-bound action `00C7F5A0..00C7F5A7` is8B/2 instructions. The new raw-pool Source composes the accepted complete pair destructor and current CRT free. Existing actual AV/NativeStringStorage cleanup remains in its original domain, and correct STL/EH names are preserved.

## Storage and original interface

Actual9Ch nodes carry links0/4/8, pooled key length0C/data10, the84h record at14, color98 and nil99. Native ECX is an opaque tree word: retained inEBX and forwarded only to right recursion; it is never dereferenced. Incoming EDX is unused. One stacked node word is consumed by RET4. There is no semantic EAX result; initial nil can retain FS residue, nonempty completion free residue. The Source interface adds stacked RawCtx and is not a binary ABI replacement.

Only finite uniquely owned valid nodes and a live sentinel are admitted, with actual borrowed publication/gate/manager cells and matching current-CRT allocations. No synthetic head, count, null or ownership check is added. The outer tree header/count is untouched by this subtree entry. Node producer55BB40 has9Ch allocation, pair+C, color98 and nil99=0; that producer is layout evidence only, without new copy/allocation Source scope.

## Complete destruction schedule

| Native site | Recovered work |
|---|---|
| 006A755B | Read byte currentnode+99; nonzero skips all link/payload/free work. |
| 006A7570/76 | Capture CURRENT right+8 and recurse with the same opaque tree. Key state is-1. |
| 006A757B | After recursion, reread/capture CURRENT left. |
| 006A757D/80 | Form pair=node+C and save it by overwriting the original incoming node stack word. |
| 006A7587/8F | Arm state0, destroy record pair+8=node+14. |
| 006A7594/97/99 | Capture CURRENT keydata, test it, then disarm state-1 before branch/length/getter. |
| 006A75A3..B3 | For nonnull data capture CURRENT length+1 with DWORD wrap, pre-push unused1/size/data, resolve actual pool and return captured block/size. Null skips this. |
| 006A75B9 | Unconditionally free ORIGINAL current node after successful pair work. |
| 006A75C1/C8/CA | Test captured-left nil99 after free, then select it as nextcurrent and repeat when ordinary. |

Source retains explicit right-recursive/left-iterative work and raw byte/DWORD loads. The independently compared complete pair55B8B0 is used once at node+C: its record+8 is node+14, its key0/4 is node+C/10, and its data/test/disarm/conditional length/getter/return schedule matches the native inlined pair phase. Different register allocation and saved-frame locations are qualified. Scalar deletion6A6EB0 is not substituted, because its flags word/read/test is absent from the subtree.

## EH action and failure progress

HandlerC7F5A8 selects FuncInfoDACAAC: magic19930522,maxState1,mapDACAA4, no try/IP/type list,rawEHFlags1. Map state0 advances to-1 before actionC7F5A0. Let S=entryESP: registration isS-C, saved registers leaveESP=S-1C, and parent7580[ESP+20] writes currentpair to[S+4]. Runtime EBP=S; action[EBP+4] is therefore alreadyPAIR, not NODE. The standalone logical Source action accepts saved_current_pair directly and applies raw41DD20, never adds+C again and never invents an EBP reader.

The accepted pair destructor already owns CURRENT-key unwind and CurrentCpp termination on secondary C++ failure. No additional key guard surrounds that delegate. A right-recursion failure stops before currentleft/pair work. A record failure first applies its own earlier-member unwind; when it completes the pair applies CURRENT-key cleanup. Secondary C++ failures during either unwind terminate through the accepted current runtime policy. A normal key/getter failure occurs after disarm, so it propagates before node free and further left iteration. No rollback, retry or prefix salvage is added. Direct standalone logical action use can propagate.

Native FH3/type/RTTI/foreign-SEH/managed/fault/stack-exhaustion and original CRT/heap/terminate-handler identity are not established by CurrentCpp composition. Added context/by-value inputs/private output frames and original incoming-argument overwrite are not original stack/register/flags/exception ABI. Source/build proof is distinct from runtime/game validation.

## Exact native transfers

All owned transfers and the independently checked pair/handler/runtime support chain are retained below. Supporting native entries are evidence, not new Source ownership.

```text
006a7576 CALL 006a7540
006a758f CALL 0055b690
006a75ac CALL 00419cc0
006a75b3 CALL 00bd1510
006a75b9 CALL 00bf65ac
00c7f5a3 JMP 0041dd20
00c7f5ad JMP 00bf6b43
0055b8d8 CALL 0055b690
0055b8f5 CALL 00419cc0
0055b8fc CALL 00bd1510
00c6dca3 JMP 0041dd20
00bf6b64 CALL 00c07991
00c079f9 CALL 00c069a2
00c06a19 CALL 00c07b10
00c07aa3 CALL 00c04de2
```

## Verification

Fresh typed metadata at modification1379 verifies bsp.gpr `/battlestationspacific.exe`, complete bodies, non-thunk and returning properties. All169 owned bytes and53 instruction starts match PE/live; supporting complete pair/handler107B adds31 starts, so84 total current starts pass. Raw map/FuncInfo44B matches disk/live. The JSON report retains full hashes and local capture paths.

MSVC Win32 Release build and all three existing suites pass (7.68s). The JSON15 transfer rows and Markdown15 claims pass with zero failed/unchecked/warnings. Current COFF confirms opaque tree forwarding, CURRENT-left reload after recursion, one pair invocation followed by originalnodefree, captured-left nil test before nextcurrent selection, and direct standalone41DD20 delegation. There is no local CxxFrameHandler symbol/second keyguard in this object. SourceRET8 versus nativeRET4 remains explicitly qualified. No new tests are added. The five following clear/fullprefix/map wrappers remain readiness only and are not claimed or edited.
