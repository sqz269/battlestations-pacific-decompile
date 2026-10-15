# CC12 name-node holder release

Whole `008F3D80-008F3DB9` and `008F3DC0-008F3DF9` are reconstructed as
ordinary Win32 C++ functions. Each Original body is 58 bytes / 24 instructions.
The names are receiver-specific hypotheses, not recovered class symbols.
This packet adds no class destructor, copy/adoption policy or refcount.

Both functions capture the holder DWORD once. A null stored node returns without
reading the key, obtaining either process context/pool, or clearing the holder.
A nonnull node releases its owning string key, returns its allocation to the
correct pool, and only after both returns writes a DWORD zero to the holder.

| Body | Original key calls | Exact node pool | Final store |
| --- | --- | --- | --- |
| `008F3D80` | `008F3D9A -> 00419CC0`, `008F3DA1 -> 00BD1510` | `008F3DAC -> 0043B0A0`, ECX `00E17578` | `008F3DB1` |
| `008F3DC0` | `008F3DDA -> 00419CC0`, `008F3DE1 -> 00BD1510` | `008F3DEC -> 0043B0A0`, ECX `00E17540` | `008F3DF1` |

The original staged data/length+1/unused1 arguments belong to `00BD1510`, not
the singleton getter. Null key data skips both key calls and the length read.
Length+1 uses DWORD arithmetic. The existing whole raw-context overload of
`destroy_native_string_header_0041dd20(node, strings.raw_context())` supplies
exactly this key composition. `0041DD20` is an existing Source composition,
not an Original direct call from these two bodies. It does not write the key
header, copy bytes, inspect characters, or substitute borrowed empty strings.

After that helper returns, the Source obtains respectively
`enum_symbol_pool_storage_00e17578()` or
`property_library_node_pool_storage_00e17540()` from the same retained canonical
physical process and invokes the whole `return_native_enum_node_0043b0a0`.
These returned-startup accessors add Source synchronization/state checks;
successful nonthrowing accessor use is part of the conditional domain.
The holder clear is an explicit volatile DWORD store after node return.
Neither body reads or settles node+8/+C mapped payloads or external map links.
Allocator-owned slot+10 retains its real page identity for the node return.

The holder must be an accessible aligned four-byte slot. A null holder pointer
is invalid; a null stored node is valid. The holder, node, key allocation,
canonical contexts, pool/page/table/common-list storage and caller frame must
be stable and disjoint. The node must be a genuine unique currently allocated,
detached 14h slot from its exact initialized 38h owner, with valid page/table
membership, current generation and +10 identity. Caller-owned mapped payloads
and all external links must already be settled; a fabricated header or matching
profile word does not establish provenance.

Use the same retained canonical string process and its actual `01090AA8`
publication, `01090AA4` live return gate and `01090AA0` manager. Nonnull key data
must be an owning buffer with a matching allocation size/domain, including the
current CRT for large returns. Keep its process and publications live through
release. The raw helper resolves `00419CC0` for every nonnull data return,
including large frees and disabled small returns. It never returns fallback
`E177E4`, `E17654`, `E186ED`, literal or borrowed key bytes as owned storage.

Use the same canonical physical process with explicit relevant startup already
returned, the real `CE37A4 -> 00410CD0` binding, OS recursive critical section
and common `E188B4` list. Keep owner/list/bindings and all pages live throughout.
Retire all borrowers before corresponding pool/page/manager teardown. Externally
serialize mutation and trim, including common-list use across owners. The
domain excludes aliasing, reentry, concurrent trim/teardown, double-return,
stale generations, OOM/fault/SEH and failed or throwing providers. No catch,
rollback, early holder clear, implicit RAII cleanup or pool startup is added.

Original ABI: ECX is the holder, there is no EDX/stack argument, RET has no
immediate, ESI/EDI are preserved, and no semantic result is specified. The new
void APIs take explicit process references and use ordinary C++ calling
conventions. They establish no Original register/stack/EFLAGS, private CRT,
EH, class-layout, drop-in binary, startup, runtime or game equivalence.

The exact readiness family remains unchanged at
`local/SceneNameNodeHolderReadiness01`; its receipt SHA-256 is
`3437961d2b4ea2ed7c28198f1d4d9c174d89e031322e9425c5a7b5e372e3aa1e`.
All 44 files were checked read-only/single-link and copied byte-for-byte into
the new `local/WholeSceneNameNodeHolderSource01` evidence family. All 24 current
provider and tool/config files are literally identical to readiness. The
comparison records any EOL-only differences separately from byte equality.

One strict MSVC Win32 compile passed with `/MD /W4 /WX /fp:strict /O2 /Gy /GL-`.
The object is 2420 bytes, SHA-256
`ed1eafde813fadc4891c94c34f58563b6935d3ed305346682ca7a2e639d67a6a`.
The complete object was decoded/reviewed: 8 sections; 18 symbols plus 8
auxiliary entries (26 indices); 120 code bytes / 52 instructions in three
COMDATs; six code relocations and one metadata relocation. Each holder Source
body is 58 bytes / 25 instructions; the extra 4-byte / 2-instruction COMDAT is
the inline raw-context accessor. All code, data, directives, debug records,
checksums, symbols/auxiliary bytes, relocations and string-table bytes are
accounted. `.chks64` is retained as opaque compiler metadata, with no inferred
algorithm claim. The full object includes an unreferenced header-emitted
four-byte BSS weak fallback and MD release default-library directives.

The prospective closure froze 119 compiler candidates, 24 current readiness
provider/tool inputs, 68 tool files including `clui.dll`, and three build
inputs before compilation. The actual compiler manifest lists 137 inputs:
109 had own literal precompile pins; 28 were first observed and frozen after
compilation (27 MSVC/SDK headers and `native_physical_provider_pool.hpp`).
This is an explicit timing gap: full prospective coverage was not achieved.
All actual inputs are retained, and all prospective and actual measurements
were unchanged before/after the normal build. No compile replay was used.
This records known direct tools and consumed includes, not a hermetic system,
link-library/environment closure or Original private-CRT reproducibility proof.

The required normal `scripts/build.ps1` run passed `reconstructed_math` and
`tool_tests` (2/2). This translation unit was unregistered for that build;
the result validates the existing checkout. No new test/probe, selected test,
execution of these holder bodies or Native/runtime/game validation was added.
The same-worker original strict receipt and build initial, poll and terminal
receipts are preserved literally with the worker UUID and genuine rollout
prefix. No build or test replay was used.

The final local family receipt excludes only its exact own relative path;
every other nested file is enumerated, hashed and sealed read-only/single-link.
The worker changes only the header, Source, this document and its report.
Ledger, CMake, packet-state and Ghidra annotation/admission remain integrator work.
