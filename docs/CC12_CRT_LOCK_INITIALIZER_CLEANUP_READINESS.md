# CC12 CRT lock initializer cleanup: complete ordinary Source candidate

`00C11C18` is a complete nine-byte, four-instruction helper: push 10, call the actual canonical `__unlock`, pop ECX, return. Original PE bytes, all four live Ghidra instruction starts, and the fresh saved listing agree. The new ordinary Source helper compiles for MSVC Win32 and resolves its sole indexed COFF call relocation to the existing genuine canonical unlock object. It remains unregistered pending Root integration and the normal build.

The Native gate covers only `00C11C18..00C11C20`. Exact bytes are `6a0ae812ffffff59c3`, SHA-256 `7c8670c564d00524ac491f13c55e55718cd88f082aaecb91b1a141f2e86a4042`. There was no historical export; the saved listing is labelled fresh. Analysis used verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. No Native child body, fixed descriptor contents, handler or parent scope data was opened. Existing `LIBCRT_unmatched_00c11c18` and correct `__unlock` names/comments remain unchanged.

## Exact helper and real ownership

| Native start | Instruction | Effect |
| --- | --- | --- |
| `00C11C18` | `PUSH 10` | Supply one DWORD lock index. |
| `00C11C1A` | `CALL 00C11B31` | Invoke canonical `__unlock`. |
| `00C11C1F` | `POP ECX` | Remove the caller-owned index; ECX becomes 10. |
| `00C11C20` | `RET` | Return to the helper's caller. |

The helper takes no explicit arguments and has no own frame, parent-EBP read, FS access, local allocation, branch or EH region. POP and RET preserve the child's residual EAX, EDX and arithmetic flags. There is no meaningful return value. The declared ordinary `__cdecl` interface has a naked MSVC x86 body so compilation retains the separate call and POP rather than replacing them with a tail jump or stack ADD.

`unlock_native_crt_initializer_lock10_00c11c18()` calls the genuine declared `unlock_native_crt_canonical_00c11b31`. Its caller must own the readable canonical descriptor at `00E164C8` (`00E16478 + 10*8`) and the valid, initialized critical section stored there, acquired by the current thread and alive throughout the call. This is an actual fixed-domain ownership precondition. The helper supplies no storage, heap, initialization, acquisition, replacement lock or callback policy. Materialized CRT pages alone cannot meet it.

Source files are `include/bsp/native_crt_lock10_unlock_helper.hpp` and `src/native_crt_lock10_unlock_helper.cpp`. The existing canonical provider reads the actual indexed descriptor and calls the real `LeaveCriticalSection` import. Its Source contract and entire actual normal-build object were reviewed without opening that Native child. There is no invented provider or unresolved stand-in. Direct-call relocation changes code placement; original fault PCs and parent SEH-table compatibility are not established.

## Focused compile and complete object review

The compile uses VS 18 Community `VsDevCmd -arch=x86 -host_arch=x64`, with the existing Release-style `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17` flags. It exits 0 without diagnostics. This is a compile-only object check; no executable, runtime probe, new test or normal project build runs in the worker packet.

The new object is 810 bytes, SHA-256 `c4df257103a3be9fea2b78169e1566e733bd3305572bd666ef3759c5aaacabf5`. Its sole code section is nine bytes `6a0ae80000000059c3`, four instructions, with one REL32 relocation at offset 3, symbol record 9, to `?unlock_native_crt_canonical_00c11b31@bsp@@YAXH@Z`. This matches the genuine strong provider at symbol record 8, section 3, value zero in Root's existing 802-byte `native_crt_canonical_unlock.obj`, SHA-256 `39da5fa1a78fc93aa9488008ddf1fc5bfe3f838a97be52c6db1dcc2aa8982619`.

That provider's entire code is 21 bytes/seven instructions, with one DIR32 relocation at offset 15 to `__imp__LeaveCriticalSection@4`. Both complete COFFs were reviewed by original symbol index, including auxiliary records: four sections and 12 total/eight primary symbol records each. The remaining sections contain only library directives, compiler/object-path metadata and checksums. There is no other executable section, EH section or additional relocation. The provider is byte-stable before and after the compile. This establishes actual-object resolution; it is not a new archive/link/runtime claim.

## Integration and limits

Root's historical Source539 receipt records 539 selected project inputs, 59 Core/two App whole objects, 92 positive Core providers, three checks and its atlas regression case. Those inputs are a selected project union, not the full compiler/SDK closure. Its normal build predates this helper and provides no new-helper registration or test credit. The receipt, six relevant current Source files and local Native/COFF evidence are pinned separately in the JSON.

A targeted current CMake search finds no new helper entry. Root must review and register the cpp, run the normal Win32 build/checks, and review the fresh built object/provider before ordinary Source admission. Root also owns any ledger and Ghidra annotation changes. This packet commits exactly four owned files and makes no CMake, ledger or GPR change.

The containing initializer remains held: its six missing live starts, nullable canonical allocation, initialization dispatch, PTD/errno, canonical free, diagnostics, prolog/handlers, heap and lock-10 bootstrap ownership are unresolved. The complete child adds one ordinary Source candidate and a successful focused compile. It adds zero registered Source, parent Source, Native ABI, runtime lock-release, fault/unwind, faithful-startup or gameplay credit.
