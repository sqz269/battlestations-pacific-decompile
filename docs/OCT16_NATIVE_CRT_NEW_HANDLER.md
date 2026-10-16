# Actual __callnewh in the supplied decode-context domain

The complete 34-byte, 15-instruction library entry 00C055B1 is reconstructed in the supplied actual decode-context domain. The default MSVC Win32 build passed all three existing suites. All emitted CMake code, relocations and noncode sections were inspected. Concrete CRT/TLS/handler ownership remains open.

Keep the actual library name __callnewh. The provisional C++ name is invoke_native_crt_new_handler_00c055b1. Native cdecl takes one 32-bit size, uses plain RET/caller-pop 4 and returns exact 0/1. Source takes requested size plus the SAME existing NativeCrtPointerDecodeContext and the actual current encoded 0109DE44 reference, caller-pop 12.

Read the current encoded word and call the actual published decoder with that same context. Use its actual returned pointer; the decoder's outgoing first-word writeback is discarded. Null skips size access and handler invocation. Nonnull precedes an explicit volatile read of this NEW Source size parameter's own word, then actual int32 __cdecl(uint32) handler invocation. Any nonzero return, including negative values, becomes 1; zero becomes 0.

| Whole entry | Native site | Transfer |
| --- | --- | --- |
| 00C055B1 | 00C055B7 CALL 00C04FDE | actual decoder, same added context |
| 00C055B1 | 00C055C5 | indirect actual decoded cdecl size handler |

Native TEST EAX occurs before each POP ECX; POP preserves those condition flags. Source stack cleanup, flags/registers and parameter frames differ. The volatile Source access is not a public Native frame reader or a claim about Original outgoing argument-word aliases.

The actual CMake object is 915 bytes: one 45-byte/22-instruction public code section, one C04FDE REL32, and one decoded ECX indirect call. Encoded-pointer sampling is at +9; decoder CALL +0C; returned EAX is captured into ECX at +11 before ADD ESP,8 at +13. TEST/JNE at +16/+18 skips size on null. Only the nonnull arm loads this Source parameter at [EBP+8], +1C, then calls the actual handler at +20. NEG/SBB/NEG at +25/+27/+29 normalizes the result. Both returns are plain RET.

All 47 directive bytes, 204 CodeView/compiler metadata bytes and 32 .chks64 bytes were read. There are no code helpers, EH, BSS, runtime data, code padding or unclassified file gaps in this object. These are static emitted-Source checks, not Native execution or fault/ABI validation.

No injected callback, new global/default/CRT/heap/TLS/handler owner, std::new_handler(void) policy, errno, lock, retry, catch, cleanup, LastError wrapper or noexcept adapter is added. The inherited decoder/module-gate services and actual encoded-handler publication remain supplied/open. Original runtime, fault, FH3/SEH, private register/stack/outgoing-slot, binary ABI and gameplay parity are unproved.

The authorized preparation and build baseline is exact released Main 92a14c97874ee9b499f3296d0355ce27868de19a. Root relaxed the 1c pre-sync gate; all 31 relevant provider LF hashes match both published 92 and 1c. No unpublished metadata was consumed. After explicit foreign Main543 release, the queued registration gate was successfully claimed; one Core line was appended and snapshotted, then the gate released before build.

Before build, original TLS-getter 76aec and secure-move 2c879 objects were verified and copied into read-only content-addressed archives. local/oct16_native_crt_new_handler_prior_objects.json retains their historical cited paths and provenance. All 15 prior artifacts and Root 15/22 seals remain unchanged. Frozen preparation cba304b8 is a historical pre-build snapshot; it was not overwritten.

./scripts/build.ps1 exited 0: reconstructed_math, native_math_differential and tool_tests passed, total CTest 7.99 seconds. These suites do not execute this new handler path. No new tests, probes or Native execution were performed. Whole malloc, V6, SBH and private unwind Source are outside this packet.

The report retains Native bytes/current starts/calls, actual object/build hashes, 31 provider identities, distinct preliminary Root readiness/Source-only receipts, and explicit limits. Final Root frozen-pin acceptance remains pending; Ghidra has not been mutated.
