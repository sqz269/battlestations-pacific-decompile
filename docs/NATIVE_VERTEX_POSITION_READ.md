# Actual logical vertex position reader

Addresses reconstructed: `004768D0` and `00475F80`. Descriptive names are
hypotheses. Ghidra stayed read-only; repository wrappers verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before every batch.

| Routine | Original ABI and exact bounds | Coverage |
| --- | --- | --- |
| `read_native_vertex_position_004768d0` | ECX actual logical vertex stream; out float3 and vertex index on stack; native EAX returns output; body `004768D0..00476B40`, final `RET8` at `00476B3E`, length3 | complete for constructor-null decode metadata and every initialized native type case; partial projection for undefined types which natively use uninitialized locals and now return false |
| `unpack_native_position_9bit_00475f80` | ECX unused; packed DWORD and output pointer on stack; body `00475F80..00475FE3`, final `RET8` at `00475FE1`, length3 | complete valid-memory computation and output stores; no native ABI replacement |

This reader consumes the same raw logical stream and current mapped storage
already used by the actual Text builders. There is no new stream owner,
physical mapping, vertex array, parsed-payload copy or material state. The
three-float scratch matches the native local frame. Caller ownership keeps
stream, mapping, decode metadata and output extents valid for the whole call.
No automatic lock/unlock, retain/release or capacity guard is inserted.

Producer `00B61E20` writes stride `+0C` from declaration `+CC` at `00B61EA3`.
Its position usage0 lookup produces byte offset `+10` at `00B61EC3`, flat
declaration-element index `+18` at `00B61ECF`, and declaration type `+14` at
`00B61ED7`. The callees establish these meanings: `00B47C20` reads the selected
declaration element's type word; `00B47CE0` scans the flat 14h element array for
usage/occurrence and returns its index. Base construction sets `+50=0` at
`00B61E6B` from EBX zeroed at `00B61E55`.

Compressed metadata has a separate real producer. `00B93800` selects the last
mesh stream, gets its actual declaration, allocates declaration count times20h
bytes, reads those raw bytes at `00B9384F`, and attaches the allocation through
`00B61D90` at `00B93869`. That leaf only stores the pointer at `+50`; it does
not copy/decode it or release any prior pointer. Existing
`MeshVertexDecodeRecord` describes each record as scale float4 then offset
float4. This reader accesses that live raw allocation directly using
`+50 + (+18 << 5)`. The semantic `LogicalVertexStream` optional byte vector is
a different existing projection and is not substituted for the raw pointer.
Constructing/adopting actual compressed stream ownership remains with that
producer; the getter borrows its retained bytes.

With null `+50`, type is not read: stride times index plus offset plus mapped
base addresses three floats. Each output lane uses an x87 load/store pair
before reading the next lane, retaining native overlap behavior.

With nonnull `+50`, byte table `00476B68` and jump table `00476B44` select:

| Type | Native case | Decode before scale/bias |
| --- | --- | --- |
| 2,3 | `004768FD` | First three float bit patterns copied with MOVSS semantics |
| 5 | `00476930` | First three unsigned bytes converted with CVTSI2SS |
| 7 | `004769B4` | Signed low16, high16, next low16 from two DWORD reads; CVTSI2SS |
| 8 | `0047695B` | Three unsigned bytes, x87 FILD/divide by double255, float spills |
| 10 | `004769F6` | Three signed16 values, FILD/float spills, divide by double32767 |
| 12 | `00476A5F` | Three unsigned16 values, FILD/float spills, divide by double65535 |
| 13 | `00476991` | One packed word through `00475F80`: masks1FFh at shifts0/10/20 |
| 16 | `00476AAB` | Actual D3DXFloat16To32Array(output, current input, 3) |

Type13 deliberately computes mapped base plus FOUR times `(stride*index +
position_offset)`. Other cases add the relative address without that extra
factor. The second caller of `00475F80`, at `0070FEC0` inside `0070FDB0`, uses
the same factor with stream attribute offset `+1C`. This observed arithmetic
is preserved; neither a normal ten-bit decoder nor an ordinary byte address
is substituted. Masked values are always0..511, so the native unsigned-FILD
correction branches are unreachable. The three x87 output stores remain.

Signed normalized values are not clamped: -32768 divided by32767 may be below
-1. No finite guards, SSE replacement for x87 division, FMA or rounding-mode
reset is added. Three decoded values spill to native float locals first.
After decoding, the reader reloads CURRENT `+18` and `+50`, including after
the external half conversion. For each axis it loads record scale, multiplies
the corresponding local, adds record offset and spills back to the local.
All three transforms finish before output is published by three x87 pairs.
Thus output overlapping source or decode metadata has the native staging and
store order. Invalid memory and unmasked floating exception delivery are not
wrapped in a substitute error path.

All other types (including out-of-range values and in-range4/6/9/11/14/15)
jump directly to `00476AC6` without initializing the native scratch. Their
result is not a defined format conversion. The new interface returns false
before output writes and supplies no default values. With valid initialized
cases it returns true; native EAX's output-pointer return is represented by
the explicit output reference in the new C++ API.

The half case uses a concrete borrowed import binding. Read-only PE inspection
of the original executable verifies `d3dx9_40.dll!D3DXFloat16To32Array` at
IAT `00CE23E4`; saved game thunk `00A4C2AE..00A4C2B3` is exactly a jump through
that IAT. `00476AB6/ABF/AC0` push count3, input and local output; the imported
WINAPI signature has three arguments and 0Ch callee cleanup. The existing
fetched DirectX SDK header declares `(FLOAT*, const D3DXFLOAT16*, UINT)`;
the input word is the same two-byte half representation. Installed 32-bit
`C:/Windows/SysWOW64/d3dx9_40.dll` exports this function at RVA1FDE04 and uses
its own runtime dispatch. The inspected scalar body ends with `RET0C`; no
assumption selects that scalar path over the DLL's current dispatch.

`NativeD3dx9Float16Import` resolves only that exact export from the supplied
actual module, following the existing `NativeD3dx9SurfaceSaveImport` pattern.
It performs no DLL load, alternate-version search, callable substitution or
handwritten half conversion. Missing module/export fails during binding.
The caller must keep the module loaded for the binding and every active read.
The library thunk retains its existing library name and has no new ledger
reconstruction record or Ghidra mutation.

All18 direct getter sites in nine functions were inspected in the preceding
wrapped packet, and the complete xref set was rechecked unchanged for this
packet. Both packed-leaf call setups were inspected. The machine-readable
report includes every site with containing function and native target; it
also records producer calls and exact body/return boundaries. Full-function
register filters establish ESI as the saved stream and distinguish the input
index slot from its later reuse as integer conversion scratch.

Validation: strict MSVC Win32 `/std:c++17 /W4 /WX /O2 /MD /fp:strict` compile
passed. Exact call verification is recorded in the report. No new tests,
native differential or game-render validation is claimed. The committed
wrapped files remain unchanged; integration must supply the borrowed import
and call this getter to close their decoded-position dependency. The existing
unsupported-format result remains explicit. Combined build belongs to the
primary integrator.

## AD validation and shared attribute readers

`docs/NATIVE_VERTEX_ATTRIBUTE_READ_AD.md` adds normal, UV and colour readers in the same module, sharing the existing byte/short, nine-bit and actual half-import implementations. The existing position entry and its coverage boundary remain unchanged. The AD original-byte fixture independently verifies 72 position comparisons across every initialized decoded format and the recorded x87 precision/rounding modes; it does not add a game-render or native ABI claim.
