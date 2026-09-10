# Note resource parser

Addresses: 00408720, 00718710, 00718f50, 00719000, 00b86890, 00b868b0, 00be9fe0, 00bea010.

The game registry's `Note` parser (`00719000`) returns a new reference-counted
item containing one string. The serialized value uses the structured reader's
counted-string read, then the parser copies only the prefix before the first
NUL. This is a payload and ownership contract; it does not make a host string
or resource object compatible with the native layout.

## Parse path and original ABI

`00719000` takes an unused ECX parser and one stack node-handle pointer, returns
the item in EAX, and ends with `RET 4`. On successful allocation it creates a
`28h`-byte item, calls base constructor `00b868b0` (reference count `+4 = 1`),
installs vtable `00cfd860`, initializes the string, then calls `00718f50` with
ECX=node handle and EDX=item`+8`. It adds no further retain before returning.
The allocation-null branch still invokes the helper with destination address
8, so a safe null-result contract cannot be inferred.

| Item offset | Observed field |
|---|---|
| `+0` | Primary vtable `00cfd860` |
| `+4` | Intrusive reference count, initially 1 |
| `+8` | Character-string prefix; no explicit initializer store here |
| `+0Ch..+1Bh` | 16 inline bytes; first byte initially NUL. With capacity at least 16, the first DWORD is a heap pointer. |
| `+1Ch` | Logical string length, initially 0 |
| `+20h` | String capacity, initially 15 |
| `+24h..+27h` | Allocated tail not interpreted by these bodies |

`00718f50` returns its original node-handle pointer in EAX and uses a plain
`RET`. Its complete assembly contains no recursive call, despite the original
candidate callgraph. The wrapper chain is:

1. `00bea010`: ECX=node handle; dereference it and forward the hidden stack
   native-string output pointer to `00be9fe0`; EAX=output, `RET 4`.
2. `00be9fe0`: ECX=node; call `00bf0510` with reader=node`+8`, the hidden
   output, and the address of remaining bytes at node`+20h`; EAX=output,
   `RET 4`.
3. `00718f50`: read returned data pointer at native string`+4`, substitute
   address `00e19b94` when null, scan to NUL, call `00408720` with that
   measured count, then destroy the temporary native string.

## String data and byte accounting

The same-session [structured-reader audit](STRUCTURED_READER.md) establishes
`00bf0510` and concrete stream method `00be4620`. The adapter passes a nonnull
actual-count output to stream virtual `+48h`, then subtracts that reported
count from the node's remaining DWORD. Both inspected stream vtables map the
slot to `00be4620`, which reads a DWORD character count through virtual
`+38h`, allocates that logical length, fills its characters with spaces, and
performs one raw read through virtual `+24h` for the declared count.

The reported transfer count includes actual prefix bytes plus actual character
bytes. A short character read leaves spaces in the unfilled suffix and keeps
the declared stored length. Zero length yields an empty native string. A
short DWORD prefix may retain pointer-address bits in unread bytes, so a
portable bounded implementation must reject an incomplete prefix instead of
claiming a deterministic native length.

The Note helper subsequently measures with `strlen`. For example, serialized
characters `a`, NUL, `b` consume all three transferred bytes but produce Note
text `a`. For a complete DWORD length prefix of 3 and only one transferred
character `a`, the intermediate logical string is `a` followed by two spaces,
subject to the established stream and allocation contracts.

No helper in this parse path explicitly skips the node's unread payload.
Closing, shared-cursor lifetime, and parent accounting remain the reader and
outer dispatcher contracts. Remaining-byte bookkeeping is not a bounds check.

`00408720` is an observed MSVC-style counted-character assignment; its exact
library symbol is unproven. ECX is the destination string, stack arguments are
source pointer and unsigned count, EAX returns the destination, and it ends
with `RET 8`. Relative string offsets are buffer `+4`, length `+14h`, capacity
`+18h`. The fresh, non-aliased path reserves when necessary, copies the count,
sets length, and writes a terminating NUL. Alias assignment, reserve internals,
CRT failures, and native SSO layout are external to the host payload projection.

## Native destruction

The first four vtable slots at `00cfd860` are `00bd30e0`, `00718710`,
`00713750`, and `00717ca0`. The established `00bd30e0` helper invokes virtual
`+4(1)` for a nonnull object; it does not decrement a reference count itself.
The last two slots remain unaudited here.

Scalar deleting destructor `00718710` takes ECX=item and a stack deletion-flags
byte, returns the original pointer, and ends with `RET 4`. It frees item`+Ch`
only when capacity at `+20h` is at least 16, then restores capacity 15, length
zero, and first inline byte NUL. It calls `00b86890`, which writes an
intermediate vtable and tail-jumps to the established reference-base destructor
`00bd30f0`. The outer item is freed only when deletion-flags bit 0 is set.
`00b86890` makes no pointer adjustment despite its old classification label.

The native free calls at `0071871d` and `00718743` both return. Their
continuations contain `ADD ESP,4` at `00718722` and `00718748`; initial Ghidra
flow omitted those instructions and produced false early returns. The primary
integrator must repair those per-call overrides and refresh exports before
using decompiled control flow as evidence. Shared CRT declarations must retain
their established identity.

## Verification and limits

[The audit report](../reports/resource_note_parser_audit.json) contains eight
complete code spans (627 bytes), the 16-byte vtable prefix, original ABI,
SHA-256 values, prior annotations, flow-repair evidence, and annotation
proposals. All listed bytes match live Ghidra and the installed PE. Seven
code bodies decode fully through return; the base destructor decodes fully
through its tail jump. Underlying string-stream reads reuse the independently
audited same-session structured-reader evidence.

A host Note payload containing `std::string` can preserve these parsed
characters and ownership while exposing a new interface. This audit does not
reconstruct native allocator failures, arbitrary stream vtables, native class
layout, the two unaudited Note virtual slots, or an entire resource lifecycle.
No C++ source, shared metadata, Ghidra annotations, tests, or game state was
changed by this packet; no build, fixture, ABI-compatibility, or game-validation
claim is made.
