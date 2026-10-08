# CC12 next property storage readiness: 008EF270

Read-only result: the candidate is an independently ready call-free raw storage
leaf after Root registration. Its actual tag is **7**. No Source/header,
build, probe, native execution or Ghidra mutation was performed; Source and
ready-Source credit remain zero.

The complete range `[008EF270,008EF2AF)` is 63 bytes / 19 instructions, with
zero CALLs. Its SHA-256 is
`9010f6007c93fa311d1e2d7f6c89a06aa802af3575be8e5c22edd26aa36d6872`.
Every saved Ghidra instruction start matches gap-free PE decoding. Full bytes,
prototype, comments and listing remained unchanged across the audit.

The proposed future physical interface is:

```cpp
void* __fastcall construct_native_scene_property_record_type7_storage_008ef270(
    void* actual_record_ecx, void* unused_edx,
    const void* actual_payload_words) noexcept;
```

ECX supplies actual fresh/unowned writable 56-byte destination storage. The
stack DWORD at entry `ESP+4` is an actual pointer to a borrowed readable
12-byte span. Input and destination must be disjoint and remain valid during
the call; the destination must also be disjoint from the active target frame.
Neither a C++ vector object nor a fabricated receiver is required.

EAX returns the destination, ECX becomes zero, and EDX retains the **second**
input DWORD (`input+4`). `RET4` removes the pointer argument. EBX, ESI, EDI,
EBP and DF are preserved. Final defined XOR flags are CF=OF=SF=0, ZF=PF=1;
AF is undefined. No numeric conversion, SSE, x87 or MXCSR operation occurs.

| Destination | Effect |
| --- | --- |
| `+00,+04` | DWORD opaque phase identity `00CE89D4`, then DWORD tag 7 |
| `+0C,+10,+14` | Three raw DWORDs loaded from input `+0,+4,+8` |
| `+18,+1C,+20,+24,+30,+34` | Zero DWORDs |
| `+2C` | Byte 1 |
| `[08,0C),[28,2C),[2D,30)` | Preserve all 11 bytes |

Exactly 45 bytes are written. Phase/tag stores precede the first pointed read;
each input load then precedes its corresponding output store. The last source
load uses ECX itself, after which XOR clears ECX. Preserve this exact order;
there is no snapshot-copy or overlapping-memory promise. Invalid input could
fault after phase/tag writes and is outside the valid-span contract.

The input pointer is not retained, allocated or freed. Existing documentation
calls tag 7 V3/Vector3, but the admitted proposal is raw inline word transport.
The caller retains both storage lifetimes. Native owning record/declaration,
float parser/arithmetic, phase dispatch and destruction remain outside scope.

One genuine caller was read: entry `[008F4F60,008F4F8D)` and tag-7 arm
`[008F5134,008F5168)`, 52 bytes / 17 instructions. Exact cell `008F52D4`
contains `008F5134`. The arm requests `38h` from `00BF681B`, checks the result,
forms the actual pointer `source+0C`, pushes it, places the returned destination
in ECX, and calls `008EF270` at `008F514C`. It subsequently copies the source
ordinal at `+34`. At leaf entry T, `[T+4]` is the pointer and `[T+8]` is saved
EDI; RET4 restores the caller's pre-push stack. The allocator body, null/default
branches, EH handler and whole clone remain unexpanded.

The graph counts 12 nodes plus 11 edges, **23/24**, including all named
incomplete boundaries, borrowed input storage and future canonical allocation
context. No other constructor or root census was queried.

The future packet should own the four type-7 files listed in the
[report](../reports/cc12_property_record_next5_readiness.json). Its minimal new
family should compile only the new constructor, unchanged current canonical
allocation/free TU and a new ignored probe. Use two actual 56-byte destinations,
two guarded real three-word inputs, one Source and one unchanged Original call,
and two matching frees. Check all destination/source bytes and guards, the
actual stack pointer argument, RET4, EAX/ECX/EDX, nonvolatiles, DF and defined
flags. Exact fresh COFF/linked bodies, current mapped I386 CRT providers and
the complete map-owned cookie helper must be gated before and after. No FP or
numeric suite, old object, old recipe execution or successful-phase replay is
needed or authorized by this readiness audit.

All 2,333 earlier artifact pins and frozen Source associations are preserved.
Ignored `local/n5a/` contains the full proposal, raw evidence, metadata copies,
readiness receipt and complete recursive manifest. Root separately owns
registration and any subsequent Source work, independent fixtures, main Win32
checks and locked Ghidra publication.
