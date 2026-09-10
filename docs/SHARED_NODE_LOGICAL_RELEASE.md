# Shared node logical release

`release_node_logical_00b6f310(NodeLogicalReleaseState)` now contains the single
reconstructed `00B6F310` traversal. The generated-model entry delegates to it
using its existing vector and owned diagnostic fields. A concrete native owner
can supply its actual transform, released byte, self-reference and point-light
array without creating another model, another vector or another hierarchy.

The interface is new C++ targeting MSVC Win32. It does not reproduce the native
object ABI. Descriptive names remain reconstruction names. This storage
refactor does not reconstruct a camera reference adapter, native count binding
or a different terminal destructor; those consumers are separate packets.

## Borrowed state

`NodeLogicalReleaseState` contains references to the existing
`GeneratedModelLifetimeRuntime`, `CameraTransform`, actual `uint8_t` released
byte `+44`, and the concrete `RenderCommandReference`. Its
`NodePointLightReleaseView` contains only a context pointer and three required,
nonthrowing callbacks:

| Callback | Contract |
| --- | --- |
| `live_count(context)` | Read the actual current count for every unsigned loop comparison. |
| `live_element(context, index)` | Read the actual current begin/element and resolve that borrowed light binding. |
| `shrink_to_zero(context)` | Perform the `00B6EC70(0)` count shrink, retaining allocation and capacity and releasing no light owner. |

The state is passed by value, but its members only borrow their owners. It does
not read or initialize the released byte during construction, acquire a self
reference, allocate list storage or extend the host companion's lifetime.
Every field must belong to the same node, and all referenced companions must
remain live through their last callback. The final self-release callback may
destroy the backing, the companion and the runtime; the shared operation reads
none of them afterward.

The loop uses `uint32_t`, matching native `EDI` and the unsigned `JBE`/`JC`
comparisons. Native signed descriptors must have a validated nonnegative count
and a valid array extent at the adapter boundary. Negative-count memory walks
are not a supported input domain, and the shared routine does not silently
clamp or reinterpret an invalid descriptor as an empty list.

The existing generated-model wrapper uses private callbacks on the same
`point_lights_164` vector. Its count and elements are read live, and its shrink
calls the existing `resize_generated_model_point_lights_00b6ec70(vector, 0)`.
Generated-model construction, diagnostic reference ownership, physical
disposal and the terminal `00B750C0` path are unchanged.

## Recovered ordering

The installed executable and live Ghidra agree on all 176 bytes in
`[00B6F310, 00B6F3C0)`, including the final `RET`. Ghidra queries use
`tools/bsp.py ghidra`, which verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before reading. The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete evidence and validation hashes are in
[the audit](../reports/shared_node_logical_release_audit.json).

The original ABI is ECX = node with no stack arguments. A nonterminal call
returns with `RET0`; a decrement to zero tail-calls the current virtual `+00`.
No stable return value is assigned to the new C++ interface.

1. `00B6F319..00B6F339` reloads count `+168` for each comparison and begin
   `+164` for each element, calling the existing point-light backlink remover
   `00B7C1A0`. No light reference is released.
2. `00B6F33B..00B6F342` invokes `00B6EC70(0)`. Logical release does not free
   the raw array. The concrete terminal destructor remains responsible for it.
3. `00B6F347..00B6F370` repeatedly detaches the current first child, publishes
   its next sibling as the new head, clears the new head's previous link and
   the detached child's parent/next links, then invokes its actual virtual
   `+18` through the existing runtime association. The next iteration reloads
   the head after the callback. Child count `+38` is never changed.
4. `00B6F372` checks byte `+44`. All preceding traversal runs even when the
   byte was already nonzero. A nonzero byte skips the following work.
5. `00B6F377` captures attached-owner `+A0` before clearing root `+A4`, parent
   `+30`, first child `+34`, previous `+40` and next `+3C`, in that order. The
   routine then stores byte `+44 = 1`. The shared operation now makes this
   capture order explicit; the previous model-only helper read `+A0` after
   these stores, with no intervening callback on its separate-field domain.
6. For a nonnull captured attachment, the existing `00B8F4C0` helper removes
   its matching backlink. The caller repeats the null store to `+A0`; the host
   notification projection is cleared with it.
7. `00B6F3A3..00B6F3BA` releases exactly one self-reference and dispatches the
   current terminal operation only when the count reaches zero. No field is
   read after this release. There is no additional decrement in this wrapper.

`GeneratedModelLifetimeRuntime::unbind` already finds and erases the companion
pointer itself. It does not call `transform()` or read node backing, so a
concrete terminal adapter may use it after returning the native slot. This is
different from scene unbinding: any scene cleanup requiring native fields must
occur before those fields cease to exist. This packet changes neither runtime.

## Validation and limits

`./scripts/build.ps1` passed with MSVC Win32 `/W4 /WX /fp:strict`; its existing
CTest suite passed 1/1. A single ignored host regression sequence in
`local/shared_node_logical_release_check.cpp` also passed under strict Win32
compilation. It covers the new storage boundary without adding tracked tests:

- Both the unchanged diagnostic vector entry and a view over actual
  `NativeNodePointLightArray` fields remove borrowed backlinks and retain the
  same list allocation/capacity.
- A child callback inserts a new head, which is visited before the old next
  child; detached links and preserved child count are checked at callback time.
- A repeated call with byte `0x7F` still removes new light/child links but
  retains attachment/root state and does not decrement the self-reference.
- A cold terminal call makes the actual `NativeNodeStorage` page inaccessible
  before identity-only runtime unbinding. Returning through the shared body and
  virtual wrapper succeeds without accessing the protected backing.

The host sequence uses the existing diagnostic atomic reference counter. It
does not validate the separately developed borrowed native `+04` counter or a
camera pool return. No original function bytes were executed for this refactor,
and the new API is not game-validated or a drop-in ABI replacement. Native EH,
concurrent mutation, invalid array extents, missing dispatch associations and
throwing callbacks remain outside the established interface contract.

The worker changed only the lifetime header/source and this packet's document
and audit. It made no Ghidra, ledger, shared-metadata or CMake changes.
