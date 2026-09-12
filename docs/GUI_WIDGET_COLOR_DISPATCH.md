# Actual widget color getter and alpha dispatch

Addresses: `AA68F0`, `AA6980`. Both use ECX widget and one stack argument,
return with RET4. Getter returns the supplied float4 output pointer in EAX;
alpha setter has no claimed return value. These are new Win32 C++ interfaces.

`read_gui_widget_color_00aa68f0` resolves the same live actual Model, current
mesh, section0 and material. It returns raw material diffuse words when geometry
and a nonzero section count exist; otherwise it reads the canonical base color.
This matters for timed alpha interpolation: the getter must not substitute
widget color when the material differs. Missing or non-Model native node
profiles remain explicit errors before interpreting +180.

The getter preserves the two native alias-sensitive DWORD copy sequences.
The material branch copies words0/1 separately, loads both2/3 before storing
them; the base branch captures0/1, stores0, reads2, stores1, reads3, then stores
2/3. No float conversion, x87 load or bulk copy replaces these operations.

`set_gui_widget_alpha_00aa6980` stores only base alpha, then updates only the
actual material diffuse alpha behind the native geometry/count gates. It
preserves bits through MOVSS-equivalent copies and leaves RGB untouched. The
existing transform alpha is the same projected field. Text current4C remains
its existing AB6AD0 override, including shadow alpha; it is not dispatched to
the inherited base setter.

Live profile tables establish base current54 for Screen, Group, Text, Icon,
ClipBox and FrameBox, and base current4C for those types except Text. Their
current5C is A9E110, which returns the canonical +60 type word. Unknown profiles
throw explicitly. The owner hooks expose these real operations to timed-entry
dispatch. A shared vtable target does not establish a Group-backed Screen's
Model geometry ABI; the Model-domain requirement remains.

Relevant table bases are D5BE38, D5CB80, D5C6C8, D5C4C0, D5D058 and D5D130.
Complete AA68F0..AA6971 and AA6980..AA69DC listings establish stores and RET4;
AD39A0/AC2F40 consume the getter/alpha profile through current54/current4C.
Report rows separate direct calls from resolved indirect profile targets.
Strict Win32 compilation, combined build and focused owner-fixture evidence
are recorded in the integration report; no rendering/gameplay claim follows.
