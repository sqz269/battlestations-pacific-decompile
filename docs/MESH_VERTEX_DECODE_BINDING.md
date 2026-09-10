# Mesh vertex decode constants

`src/mesh_decode_bindings.cpp` connects parsed
`MeshVertexStreamPayload.compressed_format_bytes` to the existing reflected
vertex constant buffer. The caller supplies the selected draw section's ordered
stream pointers and the `ShaderConstantBindings` produced by `00b3aea0`.
The existing `00b35820` shader emitter performs `input * scale + offset`;
this change never transforms vertex payload bytes on the CPU.

The implementation projects accessor `00b61e10` and material-pass fragment
`00b428c0..00b42a7b` of `00b42350`. The primary integrator owns its connection
to material execution and the existing D3D9 probe. This document alone does
not establish a successful draw, native object lifetime, or game validation.

## Native evidence and interfaces

The configured live Ghidra queries verify project `bsp`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit language and image base
`00400000`. The original target is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`;
the existing project is `C:/Users/sqz269/bsp.gpr`. The getter's complete 13 bytes
and the fragment's complete 444 bytes match the original PE. Seven captured
spans totaling 721 bytes also cover the owner and descriptor count mapping.
Addresses, hashes, file offsets and preserved annotations are in
`reports/mesh_vertex_decode_binding_audit.json`.

The getter receives the logical stream in ECX and an element-index DWORD on
the stack. It returns `[ECX+50] + index*20h` in EAX and ends with `RET4`.
It has no native null/bounds check. The new
`read_mesh_vertex_decode_record_00b61e10` returns a checked value containing
four little-endian scale floats followed by four offset floats. It rejects
absent or short metadata without altering its output. The material fragment
supplies identity values when stream `+50` is null; the checked getter itself
does not invent an absent record.

`pack_mesh_vertex_decode_constants_00b428c0` is a new C++ interface to a
fragment, not a standalone native function. Its parent receives ECX pass,
entry and override stack arguments and ends with `RET8`. It consumes:

- Ordered `const MeshVertexStreamPayload*` values corresponding to the selected
  section's stream pointers at `+3C`, count `+4C`. A selection may repeat or
  reorder streams; the API does not silently select every resource stream.
- Actual reflected semantic slots 24 and 25 for `cVtxElemScale` and
  `cVtxElemOffset`. These map to native metadata bytes `+20/+21` under pass
  `+70`; they are register indices, not declaration element numbers.
- The primary and, when needed, shadow descriptor limits described below.
- The existing VS word vector used by material constant packing and upload.

## Primary and shadow descriptor limits

The descriptor owner chain is now established by direct stores, not a guess
based on matching offsets:

1. In descriptor loading `00b45ee0`, `00b45f58` stores the newly constructed
   primary descriptor at effect `+C4`; `00b45f5e` reads its Shader table.
2. After program creation, `00b4632b..00b46338` passes that effect as the stack
   argument to `00b172b0`, with the returned pass in ECX. The complete
   10-byte helper stores `[ECX+14] = stack argument`, `RET4`. Proposed name:
   `BSP_MaterialPass_SetEffectOwner`; its previous name is `FUN_00b172b0`.
3. Shader-table reader `00b43b00` reads `CompressedElemCount` with default 999
   and writes descriptor `+20` at `00b43f61`.
4. The configured `ShadowShader` name is stored at primary descriptor
   `+100/+104`. `00b46537..00b46589` copies and resolves that name; the newly
   created separate descriptor is read at `00b465c3`.
5. `00b465c8` reads that shadow descriptor's `+20` and `00b465db` copies it to
   the primary descriptor's `+24`. This is separate from the earlier
   `shadow_passtrough.shfx` descriptor read at `00b46528`.
6. The owned material fragment dereferences pass `+14`, then effect `+C4`,
   and selects primary `+24` when `[entry+10]+198 == 2`; other selector values
   use primary `+20`.

`MeshDecodeDescriptorLimits.element_limit20` therefore accepts the evaluated
primary `ShaderLuaCode.options.compressed_element_count`. The optional
`element_limit24` accepts the evaluated configured ShadowShader's count.
Selector 2 requires that explicit shadow value. The host does not fall back
to the primary count when the shadow path has not been resolved. Both values
are DWORD bit patterns, including a cast from the Lua adapter's signed value.
These field reads establish the count source, not the complete native effect,
shadow-pass construction or retained ownership lifecycle.

## Exact iteration and selective writes

The scale register's `FF` value skips the entire branch before inspecting
streams or limits. The offset register has no independent `FF` guard. Native
reflection counts are not consulted by this fragment and are not new clamps
in the host helper.

Let `S` and `O` be the reflected scale and offset register cursors, and
`gap = O - S`. The native remaining-gap DWORD starts with the same bit pattern.
At the start of each stream, native execution stops only if the remaining-gap
DWORD or selected descriptor-limit DWORD is zero. For a visited stream:

- Its work count is the signed minimum of its declaration element count and
  `O - S`, followed by a signed-positive test. The declaration count's DWORD
  is interpreted as signed for this comparison.
- Record indexing restarts at zero. Each record writes four scale components
  and then four offset components. Missing metadata writes four `1.0f` scale
  values and four positive-zero offset values.
- Both destination cursors advance together. Their difference stays constant.
  The remaining-gap DWORD and selected limit decrement once per element with
  32-bit wrap, and are tested again only at the next stream boundary.

Consequently, replacing this with `min(elements, remainingGap, remainingLimit)`
would change native behavior. For example, with gap 3, limit 1 and a stream
of three elements, all three records are written and the limit becomes
`FFFFFFFE`. With gap 3 and successive streams of two and two elements, four
records are written; the remaining gap becomes `FFFFFFFF`. The fourth scale
write can overlap the first offset register. The implementation replays each
scale/offset pair in native order, preserving that overwrite.

The host checks every visited source record and destination float4 before
changing the VS vector. It stages values so a late bounds error leaves the
buffer and statistics untouched. Null visited streams and a missing selector-2
shadow limit are also explicit host errors. Unvisited streams are not checked.
The buffer is never resized or cleared, and bytes outside the actual writes
remain untouched. These are host bounds safeguards, not recovered native
failure behavior. The observed x87 record copies provide ordinary finite-value
agreement; exceptional NaN payload conversion and floating-point exception
state are not claimed to match the native load/store sequence.

## Integration and validation boundary

The source composes directly with `MeshResourcePayload.vertex_streams` through
its `MeshVertexStreamPayload` values and with `ShaderConstantBindings` from
real compiled reflection. The caller selects the draw section, resolves primary
and shadow Shader scripts as needed, invokes this helper while constructing
the existing material VS vector, then uses the existing material upload path.
The shared `material_constants.cpp` parameter packer remains a separate earlier
fragment of the same native owner.

The focused integration should read a nonidentity parsed record, compare the
affected reflected register values after upload, and use the existing D3D9
draw/readback to check the generated shader result. A successful C++ build or
native PE-byte comparison is not that rendered proof. Native memory layout,
allocator ownership, binary compatibility and game runtime remain outside this
bounded value projection. Current build and probe results belong in the audit's
validation fields when the primary integration has actually run them.
