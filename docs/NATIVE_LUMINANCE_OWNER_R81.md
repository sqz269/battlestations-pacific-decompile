# Native luminance-adaptation owner

Addresses: 00b50d40, 00b50dd0, 00b50fe0, 00b51090

## Result and evidence boundary

R81 reconstructs the complete constructor B50D40[136], destructor B50DD0[526],
scalar destructor B50FE0[30], and initializer B51090[2109]: 2,801 native bytes.
All providers are the existing concrete post20, string, material, texture,
surface and frame implementations. The strict Win32 build and three CTests pass.

A focused probe executes original/source constructor, 241-byte holder-creation
prefix, and full normal lifetime paths with real D3D resources. **The complete
initializer, B4E470 construction, shader registration and native exception paths
were not executed.** Application resource startup and gameplay remain open.

## Constructor and storage

B107F0 requests 250h at B10F25, constructs at B10F47 and initializes at B10F66.
B50D40 is native ECX/EAX/RET. It captures the raw CE6650 float bits (3F7AE148,
approximately 0.98) before any owner store, stamps CEB130/count1/null08/null0C,
then D61FE0, and writes the captured bits to +240. It zeros +214..23C, +244,
+248, the byte +24C, then +210, in that order. Arrays +10..20F and padding
+24D..24F remain preimages. The source interface adds the actual constant view.

## Complete initializer

Native B51090 takes ECX owner and one stacked input holder, RET4. It creates
six 18h holders with full B4E020, format114 (R32F), multisample0/mode0/null
external surface. The first four are 1x1,4x4,16x16,64x64 at +228/+22C/+230/+234;
two more 1x1 holders go to +238/+23C. Loop publications precede disarming state0;
the final two disarm states1/2 before publication.

Six full B4E470(count3,null) children are created:

| Member | Name address and bytes | Native resize length |
| --- | --- | --- |
| +210 | D5E1D0 cleartargets.mshd | 17 |
| +214 | D62050 SampleAvgLum.mshd | 17 |
| +218 | D6203C ResampleAvgLum.mshd | 19 |
| +21C | D6203C ResampleAvgLum.mshd | 19 |
| +220 | D62024 ResampleAvgLumExp.mshd | 22 |
| +224 | D62008 CalculateAdaptedLum.mshd | 24 |

Each name uses zero-header, resize-preserve and current length+1 copy. The
returned child publishes before disarming and returning the name. Normal returns
clear mask bits1/2/4/8/10 first; the last bit20 remains set after disarming.

The four sampling children each bind current input texture0, register the SAME
borrowed 16-float4 array at owner+10 through B18AC0/B17E10(count64,matrix0), then
bind the destination holder's primary surface to current post frame color0:

| Child | Input holder | Output holder |
| --- | --- | --- |
| +214 | Caller input | +234 (64x64) |
| +218 | +234 | +230 (16x16) |
| +21C | +230 | +22C (4x4) |
| +220 | +22C | +228 (1x1) |

Every material/frame access reloads the current member. The final child +224
registers scalar `cAdaptationPercent` at +240 through B18B20(count1,matrix0),
using a distinct local name header. This initializer does not bind textures or
frame targets for the clear/adaptation children, populate either parameter array,
update +240/+24C, or perform rendering.

Finally it captures current F8D394, dispatches actual D5F0A8/+88=B2A070 with
(1,1,1,114,2), publishes the texture to +244, invokes its actual D61948/+30=
B3FD80(0,0), and publishes the retained surface to +248. There is no caller
cleanup state around these two operations. The same canonical renderer, pools,
string service, accounting and synchronization domains serve every resource.

## Cleanup and lifetime

B51090 handler CBFF01 selects FuncInfo DF896C, 26-entry map DF8990. States0/1/2
free only the current raw holder. Raw-post states3/6/10/14/18/22 free the current
raw post; states4/7/11/15/19/23 consume their name bit and then enter the preceding
raw-post state. States5/8/12/16/20/24 use the same masked name action then -1
(unvisited normally). States9/13/17/21 return the common sample-name header;
state25 returns the distinct adaptation header. No published-child rollback is
added. Completed post20 storage is preserved if later host binding fails.
Cleanup edges are consumed before calls; a secondary C++ cleanup exception
finishes remaining edges and replaces the first. This does not prove native
FH3/SEH/double-exception behavior.

Six canonical post companion blocks are prepared before entry and persist after
settlement. Reset releases nothing and requires external disposition, parameter
borrow quiescence, and all six post blocks already idle. Diagnostics preserve
acquired identities; unsupported native profiles are explicit contract errors.

B50DD0 stamps D61FE0 and arms full common-base B0F5E0 cleanup. The first four
holder slots +228..234 each capture their current child and freshly read CE2220
only when nonnull. It next captures child+210 BEFORE capturing CE2220 once,
then uses that target for +210,+238,+23C,+214,+218,+21C,+220,+224,+248,+244.
Each nonnull slot clears only after decrement/zero-terminal returns. Actual
D61EC0 posts use their canonical reference; D61EB8 holders, D61948 textures and
D619A0 surfaces use the existing full deleting lifetimes. Normal exit disarms
and calls full B0F5E0; the destructor's single native unwind action CBFDA0 also
calls B0F5E0. Source cleanup terminates on a secondary C++ exception as documented
by the existing effect lifetime policy; native EH remains unproved.

B50FE0 destroys, frees only when flags bit0 is set, and returns the original
pointer (native ECX/one stack word/EAX/RET4). Its false CALL_RETURN override at
B50FF0 hid ADD ESP4 at B50FF5..B50FF7. The locked repair restored those three
bytes, kept CRT callee flags unchanged and saved the project.

## Runtime validation scope

- Four original/source cases: flags0 and flags1. Whole250h constructor equality,
  six real R32F holders and normalized whole250h creation-prefix equality;
  only six holder identity words are normalized. Arrays and padding checked.
- Original B51090..B51180 prefix ends before the first post allocation. A
  synthetic epilogue consumes outgoing PUSH20, restores FS/saved registers,
  and RET4. Allocation and B4E020 use complete source ABI adapters.
- Each lifetime case adds six explicit sparse20h post fixtures with real frame
  owners and retained surfaces; node/material/draw fields are null. This does
  not execute or stand in for B4E470 initialization. Flags0 additionally keeps
  a caller post reference alive across parent destruction, then releases it.
- A separate full-source B2A070(1,1,1,114,2)/B3FD80 path supplies actual system-
  memory R32F readback texture/surface fields. GetDesc confirms pool/format.
  This validates their destructor interaction, not the initializer's final
  original dispatch sequence.
- Original constructor, derived destructor and scalar body execute in copied
  buffers; original BD30E0 executes at its actual address. Parent reserves three
  code bands before child heap initialization. Zero-terminal adapters invoke
  full existing source lifetimes; the base call uses full source B0F5E0.
- All four cases restore texture/surface tracking and leave the registry empty.
  Singleton drain returns final device/API references to zero; child exits0.
  Native exception handlers are fail-fast traps and are not reached.
- MSVC Win32 /MD /O2 /W4 /WX /fp:strict build; three existing CTests pass.
  No new repository tests. Artifact hashes and native call checks are in the
  report, distinct from the bounded runtime evidence.

## Remaining work

Execute full B4E470 and the complete luminance initializer with the canonical
shader/material/geometry domains; recover later adaptation rendering/array
population and remaining B107F0 providers, then wire complete resource startup
into the application. Native failure/EH, original entry ABI, gameplay and the
separate R79 bloom tail-borrow consumer remain open.
