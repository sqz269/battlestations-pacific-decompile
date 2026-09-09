# Hardware vertex layout conversion

`00b60790` creates a layout from input stream declarations; `00b60a10` creates the
COM declaration lazily from retained streams when layout `+40h` is null. Both use
the same packing algorithm. `00b48a00` retains a declaration in the next 12-byte
stream slot at `+8h`, initializes the other fields to 1 and 0, and increments count
`+38h`. `00b47d60` sums declaration strides into layout `+3ch`.

The C++ layout ports append and stride calculation using shared ownership. Its
creation method ports the conversion/API/stride portion of `00b60a10`; diagnostic
strings and singleton calls remain omitted, so it is counted as a fragment. The
new interface returns HRESULT/S_FALSE and releases its COM owner on destruction;
this does not recreate the native constructor, registry or full destructor.

For every stream in order, conversion emits each flat declaration record as an
8-byte D3DVERTEXELEMENT9: stream number, low 16 bits of offset, low 8 bits of type,
method and usage. UsageIndex comes from fourteen counters shared across streams.
The record's final -1 field is not read. Counters increment by usage rather than
resetting per stream. The converter appends the D3D declaration END element and
calls CreateVertexDeclaration, then recalculates aggregate stride even on failure.
The native code uses stack arrays; the port uses a vector and bounds preconditions
for four streams and valid counter indices. Large or invalid declarations and
allocation-failure behavior are not claimed equivalent.

The diagnostic probe now creates a hardware FLOAT4/POSITIONT plus D3DCOLOR/COLOR
layout, queries its elements through GetDeclaration, verifies element count 3
(including END), color offset 16, and stride 20, then binds it with
SetVertexDeclaration. It no longer uses SetFVF. Both non-indexed and indexed pixel
readbacks pass with this declaration. This verifies the single-stream conversion
and rendering path; cross-stream usage numbering is assembly-grounded and has
not been checked against native execution. No additional CTest cases were added.

The device call sites were located through the CreateVertexDeclaration vtable
offset 158h and checked in assembly, including byte/word truncation and the shared
usage counters. Retained evidence is in `reports/hardware_vertex_layout_*`.

## Renderer binding and atlas integration

`00b23f20` is a thiscall method with one logical layout argument (RET 4).
It compares the renderer's cached layout at `+17b4h` before entering the optional
guard, retains the replacement and releases the previous layout. For a nonnull
replacement it obtains the native declaration through virtual slot `+8h` and
calls device `SetVertexDeclaration` (vtable `+15ch`). A null replacement releases
the logical cache without unbinding the device declaration. Every replacement,
including null, increments renderer counter `+1bach`; identity skips do not.

The concrete constructor `00b60cb0` installs vtable `00d62af4`; its `+8h` entry
`00b5ff00` is exactly `mov eax,[ecx+40h]; ret`. This getter does not lazily create
a declaration. The C++ binder preserves this behavior and uses shared ownership
for the logical layout. Its HRESULT/S_FALSE return is a new interface; native
code ignores the device HRESULT. Native intrusive reference counts, object ABI,
and concurrent misuse are not reproduced by this interface.

The installed-atlas probe now uploads its quad through the recovered logical
vertex stream lock/unlock/bind path and draws with `00b21b40`, using this binder
for a FLOAT4/POSITIONT plus FLOAT2/TEXCOORD declaration. It checks identity skip,
null replacement, the retained device declaration and two replacement counts.
The resulting BMP hash is unchanged from the earlier atlas probe. The Win32
Release build, both existing CTests and the complete D3D9 probe passed; no new
CTest was added. Byte comparisons and assembly are retained in
`reports/vertex_layout_binding.json`.

The host still projects recovered GUI coordinates into POSITIONT vertices and
uses a diagnostic fixed-function material. Full native GUI ownership, material
and generated shader integration remain necessary before a game rendering claim.
