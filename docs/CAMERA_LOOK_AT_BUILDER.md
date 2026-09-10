# Camera look-at matrix builder

`build_camera_look_at_00b63f10` reconstructs the complete 978-byte function
`00B63F10..00B642E1`. It accepts an actual destination matrix, eye/target/up
vectors, and the caller's `CameraAxesCrtAccess`, returning the destination by
reference. It does not construct or dispatch a camera owner. The descriptive
name is a hypothesis based on assembly and callers, not a recovered symbol.

## Native arguments and arithmetic

The original entry takes destination in ECX, eye in EDX, then a target pointer
and three up-vector float words on the stack. It returns destination in EAX
and executes `RET 10h`. The saved Ghidra signature currently says no arguments;
the pseudocode also makes pointer/integer interpretations of reused float
argument slots. Those signatures are not the ABI evidence.

The new typed wrapper pushes the up words with integer instructions. It adds
no x87 load/store or conversion before the native arithmetic. The internal
kernel preserves all 309 native instruction sites, original local/argument
offsets, x87 stack operations, SSE scalar subtraction, branches and output
stores. Its added trailing CRT pointer is at `ESP+50h` before each length call;
four added MOVs load EDX with it, and its return pops 14h instead of 10h.
Absolute constant operands and direct helper calls use reconstructed symbols.

1. Normalize the supplied up vector. A length comparison that is nonpositive
   or unordered chooses positive-zero reciprocal; no substitute up vector is
   installed. All component products retain the original float spill points.
2. Load and spill eye, subtract it from target component by component, then
   normalize that forward vector with the same comparison policy.
3. Compute the normalized forward/up dot product in the original x87 schedule
   and spill it to float. The nonpositive/unordered branch computes its
   magnitude with SSE `negative_zero - dot`; this is not replaced with `fabs`.
4. Perturb up only when that float value is ordered greater than the exact
   double `0.999` at `00D62BA0`. A strictly dominant absolute forward X adds
   signed forward X times the perturbation to up Z; a strictly dominant Y or
   Z adds the corresponding signed component times the perturbation to up X.
   Equal-component ties can leave up unchanged. The perturbation constant is
   the exact double value `0.10000000149011612` at `00D7A3A0`, the widened value
   of float `0.1`, not the usual double literal `0.1`.
5. Compute and normalize `right = up cross forward`, then compute and normalize
   `up = forward cross right`. There are exactly four length calls and two
   cross calls in the complete builder.
6. Write right into matrix words 0/4/8, final up into 1/5/9, and forward into
   2/6/10. Words 3/7/11 are positive zero and word 15 is positive one. Words
   12/13/14 are negated dot products of eye with the corresponding axis, each
   with its original intermediate float spill and subsequent `FCHS`.

No destination store precedes the fourth length call. Zero vectors and tied
parallel inputs may produce a degenerate matrix; preserving those native
branches is deliberate. The typed wrapper rejects a missing CRT binding before
reading any vector or storing output. It adds no geometric input checks.

## Shared vector and CRT boundary

`camera_vector_length_00419440` and `camera_vector_cross_004f9b30` now expose
the already reconstructed kernels from `system_camera_axes.cpp`. Their
instructions and arithmetic are unchanged. Existing camera-axis code uses
these same entrypoints, and their existing Ghidra names
`BSP_Vector3f_Length` / `BSP_Vector3f_Cross` remain correct.

Length retains ECX vector and ST0 float result and adds EDX CRT access. Cross
retains ECX destination, EDX left vector, stack right vector, `RET 4`, and EAX
destination. Cross stores its first output before some source reads; callers
must not infer arbitrary alias safety from the mathematical expression.

The existing CRT reconstruction in [SYSTEM_CAMERA_AXES.md](SYSTEM_CAMERA_AXES.md)
loads the actual borrowed mode slot at each native branch and calls the actual
`__87except` adapter when required. The caller supplies both live bindings;
there is no new process global, square-root implementation, or error policy.
The adapter may alter the result and saved control word. Full CRT errno,
matherr and SEH behavior belongs to that actual adapter, outside this builder.

## Evidence and validation

- Read-only guarded Ghidra CLI calls verified project `bsp`, existing
  `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` before each
  export/byte query. The full builder export and assembly were inspected.
- Twelve code/boundary spans and eight data spans matched installed executable
  bytes or native zero-fill exactly. They cover the full builder, both vector
  kernels, eight CRT routing kernels, and a five-byte `__87except` boundary
  preimage. The report records every extent, byte hash and operand relocation.
- One local fixture ran only these sparse copied spans, preserving native
  relative addresses. Other pages remained inaccessible and unused committed
  bytes were INT3. Twelve absolute operands were rebased; only the external
  `__87except` entry was redirected to the same controlled observation handler
  used by the host. No imports, PE entrypoint, whole executable mapping or
  unresolved gameplay call stubs were used.
- **96 complete matrices, 6,144 matching matrix bytes, zero failures.** Twelve
  focused input configurations cover ordinary geometry, each dominant-axis
  parallel branch, antiparallel Y, equal-component tie, either side of the
  threshold, coincident eye/target with zero up, signed zero/subnormal input,
  and signaling NaN with/without handler edits. Each ran with control words
  `027F`, `067F`, `0A7F`, `0E7F` and both CRT mode values. Matrix bytes, returned
  destination identity, x87 status (low 15 bits including TOP/condition flags),
  x87 control and full MXCSR matched. All input vector bytes remained unchanged.
- **94 matching CRT dispatch records.** The fixture compared operation, type,
  name text, argument/result bits, saved control word and call order. Every
  callback saw untouched destination preimage. A handler changed the result
  to double 0.25, changed the saved control word to `067F`, and set the actual
  mode slot to 1; native and host produced identical subsequent behavior.
  This verifies the binding and live reload, not the real CRT's downstream
  exception/errno effects. Missing-binding rejection preserved destination.
- The new builder and shared kernel source compiled in the focused fixture
  with MSVC Win32 C++17, `/fp:strict /O2 /W4 /WX`. Existing seed verification
  matched all eight seeds. `scripts/build.ps1` passed the repository build and
  both existing CTest checks. Shared CMake integration of the new source is
  reserved for the primary agent; the focused fixture compiled it explicitly.

No native camera constructor, `00B700E0` wrapper, virtual setters, raw object
lifetime, unmasked FP traps, arbitrary pointer aliasing, concurrent input/CRT
mutation, live rendering or gameplay validation is claimed. Ghidra annotations,
sharded ledgers and shared build metadata are unchanged by this worker.
