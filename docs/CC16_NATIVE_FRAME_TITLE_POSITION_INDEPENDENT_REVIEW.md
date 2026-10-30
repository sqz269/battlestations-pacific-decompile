# Native Frame title, position, and collector independent review

The two production files at `40d286ed3b1451a7a58d7b9f3a315b990ddf5217` pass this bounded static review. No actionable correctness defect was found. The seven genuine application bindings, changed raw Text updater, common build, and runtime validation remain open; whole front-end readiness remains false.

Reviewed files:

- `include/bsp/native_front_end_frame_title_position.hpp`
- `src/native_front_end_frame_title_position.cpp`

The review tree merged the sealed packet with commit `994378998` while preserving the four GlyphQuad files from `6d0dc92b8c5cac92fa658d4b96edf00cd5c58239` unchanged. This review changes only its own document and JSON report. It adds no reconstructed body, Original, vendor, duplicate-body, ABI, activation, or runtime credit.

## Evidence checked

The entire 126-line header and 270-line implementation were read, followed by every instruction in the four saved assembly bodies. Independent checks matched both immutable Git Source pins, all 24 immutable input pins, all eight raw saved exports, the original PE identity, all four complete PE body hashes, six literal preimages and FNV checks, and three FH3 maps with six cleanup actions. No hash, length, instruction-count, map, or branch-target mismatch was found. Git blob bytes, rather than Windows working-copy CRLF bytes, are the Source hash basis.

| Entry | Inclusive end | PE bytes | Instructions | x87 forms | PE body SHA-256 |
| --- | --- | ---: | ---: | ---: | --- |
| `005189B0` | `00518B8C` | 477 | 126 | 12 | `7fe5fc22fbc04236b09f559010c1d41c03d351802efb282a9b110cdd5bae3109` |
| `005186B0` | `00518818` | 361 | 109 | 0 | `f6c53c855833087d8674ce2563ed7d98c45c4593990e644452eeba966fa8a8da` |
| `00518820` | `005189A1` | 386 | 119 | 12 | `46dbf6f367daac3e5f2ebce86834a8f5720b5491f033b9447ff6754d48fb5df3` |
| `00518E00` | `00518E70` | 113 | 34 | 0 | `7857c9c253e61d684c39661b56ef128ce7c470bda15f3a5939d317f4c19a89e1` |

Total scope is 1,337 physical bytes, 388 instructions, and 24 static x87 forms. The installed PE remains 12,223,752 bytes with SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Live `bsp.py ghidra count` passed the tool's `Client.verify()` check for project `bsp`, program `/battlestationspacific.exe`, x86:LE:32:default, and image base `00400000`; live and saved counts were both 77,632. The configured project is `C:/Users/sqz269/bsp.gpr`. No live analysis was mutated or exported.

## Title path: `005189B0`

The initial Page test and the title-child lookup each call the real Frame getter twice, retaining the first returned Frame for the slot base and reading `+160` from the second. Both phases read own selector state before global title selector `E18D8C`. The later box lookup also makes two fresh getter calls but reads `E18D8C` before the second Frame's `+160`. The Source helper preserves each order and does not collapse multiple returned generations into the current publication.

Both found widgets remain captured across raw Text update and subsequent calls. The real `00ABAED0` binding receives the captured Text, the captured incoming native string header, and literal argument one. The current Game cell is captured after that call; the `+61F` test precedes the short-circuited `+634` test. No semantic Game copy or default data is supplied.

The title name header's first DWORD becomes the quotient's actual FLOAT home after the native header return. Source reuses that same scratch word. The unfixed-width path preserves the exact x87 sequence: load Text `+114`, divide by the DOUBLE at `CEC380`, spill to FLOAT, load DOUBLE pad at `CEC3E8`, reload the FLOAT quotient, `FCOMI`, `FSTP ST1`, then `JBE`. Unordered comparisons reach the minimum branch. The greater branch spills the quotient again; the minimum branch pops it. Both empty the newly used x87 stack entries before calling the true `AA6740` pointer-returning getter.

The greater branch reloads the same quotient home after the getter, reads height from the returned pointer `+4`, reloads the current DOUBLE pad, adds, and spills the width. The minimum and fixed branches load height before their respective FLOAT width constant. The actual `AA6740` assembly is `LEA EAX,[ECX+20h]; RET`; a width-only or precomputed substitute would violate this contract. The Source does not carry ST0 across a C++ callback, reset FP state, or replace these forms with C++ floating arithmetic.

The selected size scratch remains alive through the final call. The current profile and `+58` target are captured from the already captured box only at that final native epoch. The required operation receives receiver, profile word, target word, and actual FLOAT2 address; the words are not invoked as host function pointers.

## Position paths: `005186B0` and `00518820`

Both routines construct the actual temporary native string headers with raw resize and copy, make two Frame getter calls per lookup, and load title selector before second-Frame `+160` on each lookup. They capture the title and box widgets before operating on either. The setter dispatches the captured box with the incoming ECX vector before dispatching the captured title with the incoming EDX vector; it does not snapshot the vectors ahead of callbacks.

The getter directly reuses the existing whole `gui_widget_get_local_position_00aa67f0` implementation. Its saved assembly is `LEA EAX,[ECX+0Ch]; RET`. Source copies the box's X, then Y, then Z with separate `FLD`/`FSTP` pairs, then reaches the title getter and performs its three pairs. Output/source overlap therefore propagates in the native order. A six-FLOAT snapshot or an early second getter would change behavior. The 12 getter x87 forms and all 12 title forms were checked by path; different Source branch layout is not treated as a native instruction-order discrepancy.

## Collector: `00518E00`

The collector uses the captured Frame throughout and performs three separate own/selector phases. Backdrop reads `+160` before `E18D84`, applies `own*13 + selector`, and uses slot byte offset `0C + 4*index`. Panel and title each reload `+160` before reading `E18D88` or `E18D8C`, using `(selector + own*2 + 44)*4` and `(selector + own*2 + 4E)*4` respectively. Unsigned DWORD arithmetic preserves the native wrap behavior.

The first Page value is tested before the output argument home is captured. Source captures that output home exactly once even when the first Page is null, then retains it across every reached append callback. Later phases reload their own and selector values rather than recomputing an earlier snapshot. Each nonnull test passes the actual Frame slot address to `004D6790`, allowing the real callee's later slot read; it does not append a copied Page value or substitute a semantic vector. No additional Page retain, Frame getter, output-home reload, or rollback is introduced.

## Native strings and exception intervals

Each constructor/copy must return before its temporary becomes armed. Only the reached repeated Frame getter and child-lookup interval is guarded. Normal return disarms before the explicit current-header pool return. Exceptional return invokes the raw-pool `destroy_native_string_header_0041dd20` overload once, then rethrows if cleanup returns; a throwing cleanup is transported without adding termination, retry, or rollback policy. The selected overload resolves the genuine manager and preserves data capture followed by current length-plus-one before pool return. The noexcept storage overload is not selected.

| Body | FuncInfo | Unwind map | State 0 action / header | State 1 action / header |
| --- | --- | --- | --- | --- |
| `005189B0` | `00D93DFC` | `00D93DEC` | `00C6A970` / EBP-44 | `00C6A978` / EBP-36 |
| `005186B0` | `00D93D94` | `00D93D84` | `00C6A930` / EBP-28 | `00C6A938` / EBP-20 |
| `00518820` | `00D93DC8` | `00D93DB8` | `00C6A950` / EBP-28 | `00C6A958` / EBP-20 |

Every map has two states with parent -1; every action tails to `0041DD20`. The three handler FuncInfo immediates and their `00BF6B43` branches were independently decoded from the installed PE. These checks establish the recovered cleanup intervals, not binary-compatible FH3/SEH behavior of the new C++ interfaces.

## Required integration gates

All seven pure required bindings remain genuine obligations: `004C1AC0` with the same manager/publication/custody; `00AA7E00` with the actual Page/widget/list graph; whole changed `00ABAED0`; the true pointer getter `00AA6740`; actual `00AA7DC0` transform services; finite current widget `+58` receiver/target dispatch; and raw `004D6790` vector insertion with a late slot-address read. None has a fake implementation or default fallback in this packet. Existing normal bodies may be reused, but the private provider and actual application binding still have to be supplied.

The nondefault data view authenticates six original-VA literal spans at `CEC3D8`, `CEC3C4`, `CEC380`, `CEC3E8`, `CEC3E4`, and `CE3E18`. It borrows the same canonical RO owner and does not establish a second data authority. The retained startup consumer must keep that owner, current selector cells, existing Settings Game reference, raw string-manager context, scratch, captured Frame/Page/widget objects, and all operation providers alive across deletion and singleton-drain paths. Old live Frames may differ from current publication.

No build, test, probe, process, game, new export, CMake, ledger, shared application/view, activation, or Ghidra write was performed for this review. Original register ABI, exact private-stack aliases beyond the exposed scratch, FH3/SEH identity, exact fault timing, unmasked FP exceptions/stack faults, gameplay, and visual parity remain unproved. The review accepts the bounded ordinary Source composition with these explicit gates; it does not activate the front end.
