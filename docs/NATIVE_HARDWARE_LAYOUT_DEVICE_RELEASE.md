# Hardware-layout device declaration release

`release_native_hardware_layout_device_resource_00b600b0` reconstructs the
complete 52-byte function `[00B600B0,00B600E4)`. The original receives the actual
hardware-layout owner in ECX and ends with RET. No semantic result is exposed.
Its descriptive name is a reconstruction hypothesis, not a recovered symbol.

The source borrows the existing writable owner and its actual COM declaration
pointer at `+40h`. The hardware-layout payload is `44h` bytes, with raw-pool slot
metadata at `+44h`; no separate owner or COM provider is created by this API.

## Captured and current pointers

| Native instructions | Required behavior |
| --- | --- |
| `B600B4..B600B9` | Capture initial owner `+40h` in EDI; test that captured pointer. |
| `B600BB..B600C1` | For nonnull capture, read its current table and AddRef slot `+04h`, then call with that same declaration. |
| `B600C3..B600C9` | Reload the captured declaration's table and Release slot `+08h`; call Release on the captured declaration even if AddRef changed owner `+40h`. |
| `B600CB..B600D0` | Independently reread the current owner `+40h`, including after an initially null capture, and test the new value. |
| `B600D2..B600D8` | If current pointer is nonnull, read its current table and Release slot, then release that current declaration. |
| `B600DA..B600E3` | Clear actual owner `+40h` only after that final call returns, then restore EDI/ESI and return. |

The source uses the actual Win32 COM reference-call ABI and explicit DWORD
reads, following the repository's actual-storage COM dispatch convention.
Explicit reads preserve a changed table after AddRef and changed owner fields
after either captured call. The final clear overwrites a pointer installed by
the final Release callback. A current null pointer skips both that call and the
clear. AddRef and Release counts do not gate the operation.

The function has no local exception handler or cleanup map. The source adds no
`noexcept`, catch, rollback, retry or cleanup guard. If a call does not return,
later calls and the final clear are not performed. This packet does not execute
artificial throwing COM methods.

The owner profile, retained CPU declarations, descriptor metadata, renderer,
hardware-layout tree and raw-pool slot are untouched. This is the device-resource
release operation used by device recreation. Full owner destruction remains the
separate existing `B60700` operation and its dependencies.

## Complete original-code comparison

A fresh guarded Ghidra query verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before reading the entry. All 52 bytes and all
eight original instruction seeds matched the installed executable, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete body has 25 instructions and no internal direct-call dependency.
Its private reference image executes unchanged: **zero instruction patches,
operand relocations or native entry adapters**. Both final code postimages
match the original 52 bytes.

The source side links the actual integrated `bsp_core.lib` produced by this
worktree's strict build, frozen as `local/hardware_release_fixture/bsp_core.lib`,
SHA-256 `a43c055e80a0c0f3135f97827f2331c7e4260e4288fdb7f4aa7f77c3a0498e5a`.
The link map resolves the function to
`bsp_core:native_hardware_layout_device_release.obj`. Fixture compilation does
not recompile the production source or substitute its symbol.

The fixture creates an isolated hidden window, real D3D9 HAL device and distinct
real `IDirect3DVertexDeclaration9` objects. The observed adapter was NVIDIA
GeForce RTX 5090. Actual `GetDeclaration` round trips verify the declaration
contents before testing. Each declaration retains a fixture reference so the
observation can safely inspect it after a tested Release.

Observers replace only the actual objects' table pointers with private copies.
Every reached AddRef/Release observer restores the genuine table, immediately
forwards exactly one call to the saved Windows implementation, then restores
the observed table. Real reference counts are recorded. No fake COM object or
production provider supplies the reference behavior. Runtime entry captures
verify the genuine creation and reference methods in the 32-bit Windows
`d3d9.dll`, including 16 actual code bytes at each recorded entry and only normal
loader relocations when matching disk. Thirty module-entry records also pin
the fixture image used to resolve ASLR-adjusted source PCs.

One focused three-case sequence covers the distinct branch and reload risks:

| Case | Observed calls and owner behavior |
| --- | --- |
| Initial null | No COM calls and no owner stores. |
| Captured/table/current changes | Initial A AddRef returns 3; its observer changes owner to B and A's Release table. The pair still Releases captured A through its new table, returning 2; that observer changes owner to C. Final current C Release returns 1 and changes owner back to B. Only after returning does the original/source function clear owner `+40h`. |
| Pair leaves current null | The same captured A AddRef/Release pair runs, but its second observer clears owner `+40h`. Final Release and the production clear are both skipped. |

The owner page is protected read-only during each call. A vectored exception
observer executes each actual store once and single-steps before taking a full
`48h` owner snapshot. It distinguishes fixture callback mutations from the
production store. Both modes observe five callback mutations and exactly one
production clear; that clear is at native `B600DA` or the actual compiled store
inside the mapped library function. All five native call return PCs and all
five source call return PCs are checked separately. Every snapshot preserves
all owner words except `+40h`, including raw-pool metadata at `+44h`.

## Validation and limits

`./scripts/build.ps1` passed the strict MSVC Win32 build and both existing CTest
checks, `reconstructed_math` and `native_math_differential`. An ignored local
CMake include registered this source in the worktree build; no shared build
file was edited by this packet. The fixture itself also compiled with
`/W4 /WX /O2 /Oy- /MD /EHsc /fp:strict`.

Both real D3D9 process runs passed and match **759 DWORDs / 3,036 bytes** across
all three cases. This includes five real COM reference calls, six observed
owner writes and all intermediate states. Only actual declaration pointer
identities in owner `+40h` are normalized; reference counts and all remaining
owner data are exact. Fixture references are finally released through the real
runtime after observation, with zero final counts checked.

The audit pins current source, integrated library, native bytes, compiler/map,
runtime module entries, raw and decoded traces, and the successful build log.
No permanent test suite or target was added. The primary integrated CMake
registration and repeated both real D3D9 runs against its frozen main library,
SHA-256 `73e9e8ce26ca2b6e1bcb2334bf422285239ebe889cefc4f2ba5d3c911e7a22af`.
It rechecked all 37 worker pins, current source, fresh original52 bytes, actual
call/store PCs, both complete code postimages and 30 runtime module entries.
The same759 DWORD comparison and both existing CTests passed. Saved Ghidra
annotations preserve the prior name/comments, and the complete reconstruction
record and refreshed export are registered.

In the primary native-side run, Windows resolved Direct3DCreate9 through its
32-bit apphelp.dll compatibility shim. That actual16-byte entry and PE identity
matched the installed DLL with normal loader relocations only. The host-side
factory and every tested declaration creation/AddRef/Release method in both
runs resolved to actual d3d9.dll. This factory-only distinction is recorded
in the primary audit; the method provenance checks remain unchanged.

This establishes the complete function's source behavior and a bounded native
instruction comparison with real D3D9 objects. It does not establish full device
recreation, rendering, actual game integration, arbitrary COM exceptions,
concurrent owner mutations, or original binary replacement ABI compatibility.
