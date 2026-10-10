# CC12 instance-generator metadata-context borrow

This Source packet adds `GameNativeRendererApplication::borrow_instance_generator_context()`, returning a stable `const NativeInstanceGeneratorContext&` for serialized, ready-state metadata inspection. It constructs no `NativeInstanceGeneratorOwners` and introduces no consumer or Native activation path. Its new C++ interface is not a recovered Native ABI or a drop-in replacement.

The packet follows the accepted [lifetime readiness review](CC12_CONTEXT_LIFETIME_STARTUP_HANDOVER_PRIMARY_REVIEW.md), accepted by Root at `7f7898bdb8ae414ac4be489ed9d435555ff0d245`. The frozen worker baseline remains `d9d076fa0617d61979e8189bd5083d4d59366f36`; the earlier Source749 reference remains `e9396b4883232059bdad28753bc95403536403bf`. Preparation preceded authorization, while all Source edits and compilation followed the recorded acceptance. Root's raw CRLF report and its distinct LF Git blob are both retained and compare equal after line-ending normalization.

Before handoff, Root reported main `7e33f2025` and a later Source819 build with four unrelated raw-ID Source changes. Its `Source819_frozen.json` receipt is retained separately and matches the supplied SHA-256. This worker was not resynchronized: all comparisons below retain the original Source749 baseline. The later receipt's 823 referenced artifacts are not claimed included, nor is equality against all current Root Source asserted.

## Binding and lifetime contract

Every borrow requires the application's `ready` phase and a false result from its existing acquisition-retention query. It applies the existing graphics-domain metadata check and verifies the original declaration cache/loading, stream, geometry and vertex identities. The context uses the already constructed `p.section_layouts`, whose original provider bindings remain unchanged.

The other inputs are the existing process-owned serial reference, four original mapped profile pointers, and the original mapped declaration-name pointers. The profiles at `D62190`, `D61BFC`, `D61C1C` and `D619F8` are checked as numeric two-word selectors: `BD30E0` followed respectively by `B55CB0`, `B450A0`, `B451A0` and `B417C0`. These numeric values are never invoked as host calls. The names come from mapped `D61C08` (18 bytes) and `D61C28` (42 bytes); no replacement strings are fabricated.

One appended `unique_ptr` caches the plain 36-byte context. First use allocates and publishes it; subsequent permitted borrows skip allocation and revalidate all six top-level fields, including all four profile pointers and both name pointers. The cache is never rebuilt or rebound. There is no serial value read, increment or reset. Allocation failure occurs before publication and leaves the renderer phase unchanged.

The caller must retain the application and its original providers, serialize uses, and end **all borrowed views and copies** before frame entry, reset/recreation, shared drain or provider destruction. Reborrow afterward. Const is shallow and the context can be copied; this is a documented metadata-only contract, not a capability boundary. No Native construction, factory/finalizer, registration, setter, release or mutable service use is permitted through the borrow. It adds no acquisition tracking or retention enforcement.

## Complete Source and compiler evidence

The frozen baseline contains 50 source roots, a 709-file quoted-include closure, four additional configuration files, and all 713 matching Git blobs. The candidate adds one include edge (1,464 to 1,465), with no unresolved quoted includes. All 753 earlier Root Source749 pins remain frozen; 749 are selected Source inputs. Exactly the application header and implementation change across the 4,205-file tracked Source census. The compiler's complete dependency reports additionally freeze 1,059 whole Source/header files, including external, MSVC and SDK headers.

Four standalone Win32 Release objects retain baseline/candidate normal and `/d1reportAllClassLayout` builds, exact commands, compiler tools, diagnostics and twenty complete dumpbin outputs. `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17` and the captured normal target settings apply. Normal/diagnostic function code and call edges agree completely; all 792 baseline application functions match the captured Root object, and all 812 candidate functions match the ordinary worker build object.

The normal `./scripts/build.ps1` run passed the existing `reconstructed_math` and `tool_tests` checks. No tests were added. Its full log, executable, map, configuration, all 95 explicit link inputs and all 1,990 Core archive members are frozen. The new accessor has one application-object definition and no Core archive definition. The definition census covers explicit object inputs and all Core members; it does not claim to search every system-library export. Root's seeded three-check integration build remains separate.

## Object changes and preserved behavior

Complete Root and worker provider review covers 50 objects: 8,986 old functions and 9,006 candidate functions. All 8,194 functions in the other 49 objects preserve raw instruction bytes and complete relocation targets/addends under explicit generated-name bijections. Unreferenced lambda COMDATs are paired only through unique remaining whole-function signatures, with candidate groups and already matched definitions retained. No instruction-byte masking is used; the initially incomplete name comparison is preserved as an attempt.

The application retains 781 of its 792 old functions byte-for-byte with matching edges. All eleven differences are accounted for:

- Eight allocation/deletion wrappers change only the `Impl` size operand from 9,932 to 9,936 bytes.
- The constructor adds a null cache-pointer store and advances/repositions its EH state from 13 to 14. Every old instruction and relocation has an explicit counterpart.
- The destructor adds a guarded sized delete of the plain 36-byte context before the old mesh/provider cleanup. Every old instruction, branch destination and call remains accounted for.
- The old mesh unwind funclet preserves its 14-byte body and edge. Five `INT3` section-end padding bytes now follow the new context unwind funclet.

The twenty added functions comprise the 612-byte accessor (172 decoded instructions), eighteen ordinary context/`unique_ptr` helpers and one context-only EH funclet. Standard-library `reset`, `release` and delete helpers manage the ordinary metadata pointer; they are not Native cleanup. The accessor's move assignment contains a guarded sized-delete block. Under the serialized first-borrow precondition its destination is null, and cached borrows branch over allocation, publication and that delete block.

Diagnostic class layouts retain all 54 old `Impl` member rows at their original offsets and append the cache at offset 9,932. The context's x86 offsets are graphics 0, layouts 4, profiles 8, serial reference 24, generic name 28 and building name 32. All 940 old application code symbols map to their original bodies or the reviewed instruction positions; two generated local label ordinals change explicitly.

All noncode sections are retained and compared across 55 object pairs. Other providers differ only in compiler debug/checksum data or explicitly decoded RTTI generated-name spelling. The application adds four error strings and seven compiler metadata sections. Its constructor FH3 table adds one unwind state while preserving all earlier entries and edges; its 48 SafeSEH handler names and order are unchanged despite different physical symbol indices. The only new undefined external is the already implemented process serial accessor.

## Reproduction and limits

The [machine report](../reports/cc12_instance_generator_context_borrow_source.json) pins the evidence under `local/cc12_instance_generator_context_borrow_source/`. `immutable_manifest.json` and the ZIP retain whole files; `bundle_receipt.json` and the final replay receipt are detached to avoid circular hashes. Run:

```powershell
python local/cc12_instance_generator_context_borrow_source/review_evidence.py
```

The read-only replay verifies the manifest and ZIP, reparses 104 full COFF objects (21,200 functions, 109,858 physical primary/AUX records and 60,802 relocations), recomputes the 55 function comparisons and 58 layout/noncode/semantic reports, and checks complete Source/Git, include, dependency and ordinary link context. It performs no compilation, child-process calls, Native entry, Ghidra mutation or original-game execution.

The leased Native boundary anchors remain `B451D0`, `B85610`, `B55B20`, `B44FD0`, `B450D0`, `B55BE0`, `B55CB0`, `B450A0`, `B451A0`, `B41710`, `B417C0`, `B41780` and `B417E0`; this packet changes none of those bodies or their original ABI. Prior serial-owner and lifetime evidence retains its pre-first-use/dynamic-writer limits. This is Source and ordinary build evidence only. Root independent review/integration, Native owner activation, startup, rendering, mesh/parser use and gameplay validation remain separate holds.
