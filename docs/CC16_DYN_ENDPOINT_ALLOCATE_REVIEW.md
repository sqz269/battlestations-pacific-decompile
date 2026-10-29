# Dyn endpoint allocator: independent bounded review

**Pass for normal-body source and the actual initializer contract; no blocking
finding.** Candidate `28d67fc121620e3c1770f7f1a4463ee838854953` matches the complete
207-instruction, 711-byte `00C55A60..00C55D26` listing. Root's actual initializer
header/source was used, and the emitted constructor dependency matches its
real definition. The small root `00C5A850` addition also passes the bounded
listing/source review below. This review adds zero body or runtime credit.

Review worktree base is accepted `5f686b664`; root's final private source batch
is `77d9e3203`. Only this document and its JSON report are owned review outputs.
No production source, CMake, shared metadata, Ghidra or installed file changed.

## Native evidence and ABI

Queries used the verifying BSP client for project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`. Pseudocode was inspected first;
all 207 assembly rows were then read, including the final six epilogue rows.
All 711 live bytes equal the installed PE bytes. Body SHA-256 is
`8f96d8eaf4f36fcb710d8f5bf1c86cf4d6ad36da2f7ecccd5e1ed3a0bcc5ea60`;
installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Native ECX is manager, copied to ESI at `C55A81`. Stack arguments are uint32
token, six-word bounds pointer and the low byte of the third word. EAX returns
the captured endpoint. EBX/EBP/ESI/EDI are preserved, and `C55D24` is `RET0Ch`.
The public cdecl function has an additional explicit CRT reference and supplies
new C++ transport; it is not a binary ABI adapter. Descriptive names remain
hypotheses.

## Pool, vector and callback epochs

| Selected flag | Free head | Page data/count/capacity | Live count | Sentinel / tail |
|---|---|---|---|---|
| nonzero | +104h | +F8h / +FCh / +100h | +190h | +14Ch / +188h |
| zero | +68h | +5Ch / +60h / +64h | +F4h | +B0h / +ECh |

Each depleted selected pool requests **68000 decimal bytes** (`109A0h`), exactly
1000 slots of 44h bytes. The first 999 links store the next slot through +40h;
the final link at pool+1099Ch is zero. Allocation failure is not repaired:
the original subsequent stores remain, including the invalid null path.

The page-capacity load and equality comparison precede free-head publication
(`C55AC4/CA/D0`, `C55BC2/C5/C8`). Publication precedes any new/free callback.
Only equality grows; capacity is wrapping `2*old+2`, published before the
wrapping four-byte allocation request. Copy count/data reloads remain live;
each computed destination is tested separately, including wrapped/null cases.
The old data word is captured after copying, free runs before new-data
publication, and append reloads count then data. Count increments even when a
zero computed destination suppresses the store. These same epochs are retained
in the pending vector at +194h/+198h/+19Ch, after the initializer returns.

Taking an endpoint reloads the selected free head after page callbacks,
captures next+40h, increments live count before publishing the next free head,
and inserts the endpoint into the selected intrusive ring. Both tail reads are
retained: `C55B6E/B77` and `C55C43/C4C`. The first precedes endpoint+3Ch storage;
the second follows it before updating old-tail+40h. This preserves synchronous
callback and storage-alias epochs without claiming concurrent equivalence.

## Constructor and exception boundary

At `C55C71..C55C7F`, native code loads bounds, loads token into ECX, pushes
bounds, reloads the flag into DL, pushes the captured endpoint, then enters
the real initializer. Its native contract is ECX token, DL flag,
stack(endpoint,bounds), `RET8`. Root's explicit C++ contract is
`(token,uint8 flag,endpoint,bounds,same CRT)`, and the candidate matches it.
The returned constructor value is discarded; the captured endpoint is appended
and returned, as native EDI is at `C55D14`.

The constructor-only state0 region resets to -1 at `C55C84`. Handler `CC83DB`
loads FuncInfo `E04290`, maxState 1, whose `E04288` map is
`{previous=-1, action=CC83CA}`. That action pushes two saved locals and calls
`00401130` at `00CC83D2`. Independent live bytes show **00401130 = C3, RET**.
There is no basis for an outer pool refund, endpoint destructor or rollback.
The candidate correctly adds none. Root's real initializer retains its own
vector-storage cleanup/rethrow. Original FH3/SEH transport, stack/fault PCs and
register-frame identity remain outside this normal-body source qualification.

## Original call sites

| Site | Target | Role |
|---|---|---|
| 00C55AA0 | 00BF9F1A | nonzero pool malloc |
| 00C55AE7 | 00BF55BE | nonzero page growth |
| 00C55B27 | 00BF6989 | nonzero old-page storage free |
| 00C55B9A | 00BF9F1A | zero pool malloc |
| 00C55BD9 | 00BF55BE | zero page growth |
| 00C55C0E | 00BF6989 | zero old-page storage free |
| 00C55C7F | 00C4CE60 | actual initializer |
| 00C55CA9 | 00BF55BE | pending growth |
| 00C55CE7 | 00BF6989 | old pending storage free |

## Object-only dependency check

Exact candidate source and root's actual initializer snapshots compiled as
MSVC Win32 objects with `/std:c++17 /EHsc /fp:strict /O2 /Oy- /MD /W4 /WX`.
Both objects are I386; no executable was linked or run. The allocator object's
sole undefined in-house symbol is the explicit initializer, and its complete
decorated name exactly matches the definition emitted by the actual root
initializer object. The emitted call passes the same CRT, bounds, captured
endpoint, flag and token; the final return reloads captured EDI, not callee EAX.
Artifacts are in `local/endpoint_allocate_review`; source/object hashes are in
the report. No fake constructor, stub or default provider supplied this check.

## Small root dispatcher follow-up

The complete `00C5A850..00C5A8AC` body is 93 bytes / 35 instructions. Live and
installed bytes match, SHA-256
`80dfbd63bfdcdbca3601a6fb3eda31742041ed43587d8a56e539ccbad96009a0`.
Its single unsigned +198h comparison selects pending count >10; `C5A85D`
tail-jumps to real `C54DA0`. Otherwise `C5A864` enters real `C4CBF0`, then
loads head+AC and captures sentinel+B0 once. The real axis updater receives
ESI=0/1/2 at `C5A884`, `C5A890`, `C5A89C`, with next+40 reloaded only afterward
at `C5A8A1`. Root's header/source preserves that order and forwards genuine
volatile feature-word/half-cell references without a default or early value
snapshot. ECX is manager, no stack arguments, plain RET. The new void C++ API
does not promise incidental native EAX. No blocking finding; no separate test
or runtime credit is attached to this additional read.

Both interfaces remain unwired new C++ APIs. Root owns registration, common
build/link acceptance, locked annotations and actual caller/provider custody
or a truthful dead-chain conclusion. Root separately reports Win32 Release
and both suites passing in 11.23 seconds, plus an unexecuted retained batch-b
link forcing all five recent entries/dependencies with zero game-map entries;
those are primary receipts, not an independent runtime result from this review.
