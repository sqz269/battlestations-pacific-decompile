# Menu pointer dispatch and copy ownership

This batch reconstructs pointer hit testing and Listbox activation through the
existing canonical widget, input, page and listener owners. The complete normal
MainMenu registration caller now composes its existing layout, spacing and
renderer services. Text copy construction advances through an explicit base-copy
admission boundary; it does not yet enable a complete copied Text factory.

## Pointer and registration behavior

`00AA2F10` publishes pointer position and scans the live page registry.
`00AA8BD0` visits children before the current widget, uses the actual bounds
dispatcher, and selects the smallest child-origin Z; equal depth keeps the first
hit. Assembly review corrected an earlier interpretation of that Z operand as a
pivot product. The selected widget then receives current input activity and its
actual current68 dispatch.

Seven supported base profiles use `00AA7190`, which bubbles to the parent when
there is no listener or the native gate requests it. Otherwise it calls current
listener00 and button-edge listener04/08 with the native device capture and
listener reload order. Listbox has a distinct current68, `00A9CA60`; it must not
inherit this base implementation.

The Listbox pointer caller preserves the separate row selection and current80
refresh, live paging-delay reads, activation publication `00F8BC84`, listener04/10
and sound reloads. Its second button edge reads raw current/previous history,
including when ordinary device-query validity is false. Only an ordered zero
from current24 permits the activation path. The activation adapter at `00696430`
requires the actual game action55 provider. MainMenu listener10 at `004F8F30` is
an empty RET8 body, now defined separately from its following alignment bytes.

`00582F30` registers the same screen, invokes its actual layout and spacing
operations, writes the established screen fields in order and calls the required
renderer presentation service with live settings. The executable's manual menu
attachment path remains separate and has not been converted into full native
screen registration.

## Copy boundaries

`00AA9520` copies into a fresh canonical base owner without invoking the ordinary
base/default Text constructor. It preserves destination words the native caller
does not write and keeps acquired model creators visible across an interrupted
provider call. Type admission is a separate operation, after copy completion.

The table at `00D5C0B8` contains DWORD clone flags, not type names. Types0 through
16, including Text, use `0x3E`; types17 and18 use `0x26`. The native primary Model
current10 call is at `00AA96FC`, with parent zero. It obtains the cloned name from
the current source Model. Text's `0x3E` additionally copies geometry streams;
the existing `0x26` helper cannot stand in for this required provider.

`00ABB2C0` now has an explicit after-base derived lifetime admission and ordered
cursor/draw-section/content continuation. `00AB8910` constructs the cursor with
actual Model, mesh, section, material and layout owners and required renderer
slots. Material retention still requires the real raw Text identity and its
reference-count/deleting-owner contract; the semantic C++ widget cannot replace
that identity. An unfinished copied lifetime remains constructing and blocks
external owner operations and retirement. There is no copied
`GuiTextRuntimeImplementation` adoption path or completed Text clone factory.

## Evidence and validation

The batch report records source commits, saved annotation preimages, instruction
and call-site checks, and the exact build/fixture results. Fixture coverage is
bounded: actual Group/Model ownership, pointer hit ordering, listener dispatch,
raw input queries, Listbox callback reloads, and the base-copy prefix at an
explicitly interrupted `0x3E` provider. Successful `0x3E` stream cloning and copied
Text continuation are not exercised. No permanent tests were added.

The reconstructed interfaces are new C++ interfaces. Populated menu registry
execution, the enabled pointer/cursor path, renderer presentation, full copied
Text ownership and gameplay remain unvalidated. Exported bodies, reconstructed
callers, compilation and scoped fixtures are separate evidence levels.
