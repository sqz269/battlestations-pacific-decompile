# CC12 dormant borrowed full Application shell

The explicit `construct_native_application_00737970(void*, NativeApplicationSingletonBaseContext&)` now implements the admitted ordinary full shell over the genuine `construct_native_application_loop_base_00bea970`. It captures the original receiver, calls that Source child exactly once with the same context, ignores its result, performs the eight specified stores only after normal return, and returns the captured receiver. It has no caller or startup activation.

The accepted Original evidence is the complete 42-byte / 15-instruction `00737970` body, retained in the immutable readiness archive together with the 22-byte / 8-instruction `00BEA970` body. This packet performs no Native/GPR query, mutation or export. Original ECX/no-stack-argument/EAX-original/RET0 behavior is evidence for the reconstruction; the explicit C++ context API is a new Source interface, not a drop-in binary ABI.

## Source contract

The existing header and TU are exact byte prefixes of the new files. The only TU additions are the needed existing `native_shader_preload.hpp` type include and the appended function. Existing function bodies remain exact. `NativeApplicationPointerVectorStorage` already supplies the actual 12-byte header with `void** data_00` and signed 32-bit count/capacity. The new helper uses volatile typed member assignments and an actual `void* volatile` Game cell, with no integer alias over pointer objects.

Entry requires stable aligned actual enclosing 1Ch storage, an already-live uint32 profile at +0, an already-live vector header at +08 with its actual member objects at +08/+0C/+10, and a distinct already-live void* Game object at +14. These lifetimes and valid byte accesses must exist before the child call. The helper starts no lifetime and creates no owner. Prior ownership must permit overwriting the preimages without releasing them; no all-zero preimage is assumed. The new outer stores touch only their specified fields; they preserve +05..07 and +1B themselves. Genuine child/provider effects remain governed by the inherited contract.

All inherited BEA970/BEA810 actual manager/Application cells, shared manager domain, provider contracts, disjointness, captured-section identity, current-depth rules, reentrancy and escaped-reference lifetime obligations remain prerequisites. The null-section path skips Enter/depth/Leave but still arms the child guard before publication. Volatile adds ordered accesses in the admitted Source domain; it establishes neither synchronization nor invalid-storage/hardware/foreign-exception behavior.

After normal child return the exact Source schedule is: profile CFEAB0 at +0; typed null data at +08; signed zero count and capacity at +0C/+10; typed null Game at +14; byte +19=0; byte +1A=0; byte +18=1. No preimage load, aggregate initialization, memset, placement construction, reserve, resize, preload or Game-frame call is added. No new context, provider, cleanup, catch, rollback, free, retain, owner or canonical cell producer is introduced.

If the child escapes or does not return, none of these outer stores is reached. Its existing ordinary C++ cleanup remains the sole cleanup, including base reset, captured section/current depth, normal-Leave uncertainty and termination if a second ordinary C++ exception escapes cleanup. Publication or registry effects may survive. A reset to CE3818 grants no safe-drain or storage-return authority: the existing manager can dispatch CE3818 with flags1 to the generic singleton free path. Actual escaped references must remain valid under their original lifetime contract.

## Positive current compiler evidence

MSVC inlines the genuine child call. The new normal body is 245 bytes and has zero out-of-line child-call relocations. At 0x28 it captures the original receiver in ESI and at 0x4C the captured section in EDI. The old child's EDI/ESI allocation and its ECX depth temporary differ, so literal child-body byte equality is not claimed. Every instruction from 0x28 through 0xB1 matches the child's operation, offset, width and receiver/section/depth role after explicit register-role substitution. The prologue and relocated epilogue match exactly; provider relocation sites remain 69, 96, 129, 147 and 165 for getter, Enter, getter, registration and Leave.

The child profile D68BC8 store is at 0xA9, EAX/original receiver at 0xAF, and byte +04=0 at 0xB1. Full-shell stores begin only at 0xB5:

| Code offset | Actual receiver field | Store |
| --- | --- | --- |
| 0xB5 | +00 | DWORD CFEAB0 |
| 0xBB | +08 data pointer | DWORD zero from typed nullptr |
| 0xC2 | +0C signed count | DWORD zero |
| 0xC9 | +10 signed capacity | DWORD zero |
| 0xD0 | +14 Game pointer | DWORD zero from typed nullptr |
| 0xD7 | +19 | BYTE zero |
| 0xDB | +1A | BYTE zero |
| 0xDF | +18 | BYTE one |

The typed pointer assignments are positively verified as four-byte zero stores under this Win32 compilation. EAX remains the captured original through the stores and RET at 0xF4. No portable all-zero-null-representation assumption replaces this inspection.

The added 50-byte cleanup/handler section is byte-exact to the current child's cleanup, with only the wrapper EH symbol substitutions. Its handler starts at actual byte 21. The new 52-byte xdata table has unwind-table symbol at byte 0 and function-info symbol at byte 16, with the same two-state chain: state1 GuardCleanup then predecessor state0 BaseCleanup. All symbol values, ordered relocations, complete payloads, section headers and auxiliary records are retained.

The Source base cleanup lifetime/armed byte precedes D68BC4. In the emitted /EHsc body, state0 selection follows that non-C++-throwing store and precedes the first provider; state1 is selected at 0x78 before publication at 0x7C and remains through normal Leave at 0xA3. The compiler elides explicit disarm/selector-clear stores through the eight valid-storage non-C++-throwing full-shell stores. This is ordinary C++ evidence only, without Original FH3/private-frame or hardware-fault equivalence.

## Complete cohort and extra include material

The full Core archive has 1,993 entries / 1,990 COFF members; all 80 App objects are included. All 174,401 old code sections retain exact payloads and symbolic relocations, with zero changed or removed old code. The three additions total 333 bytes: normal wrapper 245, cleanup/handler 50, and a 38-byte compiler `??_H@YGXPAXIIP6EPAX0@Z@Z` section emitted by the needed type include. There are 17 whole-object hash drifts, distinguished from code changes.

Actual compiler read/write/command/items logs identify the same 17 header consumers and complete 20-TU before/after cohort. Dependency paths increase from 514 to 515 through the existing type header; all 40 complete physical object indices are retained. The full cohort contains 1,041 exact matched old non-code sections, all 27 old vtables and all 122 old EH/SEH tables exact. The constructor provider TU's five old EH/SEH tables total 236 bytes; the added 52-byte table brings that total to 288. All old public function-info symbols are at byte 16.

The include also emits two constant digit-pair tables: 200 ASCII bytes and 400 UTF-16LE bytes encoding 00 through 99. A full Core/App target graph verifies all 2,531 old definitions and all 67 references unchanged. The new compiler helper matches all 579 old same-symbol peers exactly; each new constant table matches all 976 old peers exactly. Actual symbol/COMDAT auxiliary records, offsets, payloads, relocations and the helper's complete decode are retained. There are no references to these three symbols from the new wrapper or its entire TU.

The only changed ordinary non-code metadata are `.drectve`, `.sxdata` and `.chks64`. Exact directive insertion is three annotate_string/vector/optional=0 mismatch markers plus two uuid.lib default directives. UUID default directives already occur in 684 objects and now occur in 685; the actual after-link UUID library is retained alongside the five provider libraries. SXDATA raw indices are resolved to unchanged old handlers plus the new handler at byte 21. Every CHKS64 record is checked against the associated payload's eight-byte SHA-256 prefix, or zero for empty/self sections; this is a checked observed correspondence, not a broader compiler-format claim. Twelve physical section-index/pointer/header changes are separately recorded.

Positive current COFF/import providers resolve every selected direct relocation. Game `.text` remains exact and the map differs only by timestamp. There are zero Source callers, direct entry relocations or Game map entries for the new constructor; its five internal EH-family references provide no activation evidence.

## Baseline and replay

The independent before capture freezes Main `f56daf8e390f3e3e0737ba77aa4a1a8857ab7579`, 4,228 Source/build-input paths including 4,210 src/include files, and 4,253 actual Git rows across 11 real tree objects. All 3,356 retained c491 primary Source inputs are LF-equal before the change, with eight raw newline variants. Its four historical artifact paths remain provenance only. Fresh captures and builds supply this packet's current compiler evidence. A separate read-only observation of Main `8268054c5c7b990f3fa51641d7d9240833e902d5` has no Source delta; its eight metadata paths do not alter the frozen worker epoch.

Both normal `scripts/build.ps1` runs and all three existing checks passed. Before: 2026-10-11T02:14:23.728093+00:00 to 2026-10-11T02:14:46.429208+00:00. After: 2026-10-11T02:25:13.339001+00:00 to 2026-10-11T02:25:37.231492+00:00. Checks are reconstructed_math, native_math_differential and tool_tests. The existing duplicate spawn_request LNK4006 warning remains. The 2,995-byte verified seed metadata stays read-only and unchanged. No new tests, ad hoc probes or game/OS/audio paths were added or executed.

The [JSON report](../reports/cc12_application_full_borrowed_shell_dormant_source.json) pins the complete evidence ZIP, strict helper, all current helper sources, actual Source/Git objects, builds, physical COFF indices, complete code/data/peer graphs and linked artifacts. The supplied-ZIP replay passed both in place and from distinct physical copies. After dependency preload it opens only the supplied ZIP once, with all other opens/writes, subprocesses, network, new library loads and directory enumeration blocked. The accepted full readiness layer replays its 12 helpers and the nested loop29, outer11, earlier readiness108 and BEA810 implementation23 helper graphs; all 853 earlier readiness pins remain physically retained. Capture/build/packaging helpers are hashed and retained but inert during replay.

The final four-file commit is separately checked against every actual staged/final Git blob, working bytes, Source roots and exact provisional pair. This completes only the dormant borrowed Source shell and current static/compiler validation. Raw ownership, actual canonical Application/name-cell production, frame/name handoff, caller/startup activation, callable profile binding, retirement/storage return, Original FH3/external ABI and live game behavior remain unproved.
