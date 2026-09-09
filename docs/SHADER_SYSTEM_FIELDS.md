# Shader builder field preparation

Three typed operations now replace the probe's reduced system-field projections:

- `00b35930` appends copied base then effect vertex-input records from descriptor
  +D0h/+D4h into builder+4h. Names, scalar types, counts, semantic kinds and indices
  are preserved; component mask +10h is reset to zero. It neither clears nor
  deduplicates, and does not filter by semantic. Typed output must be distinct
  from its source vectors; native aliasing/mutation during traversal is unsupported.
- `00b35be0` appends sixteen fixed vertex system fields to builder+10h. They are
  ScreenSpacePos, ObjectSpacePos, WorldSpacePos, Normal, Tangent, BiNormal,
  CustomValue0..3, CustomVec2_0..3 and CustomVec4_0..1. Position values are float4;
  directions are float3; custom widths are1/2/4. Position semantic is0, Normal3,
  Tangent5, BiNormal4 and custom fields2. All indices and masks are zero.
- `00b372d0` appends sixteen pixel system fields to builder+34h: DiffuseColor,
  SpecularColor, EmissiveColor, SpecularPower, Normal, Depth, then the same ten
  custom values. Color widths4, power/depth1, normal3; all semantic kinds2,
  indices and masks zero. No descriptor condition controls these fixed entries.

All routines use ECX builder and RET. They allocate native records and append
pointers, whereas the port appends owning values. Allocation failure/null-entry
behavior, native string ownership and exception ABI are not reproduced. Repeated
calls append another set, consistent with the native routines.

Full body bytes agree between original disk and saved Ghidra. Each fixed-field
constructor argument set was checked against assembly pushes, including EBP=1
for scalar widths. Tables and hashes are in `reports/shader_system_fields.json`;
the generated include files retain their order. The underlying field constructor
`00b34e20` confirms that its final argument initializes component mask+10h.

The existing debug/dummy fixture uses both full system lists and the recovered
input-copy path. VS3/PS3 compile and render the same centerFF407FBF/outsideFF000000
pixels. Build, two existing CTests and full D3D9 probe pass; no test targets were
added. This does not validate all unused fields in a game material. Descriptor
parsing, interpolator selection/filtering, runtime values and full material/game
execution remain unfinished.
