# Native depth-downscale and particle-blend initialization

Addresses: 00b540b0, 00b542d0, 00b4cb20, 00b4cb70

## Result and evidence boundary

R77 reconstructs four complete bodies, totaling 1,045 native code bytes, using
the existing concrete post-effect, material, texture/surface and frame providers.
Both full initializer source bodies pass the strict Win32 build and existing
CTest suite. Their x87 fragments and two small resource helpers pass focused
original/source comparisons. **The complete initializers were not executed**;
shader/material startup, native unwind and application/gameplay remain open.

| Entry | Native extent | Original ABI | Source coverage |
| --- | --- | --- | --- |
| B540B0 | B540B0..B542A3, 500 bytes | ECX existing pass, four stack words, RET10h | Complete depth-downscale initializer |
| B542D0 | B542D0..B544DA, 523 bytes | ECX existing pass, nine stack words, RET24h | Complete particle-blend initializer |
| B4CB20 | B4CB20..B4CB23, 4 bytes | ECX holder, EAX borrowed surface, RET | Exact primary-surface getter |
| B4CB70 | B4CB70..B4CB81, 18 bytes | ECX post-effect, stack surface, RET4 | Complete color-target wrapper |

Names are descriptive hypotheses. New context/block interfaces differ from the
original ABI. Pass storage is an existing aligned 20h object with its profile
and count already initialized; neither routine initializes the outer lifetime.

## Concrete initialization sequence

Both routines allocate a 20h post-effect and, when nonnull, construct a pooled
native name and call full B4E470 with vertex count 3 and null optional input.
They publish the returned child at pass +08, disarm caller cleanup, then return
the temporary name through the actual raw string-pool service. They do not
release a prior +08 child. Depth uses D620C0/downscale_depth.mshd; particle uses
D5E41C/blendparticles.mshd.

Depth computes its inline float4, registers cSampleOffsets from D5E40C with count
one through B18AC0's complete B17E10 forwarding contract, then reads the input
holder's borrowed texture with B4CB10 and binds current material slot 0. Particle
first binds its two input textures to current material slots 0/1, then computes
and registers the offsets. Each material access freshly reloads pass +08 and
the child's +14 through the existing B4CBA0 getter. B189F0 uses its complete
unchecked native field implementation and the same canonical actual owners.

The registration borrows pass +10..1F; it does not copy four values into a new
parameter cache. Actual B17E10/B44D60 handles the existing material/effect tables,
parameter pool and names. Those domains and the inline source must outlive all
consumers. The first particle argument at original stack +04 is unread; its
next two words are textures and the remaining six are holder arguments.

Each routine then allocates 18h and invokes full B4E020. Depth passes original
width/height/format, zero multisample/mode and null external surface. Particle
forwards width, height, format, multisample, mode and external surface unchanged;
the holder interprets the mode's low byte. The original dimensions, not doubled
arithmetic temporaries, reach the holder. After disarming cleanup, each publishes
+0C, reads the returned holder's primary +0C via B4CB20, reloads current pass +08,
and invokes B4CB70. That helper freshly reads post-effect +08 as the frame group
and calls complete B1FAB0 with color slot 0 and the supplied surface.

Allocation-null paths publish null and continue as the original does. Source
valid-storage checks diagnose a later required null dereference; they do not
invent a fallback child, successful return or parent rollback.

## Exact arithmetic and store order

Depth uses wrapping DWORD 2*width and 2*height; particle uses the original DWORDs.
FILD interprets each as signed, and a negative value adds the original double
2^32 at D57DA0. FLD reads the original double 0.5 at D7A280 once and duplicates it.
The source preserves the x87 division stack and the caller's precision/rounding
controls, spills x to float, then computes y. It writes exact zero bits to +18/+1C
and zeros the parameter-name header before spilling y, then reloads both float
temporaries and stores +10/+14 in order. It does not replace this sequence with
SSE division or a separately rounded integer-to-float conversion.

The shared implementation helper corresponds to B5414B..B541C2 and
B54397..B5440B. It is not an additional original function. Private-stack aliasing,
unmasked floating exceptions and instruction-level ABI parity remain unproved.

## Five-state cleanup and persistent host storage

Depth's handler CC00E7 points to FuncInfo DF8BF4 and map DF8C18. Particle's handler
CC0137 points to FuncInfo DF8C40 and map DF8C64. Both maps contain:

| State | Next | Cleanup |
| --- | --- | --- |
| 0 | -1 | Free raw post-effect allocation |
| 1 | 0 | Consume mask bit, return completed effect-name header |
| 2 | -1 | Same masked name cleanup; not visited by the normal bodies |
| 3 | -1 | Return parameter-name header |
| 4 | -1 | Free raw holder allocation |

The raw-free funclets' complete disk tails include POP ECX/RET after free even
where their existing Ghidra body ends early. This packet reads and records them;
it makes no unrelated funclet edits and neither initializer has a listing gap.
Publication is not rolled back, and a partial holder failure frees only its raw
outer allocation after the existing callee's base cleanup. Any surviving nested
resources remain visible in persistent acquired records for external disposition.

Preparation reserves the full existing B4E470 companion block before execution.
After B4E470 native completion, a host binding diagnostic must preserve the raw
child and canonical companions rather than free a completed native object. The
parent block survives settled failures and adds no ownership. Name-return/free
edges are consumed before calls, preventing retries. The source policy lets a
secondary C++ cleanup exception finish remaining edges and replace the first;
this is explicit source behavior, not a claim about native FH3/double exceptions.
Reset requires external quiescence and an already-reset child block, and releases
nothing automatically. Parameter-source borrows must have ended first.

## Validation

- Strict MSVC Win32 /MD /O2 /W4 /WX /fp:strict build and all three CTests pass.
- 1,437 selected live Ghidra bytes match the PE: 1,045 packet bytes, 40 bytes of
  existing getters/parameter wrapper, and 352 bytes of names/constants/unwind data.
- One ignored focused probe compares the two original arithmetic fragments with
  the compiled production helper across 192 pairs: eight dimension pairs,
  three x87 precisions, four rounding modes and both doubling modes. Whole 24h
  guarded buffers, zeroed name headers, exception flags, stack-top bits and
  unchanged control words match, including wrapped zero/high-bit dimensions.
- The probe executes original B4CB20 and B4CB70 against real D3D9 surface owners
  and the full existing frame setter. It checks retain, equality, replacement
  and null assignment on original/source paths, tracking restoration, two raw
  singleton registrations drained and final device/API references zero.
- The compiled four-byte getter equals the original 8B410CC3. No new repository
  tests were added. The complete initializers and their EH handlers are not
  executed by this probe; no mocked post-effect producer is used as proof.

The probe's renderer is an explicit zeroed 1D94h fixture with a real HAL device,
canonical pools/raw strings/scalars/support and complete R75 holder construction.
Its post-effect view supplies only the real frame pointer needed by B4CB70.
The arithmetic entry adapter supplies the fragment's original register/stack
inputs and consumes its two outgoing resize arguments before returning.
The report seals the tested inputs before integration and records the separate
combined-build receipt. The game image and full data image are not published.

## Remaining dependency work

Bind these complete initializer interfaces only after the application can supply
the existing B4E470 shader/material/geometry domains and persistent companion
blocks. Continue B107F0's remaining graph, including B544F0/B546F0 and B54F90,
without replacing constructors or resource loaders with empty producers.
Full initializer execution, failure/FH3/SEH, application startup and gameplay
are still required before any broader reconstruction-completion claim.

## Correction from docs/NATIVE_DOWNSCALE_PASS_INITIALIZERS_R78.md

R78 adds complete B544F0/B546F0 source and full B4CD30 numeric-helper source.
The existing R77 persistent cleanup block is reused without changing its data
layout or the R77 initializer bodies. Numeric comparisons pass; complete pass
initializer execution, B54F90 bloom initialization and application binding remain open.
