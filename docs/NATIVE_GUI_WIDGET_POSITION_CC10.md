# Raw GUI widget position setter

`set_native_gui_widget_local_position_00aa7dc0` reconstructs the complete
normal body `[00AA7DC0,00AA7DF4)`, 52 bytes. Its descriptive name is a
hypothesis. Native ECX is the actual widget, the stacked argument points to
three float32 coordinates, and the final instruction is `RET 4` at AA7DF1.
The explicit source interface adds caller-owned transform/bounds scratch and
the actual provider/global bindings; it is not binary compatible.

The three MOVSS loads at AA7DC4, AA7DC8 and AA7DCD precede every destination
write. Their DWORD bits are captured first, then stored at widget+0C, +10
and +14 in that order. The source preserves this snapshot even if the input
overlaps those fields. These are bit copies, with no floating-point
conversion or x87 effects.

The same widget then reaches AA7220 at AA7DE4 and AA70E0 at AA7DEB. Both are
the actual raw providers. The transform's external virtual dispatch can
mutate the widget, so the bounds routine executes afterward against current
storage. No bounds state, model pointer or half constant is captured early
by this wrapper. There is no parent traversal, logical GuiWidgetOwner
projection, validation fallback, implicit scratch initialization or rollback.

Every underlying scratch, global, actual-owner and dispatch contract remains
required. Exceptions/unmasked faults, native ABI and application/gameplay
equivalence are not established. Validation and exact call/byte evidence are
recorded in `reports/native_gui_widget_position_cc10.json`; the wrapper has
static/build coverage only. No new tests are needed for this small composition.
