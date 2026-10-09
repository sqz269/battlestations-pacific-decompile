# CC12 GlobalConfig lifetime-owner escape

The whole-owner escape at `004326E7 -> 00BD0C30` stores the GlobalConfig pointer
in the singleton lifetime manager's pointer vector. The native registration
and slot-construction code treats that pointer as a DWORD value: it does not
dereference GlobalConfig, read its method table, adjust its reference count,
or address its second-array fields `+2CCh/+2D0h/+2D4h` as destinations.

This closes the **direct registration-store classification**, not the complete
escaped-owner graph. Returning validation handlers, allocation/CRT services,
future consumers and eventual current-method-table scalar deletion remain
separate boundaries. The owner is globally published before registration, so
the absence of an explicit owner argument to an error handler does not prove
that handler cannot obtain or mutate it.

The [report](../reports/cc12_global_config_lifetime_owner_escape.json) preserves
the complete inspected bytes, decodes, exact call sites, store classification,
source-review references and open boundaries. This continues the
[direct-caller audit](CC12_GLOBAL_CONFIG_SECOND_ARRAY_DIRECT_CALLERS.md).
Descriptive native names remain hypotheses.

## Distinct owner identities

Let **G** be the native GlobalConfig allocation, and **M** the singleton lifetime
manager. M has raw vector begin/end/capacity-end at `+4/+8/+Ch`, with four-byte
slots. Its `+0` word and `+10h` section pointer are untouched by registration.
These offsets are M's fields, not G's fields.

The native getter `00432650` publishes G to `00F878E4` at `004326D4`. It then
obtains M at `004326D9`, reloads the published G at `004326DE`, pushes G at
`004326E4`, moves M into ECX at `004326E5`, and calls `00BD0C30` at `004326E7`.
Thus the escaped value is the whole owner, not a second-array slot address.

The getter holds a captured lifetime section around its recheck, construction
and normal registration sequence. Registration adds no internal lock. The
getter's constructor effects, manager construction, OS section internals and
exceptional unwind are not re-audited by this packet.

## Registration and entry construction

| Entry | Native contract and actual effects |
| --- | --- |
| `00BD0C30` | ECX = M, stack = G, `RET 4`. Validate unsigned begin <= end before testing G for null. A returning invalid-parameter handler continues. For non-null G, pass the address of that actual stack argument to `00BD0BC0`. No non-stack store occurs in this body. |
| `00BD0BC0` | ECX = M, stack = value-slot pointer, `RET 4`. With capacity, load G through that stack-word address, store G at captured vector end (`00BD0BF2`), then store end+4 to M+8 (`00BD0BF7`). Otherwise validate the captured end and call checked insertion with a local eight-byte iterator result. |
| `00BD08D0` | ECX = M; stack result, iterator owner, position, value-slot pointer; `RET 10h`. Preserve the insertion index, validate as native, call `00BD0700` with literal count 1, then write result position at result+4 before M at result+0 (`00BD094C` / `00BD0950`). In this caller, that result is stack storage. |
| `00BD0700` | ECX = M; stack iterator owner, position, count, value-slot pointer; `RET 10h`. Capture the pointed-to DWORD at `00BD0708` into its own argument word at `00BD070A` before count testing, allocation, copy or free. At this call the pointed-to word is registration stack storage, not the GlobalConfig object. |

There is no separately constructed per-owner entry, stored deleter callback,
type check, duplicate check or AddRef. A successful append records one owner
pointer in the manager's vector for later consumption.

The wrappers retain unsigned comparisons, modulo32 subtraction, signed `SAR 2`
distances, captured values and native reload order. Validation calls are not
assumed to terminate. The six direct `00BF6713` call sites are `00BD0C3B`,
`00BD0C09`, `00BD08F6`, `00BD0903`, `00BD092D`, and `00BD0943`. Null registration
can therefore still invoke a validation handler.

## Vector growth and stores

`00BD0700` checks the requested count against the native `3FFFFFFFh` limit.
Growth chooses at least size+count, using capacity+capacity/2 unless that
candidate exceeds the native limit. It allocates a replacement slot buffer,
copies the prefix, fills the inserted pointer value, and copies the suffix.
It then reloads current begin/end for the retained count and frees the current
non-null vector begin.

After free returns, the replacement header is published in this exact order:

| Instruction | Destination | Value |
| --- | --- | --- |
| `00BD080C` | M+4 | Replacement begin |
| `00BD0811` | M+Ch | Replacement capacity-end |
| `00BD0814` | M+8 | Replacement end |

Both native in-place insertion branches are retained in the audit. Their header
stores at `00BD0863` and `00BD0890` update M+8; their other destinations are slot
ranges supplied to the copy/fill helpers. No branch is erased by assuming an
error callback cannot return or modify state.

The four entry-storage leaves are fully inspected:

- `00BD0500` copies a slot range through `00BF67A7` using raw subtraction and
  `SAR 2` byte-count arithmetic, and returns the captured advanced destination.
- `00BD0560` reloads the captured value word for each DWORD store at
  `00BD0579`; it never dereferences that DWORD as an object pointer.
- `00BD0160` assigns the value word at `00BD0173` until the destination reaches
  the supplied end address.
- `00BD0180` derives a backward-copy destination and calls `00BF67A7` for a
  signed-positive slot count.

The resulting destinations are vector storage, M's header and local invocation
or iterator-result words. None is derived from G. This provenance statement
does not prove that malformed vector pointers cannot alias arbitrary memory.

## Callbacks and failure boundaries

`00BF6713` supplies five zero arguments to `00BF66EF`. That dispatcher reads
the current encoded handler cell `0109DD64`, calls `00C04FDE`, and tail-jumps
the resulting non-null pointer at `00BF6703`. The handler may return. Its actual
current identity and side effects remain open.

The vector copy service `00BF67A7` checks zero length, null addresses and
destination size. Invalid input writes errno through `00BFFB8B` and invokes
`00BF66EF` with five zero arguments. The valid path calls `00BF87E0` memmove.
The vector leaves ignore its returned status. Those underlying services are
explicit boundaries, not substitutes for a guaranteed no-op error path.

`00BCFEB0` checks unsigned count*4 overflow, then calls `00BF681B` with the byte
size. That allocation entry calls `00BF9F1A`; failure reaches `00C055B1`.
`00C055B1` rereads handler cell `0109DE44`, obtains a pointer through
`00C04FDE`, and calls it at `00C055C5` with the byte size. A nonzero result retries
allocation. No G pointer is explicitly supplied. Original heap/newmode service
internals, additional allocator callbacks and exhausted-allocation exception
effects are outside this registration audit.

The complete `00C04FDE` body also preserves an open service boundary: it uses
current TLS/IAT/PTD or Win32 module state to select and optionally invoke a
decoder. It is not assumed to be an identity operation or to have no side
effects. Current imported/function-pointer targets are not resolved here.

Length failure enters `00BD0590`, which builds a temporary legacy string and
exception payload and calls the throw runtime. The existing canonical
[allocation/throw review](NATIVE_SINGLETON_VECTOR_ALLOCATION.md) already covers
that source implementation; this packet does not duplicate its string or
exception reconstruction. The free edge `00BF65AC -> 00BF9DC8` receives the
vector allocation, not G. Core insertion has no added replacement-buffer
cleanup or rollback; provider exceptions and caller unwind remain explicit.

## Lifecycle and source boundaries

Registration stores the owner pointer but invokes no registered-owner method.
The later manager drain, current owner method-table lookup and terminal
scalar-deletion path are **outside the inspected registration contract**.
The existing [lifetime runtime review](NATIVE_SINGLETON_RUNTIME.md) is a routing
reference for that separate consumer; it does not establish GlobalConfig's
terminal dispatch here.

Current source review confirms that `src/global_config.cpp` publishes its
context owner and calls a second manager's `register_object` interface.
`ConcreteSingletonLifetimeManager` has separate register/append logic and a
required `destroy_registered` callback. Complete raw registration wrappers,
insertion, leaves and allocation modules also already exist. These observations
avoid duplicate implementation; they do not equate a typed facade with a native
owner or establish the current executable's terminal provider bindings.

The direct native stores therefore supply no second-array producer. However,
the whole-owner escape remains real, and G is already published before any
registration error or allocation callback. This evidence does not prove the
second array stays empty, justify a no-op teardown callback, or establish
whole-program absence of writers.

## Verification

Fresh guarded queries verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 18 captured native spans, totaling 1,667 bytes,
match the installed executable SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
They decode completely into 616 instructions; final return, tail-jump and
throw-call boundaries are retained. The report also preserves 28 instruction
contexts and verifies all six direct validation calls and all twelve calls in
`00BD0700`.

Only this document and its JSON report change. No source, Ghidra database,
ledger, build, native execution or runtime test was changed or run. Static
native-byte/store evidence is separate from CRT/ABI closure and game validation.
