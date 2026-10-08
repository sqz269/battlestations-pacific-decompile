# CC12 remaining property storage readiness: 008EF2F0

The selected whole constructor is independently ready for a future qualified
raw-storage Source packet using the admitted current allocator and standard
copy boundary. It stores tag **8** and either retains an actual input pointer
or allocates and copies the requested bytes. This audit has **Source 0 and
ready-Source 0**: no C++, build, probe, native execution or Ghidra mutation.
Root registration and independent implementation validation remain pending.

## Whole native body

`[008EF2F0,008EF360)` is exactly 112 bytes / 34 instructions / three basic
blocks. Native SHA256 is
`4c3786af3a642703dc38315ae63d9dee0468552f47a03bc1405fc0e90ab5c928`.
All instruction starts match gap-free PE decoding and the saved Ghidra listing.
The complete bytes, name, old prototype and comments stayed unchanged.
The existing descriptive name `BSP_SceneProperty_ConstructByteArray` remains
a hypothesis. The phase word `00CE89D4` is an opaque literal, not a Source vtable.

Only two genuine CALL operands may change in a future qualified Source body:
`008EF31E -> 00BF55BE`, operand `[47,51)`, and
`008EF32D -> 00BF7680`, operand `[62,66)`. All other **104 bytes** remain literal.
No prefix clipping, synthetic receiver or fake provider is proposed.

```cpp
void* __fastcall construct_native_scene_property_record_type8_byte_array_storage_008ef2f0(
    void* actual_root_ecx, void* unused_edx, const void* actual_bytes,
    std::uint32_t byte_count, std::uint32_t copy_flag_bits);
```

There is no `noexcept` promise. At entry ESP=T, ECX is the real root, EDX is
unused, and `[T+4]/[T+8]/[T+C]` are the actual pointer/count/copy-flag DWORDs.
The very first instruction compares only the **low byte** of `[T+C]` with zero.
High 24 bits are ignored. Both exits return root in EAX and execute `RET 0C`.
ESI/EDI are saved locally; EBX/EBP follow normal provider ABI in the copy branch.

## Exact stores and order

| Root location | Width | Result |
|---|---:|---|
| `+00` | 4 | Opaque literal `00CE89D4` |
| `+04` | 4 | Literal tag 8 |
| `+18`, `+1C` | 4 each | Zero |
| `+24` | 4 | Actual byte count |
| `+20` | 4 | Exact retained input pointer or actual new copied child |
| `+34` | 4 | Zero |
| `+2C` | 1 | One, in both branches |

Exactly **29 bytes** are written; **27** are preserved: `[08,18)`, `[28,2C)`
and `[2D,34)`. The DWORD at `+30` is preserved. Byte `+2C` does not distinguish
these branches and establishes no ownership meaning.

The copy-control byte is read before any store. Phase/tag stores precede the
byte-count load. After zeroing `+18/+1C` and storing count at `+24`, JZ uses
the original comparison flags, preserved through all intervening PUSH/MOVs.
The copy branch allocates before loading the input pointer stack word and
stores the actual allocated pointer at `+20` before calling memcpy. The
retaining branch loads the input pointer into EDX and stores it at `+20`.
Both zero `+34`, write byte 1 at `+2C`, restore registers and return.
Do not move argument reads earlier or introduce overlap/snapshot-copy promises.

## Branch-specific ABI and lifetime

For low-byte zero, ECX remains root and EDX becomes the actual input pointer.
No pointed payload bytes are read and no provider is called. The retained
comparison sets all six arithmetic flags: `EFLAGS & 8D5 == 44`, including
**AF=0**. This is a CMP result, not the earlier constructors' undefined-XOR-AF
case. The branch locally leaves DF, ES, FPU and MXCSR untouched.

For low-byte nonzero, the allocator is CDECL with one actual size argument.
It returns with ESP=T-12, leaving that size argument in place. Three further
pushes put memcpy's destination/source/count at its entry `M+4/M+8/M+C`, where
M=T-28. The old allocator argument remains at `M+10`. After memcpy returns,
`ADD ESP,10h` changes T-24 to T-8; pops and `RET12` then restore the caller.
ECX/EDX are real provider residuals and must not be assigned guessed values.
Final arithmetic flags come from that actual stack ADD. Derive CF/PF/AF/ZF/SF/OF
from `(T-24)+16` with mask `8D5`; there is no fixed final XOR flag pattern.
Require DF clear for the common current-CRT contract. No whole-call ES/FPU/
MXCSR or blanket DF preservation is promised.

The qualified contract supplies a fresh unowned writable 56-byte root and an
actual stable, readable **positive-length** byte span, with nonwrapping ranges.
Root, span and active frame are disjoint. Payload bytes have no NUL, numeric
or floating-point interpretation. Zero size, invalid/overlapping spans and
failure/reentry/EH are outside this proposal.

The retaining branch performs pointer storage only. Under this raw contract,
the input stays caller-managed and live while the retained pointer is used;
there is no newly allocated child to free. No native class ownership transfer
is inferred. The copying branch yields a real current-domain owned child:
observe it while live, then free it exactly once with unchanged
`singleton_lifetime_free` before disposing its root. Never dispatch the phase
or invoke a property destructor on these raw fixtures.

## One genuine caller and providers

Entry `[008F4F60,008F4F8D)` is 45 bytes / 13 instructions. Tag-8 cell `008F52D8`
contains `008F5168`; arm `[008F5168,008F51AE)` is 70 bytes / 22 instructions,
SHA256 `d5374472e122428d91b2bd7861a1ffe3cb1447eec25b6b4d8f45583a221472c1`.
It genuinely requests `38h` bytes from `00BF681B`, tests the result and preserves
that TEST through the state-3 store before the null branch. It loads count from
source `+24` and pointer from source `+20`, pushes `1`, count and pointer, sets
ECX to the allocated root, and calls the constructor at **008F5192**.
`008F518C` is the `PUSH 1` instruction mentioned by the old comment.
After `RET12`, it overwrites root `+34` with the source ordinal, restores the
saved FS chain and registers, and returns. Caller final flags/registers are
therefore distinct from the constructor return contract.

The current canonical allocation/free source and the physical duplicate57
admission already establish genuine current malloc/new-handler and standard
VCRUNTIME memcpy contracts. The future constructor needs a real private
noinline CDECL size adapter to the unchanged canonical service, with native
and host byte counts equal. That Source-only adapter gets zero Native credit.
The ledger's `NativeMpkgRuntimeServices::allocate_00bf55be` is a contextual
virtual service and cannot be substituted as a physical CDECL entry.

Original `00BF55BE/00BF681B`, private `00BF7680` and its named `00C0C82B`
dependency remain unexpanded private boundaries. The phase handler, clone
EH/null/default and whole class/clone are also unexpanded. The graph explicitly
includes them: 12 nodes plus 12 edges, **24/24**. Existing provider contracts
close the qualified current domain; they do not admit private CRT or EH bodies.

## Future packet and sealed audit

Proposed packet: `cc12_scene_property_record_type8_byte_array_storage`, owning
only the four paths listed in the report. Its minimal fresh family uses three
TUs: constructor plus real size adapter, unchanged canonical allocation/free,
and a new ignored probe. Four raw calls cover Source/qualified Original for
both branches; four real roots plus two real copied children require six
allocations/frees. Guarded inputs stay caller-managed. Exercise high flag bits
with low byte zero and distinct nonzero low bytes, capture all three stack
DWORDs before PUSHFD, and verify branch-specific registers/flags and all bytes.
Ordinary genuine compiler callers are statically gated. Rebind only the two
real CALL operands in full Original112. Whole code, helper/import ownership,
actual mapped I386 heap/copy provider identities and all input/old artifact
pins require fresh before/after gates. No old accepted process may be replayed.

Ignored `local/rma` is sealed: **297 artifacts plus manifest**, SHA256
`6abe2739325b9e347d16c5b050ca6e821e3a23a61f96d73ce3722e4d195097eb`.
All 4,640 prior artifacts, including completed type5's 363 files, stayed exact.
There are 13 frozen audit inputs, 13 provider inputs and 23 context copies.
Eighteen read-only live batches checked the configured project/program first.
All candidate and caller bytes/instruction starts and candidate metadata passed
bookends. One discovery-only unsupported CLI option was preserved; all actual
audit queries/helpers passed. No C++/compile/probe/native/Ghidra-write occurred.
The [report](../reports/cc12_property_record_remaining_readiness.json) carries
complete byte/instruction evidence, stack equations, source pins and receipts.
