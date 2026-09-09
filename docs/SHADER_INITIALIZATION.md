# Shader field initialization and vertex input decoding

`00b357d0` emits zero assignments for every field in a supplied list. Its ABI is
thiscall with instance-name string and field-list arguments, RET8. It emits
two tabs, instance name, dot, field name, `=0;` and a newline. It ignores type,
component count, mask and semantics. Assembly preserves two variadic arguments
that pseudocode collapses: the field name is pushed before calling the instance
string getter `00419ca0`, followed by the instance name. Names follow `%s`
termination. The new string/vector interface does not reproduce allocator ABI.

`00b35820` uses ECX builder with no stack arguments (RET). Base descriptor byte
+1Fh enables it. It visits the prefix bounded by the smaller unsigned value of
descriptor +20h and builder input-field count +8h. Each field receives:

```hlsl
IN.Name=IN.Name * cVtxElemScale[i].xyzw + cVtxElemOffset[i].xyzw;
```

The actual swizzle comes from field component count via `00b34e90`: x, xy, xyz,
or xyzw for 1 through 4; other counts yield an empty string. It does not consult
component mask or semantic. The port retains that malformed-input text behavior
instead of inventing a valid fallback. Disabled decoding emits nothing. Constant
array declarations are the caller's responsibility. The native constant emitter
omits array syntax for count one; decoding therefore needs a compatible registry
declaration, not an assumed one-element vector array.

Original/saved body matches, assembly and source/swizzle literals are in
`reports/shader_initialization.json`. The existing shader probe checks all four
output zero assignments and input prefix clamping (limit3 with one input), then
compiles this code in its generated vs_2_0 shader and verifies device binding.
The first fixture declared scale/offset with count1 and failed HLSL compilation;
it was corrected to count2 while preserving native emitter behavior. Win32
Release build, both existing CTests and the full D3D9 probe pass. No additional
test target was added. Constant values and generated shader pixel output are
not validated by this compilation/binding fixture.

These routines are dependencies of `00b39110` vertex-source generation; the zero
emitter is also used by the pixel generator. System-value descriptor selection,
full helper code, descriptor wrappers and complete main generation remain work
before the reconstructed game material path can run.
