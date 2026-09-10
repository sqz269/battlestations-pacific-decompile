# Building instance data

`src/building_instance.cpp` projects the complete value-writing behavior of
`00b55780` into a 36-float record for the installed `InstanceGenerator="building"`
route. Inputs are an already refreshed world matrix, ordered point-light values,
entry visibility and material diffuse alpha. The helper does not create a
substitute scene object or change the material shader.

The exact instance declaration name is
`uf44uf44uf44uf44uf44uf44uf44uf44uf44.mvfm`: nine float4 elements, 144 bytes.
`building_instance_vertex_format`, `building_instance_stride` and
`BuildingInstanceData` expose that contract to the existing declaration/buffer
and material draw integration.

## Native selection and declaration route

All six owned complete functions, their two already named getter dependencies,
the building vtable, declaration string, selector literal and unsigned-count
conversion constant were captured from the verified `bsp` Ghidra project and
matched against the original installed PE. Full spans, hashes and original
annotations are in `reports/building_instance_data_audit.json`. The configured
program is `/battlestationspacific.exe`, image base `00400000`, x86 little-endian
32-bit; the existing project is `C:/Users/sqz269/bsp.gpr`.

`00b451d0` receives ECX effect and two stack arguments, then returns with
`RET8`. It reads the primary descriptor at effect `+C4`, checks the
`InstanceGenerator` string at descriptor `+28/+2C`, and selects the building
constructor `00b450d0` when it matches the native `building` literal at
`00d61c5c`. It also recognizes the separate `generic` route. Empty or
unrecognized strings return without creating a binding.

The building constructor allocates no scene values. It supplies the exact
41-character declaration name at `00d61c28` to base constructor `00b55b20`,
then sets its vtable to `00d61c1c`. That vtable's `+8` entry is `00b55780`.
The constructor receives ECX generator and two stack arguments, returns the
generator in EAX, and ends with `RET8`.

Base constructor `00b55b20` receives ECX generator, a draw section, a forwarded
opaque argument and the instance declaration name, `RET0C`. It sets reference
count `+4` to one, clears generator `+8/+C/+10/+14/+18`, resolves the instance
declaration through renderer virtual `+38`, and stores it at generator `+10`.
It obtains the selected section's first stream declaration through
`[section+3C]` virtual `+24`, then supplies that declaration followed by the
instance declaration to renderer virtual `+40`; the result goes to generator
`+14`. The forwarded second argument has no read in the inspected base body.
This establishes declaration order for the installed route, not a replacement
for native renderer allocation or the complete instancing lifecycle.

After construction, `00b451d0` places the generator in a separate 10h-byte
binding object, then attaches that object to the draw section:

- `00b41780`: ECX binding, stack generator, `RET4`, retained field `+C`.
- `00b417e0`: ECX draw section, stack binding, `RET4`, retained field `+5C`.

Both setters skip identical pointers. Otherwise they store the new pointer,
increment its count at `+4` when nonnull, decrement the old pointer's count,
and call old virtual `+0` when the decrement reaches zero. In `00b451d0`,
each local reference is released after passing it to the corresponding setter.
These complete native ownership operations are audited; the value writer does
not introduce fake C++ native owner objects or claim to reproduce their lifetime.

## Record writer and layout

`00b55780` receives the render entry as stack argument 1 and the output pointer
as stack argument 2, and ends with `RET8`. ECX is unused in the complete body;
its placement at the generator vtable's `+8` is consistent with virtual-call
use. The output must cover 144 bytes.

Native entry `+C` supplies the transform. If transform valid bit `+5C & 2` is
clear, the native writer calls the existing `00b6db70` world refresh. It then
reads world matrix `+F0`, whose 16-float storage matches the existing
`CameraMatrix`/`CameraTransform.world` representation. The new helper accepts
the refreshed matrix explicitly; callers retain responsibility for refresh.

| Output float4 | Source and final contents |
| --- | --- |
| 0, words 0..3 | world indices 0, 4, 8, 12 |
| 1, words 4..7 | world indices 1, 5, 9, 13 |
| 2, words 8..11 | world indices 2, 6, 10, 14 |
| 3..5, words 12..23 | First three ordered light position/radius float4s, light `+1EC..+1F8`; zero when absent |
| 6, words 24..27 | First light RGB; W becomes entry visibility |
| 7, words 28..31 | Second light RGB; W becomes clamped light count as float |
| 8, words 32..35 | Third light RGB; W becomes material diffuse alpha |

The light list comes from `00b6dc50`, which returns transform `+164`.
The pointer array is at list `+0`, its DWORD count at `+4`. Native valid
nonnegative counts of three or more clamp to three; ordered inputs beyond the
first three are ignored. Each light first supplies its complete position/radius
record and complete color at light `+184..+190`. An absent light writes two
positive-zero float4s. Only after all three slots are populated does the writer
overwrite color W fields, in this order:

1. Word 27, output `+6C`: entry `+18` visibility.
2. Word 31, output `+7C`: clamped light count converted to float.
3. Word 35, output `+8C`: material diffuse alpha. Native entry `+4` gives the
   section; section `+20` gives the material; `00b179f0(0)` returns material
   `+38`, whose `+C` is alpha. The getter ignores its stack index and uses
   `RET4`.

The typed writer preserves this sequence, including full source color copies
before the three scalar overwrites. It does not multiply lights by visibility,
scale radius, infer falloff, choose scene lights, or replace the supplied diffuse
alpha with a default.

## Count, float and interface boundaries

The native count clamp at `00b55814..00b5581A` is signed, but later
presence checks are unsigned. A corrupted count whose high bit is set bypasses
the signed clamp, can read all three light pointers, and is converted with
`FILD` followed by addition of the exact float `4294967296.0` at `00ce3978`.
That malformed-container behavior is not equivalent to a valid list count.
The host interface explicitly accepts vectors of at most `INT_MAX` elements;
larger vectors fail without altering output. Within that domain its count and
first-three selection match the native branches.

Native matrix words and the final visibility/alpha values use sequential x87
`FLD/FSTP`. Light position words use `MOVSS`/integer stores and light colors
use integer copies; the typed helper preserves their bits with `memcpy`.
For world/visibility/alpha it is a float value projection: ordinary finite
values agree, while native x87 exceptional NaN conversion and floating-point
exception state are not reproduced. Its staged output deliberately provides
stable host input/output overlap behavior; native arbitrary overlapping scene
and output storage is outside the interface.

The helper has a new C++ ABI. It reconstructs the record contents; generator
selection/construction and retained setters are native audits, not completed
host implementations. Real GPU buffer allocation, stream frequency, combined
declaration, shader compilation, constants and material drawing remain in the
primary integration. Build success and byte parity alone do not demonstrate a
rendered mesh or original game behavior. The audit records build/readback/draw
results only after those checks run.
