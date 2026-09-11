# Native GUI clip parameter registration

Address: `00AA9F10`, named `BSP_UIContext_RegisterClipParameters`.

`register_native_gui_clip_parameters_00aa9f10` binds the same retained widget,
actual `NativeMaterialStorage`, actual parameter pool, and canonical ClipBox
fields. It uses `register_native_material_parameter_00b17e10`; the existing
`MaterialCloneState` projection in `gui_material_binding.cpp` remains a separate
interface. Names are descriptive hypotheses, not recovered symbols.

The original receives the widget in ECX and one material pointer on the stack.
Its `RET4` is at `00AAA0D7`; the last instruction is the five-byte jump at
`00AAA0E4`, giving inclusive body end `00AAA0E8` and length 473 bytes. This C++
interface is not a binary replacement. The full assembly was checked because
the decompiler drops the stack argument and reports overlapping globals.

The traversal tests self, then successive parent pointers, for current type
identity 16. Supported retained GUI implementations expose their fixed native
virtual `+5C` identities through the canonical transform's `type_id`. It captures
that ancestor once, writes the widget's `clip_enabled_e8` before allocating any
name, and registers `cClip`. The current image's `D7A24C=1` and `D7A218=0` constant
profile is used. No arbitrary replacement virtual identities are supported.

| Name | Source | DWORD count | Native call |
| --- | --- | --- | --- |
| `cClip` | Same widget `+E8` | 1 | `00AA9FA1`, `00B18B20` |
| `cClipCenter` | Captured ClipBox `+EC` pair | 2 | `00AAA005`, `00B18B00` |
| `cClipBorder` | Captured ClipBox `+F4` quartet | 4 | `00AAA053`, `00B18AC0(1)` |
| `cAspectRatio` | Supplied actual `00E12FC0` slot | 1 | `00AAA09D`, `00B18B20` |

Each name uses the existing NativeString storage and is released before the
next native phase. `cClip` uses the original resize/copy construction. The
remaining names use `0041E870`. After releasing the first name the code reloads
the live flag. Assembly `UCOMISS/LAHF/TEST AH,44/JNP` skips the other three names
only for ordered zero; nonzero and NaN remain active. There are no subsequent
flag checks. An inactive call leaves existing center/border/aspect records alone.
The wrappers' complete bodies establish counts 1, 2 and four times the vector
count, all with matrix byte zero.

Center and border addresses resolve from the captured canonical ClipBox after
their respective name allocations. The material binder copies parameter names
but borrows source memory. No widget or ancestor retain is introduced. Caller
and resource domains must keep the widget, captured ancestor, initialized clip
fields, material and aspect-ratio slot alive across callbacks and later packing.
An active flag after a callback without a captured ancestor, corrupt ownership,
native allocation failure/SEH, and arbitrary virtual profiles are outside the
supported domain. The C++ temporary headers do not reproduce native stack identity.

All nine attributed native caller sites across seven functions were checked at
their exact instructions. There is also a direct xref at `00ABCE7D`, with live
bytes `E8 8E D0 FE FF`, targeting this routine but having no containing Ghidra
function. It remains explicitly unattributed in the report; no enclosing
candidate was substituted.

Validation: normal MSVC Win32 build with warnings as errors and strict FP passes,
including this source; both existing tests pass. The report verifier checks
exact direct call sites and live callee boundaries. No new tests were added.
This binding has no focused execution, ABI differential, or game validation.
