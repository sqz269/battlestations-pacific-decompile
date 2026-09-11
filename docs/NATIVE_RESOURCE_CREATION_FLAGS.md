# Native resource creation flags

`translate_native_resource_creation_flags_00b20a80` reconstructs the complete
245-byte `B20A80..B20B74` body. The existing descriptive Ghidra name
`BSP_D3D9_TranslateResourceFlags` is retained as an interpretation, not a
recovered symbol. This helper has no calls or surrounding renderer dependency.

The original ABI takes the actual usage-output pointer in ECX, the actual
pool-output pointer in EDX, then flags and resource kind on the stack. It returns
with `RET 8`, preserves EBX/ESI/EDI, and has no semantic return value. The new
MSVC Win32 C++ API takes raw `void*` outputs and captured `uint32_t` inputs; it is
not a binary-compatible replacement.

## Output storage and order

Both outputs designate four actual bytes. They can be unaligned, exactly equal,
or partially overlap. The helper captures both input values before output
mutation. For supported pool codes, it writes the pool first and usage last;
the final usage bytes therefore replace any overlapping pool bytes. It never
reads either output. The source uses separate four-byte `memcpy` stores to
support unaligned storage without imposing C++ typed-pointer alignment.

`flags & 0xF` selects pool values 0, 1, 2, or 3 and writes that DWORD. These
correspond to D3DPOOL_DEFAULT, MANAGED, SYSTEMMEM, and SCRATCH. **Every other low
nibble leaves the pool output entirely untouched, without even requiring that
pointer to be accessible.** It does not synthesize a default or return an error.
The buffer restore caller at `B4A040` can supply an untouched stack-local pool
word, so callers must preserve this conditional write rather than replacing it
with an always-initialized typed result.

For pool code zero and resource kind 6 or 7, the captured engine flags are first
ORed with `0x10000`. This precedes the usage-field comparisons below. It does not
unconditionally set D3DUSAGE_WRITEONLY: an existing field value `0x20000` becomes
`0x30000`, which fails the exact `0x10000` comparison.

## Usage translation

Usage starts at zero and accumulates these outputs:

| Captured or adjusted engine flags | Additional condition | Usage bits |
| --- | --- | --- |
| `flags & 0x10` is nonzero | None | `0x1` RENDERTARGET |
| `(flags & 0xF00) == 0x100` | None | `0x2` DEPTHSTENCIL |
| `(flags & 0xF00) == 0x200` | None | `0x4000` DMAP |
| `(flags & 0xF00) == 0x300` | None | `0x40` POINTS |
| `(flags & 0xF00) == 0x400` | None | `0x100` NPATCHES |
| `(flags & 0xF00) == 0x500` | None | `0x80` RTPATCHES |
| `(flags & 0xF000) == 0x1000` | None | `0x200` DYNAMIC |
| `(flags & 0xF0000) == 0x10000` | Kind 6 or 7 | `0x8` WRITEONLY |
| `(flags & 0xFF000000) == 0x01000000` | Kind 3 or 5 | `0x400` AUTOGENMIPMAP |

The masked fields use equality, not individual-bit tests. Other field codes
contribute no usage bits. Kind 3 is texture, 5 cube texture, 6 vertex buffer, and
7 index buffer, corroborated by the local Windows SDK `d3d9types.h` definitions
pinned in the audit. Usage is always written once at the end, including zero.

## Evidence and validation

`reports/native_resource_creation_flags_audit.json` records guarded live Ghidra
and installed-PE equality for the entire body and its separate 16-byte jump
table at `B20B78`. The native pool stores are at `B20AA1`, `B20AB2`, `B20ABA`, and
`B20AC2`; the usage store is at `B20B6E`. All are DWORD stores. The complete
instruction decode contains no calls and no output reads.

The ignored focused fixture runs the complete original body using its original
register/stack ABI. It relocates only the jump-table address operand and its
four entries; it substitutes no native body, branch, or external service. It
compares all 32 backing bytes per invocation, with no pointer normalization:
24 representative mapping inputs, seven exact/partial overlap layouts, one
unaligned disjoint layout, two inaccessible unused-pool cases, and one case
where outputs overwrite caller storage holding the captured input values.
All 35 phases agree, producing 1,960 identical trace bytes.

The fixture links the actual compiled object from the primary `bsp_core.lib`.
The optimized source object confirms one early four-byte pool store and one of
two exclusive four-byte usage exit stores. Invalid pool codes branch before
loading the pool argument; the body has no calls or output reads. The strict
Win32 repository build and both existing CTests pass, with all eight native
seed spans verified. No permanent test or shared CMake/ledger/Ghidra change is
part of this worker packet; ignored local registration includes the source in
the primary build pending integration.

These checks establish the helper's reconstruction and focused native-byte
agreement. Reached outputs still require valid four-byte backing storage; no
concurrency, exception-recovery, device-restoration, or game-validation claim
follows from this pure helper.
