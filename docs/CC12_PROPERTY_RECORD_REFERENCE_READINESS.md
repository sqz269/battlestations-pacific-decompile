# CC12 reference-named property constructor readiness

`008EF2B0`, currently named `BSP_SceneProperty_ConstructReference`, is a bounded
63-byte / 19-instruction constructor with **one real CALL**. Its physical
storage contract is established, but it is **not independently ready** at the
frozen `883cdffe7` baseline: the required `00438E40` raw string-duplicate packet
still has `done=false` and awaits Root's independent admission. Existing Source
availability does not close that dependency. Source and ready-Source credit
remain zero; this audit created no implementation or probe.

Whole range: `[008EF2B0,008EF2EF)`, SHA-256
`a3f703ea58a3c6f7fab8ce6d36764b8b1445cd101e5a0236a493d45718138e15`.
The existing descriptive name and its semantic labels remain hypotheses.

The proposed future padded physical interface is:

```cpp
void* __fastcall construct_native_scene_property_record_type5_storage_008ef2b0(
    void* actual_record_ecx, void* unused_edx, const char* actual_text,
    std::uint32_t word_18_bits, std::uint32_t word_08_bits);
```

There is no `noexcept` promise. ECX supplies actual fresh/unowned writable
56-byte storage. At entry T, `[T+4]` is an actual borrowed text pointer or null,
`[T+8]` is the raw word stored at `+18`, and `[T+C]` is the raw word stored at
`+08`. `RET0C` consumes all three DWORDs and EAX returns the destination.

| Destination | Exact effect |
| --- | --- |
| `+00,+04` | DWORD opaque phase identity `00CE89D4`, then DWORD tag 5 |
| `+08,+18` | Third and second raw stack words, respectively |
| `+1C` | Full EAX result of the genuine `00438E40` call |
| `+20,+24,+34` | Zero DWORDs |
| `+2C` | Byte 1 |
| `[0C,18),[28,2C),[2D,34)` | Preserve all 23 bytes |

Exactly 33 bytes are written. The third word is loaded before PUSH ESI; the
second is loaded after that push. Phase/tag and both raw words are stored
before loading the first argument into ECX and calling the duplicate provider
at `008EF2D2`. The returned allocation pointer is stored before XOR EAX and
the remaining writes. A provider failure can therefore occur after partial
initialization; failure/unwind behavior is outside this proposed contract.

ESI is explicitly saved/restored. EBX, EDI and EBP are locally untouched and
depend on the real provider's normal ABI. ECX and EDX retain provider effects:
the recorded null-provider contract gives ECX=0 and unchanged incoming EDX;
nonnull residual values are unconstrained. Final constructor XOR flags are
CF=OF=SF=0 and ZF=PF=1; AF is undefined. Require DF clear for genuine current
CRT calls. There is no blanket DF, ES, FPU or MXCSR preservation claim for the
whole call, although the constructor itself has no FP instructions.

The existing provider contract permits null or a live, readable NUL-terminated
byte object with representable `n+1` and no address wrap. The input remains
borrowed and must be disjoint from the destination and active target frame.
Its pointer is not stored directly. The duplicate result retained at `+1C`
is null or an actual owned current-domain allocation. A future bounded fixture
must observe each child live and free it once through the matching canonical
free, separately from the root. This establishes no reference counting,
native class ownership, phase dispatch or World relationship.

One genuine clone witness was read. Entry `[008F4F60,008F4F8D)` retains the
source and checks its unsigned tag. Tag-5 cell `008F52CC` contains `008F502D`.
The selected arm `[008F502D,008F5075)` is 72 bytes / 23 instructions, SHA-256
`df8ce7253fe1cc9792951e17c2ce57d7b88b02e3ec76462908ad2d4a8c14a881`.
It requests `38h`, saves the returned allocation in EH scratch, sets state 0,
checks null, pushes source words `+08`, `+18`, then `+1C`, supplies the new
destination in ECX, and calls this constructor at `008F5059`. It subsequently
copies the source ordinal at `+34`. Full stack derivation is in the report.
The native allocator, EH handler, null/default branches and whole clone remain
unexpanded. No other constructor or class/world census was queried.

The graph totals **23/24**: 12 nodes and 11 edges, including named incomplete
boundaries and borrowed/returned storage. The duplicate node encapsulates its
separately recorded allocation/copy/free contract; its body and transitive
native closure were not queried or admitted here.

After Root admits the provider and explicitly registers a separate packet, a
minimal future family needs four fresh TUs: new constructor, admitted current
raw duplicate, unchanged canonical allocation/free, and a new ignored probe.
One Source and one current-domain-bound Original call on guarded nonempty text
would require two real roots plus two real duplicate results and four matching
frees. Ordinary and raw callers must preserve the three full argument words
and `RET12`; nonnull ECX/EDX must be recorded without guessed equality.

A relocated Original constructor must rebind exactly the CALL operand
`[35,39)` to the same genuine admitted provider, preserving all other **59**
bytes and the E8 opcode. It cannot be described as an unchanged 63-byte
Original or original-private-CRT execution. Fresh complete code/caller/cookie,
actual mapped I386 heap/copy providers and all consumed inputs require gates.
No old object, archive, mock result or accepted-family replay is appropriate.

Five PE/live byte ranges, all saved instruction starts, the complete candidate
listing/prototype/comments, and the caller listings passed bookends. All
3,560 prior pins remain unchanged, including sealed type-7/type-4 evidence and
stopped helper receipts. One unsupported CLI pagination argument was retained
as a failed read-only command; a supported query returned complete metadata.
There were zero builds, probes, target entries or Ghidra mutations.

Ignored `local/rfa/` contains the proposal, raw evidence, frozen provider
context, historical metadata, deliverable copies, readiness receipt and a
complete recursive manifest. The [report](../reports/cc12_property_record_reference_readiness.json)
records the precise prerequisite and future contract. Root owns admission,
registration, any later Source work and independent validation.
