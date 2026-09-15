# Native shadow-entry iteration

This packet supplies the complete AD7440 traversal and its two actual one-byte RET callees, AE2CA0 and AE0750: **three native bodies / 190 bytes**. It preserves all twelve validation calls, both entry calls, current range reads and captured iterator values. AD7360, AD72A0 and their missing providers remain separate work.

| Native entry / inclusive range | Coverage | Original ABI and source boundary |
|---|---|---|
| AD7440..AD74FB, 188 bytes / 69 instructions | Complete literal naked body | ECX actual owner, no public stack arguments, RET; source fastcall EDX is unused. |
| AE2CA0, 1 byte | Complete exact RET body | Entry receiver is not read. Separate private naked definition; explicit fixed call remains. |
| AE0750, 1 byte | Complete exact RET body | Entry receiver is not read. Separate private naked definition; explicit fixed call remains. |

`dispatch_native_entry_pass_00ad7440` receives the caller's actual owner address. It first addresses the descriptor at owner+64h, then owner+54h. Begin and end are current DWORD pointers at each descriptor+4/+8; entries are four-byte pointer words. The unused descriptor fields and complete class layout are not inferred. No `std::vector`, owner proxy or callback abstraction is introduced.

The first cursor is captured from begin and checked against current end. Each loop head captures end into EBX, validates the current begin against that captured end, performs the original owner/self checks, then compares cursor with captured end. Before dereference, owner and current-end bounds checks run. After the fixed entry call, another current-end check precedes cursor += 4 and the next head reload. Returning validation handlers can change storage; captured cursor/end remain captured while later reads stay current. The second descriptor is read only after the first loop completes. Preserve the apparently redundant self comparisons and the three-byte `8D 49 00` alignment LEA at AD74AD.

The two entry callees are proven full native RET instructions, not guessed no-op implementations. Their fixed calls remain in the naked body; traversal and checks are not optimized away. The native bodies contain no x87/SSE arithmetic, direct global access, allocation or retain operation. Actual readable/writable storage and lifetimes remain the caller's responsibility; native unsigned DWORD arithmetic and unchecked dereferences are preserved.

## Concrete validation service

Each BF6713 site calls a fixed noinline cdecl wrapper around the real SDK `_invalid_parameter_noinfo()`, following the established pattern in `native_singleton_vector_registration_wrappers.cpp:12`. The source-CRT handler can return, so the original continuation remains present. No unconditional throw, fatal, default-success or caller-supplied callback replaces the service. No new synchronization, owner registry or generic iterator framework is added.

Original BF6713 consists of five zero arguments to BF66EF, stack restoration and RET. The source wrapper uses the current source runtime's handler domain; original encoded handler global 0109DD64, BF66EF/Watson internals, service register identities and exception ABI remain unproved. Complete traversal-byte comparison therefore normalizes the twelve service CALL operands. It does not claim original game-CRT identity.

## Native caller and retained evidence

AD7A30 is the sole observed caller, at AD7B2B, when its current receiver+6 byte is zero. The already reviewed EG parent evidence distinguishes this call from the camera path: the parent's first camera argument backing is reused for an x87 spill after EDI capture, while its second incoming float is unused. This packet adds no AD7A30 implementation or camera behavior.

Accepted EK admission at `9cdfda75261de1e03c4d0da59006b1dbc035eb0a` compared all complete bodies against original PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` and guarded live Ghidra bytes. Native listings show zero gaps. The report pins that admission, complete native evidence, source contracts and original call sites. Source starts from published `76ebaeec3c740be11baf39b2c72015b0593e3920`; retained EK/EI artifacts remain unchanged. The only shared build registration change is an isolated startup append, as prescribed by COORDINATION.md 72–75.

## Validation status

Source is complete; primary source review and build are pending. Required generated proof is the complete 188-byte COFF body with exactly fourteen CALL operands normalized, exact one-byte RET bodies for both named entry helpers, and a concrete SDK relocation from the service wrapper. Every other byte, including branch distances and the alignment LEA, must match. The primary plans one coupled EL+EN build after accepting both candidates and their integration. Its exact-commit manifest must retain inputs before/after, verified seed, four libraries, the new scene object, changed `native_traceline_render.obj`, new `native_shadow_entry_iteration.obj` and log, with the actual existing CTest count.

No new tests or runtime fixture are proposed. Build and static instruction agreement do not establish original CRT, game, renderer, concurrency or exception/fault behavior. Ghidra annotations and ledger integration belong to the primary; this worker does not edit them.
