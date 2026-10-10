# Ordinary gameplay effect ID wrapper Source

`acquire_gameplay_effect_by_id_00870cd0` now composes the genuine canonical
manager getter and lower ID acquisition. It captures the actual output address
and signed ID, always calls the getter, loads the actual borrowed volatile flag
word only after successful getter return, forwards its full width, and returns
the captured output address independently of the lower return value.

Baseline: `b8ac06b7b13557f4a2698cce0693342047804dad`. The four owned files are
the new `include/bsp/gameplay_effect_id_wrapper.hpp`,
`src/gameplay_effect_id_wrapper.cpp`, this document, and
`reports/cc12_gameplay_effect_id_wrapper_source.json`. Existing providers,
headers, CMake and ledgers are unchanged. Root owns normal registration,
build/integration and any later annotation.

## API and observable order

```cpp
void** acquire_gameplay_effect_by_id_00870cd0(
    void*& out, std::int32_t id,
    const volatile std::uint32_t& actual_flag_word,
    GameplayEffectAcquisitionContext& context);
```

`out` is the actual live C++ pointer object. The flag reference borrows the
actual live `uint32_t` object, not a snapshot. The context and its manager,
lifetime domain, publication, string, Lua and component services must be genuine
compatible application objects with stable valid lifetimes/bindings. A
successful getter must return a live canonical `GameplayEffectManager` before
the C++ reference passed to the lower function is formed. This ordinary API
does not add a null-manager fallback or claim Native null/fault behavior.

Providers may change the valid borrowed flag object before its post-getter
load; that change is observed. Output/flag must not alias private wrapper
locals, reference bindings, context metadata, or incompatible C++ objects.
These exclusions do not prohibit valid provider mutation of the borrowed flag.
No raw Native parent slot or raw manager is cast into these C++ objects.

The wrapper does not clear the output first, narrow or normalize the flag,
skip manager work for ID0, check a provider result, catch an exception, retry,
or roll back completed effects. If the getter fails, the flag load and lower
call are not reached. Lower failure follows the genuine provider's existing
error and lifetime contracts, including its cleanup/termination boundaries.
The wrapper adds no local cleanup policy.

The accepted Native body remains retained evidence only: 42 bytes / 18
instructions at `00870CD0..00870CF9`, with epoch-10 typed receipts and Root's
primary review. No Native bytes, bodies, metadata, callees, tables, handlers,
or data values were freshly opened for this Source packet.

## Actual command-matched compilation

Three complete ordinary objects were compiled: the new wrapper and unchanged
`gameplay_effect_acquisition.cpp` and `gameplay_effect_manager.cpp`. MSVC
19.51.36244.0, toolset 14.51.36231, Hostx64/x86 completed all three with exit 0.

The compiler arguments come from the actual Root Release `CL.command.1.tlog`,
parsed with `CommandLineToArgvW`, and the generated `bsp_core.vcxproj` is frozen
beside it. Only source, repository include, object and PDB output paths were
relocated; `/Bv` and `/sourceDependencies` were added for evidence. Option order
is preserved. Actual options include `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict
/std:c++17 /TP`, the original defines and dependency include paths.

`/Oy-` is already in those successful Root command records. It is not an extra
audit option, and no older variant without `/Oy-` is used as evidence for these
objects. The generated project has 1,914 `ClCompile` entries and does not yet
register this new candidate. That count is distinct from Root's selected Source
checkpoint numbering. No CMake build, link, test or executable probe ran here.

The emitted wrapper is 42 **Source** bytes / 20 instructions. Its equal byte
length to the Native body is incidental; its bytes and ordinary `__cdecl` ABI
are different. It has no wrapper EH section or local handler. The decisive
offsets in its complete section 6 are:

| Offset | Emitted operation |
| --- | --- |
| `09` | Direct call to the genuine canonical manager getter. |
| `0E` | Load the borrowed flag object's address from the private argument area. |
| `15` | One full DWORD volatile read of the actual flag, after the getter. |
| `17..1C` | Pass flag, ID, actual output, and manager to the lower C++ call; context was pushed at `11`. |
| `1D` | Direct call to the genuine lower ID acquisition. |
| `22` | Caller stack cleanup for both ordinary calls. |
| `25` | Replace lower EAX with the captured output address. |
| `29` | Plain Source `RET`; no Native `RET4` promise. |

The optimizer reloads the private C++ out/ID arguments after the getter even
though their Source captures precede it. Under the explicit private-frame and
binding alias exclusions, those stable argument values cannot be changed by
the provider. The actual volatile flag read remains after the getter. This
compiled schedule is recorded explicitly, not presented as the Native register
capture sequence or as support for private-frame aliases.

## Exact COFF definitions and complete physical coverage

All physical sections, complete raw payloads, symbols including AUX records,
and relocations are indexed. BSS is retained as logical zero-fill extent rather
than invented file bytes. All executable sections were decoded completely as
x86-32, including padding and terminal bytes; no code tail remains undecoded.
Non-code sections remain data. Full dumpbin headers, symbols, relocations,
directives and assembly are also frozen.

| Object | Sections | Symbols + AUX | Relocations | Code sections / bytes / instructions |
| --- | ---: | ---: | ---: | ---: |
| Wrapper | 7 | 22 | 2 | 1 / 42 / 20 |
| Lower acquisition | 179 | 564 | 242 | 102 / 4,930 / 1,909 |
| Canonical manager | 182 | 529 | 195 | 100 / 2,769 / 1,068 |

The wrapper's positive definition is exact symbol index `13h`, section 6.
Its only direct externals are `REL32` at offset `0Ah`, symbol `11h`, for
`get_gameplay_effect_manager_004c1650`, and offset `1Eh`, symbol `12h`, for
`acquire_gameplay_effect_by_id_008700e0`. They resolve to the actual compiled
manager definition at index `183h`, section 144, **283 bytes**, and lower
definition at index `16Bh`, section 126, **719 bytes**, respectively. No stub,
similarly named replacement, or source declaration alone supplies these edges.

The complete section-level relocation traversal from the wrapper contains
20 sections and 85 edges, including compiler metadata and all definition
candidates for encountered COMDAT names. Its 51 external frontier edges refer
to 25 distinct genuine Source/library names. There are no weak-alias boundary
records in this selected traversal. This is a relocation closure, not a whole
program link, actual virtual dispatch binding, or runtime proof.

## Selected error and lifetime graphs

The entire compiled lower body, manager body, their EH code/data and selected
cleanup helpers were reviewed with exact indexed relocations. The lower's
Source FuncInfo and unwind map contain 13 states; the manager's contain one.
They are current compiler Source EH records, not recovered Native FH3 identity.

The lower map records these actual action chains:

| State | Next | Action |
| --- | ---: | --- |
| 0 | -1 | Borrowed Lua host destructor. |
| 1 | 0 | Globals Lua reference cleanup. |
| 2 | 0 | Source `std::terminate` boundary during cleanup. |
| 3 | 0 | Effects Lua reference cleanup. |
| 4 | 3 | Definition Lua reference cleanup. |
| 5 | 3 | Source termination boundary. |
| 6 | 0 | Source termination boundary. |
| 7 | 4 | Name-field Lua reference cleanup. |
| 8 | 7 | Owned name string cleanup. |
| 9 | 8 | Standard-library pending node-allocation cleanup. |
| 10 | 4 | Source termination boundary. |
| 11 | 3 | Source termination boundary. |
| 12 | 0 | Source termination boundary. |

The lower miss path destroys its name, releases Name/definition/Effects Lua
references, and destroys the borrowed host before reloading the current map
value into the output at offsets `2AF..2B5`. Its final tree-length error call
at `2C9` and trailing `INT3` at `2CE` are included. The normal rejection path
stores null before its reference cleanup. The direct lower ID0 path remains
inside the provider and cannot bypass the wrapper's preceding manager call.

The manager's single unwind action destroys its captured section guard,
decrementing the captured recursion count and leaving that same section.
Publication/registration precede normal section release and the final volatile
publication reload. No wrapper rollback or new free path is inserted when a
genuine constructor/allocation/registration operation fails. Existing provider
allocation, reference validity and error obligations remain in force.

The graph explicitly stops at the real Lua host, NativeString, definition
allocation/identity, component loader, singleton lifetime/section services,
operator new/delete, length-error, security-cookie, frame-handler and terminate
providers. Complete current Source context for the repository frontiers is
frozen; those extra frontier bodies are not claimed as compiled by this packet.
Required component, string, Lua, manager and virtual lifetime bindings are not
fabricated to close the graph.

## Evidence and remaining admission

The bundle contains all three objects and complete compiler dependencies,
whole current Source/header snapshots and baseline Git copies, actual external
headers, tool hashes, command records, complete indexed COFF/code, selected EH
graphs, and retained Native/readiness inputs. The 25 project compiler inputs
and 197 external headers are frozen in full; additional genuine Source frontier
files and older immutable bundles are preserved separately. Artifact counts
and archive hashes are recorded in `pin_replay.json` and the paired report.

This packet is reconstructed and object-compiled ordinary Source. Root's normal
registration/build/ledger work is pending. No original ECX/EDX/RET4 interface,
raw-to-canonical parent storage bridge, profile vtable binding, Native EH/fault
behavior, startup, or gameplay equivalence is admitted. No GPR mutation,
configuration change, provider edit, test, probe, link, or runtime execution
was performed.
