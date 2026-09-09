# Independent perspective builder verification

Read-only inspection on 2026-09-09, before implementation review. Verified
`bsp`, `/battlestationspacific.exe`, x86 LE32, base00400000 through exporter
Client.verify. Fresh live disassembly and bytes were compared against the named
installed executable via PE RVA-to-file mapping. No code, metadata, names or
Ghidra state were changed in this investigation.

## ABI and complete matrix result

`00b642f0` takes destination float16 in ECX and four stack float32 arguments
`verticalFov, aspect, nearZ, farZ`, returns destination in EAX, and ends RET10h.
It preserves ESI and leaves no additional x87 stack result. There is one direct
call,00412e20, and two absolute global reads: double0.5 at00d7a280 and float+1
at00d7a24c.

`00412e20` takes one stack float32 angle, returns an x87 ST0 value which has
already been rounded to float32, and ends RET4. PUSH ECX supplies temporary
storage and POP ECX discards it: ECX is not preserved as a meaningful output.
It has no global references or calls and uses FSINCOS followed by FDIVP.

The matrix has nonzero assignments at flat indices0,5,10,11,14. Other eleven
slots are explicitly written as positive zero with XORPS/MOVSS. For ordinary
finite valid perspective inputs, source-level algebra is:

```
h = float32(x87(fov * double(0.5)))
t = float32(x87(sin(h) / cos(h)))  // FSINCOS, not generic libm tan
s = float32(x87(1 / t))
q = float32(x87(far / (far - near)))
m[0]  = float32(x87(s / aspect))
m[5]  = s
m[10] = q
m[11] = +1.0f
m[14] = float32(x87((-near) * q))
```

Here `float32` means the native FSTP store under the ambient x87 rounding
control; it does not force round-to-nearest. `x87` means the original instruction
sequence under the ambient precision/exception control, not infinite-precision
real arithmetic. Parent implementation should preserve these distinctions.

## Exact boundaries and instruction order

1.00b642f0 loads fov;00b642f5 multiplies by double0.5.00b642fe spills tofloat32
in the caller's argument slot, then00b64302/306 reloads/stores that value as the
wrapper argument. This is a deliberate float32 half-angle boundary.
2.00412e25 executes FSINCOS, leaving cos atST0 and sin atST1.00412e27 divides
sin by cos and pops.00412e29 spills the ratio tofloat32, and00412e2c reloads it
as the return. A compiler's double or float tan is not bitwise equivalent proof.
3.00b6430e/313 computes reciprocal. After writing zero slots,00b6434e stores s
tofloat32.00b64364 copies those exact stored bits to m[5].
4.00b64352 loads far;00b64356 loads near and00b6435a duplicates near.
00b64362 computes far-near;00b64369 divides far by that denominator. **There is
no float32 store of far-near.**00b6436b exchanges stack entries, and00b6436d
stores q tofloat32 (overwriting the far argument). Near remains on x87 stack.
5.00b64371 reloads s,00b64375 divides by float aspect, and00b64391 stores m[0].
While that value is on stack,00b64379/00b6437f copies stored q bits to m[10], and
00b64384/00b6438c copies constant+1 to m[11].
6.00b64393 FCHS negates the retained near value.00b64395 multiplies it by the
already float32-rounded q.00b64399 stores m[14]. Do not compute `-near*far /
(far-near)` without the q store, and do not replace FCHS with subtraction from+0
while claiming signed-zero equivalence.

Neither function reads or changes the x87 control word. Hardware FSINCOS range
behavior, exceptions, singular inputs and nonfinite propagation are not made
safe by this code. A bounded valid-input API may restrict them explicitly, but
should not call that restriction recovered native validation.

## Signed zero and constants

Zero-filled matrix slots are +0 (bits00000000). m[14] can be -0: for near=+0
and ordinary positive q, FCHS produces -0 and multiplication preserves its sign.
Near=-0 flips to+0 before multiplication. Other signs depend on arithmetic and
ambient modes and should be checked with bitwise native comparisons.

The nearby constant00d7a208 is indeed -0 (bits80000000), but **neither of these
two routines reads it**. It belongs to other inverse/transform helpers inspected
in the larger camera analysis. Do not introduce that global into this builder.
The actual builder constant bytes are double0.5=`000000000000e03f` and
float+1=`0000803f`.

## Byte provenance

All listed ranges match the current installed disk bytes and saved Ghidra bytes.
Lengths include the complete RET instruction and exclude adjacent padding/code.

| Start | Bytes | SHA256 |
|---|---:|---|
|00b642f0|176|434aa71d29dc916a29e4b8723c612f341b54ab7a7c0c922ab0afee503c2e6bf3|
|00412e20|19|6282a7c6801acc042953893b813184c14e395f5215fa6bd1981b9d159d363ee0|
|00d7a280|8|4cfa5b42ca669328764e67cd9a34bb8f90b16ed7ca8d85e8443783d7ccce15ed|
|00d7a24c|4|e00e5eb9444182f352323374ef4e08ebcb784725fdd4fd612d7730540b3e0c8c|

For reference only,00d7a208's four negative-zero bytes hash to
`6d58692645c9d1cfaf13541cbd258f86193ef63c2f1d38f6bbca9617372d7bd6`.
No native bytes were executed during this independent check. This document
establishes byte/assembly evidence and ABI, not build or fixture success;
parent integration owns implementation and validation reports.

## Implementation follow-up

`src/camera_projection.cpp` now implements the perspective builder, four scalar
setters, explicit projection setter and lazy getter through a new owning
CameraProjection interface. Its zero-initialized fields are adapter defaults,
not recovered camera constructor values. Native camera offsets and object ABI
are not reproduced. The FSINCOS helper is inlined with its float32 spill/reload;
the getter retains its argument spills and both x87 matrix copies. No input
validation or fallback projection is invented for degenerate inputs.

The existing generated shader draw uses FOV1.57079637, aspect1, near0.1, far100
and a triangle atZ=1. It exercises an explicit identity override, valid-cache
retrieval, invalidation by setting the same aspect, and lazy perspective rebuild.
It checks the rebuilt depth entries and preserved dual matrices, then uploads
the transposed result atc15. CenterFF407FBF/outsideFF000000 and state restoration
pass. Win32 Release build and both existing CTests pass. No new test target was
added; native differential FP, alternate control words and degenerate inputs
remain untested. This does not establish world/view camera integration or a
runnable game. Body hashes for all seven routines and supporting copy/tangent
helpers are in `reports/camera_projection.json`.
