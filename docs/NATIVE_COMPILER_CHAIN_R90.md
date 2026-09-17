# Concrete compiler continuation and sampler loading

Addresses: 00B3B3C0, 00B3C3A0, 00B3B280, 00B1A4F0, 00B1B4D0,
00BECB20, 00BECCD0. B1A51D..B1AA29 is also recorded as a separate
continuation fragment, with its preceding pump requirement intact.

## Result and provenance

R90 connects the existing compiler prefix to a concrete continuation and
integrates the raw platform-message, sampler-cache and descriptor-sampler
dependencies. Ten source/header files are byte-identical to reviewed orch5
commit `769bdc3e3`, including its corrected capture and string-return order.
They register through the current `cmake/startup.cmake` and use the existing
raw resource-support adapter, shader constructors and canonical owners.

| Entry | Normal source behavior and original ABI |
| --- | --- |
| B3C3A0 | Existing wrapper; ECX effect, EDX root descriptor, seven stack arguments, RET1C |
| B3B3C0 | Existing prefix plus exact B3B513/B3B536 continuation; ECX builder, three stack arguments, RET0C |
| B3B280 | Current sampler/state loops; ECX builder, pass/descriptor on stack, RET8 |
| B1A4F0 | Platform pump before first name read, then full cache continuation; ECX cache, four stack arguments, RET10 |
| B1B4D0 | Pooled lowercase name and Default options; ECX singleton, name on stack, RET4 |
| BECB20 | Raw loading/focus/online/input cursor schedule; ECX platform, loading byte on stack, RET4 |
| BECCD0 | Real Win32/XLive thread-message loop then loading cursor update; ECX platform, RET |

These names describe recovered behavior; they are not recovered source
symbols. C++ entry interfaces and retained failure frames have separate ABIs.

## Contracts retained

The compiler attaches its child to the original prefix frame before any
continuation effect. It creates actual pass/reflection/shader storage,
registers the same owners, and preserves the native release order. Cached
reflection captures current metadata before reading bytecode. Generated
source, cache writes, sampler states and shadow selection retain their
native gates and capture order. A native null result can retain orphaned
pass/COM storage; source exceptions retain diagnostic frames without
inventing rollback or replay.

Sampler loading captures the actual platform before BECCD0 and reads the
input name afterward. It preserves alias scans, current factory selectors,
date output, vector append and reference handling. Default loading returns
current option data and captured requested data using current lengths.
Descriptor loading uses current unsigned counts, stages, state lists and
builder counters. B5F100 consumes a caller-provided B-16 DWORD after replacing
only its low byte. Each successful source1 release requires a matching,
ordered native residue; missing evidence is an explicit source boundary.

The platform pump uses actual PeekMessage, XLive ordinal5030, TranslateMessage
and DispatchMessage. Its cursor path borrows the current raw online/input
cells and existing services. No implicit manager construction is introduced.

## Current validation

Strict MSVC Win32 build and all three existing CTests pass. Seven complete
native bodies total 6,774 bytes and match live Ghidra and the original PE.
Three unlisted three-byte spans are unchanged same-register LEA alignment.
There are no unresolved listing gaps. Fifty retained fixture inputs match
fresh native bytes across 35 unique spans. All 195 direct call rows pass
the live verifier; 33 indirect calls retain their separate contracts.

The current CMake library passes these retained fixtures without source
object overrides:

- Full cached B3C3A0/B3B3C0 source path: real D3D9 HAL, installed D3DX9_40
  reflection, actual VS/PS COM bytecode, hot white-texture acquisition,
  nine canonical owners, root-then-mode render states and normal builder
  cleanup. Root/mode sampler lists are empty; original B3B280 verifies the
  two unused scratch preimages. The whole original compiler is not run.
- Three original/source cache comparisons: actual null factory/date/vector
  paths, existing-resource retain, pool state and real window-message
  mutation before the first name read.
- One original/source nine-sampler schedule: source0/1-hit/1-null/2/3/unknown,
  live descriptor/list/stage/counter mutation, pooled Default loading,
  binding retention and recorded release residue. Child providers and an
  imported atomic bridge are shared between the two legs.
- Source retained-failure/replay checks and two isolated destructor guards
  returning the expected exit77. Normal fixture processes return zero.

Fixture bindings were adapted to the current shared raw support context.
An initial newline-format/old-surface-adapter compile failure is retained.
The driver now returns zero after checking an expected guard77. Final normal
sampler runs refresh their relocated images after guard runs, using the same
tested executables. No repository test cases were added.

## Open work

Generated shader-source paths, compiler-null paths, unsupported vertex
textures, full teardown and original FH3/SEH remain untested here. The
sampler/platform fixtures cover a null-online guard with real pretranslate;
nonnull raw online/input/manager/client ownership and live SDK behavior are
still open. Fixture arenas remain alive until process exit.

The application does not yet supply this concrete compiler context to its
full numeric material/post-effect graph. Next, connect the actual material
factory and resource graph, then validate installed-data construction,
drawing, teardown and gameplay. The successful cached fixture does not
establish completion of those paths.

Evidence and frozen integration receipts:
`reports/native_compiler_chain_r90.json`.
