# Fallback resource type initializer Source

The complete observed `B86A00..B86A7C` publication schedule now has an ordinary,
unbound C++ interface. It borrows the actual fallback guard and receiver and calls
the genuine Scene and counter providers. Three isolated strict MSVC Win32 objects
compile successfully. Production backing, consumer attachment, original ABI and
CRT placement remain unestablished.

Baseline: `dc35052f65c95cbce1f49f5e85053d1bb0806927`. This packet owns only
`native_fallback_resource_type_initializer.hpp/.cpp`, this document and its report.
It changes no existing provider, CMake registration, ledger or Ghidra state.

## Evidence and interface

The accepted [delegate readiness](CC12_RESOURCE_CLASSIFIER_FALLBACK_DELEGATE_READINESS.md)
supplies the complete 125-byte, 31-instruction Native body and its verified
ownership. That is retained evidence; this packet opened no new Native window,
callee, name, data, table, handler, pseudocode or disassembly. The original has an
ECX receiver, preserved ESI, no stack arguments and ordinary RET. The new function
accepts a C++ context reference and does not reproduce that ABI or original EH and
hardware-fault behavior.

`NativeFallbackResourceTypeInitializerContext` requires:

- The actual process guard byte `0109020D`, independent of the receiver.
- Stable actual storage for four receiver words `[own, Scene, root, name]`. The
  observed startup receiver is `0109021C`; no backing is created here.
- The exact numeric original name token `00D631F4`. It is neither dereferenced nor
  replaced with a local string, and no literal contents or extent are inferred.
- Canonical actual Scene storage `01090210`, exactly the same current storage used
  by the supplied genuine `NativeMeshResourceTypeIds` instance.
- The same retained `TypeIdCounterLifetime` and root domain used by that Scene
  provider. The caller guarantees correct disjoint storage, stable bindings and
  sufficient lifetimes across every dependency call.

These are explicit caller contracts. This interface adds no validation, private
counter, process array, copied IDs, fake guard, no-op provider or automatic startup
call. The four receiver words are observed roles, not a recovered full class or
vtable layout. Scene `01090210` and fallback `0109021C` remain distinct identities.

## Ordered behavior

A nonzero fallback guard returns without publishing destination state. The zero
path performs the following sequence:

1. Publish fallback guard `1`, then the original name token to receiver word 3.
2. Invoke `initialize_scene_resource_00b869c0` on canonical actual Scene storage.
   Its existing implementation represents the Native inline Scene schedule:
   guard/name publication, genuine root initialization, current root-ID copy,
   genuine shared counter consumption and Scene own-ID publication.
3. Load current Scene word 0 and immediately store receiver word 1. Then load
   current Scene word 1 and store receiver word 2. These are alternating
   load/store operations; they do not use the camera fragment's paired captures.
4. Call the real counter getter `get_006fac20`, load the returned owner's current
   `next_id_04`, store its wrapping unsigned 32-bit increment, and only then publish
   the captured old value to receiver word 0.

The Source contains no catch, reset or rollback. If Scene or counter initialization
throws a Source exception, the guard remains set and all earlier writes persist.
A retry observes the sticky guard and performs no repair. Invalid bindings, native
fault handling, concurrency and original exception ABI are not supplied by these
ordinary C++ contracts.

## Compiler and complete COFF evidence

The candidate, `native_mesh_subset_loading.cpp` and `light_type_bootstrap.cpp` were
compiled as separate ordinary x86 objects using MSVC `19.51.36244.0`, toolset
`14.51.36231`. The generated Release project explicitly supplies `/W4 /WX /O2
/Ob2 /EHsc /MD /fp:strict /std:c++17` settings. The captured provider command log
also contains `/Oy-`; the project has no explicit `OmitFramePointers` property.
This audit reuses that recorded option without labeling it an explicit generated
property. Exact argv, include paths, definitions, environment selection, `/Bv`,
dependency records and complete separate stdout/stderr are retained.

All three compiler commands and all fifteen `dumpbin` commands returned zero.
The compiler's stderr contains `/Bv` component/version information. No fresh
normal build, link, probe, test or executable run was performed; Root owns later
registration and the normal build.

| Newly compiled Source body | Positive symbol index | Bytes | Instructions |
| --- | --- | --- | --- |
| Fallback initializer | `0x16` | 87 | 33 |
| Genuine Scene initializer | `0x600` | 74 | 30 |
| Genuine counter getter | `0x3C` | 205 | 66 |
| Genuine root initializer | `0x3F` | 38 | 13 |

The candidate's call instructions at `+22h` and `+41h` have REL32 operands at
`+23h` and `+42h`. Their exact physical symbol indices identify the genuine Scene
and counter references; each has a unique positive definition in the freshly
compiled providers. The Scene body references the genuine root and shared counter.
Selected relocation graphs follow local definitions, including the counter's
compiler EH helper, by index. Remaining sound-lifetime/allocation and CRT
dependencies are preserved as external boundaries of this three-object audit.
They are not stubbed or claimed to have been linked here.

The complete three objects retain 798 sections, 2,451 physical symbol records
including every auxiliary record, 1,057 relocations and all 490 decoded functions.
Every retained function decodes completely. The candidate's separate vector
constructor helper is retained independently; its bytes are not counted as the
fallback initializer. Raw section contents and full dumpbin outputs accompany
the indexed records. All counts describe newly compiled Source artifacts.

The complete candidate code shows the Scene loads at `+2Ah` and `+35h`, separated
by receiver stores at `+2Fh` and `+3Bh`. It stores the counter increment at `+4Ch`
before own-ID publication at `+52h`. The Scene provider likewise stores its counter
increment before its own-ID publication. This is compiler evidence for the
ordinary Source schedule, not a new Native byte or binary-ABI comparison.

## Frozen inputs and remaining gates

All three whole translation units and every include in their compiler dependency
records are frozen. The input record contains 114 project inputs, including the two
new Source files and 112 baseline Git blobs, plus 236 external compiler include
files. Baseline equality is checked after CRLF normalization; exact working bytes
and exact Git blobs are retained separately. Seven compiler components reported by
`/Bv`, selected build tools, complete build configuration and recorded provider
commands are also frozen. This does not claim a complete operating-system DLL or
whole-application dependency closure.

The actual production fallback guard/descriptor owner and lifetime, application
consumer and all remaining genuine loading/forwarding/deletion bindings still need
their own admitted composition. No original CRT slot/order, numeric-ID ordering,
full startup execution, fixture execution, runtime parity, Present or gameplay
result is claimed. Existing discovery/readiness documents remain unchanged.

Report: [Source, compiler, full COFF and input pins](../reports/cc12_fallback_resource_type_initializer_source.json).
