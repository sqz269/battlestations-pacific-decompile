# Native cube-texture owner pointer-array reserve

This packet reconstructs the complete `00735FF0..0073604E` routine, 95 bytes.
The cube factory at `B2A380` uses it for the actual renderer+1B00h generic
resource registry. Other native callers also use this pointer-array operation;
it does not create, destroy or retain cube owners.

Native ECX is the actual 12-byte header, whose DWORD fields are table pointer,
current count and capacity at +0/+4/+8. Its single stack argument is a signed
capacity request. It returns with `RET4` and no semantic result. The new
`reserve_native_cube_texture_owner_array_00735ff0(void*, int32_t)` interface
uses MSVC Win32 storage and the existing actual CRT allocation/free services;
it is not a drop-in original ABI replacement.

## Current storage and operation order

Clamp the signed request to at least one, then compare current capacity using
a signed comparison. Adequate capacity returns before touching the table or
count. Otherwise, compute `request*4` with DWORD wrapping and pass those exact
bytes to `singleton_lifetime_allocate`, corresponding to the original
`BF55BE -> BF681B` operator-new boundary. This shared provider performs actual
CRT malloc, invokes the real new handler on failure, retries on its nonzero
result and throws on zero. No overflow guard or alternate array is added.

After allocation, index zero is compared against the current signed count.
For each reached element, test the current destination cursor for null, then
reload the current source table and copy exactly one DWORD. Increment the
index and destination cursor with DWORD wrapping, and reread current count
for the next signed comparison. The production source uses volatile field
accesses and integer address arithmetic to retain these loads and stores.
It does not cache the input table/count across allocation or replace the loop
with a bulk memory copy.

After copying, capture the **current** table and call actual
`singleton_lifetime_free`, corresponding to `BF6989 -> BF65AC`. The full
returning-free continuation at `736041..736049` is nine bytes that were absent from the
original Ghidra listing and are now restored in the saved function. It restores the caller stack, publishes replacement
table first, publishes requested capacity second, and restores EBX before
the common epilogue. Current count remains untouched, including changes at
an allocation/free boundary. There is no FH3 frame, owner release, count
initialization or cleanup/rollback on allocation failure.

Reached storage must have valid backing. Logical malformed-header behavior,
concurrent mutation and exhausted address-space traversal are not made safe.
The native per-destination null test is preserved; it is not a general null
allocation recovery policy. This helper borrows the caller's actual header
and does not substitute a renderer, owner, device, pool or registry.

## Focused original-body comparison

The ignored fixture independently captures three fresh live-Ghidra/installed
PE spans, 105 bytes: the complete 95-byte body and two five-byte CRT entry
thunks. Every query verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The fixture rechecks those complete PE preimages when loading its isolated
sparse image and protects the code pages before execution.

The owned body is untouched, including both original relative calls and the
entire returning-free tail. Only the two external CRT thunk entries bridge:
operator new reaches the actual primary-library `singleton_lifetime_allocate`,
and free reaches an observer that performs actual CRT free. The source side
calls the real primary-library reserve and the same actual shared services.
IAT observers invoke the original host malloc/free function pointers; no
operation under reconstruction is replaced with a source callback.

Five bounded phases compare current raw fields, surrounding header canaries,
all still-live source-table words and the complete reached replacement buffer:

1. Grow from three to six slots; copy three words, preserve all spare words,
   free the old table and retain count.
2. After actual malloc, the observer redirects current table/count/capacity to
   a second real table. Copy and free use that current table. After actual
   free, another controlled change redirects table/count/capacity again; final
   publication preserves the changed count and stores replacement/capacity.
3. A negative request and negative raw capacity/count verify signed clamp,
   growth comparison and skipped copy.
4. Adequate capacity returns without touching an invalid unused table or
   invoking either allocation service.
5. Request `40000000h` wraps to zero allocation bytes and reaches actual
   `malloc(0)`; count zero causes no replacement-buffer reads or writes.

The second phase is a deliberate service-boundary perturbation, not ordinary
CRT behavior. Replacement bytes are fixture-prefilled for observation, not
initialized by production code. Freed storage is never read. Pointer identity
normalization applies only to explicit old/alternate/replacement identities;
all actual header values, payload words and spare-byte patterns are retained.
The write order itself is established by full native instructions and ordered
volatile source stores; the comparison observes the boundaries and final state.

The comparison passed with 426 identical DWORD trace entries (1,704 bytes)
across 26 snapshots. Each side made four actual malloc calls with requests
24, 24, 4 and 0 bytes, and four actual free calls, three with nonnull pointers.
All three runtime span postimages match only the two listed external bridges;
the owned body is unpatched. Three linker-map provider checks confirm the
primary-library reserve, allocation and free implementations, and both actual
CRT module paths are verified against Win32 PE014c files.

`./scripts/build.ps1` passed with both existing CTests, and all eight native
seed spans matched disk. An ignored CMake source-registration include places
the owned production translation unit in primary `bsp_core`; the strict
fixture links that exact library. No tracked tests were added. The audit pins
the source, library, object, executable, map, native spans, complete traces
and evidence scripts.

The primary integrated permanent CMake registration, reran the same fixture
against the frozen primary library, and independently verified 426 DWORDs,
26 snapshots, all runtime postimages, real providers and actual CRT modules.
Its immutable library SHA-256 is
`e9c69bcef040ab9090081dfb0c72d3c4587f086476c31e872470c84bd7377934`.
The primary also checked 28 worker artifact/source pins and three fresh live/PE
spans. The returning-free continuation is restored, the complete name and
reconstruction records are registered, Ghidra is saved and the export refreshed. No original ABI compatibility or game validation is claimed.
