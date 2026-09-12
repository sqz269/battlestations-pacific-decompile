# Native physical-to-memory stream conversion

Addresses: BEF750, BF4FF0, BF4F20, BF4F90, BF5030, BE4530, CD8FC0,
CD9030. Function names remain descriptive hypotheses.

The existing BEF750 actual-owner reconstruction now accepts numeric D691B0
physical streams through explicit physical services and current type-ID storage.
Each original virtual call reloads the owner table and its current slot: +0C
type query, +1C seek, +30 cached size, and +24 read. The memory token is read
before the first table load. The backing's current data pointer is read before
the read-method lookup. The original numeric table words stay unchanged.

The failed memory-type query takes the original copy path: seek(0,0,0), retain
the cached size's low DWORD, allocate backing, perform one read without an
actual-count output, wrap it, then release the temporary backing reference.
Seek/read status is ignored. Short reads do not trim the requested length, and
the source cursor is not restored. The source reference count is unchanged.
The memory-profile sharing path and external callable-table path remain intact.

NativeStreamTypeIds binds existing file, memory and physical guards/descriptors
to the shared TypeIdCounterLifetime and root bootstrap domain. These methods
initialize the actual descriptor shapes; consumers continue reading their
current words. Zero IDs remain ordinary values: uninitialized memory/physical
descriptors can compare equal and take the native unsafe backing-sharing path.
This binding adds no automatic initializer call or zero-ID normalization.

The source context requires a physical context and ID storage together, plus
readable original D691B0 table bytes. Missing context or unsupported slot words
throw source invalid_argument; this is a C++ domain boundary, not native error
handling. A real ReadFile failure still requires the current manager publication
and its callable field+90 failure method through BD9E30.

## Validation boundary

The combined strict Win32 build and both existing CTests pass. Four original
BEF750/source comparisons pass with real physical type, seek, size and read
leaves: exact file size, nonzero high DWORD with low size seven, short-read EOF
with the larger requested length retained, and zero size. Both cached and OS
cursors, all source-owner bytes except the reference adapter's table token,
initialized data prefixes, wrapper/backing lengths and reference counts agree.
Memory counters return to their starting values after both results are released.
The existing 32-case FileStore fixture and 1,152 type-initializer comparisons
also pass after relinking against the combined library.

The Lua composition initializes file/memory/physical descriptors and shares the
canonical counter domain. It passes fundamentals/bootstrap and installed shader
loads, nested duplicate memory-backed scripts, and a FileStore entry retaining
an actual physical stream. Clearing Platform before the retained physical
fundamentals load and observing it restored verifies script execution. Shutdown
clears the type counter publication as well as existing owners; descriptor
guards correctly stay set. No automatic startup registration is introduced.

Windows had already reserved BF0000 in the probe process. The original converter
and physical methods therefore execute at a common translation of 30000000.
Their instruction bytes stay unchanged. A separate reference owner copy uses a
callable table pointing to the translated methods; only its table word is
normalized for owner-byte comparison. The reconstructed side keeps its original
D691B0 table and owner identity. Allocation, backing construction and wrapper
creation use reconstructed dependencies through three separate reference call
target stubs. ReadFile failure and native FH3 paths are excluded: their reference
targets/handler are not installed in this fixture. The short-read tail is
uninitialized and is deliberately excluded from data comparison.

Results and frozen artifacts are recorded in reports/native_physical_memory_binding.json
and reports/native_ao_integration.json. Explicit descriptor initialization in the
fixture does not prove original startup initializer ordering. Manager,
mount and FileStore tree records remain explicitly initialized fixture inputs.
Original exception identity, general archive ownership, complete application
startup and gameplay are outside this packet's validation scope.

## Follow-up packets

Use the manager packet map's allocation producers before reconstructing complete
manager/container construction and destruction. Bind startup's derived D68D04
identity explicitly after its ownership contract is established. Actual archive
substreams and inflater ownership require their own reconstruction packets.

## Integration correction from docs/NATIVE_ADOPTED_SUBSTREAM.md

The next packet composes D68DB0 source-owning substreams with the existing memory/physical paths. Construction adopts without retaining; nested release reaches the real physical HANDLE closure. The original physical conversion pairs remain passing in the newer fixture alongside substream conversion and Lua execution. This addition does not reconstruct native MPKG provider/directory creation or initializer ordering.

Evidence: reports/native_adopted_substream.json.
