# Raw Dyn velocity and position integration (R151)

Addresses: **00C41550–00C41AC8** (1,401 bytes) and
**00C5B1B0–00C5BB29** (2,426 bytes). Both full bodies are reconstructed in
`src/native_dyn_world_integration.cpp`, with explicit C++ entry points in the
matching header. Names remain descriptive hypotheses. This packet advances the
raw world-step pipeline; it does not admit that pipeline into ordinary gameplay.

## Native contracts and arithmetic

Velocity takes the world in ESI and a stacked float dt, returning with RET 4.
Position uses EBX for the world with the same original stack contract. Both walk
the dynamic-body list at world+204h, sentinel world+208h, next body+84h, and skip
bodies with flag 10h. Position also returns immediately when world+290h is zero.
Existing world/body constructors provide these records; no substitute globals
or new physics storage are introduced.

Velocity applies force using inverse mass, optionally adds gravity (flag 4
disables gravity), clamps damping, computes the nine world-inertia words at
motion+60h..80h, and applies torque. Motion+B4h enables the constrained inertia
rewrite after angular velocity has already been updated. The original zero
products, retained x87 operands and double spills are preserved.

Position consumes both ordinary and solver correction velocities. It moves the
position, rotates the basis using native FSIN/FCOS, re-orthonormalizes through the
existing float-sqrt service, applies damping again, limits speeds, updates the
signed sleep countdown/flags, and clears force/torque and correction vectors.
It preserves the native threshold comparisons, including unordered branch
behavior, and the exact double constants at D7A220/D7A390 and float at D7A310.

One native asymmetry matters: on countdown expiry, C5BA86–C5BAB7 stores linear
X back unchanged, retains `X * 0.8999999761581421` on the x87 stack, and uses
that product to scale the other five velocity components. It is not a uniform
0.9 decay of all six components. The exact instruction audit and differential
fixture retain this behavior. D7A220 is the exact double 100; D7A310 is the
float angular threshold approximately 1e-5.

The prior semantic `rigid_body_integration.cpp` projection is separate. Its
introductory claim that these routines are SSE-free and all intermediates are
float32 does not describe the native bodies: the listing contains MOVSS/XORPS,
FCOMI/FCOMIP, double spills and x87 values retained across several operations.
The raw implementation preserves the actual schedule rather than relying on
equivalent-looking float expressions or host trigonometric functions.

## CRT binding and source ABI

The five position-stage sqrt calls consume existing 004011D0 (31 bytes) and
the shared reconstructed CRT ST0 sqrt service. The adapter preserves 8-byte
stack alignment and the original float store/reload. It receives the borrowed
`CameraAxesCrtAccess`, including the actual dispatch word and exception handler.
There is no default math policy or process-global context added by this module.

The private position kernel carries an extra context word, supplied from the
original caller frame at each sqrt call; its private RET is 8. Public wrappers
bind ESI/EBX and preserve the caller's nonvolatile registers. These source
interfaces are not certified drop-in binary replacements.

## Evidence and validation

- **3,878 bytes** match live Ghidra and the installed PE: 3,827 new body bytes,
  the existing 31-byte sqrt reference and 20 constant bytes. The installation
  is unchanged. All six direct CALL rows, including the reference boundary,
  are checked against live function ranges and instructions.
- A separate COFF audit compares **1,108 native instructions and 27 branch
  destinations**, exact constant bytes, all five context transfers and the
  sqrt adapter. Non-adapted instructions retain their exact encodings.
- Strict MSVC Win32 compilation and all three existing CTests pass.
- One focused local native/source fixture compares **3,072 world pairs and
  37,327,840 normalized bytes**. It covers 32 finite world configurations,
  12 x87 precision/rounding combinations, four MXCSR DAZ/FTZ combinations,
  both borrowed CRT dispatch values and two velocity/position cycles per pair.
  Cases include empty/multiple bodies, disabled integration, gravity exclusion,
  constrained inertia, zero/negative/positive dt, damping clamp, basis rotation,
  speed limits and sleep-countdown boundaries. Complete records are compared
  after each stage. x87 control is unchanged and its stack is empty on return.
- The non-bypass, reduced-precision sqrt path produces **31,048 type-8 CRT
  callback records per side**. The fixture compares operation, type, saved
  control, input and result bits using a controlled identity handler. It does
  not read the unary exception record's uninitialized second argument.
- The existing process lifecycle fixture now obtains derived inertia from
  velocity integration and sleep eligibility from position integration. It
  uses real process/world/body/SAP/contact/group producers and both solver
  modes on one worker (three batches, nine worker allocations). Position
  changes and accumulator clearing are observed. Complete native physics
  destruction, pool trim and actual atexit leave zero tracked allocations;
  the fixture explicitly closes 101 handles left by native cleanup.

The first broader math run stopped at the deliberately rejecting CRT callback:
a positive finite sqrt under 24-bit precision requested type-8 handling. This
was a fixture limitation. Capturing the real callback contract with an identity
handler allowed the full comparison to pass without changing either kernel.

## Limits and next work

The differential fixture exercises finite inputs and masked floating-point
exceptions; it is not exhaustive IEEE, unmasked hardware-fault or concurrent
mutation proof. Original CRT behavior is consumed through the supplied access;
the test identity handler is not a reconstruction of application error policy.
The bodies contain no original EH registration that this port removes, but
native fault unwinding and external binary ABI compatibility remain unproven.

The composed fixture still supplies cube geometry and transfers an actual SAP
pair into the scene vector. It no longer supplies a derived inertia matrix or
sets the sleep-eligibility flag. Collision-pass orchestration 00C57070 and world
substep 00C5BB30 remain to connect these stages automatically, followed by raw
game admission and gameplay validation. The serialized application launcher
was occupied by another harness during this packet; no competing run was started.

See `reports/native_dyn_world_integration_r151.json` for hashes, Ghidra receipts,
reference adaptations, object audit, probe results and integration evidence.
