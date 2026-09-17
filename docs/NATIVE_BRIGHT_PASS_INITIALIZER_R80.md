# Native bright-pass initialization

Addresses: 00b54940

## Result and evidence boundary

R80 implements complete B54940..B54CB4 (885 bytes) using the existing concrete
post-effect20, raw-string, material, texture-holder and frame-target providers.
The strict MSVC Win32 build and three existing CTests pass. Selected live Ghidra
bytes match the original PE, and every direct call is recorded and checked.
**The initializer itself has not been executed.** These checks do not establish
native ABI, exception behavior, application resource initialization or gameplay.

The original interface is ECX existing 224h owner, four stacked words (input
holder, output width, output height, format), RET10h. The service's inline
constructor at B10F6B..B10F9A requests 224h, writes CEB130/count1/null children,
then profile D5E1B4. B10FAE invokes this initializer with format21. The new C++
interface requires explicit contexts and a persistent companion block.

## Normal sequence

1. Allocate 20h and construct full B4E470 with `brightpassfilter.mshd` at D62138,
   count3 and null optional input. Publish the returned child to owner+08, disarm
   cleanup, then return the temporary name through the actual raw-string pool.
2. Register the following scalar sources, in order, through existing B18B20.
   Every call reloads current owner+08 and child+14. B18B20 forwards count1 and
   matrix0 into full B17E10/B44D60; it does not snapshot the source value.

| Native name bytes | Address | Source offset | Resize length | Cleanup state |
| --- | --- | --- | --- | --- |
| cBrightPassTreshold | D62124 | +210 | 19 | 3 |
| cBrightPassOffset | D62110 | +214 | 17 | 4 |
| cMiddleGray | D5E3F0 | +218 | 11 | 5 |
| cMinLuminance | D5E3C0 | +21C | 13 | 6 |
| cMaxLuminance | D5E3B0 | +220 | 13 | 7 |

The native spelling `Treshold` is preserved. Each temporary starts with a zero
eight-byte string header, calls resize-preserve, copies current length+1 bytes
when data is nonnull, arms its state for registration, disarms, then returns the
name. The first four reuse one header; the last uses its distinct native header.

3. Obtain the current input-holder texture through B4CB10 and bind current
   material slot0 through full unchecked B189F0.
4. Allocate 18h and construct full B4E020(width,height,format,0,0,null), then
   disarm and publish the returned holder to owner+0C.
5. Obtain that holder's primary surface through exact B4CB20 and bind it to the
   current post-effect frame's color0 through full B4CB70/B1FAB0.

Only child words +08/+0C are written in the parent. This initializer does not
write the scalar values, clear +10..20F, reset the profile/count, release old
children, populate kernels or run a rendering pass. Existing scalar preimages
and all five source lifetimes remain the caller's responsibility.

## Nine-state cleanup

Handler CC0247 selects FuncInfo DF8D24, with nine-entry map DF8D48:

| State | Next | Native action |
| --- | --- | --- |
| 0 | -1 | Free current raw post20 allocation, CC01F0 |
| 1 | 0 | Consume name-mask bit1, return effect-name header, CC01FB |
| 2 | -1 | Same masked return; unvisited normally |
| 3/4/5/6 | -1 | Return common parameter header, CC0214/21C/224/22C |
| 7 | -1 | Return distinct final parameter header, CC0234 |
| 8 | -1 | Free current raw holder allocation, CC023C |

The normal effect-name return leaves bit1 set after disarming state to -1.
The source consumes each cleanup edge before calling it, and does not roll back
published children. If full B4E470 completed native construction before a host
binding failure, its raw allocation is preserved for explicit disposition.
A secondary C++ cleanup exception finishes remaining edges and replaces the
first exception; this is the explicit source policy, not proven native FH3/SEH
equivalence. Invalid/null native dispatch domains fail as source contract errors.

The companion is prepared before entry and retains the full existing B4E470
block. Unused preparations are cancelled when settling. Reset releases nothing:
the caller must dispose survivors, end borrowed parameter lifetimes, and reset
the nested post block first. The wrapper has no alternate material/shader state.

## Validation and follow-up

- Strict /MD /O2 /W4 /WX /fp:strict Win32 build; all three existing CTests pass.
- 1,284 selected live/PE bytes: main885, existing scalar wrapper22, unwind97,
  FuncInfo/map108, six name extents100, and service caller72.
- Main body has 280 instructions and zero listing gaps. Report records all42
  main calls plus the wrapper's one forwarding call. No new repository tests.
- Ghidra receives a descriptive hypothesis name and evidence through the locked
  tool, preserving previous comments and refreshing the export.

Next, execute complete post-effect initialization with the canonical shader,
material and geometry domains, recover the remaining B107F0 providers (including
its B50D40/B51090 chain), and bind the complete resource-service path into the
application. The initializer runtime, native failure paths and gameplay remain
open. R79's separate bloom +430 tail-borrow question remains open as well.
