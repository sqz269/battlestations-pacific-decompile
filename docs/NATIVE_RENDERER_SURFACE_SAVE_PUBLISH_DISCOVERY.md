# Renderer surface-save publication discovery

This read-only pass establishes complete `B5E490[107]`, `B23C50[235]`, and the
reached `B24DC0[9]` field getter. The primary approved these 351 bytes as the next
three-entry source packet. This commit contains discovery only.

The fresh guarded capture matches the original PE for 21 spans / 2,187 bytes
including overlaps (1,978 unique bytes); 363 complete code instructions are
decoded. The 869-byte CRT copy body includes jump tables and is deliberately
not reported as an uninterrupted instruction stream. The audit embeds all
bytes, hashes, target identity, imports, and the exact proposal.

## Original interfaces and storage

Both parent entries receive a raw owner in ECX and an eight-byte string-header
pointer on the stack, return with RET4, and provide no semantic result. Neither
has an EH registration or unwind cleanup. B24DC0 reads the current DWORD at
renderer+197C and returns with RET4, ignoring its stack scalar. Its ownership
meaning is not inferred.

The worker needs its actual 54h-byte prefix. Capture accesses the embedded
worker at renderer+1D2C, requiring at least renderer[0,1D80); this is an accessed
prefix, not a complete allocation-size claim. The current renderer profile
must be the actual D5F0A8 identity installed by B3243B. The borrowed profile
window must include every byte in [D5F0A8,D5F1D4), exactly 12Ch bytes. This is
the full required window, not an assertion about the full vtable extent.
Its +110 DWORD maps to B23C50 and its reached +128 DWORD maps to B24DC0.

## Publication and exception boundary

B5E490 enters the captured owner+48 lock and increments that lock's depth.
Only then does it read the producer index and capture the destination header.
Exact source/destination header identity skips the resize and copy. Otherwise
the source length is captured for full 41DD40 with preserve=true. After that
provider returns, the current source length gates a copy whose size is the
CURRENT DESTINATION length. Source data is read next, then destination data.

The copy supports overlapping buffers: BF7680 explicitly chooses backward
copy when required. The proposed source follows the existing string-family
host boundary: memmove for valid nonwrapping ranges and omission of a zero-byte
copy. The original zero paths return without accessing source/destination
data. This does not reconstruct the generic CRT's ISA dispatch, mutable speed
global, partial hardware faults, or incidental registers. Persistent-header
copy overlap differs from fresh-allocation record/list copies.

The destination header remains captured across callbacks. The final producer
is reread, incremented with DWORD wrap, and reduced by CDQ/IDIV5 (signed
remainder). The current lock is reread for decrement/leave. No exception
cleanup is added: failure may retain a held lock and current partial changes.

## Capture and COM schedule

Zero source length returns before ANY renderer/context memory read. Nonzero
captures current device1A10 and calls full B5E380 on embedded worker1D2C.
It captures the current renderer profile before publishing the result to
1D24, then reads selector+128 after publication. The full field getter runs
with ignored scalar0; current wrapper+2C is read before current destination.
The exact named import receives
`D3DXLoadSurfaceFromSurface(dst,0,0,src,0,0,1,0)`.

Each GetDesc, LockRect, and UnlockRect reloads the current surface and actual
COM table at the original point. The locked pointer is captured once. The
alpha loop walks contiguous DWORDs, ignores Pitch, and reloads the wrapped
Height*Width after every store, with an unsigned index comparison. Its initial
TEST/JBE skips zero only. At the original stack base after saved EBX/ESI/EDI,
description is +14h, Width +2Ch, Height +30h, locked Pitch +Ch and pBits +10h.
These output buffers begin uninitialized. A final, newly reloaded GetDesc
after UnlockRect is retained before publisher invocation. HRESULTs are
ignored; no surface/backbuffer release, rollback or validation is inferred.

## Ready providers and approved source boundary

Full B5E380 and its real thread/queue/COM chain are in f99fc486. Full 41DD40
is supplied through the actual owning pooled-string adapter and its canonical
pool publication/gate/lifetime. The raw +128 getter is the only new reached
native leaf. C2DFD4 is the concrete import forwarder to CE240C, named
`d3dx9_40.dll!D3DXLoadSurfaceFromSurface`. The installed x86 module exports it
as ordinal191 at RVA28C044; this was checked without loading or executing it.

The approved four-file packet uses an explicit fixed EDX context borrowing
the actual pooled-string adapter, a concrete named import object, and the
full required profile window. The import borrows the caller-owned actual
d3dx9_40 module, resolves that exact name, and has no DLL fallback or callable
setter. Existing application save binding must be installed through every
thread return. Context, native caller ABI and original SEH boundaries remain
explicit. Original image/project state and prior seals were not changed.


Primary verified all 64 sealed worker pins, 0 additional report pins, and freshly reread all 21 guarded spans (2,187 bytes). Review confirms the 300-byte borrowed profile window and overlapping-copy contract. The three-entry source packet is approved separately; this discovery executes no source or original body. Immutable evidence: `local/renderer_surface_save_publish_discovery_primary/`.
