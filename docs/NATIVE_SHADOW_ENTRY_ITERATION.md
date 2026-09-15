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

The initial source commit is `c44ccb401148204ba584ffa4cd3b249a739e8052`. The first coupled build at `cd637cb1e8e841b790d33abaceb2d5a061ae2c12` failed on EL's reserved `bound` assembler labels before tests. The complete inputs, evidence and build directory are archived and rehashed; the primary corrected exactly four label/reference tokens.

The follow-up at `07dd326abfdf78a6b80804036af2ce2e180841f6` passed the build and both existing CTests with all 2,643 tracked inputs plus the actual seed unchanged. Full EN object review matched the complete 188-byte traversal after exactly fourteen CALL operands, its concrete six-byte SDK IAT jump and whole-object library membership. Both private helpers instead emitted `C2 00 00` (`RET 0`), failing the promised one-byte `C3` identity. That successful build and its generated mismatch are separately archived, with no attempt history overwritten.

The correction changes only those two helper return statements to explicit `_emit 0c3h`. Exact tested source `ac3784a5f737fe6b5b384a06901ea033e39d5a5d` passed the fresh `en3` build and both existing CTests, with all 2,644 inputs and prior evidence unchanged. Complete generated review now verifies the 188-byte traversal after exactly fourteen CALL operands, both distinct one-byte `C3` sections with zero relocations, the six-byte SDK IAT jump, and exact whole-object membership in the current library. Both complete EL scene/traceline objects remain byte-identical to the earlier passing build. The primary independently confirmed these final checks.

The final manifest is `local/en3-final-build-stamp.json` (SHA256 `bbd562168782928d12d906d881374bb63f49cbf7c7880a302d08679ad8acfa20`); complete generated evidence is `local/en3-generated-review.json` (SHA256 `5b88ad776019baf2241f97aaed079e55cc69e1bb14cecd8858769e7ce3a81e42`). The report pins the current four libraries, three objects, all attempt commits, raw source, logs and archived failure evidence. This final documentation update changes no build inputs. Parent AD7A30 remains incomplete.

No new tests or runtime fixture are proposed. Build and static instruction agreement do not establish original CRT, game, renderer, concurrency or exception/fault behavior. Ghidra annotations and ledger integration belong to the primary; this worker does not edit them.
