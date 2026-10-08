# Tag-9 four-byte array storage readiness

Tag 9 selects cell `008F52DC`, which points to `008F51AE`. The complete clone
arm is `[008F51AE,008F51FA)`, 76 bytes and 24 instructions. Its actual producer
call is **`008F51DE -> 008EF360`**; `008F51DB` is the preceding `PUSH EAX`.
The one inspected producer is complete `[008EF360,008EF3D4)`, 116 bytes and
36 instructions, with two `RET0C` paths.

The whole producer is suitable for a future qualified raw-storage Source
packet using genuine current allocation/copy/free providers. **Source 0,
ready Source 0, new Source packets 0.** This audit implements no C++ and runs
no compiler or target process. Worker and Root independent fixtures, the
main build and publication remain future admission steps.

## Scope and dispatch evidence

The frozen existing 45-byte clone entry uses the raw source tag in EAX and
`JMP [008F52B8 + tag*4]`, guarded by an unsigned comparison with `0B`.
There is no index adjustment. The previously witnessed tag-8 cell establishes
the same table base; `008F52B8 + 9*4 = 008F52DC`. No old clone-entry, tag-6,
tag-8 or type-8 body is re-queried.

| Fresh span | Size | SHA256 |
| --- | ---: | --- |
| Cell `008F52DC`, bytes `AE 51 8F 00` | 4 | `31c8dd34688b0a9faa12c3d4aa690d04f47ef878dab0bd7e29061b68549afe2e` |
| Arm `[008F51AE,008F51FA)` | 76 | `0856515378d5cccafc4934e298058a4d8a0a9448c5c6effcf695fd9468fa070c` |
| Producer `[008EF360,008EF3D4)` | 116 | `dce8fb8621af17a5791a0e80cca91982def8914d2f3a9e516ee1a2344f05b5e3` |

All 196 bytes match installed PE and live Ghidra before and after inspection.
All 24 arm and 36 producer instruction starts, mnemonics and operands agree
gaplessly. Producer prototype, comments and pseudocode, plus the selected
arm's enclosing metadata, are unchanged. Every batch verifies the configured
`C:/Users/sqz269/bsp.gpr`, actual program `/battlestationspacific.exe`, language
and image base. Installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

`008EF360` and the called helper `008EF7F0` were explicitly checked unleased;
only the producer was claimed and inspected. `008EF7F0` remains unexpanded.
The externally owned `008EF780` and `008F41F0` were neither queried nor
reclaimed. Native heap, class and World graphs were not expanded.

## Whole producer contract

The historical `BSP_SceneProperty_ConstructFloatArray` name is a hypothesis.
The machine body performs byte copying and integer count arithmetic; it
contains no floating-point load, conversion or numeric element operation.
The proposed API therefore describes **four-byte elements**:

```cpp
void* __fastcall construct_native_scene_property_record_type9_four_byte_array_storage_008ef360(
    void* actual_root_ecx, void* unused_edx,
    std::uint32_t element_count_bits, const void* actual_element_bytes,
    std::uint32_t copy_flag_bits);
```

At entry T, ECX is actual fresh, unowned, writable 56-byte root storage; EDX
is unused. Stack words `T+4`, `T+8`, `T+C` are count, data pointer and full flag
DWORD. Require positive `n <= 3FFFFFFFh`, an actual readable nonwrapping
`4*n`-byte span, disjoint root/input/active frame, stable input lifetime and
DF clear for current providers. Zero size, count wrap, invalid/overlapping
spans, allocation failure, reentry and unwind are outside this successful
qualified domain. The function makes no `noexcept` promise.

The exact ordering differs from the type-8 byte-count constructor:

1. Save ESI and EDI, then read the count from `[T+4]` before root stores.
2. Execute `ADD EDI,EDI` at `008EF368`; store phase `00CE89D4` and tag 9;
   execute the second `ADD EDI,EDI` at `008EF377`.
3. Compare the low flag byte at `[T+C]` with zero at `008EF379`, after the
   phase/tag writes and both additions. Zero `+18/+1C` and store the computed
   byte count at `+24` without changing the comparison flags.
4. Retain the input pointer or allocate and copy. In the copying branch,
   read the input argument after allocation returns and store the actual
   allocation at `+20` before memcpy.

Both additions are DWORD modulo operations. The future whole-body Source
must preserve them and their order; it must not replace them with signed C++
multiplication, add an overflow guard inside the body, or infer float meaning.

The constructor writes 29 bytes: `[00,08)`, `[18,28)`, byte `2C`, and
`[34,38)`. It preserves 27 bytes: `[08,18)`, `[28,2C)`, `[2D,34)`, including
the DWORD at `+30`. Fields `+18/+1C/+34` become zero, `+20` receives the actual
pointer, `+24` receives **byte count `4*n`**, and byte `+2C` becomes one in
both branches. The phase value is opaque storage; no class or virtual
dispatch is permitted by the raw contract.

A zero low flag byte retains the exact input pointer without provider calls.
The caller manages its lifetime; it is not a newly allocated child to free.
Nonzero low bytes allocate and copy exactly `4*n` bytes. That actual child is
observed while live, then freed once with the matched canonical free before
the root is disposed separately. Byte `+2C=1` is not an ownership discriminator.

The only direct calls are `008EF392 -> 00BF55BE` and
`008EF3A1 -> 00BF7680`. A future qualified binding changes only their rel32
operands `[51,55)` and `[66,70)`; all **108 other bytes** remain literal.

## Physical return and flag requirements

Both exits restore ESI/EDI, return root in EAX and execute `RET0C`. EBX/EBP
are locally untouched and follow genuine providers' normal ABI. On the
retaining branch, ECX remains root and EDX becomes the actual input pointer.
Final CMP flags satisfy `EFLAGS & 8D5 = 44`, including AF zero; the earlier
count-add flags are overwritten.

On the copying branch, the allocation size push remains at `T-12` after its
CDECL call returns. The three memcpy arguments reach `T-24`, and memcpy
enters at `T-28`. Its normal return leaves `T-24`; final `ADD ESP,10h`
computes `(T-24)+16=T-8` and defines all six arithmetic flags. Actual provider
ECX/EDX residuals are observed without guessed assertions. No blanket
DF/ES/x87/MXCSR preservation through providers is promised.

## Genuine caller transport and open boundaries

The selected arm allocates `38h` through `00BF681B` at `008F51B0`, preserves
the result in EDI and scratch, tests it, writes EH state 4, and branches to
the unexpanded `008F528A` null tail if needed. It loads source `+20`, pushes
flag 1 at `008F51D1`, pushes that data pointer at `008F51D3`, then calls
`008EF7F0` at `008F51D6` with ECX equal to the source. Thus the two future
producer arguments are already stacked during the helper call.

Let S be the original clone entry ESP. The arm starts at `S-24`; the helper
enters at `S-36`, with staged data and flag at its `+4/+8`. The caller requires
the helper to preserve those words on return, but its body/RET and returned
element semantics are unqueried. After the helper, `PUSH EAX` places the
producer count at `S-36`, and the actual producer CALL enters at `T=S-40`.
The inspected producer's `RET12` restores `S-24`.

The arm then copies source `+34` through returned EAX `+34`, restores EDI,
ESI and the saved FS link, adds `10h`, and returns. A future constructor
fixture supplies raw valid count/data/flag inputs; it must not fabricate a
native parent, owner, class instance, provider or ordinal fixture to execute
this larger clone path. Indexed caller `008F3510`, count helper `008EF7F0`,
private heap/copy interiors, null/default/EH tails and full clone ownership
remain named incomplete. They are outside the qualified standalone producer.

## Future implementation and evidence preservation

Current canonical source/header and the admitted provider-context report
match their earlier accepted hashes. That report has Source admission 1 and
completed independent/main checks. Type-8 Source admission is not used as a
dependency. The future producer includes its **own** genuine private noinline
CDECL size adapter forwarding `{object, actual_bytes, actual_bytes}` to the
unchanged canonical allocator, plus genuine standard x86 memcpy. The adapter
earns zero Native credit. A contextual virtual allocator wrapper and a
fabricated external reference to type-8's TU-local adapter are invalid substitutes.

The proposed packet is `cc12_scene_property_record_type9_four_byte_array_storage`.
It needs three fresh TUs: new whole constructor/own adapter, unchanged canonical
allocation/free, and a new probe. One worker fixture would execute Source and
bound Original once per branch: four actual 56-byte roots and two copied
children, six allocations/frees, guarded opaque input bytes, full roots and
live children. It must capture count/data/flag stack words before PUSHFD,
independently record R before pushes, Q before CALL and T at entry, and derive
copy flags from T. Ordinary callers, full COFF/linked bodies, map-owned helpers,
imports and actual mapped I386 heap/copy identities require fresh gates.
Root then independently reproduces the evidence and runs the full main build.
No old probe, phase or helper execution is reusable as new validation.

The graph contains 12 nodes and 11 edges, within the 24-item budget. The
baseline is `04392020cb3cf6b3a6432a470c3dc8ae1de3bf66`. The new `local/t9r`
family preserves all 5765 prior artifacts: tag-6's 143 files including its
manifest and 5622 earlier pins. Seven strict inputs, eight provider-context
inputs and 27 frozen copies are checked. Moving metadata is preserved as
snapshots. The report supplies the exact recursive manifest, bookends and
proposal. Only this document and its report are committed; no Root type-8
independent-family file is touched, and no startup/gameplay claim is made.
