# Whole ordinary scene-property cloning (CC12)

`clone_native_scene_property_bag_008f41f0` reconstructs the whole 154-byte,
49-instruction bag producer; `clone_native_scene_property_record_008f4f60`
reconstructs the whole 854-byte, 274-instruction record dispatcher, including
all twelve tags. These are new ordinary C++ interfaces. Names are hypotheses.
The accompanying report records current validation and integration gates.

The bag producer allocates a real 0x114-byte canonical root and calls the
existing raw 61-byte initializer before traversing the source's real map with
`00480690` and `0047E480`. It captures each node's actual key before recursively
cloning that node's record and inserts through the full `008F33F0` wrapper.
Head insertion, bucket traversal, ordinal stamping, and owner publication
therefore remain observable. Source owner and ordinal counter are not copied.
An empty key uses a distinct retained const-zero `F89450` role appended to
`GameNativeStringProcess`; the Original writable zero is not assumed immutable
outside this bounded Source domain.

The record dispatcher reads the tag and rejects unsigned tags above 11 without
allocation. For a valid tag it allocates the real 0x38-byte record before any
arm payload read, invokes the exact existing raw constructor, then physically
rereads source ordinal `+34` and stores it in the returned record. Partial raw
constructor writes remain partial; the dispatcher does not initialize padding
or invent record ownership. Type 6 allocates its record before recursively
producing its actual child bag and attaching that child with `008EF780`.

Payload read order follows the whole listing: Type 4 reads `+0C` then `+28`;
Type 5 reads `+08`, `+18`, then `+1C`; Type 8 reads count `+24` before pointer
`+20`; Types 9/10/11 read pointer `+20` before calling `008EF7F0` for the count.
The current ordinary compiler stages the full copy flag after the count call,
while the Original stages that stack argument earlier. The value is still one;
Original stack staging and register identity are not claimed.

Type 1 uses a private naked integer-return helper with exact emitted bytes
`D9 41 0C 51 D9 1C 24 58 C3`: `FLD m32`, push, `FSTP m32`, pop, return. It
does not use an intermediate C++ float or reset floating-point state. Its
domain requires an empty x87 stack, all six exception masks already set,
valid PC 24/53/64 and any rounding mode, and no pending unmasked exception at
each conversion. Every binary32 input is permitted; hardware quieting and
sticky exception status are retained. Undefined condition codes, popped
register contents, and instruction/data/opcode environment addresses are not
oracles. The selected runtime probe is evidence for its selected cases, while
the complete helper inspection supports the instruction-level contract.

Both mutually recursive functions share one concrete context borrowing the
same actual canonical string and physical-pool processes as insertion and
retirement. Explicit `CC8A30` startup must have completed. Source maps must
have consistent counts and 64 buckets, finite live nodes, canonical owning
keys, and valid nonnull records. Keys are closed NUL-free ASCII, including
empty, in the current C locale. Ownership graphs are finite, unshared and
acyclic; constructor payload spans and text terminators must be valid. Type 8
has positive byte count; Types 9/10 use byte size `4*n`, with
`1 <= n <= 0x3FFFFFFF`; Type 11 uses `12*n`, with
`1 <= n <= 0x15555555`. Require successful canonical allocation/providers,
DF zero, sufficient stack, stable processes and pool membership, and no
concurrent mutation, trim, invalid aliasing, overflow or reentry.

The complete current strict Win32 object contains ten emitted functions:
1,117 section bytes, comprising 1,068 instruction bytes/323 instructions,
a 48-byte twelve-entry switch table and one `90` alignment byte. Whole
inspection accounts for every byte, relocation and normal control-flow
edge; the helper contains the only two FP instructions. Independent ownership
and FP reviewers accepted the source and whole object. The required worker
normal build passed its two configured tests; at that point the new clone TU
awaited the integrator's shared CMake registration. Strict standalone TU
compilation is recorded separately from registered-library integration.

The focused Source probe uses one actual mixed recursive graph with all twelve
tags, nested bags, an empty key, a bucket collision, nonzero ordinals, and real
owned text/array allocations. It checks the complete borrowed source spans,
distinct owning output spans, key traversal/head insertion, copied ordinals,
zero-ordinal stamping and child backlinks, then uses the same concrete lifetime
context for recursive cleanup. Six selected Type 1 hardware-reference cases
cover finite, signed zeros, subnormal, QNaN and SNaN inputs with selected PC/RC
controls. Fixture code alone sets and restores FP state. The report states
whether this probe has actually linked and run; this description alone is not
runtime evidence. Completion of cleanup is not an independent per-free trace
or a leak counter.

Original ECX/stack/RET, FS/SEH and failure unwind, root-null faults, OOM,
historical CRT, class/vtable dispatch, World/startup, and game behavior remain
outside this ordinary contract. Earlier empty-only fragments and their narrow
ABI qualifications remain unchanged. No Native profile is launched by this
packet. The Type 6 public storage comment can be widened to this producer only
after the complete populated Source producer and recursive chain have passed.
