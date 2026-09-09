# Pixel shadow source helpers

`00b38230` emits GetShadow and `00b382b0` emits GetMapShadow. Both use ECX builder
with no stack arguments (RET), select a path from builder byte +AAh, and append
literal HLSL through the newline-appending helper. The typed APIs take that flag
explicitly and preserve all source text in `src/shader_shadow_literals.inc`.
Original disk and saved Ghidra body/literal comparisons are recorded in
`reports/shader_shadow_helpers.json`. Native string allocation ABI is not ported.

GetShadow chooses UV3/2/1/0 through ordered WorldPos z/y/x tests; UV3 also sets
shadowfade from saturate((WorldPos.w+200)/200). With +AAh nonzero, both helpers
project xy by w, saturate z, and use tex2Dproj. GetShadow multiplies this sample
by ShadowMultiplier and adds shadowfade; GetMapShadow returns the sample.

With +AAh zero, both helpers read four red-channel samples, compare saturated
depth using strict less-than, and bilinearly interpolate the comparison values.
GetShadow uses cShadowMapSizeData.xy/zw for fractional coordinates and texel
offsets. GetMapShadow hardcodes 2048 and 1/2048. These are native source choices,
not inferred improvements. Both paths and the default ShadowMultiplier=1.0 are
preserved. The actual device feature that sets +AAh remains unresolved.

The existing pixel shader fixture includes filtered GetShadow and projected
GetMapShadow; it supplies cShadowMapSizeData and compiles as ps_2_0. Shader
creation/binding, full D3D9 probe, Win32 Release build and both existing CTests
pass. These helpers are unused by the diagnostic main. Neither shadow pixel
output nor the other two branch combinations were exercised; byte-grounded
source preservation is separate from rendering validation. No new test target
was added.

Full pixel generation calls both helpers only when effect descriptor +15h is
nonzero and passes the same +AAh flag to each. The mixed fixture is deliberately
a compilation check of two implementations, not a native descriptor combination.
Both helpers can now be integrated into the remaining full pixel generator.
