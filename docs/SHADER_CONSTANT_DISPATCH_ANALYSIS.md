# Runtime system constant source dispatch

The concrete source for the fixed system-register prefix is `00b46a70`.
It gathers camera, timer, fog and lighting values into a local register block,
then uploads the same block to vertex and pixel stages. It is separate from
the per-material tail writer `00b42350`. This read-only investigation verified
project `bsp`, program `/battlestationspacific.exe` before each analysis batch;
no C++, Ghidra metadata, names or saved project state changed.

## Registry consumers and dispatch boundary

`00b5b890` returns the registry vector header at this+4; its recorded caller
is declaration emitter `00b38ff0`. Name lookup `00b5b960` is called by shader
reflection `00b3aea0` and descriptor parsing `00b59e70`. The latter uses found
record dimensions/count through 00b5b840/850/860 and passes the record onward
to 00b58160. These are declaration/metadata consumers, not value providers.

Source-semantic getter `00b5b830` reads record+1Ch. Assembly in 00b3aea0 at
00b3afd6..00b3b001 uses that integer to index the compiled shader's register
byte table at +8 and register-count byte table at +3Eh. An existing non-FF
register byte skips replacement. Those tables drive selective material-tail
updates in 00b42350. There is no evidence here of a general runtime switch
over registry semantic ids for the fixed prefix.

`00b46a70` instead assigns fixed register positions matching native registry
order. ABI: ECX is an optional scene/lighting owner, EDX is camera/context;
plain RET, no stack arguments. Both are preserved in EBP/EDI. The precise
owner class names remain hypotheses. Command executor 00b1d950 calls it at
00b1d9aa with ECX=command+4 and EDX=context+8, before batch execution. Another
caller 00b17240 supplies its this+4 and result of 00b1bf40.

## Confirmed register mappings

After its prologue, the register block starts at ESP+14h. Both final uploads
pass this address, start register zero and global 00e13078 (currently 77).
Stack-relative LEAs around pushes were checked against assembly; pseudocode's
many separate local variables are slices of this single block.

| Register | Registry name | Source evidence |
|---|---|---|
| c0..3 | cScreenToTextureMat | 00b0d100 returns renderer/service+1D8h; transpose00b404a0 |
| c6.xyz | cWorldSpaceEyePos | camera+120h/+124h/+128h after dirty-transform refresh |
| c7..10 | cViewMat | camera helper00b6fcb0, then transpose |
| c11..14 | cInvViewMat | camera+F0h, then transpose |
| c15..18 | cViewProjMat | camera helper00b70490, then transpose |
| c19..22 | cInvViewProjMat | camera helper00b70510, then transpose |
| c23..26 | cProjMat | camera helper00b6fcf0, then transpose |
| c27..30 | cSysParam0..3 | 16 words at globals0108fc30..0108fc6c |
| c31.xyz / c32.xyz | cWorldSpaceCamXaxis/Yaxis | helpers00b70fe0/00b70ea0 |
| c33 | cTime | timer/service sources described below |
| c34 | cElapsedTime | ratio, service data and reciprocal described below |
| c35.xyz | cFogParams | fog owner+6Ch,+70h,+68h, in that order |
| c36.xyz | cFogHeightParams | fog owner+74h,+78h,+7Ch |
| c37 | cFogColor | four words from fog owner+8h |
| c38..41 | cFogDirColor4 | four calls to00b84fd0(index0..3), each copies float4 |

Fog owner is camera+184h. When null, the entire fog-dependent block is skipped.
Its getter bodies are plain FLD [ECX+offset]; RET for scalar fields, and
00b84c60 returns ECX+8 for color. Later paths populate underwater fog and other
values, optional scene lighting/ambient/shadow data, and optional context data.
Their full object contracts were not recursively reconstructed.

Time c33 components are assigned at 00b46cb4..00b46cf2:

- x: float at `[global01090ab0]+4`.
- y: float at `004de4b0()` result+18h.
- z: getter00af0460 reads `[global00f8c274]+2Ch` and returns ST0.
- w: getter00ad5700 reads `[global00f8c210]+8h` and returns ST0.

Elapsed c34 components at 00b46d01..00b46d6d are:

- x: timer virtual+1Ch returns two signed 64-bit integers; FILD/FILD/FDIVP
  computes first/second, then FSTP float32.
- y: byte at `00b0cf30(global00f8d39c)+24h`, converted to float.
- z: float at that service result+28h.
- w: reciprocal of camera+1C4h, calculated with x87 and stored float32.

The registry names identify intended shader slots. They do not prove the
units or independent clock meanings of each component.

## Implementable dependency and remaining uncertainty

The smallest complete next routine is `00b404a0`, a 101-byte matrix transpose
writer used for all six matrices above. ABI: destination float[16] in ECX,
source float[16] as one stack argument, RET4. It writes
`dst[4*r+c] = src[4*c+r]` in destination order, using an FLD/FSTP pair per
element. A typed nonoverlapping source/destination API is immediately feasible;
the original sequential writes are not an in-place transpose. Exact special
NaN conversion/status behavior requires retaining x87 loads/stores, whereas
an ordinary finite-word transpose must explicitly bound its equivalence.

A useful next larger fragment is the fog mapping at 00b46ddd..00b46e9a,
with projected fog fields and explicit output capacity. It must preserve
unwritten float4 components and skip writes when fog is absent. It is a
fragment of00b46a70, not a recovered standalone function.

Do not implement the entire prefix by silently zero-initializing it and call
that native parity. This function reserves stack storage without clearing it;
float3 padding and fields gated by absent owners can remain unwritten, and
the cloud-shadow slots c4..5 have no identified writes in this body. Both
stages still receive the full77-register span. A new safe deterministic
interface can supply defaults, but must label that as a behavior choice.

The caller does not test upload HRESULTs. A full port also needs the scene,
camera dirty-cache helpers, timers, fog/lighting/shadow owners and original
matrix evaluation conventions. This report establishes their dispatch entry
and selected field mappings, not native ABI compatibility or game validation.

## Byte verification

Saved-image bytes matched the original executable for all checked ranges:

| Address | Length | SHA-256 |
|---|---:|---|
| 00b46a70 | 3017 | 60ecc210cbbafe2cbeece90dd0d4a6ebac84d90bc598c725729451981e93b3c2 |
| 00b404a0 | 101 | 6b9c5862e0bd255777adc773d325a5dbbe575adc8548b8e40f583f5bf6fd5e30 |
| 00b5b830 | 4 | 0b78473b3a56c63ff2a64431fb600078066cbe407d3dee7ea8fd7575bd7f82d5 |

Fog getters00b84cb0/cc0/ca0/c60 also matched their four-byte original bodies.
These comparisons establish only the listed ranges, not whole-image identity.
