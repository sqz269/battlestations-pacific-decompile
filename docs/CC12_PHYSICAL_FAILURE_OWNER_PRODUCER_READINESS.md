# Physical failure owner producer readiness

There is an existing concrete Source owner producer: `GameNativeVfsApplication` allocates A0h storage, initializes it through the recovered VFS constructor chain, retains its services, and lends access through `GameNativeVfsRuntime`. **The owner is still unready for a raw failure call because callable-code-domain publication at manager+90 is missing.** A new allocator or a mock owner would not resolve that boundary.

This audit changes only this document and its JSON report. It makes no Source/CMake/metadata/Ghidra edits, build, link, fixture, probe, test, or entry call. The pending raw-entry packet is untouched. Baseline: `4d78da36320e646af15ee89842a530fe86013cca`.

## Exact Original producer chain

| Site | Retained scope | Established effect |
| --- | --- | --- |
| `0073D615-0073D63B` | 39-byte / 9-instruction parent fragment | Push A0h, call allocation helper BF681B, retain EAX, compare it with incoming EDI, and call BEDA60 with ECX=EAX on the unequal branch |
| `00BEDA60-00BEDAAE` | Whole 79 bytes / 22 instructions | Call BE1DC0; write derived profile D68D04; get/register physical factory; return captured owner |
| `00BE1DC0-00BE1F5A` | Whole 411 bytes / 115 instructions | Call BDA6F0 first; initialize A0h fields and six real sentinel/container allocations; write +18=FFFFFFFF and +90=0; make six ordered path-canonicalization calls |
| `00BDA6F0-00BDA780` | Whole 145 bytes / 40 instructions | Publish actual global 0109CEEC under the captured lifetime section and register current publication |
| `0073D63C-0073D65B` | 32-byte / 4-instruction callback fragment | Reload actual 0109CEEC before each store: +90=00530620, then +8C=00735B30 |

All these bytes were freshly read from the verified Ghidra program and match the installed PE. The two parent fragments do not claim the whole Application_Initialize function. The allocation fragment's incoming EDI/EH state and allocator body remain parent prerequisites; no wider parent control-flow admission was needed to identify this producer.

The actual publication instruction is `00BDA746`, inside the base constructor, not in BEDA60 or the application caller. The base first gets the lifetime manager and captures its section+10, enters/increments that section if present, publishes the VFS owner, gets the lifetime manager again and registers **current** 0109CEEC, then releases the captured section. The lifetime-section recursion field+18 is distinct from the VFS manager field18.

Publication precedes full manager initialization: the later BE1DC0 instructions at `00BE1DFF` and `00BE1EEE` write owner+18=FFFFFFFF and owner+90=0. Those are constructor stores, not observed runtime preimages after later calls. Publication alone cannot establish an initialized callable owner. The global's static initial image is four zero bytes in the zero-filled tail of `.data`; it is not a manager pointer.

## Existing genuine Source ownership

`GameNativeVfsApplication::Impl::initialize_core` uses a one-attempt guard, establishes its real type/process-pool prerequisites, and calls `singleton_lifetime_allocate` with A0h native and host sizes. That existing allocator uses `std::malloc`, with CRT new-handler retry or `bad_alloc` on failure. The application retains the resulting `manager_storage_a0` and a concrete `GameNativeVfsRuntime`, binds that runtime to the singleton host, and calls `construct_and_register_core`.

The runtime runs the existing BEDA60 -> BE1DC0 -> BDA6F0 Source chain on that allocation, explicitly republishes it into the shared Source cell, installs callback identities, and registers the FileStore and MPKG factories. Source BE1DC0 initializes the real storage, including +18=FFFFFFFF and +90=0, six allocated raw sentinels, and six path operations. This is an application-owned initialized object path, not a callback-only adapter or a zeroed 94h probe buffer.

The application owns its `vfs_0109ceec` Source cell; `NativeVfsOwnerServices` shares it with the manager and raw-service contexts. This Source reference is not proof of storage at Original absolute address 0109CEEC. `GameNativeVfsRuntime::actual_manager()` returns the actual input storage pointer, and `borrow_raw_services()` lends the publication reference and retained services. The plain pointer getter does not certify successful initialization. Borrowing must follow normal completion of the real initializer and respect the lifetime below.

## Lifetime boundary

Before core registration touches A0h, failure to retain the runtime frees the allocation. Once native construction/registration may have partially published state, the application retains the runtime and allocation instead of inventing a rollback. Its documented caller contract requires `GameSingletonHost.shutdown()` before the application bundle is destroyed, including after such a partial failure.

The shutdown Source body destroys the shared singleton manager with the retained deletion bindings, retires VFS services after the drain, records observers, frees the lifetime manager and clears its Source cell. The A0h object passes to the registered deleting-destructor domain once registration begins. This audit identifies that existing ownership contract; it does not execute or newly admit the whole constructor, unwind, or teardown graph.

## First absent provider and stopping point

The current Source installer `install_native_vfs_startup_callbacks_0073d63c` is complete **32-byte / 9-instruction COFF**, with **zero relocations**. It stores literal Original identity `00530620` at +90; there is no relocation to a Source callback symbol. Production typed identity dispatch maps that number to a named Source handler. A raw indirect call through +90 needs a real executable target in the same process/code domain, with owner and code lifetime retained through the call. The literal identity does not establish that Source contract.

The first missing provider is therefore **callable-domain publication/lending at the existing 0073D63C installer boundary**, specifically its +90 store at 0073D642. It is not a missing manager allocation or getter. No actual runtime manager, field18/+90 preimage, completed initialization, callable target address, or drain was captured here.

The smallest next packet would define this callable publication/lending contract using the real application-owned manager and an explicit policy compatible with the existing identity-dispatch route. It must not substitute a mock owner, a callback-pointer store into fake memory, a context-only adapter, synthetic 0109CEEC, or a silently reinterpreted identity. The sealed raw-entry bodies provide static evidence only; they do not publish a target or establish linked/loaded code. Stop here before a larger VFS expansion. Connected BF5030 still requires its separate actual publication and field18 provenance.

## Retained evidence

Manifest: `local/cc12_physical_failure_owner_producer_readiness_20261008a/audit01/manifest.json`, SHA-256 `014e2d607eb060ccb22a9565e0f267708c79ca486e26175932fb313341bbc124`. Whole selected Source COFF: SHA-256 `9a41b638dc81cfb298a6e3dc65f1b547a1894509b98ed85681fae9956747f38d`.

The **50 physically retained transfer rows** contain nine whole Source providers and headers, nine whole existing objects, the whole installed PE and core archive, selected contracts/recipes, and audit tools. Seven core objects match exact unique complete archive members. The two application/host objects belong directly to `bsp_game`; no archive membership or linked/loaded-body claim is made for them. All 14 selected complete Source functions decode fully, and every selected capture/copy/after hash agrees.

All copies were made during this read-only audit after earlier builds. There is no new build or exhaustive compiler/OS closure claim. The immutable native records include whole 79/411/145 bodies, exact 39/32 fragments, and the static global preimage with its runtime limitation.

Machine-readable details: [cc12_physical_failure_owner_producer_readiness.json](../reports/cc12_physical_failure_owner_producer_readiness.json).

Primary review rehashed all50 physical inputs, all706 Native bytes across three whole bodies/two exact fragments,18 whole current Source/header texts,14 selected complete functions and all835 functions in nine actual current objects. The seven core objects have exact unique retained/current archive members; the two direct game objects carry no new link claim.98 compiler path-derived generated symbols are paired by unique complete unchanged bodies and a bijective call-target map. The current32-byte/9-instruction zero-relocation installer still rereads publication twice and stores the two Original identities. Previously pending raw C3/notifier Source bodies are now independently admitted as static whole entries; callable owner publication/lending remains absent. Receipt: `local/cc12_release_body_and_failure_owner_primary_review/failure_owner.json`. No actual owner runtime preimage, shutdown, new entry execution, Source, build/test or class/game admission.
