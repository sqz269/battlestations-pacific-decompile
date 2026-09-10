# Render batch preparation

`00b51df0` now has a checked C++ projection in `instance_sort.cpp`. It queries
the original batch index's sort configuration, generates the recovered key for
enabled index zero, and invokes the existing native-order sort with the selected
comparator. Other enabled indices retain the signed-material/depth comparator.
This is a borrowed material-field interface, not a native object-layout overlay
or a full render-frame implementation.

Evidence was refreshed on 2026-09-10 from project `bsp`, saved project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Every live CLI
query verifies project/program/language/image base. Sixteen complete function
ranges match the installed PE, whose SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Addresses, complete range bytes, hashes, current annotation preimages, and
unapplied annotation proposals are in
`reports/render_batch_preparation_audit.json`.

## Original ABI and control flow

| Address | Original ABI | Established behavior |
| --- | --- | --- |
| `00b51df0` | ECX batch; stack batch index; RET 4; virtual +0C | Query configuration, optionally generate keys, choose comparator, sort |
| `00b1cb30` | ECX queue; stack index, byte-output, DWORD-output; RET 0C | Copy queue fields and return AL=1 |
| `00bf7456` | Input ST0; pops one x87 value; returns EDX:EAX; RET | Signed64 truncation helper; batch consumes only EAX |
| `00b51b00` | ECX left entry, EDX right entry; RET; AL predicate | Compare unsigned high DWORD, then unsigned low DWORD |
| `00b51ab0` | ECX left entry, EDX right entry; RET; AL predicate | Signed effect+B0 ascending, then entry+14 float descending |
| `00b1dce0` | ECX first slot, EDX one-past-last; stack ideal, comparator; RET 8 | Recovered insertion/partition/heap sort |
| `00bf7df0` | Four stack DWORDs comprising two signed/unsigned64 bit patterns; RET 10 | Existing CRT `__allmul`, low64 product |

`00b1cb30` copies the byte at `queue+4+index*8` first, then the DWORD at
`queue+8+index*8`. Its return is AL=1; the remainder of EAX comes from the copied
DWORD and is not a normalized Boolean. The C++ getter returns a checked Boolean
and preserves both copied values. A missing index fails without altering output;
the original native function does not check bounds.

The caller saves the original index in EDI before passing its stack index slot
as the getter's DWORD output. `00b51e26` tests EDI. Thus the copied DWORD never
selects the comparator. Any nonzero enable byte sorts; zero skips all entry and
count reads. The projected disabled path likewise accepts untouched malformed
entry pointers. Configuration lookup still happens first.

Index zero generates every key before sorting. All other indices skip key
generation and use `00b51ab0`, preserving previous key fields. No opaque or
transparent label is inferred for an index.

## Material field reads and key layout

The index-zero loop reads:

1. `entry+4 -> section`, then `section+20 -> material`.
2. Signed WORD `material+34`, `material+7C -> effect`, and DWORD `effect+B0`.
3. Only when the signed count is positive, pointer slot zero at `material+10`.
   Only when that pointer is nonnull, DWORD `texture+20`.
4. BYTE `effect+C0`, then float `entry+14`, then writes the key at `entry+20/+24`.

`CMP word [ECX+34],0` and `JLE` at `00b51e4c/5f` establish the signed16 count.
Negative counts do not touch slot zero. A zero/nonpositive count or null slot
contributes texture value zero. `MOVZX byte [EBX+C0]` establishes the unsigned
byte. B0 is read as a DWORD and masked to its low six bits for this key, while
the other comparator uses the entire signed DWORD.

With all arithmetic below unsigned:

```text
texture = count34 > 0 AND slot0 != null ? texture20 : 0
prefix = ((uint32(effectB0) & 0x3f) * 0x1000 + (texture & 0xfff))
         * 0x100 + uint8(effectC0)
key = (uint64(prefix) << 37) + uint64(depth_helper_EAX)
```

The second `__allmul` operand is high DWORD `0x20`, low DWORD zero, which is
`2^37`, not `0x20` and not `2^32`. The prefix has 26 bits. Key bits 57..62 are
effect B0's six bits, 45..56 are texture20's twelve bits, 37..44 are effect C0,
32..36 remain zero, and 0..31 contain the depth word. Bit63 is zero for newly
generated keys. `XOR EDX,EDX` immediately after the depth helper means negative
depths use the low DWORD modulo `2^32`; there is no sign extension into the key.
For example, depth `-1.75` contributes `0xffffffff`, not a signed64 `-1`.

`RenderBatchMaterialKeySource` is an explicit adapter to the actual retained
material/effect/texture objects. Native code has direct field reads and no
callback here. The adapter must obey the conditional texture access and cannot
change entry pointers, storage, count, bindings, depth, or configuration. Native
code reloads the batch pointer/count at `00b51ec2/c9` and reads the next array slot
at `00b51e40`; the C++ loop also indexes the current slot/count each iteration,
within the stable-storage contract. Concurrent mutation and adapter-induced
mutation are outside the reconstruction's supported domain.

The adapter reads a material projection for each entry before that entry's key
is written. Duplicate entry pointers are allowed and cause repeated reads and
writes. A failed adapter or invalid entry retains prior key writes and does not
start the sort. Native does not have this error channel. Key generation does not
validate or repair the caller's material ownership; the actual owning queue,
retained clones, and assignments of effect/texture fields remain integration
responsibilities.

## Exact batch-depth numerical result

Pseudocode loses the x87 input to `00bf7456`; its assembly establishes the
conversion. `00b51eac` loads the binary32 depth into ST0. The helper duplicates
ST0, stores its sign as a float, uses `FISTP qword` under the current x87 rounding
mode, reloads that integer with `FILD`, and subtracts it from the original value.
The residual's sign/nonzero bits drive ADC/SBB corrections toward zero. The
zero/`0x8000000000000000` shortcut at `00bf74b5..c5` handles zero and the x87
integer-indefinite result. The helper leaves the caller's control word unchanged.

For binary32 inputs and masked x87 exceptions, its EAX result is:

- Finite values whose truncation fits signed64: low32 of truncation toward zero.
- NaN, infinity, and signed64 overflow: zero, from integer-indefinite's low DWORD.
- Exactly `-2^63`: a valid signed64 result whose low DWORD is also zero.
- Signed zero, subnormals, and magnitudes below one: zero.

The implementation extracts binary32 sign, exponent, and significand with
`memcpy`, then shifts unsigned integers. It performs no potentially overflowing
float-to-integer cast. Exponents below127 produce zero. Exponents at least190
(magnitude at least `2^63`, including NaN/infinity) also produce zero. For the
remaining domain it keeps the integral significand bits modulo `2^32` and
negates modulo `2^32` for negative input. Exponents at least55 after debiasing
already imply 32 trailing integer zeros and avoid oversized shifts.

This is the batch caller's numeric EAX projection. It does not emulate x87
exception flags, precision/inexact flags, unmasked traps, or arbitrary
extended-precision ST0 values. In particular, a native unmasked invalid exception
can interrupt the conversion instead of returning a key. The C++ function does
not change floating-point control state or raise that emulated exception.

## Sort reuse and validation

The previously recovered helpers now carry a comparator parameter through every
comparison and recursive call. Rotation, insertion cutoff32, median/ninther
threshold42, equal-band swaps, ideal-budget shrinking, right recursion on equal
side sizes, and heap child tie selection are unchanged. The public category-one
API and its finite-depth preflight remain intact. See
`docs/INSTANCE_CATEGORY_SORT.md` for the ten helper ABIs and assembly evidence.
Index-zero sorting uses only the generated unsigned key and accepts every
binary32 depth under the numeric contract above. Successful sort/preparation
allocates no internal pointer or key storage; adapter allocations are its own
responsibility. Configuration and count validation are host checks.

Validation performed in this worktree:

- `scripts/build.ps1`: MSVC Win32 Release build and existing CTest passed.
- The unchanged isolated category-one corpus was copied from the instance-upload
  worktree and recompiled against this modified source with `/W4 /WX /fp:strict`:
  90 native runs, 30 forced heaps, and 5,652 pointer positions matched exactly;
  the finite-depth rejection guard passed. Both copied input files retain their
  original SHA256 hashes.
- One focused native batch probe ran 4,141 binary32 bit patterns under all four
  rounding modes and all three valid precision modes: 49,692 EAX comparisons
  matched. Inputs include signed zero/subnormal, fractional boundaries, powers
  near32/64-bit conversion limits, maximum finite values, infinities, quiet and
  signaling NaNs, and a fixed seeded sample of bit patterns.
- That probe ran the complete original `00b51df0` on synthetic native object
  layouts for indices0/1/2 and counts0,1,32,33,41,42,65,257: 24 runs, 1,413 exact
  pointer positions and per-record key checks. Five forced unsigned-key heap
  runs also matched. Duplicate pointers, signed16 count extremes, null slot
  zero, nonpositive counts with deliberately invalid nonnull texture pointers,
  original-index versus copied-DWORD selection, noncanonical true bytes,
  disabled malformed entries, adapter read order, and partial failure passed.

The native probe runs verified code ranges in an isolated 32-bit process. It
preserves relative offsets, adjusts the two absolute comparator PUSH operands
for relocation, and adapts the singleton CALL to the synthetic queue. No other
instruction or function body is changed. The original first attempt exposed
those absolute comparator operands; after relocation repair the final corpus
passed. The original game process and installation were untouched.

The audit records local probe sources, generated byte reference, executables,
results, and hashes so the integrator can rerun and preserve the small probe.
Those `local/` artifacts are ignored; this packet commits only the header,
implementation, this document, and audit. Ghidra and shared ledgers were not
mutated by the worker. Descriptive application names remain hypotheses; the
existing CRT helper name is retained. Reconstruction, build, and native
synthetic differential checks are established. Original ABI compatibility,
full-frame execution, and game validation are not established.
