# Mesh vertex decode constants

`src/mesh_decode_bindings.cpp` connects the actual draw section's
`LogicalVertexStream.compressed_format_bytes_50` owners to the existing reflected
vertex constant buffer. The caller resolves the current ordered streams after
preceding material-builder callbacks and supplies the `ShaderConstantBindings`
produced by `00b3aea0`. The original parsed `MeshVertexStreamPayload` overload
remains available and uses the same immediate native-copy implementation.
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
`reports/mesh_vertex_decode_binding_audit.json`. The live-owner correction,
fresh native bytes, constructor state and exact x87 comparison are recorded in
`reports/material_decode_live_owners_review.json`.

The getter receives the logical stream in ECX and an element-index DWORD on
the stack. It returns `[ECX+50] + index*20h` in EAX and ends with `RET4`.
It has no native null/bounds check. The new
`read_mesh_vertex_decode_record_00b61e10` returns a checked value containing
four little-endian scale words followed by four offset words. It preserves
source bits without FP conversion; the constant packer performs the native x87
conversions. It rejects
absent or short metadata without altering its output. The material fragment
supplies identity values when stream `+50` is null; the checked getter itself
does not invent an absent record.

`pack_mesh_vertex_decode_constants_00b428c0` is a new C++ interface to a
fragment, not a standalone native function. Its parent receives ECX pass,
entry and override stack arguments and ends with `RET8`. It consumes:

- Ordered `const LogicalVertexStream*` values corresponding to the selected
  section's stream pointers at `+3C`, count `+4C`. A selection may repeat or
  reorder streams; the API does not silently select every resource stream.
  The preserved parsed overload instead accepts `const MeshVertexStreamPayload*`.
- Actual reflected semantic slots 24 and 25 for `cVtxElemScale` and
  `cVtxElemOffset`. These map to native metadata bytes `+20/+21` under pass
  `+70`; they are register indices, not declaration element numbers.
- The primary and, when needed, shadow descriptor limits described below.
- The existing VS word vector used by material constant packing and upload.

## Actual logical-stream storage

The actual base constructor is `00B61E20`; `00B61EA0` lies inside that function.
It clears EBX at `00B61E55`, then stores EBX to stream `+50` at `00B61E6B`.
Derived constructor `00B4BC00` calls this base at `00B4BC25`. The host
`std::optional<std::vector<uint8_t>> compressed_format_bytes_50` defaults to
disengaged, preserving native null separately from a present empty allocation.

Mesh handler `00B93800` obtains the last stream's actual declaration count,
allocates/reads `count*20h` bytes, then attaches that allocation with `00B61D90`.
The setter stores its stack pointer argument to stream `+50`; it does not copy
or synthesize records. Native destructor `00B62010` loads `+50` at `00B62060`
and frees a nonnull pointer at `00B62068`. Host optional/vector storage owns the
corresponding byte allocation, without claiming the native allocator or full
stream-destruction ABI.

The existing GPU mesh creation helper receives the complete parsed payload,
including that later metadata attachment. It copies the exact parsed bytes into
the actual logical owner only when `has_compressed_data` is true and rejects a
present record allocation whose size differs from `element_count*20h`. This
combines existing parsed construction stages; compressed bytes are not an
invented argument to the original native stream constructor.

Generated geometry `00B4C8D0` obtains its instance declaration at `00B4C975` and
calls the renderer factory with count0/flags1000 at `00B4C987`. It retains the
original mesh stream as stream0 and the fresh instance stream as stream1; it
does not attach compressed metadata to stream1. The existing host generated
stream uses `new LogicalVertexStream`, so its new optional field retains the
verified constructor-null value without an extra generated-data workaround.

Concrete vtable `00D61D6C` slot+24 (`00D61D90`) points to `00B48CE0`, exactly
`MOV EAX,[ECX+68]; RET`. `00B47900` reads declaration+10 and returns. These are
plain retained-owner reads, so this supported path introduces no callback
interface. The actual-owner overload reads `stream.declaration->elements()`;
it does not use a copied parsed layout or infer count from compressed byte size.
Unknown stream subclasses/overridden getters are outside this concrete owner
projection.

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
- Record indexing restarts at zero. Metadata presence is captured once per
  stream before the declaration getter, as at `00B4294E/5B`; actual record
  backing is reread per element, matching `00B61E10`. Each record immediately
  writes scale0..3 and then offset0..3 through eight sequential `FLD m32` /
  `FSTP m32` pairs (`00B429D4..00B42A12`). There is no whole-record or whole-call
  value staging. Missing metadata writes raw `3F800000` scale words (verified
  global `00D7A24C`) followed by raw positive-zero offset words.
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

The host checks each visited source record and its two destination float4s
before writing that record. Later errors retain completed records and their
overwrites. Statistics reset at call entry, then track visited streams,
completed records and remaining DWORD counters even on failure. Null visited
streams, missing actual declarations and a missing selector-2 shadow limit are
explicit host errors. Unvisited streams are not checked. These safeguards are
not native error paths. Neither overload resizes or clears the output.

The earlier implementation staged records and used C++ float assignments.
A focused Win32 check found that signaling bits `7F812345` survived unchanged
with x87 status0, whereas native `FLD`/`FSTP` produced `7FC12345` and invalid
status1. The current writer executes those same per-word x87 instructions under
the caller's control word, preserving their NaN conversion and exception-flag
effects instead of relying on compiler-selected float moves. It does not
install an exception handler or alter exception masks. Fixtures use masked
exceptions; complete unmasked-exception handling is not established.

## Integration and validation boundary

The source composes directly with the actual selected draw-section stream
owners and `ShaderConstantBindings` from real compiled reflection. The caller
resolves those owners after prior dynamic/skin callbacks, selects primary and
shadow descriptor limits explicitly, invokes this helper while constructing
the existing material VS vector, then uses the existing material upload path.
There are no callbacks within the supported concrete getter loop; stream/list
storage must remain valid during it. The parsed overload remains for existing
payload consumers and uses the same write schedule.
The shared `material_constants.cpp` parameter packer remains a separate earlier
fragment of the same native owner.

The corrected source passed `scripts/build.ps1` and both existing math checks.
The existing ignored local decode fixture was extended to execute the complete
444-byte native fragment and three complete getters, with only address/call
relocations and an exit return outside the assigned fragment. Both overloads
matched the full VS buffer and x87 exception flags for two compressed mesh
records followed by three constructor-null instance identities: five total
records, overlapping destination registers and both DWORD counters underflowing.
A second native case changed the actual declaration and compressed backing
between calls and matched again. A late short-metadata check retained the first
quieted-NaN record and partial statistics. No tracked test suite was added.

These strict compilation and isolated native-fixture checks do not establish
complete material-builder execution, native object/allocator ABI, device upload,
rendered parity or game behavior. The GPU metadata transfer is source-verified;
the correction's fixture does not itself create a live device or generated
geometry object.
