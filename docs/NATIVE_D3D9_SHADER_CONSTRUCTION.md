# Native D3D9 shader wrapper construction

Addresses: 00b5f9b0 00b5faf0 00b289a0 00b289f0 00b253e0 00b25450 00b22dd0 00b22e30

This packet reconstructs the complete normal bodies of both actual 10h shader
wrapper constructors and their six renderer registry helpers. It uses the
existing actual string pool, resource-support singleton, shared lifetime domain
and canonical `NativeRenderActualOwners` domain. No second engine reference
count, COM object, renderer or registry is created. Names are hypotheses, not
recovered symbols. These are explicit C++ interfaces, not original ABI/FH3
adapters or shader terminal lifetime implementations.

| Routine | ABI | Bytes / instructions | Coverage |
|---|---|---|---|
| B5F9B0 pixel constructor | ECX fresh10h; stack actual COM or null; EAX same; RET4 | 320 / 107 | Complete normal body |
| B5FAF0 vertex constructor | Same | 320 / 107 | Complete normal body |
| B289A0 / B289F0 register | ECX actual renderer; stack borrowed wrapper; RET4 | 73 / 26 each | Complete |
| B253E0 / B25450 remove | ECX actual0Ch array; stack pointer cell; AL found; RET4 | 103 / 46 each | Complete |
| B22DD0 / B22E30 reserve | ECX actual0Ch array; stack signed capacity; RET4 | 95 / 38 each | Complete |

The saved `C:/Users/sqz269/bsp.gpr` program `/battlestationspacific.exe` and
installed executable agree for all eight bodies, both string literals and
the registry producer fragment. The report records exact instruction counts,
all call rows, inclusive ends and hashes. The integrator repaired the original
post-free gaps in B22DD0/B22E30 before implementation; the two bodies now include
publication of data and capacity after BF6989. No worker Ghidra mutation or
new function definition was performed.

Both constructors publish CEB130, write the actual +04 count to1, publish
D62A60 (pixel) or D62A70 (vertex), then clear +0C and +08 in that order.
A nonnull supplied COM shader is stored at +08 before its CURRENT vtable+04
AddRef call. Saved EDI is literal zero across the stdcall, so the old-COM
Release sites B5FA14/B5FB54 are unreachable under the original ABI. There is
no wrapper engine retain beyond initialization of its one actual count.

Pixel registration at B5FAD4 occurs after both pooled name cleanups. Vertex
registration at B5FB5D occurs before either name allocation. Each reads the
current F8D394 renderer at that point. B289A0 uses renderer+1AC4 and B289F0
uses +1AD0. B32410 produces the two 0Ch headers with six zero stores at
B325D6..B325F9; EBX is zeroed at B32439 and not reassigned before them.

The first temporary is resized to11 (`PixelShader`) or12 (`VertexShader`).
Its length is captured for the initial copy, second resize and final cleanup.
The second copy uses current second length and current first data, then calls
the unchanged, zero-argument B3E730 support singleton. The current wrapper+08
snapshot in a neighboring stack local is not an argument to B3E730. Cleanup
returns captured second data with current second length, then current first
data with captured first length. Copies use BF7680's overlap-capable memmove
contract. The source retains both actual headers and the COM snapshot on error.

The registry helpers swap-remove only the first matching pointer, replacing
it with the last slot and decrementing current count without clearing the old
slot. Registration then grows only at count==capacity, using signed
max(DWORD(capacity*2),1), and appends the borrowed identity. Reserve clamps its
signed request to1, reloads current count/data while copying, frees old storage,
then publishes new data and capacity. No element retain, release or ownership
projection occurs. BF55BE/BF6989 are the existing allocation/free thunks.

The caller B3B3C0 allocates exactly10h before each constructor. B3BC9E passes
the actual vertex COM slot at stack+24; B3BFB6 passes the pixel slot at+28.
Each constructor consumes one stacked argument. All memmove call sites use
three cdecl arguments and ADD ESP,0Ch; allocator/free calls clean one DWORD.

Construction and reserve operations are persistent, one-shot host frames.
Failure preserves the actual partially written wrapper, COM acquisition,
name headers, published renderer entry and pending child state. The caller
keeps those owners/domains alive and excludes retirement. Diagnostic cleanup
does not undo registration, release COM or emulate original private FH3.
Canonical companions must borrow raw+04 in the existing owner domain. The
constructors never call its zero-count-only `resolve_actual` protocol.
B5F410/B5F490/B5F6E0/B5F700 terminal providers remain required externally;
pass-slot setters and terminal deletion are outside this packet.

Validation passed: default MSVC Win32 C++17 `/W4 /WX`, both existing CTests
after seed verification, and one focused original/source executable. It runs
all eight copied original bodies with only direct CALL operand relocations,
the real support singleton and string pool, original registry-header producer
stores and actual HAL-created COM shaders. Eight comparisons cover null/real
COM input, exact wrapper words and AddRef, pooled traces, registration timing,
renderer-publication mutation, registry growth and swap-last/reappend order.
Failed second allocation preserves the first name/COM and distinguishes
pixel-unpublished from vertex-published state; replay and retirement guards pass.

The fixture performs actual support/string-pool canonical shutdown. Shader
wrappers, their canonical companions and real COM holds remain in a process
arena until exit because their terminal provider is unavailable; its fail-fast
boundary is never treated as successful deletion. No shader-owner retirement,
native FH3 compatibility, draw/readback or game behavior is claimed. The report
and final local manifest pin the inputs, original/linked bytes, runners, results
and libraries needed to reproduce these construction-only observations.

Thirty numeric direct-call rows pass the live verifier; four dynamic COM sites
are documented separately. All eight linked bodies differ from original bytes
only at direct CALL operands. The +0C field remains explicitly opaque.
