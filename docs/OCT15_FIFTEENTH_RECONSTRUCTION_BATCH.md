# Fifteenth reconstruction batch

This batch adds eight complete ordinary Source entries, one complete8B EH
cleanup action with an explicit existing-message receiver, and three admitted
full-selection prefixes. Owned total:1,713bytes/611instructions. Descriptive
names are hypotheses; original STL/library/EH identities remain preserved.

| Packet | New scope | Owned bytes / starts |
| --- | --- | --- |
| Scalar iterator | whole5531F0 increment and556DC0 iterator erase | 778 /269 |
| Bit/sensitivity maps | full prefixes559280,559A90;whole55A430,55AAF0 | 358 /144 |
| Scalar map | full prefix444B10;whole445900 | 179 /72 |
| Scalar range | whole5572F0 including partial loop | 200 /76 |
| DWORD allocation | whole553260,556C20;explicit-message actionC6D790 | 198 /50 |

The iterator/range and allocation entries directly reuse completely matched
accepted services. Complete call-displacement/EH comparisons support those
bindings; emitted iterator/range COFF preserves the physical argument words
and restores any temporary EBP frame before tail transfer. The owning length
helper already cleans its temporary; its separate action is never applied twice.

Map cleanup retains actual RawPoolContext services, selected owner/head tests,
CURRENT-head reload/reset/free, ordered output publication,4then8clear and
defined EAX0. Getter failures retain earlier effects and stop later work. The
three prefixes have explicit stable full-selection preconditions. Their whole
ordinary partial erasers are outside the new scope. Historical actual APIs and
fixtures keep their existing contracts; no callback or noexcept bridge is added.

Primary review found no issues:10iterator checks,9bit/sensitivity checks,
7range checks,7allocation checks, plus12independent scalar-cleanup checks.
Independent complete PE comparisons support6iterator/helper pairs,2map pairs,
4range/helper pairs and5allocation/error/action pairs. Full native bodies,
original message/type/unwind metadata and current instruction starts provide
the remaining packet-specific provenance described in each component report.

MSVC Win32 Release and all3existing suites passed in8.00s at Source
`d1fd051ea1668bb471592736afbb42795ef6d80b`. The initial scalar/iterator/map
union also passed3/3 in8.05s. All93report transfer rows (81distinct sites,
12duplicates) and67Markdown claims pass, with0failed/unchecked/warnings.
No new tests were added. Twelve Ghidra annotation updates preserve prior
names/comments/preimages and save the existing bsp project. Forced snapshot
retains76771total/76329internal functions; affected exports are refreshed.

The report records component pins, reviewed physical/git-blob hashes,
independent receipts, tested Source and eventual Main integration. Later
concurrent Source additions are distinguished from the exact tested union.

These are reconstructed and build/static-checked Source interfaces. Current
CRT/CPP/RawCtx/private frames do not prove original ABI/flags/FH3/SEH/RTTI/
heap/fault/runtime or game behavior. Existing math native differential tests
cover older seeds. Device-record/pair cleanup and DWORD insertion/append work
continue as separate bounded packets; this batch does not mark them complete.

Main Source integration is 752c379e0f4d6e15c50a21074f69c34356c96335.
Source, headers, CMake, tools and scripts there exactly equal the tested union;
all10owned Source/header hashes were rechecked. The unrelated Main untracked
report and argon_v2_seg40.cpp are preserved. Publication uses one batch push
after this metadata receipt; later independent additions may follow.
