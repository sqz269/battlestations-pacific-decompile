# Remaining actual renderer array cleanup

This packet supplies four actual array destructors used by the renderer's FH3
states 15 through 18 and their three previously missing direct resizes. Every
entry receives the actual 0Ch data/count/capacity header. The entries never read
or destroy pointees, and introduce no projected array, owner map or callback.

| Entry | Complete bytes | Actual renderer header | Source entry |
| --- | ---: | --- | --- |
| B280B0..B280C6 | 23 | +1AE8 | destroy_native_renderer_array_00b280b0 |
| B280D0..B280E6 | 23 | +1AF4 | destroy_native_renderer_array_00b280d0 |
| 737BF0..737C06 | 23 | +1B00 | destroy_native_renderer_array_00737bf0 |
| B280F0..B28106 | 23 | +1B0C | destroy_native_renderer_array_00b280f0 |
| B22EE0..B22F2F | 80 | +1AE8 | resize_native_renderer_array_00b22ee0 |
| B22F30..B22F7F | 80 | +1AF4 | resize_native_renderer_array_00b22f30 |
| B22F80..B22FCF | 80 | +1B0C | resize_native_renderer_array_00b22f80 |

All seven cover their complete native bodies, totaling 332 bytes. Descriptive
names are source hypotheses. The header preserves address names where the
array's resource interpretation is unnecessary to its behavior.

## Substantive reuse

The three 80-byte resize bodies match the published raw 737390 body byte for
byte after normalizing only their reserve CALL displacement. Their B226B0,
B22710 and B22850 reserve targets are each the same complete 95-byte body as
published raw 735FF0 after normalizing only two CALL displacements. All those
reserve CALLs reach BF55BE allocation and BF6989 free. Fresh complete live/PE
captures and normalized hashes prove both equivalence classes.

The new resizes therefore call the existing
`resize_native_procedural_pointer_array_00737390`, which calls the substantive
`reserve_native_cube_texture_owner_array_00735ff0`. No source copy or substitute
host method was needed. The existing material-state-cache projected reserve
interfaces, which add diagnostic guards, are not this raw route. No dependency
merge or existing provider edit was necessary. Reserve addresses and 737390
remain read-only dependencies, not newly claimed bodies.

Resize compares signed requested count against current signed capacity. Reserve
clamps its own capacity request to at least one and allocates with wrapping
DWORD byte multiplication. It repeatedly reloads current count/base while
copying, skips a null destination cell, frees CURRENT old data, then publishes
new data followed by capacity. Count remains unchanged during that reserve.
Its existing allocator supplies the real CRT allocation/new-handler loop; a
failure propagates before subsequent stores. There is no new rollback guard.

After reserve, resize captures count into its local index. Growth reloads current
base each iteration, zeroes a nonnull computed cell, and advances the wrapping
index without publishing count. Shrink repeatedly decrements CURRENT count,
then publishes the requested count. It never releases a pointee or clears
removed cells. Negative counts and capacities retain native signed comparisons;
the caller must supply valid extents for every reached access.

Every destructor calls resize0 before loading the data pointer to free. It does
not clear the header's stale pointer or capacity. A negative capacity can cause
even resize0 to reserve one cell: the final free must use the newly published
pointer. A resize exception bypasses final free; previous effects remain.

## Parent and ABI boundaries

Both fresh parent map slices confirm states 15,16,17,18 step to 14,15,16,17.
Constructor actions CBDE13/24/35/46 reload the renderer from [EBP-45C]; destructor
actions CBDFC6/DFD4/DFE2/DFF0 use [EBP-14]. Each adds the corresponding actual
header offset and tail-jumps to the destructor above. These are read-only
observations of parent state maps DF6710 and DF681C, not parent ownership or a
claim to implement the native FH3 dispatcher.

Native destructors use ECX header and plain RET. Native resizes use ECX header,
one stacked signed count and RET4. Source fastcall resize adapters reserve EDX
explicitly so the count remains stacked. Existing shared providers use their
published C++ interfaces; private call frames and host CRT are new. No x87 or
local FH3 cleanup occurs in these seven bodies. C++ allocation failure can
propagate, but native asynchronous SEH, unrestricted storage aliasing/reentrancy,
original exception identity and full parent EH parity are not established.

Saved/live B280B0/B280D0/B280F0 bodies stop at their BF6989 CALL. Complete original
tails add `ADD ESP,4; POP ESI; RET`, ending five bytes later at C6/E6/B28106.
B22850 has a full declared body but its returning-free flow omits publication
instructions at B228A1..B228A9. These repair needs are reported to the integrator.
The worker performed no Ghidra mutation.

## Validation

The strict Win32 build and both configured CTests pass after eight verified
native seeds. The report carries 22 complete live/PE spans and 28 direct/tail
transfer sites, including the reused providers and both parent map slices.
Each Ghidra wrapper verifies the configured bsp project, program path, x86
language and image base before its read-only query.

One local /MD manifested probe executes the seven original bodies with the
complete reserve/resize dependencies relocated into private memory. Only direct
CALL displacements are rebound; allocation/free use the same genuine source CRT
providers. The corresponding rebuilt source entries run against separate actual
headers. It compares grow/copy/zero behavior, signed shrink to -1, retained poison
pointer cells, and all four negative-capacity destructor replacement tails.
The fixture never dereferences or frees the final dangling header pointer.

The probe records actual process modules using Toolhelp32, verifies each loaded
image is x86, and resolves its path through a file handle in that same 32-bit
process. This resolves System32 aliases to the actual WOW64 file before the
64-bit evidence freezer reads it. Its source,
generated original-byte header, executable, library, logs, compiler inputs,
toolchain files, measured module files and original PE are frozen by hash.
Allocation failure and parent/native FH3 dispatch are static/source-contract
evidence only. No in-game renderer, real referenced resource or gameplay was run.

## Integrated validation at aa836cdd

Seven distinct actual entries reuse instruction-equivalent raw DWORD-array providers. Original/source signed resize and all four negative-capacity cleanup paths passed; allocation failures remain static. The combined strict Win32 build, eight seed checks and both CTests passed.
The six final-library fixtures, 503 direct/tail audit rows, 31 saved/read-back
annotations, and 59 live/PE spans are preserved in `local/checkpoints/aa836cdd/native-renderer-parent-dependencies/validation.json`
(SHA256 `e6cbd24bb0166528f733e3075862ab1edb655ea9624b623e7cf08f6df90fdbc3`). Original CRT/FH3/SEH/private-frame identity,
full renderer lifecycle/adoption and gameplay remain unvalidated.
