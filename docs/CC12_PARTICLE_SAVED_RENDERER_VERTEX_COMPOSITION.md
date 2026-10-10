# Saved-renderer vertex composition

`create_native_vertex_from_captured_renderer_5c` is a dormant Source helper for the particle constructor's already-captured renderer/`+5C` pair. It validates the supplied header, saved B287C0 token, nonnull profile view, and canonical owner-domain identity; calls the existing actual B287C0 provider with the exact saved arguments and acquired creator pointer; then performs existing canonical stream admission. It returns the same acquired creator. The [report](../reports/cc12_particle_saved_renderer_vertex_composition.json) retains Source, build, compiled-body, provider, and replay evidence.

The complete resource delegate and persistent particle-model custodian remain unimplemented. No application caller, Native address ownership, Ghidra read/write, CMake change, new translation unit, test, standalone probe, registry, or ownership policy is added.

## Interface and caller obligations

The new interface is declared in `include/bsp/gui_text_native_renderer.hpp` and defined in `src/gui_text_native_renderer.cpp`:

```cpp
void* create_native_vertex_from_captured_renderer_5c(
    void* actual_renderer, std::uint32_t captured_entry,
    std::uint32_t first, std::uint32_t bytes, void* actual_descriptor,
    NativeStreamCloneServices&, NativeStreamCloneAcquired&);
```

The caller supplies the actual live renderer and descriptor, the actual immutable Native profile view, real physical/device/pool services, matching canonical owners, and a fresh persistent acquired frame. Header/token/nonnull checks do **not** establish that a pointer names the actual Native object or that a copied layout has valid provenance. Profile extents, object storage, current globals used inside the original providers, and all callback/service lifetimes remain the caller's obligations. Existing real device-recreation requirements remain reached provider boundaries.

The helper checks the supplied receiver's header against D5F0A8 and compares the saved code token directly with B287C0. It checks that the vertex context's actual renderer profile view is nonnull; it does not read its current `+5C` value. The geometry owner's canonical registry identity must equal the vertex context's registry identity. These are host interface guards, not a recovered original function ABI. The existing current-slot helpers and all other pre-existing source text are preserved byte for byte.

The captured renderer is never replaced from `00F8D394`, and the saved entry is never recomputed. `first`, `bytes` and `descriptor` are forwarded unchanged to `create_native_registered_vertex_stream_00b287c0`. For the existing particle call site, those values are `0`, `0x1000` and the descriptor returned after renderer/entry capture. This packet adds no call site to connect that particle delegate to the helper.

## Failure and publication schedule

The existing `begin_stream` enforces null creator/companion and `phase == empty`, then sets `vertex = true` and `phase = factory`. The Native provider receives `&acquired.creator`, so a completed creator is stored before renderer-array append. The provider's original constructor-allocation unwind and late current `+58` registration behavior remain unchanged.

After the factory returns, the helper applies the existing null-creator `std::bad_alloc` policy. A null creator is not evidence that the renderer had no effect: B287C0's original behavior includes appending its result, including null. The frame stays in its entered phase if this failure occurs.

The helper next sets `phase = registration`, calls `geometry.register_stream_clone_creator(acquired, services)`, then sets `phase = complete` and returns `acquired.creator`. Existing GUI stream admission records a created companion in the acquired frame before canonical binding. An exception can therefore preserve a completed Native creator, partial renderer-array effects, or a companion whose canonical admission did not finish. No catch, cleanup, retry, extra Native retain, automatic disposal, or new custody is introduced. The caller must preserve the frame and real services until the entered effects and relevant callbacks are resolved under the existing contract.

This is an independently usable composition boundary. It does not provide the mesh/material/section preconstruction admission seams or establish persistent particle-record custody and later quiescence identified by [the readiness review](CC12_PARTICLE_RESOURCE_DELEGATE_CUSTODY_READINESS.md).

## Fresh build and compiled evidence

Baseline: `58d45d1de060e5a23b28ca954a284f8cdf1bbf9f`. The normal MSVC Win32 Release build ran before the edit at `2026-10-10T22:49:11Z` and after it at `22:52:06Z`. Both completed successfully and passed the existing `reconstructed_math`, `native_math_differential` and `tool_tests` checks. The existing verified math seed was retained unchanged. These checks do not execute the new dormant helper.

The compiler logs identify 89 header consumers. Full before/after object, symbol, auxiliary-record, section, relocation and payload indices cover all of them plus the other three selected provider translation units: 92 TUs per phase, 184 complete indices. Their 706 project dependency paths, original compiler read/write/command/item logs, actual project items, all 1,990 Core COFF members, and all 80 application objects are retained. The dedicated roster is `local/cc12_particle_saved_renderer_vertex_composition/cohort_roster.json`, 173,530 bytes, SHA-256 `9786a77376b4476e55291413c4795822d977a6b79a6accf154cd3bde20a36bf6`.

All 174,356 old Core/App code sections have identical bytes and symbolic relocations. None were removed or changed, and no private-name normalization was required within this worker's before/after pair. Exactly one new section appears in `gui_text_native_renderer.obj`: 229 bytes, 74 fully decoded instructions, SHA-256 `07a2b2c1bfaff35f2d65091b96c506956bfbb441f98c0a00383c96cf3dfd708d`.

The complete added symbol is:

```text
?create_native_vertex_from_captured_renderer_5c@bsp@@YAPAXPAXIII0AAUNativeStreamCloneServices@1@AAUNativeStreamCloneAcquired@1@@Z
```

The compiled body checks the receiver at `0Ch..1Ah`, saved token/nonempty profile view at `20h..33h`, owner identity at `35h..42h`, and fresh frame at `47h/4Ch/52h`. It sets the factory phase at `5Dh`; pushes `&creator`, the same vertex context, descriptor, bytes, first, and saved receiver; and calls B287C0 at `71h`. The null-result branch is at `7Ch`, registration phase at `7Fh`, canonical admission at `89h`, same creator load at `8Eh`, complete phase at `91h`, and return at `9Dh`. All branches and seven calls are retained and checked. No `+5C` or current-renderer load occurs in this body.

All 13 direct relocations of the new body have positive COFF or current import definitions. The complete selected-provider review covers 62/63 decoded functions before/after and 188/201 direct relocation edges, with 65/68 positive provider sections. Four current CRT archives plus Kernel32's current import archive, named by the actual link log, supply the retained external import evidence. These import records do not prove runtime DLL implementations or unbounded transitive closure.

The full Core/App census contains zero relocations to the new helper in any section, and the Game link map contains no helper symbol. Game `.text` remains exactly 3,992,064 bytes with SHA-256 `e34181ca5affdc445a64cc6f0e632c6dac79bc8748ce632f89c49cb2f584d86b`. Whole-object equality is a separate claim: 89 objects changed, including three application header consumers, while their old code remains exact. The PE `.rdata` and image timestamp differ; `.data`, `.rsrc`, `.reloc` and `.text` are exact, and the map differs only in its timestamp line. Whole image equality is not claimed.

## Closed Source and portable acceptance

The frozen baseline and actual staged tree close all six Source roots through real Git commit/tree/blob objects. All 4,228 Source paths, five extra epoch metadata paths, and twenty named prerequisite documents are retained: 4,253 actual Git/current-file rows. The actual staged Source tree is `d32ffdb105ad5c045e2d7e6f9b62935b7f672c45`. Final commit verification requires its six Source root identities unchanged.

The accepted Root Source3356 adapter manifest is pinned by SHA-256 `fa67d37ee3567afdd432c4096504dda8e1b805f3dd5e7f0ec752501041e0fb07`. The prior adapter4245 and readiness4249 Source snapshots are also verified against their accepted report pins. All three agree with this packet's baseline; only the two authorized helper files differ afterward. Historical build declarations are not followed or credited. The worker builds above supply this packet's independent artifact evidence; Root-versus-worker symbol matching and integration acceptance remain separate.

Before/after finite lexical checks each cover 4,211 C/C++ corpus files and 81 queries. The new helper has exactly its declaration and definition. The existing caller, resource, random, lifecycle and admission excerpts/order checks remain retained. These lexical results are not AST, dynamic reachability, original Native ABI, or quiescence proofs.

Both replay executions passed with identical results: the original evidence ZIP and a physically copied helper/ZIP in `relocated/`. After standard-library/Capstone preload, the audit guard permits exactly one read of the supplied ZIP and blocks all other file opens, writes, processes, networking, new library loads and directory enumeration. Replay rederives source closure, prerequisite pins, compiler cohort, all full COFF indices, old/new code census, exact argument forwarding, positive direct providers and artifact comparisons from retained bytes. It does not compile or execute the archived binaries.

ZIP: 114,468,220 bytes, SHA-256 `8a1f44e62293bdbb142dee4d4fb6a52a81bbebec5a16a9d21e0aa8f6bb405c5d`. Replay helper: 7,443 bytes, SHA-256 `d11e4b3eec40a579556ffb226a21fc52d8af4fe11e04a77e775cf66bbe55c36f`. Both are under `local/cc12_particle_saved_renderer_vertex_composition/`; run `python portable_replay.py evidence.zip`. The report records both execution results and pins this document, helpers, receipts and retained artifacts. Final checks require exactly four owned tracked files and a clean committed worker tree.

This establishes a built and statically checked dormant Source composition. Original entry ABI/SEH compatibility, live helper execution, device behavior, completed resource delegate/custodian, particle construction, callback quiescence and gameplay remain unproved.
