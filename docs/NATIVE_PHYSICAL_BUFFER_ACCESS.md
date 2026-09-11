# Native physical buffer access

This packet implements six complete operations on actual physical vertex/index
buffer storage. Each independently named MSVC Win32 library function contains
the original instruction sequence, with no code relocation, forwarding alias,
semantic wrapper or callback adapter. Names remain reconstruction hypotheses.

| Native entry | End exclusive | Bytes | C++ entry |
| --- | --- | ---: | --- |
| `B23270` | `B232A4` | 52 | `release_native_physical_vertex_buffer_for_reset_00b23270` |
| `B23180` | `B231B4` | 52 | `release_native_physical_index_buffer_for_reset_00b23180` |
| `B4B800` | `B4B804` | 4 | `native_physical_index_buffer_capacity_00b4b800` |
| `B4B9B0` | `B4B9B4` | 4 | `native_physical_vertex_buffer_capacity_00b4b9b0` |
| `B4B820` | `B4B838` | 24 | `unlock_native_physical_index_buffer_00b4b820` |
| `B4B9D0` | `B4B9E8` | 24 | `unlock_native_physical_vertex_buffer_00b4b9d0` |

Declarations are in `include/bsp/native_physical_buffer_access.hpp`; independent
bodies are in `src/native_physical_buffer_access.cpp`. Original inputs are ECX
pointing to a readable/writable actual 2Ch physical owner and no stack arguments;
all six use plain RET. Explicit `__fastcall` places the single host argument in
ECX. Capacity returns EAX and is noexcept; the other four expose no stable result
contract and remain throwable. Native EBX, ESI, EDI, EBP and stack balance survive
normal returns. The functions have no local FH3 or cleanup state.

Private and pooled physical profiles share capacity DWORD `+18`, lock depth
DWORD `+20`, and current COM pointer `+28`. The routines do not interpret the
owner's table or construct/destroy the owner. They do not reset cursor, flags,
diagnostic arrays, reference count, capacity, or pool bookkeeping.

Both release entries capture current `owner+28`. A nonnull captured object gets
AddRef from its current COM table slot `+04`; its table is then reloaded for
Release `+08`. That pair stays on the captured object even if AddRef changes the
owner's COM field. After the pair, the function reloads `owner+28`, captures that
object's current table and performs its Release. Only after this final call
returns does it clear `owner+28`. A callback's replacement pointer can therefore
be overwritten by the final clear. A throwing call prevents subsequent calls
and stores; there is no automatic cleanup or retry.

Each capacity entry loads the raw DWORD at `owner+18`, preserving every bit.
Each Unlock entry captures current COM `+28`; null returns without changing
depth. Nonnull selects stdcall Unlock from its current table slot `+30`, ignores
the HRESULT and subtracts one from the then-current `owner+20`. This is a wrapped
DWORD decrement and sees any callback mutation. A throwing Unlock prevents it.
COM owner replacement during Unlock survives because the function never clears
the COM field. SDK 10.0.26100.0 pins the x86 IUnknown/IDirect3DVertexBuffer9/
IDirect3DIndexBuffer9 slot and stdcall contracts used here.

## Verification

The immutable predecessor discovery is `78212f9`, documented in
`docs/NATIVE_BUFFER_DEVICE_RESET_NEXT.md`. This implementation separately
recaptured all six complete spans through guarded Ghidra queries and compared
them with the installed PE: 160 bytes, all equal. Project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base `00400000` were verified
before every live query. This worker did not annotate or change Ghidra.

Private `local/physical_buffer_access_sources.cmake` registers the owned source
through CMake's existing deferred mechanism. `scripts/build.ps1` completed the
strict MSVC Win32 Release build and both existing CTests after all eight native
seed spans were verified. No tracked CMake file or permanent test was changed.

The ignored focused fixture links a byte-for-byte frozen copy of that build's
`bsp_core.lib`; it never recompiles the owned source. Its map identifies all six
symbols in `bsp_core.linked:native_physical_buffer_access.obj`, with distinct
addresses (`/OPT:NOICF`). The audit parses each independently named COFF symbol,
checks its full body and relocation records, and checks the linked executable's
body. The live fixture also compares each library body before executing it.

The original installed PE spans are copied unchanged to a separate page, which
is then made read/execute. All branches remain inside their respective bodies;
calls are indirect COM calls. No native code bridge, address patch, relocation,
external game helper or exception-handler admission workaround is necessary.
The fixture verifies all original and library code postimages after the run.

One bounded fixture exercises seven states through both the vertex and index
functions: raw capacity; null release; a release chain that changes COM table
and owner pointers; final Release throwing after replacing the owner pointer;
null Unlock; E_FAIL Unlock changing current depth to zero before decrement;
and throwing Unlock. Its deterministic endpoints use the actual x86 stdcall COM
slots and receive the actual selected COM object. They are fixture endpoints;
production functions call the owner's current COM directly.

Both paths reuse the same actual owner/COM storage addresses, so complete owner
words, COM metadata and event frames compare literally without pointer-role
normalization. All 14 paired cases match: 1,600 DWORD observations and 64 event
frames. Ten normal-return invocations per implementation also check EBX, ESI,
EDI, EBP and ESP; the four exception cases propagate to the outer fixture catch
with the native post-call field mutations and absence of later stores preserved.
The comparison records EAX as an observation without extending the release or
Unlock API contracts.

`reports/native_physical_buffer_access_audit.json` contains complete spans,
assembly, byte identity at the object/linked/runtime layers, exact symbol and
link provenance, fixture trace checks, build/seed results and artifact hashes.
This establishes these six machine-entry contracts and their bounded COM
behavior. It does not establish full subsystem replacement, successful D3D9
device reset, GPU behavior, game compatibility or visual parity. Physical Lock,
Attach, COM getters, owner lifetime and logical save/restore remain separate
providers; existing semantic `D3D9BufferBinding` code is unchanged.

## Primary integration

The primary registered the source in CMake, completed the strict Win32 build
and passed both existing CTests. It independently verified 39 worker artifacts,
two SDK pins, current source and six fresh live-Ghidra/PE spans (160 bytes).
The unchanged fixture linked the completed main library, SHA-256
`7fd95f1a1148ec3d57f8ad48eca5fef33b8ba1f56fc30e63264ea2b5b0ed7030`. All six independent
COFF, linked and runtime bodies exactly match the complete native bytes, with
zero code changes, bridges or relocations. The same 14 paired scenarios,
1,600 literal DWORDs and 64 event frames agree. The fixture uses deterministic
stdcall COM endpoints; this remains distinct from real D3D9 validation.
Saved Ghidra annotations preserve previous names/comments; all six complete
records and refreshed exports are registered. No permanent tests were added.
