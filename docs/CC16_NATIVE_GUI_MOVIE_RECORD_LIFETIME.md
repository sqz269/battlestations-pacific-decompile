# Raw Movie record-string lifetime contracts

Frozen published base: `02a78bd1fc85cd7e2ff06450d1aa7edeae15ae94`.
This adds only the14h/24h record-string range contracts required by raw Movie
destruction. They compose the existing genuine raw NativeString destructor;
they supply no allocator, global, vector tidy/free, decoder/owner or consumer.
This is library-contract composition with zero vendor/native-body, original
ABI or activation credit.

## Native contract and explicit inputs

| Contract | Inclusive native span | Size/starts | Stride | Direct native calls |
| --- | --- | --- | --- | --- |
| subtitle strings | AAF0E0..AAF117 | 56B/24 | 14h | AAF100->419CC0, AAF107->BD1510 |
| extratitle strings | AAF120..AAF157 | 56B/24 | 24h | AAF140->419CC0, AAF147->BD1510 |

Both originals receive first in ECX and end in EDX, save first/end in ESI/EDI,
and return with RET8. Their two stacked pointer/tag slots are ignored. No
semantic EAX result is established. The new Source entries instead take actual
first/end plus a required `NativeStringRawPoolContext&` under ordinary cdecl.
They omit the dead native stack slots; they do not fabricate values or claim
private-stack, register/stack ABI or native FH3/SEH equivalence.

Each range caches end once. While current is unequal to that value, it consumes
only the record's first8-byte native string header, then advances the DWORD
address by the fixed stride. Empty ranges touch no header/pool. There is no
alignment/range/count guard or replacement checked-container policy; callers
must supply genuine constructed, aligned raw record storage and valid equality
termination/backing through all callbacks. No semantic C++ record is overlaid.

Per header, native code captures data+4 first. Null skips length and both pool
calls; otherwise it reads current length+0, wraps length+1 as a DWORD, obtains
the current419CC0 pool and returns the captured buffer through BD1510. Headers
remain untouched, including replacements during release. End is not reloaded
from a container after callbacks. No extratitle record+18 object is deleted,
and no other row field is read or cleared.

## Genuine dependency and custody

`destroy_native_string_header_0041dd20(header, NativeStringRawPoolContext&)`
already has the exact data/length/wrapped-size/getter/return/header-preservation
schedule. This overload is called directly. No `NativeStringStorage` noexcept
interface or `crt_string_storage` fallback is selected. The raw getter's
existing C++ exception contract propagates without adding local cleanup.

The context borrows the SAME actual01090AA8 pool publication,01090AA4 live
small-return gate and01090AA0 manager. Actual suppliers already exist as
`GameNativeStringProcess::raw_context()` and `GameVfsHost::raw_strings()`;
the caller must select the same domain that owns its actual string buffers.
The context owns nothing and no substitute is constructed here. Each nonnull
header obtains the current pool again, including large frees and disabled
small returns. The gate remains a live cell, not a captured Boolean.

Keep the record backing, context/cells, providers and independent cleanup
metadata alive through the operation/failure disposition. Each captured buffer
must remain valid until its own release; afterward its bits may dangle. If a
getter throws, the current step does not advance and earlier headers may already
have consumed buffers. There is no prefix rollback, second destruction, retry,
header repair or dependency drain. The actual outer caller/stage owns the
subsequent cleanup and failure custody. These procedures add no one-shot owner
or arbitrary-reentrancy guarantee.

## Static validation

Read-only BSP wrappers verified the existing configured bsp project
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/image400000 before
analysis batches; live/snapshot count77631. Complete pseudocode was checked
before both complete assemblies/live56-byte spans and bounded disk decoding.
The report pins the original bytes, four native direct-call rows and unchanged
real string/pool/context/supplier source.

Strict MSVC Win32 objects passed `/std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4
/WX /fp:strict` for the new range source and unchanged `src/native_string.cpp`.
Each emitted public range is40 bytes/19 instructions: cached EDI end, empty exit
before the header call, fixed ESI14h/24h increment, comparison against that same
EDI, no record/header stores. Each has one physical REL32 relocation at+16 to
the raw-context41DD20 symbol, defined in the separately compiled real dependency.
The compiler also emits identical private template COMDATs; they are not new
Source APIs, reconstructed bodies or a runtime graph.

The dependency's selected raw overload is49 bytes/23 instructions. Fresh object
inspection confirms data load+06, null branch+0B before length+0D, wrapped
increment+14, real pool getter call+1A and pool return+25, with no header stores.
The static PE/COFF audit and existing native call checker receipts are recorded
in the report. They prove emitted shape/definition availability, not a final
link, executed fixture, original ABI or runtime acceptance.

## Remaining admission

No new tests, full build, native execution, probe, game or DLL load ran. Only
the four owned new files changed; CMake/shared metadata/Ghidra, current semantic
APIs, the admitted raw default constructor and all guards remain untouched.
Root/subtitle/child-Page guards stay false. The primary owns independent review,
CMake/common build/link and any later admission.

This closes neither vector backing free/tidy AAF720/AAF7A0 nor whole AAFEF0,
AB0480 flags1/pool ownership, decoder-scalar/base/model/observer custody, factory,
loaded/properties/Page children or playback. The separately owned raw record
allocation prerequisite is not a dependency consumed or admitted by this packet.
Do not turn range cleanup into whole-container or Movie lifetime credit.
