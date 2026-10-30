# Independent Frame registration/selection review

**Accepted corrected library Source `81f3a296111b1337b2074124c065c66f90cca192` for the joined build.** One blocking policy issue in original Source `2ed82e8ceafef8d47d9e5bf1d4d91cb744777def` was fixed and independently verified. No other behavioral issue was found. This is static Source/listing/PE review; compilation, actual caller/provider composition, Native ABI, runtime and activation remain unproved.

The review covers all four production files: new `native_front_end_frame_selection.hpp/.cpp` and the existing `native_front_end_frame_publication.hpp/.cpp` change. Initial evidence metadata is `79644264387fa1cf4a977b155f6603e75cc03830`; correction is `81f3a296111b1337b2074124c065c66f90cca192`. The reviewer authored only this document and `reports/cc16_frontend_frame_native_selection_independent_review.json`, with no Source/address writes.

## Finding and correction

F1: original selection CPP's manual `noexcept`/`std::terminate` wrapper imposed process termination if GUI getter/AA5840 failed and the real raw41DD20/pool cleanup also threw. The Native EH evidence proves which header action runs; it does not establish that secondary failure must terminate. This was reported immediately to Root/author.

The corrected helper calls the genuine raw cleanup once without the manual `noexcept`/termination wrapping. Direct cleanup may throw; secondary Native FH3/SEH transport remains explicitly unproved. No cleanup/retry/acknowledgment or replacement failure policy was added. Independent byte comparison proves every Source byte outside that helper is identical to the reviewed original. Corrected selection CPP is8809 LF bytes, Git blob `82687829c7d3f226728ba91928b7887714188f88`, SHA256 `77f7fe7e707340912cffc66d1d07dd5c678af896f189cf476ec1cd6c514e4070`.

## Evidence and behavior checked

All saved listing lines were paged:280 for517ED0,347 for518250 and56 for4C1AC0. The listings contain276+345=621 registration/selection instruction rows, plus56 getter rows. Saved padding gaps are not invented executable work or body credit. Both new bodies contain no x87 arithmetic.

All4 production Source and15 input immutable Git-LF pins match, before and after correction; all6 saved-export raw-byte hashes match. The previously repaired closure-report entry has its immutable LF Git blob and historical CRLF disk hash recorded separately. This review independently reproduced the CRLF hash/byte count from that LF blob, proving the difference is only line endings. Corrected pins and historical reviewed pins remain distinct in the review report.

The installed original PE SHA256 was independently verified. Both FuncInfo records, unwind maps and all six25-byte action bodies were decoded directly from the authenticated PE. They confirm three actions per function, flags1/2/4, header offsets and clear-before-tail41DD20. This is static EH evidence, with no live Native unwinding or failure-transport validation. No new live Ghidra query was needed.

| Area | Independent result |
| --- | --- |
|517ED0 registration | Real raw4F71D0 precedes own+160=4.13/2/2 outer columns and5 inner sets preserve captured outer CURRENT names and each row's CURRENT own reload. Actual Page/null publication precedes normal name return. |
| Raw temporary strings | Actual8B headers are initialized only on reached nonnull-name arms. Registration shares its first backdrop/panel header and uses distinct title header; selection uses three headers. Real raw41DD40/current output data-length/memcpy precede cleanup arm; normal disarm precedes real pool return. |
| Request/commit homes | Initial DWORD comparison, initial commit BYTE, per-row retirement request reads, independent per-group acquisition request reads and final commit BYTE then request DWORD retain their separate epochs. No range clamp or narrowed bool/value copy. |
| Count1 retirement | Real GUI getter runs before SLOT reload and concrete wholeAA31F0 on the same Page lifetime provider. AA31F0 removes the first identical registry entry then dispatches current20/fresh04(flags1), without decrementing Page+4. |
| Other-count retirement | Capture Page, read CE2220 target at EACH reached call, then capture current profile/current00 at returned zero and use required throwing genuine dispatch. Clear only after returned retirement, including callback replacement. No retired Page read follows. |
| Acquisition | SLOT read precedes the unconditional CURRENT name read and branch, including an already-populated slot. Group request captures and later name reads preserve callbacks. AA5840 flags are(header,1,0) for registration and(header,1,1) for set acquisition. |
| Raw addresses/authority | Win32 unsigned DWORD arithmetic preserves13/2/2 slot/name formulas and wrapping request bits. No bank/default names/semantic receiver/codeword-to-callable cast or new owner is supplied. |
|4C1AC0 custody | Both public APIs share the existing genuine getter schedule. Four metadata-only noexcept notifications occur after allocation, after construction/before publication, after CURRENT+8 capture/before the second manager, and after captured free. Fast existing return adds no epoch; captured allocation/result/current registration identities remain separate. |

Metadata callback purity, stable preprepared receiver/operation records and genuine manager/publication reference identity remain actual caller obligations. The API cannot itself prove that an eventual callback obeys its metadata-only/no-allocation/no-reentry contract. The current00 Page dispatch remains a required throwing provider; this review does not substitute a canonical `noexcept` zero terminal or manufacture a fresh04 implementation.

## Limits

The used Startup frontend app, canonical/raw95 producers and current dispatch, same retained Page00/fresh04 services, whole Frame destructor/scalars and both drains, writer/apply/title/position/collector/actual caller migration, changed Text/nonempty layout/glyph recursion and reached Text/Icon/FrameBox factories remain required. Whole frontend readiness and activation remain false; Page guard remains false.

No build, test, probe, reconstructed/Original execution, game/process action, Ghidra mutation, CMake or ledger edit occurred in this review. The joined MSVC Win32 build remains Root's separate receipt. New Original/duplicate/vendor-body, ABI, runtime, gameplay and activation credit are zero.
