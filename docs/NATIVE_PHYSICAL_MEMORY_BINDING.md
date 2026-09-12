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

Results and frozen artifacts are recorded in reports/native_physical_memory_binding.json
and reports/native_ao_integration.json after combined verification. The focused
fixture initializes stream descriptors explicitly within the canonical lifetime
domain; that does not prove original startup initializer ordering. Manager,
mount and FileStore tree records remain explicitly initialized fixture inputs.
Original exception identity, general archive ownership, complete application
startup and gameplay are outside this packet's validation scope.

## Follow-up packets

Use the manager packet map's allocation producers before reconstructing complete
manager/container construction and destruction. Bind startup's derived D68D04
identity explicitly after its ownership contract is established. Actual archive
substreams and inflater ownership require their own reconstruction packets.
