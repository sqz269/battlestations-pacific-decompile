# Native shader device-reset storage

This packet reconstructs five complete native routines, 353 bytes, over the
caller's actual renderer, shader-owner and global storage:

| Native function | Bytes | Native ABI and operation |
|---|---:|---|
| `B1FEF0..B1FEF6` | 7 | ECX renderer, EAX borrowed device at +1A10h, RET; no AddRef |
| `B5E750..B5E7B8` | 105 | ECX pixel owner, RET; save bytecode and release current shader |
| `B5E810..B5E878` | 105 | ECX vertex owner, RET; same native storage operation |
| `B5E890..B5E8D3` | 68 | ECX pixel owner, RET; recreate shader, free current bytecode |
| `B5E8E0..B5E923` | 68 | ECX vertex owner, RET; recreate shader, free current bytecode |

The four owner operations have no semantic return value. These are new MSVC
Win32 C++ entry interfaces, not original binary replacements. Owner fields
are the actual COM pointer at +08h and bytecode pointer at +0Ch. No owner
constructor, private renderer, shadow registry or callback provider is added
to production. The genuine getter is implemented and reused directly.

## Save, query and release order

Both native save prologues begin with `PUSH ECX`. The size-query DWORD therefore
starts with the incoming **owner pointer bits**, not zero or an uninitialized
value. The source preserves this input even though a normal GetFunction size
query overwrites it. No invented result or size policy handles failed queries.

Capture owner+08h. Null skips the entire operation without reading bytecode.
On the captured shader, call AddRef and then Release, reusing that captured
object even if AddRef changes the owner's current COM field. Reload current
owner+08h and call GetFunction with null output and the actual size DWORD.

Pass the returned size directly to the existing actual CRT allocation service
corresponding to `BF55BE -> BF681B`. After allocation, capture the current
shader **before** publishing the allocated pointer at owner+0Ch. Call
GetFunction on that captured current shader, using the allocated pointer and
the same size DWORD. No HRESULT test intervenes.

Reload current owner+08h once more. If nonnull, Release that object, then clear
owner+08h even if Release changed it. A null current field skips both the
Release and the explicit clear. Bytecode remains whatever is currently stored
at +0Ch. Existing bytecode is not freed before publication. No local cleanup
frame, catch, allocation rollback or extra AddRef is introduced.

## Restore, current global and bytecode disposal

The restore APIs take the **address** of the actual four-byte renderer global
`F8D394`, rather than a captured renderer value. A nonnull COM field returns
before reading bytecode or the global. A null bytecode field also returns
before reading the global. Only the reached path loads the current renderer
from that actual global and calls the genuine `B1FEF0` device getter.

Reload current bytecode after the getter and pass it, with the actual owner+08h
output cell, to device CreatePixelShader or CreateVertexShader. Ignore HRESULT.
After that COM call, reload current owner+0Ch and free that pointer through the
actual shared CRT boundary. Clear owner+0Ch after free returns, including when
free-boundary effects changed it. Failed creation still reaches free and clear;
the native routine does not add output repair or retry.

The complete returning-free tails `B5E8C7..B5E8D0` and `B5E917..B5E920`, ten
bytes each, contain `ADD ESP,4; MOV [ESI+0Ch],0`. They are present in both fresh
Ghidra bytes and the installed PE. The primary restored both continuations
and saved the complete function bodies and refreshed listings.
Unrelated owner fields and all renderer bytes remain unmodified.

## SDK and complete native evidence

Fresh guarded queries verify `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before each read. Seven spans / 363 bytes match
the installed PE: all five complete bodies plus two external CRT thunks.
The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All code bytes decode completely, including both returning-free tails.

The installed SDK 10.0.26100.0 `shared/d3d9.h` declaration order independently
confirms both five-method shader interfaces: AddRef at +04h, Release at +08h
and GetFunction at +10h. In the 119-method device interface, index106 / +1A8h
is CreatePixelShader and index91 / +16Ch is CreateVertexShader. The real COM
calls use their Win32 stdcall stack ABI. The header and full declaration-order
evidence are hash-pinned in the audit.

## Real Direct3D original-body comparison

The ignored focused fixture creates two actual Direct3D9 HAL devices per side
on the NVIDIA GeForce RTX 5090 and compiles five distinct real `ps_2_0` and
five `vs_2_0` programs through D3DCompile. Shader creation, reference counting,
GetFunction and restoration use actual runtime COM objects. Their observed
GetFunction/Create methods resolve to the real Win32 `d3d9.dll`.

Per-object vtable observers forward every reached COM call to its original
runtime method. Controlled field changes happen around those real operations;
no fake COM method implements the behavior being compared. Extra fixture-held
COM references keep observed shaders alive and are balanced at teardown.
The production code contains none of these observers or held references.

The native fixture retains all five original bodies and their relative calls,
including the genuine getter. Only two absolute renderer-global operands are
relocated inside the restore bodies. Two external CRT entry thunks bridge to
the actual primary-library allocation service and observed actual CRT free.
Every runtime postimage is checked against exactly these changes. Caller-owned
global storage is initialized explicitly; no serialized BSS-value claim is made.

Ordinary pixel/vertex save and restore preserve the exact real shader bytecode.
Controlled runs change the current owner shader after real AddRef, GetFunction
query, allocation, bytecode copy and final Release. They prove the captured
initial reference calls, current query/copy/release targets, publication before
copy and final clear after Release. Further controlled restore runs redirect
the bytecode field after real CreateShader and after actual free, verifying
that the current pointer is freed and the final field is cleared. Unfreed
old/replaced buffers are retained until fixture teardown, matching the native
operation's absence of cleanup. These perturbations are not normal COM/CRT
behavior and are reported separately from actual calls and HRESULTs.

Two invalid programs produce genuine `D3DERR_INVALIDCALL` results, one for each
shader type; both implementations still free and clear bytecode. The blocked
restore paths execute with native global storage protected PAGE_NOACCESS and
a null source global-address argument. No external service is reached. Direct
getter calls verify both actual renderer/device bindings. No shader is drawn;
this is storage, COM and instruction evidence, not a render or gameplay check.

All reached live buffer bytes, raw owner fields and canaries, current global
identity, reference counts and COM/CRT events are compared. Only explicit
pointer identities and the verified initial owner-address size value are
normalized. Freed buffers are never read. Full traces and runtime binaries
are retained and pinned with provider maps and real module hashes.

The comparison passed with 5,925 identical DWORD trace entries (23,700 bytes),
104 storage snapshots and 169 total records. Each side performed four actual
malloc calls (128, 128, 144 and 144 bytes), six nonnull actual frees, four
AddRefs, eight Releases, eight GetFunction calls and six CreateShader calls.
The two failed creations returned the actual `8876086Ch` HRESULT. All seven
runtime postimages and seven primary-library providers passed independent
checks; the actual CRT and COM modules were verified as Win32 PE014c files.

`./scripts/build.ps1` passed with both existing CTests, and all eight native
seed spans matched disk. An ignored CMake registration include compiled the
owned source into primary `bsp_core`; the strict fixture linked that exact
library. No tracked tests were added. No production source changed after the
API freeze and successful primary build.

The primary completed permanent CMake registration and repeated the full
comparison against its frozen main library, SHA-256
`07e5f7de1881917f2ca637b16476eb43867c9648af77efbcfd798a58625de212`.
All 5,925 DWORDs, 104 snapshots, seven original runtime postimages, seven
actual library providers and real Win32 modules passed independent checks.
The primary also rechecked 38 worker pins, four current sources, installed SDK
and seven fresh live/PE spans. Both existing CTests passed. Full reconstruction
records and saved Ghidra annotations now cover all five entries; returning-free
flows at `B5E8C2` / `B5E912` are restored and exports refreshed. The existing
correct getter name and all prior comments were preserved. Descriptive shader names are hypotheses. Valid
reached storage and COM lifetimes remain caller preconditions; no original
ABI compatibility, complete renderer recreation or game validation is claimed.
