# Native renderer device startup

Addresses: `00B2AEB0`, `00B49940`, `00B49950`.

This packet reconstructs all three normal bodies over the caller's actual raw
renderer, pools and current concrete providers. The C++ interfaces add borrowed
contexts and a reference to the ten live DWORD argument slots. They are not
drop-in native calling conventions. Descriptive names remain hypotheses.

| Entry | Inclusive body | Bytes | Original ABI | Coverage |
|---|---|---:|---|---|
| B2AEB0 | B2AEB0..B2B1F1 | 834 | ECX renderer; ten DWORD stack slots; RET28; void | complete normal body |
| B49940 | B49940..B4994B | 12 | ECX raw index slot; no stack arguments; RET | complete |
| B49950 | B49950..B4995B | 12 | ECX raw vertex slot; no stack arguments; RET | complete |

Normal-body total: **858 bytes**. The raw EH handler and metadata below are
dependencies, not additional reconstructed functions in this packet.

## Actual storage and providers

`NativeRendererDeviceStartupSlots` occupies exactly 40 bytes: window, fullscreen,
width, height, format, backbuffer count, multisample, depth format, sync and
fullscreen refresh. Fullscreen is tested through its low byte only. Sync uses
bit 0 of the full word: `(~(sync << 31)) & 80000000`. The source borrows the live
slots so late fullscreen/multisample and x87 reads remain separate accesses.

`NativeRendererDeviceStartupContext` borrows Reset, default-surface and physical
owner contexts. Its mutable thread reference MUST name the same actual cell
that Reset borrows as a const volatile reference. The default-surface pool must
already be the bound canonical pool. Its synchronization, physical owner,
publication, resource-support and lifetime domains must be the same instances
used by Reset/recreation; no duplicate renderer, thread, pool or publication is
created. Renderer D5F0A8 and pooled D61E58/D61E7C current profiles are the admitted
domain. The profile arrays come from the existing Reset/recreation context.

The original parent itself produces the presentation/dynamic fields. Existing
B4BBB0/B4BB60 produce the 2Ch physical owner layouts and pooled profile tokens.
B49500 consumes the trailing slab index at slot+2C, outside that owner. Existing
headers and source were checked before using those offsets. Complete renderer
and pool construction remain separate dependencies of the caller.

| Call sites in B2AEB0 | Native callee | Current actual provider or API |
|---|---|---|
| B2AEDC | BF79F0 | byte clear, 38h bytes; cdecl ADD ESP,Ch at B2AF2C |
| B2AFB2 | D3D9 table+38 | GetDeviceCaps, real current COM, four DWORDs/RET10 |
| B2AFF9 | D3D9 table+40 | CreateDevice, seven DWORDs/RET1C, actual writable outputs |
| B2B01A / B2B026 | CE223C / CE237C imports | real GetCurrentThreadId / timeBeginPeriod(1), RET / RET4 |
| B2B042 | B24460 | actual cached render state A1, two slots/RET8 |
| B2B049 | B238D0 | full actual default-surface capture, RET |
| B2B050 | B26170 | full actual default-state initialization, RET |
| B2B05B | B24A40 | stream frequencies 0..3, value 1, RET8 |
| B2B06C / B2B0EC | B4B360 / B4B350 | actual initialized vertex/index pool acquisition, RET |
| B2B084 / B2B102 | B4BBB0 / B4BB60 | pooled 2Ch constructors, EAX owner, RET |
| B2B0BD / B2B13C | device table+68 / +6C | real CreateVertexBuffer / CreateIndexBuffer, seven slots/RET1C |
| B2B0D9 / B2B158 | current pooled profile+14 | full B4C370 / B4C250 attachment, three slots/RET0C |
| B2B0E5 / B2B164 | current temporary COM table+08 | Release, one slot/RET4 |
| B2B1C8 | current renderer profile+F0 | full B21960 with actual outgoing float slot, RET4 |
| B2B1D2 | B2ABD0 | complete actual Reset provider, RET |

The two return thunks call B49500 at B49946/B49956, selecting the actual
0108FDA8/0108FDE0 pools. Its stack raw slot is consumed by RET4; each thunk ends
with RET. Their only live code xrefs are the two cleanup actions below. The
startup parent's live xref is D5F0AC (renderer profile+4); no direct caller was
reported. This does not establish every indirect caller's admission contract.
All direct callees' bodies and existing provider source contracts were reviewed;
numeric call rows and their containing functions are in the JSON report.

## Ordering and floating point

The source preserves the interleaved presentation writes after the 38h clear.
Width/height caches +1A20/+1A24 publish before GetDeviceCaps. The caps structure
is uninitialized output storage, and both caps/CreateDevice HRESULTs are ignored.
DevCaps bit10000 and the **low 16 bits** of VertexShaderVersion >=0101 choose
hardware40 or software20; MULTITHREADED4 is always included. Device output +1A10
is passed directly even when nonempty. No extra rollback or error guard is added.

CreateDevice always initially sees Windowed=1. Afterward the native order writes
lost+1D8A=0, ready+1D8B=1, stored Windowed from fullscreen's low byte, then the real
thread cell, then timeBeginPeriod(1). A1 uses the original multisample slot, even
when D3D has modified the actual presentation block. Default surfaces, defaults
and four frequencies follow.

Each allocated wrapper publishes to +1974/+1978 before the current-device
CreateBuffer call. VB is 16 MiB, IB is 1 MiB/INDEX16; both usage208, DEFAULT pool,
null shared handle. Local writable COM outputs start at zero. Current wrapper
and current +14 attachment target are captured after creation; flags1000 and
capacity are passed to the full attachment provider. The temporary output cell
is reloaded after attachment and its current COM Release is called. Failures
after construction do not return the slot or undo prior publication.

The x87 helper retains FILD of the original width/height slots, separate sign
reads, conditional FADD of actual CE3978 bits4F800000 (2^32), FDIVP and comparison
with CF5750 bits3FF5555560000000. The JBE branch includes unordered comparisons.
Ready+1D8C publishes between the width sign test and correction branch. The
current renderer profile is captured before FLDZ/FSTP; the outgoing float word
is spilled, aspect byte+1A14 publishes, and only then is current profile+F0 read.
The actual gamma provider receives that live outgoing word by reference.

B2AF41 sets EBX=1. The full listing has no later EBX/BL write before B2B1CC:
the final pending store is **1**, followed by complete Reset at B2B1D2. Reset may
leave that request pending or consume it according to its actual current state.

## Exception evidence

| Native dependency | Inclusive span | Interpretation |
|---|---|---|
| Handler (initially undefined) | CBD436..CBD43F (10 bytes) | MOV EAX,DF5C8C; JMP BF6B43; integrator subsequently defined FUN_00cbd436 |
| FuncInfo | DF5C8C..DF5CAF (36 bytes) | magic19930522, two states, map DF5C7C |
| Unwind map | DF5C7C..DF5C8B (16 bytes) | (-1,CBD420), (-1,CBD42B) |
| Existing vertex cleanup | CBD420..CBD42A (11 bytes) | read saved slot [EBP-144], tail B49950 |
| Existing index cleanup | CBD42B..CBD435 (11 bytes) | read saved slot [EBP-144], tail B49940 |

Each state covers only its corresponding pooled constructor; B2B0B2/B2B131
disarm before the real COM calls. Source cleanup scopes reflect that boundary.
The current constructors are noexcept and contain no throwing calls. No ordinary
constructor exception was forced; original hardware-fault/SEH and private stack
alias identity are not claimed. This worker performed no Ghidra writes. Root subsequently defined and saved the
exact handler as FUN_00cbd436; a fresh read-only proto check confirms its body.
See root reports/native_renderer_ay_analysis_sync.json. The initial missing-function
observation remains capture history and does not change normal-body counts.

## Verification

Strict owned-source MSVC Win32 `/W4 /WX /O2 /MD /fp:strict /EHsc` compilation
passed. `scripts/build.ps1` passed and, after eight verified native seeds, both
existing CTests passed. CMake registration is deliberately reserved for the
integrator: the repository build covers the baseline dependencies; the strict
compile and explicit-source fixture link cover this new translation unit.

External fixture: `C:/Users/sqz269/bsp-ay-device-startup`. It copied the AX
environment/helpers without modifying them. All original code runs in its own
loader-reserved image section; no foreign allocation was removed. Nine live
Ghidra/disk spans, **1,026 bytes**, match: all 858 owned bytes plus EH, literals
and pooled profiles. Five explicit absolute operands relocate the handler,
thread/pending cells and two pools. The full parent bytes otherwise remain
unchanged; direct native calls bridge to current concrete providers. Raw Reset
and its EH bytes are also retained from AX.

The final replay passed **9 original/source pairs**, **71,424 normalized state
bytes** and **8,044 event DWORDs**, with source, all three libraries and fixture
inputs unchanged before/after. Real NVIDIA RTX 5090 HAL CreateDevice and the
16 MiB VB/1 MiB IB run in all 18 parent calls. Case8 adds one real Reset and three
real 100 ms Sleeps in each version, observing pending1 before Reset and pending0
afterward. The platform focus result is instrumented in this private process;
its hidden window is never shown/focused, and foreground/focus are checked
unchanged around every parent. Every timer request is balanced by harness cleanup.

Cases0..7 cover ordinary/above-threshold/unsigned/zero/NaN/infinite aspect inputs,
fullscreen low-byte and sync-bit differences, synchronization mode1, and caps
selection. Case1 calls real successful caps/CreateDevice but substitutes E_FAIL
results; this is ignored-HRESULT instrumentation, not real failed-device recovery.
For invalid/huge dimensions or multisample inputs, the fixture records the raw
parameters then supplies valid 64x48 parameters to the real API. Gamma takes its
equal-zero skip path; no display gamma change is induced. Real buffer-failure,
constructor EH, concurrent startup, full application startup and gameplay are
not established. Source and original normal paths both use actual current
surface/pool/owner/attachment/Reset providers, not semantic startup hosts.

`verified-8case.zip` preserves the earlier eight-case evidence independently.
`verified-replay.json`, `verified-replay.log`, `live_spans.json`, `native_pins.json`
and the final capture record exact inputs, hashes, operand edits and observations.
