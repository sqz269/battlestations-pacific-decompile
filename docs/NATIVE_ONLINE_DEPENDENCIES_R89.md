# Raw online dependencies for the platform message pump

Addresses: 00737D60, 007F9290, 007F9340, 00A3E560, 00A3E600,
00A3E6A0, 00A3E700, 00A3EAE0, 00A3EBD0, 00A3ED10, 00A3ED60,
00A3EF20, 00A3F3E0, 00A3F440, 00A3F4A0, 00A3F500, 00A3FA70,
00A3FDE0, 00A3FF20, 00A40020, 00A40110, 00A40510, 00A409F0.
The notification9 helper covers only A401A9..A401CC within A40110.

## Scope and provenance

R89 integrates ten reviewed raw-storage modules from orch5 commit
`769bdc3e3`: notifications, sign-in, storage, storage requests, achievements,
sign-in UI, profile callback, notification leaves, dispatcher and pump.
All twenty source/header files are byte-identical to that predecessor.
Each source registers through the current `cmake/startup.cmake`.
The companion report records individual body bounds, original ABI,
descriptive names, prior reports, source hashes and current validation.

The native manager is borrowed actual 3F0-byte storage. Its constructor,
publication, destructor and complete platform/client ownership are not
provided here. The older semantic online interfaces remain separate.

## Recovered behavior

Sign-in refresh preserves the complete 128-byte SDK name output, including
the untouched tail on supported partial failure. It reloads state after
callbacks. Polling samples the current clock before reading the saved
timestamp, then rounds through float before an ordered x87 comparison
with one second. Callback exceptions retain the native preceding stores.

Storage requests use the selected user, current state and native payload
store order. Upload/download bodies preserve the actual overlap/result
fields, nine-byte payload, pending-progress paths and matching allocation
pair. Achievements retain wrapped SAR queue counts, saturated allocation,
returning CRT guards, first-match queue erasure and current field reloads.
The forced pending-batch release behavior is preserved without asserting
that a real asynchronous SDK can safely retire that storage.

Sign-in UI uses an explicit 28-byte caller scratch preimage. Its four wide
temporary strings return through the actual pool; choice/overlap outputs
live in the manager. Reset retains captured queue endpoints across a
returning invalid-parameter callback. Profile setters preserve overlapping
copies, display/base fallback and the 31-byte game mirror. The profile
callback reloads the published game independently before each setter.

The dispatcher captures the manager, creates a listener when needed and
processes current SDK outputs in order. Notification9 calls the captured
hook in CL, rereads the mutable parameter and writes captured manager+3E8.
Invite failure still copies the specified 54-byte output from a caller
preimage. Update branches preserve their string cleanup states and direct
process-exit behavior. The current client+28 slot must be a callable method;
the numeric original client implementation remains a separate dependency.

The pump runs dispatcher, UI, clock sample, current upload/download states,
achievements and a second sample into the same output. It rounds seconds
to float, initializes the timestamp on the first call, rejects unordered
comparisons and invokes callback20 only after an ordered delta above two
seconds. The final timestamp store overwrites callback mutations.

## Current validation

Strict MSVC Win32 build and all three existing CTests pass. Twenty-three
full bodies total 6,347 bytes and match live Ghidra and the original PE;
their exported listings have no missing bytes. Ten retained original-code
arrays and supporting table/EH/constant spans match fresh live/PE bytes.
The report verifies all 146 direct call rows; sixteen indirect calls are
recorded separately. Eight seed checks match. Guarded queries verify
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

Eight existing fixture programs link the current registered CMake library:

| Fixture | Current evidence |
| --- | --- |
| Notification9 | Four unchanged-original/source branch comparisons; raw owner and callback mutation |
| Achievements | Thirteen original/source cases with sixteen redirected child call sites |
| Sign-in reset/UI | Two original/source reset cases; source scratch-preimage, CRT and pool cleanup checks |
| Pump/dispatcher | Ten original/source x87 pump cases; source notification ordering, partial outputs and cleanup/replay |
| Sign-in, storage, requests, leaves | Source contract checks using fake SDK/registry/Shell providers |

Two isolated children execute actual `_exit(0)` after fake update providers.
No installer, account/network request or system update is performed.
The original pump calls five reconstructed children; original whole
dispatcher, sign-in, storage and UI bodies are not executed. Original
achievement/reset callees are redirected at documented fixture boundaries.

The notification fixture initially lacked its predecessor's fixed-address
linker flags and could not reserve its original global page. Restoring
those flags resolved the harness failure. The UI fixture needed current
Lua/zlib libraries. Initial logs are retained; no production code or fixture
assertion changed. No new repository tests were added.

## Limits and follow-up

These are explicit C++ interfaces, not original ABI/FH3/SEH replacements.
SDK name/localization domains, failed-QPC behavior, CRT identity, allocation
failure and actual asynchronous borrowed-text lifetime retain documented
boundaries. Complete raw manager/client construction and live SDK behavior
remain unproved. The application was not rerun because the actual compiler
and raw platform composition are still absent from main.

Next dependencies are the raw BECB20/BECCD0 platform load-message path,
sampler cache continuation/entry, descriptor samplers and the full material
compiler. Full post-effect construction, teardown, drawing and gameplay
remain open. See `reports/native_online_dependencies_r89.json` for frozen
tested and integrated artifacts and saved Ghidra annotation receipts.
