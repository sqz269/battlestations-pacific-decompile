# Native CRT free lock-4 unlock helper Source candidate

`unlock_native_crt_free_lock4_00bf9e1e()` reconstructs the complete, separately defined nine-byte helper at `00BF9E1E..00BF9E26`. It calls the existing genuine canonical unlock Source provider with index 4 and preserves the original `POP ECX` before returning. This packet delivers a compiled Source candidate; Root owns registration, the normal build and admission.

## Native evidence and scope

The approved read-only gate in commit `630ae44bcdfdb770270f0c8e00ef758d5f0782ae` checked all nine original PE/live bytes and four exact saved/live instruction starts. Native bytes are `6A 04 E8 0C 7D 01 00 59 C3`, SHA-256 `851c6aed22514b3169b21db40740ec407cda2f0be832b2a0b59e427a75e09175`. No new Native child/data query or Ghidra mutation was needed.

| Native address | Operation | Source preservation |
|---|---|---|
| `00BF9E1E` | `PUSH 4` | Literal argument to the real canonical child |
| `00BF9E20` | `CALL 00C11B31` | Direct call through its genuine C++ declaration |
| `00BF9E25` | `POP ECX` | Retains argument cleanup and ECX=4 on normal return |
| `00BF9E26` | `RET` | Plain return, no input arguments or own frame |

The helper remains distinct from the surrounding discontiguous free function. The ordinary call is established; the unopened SEH table is not an unwind-path proof. The implementation is guarded for MSVC Win32 and uses no injected callback, private global copy, fake owner, replacement lock, initialization, acquisition, `noexcept`, catch or new EH frame.

## Caller requirements and residual state

The caller must provide the readable actual fixed DWORD descriptor at `00E16478 + 4*8 = 00E16498`, pointing to a valid initialized Win32 critical section already acquired by the calling thread. The descriptor and lock must remain valid through the real `LeaveCriticalSection` call. Canonical page admission alone does not satisfy these requirements.

The public entry is `void __cdecl bsp::unlock_native_crt_free_lock4_00bf9e1e()`. It has no input argument words, own frame or EBP-relative accesses. `PUSH 4` supplies the child's argument; the returning cdecl child leaves it for the original `POP ECX`, then plain `RET` consumes the helper continuation. EAX/EDX and arithmetic flags retain child residual state; there is no meaningful return value. Preserved registers follow the genuine child and Win32 API contracts. Original fault PCs and exception behavior are not admitted.

## Compilation and complete emitted-object review

The single real Source object compiled successfully using the existing project setup: Visual Studio 18 Community, MSVC `19.51.36244.0`, `VsDevCmd -arch=x86 -host_arch=x64`, and the Core Release `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17` settings. No executable or new test was built. The exact response file and compiler/log pins are retained in the JSON receipt.

The 798-byte candidate object has one executable section, exactly nine bytes: `6A 04 E8 00 00 00 00 59 C3`. Complete decoding yields four instructions and one call, with no other code, hidden calls or EH data. The only code relocation is `IMAGE_REL_I386_REL32` at offset 3, **symbol index 9**, naming `?unlock_native_crt_canonical_00c11b31@bsp@@YAXH@Z`. Auxiliary symbol records retain their real indices.

The current existing normal-build canonical-unlock object defines that exact symbol at **index 8**, section 3, value 0. Its complete 21-byte/7-instruction body reads the actual fixed descriptor and contains one `IMAGE_REL_I386_DIR32` relocation at offset 15, index 9, to `__imp__LeaveCriticalSection@4`. This establishes the concrete provider target at the object boundary; it is not a new executable or Core-library link. Both objects' other sections contain only default-library directives, compiler/object metadata and checksum data, with no relocations.

Candidate object SHA-256: `e851fb864240e372d220e04552021641d40b35529ce94cae3edf2501948e2200`. Provider object SHA-256: `39da5fa1a78fc93aa9488008ddf1fc5bfe3f838a97be52c6db1dcc2aa8982619`. Complete indexed sections, symbols, auxiliary records, relocations and decoded instructions are included in the report. Provider Source files match Root after LF normalization.

## Admission boundary

Only the header, implementation, this document and its report are changed. CMake registration, normal `scripts/build.ps1`, fresh normal-build whole-object/provider review, ledgers and coordinated Ghidra annotation remain Root work. Worker admission, original ABI, runtime/fixture, faithful startup and gameplay credits are zero.

This leaf closes no lock initialization/acquisition/lifetime owner, allocator selector/heap/lookup/error service, full free dispatcher, gunnery destructor, SEH table or native caller reachability. Those requirements from the prior readiness report remain in force.

Current inherited Root context is Source511 in `reports/cc12_unit_tick_fragment_primary_review.json`, SHA-256 `6483e4c7f10827366aa5cab0605ce946bfce8dac97e44c5a29a1641ae34388b7`: 44 selected Core objects, 1 application object, 52 positive Core roots and 3 passing existing checks. Its latest smoke has 3 ticks, 2 Presents, 1 skip, PressStartPoll/state 2, zero mission frames and no injected input or visual proof. These results were pinned, not rerun; old receipts were not compared with current artifacts.
