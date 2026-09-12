# Native Lua and VFS method composition

Addresses: B66CA0, B69D40, B69E00, BDEF90, B68340, BDF310, BDA690,
BE41A0. This packet composes previously reconstructed consumers and the physical,
FileStore and logging provider packets. Names remain descriptive hypotheses.

`NativeLuaVfsDispatch` separates a captured native table word from the C++ method
that implements its target. The existing callable original-ABI adapter remains
available for explicit external services. `NativeVfsRuntimeBindings` handles the
supported original numeric identities directly, without replacing owner vtables,
registering shadow owners, or changing their reference counts.

| Owner profile | Reached methods |
| --- | --- |
| D685B4 manager | +04 BDF310 open; +08 BDD440 existence |
| D69168 physical provider | +08 BF4BA0 open |
| D689E8 FileStore | +08 BE5FA0 open |
| D691B0 physical stream | +18 BF5020 validity; +24 BF5030 read; +2C BF4FA0 fresh OS size; +30 BF4F90 cached size; +00 BF55A0 recycle |
| D642C0 memory stream | +18 BEF4C0 validity; +24 BEF590 read; +2C BE41A0 size wrapper; +30 BEF600 signed size; +00 BD30E0 then current +04 BB8F90 deletion |

These native table bytes must be readable at their original addresses, matching
the existing physical and mount-lookup consumers' storage contract. Their words
identify reconstructed functions; the native binding does not execute original
code through them. Other profiles or unreconstructed slot targets produce an
explicit source boundary. Manager/mount and FileStore-tree construction, runtime
type registration, and general archive provider ownership remain separate work.

## Preserved sequencing

B66CA0 keeps the native table capture across the third length call and the read.
The read method is selected from that captured table after the length call. All
other original table loads remain fresh, including the zero-reference dispatch
after the actual InterlockedDecrement. The invoker reloads deleting slot +04 and
passes flag 1; it does not decrement again. Physical streams follow their pool
recycle method, retaining the dead address for the next actual acquisition.

BDEF90 uses the same dispatcher for each candidate's current existence method.
Captured suffix range, candidate mutation, duplicate order and temporary cleanup
are retained. Fundamentals uses the concrete VFS path while keeping the native
closed-stream preimage/reference behavior and low-DWORD allocation/read size.
Its slot +04 read now occurs inside the source dispatcher after arming path
cleanup; invalid-table SEH parity is excluded. Normal readable-table ordering is
the supported domain, with existing string-pool cleanup limitations inherited.

BDF310 delegates physical/FileStore provider opening, actual stream size, and
BDE9C0 to the binding. The logging call still precedes the captured manager's
successful-open counter and the returned stream's current size method. The
complete logging packet establishes that BDE9C0 constructs and destroys its
formatted strings without emitting output; no absent sink is invented.

BE41A0 is a complete 20-byte wrapper: call current +30, optionally store the high
DWORD, return the low DWORD. Its new source interface uses the same explicit
dispatcher. BF4FA0 remains distinct: it queries the live physical HANDLE and
discards the high DWORD, while BF4F90 reads the cached 64-bit size.

## Verification boundaries

The combined Win32 `/W4 /WX` build and both existing CTests pass. Six paired
original-instruction/source BE41A0 calls agree on low/high results and optional
output. The three earlier Lua file, override and fundamentals fixtures also
pass against the combined library under distinct artifact names.

The composition fixture passes actual Lua bootstrap and fundamentals loading,
installed shader reads through physical HANDLE streams, FileStore lookup and
fresh shared-backing wrappers, nested DoFile and duplicate suffix execution
(`MCPP`). Stored stream cursors and reference counts remain unchanged. Canonical
shutdown clears published owners and returns memory counters to zero.
`reports/native_lua_vfs_binding.json` and `reports/native_an_integration.json`
record the source, artifacts and independent reviews.

The composition fixture supplies initialized actual manager/mount and FileStore
tree records, fixture-seeded type IDs (11, 22, 33), and the original read-only table
bytes. It exercises concrete providers, raw owners, shared lifetime domains and
Lua; it does not establish native construction of those seeded trees, full
application startup, original exception identity/ABI, rendering or gameplay.
Its manager failure callback and logging gate are null, and manager+79 is zero:
successful I/O and the inactive logging path are the composition domain. The separate provider and
logging fixtures cover read-failure callbacks, retry and active builder paths.
Fixed, readable original table bytes are required; arbitrary slot mutation and
cross-profile method substitutions are not validated. The FileStore conversion
and runtime binding must share the same memory-owner counters and profiles.

## Follow-up packets

Recover actual VFS manager/provider/tree population and startup registration,
then connect this source composition into the application startup owner. Add
native bindings for archive stream classes and physical-to-memory conversion
where the current FileStore conversion requires callable external methods.

## Integration correction from docs/NATIVE_PHYSICAL_MEMORY_BINDING.md

The newer composition uses the actual file, memory and physical type initializers in the shared root/counter lifetime domain. It also loads fundamentals through a FileStore entry retaining an actual physical stream; clearing Platform before the load and observing its restoration verifies execution. Counter publication clears during canonical shutdown while descriptor guards stay set. Initializer order, manager and mount records, and FileStore tree population remain explicit fixture inputs; this does not establish full native startup.

Evidence: reports/native_ao_integration.json; reports/native_physical_memory_binding.json.
