# Native resource-record array reserve and resize

This packet reconstructs complete `00B2FF00..00B2FFDD` and
`00B30340..00B3040C` on the actual 12-byte array header at resource-container
`+4`: record pointer `+0`, count DWORD `+4`, capacity DWORD `+8`. Records are
the existing actual Win32 `2Ch` `NativeRenderResourceRecord`. The methods
accept that header address directly and do not create a vector or owner copy.

Both original methods receive `ECX=header`, one requested DWORD on the stack,
and return with `RET4`. Neither establishes a semantic return value. The public
C++ functions therefore return `void` and explicitly take the existing string
pool and lifetime invalid-parameter callback domain. They are C++ interfaces,
not binary replacements for the original methods.

## Reserve: `00B2FF00`

The signed comparison at `B2FF1D/23` clamps requests below 64 to 64. Signed
`capacity >= request` at `B2FF31/34` returns before reading the record pointer.
Allocation size is the low DWORD of `request * 2Ch`; there is no overflow
guard or maximum-size check. Thus `40000000h` requests zero allocation bytes.
`BF55BE` is a complete five-byte jump to existing `BF681B` allocation.

The allocation remains a captured local. Starting with index zero, each copy
iteration uses that allocation plus wrapped `index * 2Ch` as destination. A
zero destination address skips construction. Otherwise it reloads the current
header pointer and calls the complete existing `B2FC60` record copy constructor.
The signed loop limit reloads current count after each call. Placement
construction in C++ starts a raw record lifetime without value initialization.

After copying, a separate index-zero loop reloads current pointer and count for
each complete existing `B2F990` record destructor. `B2FFB8` then reloads the
current pointer for free. The returning `BF6989 -> BF65AC` tail is essential:

| Address | Publication after free returns |
| --- | --- |
| `B2FFC4` | Actual header pointer = captured allocation |
| `B2FFC6` | Actual header capacity = clamped request |

Count is not restored or copied. Changes to it made during old-storage release
remain visible. No resource ownership call is added to copy or destruction.

## Resize: `00B30340`

Signed `request > current capacity` calls the full reserve method. Resize then
captures current count as its construction index. Growth compares that local
index to the requested signed value, reloading current data for each address.
It does not increment the header count while constructing.

For a nonzero destination address the native stores are:

1. Empty name length and data at `B30391/93`.
2. Arm partial-name cleanup at `B3039B`, call actual `4C3020` sentinel allocation.
3. Publish the captured sentinel at record `+0C`, then zero alias count at `+10`.
4. Zero five DWORDs in descending order `+24,+20,+1C,+18,+14`.

Record `+08` and resource pointer `+28` are untouched, including on successful
default construction. Sentinel string words likewise remain uninitialized as
established by the existing sentinel constructor. A zero wrapped destination
address skips these stores and still advances the local index.

After growth, resize compares the requested signed count to the current header
count. Each shrink iteration decrements the actual count first, reloads it and
the current data pointer, then destroys that record. It compares current count
again after the destructor. `B303FA` finally publishes the requested DWORD.

All multiplication, addition and decrement use unsigned 32-bit operations;
signed interpretation is applied only to comparisons. No negative-request,
null-header, capacity-consistency or maximum-size guard is introduced.

## Exception evidence and deliberate absence of cleanup

Both complete 36-byte FuncInfo structures have magic `19930522`, no try map or
exception-specification list, and EH flags 1. The support maps and funclets were
matched against live Ghidra bytes and the installed PE before fixture execution.

| Owner | Handler / FuncInfo | Unwind action |
| --- | --- | --- |
| Reserve | `CBD987` / `DF628C`, max state 1 | State 0 to -1: `CBD970` |
| Resize | `CBDAB1` / `DF63A4`, max state 2 | State 1 to 0: `CBDAA9`; state 0 to -1: `CBDA90` |

`CBD970` computes arguments from captured allocation/index/destination and calls
`00401130`. The complete implementation of `00401130` is one byte, `C3` (`RET`).
It does not destroy prior copies, free replacement storage or roll back the
header. Reserve allocation occurs before the copy state is armed. Therefore a
failed allocation or a failed later record copy propagates; completed replacement
records and their allocation are not cleaned up by this owner.

Resize `CBDAA9` reads the captured current record/name pointer from frame `-10h`
and jumps to the existing actual-header `0041DD20` destructor. That destructor
reads the current name data and current length-plus-one and retains the fields.
Then `CBDA90` reads current header data and computes the captured index address
for the same no-op `00401130`. The C++ catch preserves that final header read.
There is no cleanup of previous completed growth records and no count rollback.
The failure can leave completed records outside the current count.

These are the native exception paths, not desirable container guarantees. No
RAII allocation guard or replacement cleanup is inferred from the no-op call.

## Validation

`reports/native_render_resource_record_array_audit.json` pins the complete owner
bytes, exception support, old names/prototypes/comments, relevant current source
and local verification artifacts. All 21 selected live Ghidra spans match the
installed PE. The two array owners, sentinel constructor, no-op leaf and both
allocation/free thunks execute from their verified bytes in the private fixture.

The focused fixture passed 10,900 normalized words (100 events, 109 words each)
across nine variations of the same array operation. It covers signed minimum
and its no-op branch, zero-byte wrapped allocation, current source/destination
data changes, count changes during destruction/free, final publications, second
copy/sentinel allocation failure, untouched default fields, shrink, and complete
resize-to-reserve composition. Five native record-copy calls, seven native
record-destroy calls and one native resize name-unwind call were exercised.

The original callers use the real complete reconstructed record copy/destructor
and name destructor through explicit composition hooks. The fixture compiles
unchanged dependency bodies with only selected public symbol names redirected
for observation. Allocation/free wrappers forward to the existing real lifetime
allocator and actual CRT free, with controlled callback mutation or one sentinel
allocation exception. Partial-name release uses an actual `SizedStoragePool`
and its real system-allocation branch. Deliberately abandoned allocations are
freed only after the observation window; this is fixture cleanup, not behavior
added to the reconstructed methods.

Exception metadata pointers are relocated after preimage checks. The original
handler-registration immediates point at private executable handler bridges to
the host `__CxxFrameHandler3`. Original owner instructions, relative owner calls
and cleanup funclets otherwise execute unchanged. This validates the caller
behavior at the stated composition boundaries, not the installed game's CRT,
all original transitive callees, binary exception ABI or live game state.

Strict MSVC Win32 `/O2 /Oy- /EHsc /fp:strict /W4 /WX` compilation and the fixture
pass. The normal build passes both existing tests after all eight native seed
comparisons pass. A private CMake include registers only this packet's new source
for the build; no shared CMake, tracked tests, ledgers or Ghidra data are changed.

## Integration and limits

Register `src/native_render_resource_record_array.cpp` in the primary build.
Suggested descriptive names are `BSP_NativeRenderResourceRecordArray_Reserve`
and `BSP_NativeRenderResourceRecordArray_Resize`. Preserve existing annotation
preimages and repair the false no-return flow after free before refreshing
exports. The old stored prototypes had no parameters; assembly supplies the ABI.

The explicit allocator boundary retains matching native/host byte counts on
Win32. Existing allocation failure and noexcept release policies remain in force;
the fixture does not claim equivalence for exceptions thrown by native free.
No new resource-container constructor, accounting, insertion, removal or entire
container lifetime is established here. No game validation has been performed.
