# Native descriptor sampler prerequisites

Addresses: 00b3b280, 00b5f100, 00b44cf0, 00b18fb0

This packet implements three actual-storage leaves needed by B3B280. **B3B280
remains analyzed and unimplemented:** the actual singleton/cache texture loader
is still missing. The semantic `material_samplers.cpp` path is not an actual
replacement. Descriptive BSP/C++ names are hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B5F100 texture route append | ECX pass; stacked index and stage DWORD (only low byte read); RET 8 | Complete B5F100..B5F150, 81 bytes |
| B44CF0 binding append | ECX pass; stacked word, retained owner, actual name pointer; RET Ch | Complete B44CF0..B44D59, 106 bytes |
| B18FB0 effect texture name | ECX effect; stacked unsigned index and actual name pointer; RET 8 | Complete B18FB0..B18FFF, 80 bytes |
| B3B280 descriptor samplers | ECX B0h builder; stacked pass then descriptor; RET 8 | Complete B3B280..B3B3B1 listing analyzed, 306 bytes; no C++ implementation |

All implemented interfaces operate on existing actual storage. B5F100 uses the
pass+24 `NativeMaterialStateArray` and existing B40C80 reserve. B44CF0 uses the
88h `NativeMaterialPassStorage` and B44690 binding constructor. B18FB0 uses the
C4h `NativeMaterialEffectBaseStorage`, its eleven actual names at +3C and signed
word count at +94. Existing producers B5F720/B44B10 and B18D60 establish these
layouts. No new pass/effect identity, resource vector or private owner domain
is introduced.

B5F100 writes only one byte into a four-byte local at entry ESP-4, then later
copies that whole DWORD into the row's second word. The three upper bytes are
unwritten stack preimage. Its explicit C++ interface therefore takes a borrowed
volatile DWORD scratch location whose preimage the caller supplies. It writes
byte zero before reserve and reads the entire current word after reserve; a
boolean conversion or zero-filled padding would change the original behavior.
The first row word is the captured index. Reserve is called only when current
count equals capacity, with signed doubled capacity clamped to at least one.
After reserve it reloads count/data, writes both DWORDs if the computed row
address is nonzero, then increments the current count. Accessible live extents
remain the caller's responsibility; the existing reserve rejects corrupt or
overflowing allocations.

B44CF0 allocates 16 bytes through the shared singleton heap and
reuses the actual B44690 constructor, including its name copy and retained
reference behavior. Only after construction does it read current pass+6C,
publish the binding at pass+5C+count*4, and increment current count. It does not
release an overwritten slot. A completed binding whose current index exceeds
the four accessible slots is retained in the failed host frame for explicit
cleanup, instead of writing outside the pass.

The one-state native unwind map at DF7D74 maps state zero to CBF400. Raw
CBF400..CBF40A bytes show `MOV EAX,[EBP-10h]; PUSH EAX; CALL BF65AC; POP ECX;
RET`, establishing raw allocation release on a throwing constructor. The reused
B44690 constructor already cleans its name; this wrapper frees only the raw
binding allocation on that exception. It marks the retained diagnostic pointer
as freed, so it cannot be mistaken for a live cleanup obligation.

At review time Ghidra's stored `Unwind@00cbf400` body still ends at CBF408, the
last byte of CALL. Its POP/RET at CBF409..CBF40A have no containing function.
The integrator attempted the official tail repair, but reported that stored
body extension remained incomplete. The raw eleven-byte span and unwind data
are pinned separately. This packet makes no full-repair claim and performs no
Ghidra mutation. All four normal bodies have no flow gaps.

B18FB0 sign-extends the signed count word, compares the index against that value
as unsigned, and writes low16(index+1) before any string operation when required.
A count of -1 therefore does not become an ordinary negative bound. Only then
does it skip a same-header copy. Otherwise it resizes the actual destination,
rereads source length, and, if nonzero, copies current destination length from
current source data to current destination data. The original BF7680 selects a
reverse copy for overlap, represented by `memmove`. Valid indices are 0..10;
out-of-range indices are host errors rather than native out-of-bounds writes.

`NativeDescriptorSamplerOperation` is persistent, one-shot host state. It
retains owners, scratch, source/destination headers, binding/context pointers
and the last native call site on failure, rejects replay, and terminates if an
unresolved failed frame is discarded. Callers keep the shared contexts and
actual storage alive and exclude retirement while a frame runs or remains
failed. Diagnostic acknowledgement frees nothing. Original FH3 stack encoding,
native exceptional execution and external retirement interception are not
provided.

The remaining B3B280 behavior was checked directly in assembly:

- It walks current descriptor C4/C8 entries with a captured sampler pointer
  for each iteration. Source type zero appends the sampler index. Source type
  three first sets the name in actual builder+78 effect storage, then rereads
  the sampler index and appends `FFFFFFFFh-index`.
- Source type one calls 4DE4B0 then B1B4D0 with the actual source-name header.
  A null result skips the texture/binding append and builder+8C increment.
  A nonnull result goes through B44CF0, sets pass byte80, decrements the
  returned owner's actual +4, dispatches its current virtual zero on final
  release, and then increments current builder+8C.
- Other source types skip those texture operations. Every source type still
  processes the current sampler+24 state list and advances a stage counter.
  Each state call uses current vertex byte0C and current builder90+16 or
  builder94 slot; it forwards value/state to actual B5ED60. The list pointer,
  count and subsequent stage byte are reread after callbacks.

The remaining dependency is concrete. Existing 4DE4B0 exposes a projected
`ParticleClock`; B1B4D0 consumes the actual 1Ch owner and passes owner+4 to
B1A4F0. B1B4D0 copies/lowercases a pooled name, creates pooled `Default`, calls
B1A4F0 with name/default/1/1, and cleans both temporaries. B1A4F0 contains a
2Ch record/alias cache, path normalization, virtual resolution/loading/retain
operations and file-date handling. It has not been reconstructed here. Its
virtual profiles, actual singleton storage and returned-owner lifetime must
be established before the full B3B280 route can be supplied. A null/no-op loader
or another projected owner would not satisfy that dependency.

Validation: default MSVC Win32 /W4 /WX build and both existing CTests passed
after seed verification. One ignored local fixture links the registered
library and relocates only the three implemented normal bodies: 267 original
bytes match both installed PE and live Ghidra. The 306 analyzed B3B280 bytes
and 55 unwind metadata/code bytes are separate evidence.

The fixture uses actual pass slots, canonical string storage and a canonical
actual-reference binding. Original/source results and selected heap/string
traces agree for scratch preimage and reserve-time mutation, binding publication
after a count change, retained counts, name count publication, live source
replacement, self-header and negative-count cases, and overlapping buffers.
Both legs share existing reserve, binding-constructor and resize boundaries;
this is not a new independent differential test of those children. Source-only
failures check retained scratch, constructor raw-free, preallocation name count,
replay rejection, explicit diagnostic cleanup and failed-frame exit 77. Native
exceptions are not executed by the copied bodies. No full B3B280, real texture
loading, shader compilation, rendering or gameplay validation is claimed.
