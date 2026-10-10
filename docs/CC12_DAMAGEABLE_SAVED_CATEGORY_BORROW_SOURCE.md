# Saved category pointer borrow: ordinary Source helper

This packet adds only `try_borrow_saved_category_data(expected, output)` to the existing `NativeDamageableClassSectionMshFieldFragment`. It exposes the original pointer saved by `open()` for the separately reviewed category/FireID caller contract. It implements no category lookup, effect acquisition/release, raw manager adapter, or application call site. The new helper has no recovered Native function address or whole-function reconstruction credit.

The accepted prerequisite is [the capture contract](CC12_DAMAGEABLE_CATEGORY_FIRE_ID_CAPTURE_CONTRACT.md). The accompanying [machine-readable report](../reports/cc12_damageable_saved_category_borrow_source.json) retains the current Source, before/after COFF, exact commands, compiler identities, archive provenance, and baseline evidence. Local replay: `python local/saved_category_borrow_verify.py` from this worker worktree.

## Contract and effects

The method is `const noexcept` and returns true only while this owner has private state 17 and `expected` is the exact scratch object borrowed at construction. Rejection leaves `output` untouched. Success assigns only `static_cast<const char*>(saved_data_)`; a captured null pointer is a successful result, distinct from rejection. The output reference must designate a live, disjoint pointer object and must not alias this owner's private storage.

The caller remains responsible for the original saved buffer's lifetime, current bytes, and any required NUL termination throughout later callbacks. Borrowing does not copy text, establish buffer validity, or freeze its bytes. Owner/scratch/bindings stay stable without reentry. The method does not inspect current S18, read saved length, call providers, retain/release anything, change state, or close the owner. An invalid guard is a caller-precondition failure; it is not an empty-category fallback.

The existing constructor, destructor, `open`, and `close` are byte-for-byte unchanged at the Source level after removal of the two added declaration/definition blocks. No fields, includes, virtual members, or copy/move operations were added. The owner remains 20 bytes under the actual Win32 compiler: scratch 0, string context 4, saved data 8, saved length 12, private state 16. The scratch also remains 20 bytes.

## Emitted behavior and preserved providers

The actual normal-build Msh object contains six complete code sections, 433 bytes. Its new method is physical section 10, 36 bytes / 16 instructions, with no relocations, calls, floating-point instructions, or added EH metadata. It compares `[this+16]` with 17, then the expected scratch address with `[this]`. Only the success path loads the output address and `[this+8]`, stores the pointer once, and returns true. Either guard failure returns false without loading the output argument. Null is not filtered.

All five pre-existing Msh code sections (397 bytes) remain identical. The four direct provider objects also preserve their 153 code sections / 10,254 bytes. Across the five complete objects this is 158 unchanged code sections / 10,651 bytes, plus the new 36-byte method. Relocation comparison preserves offset, kind, target value/storage, and target section except the Msh EH metadata section's recorded shift. Twelve Lua/String relocations have different compiler-generated anonymous-namespace discriminators between worktrees; their complete old/new names are retained and normalized explicitly. No claim of literal private symbol-name identity is made.

The existing destructor handler still refers to the same 36-byte FuncInfo (magic `19930522`, maxState 0, flags 5), now section 12; its SafeSEH entry shifts from symbol index 37 to 38 and still binds the same handler in section 6. This is preservation of the ordinary compiler's existing EH, not Native FH3/SEH equivalence.

The fresh Msh object's seven project references resolve by exact decorated name to the actual provider sections: Lua lookup/destructor/string getter (49/38/69), raw string destructor/resize (56/65), pool getter (30), and pool return (20). The report preserves every code section, indexed relocation, physical section header, symbol/AUX record, weak fallback, and selected reference graph in all five before/after objects, including the explicit Lua/CRT/MSVC/Win32 and singleton frontier. The new method adds no edge to that graph. Source and archive provenance also cover all 18 implementation files in the 58-file owner/provider closure; archive membership alone is not instruction-level proof for the other 13 objects.

## Compilation and retained baseline

Before modifying or building, the packet physically copied all 750 retained Source746 inputs/artifacts (90,334,689 bytes), the Root primary receipt and manifest, and all 58 current Source preimages at `eba1f3dccfcfa62caa50c64a4926a7998330a4e5`. The prior Root context has 746 selected inputs, 106 Core objects, five App objects, 296 positive selected definitions, and three passing checks. Those are inherited evidence, separate from this worker's build.

The worker ran `./scripts/build.ps1` successfully. Its two configured existing checks passed: `reconstructed_math` and `tool_tests`, 7.41 seconds total. `local/seed_reference.hpp` was absent; this worker does not claim `native_math_differential` or three checks. No new tests or probes were added. The normal Core library contains 1,986 members and exactly one positive definition of the added method; this establishes archive availability, not application activation.

Two additional isolated object compiles used the exact ordered Root Msh command with localized source/include/output paths and only include, assembly, and class-layout evidence flags added. The compiler is MSVC 14.51.36231, Hostx86/x86, with Root `/O2 /Ob2 /Oy- /MD /fp:strict /EHsc` settings. Preimage and candidate emitted code match their actual before/after archive objects, including indexed provider bindings. Whole commands, translations, complete include occurrences, compiler/tool/header pins, and both class-layout records are retained.

## Credit boundary

This is a build-tested ordinary Source accessor with reviewed emitted guards and an unchanged owner/provider contract. It does not activate a category/ID caller, adapt the current typed effect manager to the actual sixteen-byte raw manager, or establish current virtual release bindings. Native register ABI, x87/FP identity, Native FH3/SEH/longjmp behavior, full-parent composition, application wiring, and game/runtime validation remain held. No Ghidra queries or mutations, new Native byte windows, shared ledger/CMake edits, or native probes were used. Root owns the final integration and three-check build.
