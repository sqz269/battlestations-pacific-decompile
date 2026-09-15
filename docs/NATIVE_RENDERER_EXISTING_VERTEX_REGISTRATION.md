# Native existing vertex registration

Addresses: 00B28A40.

The complete 73-byte B28A40 body removes one matching raw vertex-stream pointer from the actual renderer+1AAC array and appends the current original argument word. Its sole observed caller B4AA78 reads current F8D394 at B4AA71 and passes the same constructed stream. This entry itself reads no global renderer/context and introduces no retained ownership or secondary registration. The descriptive name is a hypothesis; existing Ghidra name/comments are retained in evidence.

The original ABI is ECX renderer, public stream DWORD at entry ESP+4, RET4. The new fastcall interface adds unused EDX to keep that public stack word in place; incidental EAX is not exposed because the current caller ignores it.

All 26 native instructions are preserved in the naked body. After saving ESI, it derives renderer+1AAC and passes **the address** of `[ESP+8]` to the fixed B25300 bridge. After removal, it reads current capacity/count and grows only when equal, using wrapped doubling and signed minimum1. The B22D10 bridge executes the existing real allocator/copy/free provider. The body then reloads current count/data, computes the DWORD destination, and skips the store only for a computed-null destination. It reads the current public stream word only after that check and increments current count after the possible aliasing store, including the null-destination path.

Both bridges call existing concrete C++ providers, not application callbacks. Each bridge's explicit unused EDX keeps its one stacked argument and RET4 convention. B25300 receives the actual public-word address, and B22D10 receives the computed capacity. No host array, allocation framework, retain, validation fallback or native cleanup frame is added.

The supported provider domain requires valid reached raw storage and the existing current-CRT malloc/new-handler/free and C++ exception contracts. Existing array aliases and current reads remain explicit; arbitrary aliases into compiler-generated bridge frames, original helper register identities, original CRT globals/hooks, OOM/SEH/unwind equivalence and gameplay are not established. Exceptions are not converted to success or repaired by new rollback.

EU retains complete ET native/live/PE evidence, all three native CALL rows (two internal and B4AA78), provider source pins, and the prior names/comments. The package supplies 73 native bytes and no credit for the existing B25300/B22D10 providers, B4A9B0 constructor, AE47E0 parent or its unresolved lifetime composition.

Source candidate validation is complete; compilation and generated-code inspection await the parent's single coupled ES+EU build. Expected emitted B28A40 is the full 73 bytes after exactly two CALL rel32 operands; both complete compiler bridges also require inspection. No standalone build, syntax executable, new test, probe or runtime validation was performed.
