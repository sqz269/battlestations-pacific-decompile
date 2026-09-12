# GUI actual input and pointer update

Addresses: `004BA6D0`, `00AA3910`, constructor fragment `00AA5D70`;
related Listbox clear `00A9BEC0`.

`GuiInputSource` borrows the actual F8BBF4 publication and existing
`NativeInputDeviceRuntime`. The older typed input domain remains separately
supported. No raw allocation is cast to `InputDevice`, and no backend, device,
class vector or input sample is copied into a second owner. The 4BA6D0 lookup
uses the actual vector at `6C + class*24h`, with begin/end at `+4/+8`.
Widget AA87B0 and Listbox A9D030 bind the same publication identity.

`GuiPointerRuntime` reconstructs AA3910's complete normal caller (194 native
instructions, AA3910..AA3BC6, ECX manager, RET). It captures the first mouse,
sets the latch bit before first sampling, and queries X/Y three times on the
first frame or twice thereafter. Each X/Y publication waits for its Y call.
Disabled manager48 still updates the latch; an empty mouse vector returns
before all pointer writes. Device calls use the existing finite native-profile
dispatcher. Hardware polling and the native class-vector producer are separate.

Integer latch samples retain SSE CVTSI2SS signed conversion; differences use
x87 FILD/FSUB/FSTP. Both double factors are captured once per pair and each
multiply spills to binary32. Position accumulation, unspilled vector-length
comparison, X clamp, mixed SSE/x87 Y clamp and argument spills retain their
native order. Unordered comparisons do not wake a hidden cursor or force a
NaN coordinate to a bound. Cursor50 is reloaded at native callback boundaries.
The last current88 requires the same actual Icon companion.

The manager owns one pointer-field set. AA5D70's fragment captures live CE3800
once for both position lanes, clears enabled48 and exclusive6C, and leaves
delta64/68 untouched. Missing position/delta production is represented by
disengaged optionals, without invented coordinate defaults. This fragment is
not the full manager constructor.

AA2F10 remains an explicit required provider. It must execute its actual page
walk and AA8BD0 recursive hit tests, using the same manager and frame globals.
The caller provides no successful fallback. Manager, position storage, devices,
cursor companions and providers must survive callbacks. Corrupt native vectors,
original SEH and whole-object binary compatibility are outside this C++ API.

The accompanying A9BEC0 clear uses the sole FC row nodes: suppress114, delete
nonnull payloads through actual current4(flags1), clear nodes, select end,
restore114, clear110, refresh80(false). It does not route through per-row
removal, which would introduce additional visibility/layout/listener effects.
The official flow tool repaired its ten-byte post-free gap.

Release Win32 /W4 /WX build and both existing CTests passed. Those tests do not
establish pointer, hardware input, render, native ABI or gameplay equivalence;
focused fixture results are recorded separately in the batch report.
