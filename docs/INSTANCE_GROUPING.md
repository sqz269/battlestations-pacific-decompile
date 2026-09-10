# Instance visibility and grouping

The typed fragment of native `00b1dff0` applies descriptor-controlled visibility,
routes entries without an instance binding directly to a render queue, and
collects bound entries into two categories per binding ID. It feeds the recovered
upload traversal through `InstanceUploadGroup`; the resulting entry pointers
remain stable until their queue is consumed. These are new C++ interfaces, not
native object-layout or ABI replacements.

## Evidence and arithmetic

Native `00b1dff0` takes ECX=context and one stack entry pointer, returning with
RET4. Its descriptor is `entry.section.material.effect+ C4`; byte16 enables the
visibility path and float18 supplies the fraction for `00b73770`. It refreshes
the camera and source model world matrices if their valid bit2 is clear, then
uses their translations. The host interface requires those refreshed positions.

The difference is stored as three floats. `00419440` stores each squared term,
adds y-squared and x-squared before z-squared, stores the sum, and calls the CRT
square-root helper with an x87 register input. The finite nonnegative square-root
core is used here; the CRT's exceptional-input diagnostics are not reconstructed.
Camera modes selected by `(1 << (mode & 31)) & 0x17` use the clamped float result
of `(camera[178] - distance) / double(200)`. Other modes use one.

`00b73770` chooses default threshold bits `3f7eb852` when its native stream index
is zero; otherwise it reads `object + 4 + index*16`. The typed caller supplies
the resolved threshold. The routine stores width=`threshold*fraction` as float,
keeps the subtraction/reciprocal/product in x87, stores the final fade as float,
and clamps only ordered values below zero or above one. NaNs pass through.
The grouping caller multiplies this fade by prior entry visibility and camera
fade without an intermediate float store, then writes entry+18 even when it
rejects the result. Rejection compares with the native double at `00ceb690`,
which is exactly promoted float0.03 (`0.029999999329447746`).

At `00b1e1c7` the original temporarily sets x87 truncate rounding, converts the
visibility to a signed qword, and uses its low DWORD with unsigned min1. A zero
low word selects category1; everything else selects category0. This is not a
general comparison with one: negative/nonfinite/large inputs have the observed
conversion behavior. The host retains this instruction sequence and restores the
control word. Masked floating-point exceptions are the normal supported domain.

## Group and queue behavior

Without section+5C binding, ordered visibility below one selects context queue1;
otherwise the effect's queue-index DWORD at AC selects the queue. The borrowed
source entry pointer is appended through the recovered queue helper.

A binding's DWORD+8 is an index into the context's zero-filled pointer array.
On first encounter native code publishes a new 4Ch group and retains the binding
before constructing its two generated models. It allocates two output entries
from a global InterlockedIncrement pool, creates category0 then category1 geometry,
and reserves eight source pointers for each category. It mutates cloned diffuse
RGBA to one of red, green, blue, yellow, magenta by unsigned ID modulo5. The sixth
initialized cyan color is unreachable through that modulo. The host factory
receives that actual tint and must apply it to the retained material projection.

The selected count becomes one, the group enters the ordered traversal list,
and the borrowed source entry is appended. Later encounters increment that
category's count before appending; the first binding/generator remains retained.
Source order is preserved here. Category1 sorting occurs in the upload routine.

The typed state owns pinned group and output-entry storage and retains binding,
generator, model and geometry dependencies. `std::vector` replaces native raw
pointer array allocation; raw capacities, exact pool addressing and intrusive
reference counts are not reproduced. Factory calls are mandatory. Allocation
failure/invalid host inputs return errors or throw; a failed group factory leaves
the published group, and that frame must be abandoned. Native diagnostic string
composition and full model/material constructors remain separate dependencies.

## Validation boundary

`reports/instance_grouping_audit.json` records installed-PE/live byte identity,
exact native spans, ABI and proposed annotations. Compilation and the installed
model draw are recorded by the primary batch integration report after integration.
No full scene manager, native frame pool, binary replacement or gameplay is
established by this fragment.
