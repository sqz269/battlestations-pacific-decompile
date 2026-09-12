# Canonical pointer listener dispatch

Addresses: 00aa7190, 004fa100, 004fa120, 004ba6d0

`GuiWidgetFrameRuntime::dispatch_current68` reconstructs the complete normal
AA7190 caller over the existing widget, parent, listener and input owners.
Its native ABI is ECX widget, one unread stack argument, RET4; the new C++
interface is not binary compatible. The verified body AA7190..AA7218 contains
49 instructions and no gaps. Descriptive names are hypotheses.

If listener DC is null or byte79 is set, it dispatches the same parent's current68
with the child as argument. Otherwise it reloads listener00 and passes this widget,
then obtains class1/index0 from the current F8BBF4 backend. The captured mouse
survives both later calls. Rising button0 invokes fresh listener04; rising button1
then invokes fresh listener08. It does not cache listener identities or histories
across callbacks. Missing providers throw after already completed native effects;
there is no automatic retry or suspended native exception frame.

The CEB110 base and CEFC04 main-menu listener tables both use 4FA100 and 4FA120
for slots00 and08. Both bodies are exactly RET4. Their empty adapters implement
these verified bodies; other listeners must provide their actual targets.

`align_bounds64` now exposes the existing actual Text/base dispatcher while
borrowing the same owner. `GuiInputDeviceRef::activity_current28` captures raw
device+0 and invokes the existing finite NativeInputDeviceRuntime dispatcher.
The separate typed input domain requires an explicit InputFocusDeviceHost;
the old typed constructor does not fabricate activity when that path is reached.

The same captured-device reference exposes current20 and current24, capturing
the raw profile anew at each call or dispatching the actual typed state device.
The mouse A99FE0 axis return spills binary32 at A9A0A0 and reloads it before
RET4, so passing that float to the Listbox comparison does not narrow an
unspilled native result. Its providers remain the existing input implementation.

Supported current68 profiles are Screen, Group, Text, Icon, ClipBox,
Section and FrameBox. Unsupported profiles fail explicitly. Their native slot
words and each call site are recorded in the companion report. The owner domain
requires live canonical parents and listeners through callbacks; recursive entry
to an already active owner is diagnosed rather than supported as native reentry.

Listbox is a separate current68 target: D5BC60 contains A9CA60. It performs
row selection, scrolling and activation and cannot inherit AA7190. This dispatch
currently rejects Listbox explicitly; its full caller remains a follow-up.

Validation: Win32 build and both existing CTest checks passed. The focused local
GUI frame fixture passed one added sequence exercising child-to-parent dispatch, changed
listener identity after00, changed backend after04, and button1 from the retained
mouse. Fixture execution is recorded separately. No new permanent tests were added.
Game, enabled manager hit traversal and binary ABI compatibility are unverified.
