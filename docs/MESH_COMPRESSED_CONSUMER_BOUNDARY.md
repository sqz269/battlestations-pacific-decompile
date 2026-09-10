# Mesh compressed-vertex metadata consumer

Addresses: 00b61d90, 00b61e10, 00b42350 (fragment 00b428c0..00b42a7b),
00b35820, 00b4bc00, 00b61e20, 00b49980, 00b49a80.

The mesh's `CompressedVertexFormatData` records provide **vertex-shader
scale and offset constants**. Each 20h-byte record contains four scale
floats followed by four offset floats. The native shader generator emits
`input * scale + offset`. The inspected constructor and upload path use
declaration stride and preserve the serialized vertex representation;
they do not consume these records as a CPU decompression instruction.

This connects the [mesh payload parser](MESH_VERTEX_INDEX_PAYLOADS.md) to
the previously audited [compiled constant binding](SHADER_COMPILED_CONSTANT_BINDING.md)
and [vertex shader generation](SHADER_INITIALIZATION.md). The next runtime
packet is `mesh_vertex_decode_constant_binding`.

## Direct native chain

1. Parser `00b93800` attaches the record allocation to the last appended
   vertex stream through `00b61d90`, which stores it at stream `+50`.
2. Accessor `00b61e10` returns `[ECX+50] + index*20h`; the index is one stack
   DWORD, EAX holds the result, and the function ends with `RET 4`. It has no
   null or bounds check. Proposed name: `BSP_LogicalVertexStream_GetDecodeRecord`.
3. Its single direct caller in both saved and live queries is callsite
   `00b429cf` inside `BSP_MaterialPass_BuildShaderConstants` (`00b42350`).
   This does not establish that no other inline consumer exists.
4. Fragment `00b428c0..00b42a7b` reads the draw section through entry `+4`,
   visits stream pointers beginning at section `+3C` with count `+4C`, and
   gets each declaration's element count via stream virtual `+24` and
   `00b47900`. Record indexing restarts at zero for each stream.
5. Eight native `FLD`/`FSTP` pairs copy each record's two float4 values into
   the shared VS register buffer at `0108ebf4`. A stream with null `+50`
   supplies scale `(1,1,1,1)` and offset `(+0,+0,+0,+0)` instead.
6. Existing generator `00b35820` emits the native literal at `00d5f400`:

```hlsl
IN.field = IN.field * cVtxElemScale[index].swizzle
                   + cVtxElemOffset[index].swizzle;
```

The generator gate is base-descriptor byte `+1F`; its field prefix is the
smaller unsigned value of descriptor `+20` and builder input count `+8`.
The established helper selects x/xy/xyz/xyzw for component counts 1..4.
The existing typed implementation is `src/shader_source.cpp`.

## Register and iteration constraints

The material fragment reads actual scale and offset register bytes from
`[pass+70]+20/+21`. These correspond to reflected `cVtxElemScale` and
`cVtxElemOffset`, not fixed declaration-order register numbers. It skips
the branch if scale is FFh; it has no separate absent-offset guard.

The initial remaining capacity is `offsetRegister - scaleRegister`. Both
destination cursors advance together across streams. Per-stream work is
bounded by the declaration element count and the difference between the
two current cursors. Remaining capacity and a separate descriptor limit
are decremented per element but checked for zero only at stream boundaries.
The descriptor limit comes from `[pass+14]->C4 +20`, or `+24` when
`[entry+10]+198 == 2`; the complete owner-field mapping remains deferred.
Replacing this with a different per-element clamp would change the observed
native algorithm. A host implementation can explicitly reject unsupported
range combinations without claiming that its rejection is native behavior.

The branch performs no register-buffer-wide clear. The existing material
execution audit establishes that caller `00b43410` uploads positive dynamic
constant tails after the builder. The existing D3D shader probe supplies
identity decode values directly at reflected registers; it does not connect
newly parsed mesh records to the material-pass fragment.

## Concrete next packet

Implement a bounded projection of `00b428c0..00b42a7b` with the accessor
contract from `00b61e10`. Feed it parsed per-stream records and the selected
draw section's stream order, preserve identity values for absent metadata,
and connect its output to reflected constant locations and the existing
material execution upload. Reuse shader emitter `00b35820` and reflection
`00b3aea0`. Keep declaration-defined D3D attribute interpretation explicit.

The smallest useful validation uses one nonidentity parsed record, checks
the reflected register values, then exercises an existing focused D3D
draw/readback. Parsing success, correct register values, and correct
rendered geometry are separate evidence claims.

## Evidence and boundaries

`reports/mesh_compressed_consumer_boundary_audit.json` contains the getter
annotation proposal, exact next packet, and 13 PE/live-matched ranges,
including the complete getter, 1BCh-byte material fragment, complete shader
emitter, native decode literal, constructor, lock and unlock. Every live
query verified project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Binary SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The neighboring pointer cleanup `00b61da0` has a stale `CALL_RETURN` at
`00b61daa` hiding its clear-after-free tail. Stream vtable `+30` points to
missing function `00b4aab0`, whose native body is only RET. Both findings
are deferred and do not affect the confirmed constant-consumer path.

This packet changes documentation and evidence only. Existing material-pass
and shader-emitter names remain correct. No Ghidra/shared-ledger changes,
C++ implementation, build, fixture, native ABI, or gameplay validation was
performed here. Native x87 load/store copies are not proof of exact
memcpy-equivalent treatment of every NaN payload.
