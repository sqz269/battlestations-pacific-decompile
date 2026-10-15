# Native string and x87 scalar copy construction

Addresses: `00444070`, `004440C0`.

Both complete normal bodies are reconstructed against caller-supplied actual
storage and the concrete `NativeStringRawPoolContext`. The public entry points
are in `bsp/native_string_copy_construct.hpp`; the implementation composes the
accepted actual-header `00426060` body, then executes the observed x87 scalar
load/store. There is no callback, copied header owner, or pool stub.

| Routine | Inclusive body | Bytes / instructions | Coverage | Original ABI |
| --- | --- | --- | --- | --- |
| `00444070` | `00444070..004440BA` | 75 / 29 | complete | ECX destination; stack source header, scalar address; EAX destination; `RET 8` at `004440B8` |
| `004440C0` | `004440C0..00444107` | 72 / 28 | complete | ECX destination; stack source record; EAX destination; `RET 4` at `00444105` |

The minimum destination storage is 12 bytes: a native eight-byte string header
at `+0`/`+4`, then four bytes accessed as x87 single precision at `+8`.
`00444070` needs only an eight-byte source header and an independent readable
four-byte scalar address. `004440C0` reads the scalar from source `+8`.
The constructor writes establish this layout. No higher-level meaning for the
scalar or recovered source type is claimed; the existing descriptive Ghidra
names are retained. Live Ghidra still has unknown prototypes; the ABI above
comes from register provenance and stack cleanup, not those prototypes.

## Header and scalar schedule

1. Capture ECX as ESI and the stacked source as EDI. Compare their addresses.
2. Store zero to destination length, then destination data, before branching
   on the comparison. Identity therefore abandons the old buffer too.
3. For distinct headers, capture the source length and call `0041DD40` with
   preserve `1`. This uses the actual raw pool publication cells, singleton
   manager, allocation, and return bodies through the accepted helper.
4. Reload the source length for the nonempty guard. If nonzero, read current
   destination length, current source data, then current destination data and
   copy that current destination length. The overlap-capable native `BF7680`
   body justifies the raw helper's `memmove`; no stale source snapshot is used.
5. After all header work, load and store the scalar with x87 `FLD m32real` and
   `FSTP m32real`, then return the actual destination address.

| Native call site | Containing routine | Callee | Verified binding |
| --- | --- | --- | --- |
| `0044408E` | `00444070` | `0041DD40` | `resize_native_string_header_0041dd40`, through raw `00426060` composition; current captured source length, preserve 1 |
| `004440A3` | `00444070` | `00BF7680` | current destination/source pointers and destination count; overlap-safe host copy |
| `004440DE` | `004440C0` | `0041DD40` | same concrete raw-pool resize binding |
| `004440F3` | `004440C0` | `00BF7680` | same current-field copy binding |

The header prefix was compared instruction by instruction with `00426060`.
The destination initialization, identity branch, reads on both sides of the
resize, copy count and pointer-read order all match. `0041DD40` and `BF7680`
bodies were read before selecting those dependency contracts. The accepted
raw resize's allocation/return exception and valid-memory domain applies here.

At `004440AB`, the first routine reloads stacked argument 2 into ECX, then
`004440AF: D9 01` loads `[ECX]`. Its `004440B2: D9 5E 08` stores destination
`+8`. The second routine uses `004440FB: D9 47 08` to load source `+8`, then
`004440FF: D9 5E 08` to store destination `+8`. The intervening `POP EDI`
does not access the scalar. These are floating-point operations, not raw
DWORD copies. The MSVC Win32 implementation emits an explicit inline-assembly
pair, following the established authored-fog scalar precedent. It leaves the
x87 control word alone and adds no `FWAIT`; signaling NaNs, denormal operands,
pending exceptions and x87 stack/status effects belong to the same operation
pair. Binary32 value preservation alone would not establish this behavior.

Both routines execute the scalar pair on empty and identity paths. Identity
can quiet a signaling NaN at the same scalar address. The first routine's
scalar address may alias the destination header, either data buffer, or the
scalar slot; the load observes the bytes after all header stores, allocation
and copying. The second routine also reloads source `+8` at that late point.
All aliases must remain within the accepted live-memory/ownership domain of
the header helper. No validation, null guard, or rollback is added.

## Exceptions and verification limits

There is no local native EH frame or cleanup in either body. A pool/getter
failure prevents the scalar load/store and leaves the actual header state
reached by the accepted helper. The Source interface adds no output destructor
or catch. The inline x87 operation order is retained; host instruction
addresses, stack/register frames, native fault resumption and SEH identity are
not supplied by this ordinary C++ interface. Existing raw-header helpers omit
a zero-count host copy after reading the same fields; that inherited boundary
adds no memory accesses or cleanup.

All live queries used the verified `bsp` project configured as
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`. Live bodies, disassembly, bytes,
function boundaries and empty xrefs were checked. No live caller or producer
was available to justify a stronger application-level type or integration
claim. Prior Ghidra names/comments were saved locally; annotations append
evidence, preserve those names/comments, save the project, and refresh exports.

The machine-readable report records the build, existing checks, annotation
receipt, and compiled x87 inspection. Source/build evidence does not establish
drop-in native ABI compatibility, native differential execution, a reachable
`bsp_game` integration, or game validation. No new test case is added.
