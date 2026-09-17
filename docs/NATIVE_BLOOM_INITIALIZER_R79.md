# Native bloom owner initialization

Addresses: 00b54f90

## Result and evidence boundary

R79 reconstructs complete B54F90..B55542 (1,459 bytes), using the full existing
texture-holder, post-effect20, material and raw-string providers. The strict
Win32 build and three CTests pass. A real-D3D probe compares the complete
363-byte creation/numeric prefix, and a separate numeric comparison covers
precision/rounding edge cases. **The full initializer, shader registration and
native exception paths were not executed.** Application/gameplay remain open.

Original ABI: ECX existing 43Ch bloom owner, five stacked words (input holder,
output width, output height, format, exact parameter+438 bits), RET14h. The new
source interface adds persistent host state and concrete contexts. The owner
must already have undergone B54E70 construction; no whole-object initialization
or old-child release is introduced. The name is a descriptive hypothesis.

## Texture creation and exact numeric ordering

The initializer stores the final argument's bits at +438. It separately obtains
the input holder's current texture twice, dispatching current +3C/B3CE50 for
reported unsigned width and current +40/B3CE60 for height. The results publish
to +10/+14. The admitted concrete texture profile is D61948; other profiles are
source contract errors, not fallback resource domains.

It allocates and fully constructs three 18h B4E020 holders, in order, publishing
to +18/+1C/+20. Each receives the passed output width/height/format with zero
multisample/mode and null external surface. State0 covers only the current raw
allocation; it disarms after each publication. Completed earlier holders survive
a later failure, as in the original.

For the first output holder, it captures a texture for the height getter BEFORE
freshly obtaining the texture used for width. Width conversion uses FILD plus
conditional FADD of float 2^32 at CE3978; FDIVR reads double 0.5 at D7A280 and
spills a float BEFORE calling the captured texture's height getter. After that
call it converts height, rereads double 0.5, duplicates it on x87, and stores the
output half-texel pair at +428/+42C. The retained x87 half value then divides
the freshly read input dimensions +10/+14 and stores +430/+434. The source
preserves the original multiple reads, sign-test ordering, float spills/reloads
and caller floating-point controls. Parameter+438 is only a bit copy.

The service has two evidenced 43Ch allocations/calls: B10FB3/B10FD5/B1101C and
B11DE5/B11E07/B11E3B. Its original format words are 21 and 113. Selected live/PE
caller spans verify these interfaces without treating the larger B107F0 body
as implemented.

## Post-effect creation and borrowed parameters

Two persistent existing B4E470 companion blocks are prepared before execution.
The original native sequence creates the blur child first, at +24, from the
12-character blur5x5.mshd name at D62180 with vertex count3 and null optional
input. It publishes before returning the temporary pooled name. It then
registers these current blur material parameters:

| Name | Borrowed receiver offset | Float4 count | Native cleanup state |
| --- | --- | --- | --- |
| cSampleOffsets | +228 | 16 | 4 |
| cSampleWeights | +328 | 16 | 5 |
| cTextureOffset | +430 | 1 | 6 |

Only then does it capture the input holder's current texture and bind blur
material slot0 through complete unchecked B189F0. It next creates the bloom
child at +08 from the 10-character bloom.mshd name at D62164, again through
complete B4E470(count3,null), and registers:

| Name | Borrowed receiver offset | Float4 count | Native cleanup state |
| --- | --- | --- | --- |
| cSampleOffsets | +28 | 16 | 10 |
| cSampleWeights | +128 | 16 | 11 |
| cTextureOffset | +428 | 1 | 12 |

Every registration freshly reloads the current member and +14 material. The
complete B18AC0 forwarding contract sends four times the vector count and
matrix0 into actual B17E10/B44D60. Names are built with the original zero-header,
resize-preserve and length+1 copy sequence, then returned through the actual
raw string pool after disarming cleanup. The last texture-offset name uses its
distinct native local-header projection. No bloom texture-slot assignment or
frame-color binding is invented; later draw/update code owns those operations.

All four 100h sample arrays, +28..427, retain their preimages. This initializer
registers their addresses but does not fill kernels or weights.

### Native tail-borrow boundary

Blur's cTextureOffset is one float4 starting at +430, so its descriptor spans
[430,440) while both evidenced caller allocations request **43Ch**. Its fourth
DWORD is beyond that requested extent. The initializer/registration does not
read source values, and this packet does not enlarge the object, clear an extra
word, or silently reduce the count to hide the discrepancy. The later shader
consumer, actual readable allocation padding and any native reliance on this
tail remain unproved and must be resolved before broader runtime claims.

## Thirteen-state cleanup

Handler CC0303 points to FuncInfo DF8DBC, thirteen-entry map DF8DE0. States are:

| State | Next | Cleanup |
| --- | --- | --- |
| 0 | -1 | Current raw holder only |
| 1 | -1 | Raw blur allocation |
| 2 | 1 | Consume mask bit1, return common name |
| 3 | -1 | Same masked blur-name return; unvisited normally |
| 4/5/6 | -1 | Current common parameter name |
| 7 | -1 | Raw bloom allocation |
| 8 | 7 | Consume mask bit2, return common name |
| 9 | -1 | Same masked bloom-name return; unvisited normally |
| 10/11 | -1 | Current common parameter name |
| 12 | -1 | Distinct final parameter name |

The normal blur-name return clears its mask first; the normal bloom-name return
leaves bit2 set. The source retains this difference. Published holders/children
and parameter borrows are never rolled back. A post-effect that completed native
construction survives a later host companion-binding failure instead of being
raw-freed. Cleanup edges are consumed before calls; the explicit source policy
finishes remaining edges after a secondary C++ exception, then replaces the
first exception. This does not prove original FH3/SEH/double-exception behavior.

The block retains acquired raw identities, nested holder diagnostics, both
canonical post-effect companion blocks and name headers. Reset releases nothing:
external disposition and parameter-borrow quiescence must precede resetting both
child blocks and then the parent. The full main body has zero Ghidra listing
gaps; existing free-funclet disk tails are recorded without unrelated mutations.

## Validation

- Strict MSVC Win32 /MD /O2 /W4 /WX /fp:strict build and all three CTests pass.
- 2,038 selected live Ghidra bytes match the original PE: 1,459 initializer bytes,
  two existing four-byte dimension leaves, and 571 bytes of names/constants,
  table slots, unwind records and two caller spans.
- One ignored probe compares 96 original/source numeric cases: eight input/output
  dimension pairs, three x87 precisions and four rounding modes. The original
  B5505F..B550FA fragment includes its actual height-getter call. Whole guarded
  440h buffers, exception flags, stack-top bits and unchanged control words match.
  Floating exceptions are masked; zero/high-bit unsigned dimensions are included.
- Four original/source real-D3D cases execute the B54F90..B550FA prefix in native
  formats 21 and 113. A real 256x128 input and three 32x16 outputs are created.
  Whole43Ch parent bytes match after normalizing only the three holder identities;
  sample-array preimages and +438 bit patterns, including a NaN payload, survive.
- Prefix teardown uses complete R76 bloom lifetime and R75 holder lifetime.
  Texture/surface tracking returns to baseline, two singleton registrations drain,
  and final device/API references are zero. No fake post-effect constructor is
  used: the prefix stops before the first B4E470 child allocation.
- The original prefix uses exact original dimension leaves in a parent-reserved
  code band and ABI adapters to the complete holder/shared allocator providers.
  Its synthetic epilogue consumes the outgoing PUSH20, restores the native FS
  chain/saved registers and returns with five arguments. Original EH is an
  unreached trap. The probe's renderer is an explicit zeroed1D94h fixture with
  canonical resource services and a real HAL device, not application startup.
- No new repository tests. The report seals tested artifacts before integration
  and records a separate combined-build receipt. Game PE/full data images are
  not included in public evidence.

## Remaining work

Execute the complete initializer with the existing shader/material/geometry
domains and persistent companion blocks, resolve the +430 tail borrow in its
actual consumer, and continue B107F0's remaining providers and application
binding. Native exception/failure, later kernel/weight population, full resource
startup and gameplay remain open.
