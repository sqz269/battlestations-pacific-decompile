#pragma once

namespace bsp {
// Complete B28A40[73]. Original ECX renderer, public stream DWORD at S+4,
// RET4; the current caller ignores incidental EAX. Unused EDX preserves S+4.
// On the actual renderer+1AAC array: remove one matching pointer using the
// ADDRESS of that public word, then append its current value. No retain.
// Equality-only growth uses wrapped doubling and signed minimum1. Reload
// data/count after reserve; skip only a computed-null destination, then
// increment CURRENT count even on that path. No snapshot of the stream word.
//
// Fixed bridges use the existing B25300 and B22D10 source providers and their
// actual current-CRT allocation/free domain. Reached storage must be valid;
// arbitrary aliases into compiler bridge frames, original helper register
// identities, CRT hooks/OOM/SEH behavior and game integration are not proved.
// No new registry, callback provider, cleanup frame or secondary registration.
// AE47E0 and its classification/construction/lifetime closure remain separate.
void __fastcall register_native_renderer_existing_vertex_00b28a40(
    void* actual_renderer, void* unused_edx, void* actual_stream);
} // namespace bsp
