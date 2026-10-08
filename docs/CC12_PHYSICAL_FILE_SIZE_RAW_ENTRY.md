# CC12 physical file-size raw entry

`00BF4FA0..00BF4FBA` is reconstructed as the distinct naked Win32 fastcall entry `raw_query_native_physical_file_size_00bf4fa0`. The existing ordinary C++ helper, cached-size helper, and prior raw entries are unchanged. The whole 27-byte body, its import, and three real-OS Source/Original pairs passed; this does not admit a Source class or lifetime.

## Original contract and complete body

The caller supplies actual stable backing in ECX, with a live HANDLE DWORD at `+8h`, and one unused stacked DWORD. The entry performs a fresh `GetFileSize` query and returns with `RET4`. The low DWORD is in **EAX**. The initially-zero stack scratch containing the high DWORD is popped into **ECX**. EDX is an unpromised volatile register; this entry does **not** return a uint64 in EDX:EAX. The C++ declaration exposes only EAX.

```asm
push ecx
mov ecx, [ecx+8]
lea eax, [esp]
push eax
push ecx
mov dword ptr [esp+8], 0
call dword ptr [00CE2114] ; KERNEL32.dll!GetFileSize
pop ecx
ret 4
```

There is no receiver/cache write. All 27 live Ghidra bytes agree with the installed image. The complete normal-production COFF body has nine instructions and exactly one `DIR32` relocation, `__imp__GetFileSize@8`, at byte19. Only that four-byte operand is normalized. The unique linked body is whole-function equal outside that operand, which resolves to the actual PE import and runtime OS export. Exact whole-object membership in the normal `bsp_core.lib` was also checked. No instruction, call, additional relocation, or receiver field is normalized.

## Fresh real-OS evidence

The one new family is `local/cc12_physical_file_size_raw_entry_20261008a/run01`. Static Original/COFF/unique-linked proof finished before execution. The probe then checked loaded bytes, both actual entry addresses, the Source IAT, and the real OS module before any fixture production entry ran. The Original 27-byte code is unchanged in private RX memory; its actual `00CE2114` import cell is privately mapped read-only and bound to the same real `GetFileSize` export. There is no replacement API callback.

The current Source raw constructor `00BF50D0` supplies backing for this new fixture. Its whole 43 bytes independently agree with live Original, production COFF and unique linked bytes without normalization. It is called three times as an input producer; the Original constructor is not executed. This constructor writes literal Original profile `00D691B0`. The fixture supplies actual `CreateFileW` handles, nonzero cached position/size words, and 16-byte guards around a live aligned 20h backing. No Source vtable/class is fabricated or dereferenced. Executed production bodies total 70 Source bytes and 27 Original bytes; the OS implementation is outside those counts.

| Pair | Actual file/handle | EAX low | ECX high | LastError Source/Original | OS cursor before/after |
|---|---|---|---|---|---|
| Small | Real 64-byte file | `00000040` | `00000000` | `5A5A` / `5A5A` | 17 / 17 |
| Sparse | Real `0x100000023`-byte file | `00000023` | `00000001` | `5A5A` / `5A5A` | 29 / 29 |
| Invalid handle | `INVALID_HANDLE_VALUE` | `FFFFFFFF` | `00000000` | 6 / 6 | 13 / 13 on observed valid file |

The sparse attribute is confirmed before extending the file; independent `GetFileSizeEx` observations confirm both actual lengths before and after. All three pairs preserve every byte of the 64-byte receiver/guard image, the real cursor, ESP/RET4 cleanup, and EBX/ESI/EDI/EBP. Source and Original use the same actual backing address with different unused EDX/stack seeds. EDX is recorded but never compared or canonicalized. EFLAGS and other unpromised state are not qualified. LastError is captured immediately: successful preservation of the seeded value and invalid-handle ECX zero are observations from this run, not broader API promises. The real OS import/export resolves to `C:/Windows/SysWOW64/kernel32.dll` in this process. Both temporary files were deleted on close.

## Build, link and retained evidence

`./scripts/build.ps1` passed with all three existing CTests: `reconstructed_math`, `native_math_differential`, and `tool_tests`. The fresh fixture adds no permanent tests. Its initial two-object/zero-BSP-archive link failed with 10 unresolved externals (exit 1120); the exact failed recipe/log is retained. Linking the genuine current core, Lua511 and zlib121 archives succeeded. No stub or adapter was added to satisfy the linker, and unexecuted ordinary providers were not admitted. The probe embeds an `asInvoker` manifest.

The input manifest retains equal before/after hashes and frozen copies for the whole production object/archive, both explicit objects, actual sources and headers, compiler backends, OS DLLs, libraries, and recipes. It includes 202 actual normal-module inputs, 165 actual probe headers, 19 searched libraries, 16 recipe/tool records, 46 compiler/backend files, and 8 generated recipes. Whole object and archive copies remain available even after later builds replace mutable build paths. The report records their exact paths/hashes together with all artifact hashes and gate/execution timestamps.

See [the report](../reports/cc12_physical_file_size_raw_entry.json), `run01/whole_size_proof.json`, `run01/inputs/manifest.json`, and `run01/execution.log`. No historical seek, constructor or open fixture was replayed. Source class/table/lifetime, raw substream dispatch, universal CPU/OS compatibility, startup and gameplay remain unqualified. Ghidra annotation and shared ledger integration belong to the primary.

The real API and sparse-file setup follow Microsoft's [GetFileSize contract](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize) and [FSCTL_SET_SPARSE contract](https://learn.microsoft.com/en-us/windows/win32/api/winioctl/ni-winioctl-fsctl_set_sparse); the ABI conclusions above come from the complete local instruction sequence and fixture, not from assuming a C++ 64-bit return.
