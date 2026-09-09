# Compiled shader constant binding

`00b3aea0` obtains actual constant registers from the compiled shader through
`D3DXGetShaderConstantTable`. The nominal declaration-order slots87/95 for
`cVtxElemScale`/`cVtxElemOffset` are not runtime binding locations. The declaration
generator omits explicit register annotations outside the77-register prefix.
This investigation verified project `bsp`, program `/battlestationspacific.exe`
before each live export/read batch. Raw evidence is under ignored
`exports/bsp/compiled_constant_analysis/`. No C++, Ghidra metadata or saved
analysis was modified by this investigation.

## Reflection contract

Original ABI of `00b3aea0`: first stack argument is shader DWORD bytecode,
second stack argument is the destination metadata object; `RET8`. Incoming ECX
is not consumed as an object pointer. The pseudocode's apparent uninitialized
return-address/object variable is a stack-analysis artifact; assembly reads
the second argument into EBP at00b3aee5. Calls00b3bb65..69 push the metadata
pointer, then bytecode pointer, confirming argument order.

The routine calls D3DXGetShaderConstantTable at00b3aecf, then table virtual
GetDesc(+14h), GetConstant(+20h; parent handle null, sequential constant index),
and GetConstantDesc(+18h). It supplies room for256 descriptors, but consumes
the first result per top-level constant. HRESULT/error/null handling is absent
in this body. The table is released at00b3b11f.

For descriptors whose Type field equals3 (FLOAT), it looks up the returned
name in the system registry through00b5b960. A found record's source semantic
id comes from00b5b830 (record+1Ch). Destination metadata has:

| Offset | Meaning |
|---|---|
| +8h .. +3Dh | 54 one-byte system register indices; FFh means absent |
| +3Eh .. +73h | Corresponding one-byte register counts |
| +74h | One-byte end register for the largest-start accepted float constant |
| +78h/+7Ch/+80h | Non-system constant record vector |
| +84h | Sampler register bit mask |

The constructor fragments00b3b57c..5b5 and00b3b5d6..60f initialize only the
54 register bytes toFFh, not the count bytes. Count bytes are meaningful only
when their register byte is present. A previously populated system semantic
is skipped rather than overwritten. Unknown float names go through00b3a750
into material-parameter metadata, carrying reflected register/count and
shape. The full vector/string ownership path is not reconstructed here.

Assembly00b3afe6..b001 truncates RegisterIndex and RegisterCount to bytes.
Registry semantic24 (`cVtxElemScale`) therefore occupies metadata+20h, with
count+56h; semantic25 (`cVtxElemOffset`) occupies+21h, count+57h. These are
independent reflected locations. Samplers are recognized independently by
RegisterSet==3 at00b3b0cc and OR `1 << RegisterIndex` into+84h (native x86
shift masks the count to five bits). Float processing uses Type==3, not a
RegisterSet==FLOAT4 test.

End-register tracking compares start indices, not all end positions. On a
strictly larger start it records that descriptor's count; equal starts keep
the earlier count. Duplicate already-populated system semantics bypass that
update. Final end is truncated to a byte. A safe host adapter may reject
overflow and malformed descriptors, but these checks would be new behavior.

`00b3b3c0` invokes reflection on both freshly compiled and cache-loaded vertex
and pixel bytecode (call sites00b3b854,00b3bb69,00b3bdbf,00b3be7f). Vertex
compiler00b60f60 calls D3DXCompileShader with flags0x1200. The present host
D3DCompile dependency is not proven to produce the original compiler's
register allocation or bytecode.

## Decode constant values and upload

The material builder's fragment00b428c0..00b42a7b reads the two register bytes
from `[pass+70h]+20h/+21h`. If scale isFFh it skips the branch. It does not
independently guard an absent offset. It sets an initial remaining capacity
to `offsetRegister - scaleRegister`, relying on compiler layout rather than
the two reflected count bytes.

The input entry+4h supplies a section's stream pointer array beginning+3Ch,
count+4Ch. Each stream's virtual+24h returns layout metadata;00b47900 reads
its+10h element count. Stream+50h is the optional per-element decode record
pointer. Helper00b61e10 has ECX=stream and one stack element index, RET4;
it returns `[stream+50h] + index*20h` with no checks.

For a stream with decode records, each32-byte record provides four scale
floats followed by four offset floats. Native FLD/FSTP pairs copy them into
the shared VS float-register buffer at0108ebf4. For a stream without records,
00b42a17..00b42a38 writes scale `(1,1,1,1)` and offset `(+0,+0,+0,+0)` using
MOVSS. The scale literal00d7a24c was live-read as bits3f800000. This proves
identity decode is the native fallback for an uncompressed stream even when
the descriptor enables `CompressedVertices`.

The loop advances both destination ranges together and decrements remaining
capacity and a separate descriptor limit after each element. The latter is
loaded from `[pass+14h]->c4h +20h`, or+24h for entry mode2. Those owner fields
are not fully recovered here. The limit is tested for zero at stream
boundaries; do not silently translate it into an arbitrary per-element clamp
and call that exact native behavior. No buffer-wide clear occurs.

Caller00b43410 runs the material builder and uploads only positive tails
`[77, metadata.endRegister)` through the existing VS/PS constant setters.
Its full original ABI includes several draw arguments and is not recovered
by this focused investigation. The shared system prefix was populated
separately. This makes native runtime behavior depend on compatible compiler
allocation of dynamically supplied constants outside that prefix.

## Smallest useful integration

For the existing debug triangle, enable the evaluated descriptor's decode
flag/limit, compile the final filtered VS, and obtain its actual constant
table. Find scale and offset independently by name, validate their float4
register ranges, and upload identity scale/zero offset values for its two
uncompressed input elements. Query the final bytecode, because interpolator
filtering can change compiler optimization and allocation. A direct upload
to the reflected ranges after the diagnostic prefix is a valid explicit
host adapter; it is not a reconstruction of the native fixed-tail uploader.
Ensure these ranges do not overlap other active constants, especially the
view-projection matrix. Do not assume87/95 or infer count from location gap
for a different compiler.

This can reuse the existing draw/readback probe without a new test suite.
Report actual reflected locations/counts and unchanged expected pixels as
host fixture evidence. General compressed mesh decoding still requires the
native stream/layout ownership and actual per-element decode records.
Register reflection alone does not establish game validation, native bytecode
parity or drop-in ABI compatibility.

## Byte verification

These saved-image ranges matched the original executable byte-for-byte:

| Address | Bytes | SHA-256 |
|---|---:|---|
| 00b3aea0 | 667 | 92686a091b1848526ff214ef27bd9c565b6de1144922de9a9779f817bc70a7bb |
| 00b61e10 | 13 | 5fe3cbfe6f2e7862e70bd7a669c43c07281175d6aa8cb4556254679bcc348c4a |
| 00b47900 | 4 | 74032ad0e931f3c4730df99da43479f9f5315cc898c7c8614278631cb2c16a62 |
| 00b60f60 | 604 | e63a9273b3513f86036092e16674f849dde8d9e26af6b35e94e8323d3542cb6f |
| 00b43410 | 600 | 0dca81aa71a05c9c00cf5656a12f976cbefe8f0415eb9bc73e290cce0673399c |
| 00b428c0 fragment | 444 | 166aecc12b9e5fddc233eb109cff881737f46764ddc07f21fff3b725efa547da |
