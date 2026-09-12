# Canonical Listbox pointer handler

`GuiListboxRuntime::pointer68_00a9ca60` reconstructs the complete normal
`00A9CA60..00A9CC46` caller: 147 instructions, no flow gaps, ECX actual
Listbox and one incoming-widget stack argument, RET4 at `00A9CB55` and
`00A9CC44` (both length 3). D5BC60 contains `60 CA A9 00`, establishing
this distinct current +68 target. Unlike `00AA7190`, its argument is read.
This is a new C++ interface over existing owners, not a raw180h/native ABI.

The parent frame binding enters this method through
`GuiWidgetFrameRuntime::dispatch_current68`, holds its actual owner borrow,
and requires the same input publication as frame40. The method also increments
the existing Listbox operation count, preserving its retirement checks. It uses
the same FC rows, selected iterator, listener114, paging fields, and current
input/sound bindings; no device sample, widget tree, or second row list is made.

## Gates and input

The first gates require byte +14A nonzero and hidden77 zero. With +138 set,
incoming null or incoming active85 zero returns. These four new raw fields
were checked against the entire constructor `00A9DF40..00A9E066`: +14A=1
at `00A9E002`, +138=0 at `00A9E010`, +140=0 at `00A9E01E`, and +141=0 at
`00A9E024`. They reside in the existing `GuiListboxFields`; property producers
must mutate these same cells. This packet does not claim full properties.

At `00A9CAA8`, `004BA6D0` returns current F8BBF4 class1/index0 (RET8). This
device is retained across its current +24(10) at `00A9CAB8` and fresh current
+20(0) at `00A9CAD5`. The latter is not a raw history read: actual mouse
`00A99F70` checks validity and button swapping before reading raw button220.
The device query/value APIs reuse the finite native dispatcher or the separate
typed `InputStateDevice`; they do not cast one representation into the other.

The native x87 gate is `FLDZ / FXCH / FUCOMIP / FSTP / LAHF / TEST AH,44h /
JP exit`. Only **ordered zero** continues: equal yields mask40h (odd parity),
nonzero yields00h, and unordered yields44h (both even parity). The explicit
instruction sequence is retained. The complete mouse +24 body `00A99FE0`
was read; its code10 path performs signed NEG, CVTSI2SS, live scaling, then
FSTP binary32 at `00A9A0A0` and FLD at `00A9A0A4`. Thus float transport does
not further narrow ST0. If current +20 returns zero and +140 is nonzero,
the caller returns; otherwise it continues with that captured raw query byte.

## Paging and selection

With paging148 enabled, incoming +15C and then +160 are compared in order.
An arrow match requires nonzero captured current20 and COMISS(positive0,168)
with carry clear; positive or unordered delay exits. It invokes the existing
current84/A9DA60 with raw direction0 or1. After all navigation callbacks it
reads live `00D5BBA0` and stores those MOVSS bits into +168, then returns.
The inspected constant was `3DCCCCCD`; it is not hardcoded. Missing producer
state for reached arrow/delay fields remains an explicit error.

The non-arrow path calls the **current F8BC84** callback at `00A9CB5C`, with
ECX pointing to a byte output. It does not use current144 or F8BC08. The
constructor leaves144 null, but that field belongs to the separate frame40
path. The initialized pointer callback is `00696430`: at `004DD6DB/E0`,
OnInitOnce passes it to `00AA6BE0`, which stores ECX into F8BC84. The complete
`00696430..00696443` body calls `004C43C0` action55 against the fresh E188A8
game, copies AL to the supplied output, and returns with no stack arguments.
The small source adapter requires an actual action provider; it does not
fabricate an action table or successful response. `004C43C0`'s complete body
was read: its 30h action record checks current28/current24 and previous20/1C,
including ordered COMISS comparisons, before returning AL. Its real singleton
and valid record storage remain the provider's obligation.

When the output byte is nonzero and the selected row differs from the incoming
pointer, `A9C740` selects the pointer and performs its own current80 for a
nonnull input; `A9CB88` then calls current80(false) again. The second refresh
is preserved, including paging's repeated listener08/sound effects. Null,
hidden, or nonmember incoming pointers retain A9C740's existing behavior.
If listener114 exists and the current selected row is nonnull and not hidden,
current listener04(row,Listbox) follows. Regardless of this listener/row gate,
the current F8BC0C sound callback receives CL0/DL1 at `00A9CBCB`.

## Second edge and listener identity

After those callbacks, the caller rereads +141. If set, it rereads F8BBF4 and
gets class1/index0 again. Null now returns normally. `0067C170(1)` is the
complete nine-instruction direct history test: current[1] at +0D nonzero and
previous[1] at +10D zero. It does not use swapped buttons, device-valid210,
the earlier mouse, or another current20 query. Existing actual mouse history
accessors implement this read. A rising edge gates listener10(row,Listbox)
using fresh selected/hidden/listener state, then reloads F8BC0C for CL0/DL1
even when no listener or eligible row exists.

At `A9CBAB/A9CBB1` and `A9CC1B/A9CC21`, native code captures listener114's table,
calls `00425E50`, then reloads114 as receiver before dispatching captured +04
or +10. `00425E50` was read through `00425EAC`; in the valid iterator domain
it only reads the existing selected node and has no callback. Captured target
and fresh receiver therefore stay associated in this documented single-thread
domain. The implementation uses the existing listener dispatcher and does not
invent a second vtable-owner registry. Invalid STL diagnostic callbacks and
concurrent listener replacement in that window are outside this domain.

The std::function publications are dispatch metadata borrowing actual global
and owner state, not independent mutable game-state captures. An immediate
callable copy preserves the selected target's lifetime if it replaces its own
publication while running. Each later native callback reloads the current
publication; no callable is reused across a different native call site.

Listener10 is required on `GuiListboxFrameListener`/`GuiListboxFrameCalls` and
dispatched with the same owner/active-call guards as04. The actual main-menu
CEFC48+10 word at CEFC58 is `30 8F 4F 00`. `004F8F30` has no Ghidra function:
the verified live body is `C2 08 00`, RET8 at entry, inclusive end `004F8F32`,
instruction length 3. Following CC bytes are alignment, not body. Only this
proven main-menu implementation gets the empty current10 adapter; other
listeners must provide their actual targets.

## Coverage and validation

Coverage is complete ordinary A9CA60 and complete00696430/004F8F30 behavior
over the documented live owner/provider and valid iterator domain. Downstream
current84, current80, listener04/10, action55, GUI resource/model effects, and
actual input publication bindings retain their existing supported domains.
Unresolved providers throw where reached; completed native effects persist.
There is no pending continuation, implicit retry, successful fallback, or
original SEH/binary ABI claim.

The final combined Win32 build passed, with both existing CTest cases passing.
The call checker verified all 12 direct rows with zero failures; the nine
indirect rows were checked against assembly. The ignored fixture passed through
`frames.dispatch_current68`, checking double refresh, fresh backend/listener/
sound dispatch, activation self-replacement, direct history despite an invalid
mouse flag, NaN gates, the paging delay read after callbacks, active owner guards,
and normal model retirement. Exact logs and call rows are recorded in
`reports/gui_listbox_pointer_runtime.json`.

The fixture uses actual canonical Listbox/Group/model owners and typed mouse
storage. Its resource initialization, input and listener targets are explicit
fixture boundaries; no permanent tests were added. Build/fixture evidence is
distinct from installed GUI, raw-device SDK polling, rendering, complete manager
hit traversal, or gameplay validation.
