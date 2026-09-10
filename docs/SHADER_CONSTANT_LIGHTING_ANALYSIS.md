# Lighting, shadow and unwritten system registers

This continues the read-only assembly analysis of 00b46a70 documented in
SHADER_CONSTANT_DISPATCH_ANALYSIS.md. The original ABI is ECX=optional scene
owner, EDX=camera/context, plain RET. It uploads the local 77-float4 register
block to both stages. No semantic-id lookup occurs in this body.

For the offsets below, SP means ESP after the four saved-register pushes and
before temporary call arguments. Register cN starts at SP+14h+16*N. Checked
every analysis batch against project bsp and program /battlestationspacific.exe.
No Ghidra, C++, shared metadata or project-save changes were made.

## Lighting path: c42..52

At 00b46ed1, null incoming scene owner skips the lighting and shadow path.
Otherwise getter00b72110 returns sceneOwner+1Ch pointer, called A below. Null A
also skips it. A+1Ch is a list sentinel pointer; the code loads its first node
and checks node != sentinel, invoking00bf6713 on failure. The valid path uses
node+8 as light object L and A+10h as environment object E. It does not contain
independent null checks for L or E. Complete class names and list lifetime are
not established.

| Register | Name | Source and write |
|---|---|---|
| c42.x | cSpecularPower | Float global00cfad80=128.0; store00b47035 |
| c43 | cAmbientColor | E+18h, except camera+198h==3 selects E+28h |
| c44 | cDirLightDiffuseColor | L+184h, except mode3 selects L+1B4h; four words |
| c45 | cDirLightSpecularColor | L+194h..1A0h; four words |
| c46..51 | cAmbientCube | Six float4 records E+38h+i*10h, i=0..5 |
| c52.xyz | cDirLightWorldSpaceDir | L+1E0h,+1E4h,+1E8h |

The branch at00b46ef8 selects the two ambient/diffuse variants only. Specular,
ambient cube and direction are shared afterward. Helpers00b7aa20/00b7aa30
return E+18h/E+28h, with ECX input and EAX output, plain RET. Getter00b7aa40
uses ECX=E and one stack index, returning E+38h+16*index with RET4. The loop
at00b47050..00b47079 copies each returned four-word record.

## Shadow path: c53..70 and c76

Getter00b7aab0 receives ECX=L and returns `[L+174h]` in EAX, plain RET. Its
null result S skips the entire shadow path at00b47086. On the valid path:

| Register | Name | Source |
|---|---|---|
| c53..56 | cShadowMapMat0 | S+144h, 16 floats transposed |
| c57..60 | cShadowMapMat1 | S+184h, 16 floats transposed |
| c61..64 | cShadowMapMat2 | S+1C4h, 16 floats transposed |
| c65..68 | cShadowMapMat3 | S+204h, 16 floats transposed |
| c69.xyz | cShadowLightDir | S+38h,+3Ch,+40h |
| c70 | cShadowLimits | S+390h,+394h,+398h,+39Ch |
| c76 | cShadowMapSizeData | width,height,1/width,1/height from virtual accessors |

The matrix sequence00b4708c..00b474c3 consists of MOVSS loads/stores, mapping
`dst[4*r+c]=src[4*c+r]` without floating-point arithmetic. Unlike matrix helper
00b404a0's x87 FLD/FSTP, these copies preserve raw signaling-NaN bits and do not
perform x87 conversion. A shared numerical helper must preserve this difference
or state its restriction to ordinary values.

For c76, the code calls S virtual+8h, then returned object virtual+3Ch for
width. It independently calls S virtual+8h again, then returned object
virtual+40h for height. Those object and accessor ABIs use ECX and plain return;
concrete vtable classes are not resolved here. The returned DWORDs are treated
as unsigned: FILD signed32 followed by conditional addition of float 2^32
(00ce3978) when the sign bit is set. Width and height are stored as float32
before reciprocals use FLD1/FDIV. Zero dimensions have no guard. Getters can
in principle return different objects on the two calls; caching their result
would be an additional assumption.

## Context and underwater fog: c71..75

These paths do not require a lighting or shadow owner:

| Register | Name | Condition and source |
|---|---|---|
| c71 | cShadowDepthScale | camera+43Ch pointer non-null: copy its float4 |
| c72.xy | cFogUWParams | fog owner camera+184h non-null: fields+80h,+84h |
| c73.xyz | cFogUWHeightParams | same fog owner: fields+88h,+8Ch,+90h |
| c74 | cFogUWColor | same fog owner: four words+18h..24h |
| c75.x | cIsOverWave | camera byte+174h converted unsigned to float |
| c75.y | cIsOverWave | byte `[global00f8c210]+11h` converted unsigned to float |

The c72/c73/c74 work happens earlier at00b46d97..00b46eca, inside the same
fog-owner gate as c35..41. Scalar getters00b84da0/db0/e20/e30/e40 each use
ECX=fog owner, FLD field, plain RET with ST0 result. Color getter00b84c90
returns ECX+18h in EAX, plain RET. Getter00ad5740 returns AL=[ECX+11h]; its
upper EAX bytes are not part of the value. The caller explicitly MOVZXs AL.

Getter00b6feb0 returns camera+43Ch in EAX. The c71 path at00b475b4 tests one
call's result, calls it again if non-null, then copies the four words. It is
reached even when lighting or shadow checks skipped their work. The registry
name suggests depth scaling, but the field's higher-level producer is untraced.

## Exact unwritten spans within this scope

The function never initializes these components in any path:

- c4..5 in full: nominal cCloudShadowMat records, with no identified writer.
- c6.w, c31.w, c32.w, c35.w and c36.w from the camera/axes/fog stages.
- c42.yzw, c52.w, c69.w, c72.zw, c73.w and c75.zw.

Additional conditional unwritten spans are:

- Missing scene owner or A: c42.x, c43..51 and c52.xyz, plus all shadow output.
- Missing shadow S: c53..68, c69.xyz, c70 and c76.
- Missing fog owner: c72.xy, c73.xyz and c74 (also c35..41 from the other report).
- Missing camera+43Ch pointer: c71.

These are uninitialized portions of a local stack buffer, not necessarily
retained GPU register values. The routine still uploads all 77 float4s in both
stages. No call before upload has a destination covering c4..5: the first
00b404a0 invocation writes exactly c0..3 and its next invocation begins c7.
There is no memset, zeroing loop or whole-block constructor in this body.

A typed reconstruction should expose a selective write contract over supplied
storage and make deterministic defaults an explicit caller choice. Emulating
arbitrary stale stack contents would not give a useful or stable interface.
The complete ordered typed prefix now implements this selective write contract;
see `SYSTEM_CONSTANT_BUILDER.md` and `SYSTEM_LIGHTING_CONSTANTS.md`. Its supplied
owner bindings and installed-asset probe do not establish native scene/world
construction or gameplay. The empty-list handler may return; the exact retained
sentinel and subsequent field reads are covered by `SYSTEM_EMPTY_LIGHT_HANDLER.md`.

## Byte evidence

The complete 3017-byte body 00b46a70 matches original disk and saved image,
SHA-256 `60ecc210cbbafe2cbeece90dd0d4a6ebac84d90bc598c725729451981e93b3c2`.
Also matched: 00b7aa40 (14 bytes), 00b7aab0 (7), 00b6feb0 (7), 128.0 literal 00cfad80
(4), and 2^32 literal 00ce3978 (4). These establish only the checked ranges,
not whole-image identity. This is assembly-grounded analysis, with no new
compiled or runtime/game validation.
