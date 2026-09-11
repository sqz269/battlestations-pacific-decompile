# Native hardware layout CreateIfMissing

`create_native_hardware_layout_if_missing_00b60a10` reconstructs the complete
666-byte body at `00B60A10..00B60CA9`. This is a descriptive name. The existing
stream-appending construction entry B60790 remains separate.

The original receives the actual owner in ECX and returns with RET, without a
semantic return value. The new MSVC Win32 interface keeps ECX for the owner and
adds EDX for a `NativeHardwareLayoutConstructContext*`. Its compiler-generated
frame and exception ABI are new; this is not a binary replacement.

The first raw read is DWORD owner+40. A nonzero value returns before reading
owner+38, streams, context, storage, renderer or any provider. That path permits
a null context. On a miss, the fixed context and all subsequently used raw
objects must be valid. The context's references identify the application's
actual mutable slots and shared lifetime, rather than cached values.

## Captured stream pointers and current declaration data

B60A3E captures owner+38 once. The initial JBE/JC copy loop uses this value as an
**unsigned** bound and copies each existing pointer at owner+8+0C*i into four
local pointer slots. The later JLE/JL traversal uses the signed captured local
count. The supported original scratch domain is count 0..4. A negative bit
pattern enters the unsigned copy loop and is outside that domain; it is not a
signed early exit. No bounds check or recovery policy is added.

All pointers are captured before any diagnostic call. Each captured declaration
is then read through its current raw pointer at +0C and signed count at +10;
both are reread during its 20-byte record loop. Output packs stream WORD,
offset WORD, type/method/usage bytes and the old low byte of the corresponding
usage counter. The usage counter index is the full raw usage DWORD, followed
by a wrapping DWORD increment. Raw usage must be below 14, and emitted elements
including the final END record must fit 16 slots. Out-of-domain stack
overwrites, concurrent mutation and pointer faults are not ported contracts.

No stream is appended, retained or released. The four-byte unreachable
alignment instruction at B60ABC is `8D 64 24 00` (`LEA ESP,[ESP]`), included in
the complete original byte evidence.

## Diagnostics and cleanup

Every captured stream, including one with zero raw elements, performs the
original temporary `VertexFormat` string and diagnostic-record string work.
The readonly D61BD0 literal is the exact 13 bytes including NUL. Temporary
length/data and later record length/data are captured in their native order.
The record's first word receives the current owner+40. The second memcpy is
gated only by captured temporary length; it uses captured record length, even
when that is zero. This differs from the neighboring construction source and
is implemented independently here.

FH3 handler CC1346 uses info DFA0EC and map DFA0DC. State 0 destroys the current
temporary header via CC1330 -> 41DD20. State 1 first destroys the current record
via CC133B -> B3F4C0, then the temporary. State 0 arms after the temporary and
record first word are prepared; state 1 arms after the second copy and before
the complete B3E730 singleton call. Normal cleanup changes to state 0 before
releasing the captured record buffer, then disarms before releasing the
captured temporary buffer. It uses captured sizes/pointers; exceptional cleanup
uses current headers. Source RAII preserves this C++ cleanup order. Native SEH,
asynchronous faults and native FH3 ABI compatibility remain separate.

The temporary has no cleanup owned by B60A10 while its first resize and literal
copy execute: native state remains -1 until B60B88. Source likewise creates
`StringCleanup` only after the temporary is prepared and the record first word
is stored. It does not add an earlier owner or rollback for those operations.

Both strings use the same supplied `NativeStringStorage` owner. For actual pool
composition, supply `ActualNativeStringPoolStorage` bound to the one 01090AA8
publication, current 01090AA4 return gate and canonical 01090AA0 lifetime with
its `NativeStringPoolLifetimeBinding`. Each actual adapter operation calls the
complete 419CC0 getter, followed by complete BD1120 or BD1510. The support
singleton uses the actual 0108FEDC slot and this same lifetime. A semantic or
CRT allocator is a narrower explicit host boundary, not actual pool proof.
The pre-existing `release` interface is noexcept; throwing lazy pool creation
during release terminates and is not native cleanup-failure equivalence.

## Current renderer, output and stride

After all diagnostics, the END record's first DWORD is stored, the current
renderer slot is captured, and its second DWORD is stored. Current renderer
+1A10 supplies the device, whose current table+158 receives the output array
and the actual address owner+40. HRESULT is ignored. The complete B47D60 stride
provider then uses the current owner list, count and declaration+CC strides;
it does not use the earlier captured pointer list. No local rollback or extra
resource ownership is introduced.

## Verification and limits

The isolated strict Win32 library build, both existing CTests and all eight
native seed checks passed. No permanent test was added. The ignored fixture is
`local/hardware_layout_create_if_missing_fixture/` in worktree
`J:/PROG/battlestations-pacific-decompile-native-hardware-layout-create-if-missing`.
Its `sealed.json`, `verified.json`, scripts, frozen library/objects/source
closure, original inputs and outputs form the replay bundle.

One paired sequence executes full original B60A10 and B47D60 and the actual
frozen-library implementation/providers. String and singleton calls use fixed
context ABI bridges. Original 419CC0+BD1510 is composed through the owning
storage release interface, whose actual adapter executes those full providers.
No provider is recompiled or replaced for observation. The actual pool and
support singleton are already published for the measurement, so this does not
test lazy creation during the owned body.

After a real pool allocation, the storage observer changes owner count/list,
the second captured declaration's raw input and the current renderer. Both
paths still emit two captured streams, use the changed raw data and device,
perform four real pool allocations and releases, and receive S_OK from a real
D3D9 HAL CreateVertexDeclaration. GetDeclaration returns the exact submitted
elements. After the real API returns, the observer changes current stride
inputs; both paths compute 123. The actual driver's vtable is restored during
underlying driver calls. A paired early hit succeeds with unused owner fields
protected and a null context.

The proof independently compares 20,788 observable bytes, normalizing only
the returned COM identity at owner+40; all literal raw arenas and complete
actual pool postimages are retained. Pool histories are not equated across
sequential runs. All 254 mapped COFF sections, including 136 library sections,
have complete byte/relocation checks. Eight exact archive members and 27 source
and header files are frozen. Five snapshots verify unchanged original spans
and whole runtime source text/rdata, with 68 real import and six COM provider
observations checked against the loaded DLL files.

Failure HRESULTs and injected exceptions were not tested; cleanup states were
checked statically. This fixture does not establish native caller ABI,
out-of-domain behavior, a complete device-recreation loop or gameplay behavior.
