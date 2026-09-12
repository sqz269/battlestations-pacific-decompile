# Canonical Text child deletion

Addresses: `00AB8EE0`, `00AB8250`, `00AA9730`, `00AB9650`.

`GuiTextChildDeletion` supplies the concrete C++ ownership transport consumed by
`clear_gui_text_glyph_children_00ab80c0`. It operates on the existing
`GuiWidgetOwnerRuntime`, existing logical child list and exact `GuiTextLifetime`.
There is one borrowed companion pointer in the existing widget owner. No Text
factory, replacement widget, native allocator callback or alternate tree is added.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00AB8EE0` | ECX Text; flags DWORD stack; EAX original object; RET4 | Partial: ordinary canonical Text semantic destruction and C++ wrapper disposition; native Text pool return, raw vtable and original return ABI excluded |
| `00AB8250` | ECX Text; no stack arguments; RET | Partial composition: existing actual shader/shadow/glyph cleanup and typed string/vector destruction, followed by the bounded base tail; native strings/pool/SEH excluded |
| `00AA9730` | ECX base widget; no stack arguments; RET | Partial: scene release, current Text-child deletion, self-detach, final main-node release, zero timed-entry header, base child-list destruction. Nonzero timed-entry storage `00AA97F6..00AA9950` and non-Text/unbound child delete4 at `00AA9797` remain boundaries |
| `00AB9650` | ECX destination; no stack arguments; EAX this; RET | Partial extension: same companion association is published after defaults, before `00AB98D8` calls `00AB8530`; native construction remains the existing bounded implementation |

Names describe reconstruction hypotheses, not recovered symbols. Correct CRT and
Win32 library identities are retained. This is a new C++ ABI, not a binary patch.

## Ordering and ownership

`00AB8EE3` first invokes derived destruction. That releases the captured actual
shader, releases and clears the shadow after callbacks, clears the glyph vector
through the concrete child transport, and destroys font-name, shader-name,
optional-wide-string, glyph-vector allocation, source string and final text string.
The original order is preserved by the existing canonical lifetime implementation.

The base call at `00AB83B9` then installs the base dispatch phase. The new code
executes the existing child scene releases followed by the current main-node
release, without invoking the derived hook again. It never calls `retire_tree`:
that is the distinct page-manager sequence with a scene pass before destruction.
The current base object's type query cannot reenter Text's secondary-node hook.

The base child-deletion loop reads the current front after every delete4(1).
A child remains in its parent's list throughout its derived destruction; its
own base self-detach removes and transfers its existing unique_ptr. The detach
sequence is the already implemented node-parent/root/unlink/list-removal path,
with current node reloads after callbacks. The parent's logical count therefore
changes at the native child-base phase, rather than through a preliminary erase.

`AB80C0` separately detaches a glyph before reloading its vector slot. The transport
retains that exact allocation even if callbacks replace the slot with another
child. It consumes only the allocation of the object actually deleted. Remaining
live detached handles must subsequently be deleted or explicitly reattached.

Flags are applied after the same semantic destruction in both cases. Flag bit0
set frees the transferred C++ wrapper. Flag bit0 clear still destroys the empty
base child-list allocations and removes the runtime owner/implementation; it
retains the wrapper allocation. `take_detached_storage` can subsequently transfer
that completed storage for disposal. No `before_destroy` page-retirement fallback
is left on completed storage. External flags0 wrapper owners keep their allocation.
The native reference-count word is not decremented by the final base destructor:
`00BD30F0` only writes its base vtable.

The companion is published before constructor callbacks and unpublished even if
failed-construction shadow cleanup throws. During scalar deletion it remains
discoverable through every native effect, then is unpublished at full base
completion before the runtime record is erased. A later C++ destructor of an
externally embedded, already completed companion does not dereference that dead
record. Destructive reentry and deletion during construction are outside the domain.

## Explicit remaining boundaries

The canonical `pointers_88_90` words are not assigned an invented callback type.
`AA9390` initializes them to zero. Actual producer `AA8B00` grows the pointer/count/
capacity header through `AA77B0`/`AA6F30`, then passes ECX=index, EDX=widget to
`AD3A80`. The latter allocates 14h timed entries: indices0/1 produce profile
`D5CA74` via `AC2ED0`, index2 profile `D5D210`, other indices return null.
`AA87B0` advances these entries through `AD39A0`. Their deleting entries are
`AC2FE0` and raw `AD3A40`, which reset type and conditionally call CRT free.
`AA6F30` allocates its pointer array through `BF55BE`, then uses `BF6989` free.
The CRT body frees through its original heap `0109E1BC` and optional small-block
heap path. There is no existing canonical allocation association for these raw
timed entries/headers. A nonzero header is therefore retained at `AA97F6`; this
packet neither zeroes it nor passes its pointers to an unrelated host allocator.

A boundary throws `GuiTextDeletionBoundary`, retaining the runtime owner,
companion, original flags and transferred wrapper. In particular the void
delete4 call does not return to `AB8153`, so `AB80C0` cannot zero its slot as if
deletion had completed. The companion's scalar-stage field is diagnostic C++
control state, not another native Text projection. There is deliberately no
resume method: a nested failure also needs the parent's clear-loop and derived
cleanup frames, which this packet does not reconstruct. Destroying a transport
or companion containing such unfinished work terminates instead of implicitly
running unfinished native phases. Its owner/environment must remain alive.

Native `AB75A0` is outside the C++ ABI even for successful flag1 cases. Both native
callers pass pool `F8BDF0`; the callee uses the hidden object+1F4 chunk index,
pool+28 block array, free-slot words at block+7E00, count+7E80 and pool+34 minimum
chunk under the pool critical section. A C++ wrapper has none of that allocation
metadata. Flags0 also excludes native container header/sentinel bytes and SEH;
it is not advertised as native ABI completion merely because pool return is skipped.

## Evidence and validation

Every live Ghidra query used the verified existing project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Worker analysis was
read-only. Full callee bodies, actual constructor stores, deleting-vtable bytes,
all direct constructor/destructor/pool callers and relevant whole-listing register
assignments were read. ESI retains Text/base `this`; EBX is initially zero in
destructors; `AA9730` rezeros EBX after timed-entry calls. Native scalar RET4
establishes its one flags argument; every CRT free stack cleanup is four bytes.

The saved `AA9730` function stops at `AA9950` because of an incorrect noreturn
edge. Exact disk listing `AA9951..AA99B8` supplies its remaining list/base calls;
final instruction is RET at `AA99B8`, length1, final inclusive byte `AA99B8`.
Raw timed-entry delete `AD3A40..AD3A5E` ends RET4 at `AD3A5C`, length3, inclusive
byte `AD3A5E`. Neither range was written or redefined by this worker.

Strict MSVC Win32 `/W4 /WX /O2 /fp:strict` compilation covers child deletion,
changed canonical lifetime and existing widget owner. The ignored include overlay
is an exact copy of the parent's canonical `gui_text.hpp` with uint8 shadowed
(SHA256 `0BAE8F4BB3F2593CD4EC286EB96219E6075E4825893542BBA3C6BE69DA69EB88`).
The stronger exact-call report verifier records current results in the report.
No new tests or test targets were added. The normal combined build remains the
parent integration check. The actual Text factory is still disabled while its
virtual behavior is incomplete; these new operations are not reached by the game
executable. There is no executed-operation, native differential or game proof.

## 2026-09-12 timed and clip ownership integration

The base-entry phase now calls the canonical timed allocation owner; nonzero
headers are accepted only with actual ownership. Active allocation/update/drain
or pending base clip operations reject the owner's scalar deletion before phase
stores. Already completed timed retirement is skipped once by page retirement.
This does not establish the rest of the generic widget destructor. See
GUI_TIMED_ENTRY_OWNER.md and reports/orch5_timed_clip_batch.json.
