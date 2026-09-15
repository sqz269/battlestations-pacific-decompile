# Particle resource records and vector

## Reconstructed storage and entries

The six complete original bodies operate on actual Win32 storage and borrow the
application's `NativeStringRawPoolContext`. No owner projection or unresolved
allocation, list, or validation callback is supplied by the caller.

| Entry | Complete original span | Original ABI | Source behavior |
| --- | --- | --- | --- |
| 0086FCF0 | 0086FCF0..0086FD67, 120 bytes | ECX record, RET | Destroy aliases/sentinel/name |
| 0086FF50 | 0086FF50..0086FFF1, 162 bytes | ECX destination, stack source, EAX destination, RET4 | Deep construct name/list, copy six tail words |
| 00870000 | 00870000..008700DC, 221 bytes | ECX vector, stack signed capacity, RET4 | Reserve with minimum 64 records |
| 00870AC0 | 00870AC0..00870B2E, 111 bytes | ECX vector, stack source record, RET4 | Append a copied record |
| 00870B30 | 00870B30..00870BFB, 204 bytes | ECX vector, stack signed count, RET4 | Grow/default-construct or shrink/destroy |
| 00871310 | 00871310..00871363, 84 bytes | ECX cache owner, RET | Release resources and destroy records from the end |

Record stride is `0x2c`: native string length/data at `+0/+4`, untouched word at
`+8`, actual alias sentinel/count at `+0xc/+0x10`, five payload words at
`+0x14..+0x24`, and resource pointer at `+0x28`. Vector storage is actual pointer,
signed count and signed capacity at `+0/+4/+8`; the cache embeds it at `+4`.
Default construction preserves the existing `+8` and `+0x28` bytes. Copy
construction preserves destination `+8` and copies all six tail words in order.

All six full spans and the dependency/EH/table spans were freshly compared
between the selected `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` and the installed PE. Exact bytes, SHA-256 hashes,
call rows and prior annotations are retained in
`reports/native_particle_resource_records_orch4.json`.

## Current fields and ownership

Name construction compares identity before zeroing the destination. Name copy
resizes the actual header and then reloads both actual headers. The source raw
path uses overlap-safe byte movement matching the native BF7680 implementation.

Reserve captures its fresh allocation, but reloads old vector data/count in
each copy/destruction iteration. The returning-free tail publishes the new
pointer at 008700C4, then capacity at 008700C6. Append doubles capacity with
DWORD wrap and signed comparison, constructs before incrementing the current
count, and retains a potentially invalidated source pointer exactly as native.
Resize decrements the current count before each destruction; clear decrements
the current count after destruction. These schedules intentionally differ.

Clear loads the current last resource before dispatching the current cache
slot `+0x10`, then reloads count and data before destroying the current last
record. Known D0DAF0/D0DB40 identities use the genuine sibling provider
`release_native_particle_resource_00871420`; their verified slot targets agree.
Other caller-owned tables require actual callable Win32 thiscall targets.
The resource's terminal virtual call remains actual runtime dispatch.

## Raw shared alias providers

Seven existing algorithms gain public raw-pool overloads: 0044BCB0, 004CE6F0,
004D05E0, 004D0990, 004D26A0, 004D0A10 and 004D48A0. Shared templates retain
the old host overloads. Every nonnull string return resolves current 00419CC0
publications before BD1510; getter exceptions can propagate. This avoids the
older `ActualNativeStringPoolStorage::release` noexcept interface boundary.

Raw range/erase validation uses a fixed internal binding to the current CRT
`_invalid_parameter_noinfo`, preserving returning handlers. Public raw APIs
accept no validation callback. Unused host destroy callbacks are neither read
nor invoked. Node/count/insertion/rollback schedules use the existing complete
providers, including owning node catch and list/range catch cleanup.

## Exception ownership

The FH3 states and actions were recovered from DC7D60/DC7DC0/DC7DEC/DC7F04/
DC7F30 and their FuncInfo records. C95DF0 and C95E30 destroy the current name;
C95FA9 does the same while default sentinel construction unwinds. These are
true unwind actions: source RAII destructors are noexcept, so a second cleanup
exception terminates. Normal name release occurs after the destructor's name
cleanup is disarmed and can propagate a getter failure.

Reserve and append have only RET-only placement-delete cleanup at 00401130.
Resize adds the current name action while constructing a record, but has no
whole-vector cleanup. Completed fresh records/allocation are not rolled back
when a later record fails. Native partial state is retained deliberately.
By contrast, explicit list/range catch cleanup may replace an original
exception; existing host overload behavior is unchanged.

## Evidence limits

The parent repaired missing returning-free tails 0086FD29..0086FD67 and
008700C0..008700C9 after independent disk/live preflight; this worker made no
live Ghidra mutations. 0087005D..0087005F is skipped alignment after an
unconditional jump, not a missing behavioral tail. Proposed descriptive names
are reconstruction hypotheses, not recovered symbols; prior values are saved.

This packet adds C++ interfaces, not original register/stack/FH3 binary entry
replacements. Current source CRT, allocator, pool publication and exception
domains apply. Arbitrary SEH/fault/OOM timing, native FH3 transport and gameplay
are unproved. The cache resource acquisition slot 0086BA60 is a separate
prerequisite and is not supplied or claimed here. Validation results are in
the report; build checks and the bounded probe are not game validation.

Strict `scripts/build.ps1` Release and both existing CTests passed. A local
composition probe verified preserved default bytes, pooled deep reserve/copy,
append/shrink and known-cache release using the genuine sibling 00871420 body.
It also executed the complete installed 84-byte 00871310 body from private
probe storage, with exactly its two direct CALL displacements adapted to the
real reconstructed record destructor and resize providers. A callable cache
slot bridge invoked the genuine sibling resource-release provider. Both original
clear and source clear correctly reloaded a count changed to zero by the
terminal resource callback and skipped record destruction. Other record/vector
and alias helpers were exercised only as source composition. No original
record/list helper execution or native exception injection is claimed.
