# Raw Button storage and registry prerequisite

`native_gui_button_storage` provides actual Button allocation, construction,
copy, destruction and scalar retirement. `native_gui_button_identity` registers
one canonical companion over the same existing atomic+04. Registry linkage uses
the caller's **same actual F8BD38 list and F8BCF8 DWORD**. There is no private
sentinel, process/global owner, projected `GuiButtonRegistry`, second count,
layout, `GuiWidgetOwner` cast or native retain. Input is outside this packet.

The missing process singleton producer is an explicit source prerequisite: it
must own and initialize the sole actual list/focus storage and lend those exact
objects. `NativeGuiButtonRegistryBinding` only borrows them and preflights an
initialized aligned sentinel outside native operations. It creates/initializes
nothing. No existing projected registry can opt in by copying its contents.
Retained factory admission, page construction and native Button dispatch remain
separate prerequisites; this packet does not activate them or a manager chain.

Evidence comes from saved listings and narrowly inspected live handler tables
in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, function count77629.
Names are descriptive hypotheses. Interfaces are new C++, not drop-in native
register/stack/FH3/SEH adapters; fault transport and game delivery are unproved.
Existing names/comments were preserved and evidence appended through the Ghidra
write lock; saved preimages are in
`local/ghidra-annotations-20261024T114709Z.json`. Affected exports were refreshed.

| Original entry | Actual-storage contract |
| --- | --- |
| `00AA4440` | ECX optional source, EAX slot/null, RET. RequestFCh through existing AAC260; default AAC6F0 or copy AA3C60. Split callable arms require only reached inputs. Constructor failure returns its captured failed slot through AABD60, after constructor cleanup. |
| `00AAC6F0` | ECX actual Button, EAX same, RET. Raw AA9390(type15), profile D5BED0, DWORDs EC/F0/F4/F8=0. Append to actual F8BD38 with the native capture/allocation/count/link schedule below. Only default construction registers. |
| `00AA3C60` | ECX destination, stack source, EAX same, RET4. Raw AA9520, D5BED0, DWORD EC/F0 copies; zero then deep-copy actual F4 pooled string. Does not read or change Button registry/focus. |
| `00AAC530` | ECX actual Button, RET. Stamp D5BED0, remove ALL matching payloads from SAME actual registry, clear ANY nonzero F8BCF8 without identity comparison, release CURRENT F4 string, raw AA9730. This applies to copies too, even though they were never registered. |
| `00AA3D10` | ECX actual Button, stack flags, EAX saved identity, RET4. Complete AAC530, inspect CURRENT flags low byte afterward; SAME AABD60 slot return iff bit0. No count decrement or count test. |

The existing `GuiButtonPoolTraits` bind F8BD00, pool profile D5C238, bodyFCh,
slot100h,64 slots/slab and allocator AAC260. Explicit existing
`GameNativeGuiWidgetPoolProcess<GuiButtonPoolTraits>` startup is required.
FC is the pool-owned slab index. The derived tail EC..FB contains two borrowed
DWORDs EC/F0 and native8h string F4/F8 (length/data). Listener/label-widget names
are interpretations; these bodies only copy their bits, with no extra retain,
projection or allocation. Canonical raw AA9390/AA9520/AA9730 own all base state.

The actual registry header is the same0Ch `{opaque, sentinel, count}` layout as
the existing `NativeEffectDeletionListStorage`; each node is0Ch `{next,prev,value}`.
No additional list/count is stored in metadata. Native AAC6F0:

1. Captures current sentinel at F8BD3C, its previous node, and address of that
   captured sentinel's previous cell **before allocation**.
2. AAC210 allocates0Ch in the same existing CRT domain, writes captured next/
   previous, then reads the caller's Button pointer cell for the payload.
3. AAC5D0 grows the CURRENT header count by1 using unsigned
   `3FFFFFFF-count < increment` length-error semantics.
4. Writes the allocated node to the captured sentinel-previous cell, reloads
   the node's CURRENT previous pointer, then writes that node's next cell.

AAC210 is instruction-equivalent to existing A9B790/8665F0; AAC5D0 to existing
A9D480/8675E0. AAC310 is the same full remove-all body as existing A9BD50,
including initial-end/payload capture, current-head iterator checks, link
rereads, node free and CURRENT count decrement. These are qualified stock list
specializations reused through the canonical services, **not new vendor-library
reconstruction credit**. The list holds raw pointers without retaining payloads.
Native checked-iterator/error ABI remains the existing source substitution.

F4 copy reuses the existing raw pooled-header 00426060 schedule matching the
inlined AA3C60 body: identity capture, zero both destination fields, resize to
current source length with preserve=true, late source-length guard, then current
destination length/source-data copy. The SAME `NativeStringRawPoolContext` is
required; no noexcept adapter or `std::string` is introduced. Existing raw-helper
zero-byte-copy and owning CRT exception transport limits still apply.

Constructor/destructor failure states were checked through original FuncInfo:

| Body / handler | Map and reached unwind |
| --- | --- |
| AAC6F0 / CB7626 | DEDEE0 -> DEDED0: state1 CB7618 destroys actual F4 header, then state0 CB7610 calls AA9730. |
| AA3C60 / CB7008 | DED6D0 -> DED6C8: state0 CB7000 calls only AA9730. A failed string copy has no derived-string cleanup. |
| AAC530 / CB75D8 | DEDE80 -> DEDE78: state0 CB75D0 calls AA9730 if derived work fails. Normal base destruction is already state-1 and is not retried. |
| AA4440 / CB70D0 | DED7E0 -> DED7D0: both CB70C0/CB70C8 return the captured failed slot through AABD60. |

`NativeGuiButtonDefaultAcquired` retains reached stages and the allocated node
as diagnostics, not another owner. Native count-growth failure occurs after node
allocation but before linking; constructor unwind frees string/base, **not that
unlinked node**. The provider does not invent rollback. Its opaque payload may
name a subsequently returned widget slot and must not be treated as a live
Button. Copy retains its existing base-clone acquired frame and partial string
state; only observed base cleanup runs. Second failure during true unwind
terminates through the existing source policy. Failed frames/allocations require
explicit diagnosis; operations are one-shot and cannot be replayed.

`NativeGuiButtonReference` borrows the completed positive actual+04 atomic,
validates D5BED0 cells0/1 as BD30E0/AA3D10 (live bytes confirmed), and registers
that exact reference in the canonical renderer actual-owner registry. It never
registers in the Button list or increments/initializes the count. Zero follows
BD30E0/current04(flags1); explicit scalar remains count-independent. Retirement
unbinds the opaque raw key after terminal work with no post-pool payload read.
External serialization prevents reuse until unbind completes. Live companion
destruction terminates; entered terminal failure retires metadata but preserves
failed storage/providers without retry. A failed registration leaves native
allocation cleanup with the caller.

`NativeGuiWidgetLifetimeContext` still requires genuine current table/class,
child scene/scalar, timed-entry scalar and list-removal services over actual
objects. There is no successful empty fallback. No focus/input callbacks, raw
child/layout producer, page authority or process registry owner is fabricated.

Validation: `scripts/build.ps1` completed Win32 Release, including both new
sources; existing CTests passed2/2 (10.56s). The temporary private CMake source
registration was restored byte-for-byte (SHA256 equality checked). No new test
or game run was added. This is source/build proof, not native ABI or native
Page/Button runtime delivery.
