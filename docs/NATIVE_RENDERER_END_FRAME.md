# Actual renderer EndFrame bindings

This packet supplies the full `00B2D8E0..00B2DBCC` call schedule, its default and
save-header wrappers, both actual physical-buffer rewinds, and their two concrete
logical-stream leaves. It does **not** close the EndFrame dependency graph.
Queue execution, three overlay bodies and current query polling remain required
substantive providers. No provider defaults to success or a no-op.

Names are descriptive hypotheses. Evidence was read from the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and compared with
the installed PE. This packet makes no Ghidra mutations.

| Native entry | Coverage and original ABI |
| --- | --- |
| `00B2D8E0` | 749 bytes; ECX actual renderer, one stack save-header word, RET4 |
| `00B2F4A0` | 8 bytes; PUSH0, CALL B2D8E0, RET |
| `00B2F4B0` | 5 bytes; JMP B2D8E0, forwarding its stack argument |
| `00B232B0` | 162 bytes; ECX actual physical vertex buffer, RET |
| `00B231C0` | 162 bytes; ECX actual physical index buffer, RET |
| `00B48D40` | 8 bytes; ECX actual logical vertex, write +5C=FFFFFFFF, RET |
| `00B48DD0` | 8 bytes; ECX actual logical index, write +0C=FFFFFFFF, RET |

The two leaves use naked MSVC Win32 entries and preserve their original bytes.
The larger bodies have new C++ interfaces with borrowed contexts; they are not
binary replacements for native ECX/stack/FH3 callers.

## Publications, dispatch and provider frontier

`NativeRendererEndFrameContext` borrows the same actual renderer, AA0 queue
manager/getter and F8D440 queue publication, binding/cache ownership domains,
string pool, surface-save import, viewport-clear constants, F8D394 renderer
publication, synchronization cells, D4CC in-EndFrame byte, E1306C clear byte,
D4B9 Present-loss byte and 0108FE88 counter publication. It creates no manager,
registry, publication, owner map, lock or alternative cache. These contexts and
the entire actual owner graph must outlive each call and remain mutually
compatible. Existing contexts retain their own lifetime and failure restrictions.
The shared 01090AB0 clock and 0108D4B8 device-ready cell are not read by this
native body; its dependencies must continue using the application's same cells.

Every renderer virtual dispatch rereads the actual profile and the borrowed
numeric D5F0A8 table at that site. Supported slots are +08 B21430, +2C B1FE20,
+C4 B28D00, +110 B23C50, +130 B24710, +134 B24840 and +138 B24B00.
The numeric original table is data, never a callable address in the rebuilt
image. Unsupported profiles or slots throw an explicit binding-domain error.
Device EndScene and Present instead execute their current real COM table slots.

`NativeRendererEndFrameRemaining` requires these actual bodies:

- B1EBE0 execution on the actual queue returned by 004C11F0. Existing
  `RenderCommandQueue` execution is a projection and is not reinterpreted.
- B2BB90 (1765 bytes), current renderer +C4 B28D00 (966 bytes), and B2B580
  (1549 bytes). Their full resource/material/draw graphs remain external;
  their initial gates are not substituted for those bodies.
- Current query +10 dispatch for each eligible actual +19A0 array member.
  The observed D62AD0 slot is B5FCA0. Existing `D3D9OcclusionQuery` is a
  projection without the native vtable/intrusive prefix. The provider must
  perform the current dispatch and ignore no required operation; parent ignores
  AL and adds no retry, wait or local lost-device gate.

`NativeXLiveRenderImport` resolves ordinal 5002 from the already loaded
`XLiveLibrary::module_handle()`. The PE import record identifies CE25D8 as
`xlive.dll` ordinal 5002; C2F1CC is its six-byte indirect JMP. B2DAA2 supplies
no arguments and ignores EAX, so the typed host binding is no-argument stdcall.
The library must outlive the adapter and all calls. Missing module/export is an
explicit error; no loader, SDK initialization or replacement DLL is provided.
C2F1CC is dependency evidence, not an additional claimed reconstruction address.

## Native schedule and failure state

An inactive DWORD +1998 returns without touching context. For an active frame,
D4CC triggers DebugBreak when already set. The mode comparison precedes the
D4CC=1 write. Optional guard entry saves the actual owner before entering;
its sole cleanup state arms only after entry. Queue execution is followed by
20 texture unbinds, four distinct stream unbinds, and index unbind.

The vertex rewind runs under the captured embedded section at renderer+19F4,
with its tracked +18 depth incremented. That section has **no EH cleanup** in
the native function. Its decrement/Leave occurs only after the rewind returns.
The index rewind then runs without that extra section. Both child rewinds use
the current F8D394 renderer only when entry mode is enabled, read the signed
initial count before arming their sole cleanup state, and reread array/count
and current child profile on each iteration. D61D6C/+08 binds B48D40 and
D61DE0/+08 binds B48DD0. Only after all children return are +1C and then +24
cleared; unrelated fields, including physical +20 depth and +28 COM owner,
are untouched. Earlier child writes and uncleared final counters survive a
later provider failure. Common implementation shares the two full loop bodies.

The parent then consumes its save-header argument, calls optional surface save,
the three overlay providers, and sixteen cached-state writes. Duplicate state
1A and 34 writes remain in their exact positions. XLiveRender precedes optional
EndScene. Cache clear precedes statistics publication. The initial unsigned
pending count comparison precedes +1998=0; subsequent iterations reread the
array and current unsigned count. Only query state +08=0 invokes current +10.

Optional Clear sends count0, null rectangles, flags1, zero ARGB, depth1 and
stencil0, then clears E1306C. Its original FLD1/FSTP produces exact 3F800000;
source sends that exact float through the existing substantive clear provider.
Original scratch stack bytes and x87 stack overflow/status behavior are not
claimed by this C++ interface. The source likewise does not preserve an alias
to the original caller's stack save-header slot.

Both EndScene and Present recheck current +1D90 and byte +1D8A. Present also
executes the current +2C active getter and, only if AL=0, calls current device
+44 with four null arguments. Only exact HRESULT 88760868 sets D4B9=1.
Current 0108FE88 owner+08 is cleared, renderer+14 increments with DWORD wrap,
the current mode is captured, D4CC clears, the guard disarms, and normal leave
uses the saved renderer and raw saved guard word. Normal/exception leave reads
the current mode and current renderer+04 lock through existing actual providers.
Skipped entry followed by enabled exit remains outside the valid native domain.

The three FH3 FuncInfo records (DF5E24, DF547C, DF5450) each have maxState1.
Their only map entries lead to CBD580, CBCE90, CBCE70 respectively, each
`LEA ECX,[EBP-14]; JMP B21110`. This proves the optional guard is the only
registered cleanup, including the parent's lack of embedded-section cleanup.
Source uses the existing guard cleanup policy, including termination for a
second C++ exception. It does not add rollback of D4CC, cache writes or counters.
Original FH3 stack metadata, asynchronous SEH and unrestricted reentrancy remain
unproved. A failure inside parent vertex rewind can leave the embedded lock
held; callers must not silently treat the parent as a transaction.

## Evidence and validation limits

Seventeen complete live/disk spans pin all seven bodies, three numeric profiles,
the import thunk and three FH3 handler/map pairs. Saved assembly/pseudocode and
live prototypes are retained separately. The live default prototype has no
parameters and decompilation misidentifies the save-header stack word as an EH
local; the actual MOV at B2D9C6 and RET4 establish the argument contract.
No no-return repair was needed: the saved body reaches B2DBCA RET4. The six
bytes D96A..D96F and three bytes DAED..DAEF are skipped padding, not lost tails.
The parent's complete range is established independently of B32920's truncated
saved body. No root-owned constructor coverage is inferred here.

The report records strict build, seed, two configured CTest and focused fixture
results plus immutable artifact hashes. One local probe compares relocated
original vertex/index bodies with source under real recursive Win32 critical
sections, checks byte-identical leaves, and checks source partial failure and
negative signed count. Original direct guard calls route to the same substantive
guard providers; original child virtual slots call the exact leaf providers.
Original FH3 handlers are not exercised by the fixture. Both inactive wrappers
are also checked with a null context. No active full-frame fixture or loaded
XLiveRender execution is claimed. ABI compatibility of the two leaves does not
establish parent ABI, unrestricted concurrency, rendering or gameplay parity.
