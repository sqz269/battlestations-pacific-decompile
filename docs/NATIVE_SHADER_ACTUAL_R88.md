# Actual shader construction and lifetime integration (R88)

Addresses: 00B5F9B0, 00B5FAF0, 00B289A0, 00B289F0, 00B22DD0,
00B22E30, 00B5E720, 00B5E7E0, 00B3F4C0, 00B5F410, 00B5F490,
00B5F6E0, 00B5F700.

R88 brings the reviewed shader construction implementation from orch5 commit
`043652d3` into the current main lineage. Construction and destruction now
borrow the application's existing raw manager and support publication. This
closes another dependency of the unintegrated material compiler.

## Source and ABI

| Entries | Coverage | Native interface |
| --- | --- | --- |
| B5F9B0/B5FAF0 | Complete normal constructors and recovered cleanup ordering | ECX10h owner, stacked COM, EAX owner, RET4 |
| B289A0/B289F0 | Complete shared registry append bodies | ECX renderer, stacked owner, RET4 |
| B22DD0/B22E30 | Complete shared reserve bodies | ECX array, stacked signed capacity, RET4 |
| B5E720/B5E7E0 | Complete tails to BD30F0 | ECX owner, RET |
| B3F4C0 | Complete raw-pool overload; older storage overload preserved | ECX0Ch diagnostic record, RET |
| B5F410/B5F490 | Existing complete destruction bodies; new borrowed lifetime binding | ECX10h owner, RET |
| B5F6E0/B5F700 | Existing complete scalar-deletion bodies; same binding | ECX owner, stacked flags, EAX old address, RET4 |

The source keeps the current `NativeResourceSupportRawContext` and
`NativeResourceSupportLifetime`. Its raw constructor borrows AA0/AA8/AA4/FEDC
and current renderer cells; it creates no second manager or owner registry.
Both retained construction and destruction contexts accept that existing
lifetime adapter; semantic-domain aggregate callers remain accepted.
The existing resource-support implementation is unchanged. The context-taking
C++ interfaces and context layout are not original binary ABIs.

Constructors stamp base/count1 and derived profile, clear0C/08, store and AddRef
the incoming COM shader. Vertex registration precedes the two temporary names;
pixel registration follows both normal pool returns. The shared registry body
removes a prior match, grows only at count==capacity and appends the borrowed
pointer. Its raw overload introduces no diagnostic operation allocation.

The first temporary's captured length and the second temporary's captured data
control normal cleanup. Current pool lookup follows each pointer/size capture.
Copying uses overlap-safe memmove, matching native BF7680. The three recovered
states arm only after completed operations and disarm before normal cleanup.
Source C++ exceptions run armed second-name, first-name and base cleanup;
acquired COM, completed vertex registration and early support publication
survive where native cleanup leaves them. Legacy retained-operation failures
keep their previous no-replay and destructor-guard behavior.

## Current validation

The final strict Win32 build and all three existing CTests pass. Twenty fresh
live/PE spans total1,753 bytes, including all13 owned entries, registry-removal
dependencies, shader profiles, names and constructor unwind metadata. Every
Ghidra batch verifies `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`.

The prior manifested /MD probe is adapted to current borrowed contexts and
linked against the current CMake library, with no source-object override.
Pixel and vertex normal construction match copied original code using real
HAL shaders, actual pooled strings, AA0/AA8/FEDC publications, registry growth
and support/pool drain. All eight copied bodies are rechecked: only direct
CALL displacement bytes are changed to reach concrete providers.

The source-side normal cases now finish through complete flag1 scalar deletion
on heap owners. Both unregister, release the real COM hold, stamp the base and
free their raw owner; the creator COM count returns to its starting value.
This checks the new destruction lifetime binding. It does not execute original
scalar-deleter machine code or admit shader reference companions.

Two existing source-only exception cases throw from real CRT validation after
support publication. They verify ordered temporary returns, base stamp,
retained COM/vertex registration, section unlock and no repeated registration.
The legacy failed-frame/replay check passes; destruction of an unresolved frame
exits77 as expected in a separate process. No new repository tests are added.

## Limits and next dependency

Native FH3/SEH delivery and all failure sites remain unproved. Original normal
constructors share reconstructed dependencies with source. Renderer arrays are
fixture storage; this is not full renderer, shader companion or compiler graph
validation. Legacy failed diagnostic state remains alive until process exit.
The application does not yet bind this compiler path and is not rerun here.

Next, integrate the prior sampler cache and actual message/online dependencies,
then the full compiler continuation and post-effect path. Full drawing and
gameplay remain open. Current hashes, native call rows, Ghidra annotation
history and integration receipt are in `reports/native_shader_actual_r88.json`.
