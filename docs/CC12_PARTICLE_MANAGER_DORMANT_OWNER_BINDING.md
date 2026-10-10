# Dormant particle-manager owner and deletion binding

This implements the six-file Source binding approved after
`CC12_PARTICLE_MANAGER_RAW_DELETION_OWNER_READINESS.md`. The implementation uses
the accepted raw `NativeParticleModelManagerAccess` overload and leaves startup
activation to the separate caller packet. No production call to the new binder
or context borrower was added. Baseline: `d7a98030e1130d977b451de59d5eb06e4472f634`.

## Binding contract

`GameNativeStringProcess` now retains one typed
`NativeParticleModelManagerStorage* volatile` publication for `F8C274`.
`particle_manager_00f8c274()` returns the exact cell without creating an owner.
The class also retains a permanent Source admission marker, distinct from Native
storage. `claim_particle_manager_domain()` succeeds once and is never reset.

`GameSingletonHost::bind_particle_manager_domain(GameNativeReadOnlyData&)`
validates the mapped four-byte spans at `D5D7EC`, `D5D7F8` and `D7A24C`, checks
the first two words against `AF0870` and `AF1080`, and retains the live third-word
reference. It never caches the third word's value. The inline optional context
borrows the exact process `F8C274` cell and this host's canonical actual `01090AA0`
cell through the raw `SoundLifetimeAccess` overload.

The first call requires an externally established **valid, quiescent domain with
no prior particle-construction attempt**. A null publication is only a rejection
check; it does not establish that history or prove the registry has no surviving
owner. The function does not scan, adopt or repair an unknown registry. The
permanent claim blocks a new or replacement host after any earlier successful
binding, including after publication is cleared or a host address is reused.
This deliberately conservative Source admission rule is not recovered Native
behavior. It is not a concurrent binding protocol; the quiescent precondition
excludes competing first calls.

An already-engaged context on the same retained host permits a repeat call only
when the publication reference, actual manager-cell domain, live `D7A24C`
reference and installed deletion-context pointer all match. All span, profile
and reference checks precede the first permanent claim. After the claim, inline
emplace invokes the existing `noexcept` raw Access constructor, followed by
the pointer binding. No owner is allocated, fetched, published or registered.
Host construction has already obtained the canonical process, so the binder's
process access does not create its otherwise-lazy retained container.

`particle_manager_context()` only borrows an engaged context with its matching
deletion binding. The host explicitly disallows copying and moving. Its optional
context remains alive during the destructor's `shutdown()` body. The caller must
retain the verified mapped data and every raw owner dependency through that drain;
the reference alone does not keep a readonly-data service alive. The canonical
application data owner supplies the intended process lifetime.

## Raw deletion and compiled layout

The appended `NativeSingletonDeletionBindings::particle_manager` pointer has a
compiled Win32 offset of 192; the structure now has size 196. Existing earlier
members retain their offsets. The dispatcher admits `D5D7EC -> AF0870` and
`D5D7F8 -> AF1080` only with this context. It forwards the popped receiver and
full flags word unchanged. Missing binding and unknown profiles retain the
existing contract-error throw. There is no current-publication equality guard,
null skip, alternate receiver, or current-publication free.

| Source layout or emitted body | Before | After |
| --- | --- | --- |
| Retained process allocation | 56 bytes | 64 bytes |
| New process publication / claim marker | absent | offsets `38h` / `3Ch` |
| `GameSingletonHost` allocation | 276 bytes | 300 bytes |
| Host optional context / engagement byte | absent | offsets `118h` / `128h` |
| Host's borrowed deletion pointer | absent | offset `FCh` = bindings start `3Ch` + `C0h` |
| Native raw manager storage | `34h` | unchanged |

These Source class sizes and offsets come from the real build's constructor,
allocation and accessor code; they are not Native class-layout claims. Growing
the embedded deletion table shifts later host members. The process getter emits
`LEA EAX,[ECX+38h]; RET`; the claim method is a 16-byte leaf with no calls.

The 283-byte compiled binder makes three `data_at` calls, compares only the two
profile words, saves the returned `D7A24C` pointer, and passes that pointer to the
raw Access constructor. Its successful path has no allocator, manager getter,
registration or constant-value load. Failure paths retain ordinary Source
`logic_error` construction/throwing and can use that exception implementation's
allocation facilities.

In the compiled dispatcher, incoming receiver `ECX` is captured in `EDI` and
bindings `EDX` in `EBX`. Both new paths load context from `[EBX+C0h]`, then push
that context, the full flags word `[EBP+0Ch]`, and `EDI` for the corresponding
scalar call. The complete section is retained: 3,280 code/padding bytes and
265 inline jump/selector-table bytes. The table bytes are not disassembled as
instructions. The existing manager implementation's 27 complete function sections
retain their payload/label/relocation identities across the Root and worker builds.

The scalar bodies still destroy/free/return their captured receiver. Their base
destructor separately unregisters the current publication through its existing
distinct manager observations, clearing it only after unregister returns.
Derived section/array order, weak-pointer behavior, stale fields and ordinary
exception behavior are unchanged. Later construction must allocate the raw `34h`
receiver in the Source CRT domain consumed by `singleton_lifetime_free`.

## Validation and retained evidence

One normal `./scripts/build.ps1` invocation passed Release MSVC Win32 compilation
and all three existing checks: `reconstructed_math`, `native_math_differential`
and `tool_tests`. The existing math seed was copied unchanged from the accepted
build; SHA-256 is `26cdde71970ec8524c92d5e54e17c5587e817198ef16f3b6f2b8be25cd556d2c`.
No test or fixture was added. The unchanged duplicate `spawn_request_id_matches`
LNK4006 warning remains. The existing differential check executes its retained
math seeds; no particle Native code, startup, game or new runtime probe was run.

Current-before provenance is the integrator worktree's accepted Source3350 build,
not the main checkout's older build directory. The initial Oct 8 capture remains
preserved locally as historical rejected build provenance and is excluded from
current code/consumer comparisons. Exact worker six-file preimages equal the
accepted Root Source bytes. Of the 819 selected before inputs, 816 match the
Source3350 frozen manifest; `cmake/lua.cmake` and the current compiler consumers
`game_native_entity_registry_process.cpp` and `native_scene_property_bag_storage.cpp`
are separately pinned from their current Source/Git inputs and current CL records.

Each current phase retains 819 complete Source/build inputs, all 56 CL
read/write/command/items logs from 14 Release project directories, 29 complete
consumer objects (26 consumers of the changed headers plus three support consumers),
and 15 whole matching Core archive members plus the complete archive. All
qualifying write groups are retained; a write group is not assumed to be unique
or proof of fresh compilation. The fresh worker build log establishes its compile
run. External compiler/SDK/build-dependency paths are enumerated, not presented as
a complete retained compiler-installation preimage.

The guarded ZIP-only replay verifies 3,531 complete payloads, 58 objects, all
30 selected Core members, 110 `.bss` sections, Source/Git identities, the two
consumer cohorts, 26 selected complete function sections, compiled binding/call
facts, and reused finite Native evidence. A zero raw-file pointer always means
zero file-backed section bytes, independent of declared `.bss` size. Complete
object and nondebug identities are recorded separately; whole-object equality
across Root/worker paths is not claimed. Function comparison normalizes only
anonymous-namespace compiler hashes and checks payloads and relocation identities.

Copy `portable_replay.py` and `evidence.zip` from
`local/cc12_particle_manager_dormant_owner_binding/` and run
`python portable_replay.py evidence.zip` with Capstone available. After imports
and decoder setup, its audit guard permits only reads from that ZIP and rejects
checkout/Git/Ghidra/compiler/SDK/network/subprocess access and writes. A physically
relocated copy produces the same passing result. Separate `verify_git.py
--check-working` verifies 819 baseline Git blobs and current working files.

The report is `reports/cc12_particle_manager_dormant_owner_binding.json`.
It pins the full captures, compiled analysis, scripts, build/test evidence,
portable bundle/results and separate Git result. No Ghidra mutation or fresh
Native query was needed; the accepted two profile DWORDs, complete retained
scalar bodies and typed project metadata remain explicitly attributed to the
prior review epoch.

Production startup activation, its allocation/partial-construction failure
containment and four shutdown/exit guards remain the separately approved caller
work. This packet does not make an invalid graph safe for ordinary drain, reset
the context or permanent claim, repair publications, or establish original ABI,
SEH identity, runtime startup success or gameplay equivalence. Unrelated model
construction/lifetime consumers still require their later typed-authority adaptation.
