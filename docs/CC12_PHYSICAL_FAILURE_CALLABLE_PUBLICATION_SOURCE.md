# Guarded physical failure callable publication: Source

Implemented the reviewed explicit opt-in on the existing genuine application/runtime owner. `GameNativeVfsApplication::publish_and_borrow_raw_failure_manager()` qualifies the existing raw C3 symbol, publishes it only at the retained manager’s `+90`, and returns a borrowed pointer. Normal startup keeps its Original numeric identities. **The normal Win32 build and all three existing checks passed; the new API and raw entries were not executed.**

This is a Source adaptation at existing interior store `0073D642`, with **zero additional Original function credit**. Exactly eight existing Source/header files and this document/report change. Accepted design/owner reports, raw entry Source/header, CMake, ledgers, packet registry and Ghidra are untouched. Baseline: `b70e66e21faaef8dec62c8463faaa287a277377d`.

## Implemented guards and dispatch

The public application method checks a new private completion flag set only after the final `bind_resource_registry_domain` succeeds. It calls a **private** runtime method through application friendship, so the existing `runtime()` accessor cannot bypass completion. The runtime rejects `!core_registered` or `retired` before owner access, captures the volatile Source publication once, and requires that pointer to equal its retained A0h allocation.

The trusted target is `&raw_ignore_native_vfs_mount_failure_00530620`; its single volatile code byte must be C3. No caller-supplied address is read. The owner’s `+90` must contain either `00530620` or that exact target, and `+8C` must still contain `00735B30`. Only then does the API write `+90` and return the captured owner. It neither invokes a callback/notifier nor changes `+8C`, `+18` or a publication cell. Repetition is idempotent under the same guards.

The shared finite dispatcher preserves the literal-identity route through the existing typed no-op. The exact Source symbol branch qualifies C3 and calls the named raw function **directly**; unknown values return false without dereference. The two callers retain their existing mount-specific rejection and `unsupported()` diagnostic. Typed C++ dispatch carries no raw register/flag ABI claim.

The caller must serialize publication and all uses against startup, publication/callback mutation and shutdown. Finish every borrowed use before `GameSingletonHost.shutdown()` begins, and retain the existing application/runtime/host/data/services/code through use and shared drain. This API adds no allocator, owner class, synchronized lease, fallback or lifetime extension.

## Whole Source bodies and private layout

| New Source body | Complete bytes / instructions |
| --- | --- |
| Public application publication method | 60 / 19 |
| Private runtime publication method | 138 / 45 |
| Trusted raw-target qualifier | 53 / 18 |
| Finite typed dispatcher | 78 / 28 |

The runtime’s compiled body checks its two phase flags first, captures publication at body offset 33, and performs its sole owner store at offset 83: `898E90000000` (`[ESI+90]=ECX`). The successful return is that captured owner. There is no indirect call in this publisher.

The appended completion byte is at host `Impl+524h`; host allocation/sized deletion grows from `524h` to `528h`. **The native VFS allocation remains A0h.** The old Impl constructor retains its complete bytes and ordered relocations around one 7-byte zero-store insertion. Initialization adds the 7-byte true-store immediately after the registry-binding call, with two existing branch displacements adjusted. Existing private field offsets remain unchanged.

## Preservation and intentional compiler changes

Across ten complete provider objects, **950 old bodies** retain exact full bytes and ordered relocations under the same symbols. One removed generated `make_unique` cleanup path uniquely corresponds to the complete **19-byte** existing Impl-constructor cleanup, including its ordered destructor relocation. The report records both exact path names; no shortened-name or byte-window match substitutes for that correspondence.

**Twelve old bodies intentionally change.** Two are the finite-dispatch callers. The other ten are application completion/constructor, allocation wrappers, sized deletions and generated EH paths affected by the appended byte and compiler outlining. The JSON report lists every exact symbol with complete old/new bytes, ordered relocation records and an explanation. No blanket unchanged-object or Original EH/ABI claim is made.

The protected bodies remain exact:

| Protected Source body | Exact body |
| --- | --- |
| Startup installer | Whole 32 bytes / 9 instructions, zero relocations; two separate publication reads and both identity stores preserved |
| Both ordinary typed no-ops | `C20000` each |
| Raw callback | `C3` |
| Raw notifier | `8B8190000000FFD0C20400` |

Eight whole core objects uniquely match complete archive members; the two application/host objects belong directly to the game target. Four whole objects are byte-identical before/after; the stronger preservation claim above concerns complete function bytes and relocation records in all ten objects.

## Ordinary game link evidence

The actual Release map timestamp and preferred base match the retained executable. Complete COFF relocation replay matches **five whole linked bodies**: installer, finite dispatcher, raw C3, mount dispatcher and runtime open-failure dispatcher. Every DIR32/REL32 operand resolves through actual map rows, sections and same-VA aliases; nothing is accepted by searching for a C3 byte pattern.

The map identifies the raw C3 at preferred VA `103676F0` and finite helper at `1031AD00`. The finite helper’s complete address comparisons, code-byte read and direct E8 target replay to that exact symbol. Uniqueness concerns the selected symbol/address, not the occurrence of C3 across the image. These are linked-image addresses, not observed loaded process addresses.

The public/private publication APIs, standalone qualifier and raw notifier have no unique normal-map entry. Their evidence remains **whole object/archive only**; no linked or loaded body is claimed for them.

## Validation and retained inputs

Normal command: `./scripts/build.ps1 -Diagnostic`, Release Win32. Passed: `reconstructed_math`, `native_math_differential`, `tool_tests`. No new tests, ad hoc links/probes, API calls or raw-entry calls were added. The existing checks retain their established scope.

Evidence family: `local/cc12_physical_failure_callable_publication_source_20261008a`. The before-edit, before-build and after-build phases each physically retain 629 selected rows. All 460 unique actual compiler include/ambient inputs for the four changed translation units were copied before the build and agree afterward; their CPP files are separately retained. Include candidates not selected by the actual read logs are not described as consumed. Whole proof SHA-256: `63db71bab82aea955d4b8b3143b3b67a9b6de5046b1e8fcd352ff0cdac99ee50`.

The three project libraries have pre/post artifact copies. All 23 recorded link libraries are retained; the 20 external SDK/CRT import libraries were copied **after build only**, with explicit timing. The normal map, executable, build/check log and link records are retained. No exhaustive compiler/linker/runtime-DLL/OS closure is claimed.

The Source feature is built and statically qualified within these limits. No initialized live callable owner, loaded-code lifetime, shutdown concurrency, complete Original ABI, factory/lifecycle graph, connected BF5030/read or gameplay result is established. The raw notifier still treats EDX and its discarded stack DWORD independently and reads neither fixed global 0109CEEC nor manager `+18`.

Full evidence, the twelve intentional old-body changes and the unique generated-path correspondence are in [the JSON report](../reports/cc12_physical_failure_callable_publication_source.json).

Primary review independently rehashed all1919 retained phase/post-proof copy rows and460 actual compiler inputs, confirmed614 selected inputs stable and15 expected build outputs changed, matched all eight current Source/header texts, and reconciled all963 old functions (950 exact unchanged, one unique19-byte cleanup correspondence,12 deliberate whole-body changes). Every one of968 current functions in ten complete objects agrees with its production body and ordered relocations, with bijective compiler-path symbol mapping. Eight core objects match unique full archive members. Root replayed five full worker/current linked bodies and20 operands per image using real map-symbol/section/ICF-alias evidence. The combined MSVCWin32 build/all three existing checks pass. Public/private publication APIs, standalone qualifier and notifier retain object/archive-only ordinary-link status; the feature, raw entries and real initialized owner were not executed. Root registered a Source-only interior-store adaptation with zero Original-function increment and retained the existing parent name. Receipt: `local/cc12_physical_failure_callable_publication_primary_review/receipt.json`.

Final combined review after the incoming main type8 Source registration rechecked26 selected whole objects, all1424 complete functions,23 unique current core archive members and five complete linked bodies with20 relocated operands. Selected Source pins are current, and the normal MSVCWin32 build/all three existing checks pass. No new API/raw-entry execution or game proof is added. Receipt: `local/cc12_facade_callable_final_after_main/receipt.json`.

Final combined review after incoming main reference Source registration rechecked the same26 whole objects/1424 functions,23 unique current core members and five complete linked bodies/20 operands, all unchanged. Current normalWin32/all three existing checks pass; no new execution. Receipt: `local/cc12_facade_callable_final_after_main_reference/receipt.json`.
