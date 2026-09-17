# Native 4x4 and 2x2 downscale initialization

Addresses: 00b544f0, 00b546f0, 00b4cd30

## Result and scope

R78 reconstructs two complete initializer bodies and the complete 2x2 sampling
helper, totaling 1,093 native code bytes. They use R77's existing prepared
post-effect companion and five-state cleanup block, concrete material/texture
providers and real frame-target binding. The new initializer bodies are build
tested. The full numeric helper and a 4x4 arithmetic fragment are original/source
tested; **neither complete initializer has been executed**. Application wiring,
shader/material startup, native exceptions and gameplay remain open.

| Entry | Extent | Original interface | Result |
| --- | --- | --- | --- |
| B544F0 | B544F0..B546D4, 485 bytes | ECX existing220h pass; input holder, output width/height/format; RET10h | Complete 4x4 initializer source |
| B546F0 | B546F0..B548A1, 434 bytes | ECX existing90h pass; same four stack words; RET10h | Complete 2x2 initializer source |
| B4CD30 | B4CD30..B4CDDD, 174 bytes | ECX unused; unsigned width/height/output stacked; RET0Ch | Complete numeric source and original/source comparison |

The service B107F0 pushes 220h at B10E5A and 90h at B10EBE. Its inline
constructors initialize only profile, count and +08/+0C, ending with D5E18C and
D5E1A0. Calls B10EB9/B10F20 enter these initializers. The selected 203-byte
caller span is live/PE verified; untouched inline arrays are genuine preimages,
not source omissions to fill with invented defaults. Names remain hypotheses.

## Shared creation and binding sequence

Each initializer allocates a 20h post-effect, constructs its actual pooled shader
name and calls complete B4E470(count3,null) through persistent canonical companion
storage. It publishes pass +08 before disarming cleanup and returning the name.
Shader names are D620E8/downscale4x4.mshd and D620FC/downscale2x2.mshd.

The 4x4 pass constructs cInvTextureSize from D620D8 and registers one borrowed
float4 at pass +210. The 2x2 pass constructs cSampleOffsets from D5E40C and
registers four borrowed float4s at +10. Both register BEFORE writing those values,
then return the temporary parameter name. The complete B18AC0 forwarding contract
maps vector counts 1/4 to 4/16 DWORDs with matrix flag zero in actual B17E10/B44D60.
Registration borrows receiver memory; it does not copy a new parameter cache.

After registration, both capture the input holder's borrowed texture with B4CB10,
freshly reload the current post-effect/material, and invoke complete unchecked
B189F0 for texture slot 0. Only then do they capture current configuration
publication 0109CF04 and produce their parameter values. The passed output
dimensions are separate from these global configuration dimensions.

Each allocates an 18h holder and calls full B4E020 with the passed output
width/height/format and zero multisample/mode/external surface. It disarms caller
cleanup, publishes +0C, reads the returned holder's primary surface, reloads
current pass +08 and calls the complete color0 wrapper. Prior pass children are
not released or restored, and no fallback producer is substituted on failure.

## 4x4 signed inverse size

B5462B captures the global configuration once. It reads +24 as a signed DWORD
and uses FILD, then reads +28 from that captured configuration and uses FIDIV.
FLD1/duplicate/FDIVRP form 1/width and 1/height without unsigned correction.
The source preserves the x87 stack schedule, writes zero bits to +218/+21C
before the spills, then reloads float temporaries into +210/+214. It leaves
the entire +10..20F region untouched. Private native stack scratch aliases are
outside the new source interface.

## 2x2 unsigned sample offsets

B54828 captures configuration once, reads height +28 before width +24, and calls
B4CD30 with output at pass +10. The helper independently converts each DWORD
with FILD and conditional FADD of the original **float** 2^32 at CE3978.
It computes reciprocals with the original x87 stack and single-precision spills.
CE3D78 supplies double 1.5; CEC9E0 supplies double -0.5. The four records are:

| Record | X | Y | Z/W |
| --- | --- | --- | --- |
| 0 | -1.5 / width | -1.5 / height | unchanged |
| 1 | -0.5 / width | -1.5 / height | unchanged |
| 2 | -1.5 / width | -0.5 / height | unchanged |
| 3 | -0.5 / width | -0.5 / height | unchanged |

These formulas describe the result, not a license to replace the recovered
operation order: x87 precision/rounding controls, intermediate float stores,
reloaded Y values and partial stores remain significant. The source preserves
the full instruction-level arithmetic schedule, including both row iterations
and final x87 stack disposal. It never clears Z/W or the receiver's other fields.

## Cleanup and host lifetime

The 4x4 handler CC0187 points to FuncInfo DF8C8C/map DF8CB0. The 2x2 handler
CC01D7 points to FuncInfo DF8CD8/map DF8CFC. Both five-state maps match R77:
state0 frees raw post storage; state1 consumes a name mask then advances to0;
state2 returns the same name and is not visited normally; state3 returns the
parameter name; state4 frees raw holder storage. All other edges end at -1.
There is no cleanup of already-published children or borrowed parameter arrays.

R77's block supplies exactly this schedule without a second ownership domain.
The extension adds friend/member declarations only; it adds no block data members
and does not modify either R77 initializer body. A completed B4E470 child survives
host binding failure; consumed cleanup edges are not retried. Source secondary
C++ cleanup exceptions finish remaining edges and replace the first exception.
Native FH3/SEH/double-exception equivalence remains unproved. External disposition
and host quiescence precede resetting the shared block.

Both initializers and the numeric helper have zero Ghidra listing gaps. Their
unwind funclets are recorded with full disk POP ECX/RET tails after free, without
changing unrelated function definitions or library flags.

## Validation

- Strict MSVC Win32 /MD /O2 /W4 /WX /fp:strict build and all three CTests pass.
- 1,663 selected live Ghidra bytes match the original PE: 1,093 packet bytes and
  570 bytes of caller, unwind, constants and names.
- One ignored numeric probe uses original readonly constants and executes the
  entire original B4CD30 body against the compiled source for 96 pairs: eight
  dimension pairs, three x87 precisions and four rounding modes. Whole guarded
  48h buffers match, including all untouched Z/W words and outer guards.
- The same probe compares the exact 71-byte B5462B..B54671 fragment against the
  compiled source helper for another 96 pairs. Whole guarded 224h receiver
  buffers match. Both comparisons also match x87 exception flags and stack-top
  bits, and preserve the selected control word. Zero/high-bit dimensions are
  included with floating exceptions masked.
- The fragment adapter supplies ESI=owner+210 and scratch stack, relocates the
  single configuration cell, and consumes the outgoing allocation argument.
  It does not execute the allocator, full initializer or native EH handler.
- No new repository tests; no new game/application or D3D execution claim.
  The report seals the tested inputs before integration and separately records
  the combined build. The original game image/full data image are not published.

## Remaining work

All four small downscale/blend pass initializers now have complete source bodies.
They still require full execution with the existing shader/material/geometry
domains before application resource-service binding. Continue B107F0's remaining
subordinates, especially B54F90 bloom initialization, preserving concrete
constructors and lifetime blocks. Native failure/unwind, unmasked FP exceptions,
application startup and gameplay remain open.
