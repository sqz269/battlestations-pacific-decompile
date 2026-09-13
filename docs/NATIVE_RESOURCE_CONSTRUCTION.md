# Native resource construction

Four complete bodies now operate on actual raw Win32 resource storage:
`B88260` base construction, `71B810` game-resource construction, `71B870`
game-factory creation and `B88340` default-factory creation. They cover 465
original bytes. The [report](../reports/native_resource_construction.json) and
[frontier audit](NATIVE_GAME_RESOURCE_CREATE_FRONTIER_BC.md) preserve complete
live/PE spans, call sites, original ABI and FH3 state evidence.

The new interface adds `NativeStringRawPoolContext` and omits the unused factory
ECX argument. It creates raw resources with reference count 1 and owned names.
Actual load/cache composition and successful-resource teardown remain incomplete;
the result is not an owning C++ object or a callable native vtable.

| Entry | Native extent / interface | Implemented behavior |
| --- | --- | --- |
| `00b88260-00b88318` | Caller supplies at least44h; ECX object, name stack, EAX same object, RET4 | Ref-base/resource profiles; name copy; zero+10..+40 |
| `0071b810-0071b869` | Caller supplies74h; ECX object, name stack, EAX same object, RET4 | Base first, CFD8CC, three derived pointer triplets |
| `0071b870-0071b8ce` | Allocate74h; unused factory ECX, name stack, EAX object/null, RET4 | Construct game resource; free allocation on construction failure |
| `00b88340-00b8839e` | Allocate44h; unused factory ECX, name stack, EAX object/null, RET4 | Construct base resource; free allocation on construction failure |

## Name and partial-state ordering

Base construction stamps `CEB130`, sets reference count 1, compares the supplied
name with `object+8`, then stamps `D63228` and clears the destination header.
The comparison precedes both header stores. Self-name construction therefore
discards the incoming header and does not resize or copy it. No automatic
destructor attempts to recover the discarded buffer.

For distinct headers, the source length is read for the existing raw `41DD40`
resize with preserve1. After resize returns, the code rereads source length;
if nonzero it reads current destination length, current source data and current
destination data, in that order. The fixed CRT `memmove` bridge preserves
`BF7680`'s nonzero overlap domain. As in the existing raw string implementation,
zero-byte copies are omitted after these header reads; no original CRT
invalid-pointer or return-register behavior is claimed. No library algorithm
was ported or type-erased string owner substituted.

Only successful name construction zeroes `+10..+40`, including six positive-zero
float words. The derived constructor then writes `CFD8CC` and zeroes
`+48/+4C/+50`, `+58/+5C/+60`, `+68/+6C/+70`. Incoming words `+44`, `+54`, `+64`
remain untouched. The implementation neither creates STL containers there nor
adds a terminator beyond the native resize service's own write.

## Exception ownership

The base constructor's only reached cleanup state invokes the existing seven-byte
`BD30F0` body: stamp `CEB130` and return. It does not return a failed name buffer
or initialize later fields. Its C++ catch performs exactly that source cleanup
and rethrows. The derived constructor has no added catch: its original state
remains -1 throughout, despite a three-row static unwind table.

Each outer create body allocates through the existing `BF681B` source service,
using identical native and host extents. If construction throws, its catch frees
the exact saved allocation through the existing free service, then rethrows.
An exception thrown by allocation itself occurs before that catch. The explicit
null-allocation return branch remains, though the production allocator has a
stronger throw-on-failure contract. No cache/global publication or singleton
registration occurs here.

Original FH3 cleanup funclets `C84FA0` and `CC25B0` have verified `POP ECX; RET`
tails at `C84FA9..C84FAA` and `CC25B9..CC25BA` that are missing from current
Ghidra function bodies. The report leaves these repairs to the integrator.
No Ghidra mutation was made by this worker. Source C++ catches do not reproduce
the original FH3 frame layout, runtime handler identity or asynchronous SEH.

## Validation and remaining scope

Validation uses the strict MSVC Win32 build and both existing CTests. One local
capsule addresses the concrete aliasing/exception risk: it compares the original
four normal bodies with the exact current construction object from `bsp_core.lib`,
and links the exact current ref-counted base object. The original reference is
frozen in a separate invocation before any source execution. Shared test-only
allocation/resize adapters expose self-name alias, post-resize source/header
changes, overlapping data, counted embedded NULs and allocation-null returns.
Source-only exception observations check partial state and exact outer allocation
ownership against the static FH3 evidence. They do not execute original FH3.

The capsule is not a test of the complete actual pool, resource teardown, cache,
reader, parser-result ownership or gameplay. Production code calls concrete raw
pool/ref-base/allocation services; the test adapters are confined to ignored
`local/resource-construction-fixture-bc/`. All attempts, compiler inputs, object
and archive identities, fixed original output and final-commit replay remain
local evidence for integration review. No permanent tests were added.

Successful resources still require `B88430`/`718810` destruction, including
actual cache-name removal, item deletion profiles, hierarchy-pool return and
array unwind helpers. Existing typed reader/parser/model implementations remain
separate and available. No dummy owning resource, stubbed load result or process
success path was introduced.
