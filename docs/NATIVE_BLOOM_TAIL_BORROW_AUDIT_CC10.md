# Bloom tail borrow: actual consumer and installed shader audit

The blur parameter **does cause a four-DWORD CPU read when its installed normal
pass is selected**. The fourth DWORD starts at bloom+43C, beyond the evidenced
43Ch allocation request. The installed shader consumes only xy, but this does
not narrow the preceding CPU copy. Extra readable allocator extent remains
unproved. No allocation, parameter count, padding value or implementation is
changed by this audit.

This closes the *consumer* part of the earlier boundary in
`native_bloom_initializer.hpp` and `NATIVE_BLOOM_INITIALIZER_R79.md`; it does not
close the backing-storage or broader runtime boundary.

## Registration and actual CPU consumer

B107F0 requests 43Ch for both independent bloom owners at B10FB3 and B11DE5.
Full B54F90 takes bloom+430 into EBX at B550E2. B552CB pushes vector count1,
B552CD pushes that borrowed pointer, and B552E2 calls B18AC0 on the current
blur child's material. B18AC0 passes `4 * vector_count` and matrix0 to B17E10
at B18AD8; its complete body is [B18AC0,B18AE0), ending RET0C at B18ADD/3 bytes.

Existing actual B17E10/B44D60 registration stores source/count/matrix and the
matched stage register for each selector. It does not read source words or
clamp their count to reflected columns, bytes or register count. The source
providers are in `native_material_parameters.cpp`, not the logical binding
projection. B44D60 compares each ordinary reflected name and uses the matching
register; it has no shape-dependent count adjustment.

The actual full consumer is `build_native_material_constants_00b42350` in
`native_material_constant_build.cpp`. Its original [B423C5,B42694) parameter
region gets the material table and selector `entry+10 -> +198`, then reads VS
register `parameter+14+4*selector`. A nonnegative register with matrix byte0
takes B425B1: load `parameter+C`, load source+8, double the count twice, and
call BF7680/memmove at B425C7. The corresponding PS path uses register
`parameter+4C+4*selector` and calls BF7680 at B42665. Each selected nonmatrix
path copies exactly `word_count*4` bytes; neither consults reflected shape.

For this descriptor count4, the VS path copies [bloom+430,bloom+440) into
VS bank register77. The first pair is the calculated half-texel offset;
+438 contains the original parameter bits. The fourth DWORD at +43C lies
outside the request. Negative register branches skip their stage, but the
installed normal VS matches, so that skip does not resolve this case. This is
static read-span proof, not execution of the out-of-request read.

## Genuine installed shader metadata

The installed `shaderfx/shaders.bin` is 2,250,700 bytes, 1,628 records, SHA256
`dcc65686e26e0b7b4f4179b45b153aaa353696b0b1b08c32251fbc7c97c0ecf8`.
The bounded extraction reaches its exact end and matches the four relevant
row hashes already recorded by R100's actual application reflection fixture.

| Installed rows | Bytecode | SHA256 | cTextureOffset |
|---|---:|---|---|
| 1032/1034: blur5x50F3.vso / blur5x50T3.vso | 228 bytes each | 3988b9fb1e2f666ab54f538b9d8c9997f5c9d401ab5d94c46891e09752d1faf7 | FLOAT/vector, register77/count1, rows1/columns2/elements1, Bytes8 |
| 1033/1035: blur5x50F3.pso / blur5x50T3.pso | 1156 bytes each | 0e23592af737429677ca1e8b727750cdaa54013fb2a5ade0032b38e7e8a36f3a | absent |

The installed blur5x5.shfx declares `float2 cTextureOffset` and adds it to UV.
Its sole combiner is RM_NORMAL; installed dx9_lua.inc defines RM_NORMAL=0.
D3DX disassembly of both installed VS binaries contains
`add o1.xy, c77, v1`. Thus the GPU uses xy, while CPU packing still reads xyzw.

Both CTAB decoding and the installed **64-bit** System32 d3dx9_40 DLL's
GetShaderConstantTable/GetConstantDesc confirm the table above. The latter is
a read-only metadata query, not execution of the game's 32-bit reflection or
draw path. Existing R100 separately exercised the actual source reflection
provider over all 1,628 installed rows. The recovered 52-entry B5BF70 system
constant registry has no cTextureOffset entry: B3AEA0 appends it as ordinary
material metadata, which B44D60 can match. The native registration and CPU
consumer proofs, not the logical MaterialParameterBindings projection, support
the four-DWORD conclusion.

## Allocator boundary remains open

Original BF681B forwards the requested size to BF9F1A malloc. In malloc,
heap mode1 passes `max(size,1)` directly to HeapAlloc; the observed mode1 path
passes 43Ch unchanged. Only the alternate fallback rounds `(size+15)&~15`,
which would be 440h. The original C11964 heap selector returns1 for NT platform2
with major version greater than4. C119BF stores that selection at 109ED7C;
the only observed write xref is C119EA. Alternate-mode rounding therefore
cannot establish unconditional 440h backing for normal NT execution.

The source producer uses `singleton_lifetime_allocate` with native_bytes and
host_bytes both43Ch. That provider calls `std::malloc(host_bytes)` and never
queries usable size. The selected build SDK's 10.0.26100.0 UCRT malloc_base.cpp
likewise passes `max(size,1)` to HeapAlloc. Its msize.cpp queries HeapSize; no
such query/admission exists at the bloom allocation. These are source facts,
not proof of the installed runtime DLL's exact usable size. Heap alignment,
adjacent accessible pages or an unobserved allocator size class do not prove
this object's usable tail. No read of +43C was attempted in this audit.

## Evidence and next bounded work

The report indexes exact PE/live byte blocks, instruction endpoints, direct
call rows and SHA256s for ignored shader assets, DLL/SDK sources, listings and
metadata queries under `local/bloom_tail_borrow_*`. Ghidra remained read-only.
No tracked C++, ledger, shader, allocation or test changed; a C++ rebuild is
not needed for this evidence-only packet.

One independent next step is the missing retained application runtime-texture/
surface/holder context group. DestructionGraph already supplies its actual
texture/surface domains, renderer publication and pools; DeviceGraph supplies
recreation, and CameraGraph supplies the actual decrement cell. The real
increment import still needs a retained application binding for the getter.
There are no application-owned runtime/holder context instances yet. The
required cumulative allocation cells108D4BC/108D4C0 also lack process-owner
accessors: PE inspection places them in the loader-zero tail of .data, and
their bounded live xrefs are the existing B2A070 and B2A7C0/B2A9A0 accounting
reads/writes. A separately reviewed packet can retain those two real cells and
contexts, then bind the optional direct-terminal domain; it need not call the
whole initializer or draw bloom.

Alternatively, an exact allocator audit can compare the native mode1 heap's
reported size and the current Win32 source CRT's `_msize` for unchanged43Ch
requests, without reading padding. Only a demonstrated usable extent for the
actual allocation could admit the extra DWORD; a contrary result stays a
boundary. Neither investigation should prevent unrelated normal successful
provider work. Full post-effect composition still needs the retained actual
material-effect cache/compiler context and operation blocks. Distortion's
unwritten cleanup preimage must not be invented, and no whole-initializer
runtime attempt is authorized by this report.
