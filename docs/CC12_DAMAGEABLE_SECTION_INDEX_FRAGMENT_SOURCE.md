# Damageable section Index fragment: ordinary Source

This packet supplies only `[0087CF35,0087CF70)` of parent `0087CA80`: 59 retained bytes, 13 instructions. It adds an ordinary MSVC Win32 C++ entry with the actual Lua and integer-conversion providers. It does not supply the parent entrypoint, section iteration, category/effect fields, an application caller, or a Native ABI bridge.

The accepted inner-field readiness receipt is the Native evidence source. This packet decodes only its retained Index bytes. It opens no new Native body, child, data, handler, PE, or Ghidra window. The retained slice SHA-256 is `be81b83ec177bf9a6831d9827f654ddd607b4d8ca664c77c0c950bd3a602e4b5`.

## Entry contract and ownership

The caller supplies the same successful live Msh owner and scratch for this invocation, its live actual S2C Lua row, fresh aligned 14h S6C storage, and the previously selected actual writable 30h row. The owner reference is a caller precondition; this entry performs no runtime state check. Msh S18 and its original saved captures remain live in outer state17. Index neither reads nor closes that string and introduces no saved-buffer accessor.

The key is borrowed actual `00CE55B4` NUL-terminated storage. The mode is borrowed actual mutable `0109EEA4` DWORD storage, exposed by a `const volatile uint32_t&`; an address binding may be captured before the getter, but the mode value must be read at conversion time. These are actual storage identities, not copied semantic substitutes. Storage, bindings, capacities, tracking references, Lua owner/index/error-handler state, and actual raw row lifetime must remain valid across callbacks. Required private-frame and scratch disjointness/alignment apply; reentry and binding replacement are excluded. Valid callback changes to the actual row, header, and mode value are preserved. No row reselection or post-callback repair is added.

## Retained operation and ordinary cleanup

| Native operation | Source operation |
| --- | --- |
| CF35 key; CF43 B67800 on actual S2C into S6C | Genuine protected named lookup, borrowed actual key; completes before arming18 |
| CF4A state18; CF52 B66270 | Arm private ordinary guard, call direct actual `native_lua_number_00b66270` |
| CF57 BF7420 | Assembly caller consumes that getter's ST0 through the genuine DWORD-mode converter |
| CF60 row+8 store | Store EAX bits directly to the actual raw row while18 |
| CF63 state17; CF6B B67700 | Lower17 before destroying the actual S6C object |

The retained Native EH record is state18 ->17, action `00C969AF`, cleanup `00B67700` on `[ebp-94h]`. It is reused as retained evidence, not re-opened. The new ordinary guard destroys only S6C when armed18 and lowers17 before attempting cleanup. A failed lookup leaves17. A normal cleanup failure cannot retry the field because the guard is already17. Secondary guard-cleanup failure follows ordinary C++ `noexcept` termination. This is not a Native FH3, SEH, longjmp, fault, or double-exception equivalence claim.

The direct numeric getter can accept a numeric string through the actual Lua `lua_tonumber`; unsupported values yield Lua's zero. No exact-number predicate, number-or fallback, bool-mode helper, C++ integer cast, or default floating-point policy is inserted. The selected fresh getter calls the real float32 narrowing provider and returns ST0. Its ordinary ABI contains a float64 spill of Lua's return, then the genuine float32 spill/reload in the narrowing provider. These remain explicit precision/status/trap qualification boundaries.

The existing naked `BF7420` Source reads the complete live DWORD. A nonzero word takes its FSTP double/CVTTSD2SI path; zero tail-jumps to the existing complete `BF7456` Source. Both actual definitions are present in the freshly compiled provider object. The full fallback, including both special-path pops, was inspected. No caller float spill or FP instruction occurs between the getter and converter. No float object is invented over the raw row.

## Fresh compiled evidence

The immutable baseline is commit `46e5787d44256ccba93217a51c4384471d21b57f`. Root Source733's 737 frozen pins, command tlog, project, and physical primary receipt were copied or verified before compilation. The Root primary receipt's CRLF bytes differ from the tracked LF bytes; parsed JSON is equal and both physical hashes are recorded. Root's normal build and three existing checks are inherited context, not admission of this new fragment.

Four units were compiled with the exact ordered current Root Release argument records. Only source/include/repository/output paths were localized, and `/showIncludes`, `/FAs`, and `/Fa` evidence outputs were added. The candidate uses Root's Lua-unit option record; each existing provider uses its own. The receipt contains original and localized argv, full commands, complete logs, compiler/header pins, assembler output, dumpbin output, raw objects, physical section indices, symbol/AUX records, raw section headers, relocations, and exact code decodes. This is a focused compile, not a new normal build or a linker/run result.

| Object | Physical sections | Code sections | Code bytes |
| --- | ---: | ---: | ---: |
| New Index | 11 | 4 | 351 |
| Actual Lua objects | 118 | 66 | 4,240 |
| Actual numeric | 11 | 5 | 211 |
| Actual render batch keys/converters | 12 | 8 | 425 |
| Total | 152 | 83 | 5,227 |

The candidate main is physical section6, 200 bytes. Its key offsets are lookup+4A, arm18+5D, getter+6D, caller stack adjustment+72, load ECX mode address+75, converter+78, row+8 store+80, lower17+86, destructor+8D. There is no intervening FP instruction or mode-word load. The mode-address load at+4F is a binding read, not the mutable DWORD read. The raw row publication precedes disarming, independently checked in emitted code.

The guard is section4, 80 bytes; section5 is its 29-byte handler; section7 contains the 42-byte main unwind/handler code. SafeSEH uses symbol indices26 and30. Main section9 has a two-state unwind map (guard and terminate) and FH3 flags1; guard section10 has zero states and flags5. The live ordinary guard registration covers both calls inside inline assembly. Source EH indices0/1 are distinct from the explicit reconstruction states17/18. The header-generated unused weak AVX2 cell is inventoried, not claimed as a reconstructed Native global.

All four direct project references resolve by exact decorated name to unique real definitions: Lua lookup section49, Lua destructor38, direct getter65, and DWORD converter section8 of the render-keys object. The selected graph includes the actual float32 helper section7 and converter fallback section10. All 66 Lua code sections and five numeric code sections, including indexed relocations, equal the accepted Chance artifacts. Unselected render/helpers and other provider sections are whole-object inventory only. External Lua, CRT, and MSVC runtime symbols remain explicit linker/library dependencies.

## Replay and limits

`reports/cc12_damageable_section_index_fragment_source.json` contains the complete audit, current quoted-header Source closure, bounded reads/searches, baseline Git blob pins, compiler inputs, Root737 frozen pins, and evidence manifest pin. The ignored `local/section_index_source_verify.py` verifies the report in the worker checkout. `local/section_index_source_evidence.zip` contains every manifest-pinned input payload, including the frozen Root artifacts, plus `report.json`, `manifest.json`, and `verify.py`. After extraction, `python verify.py --bundle-root .` replays byte/pin, Source snapshot/read, retained-slice, complete COFF, exact-definition, dependency, and emission-order checks without the original checkout. It uses the bundled baseline blob bytes rather than requiring Git. The zip SHA is supplied in the closeout receipt to avoid a report/archive hash cycle.

The packet adds no tests, probe, runtime execution, normal build, CMake registration, ledger entry, Ghidra mutation, or whole reconstructed function. Root owns registration, normal build, and integration. Full parent/state741 composition, original ABI, FP precision/status/trap/fault parity, Native EH, application storage binding, and runtime/game validation remain held.
