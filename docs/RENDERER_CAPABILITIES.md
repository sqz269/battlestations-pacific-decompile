# D3D9 renderer startup capabilities

Packet `orch2_renderer_capabilities` reconstructs the query and decision behavior
of `00b2c8e0` and the adapter-description fragment of constructor `00b32410` in
`src/renderer_capabilities.cpp`. Names are descriptive hypotheses. The API owns
ordinary C++ state and borrows `IDirect3D9&`; it never creates, AddRefs or releases
that interface. This is not the original renderer object or a binary replacement.

## Startup contract

Use the same live interface in this order:

1. `enumerate_settings_resolutions_00b27d80(settings, settings_queries)`.
2. `query_renderer_adapter_identifier_00b32410(capabilities, api)`.
3. `gather_renderer_capabilities_00b2c8e0(capabilities, settings, api)`.
4. Later settings AA queries and device creation continue borrowing that API.

The native calls occur at `00b3285c`, `00b32874` and `00b3289b`. The first
identifier query is `GetAdapterIdentifier(0, 0, &identifier)`, and the
case-sensitive `strstr(identifier.Description, "NVIDIA")` result goes to
renderer+1D88h at `00b32892`. Description is identifier+200h, established by
the stack pointers and checked against the SDK layout. The frame-counter reset
at `00b32898` is outside this capability API. Other constructor work is also
outside it. Constructor ABI: ECX renderer, plain RET returning this; the fragment
has no separate native entry/exit. Its relevant instructions are
`00b32861..00b32892` (final store ends before `00b32898`).

The gather routine uses ECX renderer, plain RET, complete body
`[00b2c8e0,00b2d8de)`. It calls `GetDeviceCaps(0, HAL)` at `00b2c91a`, probes
ATOC, and calls `GetAdapterIdentifier(0,0)` again at `00b2c9bb`. The second
description is classified by case-sensitive substring matches in order:
`8800`, `8600`, `8200`, `ATI`. Any match sets renderer+1D89h; no match clears it.
Both identifier records are retained separately as query evidence.

## Device-cap field mapping

The native caps buffer starts at the post-prologue ESP+1F4h. Assembly was needed
because the decompiler splits that buffer and ignores transient COM argument
pushes. SDK field offsets are compile-time assertions in the implementation.

| Renderer offset | Established source/rule |
| --- | --- |
| 1B18h, 1B1Ch, 1B20h | MaxTextureWidth, MaxTextureHeight, MaxVolumeExtent |
| 1B24h, 1B28h | MaxAnisotropy, MaxSimultaneousTextures |
| 1B2Ch, 1B30h | Packed PS <=0104h: 4/false; exactly 0200h or 0300h: 8/true; other values retain prior fields |
| 1B40h | LOWORD(PixelShaderVersion), also settings record+28h |
| 1B44h | LOWORD(VertexShaderVersion), overridden with 0101h for software VP |
| 1B48h | Packed PS <0200h: 1; otherwise 2, also settings shader ceiling |
| 1B4Ch | MaxVertexShaderConst |
| 1B50h | Always false at routine completion, including when INST succeeds |
| 1B51h | MaxUserClipPlanes != 0 |
| 1B52h | DevCaps2 & 1, stream-offset support |
| 1B53h, 1B54h | Caps2 & 20000h (fullscreen gamma), Caps2 & 100000h (calibration) |
| 1B55h | ATOC probe success sets true; failure leaves prior value |
| 1B74h | DevCaps hardware transform/light bit 10000h absent OR packed VS <0101h |

The consumer meaning of 1B2Ch/1B30h remains provisional; the arithmetic and
assignments are established. Constructor zero writes for the mapped scalar
fields and vectors are visible at `00b32657..00b326f5`. Retained raw query
records and their validity flags are C++ additions, not invented native fields.

The declaration-type vector at renderer+1B5Ch appends 0,1,2,3,4, followed by
5,8,9,10,11,12,13,14,15 for set `D3DCAPS9.DeclTypes` bits 0..8, in that order.
The source is caps+ECh (stack+2E0h), not one of the nearby adapter-group fields.
The vector is not cleared on a repeated gather.

## Format probes

All calls use adapter 0, HAL and adapter format X8R8G8B8 (16h). Only HRESULT
exactly zero counts as support, including for all auxiliary probes.

- ATOC (`434F5441h`) uses usage 0 and resource type SURFACE (1).
- INST (`54534E49h`) has the same arguments and is queried only below PS 0300h.
  Native may set 1B50h at `00b2ce9d`, but clears it unconditionally at
  `00b2ceae`; the implementation retains the conditional query and final false.
- The table built at `00b2ceba..00b2d70f` contains exactly 57 stride-8 entries.
  Their exact order and four metadata bytes are recorded in the JSON report.
  Only resource type TEXTURE (3) is iterated: ESI starts at 3 and the loop
  continues only while ESI <=3.
- Each table entry first probes usage 0. Only success proceeds to conditional
  usage 1 (render target), 2 (depth stencil), and 200h (dynamic) probes, in that
  order. The corresponding first three metadata bytes select those calls.
  The fourth metadata byte is initialized but never read in this routine.
- Every supported format then probes usage 80001h (render target plus
  QUERY_POSTPIXELSHADER_BLENDING), regardless of table metadata, and appends a
  native stride-12 record under resource-type index 3 at renderer+1B68h.
  Existing entries are retained. No cube/volume-format support is fabricated.

Native record+4 holds the three auxiliary result bytes and a known zero fourth
byte. Native record+8 initializes only its low result byte, then copies the
whole DWORD from the stack; its upper three bytes are indeterminate. The C++
record exposes only the established booleans. It does not claim native layout.

## Adaptations and remaining bindings

Native ignores GetDeviceCaps/GetAdapterIdentifier HRESULTs and reads their
uninitialized output buffers on failure. This API returns negative HRESULTs
and keeps both capability and settings output state unchanged. A successful
identifier with no NUL in its fixed Description array returns E_INVALIDARG.
This explicit boundary prevents undefined stack/string reads, with no fallback
hardware caps. Format-probe failures remain ordinary unsupported results and
do not abort gathering. Device-cap success with a positive HRESULT is accepted;
strict equality applies to format probes, as established by their TEST/JNZ.

Native sized-string copy/free and vector reserve/grow helpers `0041dd40`,
`00419cc0`, `00bd1510`, `00b236b0`, `00b2ae20` and `00b22b30` are represented
by bounded description reads and C++ vectors. Their allocator failure, SEH and
native container layout are not reproduced. A valid Description match can
never have the native redundant pointer difference -1, so ordinary substring
presence preserves that branch. This packet does not reconstruct the renderer
vtable/object, the remaining constructor or renderer feature consumers.

The C++ gather copies current state into a temporary before its first format
probe, appends and updates that temporary, then publishes on successful
completion. Native stores fields and appends vectors incrementally. This defers
publication observable through reentrant query callbacks and leaves outputs
unchanged if a C++ allocation throws. It changes allocation order and failure/EH
behavior; successful result equivalence is separate from those native contracts.

The startup owner must retain both the shared COM interface and these output
states, check HRESULTs, and pass the settings state to the existing settings
binding. Do not run the older settings-only gather as an additional caps query.
The later `00b2aeb0` device prefix has its own native GetDeviceCaps call and
should retain it; sharing the COM owner does not remove that observed query.

## Evidence and verification

Live Ghidra reads and exports used `bsp.py`'s verified `bsp` project and
`/battlestationspacific.exe` target from `C:/Users/sqz269/bsp.gpr`. No Ghidra
mutations were performed by the worker. RET is present at `00b2d8dd`. The two
listing gaps `00b2d71d..00b2d71f` and `00b2d726..00b2d72f` are jump-over LEA
padding; live bytes and flow inspection found no missing reachable block.

One ignored Win32 fixture derives its expected format table from the exported
assembly and compares the complete D3D call trace. It checks strict HRESULT-zero
format acceptance, mapped limits/flags, declaration selection, append and sticky
flag behavior, unusual shader versions, software VP and failed-query retention.
It then performs the startup query sequence against a real Direct3D9 interface.
Observed adapter: NVIDIA GeForce RTX 5090; PS 0300h; settings ceiling 2;
26 resolutions; 34 supported texture formats; 10 declaration types; ATOC true;
NVIDIA description flag true; legacy-description flag false.

MSVC Win32 Release builds with the repository warning policy, and both existing
CTests (`reconstructed_math`, `native_math_differential`) pass after all eight
native seed comparisons pass. The focused fixture is API and
decision validation, not a differential execution of the complete native gather,
native ABI validation, device creation or gameplay validation. Original game
installation files were read only. Ignored evidence lives in
`local/renderer_capabilities_fixture.log`, `local/renderer_formats.json`,
`local/renderer_capabilities_build.log` and the shared function exports.
Fixture sources/runners to preserve are `local/renderer_capabilities_fixture.cpp`,
`local/renderer_capabilities_fixture/CMakeLists.txt`,
`local/build_renderer_fixture.ps1`, `local/extract_renderer_formats.py`, and
`local/renderer_formats_expected.inc`.
